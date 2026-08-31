# CartPcb

Functional simulation of NES/Famicom/Dendy cartridge **PCBs** (printed circuit boards).

A cartridge is simulated as a physical board: a set of components (memory chips, mapper chips, discrete logic) and the wires (nets) that connect them to each other and to the cartridge edge connector. The description of a board is data — a JSON document — not C++ code.

CartPcb is the successor of the retired `Mappers` component (issue #509). It replaced `Mappers` completely and solved every problem listed in the old `Mappers/Readme.md`.

---

## 1. Motivation

The retired `Mappers` component had accumulated the following problems, all of which CartPcb solves:

| # | Problem (from the old `Mappers/Readme.md`) | How CartPcb solves it |
|---|-------------------------------------|------------------------|
| 1 | Working with memory dumps (PRG/CHR) is unclear; the CHR region is called "CHR-ROM", PRG is not supported at all. | CartPcb works with **explicit image dumps**: `CartImage` carries PRG and CHR images, sized and named per the PCB definition. CHR is just the CHR image; nothing is called "CHR-ROM" anymore. |
| 2 | Emulation of `.nes` mappers is chaotic: translating an iNES mapper number into board components, with no way to express the many hardware variations of the same mapper. | CartPcb is **PCB-centric and data-driven**. A board is a JSON document listing components and wiring. The same mapper chip (e.g. MMC1) wired differently = a different board JSON, not a fork of C++ code. |
| 3 | No ROM chip support; everything is a raw byte array. | CartPcb uses `RomChip` (JEDEC-style ROM, `Common/BaseBoardLib`) and the reworked `SRAM` (`Common/BaseBoardLib`), with `/CE` / `/OE` / `/WE` / address / data / `dz` semantics. |
| 4 | MMC1 emulation needs debugging; the divider is likely wrong. | MMC1 was extracted from Mappers into `Chips` as a standalone chip class with its own unit tests. CartPcb only wires chips into boards; it does not implement chips. |

In addition, the issue #508 specifies these architectural motivations:

- **No iNES dependency for identification.** iNES headers and iNES mapper numbers are rigid and lossy (many real boards share a mapper number but differ electrically).
- **Identification by nescartdb.** Cartridge/PCB type is identified by the CRC32 of the PRG/CHR images using the nescartdb database (converted to JSON, see `Nescartdb/Readme.md`).
- **JSONES becomes a subset.** A custom, user-authored PCB JSON (single-cartridge format, see §6.5) is a valid CartPcb input, so researchers can describe unlicensed/undocumented PCBs (e.g. krzysiobal's collection) without touching iNES.
- **`Mappers` is fully retired.** The cartridge-port contract (the former `Mappers::AbstractCartridge`) lives in CartPcb (see §7.5).
- **MMC1 lives in Chips.** CartPcb is *only* for PCB simulation.

## 2. Scope

### In scope

- Loading a board description (JSON) and instantiating the board: components + wiring.
- Simulation of memory chips on the CPU and PPU buses (ROM and RAM, battery-backed or not).
- Wiring of discrete logic and mapper **chips** (implemented elsewhere, e.g. `Chips`) into the board.
- Mirroring (hardwired and mapper-controlled), WRAM gating, bus conflicts where expressible.
- The cartridge edge-connector interface (the signal contract of today's `Mappers::AbstractCartridge`, now part of CartPcb).
- Identification: `NesCartDb` component that maps PRG/CHR CRCs to a PCB type.
- Debug support: memory-map descriptors and debug-info providers per board (as today).

### Out of scope (by design)

- Implementation of large chips (MMC1, MMC3, etc.) — that is `Chips`' job. CartPcb consumes chips.
- Audio-expansion chips are out of scope initially (the connector still carries the Famicom audio line; board authors can add simple pass-through wiring).
- Cycle-exact electrical behavior of every exotic board. CartPcb models the *functional* circuit; boards are added incrementally.

## 3. Terminology

- **PCB / board** — the cartridge printed circuit board; the thing being simulated.
- **Component** — a part on the board: a ROM chip, a RAM chip, a mapper chip, a CIC, a pull-up, etc.
- **Pin** — a connection point of a component (named, e.g. `n_CS`, `A13`, `D0`).
- **Net** — a wire connecting pins together. Nets carry `TriState` values.
- **Edge connector** — the cartridge port interface to the motherboard (CPU bus, PPU bus, control signals, audio, expansion port).
- **CartImage** — the raw dumps that are loaded into the ROM chips: PRG image, CHR image (and optional battery RAM).
- **ROM chip** — a read-only memory chip; most NES ROMs are JEDEC-standard devices or similar (e.g. the 27xx EPROM family). Modeled by `RomChip` in `Common/BaseBoardLib`.
- **JEDEC** — the industry-standard pinout family for memory chips (`A0..An`, `D0..D7`, `/CE`, `/OE`, optional `/WE`); `RomChip` and `SRAM` follow it.
- **Board type** — the nescartdb name of a board, e.g. `HVC-SGROM`, `NES-NROM-256`, `NES-UNROM`.
- **nescartdb** — the bootgod cartridge database (see `Nescartdb/Readme.md`).
- **JSONES** — the single-cartridge JSON meta-information format (issue #189); a subset of the CartPcb board format.

## 4. Design principles

1. **Data over code.** A board family is described once, in JSON. Adding a new PCB revision must not require C++ changes.
2. **Components are chips; boards are wiring.** CartPcb implements the net model and board assembly. Memory chips (`RomChip`, `SRAM`) live in `BaseBoardLib`; everything else (MMC1, ...) is a chip class injected from `Chips`.
3. **Explicit images.** PRG and CHR are named, sized regions of `CartImage`. The images are loaded into `RomChip` instances, and the board wiring attaches those ROM chips to the buses — never "the CHR-ROM byte array".
4. **Identification by content, not by header.** PRG/CHR CRC32s select the board through nescartdb.
5. **User-extensible.** Small JSON files in a user directory augment/override built-in boards (JSONES path).
6. **One simulation contract.** The edge-connector signal interface is the same as today's `AbstractCartridge`, so motherboards (`NESBoard`, `FamicomBoard`) keep working unchanged.

## 5. PCB description format (CartPcb JSON)

A board is described by a JSON document with two parts:

- **`components`** — the component inventory (maps 1:1 to the nescartdb board content: `prg`, `chr`, `wram`, `vram`, `chip`, `cic`, `pad`).
- **`circuit`** — the CartPcb extension: how the components are wired to the buses and to each other. nescartdb does not describe wiring, so this part is authored (see §5.3).

### 5.1 Example: NROM-256

```json
{
  "schemaVersion": 1,
  "board": {
    "type": "NES-NROM-256",
    "pcb": "NES-NROM-256-02",
    "mapper": 0,
    "system": ["NES-NTSC", "NES-PAL"],
    "components": {
      "prg": { "kind": "rom", "bus": "cpu", "size": 32768 },
      "chr": { "kind": "rom", "bus": "ppu", "size": 8192 }
    },
    "circuit": {
      "mirroring": { "mode": "scroll" },
      "cpu": {
        "prg": { "chip": "prg", "n_cs": "nROMSEL", "addr": "cpu_addr[13:0]" }
      },
      "ppu": {
        "chr": { "chip": "chr", "n_cs": "!nPA13", "addr": "ppu_addr[12:0]" }
      },
      "nets": []
    }
  }
}
```

Notes:
- Sizes are in **bytes** (nescartdb's `"256k"` is converted to `32768` by the conversion tool).
- `cpu_addr`, `ppu_addr` and the connector signals (`nROMSEL`, `nPA13`, `RnW`, `nRD`, `nWR`, `M2`, ...) are the predefined net sources; the full signal set is defined in §7.2.
- `prg` with `n_cs = nROMSEL` reproduces the classic NROM PRG decode; the CHR chip is enabled by `!nPA13` (on NROM boards the CHR `/CE` is tied to PPU `A13`, low for `$0000-$1FFF`; `nPA13 = !A13`), exactly as in the current `NROM` implementation but expressed as data.
- `kind: rom` components are instantiated as `BaseBoardLib::RomChip` (JEDEC-style), `kind: ram` as `BaseBoardLib::SRAM`. `PcbFactory` loads the PRG/CHR images into the ROM chips; the wiring attaches the chips' pins.
- `"mirroring": { "mode": "scroll" }` declares that the board has a **scroll solder jumper** (see §5.3 and §7.4): the concrete scroll value (H or V) is a parameter of the *cartridge instance*, not of the board. The loader derives it from the `.nes` header (iNES Flags6 bit 0).

### 5.2 Example: MMC1-based board (SGROM, mapper 1)

```json
{
  "schemaVersion": 1,
  "board": {
    "type": "HVC-SGROM",
    "pcb": "HVC-SGROM-03",
    "mapper": 1,
    "system": ["Famicom"],
    "components": {
      "prg": { "kind": "rom", "bus": "cpu", "size": 262144 },
      "chr": { "kind": "rom", "bus": "ppu", "size": 8192 },
      "mmc1": { "kind": "chip", "chip": "MMC1" }
    },
    "circuit": {
      "mirroring": { "mode": "mapper", "net": "mmc1.VRAM_A10" },
      "cpu": {
        "prg": {
          "chip": "prg",
          "n_cs": "mmc1.PRG_nCE",
          "addr": "mmc1.PRG_A17..PRG_A14 | cpu_addr[13:0]"
        }
      },
      "ppu": {
        "chr": {
          "chip": "chr",
          "n_cs": "!nPA13",
          "addr": "mmc1.CHR_A16..CHR_A12 | ppu_addr[11:0]"
        }
      },
      "nets": [
        { "name": "mmc1.M2",       "from": "M2" },
        { "name": "mmc1.nROMSEL",  "from": "nROMSEL" },
        { "name": "mmc1.CPU_RnW",  "from": "RnW" },
        { "name": "mmc1.CPU_A13",  "from": "cpu_addr[13]" },
        { "name": "mmc1.CPU_A14",  "from": "cpu_addr[14]" },
        { "name": "mmc1.CPU_D0",   "from": "cpu_data[0]" },
        { "name": "mmc1.CPU_D7",   "from": "cpu_data[7]" },
        { "name": "mmc1.PPU_A10",  "from": "ppu_addr[10]" },
        { "name": "mmc1.PPU_A11",  "from": "ppu_addr[11]" },
        { "name": "mmc1.PPU_A12",  "from": "ppu_addr[12]" }
      ]
    }
  }
}
```

This is the exact hardware structure the old `MMC1_Based` implemented in C++ — now it is *data*: a chip instance (`MMC1` from `Chips`) wired to the CPU/PPU buses, with the resulting address lines feeding the ROM chips.

### 5.3 Example: UNROM with its glue logic (74LS161 + 74LS32)

Issue #525: the board description is as close to the real PCB as possible — the bank register is a **74LS161** chip and the PRG address multiplexer is a **74LS32** quad OR gate (exactly the wiring of the old `Mappers::UNROM`):

```json
{
  "schemaVersion": 1,
  "board": {
    "type": "UNROM",
    "components": {
      "prg": { "kind": "rom", "bus": "cpu" },
      "chr": { "kind": "ram", "bus": "ppu", "size": 8192 },
      "ls161": { "kind": "chip", "chip": "LS161" },
      "ls32": { "kind": "chip", "chip": "LS32" }
    },
    "circuit": {
      "mirroring": { "mode": "scroll" },
      "cpu": {
        "prg": {
          "chip": "prg",
          "n_cs": "nROMSEL",
          "addr": "ls32.Y3 | ls32.Y0 | ls32.Y1 | cpu_addr[13:0]"
        }
      },
      "ppu": {
        "chr": { "chip": "chr", "n_cs": "!nPA13", "n_oe": "nRD", "n_we": "nWR", "addr": "ppu_addr[12:0]" }
      },
      "nets": [
        { "name": "ls161.CLK",  "from": "nROMSEL" },
        { "name": "ls161.nRST", "from": "vdd" },
        { "name": "ls161.nLD",  "from": "RnW" },
        { "name": "ls161.EN_T", "from": "gnd" },
        { "name": "ls161.EN_P", "from": "gnd" },
        { "name": "ls161.P0",   "from": "cpu_data[0]" },
        { "name": "ls161.P1",   "from": "cpu_data[1]" },
        { "name": "ls161.P2",   "from": "cpu_data[2]" },
        { "name": "ls161.P3",   "from": "gnd" },
        { "name": "ls32.A0",    "from": "ls161.Q1" },
        { "name": "ls32.B0",    "from": "cpu_addr[14]" },
        { "name": "ls32.A1",    "from": "ls161.Q0" },
        { "name": "ls32.B1",    "from": "cpu_addr[14]" },
        { "name": "ls32.A2",    "from": "gnd" },
        { "name": "ls32.B2",    "from": "gnd" },
        { "name": "ls32.A3",    "from": "cpu_addr[14]" },
        { "name": "ls32.B3",    "from": "ls161.Q2" }
      ]
    }
  }
}
```

Notes:
- The `LS161` is clocked by `/ROMSEL` with `/LD = R/W`: a CPU write to `$8000-$FFFF` loads the data bus into the bank register at the `/ROMSEL` rising edge — the same moment the current latch-based model captured the bank.
- The `LS32` ORs the bank bits with CPU `A14` (the classic UxROM trick): in the `$8000` window the bank bits reach the PRG address unchanged, and in the `$C000` window the OR with `A14 = 1` saturates the bank bits to the **last** 16 KiB bank.
- The generic `latch` component kind is still supported as a simplification for user-authored (JSONES) boards, but the built-in UxROM/AxROM boards now use the real chips.

### 5.4 The `circuit` section (CartPcb extension)

Because nescartdb only describes the component inventory, CartPcb adds a `circuit` section. Its exact expression language is finalized in the implementation issue; the concepts are:

- **Bus attachments** (`cpu` / `ppu`): for each memory component (a `RomChip` or `SRAM`), the chip-select (`n_cs`), output-enable (`n_oe`), write-enable (`n_we`) and address sources. Address sources can combine bus bits and chip outputs (`mmc1.PRG_A17..PRG_A14 | cpu_addr[13:0]`), which is how bank switching is expressed. Concatenation is MSB-first: the left term occupies the highest address bits, so MMC1's `PRG_A17..PRG_A14` (descending) maps `PRG_A14` to address bit 14.
- **Mirroring (Scroll)**: there is no "Mirroring" attribute of a board (issue #525). A board either
  - has a **scroll solder jumper** — `"mirroring": { "mode": "scroll" }` — a physical jumper that selects H Scroll (`VRAM_A10 = PA10`) or V Scroll (`VRAM_A10 = PA11`). The scroll value is a *cartridge instance* parameter, extracted from the `.nes` header (iNES Flags6 bit 0, inverted) or the nescartdb pads; or
  - drives `VRAM_A10` from a **mapper chip** output — `"mirroring": { "mode": "mapper", "net": "mmc1.VRAM_A10" }` — and needs no Scroll parameter; or
  - leaves mirroring unspecified (boards without VRAM wiring).
  The legacy `"mode": "hardwired"` (with `h`/`v` fields) is accepted as a compatibility alias for `"scroll"`; its `h`/`v` become the factory default.
- **Nets**: named connections between chip pins and bus signals, written as a flat list of `name` / `from` pairs. Any net can be used as `n_cs` / `n_oe` / `n_we` / address source / mirroring source, so arbitrary discrete-logic wiring (e.g. LS139-based decoders, UNROM's `nROMSEL`/`A14` decode) is expressible without new C++.

Memory images are loaded into the ROM chips; the bus attachments describe how each ROM chip's pins (JEDEC: `A0..An`, `/CE`, `/OE`, `D0..D7`) connect to the bus and to chip outputs.

The netlist model is deliberately simple (nets carry `TriState`; posedge/negedge timing lives inside chip classes). This keeps board descriptions declarative while chip timing stays in the chip simulators.

## 6. Runtime architecture

### 6.1 Modules

```
CartPcb/                     (new top-level component, C++/native)
  CartPcb.h                  public header
  Pcb.h / Pcb.cpp            the simulated board
  PcbFactory.h/.cpp          JSON -> Pcb
  PcbLoader.h/.cpp           locate & parse PCB JSON (built-in Nescartdb + user dir)
  NesCartDb.h/.cpp           CRC32 -> board type lookup (nescartdb index)
  InesTranslator.h/.cpp      iNES-header fallback for "wild" dumps (issue #514)
  CartImage.h                PRG/CHR dumps + battery RAM
  Readme.md                  this specification
```

Dependencies: `Common/BaseLogicLib` (TriState), `Common/JsonLib` (JSON parsing), `Common/BaseBoardLib` (`RomChip` — JEDEC-style ROM, `SRAM` — static RAM, `LS161`/`LS32` glue logic), `Nescartdb/` (data), and `Chips/` for mapper-chip classes.

### 6.2 Key classes

- **`Pcb`** — owns the component instances and the net map; `sim()` drives all components from the edge-connector inputs and produces the outputs. Follows the same "stick signals in, something happens" style as the rest of the emulator.
- **`PcbFactory`** — builds a `Pcb` from a board JSON + a `CartImage` (allocates chips, wires nets, validates the image sizes against the components).
- **`PcbLoader`** — resolves a board type to a JSON document: built-in `Nescartdb` data first, then user JSONs (which may add or override).
- **`NesCartDb`** — loads the converted nescartdb JSON once and answers "given PRG/CHR images, which board type(s) match?" by the CRC32 of the PRG and CHR dumps. This is the component that translates PRG/CHR CRCs into a PCB type.
- **`InesTranslator`** — the "wild dump" fallback (issue #514): when the PRG/CHR CRCs are not in nescartdb, translates the iNES header (mapper number + PRG/CHR sizes) into a board type so homebrew runs through the same board path.
- **`RomChip`** — in `Common/BaseBoardLib`, not CartPcb. A JEDEC-style ROM chip: pins `A0..An`, `D0..D7`, `/CE`, `/OE`; a ROM ignores `/WE` writes. `PcbFactory` loads the PRG/CHR images into the `RomChip` instances.
- **RAM** — `BaseBoardLib::SRAM`, reworked so it is clearly a simulator of typical 80s–90s static RAM chips (standard pin functions, `n_CS`/`n_WE`/`n_OE` semantics, power-up state). Used for WRAM/VRAM components, including battery-backed ones.
- **`CartImage`** — the explicit PRG/CHR dumps (plus battery RAM content), loaded into the ROM chips by `PcbFactory`. Replaces the raw `uint8_t* nesImage` + iNES-header parsing inside cartridge classes.

### 6.3 Chip interface for injected chips

Mapper chips (e.g. `MMC1`) are plain chip classes, following the pattern of today's `Mappers::MMC1`:

```cpp
class MMC1 {
    void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]);
};
```

The board JSON's `nets` list defines which pins exist and what drives them; the C++ chip class must expose a matching pin map (name ↔ input/output index). The pin-name contract between JSON and chip classes is part of the implementation issue.

### 6.4 Loading & lookup flow

```
CartImage (PRG/CHR dumps loaded into ROM chips)
   |
   v
NesCartDb.FindBoard(crc32(PRG), crc32(CHR))  -->  board type (e.g. "HVC-SGROM")
   |
   v
PcbLoader.Load(board type)  -->  board JSON  (built-in Nescartdb, or user override)
   |
   v
PcbFactory.Create(board JSON, CartImage)  -->  Pcb
   |
   v
CartPcbCartridge (CartPcb cartridge port)  -->  Board::InsertCartridge
```

If no nescartdb match is found and no board type is forced, the **iNES fallback** (§8) translates the dump from its header — the iNES mapper number and the PRG/CHR sizes — so homebrew and other "wild" dumps run through the same board path (issue #514).

### 6.5 Custom PCB JSONs (JSONES subset)

Any single-cartridge board JSON (§5) can be provided by the user:

- The format is exactly the CartPcb board format (a "JSONES" file is simply a board JSON with `game`/`cartridge` meta-information added, mirroring the nescartdb `game` → `cartridge` → `board` nesting).
- Location: a user data directory (decided in the implementation issue; candidates: next to the executable, `%APPDATA%`, or a `CustomBoards/` folder passed on the command line).
- Loading order: built-in `Nescartdb` data first; user JSONs are merged on top (a user file with the same `board.type` overrides the built-in; a user file with a new type is added).
- Use case: describing unlicensed/undocumented PCBs (krzysiobal's collection, Famiclones, etc.) that cannot be expressed via iNES mapper numbers.
- Identification bypass: for a board that is not in the nescartdb index (no CRC match possible), the host application can force a board type (`CartPcb::SetForcedBoardType`, exported as `SetForcedBoardType` in the Core API); the board definition is then loaded directly from the built-in/user board JSONs.

## 7. Simulation model

### 7.1 Edge-connector contract

Defines the cartridge port — the signal contract of the former `Mappers::AbstractCartridge`, which now lives in CartPcb (`CartPcb::Cartridge`):

- Inputs (`CartInput`): `M2`, `nROMSEL`, `RnW`, `nRD`, `nWR`, `nPA13`, and NES-only `SYSTEM_CLK`, `CIC_CLK`, `CIC_TO_CART`.
- Outputs (`CartOutput`): `VRAM_A10`, `VRAM_nCS`, `nIRQ`, and NES-only `CIC_RST`, `CIC_TO_MB`.
- Famicom audio out (`CartAudioOutSignal`) and NES expansion port (`exp`).

A `CartPcbCartridge` owns a `Pcb` and forwards the edge-connector signals into it. Motherboards (`Breaknes/BreaksCore/NESBoard.cpp`, `FamicomBoard.cpp`) use `CartPcb::Cartridge*`.

### 7.2 Net sources

Predefined sources available to board JSONs:

| Source | Meaning |
|--------|---------|
| `cpu_addr[n]`, `cpu_addr[a:b]` | CPU address bus bits |
| `ppu_addr[n]`, `ppu_addr[a:b]` | PPU address bus bits |
| `cpu_data[n]` | CPU data bus bits |
| `M2`, `nROMSEL`, `RnW`, `nRD`, `nWR`, `nPA13` | edge-connector control signals |
| `<chip>.<PIN>` | any chip output pin (e.g. `mmc1.PRG_A14`, `mmc1.VRAM_A10`) |
| `gnd`, `vdd` | constants |

Derived expressions (`|` for concatenation, `&`/`|`/`!` boolean operators) form address lines and chip-selects.

### 7.3 Memory chip semantics

`RomChip` (JEDEC-style) and `SRAM` follow the classic memory-chip protocol (`/CE` — chip enable, `/OE` — output enable, `/WE` — write enable, `A0..An`, `D0..D7`):

- Read: `/CE == 0 && /OE == 0` → drive data bus (`dz` handling identical to SRAM).
- Write (RAM only): `/CE == 0 && /WE == 0` → latch data. A ROM ignores `/WE`; CHR-RAM boards use `SRAM` on the PPU bus instead of a ROM.
- Address width is derived from the component `size` (e.g. 32 KiB → `A0..A14`).
- Debug: `Dbg_ReadByte` / `Dbg_WriteByte` / `Dbg_GetSize` per chip, exposed through the board's debug interface.

### 7.4 Mirroring → Scroll

There is no "Mirroring" definition for a declarative PCB description (issue #525). The term "Mirroring" belongs to the iNES header; on the board there is only a solder jumper and the scrolling arrangement it selects (nesdev "arrangement"):

| PCB jumper (Scroll) | Wiring | iNES header (Flags6 bit 0) |
|---------------------|--------|----------------------------|
| **H Scroll** | `VRAM_A10 = PA10` | `1` ("vertical mirroring") |
| **V Scroll** | `VRAM_A10 = PA11` | `0` ("horizontal mirroring") |

The two terms are mutually exclusive: H Scroll (PCB) == vertical mirroring (iNES).

- `"mode": "scroll"`: the board has a scroll jumper. The concrete scroll value is a **cartridge instance** parameter, applied by the loader from the `.nes` header (`Pcb::ApplyScrollFromHeader`, bit 0 = 1 → H Scroll) or from the nescartdb pads (`ApplyPadScroll`). Boards that drive `VRAM_A10` from a mapper chip output have **no** Scroll parameter.
- `"mode": "mapper"`: `VRAM_A10` is driven by a net (e.g. `mmc1.VRAM_A10`), exactly like MMC1 boards.
- Boards without VRAM wiring (e.g. some discrete boards) can leave mirroring unspecified.

The PPU's internal 2 KiB VRAM and its `VRAM_A10` line are simply called **VRAM**.

### 7.5 Relationship to existing components

| Component | Role |
|-----------|------|
| `CartPcb::Cartridge` | The cartridge-port contract (inputs/outputs/debug); the former `Mappers::AbstractCartridge`. `CartPcbCartridge` implements it. |
| `Mappers` component | **Retired** (issue #509). The whole folder was removed; this document replaces the old `Mappers/Readme.md`; build files (`CMakeLists.txt`, VS projects) updated. |
| `Chips/MMC1` | The MMC1 chip class (moved from Mappers), with unit tests. |
| `Chips`, `BaseBoardLib` | Provide chip classes (`MMC1`, `RomChip`, `SRAM`, `LS161`, `LS32`) consumed by CartPcb. |
| `Common/JsonLib` | JSON parsing for board JSONs and the nescartdb JSON. |
| `Nescartdb/` | The converted database (identification data). |

## 8. Identification (NesCartDb)

- Input: PRG image, CHR image. Identification is by the **CRC32 of the PRG and CHR dumps**; SHA1 is not used.
- Lookup: build a map from PRG CRC + CHR CRC → cartridge → `board.type` once at load; find all cartridges whose PRG and CHR CRCs match the images.
- Ambiguity: some dumps match multiple cartridges/boards (same ROMs on different PCBs). `NesCartDb` returns the full candidate list; the caller picks the first match or the one whose `system` fits the motherboard. This ambiguity is a *feature* compared to iNES: the information is surfaced instead of being silently lost.
- If no match is found, the **iNES fallback** (`InesTranslator`, issue #514) translates the dump from its header: the iNES mapper number (Flags6/7) and the PRG/CHR sizes select a board type (mapper 0 → NROM-128/256 by PRG size, 1 → MMC1 (SGROM for CHR-ROM / SHROM for CHR-RAM), 2 → UNROM, 3 → CNROM, 7 → AOROM). The translated dump runs through the same `PcbLoader` → `PcbFactory` path, with a warning logged. Unknown mappers (e.g. MMC3 until the chip is implemented) are not loaded. A board not in the database can still be forced via `SetForcedBoardType` (§6.5).

## 9. Board coverage plan

Boards are added incrementally, each as: board JSON(s) + unit test + (while the old implementation existed) parity test:

1. **NROM** (`NES-NROM-128/256`, `HVC-NROM-*`, `IREM-NROM-*`, ...) — no mapper chip; PRG + CHR + a scroll jumper. Replaces the old `NROM`.
2. **UxROM** (`NES-UNROM`, `NES-UOROM`, ...) — discrete logic PRG bank switch (A14), 8 KiB CHR-RAM. Described with the real glue logic: a **74LS161** bank register and a **74LS32** address multiplexer (issue #525). Replaces the old `UNROM`.
3. **AxROM** (`NES-AOROM`, `NES-ANROM`, ...) — 1-screen mirroring via the **74LS161** bank register (Q3 → `VRAM_A10`). Replaces the old `AOROM`.
4. **MMC1 family** (SGROM, SLROM, ...) — `MMC1` chip from `Chips` wired per §5.2. Replaces the old `MMC1_Based`.
5. **Fallback boards** (issue #514): `CNROM` (latch-based CHR bank switch) and `SHROM` (MMC1 + CHR-RAM) exist so the iNES fallback can translate mapper 3 and mapper-1-CHR-RAM dumps.
6. **Next candidates**: MMC3 family, discrete logic boards (BxROM, ...).

## 10. Migration checklist (Mappers → CartPcb)

Completed in issue #509:

1. The cartridge-port contract (ex-`Mappers::AbstractCartridge`) moved into CartPcb (`CartPcb::Cartridge`).
2. `CartImage` replaced raw `uint8_t* nesImage`; PRG and CHR are explicit, named, sized dumps loaded into `RomChip` instances.
3. `RomChip` (JEDEC-style, in `BaseBoardLib`) and the reworked `SRAM` replaced raw byte arrays.
4. `NesCartDb` (CRC32 only) + `PcbLoader` + `PcbFactory` replaced the iNES-mapper `switch` in the old `CartridgeFactory`.
5. `MMC1` moved to `Chips` (with unit tests); the legacy unmasked PRG read was fixed.
6. NROM/UNROM/AOROM/SGROM/SLROM were re-expressed as board JSONs and passed A/B parity tests against the old implementations.
7. The entire `Mappers` component was removed (including `AbstractCartridge` and `CartridgeFactory`); build files updated.
8. All consumers locate the Nescartdb JSON and custom board dirs per `Nescartdb/Readme.md`.

## 11. Debug & testing

- **Memory map**: each board registers its memory regions (PRG, CHR, WRAM) as `MemDescriptor`s with per-chip `Dbg_ReadByte`/`Dbg_WriteByte`, preserving the debugger memory view (and fixing the PRG support that was missing).
- **Debug info**: boards expose debug entries per net/chip (category = board type), e.g. current PRG bank, `nROMSEL` state, scroll mode.
- **Unit tests** (`UnitTest/`): JSON parse → board instantiation; chip `/CE`/`/OE`/`/WE` behavior; mirroring; bank-switch address math for each supported board; MMC1 chip tests; JSONES custom board loading.
- **`Dbg_ReadPRGByte`** stays on the CartPcb cartridge port (used by the debugger and Nintendulator log disassembler); it derives from the PRG chip mapping.

## 12. Non-goals / future work

- Pin-level timing and propagation delays are not modeled in v1; nets are combinatorial, timing lives in chip classes.
- Complex discrete logic boards are expressible via nets but will be added as the netlist language matures.
- Famicom sound expansion (VRC6, FDS, ...) is a later topic; the connector already carries the audio line.
