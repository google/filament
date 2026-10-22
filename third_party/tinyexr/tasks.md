# TinyEXR v3 JPH Status

Current state:

- `src/exr_jph.c` is a clean-room C11 HTJ2K/JPH front end under BSD-3-Clause.
- OpenEXR HT wrapper parsing is implemented.
- JPEG 2000 marker/profile validation is implemented for the intended OpenEXR subset: SOC/SIZ/CAP/COD/QCD/QCC/NLT/SOT/SOD/EOC handling, RPCL, one layer, 5 reversible 5/3 decompositions, 128×32 HT codeblocks, default precincts, and single tile part.
- Packet parsing validates precinct order, inclusion tag trees, missing-MSB tag trees, pass counts, pass lengths, codeblock byte bounds, cleanup `Scup`, and HT stream stuffing.
- Internal HT forward/reverse/MEL readers exist and are covered by tests.
- Codeblock callback boundary exists. Validated codeblock descriptors include component, resolution, band, subband coordinates, dimensions, pass lengths, missing MSBs, and byte ranges.
- Component coefficient planes are allocated with the codec allocator.
- **Cleanup pass decode** (MagSgn + MEL + VLC, quad-based) is fully implemented for both the int32 (all-HALF) and int64 (FLOAT/UINT) code paths.
- **SPP and MRP refinement passes** are fully implemented for the int32 path and the int64 path.
- Non-empty codeblocks decode correctly through the full pipeline: cleanup → SPP → MRP → coefficient reconstruction → subband-to-plane mapping → inverse 5/3 → inverse RCT (if enabled) → NLT type 3 → TinyEXR block layout store.
- All-empty JPH codestreams decode to zero blocks (short-circuit at the first all-empty band).
- Both scanline and tiled OpenEXR HTJ2K files are supported (decode and encode).
- `make c11-gate` passes (strict `-std=c11 -Wall -Wextra -Werror`).
- `make test-c` passes: **170 passed, 0 failed** (including 12 real-file decode-match tests against openexr-images fixtures, 2 in-tree regression-fixture decode tests, 9 HTJ2K round-trip encode-decode tests, subsampling round-trip, UINT round-trip, mixed HALF/FLOAT round-trip, and synthetic malformed-payload rejection).
- `make fuzz-corpus` passes: 27 regression files, no crash, no sanitizer error (ASan+UBSan+LSan).
- `src/exr_jph.c` has no `assert()`, `abort()`, or `exit()` calls.

## Feature support matrix

| Feature | Decode | Encode |
|---------|--------|--------|
| HALF (16-bit float) | ✅ | ✅ |
| FLOAT (32-bit float) | ✅ | ✅ (kmax=33) |
| UINT (32-bit unsigned) | ✅ | ✅ (kmax=33, NLT type 0) |
| Subsampling (x_sampling/y_sampling > 1) | ✅ | ✅ |
| Tile format (single-level) | ✅ | ✅ |
| Scanline format | ✅ | ✅ |
| Mipmap / ripmap | via tiled writer | via tiled writer |
| HTJ2K32 (32-line blocks) | ✅ | ✅ |
| HTJ2K256 (256-line blocks) | ✅ | ✅ |
| Reversible 5/3 wavelet | ✅ | ✅ |
| NLT type 0 (no transform) | ✅ | ✅ |
| NLT type 3 (sign-magnitude) | ✅ | ✅ |
| Multicomponent RCT (mc_trans=1) | ✅ (int32 + int64) | ✅ (3-channel same-type parts) |
| Deep data | ❌ (writer emits HTJ2K chunks as uncompressed blocks for deep) | ❌ |

## Intentionally unsupported (non-goals)

- **DWA (DWAA / DWAB):** lossy DCT codec; returns `EXR_ERROR_UNSUPPORTED`.
- **Irreversible 9/7 wavelet:** out of scope for clean-room HTJ2K subset.
- **Multi-tile-part codestreams:** one tile part per tile.
- **Precinct sizes ≠ default:** only default precincts (full-tile, single precinct) are accepted.
- **Progression order ≠ LRCP/RPCL:** progression-order changes (POC) outside the default are rejected.

## Key files

- `src/exr_jph.c` (6066 lines): JPH parser, packet parser, HT entropy decoder (cleanup + SPP + MRP), inverse 5/3 wavelet, inverse RCT, NLT, writer pipeline.
- `src/exr_jph_simd.c` (277 lines): runtime-dispatched SSE2/AVX2 NLT, pack, and inverse 5/3 kernels.
- `src/exr_ht_decode.c` (40 lines): stub for future T2 refactoring.
- `src/exr_internal.h`: internal JPH helper declarations.
- `test/unit/test_exr_v3.c` (1421 lines): synthetic JPH profile, transform, packet, HT reader, SIMD parity, and round-trip tests.
- `jph.md`: earlier OpenEXR JPH investigation and implementation plan.

```
Continue implementing TinyEXR v3 JPH/HTJ2K decode in /mnt/nvme02/work/tinyexr.

Read tasks.md, jph.md, src/exr_jph.c, src/exr_internal.h, and test/unit/test_exr_v3.c first. The current JPH front end parses OpenEXR HT wrapper/JPEG2000 profile, validates RPCL packet metadata and HT codeblock segment structure, has HT forward/reverse/MEL readers, allocates coefficient planes, decodes complete all-empty JPH codestreams through inverse 5/3/RCT/NLT and block store, and returns EXR_ERROR_UNSUPPORTED for non-empty codeblocks.

Continue from the non-empty codeblock boundary. Implement the next smallest verifiable piece toward HT cleanup entropy decode in pure C11, keeping it portable, low-memory, and hardened. Do not use assert(), abort(), or exit(); return exr_result errors for all malformed input. Preserve permissive licensing: if referencing/copying OpenJPH tables or code from /mnt/nvme02/work/openexr/external/OpenJPH, keep BSD-2 copyright/license attribution and do not introduce GPL code.

Use apply_patch for edits. Keep unrelated dirty files untouched. After each slice run:
- make c11-gate
- make test-c
- rg -n "assert\\(|abort\\(|exit\\(" src/exr_jph.c

Current known passing state before this continuation: make c11-gate passes, make test-c reports 129 passed, 0 failed.
```
