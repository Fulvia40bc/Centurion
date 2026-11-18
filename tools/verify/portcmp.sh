#!/bin/bash
# portcmp.sh NAME HUMANS "SETUP_CLICKS" "CHOICE_CLICKS" [port args, e.g. --original]
# The port side of tools/dosbox/run_setup.sh: runs advciv.exe (ARM64 build) with the start seed taken from DOSBox-X's
# NAME.bin and the same clicks (START A NEW GAME, the setup clicks, START GAME, per human the choice clicks and
# SELECT; "A/B/..." gives each human its own clicks), to main 0x11854. It compares the frame with capture/n3/NAME.png
# (pointer at 320,240 as in the capture) and the data object with NAME.bin (dscompare.py, by_design.txt), and prints
# the nations' +0x9A bytes. Output files go to $OUT (default C:/work/advciv_cmp).
name=$1; humans=$2; clicks=$3; choice=$4; shift 4
S=${OUT:-C:/work/advciv_cmp}; mkdir -p "$S"
seed=$(python -I -c "
m=open('C:/DOSBox-X/advciv/$name.bin','rb').read(); t=m[0xF422C-0xB0000:0xF4230-0xB0000]
print('%08X'%((t[3]<<24)|(t[0]<<16)|(t[2]<<8)|t[1]))")
args=(--click 320 150); for c in $clicks; do args+=(--click ${c%,*} ${c#*,}); done; args+=(--click 320 428)
IFS='/' read -ra per <<< "$choice"
for i in $(seq 0 $((humans-1))); do
  if [[ "$choice" == */* ]]; then cs=${per[$i]}; else cs=$choice; fi
  for c in $cs; do args+=(--click ${c%,*} ${c#*,}); done; args+=(--click 251 409)
done
/c/work/advciv_port/aarch64/advciv.exe C:/work/advciv_run --seed $seed "${args[@]}" "$@" --dump-ds $S/p_$name.bin --headless-frame $S/p_$name.png --pointer 320 240 2>&1 | tail -1
echo "frame: $(python -I /c/work/AdvCiv/tools/verify/diffpng.py /c/DOSBox-X/advciv/capture/n3/$name.png $S/p_$name.png 2>&1 | tail -3)"
python -I /c/work/AdvCiv/tools/verify/dscompare.py /c/DOSBox-X/advciv/$name.bin $S/p_$name.bin /c/work/AdvCiv/tools/verify/by_design.txt 2>&1 | grep -v "by design:" | tail -12
python -I -c "
m=open(r'$S/p_$name.bin','rb').read()
print('port $name nations +0x9A:', [m[0xF14CC-0xB0000+0xC7*n+0x9A] for n in range(1,10)])"
