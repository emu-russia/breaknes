# BreaksCore

The native part of the emulator.

## Abstract Board

The abstract motherboard is used as a base class for all other NES/Famicom/Dendy variations.

The Board Factory is used to instantiate the motherboard:
- The parent application tells the factory what board to create (by name).
- This creates a class for the corresponding motherboard (NES.cpp, Famicom.cpp).

So far it's a bit abstract, but later we will make the links between classes.
