# Package the port for players: one zip per architecture (HANDOFF TASK 21).
#   powershell -ExecutionPolicy Bypass -File tools\package.ps1 [-Build C:\work\advciv_port] [-Out C:\work\advciv_dist] [-Version V]
# PowerShell 5.1 and 7, no modules beyond what Windows has. For each architecture (aarch64, x86_64) it writes
#   advciv-<V>-windows-<arm64|x64>.zip into -Out, holding one folder AdvCiv\ with advciv.exe, every DLL it needs
#   (found transitively from the PE imports), the soundfont only when FluidSynth ships, LICENSES\ with one licence
#   file per shipped third-party part, and README.txt. It prints each zip's file list with sizes and SHA-256.
# Nothing else is ever packaged: no test programs, no .o files, no game data (a chosen file whose name matches the
# original game's is refused with exit 1).
param(
  [string]$Build   = 'C:\work\advciv_port',
  [string]$Out     = 'C:\work\advciv_dist',
  [string]$Version = '',
  [string]$Objdump = 'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\bin\llvm-objdump.exe',
  [string]$Repo    = (Split-Path -Parent $PSScriptRoot),   # the repository holding tools\ and port\
  [string]$BuildPort = '',   # default: $Repo\tools\build_port.ps1, read to see whether opl.c is linked into the game
  [string]$Sdl3License       = 'C:\work\toolchain\deps\SDL3-3.4.20\LICENSE.txt',
  [string]$FluidSynthLicense = 'C:\work\toolchain\deps\fluidsynth-2.6.2\LICENSE',
  [string]$LlvmMingwLicense  = 'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\LICENSE.TXT',
  [string]$Soundfont         = 'C:\work\toolchain\soundfont\GeneralUser-GS.sf2',
  [string]$SoundfontLicense  = 'C:\work\toolchain\soundfont\LICENSE.txt',
  [string]$NukedLicense      = '',   # default: $Repo\port\third_party\nuked-opl3\LICENSE
  [hashtable]$ImportTable = $null,   # test hook: file path -> array of imported DLL names; skips llvm-objdump
  [string]$ImportTableJson = ''      # test hook: same table as a JSON file (for running the script as a child process)
)

# DLLs that are part of Windows itself and are never shipped. Matched case-insensitively, by exact name or prefix.
$script:WindowsDllExact = @('KERNEL32.DLL', 'USER32.DLL', 'GDI32.DLL', 'SHELL32.DLL', 'ADVAPI32.DLL', 'OLE32.DLL',
  'OLEAUT32.DLL', 'IMM32.DLL', 'VERSION.DLL', 'WINMM.DLL', 'SETUPAPI.DLL', 'CFGMGR32.DLL', 'WS2_32.DLL',
  'UCRTBASE.DLL')
$script:WindowsDllPrefix = @('API-MS-WIN-', 'EXT-MS-WIN-')

# Names of the original game's files: a chosen file matching any of these is refused (never packaged).
$script:GameFilePatterns = @('*.CT?', '*.DAT', '*.GIF', '*.FLC', '*.XMI', '*.MDI', '*.DIG', 'AC_MAIN.EXE', 'AC_START.EXE')

function Test-WindowsDll {
  param([string]$Up)   # an upper-cased DLL name
  if ($script:WindowsDllExact -contains $Up) { return $true }
  foreach ($p in $script:WindowsDllPrefix) { if ($Up.StartsWith($p)) { return $true } }
  return $false
}

function Test-GameFileName {
  param([string]$Name)
  foreach ($pat in $script:GameFilePatterns) { if ($Name -like $pat) { return $true } }
  return $false
}

function Get-Imports {
  # The PE import table of one file: from $ImportTable when the test supplied it, else llvm-objdump -p.
  param([string]$Path)
  if ($null -ne $ImportTable -and $ImportTable.ContainsKey($Path)) {
    Write-Output -NoEnumerate @($ImportTable[$Path])   # -NoEnumerate: an empty import list must not become $null
    return
  }
  if (-not (Test-Path $Objdump -PathType Leaf)) {
    [Console]::Error.WriteLine("package: llvm-objdump not found at '$Objdump' (pass -Objdump, or -ImportTable for a test)")
    return $null
  }
  $out = & $Objdump -p $Path 2>$null
  if ($LASTEXITCODE -ne 0) {
    [Console]::Error.WriteLine("package: llvm-objdump failed on '$Path' (exit $LASTEXITCODE)")
    return $null
  }
  $names = @()
  foreach ($line in $out) {
    if ($line -match '^\s*DLL Name:\s*(\S+)\s*$') { $names += $Matches[1] }
  }
  Write-Output -NoEnumerate $names
  return
}

