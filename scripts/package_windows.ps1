param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [Parameter(Mandatory = $true)][string]$MingwBin,
    [string]$OutputDir = "dist/CinemaBooking-Windows-x64"
)
$ErrorActionPreference = 'Stop'
$build = (Resolve-Path -LiteralPath $BuildDir).Path
$qt = (Resolve-Path -LiteralPath $QtRoot).Path
$mingw = (Resolve-Path -LiteralPath $MingwBin).Path
$exe = Join-Path $build 'cinema_booking_qt.exe'
$deploy = Join-Path $qt 'bin/windeployqt.exe'
if (-not (Test-Path -LiteralPath $exe) -or -not (Test-Path -LiteralPath $deploy)) {
    throw 'Build Qt Release and provide the matching Qt root before packaging.'
}
$out = [IO.Path]::GetFullPath($OutputDir)
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Path $out | Out-Null
Copy-Item -LiteralPath $exe -Destination $out
$oldPath = $env:Path
try {
    $env:Path = "$(Join-Path $qt 'bin');$mingw;$oldPath"
    & $deploy --release --no-translations --dir $out (Join-Path $out 'cinema_booking_qt.exe')
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }
} finally { $env:Path = $oldPath }
foreach ($name in @('libstdc++-6.dll', 'libgcc_s_seh-1.dll', 'libwinpthread-1.dll')) {
    $source = Join-Path $mingw $name
    if (-not (Test-Path -LiteralPath $source)) { throw "Missing MinGW runtime: $name" }
    Copy-Item -LiteralPath $source -Destination $out -Force
}
if (-not (Test-Path -LiteralPath (Join-Path $out 'platforms/qwindows.dll'))) {
    throw 'windeployqt did not include the Windows platform plugin.'
}
# Launch with a restricted PATH to catch missing app-local runtime files.
$savedPath = $env:Path
try {
    $env:Path = "$env:SystemRoot/System32;$env:SystemRoot"
    $process = Start-Process -FilePath (Join-Path $out 'cinema_booking_qt.exe') `
        -WorkingDirectory $out -WindowStyle Hidden -PassThru
    if ($process.WaitForExit(3000)) {
        throw "Packaged GUI exited early with code $($process.ExitCode)."
    }
    Stop-Process -Id $process.Id
} finally { $env:Path = $savedPath }
# Keep only deployment files. User data belongs in AppData or a selected folder.
$zip = "$out.zip"
if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force }
Compress-Archive -Path $out -DestinationPath $zip -CompressionLevel Optimal
Write-Output "PACKAGE=$out"
Write-Output "ZIP=$zip"
