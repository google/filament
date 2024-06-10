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

#include <image/ImageSampler.h>
#include <image/ImageOps.h>

#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/Panic.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace image;

namespace {

using namespace filament::math;

struct FilterFunction {
    float (*fn)(float) = nullptr;
    float boundingRadius = 1;
    bool rejectExternalSamples = true;
};

const FilterFunction Box {
    .fn = [](float t) { return t <= 0.5f ? 1.0f : 0.0f; },
    .boundingRadius = 1
};

const FilterFunction Nearest { Box.fn, 0.0f };

const FilterFunction Gaussian {
    .fn = [](float t) {
        if (t >= 2.0) return 0.0f;
        const float scale = 1.0f / std::sqrt(0.5f * f::PI);
        return std::exp(-2.0f * t * t) * scale;
    },
    .boundingRadius = 2
};

const FilterFunction Hermite {
    .fn = [](float t) {
        if (t >= 1.0f) return 0.0f;
        return 2 * t * t * t - 3 * t * t + 1;
    },
    .boundingRadius = 1
};

const FilterFunction Mitchell {
    .fn = [](float t) {
        constexpr float B = 1.0f / 3.0f;
        constexpr float C = 1.0f / 3.0f;
        constexpr float P0 = (  6 - 2*B       ) / 6.0f;
        constexpr float P1 = 0;
        constexpr float P2 = (-18 +12*B + 6*C ) / 6.0f;
        constexpr float P3 = ( 12 - 9*B - 6*C ) / 6.0f;
        constexpr float Q0 = (      8*B +24*C ) / 6.0f;
        constexpr float Q1 = (    -12*B -48*C ) / 6.0f;
        constexpr float Q2 = (      6*B +30*C ) / 6.0f;
        constexpr float Q3 = (    - 1*B - 6*C ) / 6.0f;
        if (t >= 2.0f) return 0.0f;
        if (t >= 1.0f) return Q0 + Q1*t + Q2*t*t + Q3*t*t*t;
        return P0 + P1*t + P2*t*t + P3*t*t*t;
    },
    .boundingRadius = 2
};

// Not bothering with a fast approximation since we cache results for each row.
float sinc(float t) {
    if (t <= 0.00001f) return 1.0f;
    return std::sin(f::PI * t) / (f::PI * t);
}

const FilterFunction Lanczos {
    .fn = [](float t) {
        if (t >= 1.0f) return 0.0f;
        return sinc(t) * sinc(t);
    },
    .boundingRadius = 1
};

// Describes a Multiply-Add operation: target[targetIndex] += source[sourceIndex] * weight.
// This allows us to cache the weights computed by evaluation of the filter function, as well as
// the indices that are generated from careful alignment of sample and target samples.
// We allow signed source indices to accommodate external source samples whose values depend
// on the wrap-mode configuration in the sampler.
struct MadInstruction {
    uint32_t targetIndex;
    int32_t sourceIndex;
    float weight;
};

using MadProgram = std::vector<MadInstruction>;

// Generates a list of MAD instructions that transforms a row of samples of length "nsource"
// into a sequence of length "ntarget" using the given filter function.
//
// The given left / right floats define a source range within [0,1] such that 0 is at the left edge
// of the the left-most pixel and 1 is at the right edge of the right-most pixel.
//
// Regarding our nomenclature, we use prefixes as follows:
//    n....number of samples in the row
//    d....delta (i.e. the normalized width of a single pixel square)
//    x....normalized coord in [0..1] where 0/1 are the outer edges of the range.
//    i....integer index where 0 is the left-most pixel and n-1 is the right-most pixel.
void generateMadProgram(uint32_t ntarget, uint32_t nsource, float left, float right,
        FilterFunction filter, float radiusMultiplier, MadProgram* result) {
    const float dtarget = 1.0f / ntarget;
    const float fnsource = float(nsource) * (right - left);
    const bool minifying = float(ntarget) < fnsource;
    const float domainScale = (minifying ? ntarget : fnsource) / radiusMultiplier;

    // As an optimization, compute the "filterBound", which is the half-width of the filter within
    // the [0,1] domain. If this were a huge number, the filtered results would look the same, but
    // the filter would perform very poorly because it would be iterating over a lot more samples
    // than necessary.
    const float filterBounds = domainScale * std::abs(filter.boundingRadius);

    // Iterate through target samples. "xtarget" points to the center of each target pixel.
    float xtarget = dtarget / 2.0f;
    for (uint32_t itarget = 0; itarget < ntarget; ++itarget, xtarget += dtarget) {

        // For this particular target pixel, we'll be accumulating a count and sum so that we can
        // adjust the weights afterwards. This allows us to reject some of the source samples.
        uint32_t count = 0;
        float sum = 0;

        // Iterate through source samples that lie within the bounded region.
        const auto isource_lower = int32_t((xtarget - filterBounds) * nsource);
        const auto isource_upper = int32_t(std::ceil((xtarget + filterBounds) * nsource));
        for (int32_t isource = isource_lower; isource <= isource_upper; ++isource) {
            const float xsource = (((isource + 0.5f) / nsource) - left) / (right - left);
            const bool outside_image = isource < 0 || isource >= int32_t(nsource);
            const bool outside_range = xsource < 0 || xsource >= 1.0f;
            if (filter.rejectExternalSamples && (outside_image || outside_range)) {
                continue;
            }
            const float t = domainScale * std::abs(xsource - xtarget);
            const float weight = filter.fn(t);
            if (weight != 0) {
                result->push_back({itarget, isource, weight});
                sum += weight;
                ++count;
            }
        }

        // Normalize the set of weights that were recently appended to the MAD program.
        if (sum != 0) {
            MadInstruction* mad = result->data() + result->size() - count;
            for (uint32_t i = 0; i < count; ++i, ++mad) {
                mad->weight /= sum;
            }
        }
    }
}

// Transforms a MAD program intended for single-channel data into a program intended for
// multi-channel data.
void expandMadProgram(uint32_t nchannels, MadProgram* program) {
    if (nchannels == 1) {
        return;
    }
    MadProgram result;
    result.reserve(program->size() * nchannels);
    for (auto mad : *program) {
        mad.sourceIndex *= nchannels;
        mad.targetIndex *= nchannels;
        for (uint32_t j = 0; j < nchannels; ++j, ++mad.sourceIndex, ++mad.targetIndex) {
            result.push_back(mad);
        }
    }
    program->swap(result);
}

FilterFunction createFilterFunction(Filter ftype) {
    FilterFunction fn;
    switch (ftype) {
        case Filter::MINIMUM:
        case Filter::BOX:              fn = Box; break;
        case Filter::NEAREST:          fn = Nearest; break;
        case Filter::HERMITE:          fn = Hermite; break;
        case Filter::MITCHELL:         fn = Mitchell; break;
        case Filter::LANCZOS:          fn = Lanczos; break;
        case Filter::GAUSSIAN_NORMALS:
        case Filter::GAUSSIAN_SCALARS: fn = Gaussian; break;
        case Filter::DEFAULT:
            PANIC_PRECONDITION("Unresolved filter type.");
    }
    return fn;
}

template <class VecT>
void normalizeImpl(LinearImage& image) {
    const uint32_t width = image.getWidth(), height = image.getHeight();
    auto vecs = (VecT*) image.getPixelRef();
    for (uint32_t n = 0; n < width * height; ++n) {
        vecs[n] = normalize(vecs[n]);
    }
}

void normalize(LinearImage& image) {
    FILAMENT_CHECK_PRECONDITION(image.getChannels() == 3 || image.getChannels() == 4)
            << "Must be a 3 or 4 channel image";
    if (image.getChannels() == 3) {
      normalizeImpl< filament::math::float3>(image);
    } else {
      normalizeImpl< filament::math::float4>(image);
    }
}

LinearImage resampleImage1D(const LinearImage& source, MadProgram* program,
        uint32_t twidth, Filter filter, float left, float right, float filterRadiusMultiplier) {
    const uint32_t swidth = source.getWidth();
    const uint32_t sheight = source.getHeight();
    const uint32_t nchan = source.getChannels();
    const bool mag = twidth > swidth;
    if (filter == Filter::DEFAULT) filter = mag ? Filter::MITCHELL : Filter::LANCZOS;
    const FilterFunction hfn = createFilterFunction(filter);

    // Generate a flat list of multiply-add (MAD) instructions.
    program->clear();
    generateMadProgram(twidth, swidth, left, right, hfn, filterRadiusMultiplier, program);
    expandMadProgram(nchan, program);

    // Allocate the target image.
    LinearImage result(twidth, sheight, nchan);
    float const* sourceRow = source.getPixelRef();
    float* targetRow = result.getPixelRef();

    // The MIN filter is special because it starts with non-zero values and ignores filter weights.
    if (filter == Filter::MINIMUM) {
        for (uint32_t n = 0; n < twidth * sheight * nchan; ++n) {
            targetRow[n] = std::numeric_limits<float>::max();
        }
        for (uint32_t row = 0; row < sheight; ++row) {
            for (auto mad : *program) {
                const float a = sourceRow[mad.sourceIndex];
                const float b = targetRow[mad.targetIndex];
                targetRow[mad.targetIndex] = std::min(a, b);
            }
            targetRow += twidth * nchan;
            sourceRow += swidth * nchan;
        }
        return result;
    }

    // Resize the image horizontally by executing the MAD instructions over each row.
    for (uint32_t row = 0; row < sheight; ++row) {
        for (auto mad : *program) {
            targetRow[mad.targetIndex] += sourceRow[mad.sourceIndex] * mad.weight;
        }
        targetRow += twidth * nchan;
        sourceRow += swidth * nchan;
    }

    // Perform post processing for the current pass.
    if (filter == Filter::GAUSSIAN_NORMALS) {
        normalize(result);
    }
    return result;
}

} // anonymous namespace

