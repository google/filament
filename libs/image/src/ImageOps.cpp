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

#include <image/ImageOps.h>

#include <math/vec3.h>
#include <math/vec4.h>
#include <utils/Panic.h>

#include <algorithm>
#include <memory>
#include <ratio>

using namespace filament::math;

namespace image {

LinearImage horizontalStack(std::initializer_list<LinearImage> images) {
    size_t count = images.end() - images.begin();
    return horizontalStack(images.begin(), count);
}

LinearImage horizontalStack(const LinearImage* first, size_t count) {
    FILAMENT_CHECK_PRECONDITION(count > 0) << "Must supply one or more images for stacking.";

    // Compute the final size and allocate memory.
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t nchannels = 0;
    for (size_t c = 0; c < count; ++c) {
        const auto& img = first[c];
        width += img.getWidth();
        if (height == 0) {
            height = img.getHeight();
        } else {
            FILAMENT_CHECK_PRECONDITION(height == img.getHeight()) << "Inconsistent heights.";
        }
        if (nchannels == 0) {
            nchannels = img.getChannels();
        } else {
            FILAMENT_CHECK_PRECONDITION(nchannels == img.getChannels()) << "Inconsistent channels.";
        }
    }
    LinearImage result(width, height, nchannels);

    // Copy over each row of each source image.
    float* dst = result.getPixelRef();
    for (int32_t row = 0; row < height; ++row) {
        for (size_t c = 0; c < count; ++c) {
            const auto& img = first[c];
            uint32_t swidth = img.getWidth();
            float const* src = img.getPixelRef() + row * swidth * nchannels;
            memcpy(dst, src, swidth * nchannels * sizeof(float));
            dst += swidth * nchannels;
        }
    }
    return result;
}

LinearImage verticalStack(std::initializer_list<LinearImage> images) {
    size_t count = images.end() - images.begin();
    return verticalStack(images.begin(), count);
}

// To stack images vertically, we transpose them individually, then horizontalStack them, then transpose the
// result. This is incredibly lazy, but since we use row-major ordering, copying columns would be
// really painful.
LinearImage verticalStack(const LinearImage* first, size_t count) {
    FILAMENT_CHECK_PRECONDITION(count > 0) << "Must supply one or more images for stacking.";
    std::unique_ptr<LinearImage[]> flipped(new LinearImage[count]);
    int i = 0;
    for (size_t c = 0; c < count; ++c) {
        flipped[i++] = transpose(first[c]);
    }
    auto result = horizontalStack(flipped.get(), count);
    return transpose(result);
}

LinearImage horizontalFlip(const LinearImage& image) {
    const uint32_t width = image.getWidth();
    const uint32_t height = image.getHeight();
    const uint32_t channels = image.getChannels();
    LinearImage result(width, height, channels);
    for (uint32_t row = 0; row < height; ++row) {
        for (uint32_t col = 0; col < width; ++col) {
            float* dst = result.getPixelRef(width - 1 - col, row);
            float const* src = image.getPixelRef(col, row);
            for (uint32_t c = 0; c < channels; ++c) {
                dst[c] = src[c];
            }
        }
    }
    return result;
}

LinearImage verticalFlip(const LinearImage& image) {
    const uint32_t width = image.getWidth();
    const uint32_t height = image.getHeight();
    const uint32_t channels = image.getChannels();
    LinearImage result(width, height, channels);
    for (uint32_t row = 0; row < height; ++row) {
        float const* src = image.getPixelRef(0, row);
        float* dst = result.getPixelRef(0, height - 1 - row);
        memcpy(dst, src, width * channels * sizeof(float));
    }
    return result;
}

template<class VecT>
LinearImage applyScaleOffset(const LinearImage& image,
        typename VecT::value_type scale, typename VecT::value_type offset) {
    const uint32_t width = image.getWidth(), height = image.getHeight();
    LinearImage result(width, height, image.getChannels());
    auto src = (VecT const*) image.getPixelRef();
    auto dst = (VecT*) result.getPixelRef();
    for (uint32_t n = 0, end = width * height; n < end; ++n) {
        dst[n] = scale * src[n] + VecT{offset};
    }
    return result;
}

LinearImage vectorsToColors(const LinearImage& image) {
    FILAMENT_CHECK_PRECONDITION(image.getChannels() == 3 || image.getChannels() == 4)
            << "Must be a 3 or 4 channel image";
    return image.getChannels() == 3
        ? applyScaleOffset<float3>(image, 0.5f, 0.5f)
        : applyScaleOffset<float4>(image, 0.5f, 0.5f);
}

LinearImage colorsToVectors(const LinearImage& image) {
    FILAMENT_CHECK_PRECONDITION(image.getChannels() == 3 || image.getChannels() == 4)
            << "Must be a 3 or 4 channel image";
    return image.getChannels() == 3
        ? applyScaleOffset<float3>(image, 2.0f, -1.0f)
        : applyScaleOffset<float4>(image, 2.0f, -1.0f);
}

LinearImage extractChannel(const LinearImage& source, uint32_t channel) {
    const uint32_t width = source.getWidth(), height = source.getHeight();
    const uint32_t nchan = source.getChannels();
    FILAMENT_CHECK_PRECONDITION(channel < nchan) << "Channel is out of range.";
    LinearImage result(width, height, 1);
    auto src = source.getPixelRef();
    auto dst = result.getPixelRef();
    for (uint32_t n = 0, npixels = width * height; n < npixels; ++n, ++dst, src += nchan) {
        dst[0] = src[channel];
    }
    return result;
}

LinearImage combineChannels(std::initializer_list<LinearImage> images) {
    size_t count = images.end() - images.begin();
    return combineChannels(images.begin(), count);
}

LinearImage combineChannels(LinearImage const* img, size_t count) {
    FILAMENT_CHECK_PRECONDITION(count > 0) << "Must supply one or more image planes for combining.";
    const uint32_t width = img[0].getWidth();
    const uint32_t height = img[0].getHeight();
    for (size_t c = 0; c < count; ++c) {
        const LinearImage& plane = img[c];
        FILAMENT_CHECK_PRECONDITION(plane.getWidth() == width)
                << "Planes must all have same width.";
        FILAMENT_CHECK_PRECONDITION(plane.getHeight() == height)
                << "Planes must all have same height.";
        FILAMENT_CHECK_PRECONDITION(plane.getChannels() == 1) << "Planes must be single channel.";
    }
    LinearImage result(width, height, (uint32_t) count);
    float* dst = result.getPixelRef();
    uint32_t sindex = 0, dindex = 0;
    while (dindex < width * height * count) {
        for (size_t c = 0; c < count; ++c, ++dindex) {
            const LinearImage& plane = img[c];
            float const* src = plane.getPixelRef();
            dst[dindex] = src[sindex];
        }
        ++sindex;
    }
    return result;
}

// The transpose operation does not simply set a flag, it performs actual movement of data. This is
// very handy for separable filters because it (a) improves cache coherency in the second pass, and
// (b) allows the client to consume columns in the same way that it consumes rows. Our
// implementation does not support in-place transposition but it is simple and robust for non-square
// images.
LinearImage transpose(const LinearImage& image) {
    const uint32_t width = image.getWidth();
    const uint32_t height = image.getHeight();
    const uint32_t channels = image.getChannels();
    LinearImage result(height, width, channels);
    float const* source = image.getPixelRef();
    float* target = result.getPixelRef();
    for (uint32_t n = 0; n < width * height; ++n) {
        const uint32_t i = n / width;
        const uint32_t j = n % width;
        float const* src = source + channels * n;
        float* dst = target + channels * (height * j + i);
        for (uint32_t c = 0; c < channels; ++c) {
            dst[c] = src[c];
        }
    }
    return result;
}

LinearImage cropRegion(const LinearImage& image, uint32_t left, uint32_t top, uint32_t right,
        uint32_t bottom) {
    uint32_t width = right - left;
    uint32_t height = bottom - top;
    uint32_t channels = image.getChannels();
    LinearImage result(width, height, channels);
    float const* source = image.getPixelRef(left, top);
    float* target = result.getPixelRef();
    for (int32_t row = 0; row < height; ++row) {
        memcpy(target, source, width * channels * sizeof(float));
        target += width * channels;
        source += image.getWidth() * channels;
    }
    return result;
}

int compare(const LinearImage& a, const LinearImage& b, float epsilon) {
    auto w = a.getWidth();
    auto h = a.getHeight();
    auto c = a.getChannels();
    if (b.getWidth() != w || b.getHeight() != h || b.getChannels() != c) {
        return -1;
    }
    float const* adata = a.getPixelRef();
    float const* bdata = b.getPixelRef();
    return std::lexicographical_compare(adata, adata + w * h * c, bdata, bdata + w * h * c,
            [epsilon](float x, float y) { return x < y - epsilon; });
}

void clearToValue(LinearImage& image, float value) {
    const uint32_t nvals = image.getWidth() * image.getHeight() * image.getChannels();
    float* data = image.getPixelRef();
    for (uint32_t index = 0; index < nvals; ++index) {
        data[index] = value;
    }
}

// Please avoid using numeric_limits::infinity here, it has undesireable properties in the context
// of our EDT algorithm. We simply need a large number that represents the largest squared distance
// that ever needs to be considered.
static constexpr float INF = 4096.0f * 4096.f;

// void edt(f, d, z, v, i, n)
//
// Finds the lower envelope of a sequence of parabolas, as per Felzenszwalb and Huttenlocher.
// This operates on a single row of an image.
//
//   f...source data (returns the Y of the parabola vertex at X)
//   d...destination data (final distance values are written here)
//   z...temporary used to store X coords of parabola intersections
//   v...temporary used to store X coords of parabola vertices
//   i...resulting X coords of parabola vertices
//   n...number of pixels in "f" to process
//
static void edt(const float* f, float* d, float* z, float* v, float* i, size_t n) {
    int k = 0;
    v[0] = 0;
    z[0] = -INF;
    z[1] = +INF;
    for (size_t iq = 1; iq < n; ++iq) {
        const float fq = iq;
        float fp, s;
        int ip;

        // If the new parabola is lower than the right-most parabola in
        // the envelope, remove it from the envelope. To make this
        // determination, find the X coordinate of the intersection (s)
        // between the parabolas with vertices at (q,f[q]) and (p,f[p]).
        fp = v[k];
        ip = fp;
        s = ((f[iq] + fq * fq) - (f[ip] + fp * fp)) / (2.0f * fq - 2.0f * fp);
        while (s <= z[k]) {
            k = k - 1;
            fp = v[k];
            ip = fp;
            s = ((f[iq] + fq * fq) - (f[ip] + fp * fp)) / (2.0f * fq - 2.0f * fp);
        }

        // Add the new parabola to the envelope.
        ++k;
        v[k] = fq;
        z[k] = s;
        z[k + 1] = +INF;
    }

    // Go back through the parabolas in the envelope and evaluate them
    // in order to populate the distance values at each X coordinate.
    k = 0;
    for (size_t iq = 0; iq < n; ++iq) {
        const float fq = iq;
        while (z[k + 1] < fq) ++k;
        const float dx = fq - v[k];
        d[iq] = dx * dx + f[int(v[k])];
        i[iq] = v[k];
    }
}

static LinearImage computeHorizontalEdt(const LinearImage& src, LinearImage cx) {
    const uint32_t width = src.getWidth();
    const uint32_t height = src.getHeight();
    LinearImage tmp0(width + 1, height + 1, 1);
    LinearImage tmp1(width + 1, height + 1, 1);
    LinearImage dst(width, height, 1);

    // TODO: use utils::job::parallel_for.
    for (uint32_t row = 0; row < height; ++row) {
        const float* f = src.getPixelRef(0, row);
        float* d = dst.getPixelRef(0, row);
        float* z = tmp0.getPixelRef(0, row);
        float* v = tmp1.getPixelRef(0, row);
        float* i = cx.getPixelRef(0, row);
        edt(f, d, z, v, i, width);
    }

    return dst;
}

// Implements the paper 'Distance Transforms of Sampled Functions' by Felzenszwalb and Huttenlocher
// but generalized to compute a coordinate field rather than a distance field. Coordinate fields are
// more broadly useful and transforming them into distance fields is extremely cheap.
LinearImage computeCoordField(const LinearImage& src, PresenceCallback presence, void* user) {
    const uint32_t width = src.getWidth();
    const uint32_t height = src.getHeight();
    LinearImage f0(width, height, 1);
    for (uint32_t row = 0; row < height; ++row) {
        float* pf = f0.getPixelRef(0, row);
        for (uint32_t col = 0; col < width; ++col) {
            pf[col] = presence(src, col, row, user) ? 0.0f : INF;
        }
    }

    LinearImage cx(width, height, 1);
    LinearImage cy(height, width, 1);

    f0 = computeHorizontalEdt(f0, cx);
    f0 = transpose(f0);
    f0 = computeHorizontalEdt(f0, cy);
    f0 = transpose(f0);

    // NOTE: this could be extended to compute a volumetric distance field by transposing
    // X with Z at this point (rather than X with Y) and re-invoking computeHorizontalEdt.

    LinearImage coords(width, height, 2);
    for (uint32_t row = 0; row < height; ++row) {
        for (uint32_t col = 0; col < width; ++col) {
            float y = cy.getPixelRef(row, col)[0];
            float x = cx.getPixelRef(col, y)[0];
            float* dst = coords.getPixelRef(col, row);
            dst[0] = x;
            dst[1] = y;
        }
    }

    return coords;
}

LinearImage edtFromCoordField(const LinearImage& coordField, bool sqrt) {
    const uint32_t width = coordField.getWidth();
    const uint32_t height = coordField.getHeight();
    LinearImage result(width, height, 1);
    for (int32_t row = 0; row < height; ++row) {
        const float frow = row;
        float* dst = result.getPixelRef(0, row);
        for (uint32_t col = 0; col < width; ++col) {
            const float fcol = col;
            const float* coord = coordField.getPixelRef(col, row);
            const float dx = coord[0] - fcol;
            const float dy = coord[1] - frow;
            float distance = dx * dx + dy * dy;
            if (sqrt) {
                distance = std::sqrt(distance);
            }
            dst[col] = distance;
        }
    }
    return result;
}

// Dereferences the given coordinate field. Useful for creating Voronoi diagrams or dilated images.
LinearImage voronoiFromCoordField(const LinearImage& coordField, const LinearImage& src) {
    const uint32_t width = src.getWidth();
    const uint32_t height = src.getHeight();
    const uint32_t channels = src.getChannels();
    LinearImage result(width, height, channels);
    for (int32_t row = 0; row < height; ++row) {
        for (uint32_t col = 0; col < width; ++col) {
            const float* coord = coordField.getPixelRef(col, row);
            uint32_t srccol = coord[0];
            uint32_t srcrow = coord[1];
            float* presult = result.getPixelRef(col, row);
            const float* psource = src.getPixelRef(srccol, srcrow);
            for (uint32_t channel = 0; channel < channels; ++channel) {
                presult[channel] = psource[channel];
            }
        }
    }
    return result;
}

void blitImage(LinearImage& target, const LinearImage& source) {
    FILAMENT_CHECK_PRECONDITION(source.getWidth() == target.getWidth())
            << "Images must have same width.";
    FILAMENT_CHECK_PRECONDITION(source.getHeight() == target.getHeight())
            << "Images must have same height.";
    FILAMENT_CHECK_PRECONDITION(source.getChannels() == target.getChannels())
            << "Images must have same number of channels.";
    memcpy(target.getPixelRef(), source.getPixelRef(),
            sizeof(float) * source.getWidth() * source.getHeight() * source.getChannels());
}

} // namespace image
