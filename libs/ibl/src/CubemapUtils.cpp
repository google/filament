/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <ibl/CubemapUtils.h>

#include "CubemapUtilsImpl.h"

#include <ibl/utilities.h>

#include <utils/JobSystem.h>

#include <math/mat4.h>

#include <algorithm>

#include <math.h>
#include <string.h>

using namespace filament::math;
using namespace utils;

namespace filament {
namespace ibl {

void CubemapUtils::clamp(Image& src) {
    // See: http://graphicrants.blogspot.com/2013/12/tone-mapping.html
    // By Brian Karis
    auto compress = [](float3 color, float linear, float compressed) {
        float luma = dot(color, float3{ 0.2126, 0.7152, 0.0722 }); // REC 709
        return luma <= linear ? color :
               (color / luma) * ((linear * linear - compressed * luma)
                                 / (2 * linear - compressed - luma));
    };
    const size_t width = src.getWidth();
    const size_t height = src.getHeight();
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            float3& c = *static_cast<float3*>(src.getPixelRef(x, y));
            // these values are chosen arbitrarily and seem to produce good result with
            // 4096 samples
            c = compress(c, 4096.0f, 16384.0f);
        }
    }
}

void CubemapUtils::highlight(Image& src) {
    const size_t width = src.getWidth();
    const size_t height = src.getHeight();
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            float3& c = *static_cast<float3*>(src.getPixelRef(x, y));
            if (min(c) < 0.0f) {
                c = { 0, 0, 1 };
            } else if (max(c) > 64512.0f) { // maximum encodable by 10-bits float (RGB_11_11_10)
                c = { 1, 0, 0 };
            }
        }
    }
}

void CubemapUtils::downsampleCubemapLevelBoxFilter(JobSystem& js, Cubemap& dst, const Cubemap& src) {
    size_t scale = src.getDimensions() / dst.getDimensions();
    processSingleThreaded<EmptyState>(dst, js,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
                const Image& image(src.getImageForFace(f));
                for (size_t x = 0; x < dim; ++x, ++data) {
                    Cubemap::writeAt(data, Cubemap::filterAtCenter(image, x * scale, y * scale));
                }
            });
}


/*
 * Area of a cube face's quadrant projected onto a sphere
 *
 *  1 +---+----------+
 *    |   |          |
 *    |---+----------|
 *    |   |(x,y)     |
 *    |   |          |
 *    |   |          |
 * -1 +---+----------+
 *   -1              1
 *
 *
 * The quadrant (-1,1)-(x,y) is projected onto the unit sphere
 *
 */
static inline float sphereQuadrantArea(float x, float y) {
    return std::atan2(x*y, std::sqrt(x*x + y*y + 1));
}

float CubemapUtils::solidAngle(size_t dim, size_t u, size_t v) {
    const float iDim = 1.0f / dim;
    float s = ((u + 0.5f) * 2 * iDim) - 1;
    float t = ((v + 0.5f) * 2 * iDim) - 1;
    const float x0 = s - iDim;
    const float y0 = t - iDim;
    const float x1 = s + iDim;
    const float y1 = t + iDim;
    float solidAngle = sphereQuadrantArea(x0, y0) -
                        sphereQuadrantArea(x0, y1) -
                        sphereQuadrantArea(x1, y0) +
                        sphereQuadrantArea(x1, y1);
    return solidAngle;
}

Cubemap CubemapUtils::create(Image& image, size_t dim, bool horizontal) {
    Cubemap cm(dim);
    Image temp(CubemapUtils::createCubemapImage(dim, horizontal));
    CubemapUtils::setAllFacesFromCross(cm, temp);
    std::swap(image, temp);
    return cm;
}

void CubemapUtils::setFaceFromCross(Cubemap& cm, Cubemap::Face face, const Image& image) {
    size_t dim = cm.getDimensions() + 2; // 2 extra per image, for seamlessness
    size_t x = 0;
    size_t y = 0;
    switch (face) {
        case Cubemap::Face::NX:
            x = 0, y = dim;
            break;
        case Cubemap::Face::PX:
            x = 2 * dim, y = dim;
            break;
        case Cubemap::Face::NY:
            x = dim, y = 2 * dim;
            break;
        case Cubemap::Face::PY:
            x = dim, y = 0;
            break;
        case Cubemap::Face::NZ:
            x = 3 * dim, y = dim;
            break;
        case Cubemap::Face::PZ:
            x = dim, y = dim;
            break;
    }
    Image subImage;
    subImage.subset(image, x + 1, y + 1, dim - 2, dim - 2);
    cm.setImageForFace(face, subImage);
}

void CubemapUtils::setAllFacesFromCross(Cubemap& cm, const Image& image) {
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::NX, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::PX, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::NY, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::PY, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::NZ, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::PZ, image);
}

Image CubemapUtils::createCubemapImage(size_t dim, bool horizontal) {
    // always allocate 2 extra column and row / face, to allow the cubemap to be "seamless"
    size_t width = 4 * (dim + 2);
    size_t height = 3 * (dim + 2);
    if (!horizontal) {
        std::swap(width, height);
    }

    Image image(width, height);
    memset(image.getData(), 0, image.getBytesPerRow() * height);
    return image;
}

