# Fetch and build the port's third-party libraries for ARM64 and x86_64 Windows with llvm-mingw.
#   powershell -ExecutionPolicy Bypass -File tools\build_deps.ps1
# Every download is pinned by version and SHA-256. Nothing here goes into git (binaries and the SoundFont live under
# $Prefix). Licences: SDL3 zlib; FluidSynth LGPL-2.1 (shipped as its own DLL); GeneralUser GS (S. Christian Collins)
# free for use in software projects, see soundfont\LICENSE.txt; libc++/libunwind Apache-2.0 with LLVM exception.
# Result:
#   $Prefix\sdl3-3.4.20\<arch>\{bin\SDL3.dll, include, lib}
#   $Prefix\fluidsynth-2.6.2\<arch>\{bin\libfluidsynth-3.dll, include, lib}
#   $Prefix\soundfont\GeneralUser-GS.sf2
param(
  [string]$Prefix    = 'C:\work\toolchain',
  [string]$Toolchain = 'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\bin'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$deps = Join-Path $Prefix 'deps'
New-Item -ItemType Directory -Force $deps, (Join-Path $deps 'build'), (Join-Path $Prefix 'soundfont') | Out-Null

$gu = '684543d5e5efaef08d02be50dcda8d552478fa60'   # GeneralUser-GS commit, 2026-02-23 ("update documentation to r6")
$files = @(
  @{ url = 'https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz'
     out = Join-Path $deps 'SDL3-3.4.20.tar.gz'
     sha = 'f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f' },
  @{ url = 'https://github.com/FluidSynth/fluidsynth/archive/refs/tags/v2.6.2.tar.gz'
     out = Join-Path $deps 'fluidsynth-2.6.2.tar.gz'
     sha = 'ca376b42a3c1b186a4c8c248931789a455ac94a89b777394969d678010c2f91f' },
  @{ url = "https://github.com/mrbumpy409/GeneralUser-GS/raw/$gu/GeneralUser-GS.sf2"
     out = Join-Path $Prefix 'soundfont\GeneralUser-GS.sf2'
     sha = '9575028c7a1f589f5770fccc8cff2734566af40cd26ed836944e9a5152688cfe' },
  @{ url = "https://raw.githubusercontent.com/mrbumpy409/GeneralUser-GS/$gu/documentation/LICENSE.txt"
     out = Join-Path $Prefix 'soundfont\LICENSE.txt'
     sha = $null }
)
foreach ($f in $files) {
  if (-not (Test-Path $f.out)) {
    Write-Host "fetch $($f.url)"
    Invoke-WebRequest -Uri $f.url -OutFile $f.out -UseBasicParsing
  }
  if ($f.sha) {
    $h = (Get-FileHash -Algorithm SHA256 $f.out).Hash.ToLower()
    if ($h -ne $f.sha) { throw "SHA-256 mismatch for $($f.out): $h (want $($f.sha))" }
  }
}

# CMake and Ninja from PyPI (user site), pinned
python -m pip install --user --quiet cmake==4.4.5 ninja==1.13.2
if ($LASTEXITCODE) { throw "pip install cmake/ninja failed ($LASTEXITCODE)" }
$scripts = python -c "import sysconfig; print(sysconfig.get_path('scripts', 'nt_user'))"
$env:PATH = "$scripts;$env:PATH"

Push-Location $deps
try {
  foreach ($t in 'SDL3-3.4.20', 'fluidsynth-2.6.2') {
    if (-not (Test-Path $t)) { tar xzf "$t.tar.gz"; if ($LASTEXITCODE) { throw "tar $t failed" } }
  }
} finally { Pop-Location }

function Build([string]$name, [string]$src, [string]$arch, [string[]]$opts) {
  $b = Join-Path $deps "build\$name-$arch"
  $inst = Join-Path $Prefix "$name\$arch"
  $tc = Join-Path $repo "tools\cmake\llvm-mingw-$arch.cmake"
  & cmake -G Ninja -S (Join-Path $deps $src) -B $b "-DCMAKE_TOOLCHAIN_FILE=$tc" "-DLLVM_MINGW=$($Toolchain -replace '\\','/')" `
          -DCMAKE_BUILD_TYPE=Release "-DCMAKE_INSTALL_PREFIX=$inst" @opts
  if ($LASTEXITCODE) { throw "${name} ${arch}: configure failed" }
  & cmake --build $b
  if ($LASTEXITCODE) { throw "${name} ${arch}: build failed" }
  & cmake --install $b | Out-Null
  if ($LASTEXITCODE) { throw "${name} ${arch}: install failed" }
}

$fsOff = 'alsa','aufile','dbus','jack','ladspa','libsndfile','midishare','network','oss','dsound','wasapi','waveout',
         'winmidi','sdl3','pulseaudio','pipewire','readline','openmp','portaudio' | ForEach-Object { "-Denable-$_=off" }
foreach ($arch in 'aarch64', 'x86_64') {
  Build 'sdl3-3.4.20' 'SDL3-3.4.20' $arch @('-DSDL_SHARED=ON', '-DSDL_STATIC=OFF', '-DSDL_TEST_LIBRARY=OFF',
                                            '-DSDL_TESTS=OFF', '-DSDL_EXAMPLES=OFF')
  # FluidSynth only renders into the port's buffers: no audio/MIDI drivers, no GLib (osal cpp11), single-threaded use
  Build 'fluidsynth-2.6.2' 'fluidsynth-2.6.2' $arch (@('-DBUILD_SHARED_LIBS=ON', '-Dosal=cpp11') + $fsOff)
}
Write-Host "Done. Runtime DLLs per arch: SDL3.dll, libfluidsynth-3.dll, and libc++.dll + libunwind.dll from"
Write-Host "  $((Split-Path $Toolchain))\<arch>-w64-mingw32\bin"
