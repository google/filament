# ImageDiff Library

`imagediff` is a flexible C++ library for comparing images with configurable tolerance logic. It
supports both floating-point (`LinearImage`) and 8-bit packed (`Bitmap`) image formats, designed for
verifying rendering results against golden images.

## Features

- **Flexible Comparison Logic**: Combine checks using `AND`, `OR`, and `LEAF` nodes.
- **Configurable Thresholds**: Support for absolute (`maxAbsDiff`) error tolerances.
- **Robustness Options**: Account for GPU rendering variations using positional tolerance
  (`shiftRadius`) and local area averaging (`blurRadius`) to handle sub-pixel shifts and dithering
  artifacts without complex distribution modeling.
- **Detailed Failure Metrics**: When a comparison fails, the result includes an `averageError` array
  and a 10-bin `errorHistogram` to help diagnose whether the failure was a subtle precision shift or
  a massive geometry error.
- **Masking**: Optional per-pixel masking to ignore or weight specific regions. The library tracks
  pixels that passed specifically because of masking (`maskedIgnoredPixelCount`).
- **Global Failure Tolerance**: Allow a certain fraction of pixels (`maxFailingPixelsFraction`) to
  fail before rejecting the image.
- **8-bit Support**: Efficient comparison of raw 8-bit RGBA buffers with configurable swizzle (e.g.,
  RGBA, BGRA).
- **Visualization**: Generate a `diffImage` (unmasked absolute difference) and a `maskImage` (copy
  of the applied mask) for debugging.

## Usage

### C++ API

```cpp
#include <imagediff/ImageDiff.h>

// 1. Setup Configuration
imagediff::ImageDiffConfig config;
config.maxAbsDiff = 0.05f; // Allow 5% absolute difference
config.swizzle = imagediff::ImageDiffConfig::Swizzle::RGBA; // Default

// Enable GPU rendering robustness:
config.shiftRadius = 1; // Allows a 1-pixel shift (absorbs rasterization/MSAA differences)
config.blurRadius = 0;  // Set >0 to compare local averages (absorbs dithering)

// 2. Prepare Images
imagediff::Bitmap ref = {width, height, stride, refData};
imagediff::Bitmap cand = {width, height, stride, candData};

// Optional: Mask (single channel uint8_t for Bitmap)
imagediff::Bitmap mask = {width, height, maskStride, maskData};

// 3. Compare (enable visualization)
imagediff::ImageDiffResult result = imagediff::compare(ref, cand, config, &mask, true);

if (result.status == imagediff::ImageDiffResult::Status::PASSED) {
    // Success. Check result.maskedIgnoredPixelCount for info.
} else {
    // Handle failure (SIZE_MISMATCH or PIXEL_DIFFERENCE)
    // Inspect result.errorHistogram, result.averageError, diffImage, and maskImage.
}
```

### JSON Configuration

```json
{
    "mode": "OR",
    "swizzle": "RGBA",
    "children": [
        {
            "description": "Strict check for exact matches (fastest)",
            "maxAbsDiff": "0.01",
            "shiftRadius": 0,
            "blurRadius": 0
        },
        {
            "description": "Positional check: Allow 1-pixel edge shift but require similar color",
            "maxAbsDiff": "0.05",
            "shiftRadius": 1,
            "blurRadius": 0
        },
        {
            "description": "Distribution check: Compare local 5x5 averages to allow dithering",
            "maxAbsDiff": "0.02",
            "shiftRadius": 0,
            "blurRadius": 2,
            "maxFailingPixelsFraction": "0.005"
        }
    ]
}
```