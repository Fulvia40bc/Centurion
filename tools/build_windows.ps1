# Build the native Windows port and refresh the play folder.
#   powershell -ExecutionPolicy Bypass -File tools\build_windows.ps1
# Toolchain (unpacked once into C:\work\toolchain):
#   llvm-mingw-20261104-ucrt-aarch64 (github.com/mstorsjo/llvm-mingw) and SDL2-devel-2.32.10-mingw (github.com/libsdl-org/SDL).
# Builds an x86_64 exe (the SDL2 mingw package has no ARM64 libs); Windows on ARM runs it via emulation.
param(
  [string]$Toolchain = 'C:\work\toolchain',
  [string]$Play      = 'C:\work\Centurion-native',
  [string]$GameDir   = 'C:\Users\erikk\Desktop\Centurion-Defender-of-Rome_DOS_EN\centurion-defender-of-rome'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$cc   = Join-Path $Toolchain 'llvm-mingw-20261104-ucrt-aarch64\bin\x86_64-w64-mingw32-clang.exe'
$sdl  = Join-Path $Toolchain 'SDL2-2.32.10\x86_64-w64-mingw32'
New-Item -ItemType Directory -Force $Play | Out-Null
$src = Get-ChildItem (Join-Path $repo 'port\src\*.c') | ForEach-Object FullName
& $cc -O2 -Wall -Wextra -std=c11 "-I$sdl\include" @src "-L$sdl\lib" -lSDL2 -o (Join-Path $Play 'centurion.exe')
if ($LASTEXITCODE) { throw "compile failed ($LASTEXITCODE)" }
Copy-Item (Join-Path $sdl 'bin\SDL2.dll'), (Join-Path $repo 'port\cmap.bin') $Play -Force
# game data: copies only, the original folder is never modified
Copy-Item (Join-Path $GameDir '*.DAT'), (Join-Path $GameDir '*.DIR'), (Join-Path $GameDir 'OVL0.OVL') $Play -Force
Write-Host "Built $Play\centurion.exe"