function Resolve-Dlls {
  # Every DLL the EXE needs, transitively: an import present in the build folder is shipped (and its own imports
  # are followed); an import that is part of Windows is skipped; anything else stops with its name.
  # Returns a hashtable upper-cased name -> full path, or $null (message already printed).
  param([string]$BuildDir, [string]$ExePath)
  $found = @{}
  $visited = @{}
  $stack = New-Object System.Collections.Generic.List[string]
  $stack.Add($ExePath)
  while ($stack.Count -gt 0) {
    $p = $stack[$stack.Count - 1]
    $stack.RemoveAt($stack.Count - 1)
    $key = $p.ToUpperInvariant()
    if ($visited.ContainsKey($key)) { continue }
    $visited[$key] = $true
    $imports = Get-Imports -Path $p
    if ($null -eq $imports) { return $null }
    foreach ($dll in $imports) {
      $up = $dll.ToUpperInvariant()
      if ($visited.ContainsKey($up)) { continue }
      if (Test-WindowsDll $up) { continue }
      $cand = Join-Path $BuildDir $dll
      if (Test-Path $cand -PathType Leaf) {
        if (-not $found.ContainsKey($up)) { $found[$up] = $cand }
        $stack.Add($cand)
      }
      else {
        [Console]::Error.WriteLine("package: DLL '$dll', imported by '$p', is neither in the build folder '$BuildDir' nor part of Windows")
        return $null
      }
    }
  }
  return $found
}

function Test-OplLinked {
  # True when port/src/opl.c is in the game's file list ($game = @(...)) of build_port.ps1, i.e. Nuked-OPL3 is
  # statically linked into advciv.exe and its licence must ship.
  param([string]$BuildPortPath)
  if (-not (Test-Path $BuildPortPath -PathType Leaf)) { return $false }
  $text = [System.IO.File]::ReadAllText($BuildPortPath)
  $m = [regex]::Match($text, '\$game\s*=\s*@\((.*?)\)', 'Singleline')
  if (-not $m.Success) { return $false }
  return ($m.Groups[1].Value -match "'opl\.c'")
}

function New-ReadmeLines {
  # README.txt's lines (plain text, CRLF when written, at most 80 columns each).
  param([string]$Version, [string[]]$LicenceLines)
  $l = New-Object System.Collections.Generic.List[string]
  $l.Add('Advanced Civilization - native Windows port')
  $l.Add("Version: $Version")
  $l.Add('')
  $l.Add('WHAT THIS IS')
  $l.Add('A native Windows port of the 1995 DOS game Advanced Civilization')
  $l.Add('(Avalon Hill). The original game''s files are needed to play and are')
  $l.Add('NOT included in this package.')
  $l.Add('')
  $l.Add('INSTALL')
  $l.Add('Unzip the AdvCiv folder into the folder of an installed Advanced')
  $l.Add('Civilization - the one holding AC_MAIN.EXE and ACIV.CT1..ACIV.CT5 -')
  $l.Add('and start AdvCiv\advciv.exe. Or run, from anywhere:')
  $l.Add('    advciv.exe "C:\path\to\the\game folder"')
  $l.Add('The game reads and writes its files in that folder, exactly as the')
  $l.Add('original does: save games (SAVGAME*.DAT) and the settings file')
  $l.Add('ACIV.CFG are written there.')
  $l.Add('')
  $l.Add('SWITCHES')
  $l.Add('    advciv.exe [GAMEDIR] [--original] [--srd N]')
  $l.Add('--original keeps the 1995 behaviour, including its bugs, wherever the')
  $l.Add('port can reproduce it; without it the bugs are fixed (see FIXES.md in')
  $l.Add('the port''s source). --srd N runs the self-running demo (the original''s')
  $l.Add('/SRDn). F11 or Alt+Enter toggles full screen.')
  $l.Add('')
  $l.Add('LICENCES')
  $l.Add('Third-party parts in this package; the full licence texts are in LICENSES\.')
  foreach ($line in $LicenceLines) { $l.Add($line) }
  $l.Add('')
  $l.Add('The port''s own source is the advciv branch of the Centurion repository')
  $l.Add('(github.com/Fulvia40bc/Centurion).')
  return $l
}

