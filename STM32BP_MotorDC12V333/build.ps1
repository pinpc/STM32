$ErrorActionPreference = 'Stop'
$Project = Split-Path -Parent $MyInvocation.MyCommand.Path
$ToolRoot = 'C:\ST\STM32CubeCLT_1.23.0\GNU-tools-for-STM32\bin'
$env:Path = "$ToolRoot;$env:Path"
$Build = Join-Path $Project 'build-arm'

cmake -S $Project -B $Build -G Ninja `
  -DCMAKE_SYSTEM_NAME=Generic `
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY `
  -DCMAKE_C_COMPILER_FORCED=TRUE `
  -DCMAKE_C_COMPILER_WORKS=TRUE `
  -DCMAKE_C_COMPILER="$ToolRoot\arm-none-eabi-gcc.exe" `
  -DCMAKE_ASM_COMPILER="$ToolRoot\arm-none-eabi-gcc.exe" `
  -DARM_OBJCOPY="$ToolRoot\arm-none-eabi-objcopy.exe" `
  -DARM_SIZE="$ToolRoot\arm-none-eabi-size.exe"
if ($LASTEXITCODE -ne 0) {
    throw "CMake-Konfiguration fehlgeschlagen (Exitcode $LASTEXITCODE)."
}

cmake --build $Build --parallel
if ($LASTEXITCODE -ne 0) {
    throw "Firmware-Build fehlgeschlagen (Exitcode $LASTEXITCODE)."
}

Write-Host "Firmware erstellt: $Build\STM32BP_MotorDC12V333.hex"
