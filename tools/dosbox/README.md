# DOSBox-X automation for the Centurion port investigation

## How to run (Windows)
1. Copy `run_game.bat`, `debug_commands.txt`, `collect.bat` next to your
   `dosbox-x.exe` (or edit the paths inside them).
2. Double-click `run_game.bat` — the game launches.
3. Press **Alt+Pause**, click the **DOSBox-X Debugger** window.
4. At the `I->` prompt, enter the commands from `debug_commands.txt`
   (paste them line by line). This dumps `OVL_RUNTIME.BIN` and sets the
   breakpoints on the EXE's overlay-call sites.
5. Press **F5** to run; at each breakpoint: `EV eip`, then `D ds:<global>`
   (mapping in the file), TAB to Data View, note the 4 bytes.
6. When done, run `collect.bat` from the repo root to push dumps to GitHub.

## Notes
- All values in the debugger are hex.
- If the overlay segment isn't 3682 in your session, read it first with
  `EV ds` + `D ds:c4` and substitute.
- `MEMDUMPBIN` writes to DOSBox-X's own working directory (next to the exe),
  not the mounted drive — copy the file from there before `collect.bat`.
