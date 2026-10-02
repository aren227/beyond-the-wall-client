param(
    [ValidateSet("debug","release")]
    [string]$BuildType = "release"
)
$ErrorActionPreference = "Stop"

$ProjectName = "beyond-the-wall"

$Preset = "standalone-$BuildType"
$RepoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$OutDir = Join-Path $RepoRoot "build\$Preset"
$DistDir = Join-Path $RepoRoot "dist"

Set-Location $RepoRoot
cmake --preset $Preset
cmake --build --preset $Preset

New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
$ZipPath = Join-Path $DistDir "build-windows-$BuildType.zip"
if (Test-Path $ZipPath) { Remove-Item $ZipPath }
Compress-Archive -Path (Join-Path $OutDir "$ProjectName.exe"),
    (Join-Path $OutDir "resources") -DestinationPath $ZipPath

Write-Host ""
Write-Host "Standalone $BuildType build ready: $ZipPath"