#ifndef FILAMENT_IBL_LITE

void CubemapUtils::equirectangularToCubemap(JobSystem& js, Cubemap& dst, const Image& src) {
    const size_t width = src.getWidth();
    const size_t height = src.getHeight();

    auto toRectilinear = [width, height](float3 s) -> float2 {
        float xf = std::atan2(s.x, s.z) * F_1_PI;   // range [-1.0, 1.0]
        float yf = std::asin(s.y) * (2 * F_1_PI);   // range [-1.0, 1.0]
        xf = (xf + 1.0f) * 0.5f * (width  - 1);        // range [0, width [
        yf = (1.0f - yf) * 0.5f * (height - 1);        // range [0, height[
        return float2(xf, yf);
    };

    process<EmptyState>(dst, js,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            // calculate how many samples we need based on dx, dy in the source
            // x = cos(phi) sin(theta)
            // y = sin(phi)
            // z = cos(phi) cos(theta)

            // here we try to figure out how many samples we need, by evaluating the surface
            // (in pixels) in the equirectangular -- we take the bounding box of the
            // projection of the cubemap texel's corners.

            auto pos0 = toRectilinear(dst.getDirectionFor(f, x + 0.0f, y + 0.0f)); // make sure to use the float version
            auto pos1 = toRectilinear(dst.getDirectionFor(f, x + 1.0f, y + 0.0f)); // make sure to use the float version
            auto pos2 = toRectilinear(dst.getDirectionFor(f, x + 0.0f, y + 1.0f)); // make sure to use the float version
            auto pos3 = toRectilinear(dst.getDirectionFor(f, x + 1.0f, y + 1.0f)); // make sure to use the float version
            const float minx = std::min(pos0.x, std::min(pos1.x, std::min(pos2.x, pos3.x)));
            const float maxx = std::max(pos0.x, std::max(pos1.x, std::max(pos2.x, pos3.x)));
            const float miny = std::min(pos0.y, std::min(pos1.y, std::min(pos2.y, pos3.y)));
            const float maxy = std::max(pos0.y, std::max(pos1.y, std::max(pos2.y, pos3.y)));
            const float dx = std::max(1.0f, maxx - minx);
            const float dy = std::max(1.0f, maxy - miny);
            const size_t numSamples = size_t(dx * dy);

            const float iNumSamples = 1.0f / numSamples;
            float3 c = 0;
            for (size_t sample = 0; sample < numSamples; sample++) {
                // Generate numSamples in our destination pixels and map them to input pixels
                const float2 h = hammersley(uint32_t(sample), iNumSamples);
                const float3 s(dst.getDirectionFor(f, x + h.x, y + h.y));
                auto pos = toRectilinear(s);

                // we can't use filterAt() here because it reads past the width/height
                // which is okay for cubmaps but not for square images

                // TODO: the sample should be weighed by the area it covers in the cubemap texel

                c += Cubemap::sampleAt(src.getPixelRef((uint32_t)pos.x, (uint32_t)pos.y));
            }
            c *= iNumSamples;

            Cubemap::writeAt(data, c);
        }
    });
}

void CubemapUtils::cubemapToEquirectangular(JobSystem& js, Image& dst, const Cubemap& src) {
    const float w = dst.getWidth();
    const float h = dst.getHeight();
    auto parallelJobTask = [&](size_t j0, size_t count) {
        for (size_t j = j0; j < j0 + count; j++) {
            for (size_t i = 0; i < w; i++) {
                float3 c = 0;
                const size_t numSamples = 64; // TODO: how to chose numsamples
                for (size_t sample = 0; sample < numSamples; sample++) {
                    const float2 u = hammersley(uint32_t(sample), 1.0f / numSamples);
                    float x = 2.0f * (i + u.x) / w - 1.0f;
                    float y = 1.0f - 2.0f * (j + u.y) / h;
                    float theta = x * F_PI;
                    float phi = y * F_PI * 0.5;
                    float3 s = {
                            std::cos(phi) * std::sin(theta),
                            std::sin(phi),
                            std::cos(phi) * std::cos(theta) };
                    c += src.filterAt(s);
                }
                Cubemap::writeAt(dst.getPixelRef(i, j), c * (1.0f / numSamples));
            }
        }
    };

    auto job = jobs::parallel_for(js, nullptr, 0, uint32_t(h),
            std::ref(parallelJobTask), jobs::CountSplitter<1, 8>());
    js.runAndWait(job);
}

