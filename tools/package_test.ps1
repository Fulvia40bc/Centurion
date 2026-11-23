# Tests for package.ps1 with fake build folders (no real PE files, no llvm-objdump, no game data).
# Run:  pwsh -NoProfile -File tools\package_test.ps1     (PowerShell 5.1 or 7)
# package.ps1 is run as a child process (so the exit codes are the real ones); the import reader is replaced
# through -ImportTableJson, a JSON file holding the fake import table, so no llvm-objdump runs. The real reader
# (llvm-objdump -p) is only exercised on Erik's PC - see the HANDOFF reply.
$ErrorActionPreference = 'Continue'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkg = Join-Path $here 'package.ps1'
$pwsh = (Get-Process -Id $PID).Path

$script:fails = 0
function Check([bool]$cond, [string]$msg) {
  if (-not $cond) { $script:fails += 1; Write-Host "FAIL $msg" }
}

function New-FakeTree([string]$root) {
  # a fake build folder for both architectures, fake licences, a fake soundfont, fake repos
  foreach ($arch in @('aarch64', 'x86_64')) {
    $d = Join-Path $root "build\$arch"
    New-Item -ItemType Directory -Force $d | Out-Null
    foreach ($f in @('advciv.exe', 'SDL3.dll', 'libfluidsynth-3.dll', 'libc++.dll', 'libunwind.dll')) {
      [System.IO.File]::WriteAllBytes((Join-Path $d $f), [System.Text.Encoding]::ASCII.GetBytes("FAKE-$f-$arch"))
    }
    # scratch a player must not get, and a game file that is not imported: none of these may be packaged
    [System.IO.File]::WriteAllBytes((Join-Path $d 'plat_demo.exe'), [System.Text.Encoding]::ASCII.GetBytes('FAKE-plat_demo'))
    [System.IO.File]::WriteAllBytes((Join-Path $d 'opl3.o'), [System.Text.Encoding]::ASCII.GetBytes('FAKE-opl3.o'))
    [System.IO.File]::WriteAllBytes((Join-Path $d 'ACIV.CT1'), [System.Text.Encoding]::ASCII.GetBytes('FAKE-game-data'))
  }
  $lic = Join-Path $root 'licences'
  New-Item -ItemType Directory -Force $lic | Out-Null
  [System.IO.File]::WriteAllText((Join-Path $lic 'SDL3-LICENSE.txt'), 'zlib licence text')
  [System.IO.File]::WriteAllText((Join-Path $lic 'fluidsynth-LICENSE'), 'LGPL-2.1 text')
  [System.IO.File]::WriteAllText((Join-Path $lic 'llvm-LICENSE.TXT'), 'Apache-2.0 LLVM text')
  [System.IO.File]::WriteAllText((Join-Path $lic 'sf2-LICENSE.txt'), 'GeneralUser GS licence')
  [System.IO.File]::WriteAllBytes((Join-Path $lic 'GeneralUser-GS.sf2'), [System.Text.Encoding]::ASCII.GetBytes('FAKE-soundfont'))
  # fake repos: build_port.ps1 without and with opl.c in the game's file list; the vendored Nuked-OPL3 licence
  foreach ($r in @('repo_noopl', 'repo_opl')) {
    $rp = Join-Path $root $r
    New-Item -ItemType Directory -Force (Join-Path $rp 'tools') | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $rp 'port\third_party\nuked-opl3') | Out-Null
    [System.IO.File]::WriteAllText((Join-Path $rp 'port\third_party\nuked-opl3\LICENSE'), 'Nuked-OPL3 LGPL text')
    $game = "'main.c', 'fg.c'"
    if ($r -eq 'repo_opl') { $game = "'main.c', 'opl.c', 'fg.c'" }
    [System.IO.File]::WriteAllText((Join-Path $rp 'tools\build_port.ps1'), "`$game = @($game)`n& `$cc `@game -o advciv.exe`n")
  }
}

