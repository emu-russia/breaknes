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
