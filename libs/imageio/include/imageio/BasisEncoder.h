/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef IMAGE_BASISENCODER_H_
#define IMAGE_BASISENCODER_H_

#include <stddef.h>
#include <stdint.h>
#include <utils/compiler.h>

#include <image/LinearImage.h>

namespace image {

struct BasisEncoderBuilderImpl;
struct BasisEncoderImpl;

class UTILS_PUBLIC BasisEncoder {
public:
    enum IntermediateFormat {
        UASTC,
        ETC1S,
    };

    class Builder {
    public:
        // The Ktx2 builder is constructed with a fixed number of miplevels and layers so
        // that it can pre-allocate the appropriate BasisU input vectors.
        //
        // - mipCount: number of mipmap levels, including the base; must be at least 1.
        // - layerCount: either 1  or the number of layers in an array texture.
        //
        // For cubemaps and cubemap arrays, multiply the layer count by 6 and pack the faces in
        // standard GL order.
        Builder(size_t mipCount, size_t layerCount) noexcept;

        ~Builder() noexcept;
        Builder(Builder&& that) noexcept;
        Builder& operator=(Builder&& that) noexcept;

        // Enables the linear flag, which does two things:
        // (1) Specifies that the image should be encoded without a transfer function.
        // (2) Adds a tag to the ktx file that tells the loader that no transfer function was used.
        //
        // Note that the tag does not actually affect the compression process, it's basically just a
        // hint to the reader. At the time of this writing, BasisU does not make a distinction
        // between sRGB targets and linear targets.
        //
        // default value: FALSE
        Builder& linear(bool enabled) noexcept;

        // Enables cubemap (or cubemap array) mode. When this is enabled the number of layers
        // should be divisible by 6.
        // default value: FALSE
        Builder& cubemap(bool enabled) noexcept;

        // Chooses the intermiediate format as described in the BasisU documentation.
        // For highest quality, use UASTC.
        // default value: UASTC
        Builder& intermediateFormat(IntermediateFormat format) noexcept;

        // Honors only the first component of the incoming LinearImage.
        // default value: FALSE
        Builder& grayscale(bool enabled) noexcept;

        // Transforms the incoming image from [-1, +1] to [0, 1] before passing it to the encoder.
        // default value: FALSE
        Builder& normals(bool enabled) noexcept;

        // Initializes the basis encoder with the given number of jobs.
        // default value: 4
        Builder& jobs(size_t count) noexcept;

        // Supresses status messages.
        // default value: FALSE
        Builder& quiet(bool enabled) noexcept;

        // Submits image data in linear floating-point format.
        // This must be called for every miplevel.
        Builder& miplevel(size_t mipIndex, size_t layerIndex, const LinearImage& image) noexcept;

        // Creates a BasisU encoder and returns null if an error occurred.
        BasisEncoder* build();

    private:
        BasisEncoderBuilderImpl* mImpl;
        Builder(const Builder&) = delete;
        Builder& operator=(const Builder&) = delete;
    };

    ~BasisEncoder() noexcept;
    BasisEncoder(BasisEncoder&& that) noexcept;
    BasisEncoder& operator=(BasisEncoder&& that) noexcept;

    // Triggers compression of all miplevels and waits until all jobs are done.
    // Returns false if an error occurred. The resulting KTX2 contents can be retrieved
    // using the getters below.
    bool encode();

    size_t getKtx2ByteCount() const noexcept;
    uint8_t const* getKtx2Data() const noexcept;

private:
    BasisEncoder(BasisEncoderImpl*) noexcept;
    BasisEncoder(const BasisEncoder&) = delete;
    BasisEncoder& operator=(const BasisEncoder&) = delete;
    BasisEncoderImpl* mImpl;
    friend struct BasisEncoderBuilderImpl;
};

} // namespace image

#endif // IMAGE_BASISENCODER_H_
