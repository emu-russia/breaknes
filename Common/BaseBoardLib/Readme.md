# BaseBoardLib

This library contains components that are found on the simulated motherboards.

These are not necessarily actual chips, they can be some special debugging components, e.g. Fake6502.

## 40H368 - IO Binding

![40H368](/Wiki/imgstore/40H368.jpg)

An array of `notif0` elements (in Verilog terms) used to bind IO ports. The connection of external ports and CPU differs between Famicom/NES.

## LS139 - Famicom/NES "North Bridge"

![LS139](/Wiki/imgstore/LS139.jpg)

This paired decoder is used to select the address space of the PPU register interface, internal S-RAM ("Work RAM"), or external ROM.

The distinguishing feature is that the output from one half is fed to the same chip :-)

## LS373 - PPU VRAM Address Latch

![LS373](/Wiki/imgstore/LS373.jpg)

Used for PPU Address/Data bus multiplexing. "Memorizes" the lower 8 bits of the PPU address bus when PPU ALE=1.

## SRAM - Typical S-RAM from 90's

![SRAM](/Wiki/imgstore/SRAM.jpg)

Well, that's easy.
