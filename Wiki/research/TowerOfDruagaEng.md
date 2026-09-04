# Investigating The Tower of Druaga (ドルアーガの塔)

Task: [issue #527](https://github.com/emu-russia/breaknes/issues/527) — verify the game's
behaviour described in the nesdev Discord chat and check that the game works in Breaknes.

Summary:

1. The game **did not load** in Breaknes: the `NAMCOT-3305` board type was missing from the
   family index `Nescartdb/boards/index.json`, so `PcbLoader` could not resolve it to a board
   definition. Added the `NAMCOT-3305 → nrom.json` mapping — the cartridge loads now.
2. After that fix the game **loads and runs its boot sequence** (the `"YAMAMO"` sentinel, the
   first-boot flag `$26`), **but did not show the title screen**: the game's NMI handler
   re-entered itself 5 times per frame, overflowing the stack. The cause was a timing bug in
   the `$2000` write path (see §4): bit `$2000[7]` (NMI enable) briefly reset during the write,
   which made the NMI line glitch and the 6502 re-take the NMI mid-handler.
3. The glitch from the nesdev Discord chat (RAM corruption bypassing the first-boot check) was
   confirmed and reproduced by pre-loading the sentinel string into RAM before startup.

## 1. Cartridge identification

The dump matches the nescartdb record (MD5 `b63a55584796c106bcfd3e2c08ea1e3c`; PRG CRC32
`AF6E8571`, CHR `6DC7E8EA`, combined CRC `EB764567`):

| Parameter | Value |
|---|---|
| Game | The Tower of Druaga (Namco, Famicom, 1985-08-06) |
| Board | **NAMCOT-3305** (PCB 3305), class NAMCOT-3305 |
| iNES mapper | 0 |
| PRG | 32 KB (2×16K) |
| CHR | 8 KB |
| Mirroring | vertical (iNES header bit 0 = 1; nescartdb profile "Mirroring: Vertical"; board solder pads `h=1` = H Scroll, `VRAM_A10 = PA10`) |
| CIC | none |

`NAMCOT-3305` is electrically an NROM-256: the board carries only two mask ROMs (PRG Fujitsu
MB83256, CHR Sharp LH2367) and no extra logic. **Pac-Land** uses the same board. The mirroring
agrees across all three sources; in Breaknes the scroll jumper follows the iNES header
(`ApplyScrollFromHeader`), which matches the real board.

## 2. The "first boot" mechanism (sentinel string)

The boot code at the `$8000` vector (confirmed by disassembly and emulation):

```
$801E-$8031  clear $0500-$07FF (indirect STA ($00),Y; $0000-$04FF untouched)
$8033-$8040  check: RAM $0002-$0007 == ROM $8044-$8049 ("YAMAMO"), 6 bytes
$803B        BNE $804A  -> mismatch: first boot
$8042        BEQ $8072  -> match:    warm reset (continue state from RAM)
$8044-$8049  data: "YAMAMO"
$804A-$8052  [first boot] clear zero page $0000-$00FF
$8054-$805A  [first boot] JSR $91A2, JSR $938D, JSR $91B4 (initialisation)
$8061-$806C  [first boot] copy "YAMAMO" from $8044 to $0002-$0007
$806E-$8070  [first boot] $26 = 1 (first-boot flag)
$8072+       common path: JSR $A120, scroll/high-score initialisation, etc.
```

If `"YAMAMO"` is already present in RAM at `$0002-$0007` at startup, the game treats it as a
continuation (a reset without power loss) and restores its state from RAM — exactly what ykst
described in the nesdev Discord chat.

In Breaknes: SRAM is zeroed on power-up (`BaseBoard::SRAM`, "all memory cells read as 0") — a
deterministic emulator choice; real SRAMs have an undefined power-up state. For this game a
zeroed RAM yields the correct first boot. On reset (`Board::Reset`) the SRAM is **not** cleared,
so a subsequent reset finds the sentinel → "warm reset", matching the real behaviour.

## 3. The glitch (RAM corruption, nesdev Discord)

Rapid power-off/on partially corrupts the RAM. If the sentinel string survives while the rest of
the state is corrupted, the game takes the "warm reset" branch with damaged data: wrong high
score, a beep (possibly entering debug mode), lots of equipment on floor 1, up to an instant
ending.

Reproduction in Breaknes: write `"YAMAMO"` to RAM `$0002-$0007` and corrupt the rest of the RAM
(DebugHub `memMap` / the debugger), then start — the game goes to the `$8072` branch without the
first-boot initialisation (the `$26` flag stays garbage, the zero page is not cleared). Verified
with a headless build (the `--glitch` mode of the test harness, see `Temp/boottest.cpp`).

## 4. The title screen problem (NMI storm)

After booting, the game enters its idle loop `$80E1` (waiting for `$0C`) with NMI enabled. The
NMI handler (`$828E`) starts with an OAM DMA (`STA $4014 = $05`, 513 cycles), then writes
`$2000 = $B4` (`$82A3`) and only later, at `$82FD`, reads `$2002` (clearing the VBlank flag).
In Breaknes this caused:

- While the VBlank flag is set, the NMI line is low. The `$2000` write in the handler caused a
  **brief (4 steps) reset of bit `$2000[7]`** (NMI enable / `wire.VBL`) — the NMI line glitched
  high and low, giving the 6502 edge detector a fresh edge.
- The 6502 re-took the NMI **mid-handler** (I=1 does not block NMI): another OAM DMA
  (513 cycles) and 7 more stack bytes per re-entry. The handler never reached `$82FD`, the flag
  was never cleared — the storm repeated at V=241, 245, 250, 255, 260 (5 entries per frame).
- The stack overflowed and the game hung on a black screen (no title).

**Evidence** (headless build, Famicom/HVC board):

1. Nintendulator trace: 5 NMI entries per frame (V=241, 245, 250, 255, 260), the stack
   monotonically decreased (S: F8→F1→EA→E3→DC→...), the handler was interrupted at `$82A8`.
2. `$2002` showed vb=1 almost constantly; reads of `$2002` outside the handler saw the flag clear.
3. ROM patch "remove `STA $2000` from the NMI handler" — the storm disappeared (3 entries in 4
   frames instead of 10), the game worked.
4. ROM patch "read `$2002` right after the DMA in the handler" — the storm also disappeared.
5. For comparison: SMB (whose NMI handler deasserts the NMI line early) works correctly;
   DK (no NMI use) works correctly.

**Conclusion and fix**: the root cause is the `/DBE` formation (the PPU's `n_DBE` input) on the
board. The PPU `/CS` (the LS139 output, gated by `M2`) is active not only during PHI2 but also
during the 3 PHI1 steps that precede it (M2 leads PHI2). The PPU registers sample the data bus
level-sensitively while the write strobe is active — and during those 3 PHI1 steps the bus still
holds the value of the previous read (e.g. the high operand byte of the `STA`). So a
`STA $2000 = $B4` right after an operand read briefly resets `CTRL0[7]` (NMI enable) to 0.
Because of the one-step delay (the PPU updates the NMI output after the core samples its input),
the 6502 edge detector sees a false deassert/assert of the NMI line and re-takes the interrupt.

At the same time, for **reads** the `/CS` must stay active for the whole PHI1 phase: the 6502
core samples the data bus at the start of PHI2, so the PPU has to drive the read data early
(during PHI1). Gating the whole `/CS` with PHI2 breaks reads.

**Fix** (board only, the PPU interface is unchanged): the `/CS` stays active the whole cycle for
reads, but for writes it is gated with the PHI2 phase:

```cpp
// NESBoard.cpp / FamicomBoard.cpp
ppu_inputs[PPUSim::InputPad::n_DBE] = OR(PPU_nCE, AND(NOT(CPU_RnW), NOT(apu->GetPHI2())));
```

- read (`RnW = 1`): `/DBE` active the whole cycle → the PPU drives the data early → reads work
  as before;
- write (`RnW = 0`): `/DBE` active only during PHI2 (when the data bus is valid) → the PPU no
  longer samples the stale PHI1 data → the `CTRL0[7]` glitch disappears → the NMI storm stops.

After the fix the game **loads, runs its boot sequence and shows its title screen** (verified
with a headless build: 1 NMI per frame, `"YAMAMO"` at `$0002-$0007`, `$26 = 1`, the OAM DMA
works). SMB and Donkey Kong are unaffected.

## 5. Repository changes

- `Nescartdb/boards/index.json`: `NAMCOT-3305 → nrom.json` (the board is electrically an
  NROM-256).
- `Breaknes/BreaksCore/NESBoard.cpp`, `Breaknes/BreaksCore/FamicomBoard.cpp`: fixed the PPU
  `/DBE` formation (writes gated with the PHI2 phase) — eliminated the NMI storm that kept the
  game from showing its title screen.
- `UnitTest/CartPcbTest.cpp`: regression test `TestNamcot3305Board` (CRC identification, family
  resolution, end-to-end load of the real dump, sentinel string, mirroring).
- `Nescartdb/Readme.md`: NAMCOT-3305 mentioned in the family index description.

Temporary headless harness used for verification: `Temp/boottest.cpp` (not committed; the `Temp`
folder is in `.gitignore`).
