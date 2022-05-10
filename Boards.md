# Boards

List of supported boards.

## Bogus Board

BogusBoard is a default motherboard if BreaksCore did not recognize the board type when it created the instances or other erroneous parameters.

![BogusBoard](/UserManual/imgstore/BogusBoard.jpg)

This system contains 64 KBytes of RAM, a 6502 processor core and nothing else.

In other words, it's a minimal computing system to check if anything is ticking in the emulator.

## PPUPlayer Board

A special debug board for the PPUPlayer.

![PPUPlayer_All](/UserManual/imgstore/PPUPlayer_All.png)

The board contains the PPU, adjacent logic (Address Latch, VRAM) and partial support for the cartridge connector (only the PPU/CHR pins are used).

## Famicom

The plan is to simulate the Generic Famicom board as follows:

![fami_logisim](/UserManual/imgstore/fami_logisim.jpg)

## NES

TBD.

## Dendy/Subor/Phantom/Whatever

TBD.
