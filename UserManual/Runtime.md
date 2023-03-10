# Emulator Life Cycle

This section contains a description of the various milestones in the emulator's lifecycle.

During its operation the emulator experiences the following events:
- OnLoad
- SaveSettings
- LoadROM
- Step
- Exit

## OnLoad

The user starts the emulator.

After the application starts, the settings are loaded and a motherboard is created in BreaksCore, which is specified in the settings:
- CoreApi::CreateBoard

When created, the board itself adds all the necessary DebugInfo and MemLayout.

The simulation state is paused.

## SaveSettings

If the current board was changed when saving, the motherboard instance is recreated, just like when you run the application:
- CoreApi::DestroyBoard
- CoreApi::CreateBoard

## LoadROM

The user selects the .nes dump to run:
- CoreApi::EjectCartridge
- CoreApi::InsertCartridge

This does not recreate the board, but simulates pressing the `RESET` button on the console.

The simulation starts automatically.

## Step

CoreApi::Step is called in a loop until the user exits the application or selects another .nes image

## Exit

The user terminates the application.

If the simulation was running, it stops.

CoreApi::DestroyBoard is called.