function Invoke-Package {
  param(
    [string]$Build, [string]$Out, [string]$Version, [string]$Objdump, [string]$Repo, [string]$BuildPort,
    [string]$Sdl3License, [string]$FluidSynthLicense, [string]$LlvmMingwLicense,
    [string]$Soundfont, [string]$SoundfontLicense, [string]$NukedLicense,
    [hashtable]$ImportTable, [string]$ImportTableJson
  )
  $ErrorActionPreference = 'Continue'
  if ($ImportTableJson) {
    $obj = Get-Content $ImportTableJson -Raw | ConvertFrom-Json
    $ImportTable = @{}
    foreach ($p in $obj.PSObject.Properties) { $ImportTable[$p.Name] = @($p.Value) }
  }
  if (-not $BuildPort)   { $BuildPort   = Join-Path $Repo 'tools\build_port.ps1' }
  if (-not $NukedLicense) { $NukedLicense = Join-Path $Repo 'port\third_party\nuked-opl3\LICENSE' }
  if (-not (Test-Path $BuildPort -PathType Leaf)) {
    [Console]::Error.WriteLine("package: build_port.ps1 not found at '$BuildPort' (pass -Repo or -BuildPort) - it decides whether Nuked-OPL3 is linked")
    return 1
  }
  if (-not $Version) {
    $v = ''
    try { $v = (& git -C $Repo describe --always --dirty 2>$null) } catch { $v = '' }
    if (-not $v) { $v = 'unknown' }
    $Version = (Get-Date -Format 'yyyyMMdd') + '-' + $v.Trim()
  }
  New-Item -ItemType Directory -Force $Out | Out-Null
  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $rc = 0
  foreach ($arch in @('aarch64', 'x86_64')) {
    $tag = 'arm64'; if ($arch -eq 'x86_64') { $tag = 'x64' }
    $dir = Join-Path $Build $arch
    $exe = Join-Path $dir 'advciv.exe'
    if (-not (Test-Path $exe -PathType Leaf)) {
      [Console]::Error.WriteLine("package: no advciv.exe in '$dir'")
      $rc = 1
      continue
    }
    $dlls = Resolve-Dlls -BuildDir $dir -ExePath $exe
    if ($null -eq $dlls) { $rc = 1; continue }
    # the chosen files: the exe and its DLLs. Refuse any whose name is one of the original game's.
    $chosen = @($exe) + @($dlls.Values)
    $bad = $false
    foreach ($c in $chosen) {
      $leaf = Split-Path -Leaf $c
      if (Test-GameFileName $leaf) {
        [Console]::Error.WriteLine("package: refusing to package '$c': its name matches the original game's files")
        $bad = $true
      }
    }
    if ($bad) { $rc = 1; continue }

    # which third-party parts ship, and their licence files
    $licences = New-Object System.Collections.Generic.List[object]   # @{ Src=...; Dst='SDL3.txt'; Lines=@(...) }
    $extraFiles = New-Object System.Collections.Generic.List[object]  # @{ Src=...; Dst='GeneralUser-GS.sf2' }
    if ($dlls.ContainsKey('SDL3.DLL')) {
      $licences.Add(@{ Src = $Sdl3License; Dst = 'SDL3.txt'; Lines = @(
        '- SDL3 3.4.20, zlib licence: LICENSES/SDL3.txt.',
        '  Source: the release-3.4.20 tag of github.com/libsdl-org/SDL, built by',
        '  tools/build_deps.ps1 in the port''s source repository.') })
    }
    if ($dlls.ContainsKey('LIBFLUIDSYNTH-3.DLL')) {
      $licences.Add(@{ Src = $FluidSynthLicense; Dst = 'FluidSynth.txt'; Lines = @(
        '- FluidSynth 2.6.2, LGPL-2.1: LICENSES/FluidSynth.txt. It ships as',
        '  libfluidsynth-3.dll, which you may replace with your own build. Its',
        '  exact source is the v2.6.2 tag of github.com/FluidSynth/fluidsynth,',
        '  built by tools/build_deps.ps1 in the port''s source repository.') })
      # the soundfont ships only together with FluidSynth
      $extraFiles.Add(@{ Src = $Soundfont; Dst = 'GeneralUser-GS.sf2' })
      $licences.Add(@{ Src = $SoundfontLicense; Dst = 'GeneralUser-GS.txt'; Lines = @(
        '- GeneralUser GS 2.0.3 soundfont (ships with FluidSynth):',
        '  LICENSES/GeneralUser-GS.txt. Source: github.com/mrbumpy409/GeneralUser-GS.') })
    }
    if ($dlls.ContainsKey('LIBC++.DLL') -or $dlls.ContainsKey('LIBUNWIND.DLL')) {
      $licences.Add(@{ Src = $LlvmMingwLicense; Dst = 'llvm-mingw-runtime.txt'; Lines = @(
        '- llvm-mingw runtime DLLs libc++.dll / libunwind.dll, Apache-2.0 with the',
        '  LLVM exception: LICENSES/llvm-mingw-runtime.txt. Source: the llvm-mingw',
        '  20261104 ucrt build (github.com/mstorsjo/llvm-mingw).') })
    }
    if (Test-OplLinked $BuildPort) {
      $licences.Add(@{ Src = $NukedLicense; Dst = 'Nuked-OPL3.txt'; Lines = @(
        '- Nuked-OPL3, LGPL-2.1, statically linked into advciv.exe:',
        '  LICENSES/Nuked-OPL3.txt. LGPL: you may relink it; the full source of this',
        '  port, including that library, is the advciv branch of the Centurion',
        '  repository (github.com/Fulvia40bc/Centurion) - ask the distributor for',
        '  a copy if you cannot reach it.') })
    }
    foreach ($lic in $licences) {
      if (-not (Test-Path $lic.Src -PathType Leaf)) {
        [Console]::Error.WriteLine("package: licence file '$($lic.Src)' for $($lic.Dst) is missing (its part ships)")
        $rc = 1
      }
    }
    foreach ($ef in $extraFiles) {
      if (-not (Test-Path $ef.Src -PathType Leaf)) {
        [Console]::Error.WriteLine("package: '$($ef.Src)' is missing (its part ships)")
        $rc = 1
      }
    }
    if ($rc -ne 0) { continue }

    # stage AdvCiv\ and zip it
    $work = Join-Path ([System.IO.Path]::GetTempPath()) ('advciv_pkg_' + [guid]::NewGuid().ToString('N'))
    $adv = Join-Path $work 'AdvCiv'
    New-Item -ItemType Directory -Force (Join-Path $adv 'LICENSES') | Out-Null
    try {
      Copy-Item $exe (Join-Path $adv 'advciv.exe') -Force
      foreach ($up in @($dlls.Keys)) {
        Copy-Item $dlls[$up] (Join-Path $adv (Split-Path -Leaf $dlls[$up])) -Force   # keep the file's own name
      }
      foreach ($ef in $extraFiles) { Copy-Item $ef.Src (Join-Path $adv $ef.Dst) -Force }
      foreach ($lic in $licences) { Copy-Item $lic.Src (Join-Path $adv ('LICENSES\' + $lic.Dst)) -Force }
      $readmeLines = New-ReadmeLines -Version $Version -LicenceLines @($licences | ForEach-Object { $_.Lines })
      foreach ($line in $readmeLines) {
        if ($line.Length -gt 80) {
          [Console]::Error.WriteLine("package: README line over 80 columns: '$line'")
          $rc = 1
        }
      }
      if ($rc -ne 0) { continue }
      $utf8 = New-Object System.Text.UTF8Encoding($false)
      [System.IO.File]::WriteAllText((Join-Path $adv 'README.txt'), (($readmeLines -join "`r`n") + "`r`n"), $utf8)

      $zipName = "advciv-$Version-windows-$tag.zip"
      $zipPath = Join-Path $Out $zipName
      if (Test-Path $zipPath) { Remove-Item $zipPath -Force }   # same name: replaced; nothing else in -Out touched
      $z = [System.IO.Compression.ZipFile]::Open($zipPath, 'Create')
      try {
        foreach ($f in Get-ChildItem -Recurse -File $adv) {
          $rel = 'AdvCiv/' + $f.FullName.Substring($adv.Length + 1).Replace('\', '/')
          $e = $z.CreateEntry($rel)
          $fs = [System.IO.File]::OpenRead($f.FullName)
          $es = $e.Open()
          $fs.CopyTo($es)
          $es.Dispose()
          $fs.Dispose()
        }
      }
      finally { $z.Dispose() }

      # print the zip's file list with sizes and SHA-256
      Write-Host $zipName
      $zr = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
      try {
        foreach ($en in $zr.Entries) {
          $sha = [System.Security.Cryptography.SHA256]::Create()
          $s = $en.Open()
          $h = $sha.ComputeHash($s)
          $s.Dispose()
          $sha.Dispose()
          $hex = ([System.BitConverter]::ToString($h)).Replace('-', '').ToLowerInvariant()
          Write-Host ("  {0}  {1}  {2}" -f $en.FullName, $en.Length, $hex)
        }
      }
      finally { $zr.Dispose() }
    }
    finally {
      Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
    }
  }
  return $rc
}

# Pass every script parameter explicitly: @PSBoundParameters would carry only the ones the caller typed,
# and Invoke-Package's own parameters have no defaults, so a left-out one would arrive as '' and hide the
# script-level default (TASK 21a).
exit (Invoke-Package -Build $Build -Out $Out -Version $Version -Objdump $Objdump -Repo $Repo -BuildPort $BuildPort `
  -Sdl3License $Sdl3License -FluidSynthLicense $FluidSynthLicense -LlvmMingwLicense $LlvmMingwLicense `
  -Soundfont $Soundfont -SoundfontLicense $SoundfontLicense -NukedLicense $NukedLicense `
  -ImportTable $ImportTable -ImportTableJson $ImportTableJson)
