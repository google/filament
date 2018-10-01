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

#ifndef SRC_CUBEMAP_H_
#define SRC_CUBEMAP_H_

#include <algorithm>

#include <math/vec4.h>
#include <math/vec3.h>
#include <math/vec2.h>

#include <utils/compiler.h>

#include "Image.h"
#include "utilities.h"

class Cubemap {
public:
    explicit Cubemap(size_t dim);
    Cubemap(Cubemap&&) = default;
    ~Cubemap();

    Cubemap& operator = (Cubemap&&) = default;

    enum class Face : uint8_t {
        NX = 0,     // left            +----+
        PX,         // right           | PY |
        NY,         // bottom     +----+----+----+----+
        PY,         // top        | NX | PZ | PX | NZ |
        NZ,         // back       +----+----+----+----+
        PZ          // front           | NY |
                    //                 +----+
    };

    typedef math::float3 Texel;


    void resetDimensions(size_t dim);

    void setImageForFace(Face face, const Image& image);
    inline const Image& getImageForFace(Face face) const;
    inline Image& getImageForFace(Face face);

    inline math::double2 center(size_t x, size_t y) const;

    inline math::double3 getDirectionFor(Face face, size_t x, size_t y) const;
    inline math::double3 getDirectionFor(Face face, double x, double y) const;

    inline Texel const& sampleAt(const math::double3& direction) const;
    inline Texel        filterAt(const math::double3& direction) const;

    static Texel filterAt(const Image& image, double x, double y);

    static Texel trilinearFilterAt(const Cubemap& c0, const Cubemap& c1, double lerp,
            const math::double3& direction);

    inline static const Texel& sampleAt(void const* data) {
        return *static_cast<Texel const *>(data);
    }

    inline static void writeAt(void* data, const Texel& texel) {
        *static_cast<Texel*>(data) = texel;
    }

    size_t getDimensions() const;

    void makeSeamless();

    struct Address {
        Face face;
        double s = 0;
        double t = 0;
    };

    // Note: this doesn't apply the Image's flips
    // (this is why this is private)
    static Address getAddressFor(const math::double3& direction);

private:
    size_t mDimensions = 0;
    double mScale = 1;
    double mUpperBound = 0;
    Image mFaces[6];
};

inline const Image& Cubemap::getImageForFace(Face face) const {
    return mFaces[int(face)];
}

inline Image& Cubemap::getImageForFace(Face face) {
    return mFaces[int(face)];
}

inline math::double2 Cubemap::center(size_t x, size_t y) const {
    // map [0, dim] to [-1,1] with (-1,-1) at bottom left
    math::double2 c(x+0.5, y+0.5);
    return c;
}

inline math::double3 Cubemap::getDirectionFor(Face face, size_t x, size_t y) const {
    return getDirectionFor(face, x+0.5, y+0.5);
}

inline math::double3 Cubemap::getDirectionFor(Face face, double x, double y) const {
    // map [0, dim] to [-1,1] with (-1,-1) at bottom left
    double cx = (x * mScale) - 1;
    double cy = 1 - (y * mScale);

    math::double3 dir;
    const double l = std::sqrt(cx*cx + cy*cy + 1);
    switch (face) {
        case Face::PX:  dir = {   1, cy, -cx }; break;
        case Face::NX:  dir = {  -1, cy,  cx }; break;
        case Face::PY:  dir = {  cx,  1, -cy }; break;
        case Face::NY:  dir = {  cx, -1,  cy }; break;
        case Face::PZ:  dir = {  cx, cy,   1 }; break;
        case Face::NZ:  dir = { -cx, cy,  -1 }; break;
    }
    return dir * (1 / l);
}

inline Cubemap::Texel const& Cubemap::sampleAt(const math::double3& direction) const {
    Cubemap::Address addr(getAddressFor(direction));
    const size_t x = std::min(size_t(addr.s * mDimensions), mDimensions-1);
    const size_t y = std::min(size_t(addr.t * mDimensions), mDimensions-1);
    return sampleAt(getImageForFace(addr.face).getPixelRef(x, y));
}

inline Cubemap::Texel Cubemap::filterAt(const math::double3& direction) const {
    Cubemap::Address addr(getAddressFor(direction));
    addr.s = std::min(addr.s * mDimensions, mUpperBound);
    addr.t = std::min(addr.t * mDimensions, mUpperBound);
    return filterAt(getImageForFace(addr.face), addr.s, addr.t);
}

#endif /* SRC_CUBEMAP_H_ */
