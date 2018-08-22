/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IMAGE_IMAGESAMPLER_H
#define IMAGE_IMAGESAMPLER_H

#include <image/LinearImage.h>

namespace image {

/**
 * Value of a single point sample, allocated according to the number of image channels.
 */
struct SingleSample {
    float& operator[](int index) { return *(data + index); }
    float* data = nullptr;
    ~SingleSample();
};

/**
 * Controls the weighted average used across a window of source samples.
 */
enum class Filter {
    DEFAULT,          // Selects MITCHELL or LANCZOS dynamically.
    BOX,              // Computes the un-weighted average over the filter radius.
    NEAREST,          // Copies the source sample nearest to the center of the filter.
    HERMITE,          // Also known as "smoothstep", has some nice properties.
    GAUSSIAN_SCALARS, // Standard Gaussian filter with sigma = 0.5
    GAUSSIAN_NORMALS, // Same as GAUSSIAN_SCALARS, but interpolates unitized vectors.
    MITCHELL,         // Cubic resampling per Mitchell-Netravali, default for magnification.
    LANCZOS,          // Popular sinc-based filter, default for minification.
    MINIMUM           // Takes a min val rather than avg, perhaps useful for depth maps and SDF's.
};

/**
 * Defines a viewport inside the texture such that (0,0) is at the top-left corner of the top-left
 * pixel, and (1,1) is at the bottom-right corner of the bottom-corner pixel.
 */
struct Region {
    float left;
    float top;
    float right;
    float bottom;
};

/**
 * Transforms the texel fetching operation when sampling from adjacent images.
 */
enum class Orientation {
    STANDARD = 0,
    FLIP_X = 1 << 0,
    FLIP_Y = 1 << 1,
    FLIP_XY = FLIP_X | FLIP_Y
};

/**
 * Specifies how to generate samples that lie outside the boundaries of the source region.
 */
struct Boundary {
    enum {
        EXCLUDE, // Ignore the samples and renormalize the filter. This is probably what you want.
        REGION,  // Keep samples that are outside sourceRegion if they are still within the image.
        CLAMP,   // Pretend the edge pixel is repeated forever. Gives edge pixels more weight.
        REPEAT,  // Resample from the region, wrapping back to the front of the row or column.
        MIRROR,  // Resample from the region but assume that it has been flipped.
        COLOR,   // Use the specified constant color.
        NEIGHBOR // Sample from an adjacent image.
    } mode = EXCLUDE;
    SingleSample color;              // Used only if mode = COLOR
    LinearImage* neighbor = nullptr; // Used only if mode = NEIGHBOR
    Orientation orientation;         // Used only if mode = NEIGHBOR
};

/**
 * Configuration for the resampleImage function. Provides reasonable defaults.
 */
struct ImageSampler {
    Filter horizontalFilter = Filter::DEFAULT;
    Filter verticalFilter = Filter::DEFAULT;
    Region sourceRegion = {0, 0, 1, 1};
    float filterRadiusMultiplier = 1;
    Boundary east;
    Boundary north;
    Boundary west;
    Boundary south;
};

/**
 * Resizes or blurs the given linear image, producing a new linear image with the given dimensions.
 */
LinearImage resampleImage(const LinearImage& source, uint32_t width, uint32_t height,
        const ImageSampler& sampler);

/**
 * Resizes the given linear image using a simplified API that takes target dimensions and filter.
 */
LinearImage resampleImage(const LinearImage& source, uint32_t width, uint32_t height,
        Filter filter = Filter::DEFAULT);

/**
 * Computes a single sample for the given texture coordinate and writes the resulting color
 * components into the given output holder.
 *
 * For decent performance, do not call this across the entire image, instead call resampleImage.
 * On the first call, pass in a default SingleSample to allocate the result holder. For example:
 *
 *     SingleSample result;
 *     computeSingleSample(img, 0.5f, 0.5f, &result);
 *     printf("r g b = %f %f %f\n", result[0], result[1], result[2]);
 *     computeSingleSample(img, 0.9f, 0.1f, &result);
 *     printf("r g b = %f %f %f\n", result[0], result[1], result[2]);
 *
 * The x y coordinates live in "texture space" such that (0.0f, 0.0f) is the upper-left boundary of
 * the top-left pixel and (+1.0f, +1.0f) is the lower-right boundary of the bottom-right pixel.
 */
void computeSingleSample(const LinearImage& source, float x, float y, SingleSample* result,
        Filter filter = Filter::BOX);

/**
 * Generates a sequence of miplevels using the requested filter. To determine the number of mips
 * it would take to get down to 1x1, see getMipmapCount.
 *
 * Source image need not be power-of-two. In the result vector, the half-size image is returned at
 * index 0, the quarter-size image is at index 1, etc. Please note that the original-sized image is
 * not included.
 */
void generateMipmaps(const LinearImage& source, Filter, LinearImage* result, uint32_t mipCount);

/**
 * Returns the number of miplevels it would take to downsample the given image down to 1x1. This
 * number does not include the original image (i.e. mip 0).
 */
uint32_t getMipmapCount(const LinearImage& source);

/**
 * Given the string name of a filter, converts it to uppercase and returns the corresponding
 * enum value. If no corresponding enumerant exists, returns DEFAULT.
 */
Filter filterFromString(const char* name);

} // namespace image

#endif /* IMAGE_IMAGESAMPLER_H */
