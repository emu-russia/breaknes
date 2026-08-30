# BreaknesSDL

For those who like minimalism - Breaknes build using SDL2.

The implementation does not differ from Managed application: the same native part of BreaksCore from DLL is used. Chip revisions and motherboard cannot be selected yet (NESBoard+RP2A03G+RP2C02G is hardcoded in main.cpp).

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
