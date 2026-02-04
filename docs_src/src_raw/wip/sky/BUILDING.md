# Building Skybox Assets

This directory contains the source code for the Simulated Skybox sample. The assets (material and textures) are pre-built in the `assets/` directory.

If you need to modify the material or regenerate the moon textures, you can use the scripts provided in the `tools/` directory.

## Prerequisites

- **Filament**: You must have a built version of Filament. The scripts assume a standard CMake build output structure (e.g., `out/cmake-release`).
- **Python 3**: Required for texture generation.
- **Python Dependencies**: `numpy`, `Pillow` (automatically installed if missing, via pip).

## Rebuilding the Material

If you modify `simulated_skybox.mat`, you must recompile it into a `.filamat` file.

```bash
# Run from the sample root or tools directory
./tools/build_material.sh
```

This will update `assets/simulated_skybox.filamat`.

## Regenerating Moon Textures

If you want to change the moon's resolution, bump scale, or blur, you can regenerate the textures.

```bash
# Run from the sample root or tools directory
./tools/generate_moon_assets.sh [size]
```

Example:
```bash
./tools/generate_moon_assets.sh 512
```

This will download raw NASA data (if not present) and generate:
- `assets/moon_disk.png`
- `assets/moon_normal.png`
