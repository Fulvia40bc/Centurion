@echo off
REM Push collected dumps (OVL_RUNTIME.BIN etc.) to the GitHub repo so the
REM analysis side can pull them. Run from the repo root on your machine.
git add -A
git commit -m "dosbox dumps %date% %time%"
git push
