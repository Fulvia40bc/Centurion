# DOSBox-X control kit for Advanced Civilization (from the Centurion kit, coordinates 640x480). One action per call:
#   drive.ps1 shot NAME            -> screenshot of the game window's client area -> hunt\shots\NAME.png
#   drive.ps1 click X Y            -> left click at game coords (0..639, 0..479)
#   drive.ps1 key KEYS             -> key names separated by '+', e.g. Return, Escape, Up, F1, a, Alt+Pause
#   drive.ps1 dbg "COMMAND"        -> type a command into the debugger console (no focus needed)
#   drive.ps1 dbgkey F5            -> press a key in the debugger console
#   drive.ps1 dbgscreen            -> print the debugger console text
#   drive.ps1 menu "TEXT"          -> run a DOSBox-X menu command by its text, e.g. "Take raw screenshot" (no focus)
# Clicks/keys are sent only after verifying the game window is the foreground window.
param([string]$Action, [string]$A1, [string]$A2)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Text; using System.Runtime.InteropServices; using System.Collections.Generic;
public class D {
  public delegate bool P(IntPtr h, IntPtr l);
  [DllImport("user32")] public static extern bool EnumWindows(P p, IntPtr l);
  [DllImport("user32")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [DllImport("user32")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32")] public static extern void mouse_event(uint f, int x, int y, uint d, UIntPtr e);
  [DllImport("user32")] public static extern void keybd_event(byte vk, byte scan, uint f, UIntPtr e);
  [DllImport("user32")] public static extern uint MapVirtualKey(uint c, uint t);
  [DllImport("user32")] public static extern bool SetProcessDPIAware();
  public struct RECT { public int L, T, R, B; }
  public struct POINT { public int X, Y; }
  public static IntPtr Find(uint pid, string contains, bool exact) { IntPtr f = IntPtr.Zero;
    EnumWindows((h,l)=>{ uint p; GetWindowThreadProcessId(h,out p); if (p==pid && IsWindowVisible(h)) { var sb=new StringBuilder(256); GetWindowText(h,sb,256); var t=sb.ToString(); if (exact ? t==contains : t.Contains(contains)) f=h; } return true; }, IntPtr.Zero); return f; }
  [StructLayout(LayoutKind.Explicit)] public struct REC {
    [FieldOffset(0)] public ushort Type; [FieldOffset(4)] public int Down; [FieldOffset(8)] public ushort Rep;
    [FieldOffset(10)] public ushort Vk; [FieldOffset(12)] public ushort Scan; [FieldOffset(14)] public char Ch; [FieldOffset(16)] public uint Ctrl; }
  [DllImport("kernel32")] static extern bool FreeConsole();
  [DllImport("kernel32")] static extern bool AttachConsole(uint pid);
  [DllImport("kernel32")] static extern IntPtr CreateFile(string n, uint a, uint s, IntPtr sa, uint c, uint f, IntPtr t);
  [DllImport("kernel32")] static extern bool CloseHandle(IntPtr h);
  [DllImport("kernel32")] static extern bool WriteConsoleInput(IntPtr h, REC[] r, uint n, out uint w);
  [DllImport("kernel32")] static extern bool ReadConsoleOutputCharacter(IntPtr h, StringBuilder s, uint len, uint coord, out uint read);
  [DllImport("kernel32")] static extern bool GetConsoleScreenBufferInfo(IntPtr h, byte[] info);
  [DllImport("user32")] static extern short VkKeyScan(char c);
  static IntPtr Con(uint pid, string n) { FreeConsole(); if (!AttachConsole(pid)) throw new Exception("attach failed"); return CreateFile(n, 0xC0000000, 3, IntPtr.Zero, 3, 0, IntPtr.Zero); }
  public static void DbgType(uint pid, string s, ushort fkey) {
    var l = new List<REC>();
    foreach (char c in s) { ushort vk = c=='\r' ? (ushort)13 : (ushort)(VkKeyScan(c) & 0xFF);
      l.Add(new REC{Type=1,Down=1,Rep=1,Vk=vk,Ch=c}); l.Add(new REC{Type=1,Down=0,Rep=1,Vk=vk,Ch=c}); }
    if (fkey != 0) { l.Add(new REC{Type=1,Down=1,Rep=1,Vk=fkey}); l.Add(new REC{Type=1,Down=0,Rep=1,Vk=fkey}); }
    IntPtr h = Con(pid, "CONIN$"); uint w; WriteConsoleInput(h, l.ToArray(), (uint)l.Count, out w); CloseHandle(h); FreeConsole(); }
  [DllImport("user32")] static extern IntPtr GetMenu(IntPtr h);
  [DllImport("user32")] static extern int GetMenuItemCount(IntPtr m);
  [DllImport("user32")] static extern IntPtr GetSubMenu(IntPtr m, int i);
  [DllImport("user32")] static extern uint GetMenuItemID(IntPtr m, int i);
  [DllImport("user32", CharSet=CharSet.Unicode)] static extern int GetMenuString(IntPtr m, uint i, StringBuilder s, int n, uint f);
  [DllImport("user32")] static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  static uint FindItem(IntPtr m, string text, List<string> seen) {
    int n = GetMenuItemCount(m);
    for (int i = 0; i < n; i++) {
      var sb = new StringBuilder(256); GetMenuString(m, (uint)i, sb, 256, 0x400);
      string t = sb.ToString().Replace("&", ""); int tab = t.IndexOf('\t'); if (tab >= 0) t = t.Substring(0, tab);
      IntPtr sub = GetSubMenu(m, i);
      if (sub != IntPtr.Zero) { uint r = FindItem(sub, text, seen); if (r != 0) return r; }
      else { seen.Add(t); if (t.Equals(text, StringComparison.OrdinalIgnoreCase)) return GetMenuItemID(m, i); }
    }
    return 0; }
  // Run a menu command of the window's (Win32) menu bar by its text: WM_COMMAND, no focus, no keystrokes.
  public static string Menu(IntPtr win, string text) {
    IntPtr m = GetMenu(win); if (m == IntPtr.Zero) return "no menu bar";
    var seen = new List<string>(); uint id = FindItem(m, text, seen);
    if (id == 0 || id == 0xFFFFFFFF) return "menu item not found: " + text + " (items: " + string.Join(" | ", seen) + ")";
    PostMessage(win, 0x0111, (IntPtr)id, IntPtr.Zero); return "menu: " + text + " (id " + id + ")"; }
  // A left click by posting the window messages (no focus, no real cursor). Tried 2026-11-18: DOSBox-X (SDL)
  // ignores posted mouse messages, so it does nothing there; use pokeclick.sh. x, y: client pixels.
  public static void PostClick(IntPtr win, int x, int y) {
    IntPtr lp = (IntPtr)((y << 16) | (x & 0xFFFF));
    PostMessage(win, 0x0200, IntPtr.Zero, lp); System.Threading.Thread.Sleep(120);
    PostMessage(win, 0x0201, (IntPtr)1, lp); System.Threading.Thread.Sleep(120);
    PostMessage(win, 0x0202, IntPtr.Zero, lp); }
  public static string DbgScreen(uint pid) {
    IntPtr h = Con(pid, "CONOUT$"); byte[] info = new byte[22]; GetConsoleScreenBufferInfo(h, info);
    int w = BitConverter.ToInt16(info,0), top = BitConverter.ToInt16(info,12), bot = BitConverter.ToInt16(info,16);
    var o = new StringBuilder(); for (int y = top; y <= bot; y++) { var sb = new StringBuilder(w); uint r; ReadConsoleOutputCharacter(h, sb, (uint)w, (uint)(y<<16), out r); o.AppendLine(sb.ToString().TrimEnd()); }
    CloseHandle(h); FreeConsole(); return o.ToString(); }
}
"@
[D]::SetProcessDPIAware() | Out-Null
$proc = Get-Process dosbox-x -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $proc) { throw 'DOSBox-X is not running' }
$pid_ = [uint32]$proc.Id
$game = [D]::Find($pid_, "Advanced Civilization", $false); if ($game -eq [IntPtr]::Zero) { $game = [D]::Find($pid_, "DOSBox-X", $false) }
function ClientRect { $r = New-Object D+RECT; [D]::GetClientRect($game, [ref]$r) | Out-Null; $p = New-Object D+POINT; [D]::ClientToScreen($game, [ref]$p) | Out-Null; @{X=$p.X; Y=$p.Y; W=$r.R; H=$r.B} }
function Front { if ([D]::GetForegroundWindow() -ne $game) { [D]::keybd_event(0x12,0,0,[UIntPtr]::Zero); [D]::keybd_event(0x12,0,2,[UIntPtr]::Zero); [D]::ShowWindow($game, 9) | Out-Null; [D]::SetForegroundWindow($game) | Out-Null; Start-Sleep -Milliseconds 300 }
  if ([D]::GetForegroundWindow() -ne $game) { throw 'game window not in front - nothing sent' } }
$vk = @{ Return=0x0D; Enter=0x0D; Escape=0x1B; Esc=0x1B; Space=0x20; Up=0x26; Down=0x28; Left=0x25; Right=0x27; Tab=0x09; Alt=0x12; Ctrl=0x11; Shift=0x10; Pause=0x13; Backspace=0x08; Home=0x24; End=0x23; PgUp=0x21; PgDn=0x22; F1=0x70; F2=0x71; F3=0x72; F4=0x73; F5=0x74; F6=0x75; F7=0x76; F8=0x77; F9=0x78; F10=0x79; F11=0x7A; F12=0x7B }
1..12 | ForEach-Object { $vk["F$_"] = 0x6F + $_ }
switch ($Action) {
  'shot' {
    if ($game -eq [IntPtr]::Zero) { throw 'game window not found' }
    Front; $c = ClientRect
    $bmp = New-Object System.Drawing.Bitmap $c.W, $c.H; $g = [System.Drawing.Graphics]::FromImage($bmp); $g.CopyFromScreen($c.X, $c.Y, 0, 0, $bmp.Size); $g.Dispose()
    $small = New-Object System.Drawing.Bitmap 640, ([int](640 * $c.H / $c.W)); $g2 = [System.Drawing.Graphics]::FromImage($small); $g2.InterpolationMode = 'NearestNeighbor'; $g2.DrawImage($bmp, 0, 0, $small.Width, $small.Height); $g2.Dispose()
    New-Item -ItemType Directory -Force "$PSScriptRoot\shots" | Out-Null
    $small.Save("$PSScriptRoot\shots\$A1.png", [System.Drawing.Imaging.ImageFormat]::Png); "shot $A1 (client $($c.W)x$($c.H))" }
  'click' {
    Front; $c = ClientRect
    $sx = $c.X + [int](([double]$A1 + 0.5) * $c.W / 640); $sy = $c.Y + [int](([double]$A2 + 0.5) * $c.H / 480)
    [D]::SetCursorPos($sx, $sy) | Out-Null; Start-Sleep -Milliseconds 150
    [D]::mouse_event(0x0002,0,0,0,[UIntPtr]::Zero); Start-Sleep -Milliseconds 90; [D]::mouse_event(0x0004,0,0,0,[UIntPtr]::Zero); "clicked $A1,$A2" }
  'move' { Front; $c = ClientRect; [D]::SetCursorPos($c.X + [int](([double]$A1 + 0.5) * $c.W / 640), $c.Y + [int](([double]$A2 + 0.5) * $c.H / 480)) | Out-Null; "moved" }
  'key' {
    Front; $keys = $A1.Split('+') | ForEach-Object { if ($vk.ContainsKey($_)) { [byte]$vk[$_] } else { [byte][char]$_.ToUpper() } }
    foreach ($k in $keys) { [D]::keybd_event($k, [byte][D]::MapVirtualKey($k,0), 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 40 }
    Start-Sleep -Milliseconds 80
    [array]::Reverse($keys); foreach ($k in $keys) { [D]::keybd_event($k, [byte][D]::MapVirtualKey($k,0), 2, [UIntPtr]::Zero); Start-Sleep -Milliseconds 30 }; "key $A1" }
  'type' {
    Front
    foreach ($ch in $A1.ToCharArray()) {
      $s = [D]::MapVirtualKey(0,0) | Out-Null
      $code = [int][char]::ToUpper($ch); $shift = [char]::IsUpper($ch)
      if ($ch -eq ' ') { $code = 0x20 }
      if ($shift) { [D]::keybd_event(0x10,[byte][D]::MapVirtualKey(0x10,0),0,[UIntPtr]::Zero) }
      [D]::keybd_event([byte]$code,[byte][D]::MapVirtualKey($code,0),0,[UIntPtr]::Zero); Start-Sleep -Milliseconds 60
      [D]::keybd_event([byte]$code,[byte][D]::MapVirtualKey($code,0),2,[UIntPtr]::Zero)
      if ($shift) { [D]::keybd_event(0x10,[byte][D]::MapVirtualKey(0x10,0),2,[UIntPtr]::Zero) }
      Start-Sleep -Milliseconds 60 }
    "typed $A1" }
  'dbg' { [D]::DbgType($pid_, ("`b" * 250) + "$A1`r", 0); Start-Sleep -Milliseconds 1200; "dbg: $A1" }
  'dbgkey' { [D]::DbgType($pid_, '', [uint16]$vk[$A1]); Start-Sleep -Milliseconds 800; "dbgkey $A1" }
  'dbgscreen' { [D]::DbgScreen($pid_) }
  'menu' { [D]::Menu($game, $A1) }
  'postclick' { $c = ClientRect; [D]::PostClick($game, [int](([double]$A1 + 0.5) * $c.W / 640), [int](([double]$A2 + 0.5) * $c.H / 480)); "posted click $A1,$A2" }
  default { throw "unknown action $Action" }
}