void CubemapUtils::cubemapToOctahedron(JobSystem& js, Image& dst, const Cubemap& src) {
    const float w = dst.getWidth();
    const float h = dst.getHeight();
    auto parallelJobTask = [&](size_t j0, size_t count) {
        for (size_t j = j0; j < j0 + count; j++) {
            for (size_t i = 0; i < w; i++) {
                float3 c = 0;
                const size_t numSamples = 64; // TODO: how to chose numsamples
                for (size_t sample = 0; sample < numSamples; sample++) {
                    const float2 u = hammersley(uint32_t(sample), 1.0f / numSamples);
                    float x = 2.0f * (i + u.x) / w - 1.0f;
                    float z = 2.0f * (j + u.y) / h - 1.0f;
                    float y;
                    if (std::abs(z) > (1.0f - std::abs(x))) {
                        float u = x < 0 ? std::abs(z) - 1 : 1 - std::abs(z);
                        float v = z < 0 ? std::abs(x) - 1 : 1 - std::abs(x);
                        x = u;
                        z = v;
                        y = (std::abs(x) + std::abs(z)) - 1.0f;
                    } else {
                        y = 1.0f - (std::abs(x) + std::abs(z));
                    }
                    c += src.filterAt({x, y, z});
                }
                Cubemap::writeAt(dst.getPixelRef(i, j), c * (1.0f / numSamples));
            }
        }
    };

    auto job = jobs::parallel_for(js, nullptr, 0, uint32_t(h),
            std::ref(parallelJobTask), jobs::CountSplitter<1, 8>());
    js.runAndWait(job);
}

void CubemapUtils::crossToCubemap(JobSystem& js, Cubemap& dst, const Image& src) {
    process<EmptyState>(dst, js,
            [&](EmptyState&, size_t iy, Cubemap::Face f, Cubemap::Texel* data, size_t dimension) {
                for (size_t ix = 0; ix < dimension; ++ix, ++data) {
                    // find offsets from face
                    size_t x = ix;
                    size_t y = iy;
                    size_t dx = 0;
                    size_t dy = 0;
                    size_t dim = std::max(src.getHeight(), src.getWidth()) / 4;

                    switch (f) {
                        case Cubemap::Face::NX:
                            dx = 0, dy = dim;
                            break;
                        case Cubemap::Face::PX:
                            dx = 2 * dim, dy = dim;
                            break;
                        case Cubemap::Face::NY:
                            dx = dim, dy = 2 * dim;
                            break;
                        case Cubemap::Face::PY:
                            dx = dim, dy = 0;
                            break;
                        case Cubemap::Face::NZ:
                            if (src.getHeight() > src.getWidth()) {
                                dx = dim, dy = 3 * dim;
                                x = dimension - 1 - ix;
                                y = dimension - 1 - iy;
                            } else {
                                dx = 3 * dim, dy = dim;
                            }
                            break;
                        case Cubemap::Face::PZ:
                            dx = dim, dy = dim;
                            break;
                    }

                    size_t sampleCount = std::max(size_t(1), dim / dimension);
                    sampleCount = std::min(size_t(256), sampleCount * sampleCount);
                    for (size_t i = 0; i < sampleCount; i++) {
                        const float2 h = hammersley(uint32_t(i), 1.0f / sampleCount);
                        size_t u = dx + size_t((x + h.x) * dim / dimension);
                        size_t v = dy + size_t((y + h.y) * dim / dimension);
                        Cubemap::writeAt(data, Cubemap::sampleAt(src.getPixelRef(u, v)));
                    }
                }
            });
}

const char* CubemapUtils::getFaceName(Cubemap::Face face) {
    switch (face) {
        case Cubemap::Face::NX: return "nx";
        case Cubemap::Face::PX: return "px";
        case Cubemap::Face::NY: return "ny";
        case Cubemap::Face::PY: return "py";
        case Cubemap::Face::NZ: return "nz";
        case Cubemap::Face::PZ: return "pz";
    }
}

void CubemapUtils::mirrorCubemap(JobSystem& js, Cubemap& dst, const Cubemap& src) {
    processSingleThreaded<EmptyState>(dst, js,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            const float3 N(dst.getDirectionFor(f, x, y));
            Cubemap::writeAt(data, src.sampleAt(float3{ -N.x, N.y, N.z }));
        }
    });
}

void CubemapUtils::generateUVGrid(JobSystem& js, Cubemap& cml, size_t gridFrequencyX, size_t gridFrequencyY) {
    Cubemap::Texel const colors[6] = {
            { 1, 1, 1 }, // +X /  r  - white
            { 1, 0, 0 }, // -X /  l  - red
            { 0, 0, 1 }, // +Y /  t  - blue
            { 0, 1, 0 }, // -Y /  b  - green
            { 1, 1, 0 }, // +z / fr - yellow
            { 1, 0, 1 }, // -Z / bk - magenta
    };
    const float uvGridHDRIntensity = 5.0f;
    size_t gridSizeX = cml.getDimensions() / gridFrequencyX;
    size_t gridSizeY = cml.getDimensions() / gridFrequencyY;
    CubemapUtils::process<EmptyState>(cml, js,
            [ & ](EmptyState&,
                    size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
                for (size_t x = 0; x < dim; ++x, ++data) {
                    bool grid = bool(((x / gridSizeX) ^ (y / gridSizeY)) & 1);
                    Cubemap::Texel t = grid ? colors[(int)f] * uvGridHDRIntensity : 0;
                    Cubemap::writeAt(data, t);
                }
            });
}
#endif

} // namespace ibl
} // namespace filament
