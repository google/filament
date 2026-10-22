# texpipe — resize-aware texture compression

`texpipe` ties together the two standalone libraries in this repo — **`tir`**
(content-aware image resize) and **`texcomp`** (BC/ETC/ASTC block compression) —
to build content-aware mip chains and serialize them into multi-mip GPU texture
containers (DDS, KTX2). It is pure C11; only `texpipe_cli.c` does file I/O.

## Why

`tir` can resize with content awareness (premultiplied alpha, normal/height
modes) but has no mip-chain generator. `texcomp` has every block encoder but only
writes single-surface, single-mip files. Neither knows about the other. `texpipe`
is the missing layer: *resize → per-mip content passes → compress → container*.

## Status (Phases 0–3 — all four capabilities)

Implemented:

- **Mip-chain generation**, resample **from base** by default (one filtering
  error per level, no accumulated blur; `--mip-source previous` for the classic
  half-step chain).
- **Alpha-aware RGBA** resize+compress: `tir` premultiplied alpha feeding
  BC7/BC3/ASTC alpha-capable encoders (capability #4).
- **Alpha-coverage preservation** across LODs (Castaño): rescales each mip's
  alpha so its alpha-tested coverage matches the base level, so cutouts/foliage
  don't thin out with distance (`--content alpha`, capability #2).
- **Seam-free cubemap LOD** (capability #1): 6-face input (separate files or
  cross/strip layouts), per-mip AMD-CubeMapGen-style edge + corner fixup so
  adjacent-face borders are bit-identical at every level, written to cubemap
  DDS/KTX2 (`DDSCAPS2_CUBEMAP` / `faceCount = 6`). See the caveat below.
- **Normal / height LOD coherence** (capability #3): `--content normal` filters
  and renormalizes normals per mip (stays unit-length across LODs, verified to
  ~1e-7), `--content height` is mean-preserving. `--bake-roughness` captures the
  pre-renormalize |N| and maps it to a **Toksvig** roughness (Toksvig 2005),
  written as a companion EAC_R11 KTX2 mip chain so specular highlights don't
  alias into shimmer at distance.
- **Octahedral env maps** (`--octa`): fold-seam-aware mips for a 2D octahedral
  map (the square's outer border folds onto itself), so an HDR octahedral
  environment stays coherent across LODs under BC6H/ASTC-HDR. Pairs with the
  `envmap` tool's `convert --to octa`.
- **sRGB-aware albedo resize** (`--srgb-resize`): decode sRGB→linear, filter,
  re-encode, so albedo mips preserve linear-light energy (a black/white checker
  averages to linear 0.5, not the darkened 0.21 of naive sRGB-space filtering).
- **Packed material maps** (`--channel-ops l,l,m,l`): per-channel downsample
  rules so a single ORM/mask texture minifies correctly — `m` (majority)
  thresholds a binary metallic/mask channel (stays crisp), `r` (roughness)
  replaces the channel with its RMS `sqrt(E[c²])` so minified roughness rises to
  reduce specular aliasing, and `l` (default) stays linear.
- **Texture arrays** (`tp_write_ktx2_array`): pack N compressed chains into one
  KTX2 with `layerCount = N`.
- **Min-max height pyramid** (`--minmax`, `tp_build_minmax_pyramid`): a 2-channel
  (min,max) conservative height pyramid for parallax-occlusion / relief /
  cone-step mapping, stored as BC5. Bounds nest across levels and the coarsest
  level bounds the whole field.
- **Gutter dilation** (`--dilate N`, `tp_dilate`): flood valid texels into the
  alpha<0.5 gutter so mips/bilinear don't bleed background across atlas or
  lightmap chart borders (alpha preserved).
- **Vector displacement**: 3-channel float via the BC6H/ASTC-HDR path resizes
  mean-preserving (averaging keeps the mean displacement — correct, unlike
  normals which renormalize).
- **Cone-step ratio map** (`tp_build_cone_map`): per-texel conservative cone
  ratio for cone-step/relief mapping (O(n⁴) — modest resolutions only).
- **Ripmap** (`tp_build_ripmap`): anisotropic grid of resizes (w>>ix, h>>jy) for
  grazing-angle sampling without runtime anisotropy.
- **Kaiser filter** (`--filter kaiser`): Kaiser-windowed sinc (radius 3, β=8) —
  sharp with lower ringing than Lanczos3 (added to `tir`).
- **YCoCg decorrelation** (`--ycocg`, `tp_rgb_to_ycocg`): store colour as YCoCg
  before compression (shader inverts). Helps low-bit-depth codecs (BC1); roughly
  neutral for BC7, which already rotates colour internally.
- **Error-weighted BC7** (`--bc7-weights R,G,B,A`, `tc_bc7_options.channel_weights`):
  per-channel weights in the BC7 encode error metric — byte-identical to
  unweighted when uniform, so no regression. (ASTC error-weighting is available
  via the astcenc backend: `texcomp --encoder arm --channel-weights R,G,B,A`.)
- **Multi-mip DDS** (DX10) for the BC family and **multi-mip KTX2** (Vulkan
  formats, native cube/mip/array) for BC/ETC2/EAC/ASTC.
- Codecs: BC1/BC3/BC5/BC7/BC6H, ETC2 RGB/RGBA, EAC R11/RG11, ASTC LDR (any
  block), ASTC HDR 4x4.

### Cubemap seam caveat (honest limit)

The edge/corner fixup makes adjacent-face borders **bit-identical before
compression** (the unit test measures max border deviation ≈ 0.006 on a
direction field, and the fixup is idempotent). Block codecs (BC/ETC/ASTC)
compress each face independently, so a residual seam of up to **one quantization
step** can reappear after encoding even with identical float borders. Nothing in
a standard independent-block codec removes this fully; rely on hardware seamless
cube filtering at runtime, or prefer higher-bit-depth codecs (BC7/ASTC) on cube
borders. `--no-seam-fixup` disables the pass.

## Build

```sh
make texpipe           # build/libtexpipe.a + build/texpipe/texpipe CLI
make texpipe-c11-gate  # strict pure-C11 gate (no <stdio.h> outside the CLI)
make texpipe-test      # unit tests (per-mip PSNR, alpha coverage, containers)
```

## CLI

```sh
texpipe -i in.{exr,png} -o out.{ktx2,dds} --format bc7 [opts]
```

Key options: `--format`, `--content color|alpha|normal|height`,
`--container dds|ktx2`, `--filter`, `--edge clamp|wrap|reflect`, `--levels N`,
`--mip-source base|previous`, `--srgb`, `--alpha`, `--alpha-threshold`,
`--astc-block WxH`, `--threads`, `--part` (see `texpipe --help`).

HDR codecs (`bc6h`, `astc_hdr`) require an EXR input; LDR codecs take EXR or PNG.

Cubemaps — give all six faces (order `+x -x +y -y +z -z`) or split one packed
image:

```sh
# 6 separate square faces
texpipe --cube-face +x px.png --cube-face -x nx.png ... -o cube.ktx2 --format bc7
# one cross/strip image
texpipe -i strip.png --cube-layout strip_h -o cube.ktx2 --format astc --astc-block 6x6
```

Normal maps with a baked Toksvig roughness companion:

```sh
texpipe -i normal.png -o normal.ktx2 --format bc5 --content normal \
        --normal-enc unorm --bake-roughness --base-roughness 0.15
# also writes normal.ktx2.rough.ktx2 (EAC_R11 roughness mip chain)
texpipe -i height.png -o height.ktx2 --format bc7 --content height   # mean-preserving
```

## API

The staged C API (see `include/texpipe.h`) is:

```c
tp_build_mips()      base image  -> content-aware float mip chain
tp_compress_chain()  float chain -> compressed block payloads
tp_write_container() blocks      -> DDS / KTX2 bytes
tp_process()         one-shot of the three above
```

plus leaf helpers: `tp_alpha_coverage()` / `tp_alpha_scale_to_coverage()`
(coverage), `tp_cube_seam_fixup()` / `tp_cube_split()` (cubemap),
`tp_toksvig_roughness()` / `tp_build_roughness_chain()` (normal roughness).

## License

Apache-2.0 (matches `texcomp`). Depends on `tir` (BSD-3-Clause) and `texcomp`
(Apache-2.0); both permissive and compatible for combined works.

## Third-party notices

texpipe's library (`libtexpipe`) is original work with no third-party runtime
dependencies. The pieces below are either bundled code used by the CLI only, or
published techniques whose provenance is credited here and in the file headers.

- **stb_image** — public domain / MIT, by Sean Barrett. Bundled at
  `examples/common/stb_image.h` and compiled into `texpipe_cli.c` (LDR PNG
  loading). Not used by the library.
- **AMD CubeMapGen edge fixup** — the seam-free cubemap LOD in
  `src/texpipe_cube.c` follows the edge/corner averaging technique popularized by
  AMD CubeMapGen (technique, independently implemented from the standard cube
  face convention; no code copied).
- **Toksvig roughness** — the normal-length → roughness mapping in
  `src/texpipe_normal.c` implements Toksvig, *"Mipmapping Normal Maps"*, Journal
  of Graphics Tools, 2005.
- **Octahedral maps** — the fold-seam fixup in `src/texpipe_octa.c` uses the
  standard octahedral unit-vector parameterization (see the envmap notices for
  the survey reference).
- **Dependencies:** `tir` (BSD-3-Clause), `texcomp` (Apache-2.0, see its
  `NOTICE.md` for BC/ETC/ASTC third-party credits), TinyEXR core (BSD-3-Clause).
