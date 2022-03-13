# Breaknes Debug Capabilities

Since Breaknes is a gate-level emulator, the main focus of debugging is monitoring various signals, buses and other internal states of the emulated system.

Debug entities are divided into the following categories:
- DebugInfo: getting internal state
- MemLayout: getting memory dumps

![debug](/UserManual/imgstore/debug.jpg)

## DebugInfo

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
- GetDebugInfo: Get entries of the specified type

A Managed application must first know the number of records and allocate an array to retrieve them.

## MemLayout

The different pieces of memory within the emulated system are described by the `MemDesciptor` structure.

Methods:

- GetMemLayout: Get the number of memory descriptors that are registered in the core
- GetMemDescriptor: Get information about a memory block
- DumpMem: Get the whole memory block. We are emulating NES here, so dump sizes will be small and it won't make sense to dump in parts.
