#!/bin/bash
# run_ai.sh NAME SLOT_Y ADDR COUNT ["ADDR:X,Y ..."]
# DOSBox-X: restore the save in the slot at (320, SLOT_Y) from the title menu, play the optional click steps as
# run_steps.sh does (each "ADDR:X,Y" waits at the click-wait check ADDR and clicks X,Y there), then stop COUNT times
# at the link address ADDR (no click: a computer's code, e.g. 0x55A64, the return of the movement planner f461BC)
# and dump the data object at each stop to C:\DOSBox-X\advciv\NAME_k.bin (k = 0..COUNT-1). The run copy's saves are
# put back afterwards (KEEP_SAVES_TO=DIR copies the ones the original wrote first). CONT="X,Y" clicks X,Y at the
# generic click wait 0x78BD3 between stops (CONTINUE after a computer's moves). M7 (Claude, 21 Nov).
name=$1; sy=$2; addr=$3; count=$4; steps=$5
P=/c/work/AdvCiv/tools/dosbox/pokeclick.sh
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
R=/c/work/advciv_run; B=$(mktemp -d)
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $B/ || exit 1
finish() { taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1
           [ -n "$KEEP_SAVES_TO" ] && mkdir -p "$KEEP_SAVES_TO" && cp -p $R/SAVGAMEA.DAT $R/savgameb.dat "$KEEP_SAVES_TO"/
           cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $R/; rm -rf $B; }
trap finish EXIT
wait_stop() {   # wait (up to WAIT s, default 300) for the debugger to stop at link address $1
  local want; want=$(printf 'EIP=00%X' $(( $1 + 0x202000 )))
  for i in $(seq 1 ${WAIT:-300}); do
    local s; s=$(D dbgscreen)
    e=$(echo "$s" | grep -o 'EIP=[0-9A-F]*' | head -1)
    if [ "$e" = "$want" ] && ! echo "$s" | tail -2 | grep -q '(Running)'; then return 0; fi
    sleep 1
  done
  echo "no stop at $1 ($e)"; return 1
}
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_nt.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 30
bash $P 0x4F6D3 320 186 > /dev/null || exit 1
set -- $steps
first=${1%%:*}; [ -z "$first" ] && first=$addr
NEXT_BP=$first bash $P 0x78BD3 320 $sy > /dev/null || exit 1
k=0
for st in $steps; do
  a=${st%%:*}; xy=${st#*:}
  wait_stop $a || exit 1
  k=$((k + 1))
  nxt=$(echo $steps | tr ' ' '\n' | sed -n "$((k + 1))p"); nxt=${nxt%%:*}; [ -z "$nxt" ] && nxt=$addr
  NEXT_BP=$nxt bash $P $a ${xy%,*} ${xy#*,} > /dev/null || exit 1
done
run=$(printf '%X' $(( addr + 0x202000 )))
for k in $(seq 0 $((count - 1))); do
  wait_stop $addr || exit 1
  rm -f /c/DOSBox-X/advciv/${name}_$k.bin
  D dbg "MEMDUMPBIN 0188:2A9000 72C00 ${name}_$k.bin" > /dev/null; sleep 2
  echo "${name}_$k saved"
  # a breakpoint does not fire again after F5 from its own address: run to the call instruction (CALL=link address,
  # default ADDR - 5) first, then re-arm ADDR
  [ $k -eq $((count - 1)) ] && break
  c=${CALL:-$((addr - 5))}
  if [ -n "$CONT" ]; then   # CONT="X,Y": a click at f78BA4's wait first (e.g. 368,437: CONTINUE after "Movement for
                            # %s complete.", the box the original shows after each computer's moves)
    D dbg "BPDEL *" > /dev/null; D dbgkey F5 > /dev/null; sleep 1
    NEXT_BP=$c bash $P 0x78BD3 ${CONT%,*} ${CONT#*,} > /dev/null || exit 1
  else
    D dbg "BPDEL *" > /dev/null; D dbg "BP 0180:$(printf '%X' $(( c + 0x202000 )))" > /dev/null
    D dbgkey F5 > /dev/null; sleep 1
  fi
  wait_stop $c || exit 1
  D dbg "BPDEL *" > /dev/null; D dbg "BP 0180:$run" > /dev/null
  D dbgkey F5 > /dev/null; sleep 1
done
