# envmap — environment-map projections, SH & spherical gaussians

Pure-C11 tool for image-based-lighting representations. Converts between
environment-map projections and fits compact lighting bases (spherical
harmonics, spherical gaussians). Links `tir + texcomp + texpipe + libtinyexr3`;
only the CLI does HDR EXR I/O.

## What it does

- **Projection conversion** (`convert`) — resample between **equirectangular**
  (Y-up), **cube** (KTX/D3D/GL face order `+X,-X,+Y,-Y,+Z,-Z`, matching
  `texpipe`'s cube convention), and **octahedral** (Y-up pole). Cube output
  writes 6 face EXRs that `texpipe --cube-face` consumes directly; octa output
  feeds `texpipe --octa`.
- **Spherical harmonics** (`sh`) — project an env onto real SH up to order 4
  ((L+1)² RGB coeffs), with an optional Hanning window to curb ringing. Order 2
  (9 coeffs) is the classic diffuse-irradiance ambient term.
- **Spherical gaussians** (`sg`) — fit K SG lobes (Fibonacci axes, shared
  sharpness ≈ 0.35·K, non-negative least-squares amplitudes) — a compact
  all-frequency-ish lighting representation.
- **Image-based lighting** (`ibl` / `irradiance` / `brdflut`) — GGX-prefiltered
  specular radiance as a roughness-indexed **BC6H cube KTX2**, cosine-convolved
  diffuse **irradiance** cube, and the split-sum **BRDF DFG LUT** (R=scale,
  G=bias). Together these are the split-sum inputs a PBR shader samples.
- **Reference PBR renderer** (`shade`) — builds the float IBL from an env and
  renders a lit sphere (split-sum diffuse + specular) to an HDR EXR. Doubles as
  the ground truth for the validation harness (`make envmap-pbr-test`), which
  shades under the IBL with **source vs BC7-compressed material** and reports the
  shaded-image PSNR + normal angular error — the "does compression hurt the final
  image" gate. It also demonstrates **PBR-tuned format choice**: BC5 (2-channel,
  reconstruct-Z) vs BC7 for normals — BC5 mean 0.35°/max 0.99° vs BC7 1.39°/8.00°.

`sh`/`sg` also write `<out>_recon.exr`, an equirect reconstruction from the
coefficients for eyeballing quality.

## Build

```sh
make envmap            # build/envmap/envmap
make envmap-c11-gate   # strict pure-C11 (no <stdio.h> outside the CLI)
make envmap-test       # projection round-trips, solid-angle, SH/SG
```

## CLI

```sh
# equirect HDR -> 6 cube face EXRs (feed texpipe --cube-face)
envmap convert -i env.exr --from equirect --to cube --size 256 -o cube.exr
# equirect -> octahedral (feed texpipe --octa)
envmap convert -i env.exr --to octa --size 512 -o octa.exr

# order-2 SH (9 coeffs) + reconstruction
envmap sh -i env.exr --order 2 -o env.sh
# 24-lobe spherical gaussians + reconstruction
envmap sg -i env.exr --lobes 24 -o env.sg

# split-sum IBL inputs
envmap ibl -i env.exr --face 128 --samples 64 -o spec.ktx2    # BC6H roughness cube
envmap irradiance -i env.exr --face 32 -o irr.ktx2            # BC6H diffuse cube
envmap brdflut --size 256 -o brdf.exr                        # DFG LUT (R=scale G=bias)
envmap shade -i env.exr --albedo 0.9,0.3,0.2 --roughness 0.25 --metallic 0 -o sphere.exr
```

## Pipeline into texpipe (HDR mip + compress)

```sh
envmap convert -i env.exr --to octa --size 512 -o octa.exr
texpipe -i octa.exr -o octa.ktx2 --format bc6h --octa --container ktx2
```

## Library API (`include/envmap.h`, prefix `em_`)

`em_convert`, `em_dir_to_uv`/`em_uv_to_dir`, `em_texel_solid_angle`,
`em_sample`, `em_foreach_texel`; `em_sh_project`/`em_sh_eval`/`em_sh_window`;
`em_sg_fit`/`em_sg_eval`; plus `em_hammersley` and hemisphere/sphere sampling.

## Verification (from the tests)

Direction round-trips < 1e-6 (all 3 projections); per-texel solid angles sum to
4π within 0.03%; equirect→cube→equirect resample ≈ 87 dB PSNR; SH white-furnace
(constant env → DC only) and SH-4 smooth-field RMS ≈ 4e-4; SG constant-energy
preserved with bounded ripple.

## Notes / deferred

- SH/SG use the equirect solid-angle weights (projection-agnostic; the input is
  treated on the sphere). Convention: SH polar axis = +Y, self-consistent
  project/eval (not matched to an external SH library's axis).
- `--asg` (anisotropic SG) is reserved; the fit is isotropic today.
- Prefiltered specular IBL (GGX) and diffuse-cube irradiance were out of scope
  for this effort (see the plan catalog).

## License

Apache-2.0. Depends on `tir` (BSD-3), `texcomp`/`texpipe` (Apache-2.0),
TinyEXR core (BSD-3).

## Third-party notices

envmap is original work and ships **no third-party code** — the library is pure
C11 and the CLI reads/writes HDR images through the TinyEXR core (`exr.h`) only.
It does implement several published algorithms; credit is due to their authors
(techniques/math, independently implemented — no code copied):

- **Split-sum image-based lighting** (`src/envmap_ibl.c`): Karis, *"Real Shading
  in Unreal Engine 4"*, SIGGRAPH 2013 course notes — GGX importance sampling,
  the prefiltered environment map, and the environment BRDF LUT.
- **Octahedral unit-vector mapping** (`src/envmap_proj.c`): Cigolle, Donow,
  Evangelakos, Mara, McGuire & Meyer, *"A Survey of Efficient Representations
  for Independent Unit Vectors"*, JCGT 2014 (building on Meyer et al., 2010).
- **Hammersley / van der Corput low-discrepancy sequence** (`src/envmap_sample.c`):
  the standard radical-inverse construction.
- **Real spherical harmonics** (`src/envmap_sh.c`): Ramamoorthi & Hanrahan,
  *"An Efficient Representation for Irradiance Environment Maps"*, SIGGRAPH 2001;
  Sloan, *"Stupid Spherical Harmonics (SH) Tricks"*, GDC 2008.
- **Spherical Gaussians** (`src/envmap_sg.c`): Wang, Ren, Gong, Snyder & Guo,
  *"All-Frequency Rendering of Dynamic, Spatially-Varying Reflectance"*,
  SIGGRAPH Asia 2009 (SG lighting representation); Green, *"Spherical Harmonic
  Lighting"*, GDC 2003.
- **Dependencies:** `tir` (BSD-3-Clause), `texcomp`/`texpipe` (Apache-2.0),
  TinyEXR core (BSD-3-Clause).