function Get-FakeTable([string]$root, [string[]]$advcivImports) {
  # fake imports keyed by the exact paths package.ps1 will ask about
  $t = @{}
  foreach ($arch in @('aarch64', 'x86_64')) {
    $d = Join-Path $root "build\$arch"
    $t[(Join-Path $d 'advciv.exe')] = $advcivImports
    $t[(Join-Path $d 'SDL3.dll')] = @('USER32.dll', 'api-ms-win-core-win32-window-l1-1-0.dll')
    $t[(Join-Path $d 'libfluidsynth-3.dll')] = @('libc++.dll', 'libunwind.dll', 'api-ms-win-crt-stdio-l1-1-0.dll')
    $t[(Join-Path $d 'libc++.dll')] = @('KERNEL32.dll', 'ucrtbase.dll')
    $t[(Join-Path $d 'libunwind.dll')] = @('api-ms-win-core-heap-l1-1-0.dll')
  }
  return $t
}

function Run-Package {
  # run package.ps1 as a child process with the fake import table as JSON; returns @{ rc, out }.
  # A switch listed in $Skip is left out entirely, so package.ps1's own default is used. An empty value is
  # left out too: Windows PowerShell 5.1 drops an empty argument when the child process starts, which would
  # shift every following argument (TASK 21a).
  param([string]$root, [string]$out, [string]$version, [hashtable]$table, [string]$repo, [string]$buildPort, [string]$tmp,
        [string[]]$Skip = @())
  $json = Join-Path $tmp 'imports.json'
  $table | ConvertTo-Json -Depth 3 | Set-Content -Path $json -Encoding ASCII
  $lic = Join-Path $root 'licences'
  $pairs = @(
    @{ k = '-Build'; v = (Join-Path $root 'build') },
    @{ k = '-Out'; v = $out },
    @{ k = '-Version'; v = $version },
    @{ k = '-Objdump'; v = 'nonexistent-llvm-objdump.exe' },
    @{ k = '-Repo'; v = $repo },
    @{ k = '-BuildPort'; v = $buildPort },
    @{ k = '-Sdl3License'; v = (Join-Path $lic 'SDL3-LICENSE.txt') },
    @{ k = '-FluidSynthLicense'; v = (Join-Path $lic 'fluidsynth-LICENSE') },
    @{ k = '-LlvmMingwLicense'; v = (Join-Path $lic 'llvm-LICENSE.TXT') },
    @{ k = '-Soundfont'; v = (Join-Path $lic 'GeneralUser-GS.sf2') },
    @{ k = '-SoundfontLicense'; v = (Join-Path $lic 'sf2-LICENSE.txt') },
    @{ k = '-NukedLicense'; v = (Join-Path $repo 'port\third_party\nuked-opl3\LICENSE') },
    @{ k = '-ImportTableJson'; v = $json }
  )
  $argList = @('-NoProfile', '-File', $pkg)
  foreach ($p in $pairs) {
    if ($Skip -contains $p.k) { continue }
    if ([string]::IsNullOrEmpty([string]$p.v)) { continue }
    $argList += @($p.k, [string]$p.v)
  }
  $outText = (& $pwsh @argList 2>&1) | Out-String
  return @{ rc = $LASTEXITCODE; out = $outText }
}

function Get-ZipEntries([string]$zipPath) {
  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $z = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
  try { return @($z.Entries | ForEach-Object { $_.FullName }) }
  finally { $z.Dispose() }
}

function Get-ZipEntryText([string]$zipPath, [string]$name) {
  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $z = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
  try {
    $e = $z.GetEntry($name)
    $r = New-Object System.IO.StreamReader($e.Open())
    $t = $r.ReadToEnd()
    $r.Dispose()
    return $t
  }
  finally { $z.Dispose() }
}

function Same-Set([string[]]$a, [string[]]$b) {
  if ($a.Count -ne $b.Count) { return $false }
  return (@(Compare-Object $a $b).Count -eq 0)
}

