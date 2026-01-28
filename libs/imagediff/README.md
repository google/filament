# ImageDiff Library

`imagediff` is a flexible C++ library for comparing images with configurable tolerance logic. It supports both floating-point (`LinearImage`) and 8-bit packed (`Bitmap`) image formats, designed for verifying rendering results against golden images.

## Features

- **Flexible Comparison Logic**: Combine checks using `AND`, `OR`, and `LEAF` nodes.
- **Configurable Thresholds**: Support for absolute (`maxAbsDiff`) error tolerances.
- **Masking**: Optional per-pixel masking to ignore or weight specific regions. The library tracks pixels that passed specifically because of masking (`maskedIgnoredPixelCount`).
- **Global Failure Tolerance**: Allow a certain fraction of pixels (`maxFailingPixelsFraction`) to fail before rejecting the image.
- **8-bit Support**: Efficient comparison of raw 8-bit RGBA buffers with configurable swizzle (e.g., RGBA, BGRA).
- **Visualization**: Generate a `diffImage` (unmasked absolute difference) and a `maskImage` (copy of the applied mask) for debugging.

## Usage

### C++ API

```cpp
#include <imagediff/ImageDiff.h>

// 1. Setup Configuration
imagediff::ImageDiffConfig config;
config.maxAbsDiff = 0.05f; // Allow 5% absolute difference
config.swizzle = imagediff::ImageDiffConfig::Swizzle::RGBA; // Default

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
    // Inspect result.diffImage and result.maskImage.
}
```

### JSON Configuration

```json
{
    "mode": "OR",
    "swizzle": "RGBA",
    "children": [
        {
            "maxAbsDiff": "0.01" // Strict pass
        },
        {
            "maxFailingPixelsFraction": "0.05", // OR allow 5% failures
            "maxAbsDiff": "0.1"
        }
    ]
}
```
