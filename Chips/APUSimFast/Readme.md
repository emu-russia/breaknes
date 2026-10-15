# APUSimFast

Fast sibling of APUSim, based on BreakNES commit `215ab82`. Public enums,
audio values and debug layouts are the original APUSim types. The separately
compiled APUSim and current virtual M6502 interface remain the reference.

The second pass uses packed state and phase-dependent execution. The public
simulation stage order is preserved: pads/input bus, DPCM, OAM DMA/control and
buffer, address mux, CPU divider/core, clock/frame sequencer, register decoder,
sound channels, output pads/bus and DAC. DPCM and DMA therefore see the previous
call's clock/register signals; the CPU and sound channels keep their original
PHI-edge scheduling. A stable master phase never triggers several settling
passes inside a single API call.

## Third-pass representation

The third pass makes all addresses outside the APU register window share one
inactive decoder key, and produces the selected read/write strobes directly.
CPU high-address qualification remains separate from the low address selected
by DMA. Debug lock and live debug-register bus drives still execute in order.

DPCM and OAM DMA arbitration were tested as bounded transition tables, including
the intermediate values consumed later in the same call. Their speed difference
from direct equations was below the selection threshold; the direct equations
remain the simpler default. The frame controller stores ten
feedback bits in a word, retains the separate fifteen-bit LFSR banks, and uses
a scalar dependency key and explicit feedback comparison. GPIO has three-bit
input/output banks; register transfers and debug bus reads use packed bytes.
The DPCM period ROM remains packed through its frequency-control consumer.

