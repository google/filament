/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_DATARESHAPER_H
#define TNT_FILAMENT_DRIVER_DATARESHAPER_H

#include <stddef.h>

namespace filament {
namespace backend {

// This little utility adds padding to multi-channel interleaved data by inserting dummy values.
// This is very useful for platforms that only accept 4-component data, since users often wish to
// submit 3-component data.
class DataReshaper {
public:
    template<typename componentType, int numSrcChannels, int numDstChannels,
            componentType maxValue = std::numeric_limits<componentType>::max()>
    static void reshape(void* dest, const void* src, size_t numSrcBytes) {
        const componentType* in = (const componentType*) src;
        componentType* out = (componentType*) dest;
        const size_t numWords = (numSrcBytes / sizeof(componentType)) / numSrcChannels;
        for (size_t word = 0; word < numWords; ++word) {
            for (size_t component = 0; component < numSrcChannels; ++component) {
                out[component] = in[component];
            }
            for (size_t component = (size_t)numSrcChannels; component < numDstChannels; ++component) {
                out[component] = maxValue;
            }
            in += numSrcChannels;
            out += numDstChannels;
        }
    }
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_DATARESHAPER_H
