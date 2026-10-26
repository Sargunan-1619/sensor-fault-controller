# ==============================================================================
# WebAssembly Compilation Script for C11 Sensor Fault Controller
# Compiles ../src/sensor_controller.c directly to freestanding Wasm with 0 dependencies
# ==============================================================================

$ErrorActionPreference = "Stop"

$DemoRoot = $PSScriptRoot
$RepoRoot = Split-Path $DemoRoot -Parent
$ZigExe = "$env:USERPROFILE\.gemini\antigravity-ide\scratch\zig_tools\zig-windows-x86_64-0.13.0\zig.exe"

if (-not (Test-Path $ZigExe)) {
    if (Get-Command zig -ErrorAction SilentlyContinue) {
        $ZigExe = "zig"
    } else {
        Write-Error "Zig compiler not found. Please ensure Zig 0.13+ is installed or in PATH."
        exit 1
    }
}

Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host " Compiling C11 Sensor Controller to Freestanding WebAssembly     " -ForegroundColor Cyan
Write-Host " Compiler: $ZigExe" -ForegroundColor DarkGray
Write-Host "=================================================================" -ForegroundColor Cyan

$OutWasm = Join-Path $DemoRoot "sensor_controller.wasm"
$SrcC = Join-Path $RepoRoot "src\sensor_controller.c"
$BridgeC = Join-Path $DemoRoot "wasm_bridge.c"
$IncC = Join-Path $RepoRoot "include"
$IncDemo = $DemoRoot

& $ZigExe cc -target wasm32-freestanding -O3 -nostdlib "-Wl,--no-entry" -rdynamic "-I$IncC" "-I$IncDemo" $SrcC $BridgeC -o $OutWasm

if ($LASTEXITCODE -eq 0 -and (Test-Path $OutWasm)) {
    $Size = (Get-Item $OutWasm).Length
    Write-Host "[SUCCESS] WebAssembly compiled successfully!" -ForegroundColor Green
    Write-Host "          Output : $OutWasm" -ForegroundColor Green
    Write-Host "          Size   : $Size bytes ($([math]::Round($Size/1024, 2)) KB)" -ForegroundColor Green
} else {
    Write-Error "Compilation failed."
    exit 1
}
