# Nescartdb

Converted **nescartdb** database in JSON format, used to identify NES/Famicom cartridge PCBs by the content of their PRG/CHR dumps.

This folder is the single source of cartridge-identification data for every Breaknes consumer: the native core (BreaksCore), the managed application (Breaknes), the SDL port, and the tools (PPUPlayer, APUPlayer).

---

## 1. Why this folder exists

The old `Mappers` component identifies cartridges by the iNES header and the iNES mapper number. That is a poor fit for a functional simulator:

- The same iNES mapper number covers many electrically different boards.
- The header does not say what chips are on the board or how they are connected.
- Unlicensed/Famiclone boards often cannot be expressed with iNES at all.

The solution (issue #508) is to identify cartridges by **board type** using the nescartdb database, which describes the actual PCB of every dumped cartridge (components, sizes, CRCs, mirroring).

The native code has no XML engine, so the nescartdb XML is converted to JSON once, stored here, and consumed by everyone.

## 2. Data source

| | |
|---|---|
| Database | [nescartdb](https://nescartdb.com) (bootgod / brizzo) |
| Export used | `forum.nesdev.org - tepples/NesCarts (2017-08-21).utf8.xml` from the [MetaFight/NesCartDB](https://github.com/MetaFight/NesCartDB) mirror |
| Contents | ~3179 cartridge entries, 273 distinct board types, for all regions (NES-NTSC, NES-PAL, Famicom, Dendy, ...) |

The export is a static snapshot (2017-08-21). The conversion is deterministic: the same XML always produces the same JSON, so the JSON can be regenerated at any time.

## 3. Files in this folder

```
Nescartdb/
  Readme.md          <- this file
  nescarts.json      <- the full converted database (generated, committed)
  index.json         <- flattened lookup index: prg_crc/chr_crc -> board type (generated, committed)
```

- `nescarts.json` — the full database, faithful to the XML: `game` → `cartridge` → `board` nesting, with all attributes (see §5).
- `index.json` — a flat array of lookup records (`prg_crc`, `chr_crc`, `board.type`, `board.pcb`, `mapper`, `system`), generated from `nescarts.json`. The runtime loads this small file to answer "which board types match these PRG/CHR CRCs?" without walking the whole database.

Both files are **committed** to the repository so that no network access is needed at build or run time.

## 4. Conversion pipeline

```
Tools/NescartdbConvert/convert.py
   XML (tepples export)  --->  nescarts.json  --->  index.json
```

- The converter is a small Python script (like `Tools/DumpRegdump/DumpRegdump.py`), kept in `Tools/NescartdbConvert/`.
- It is run manually (or in CI) when the source XML is updated; its output is committed.
- The converter normalizes data for native consumption:
  - sizes: `"256k"` → bytes (`262144`), `"8k"` → `8192`, etc.;
  - booleans: `battery` → `true`/`false`, `prototype` → `true`/`false`;
  - attribute names are preserved 1:1 where possible (see the mapping table in §5).

## 5. JSON schema (`nescarts.json`)

Direct mapping of the XML structure:

```json
{
  "source": {
    "name": "NesCartDB",
    "agent": "NesCartDB",
    "author": "BootGod",
    "export": "forum.nesdev.org - tepples/NesCarts (2017-08-21).utf8.xml",
    "converted_at": "2026-09-01T00:00:00Z",
    "converter": "Tools/NescartdbConvert/convert.py"
  },
  "games": [
    {
      "name": "10-Yard Fight",
      "altname": "１０ヤードファイト",
      "class": "Licensed",
      "subclass": "3rd-Party",
      "catalog": "IF-02",
      "publisher": "Irem",
      "developer": "Irem",
      "portdeveloper": null,
      "region": "Japan",
      "players": 2,
      "date": "1985-08-30",
      "cartridges": [
        {
          "system": "Famicom",
          "revision": null,
          "crc": "836C4FA7",
          "dump": "ok",
          "dumper": "bootgod",
          "datedumped": "2007-05-06",
          "prototype": false,
          "board": {
            "type": "IREM-NROM-128",
            "pcb": "IREM-01-V",
            "mapper": 0,
            "prg":  { "name": null, "id": null, "size": 16384, "crc": "D3D248C9" },
            "chr":  { "name": null, "id": null, "size": 8192,  "crc": "9C124A53" },
            "wram": null,
            "vram": null,
            "chips": [],
            "cic":   [],
            "pad":   { "h": 0, "v": 1 },
            "peripherals": []
          }
        }
      ]
    }
  ]
}
```

### 5.1 XML → JSON mapping

| XML | JSON | Notes |
|-----|------|-------|
| `<database>` | `source` | `version`, `conformance`, `agent`, `author`, `timestamp` folded into `source`; the converter adds `export`, `converted_at`, `converter` |
| `<game>` | `games[]` | `name`, `altname`, `class`, `subclass`, `catalog`, `publisher`, `developer`, `portdeveloper`, `region`, `players`, `date` |
| `<cartridge>` | `games[].cartridges[]` | `system`, `revision`, `crc`, `dump`, `dumper`, `datedumped`, `prototype` (`sha1` intentionally omitted — identification is CRC-only) |
| `<board>` | `...board` | `type` (board type), `pcb` (board revision), `mapper` (iNES mapper number — informational only, **not** used for identification) |
| `<prg>` / `<chr>` | `board.prg` / `board.chr` | `name`, `id`, `size` (bytes), `crc` (`sha1` intentionally omitted) |
| `<wram>` / `<vram>` | `board.wram` / `board.vram` | `size`, `battery`, `id` |
| `<chip>` | `board.chips[]` | `type` (e.g. `MMC1A`, `MMC3B`), `battery` |
| `<cic>` | `board.cic[]` | `type` (e.g. `3193A`, `6113`) |
| `<pad>` | `board.pad` | `h`, `v` (mirroring) |
| `<peripherals>`/`<device>`/`<pin>` | `board.peripherals[]` | passthrough of the raw pin data |

### 5.2 Lookup index (`index.json`)

```json
[
  {
    "prg_crc": "D3D248C9",
    "chr_crc": "9C124A53",
    "system": "Famicom",
    "type": "IREM-NROM-128",
    "pcb": "IREM-01-V",
    "mapper": 0
  }
]
```

- One record per cartridge with a known PRG/CHR CRC pair.
- Identification is by **PRG CRC32 + CHR CRC32** only (SHA1 is not used): the runtime computes CRC32 of the PRG and CHR dumps and looks up `prg_crc` + `chr_crc`.
- Multiple records may share the same CRCs (same ROMs on different PCBs) — all matches are returned; the caller picks the fit (see `CartPcb/Readme.md` §8).

## 6. Consumers

| Consumer | How it uses this folder |
|----------|-------------------------|
| BreaksCore (native) | `CartPcb::NesCartDb` loads `index.json` for CRC → board type lookup. |
| CartPcb (native) | Board JSONs referenced by the index (built-in definitions + user overrides). |
| Managed Breaknes app | Locates the JSON next to the executable (copied at build time) and can also parse it directly for UI purposes (board type shown to the user). |
| SDL port | Same native path as BreaksCore. |
| PPUPlayer / APUPlayer | Use the same `Nescartdb` data via the native core or their own copy. |

**Deployment**: the `Nescartdb/` files are copied to the output directory on build (like other data files). Native code locates them relative to the executable; a command-line override (`--nescartdb <path>`) is planned.

## 7. Versioning & regeneration policy

- `nescarts.json` and `index.json` are generated artifacts, but they are **committed** (no network at run time, diffable history).
- When the source XML is updated: re-run `Tools/NescartdbConvert/convert.py`, commit the regenerated JSON, bump the `source.converted_at` timestamp.
- The JSON schema is versioned via `source.version`; breaking changes to the schema bump the version and are coordinated with `CartPcb`'s loader.

## 8. Licensing & attribution

- The data is the nescartdb database by **bootgod** (site run by bootgod and brizzo), mirrored at [MetaFight/NesCartDB](https://github.com/MetaFight/NesCartDB).
- The upstream mirror ships **no explicit license**, so the data must be treated as all-rights-reserved: keep the attribution (`source` block in the JSON and the `Nescartdb/Readme.md`), and verify redistribution terms before any release packaging.
- The XML export itself originates from a forum.nesdev.org post by **tepples** (2017-08-21); credit is kept in `source.export`.

## 9. Relationship to CartPcb and JSONES

- `Nescartdb/` provides **identification** (which PCB is this dump?) — see `CartPcb/Readme.md` §8.
- `CartPcb` provides **simulation** of the identified PCB — the board JSON format is defined in `CartPcb/Readme.md` §5.
- **JSONES** (issue #189) is a subset of the CartPcb board format: a single-cartridge JSON with `game` → `cartridge` → `board` nesting, usable as a custom board definition in addition to the built-in nescartdb data (`CartPcb/Readme.md` §6.5).