The standalone harness can pair this APU with the opt-in
`M6502Core::ExecutionBackend::ExactPacked` CPU. Existing application constructors
still select the reference CPU. The
[joined benchmark](../../UnitTest/FastSim/Readme.md#joined-third-pass-performance-acceptance)
counts the CPU once, consumes analog audio and composite video on every master
half-call, and checks the combined timing target separately from correctness.

The final [v3 component comparison](../../UnitTest/FastSim/results/appleclang21-arm64-v3-components.json)
uses nine full-second pairs per workload. Including the selected CPU, active
time falls from 2196.535 to 1293.874 ms (1.6938x median paired speedup); legacy
time falls from 2178.398 to 1281.967 ms (1.6993x). All checksums agree and every
pair is faster. These are component measurements; the combined target has its
own [joined report](../../UnitTest/FastSim/results/appleclang21-arm64-v3-joined.json).

## Packed kernels and arithmetic

- The six-stage CPU divider and fifteen-stage frame sequencer retain separate
  input/output banks and their original initialization. Frame product terms are
  six exact word comparisons. The transparent-latch ripple case is explicit.
- DPCM retains two banks for the address, sample and sample-bit counters, nine-bit
  frequency LFSR and eight-bit sample shifter. Register loads are masked byte
  operations. The six-bit output counter uses exact reversible word arithmetic.
  The period ROM has sixteen entries.
- OAM DMA uses a packed low-address counter, byte page register and byte sample
  buffer. Its sequential arbitration controller and address-mux priority retain
  their original statement order.
- Length, envelope, triangle, sweep, square frequency and duty counters retain
  main/carry banks. Register loads and triangle/duty outputs are word operations.
  Length reload uses the original thirty-two-entry ROM (count minus one).
- Square frequency registers retain both raw transparent and inverted-sum banks.
  The barrel shifter is a signed-fill word shift. An exact parallel-prefix
  implementation replaces the dual-rail adder. During sweep selection, original
  Fx/nFx rails can both be zero; unconditional integer addition would change the
  chip. An experimental arithmetic path handles complementary rails and falls
  back to the same prefix implementation. It measured within 3% of the uniform
  prefix path, so the simpler uniform path is the retained default.
- The register decoder qualifies the CPU high-address window independently from
  the muxed low address. It remembers the decoded combinational result; debug
  lock and physical debug-register bus drives still run at their original stage.
  External pad input/output and OAM buffer transfers use bytes.
- DAC lookup uses a packed channel word produced only when channel blocks run.
  Cached samples are keyed by this word and invalidated by raw/normalized mode
  setters. Every call drives the supplied output object, including cache hits;
  RAW writes preserve the inactive union tail bytes. The original float tables
  and generation arithmetic are unchanged.

Main and carry-generation counter banks initialize to zero. Debug counter writes
change only the main bank, including between carry preparation and stepping.
Load overrides clear, then step; MUX selectors use their low bit, while DLatch
clock enables require exactly One. The original sample-buffer clear produces an
inverted all-one logical output. These hardware-model details are intentionally
preserved rather than replaced by conventional NES counter formulas.

The packed storage invariant is binary state reached from the original binary
latch initialization and binary physical inputs. Physical CLK, reset, IRQ/NMI,
DBG, CPU phase/RnW and byte bus accesses retain their original contract, including
Z pad outputs. Controller temporaries still use BaseLogic and retain startup X
behavior. Arbitrary nonbinary values injected into private wires are diagnostics,
not an additional physical-input domain. No new PAL/revision behavior is added.

## Phase dependency audit

`StablePhase` compares the current dependency key and a public-debug mutation
epoch. A block is skipped only after its stored state did not change on its last
single evaluation. An unchanged key with changing stored state is evaluated again
on the next API call, preserving across-call propagation delay. The snapshot
compares the same object's trivially-copyable state representation. Padding can
prevent a skip, but cannot establish false semantic equality; padding equality
is never an acceptance criterion.

| Block | All incoming dependencies in the key | Live bus/output work |
|---|---|---|
| DPCM | ACLK1, nACLK2, PHI1, RnW, RES, n_R4015, W4010/11/12/13/15, LOCK; DB during a low-bit-active register write or an owned PHI1-low fetch | Status bits 7/4 are reapplied to current DB on a skipped call. DMC address, audio and arbitration outputs are retained from real state. |
| OAM control/address | ACLK1, nACLK2, RES, W4014, RUNDMC, PHI1, RnW, DMCRDY; DB during a low-bit-active page write | PHI2 buffer capture, live DB drive, RW/RD/WR and mux evaluation remain outside the guard. |
| Clock/frame | PHI1, PHI2, RES, W4017, DMCINT, n_R4015; DB6/7 during a low-bit-active W4017 | Status bit6 is reapplied to current DB on a skipped call. |
| Register decode | CPU address window, muxed Ax low5, RnW, PHI1, DBG, mutation epoch | Debug-lock feedback and debug register reads remain live. |

Each state snapshot includes the block's feedback latches, counters, transient
control scalars and latched decode values. It excludes parent pointers and cache
metadata. Public wire/register/status mutations invalidate every dependency cache.
The channel blocks already run only on the original PHI edge and are not cached.

Public diagnostics continue to report actual modeled values through the shared
layouts. Packed main/output banks replace the scalar arrays; private per-bit
objects and some private expanded cones are no longer available. Diagnostic wire
identity is optional (`--test-wires`); external pads, bus ownership/address/data,
per-call audio, PHI0/PHI2/SYNC and software clock counters are required.

## Validation and experiments

`UnitTest/FastSim/ApuKernelTests.cpp` uses the unchanged scalar cells as oracle:
all canonical one-bit counter controls, arbitrary four-bit main/carry banks,
full-width carry/borrow chains, reversible output counters, divider banks,
all frame product words, sample/frequency shift banks, all decoder entries,
CPU-address qualification and square register/barrel/dual-rail arithmetic.
A held-phase probe varies live DB and control inputs and exercises public debug
invalidation. Full chip comparisons cover CPU integration, real OAM/DMC activity,
natural DMC address wrap, frame IRQ, physical DBG, held clocks, short reset and
FakeM6502 playback. The independent integrated NROM fixture compares all eight
reference/packed CPU and slow/fast APU/PPU combinations, plus a paired composite
video trace. Third-pass kernel proofs also cover complete controller table
domains, future transitions, GPIO hidden banks and fresh debug-read bus storage.

The final [archived-ROM comparisons](../../UnitTest/FastSim/results/appleclang21-arm64-roms.json)
used seven mapper-0 images from [nes-test-roms commit 95d8f62](https://github.com/christopherpow/nes-test-roms/commit/95d8f621ae55cee0d09b91519a8989ae0e64753b).
The all-fast board agreed with the all-slow board at every half-call and final
memory state through the 180-frame bound after cold reset. The reference outcomes
remained one PASS (VBlank basics), three FAIL (length counter, length table,
open-bus decay) and three TIMEOUT (VBlank-clear timing, DMC $2007 read,
sprite/DMC DMA collision). All exact outcomes, status values, call counts and
messages matched the earlier reference run; differential agreement does not
turn a reference failure or timeout into a hardware-conformance pass.

Development commands (Apple Clang21 ARM64, equal O3/LTO flags for every backend):

```sh
python3 UnitTest/FastSim/build.py --build-dir /tmp/breaknes-fastsim-apu
/tmp/breaknes-fastsim-apu/fastsim --test
/tmp/breaknes-fastsim-apu/fastsim --bench apu --backend fast --workload active --cycles 1000000 --warmup 100000
```

The frozen first-pass comparator is `artifacts/fastsim-v1/fastsim` in this
repository's parent workspace. Alternating three-run 1M-cycle development
pilots measured: packed clock/frame + dependency guards + pad/OAM byte paths,
1.779x legacy and 1.761x active; adding packed DPCM pipeline, 1.848x legacy and
1.823x active. All paired checksums matched. These short pilots informed further
sound-channel work and are not the final performance acceptance campaign.

A Stage2 optional instrumentation build (`--extra-flag=-DAPUSIMFAST_PHASE_STATS`) counted
2,200,512 active calls at the DPCM stage: clock/frame evaluated 366,832 (16.7%),
DPCM 321,976 (14.6%), and OAM control/address 288,095 (13.1%). Instrumented timing
is not compared with the uninstrumented baseline.

Further candidate comparisons use `APUSIMFAST_DISABLE_PHASE` and
`APUSIMFAST_SWEEP_ARITHMETIC`; neither is enabled in the production project.
The final development pilot alternated three runs per candidate/workload with
1M cycles and 100K warmup. Frozen v1 medians were 242.032 ms legacy and 240.746 ms
active. Uniform prefix medians were 97.720/98.271 ms (2.477x/2.450x); arithmetic
plus prefix fallback measured 97.808/98.327 ms, a difference below 0.1%. Disabling
phase guards raised the arithmetic candidate to 127.596/127.852 ms, about 30%
slower. All paired checksums matched. Both retained prefix and experimental
arithmetic candidates passed full external-contract and integrated traces.
These are development pilots; final cross-binary acceptance uses full NTSC-scale
trials with a frozen source/binary identity. Raw pilot records are in
[the development experiment artifact](experiments/appleclang21-arm64-development.json).
The complete cross-binary campaign is recorded by the shared
[FastSim harness](../../UnitTest/FastSim/Readme.md). VS2026/v145 project:
`Scripts/VS2026/APUSimFast.vcxproj`.

The [final full campaign](../../UnitTest/FastSim/results/appleclang21-arm64-v2.json)
passed at 2.4545x legacy and 2.4510x active speedup over the frozen first pass.
Each workload won all nine pairs with matching checksums; each timed sample
covered 21,477,272 master clocks. Full Release, Debug and ASan+UBSan checks passed
on the final source snapshot. Native Windows execution remains unverified.

## Fourth pass (v4, 2026-10-14): idle class, settledness and the fixed-point rules

The fourth pass changed what a half-call *does*, not what it computes. Three exactness mechanisms were added
and every omission is re-run in `FASTSIM_SKIP_ORACLE` builds (`APU::sim_IdleOracle` and the guard oracles
evaluate the skipped work from the same pre-state and abort on any difference; the full suite runs in that
build as a gate).

**Idle class (round 1).** A call is idle when the divider peeks no PHI0 edge, no register strobe is decoded,
RES and DBG are low and every guarded block is settled; the call then only steps the divider banks, performs
the PHI2 buffer capture / SPR_PPU drive and the WR data output in place, copies the seven pads published by the
arming call (M2 recomputed from n_M2) and re-publishes the DAC sample (RAW fields or the two floats). Every
public mutator (`SetRAWOutput`, `SetNormalizedOutput`, the debug setters) disarms, so the next call takes the
full path. `sim()` is a thin dispatcher; the full body is out of line (`sim_FullCall`), so the idle call has
no frame. The noise channel is packed into words (16-entry period ROM generated from the NOR term lists,
11-bit frequency LFSR and 15-bit random LFSR with load-over-step priority and the ACLK1 input/output banks
kept as in the gate netlist; bounded kernel proof against the cell netlist). CoreBinding, DMA and Pads are
by-value members. Idle call 11.0 → 3.3 ns including the harness; legacy/active −15%/−13%.

**Live-state snapshots (round 2).** `StablePhase` compares, before and after an evaluation, only the state
that survives it: cells, registers, counters and the temporaries an evaluation reads before rewriting them
(DPCM: ED1, SOUT, BLOAD, PCM, DMC1, DOUT; DMA: prev_W4014). Temporaries that are written before they are read
on every evaluation (ACLK2, LOOPMode, n_IRQEN, DFLOAD, the step/load strobes, SPRE/SPRS/NOSPR/DOSPR,
sprdma_rdy, …) are dead at the start of an evaluation; a changing dead temporary used to keep a block
"unsettled" for extra calls.

**Fixed-point settledness (round 2).** A replay of an evaluation with the same inputs can differ from the
first only through state the evaluation changed and then re-read. Each block declares when its evaluation is
provably the identity: CLKGen with ACLK1 low (no read-before-write temporary; `sim_ACLK` updates one bit per
phase and reads the other; every transparent latch opens only while ACLK1 is high; the IRQ and Z flip-flops
are OR-absorbing; the LFSR input bank is a function of the frozen output bank), and with ACLK1 high when Z1
already equals Z and the six product terms of the new LFSR output equal the stored PLA decode (nACLK2 must be
high; PHI1/PHI2 not both high); OAM DMA on the binary transition path (frozen nospr/carry banks, absorbing
controller equations); DPCM when ACLK2 is low (no counter loads or steps, the shifter only recopies). X/Z
inputs never claim a fixed point. Two kernel proofs (`FrameFixedPointProof`, `DpcmDmaFixedPointProof`) replay
the blocks from recorded states. Result: each guarded block evaluates once per PHI0 edge instead of three to
four times per ACLK period; full calls −27%; legacy/active incl. CPU −13%. The idle-class arming additionally
requires `CLKGen::ClockChanged == false` and `RegsDecoder::StrobesChanged == false` for the arming call,
because the blocks and the WR driver consume moved clocks and changed strobes one call later (the first
reachable stale-WR case was found by the idle oracle).

**Rejected by measurement** (patches kept under `emu/artifacts/fastsim-v4/round2/rejected-patches/`):
hand-packed envelope/length/square latches (−1.4%), the exact quiescent skip of envelope units and length
counters (−2.8% in round 1, −1.9/−1.4% in round 2: that work is overlapped with the core and removing it buys
little), DPCM control packing (−0.2/−0.4%; an ablation bounds all DPCM-body work at 12–14% and places the
cost in the counters, LFSR and buffer chains), by-value flattening of the remaining blocks and a 64-state
divider table (≈1% each).

**Invariants a future change must keep.** Any new public setter must disarm the idle class. Any new producer
that the blocks consume one call later must set `ClockChanged`/`StrobesChanged` (or disarm). The fixed-point
predicates and the proofs bind to private members; a change to a block's evaluation order needs its predicate
re-derived and its proof re-run. The skip oracles compare whole block objects including dead temporaries,
which is exact for DPCM/DMA/CLKGen because every dead temporary derives from keyed inputs; a block whose dead
temporary derived from an unkeyed input would need a narrower oracle (the length-counter `reload` case in the
rejected quiescent skip). Numbers and commands: `emu/artifacts/fastsim-v4/track-reports/apu-*track-report.json`.
