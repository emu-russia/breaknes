
![logo](/Wiki/imgstore/157481692-2ecd4e71-2599-4050-9ce0-815c0336ad27.png)

NES/Famicom/Dendy emulator at the gate level.

Breaknes is the emulator part of the [breaks](https://github.com/emu-russia/breaks) chip-reversing project. The chips are not emulated "by behavior" — the logic circuits reconstructed from the real dies are duplicated in C++, so the emulated 6502 behaves the same way the real one does: same gates, same flip-flops, same buses, same quirks.

## Repository structure

| Folder | What's inside |
|---|---|
| `Breaknes/` | The emulator itself: managed GUI (`Breaknes/Breaknes`), SDL2 port (`Breaknes/BreaknesSDL`), and the native core `BreaksCore` shared by both. |
| `Chips/` | Gate-level simulators of the chips: `M6502Core` (6502), `APUSim` (2A03/2A07 CPU+audio), `PPUSim` (2C02 PPU), `MMC1`. |
| `CartPcb/` | Functional simulation of cartridge PCBs. A board is data — a JSON document with components and wiring, not C++ code. |
| `Common/` | Shared code: `BaseLogicLib` (logic primitives), `BaseBoardLib` (motherboard components: LS139, LS373, SRAM, RomChip...), `JsonLib`, `SharpTools` (C# debugger controls). |
| `IO/` | I/O devices: NES/Famicom/Dendy controllers, virtual controllers, CD4021, UM6582. |
| `Nescartdb/` | The converted nescartdb database (JSON) — identifies a cartridge by the CRC32 of its PRG/CHR dumps. |
| `Tools/` | Utilities, see below. |
| `UnitTest/` | Unit tests for the chips and cores. |
| `Wiki/` | Design documentation. |

## Components

- **BreaksCore** — the native (C++) core: motherboards (NES, Famicom), chip wiring, and a debug hub for monitoring signals and internal state.
- **M6502Core** — MOS 6502 processor core at the gate level.
- **APUSim** — the whole CPU chip (2A03/2A07): 6502 core + APU sound.
- **PPUSim** — the PPU (2C02) at the gate level.
- **MMC1** — the MMC1 mapper chip.
- **CartPcb** — cartridge PCB simulation: board descriptions are JSON, chips are wired with nets, cartridges are identified by nescartdb CRCs.

## Tools

| Tool | What it does |
|---|---|
| `Breakasm` | The simplest possible 6502 assembler (C++ + Python port). |
| `BreaksDebug` | GUI to inspect the 6502 core: signals, registers, memory, disassembly. |
| `PPUPlayer` | Replays PPU register dumps recorded in the emulator. |
| `APUPlayer` | Replays APU register dumps recorded in the emulator. |
| `PpuPumpkin` / `ApuPumpkin` | Speed profiling demos for PPUSim / APUSim. |
| `SlooowPlaaayer` | Plays audio captured from slow sources. |
| `SignalPlotDemo`, `FurryPlotDemo`, `InfernoColormap` | Demos of the custom WinForms controls. |
| `DumpRegdump` / `NescartdbConvert` | Python scripts: regdump dump / nescartdb XML→JSON conversion. |

## Progress

![progress](/Wiki/imgstore/progress.png)

## Build for Windows

Use Windows and VS2026. Open Breaknes.sln and click the Build button with your left heel. Dotnet 6.0 **Desktop** Runtime is also required.

For those who like minimalism - see Breaknes build using SDL2 (`Breaknes/BreaknesSDL`).

## Build for Linux

In general, the build process is typical for Linux. First you get all the sources from Git. Then you call CMake/make

```
# Get source
# Choose a suitable folder to store a clone of the repository, cd there and then
git clone https://github.com/emu-russia/breaknes.git
cd breaknes

# Preliminary squats
mkdir build
cd build
cmake ..
make

# Find the executable file in the depths of the build folder
./breaknes bomber.nes
```

If something doesn't work, you do it. You have red eyes for a reason. :penguin:
