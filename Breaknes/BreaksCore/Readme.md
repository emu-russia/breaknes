# BreaksCore

The native part of the emulator.

## Abstract Board

The abstract motherboard is used as a base class for all other NES/Famicom/Dendy variations.

The Board Factory is used to instantiate the motherboard:
- The parent application tells the factory what board to create (by name).
- This creates a class for the corresponding motherboard (NES.cpp, Famicom.cpp).

So far it's a bit abstract, but later we will make the links between classes.

## Boards

List of supported boards.

### Bogus Board

BogusBoard is a default motherboard if BreaksCore did not recognize the board type when it created the instances or other erroneous parameters.

![BogusBoard](/UserManual/imgstore/BogusBoard.jpg)

This system contains 64 KBytes of RAM, a 6502 processor core and nothing else.

In other words, it's a minimal computing system to check if anything is ticking in the emulator.

### PPUPlayer Board

A special debug board for the PPUPlayer.

![PPUPlayer_All](/UserManual/imgstore/PPUPlayer_All.png)

The board contains the PPU, adjacent logic (Address Latch, VRAM) and partial support for the cartridge connector (only the PPU/CHR pins are used).

### NSFPlayer Board

A special debug board for the NSFPlayer.

![NSF_Board](/BreaksAPU/NSFPlayer/NSF_Board.png)

Contains only the APU and a special type of memory, inside which is the NSF bank switcher (processing registers $5FF8-$5FFF).

Additional sound chips are not supported yet, since the primary goal is to check the functionality of the coupling of the APU and the 6502 embedded core.

### APUPlayer Board

Another debug board for "playing" the APU register dump.

![APUPlayer](/UserManual/imgstore/APUPlayer.jpg)

It is roughly the same as PPUPlayer, but simpler in that the APU does not require a cartridge connector and additional bindings associated with it.

The DPCM samples are loaded into the extended up to 64 Kbytes memory (WRAM).

### Famicom

The plan is to simulate the Generic Famicom board as follows:

![fami_logisim](/UserManual/imgstore/fami_logisim.jpg)

### NES

TBD.

### Dendy/Subor/Phantom/Whatever

TBD.
