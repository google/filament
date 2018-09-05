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

#include <string.h>

#include <math/mat4.h>

using namespace math;
using namespace image;

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

    process<EmptyState>(dst,
            [&](EmptyState&, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
        for (size_t x=0 ; x<dim ; ++x, ++data) {
            // calculate how many samples we need based on dx, dy in the source
            // x =  cos(phi) sin(theta)
            // y = -sin(phi)
            // z = -cos(phi) cos(theta)
            const double3 s0(dst.getDirectionFor(f, x, y));
            const double t0 = std::atan2(s0.x, -s0.z);
            const double p0 = std::asin(s0.y);
            const double3 s1(dst.getDirectionFor(f, x+1, y+1));
            const double t1 = std::atan2(s1.x, -s1.z);
            const double p1 = std::asin(s1.y);
            const double dt = std::abs(t1 - t0);
            const double dp = std::abs(p1 - p0);
            const double dx = std::abs(r*dt);
            const double dy = std::abs(r*dp*s0.y);
            const size_t numSamples = (size_t const)std::ceil(std::max(dx, dy));
            const float iNumSamples = 1.0f / numSamples;

            float3 c = 0;
            for (size_t sample = 0; sample < numSamples; sample++) {
                // Generate numSamples in our destination pixels and map them to input pixels
                const double2 h = hammersley(uint32_t(sample), iNumSamples);
                const double3 s(dst.getDirectionFor(f, x + h.x, y + h.y));
                float xf = float(std::atan2(s.x, -s.z) * M_1_PI);   // range [-1.0, 1.0]
                float yf = float(std::asin(-s.y) * (2 * M_1_PI));   // range [-1.0, 1.0]
                xf = (xf + 1) * 0.5f * (width -1);           // range [0, width [
                yf = (yf + 1) * 0.5f * (height-1);           // range [0, height[
                // we can't use filterAt() here because it reads past the width/height
                // which is okay for cubmaps but not for square images
                c += Cubemap::sampleAt(src.getPixelRef((uint32_t)xf, (uint32_t)yf));
            }
            c *= iNumSamples;
            Cubemap::writeAt(data, c);
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
    size_t dim = cm.getDimensions();
    size_t x = 0;
    size_t y = 0;
    uint32_t flags = 0;
    switch (face) {
        case Cubemap::Face::NX:
            x = 0, y = dim;
            break;
        case Cubemap::Face::PX:
            x = 2*dim, y = dim;
            break;
        case Cubemap::Face::NY:
            x = dim, y = 2*dim;
            break;
        case Cubemap::Face::PY:
            x = dim, y = 0;
            break;
        case Cubemap::Face::NZ:
            if (image.getHeight() > image.getWidth()) {
                x = dim, y = 3*dim;
                flags = Image::FLIP_XY;
            } else {
                x = 3*dim, y = dim;
            }
            break;
        case Cubemap::Face::PZ:
            x = dim, y = dim;
            break;
    }
    Image subImage;
    subImage.subset(image, x, y, dim, dim, flags);
    cm.setImageForFace(face, subImage);
}

void CubemapUtils::setAllFacesFromCross(Cubemap& cm, const Image& image) {
    cm.setGeometry(image.getHeight() > image.getWidth()
                   ? Cubemap::Geometry::VERTICAL_CROSS
                   : Cubemap::Geometry::HORIZONTAL_CROSS);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::NX, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::PX, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::NY, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::PY, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::NZ, image);
    CubemapUtils::setFaceFromCross(cm, Cubemap::Face::PZ, image);
}

Image CubemapUtils::createCubemapImage(size_t dim, bool horizontal) {
    size_t width = 4 * dim;
    size_t height = 3 * dim;
    if (!horizontal) {
        std::swap(width, height);
    }

    // always allocate an extra column and row, to allow the cubemap to be "seamless"
    size_t bpr = (width + 1) * sizeof(Cubemap::Texel);
    bpr = (bpr + 31) & ~31;
    size_t bufSize = bpr * (height + 1);
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

void CubemapUtils::copyImage(Image& dst, const Image& src) {
    assert(dst.getWidth() >= src.getWidth() && dst.getHeight() >= src.getHeight());
    for (size_t y = 0, my = src.getHeight(); y < my; ++y) {
        memcpy(dst.getPixelRef(0, y), src.getPixelRef(0, y), src.getBytesPerRow());
    }
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

void CubemapUtils::generateUVGrid(Cubemap const& cml, size_t gridFrequency) {
    Cubemap::Texel const colors[6] = {
            { 1, 0, 0 }, // l
            { 0, 1, 1 }, // r
            { 0, 1, 0 }, // b
            { 1, 0, 1 }, // t
            { 0, 0, 1 }, // bk
            { 1, 1, 0 }, // fr
    };
    size_t gridSize = cml.getDimensions() / gridFrequency;
    CubemapUtils::process<CubemapUtils::EmptyState>(cml,
            [ & ](CubemapUtils::EmptyState&,
                    size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t dim) {
                for (size_t x = 0; x < dim; ++x, ++data) {
                    bool grid = bool(((x / gridSize) ^ (y / gridSize)) & 1);
                    Cubemap::Texel t = grid ? colors[(int)f] : 0;
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
