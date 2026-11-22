param(
    [string]$Programmer = "C:\ST\STM32CubeCLT_1.23.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    [string]$Image = "$PSScriptRoot\build-direct\BluePill_STLink_Test.hex"
)

if (-not (Test-Path -LiteralPath $Programmer)) { throw "STM32_Programmer_CLI.exe nicht gefunden: $Programmer" }
if (-not (Test-Path -LiteralPath $Image)) { throw "Firmware nicht gefunden. Erst cmake --build build ausführen: $Image" }

& $Programmer -c port=SWD -w $Image -v -rst
if ($LASTEXITCODE -ne 0) { throw "Flash-Vorgang fehlgeschlagen (Exit-Code $LASTEXITCODE)." }
