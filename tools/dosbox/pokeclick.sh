#!/bin/bash
# pokeclick.sh LOOP_LINK_ADDR X Y [SHOT_NAME]
# A click without focus or a real mouse (works on a locked desktop): break into DOSBox-X through its menu, stop at a
# click-wait check of the game (the "cmp dword ptr [0xE1630], 0" of a mouse loop, given as a link address), write
# x, y and button 1 into the game's mouse variables 0xE1628/0xE162C/0xE1630 (run time +0x1F9000), clear the
# breakpoints and run on (NEXT_BP=link address: set that breakpoint before running on). With SHOT_NAME, a raw
# screenshot is taken 3 s later into capture/SHOT_NAME.png.
# Notes: the menu break may stop the emulator in real mode, where "0180:" is not the game's selector, so the script
# first stops in protected mode on the loop's own mouse call (int 33h AX=0003h: BPINT 33 00 03, AH then AL). Debugger
# hex tokens that look like flag names (AC, ...) are read as flags: every byte is written with a leading 0.
D() { powershell.exe -NoProfile -ExecutionPolicy Bypass -File 'C:\work\AdvCiv\tools\dosbox\drive.ps1' "$@" 2>&1; }
le() { printf '0%02X 0%02X 0%02X 0%02X' $(( $1 & 255 )) $(( ($1 >> 8) & 255 )) $(( ($1 >> 16) & 255 )) $(( ($1 >> 24) & 255 )); }
wait_eip() {   # wait up to 8 s for the debugger to stop with EIP matching $1 (a grep pattern)
  for i in 1 2 3 4 5 6 7 8; do
    s=$(D dbgscreen)
    if ! echo "$s" | tail -2 | grep -q '(Running)'; then
      e=$(echo "$s" | grep -o 'EIP=[0-9A-F]*' | head -1)
      if echo "$e" | grep -q "$1"; then echo "$e"; return 0; fi
    fi
    sleep 1
  done
  echo "timeout ($(D dbgscreen | grep -o 'EIP=[0-9A-F]*' | head -1))"; return 1
}
run=$(printf '%X' $(( $1 + 0x202000 )))
D menu "Start DOSBox-X Debugger" > /dev/null; sleep 1
D dbg "BPDEL *" > /dev/null
D dbg "BPINT 33 00 03" > /dev/null
D dbgkey F5 > /dev/null
wait_eip 'EIP=' > /dev/null || { echo "no stop on int 33h"; exit 1; }
D dbg "BPDEL *" > /dev/null
D dbg "BP 0180:$run" > /dev/null
D dbgkey F5 > /dev/null
wait_eip "EIP=00$run" > /dev/null || { echo "not at the loop $run"; exit 1; }
D dbg "SM 0188:2DA628 $(le $2) $(le $3) 01 00 00 00" > /dev/null
got=$(D dbg "D 0188:2DA628" > /dev/null; D dbgscreen | grep -E '^0188:002DA628' | head -1 | cut -c15-49)
D dbg "BPDEL *" > /dev/null
[ -n "$NEXT_BP" ] && D dbg "BP 0180:$(printf '%X' $(( NEXT_BP + 0x202000 )))" > /dev/null
if [ "$1" = "0x78BD3" ] && [ -z "$NO_REPOKE" ]; then
  # f78BA4: when a button is hit, 0x3C5D4 (called at 0x78C8C) waits for the release with fg_mousepos, which puts the
  # real (unmoved) DOSBox pointer into 0xE1628/0xE162C; callers such as the trade composer then read the click's
  # position from there. A real click keeps the pointer where it clicked, so write x, y again on the return
  # (0x78C91). With no button hit the run goes on to NEXT_BP instead and stops there as before.
  D dbg "BP 0180:27AC91" > /dev/null
  D dbgkey F5 > /dev/null
  e=""
  for i in $(seq 1 120); do
    s=$(D dbgscreen)
    if ! echo "$s" | tail -2 | grep -q '(Running)'; then e=$(echo "$s" | grep -o 'EIP=[0-9A-F]*' | head -1); break; fi
    sleep 1
  done
  if [ "$e" = "EIP=0027AC91" ]; then
    D dbg "SM 0188:2DA628 $(le $2) $(le $3)" > /dev/null
    D dbg "BPDEL *" > /dev/null
    [ -n "$NEXT_BP" ] && D dbg "BP 0180:$(printf '%X' $(( NEXT_BP + 0x202000 )))" > /dev/null
    D dbgkey F5 > /dev/null
    echo "click $2,$3 at $1: wrote $got, x,y again after 0x3C5D4"
  else
    echo "click $2,$3 at $1: wrote $got (no button hit; stopped at $e)"
  fi
  exit 0
fi
D dbgkey F5 > /dev/null
echo "click $2,$3 at $1: wrote $got"
if [ -n "$4" ]; then
  sleep 3; rm -f /c/DOSBox-X/advciv/capture/ac_main_*.png
  D menu "Take raw screenshot" > /dev/null; sleep 2
  mv /c/DOSBox-X/advciv/capture/ac_main_000.raw1.png "/c/DOSBox-X/advciv/capture/$4.png" && echo "shot $4"
fi