namespace image {

SingleSample::~SingleSample() {
    delete[] data;
}

LinearImage resampleImage(const LinearImage& source, uint32_t width, uint32_t height,
        const ImageSampler& sampler) {
    FILAMENT_CHECK_PRECONDITION(sampler.east.mode == Boundary::EXCLUDE &&
            sampler.north.mode == Boundary::EXCLUDE && sampler.west.mode == Boundary::EXCLUDE &&
            sampler.south.mode == Boundary::EXCLUDE)
            << "Not yet implemented.";
    const auto hfilter = sampler.horizontalFilter;
    const auto vfilter = sampler.verticalFilter;
    const float radius = sampler.filterRadiusMultiplier;
    const float left = sampler.sourceRegion.left;
    const float top = sampler.sourceRegion.top;
    const float right = sampler.sourceRegion.right;
    const float bottom = sampler.sourceRegion.bottom;
    MadProgram program;
    LinearImage result;
    result = transpose(resampleImage1D(source, &program, width, hfilter, left, right, radius));
    result = transpose(resampleImage1D(result, &program, height, vfilter, top, bottom, radius));
    return result;
}

LinearImage resampleImage(const LinearImage& source, uint32_t width, uint32_t height,
        Filter filter) {
    return resampleImage(source, width, height, ImageSampler {
        .horizontalFilter = filter,
        .verticalFilter = filter
    });
}

void computeSingleSample(const LinearImage& source, float x, float y, SingleSample* result,
        Filter filter) {
    const float radius = 1.0f;
    const float left = x - radius / source.getWidth();
    const float top = y - radius / source.getHeight();
    const float right = x + radius / source.getWidth();
    const float bottom = y + radius / source.getHeight();
    MadProgram program;
    LinearImage row = transpose(resampleImage1D(source, &program, 1, filter, left, right, radius));
    row = resampleImage1D(row, &program, 1, filter, top, bottom, radius);
    if (!result->data) {
        result->data = new float[source.getChannels()];
    }
    float* dst = result->data;
    float const* src = row.getPixelRef();
    for (uint32_t c = 0; c < source.getChannels(); ++c) {
        dst[c] = src[c];
    }
}

// Generates the given number of mipmaps (not including the base level) using the given filter.
// Unlike traditional mipmap generation, our implementation generates all levels from the original
// image, under the premise that this produces a higher quality result.
void generateMipmaps(const LinearImage& source, Filter filter, LinearImage* result, uint32_t mips) {
    mips = std::min(mips, getMipmapCount(source));
    uint32_t width = source.getWidth();
    uint32_t height = source.getHeight();
    for (uint32_t n = 0; n < mips; ++n) {
        width = std::max(width >> 1u, 1u);
        height = std::max(height >> 1u, 1u);
        result[n] = resampleImage(source, width, height, filter);
    }
}

uint32_t getMipmapCount(const LinearImage& source) {
    uint32_t width = source.getWidth();
    uint32_t height = source.getHeight();
    uint32_t count = 0;
    while (width > 1 || height > 1) {
        ++count;
        width = std::max(width >> 1u, 1u);
        height = std::max(height >> 1u, 1u);
    }
    return count;
}

Filter filterFromString(const char* rawname) {
    using namespace utils;
    static const std::unordered_map<std::string_view, Filter> map = {
            { "BOX",      Filter::BOX },
            { "NEAREST",  Filter::NEAREST },
            { "HERMITE",  Filter::HERMITE },
            { "GAUSSIAN", Filter::GAUSSIAN_SCALARS },
            { "NORMALS",  Filter::GAUSSIAN_NORMALS },
            { "MITCHELL", Filter::MITCHELL },
            { "LANCZOS",  Filter::LANCZOS },
            { "MINIMUM",  Filter::MINIMUM },
    };
    std::string name = rawname;
    for (auto& c: name) { c = (char)toupper((unsigned char)c); }
    auto iter = map.find(name);
    return iter == map.end() ? Filter::DEFAULT : iter->second;
}

} // namespace image
