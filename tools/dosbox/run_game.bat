@echo off
REM Launch DOSBox-X with the game mounted and running.
REM Edit GAME_DIR if your game folder is elsewhere.
set GAME_DIR=C:\Users\erikk\Desktop\Centurion-Defender-of-Rome_DOS_EN
dosbox-x.exe -c "mount c %GAME_DIR%" -c "c:" -c "centurio.exe"
