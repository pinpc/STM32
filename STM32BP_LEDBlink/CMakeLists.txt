cmake_minimum_required(VERSION 3.20)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(BluePill_STLink_Test C ASM)


set(STM32_ROOT "${CMAKE_CURRENT_LIST_DIR}/Drivers")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(BluePill_STLink_Test
    main.c
    ${STM32_ROOT}/USER/system_stm32f10x.c
    startup_stm32f103c8tx.s
    ${STM32_ROOT}/STM32F10x_FWLib/src/misc.c
    ${STM32_ROOT}/STM32F10x_FWLib/src/stm32f10x_gpio.c
    ${STM32_ROOT}/STM32F10x_FWLib/src/stm32f10x_rcc.c
)

target_include_directories(BluePill_STLink_Test PRIVATE
    ${STM32_ROOT}/USER
    ${STM32_ROOT}/CORE
    ${STM32_ROOT}/STM32F10x_FWLib/inc
)

target_compile_definitions(BluePill_STLink_Test PRIVATE
    STM32F10X_MD
    USE_STDPERIPH_DRIVER
    HSE_VALUE=8000000
)

target_compile_options(BluePill_STLink_Test PRIVATE
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
    -ffunction-sections -fdata-sections -g3 -Og -Wall -Wextra
)

target_link_options(BluePill_STLink_Test PRIVATE
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
    -T${CMAKE_CURRENT_LIST_DIR}/STM32F103C8Tx_FLASH.ld
    -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/BluePill_STLink_Test.map
    --specs=nosys.specs
)

add_custom_command(TARGET BluePill_STLink_Test POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:BluePill_STLink_Test> ${CMAKE_CURRENT_BINARY_DIR}/BluePill_STLink_Test.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:BluePill_STLink_Test> ${CMAKE_CURRENT_BINARY_DIR}/BluePill_STLink_Test.bin
    COMMENT "Creating HEX and BIN images"
)
