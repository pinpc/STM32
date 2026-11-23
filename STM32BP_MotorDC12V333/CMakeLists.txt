cmake_minimum_required(VERSION 3.20)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(STM32BP_MotorDC12V333 C ASM)

set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -ffunction-sections -fdata-sections -g3 -Og")
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -g3")
find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

set(HAL_SOURCES
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c)
file(GLOB FREERTOS_SOURCES Middlewares/FreeRTOS/*.c)
add_executable(${PROJECT_NAME}
    Core/Src/main.c
    Core/Src/system_stm32f1xx.c
    Core/Src/startup_stm32f103xb.s
    ${HAL_SOURCES}
    ${FREERTOS_SOURCES}
    Middlewares/FreeRTOS/portable/GCC/ARM_CM3/port.c
    Middlewares/FreeRTOS/portable/MemMang/heap_4.c)

target_compile_definitions(${PROJECT_NAME} PRIVATE STM32F103xB USE_HAL_DRIVER)
target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc Drivers/CMSIS/Include Drivers/CMSIS/Core/Include
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/STM32F1xx_HAL_Driver/Inc Middlewares/FreeRTOS/include
    Middlewares/FreeRTOS/portable/GCC/ARM_CM3)
target_link_options(${PROJECT_NAME} PRIVATE "-T${CMAKE_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld" "-Wl,--gc-sections" "-Wl,-Map=${PROJECT_NAME}.map" "-specs=nosys.specs")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${ARM_SIZE} $<TARGET_FILE:${PROJECT_NAME}>)
