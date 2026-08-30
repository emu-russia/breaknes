# Breaknes

NES/Famicom/Dendy emulator at the gate level.

## Two TV Sets (issue #515)

The emulator supports up to 2 virtual TV Sets. The video signal of each PPU of the
motherboard is bound to a TV Set; the binding is configured in the `BoardDescription.json`:

- `ppus` - the PPU revisions of the board (array, up to 2). The legacy single `ppu` field is still supported.
- `tvs` - the TV binding: `tvs[i]` is the index of the PPU whose signal is displayed on the i-th TV. The default binding is `TV[i]` shows `PPU[i]`. For debugging, one PPU can be bound to both TVs at once (`"tvs": [0, 0]`).
- `tv_layout` - the physical arrangement of the TVs in the window: `"horizontal"` (side by side) or `"vertical"` (one above the other).

Select a debug board ("NES Debug: one PPU on two TVs (horizontal)" or "NES Debug: one PPU on two TVs (vertical)") in the settings to see the two-TV window in action.

## Build

Use VS2022.

To make the generation of the native and managed parts in the same Build folder, you need to use this:

https://docs.microsoft.com/en-us/answers/questions/202445/output-path-ignored-with-net-5-c-classes-lib.html
