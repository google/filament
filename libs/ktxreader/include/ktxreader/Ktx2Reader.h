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

#ifndef KTXREADER_KTX2READER_H
#define KTXREADER_KTX2READER_H

#include <cstdint>
#include <cstddef>

#include <filament/Texture.h>

#include <utils/FixedCapacityVector.h>

namespace filament {
    class Engine;
}

namespace basist {
    class ktx2_transcoder;
}

namespace ktxreader {

class Ktx2Reader {
    public:
        using Engine = filament::Engine;
        using Texture = filament::Texture;
        enum TransferFunction { LINEAR, sRGB };

        Ktx2Reader(Engine& engine, bool quiet = false);
        ~Ktx2Reader();

        /**
         * Requests that the reader constructs Filament textures with given internal format.
         *
         * This MUST be called at least once before calling load().
         *
         * As a reminder, a basis-encoded KTX2 can be quickly transcoded to any number of formats,
         * so you need to tell it what formats your hw supports. That's why this method exists.
         *
         * Call requestFormat as many times as needed; formats that are submitted early are
         * considered higher priority.
         *
         * If BasisU knows a priori that the given format is not available (e.g. if the build has
         * disabled it), this returns false and the format is not added to the list.
         *
         * Returns false if the given format has already been requested.
         *
         * Hint: BasisU supports the following uncompressed formats: RGBA8, RGB565, RGBA4.
         */
        bool requestFormat(Texture::InternalFormat format);

        /**
         * Removes the given format from the list, or does nothing if it hasn't been requested.
         */
        void unrequestFormat(Texture::InternalFormat format);

        /**
         * Attempts to create a Filament texture from the given KTX2 blob. If none of the requested
         * formats can be extracted from the data, this returns null.
         */
        Texture* load(const uint8_t* data, size_t size, TransferFunction transfer = LINEAR);

    private:
        Ktx2Reader(const Ktx2Reader&) = delete;
        Ktx2Reader& operator=(const Ktx2Reader&) = delete;
        Ktx2Reader(Ktx2Reader&& that) noexcept = delete;
        Ktx2Reader& operator=(Ktx2Reader&& that) noexcept = delete;

        Engine& mEngine;
        bool mQuiet;
        basist::ktx2_transcoder* const mTranscoder;

        utils::FixedCapacityVector<Texture::InternalFormat> mRequestedFormats;
};

} // namespace ktxreader

#endif
