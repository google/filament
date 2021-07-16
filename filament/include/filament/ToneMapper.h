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

#ifndef TNT_FILAMENT_TONE_MAPPER_H
#define TNT_FILAMENT_TONE_MAPPER_H

#include <utils/compiler.h>

#include <math/mathfwd.h>

namespace filament {

/**
 *
 */
struct UTILS_PUBLIC ToneMapper {
    ToneMapper() noexcept;
    virtual ~ToneMapper() noexcept;

    virtual math::float3 operator()(math::float3 c) const noexcept = 0;
};

struct UTILS_PUBLIC LinearToneMapper final : public ToneMapper {
    LinearToneMapper() noexcept;
    ~LinearToneMapper() noexcept final;

    math::float3 operator()(math::float3 c) const noexcept;
};

struct UTILS_PUBLIC ACESToneMapper final : public ToneMapper {
    ACESToneMapper() noexcept;
    ~ACESToneMapper() noexcept final;

    math::float3 operator()(math::float3 c) const noexcept;
};

struct UTILS_PUBLIC ACESLegacyToneMapper final : public ToneMapper {
    ACESLegacyToneMapper() noexcept;
    ~ACESLegacyToneMapper() noexcept final;

    math::float3 operator()(math::float3 c) const noexcept;
};

struct UTILS_PUBLIC FilmicToneMapper final : public ToneMapper {
    FilmicToneMapper() noexcept;
    ~FilmicToneMapper() noexcept final;

    math::float3 operator()(math::float3 x) const noexcept;
};

struct UTILS_PUBLIC DisplayRangeToneMapper final : public ToneMapper {
    DisplayRangeToneMapper() noexcept;
    ~DisplayRangeToneMapper() noexcept;

    math::float3 operator()(math::float3 c) const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_TONE_MAPPER_H
