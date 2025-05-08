/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_BUFFEROBJECTSTREAMDESCRIPTOR_H
#define TNT_FILAMENT_BACKEND_BUFFEROBJECTSTREAMDESCRIPTOR_H

#include <backend/Handle.h>

#include <vector>

namespace filament::backend {

/**
 * The type of association between a buffer object and a stream.
 */
 enum class BufferObjectStreamAssociationType { TRANSFORM_MATRIX };

/**
 * A descriptor for a buffer object to stream association.
 */
class UTILS_PUBLIC BufferObjectStreamDescriptor {
public:
    //! creates an empty descriptor
    BufferObjectStreamDescriptor() noexcept = default;

    BufferObjectStreamDescriptor(const BufferObjectStreamDescriptor& rhs) = delete;
    BufferObjectStreamDescriptor& operator=(const BufferObjectStreamDescriptor& rhs) = delete;

    BufferObjectStreamDescriptor(BufferObjectStreamDescriptor&& rhs) = default;
    BufferObjectStreamDescriptor& operator=(BufferObjectStreamDescriptor&& rhs) = default;

    // --------------------------------------------------------------------------------------------

    struct StreamDataDescriptor {
        uint32_t offset;
        Handle<HwStream> stream;
        BufferObjectStreamAssociationType associationType;
    };

    std::vector<StreamDataDescriptor> mStreams;
};

} // namespace filament::backend

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out,
        const filament::backend::BufferObjectStreamDescriptor& b);
#endif

#endif // TNT_FILAMENT_BACKEND_BUFFEROBJECTSTREAMDESCRIPTOR_H
