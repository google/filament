/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_BINDINGMAP_H
#define TNT_FILAMENT_BACKEND_OPENGL_BINDINGMAP_H

#include <backend/DriverEnums.h>

#include "gl_headers.h"

#include <utils/bitset.h>
#include <utils/debug.h>

#include <new>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace filament::backend {

class BindingMap {
    struct CompressedBinding {
        // this is in fact a GLuint, but we only want 8-bits
        uint8_t binding : 7;
        uint8_t sampler : 1;
    };

    CompressedBinding (*mStorage)[MAX_DESCRIPTOR_COUNT];

    utils::bitset64 mActiveDescriptors[MAX_DESCRIPTOR_SET_COUNT];

public:
    BindingMap() noexcept
            : mStorage(new (std::nothrow) CompressedBinding[MAX_DESCRIPTOR_SET_COUNT][MAX_DESCRIPTOR_COUNT]) {
#ifndef NDEBUG
        memset(mStorage, 0xFF, sizeof(CompressedBinding[MAX_DESCRIPTOR_SET_COUNT][MAX_DESCRIPTOR_COUNT]));
#endif
    }

    ~BindingMap() noexcept {
        delete [] mStorage;
    }

    BindingMap(BindingMap const&) noexcept = delete;
    BindingMap(BindingMap&&) noexcept = delete;
    BindingMap& operator=(BindingMap const&) noexcept = delete;
    BindingMap& operator=(BindingMap&&) noexcept = delete;

    struct Binding {
        GLuint binding;
        DescriptorType type;
    };

    void insert(descriptor_set_t set, descriptor_binding_t binding, Binding entry) noexcept {
        assert_invariant(set < MAX_DESCRIPTOR_SET_COUNT);
        assert_invariant(binding < MAX_DESCRIPTOR_COUNT);
        assert_invariant(entry.binding < 128); // we reserve 1 bit for the type right now
        mStorage[set][binding] = { (uint8_t)entry.binding,
                                   entry.type == DescriptorType::SAMPLER ||
                                   entry.type == DescriptorType::SAMPLER_EXTERNAL };
        mActiveDescriptors[set].set(binding);
    }

    GLuint get(descriptor_set_t set, descriptor_binding_t binding) const noexcept {
        assert_invariant(set < MAX_DESCRIPTOR_SET_COUNT);
        assert_invariant(binding < MAX_DESCRIPTOR_COUNT);
        return mStorage[set][binding].binding;
    }

    utils::bitset64 getActiveDescriptors(descriptor_set_t set) const noexcept {
        return mActiveDescriptors[set];
    }
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_BINDINGMAP_H
