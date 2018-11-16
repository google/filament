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

#include "TextDictionaryReader.h"

#include <utils/Log.h>

namespace filaflat {

bool TextDictionaryReader::unflatten(Unflattener& f, BlobDictionary& dictionary) {
    uint32_t numStrings = 0;
    if (!f.read(&numStrings)) {
        return false;
    }

    dictionary.reserve(numStrings);
    for (uint32_t i = 0; i < numStrings; i++) {
        const char* str;
        if (!f.read(&str)) {
            return false;
        }
        // BlobDictionary hold binary chunks and does not care if the data holds text, it is
        // therefore crucial to include the trailing null.
        dictionary.addBlob(str, strlen(str) + 1);
    }
    return true;
}

}
