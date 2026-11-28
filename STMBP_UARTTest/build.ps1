[CmdletBinding()]
param(
    [switch]$Clean
)

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
    if ($Clean) {
        & $pio run -e $environment -t clean
        if ($LASTEXITCODE -ne 0) {
            throw "PlatformIO clean ist mit Exit-Code $LASTEXITCODE fehlgeschlagen."
        }
    }

    & $pio run -e $environment
    if ($LASTEXITCODE -ne 0) {
        throw "PlatformIO build ist mit Exit-Code $LASTEXITCODE fehlgeschlagen."
    }
}
finally {
    Pop-Location
}

Write-Host "Build erfolgreich: $environment" -ForegroundColor Green
