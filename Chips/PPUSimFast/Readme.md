# PPUSimFast

An optional sibling of [PPUSim](../PPUSim/Readme.md) for issue #502.
`PPUSimFast::PPU` preserves the PPU method surface and aliases the original
pad/revision enums, video values and debug structures. Its Visual Studio 2026
project is in `Scripts/VS2026`. The five implemented revisions remain supported:
RP2C02G, RP2C02H, RP2C04-0003, RP2C07-0 and UMC UA6538.

The simulator keeps the original master-half-call interface and module order.
CPU register accesses still run on every call. H/V logic, evaluation, memory,
background and sprite pipelines still run when PCLK changes. Composite phase
continues on every master call; RAW mode keeps the original early return that
pauses the composite phase ring. The FSM and sprite-evaluation control logic
retain their original sequential update order.

## Third-pass representation

The third pass packs the eight sprite FIFO lanes together, replaces expanded
H/V decoder outputs with words, and carries packed OAM, scroll and tile values
through the FSM, background, address and pixel-mux boundaries. High and low PCLK
paths retain different prepared/visible banks. The published tile complement
is stored separately: reset can make it disagree with the published direct
value. Public debug setters likewise retain the original delay before a stored
OAM or scroll value appears on an exported wire.

The composite phase generator uses its 33 reachable bank states. Video input
latches are reevaluated when their pixel-body producers run or an output-mode
change invalidates the context; master-phase recurrence and output sampling
continue on every call. RAW pause/resume, reset-forced pixel work, held clocks,
public counter resets and all five revision dividers remain explicit. Direct
VideoGen tests still accept arbitrary inputs on every invocation.

