# BreaknesSDL

For those who like minimalism - Breaknes build using SDL2.

The implementation does not differ from Managed application: the same native part of BreaksCore is used. The motherboard and chip revisions are taken from the `BoardDescription.json` (next to the executable, shared with the managed application): the SDL build looks up the board by name, the config is required (there is no built-in fallback).

## Selecting the board

By default the `NES (NES-CPU-07, 1987)` entry of the `BoardDescription.json` is used. Any other board from the same file can be selected on the command line:

```
breaknes --board "NES-101 (NESN-CPU-JIO-01, 1994-1995)" game.nes
```

## Two TV Sets (issue #515)

The SDL build renders the video signal of every connected TV Set in one window. Up to 2 TVs are supported; the physical arrangement is controlled by the `tv_layout` property of the board in `BoardDescription.json`:

- `"tv_layout": "horizontal"` - the TVs are placed side by side
- `"tv_layout": "vertical"` - the TVs are stacked one above the other

The binding of the PPUs to the TVs is described by the `tvs` array (`tvs[i]` = index of the PPU shown on the i-th TV). The default binding is `TV[i]` shows `PPU[i]`; for debugging, one PPU can be bound to both TVs at once (`"tvs": [0, 0]`).

## IO subsystem

The IO subsystem is integrated into the SDL build (issue #516). SDL2 Input is used as the source of input events:

- **Keyboard** - SDL key names (`Up`, `Down`, `Left`, `Right`, `A`, `S`, `Z`, `X`, `Return`, `Space`, ...)
- **Game controllers** (if available) - `Controller<slot>_ButtonA`, `Controller<slot>_DPadUp`, `Controller<slot>_ButtonStart`, ...; the analog sticks are mapped to digital events `Controller<slot>_AxisLeftX-`/`_AxisLeftX+`, etc.; the analog triggers to `Controller<slot>_TriggerLeft`/`_TriggerRight`.

The devices (Famicom/NES/Dendy controllers, virtual controllers), their bindings of IOState to input events and the Attach/Detach status to the motherboard ports are configured interactively:

```
breaknes --ioconfig [config_path]
```

The logic of the configurator is based on the IO settings dialog of the managed application (`Breaknes/FormIOConfig.cs`): the device pool (add/remove), the bindings (IOState <-> input event) and the Attach/Detach to ports.

The IO settings are stored separately from the managed application (`IOConfigSDL.json` by default, the managed application uses its own `IOConfig.json`), so the settings never overlap. At startup the emulator loads the settings from the same file and attaches the configured devices.

Note: virtual controllers have no on-screen dialogs in the SDL build; they are driven by the bound keyboard/game controller events like ordinary controllers.
