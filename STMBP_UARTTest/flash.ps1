[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$environment = 'bluepill_f103c8'

function Find-PlatformIO {
    $command = Get-Command pio.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $defaultPath = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\pio.exe'
    if (Test-Path -LiteralPath $defaultPath) {
        return $defaultPath
    }

    throw 'PlatformIO wurde nicht gefunden. Installiere PlatformIO oder füge pio.exe zum PATH hinzu.'
}

$pio = Find-PlatformIO
Push-Location $projectRoot
try {
    Write-Host "Baue und flashe $environment über ST-LINK ..." -ForegroundColor Cyan
    & $pio run -e $environment -t upload
    if ($LASTEXITCODE -ne 0) {
        throw "PlatformIO flash ist mit Exit-Code $LASTEXITCODE fehlgeschlagen."
    }
}
finally {
    Pop-Location
}

Write-Host "Flash erfolgreich: $environment" -ForegroundColor Green
