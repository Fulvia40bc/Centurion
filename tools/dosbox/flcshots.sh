#!/bin/bash
# flcshots.sh NAME SLOT_X SLOT_Y COUNT
# DOSBox-X: restore a save from the title menu (slot at SLOT_X,SLOT_Y) and stop COUNT times at fg_showflic's per-frame
# key check (0x9306B, link address; the frame is decoded on the screen there and the delay has not started), taking a
# raw screenshot each time: capture/steps/NAME_j.png, j = 1..COUNT (j = 1 is the prefix frame, which draws nothing).
# Compare with flcframes.py. The run copy's saves are put back afterwards, as in run_steps.sh.
name=$1; sx=$2; sy=$3; count=$4
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
R=/c/work/advciv_run; B=$(mktemp -d)
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $R/aciv.cfg $B/ || exit 1
finish() { taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1
           cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $B/aciv.cfg $R/; rm -rf $B; }
trap finish EXIT
KEY=0x9306B
want=$(printf 'EIP=00%X' $(( KEY + 0x202000 )))
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/steps
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 186 > /dev/null || exit 1
NEXT_BP=$KEY NO_REPOKE=1 bash $P 0x78BD3 $sx $sy > /dev/null || exit 1
for j in $(seq 1 $count); do
  e=""
  for i in $(seq 1 120); do
    s=$(D dbgscreen)
    e=$(echo "$s" | grep -o 'EIP=[0-9A-F]*' | head -1)
    if [ "$e" = "$want" ] && ! echo "$s" | tail -2 | grep -q '(Running)'; then break; fi
    e=""; sleep 1
  done
  [ -z "$e" ] && { echo "no stop for frame $j"; exit 1; }
  rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
  D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
  mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/steps/${name}_$j.png || exit 1
done
echo "$count frames saved"
