# Breaknes User Manual

## Main Window

![main](/UserManual/imgstore/main.jpg)

- File->Load ROM Dump...: Select the .nes ROM image. The emulation will start right away.

- Settings...: Edit the settings

- Exit

## Settings

![settings](/UserManual/imgstore/settings.jpg)

- Save: Save and close

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
