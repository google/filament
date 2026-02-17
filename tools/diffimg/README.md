# diffimg

`diffimg` is a command-line tool for comparing two images using the `imagediff` library's tolerance logic. It supports various image formats and allows for fine-grained control over comparison thresholds via JSON configuration and optional masking.

## Usage

```bash
diffimg [options] <reference> <candidate>
```

### Arguments
- `<reference>`: Path to the golden/expected image.
- `<candidate>`: Path to the test/actual image.

### Options
- `--config <path>`: Path to a JSON configuration file defining comparison thresholds.
- `--mask <path>`: Path to a grayscale mask image (0 = ignore, >0 = compare).
- `--diff <path>`: Path to output a visual difference image (unmasked absolute difference).
- `--help, -h`: Print help message.

## Output

The tool outputs a JSON object to `stdout` containing the comparison results:

```json
{
  "status": 0,
  "passed": true,
  "failingPixelCount": 0,
  "maxDiffFound": [0.0, 0.0, 0.0, 0.0]
}
```

### Status Codes
- `0`: PASSED (Images are considered the same within tolerance)
- `1`: SIZE_MISMATCH (Images have different dimensions)
- `2`: PIXEL_DIFFERENCE (Images differ beyond the allowed tolerance)

## Configuration Format

The optional JSON configuration follows the `imagediff` schema:

```json
{
    "mode": "LEAF",
    "maxAbsDiff": 0.01,
    "maxFailingPixelsFraction": 0.05
}
```

## Dependencies
- `libs/imagediff`: Comparison logic.
- `libs/imageio`: Primary image decoding (PNG, HDR, EXR).
- `libs/imageio-lite`: Fallback decoding (TIFF).
