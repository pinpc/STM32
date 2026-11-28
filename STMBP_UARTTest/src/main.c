#include "stm32f1xx_hal.h"

#include <ctype.h>
#include <string.h>

/* Blue Pill STM32F103C8T6
 * USART2 standard pins: PA2 = TX (an TTL-RX), PA3 = RX (an TTL-TX)
 * On-board LED: PC13, active LOW
 */

#define BAUDRATE        115200U
#define LED_ON_TIME_MS  120U
#define COMMAND_SIZE    32U

#define ENABLE_CYCLIC_SEND   1U
#define ENABLE_UART_RESPONSE 1U
#define CYCLIC_SEND_TIME_MS  1000U

static UART_HandleTypeDef huart2;

#if ENABLE_UART_RESPONSE
static uint8_t rx_byte;
static char command[COMMAND_SIZE + 1U] = {0};
static volatile uint8_t command_ready = 0U;
static volatile size_t command_length = 0U;
#endif

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART2_Init(void);
static void Error_Handler(void);

static void uart_print(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text, (uint16_t)strlen(text), 100U);
}

#if ENABLE_UART_RESPONSE
static void blink_connection_led(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(LED_ON_TIME_MS);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

static size_t trim_command(char *cmd, size_t length)
{
    size_t start = 0U;

    while (start < length && isspace((unsigned char)cmd[start])) {
        ++start;
    }

    while (length > start && isspace((unsigned char)cmd[length - 1U])) {
        --length;
    }

    if (start > 0U) {
        memmove(cmd, &cmd[start], length - start);
    }

    cmd[length - start] = '\0';
    return length - start;
}

static int command_is_ping(const char *cmd, size_t length)
{
    size_t index;

    if (length != 4U) {
        return 0;
    }

    for (index = 0U; index < 4U; ++index) {
        if (toupper((unsigned char)cmd[index]) != "PING"[index]) {
            return 0;
        }
    }

    return 1;
}
#endif

int main(void)
{
#if ENABLE_CYCLIC_SEND
    uint32_t last_send_time = 0U;
#endif

    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    USART2_Init();

    HAL_Delay(100U);
    uart_print("UART READY - send PING\r\n");

#if ENABLE_UART_RESPONSE
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
#endif

    for (;;) {
        uint32_t now = HAL_GetTick();

#if ENABLE_CYCLIC_SEND
        if ((now - last_send_time) >= CYCLIC_SEND_TIME_MS) {
            last_send_time = now;
            uart_print("CYCLIC TEST\r\n");
        }
#endif

#if ENABLE_UART_RESPONSE
        if (command_ready) {
            size_t length = trim_command(command, command_length);

            if (command_is_ping(command, length)) {
                uart_print("PONG\r\n");
                blink_connection_led();
            } else if (length > 0U) {
                uart_print("ERROR: send PING\r\n");
            }

            command[0] = '\0';
            command_length = 0U;
            command_ready = 0U;
        }
#endif

        __WFI();
    }
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);
}

static void USART2_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_USART2_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }

#if ENABLE_UART_RESPONSE
    HAL_NVIC_SetPriority(USART2_IRQn, 0U, 0U);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
#endif
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    oscillator.HSIState = RCC_HSI_ON;
    oscillator.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    oscillator.PLL.PLLState = RCC_PLL_ON;
    oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    oscillator.PLL.PLLMUL = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
        Error_Handler();
    }

    clocks.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV2;
    clocks.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

#if ENABLE_UART_RESPONSE
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        if (!command_ready) {
            if (rx_byte == '\n' || rx_byte == '\r') {
                if (command_length > 0U) {
                    command_ready = 1U;
                }
            } else if (command_length < COMMAND_SIZE) {
                command[command_length++] = (char)rx_byte;
                command[command_length] = '\0';
            } else {
                command[0] = '\0';
                command_length = 0U;
            }
        }
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
    }
}
#endif

static void Error_Handler(void)
{
    for (;;) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(250U);
    }
}
