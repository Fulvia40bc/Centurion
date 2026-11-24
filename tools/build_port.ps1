# Build the port's programs for ARM64 and x86_64 Windows with llvm-mingw (libraries from tools\build_deps.ps1).
#   powershell -ExecutionPolicy Bypass -File tools\build_port.ps1 [-Out C:\work\advciv_port]
# Each arch gets its own folder with the DLLs next to the programs, so a running copy is never overwritten by a
# build of the other one. Strict C11 (-Werror -pedantic) for the port's own code.
param(
  [string]$Out       = 'C:\work\advciv_port',
  [string]$Prefix    = 'C:\work\toolchain',
  [string]$Toolchain = 'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\bin',
  [string]$Soundfont = 'C:\work\toolchain\soundfont\GeneralUser-GS.sf2'   # copied next to advciv.exe (snd_ail.c's lookup)
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
  # the vendored Nuked-OPL3 is built unchanged; clang needs -Wno-static-in-inline for its 'inline OPL3_Generate4Ch'
  # (gcc does not warn). The strict flags apply to everything else.
  $nuked = Join-Path $src '..\third_party\nuked-opl3'
  & $cc @warn "-I$nuked" -c (Join-Path $nuked 'opl3.c') -Wno-static-in-inline -o (Join-Path $o 'opl3.o')
  if ($LASTEXITCODE) { throw "opl3.c ${arch}: build failed" }
  # ail_test / ail_render (HANDOFF Task 22): the AIL facade on the mixer, the sequencer and the GM and FM synths
  $snd = @('ail.c', 'musplay.c', 'mixer.c', 'wav.c', 'gm_synth.c', 'fm_synth.c', 'fmdrv.c') | ForEach-Object { Join-Path $src $_ }
  $snd += Join-Path $o 'opl3.o'
  & $cc @warn "-I$fs\include" (Join-Path $test 'ail_test.c') @snd @fmt "$fs\lib\libfluidsynth-3.dll.a" `
        -o (Join-Path $o 'ail_test.exe')
  if ($LASTEXITCODE) { throw "ail_test ${arch}: build failed" }
  & $cc @warn "-I$fs\include" (Join-Path $test 'ail_render.c') @snd @fmt "$fs\lib\libfluidsynth-3.dll.a" `
        -o (Join-Path $o 'ail_render.exe')
  if ($LASTEXITCODE) { throw "ail_render ${arch}: build failed" }
  # fm_game_cmp: the stereo-FM music through the game's facade, register for register against the host model
  & $cc @warn "-I$fs\include" (Join-Path $test 'fm_game_cmp.c') (Join-Path $test 'xmi_host.c') @snd @fmt `
        "$fs\lib\libfluidsynth-3.dll.a" -o (Join-Path $o 'fm_game_cmp.exe')
  if ($LASTEXITCODE) { throw "fm_game_cmp ${arch}: build failed" }
  # xmiseq_cmp_test (HANDOFF Task 22c): xmiseq.c against the host's XMIDI interpreter (xmi_host.c), no files needed
  & $cc @warn (Join-Path $test 'xmiseq_cmp_test.c') (Join-Path $test 'xmi_host.c') (Join-Path $src 'xmi.c') `
        (Join-Path $src 'xmiseq.c') -o (Join-Path $o 'xmiseq_cmp_test.exe')
  if ($LASTEXITCODE) { throw "xmiseq_cmp_test ${arch}: build failed" }
  # fm_dro_cmp: SBPRO2.MDI's translation (fmdrv.c) driven like AIL's host, against a DOSBox-X OPL capture
  & $cc @warn (Join-Path $test 'fm_dro_cmp.c') (Join-Path $src 'fmdrv.c') (Join-Path $src 'dro.c') `
        (Join-Path $src 'xmi.c') (Join-Path $src 'catalog.c') -o (Join-Path $o 'fm_dro_cmp.exe')
  if ($LASTEXITCODE) { throw "fm_dro_cmp ${arch}: build failed" }
  & $cc @warn (Join-Path $test 'font_test.c') (Join-Path $src 'fgfont.c') (Join-Path $src 'png_write.c') `
        -o (Join-Path $o 'font_test.exe')
  if ($LASTEXITCODE) { throw "font_test ${arch}: build failed" }
  # showflic_test (HANDOFF Task 17): self-contained plat stubs, no SDL needed
  & $cc @warn "-I$src" (Join-Path $test 'showflic_test.c') (Join-Path $src 'fg.c') (Join-Path $src 'gif.c') (Join-Path $src 'catalog.c') `
        (Join-Path $src 'fgfont.c') (Join-Path $src 'palette.c') (Join-Path $src 'png_write.c') (Join-Path $src 'flc.c') `
        -o (Join-Path $o 'showflic_test.exe')
  if ($LASTEXITCODE) { throw "showflic_test ${arch}: build failed" }
  # dro_play (HANDOFF Task 11), on the Nuked core built above
  & $cc @warn "-I$nuked" (Join-Path $test 'dro_play.c') (Join-Path $src 'dro.c') (Join-Path $src 'opl.c') `
        (Join-Path $o 'opl3.o') -o (Join-Path $o 'dro_play.exe')
  if ($LASTEXITCODE) { throw "dro_play ${arch}: build failed" }
  # the port itself, with its sound (snd.c / snd_ail.c on Mistral's ail.c, mixer, sequencer and GM synth)
  $game = @('main.c', 'fg.c', 'ui_text.c', 'ui_dialog.c', 'ui_screen.c', 'title.c', 'state.c', 'le_image.c', 'fgfont.c',
            'gif.c', 'catalog.c', 'palette.c', 'png_write.c', 'plat_sdl3.c', 'ui_widgets.c', 'setup.c', 'variants.c', 'load.c', 'gfxload.c', 'newgame.c', 'choose.c', 'ui_kit2.c', 'ast.c', 'ui_kit3.c', 'dispatch.c', 'nations.c', 'save.c', 'saveload.c', 'map.c', 'mapdata.c', 'panel.c', 'shippanel.c', 'start.c', 'aistart.c', 'info.c', 'menu1.c', 'menu2.c', 'move.c', 'stack.c', 'savewrite.c', 'rules.c', 'kit.c', 'wqsort.c', 'mstack.c', 'route.c', 'junk.c', 'loop.c', 'tax.c', 'pop.c', 'census.c', 'ship.c', 'movephase.c', 'conflict.c', 'city.c', 'formulas.c', 'cards.c', 'cal1.c', 'mono.c', 'adv.c', 'helpers2.c', 'trade.c', 'cal2.c', 'cal3.c', 'cal4.c', 'cal5.c', 'end.c', 'anim.c', 'aiq.c', 'aicity.c', 'aisea.c', 'aimove.c', 'aiplan.c', 'aiship.c', 'aitrade.c', 'aiadv.c', 'aicards.c', 'aical1.c', 'aical2.c',
            'snd.c', 'snd_ail.c', 'ail.c', 'musplay.c', 'mixer.c', 'wav.c', 'gm_synth.c', 'fm_synth.c', 'fmdrv.c', 'xmi.c',
            'xmiseq.c') | ForEach-Object { Join-Path $src $_ }
  $game += Join-Path $o 'opl3.o'
  & $cc @warn "-I$sdl\include" "-I$fs\include" @game "$sdl\lib\libSDL3.dll.a" "$fs\lib\libfluidsynth-3.dll.a" @gui `
        -o (Join-Path $o 'advciv.exe')
  if ($LASTEXITCODE) { throw "advciv ${arch}: build failed" }
  $rt = Join-Path (Split-Path $Toolchain) "$arch-w64-mingw32\bin"
  Copy-Item "$sdl\bin\SDL3.dll", "$fs\bin\libfluidsynth-3.dll", "$rt\libc++.dll", "$rt\libunwind.dll" $o -Force
  if (Test-Path $Soundfont) { Copy-Item $Soundfont $o -Force } else { Write-Host "no SoundFont at ${Soundfont}: the music is silent" }
  Write-Host "built $o"
}
