# PPUSim

PPU simulator at the gate level.

## Simulation Approaches

In general, all approaches are tried on the M6502Core, in terms of PPU nothing particularly new. We take the circuit and repeat its work in C++. In the beginning somehow, and then we try to optimize it.

All of the PPU schematics can be found here: https://github.com/emu-russia/breaks/tree/master/BreakingNESWiki_DeepL/PPU

To simplify understanding, the following image shows the "layers" in which the individual parts of the PPU are simulated:

![ppu_layers](ppu_layers.png)

## Giga Uroboros

When it comes to simulating sequential circuits, the biggest butthurt is delivered by the kind of "ouroboros" found in the OAM Eval circuit:

![OAMEval_GigaUroboros](OAMEval_GigaUroboros.png)

(By "ouroboros" we call parts of circuits where the first part depends on the second and the second part depends on the first. Such "loops" are extremely difficult to simulate with a single pass).
