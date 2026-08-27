# OpenEXR HTJ2K/JPH Investigation and TinyEXR Implementation Plan

## Scope

This note summarizes OpenEXR's High-Throughput JPEG 2000 support, usually named
HTJ2K or JPH, and outlines an implementation plan for TinyEXR v3 C. The local
reference inspected was `/mnt/nvme02/work/openexr`, especially:

- `src/lib/OpenEXRCore/internal_ht.cpp`
- `src/lib/OpenEXRCore/internal_ht_common.cpp`
- `src/lib/OpenEXRCore/internal_ht_common.h`
- `src/lib/OpenEXRCore/compression.c`
- `cmake/OpenEXRSetup.cmake`
- `external/OpenJPH`

## OpenEXR Feature Summary

OpenEXR supports two HTJ2K compression modes:

- `HTJ2K256_COMPRESSION`, enum value `10`, using 256 scanlines per chunk.
- `HTJ2K32_COMPRESSION`, enum value `11`, using 32 scanlines per chunk.

The file-layout documentation describes both as lossless JPEG 2000 coding using
the High-Throughput blocker. In OpenEXRCore these appear as:

```c
EXR_COMPRESSION_HTJ2K256 = 10
EXR_COMPRESSION_HTJ2K32  = 11
```

Compression chunk height is selected in `exr_compression_lines_per_chunk()`:

- `HTJ2K32`: 32 scanlines.
- `HTJ2K256`: 256 scanlines.

The generic compression dispatcher routes both modes to:

- encode: `internal_exr_apply_ht()`
- decode: `internal_exr_undo_ht()`

The actual JPEG 2000 codec is not implemented inside OpenEXR. OpenEXR uses
OpenJPH, a separate BSD-2-Clause C++ HTJ2K implementation.

## OpenEXR Chunk Payload Format

OpenEXR does not store a bare OpenJPH codestream directly. It prepends a small
OpenEXR-specific HTJ2K header, then stores the JPEG 2000 codestream.

All integer fields are big-endian:

```text
uint16_t magic = 0x4854  // 'H', 'T'
uint32_t payload_length
uint16_t channel_count
uint16_t cs_to_file_channel[channel_count]
optional opaque extension bytes up to payload_length
uint8_t  jpeg2000_codestream[]
```

The channel map translates JPEG 2000 component index to OpenEXR file channel
index. This is required because the encoder may reorder RGB channels so OpenJPH
can apply JPEG 2000's reversible color transform.

OpenEXR's RGB detection is heuristic:

- It looks for `R/G/B` or `Red/Green/Blue`, case-insensitive.
- It also accepts matching suffixes after a layer prefix, for example
  `main.R`, `main.G`, `main.B`.
- The RGB triplet must have matching pixel type and sampling.
- If an RGB triplet is found, the first codestream components are ordered
  `R`, `G`, `B`; remaining channels follow in file order.
- If no RGB triplet is found, codestream component order matches file channel
  order.

## OpenJPH Usage in OpenEXR

Encoding flow in `internal_ht.cpp`:

1. Build the codestream-to-file channel map.
2. Configure `ojph::codestream`.
3. Set component count, component sampling, bit depth, signedness, and nonlinear
   transform.
4. Set image offset `(0, 0)` and image extent to the chunk width/height.
5. Set coding parameters:
   - reversible transform enabled
   - codeblock dimensions `128 x 32`
   - decomposition count `5`
   - color transform enabled only for non-planar RGB chunks
6. Write the OpenEXR HT header.
7. Write OpenJPH codestream headers.
8. Push scanlines component-by-component into OpenJPH.
9. Flush the codestream.

Decoding flow:

1. Parse and validate the OpenEXR HT header.
2. Validate the channel map size and channel indices.
3. Open the remaining chunk bytes as an OpenJPH memory input file.
4. Read JPEG 2000 headers.
5. Validate decoded image width, height, component count, component dimensions,
   and subsampling against the EXR chunk/channel metadata.
6. Select planar mode if any channel has `x_samples > 1` or `y_samples > 1`.
7. Pull decoded component lines from OpenJPH.
8. Scatter component data back into TinyEXR/OpenEXR packed channel order.

OpenEXR has recent security fixes in this area. Its current decoder performs
important validation for codestream/channel width mismatches and integer
overflow before writing decoded pixels. TinyEXR should keep these checks from
the start.

## Dependency and License Notes

OpenEXR CMake finds OpenJPH >= 0.21.0 through CMake package config or
pkg-config. If not found, it uses the vendored `external/OpenJPH` copy. The
vendored OpenJPH is configured as a static dependency with executables disabled.

Licenses:

- OpenEXR HT glue files are BSD-3-Clause, copyright OpenEXR contributors.
- OpenJPH is BSD-2-Clause, with copyright notices from Aous Naman, Kakadu
  Software Pty Ltd, and the University of New South Wales.
- No GPL dependency is needed.

The OpenJPH license is permissive and compatible with TinyEXR's permissive
licensing goals, but copied source files must retain their original copyright
and license text.

## Feasibility of a zstd-style Single `.h`/`.c`

A true zstd-style extraction is not straightforward.

The zstd integration was practical because zstd is already C and can be reduced
to a small C-facing subset. OpenJPH is different:

- It is a full C++ library.
- The core is spread across codestream parsing, packet/tile/precinct handling,
  HT block coding, wavelet transforms, color transforms, memory/file adapters,
  and CPU-specific optimized paths.
