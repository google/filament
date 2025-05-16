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

#include <utils/FixedCapacityVector.h>
#include <utils/compiler.h>
#include <utils/Panic.h>

#include <stddef.h>
#include <stdlib.h>

namespace utils {

void FixedCapacityVectorBase::capacityCheckFailed(size_t const capacity, size_t const size) {
    UTILS_ASSUME(capacity < size);
    FILAMENT_CHECK_PRECONDITION(capacity >= size)
             << "capacity exceeded: requested size " << size
             << "u, available capacity " << capacity << "u.";

    // In practice, we will never reach this.
    abort();
}

} // namespace utils
