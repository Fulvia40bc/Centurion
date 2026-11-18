#!/bin/bash
# run_restored_screen.sh NAME SLOT_X SLOT_Y [LOOP_LINK_ADDR]
# DOSBox-X: restore a save from the title menu (as run_restore.sh) and let the original run on into the game until it
# first reaches the click-wait check LOOP_LINK_ADDR (default 0x78BD3, f78BA4's), i.e. the game screen of the restored
# game, waiting for the player. There the data object is dumped to C:\DOSBox-X\advciv\NAME.bin and the screen saved
# as capture/map/NAME.png: the reference for module MAP1's map-window check (docs/MAP_PLAN.md).
# The game may rewrite savgamea.dat/savgameb.dat once it runs on: they are backed up first and put back afterwards.
name=$1; sx=$2; sy=$3; loop=${4:-0x78BD3}
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
R=/c/work/advciv_run; B=$(mktemp -d)
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $B/ || exit 1
restore_saves() { cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $R/; rm -rf $B; }
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/map
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 186 || { restore_saves; exit 1; }
want=$(printf 'EIP=00%X' $(( loop + 0x202000 )))
NEXT_BP=$loop bash $P 0x78BD3 $sx $sy || { restore_saves; exit 1; }
for i in $(seq 1 90); do
  e=$(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1)
  if [ "$e" = "$want" ] && ! D dbgscreen | tail -2 | grep -q '(Running)'; then break; fi
  sleep 1
done
if [ "$e" != "$want" ]; then
  echo "$name: no stop at $loop ($e)"
  rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png; D menu "Take raw screenshot" > /dev/null; sleep 2
  mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/map/${name}_nostop.png
  taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; restore_saves; exit 1
fi
rm -f /c/DOSBox-X/advciv/$name.bin
D dbg "MEMDUMPBIN 0188:2A9000 72C00 $name.bin" > /dev/null; sleep 2
rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
D dbg "BPDEL *" > /dev/null
D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/map/$name.png
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1
restore_saves
python -I -c "
m=open('C:/DOSBox-X/advciv/$name.bin','rb').read()
g=lambda a,n=4,s=True: int.from_bytes(m[a-0xB0000:a-0xB0000+n],'little',signed=s)
print('$name: seed %08X, scroll col %d row %d, detail [0xC1A34] %d, current [0xF40F8] %d, phase [0xF4102] %d' % (g(0xCAC5C,4,False), g(0xF4884), g(0xF4888), g(0xC1A34,1), g(0xF40F8), g(0xF4102,1)))"
