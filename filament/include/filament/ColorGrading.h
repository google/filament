/*
 * Copyright (C) 2020 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_COLOR_GRADING_H
#define TNT_FILAMENT_COLOR_GRADING_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

namespace filament {

class Engine;
class FColorGrading;

class UTILS_PUBLIC ColorGrading : public FilamentAPI {
    struct BuilderDetails;
public:
    enum class ToneMapping : uint8_t {
        LINEAR        = 0,     //!< Linear tone mapping (i.e. no tone mapping)
        ACES          = 1,     //!< ACES tone mapping (ODT+RRT in )
        FILMIC        = 2,     //!< Filmic tone mapping, modelled after ACES but applied in sRGB space
        REINHARD      = 3,     //!< Reinhard luma-based tone mapping
        DISPLAY_RANGE = 4,     //!< Debug tone mapping to validate scene exposure
    };

    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& toneMapping(ToneMapping toneMapping) noexcept;

        ColorGrading* build(Engine& engine);

    private:
        friend class FColorGrading;
    };
};

} // namespace filament

#endif // TNT_FILAMENT_COLOR_GRADING_H
