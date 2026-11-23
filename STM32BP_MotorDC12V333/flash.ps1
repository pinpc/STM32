param(
    [string]$Firmware = (Join-Path $PSScriptRoot 'build-arm\STM32BP_MotorDC12V333.hex'),
    [string]$Programmer = 'C:\ST\STM32CubeCLT_1.23.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe'
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Firmware -PathType Leaf)) {
    throw "Firmware-Datei nicht gefunden: $Firmware`nZuerst .\build.ps1 ausfuehren oder den Pfad mit -Firmware angeben."
}

if (-not (Test-Path -LiteralPath $Programmer -PathType Leaf)) {
    $availableProgrammer = Get-Command 'STM32_Programmer_CLI.exe' -ErrorAction SilentlyContinue
    if ($availableProgrammer) {
        $Programmer = $availableProgrammer.Source
    }
    else {
        throw "STM32_Programmer_CLI.exe nicht gefunden. Pfad mit -Programmer angeben."
    }
}

$Firmware = (Resolve-Path -LiteralPath $Firmware).Path
Write-Host "Flashe $Firmware ueber ST-Link/SWD ..."

& $Programmer -c port=SWD -w $Firmware -v -rst
if ($LASTEXITCODE -ne 0) {
    throw "Flashen fehlgeschlagen (Exitcode $LASTEXITCODE). Pruefe ST-Link, USB und SWDIO/SWCLK-Verbindungen."
}

Write-Host 'Flashen und Verifikation erfolgreich; Target wurde zurueckgesetzt.'
