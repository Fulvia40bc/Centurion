#!/bin/bash
# run_restore.sh NAME SLOT_X SLOT_Y
# One DOSBox-X run of the title menu's RESTORE A SAVED GAME (module S1), without focus: starts the game with /NT
# (advciv_nt.conf), clicks RESTORE A SAVED GAME (0x4F6D3), takes a raw shot of the slot dialog (capture/s1/NAME_dialog.png),
# clicks the slot at SLOT_X,SLOT_Y in the dialog's click wait (f78BA4, 0x78BD3) and stops at main 0x115B0 (after the return of
# 0x5FF28 and the flag clear at 0x115A9; the port dumps at the same point). There the data object is dumped to C:\DOSBox-X\advciv\NAME.bin and the screen ("Restoring" + description)
# saved as capture/s1/NAME.png. Slots: 'a' (320,354), 'b' (320,379); CANCEL (320,404).
name=$1; sx=$2; sy=$3
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/s1
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 186 s1/${name}_dialog || exit 1
NEXT_BP=0x115B0 bash $P 0x78BD3 $sx $sy || exit 1
for i in $(seq 1 30); do
  e=$(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1)
  if [ "$e" = "EIP=002135B0" ] && ! D dbgscreen | tail -2 | grep -q '(Running)'; then break; fi
  sleep 1
done
[ "$e" = "EIP=002135B0" ] || { echo "$name: no stop at 0x115B0 ($e)"; exit 1; }
rm -f /c/DOSBox-X/advciv/$name.bin
D dbg "MEMDUMPBIN 0188:2A9000 72C00 $name.bin" > /dev/null; sleep 2
rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/s1/$name.png
python -I -c "
m=open('C:/DOSBox-X/advciv/$name.bin','rb').read()
print('$name: seed %08X, humans %d computers %d, [0xE1624] %02X' % (int.from_bytes(m[0xCAC5C-0xB0000:0xCAC60-0xB0000],'little'), m[0xE1620-0xB0000], m[0xE1621-0xB0000], m[0xE1624-0xB0000]))"
