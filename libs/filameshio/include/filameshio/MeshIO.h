/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FILAMESHIO_MESHIO_H
#define TNT_FILAMENT_FILAMESHIO_MESHIO_H

#include <utils/Entity.h>
#include <utils/Path.h>

#include <map>
#include <string>

namespace filament {
    class Engine;
    class VertexBuffer;
    class IndexBuffer;
    class MaterialInstance;
}

class MeshIO {
public:
    using Callback = void(*)(void* buffer, size_t size, void* user);
    using MaterialRegistry = std::map<std::string, filament::MaterialInstance*>;

    struct Mesh {
        utils::Entity renderable;
        filament::VertexBuffer* vertexBuffer = nullptr;
        filament::IndexBuffer* indexBuffer = nullptr;
    };

    static Mesh loadMeshFromFile(filament::Engine* engine,
            const utils::Path& path,
            const MaterialRegistry& materials);

     static Mesh loadMeshFromBuffer(filament::Engine* engine,
            void const* data, Callback destructor, void* user,
            const MaterialRegistry& materials);

     static Mesh loadMeshFromBuffer(filament::Engine* engine,
            void const* data, Callback destructor, void* user,
            filament::MaterialInstance* defaultMaterial);
};

#endif // TNT_FILAMENT_FILAMESHIO_MESHIO_H
