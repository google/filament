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

#include <utils/sstream.h>
#include <utils/ostream.h>

#include "ostream_.h"

namespace utils::io {

ostream& sstream::flush() noexcept {
    // no-op.
    return *this;
}

const char* sstream::c_str() const noexcept {
    char const* buffer = getBuffer().get();
    return buffer ? buffer : "";
}

size_t sstream::length() const noexcept {
    return getBuffer().length();
}

} // namespace utils::io
