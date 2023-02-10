# Debug Hub

Breaknes debug infrastructure.

:warning: The current implementation is based on string associative lists and cannot be considered superfast.

Since Breaknes is a gate-level emulator, the main focus of debugging is monitoring various signals, buses and other internal states of the emulated system.

Debug entities are divided into the following categories:
- DebugInfo: get/set internal state
- MemLayout: read/write memory dumps

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
- GetDebugInfo: Get all entries of the specified type. Managed application must first know the number of records and allocate an array to retrieve them.
- GetDebugInfoByName: Get one DebugInfo record with the specified name
- SetDebugInfoByName: Set one DebugInfo record with the specified name

## MemLayout

The different pieces of memory within the emulated system are described by the `MemDesrciptor` structure.

Methods:
- GetMemLayout: Get the number of memory descriptors that are registered in the DebugHub
- GetMemDescriptor: Get information about a memory block
- DumpMem: Get the whole memory block. We are emulating NES here, so dump sizes will be small and it won't make sense to dump in parts.
- WriteMem: Set the entire memory block
