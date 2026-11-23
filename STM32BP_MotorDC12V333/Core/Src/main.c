#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/*
 * Configure this for the exact encoder and gearbox. 1320 is a working
 * assumption for an 11-edge/rev motor encoder, 30:1 gearbox and TIM3 x4
 * quadrature counting. Check the motor datasheet or calibrate before relying
 * on the displayed RPM.
 */
#define ENCODER_COUNTS_PER_OUTPUT_REV 1320U
#define RPM_SAMPLE_MIN_MS 1U

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
volatile int32_t g_motor_rpm = 0; /* Watch this variable in the debugger. */

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_PWM_Init(void);
static void MX_TIM3_Encoder_Init(void);
static void MotorTask(void *argument);

static int32_t Motor_GetRpm(uint16_t current_count,
                            uint16_t previous_count,
                            uint32_t elapsed_ms)
{
    /* TIM3 is a 16-bit up/down counter; correct the subtraction across wrap. */
    int32_t delta_counts = (int32_t)current_count - (int32_t)previous_count;
    if (delta_counts > 32767) {
        delta_counts -= 65536;
    } else if (delta_counts < -32768) {
        delta_counts += 65536;
    }

    /* Signed counts preserve direction; use 64-bit math to avoid overflow. */
    return (int32_t)(((int64_t)delta_counts * 60000LL) /
                     ((int64_t)ENCODER_COUNTS_PER_OUTPUT_REV * elapsed_ms));
}

/**
 * Set the H-bridge direction and PWM duty using TIM2 compare registers.
 * Positive values drive CH1; negative values drive CH2. The input is
 * clamped to +/-1000, matching TIM2 ARR=999 (approximately 0..100% duty).
 * HAL's compare helper writes TIM2_CCR1/CCR2; zeroing the opposite channel
 * prevents both direction inputs from being PWM-driven at once.
 */
static void Motor_SetSignedPwm(int16_t value)
{
    if (value > 1000) value = 1000;
    if (value < -1000) value = -1000;

    if (value >= 0) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint16_t)value);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0U);
    } else {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0U);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)(-value));
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_PWM_Init();
    MX_TIM3_Encoder_Init();

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET);
    Motor_SetSignedPwm(0); /* safe startup: motor remains stopped */

    xTaskCreate(MotorTask, "Motor", 256, NULL, 2, NULL);
    vTaskStartScheduler();

    for (;;) {}
}

static void MotorTask(void *argument)
{
    (void)argument;
    uint16_t previous_count = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
    uint32_t previous_time = HAL_GetTick();

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));

        uint32_t now = HAL_GetTick();
        uint32_t elapsed_ms = now - previous_time;
        uint16_t current_count = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);

        if (elapsed_ms >= RPM_SAMPLE_MIN_MS) {
            g_motor_rpm = Motor_GetRpm(current_count, previous_count, elapsed_ms);
            previous_count = current_count;
            previous_time = now;
        }

        /* Replace 0 with a signed PWM command after the wiring is verified. */
        Motor_SetSignedPwm(0);
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    /* Select external HSE and multiply it by 9: a 8 MHz crystal gives 72 MHz.
     * HAL programs RCC_CR/RCC_CFGR (and the HSE predivider as applicable).
     */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&osc);

    /* SYSCLK=PLL, AHB=/1, APB1=/2 (36 MHz), APB2=/1 (72 MHz).
     * These selections configure RCC_CFGR; FLASH latency is set for 72 MHz.
     */
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

static void MX_TIM2_PWM_Init(void)
{
    TIM_OC_InitTypeDef oc = {0};
    htim2.Instance = TIM2;
    /* With a 72 MHz TIM2 clock, PSC=71 gives a 1 MHz counter tick;
     * ARR=999 gives a 1 kHz PWM period. HAL writes TIM2_PSC, TIM2_ARR,
     * CCMR1/CCMR2 and CCER when configuring PWM channels.
     */
    htim2.Init.Prescaler = 71;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim2);
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &oc, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim2, &oc, TIM_CHANNEL_2);
}

static void MX_TIM3_Encoder_Init(void)
{
    TIM_Encoder_InitTypeDef encoder = {0};
    htim3.Instance = TIM3;
    /* TIM3 counts both encoder inputs (TI1 and TI2) in encoder mode 3.
     * PSC=0 keeps every qualified edge; ARR=0xFFFF makes CNT wrap at 16 bits.
     * HAL configures TIM3_SMCR, CCMR1, CCER, PSC and ARR.
     */
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    encoder.EncoderMode = TIM_ENCODERMODE_TI12;
    encoder.IC1Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC1Prescaler = TIM_ICPSC_DIV1;
    /* Digital input filter setting 3 is a modest glitch filter; the exact
     * sampling time depends on the timer clock and filter table in the RM0008.
     */
    encoder.IC1Filter = 3;
    encoder.IC2Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC2Prescaler = TIM_ICPSC_DIV1;
    encoder.IC2Filter = 3;
    HAL_TIM_Encoder_Init(&htim3, &encoder);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    /* Enable GPIOA/GPIOB and AFIO peripheral clocks in RCC_APB2ENR. */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* PA0/PA1: alternate-function push-pull, used as TIM2_CH1/CH2 PWM.
     * On STM32F103 the pin mode is encoded in GPIOA_CRL.
     */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA6/PA7: pulled-up inputs, suitable for encoder A/B if wired there.
     * GPIOA_CRL selects input-pull mode and GPIOA_ODR selects pull-up.
     */
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PB0/PB1: low-speed push-pull outputs. ODR is set high in main() before
     * starting with zero PWM; verify that HIGH is the safe/disabled level for
     * the particular motor driver before connecting the motor.
     */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio);
}

void vApplicationMallocFailedHook(void) { taskDISABLE_INTERRUPTS(); for (;;) {} }
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{ (void)task; (void)name; taskDISABLE_INTERRUPTS(); for (;;) {} }

void Error_Handler(void) { __disable_irq(); for (;;) {} }
