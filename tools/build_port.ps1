# Build the port's programs for ARM64 and x86_64 Windows with llvm-mingw (libraries from tools\build_deps.ps1).
#   powershell -ExecutionPolicy Bypass -File tools\build_port.ps1 [-Out C:\work\advciv_port]
# Each arch gets its own folder with the DLLs next to the programs, so a running copy is never overwritten by a
# build of the other one. Strict C11 (-Werror -pedantic) for the port's own code.
param(
  [string]$Out       = 'C:\work\advciv_port',
  [string]$Prefix    = 'C:\work\toolchain',
  [string]$Toolchain = 'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\bin'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$src  = Join-Path $repo 'port\src'
$test = Join-Path $repo 'port\test'
$warn = @('-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic')
$gui  = @('-Wl,--subsystem,windows', '-Wl,-e,mainCRTStartup')   # no console window; plat_console() attaches to a terminal

foreach ($arch in 'aarch64', 'x86_64') {
  $cc  = Join-Path $Toolchain "$arch-w64-mingw32-clang.exe"
  $sdl = Join-Path $Prefix "sdl3-3.4.20\$arch"
  $fs  = Join-Path $Prefix "fluidsynth-2.6.2\$arch"
  $o   = Join-Path $Out $arch
  New-Item -ItemType Directory -Force $o | Out-Null
  $fmt = @('catalog.c', 'xmi.c', 'xmiseq.c', 'gif.c') | ForEach-Object { Join-Path $src $_ }
  & $cc @warn "-I$sdl\include" "-I$fs\include" (Join-Path $test 'plat_demo.c') (Join-Path $src 'plat_sdl3.c') `
        (Join-Path $src 'png_write.c') (Join-Path $src 'gm_synth.c') (Join-Path $src 'palette.c') @fmt "$sdl\lib\libSDL3.dll.a" `
        "$fs\lib\libfluidsynth-3.dll.a" @gui -o (Join-Path $o 'plat_demo.exe')
  if ($LASTEXITCODE) { throw "plat_demo ${arch}: build failed" }
  & $cc @warn "-I$fs\include" (Join-Path $test 'gm_render.c') (Join-Path $src 'gm_synth.c') @fmt `
        "$fs\lib\libfluidsynth-3.dll.a" -o (Join-Path $o 'gm_render.exe')
  if ($LASTEXITCODE) { throw "gm_render ${arch}: build failed" }
  & $cc @warn (Join-Path $test 'font_test.c') (Join-Path $src 'fgfont.c') (Join-Path $src 'png_write.c') `
        -o (Join-Path $o 'font_test.exe')
  if ($LASTEXITCODE) { throw "font_test ${arch}: build failed" }
  # dro_play (HANDOFF Task 11): the vendored Nuked-OPL3 is built unchanged; clang needs -Wno-static-in-inline for
  # its 'inline OPL3_Generate4Ch' (gcc does not warn). The strict flags apply to everything else.
  $nuked = Join-Path $src '..\third_party\nuked-opl3'
  & $cc @warn "-I$nuked" -c (Join-Path $nuked 'opl3.c') -Wno-static-in-inline -o (Join-Path $o 'opl3.o')
  if ($LASTEXITCODE) { throw "opl3.c ${arch}: build failed" }
  & $cc @warn "-I$nuked" (Join-Path $test 'dro_play.c') (Join-Path $src 'dro.c') (Join-Path $src 'opl.c') `
        (Join-Path $o 'opl3.o') -o (Join-Path $o 'dro_play.exe')
  if ($LASTEXITCODE) { throw "dro_play ${arch}: build failed" }
  # the port itself (so far: start-up and the title menu)
  $game = @('main.c', 'fg.c', 'ui_text.c', 'ui_dialog.c', 'ui_screen.c', 'title.c', 'state.c', 'le_image.c', 'fgfont.c',
            'gif.c', 'catalog.c', 'palette.c', 'png_write.c', 'plat_sdl3.c', 'ui_widgets.c', 'setup.c', 'variants.c', 'load.c', 'gfxload.c', 'newgame.c', 'choose.c', 'ui_kit2.c', 'ast.c', 'ui_kit3.c', 'dispatch.c', 'nations.c', 'save.c', 'saveload.c', 'map.c', 'mapdata.c', 'panel.c', 'shippanel.c', 'start.c', 'aistart.c', 'info.c', 'menu1.c', 'menu2.c', 'move.c', 'stack.c') | ForEach-Object { Join-Path $src $_ }
  & $cc @warn "-I$sdl\include" @game "$sdl\lib\libSDL3.dll.a" @gui -o (Join-Path $o 'advciv.exe')
  if ($LASTEXITCODE) { throw "advciv ${arch}: build failed" }
  $rt = Join-Path (Split-Path $Toolchain) "$arch-w64-mingw32\bin"
  Copy-Item "$sdl\bin\SDL3.dll", "$fs\bin\libfluidsynth-3.dll", "$rt\libc++.dll", "$rt\libunwind.dll" $o -Force
  Write-Host "built $o"
}
