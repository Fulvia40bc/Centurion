#!/bin/bash
# run_steps.sh NAME SLOT_X SLOT_Y "ADDR:X,Y ..." [DUMP_FROM]
# DOSBox-X: restore a save from the title menu (slot at SLOT_X,SLOT_Y), then play the steps: each "ADDR:X,Y" waits
# until the original stops at the click-wait check ADDR (a link address, e.g. 0x78BD3 for f78BA4's, 0x55FFD for the
# human movement loop), clicks X,Y there (pokeclick.sh), and goes on. Before each step whose number is >= DUMP_FROM
# (default 0), and after the last one at the last step's ADDR, the data object is dumped to C:\DOSBox-X\advciv\NAME_k.bin
# and the screen saved as capture/steps/NAME_k.png (k = the number of steps done). POKES="ADDR BYTE ..." (run time
# linear addresses, hex) are written at the first stop. The run copy's saves are kept (KEEP_SAVES_TO=DIR copies the
# ones the original wrote before they are put back).
name=$1; sx=$2; sy=$3; steps=$4; from=${5:-0}
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
R=/c/work/advciv_run; B=$(mktemp -d)
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $B/ || exit 1
finish() { taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1
           [ -n "$KEEP_SAVES_TO" ] && mkdir -p "$KEEP_SAVES_TO" && cp -p $R/SAVGAMEA.DAT $R/savgameb.dat "$KEEP_SAVES_TO"/
           cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $R/; rm -rf $B; }
trap finish EXIT
wait_stop() {   # wait for the debugger to stop at link address $1
  local want; want=$(printf 'EIP=00%X' $(( $1 + 0x202000 )))
  for i in $(seq 1 120); do
    local s; s=$(D dbgscreen)
    e=$(echo "$s" | grep -o 'EIP=[0-9A-F]*' | head -1)
    if [ "$e" = "$want" ] && ! echo "$s" | tail -2 | grep -q '(Running)'; then return 0; fi
    sleep 1
  done
  echo "no stop at $1 ($e)"; return 1
}
save_state() {   # dump and shot while stopped; leaves the emulation running
  rm -f /c/DOSBox-X/advciv/$1.bin
  D dbg "MEMDUMPBIN 0188:2A9000 72C00 $1.bin" > /dev/null; sleep 2
  rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
  D dbg "BPDEL *" > /dev/null
  D menu "Take raw screenshot" > /dev/null; D dbgkey F5 > /dev/null; sleep 2
  mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png /c/DOSBox-X/advciv/capture/steps/$1.png
  echo "$1 saved"
}
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
mkdir -p /c/DOSBox-X/advciv/capture/steps
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 186 > /dev/null || exit 1
set -- $steps
first=${1%%:*}
NEXT_BP=$first bash $P 0x78BD3 $sx $sy > /dev/null || exit 1
k=0; poked=
for st in $steps; do
  addr=${st%%:*}; xy=${st#*:}
  wait_stop $addr || exit 1
  if [ -z "$poked" ] && [ -n "$POKES" ]; then
    set -- $POKES; while [ $# -ge 2 ]; do D dbg "SM 0188:$1 0$2" > /dev/null; shift 2; done; poked=1
  fi
  if [ $k -ge $from ]; then save_state ${name}_$k; fi
  k=$((k + 1))
  # the click: pokeclick breaks in again at addr (the loop is still waiting there), writes the click, and runs on
  # to the next step's address
  nxt=$(echo $steps | tr ' ' '\n' | sed -n "$((k + 1))p"); nxt=${nxt%%:*}; [ -z "$nxt" ] && nxt=$addr
  NEXT_BP=$nxt bash $P $addr ${xy%,*} ${xy#*,} > /dev/null || exit 1
done
wait_stop $nxt || exit 1
save_state ${name}_$k
