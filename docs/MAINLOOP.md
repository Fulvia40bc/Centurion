# Main loop & frame path (from clean runtime EXE image, disasm/EXE_RUNTIME.asm)

## Architecture
- EXE owns the frame loop, video (int 0x10 palette / direct VRAM blit), DOS I/O,
  and the int 0x08/0x09 ISRs. The overlay (OVL_RUNTIME.BIN) owns game logic +
  sound, entered via far pointer [0xC4] dispatched on AH (table at ovl 0x03DE),
  and receives keyboard events via far pointer [0x9E] from the int 0x09 ISR
  (func 0x042F in EXE: scancode in, arrows 0x47-0x51 mapped, lcall [0x9E]).

## Frame path
- Work buffer: off-screen 320x200 buffer (runtime seg 2687); VRAM seg in
  global [0x9ABA] = 0xA000.
- Blit helper func_0x689F: rep movsw 9 words, then call 0x0494 with
  [0xC1DE] (work buffer) and [0x9ABA] (VRAM) -> copies work buffer to screen.

## Screen builder func_0x62C2 (pseudocode)
```
ui_begin(1)            // 0x5AC2(1)
ui_?( )                // 0x7CB2: sets [0x9B69]=4, rect(0xD4,0xB7,...)
call_overlay(8)        // 0x5D29(8)
window([0xC1EC]+0x1C, 0, 0)   // 0x8E62
[0x9AFA] = [0x6344]
rect(0xC6, 0x6B, 0x97)        // 0x7E74
ui_?(0x4DD4, 0, 0)            // 0x5AE4
ui_begin(1)
ui_?(0x4E28, 0, 0)
ui_begin(1)
ui_?()                        // 0x7CB2
window([0xC1EC]+0xE, 0, 0)
window([0xC1EC], 8, 0x4F)     // 0x8E62
... loop over items at [bp+6] ...
```

## Menu loop func_0x49CE (pseudocode)
```
for (di = start; di <= 0x55C5; di += 0x20, si += 4)
    if (*di) call_0x22EA(*si, *(si+2), 9)     // add menu item
call_0x22E2(); call_0x4A24([0x634C]); 0x4AA6(); 0x2AF8(); 0x423A();
0x53BC(); 0x680F(); draw_rect(0xC7, 0x13F, 0, 0); call_overlay(1); call_0x1D8A();
```

## Port mapping
- port keeps an offscreen 320x200 work buffer; each frame: game logic draws
  into work buffer, then present() blits work buffer -> SDL2 framebuffer.
- Keyboard arrives via SDL events -> port forwards to the same callback shape
  as the original's [0x9E] handler.
