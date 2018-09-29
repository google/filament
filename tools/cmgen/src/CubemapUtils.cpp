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

#include "CubemapUtils.h"

#include <utils/JobSystem.h>

#include <math/mat4.h>

#include <algorithm>
#include <cmath>

#include <string.h>

using namespace math;
using namespace image;
using namespace utils;

void CubemapUtils::clamp(Image& src) {
    // We clamp all values to 256 which correspond to the maximum value (before
    // gamma compression) that we can store in RGBM.
    // This clamping is necessary because:
    // - our importance-sampling (when calculating the pre-filtered mipmaps)
    //   behaves badly with with very strong high-frequencies.
    // - SH can't encode such environments with a small number of bands.
    const size_t width = src.getWidth();
    const size_t height = src.getHeight();
    for (size_t y=0 ; y<height ; ++y) {
        for (size_t x = 0; x < width; ++x) {
            float3& c = *static_cast<float3*>(src.getPixelRef(x, y));
            c.x = std::min(c.x, 256.0f);
            c.y = std::min(c.y, 256.0f);
            c.z = std::min(c.z, 256.0f);
        }
    }
}

void CubemapUtils::equirectangularToCubemap(Cubemap& dst, const Image& src) {
    const size_t width = src.getWidth();
    const size_t height = src.getHeight();
    const double r = width * 0.5 * M_1_PI;

    auto toRectilinear = [width, height](double3 s) -> double2 {
        double xf = std::atan2(s.x, s.z) * M_1_PI;   // range [-1.0, 1.0]
        double yf = std::asin(s.y) * (2 * M_1_PI);   // range [-1.0, 1.0]
        xf = (xf + 1.0) * 0.5 * (width  - 1);        // range [0, width [
        yf = (1.0 - yf) * 0.5 * (height - 1);        // range [0, height[
        return double2(xf, yf);
    };

    process<EmptyState>(dst,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            // calculate how many samples we need based on dx, dy in the source
            // x = cos(phi) sin(theta)
            // y = sin(phi)
            // z = cos(phi) cos(theta)

            // here we try to figure out how many samples we need, by evaluating the surface
            // (in pixels) in the equirectangular -- we take the bounding box of the
            // projection of the cubemap texel's corners.

            auto pos0 = toRectilinear(dst.getDirectionFor(f, x + 0.0, y + 0.0)); // make sure to use the double version
            auto pos1 = toRectilinear(dst.getDirectionFor(f, x + 1.0, y + 0.0)); // make sure to use the double version
            auto pos2 = toRectilinear(dst.getDirectionFor(f, x + 0.0, y + 1.0)); // make sure to use the double version
            auto pos3 = toRectilinear(dst.getDirectionFor(f, x + 1.0, y + 1.0)); // make sure to use the double version
            const double minx = std::min(pos0.x, std::min(pos1.x, std::min(pos2.x, pos3.x)));
            const double maxx = std::max(pos0.x, std::max(pos1.x, std::max(pos2.x, pos3.x)));
            const double miny = std::min(pos0.y, std::min(pos1.y, std::min(pos2.y, pos3.y)));
            const double maxy = std::max(pos0.y, std::max(pos1.y, std::max(pos2.y, pos3.y)));
            const double dx = std::max(1.0, maxx - minx);
            const double dy = std::max(1.0, maxy - miny);
            const size_t numSamples = size_t(dx * dy);

            const float iNumSamples = 1.0f / numSamples;
            float3 c = 0;
            for (size_t sample = 0; sample < numSamples; sample++) {
                // Generate numSamples in our destination pixels and map them to input pixels
                const double2 h = hammersley(uint32_t(sample), iNumSamples);
                const double3 s(dst.getDirectionFor(f, x + h.x, y + h.y));
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

void CubemapUtils::cubemapToEquirectangular(Image& dst, const Cubemap& src) {
    const double w = dst.getWidth();
    const double h = dst.getHeight();
    auto parallelJobTask = [&](size_t j0, size_t count) {
        for (size_t j = j0; j < j0 + count; j++) {
            for (size_t i = 0; i < w; i++) {
                float3 c = 0;
                const size_t numSamples = 64; // TODO: how to chose numsamples
                for (size_t sample = 0; sample < numSamples; sample++) {
                    const double2 u = hammersley(uint32_t(sample), 1.0f / numSamples);
                    double x = 2.0 * (i + u.x) / w - 1.0;
                    double y = 1.0 - 2.0 * (j + u.y) / h;
                    double theta = x * M_PI;
                    double phi = y * M_PI * 0.5;
                    double3 s = {
                            std::cos(phi) * std::sin(theta),
                            std::sin(phi),
                            std::cos(phi) * std::cos(theta) };
                    c += src.filterAt(s);
                }
                Cubemap::writeAt(dst.getPixelRef(i, j), c * (1.0 / numSamples));
            }
        }
    };

    JobSystem& js = getJobSystem();
    auto job = jobs::parallel_for(js, nullptr, 0, uint32_t(h),
            std::ref(parallelJobTask), jobs::CountSplitter<1, 8>());
    js.runAndWait(job);
    js.reset();
}

void CubemapUtils::cubemapToOctahedron(Image& dst, const Cubemap& src) {
    const double w = dst.getWidth();
    const double h = dst.getHeight();
    auto parallelJobTask = [&](size_t j0, size_t count) {
        for (size_t j = j0; j < j0 + count; j++) {
            for (size_t i = 0; i < w; i++) {
                float3 c = 0;
                const size_t numSamples = 64; // TODO: how to chose numsamples
                for (size_t sample = 0; sample < numSamples; sample++) {
                    const double2 u = hammersley(uint32_t(sample), 1.0f / numSamples);
                    double x = 2.0 * (i + u.x) / w - 1.0;
                    double z = 2.0 * (j + u.y) / h - 1.0;
                    double y;
                    if (std::abs(z) > (1.0 - std::abs(x))) {
                        double u = x < 0 ? std::abs(z) - 1 : 1 - std::abs(z);
                        double v = z < 0 ? std::abs(x) - 1 : 1 - std::abs(x);
                        x = u;
                        z = v;
                        y = (std::abs(x) + std::abs(z)) - 1.0;
                    } else {
                        y = 1.0 - (std::abs(x) + std::abs(z));
                    }
                    c += src.filterAt({x, y, z});
                }
                Cubemap::writeAt(dst.getPixelRef(i, j), c * (1.0 / numSamples));
            }
        }
    };

    JobSystem& js = getJobSystem();
    auto job = jobs::parallel_for(js, nullptr, 0, uint32_t(h),
            std::ref(parallelJobTask), jobs::CountSplitter<1, 8>());
    js.runAndWait(job);
    js.reset();
}

void CubemapUtils::crossToCubemap(Cubemap& dst, const Image& src) {
    process<EmptyState>(dst,
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
                        const double2 h = hammersley(uint32_t(i), 1.0f / sampleCount);
                        size_t u = dx + size_t((x + h.x) * dim / dimension);
                        size_t v = dy + size_t((y + h.y) * dim / dimension);
                        Cubemap::writeAt(data, Cubemap::sampleAt(src.getPixelRef(u, v)));
                    }
                }
            });
}


