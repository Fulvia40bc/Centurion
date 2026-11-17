# Regenerate the disassembly from the game copy (never from Erik's originals).
#   powershell -ExecutionPolicy Bypass -File tools\disasm.ps1
# Writes disasm\le\obj1.bin (code, base 0x10000), obj2.bin (data, base 0xB0000), fixups.txt, le_map.txt
# and disasm\AC_MAIN.linear.asm (linear llvm-objdump listing, addresses = link-time linear addresses).
# These are derived from the game and are not committed (see .gitignore).
param(
  [string]$Game      = 'C:\work\advciv_play',
  [string]$Toolchain = 'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\bin'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$le   = Join-Path $repo 'disasm\le'
python -I (Join-Path $PSScriptRoot 'le_unbind.py') (Join-Path $Game 'AC_MAIN.EXE') $le
if ($LASTEXITCODE) { throw "le_unbind failed ($LASTEXITCODE)" }
Push-Location $le
try {
  & (Join-Path $Toolchain 'llvm-objcopy.exe') -I binary -O elf32-i386 --rename-section .data=.text,alloc,load,code obj1.bin obj1.elf
  if ($LASTEXITCODE) { throw "objcopy failed ($LASTEXITCODE)" }
  # --start/--stop-address take file offsets (before --adjust-vma); the listing shows linear addresses
  & (Join-Path $Toolchain 'llvm-objdump.exe') -d --no-show-raw-insn -M intel --adjust-vma=0x10000 obj1.elf |
    ForEach-Object { $_ -replace ' <_binary_obj1_bin_start\+0x[0-9a-f]+>', '' } |
    Set-Content -Encoding ascii (Join-Path $repo 'disasm\AC_MAIN.linear.asm')
} finally { Pop-Location }
Write-Host "Wrote $repo\disasm\AC_MAIN.linear.asm"
