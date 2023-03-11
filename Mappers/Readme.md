# Mappers

The implementation of mappers is done (in progress) in two ways:
- Generic simulation of common mapper circuits in C++ using BaseLogicLib
- Ability to simulate unusual mappers using 6502 microcode

## Mapper microcode

The NES/Famicom is famous for its large number of mappers. To the licensed mappers, just over-10001 Chinese mappers were added, with minimal variations, but for each you have to enter your own "number" in the .NES format.

But what if you let an advanced user speak on their own in a "cast" of roms - which mapper should be used for them. And not just what kind of mapper, but to determine the behavior of the mapper yourself.

We have:
- A collection of CHR/PRG roms, possibly cobbled together as a .nes file, or possibly just a bunch of files lying around
- Schematics of the mapper / auxiliary chips that are on the cartridge
- 6502 Emulator
- 6502 firmware that simulates cartridge pin access and implements wild mapper logic

![mappers_ucode1](mappers_ucode1.png)

![mappers_ucode2](mappers_ucode2.png)

Of course, most of the mappers are easier to simulate in C++, but for universality, let's add the ability to write simulators of our own mappers.

How it will work:
- Next to .nes or just like that lies JSON with meta information (it seems like everyone voted to call this format `JSONES`)
- The path to the .asm file with microcode is written in JSON
- Our emulator has a primitive/advanced assembler that compiles the microcode into a 64KB BLOB
- Another M6502Core instance is being created, which stupidly works with microcode and nothing else
- When the main emulator simulates a microcode cartridge port, the values of the cartridge connector signals are entered into the 6502 microcode memory and control is transferred to the microcode entry point
- The microcode simulates something there, does bbrrrr and outputs the values of the output signals of the cartridge connector to memory
- The main part of the emulator picks up signals and rejoices

More details like UCode 6502 memory map etc. in the process of implementation. You also need to figure out how the microcode will gain access to the roms (obviously, they all need to be loaded into memory first, and from the 6502 side, somehow knock there). We'll come up with something.
