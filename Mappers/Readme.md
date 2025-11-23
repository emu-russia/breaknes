# Mappers

This component combines everything related to the cartridge port and mappers. Please treat the term "Mappers" in this context as a synonym for cartridge.

The implementation of mappers is done via generic simulation of common mapper circuits in C++ using BaseLogicLib.

## List of problem areas

List of what is currently problematic in this component:
- Support for working with memory dumps (PRG/CHR) is not very clear. The CHR region has been called "CHR-ROM" since the early stages of development. PRG is not supported at all
- Emulation of .nes mappers is very chaotic, in the sense of translating the mapper number into cartridge board components. This is further complicated by the fact that the same mapper (eg MMC1) can be implemented in different cartridges with a lot of variations and how to form and connect the components of the cartridge for its simulation is not very clear
- There is no support for ROM chips. Now everything is done via byte array. It is necessary to implement it by analogy with `Generic SRAM` chip in BaseBoardLib.
- MMC1 emulation requires debugging and verification. Most likely something is wrong with the divider.

## Abstract Cartridge

The emulation of mappers is based on the same principle as for the other parts of the emulator: take a device or connector, stick signals in it and something happens.

In the case of the сartridge, an "abstract port" is used which summarizes all the interfaces of the real NES/Famicom cartridge connector (sound, Expansion Port), but in a particular instance only the necessary ones are used (for example if you create a NES motherboard - there will be no sound from the cartridge).

CartridgeFactory creates a cartridge instance for the main part of the emulator based on meta-information attributes (NES header, JSONES meta-information).