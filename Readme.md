# Breaknes

![logo](/Breaknes/Breaknes/157481692-2ecd4e71-2599-4050-9ce0-815c0336ad27.png)

NES/Famicom/Dendy emulator at the gate level.

The last significant milestone:

![mariwa](/UserManual/imgstore/mariwa.png)

## Build

Use Windows and VS2022. Dotnet 6.0 **Desktop** Runtime is also required.

Builds for Linux will be available as soon as everything settles down.

## What will and will not be in the emulator

The emulator is not ready yet, so you can immediately write what will not.

- Save States. No need, play without them. Use the virtual machine and the sandbox in which the emulator is running.
- AVI Record. Nah. Use capture software
- TAS. There are already plenty of emulators for TAS
- Handy debugger. Use Mesen. The emulator will have basic debugging mechanisms to check that something is moving there. Everything else we check with specialized tools (Breaks Debugger, PPU Player etc.) and unit-tests
- Frame Skip. What is this?
- Rewind. Use the virtual machine and the sandbox in which the emulator is running.
- Speed. For now, 1-2 FPS will be enough for the first time
- Audio Recording. Use capture software
- Game Genie. No need.

In short, it is clear that the emulator will be the most casual and roughly corresponds to the real console. What the console can do, the emulator can do. It's roughly like this.

Killer features.

- Accurate simulation of the main chips (actually the main point of the whole idea)
- Customizable boards. You can design your own board.
- @eugene-s-nesdev also asked to make it possible to mold a chimera from chips, since there is no fully proper Famiclone-chip, so the best thing would be to take pieces from different ones and mold your own

## Procrastination Chart

- Breaknes GUI (managed part): 30%
- BreaksCore (native part): 50%
- 6502 Core: 100%
- APU: 13/14
- PPU: 100%
- Boards: 1/5
- Mappers: 0.5 (Only NROM partially)

The values reflect mostly a subjective feeling about reaching a critical mass of code. Bugs and improvements can be made ad infinitum.
