#!/bin/bash
# run_srd.sh NAME COUNT [EVERY]
# DOSBox-X: the self-running demo with eight computers (advciv_srd8.conf: AC_MAIN /NT /SRD8). Prints the new game's
# seed (read at 0x117E0, right after main stores the clock seed at 0x117DA: give it to the port's --seed), then stops
# at the autosave's entry 0x5E8EC COUNT times and dumps the data object at every EVERY-th stop (default 1) to
# C:\DOSBox-X\advciv\NAME_k.bin (k = 1..COUNT, the port's --stop-at 0x5E8EC k). ADDR=0x55258 PAST=0x5526D stop at the
# end-of-round save instead (the port's --stop-at 0x55258 k: one stop per round). The run copy's saves and ACIV.CFG
# (the kaboom3 counter, anim.c) are put back afterwards. FROM=k skips the dumps before stop k. SEED=0x... replays
# that game (written over the clock seed at 0x117E0). NOBUDGET=1 lifts the computers' ship-search time budget (the
# original times it with the real clock, so under emulation it cuts searches the port, whose clock stands still in
# a search, never cuts; by_design.txt lists the word). When a stop does not come within WAIT s, hang() takes a raw
# screenshot (DOSBox-X's capture folder), breaks into the debugger, prints EIP and dumps the data object to
# NAME_hang.bin. Stop a run by killing its bash (TaskStop on a wrapper leaves this script running, and its exit
# trap then kills whatever DOSBox-X is running). M7 (Claude, 21-22 Nov).
name=$1; count=$2; every=${3:-1}; ADDR=${ADDR:-0x5E8EC}; PAST=${PAST:-0x5E904}; FROM=${FROM:-1}
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
R=/c/work/advciv_run; B=$(mktemp -d)
cp -p $R/SAVGAMEA.DAT $R/savgameb.dat $R/ACIV.CFG $B/ || exit 1
finish() { taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 1
           cp -p $B/SAVGAMEA.DAT $B/savgameb.dat $B/ACIV.CFG $R/; rm -rf $B; }
trap finish EXIT
stopped_at() {   # the EIP when the debugger is stopped, else nothing
  local s; s=$(D dbgscreen)
  echo "$s" | tail -2 | grep -q '(Running)' && return
  echo "$s" | grep -o 'EIP=[0-9A-F]*' | head -1
}
wait_stop() {   # wait (up to WAIT s, default 900) for a stop at link address $1
  local want e; want=$(printf 'EIP=00%X' $(( $1 + 0x202000 )))
  for i in $(seq 1 ${WAIT:-900}); do e=$(stopped_at); [ "$e" = "$want" ] && return 0; sleep 1; done
  echo "no stop at $1 ($e)"; return 1
}
hang() {
  D menu "Take raw screenshot" > /dev/null; sleep 2
  D menu "Start DOSBox-X Debugger" > /dev/null; sleep 2
  echo "hang: $(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1) $(date +%H:%M)"
  rm -f /c/DOSBox-X/advciv/${name}_hang.bin
  D dbg "MEMDUMPBIN 0188:2A9000 72C00 ${name}_hang.bin" > /dev/null; sleep 2
  D dbgscreen > /c/DOSBox-X/advciv/${name}_hang.txt
}
bp() { D dbg "BPDEL *" > /dev/null; D dbg "BP 0180:$(printf '%X' $(( $1 + 0x202000 )))" > /dev/null; }
taskkill //IM dosbox-x.exe //F > /dev/null 2>&1; sleep 2
powershell.exe -NoProfile -Command "Start-Process -FilePath 'C:\DOSBox-X\dosbox-x.exe' -ArgumentList '-conf','C:\work\AdvCiv\tools\dosbox\advciv_srd8.conf' -WorkingDirectory 'C:\DOSBox-X\advciv'"
sleep 4
# into protected mode first: the clock read before the seed (int 21h AH=2Ch, 0x1178E) stops there; a breakpoint on
# the game's selector set in real mode never fires
D menu "Start DOSBox-X Debugger" > /dev/null; sleep 1
D dbg "BPDEL *" > /dev/null; D dbg "BPINT 21 2C" > /dev/null; D dbgkey F5 > /dev/null
for i in $(seq 1 120); do e=$(stopped_at); [ -n "$e" ] && break; sleep 1; done
[ -n "$e" ] || { echo "no stop on int 21h AH=2Ch"; exit 1; }
bp 0x117E0; D dbgkey F5 > /dev/null; sleep 1
wait_stop 0x117E0 || exit 1
D dbg "D 0188:2C3C5C" > /dev/null
row=$(D dbgscreen | grep -E '^0188:002C3C5C' | head -1)
seed=$(echo "$row" | awk '{printf "0x%s%s%s%s", $5, $4, $3, $2}')
if [ -n "$SEED" ]; then   # replay a known game: write SEED over the clock seed main has just stored
  D dbg "SM 0188:2C3C5C $(printf '%08X' $((SEED)) | sed -E 's/(..)(..)(..)(..)/\4 \3 \2 \1/')" > /dev/null
  D dbg "D 0188:2C3C5C" > /dev/null
  row=$(D dbgscreen | grep -E '^0188:002C3C5C' | head -1)
  seed=$(echo "$row" | awk '{printf "0x%s%s%s%s", $5, $4, $3, $2}')
  [ $((seed)) -eq $((SEED)) ] || { echo "seed not set ($seed)"; exit 1; }
fi
echo "seed $seed"
if [ -n "$NOBUDGET" ]; then   # the computers' ship search as on an infinitely fast PC, like the port (aiship.c)
  # int16 [0xBB3E8] = 100 is the search's time budget in percent (f6332C: 9500 hundredths); 32767 makes it 8.6 hours
  D dbg "SM 0188:2B43E8 FF 7F" > /dev/null
  D dbg "D 0188:2B43E8" > /dev/null
  D dbgscreen | grep -qE '^0188:002B43E8 FF 7F' || { echo "budget not set"; exit 1; }
  echo "budget word 0xBB3E8 = 32767"
fi
bp $ADDR; D dbgkey F5 > /dev/null; sleep 1
for k in $(seq 1 $count); do
  wait_stop $ADDR || { hang; exit 1; }
  if [ $k -ge $FROM ] && [ $((k % every)) -eq 0 ]; then
    rm -f /c/DOSBox-X/advciv/${name}_$k.bin
    D dbg "MEMDUMPBIN 0188:2A9000 72C00 ${name}_$k.bin" > /dev/null; sleep 2
    echo "${name}_$k saved $(date +%H:%M)"
  fi
  [ $k -eq $count ] && break
  # a breakpoint does not fire again after F5 from its own address: run past it (PAST) and re-arm
  bp $PAST; D dbgkey F5 > /dev/null; sleep 1
  wait_stop $PAST || { hang; exit 1; }
  bp $ADDR; D dbgkey F5 > /dev/null; sleep 1
done
