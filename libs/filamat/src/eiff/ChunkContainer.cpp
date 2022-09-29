/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "ChunkContainer.h"
#include "Flattener.h"

namespace filamat {

// This call is relatively expensive since it performs a dry run of the flattering process,
// using a flattener that will calculate offets but will not write. It should be used only once
// when the container is about to be flattened.

size_t ChunkContainer::getSize() const {
    return flatten(Flattener::getDryRunner());
}

size_t ChunkContainer::flatten(Flattener& f) const {
    for (const auto& chunk: mChildren) {
        f.writeUint64(static_cast<uint64_t>(chunk->getType()));
        f.writeSizePlaceholder();
        chunk->flatten(f);
        f.writeSize();
    }
    return f.getBytesWritten();
}

}
