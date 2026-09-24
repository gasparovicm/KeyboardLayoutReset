# Packs bin\KeyboardLayoutReset.exe into dist\KeyboardLayoutReset-<version>.msix
# for the Microsoft Store. The package is unsigned: the Store signs it after
# certification. Run build.bat first (it calls this script).
$ErrorActionPreference = 'Stop'
$root = Resolve-Path (Join-Path $PSScriptRoot '..')
$exe = Join-Path $root 'bin\KeyboardLayoutReset.exe'

# Store versions need four parts with the last one 0: 0.4.0 -> 0.4.0.0.
$version = (Get-Item $exe).VersionInfo.ProductVersion + '.0'

$stage = Join-Path $root 'obj\msix'
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item $exe $stage
Copy-Item -Recurse (Join-Path $PSScriptRoot 'Assets') $stage
$manifest = Get-Content -Raw -Encoding UTF8 (Join-Path $PSScriptRoot 'AppxManifest.xml')
[IO.File]::WriteAllText((Join-Path $stage 'AppxManifest.xml'), $manifest.Replace('{VERSION}', $version))

$sdk = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin\10.*\x64\makeappx.exe" |
    Sort-Object { [version]$_.Directory.Parent.Name } | Select-Object -Last 1
if (-not $sdk) { throw 'makeappx.exe not found; install the Windows SDK.' }

$out = Join-Path $root "dist\KeyboardLayoutReset-$version.msix"
New-Item -ItemType Directory -Force (Split-Path $out) | Out-Null
& $sdk.FullName pack /o /h SHA256 /d $stage /p $out | Out-Null
if ($LASTEXITCODE) { throw "makeappx failed ($LASTEXITCODE)" }
Write-Host "Built $out"
if ($manifest.Contains('PLACEHOLDER')) {
    Write-Warning 'Package identity is still PLACEHOLDER; set it in packaging\AppxManifest.xml before uploading to the Store.'
}
