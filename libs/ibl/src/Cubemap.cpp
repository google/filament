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

#include <ibl/Cubemap.h>

using namespace filament::math;

namespace filament {
namespace ibl {


Cubemap::Cubemap(size_t dim) {
    resetDimensions(dim);
}

Cubemap::~Cubemap() = default;

size_t Cubemap::getDimensions() const {
    return mDimensions;
}

void Cubemap::resetDimensions(size_t dim) {
    mDimensions = dim;
    mScale = 2.0f / dim;
    mUpperBound = std::nextafter((float) mDimensions, 0.0f);
    for (auto& mFace : mFaces) {
        mFace.reset();
    }
}

void Cubemap::setImageForFace(Face face, const Image& image) {
    mFaces[size_t(face)].set(image);
}

Cubemap::Address Cubemap::getAddressFor(const float3& r) {
    Cubemap::Address addr;
    float sc, tc, ma;
    const float rx = std::abs(r.x);
    const float ry = std::abs(r.y);
    const float rz = std::abs(r.z);
    if (rx >= ry && rx >= rz) {
        ma = 1.0f / rx;
        if (r.x >= 0) {
            addr.face = Face::PX;
            sc = -r.z;
            tc = -r.y;
        } else {
            addr.face = Face::NX;
            sc =  r.z;
            tc = -r.y;
        }
    } else if (ry >= rx && ry >= rz) {
        ma = 1.0f / ry;
        if (r.y >= 0) {
            addr.face = Face::PY;
            sc =  r.x;
            tc =  r.z;
        } else {
            addr.face = Face::NY;
            sc =  r.x;
            tc = -r.z;
        }
    } else {
        ma = 1.0f / rz;
        if (r.z >= 0) {
            addr.face = Face::PZ;
            sc =  r.x;
            tc = -r.y;
        } else {
            addr.face = Face::NZ;
            sc = -r.x;
            tc = -r.y;
        }
    }
    // ma is guaranteed to be >= sc and tc
    addr.s = (sc * ma + 1.0f) * 0.5f;
    addr.t = (tc * ma + 1.0f) * 0.5f;
    return addr;
}

void Cubemap::makeSeamless() {
    size_t dim = getDimensions();
    size_t D = dim;

    // here we assume that all faces share the same underlying image
    const size_t bpr = getImageForFace(Face::NX).getBytesPerRow();
    const size_t bpp = getImageForFace(Face::NX).getBytesPerPixel();

    auto getTexel = [](Image& image, ssize_t x, ssize_t y) -> Texel* {
        return (Texel*)((uint8_t*)image.getData() + x * image.getBytesPerPixel() + y * image.getBytesPerRow());
    };

    auto stitch = [ & ](
            Face faceDst, ssize_t xdst, ssize_t ydst, size_t incDst,
            Face faceSrc, size_t xsrc, size_t ysrc, ssize_t incSrc) {
        Image& imageDst = getImageForFace(faceDst);
        Image& imageSrc = getImageForFace(faceSrc);
        Texel* dst = getTexel(imageDst, xdst, ydst);
        Texel* src = getTexel(imageSrc, xsrc, ysrc);
        for (size_t i = 0; i < dim; ++i) {
            *dst = *src;
            dst = (Texel*)((uint8_t*)dst + incDst);
            src = (Texel*)((uint8_t*)src + incSrc);
        }
    };

    auto corners = [ & ](Face face) {
        size_t L = D - 1;
        Image& image = getImageForFace(face);
        *getTexel(image,  -1,  -1) = (*getTexel(image, 0, 0) + *getTexel(image,  -1,  0) + *getTexel(image, 0,    -1)) / 3;
        *getTexel(image, L+1,  -1) = (*getTexel(image, L, 0) + *getTexel(image,   L, -1) + *getTexel(image, L+1,   0)) / 3;
        *getTexel(image,  -1, L+1) = (*getTexel(image, 0, L) + *getTexel(image,  -1,  L) + *getTexel(image, 0,   L+1)) / 3;
        *getTexel(image, L+1, L+1) = (*getTexel(image, L, L) + *getTexel(image, L+1,  L) + *getTexel(image, L+1,   L)) / 3;
    };

    // +Y / Top
    stitch( Face::PY, -1,  0,  bpr, Face::NX,  0,    0,    bpp);      // left
    stitch( Face::PY,  0, -1,  bpp, Face::NZ,  D-1,  0,   -bpp);      // top
    stitch( Face::PY,  D,  0,  bpr, Face::PX,  D-1,  0,   -bpp);      // right
    stitch( Face::PY,  0,  D,  bpp, Face::PZ,  0,    0,    bpp);      // bottom
    corners(Face::PY);

    // -X / Left
    stitch( Face::NX, -1,  0,  bpr, Face::NZ,  D-1,  0,    bpr);      // left
    stitch( Face::NX,  0, -1,  bpp, Face::PY,  0,    0,    bpr);      // top
    stitch( Face::NX,  D,  0,  bpr, Face::PZ,  0,    0,    bpr);      // right
    stitch( Face::NX,  0,  D,  bpp, Face::NY,  0,    D-1, -bpr);      // bottom
    corners(Face::NX);

    // +Z / Front
    stitch( Face::PZ, -1,  0,  bpr, Face::NX,  D-1,  0,    bpr);      // left
    stitch( Face::PZ,  0, -1,  bpp, Face::PY,  0,    D-1,  bpp);      // top
    stitch( Face::PZ,  D,  0,  bpr, Face::PX,  0,    0,    bpr);      // right
    stitch( Face::PZ,  0,  D,  bpp, Face::NY,  0,    0,    bpp);      // bottom
    corners(Face::PZ);

    // +X / Right
    stitch( Face::PX, -1,  0,  bpr, Face::PZ,  D-1,  0,    bpr);      // left
    stitch( Face::PX,  0, -1,  bpp, Face::PY,  D-1,  D-1, -bpr);      // top
    stitch( Face::PX,  D,  0,  bpr, Face::NZ,  0,    0,    bpr);      // right
    stitch( Face::PX,  0,  D,  bpp, Face::NY,  D-1,  0,    bpr);      // bottom
    corners(Face::PX);

    // -Z / Back
    stitch( Face::NZ, -1,  0,  bpr, Face::PX,  D-1,  0,    bpr);      // left
    stitch( Face::NZ,  0, -1,  bpp, Face::PY,  D-1,  0,   -bpp);      // top
    stitch( Face::NZ,  D,  0,  bpr, Face::NX,  0,    0,    bpr);      // right
    stitch( Face::NZ,  0,  D,  bpp, Face::NY,  D-1,  D-1, -bpp);      // bottom
    corners(Face::NZ);

    // -Y / Bottom
    stitch( Face::NY, -1,  0,  bpr, Face::NX,  D-1,  D-1, -bpp);      // left
    stitch( Face::NY,  0, -1,  bpp, Face::PZ,  0,    D-1,  bpp);      // top
    stitch( Face::NY,  D,  0,  bpr, Face::PX,  0,    D-1,  bpp);      // right
    stitch( Face::NY,  0,  D,  bpp, Face::NZ,  D-1,  D-1, -bpp);      // bottom
    corners(Face::NY);
}

Cubemap::Texel Cubemap::filterAt(const Image& image, float x, float y) {
    const size_t x0 = size_t(x);
    const size_t y0 = size_t(y);
    // we allow ourselves to read past the width/height of the Image because the data is valid
    // and contain the "seamless" data.
    size_t x1 = x0 + 1;
    size_t y1 = y0 + 1;

    const float u = float(x - x0);
    const float v = float(y - y0);
    const float one_minus_u = 1 - u;
    const float one_minus_v = 1 - v;
    const Texel& c0 = sampleAt(image.getPixelRef(x0, y0));
    const Texel& c1 = sampleAt(image.getPixelRef(x1, y0));
    const Texel& c2 = sampleAt(image.getPixelRef(x0, y1));
    const Texel& c3 = sampleAt(image.getPixelRef(x1, y1));
    return (one_minus_u*one_minus_v)*c0 + (u*one_minus_v)*c1 + (one_minus_u*v)*c2 + (u*v)*c3;
}

Cubemap::Texel Cubemap::filterAtCenter(const Image& image, size_t x0, size_t y0) {
    // we allow ourselves to read past the width/height of the Image because the data is valid
    // and contain the "seamless" data.
    size_t x1 = x0 + 1;
    size_t y1 = y0 + 1;
    const Texel& c0 = sampleAt(image.getPixelRef(x0, y0));
    const Texel& c1 = sampleAt(image.getPixelRef(x1, y0));
    const Texel& c2 = sampleAt(image.getPixelRef(x0, y1));
    const Texel& c3 = sampleAt(image.getPixelRef(x1, y1));
    return (c0 + c1 + c2 + c3) * 0.25f;
}

Cubemap::Texel Cubemap::trilinearFilterAt(const Cubemap& l0, const Cubemap& l1, float lerp,
        const float3& L)
{
    Cubemap::Address addr(getAddressFor(L));
    const Image& i0 = l0.getImageForFace(addr.face);
    const Image& i1 = l1.getImageForFace(addr.face);
    float x0 = std::min(addr.s * l0.mDimensions, l0.mUpperBound);
    float y0 = std::min(addr.t * l0.mDimensions, l0.mUpperBound);
    float x1 = std::min(addr.s * l1.mDimensions, l1.mUpperBound);
    float y1 = std::min(addr.t * l1.mDimensions, l1.mUpperBound);
    float3 c0 = filterAt(i0, x0, y0);
    c0 += lerp * (filterAt(i1, x1, y1) - c0);
    return c0;
}

} // namespace ibl
} // namespace filament