The CPU port distinguishes inactive/released fixed points from active register
accesses. A skipped internal evaluation never suppresses a live bus driver or
leaves a new output object unpopulated. These changes do not introduce scanline
batching or reduce the number of public calls. See the
[joined benchmark and phase contract](../../UnitTest/FastSim/Readme.md#joined-third-pass-performance-acceptance)
for the combined CPU/APU/PPU target and acceptance rules.

The final [v3 component comparison](../../UnitTest/FastSim/results/appleclang21-arm64-v3-components.json)
uses nine full-second pairs per workload. Active time falls from 2206.330 to
1341.867 ms (1.6455x median paired speedup); legacy time falls from 1932.691 to
1089.765 ms (1.7717x). Every pair wins with matching checksums. Full Release,
Debug and ASan+UBSan checks pass on the final shared source snapshot. These
times do not establish the separate combined 800 ms target.

## Packed phases and arithmetic blocks

- Sprite FIFO counters keep separate current/prepared bytes. Paired sprite and
  background shifters keep separate input/output banks. An enabled transparent
  output can propagate the serial input through all eight ordered stages in one
  call; this is preserved rather than replaced by a parallel single shift.
- Sprite evaluation captures the previous OAM buffer on high PCLK, subtracts Y
  from the low eight V-counter bits, and retains the capture on low PCLK. The
  widened subtraction rejects negative differences even when the byte row wraps.
  Both 8- and 16-line ranges and copy overflow remain part of the predicate.
  Main and secondary evaluation counters use packed keep/prepared banks. Mode4
  keeps the original lower-two-bit blocking and upper-six-bit carry behavior.
- H/V counters, scroll registers, tile counters, pattern address and address mux
  use masked integer state. Tile reset clears the stored register only in the
  original selected phase; the immediate reset pulldown does not clear the
  retained complement/carry path. An unselected address source is Z and holds
  the input bank. Address debug values are reconstructed from the retained mux.
- OAM transactions use bytes with a driven mask, retained read/write buffers and
  the original PCLK decay threshold. All implemented bits of a byte are written
  together through the public interface, so a shared timestamp preserves their
  decay state. Unused attribute bits and Evaporate remain undriven. Palette RAM
  uses 28 physical six-bit cells with the original 32-address alias mapping and
  explicit read, luma and chroma pipeline banks.
- Register decoders and released control-register values are cached until their
  inputs change. Register write/release visibility, PAL BLACK delay and the
  first/second scroll-write transition remain explicit.
- Pixel-clock and video phase tables retain every latch bank, including startup,
  reset and repeated/held phases. X/Z clocks close both banks in the Release
  kernel fallback. The original public Debug `NOT(X/Z)` exception is unchanged.
  Color latches and EXT output use packed nibbles; RGB output retains its phase
  and current-summation factor.
- NTSC chroma uses direct phase selection; PAL chroma uses the original PLA masks
  packed into valid color/parity entries. The composite DAC table moves the
  original ordered floating-point minimum operations to configuration time.
  Changing external DAC levels rebuilds it. Noise is still added per sample.

The packed byte kernels use binary data at the chip's normal byte boundaries.
OAM outputs are normalized before sprite FIFO processing, including after an
unwritten or evaporated cell; pattern data is normalized by NAND. Debug
assertions check those FIFO boundaries. This does not provide arbitrary injected
X/Z storage inside private packed banks. Selector/latch distinctions are tested
where the original kernel accepts four-state controls.

## Verification

[PPU kernel tests](../../UnitTest/FastSim/PpuKernelTests.cpp) use the unchanged
scalar PPU as the oracle. They compare both banks, borrow/carry and serial
outputs across factorized exhaustive FIFO/background/sprite-counter states;
all sprite Y/V pairs; H/V prepared-bank transitions and reset; every video phase
bank state; arbitrary phase/color/burst/PAL parity inputs; color/EXT banks; pixel
divider states; and every DAC digital selection with default and nonmonotonic
external levels. Original leaves gain only the FIFO test friend declarations.
Other hidden state is initialized or exposed through the existing scalar methods.

`fastsim --test-ppu` passes the selected Release/LTO source: all five revisions
receive 2.4 million active half-calls, NTSC also receives an idle trace, and every
call compares pads, buses, H/V/PCLK getters and active deterministic video.
[Public API tests](../../UnitTest/FastSim/PpuApiTests.cpp) cover the exact
1,000,000-PCLK OAM decay boundary for Keep/ToZero/ToOne/Evaporate, refresh, custom
DAC selection and noise enable/disable. Conversion tests compare all 1,024 RAW
values for each implemented revision. Noise-enabled tests check finite output
and digital parity, without requiring process-global random samples to match.
Windows Randomize uses timing-dependent samples and is not a deterministic
sample-equivalence claim. Debug byte peeks map Evaporate's Z to zero; the API
cannot distinguish that value from a stored zero without observing the bus.

Whole-chip and archived-ROM comparisons assess the original implementation's
behavior; they do not establish hardware correctness beyond that oracle. ROMs
that fail in both implementations remain baseline issues.

## Development comparisons

The [final full campaign](../../UnitTest/FastSim/results/appleclang21-arm64-v2.json)
subsequently passed at 2.2276x idle and 2.0722x rendering speedup over the frozen
first pass. Each workload won all nine pairs with matching checksums across
21,477,272 timed master clocks per sample. Full Release, Debug and ASan+UBSan
checks passed on the final source snapshot; native Windows execution is unverified.
The shorter pilots below retain the evidence for selecting the implementations.

Three serial alternating runs used 1,000,000 master clocks and 100,000 warmup
clocks, Clang 21 ARM64, Release `-O3` with LTO for every variant. These are short
development pilots, separate from the final acceptance results above.

| Candidate | Idle median ms | Rendering median ms |
| --- | ---: | ---: |
| Frozen first-pass fast | 195.507 | 205.579 |
| Table phase, fused packed background | 88.435 | 100.565 |
| Table phase, split background methods | 91.071 | 103.679 |
| Direct arithmetic phase, fused background | 93.134 | 105.293 |

Table phase is retained. Fused background is 3.10% faster than split on the active
pilot, above the 3% preference threshold. Both alternatives passed the full PPU
suite before timing. Compile `PPUSIM_FAST_VIDEO_TABLE=0` for direct phase or
`PPUSIM_FAST_BG_PHASE=1` for split background; the defaults are table/fused.
`PPUSIM_FAST_DAC_TABLE=0` retains the scalar DAC expression for experiments.

After the closed-clock fallback and EXT packing, the selected source measured
87.327 ms idle and 100.706 ms rendering against first-pass medians 192.113 and
203.701 ms: 2.200x and 2.023x. Every variant and selected sample retained the same
workload checksum. The rendering margin is small and is not an absolute
real-time performance claim.

[Raw samples and build manifests](../../UnitTest/FastSim/results/experiments/ppu-v2/ppu-candidates.json)
and the [selected-source pilot](../../UnitTest/FastSim/results/experiments/ppu-v2/ppu-selected-final.json)
record complete commands, binary SHA-256 and build-input digests. Per-variant
manifests are stored beside them; temporary build paths identify the original
experiment location and are not install paths. Earlier exploratory pilots in
that directory lack the final manifests and are not the selected-source result.
Reproduce with the [portable harness](../../UnitTest/FastSim/Readme.md), select
identical compiler/LTO flags, and serialize timing runs.

## Fourth pass (v4, 2026-10-14): idle class and the phase-specialized edge body

The fourth pass kept the per-call contract (two `sim()` calls per master cycle, every output object written
every call) and changed the per-call *structure*. Every omission is re-run in `FASTSIM_SKIP_ORACLE` builds from
the same pre-state (`PPU::sim_IdleOracle` snapshots and restores the video generator before re-running the full
path; the edge oracle rebuilds the edge context from the stored wires after every edge body) and the full suite
is a gate in that build.

**Exactness fix found on the way (RP2C04-0003).** The third-pass RGB generator published only at pixel edges;
the reference evaluates it every call, so an emphasis write landing on a PCLK-low non-edge call changed the
reference output that call (scenario call 715,474: red 245 vs 140). The generator is now evaluated every call
for the RGB revision and `--test-rgb-emphasis` (odd period, so every master phase is visited) is part of
`--test`.

**Idle class (round 1).** A call is idle when the divider step produces no PCLK edge, the CPU port is released
with no release call owed, RES and RC are low and the video output is not RAW with no pending mode change. In
that class the reference changes only the divider, the phase ring and its sample, the per-call sampled wires
and the output objects; RESCL/Reset_FF, the register FFs and the PAL BLACK pipeline, the video latches and the
pixel body are at a fixed point. The idle call therefore steps the divider and the phase ring, produces the
sample from the phase mask captured at arming (TINT and n_PZ as masks), copies the pads, the EXT output latch,
the AD/PA drive and the pad decode captured at arming, and takes EXT_In from a 16-entry word table.
`PPU::sim` is a dispatcher: when armed, `sim_IdleCall` peeks at the live conditions without committing and
either completes the idle call or returns with no side effects, after which the out-of-line `sim_Full` runs
from scratch; X/Z clocks and composite noise take the full path. The settling "tail" call after a pixel edge
is an idle call for all five revisions (every transparent latch outside the pixel body recaptures a value
only the pixel body or a register access can change, and the edge call evaluated each once in the new phase).
Every public mutator disarms (`TestIdleClassPublicMutations` drives each one between idle-class calls on three
revisions — the first version of the idle path missed a `SetRAWOutput()` between calls). Idle call 10.1 →
5.4 ns including the harness; legacy −14%, active −6%.

**Phase-specialized edge body (round 2).** The pixel-edge body (25% of legacy calls, two thirds of the PPU's
time, ~2700 inlined instructions with ~450 byte loads and ~320 byte stores of `TriState` wires between
modules) is instantiated for PCLK = 0 and PCLK = 1 (`EdgePhase<0/1>`; `sim_PCLK` always yields a binary PCLK
from the divider table or the held-clock reconstruction), and every same-call producer→consumer value (FSM
commands, delayed H, the /OBJ_RD strobes, CLP, the VRAM strobes, OFETCH, the evaluation products, the FIFO
lanes, TH_MUX/XRB) travels in a register-resident `EdgeContextT<Phase>` instead of the `wire`/`fsm` byte
fields. Each module body is textually the reference-order evaluation it was; producers still store the wire
fields (debug accessors, `--test-wires` and the kernel tests read them), consumers read the context, and values
a stage needs from the *previous* call keep reading the stored wire. `EdgePhase<-1>` carries live values for
the field-driven kernel-test entry points via `CaptureEdge`. All body stages are always-inline
(`PPUSIM_FAST_EDGE_INLINE`, empty under MSVC) so the context is scalarized. The video generator body became
`VideoOut::sim_T<Phase>` and the edge body calls `sim_Edge<Phase>` inline where the reference evaluates the
generator, with SYNC/BURST, /PICTURE and /LL0,/LL1 taken from the context; the generic `sim()` is unchanged for
non-edge calls, VideoGen-only instances, `ConvertRAWToRGB` and the idle oracle. Legacy 21.7 → 17.2 ns, active
29.5 → 24.4 ns per half-call; derived pixel-edge call 71 → 53 ns; the idle path is untouched.

**Rejected by measurement.** A packed FSM latch bank (+0.3%), the edge context with a runtime phase (−1.9%),
a single published copy of the context (+5.9%), FSM stores derived on demand (−1.8/−2.3%), revision templating
(constant-revision upper bound −0.7/−1.2%, not built), a lean port-active non-edge path (≈0 on the joined
workload). `FASTSIM_ABLATE_PPU_BODY` and `FASTSIM_ABLATE_MASK` are timing-only diagnostics for attributing the
edge body per module group; binaries built with them are never acceptance builds.

**Invariants a future change must keep.** Any new public mutator must disarm the idle class. Any new
field-driven test entry point must capture its context through `CaptureEdge`. The hot path instantiates only
`EdgePhase<0>/<1>`, which relies on `sim_PCLK` producing a binary PCLK. The generic `VideoOut::sim` still runs
out of line on non-edge full calls. Numbers, commands and gates:
`emu/artifacts/fastsim-v4/track-reports/ppu-*track-report.json`.
