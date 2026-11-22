param(
    [string]$ArmBin = "C:\ST\STM32CubeCLT_1.23.0\GNU-tools-for-STM32\bin"
)

$gcc = Join-Path $ArmBin "arm-none-eabi-gcc.exe"
$objcopy = Join-Path $ArmBin "arm-none-eabi-objcopy.exe"
$build = Join-Path $PSScriptRoot "build-direct"

if (-not (Test-Path -LiteralPath $gcc)) { throw "ARM-GCC nicht gefunden: $gcc" }
if (-not (Test-Path -LiteralPath $objcopy)) { throw "objcopy nicht gefunden: $objcopy" }

New-Item -ItemType Directory -Force -Path $build | Out-Null
$root = Split-Path -Parent $PSScriptRoot
$drivers = Join-Path $PSScriptRoot "Drivers"
$common = @(
    "-mcpu=cortex-m3", "-mthumb", "-mfloat-abi=soft",
    "-ffunction-sections", "-fdata-sections", "-g3", "-Og", "-Wall", "-Wextra",
    "-DSTM32F10X_MD", "-DUSE_STDPERIPH_DRIVER", "-DHSE_VALUE=8000000",
    "-I$drivers\USER", "-I$drivers\CORE", "-I$drivers\STM32F10x_FWLib\inc"
)

$sources = @(
    "$PSScriptRoot\main.c",
    "$drivers\USER\system_stm32f10x.c",
    "$drivers\STM32F10x_FWLib\src\misc.c",
    "$drivers\STM32F10x_FWLib\src\stm32f10x_gpio.c",
    "$drivers\STM32F10x_FWLib\src\stm32f10x_rcc.c"
)

$objects = @()
foreach ($source in $sources) {
    $object = Join-Path $build (([IO.Path]::GetFileNameWithoutExtension($source)) + ".o")
    & $gcc @common -c $source -o $object
    if ($LASTEXITCODE -ne 0) { throw "Kompilierung fehlgeschlagen: $source" }
    $objects += $object
}

$startupObject = Join-Path $build "startup_stm32f103c8tx.o"
& $gcc @common -c "$PSScriptRoot\startup_stm32f103c8tx.s" -o $startupObject
if ($LASTEXITCODE -ne 0) { throw "Startup-Datei konnte nicht assembliert werden." }
$objects += $startupObject

$elf = Join-Path $build "BluePill_STLink_Test.elf"
$hex = Join-Path $build "BluePill_STLink_Test.hex"
& $gcc @common "-T$PSScriptRoot\STM32F103C8Tx_FLASH.ld" "-Wl,--gc-sections" "-Wl,-Map=$build\BluePill_STLink_Test.map" "--specs=nosys.specs" @objects -o $elf
if ($LASTEXITCODE -ne 0) { throw "Linken fehlgeschlagen." }
& $objcopy -O ihex $elf $hex
if ($LASTEXITCODE -ne 0) { throw "HEX-Erzeugung fehlgeschlagen." }
Write-Host "Firmware erstellt: $hex"
