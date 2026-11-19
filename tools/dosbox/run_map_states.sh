#!/bin/bash
# run_map_states.sh NAME SLOT_X SLOT_Y "X,Y ..."
# DOSBox-X: restore a save (as run_restored_screen.sh), stop at the restored game's first click wait (f78BA4's check
# 0x78BD3) and save NAME_0 (dump C:\DOSBox-X\advciv\NAME_0.bin, raw shot capture/map/NAME_0.png); then for each
# "X,Y" click it there and save NAME_1, NAME_2, ... at the next click wait. E.g. TOGGLE UNITS (600,359) four times
# walks the map detail level [0xC1A34] through all four values (module MAP1). The run copy's saves are kept.
name=$1; sx=$2; sy=$3; clicks=$4
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
R=/c/work/advciv_run; B=$(mktemp -d)
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $B/ || exit 1
# KEEP_SAVES_TO=DIR: copy the saves the original wrote during the run (its autosaves) there before the run copy's
# own are put back, e.g. to get a save at the start of a later phase
finish() { taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1
           [ -n "$KEEP_SAVES_TO" ] && mkdir -p "$KEEP_SAVES_TO" && cp -p $R/SAVGAMEA.DAT $R/savgameb.dat "$KEEP_SAVES_TO"/
           cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $R/; rm -rf $B; }
wait_stop() {   # wait for the debugger to stop at 0x78BD3 (run time 0x27ABD3)
  for i in $(seq 1 90); do
    e=$(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1)
    if [ "$e" = "EIP=0027ABD3" ] && ! D dbgscreen | tail -2 | grep -q '(Running)'; then return 0; fi
    sleep 1
  done
  echo "no stop at 0x78BD3 ($e)"; return 1
}
save_state() {   # dump and shot while stopped; leaves the emulation running
  rm -f /c/DOSBox-X/advciv/$1.bin
  D dbg "MEMDUMPBIN 0188:2A9000 72C00 $1.bin" > /dev/null; sleep 2
  rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
  D dbg "BPDEL *" > /dev/null
  D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
  mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/map/$1.png
  python -I -c "
m=open('C:/DOSBox-X/advciv/$1.bin','rb').read()
g=lambda a,n=4: int.from_bytes(m[a-0xB0000:a-0xB0000+n],'little',signed=True)
print('$1: scroll col %d row %d, detail %d, current %d, phase %d, mode %d' % (g(0xF4884), g(0xF4888), g(0xC1A34,1), g(0xF40F8), g(0xF4102,1), g(0x108804,1)))"
}
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/map
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 186 > /dev/null || { finish; exit 1; }
NEXT_BP=0x78BD3 bash $P 0x78BD3 $sx $sy > /dev/null || { finish; exit 1; }
wait_stop || { finish; exit 1; }
# POKE_DETAIL=n: set the detail level [0xC1A34] (run time 0x2BAA34) to n at the first stop, for the levels 1 and 2
# that TOGGLE UNITS never gives (it only swaps 3 and 0); NAME_0's frame is then still drawn at the old level
[ -n "$POKE_DETAIL" ] && D dbg "SM 0188:2BAA34 0$POKE_DETAIL" > /dev/null
save_state ${name}_0
k=0
for c in $clicks; do
  k=$((k + 1))
  NEXT_BP=0x78BD3 bash $P 0x78BD3 ${c%,*} ${c#*,} > /dev/null || { finish; exit 1; }
  wait_stop || { finish; exit 1; }
  save_state ${name}_$k
done
finish
