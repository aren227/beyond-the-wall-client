param(
    [ValidateSet("debug","release")]
    [string]$BuildType = "release"
)
$ErrorActionPreference = "Stop"

if (-not $env:EMSDK) {
    $candidate = Join-Path $env:USERPROFILE "emsdk"
    if (Test-Path $candidate) {
        $env:EMSDK = $candidate
    } else {
        Write-Error "EMSDK not set and $candidate not found. Install emsdk or set `$env:EMSDK."
        exit 1
    }
}
& "$env:EMSDK\emsdk_env.ps1" | Out-Null

$Preset = "web-$BuildType"
$RepoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$OutDir = Join-Path $RepoRoot "build\$Preset"
$DistDir = Join-Path $RepoRoot "dist"

Set-Location $RepoRoot
cmake --preset $Preset
cmake --build --preset $Preset

New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
$ZipPath = Join-Path $DistDir "build-web-$BuildType.zip"
if (Test-Path $ZipPath) { Remove-Item $ZipPath }
Compress-Archive -Path (Join-Path $OutDir "index.html"),
    (Join-Path $OutDir "index.js"),
    (Join-Path $OutDir "index.wasm"),
    (Join-Path $OutDir "index.data") -DestinationPath $ZipPath

Write-Host ""
Write-Host "Web $BuildType build ready: $ZipPath"
