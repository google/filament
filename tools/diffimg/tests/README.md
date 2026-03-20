# DiffImg Python Tests

This directory contains a suite of synthetic image tests to validate the `diffimg` tool's robustness configurations (`shiftRadius` and `blurRadius`).

## Files
- `gen_images.py`: A Python script that generates synthetic `PPM` images.
  - `ref.ppm`: A reference image of a white circle.
  - `cand_shift.ppm`: The same circle shifted by 1 pixel horizontally.
  - `cand_blur.ppm`: The reference circle with high-frequency dithering noise added.
- `config_strict.json`: An exact-match configuration (`maxAbsDiff = 0.01`).
- `config_shift.json`: A configuration with a 1-pixel shift tolerance (`shiftRadius = 1`).
- `config_blur.json`: A configuration with a local area average check (`blurRadius = 1`).

## How to Run

Because `diffimg` depends on `libs/imageio`, which may use varying decoders based on OS capabilities, it is recommended to test with `PNG` files.

**Prerequisites:**
- Python 3
- `sips` (macOS native) or `ImageMagick` (for Linux/Windows) to convert PPM to PNG.
- A compiled `diffimg` binary.

**Steps (macOS Example):**

1. Navigate to this directory:
   ```bash
   cd tools/diffimg/tests/
   ```

2. Generate the PPM images:
   ```bash
   python3 gen_images.py
   ```

3. Convert the generated PPM images to PNG (diffimg natively handles PNG cross-platform without needing LibTIFF or specific backends):
   ```bash
   sips -s format png ref.ppm --out ref.png
   sips -s format png cand_shift.ppm --out cand_shift.png
   sips -s format png cand_blur.ppm --out cand_blur.png
   ```

4. Run the validation checks using the compiled binary (assuming it's built in `out/cmake-release` at the project root):
   ```bash
   # Test spatial shift (should FAIL with strict, PASS with shift)
   ../../../out/cmake-release/tools/diffimg/diffimg -c config_strict.json ref.png cand_shift.png
   ../../../out/cmake-release/tools/diffimg/diffimg -c config_shift.json ref.png cand_shift.png

   # Test high-frequency noise (should FAIL with strict, PASS with blur)
   ../../../out/cmake-release/tools/diffimg/diffimg -c config_strict.json ref.png cand_blur.png
   ../../../out/cmake-release/tools/diffimg/diffimg -c config_blur.json ref.png cand_blur.png
   ```