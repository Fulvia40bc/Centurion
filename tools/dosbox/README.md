# DOSBox-X kit for Advanced Civilization

- `advciv.conf`: the profile (runs `C:\work\advciv_run`, a copy: the game autosaves after every phase).
  Start: `C:\DOSBox-X\dosbox-x.exe -conf C:\work\AdvCiv\tools\dosbox\advciv.conf -break-start`, working directory
  `C:\DOSBox-X\advciv` (MEMDUMPBIN writes there). `-break-start` stops in the debugger before the BIOS runs.
- `drive.ps1` (from the Centurion kit, 640x480 coordinates). Run it with **Windows PowerShell**
  (`powershell.exe -File drive.ps1 ...`): it attaches to the debugger's console, which does not work from pwsh 7.
  `dbg "CMD"`, `dbgkey F5`, `dbgscreen` need no focus; `shot`, `click X Y`, `key Alt+Pause` bring the game
  window to the front (only when the screen is ours to use).
- Addresses: code link address A = `0180:A+202000`; data object = `0188:2A9000`, size `72C00`.
  `MEMDUMPBIN 0188:2A9000 72C00 name.bin` dumps it; `find_objects.py DUMP disasm/le` locates and compares both
  objects in a flat dump (`MEMDUMPBIN 0188:0 800000 name.bin`).
- Screenshots go to `shots/` (not committed).
- **Exact frames:** `drive.ps1 menu "Take raw screenshot"` runs DOSBox-X's own menu command (WM_COMMAND, no focus,
  no keys). The capture lands in `C:\DOSBox-X\advciv\capture` (the profile's `captures`) as a 640x480 indexed PNG
  of the emulated frame, before any window scaling. Compare with `tools/verify/`.
  `drive.ps1 menu "TEXT"` runs any menu item by its text; it lists the items if the text isn't found.