# ---- run the cases ----
$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ('package_test_' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $tmp | Out-Null
try {
  $root = Join-Path $tmp 'tree'
  New-FakeTree $root
  $repo = Join-Path $root 'repo_noopl'
  $buildPort = Join-Path $repo 'tools\build_port.ps1'
  $fullImports = @('SDL3.dll', 'libfluidsynth-3.dll', 'KERNEL32.dll', 'api-ms-win-crt-runtime-l1-1-0.dll', 'ucrtbase.dll')

  # A. full package, both architectures
  $outA = Join-Path $tmp 'outA'; New-Item -ItemType Directory -Force $outA | Out-Null
  $r = Run-Package $root $outA 'TEST1' (Get-FakeTable $root $fullImports) $repo $buildPort $tmp
  Check ($r.rc -eq 0) "A: rc=$($r.rc) out=$($r.out.Substring(0, [Math]::Min(300, $r.out.Length)))"
  $zipA = Join-Path $outA 'advciv-TEST1-windows-arm64.zip'
  $zipX = Join-Path $outA 'advciv-TEST1-windows-x64.zip'
  Check ((Test-Path $zipA) -and (Test-Path $zipX)) "A: both zips written ($((Get-ChildItem $outA -Name) -join ', '))"
  $entries = Get-ZipEntries $zipA
  $want = @('AdvCiv/advciv.exe', 'AdvCiv/SDL3.dll', 'AdvCiv/libfluidsynth-3.dll', 'AdvCiv/libc++.dll',
            'AdvCiv/libunwind.dll', 'AdvCiv/GeneralUser-GS.sf2', 'AdvCiv/README.txt',
            'AdvCiv/LICENSES/SDL3.txt', 'AdvCiv/LICENSES/FluidSynth.txt', 'AdvCiv/LICENSES/llvm-mingw-runtime.txt',
            'AdvCiv/LICENSES/GeneralUser-GS.txt')
  Check (Same-Set $entries $want) "A: entries exactly the listed ones under AdvCiv\ (libc++.dll/libunwind.dll are imported only by libfluidsynth-3.dll, so this also checks the transitive set): got $($entries -join ', ')"
  Check (-not ($entries -contains 'AdvCiv/plat_demo.exe') -and -not ($entries -contains 'AdvCiv/opl3.o') -and -not ($entries -contains 'AdvCiv/ACIV.CT1')) "A: no test program, no .o, no game file in the zip"
  Check (-not ($entries -contains 'AdvCiv/LICENSES/Nuked-OPL3.txt')) "A: opl.c not in `$game -> no Nuked-OPL3.txt"
  # README.txt: CRLF only, at most 80 columns per line, mentions the version and the player switches
  $readme = Get-ZipEntryText $zipA 'AdvCiv/README.txt'
  Check ($readme.Contains("`r`n") -and -not ($readme.Replace("`r`n", '').Contains("`n"))) 'A: README.txt is CRLF only'
  $maxLen = 0
  foreach ($line in $readme -split "`r`n") { if ($line.Length -gt $maxLen) { $maxLen = $line.Length } }
  Check ($maxLen -le 80) "A: README lines at most 80 columns (max $maxLen)"
  Check ($readme.Contains('Version: TEST1') -and $readme.Contains('--original') -and $readme.Contains('F11') -and $readme.Contains('Alt+Enter')) 'A: README has version, --original, F11/Alt+Enter'
  Check ($readme.Contains('Unzip the AdvCiv folder') -and $readme.Contains('start AdvCiv\advciv.exe')) 'A: README keeps the unzip instruction and says to start AdvCiv\advciv.exe'
  Check ($readme.Contains('NOT included')) 'A: README says the game files are not included'
  # the printed file list has sizes and SHA-256
  Check (($r.out.Contains('advciv-TEST1-windows-arm64.zip')) -and ($r.out -match '(?m)^\s*AdvCiv/advciv\.exe\s+\d+\s+[0-9a-f]{64}\s*$')) "A: printed list with size and SHA-256: $($r.out.Substring(0, [Math]::Min(200, $r.out.Length)))"

  # B. without FluidSynth: no soundfont, no FluidSynth/GeneralUser licences
  $outB = Join-Path $tmp 'outB'; New-Item -ItemType Directory -Force $outB | Out-Null
  $noFsImports = @('SDL3.dll', 'KERNEL32.dll', 'ucrtbase.dll')
  $r = Run-Package $root $outB 'TEST2' (Get-FakeTable $root $noFsImports) $repo $buildPort $tmp
  Check ($r.rc -eq 0) "B: rc=$($r.rc)"
  $entriesB = Get-ZipEntries (Join-Path $outB 'advciv-TEST2-windows-x64.zip')
  $wantB = @('AdvCiv/advciv.exe', 'AdvCiv/SDL3.dll', 'AdvCiv/README.txt', 'AdvCiv/LICENSES/SDL3.txt')
  Check (Same-Set $entriesB $wantB) "B: no soundfont, no FluidSynth licence without libfluidsynth-3.dll: got $($entriesB -join ', ')"

  # C. a missing non-Windows DLL stops with exit 1 naming it, and writes no zip
  $outC = Join-Path $tmp 'outC'; New-Item -ItemType Directory -Force $outC | Out-Null
  $r = Run-Package $root $outC 'TEST3' (Get-FakeTable $root @('SDL3.dll', 'MISSING.dll')) $repo $buildPort $tmp
  Check ($r.rc -eq 1 -and $r.out.Contains('MISSING.dll')) "C: rc=$($r.rc), output names the DLL: $($r.out.Substring(0, [Math]::Min(200, $r.out.Length)))"
  Check (@(Get-ChildItem $outC -Filter '*.zip').Count -eq 0) 'C: no zip written'

  # D. a chosen file whose name is a game file is refused with exit 1 (the file exists in the build folder and
  # its imports are in the table, so the run reaches the name check)
  $outD = Join-Path $tmp 'outD'; New-Item -ItemType Directory -Force $outD | Out-Null
  $tableD = Get-FakeTable $root @('SDL3.dll', 'SAVEGAME1.DAT')
  foreach ($arch in @('aarch64', 'x86_64')) {
    $d = Join-Path $root "build\$arch"
    [System.IO.File]::WriteAllBytes((Join-Path $d 'SAVEGAME1.DAT'), [System.Text.Encoding]::ASCII.GetBytes('x'))
    $tableD[(Join-Path $d 'SAVEGAME1.DAT')] = @()
  }
  $r = Run-Package $root $outD 'TEST4' $tableD $repo $buildPort $tmp
  Check ($r.rc -eq 1 -and $r.out.Contains("refusing to package") -and $r.out.Contains('SAVEGAME1.DAT')) "D: rc=$($r.rc): $($r.out.Substring(0, [Math]::Min(200, $r.out.Length)))"
  Check (@(Get-ChildItem $outD -Filter '*.zip').Count -eq 0) 'D: no zip written'
  foreach ($arch in @('aarch64', 'x86_64')) { Remove-Item (Join-Path (Join-Path $root "build\$arch") 'SAVEGAME1.DAT') -Force }

  # E. opl.c in the game's file list -> Nuked-OPL3.txt ships and the README carries the source note
  $outE = Join-Path $tmp 'outE'; New-Item -ItemType Directory -Force $outE | Out-Null
  $repoOpl = Join-Path $root 'repo_opl'
  $r = Run-Package $root $outE 'TEST5' (Get-FakeTable $root $fullImports) $repoOpl (Join-Path $repoOpl 'tools\build_port.ps1') $tmp
  Check ($r.rc -eq 0) "E: rc=$($r.rc)"
  $zipE = Join-Path $outE 'advciv-TEST5-windows-arm64.zip'
  $entriesE = Get-ZipEntries $zipE
  Check ($entriesE -contains 'AdvCiv/LICENSES/Nuked-OPL3.txt') "E: Nuked-OPL3.txt ships when opl.c is linked: $($entriesE -join ', ')"
  $readmeE = Get-ZipEntryText $zipE 'AdvCiv/README.txt'
  Check ($readmeE.Contains('Nuked-OPL3') -and $readmeE.Contains('advciv branch')) 'E: README carries the source note for the statically linked part'

  # F. running again with the same version replaces the zip and touches nothing else in -Out
  $extra = Join-Path $outA 'keepme.txt'
  [System.IO.File]::WriteAllText($extra, 'keep')
  $r = Run-Package $root $outA 'TEST1' (Get-FakeTable $root $fullImports) $repo $buildPort $tmp
  Check ($r.rc -eq 0 -and (Test-Path $extra) -and (@(Get-ChildItem $outA -Filter '*.zip').Count -eq 2)) "F: rc=$($r.rc), zips replaced, other files untouched: $((Get-ChildItem $outA -Name) -join ', ')"

  # G. default version and default -Repo: <date>-<git describe --always --dirty> of this repo, never
  # 'unknown'; -Repo left at its script default (Split-Path -Parent $PSScriptRoot = this repo)
  $realRepo = Split-Path -Parent $here
  $outG = Join-Path $tmp 'outG'; New-Item -ItemType Directory -Force $outG | Out-Null
  $r = Run-Package $root $outG '' (Get-FakeTable $root $fullImports) $realRepo (Join-Path $realRepo 'tools\build_port.ps1') $tmp -Skip @('-Repo')
  $zipsG = @(Get-ChildItem $outG -Filter '*.zip' -Name)
  Check ($r.rc -eq 0 -and $zipsG.Count -eq 2 -and
         ($zipsG[0] -match '^advciv-\d{8}-[0-9a-f]{7,40}(-dirty)?-windows-(arm64|x64)\.zip$') -and
         ($zipsG[0] -notmatch 'unknown')) "G: default version zip names are <date>-<git describe>, not 'unknown': $($zipsG -join ', ')"

  # H. licence/soundfont/objdump paths left at their script defaults: where a default does not exist (this
  # sandbox), the run must stop with exit 1 and that path in the message; where the defaults exist (Erik's
  # PC), the package succeeds
  $outH = Join-Path $tmp 'outH'; New-Item -ItemType Directory -Force $outH | Out-Null
  $defaultSdl3 = 'C:\work\toolchain\deps\SDL3-3.4.20\LICENSE.txt'
  $r = Run-Package $root $outH 'TESTH' (Get-FakeTable $root $fullImports) $repo $buildPort $tmp `
    -Skip @('-Objdump', '-Sdl3License', '-FluidSynthLicense', '-LlvmMingwLicense', '-Soundfont', '-SoundfontLicense')
  if (-not (Test-Path $defaultSdl3 -PathType Leaf)) {
    Check ($r.rc -eq 1 -and $r.out.Contains($defaultSdl3)) "H (defaults missing here): rc=$($r.rc), the message names the missing default licence path: $($r.out.Substring(0, [Math]::Min(250, $r.out.Length)))"
    Check (@(Get-ChildItem $outH -Filter '*.zip').Count -eq 0) 'H (defaults missing here): no zip written'
  } else {
    Check ($r.rc -eq 0 -and @(Get-ChildItem $outH -Filter '*.zip').Count -eq 2) "H (defaults exist here): rc=$($r.rc), both zips written"
  }

  # I. build_port.ps1 left at its default ($Repo\tools\build_port.ps1) when that file does not exist: the
  # script must stop with exit 1 naming the path, not silently package without the Nuked-OPL3 check
  $repoNoBuild = Join-Path $root 'repo_nobuild'
  New-Item -ItemType Directory -Force (Join-Path $repoNoBuild 'port\third_party\nuked-opl3') | Out-Null
  [System.IO.File]::WriteAllText((Join-Path $repoNoBuild 'port\third_party\nuked-opl3\LICENSE'), 'Nuked-OPL3 LGPL text')
  $outI = Join-Path $tmp 'outI'; New-Item -ItemType Directory -Force $outI | Out-Null
  $r = Run-Package $root $outI 'TESTI' (Get-FakeTable $root $fullImports) $repoNoBuild '' $tmp -Skip @('-BuildPort', '-NukedLicense')
  $wantBp = Join-Path $repoNoBuild 'tools\build_port.ps1'
  Check ($r.rc -eq 1 -and $r.out.Contains('build_port.ps1') -and $r.out.Contains($wantBp)) "I: missing default build_port.ps1 stops the script, exit 1, naming '$wantBp': rc=$($r.rc) $($r.out.Substring(0, [Math]::Min(250, $r.out.Length)))"
  Check (@(Get-ChildItem $outI -Filter '*.zip').Count -eq 0) 'I: no zip written'
}
finally {
  Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

if ($script:fails -eq 0) { Write-Host 'package_test: all checks pass' }
else { Write-Host "package_test: $script:fails FAILURES" }
if ($script:fails -eq 0) { exit 0 } else { exit 1 }
