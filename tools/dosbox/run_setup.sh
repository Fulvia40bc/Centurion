#!/bin/bash
# run_setup.sh NAME HUMANS "SETUP_CLICKS" ["CHOICE_CLICKS"]
# One DOSBox-X run of a new game with a given setup, to the end of the nation choice (main 0x11854), without focus:
# starts the game with /NT (advciv_nt.conf), clicks START A NEW GAME, then each "x,y" of SETUP_CLICKS on the setup
# screen, START GAME, and SELECT for each of the HUMANS players; the computers then choose by themselves. At 0x11854
# the data object is dumped to C:\DOSBox-X\advciv\NAME.bin and the screen to capture\n3\NAME.png.
# Setup buttons (0xC28B8): humans -/+ (77,108)/(113,108), computers -/+ (77,133)/(113,133).
name=$1; humans=$2; clicks=$3; choice=$4   # CHOICE_CLICKS: "x,y ..." each human makes before SELECT (e.g. the right arrow 376,389);
                                            # "A/B/..." gives each human its own clicks
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/n3
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 150 || exit 1
for c in $clicks; do bash $P 0x4E171 ${c%,*} ${c#*,} || exit 1; done
bash $P 0x4E171 320 428 || exit 1
sleep 6
IFS='/' read -ra per <<< "$choice"   # "A/B/C": each human's own clicks; without '/', the same for every human
for i in $(seq 1 $humans); do
  if [[ "$choice" == */* ]]; then cs=${per[$((i-1))]}; else cs=$choice; fi
  for c in $cs; do bash $P 0x2F4EF ${c%,*} ${c#*,} || exit 1; done
  if [ $i -eq $humans ]; then NEXT_BP=0x11854 bash $P 0x2F4EF 251 409 || exit 1
  else bash $P 0x2F4EF 251 409 || exit 1; fi
done
for i in $(seq 1 40); do   # the computers choose with 750 ms waits each
  e=$(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1)
  if [ "$e" = "EIP=00213854" ] && ! D dbgscreen | tail -2 | grep -q '(Running)'; then break; fi
  sleep 1
done
[ "$e" = "EIP=00213854" ] || { echo "$name: no stop at 0x11854 ($e)"; exit 1; }
rm -f /c/DOSBox-X/advciv/$name.bin
D dbg "MEMDUMPBIN 0188:2A9000 72C00 $name.bin" > /dev/null; sleep 2
rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/n3/$name.png
python -I -c "
import struct
m=open(r'C:\DOSBox-X\advciv\\$name.bin','rb').read(); t=m[0xF422C-0xB0000:0xF4230-0xB0000]
seed=(t[3]<<24)|(t[0]<<16)|(t[2]<<8)|t[1]
nat=[m[0xF14CC-0xB0000+0xC7*n+0x9A] for n in range(1,10)]
print('$name: humans %d computers %d, start seed %08X, nations +0x9A:' % (m[0xE1620-0xB0000], m[0xE1621-0xB0000], seed), nat)"
