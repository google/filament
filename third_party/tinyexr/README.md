# Tiny EXR image library.

![Example](https://github.com/syoyo/tinyexr/blob/release/asakusa.png?raw=true)

[![CI](https://github.com/syoyo/tinyexr/actions/workflows/ci.yml/badge.svg?branch=release)](https://github.com/syoyo/tinyexr/actions/workflows/ci.yml)

`tinyexr` is a small library to load and save OpenEXR (.exr) images, good to
embed into your application. It comes in two flavours:

- **[v3 — pure-C11 rewrite (main)](#v3--pure-c11-rewrite-recommended).** The
  current main development line and the recommended version going forward.
- **[v1 — single-header C++ (old, deprecated)](#v1--single-header-c-stable).** The
  original `tinyexr.h`; still a solid, but sunsetting.

> 🌐 **Live demo (v3):** [**TinyEXR v3 WASM viewer**](https://syoyo.github.io/tinyexr/) — decode and view `.exr`
> entirely in the browser (drag-and-drop; all v3 codecs: ZIP / PIZ / PXR24 / B44 / ZSTD / HTJ2K).
> Spectral EXRs get a wavelength scrubber + **CIE→sRGB color** preview, deep images a 3D
> point cloud, and the whole UI is **mobile/touch-friendly** with a fullscreen mode.

> 🎨 **Live demo (tocio):** [**tocio OCIO + ACES 2.0 viewer**](https://syoyo.github.io/tinyexr/tocio/) — decode an
> EXR and apply an **OpenColorIO** transform (incl. the **ACES 2.0** output transforms) on **WebGL2**, with a
> live, editable OCIO config that **JIT-compiles to a GLSL shader**. See [tocio](#tocio--tiny-pure-c11-opencolorio-engine) below.

**Performance (v3) at a glance** — single-thread decode/encode vs the reference
OpenEXR library, with the vendored **libdeflate** backend on/off (and HTJ2K,
which has no deflate path). With the same backend TinyEXR meets or beats OpenEXR
on the deflate family:

![Decode throughput: tinyexr libdeflate on/off vs OpenEXR](doc/perf-libdeflate-htj2k-decode.png)

![Encode throughput: tinyexr libdeflate on/off vs OpenEXR](doc/perf-libdeflate-htj2k-encode.png)

On **ARM64 (Apple M1 / NEON)**, single-thread in-tree throughput — same shape:
TinyEXR leads the cheap codecs (none / rle / b44) and is near parity on HTJ2K:

![Decode throughput, single thread (Apple M1 / NEON)](doc/perf-arm-decode.png)

![Encode throughput, single thread (Apple M1 / NEON)](doc/perf-arm-encode.png)

See [Performance vs OpenEXR](#performance-vs-openexr) below and
[`doc/performance-vs-openexr.md`](doc/performance-vs-openexr.md) for the full
codec-by-codec and multi-threaded numbers.

## v3 vs v1 — feature comparison

| | **v3** (pure C11) | **v1** (single-header C++) |
|---|---|---|
| Language | C11 core, C++-safe header | C++ (C++11 recommended) |
| Integration | `include/exr.h` + `src/*.c` library | single `tinyexr.h` |
| Status | main dev line (next major release) | stable, battle-tested |
| Read codecs | NONE · RLE · ZIP · ZIPS · PIZ · PXR24 · B44/B44A · ZSTD · HTJ2K | NONE · RLE · ZIP · ZIPS · PIZ · PXR24 · B44/B44A · ZFP |
| Write codecs | same as read | NONE · RLE · ZIP · ZIPS · PIZ · PXR24 · B44/B44A |
| Tiled (mipmap / ripmap) | ✅ read + write | ✅ read + write |
| Multipart | ✅ read + write | ✅ read + write |
| Deep | ✅ read + write (scanline + tiled) | ✅ read (scanline) |
| Spectral (JCGT 2021) | ✅ read + write + wavelength-cube API | ✅ read + write API |
| Custom attributes | ✅ read + write | ✅ read + write |
| Streaming block I/O | ✅ bounded memory | ❌ |
| Freestanding / no-libc | ✅ | ❌ |
| WASM | ✅ core + browser viewer | loader only (experimental/js) |
| SIMD | SSE2 / SSE4.1 / AVX2 / F16C / NEON | — |
| Threading | C11 threads (opt-in) | C++11 thread / OpenMP |
| Dependencies | none in core; vendored zstd + libdeflate (libdeflate default on hosted, dropped in freestanding/WASM) | miniz (bundled) + optional zfp |

---

# v3 — pure-C11 rewrite (recommended)

TinyEXR's main development line is a ground-up rewrite as a **pure-C11** library
(`include/exr.h` + `src/*.c`) — the **v3 C API**. It is the recommended version
going forward and will be the next major release.

Highlights:

| Area | Support |
|---|---|
| **Language** | Pure C11 core (no C++ in the library); the public header is C++-safe |
| **Codecs** (read + write) | NONE · RLE · ZIP · ZIPS · PIZ · PXR24 · B44/B44A · ZSTD · HTJ2K — DWAA/DWAB intentionally unsupported |
| **Image kinds** | Scanline + tiled (ONE_LEVEL / MIPMAP / RIPMAP), multipart, deep |
| **Streaming** | Block-at-a-time I/O for bounded working memory (see below) |
| **Custom attributes** | Read **and** write arbitrary header attributes; round-trip through save |
| **Spectral** (JCGT 2021 / afichet `spectral-exr`) | Emissive / reflective / polarised; wavelength-cube load + save. Viewer: wavelength scrubber, CIE→sRGB color, per-pixel spectrum plot |
| **Portability** | Freestanding-capable core, callback file I/O, Emscripten WASM (powers the [live viewer](https://syoyo.github.io/tinyexr/)) |
| **Extras** | Allocator hook, runtime SIMD dispatch (SSE2/SSE4.1/AVX2/F16C, NEON), fuzzed sanitizer-clean test suite |

Build: `make lib` (`build/libtinyexr3.a`), `make test-c`, `make c11-gate`.
Unit tests live in `test/unit/test_exr_v3.c` (run under ASan/UBSan via `make
test-c`); `make fuzz-corpus` replays the fuzzer corpus.

The zlib backend for ZIP/ZIPS/PXR24 is selected with `DEFLATE=auto|libdeflate|intree`
(default `auto` → vendored **libdeflate**, faster on natural-image data). `intree`
builds the dependency-free pure-C codec only; freestanding and WASM builds always
use it. Both codecs link under `auto`, so you can switch at runtime with
`exr_zlib_set_backend()` (e.g. for testing or to honor a custom allocator).

## Performance vs OpenEXR

Benchmarked against the reference **OpenEXR** library (4.0-dev) on an idle AMD
Ryzen 9 3950X (Zen2), `asakusa.exr` 660×440, fully in-memory, both pinned to the
same thread count. Throughput in megapixels/s. Full writeup + charts:
[`doc/performance-vs-openexr.md`](doc/performance-vs-openexr.md).

- **Single thread, dependency-free decode** (`DEFLATE=intree`): TinyEXR is faster
  on the cheap codecs — **uncompressed ~3.4×** (2699 vs 789) and **RLE ~2.5×**
  (230 vs 93). With the pure-C deflate codec OpenEXR leads the compressed family
  (ZIP ~1.2×, PXR24 ~1.8×, ZIPS ~2.1×, PIZ ~2.7×, HTJ2K ~2.5–3×), thanks to its
  libdeflate / tuned PIZ / OpenJPH backends.
- **Single-thread encode:** ties/wins on RLE/PIZ/B44; OpenEXR is ~1.5× on
  ZIP/ZIPS, ~1.8× on PXR24, ~4× on HTJ2K.
- **Default decode uses libdeflate** (`DEFLATE=auto`): with the same backend
  TinyEXR **matches or beats** OpenEXR on the deflate family — ZIP decode
  **1.37×** (80.8 vs 58.8), sizes byte-identical. On a broad natural-image corpus
  this is a large lift over the in-tree codec (ZIP ~+73%, ZIPS ~+63%, PXR24 ~+55%).
  The in-tree inflate is *faster* than libdeflate on smooth, highly-compressible
  data (~+8% on 4K texture packs) but slower on high-entropy natural images, so
  libdeflate is the safer hosted default; both link and switch at runtime via
  `exr_zlib_set_backend()`.
- **In-tree codec tuning:** **PIZ decode ~+14%** (inline Huffman literal store +
  tighter canonical-table scan); ZSTD (vendored upstream) decodes ~410–420 Mpix/s
  here, ahead of the libdeflate ZIP path.
- **Multi-threaded** (opt-in C11 threads, `make … THREADS=1` +
  `exr_set_num_threads(n)`): per-block parallel encode/decode scales **~5×
  (ZIP) to ~8.8× (ZIPS)** to 16 threads. At 16 threads TinyEXR **out-decodes
  OpenEXR on RLE/ZIP/ZIPS/B44** (in-tree), and leads the whole deflate family
  decisively with libdeflate (ZIP decode 339.6 vs 226.6, ZIPS 358.5 vs 151.1).

Compressed sizes are essentially identical (the formats interoperate). Net:
TinyEXR is the fast, dependency-free choice for read latency and cheap codecs;
enabling libdeflate and/or threads puts it ahead of OpenEXR on the deflate family
too.

## Streaming block I/O (bounded working memory)

The pure-C11 v3 API (`include/exr.h`) can decode and encode an EXR one block at
a time — one scanline block or one tile — so peak working memory is a single
block rather than the whole image. This covers scanline, tiled
(ONE_LEVEL/MIPMAP/RIPMAP), and deep parts.

**Decode** — iterate the chunks of a part, decode each into a small caller
buffer, and unpack the channels you need:

```c
exr_reader *r;
exr_reader_open_memory(data, size, NULL, &r);     /* or _open_source for I/O */
uint32_t n;
exr_reader_num_blocks(r, /*part*/0, &n);
for (uint32_t i = 0; i < n; ++i) {
    exr_block_info bi;
    exr_reader_block_info(r, 0, i, &bi);          /* geometry, no pixel I/O */
    void *blk = malloc(bi.uncompressed_size);
    exr_reader_decode_block(r, 0, i, blk, bi.uncompressed_size);
    for (int c = 0; c < header->num_channels; ++c) {
        /* per-channel planar samples for this block */
        exr_block_extract_channel(header, &bi, blk, bi.uncompressed_size, c, dst);
    }
    free(blk);
}
exr_reader_close(r);
```

Deep parts use the two-step `exr_reader_decode_deep_counts` (to size buffers)
then `exr_reader_decode_deep_samples`.

**Encode** — describe parts with `exr_writer_add_part`, then stream blocks to a
file (or a custom seekable `exr_data_sink`); the offset table is backpatched at
`end_stream`:

```c
exr_writer *w;
exr_writer_create(NULL, &w);
exr_writer_add_part(w, &header, NULL);            /* geometry/channels/tiling */
exr_writer_begin_stream_file(w, "out.exr", EXR_COMPRESSION_ZIP);
for (int y = ymin; y <= ymax; y += lines_per_block)
    exr_writer_write_scanline_block(w, 0, y, channel_rows);  /* block-local */
exr_writer_end_stream(w);                          /* backpatch + close */
exr_writer_destroy(w);
```

Tiles use `exr_writer_write_tile` (the caller supplies each level's tiles for
mipmap/ripmap); deep parts use `exr_writer_write_deep_scanline_block` /
`exr_writer_write_deep_tile`.

## Spectral images (v3)

The v3 API reads and writes spectral EXRs in the JCGT 2021 layout
(afichet/[spectral-exr](https://github.com/afichet/spectral-exr)) — emissive
(`S{n}.{wavelength}nm`), reflective (`T.{wavelength}nm`), and polarised
(Stokes S0–S3) spectra, with the comma decimal convention. Helpers
(`exr_is_spectral`, `exr_spectrum_type_of`, `exr_spectral_channel_wavelength`,
`exr_spectral_wavelengths`, …) plus a high-level wavelength-cube
(`exr_spectral_load_from_file/memory`, `exr_spectral_setup_emissive/reflective`)
live in `include/exr.h`. The [browser viewer](#freestanding--embedded--wasm)
opens a spectral EXR as a wavelength scrubber with a per-pixel spectrum plot and
a CIE→sRGB color composite.

## Freestanding / embedded / WASM

The v3 core (`src/*.c` except `src/exr_stdio.c`) is freestanding: it depends only
on `<stdint.h>`, `<stddef.h>`, and `<limits.h>` — no `<stdio.h>`, `<stdlib.h>`,
`<string.h>`, or `<math.h>`.

- **No stdio in the core.** All file I/O lives in the optional `src/exr_stdio.c`
  (`exr_load_from_file`, `exr_save_to_file`, `exr_writer_finalize_to_file`,
  `exr_writer_begin_stream_file`, `exr_reader_open_file`). Link it for the
  convenient path-based helpers; omit it for embedded/WASM. Everything else does
  I/O through caller callbacks: `exr_data_source` (read) for the reader and
  `exr_data_sink` (write/seek/close) for the streaming writer.
- **`-DEXR_FREESTANDING`** drops the default malloc/free allocator (the caller
  must pass an `exr_allocator`; `exr_default_allocator()` returns NULL) and uses
  the internal mem/str implementations in `src/exr_freestanding.c` instead of
  `<string.h>`. `-DEXR_NO_ZSTD` drops the vendored zstd codec (and its
  allocator); `-DEXR_NO_B44` drops the B44 codec. The B44 perceptual tables are
  computed once at runtime (into `.bss`) using a small in-tree `exp`/`log`
  (`src/exr_b44.c`), so the core needs no `<math.h>` and bakes no large table
  into the object; the table test verifies they match a libm reference
  bit-for-bit (`tools/gen_b44_tables.c` regenerates a precomputed variant if one
  is ever wanted).
- `make freestanding-gate` proves the core builds with no libc (scans every
  object with `nm` for forbidden symbols) and runs a memory round-trip.

**WASM** (`make wasm`, needs `emcc`): builds `build/exr_v3.mjs` + `.wasm` from
the core plus the pure-C binding `examples/wasm/exr_wasm.c`
(`exrw_decode_rgba` / `exrw_encode_rgba` / `exrw_free`), with `FILESYSTEM=0`.
See `examples/wasm/README.md` and the `node examples/wasm/test.mjs` smoke test.

**Browser viewer** (`web/viewer/`, needs `emcc` + CMake): a self-contained
WebGL2 EXR viewer built on the v3 streaming block API — drag-and-drop / upload,
load progress, exposure / gamma / channel controls, zoom / pan, data/display
window + region overlays, a pixel picker, a deep-image 3D point cloud, a
spectral wavelength scrubber + CIE→sRGB color + per-pixel spectrum plot, and a
mobile/touch-friendly UI with fullscreen. Build it with the Emscripten CMake
toolchain (MinSizeRel + `-Oz`):

```sh
cd web/viewer
./build.sh          # emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=MinSizeRel && cmake --build build
python3 -m http.server   # then open http://localhost:8000/
```

See `web/viewer/README.md` for details.

## TODO (v3)

Contribution is welcome!

- [ ] Parallelize the deep and mipmap/ripmap paths (encode/decode are
  single-threaded there; flat scanline + single-level tiled already parallelize).
- [ ] Full luminance-chroma color in the browser **viewer**: the core now
  reconstructs subsampled `Y` + `RY`/`BY` to RGBA
  (`exr_part_yc_to_rgba_float`, used by the `examples/wasm` binding), but the
  streaming `web/viewer/` still renders such images as grayscale `Y` pending a
  whole-part hook through the reconstruction helper.
- [ ] ARM/NEON throughput benchmarks (NEON kernels are correctness-verified under
  qemu but not yet benchmarked).
- [ ] Larger-image / higher-channel-count performance sweeps.
- [ ] Multipart in the high-level spectral cube API (part 0 only today).
- DWAA/DWAB — intentionally unsupported (not planned).

## Third-party licenses (v3)

- **Core** — 3-clause BSD, dependency-free. The HTJ2K/JPH and DEFLATE/PIZ/B44
  codec implementations are original in-tree code.
- **zstd** (`deps/zstd/`, optional, on by default) — BSD-3-Clause, Facebook.
- **libdeflate** (vendored ZIP/ZIPS/PXR24 backend, default on hosted builds via
  `DEFLATE=auto`; `DEFLATE=intree` and freestanding/WASM use the pure-C codec) —
  MIT.

---

# tocio — tiny pure-C11 OpenColorIO engine

Alongside the EXR codec, this repo ships **tocio** (`sandbox/tocio/`): a tiny,
self-contained, **pure-C11** [OpenColorIO](https://opencolorio.org) config engine
and code generator. It parses an OCIO config (a YAML subset), resolves a colour
transform into a flat op list, and runs it on the CPU, ahead-of-time as
C / GLSL / Metal source, or through a small JIT — with first-class **ACES 2.0**
support.

> 🎨 **Live demo:** [**tocio OCIO + ACES 2.0 viewer**](https://syoyo.github.io/tinyexr/tocio/)
> — decode an EXR and apply an OCIO transform on **WebGL2**, with a live, editable
> OCIO config that **JIT-compiles to a GLSL shader**.

Highlights:

| Area | Support |
|---|---|
| **Language** | Pure C11, freestanding-capable (no libc/libm beyond `stdint`/`stddef`; all heap via an allocator hook), zero external dependencies |
| **Config** | OCIO YAML-subset parser: colorspaces, roles, displays/views, view transforms, looks, scene↔display reference bridging; Iridas `.cube`, Sony `.spi1d`/`.spi3d`, and ACES **CLF** LUT files |
| **Execution** | CPU interpreter (scalar + **SSE2/AVX2**, **NEON**), AOT **C** source, **GLSL** (WebGL2 / Vulkan) and **Metal** shader generators, and a **JIT** (x86-64 SSE2/AVX + AArch64 NEON machine code; emits GLSL under WASM) |
| **ACES** | **ACES 2.0** output transforms (CAM16 *JMh* tonescale + chroma / gamut compression) — SDR, HDR, and **D60-simulation** variants; ACEScc / ACEScct / ACEScg / ACES2065-1, the 1.3 RGC gamut-compress LMT, ARRI / Sony / RED / Panasonic camera-log inputs, and sRGB / Rec.1886 / PQ / DCDM / Display-P3 display encodings |
| **Ops** | Matrix, range, exponent / MonCurve, log / log-camera, fixed-function, 1D/3D LUT — invertible where the math allows |

**Validated against the real configs.** `make tocio-validate` parses the
AcademySoftwareFoundation ACES OCIO configs and compares every transform tocio
builds against PyOpenColorIO (the C++ reference engine) on fixed samples —
currently **350 / 377 transforms verified, 0 mismatches** on in-range samples.

Build & test:

```sh
make tocio-lib         # build/libtocio.a
make tocio-test        # unit tests
make tocio-fetch-ref   # fetch the ACES OCIO configs (once)
make tocio-validate    # numerical validation vs PyOpenColorIO
make wasm-tocio-demo   # web/tocio/ browser demo (needs Emscripten)
```

See [`sandbox/tocio/`](sandbox/tocio/) for the engine and
[`web/tocio/`](web/tocio/) for the demo.

---

# tir — tiny content-aware image resize

The repo also ships **tir** (`tools/resize/`): a small, fast, **pure-C11**
image resizer that is HDR-, normal-map-, and displacement/height-map-aware.
Standalone (no dependency on the EXR codec — only the CLI and benchmark link
`libtinyexr3.a` for file I/O), one arena allocation per resize, with runtime
SIMD dispatch. Separable two-pass resampling over f32 with precomputed,
edge-folded, zero-padded coefficient tables and a streaming ring buffer
(O(support) memory).

```c
#include "tir.h"
tir_image_view src = {pixels, w, h, 4, TIR_F32, 0};
tir_image_view dst = {out, w/2, h/2, 4, TIR_F32, 0};
tir_options o; tir_options_init(&o);
o.filter_x = o.filter_y = TIR_FILTER_LANCZOS3;
o.antiring  = 1.0f;   /* confine ringing to the local source range */
o.clamp_min = 0.0f;   /* hard guarantee: no negative pixels */
tir_resize(NULL, &src, &dst, &o);
```

Highlights:

| Area | Support |
|---|---|
| **Language** | Pure C11, `-Wall -Wextra -Werror` clean, overflow-checked size arithmetic; own prefix / allocator hook, zero external dependencies |
| **HDR-aware** | Values never clamped by default; negative-lobe ringing confined by `antiring` (blend toward a reduced-footprint min/max, applied **once after both passes**), so `antiring=1` + `clamp_min=0` is a hard no-negative-pixels guarantee; optional OIIO/SPI highlight compression |
| **Content modes** | **Normal maps** (snorm / unorm / NVTT-C127 / 2-channel RG with `z` reconstructed; renormalized once, `|N|` side output for Toksvig); **height/displacement** (mean-preserving area downscale, grid-vertex registration for 2ⁿ+1 terrains); correct **premultiplied-alpha** filtering; NaN/Inf scrub policies (`zero` / 3×3 `repair` / `error`) |
| **Filters / edges** | Box, triangle, B-spline, Gaussian, Mitchell, Catmull-Rom, Lanczos-2/3; clamp / reflect / wrap; f32 / f16 / u8 / u16 pixel I/O |
| **SIMD** | Runtime **SSE2 / SSE4.1 / AVX2 (+FMA, F16C)**, compile-time **NEON**, **SVE 1.0** behind a `HWCAP_SVE` gate; `deterministic=1` pins scalar kernels for bit-identical output across platforms / levels / thread counts |
| **API** | One-shot `tir_resize`, reusable sampler, **streaming** `push_row`/`pull_row` (O(support) memory), and optional C11-threads band parallelism (byte-identical to serial) |

Beats the in-tree `exr_resize_float` by **2–5×** and is competitive with
stb_image_resize2 (wins every 4-channel/HDR case on the Zen2 bench).
Validated against an independent double-precision reference resampler under the
full sanitizer matrix (ASan / UBSan / LeakSanitizer / ThreadSanitizer), an
exhaustive f16↔f32 converter sweep, and a libFuzzer harness.

Build & test:

```sh
make resize-lib          # build/libtir.a
make resize-cli          # build/tir_resize (EXR in/out)
make resize-test         # unit tests (ASan+UBSan) vs a double-precision oracle
make resize-test-tsan    # ThreadSanitizer
make resize-bench        # MP/s vs exr_resize_float (STB=1 adds stb2)
make resize-fuzz-corpus  # deterministic fuzz replay (ASan+UBSan)
```

```sh
./build/tir_resize in.exr -o out.exr --scale 0.5 --filter lanczos3 \
    --antiring 1 --clamp-min 0 --stats
```

See [`tools/resize/`](tools/resize/) for the library and its README.

---

# v1 — single-header C++ (stable)

The original single-header C++ API (`tinyexr.h`) is the **old but stable**
version. It is written in portable C++ (no dependency except STL), so it is easy
to embed into your application. To use it, simply copy `tinyexr.h`, `miniz.c` and
`miniz.h` (for zlib. You can use system-installed zlib instead of miniz, or the
zlib implementation included in `stb_image[_write].h`. Controlled with
`TINYEXR_USE_MINIZ` and `TINYEXR_USE_STB_ZLIB` compile flags) into your project.
The rest of this section documents the v1 API.

## Security

TinyEXR does not use C++ exception.

TinyEXR now does not use `assert` from v1.0.4(2023/06/04), except for miniz's assert.

## Features

Current status of the legacy **v1** `tinyexr.h` (✅ supported · ❌ not yet / not planned):

| Feature | Status |
|---|---|
| Scanline | ✅ load + save |
| Tiled (with / without LoD) | ✅ load + save |
| Custom attributes | ✅ |
| Multipart | ✅ load + save · ❌ deep multipart |
| Deep image | ✅ load (scanline, ZIPS, HALF/FLOAT) · ❌ save |
| Compression | ✅ NONE · RLE · ZIP · ZIPS · PIZ · PXR24 · B44/B44A · ZFP (tinyexr ext) · ❌ DWA (patent-encumbered, not planned) |
| Spectral EXR (JCGT 2021) | ✅ emissive (`S{n}.…nm`) · reflective (`T.…nm`) · polarised (Stokes S0–S3) |
| Line order | ✅ load increasing + decreasing, save increasing · ❌ save decreasing, random |
| Pixel format (UINT / FLOAT) | ✅ load · deep load · save · ❌ deep save |
| Big endian | ✅ scanline + multipart load/save (untested) · ❌ deep |
| Threading | ✅ C++11 thread load, OpenMP load + save · ❌ thread save, OpenMP deep, ISPC |
| C interface | ✅ easy language bindings (e.g. golang) |

## Supported platforms

| Platform | Status |
|---|---|
| x86-64 | ✅ Windows 7+, Linux (posix), macOS |
| AARCH64 | ✅ aarch64 Linux (e.g. Raspberry Pi), Android, iOS, macOS |
| RISC-V | ❌ untested (should work) |
| Big endian (SPARC, PowerPC, …) | ✅ not maintained, but should work |
| WebAssembly (JavaScript) | ✅ loader only — see [experimental/js/](experimental/js/) |
| Python binding | ✅ loader only — https://pypi.org/project/pytinyexr/ |

## Requirements

* C++ compiler(C++11 recommended. C++03 may work)

## Usage

NOTE: **API is still subject to change**. See the source code for details.

Include `tinyexr.h` with `TINYEXR_IMPLEMENTATION` flag (do this only for **one** .cc file).

```cpp
//Please include your own zlib-compatible API header before
//including `tinyexr.h` when you disable `TINYEXR_USE_MINIZ`
//#define TINYEXR_USE_MINIZ 0
//#include "zlib.h"
//Or, if your project uses `stb_image[_write].h`, use their
//zlib implementation:
//#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
```

### Compile flags

* `TINYEXR_USE_MINIZ` Use miniz (default = 1). Please include `zlib.h` header before `tinyexr.h` if you disable miniz support(e.g. use system's zlib).
* `TINYEXR_USE_STB_ZLIB` Use zlib from `stb_image[_write].h` instead of miniz or the system's zlib (default = 0).
* `TINYEXR_USE_PIZ` Enable PIZ compression support (default = 1)
* `TINYEXR_USE_ZFP` Enable ZFP compression support (TinyEXR extension, default = 0)
* `TINYEXR_USE_THREAD` Enable threaded loading/storing using C++11 thread (Requires C++11 compiler, default = 0)
  * Use `TINYEXR_MAX_THREADS` over 0 to use MIN(TINYEXR_MAX_THREADS,hardware_concurrency()) in stead off hardware_concurrency(). (default = 0)
* `TINYEXR_USE_OPENMP` Enable OpenMP threading support (default = 1 if `_OPENMP` is defined)
  * Use `TINYEXR_USE_OPENMP=0` to force disable OpenMP code path even if OpenMP is available/enabled in the compiler.
* `TINYEXR_USE_COMPILER_FP16` Enable use of compiler provided FP16<>FP32 conversions when available (default = 0)

### Quickly reading RGB(A) EXR file.

```cpp
  const char* input = "asakusa.exr";
  float* out; // width * height * RGBA
  int width;
  int height;
  const char* err = NULL; // or nullptr in C++11

  int ret = LoadEXR(&out, &width, &height, input, &err);

  if (ret != TINYEXR_SUCCESS) {
    if (err) {
       fprintf(stderr, "ERR : %s\n", err);
       FreeEXRErrorMessage(err); // release memory of error message.
    }
  } else {
    ...
    free(out); // release memory of image data
  }

```

### Reading layered RGB(A) EXR file.

If you want to read EXR image with layer info (channel has a name with delimiter `.`), please use `LoadEXRWithLayer` API.

You need to know layer name in advance (e.g. through `EXRLayers` API).

```cpp
  const char* input = ...;
  const char* layer_name = "diffuse"; // or use EXRLayers to get list of layer names in .exr
  float* out; // width * height * RGBA
  int width;
  int height;
  const char* err = NULL; // or nullptr in C++11

  // will read `diffuse.R`, `diffuse.G`, `diffuse.B`, (`diffuse.A`) channels
  int ret = LoadEXRWithLayer(&out, &width, &height, input, layer_name, &err);

  if (ret != TINYEXR_SUCCESS) {
    if (err) {
       fprintf(stderr, "ERR : %s\n", err);
       FreeEXRErrorMessage(err); // release memory of error message.
    }
  } else {
    ...
    free(out); // release memory of image data
  }

```

### Loading Singlepart EXR from a file.

Scanline and tiled format are supported.

```cpp
  // 1. Read EXR version.
  EXRVersion exr_version;

  int ret = ParseEXRVersionFromFile(&exr_version, argv[1]);
  if (ret != 0) {
    fprintf(stderr, "Invalid EXR file: %s\n", argv[1]);
    return -1;
  }

  if (exr_version.multipart) {
    // must be multipart flag is false.
    return -1;
  }

  // 2. Read EXR header
  EXRHeader exr_header;
  InitEXRHeader(&exr_header);

  const char* err = NULL; // or `nullptr` in C++11 or later.
  ret = ParseEXRHeaderFromFile(&exr_header, &exr_version, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Parse EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  // // Read HALF channel as FLOAT.
  // for (int i = 0; i < exr_header.num_channels; i++) {
  //   if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
  //     exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
  //   }
  // }

  EXRImage exr_image;
  InitEXRImage(&exr_image);

  ret = LoadEXRImageFromFile(&exr_image, &exr_header, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Load EXR err: %s\n", err);
    FreeEXRHeader(&exr_header);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  // 3. Access image data
  // `exr_image.images` will be filled when EXR is scanline format.
  // `exr_image.tiled` will be filled when EXR is tiled format.

  // 4. Free image data
  FreeEXRImage(&exr_image);
  FreeEXRHeader(&exr_header);
```

### Loading Multipart EXR from a file.

Scanline and tiled format are supported.

```cpp
  // 1. Read EXR version.
  EXRVersion exr_version;

  int ret = ParseEXRVersionFromFile(&exr_version, argv[1]);
  if (ret != 0) {
    fprintf(stderr, "Invalid EXR file: %s\n", argv[1]);
    return -1;
  }

  if (!exr_version.multipart) {
    // must be multipart flag is true.
    return -1;
  }

  // 2. Read EXR headers in the EXR.
  EXRHeader **exr_headers; // list of EXRHeader pointers.
  int num_exr_headers;
  const char *err = NULL; // or nullptr in C++11 or later

  // Memory for EXRHeader is allocated inside of ParseEXRMultipartHeaderFromFile,
  ret = ParseEXRMultipartHeaderFromFile(&exr_headers, &num_exr_headers, &exr_version, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Parse EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  printf("num parts = %d\n", num_exr_headers);


  // 3. Load images.

  // Prepare array of EXRImage.
  std::vector<EXRImage> images(num_exr_headers);
  for (int i =0; i < num_exr_headers; i++) {
    InitEXRImage(&images[i]);
  }

  ret = LoadEXRMultipartImageFromFile(&images.at(0), const_cast<const EXRHeader**>(exr_headers), num_exr_headers, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Parse EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  printf("Loaded %d part images\n", num_exr_headers);

  // 4. Access image data
  // `exr_image.images` will be filled when EXR is scanline format.
  // `exr_image.tiled` will be filled when EXR is tiled format.

  // 5. Free images
  for (int i =0; i < num_exr_headers; i++) {
    FreeEXRImage(&images.at(i));
  }

  // 6. Free headers.
  for (int i =0; i < num_exr_headers; i++) {
    FreeEXRHeader(exr_headers[i]);
    free(exr_headers[i]);
  }
  free(exr_headers);
```


Saving Scanline EXR file.

```cpp
  // See `examples/rgbe2exr/` for more details.
  bool SaveEXR(const float* rgb, int width, int height, const char* outfilename) {

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);

    // Split RGBRGBRGB... into R, G and B layer
    for (int i = 0; i < width * height; i++) {
      images[0][i] = rgb[3*i+0];
      images[1][i] = rgb[3*i+1];
      images[2][i] = rgb[3*i+2];
    }

    float* image_ptr[3];
    image_ptr[0] = &(images[2].at(0)); // B
    image_ptr[1] = &(images[1].at(0)); // G
    image_ptr[2] = &(images[0].at(0)); // R

    image.images = (unsigned char**)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 3;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    const char* err = NULL; // or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
    if (ret != TINYEXR_SUCCESS) {
      fprintf(stderr, "Save EXR err: %s\n", err);
      FreeEXRErrorMessage(err); // free's buffer for an error message
      return ret;
    }
    printf("Saved exr file. [ %s ] \n", outfilename);

    free(rgb);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

  }
```


Reading deep image EXR file.
See `example/deepview` for actual usage.

```cpp
  const char* input = "data/deepscanline.exr";
  const char* err = NULL; // or nullptr
  DeepImage deepImage;

  int ret = LoadDeepEXR(&deepImage, input, &err);

  // access to each sample in the deep pixel.
  for (int y = 0; y < deepImage.height; y++) {
    int sampleNum = deepImage.offset_table[y][deepImage.width-1];
    for (int x = 0; x < deepImage.width-1; x++) {
      int s_start = deepImage.offset_table[y][x];
      int s_end   = deepImage.offset_table[y][x+1];
      if (s_start >= sampleNum) {
        continue;
      }
      s_end = (s_end < sampleNum) ? s_end : sampleNum;
      for (int s = s_start; s < s_end; s++) {
        float val = deepImage.image[depthChan][y][s];
        ...
      }
    }
  }

```

## Examples

* [examples/deepview/](examples/deepview) Deep image view
* [examples/rgbe2exr/](examples/rgbe2exr) .hdr to EXR converter
* [examples/exr2rgbe/](examples/exr2rgbe) EXR to .hdr converter
* [examples/ldr2exr/](examples/exr2rgbe) LDR to EXR converter
* [examples/exr2ldr/](examples/exr2ldr) EXR to LDR converter
* [examples/exr2fptiff/](examples/exr2fptiff) EXR to 32bit floating point TIFF converter
  * for 32bit floating point TIFF to EXR convert, see https://github.com/syoyo/tinydngloader/tree/release/examples/fptiff2exr
* [examples/cube2longlat/](examples/cube2longlat) Cubemap to longlat (equirectangler) converter
* [examples/spectral/](examples/spectral) Spectral EXR read/write example

### deepview

`examples/deepview` is simple deep image viewer in OpenGL. It can be tested with `data/deepscanline.exr`.

![DeepViewExample](https://github.com/syoyo/tinyexr/blob/release/examples/deepview/deepview_screencast.gif?raw=true)

### Experimental

* [experimental/js/](experimental/js) JavaScript port using Emscripten

## TinyEXR extension

### ZFP

#### NOTE

TinyEXR adds ZFP compression as an experimemtal support (Linux and MacOSX only).

ZFP only supports FLOAT format pixel, and its image width and height must be the multiple of 4, since ZFP compresses pixels with 4x4 pixel block.

#### Setup

Checkout zfp repo as an submodule.

    $ git submodule update --init

#### Build

Then build ZFP

    $ cd deps/ZFP
    $ mkdir -p lib   # Create `lib` directory if not exist
    $ make

Set `1` to `TINYEXT_USE_ZFP` define in `tinyexr.h`

Build your app with linking `deps/ZFP/lib/libzfp.a`

#### ZFP attribute

For ZFP EXR image, the following attribute must exist in its EXR image.

* `zfpCompressionType` (uchar).
  * 0 = fixed rate compression
  * 1 = precision based variable rate compression
  * 2 = accuracy based variable rate compression

And the one of following attributes must exist in EXR, depending on the `zfpCompressionType` value.

* `zfpCompressionRate` (double)
  * Specifies compression rate for fixed rate compression.
* `zfpCompressionPrecision` (int32)
  * Specifies the number of bits for precision based variable rate compression.
* `zfpCompressionTolerance` (double)
  * Specifies the tolerance value for accuracy based variable rate compression.

#### Note on ZFP compression.

At least ZFP code itself works well on big endian machine.

### Spectral EXR

TinyEXR supports reading and writing spectral EXR files based on the JCGT 2021 paper:
https://jcgt.org/published/0010/03/01/

Reference implementation: https://github.com/afichet/spectral-exr

#### Spectrum Types

| Type | Channel Format | Description |
|------|----------------|-------------|
| Emissive | `S{stokes}.{wavelength}nm` | Radiance/irradiance spectra (e.g., `S0.550,000000nm`) |
| Reflective | `T.{wavelength}nm` | Transmittance/reflectance spectra (e.g., `T.550,000000nm`) |
| Polarised | `S0-S3.{wavelength}nm` | Stokes vector spectra |

Wavelengths use European decimal convention (comma as separator).

#### Spectral API Functions

```cpp
// Detection
int IsSpectralEXR(const char* filename);
int EXRGetSpectrumType(const EXRHeader* header);  // Returns TINYEXR_SPECTRUM_*

// Channel naming
void EXRSpectralChannelName(char* buffer, size_t size, float wavelength_nm, int stokes);
void EXRReflectiveChannelName(char* buffer, size_t size, float wavelength_nm);
float EXRParseSpectralChannelWavelength(const char* channel_name);
int EXRGetStokesComponent(const char* channel_name);

// Metadata
int EXRSetSpectralAttributes(EXRHeader* header, int spectrum_type, const char* units);
const char* EXRGetSpectralUnits(const EXRHeader* header);
int EXRGetWavelengths(const EXRHeader* header, float* wavelengths, int max);
```

See `examples/spectral/` for a complete read/write example.

## Unit tests

See `test/unit` directory.

## Python bindings

`pytinyexr` is available: https://pypi.org/project/pytinyexr/ (loading only as of 0.9.1)

## TODO (v1)

Contribution is welcome!

- [ ] Compression
  - [ ] B44?
  - [ ] B44A?
  - [ ] PIX24?
- [ ] Custom attributes
  - [x] Normal image (EXR 1.x)
  - [ ] Deep image (EXR 2.x)
- [ ] JavaScript library (experimental, using Emscripten)
  - [x] LoadEXRFromMemory
  - [ ] SaveMultiChannelEXR
  - [ ] Deep image save/load
- [ ] Write from/to memory buffer.
  - [ ] Deep image save/load
- [ ] Tile format.
  - [x] Tile format with no LoD (load).
  - [ ] Tile format with LoD (load).
  - [ ] Tile format with no LoD (save).
  - [ ] Tile format with LoD (save).
- [ ] Support for custom compression type.
  - [x] zfp compression (Not in OpenEXR spec, though)
  - [ ] zstd?
- [x] Multi-channel.
- [ ] Multi-part (EXR2.0)
  - [x] Load multi-part image
  - [ ] Load multi-part deep image
- [ ] Line order.
  - [x] Increasing, decreasing (load)
  - [ ] Random?
  - [ ] Increasing, decreasing (save)
- [ ] Pixel format (UINT, FLOAT).
  - [x] UINT, FLOAT (load)
  - [x] UINT, FLOAT (deep load)
  - [x] UINT, FLOAT (save)
  - [ ] UINT, FLOAT (deep save)
- [ ] Support for big endian machine.
  - [ ] Loading multi-part channel EXR
  - [ ] Saving multi-part channel EXR
  - [ ] Loading deep image
  - [ ] Saving deep image
- [ ] Optimization
  - [ ] ISPC?
  - [x] OpenMP multi-threading in EXR loading.
  - [x] OpenMP multi-threading in EXR saving.
  - [ ] OpenMP multi-threading in deep image loading.
  - [ ] OpenMP multi-threading in deep image saving.

## Third-party licenses (v1)

- **miniz** (bundled zlib) — public domain, by Rich Geldreich <richgel99@gmail.com>.
- **stb** (used by the tools) — public domain, https://github.com/nothings/stb.
- **OpenEXR** — some code is derived from OpenEXR, 3-clause BSD.
- **nanozlib** and **wuffs** — Apache 2.0.

---

# License

3-clause BSD, for both v1 and v3. Per-flavour third-party notices are listed
under [Third-party licenses (v3)](#third-party-licenses-v3) and
[Third-party licenses (v1)](#third-party-licenses-v1).

# Similar or related projects

* miniexr: https://github.com/aras-p/miniexr (Write OpenEXR)
* stb_image_resize.h: https://github.com/nothings/stb (Good for HDR image resizing)

# Author(s)

Syoyo Fujita (syoyo@lighttransport.com)

# Contributor(s)

* Matt Ebb (http://mattebb.com): deep image example. Thanks!
* Matt Pharr (http://pharr.org/matt/): Testing tinyexr with OpenEXR(IlmImf). Thanks!
* Andrew Bell (https://github.com/andrewfb) & Richard Eakin (https://github.com/richardeakin): Improving TinyEXR API. Thanks!
* Mike Wong (https://github.com/mwkm): ZIPS compression support in loading. Thanks!
