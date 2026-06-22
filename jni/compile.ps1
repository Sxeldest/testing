$ErrorActionPreference = "Stop"

Write-Host "Compiling libSAMP..." -ForegroundColor Cyan

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
if (!$scriptDir) { $scriptDir = Get-Location }

# List of potential NDK locations
$ndkBuildPaths = New-Object System.Collections.Generic.List[string]

if ($env:ANDROID_NDK_HOME) { $ndkBuildPaths.Add((Join-Path $env:ANDROID_NDK_HOME "ndk-build.cmd")) }
if ($env:ANDROID_NDK_ROOT) { $ndkBuildPaths.Add((Join-Path $env:ANDROID_NDK_ROOT "ndk-build.cmd")) }

$commonPaths = @(
    "Android/Sdk/ndk/27.0.12077973/ndk-build.cmd",
    "Android/Sdk/ndk/26.2.11394342/ndk-build.cmd",
    "Android/Sdk/ndk/25.1.8937393/ndk-build.cmd",
    "Android/Sdk/ndk/21.4.7075529/ndk-build.cmd"
)

foreach ($cp in $commonPaths) {
    $ndkBuildPaths.Add((Join-Path $env:LOCALAPPDATA $cp))
}

$ndkBuild = $null
foreach ($path in $ndkBuildPaths) {
    if (Test-Path $path) {
        $ndkBuild = $path
        break
    }
}

if (!$ndkBuild) {
    Write-Host "ERRO: Nao encontrei o ndk-build.cmd." -ForegroundColor Red
    Write-Host "Defina ANDROID_NDK_HOME atau pastikan NDK terinstall di LOCALAPPDATA."
    exit 1
}

Write-Host "Usando NDK: $ndkBuild" -ForegroundColor Green

# Run compilation
Push-Location $scriptDir
try {
    & $ndkBuild
} catch {
    Write-Error "Falha ao menjalankan ndk-build."
    Pop-Location
    exit 1
}
Pop-Location

# Copying the library
Write-Host "Copiando a biblioteca (mover)..." -ForegroundColor Cyan
$src = Join-Path $scriptDir "libs/armeabi-v7a/libSAMP.so"
$destDir = Join-Path $scriptDir "../app/src/main/jniLibs/armeabi-v7a/"

if (!(Test-Path $src)) {
    Write-Warning "Arquivo libSAMP.so nao encontrado em $src."
    exit 1
}

if (!(Test-Path $destDir)) {
    New-Item -ItemType Directory -Path $destDir -Force | Out-Null
}

Copy-Item -Path $src -Destination $destDir -Force

Write-Host "OK: Compilacao e copia concluidas!" -ForegroundColor Green