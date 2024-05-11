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

#ifndef IBL_CUBEMAP_H
#define IBL_CUBEMAP_H

#include <ibl/Image.h>

#include <utils/compiler.h>

#include <math/vec4.h>
#include <math/vec3.h>
#include <math/vec2.h>

#include <algorithm>

namespace filament {
namespace ibl {

/**
 * Generic cubemap class. It handles writing / reading into the 6 faces of a cubemap.
 *
 * Seamless trilinear filtering is handled.
 *
 * This class doesn't own the face data, it's just a "view" on the 6 images.
 *
 * @see CubemapUtils
 *
 */
class UTILS_PUBLIC Cubemap {
public:

    /**
     *  Initialize the cubemap with a given size, but no face is set and no memory is allocated.
     *
     *  Usually Cubemaps are created using CubemapUtils.
     *
     * @see CubemapUtils
     */
    explicit Cubemap(size_t dim);

    Cubemap(Cubemap&&) = default;
    Cubemap& operator=(Cubemap&&) = default;

    ~Cubemap();


    enum class Face : uint8_t {
        PX = 0,     // left            +----+
        NX,         // right           | PY |
        PY,         // bottom     +----+----+----+----+
        NY,         // top        | NX | PZ | PX | NZ |
        PZ,         // back       +----+----+----+----+
        NZ          // front           | NY |
                    //                 +----+
    };

    using Texel = filament::math::float3;


    //! releases all images and reset the cubemap size
    void resetDimensions(size_t dim);

    //! assigns an image to a face.
    void setImageForFace(Face face, const Image& image);

    //! retrieves the image attached to a face
    inline const Image& getImageForFace(Face face) const;

    //! retrieves the image attached to a face
    inline Image& getImageForFace(Face face);

    //! computes the center of a pixel at coordinate x, y
    static inline filament::math::float2 center(size_t x, size_t y);

    //! computes a direction vector from a face and a location of the center of pixel in an Image
    inline filament::math::float3 getDirectionFor(Face face, size_t x, size_t y) const;

    //! computes a direction vector from a face and a location in pixel in an Image
    inline filament::math::float3 getDirectionFor(Face face, float x, float y) const;

    //! samples the cubemap at the given direction using nearest neighbor filtering
    inline Texel const& sampleAt(const filament::math::float3& direction) const;

    //! samples the cubemap at the given direction using bilinear filtering
    inline Texel filterAt(const filament::math::float3& direction) const;

    //! samples an image at the given location in pixel using bilinear filtering
    static Texel filterAt(const Image& image, float x, float y);
    static Texel filterAtCenter(const Image& image, size_t x, size_t y);

    //! samples two cubemaps in a given direction and lerps the result by a given lerp factor
    static Texel trilinearFilterAt(const Cubemap& c0, const Cubemap& c1, float lerp,
            const filament::math::float3& direction);

    //! reads a texel at a given address
    inline static const Texel& sampleAt(void const* data) {
        return *static_cast<Texel const*>(data);
    }

    //! writes a texel at a given address
    inline static void writeAt(void* data, const Texel& texel) {
        *static_cast<Texel*>(data) = texel;
    }

    //! returns the size of the cubemap in pixels
    size_t getDimensions() const;

    /**
     * Prepares a cubemap for seamless access to its faces.
     *
     * @warning All faces of the cubemap must be backed-up by the same Image, and must already
     * be spaced by 2 lines/rows.
     */
    void makeSeamless();

    struct Address {
        Face face;
        float s = 0;
        float t = 0;
    };

    //! returns the face and texture coordinates of the given direction
    static Address getAddressFor(const filament::math::float3& direction);

private:
    size_t mDimensions = 0;
    float mScale = 1;
    float mUpperBound = 0;
    Image mFaces[6];
};

// ------------------------------------------------------------------------------------------------

inline const Image& Cubemap::getImageForFace(Face face) const {
    return mFaces[int(face)];
}

inline Image& Cubemap::getImageForFace(Face face) {
    return mFaces[int(face)];
}

inline filament::math::float2 Cubemap::center(size_t x, size_t y) {
    return { x + 0.5f, y + 0.5f };
}

inline filament::math::float3 Cubemap::getDirectionFor(Face face, size_t x, size_t y) const {
    return getDirectionFor(face, x + 0.5f, y + 0.5f);
}

inline filament::math::float3 Cubemap::getDirectionFor(Face face, float x, float y) const {
    // map [0, dim] to [-1,1] with (-1,-1) at bottom left
    float cx = (x * mScale) - 1;
    float cy = 1 - (y * mScale);

    filament::math::float3 dir;
    const float l = std::sqrt(cx * cx + cy * cy + 1);
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

inline Cubemap::Texel const& Cubemap::sampleAt(const filament::math::float3& direction) const {
    Cubemap::Address addr(getAddressFor(direction));
    const size_t x = std::min(size_t(addr.s * mDimensions), mDimensions - 1);
    const size_t y = std::min(size_t(addr.t * mDimensions), mDimensions - 1);
    return sampleAt(getImageForFace(addr.face).getPixelRef(x, y));
}

inline Cubemap::Texel Cubemap::filterAt(const filament::math::float3& direction) const {
    Cubemap::Address addr(getAddressFor(direction));
    addr.s = std::min(addr.s * mDimensions, mUpperBound);
    addr.t = std::min(addr.t * mDimensions, mUpperBound);
    return filterAt(getImageForFace(addr.face), addr.s, addr.t);
}

} // namespace ibl
} // namespace filament

#endif /* IBL_CUBEMAP_H */
