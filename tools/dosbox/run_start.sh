#!/bin/bash
# run_start.sh NAME HUMANS "SETUP_CLICKS" "CHOICE_CLICKS" "PLACE_CLICKS"
# DOSBox-X: a new game as run_setup.sh (setup clicks, START GAME, the nation choice; CHOICE_CLICKS as there), then
# the starting territories (module START, 0x454D0): before each human nation's click a raw shot of its screen
# (capture/start/NAME_pN.png), then its click from PLACE_CLICKS ("x,y" per human nation, in nation order) at the
# placement loop's check 0x45705; the computers pick by themselves. Stops at main 0x1188C: the data object is dumped to
# C:\DOSBox-X\advciv\NAME.bin and the screen saved as capture/start/NAME.png. Prints the start seed (for the port's
# --seed) and each nation's starting area (nation +0x9A and the area records' tokens are in the dump).
name=$1; humans=$2; clicks=$3; choice=$4; place=$5
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
shot() { rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png; D menu "Take raw screenshot" > /dev/null; sleep 2
         mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/start/$1.png; }
R=/c/work/advciv_run; B=$(mktemp -d)   # the original autosaves once it runs on past the stop: keep the saves
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $B/ || exit 1
# STOP=LINK_ADDR: stop there instead of main 0x1188C (e.g. 0x54A51, the loop's call of the movement phase, to play
# round 1's first phases); KEEP_SAVES_TO=DIR: keep the saves the original wrote (its autosaves) before restoring
STOP=${STOP:-0x1188C}; WANT=$(printf 'EIP=00%X' $(( STOP + 0x202000 )))
trap 'taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1; [ -n "$KEEP_SAVES_TO" ] && mkdir -p "$KEEP_SAVES_TO" && cp -p $R/SAVGAMEA.DAT $R/savgameb.dat "$KEEP_SAVES_TO"/; cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $R/; rm -rf $B' EXIT
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/start
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 150 > /dev/null || exit 1
for c in $clicks; do bash $P 0x4E171 ${c%,*} ${c#*,} > /dev/null || exit 1; done
bash $P 0x4E171 320 428 > /dev/null || exit 1
sleep 6
IFS='/' read -ra per <<< "$choice"
for i in $(seq 1 $humans); do
  if [[ "$choice" == */* ]]; then cs=${per[$((i-1))]}; else cs=$choice; fi
  for c in $cs; do bash $P 0x2F4EF ${c%,*} ${c#*,} > /dev/null || exit 1; done
  bash $P 0x2F4EF 251 409 > /dev/null || exit 1
done
k=0; last=$(echo $place | wc -w)
for c in $place; do
  k=$((k + 1)); sleep 8                # the computers before this nation pick with 750 + 500 ms waits
  shot ${name}_p$k
  if [ $k -eq $last ]; then NEXT_BP=$STOP bash $P 0x45705 ${c%,*} ${c#*,} > /dev/null || exit 1
  else bash $P 0x45705 ${c%,*} ${c#*,} > /dev/null || exit 1; fi
done
for i in $(seq 1 240); do
  e=$(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1)
  if [ "$e" = "$WANT" ] && ! D dbgscreen | tail -2 | grep -q '(Running)'; then break; fi
  sleep 1
done
[ "$e" = "$WANT" ] || { echo "$name: no stop at $STOP ($e)"; exit 1; }
rm -f /c/DOSBox-X/advciv/$name.bin
D dbg "MEMDUMPBIN 0188:2A9000 72C00 $name.bin" > /dev/null; sleep 2
rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
D dbg "BPDEL *" > /dev/null; D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/start/$name.png
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1
python -I -c "
m=open('C:/DOSBox-X/advciv/$name.bin','rb').read(); t=m[0xF422C-0xB0000:0xF4230-0xB0000]
seed=(t[3]<<24)|(t[0]<<16)|(t[2]<<8)|t[1]
nat=[m[0xF14CC-0xB0000+0xC7*n+0x9A] for n in range(1,10)]
own=[[r for r in range(1,161) if m[0xE31B8-0xB0000+69*r+0x0E+n]] for n in range(1,10)]
print('$name: start seed %08X, final seed %08X, nations +0x9A %s, areas with tokens per nation %s' % (seed, int.from_bytes(m[0xCAC5C-0xB0000:0xCAC60-0xB0000],'little'), nat, own))"
