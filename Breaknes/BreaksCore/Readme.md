# BreaksCore

The native part of the emulator.

## Abstract Board

The abstract motherboard is used as a base class for all other NES/Famicom/Dendy variations.

The Board Factory is used to instantiate the motherboard:
- The parent application tells the factory what board to create (by name).
- This creates a class for the corresponding motherboard (NESBoard.cpp, FamicomBoard.cpp).

So far it's a bit abstract, but later we will make the links between classes.

## Boards

List of supported boards.

### Famicom

The plan is to simulate the Generic Famicom board as follows:

![fami_logisim](/Wiki/imgstore/fami_logisim.jpg)

### NES

The plan is to simulate the Generic NES board as follows:

![nes](/Wiki/imgstore/nes.png)

### Dendy/Subor/Phantom/Whatever

TBD.

### Bogus Board

BogusBoard is a default motherboard if BreaksCore did not recognize the board type when it created the instances or other erroneous parameters.

![BogusBoard](/Wiki/imgstore/BogusBoard.jpg)

This system contains 64 KBytes of RAM, a 6502 processor core and nothing else.

In other words, it's a minimal computing system to check if anything is ticking in the emulator.

### PPUPlayer Board

A special debug board for the PPUPlayer.

![PPUPlayer_All](/Wiki/imgstore/PPUPlayer_All.png)

The board contains the PPU, adjacent logic (Address Latch, VRAM) and partial support for the cartridge connector (only the PPU/CHR pins are used).

### APUPlayer Board

Another debug board for "playing" the APU register dump.

![APUPlayer](/Wiki/imgstore/APUPlayer.jpg)

It is roughly the same as PPUPlayer, but simpler in that the APU does not require a cartridge connector and additional bindings associated with it.

The DPCM samples are loaded into the extended up to 64 Kbytes memory (WRAM).

## Debug Hub

Breaknes debug infrastructure.

:warning: The current implementation is based on string associative lists and cannot be considered superfast.

Since Breaknes is a gate-level emulator, the main focus of debugging is monitoring various signals, buses and other internal states of the emulated system.

Debug entities are divided into the following categories:
- DebugInfo: get/set internal state
- MemLayout: read/write memory dumps

![debug](/Wiki/imgstore/debug.jpg)

### DebugInfo

From the GUI side, `DebugInfo` is displayed as PropertyGrid. The PropertyGrid is filled in dynamically, based on debugging information entries returned by the native Breaknes core.

Debug Information Types (DebugInfoType):
- DebugInfoType_Core: Get the internal state of the M6502Core
- DebugInfoType_CoreRegs: Get the state of the 6502 registers
- DebugInfoType_APU: Get the internal state of the APU components
- DebugInfoType_APURegs: Get the state of APU registers
- DebugInfoType_PPU: Get the internal state of the PPU components
- DebugInfoType_PPURegs: Get the state of the PPU registers
- DebugInfoType_Board: Get the internal state of the motherboard components
- DebugInfoType_Cart: Get the internal state of the cartridge and mapper components

Methods:
- GetDebugInfoEntryCount: Get the number of debug entries of the specified type
- GetDebugInfo: Get all entries of the specified type. Managed application must first know the number of records and allocate an array to retrieve them.
- GetDebugInfoByName: Get one DebugInfo record with the specified name
- SetDebugInfoByName: Set one DebugInfo record with the specified name

### MemLayout

The different pieces of memory within the emulated system are described by the `MemDesrciptor` structure.

Methods:
- GetMemLayout: Get the number of memory descriptors that are registered in the DebugHub
- GetMemDescriptor: Get information about a memory block
- DumpMem: Get the whole memory block. We are emulating NES here, so dump sizes will be small and it won't make sense to dump in parts.
- WriteMem: Set the entire memory block
