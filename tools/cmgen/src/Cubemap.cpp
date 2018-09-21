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

#include "Cubemap.h"

using namespace math;

Cubemap::Cubemap(size_t dim) {
    resetDimensions(dim);
}

Cubemap::~Cubemap() {
}

size_t Cubemap::getDimensions() const {
    return mDimensions;
}

void Cubemap::resetDimensions(size_t dim) {
    mDimensions = dim;
    mScale = 2.0 / dim;
    mUpperBound = std::nextafter(mDimensions, 0);
    for (size_t i=0 ; i<6 ; i++) {
        mFaces[i].reset();
    }
}

void Cubemap::setImageForFace(Face face, const Image& image) {
    mFaces[size_t(face)].set(image);
}

Cubemap::Address Cubemap::getAddressFor(const double3& r) {
    Cubemap::Address addr;
    double sc, tc, ma;
    const double rx = std::abs(r.x);
    const double ry = std::abs(r.y);
    const double rz = std::abs(r.z);
    if (rx >= ry && rx >= rz) {
        ma = rx;
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
        ma = ry;
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
        ma = rz;
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
    addr.s = (sc / ma + 1) * 0.5f;
    addr.t = (tc / ma + 1) * 0.5f;
    return addr;
}
/*
 * We handle "seamless" cubemaps by duplicating a row to the bottom, or column to the right
 * of each faces that don't have an adjacent face in the image (the duplicate is taken from the
 * adjacent face in the cubemap).
 * This is because when accessing an image with bilinear filtering, we always overshoot to the
 * right or bottom. This works well with cubemaps stored as a cross in memory.
 */
void Cubemap::makeSeamless() {
    Geometry geometry = mGeometry;
    size_t dim = getDimensions();
    auto stitch = [ & ](void* dst, size_t incDst, void const* src, ssize_t incSrc) {
        for (size_t i = 0; i < dim; ++i) {
            *(Texel*)dst = *(Texel*)src;
            dst = ((uint8_t*)dst + incDst);
            src = ((uint8_t*)src + incSrc);
        }
    };

    const size_t bpr = getImageForFace(Face::NX).getBytesPerRow();
    const size_t bpp = getImageForFace(Face::NX).getBytesPerPixel();

    if (geometry == Geometry::HORIZONTAL_CROSS ||
        geometry == Geometry::VERTICAL_CROSS) {
        stitch(  getImageForFace(Face::NX).getPixelRef(0, dim), bpp,
                getImageForFace(Face::NY).getPixelRef(0, dim - 1), -bpr);

        stitch(  getImageForFace(Face::PY).getPixelRef(dim, 0), bpr,
                getImageForFace(Face::PX).getPixelRef(dim - 1, 0), -bpp);

        stitch(  getImageForFace(Face::PX).getPixelRef(0, dim), bpp,
                getImageForFace(Face::NY).getPixelRef(dim - 1, 0), bpr);

        stitch(  getImageForFace(Face::NY).getPixelRef(dim, 0), bpr,
                getImageForFace(Face::PX).getPixelRef(0, dim - 1), bpp);

        if (geometry == Geometry::HORIZONTAL_CROSS) { // horizontal cross
            stitch(  getImageForFace(Face::NZ).getPixelRef(0, dim), bpp,
                    getImageForFace(Face::NY).getPixelRef(dim - 1, dim - 1), -bpp);

            stitch(  getImageForFace(Face::NZ).getPixelRef(dim, 0), bpr,
                    getImageForFace(Face::NX).getPixelRef(0, 0), bpr);

            stitch(  getImageForFace(Face::NY).getPixelRef(0, dim), bpp,
                    getImageForFace(Face::NZ).getPixelRef(dim - 1, dim - 1), -bpp);

        } else {
            stitch(  getImageForFace(Face::NZ).getPixelRef(0, dim), bpp,
                    getImageForFace(Face::PY).getPixelRef(0, dim - 1), bpp);

            stitch(  getImageForFace(Face::NZ).getPixelRef(dim, 0), bpr,
                    getImageForFace(Face::PX).getPixelRef(dim - 1, dim - 1), -bpr);

            stitch(  getImageForFace(Face::PX).getPixelRef(dim, 0), bpr,
                    getImageForFace(Face::NZ).getPixelRef(dim - 1, dim - 1), -bpr);
        }
    }
}

Cubemap::Texel Cubemap::filterAt(const Image& image, double x, double y) {
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

Cubemap::Texel Cubemap::trilinearFilterAt(const Cubemap& l0, const Cubemap& l1, double lerp,
        const double3& L)
{
    Cubemap::Address addr(getAddressFor(L));
    const Image& i0 = l0.getImageForFace(addr.face);
    double x0 = std::min(addr.s * l0.mDimensions, l0.mUpperBound);
    double y0 = std::min(addr.t * l0.mDimensions, l0.mUpperBound);
    float3 c0(filterAt(i0, x0, y0));
    if (&l0 != &l1) {
        const Image& i1 = l1.getImageForFace(addr.face);
        double x1 = std::min(addr.s * l1.mDimensions, l1.mUpperBound);
        double y1 = std::min(addr.t * l1.mDimensions, l1.mUpperBound);
        c0 += lerp * (filterAt(i1, x1, y1) - c0);
    }
    return c0;
}
