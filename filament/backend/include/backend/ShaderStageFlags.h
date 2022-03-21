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

#ifndef TNT_FILAMENT_BACKEND_SHADERSTAGEFLAGS_H
#define TNT_FILAMENT_BACKEND_SHADERSTAGEFLAGS_H

#include <backend/DriverEnums.h>

namespace filament::backend {

struct ShaderStageFlags {
    bool vertex : 1;
    bool fragment : 1;
    bool hasShaderType(ShaderType type) const {
        return (vertex && type == ShaderType::VERTEX) ||
               (fragment && type == ShaderType::FRAGMENT);
    }
};

constexpr ShaderStageFlags ALL_SHADER_STAGE_FLAGS = { .vertex = true, .fragment = true };

inline utils::io::ostream& operator<<(utils::io::ostream& stream, ShaderStageFlags stageFlags) {
    stream << "{ ";
    if (stageFlags.vertex) {
        stream << "vertex";
    }
    if (stageFlags.fragment) {
        if (stageFlags.vertex)
            stream << " | ";
        stream << "fragment";
    }
    stream << " }";
    return stream;
}
} // namespace filament::backend


#endif // TNT_FILAMENT_BACKEND_SHADERSTAGEFLAGS_H
