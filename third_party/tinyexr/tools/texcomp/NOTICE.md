# TinyEXR texcomp — Licensing and Third-Party Notices

## License of this component

The TinyEXR **texcomp** texture-compression suite (everything under
`tools/texcomp/`) is original work licensed under the **Apache License,
Version 2.0**.

    Copyright 2014-2026 Syoyo Fujita and TinyEXR authors
    SPDX-License-Identifier: Apache-2.0

The full license text is in [`LICENSE`](LICENSE) in this directory, and every
source file carries an `SPDX-License-Identifier: Apache-2.0` tag.

Note: this is a licensing choice for the texcomp subcomponent specifically. The
rest of the TinyEXR project is BSD-3-Clause; Apache-2.0 is compatible with it
for combined/redistributed works.

The pure-C11 library (`make texcomp`) has **no third-party runtime
dependencies**. The pieces below are either (a) vendored elsewhere in the repo
and only used by optional CLI/test targets, or (b) small ports / spec-data
transcriptions whose provenance is credited here and in the relevant file
headers.

---

## Third-party components

### Arm astcenc — Apache-2.0
- Copyright 2011-2024 Arm Limited. Vendored at `deps/astcenc`; license in
  `deps/astcenc/LICENSE.txt`.
- **Optional C++ backend.** `make texcomp-arm` builds astcenc as an alternate
  encoder selectable with `--encoder arm`. The default `make texcomp` build is
  pure C11 with no C++ parts.
- **HDR endpoint codec port.** The ASTC HDR CEM 7 (base+scale) and CEM 11
  (RGB direct) endpoint pack/unpack in `src/texcomp_astc_hdr.c`
  (`tc_astc_cem7_pack/unpack`, `tc_astc_cem11_pack/unpack`) are ports of
  astcenc's `quantize_hdr_rgbo`, `hdr_rgbo_unpack` and `hdr_rgb_unpack`.
- **Conformance oracle.** The `texcomp-astc-arm-gate` and
  `texcomp-astc-hdr-gate` tests decode texcomp output with astcenc's conformant
  decoder to verify correctness.

### bcdec.h — MIT (dual-licensed MIT / Unlicense)
- Copyright (c) 2022 Sergii Kudlai. https://github.com/iOrange/bcdec
- The **corrected BPTC (BC7) partition + anchor tables** in
  `test/bc7_ref_decode.h`, and the **BC6H two-region (mode 9) partition table
  and block bit layout** in `src/texcomp_bc6h.c`, are transcribed / mirrored
  from bcdec (the tables published in the Khronos spec contain known errors).
- The **BC6H reference decoder** `test/bc6h_ref_decode.h` (all 14 modes) is a
  C port of bcdec's `bcdec_bc6h_half`, used test-only as the conformance oracle
  for the `texcomp-bc6h-gate`. It is not linked into the shipped library.

### Khronos ASTC / BPTC specifications
- The ISE codec, 2D block-mode tables, partition-pattern hashing, and
  unquantization tables in `src/texcomp_astc.c`, and the BC6H/BC7 block bit
  layouts, follow the Khronos Data Format Specification (ASTC) and the BPTC
  spec. Specification data (constants, bit layouts) is not copyrightable.

### Basis Universal — Apache-2.0
- Copyright (c) 2019-2024 Binomial LLC. https://github.com/BinomialLLC/basis_universal
- The UASTC LDR / UASTC HDR 4x4 **mode subsets** implemented by texcomp's
  `--format uastc_ldr` / `astc_hdr` follow the Basis UASTC specifications (all
  output is standard ASTC). The `xbc7` format is a texcomp-native take on the
  XBC7 *idea* (windowed RDO + entropy coding of BC7) — it is **not**
  bitstream-compatible with Basis XBC7.
- The `uni` universal-transcodable intermediate (`--format uni`,
  `src/texcomp_uni.c`: encode once, transcode at load to BC7/BC1/ASTC/ETC2)
  follows the *concept* pioneered by Basis Universal / UASTC — a canonical
  single-subset block re-packed cheaply to the device's GPU format. It is a
  texcomp-native intermediate and is **not** the Basis `.basis`/UASTC bitstream.
  Its optional codebook supercompression (`--basis`) is a BasisLZ-*style*
  endpoint/selector deduplication with RDO; it is **not** the Basis ETC1S codec
  and does not emit KTX2 `supercompressionScheme=1` (BasisLZ) — the KTX2 output
  uses Zstd (scheme 2) with the codebook as an inner pre-transform.

### QuickBC7 / etcpak
- BC7 quick-mode heuristics and API naming derive from a pure-C11 port of the
  QuickBC7-enabled etcpak fork.
  - Paper: *QuickBC7: Fast BC7 Texture Compression Heuristics*, Hyeon-ki Lee and
    Jae-Ho Nah, Computers & Graphics 2026.
  - Reference source: https://github.com/gusrlLee/etcpak (commit `b88c8f4`).

### Zstandard (zstd) — BSD-3-Clause (dual BSD / GPLv2)
- Copyright (c) Meta Platforms, Inc. Used via TinyEXR's vendored `deps/zstd`
  (`tinyexr_zstd`). The `xbc7` container entropy-codes the BC7 stream with zstd,
  and the `uni` KTX2 writer uses zstd for per-level supercompression
  (`supercompressionScheme=2`). Only the CLI links zstd; the pure-C11 texcomp
  library does not depend on it.

### stb_image — public domain / MIT
- Sean Barrett. `examples/common/stb_image.h`, used by the CLI (`texcomp_cli.c`)
  only, for PNG loading.

---

## Feature status (informational)

- **BC1/BC3/BC5/BC7** direct encoders; BC7 covers all 8 modes with a quick-mode
  heuristic path and an exhaustive search, plus optional windowed RDO (`--rdo`).
- **BC6H** (unsigned + signed): one-region mode 11 and two-region mode 9, both
  with least-squares endpoint refinement; the mode-11 selector search has
  SSE4.1/AVX2/NEON kernels that are bit-identical to scalar (cross-backend
  parity tested). The higher-base delta-encoded 2-region modes remain future
  work.
- **ETC2 / EAC** encoders.
- **ASTC LDR**: all fourteen 2D footprints, 1-4 partitions, dual-plane, CEM
  0/4/6/8/10/12; integer least-squares endpoint refinement; SSE2/SSE4.1/AVX2/
  NEON kernels bit-identical to scalar. Includes the constrained UASTC LDR
  19-mode subset (`--format uastc_ldr`).
- **ASTC HDR** (UASTC HDR 4x4): void-extent + CEM 7 (base+scale) and CEM 11
  (RGB direct: single / dual-plane / 2-subset), LSQ-refined.
- **xbc7**: texcomp-native supercompressed BC7 (windowed RDO + zstd), transcodes
  to standard BC7.
- **uni**: universal transcodable intermediate — encode once, transcode at load
  to BC7/BC1/ASTC/ETC2; optional codebook (`--basis`) + Zstd KTX2 mip chain.

Conformance is verified by in-tree reference decoders (`test/astc_ref_decode.h`,
`test/bc7_ref_decode.h`, `test/astc_hdr_ref_decode.h`) and by cross-checks
against astcenc; quality is tracked by `make texcomp-astc-psnr`.