void CubemapUtils::downsampleCubemapLevelBoxFilter(Cubemap& dst, const Cubemap& src) {
    size_t scale = src.getDimensions() / dst.getDimensions();
    process<EmptyState>(dst,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        const Image& image(src.getImageForFace(f));
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            Cubemap::writeAt(data, Cubemap::filterAt(image, x*scale+0.5, y*scale+0.5));
        }
    });
}

// ------------------------------------------------------------------------------------------------

void CubemapUtils::setFaceFromCross(Cubemap& cm, Cubemap::Face face, const Image& image) {
    size_t dim = cm.getDimensions() + 2; // 2 extra per image, for seemlessness
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

    size_t bpr = width * sizeof(Cubemap::Texel);
    bpr = (bpr + 31) & ~31;
    size_t bufSize = bpr * height;
    std::unique_ptr<uint8_t[]> data(new uint8_t[bufSize]);
    memset(data.get(), 0, bufSize);
    Image image(std::move(data), width, height, bpr, sizeof(Cubemap::Texel));
    return image;
}

std::string CubemapUtils::getFaceName(Cubemap::Face face) {
    switch (face) {
        case Cubemap::Face::NX: return "nx";
        case Cubemap::Face::PX: return "px";
        case Cubemap::Face::NY: return "ny";
        case Cubemap::Face::PY: return "py";
        case Cubemap::Face::NZ: return "nz";
        case Cubemap::Face::PZ: return "pz";
    }
}

Cubemap CubemapUtils::create(Image& image, size_t dim, bool horizontal) {
    Cubemap cm(dim);
    Image temp(CubemapUtils::createCubemapImage(dim, horizontal));
    CubemapUtils::setAllFacesFromCross(cm, temp);
    std::swap(image, temp);
    return cm;
}

void CubemapUtils::mirrorCubemap(Cubemap& dst, const Cubemap& src) {
    process<EmptyState>(dst,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            const double3 N(dst.getDirectionFor(f, x, y));
            Cubemap::writeAt(data, src.sampleAt(double3{ -N.x, N.y, N.z }));
        }
    });
}

void CubemapUtils::generateUVGrid(Cubemap& cml, size_t gridFrequencyX, size_t gridFrequencyY) {
    Cubemap::Texel const colors[6] = {
            { 1, 0, 0 }, // -X /  l  - red
            { 1, 1, 1 }, // +X /  r  - white
            { 0, 1, 0 }, // -Y /  b  - green
            { 0, 0, 1 }, // +Y /  t  - blue
            { 1, 0, 1 }, // -Z / bk - magenta
            { 1, 1, 0 }, // +z / fr - yellow
    };
    const float uvGridHDRIntensity = 5.0f;
    size_t gridSizeX = cml.getDimensions() / gridFrequencyX;
    size_t gridSizeY = cml.getDimensions() / gridFrequencyY;
    CubemapUtils::process<CubemapUtils::EmptyState>(cml,
            [ & ](CubemapUtils::EmptyState&,
                    size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
                for (size_t x = 0; x < dim; ++x, ++data) {
                    bool grid = bool(((x / gridSizeX) ^ (y / gridSizeY)) & 1);
                    Cubemap::Texel t = grid ? colors[(int)f] * uvGridHDRIntensity : 0;
                    Cubemap::writeAt(data, t);
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
static inline double sphereQuadrantArea(double x, double y) {
    return std::atan2(x*y, std::sqrt(x*x + y*y + 1));
}

double CubemapUtils::solidAngle(size_t dim, size_t u, size_t v) {
    const double iDim = 1.0f / dim;
    double s = ((u + 0.5) * 2*iDim) - 1;
    double t = ((v + 0.5) * 2*iDim) - 1;
    const double x0 = s - iDim;
    const double y0 = t - iDim;
    const double x1 = s + iDim;
    const double y1 = t + iDim;
    double solidAngle = sphereQuadrantArea(x0, y0) -
                        sphereQuadrantArea(x0, y1) -
                        sphereQuadrantArea(x1, y0) +
                        sphereQuadrantArea(x1, y1);
    return solidAngle;
}
