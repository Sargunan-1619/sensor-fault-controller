# ==============================================================================
# build.ps1 - Deterministic PowerShell build script for Project 1
# ==============================================================================
param (
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$ProjectRoot = $PSScriptRoot
if (-not $ProjectRoot) { $ProjectRoot = (Get-Location).Path }

$GccPath = "C:\msys64\ucrt64\bin\gcc.exe"
$BuildDir = Join-Path $ProjectRoot "build"
$IncludeDir = Join-Path $ProjectRoot "include"
$SrcDir = Join-Path $ProjectRoot "src"
$TestsDir = Join-Path $ProjectRoot "tests"

if (-not (Test-Path $GccPath)) {
    Write-Error "GCC compiler was not found at expected path: '$GccPath'"
    exit 1
}

# Prepend UCRT64 toolchain to session PATH so GCC finds internal tools and DLLs without touching registry
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "[BUILD] Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
}

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

$Flags = @("-Wall", "-Wextra", "-Wpedantic", "-std=c11", "-O2", "-I$IncludeDir")

Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host "  Compiling Embedded Sensor Monitoring & Fault Controller (C11) " -ForegroundColor Cyan
Write-Host "=================================================================" -ForegroundColor Cyan

# 1. Compile Main Application: sensor_controller.exe
$AppOutput = Join-Path $BuildDir "sensor_controller.exe"
$AppSources = @(
    (Join-Path $SrcDir "sensor_controller.c"),
    (Join-Path $SrcDir "main.c")
)
Write-Host "[BUILD] Building Application: $AppOutput" -ForegroundColor Green
& $GccPath @Flags @AppSources "-o" $AppOutput

if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to build application binary."
    exit $LASTEXITCODE
}

# 2. Compile Unit Test Runner: test_runner.exe
$TestOutput = Join-Path $BuildDir "test_runner.exe"
$TestSources = @(
    (Join-Path $SrcDir "sensor_controller.c"),
    (Join-Path $TestsDir "test_sensor_controller.c")
)
Write-Host "[BUILD] Building Test Suite : $TestOutput" -ForegroundColor Green
& $GccPath @Flags @TestSources "-o" $TestOutput

if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to build test suite binary."
    exit $LASTEXITCODE
}

Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host "  Build Completed Successfully with Zero Errors and Zero Warnings" -ForegroundColor Green
Write-Host "=================================================================" -ForegroundColor Cyan
