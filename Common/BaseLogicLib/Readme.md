# Base Logic

Basic logic primitives used in N-MOS chips.

The implementation principles are similar to SystemC, but without using SystemC :)

## Edge Simulation

In the literal sense, the edge is not simulated (in the sense of the time interval).

Instead an approach with comparison of the previous CLK value and the current one is used.

Specifically for emulation purposes (M6502Core, APUSim, PPUSim) this technique is not applied, because most of the circuits of these chips are made using NMOS technology with dynamic logic on latches (DLatch).

If you suddenly need to simulate an edge, the library has two calls (`IsPosedge` and `IsNegedge`).