- Public use goes through C++ classes such as `ojph::codestream`,
  `ojph::mem_infile`, and `ojph::outfile_base`.
- The codec is algorithmically much larger than the OpenEXR glue.

Therefore a minimal single `tinyexr_jph.h` + `tinyexr_jph.c` C11 codec is a
medium-to-large port, not a simple file extraction. It is possible, but it
should be treated as a separate codec-porting project with its own fuzzing and
conformance work.

The pragmatic first implementation should use OpenJPH through a tiny adapter.
After behavior is correct and tested, a pure C11 port can be considered if the
project still requires a no-C++ dependency.

## Recommended TinyEXR Plan

### Phase 1: OpenEXR-compatible HTJ2K container support

Add TinyEXR constants for OpenEXR-compatible enum values:

```c
EXR_COMPRESSION_HTJ2K256 = 10
EXR_COMPRESSION_HTJ2K32  = 11
```

Add line-per-chunk handling:

- `HTJ2K256`: 256 lines
- `HTJ2K32`: 32 lines

Add parser/writer code for the small OpenEXR HT chunk header:

- Validate magic `0x4854`.
- Read/write big-endian payload length.
- Read/write `channel_count`.
- Read/write `cs_to_file_channel[]`.
- Reject oversized, truncated, or inconsistent headers.
- Skip unknown extension bytes within `payload_length`.

This part can be plain C11 and small.

### Phase 2: Minimal OpenJPH adapter

Use a small TinyEXR adapter layer to isolate the C++ dependency:

- `src/exr_jph.c` or `src/exr_htj2k.c`
  - C11 TinyEXR codec dispatch, channel-map header handling, validation, and
    pack/unpack logic.
- `deps/openjph/tinyexr_openjph_adapter.h`
  - C-callable adapter API.
- `deps/openjph/tinyexr_openjph_adapter.cpp`
  - OpenJPH C++ calls hidden behind `extern "C"`.

The adapter API should avoid exposing C++ types:

```c
int tinyexr_jph_encode(
    const struct tinyexr_jph_encode_desc *desc,
    const void *packed_pixels,
    size_t packed_size,
    void *compressed,
    size_t compressed_capacity,
    size_t *compressed_size);

int tinyexr_jph_decode(
    const struct tinyexr_jph_decode_desc *desc,
    const void *compressed,
    size_t compressed_size,
    void *packed_pixels,
    size_t packed_capacity);
```

This keeps TinyEXR's public API C-compatible while allowing the first backend
to use upstream OpenJPH.

Build options:

- `TINYEXR_ENABLE_HTJ2K=OFF` by default initially.
- `TINYEXR_USE_SYSTEM_OPENJPH=ON/OFF`.
- If vendored, keep OpenJPH as a private static dependency and disable command
  line tools.

### Phase 3: Validation and compatibility tests

Add tests with OpenEXR-generated HTJ2K files:

- `HTJ2K32` and `HTJ2K256`.
- HALF, FLOAT, and UINT channels.
- RGB and non-RGB channel names.
- Layer-prefixed RGB names, for example `beauty.R/G/B`.
- More than three channels.
- Subsampled channels.
- Small images smaller than the nominal chunk height.
- Corrupt chunks:
  - bad magic
  - truncated header
  - payload length larger than chunk
  - channel map count mismatch
  - out-of-range channel index
  - codestream width/height mismatch
  - integer overflow-sized metadata

Round-trip tests should compare:

- TinyEXR decode of OpenEXR-encoded HTJ2K.
- OpenEXR decode of TinyEXR-encoded HTJ2K.
- Lossless byte/value equality for supported pixel types.

Fuzz the decoder entry point because JPEG 2000 codestream parsing is complex and
OpenEXR has already had HTJ2K-related bug fixes.

### Phase 4: Optional pure C11 codec port

Only start this if avoiding C++ is a hard requirement after the adapter backend
works.

A realistic C11 port would need to extract and rewrite these OpenJPH areas:

- Codestream marker parsing and writing.
- SIZ/COD/NLT parameter handling used by OpenEXR.
- Tile, precinct, resolution, subband, and codeblock state.
- HT block encoder/decoder.
- Reversible 5/3 transform.
- Reversible color transform.
- Memory input/output abstraction.
- Scalar C fallback paths first; SIMD can wait.

Suggested constraints for a first C11 port:

- Lossless only.
- Reversible transform only.
- Single tile matching OpenEXR chunk dimensions.
- Codeblock `128 x 32`.
- Decomposition count `5`.
- No progression/order tuning beyond OpenEXR-compatible defaults.
- Decode first, then encode.

Expected output shape if this phase succeeds:

- `deps/jph/tinyexr_jph.h`
- `deps/jph/tinyexr_jph.c`
- `deps/jph/LICENSE.OpenJPH`

This port must preserve OpenJPH BSD-2-Clause copyright/license notices for any
derived code and should document which OpenJPH revision was used.

## Suggested Near-term Direction

Implement the OpenEXR-compatible HT header and TinyEXR codec dispatch in C11,
then bind to OpenJPH behind a private C adapter. This is the shortest path to
correct OpenEXR interoperability.

A single `.h/.c` pure C11 HTJ2K codec is possible in principle, but it is not a
small extraction like zstd. It should be planned as a second-stage port after
OpenJPH-backed behavior, compatibility tests, and fuzz targets are in place.
