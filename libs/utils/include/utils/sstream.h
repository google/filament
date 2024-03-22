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

#ifndef TNT_UTILS_SSTREAM_H
#define TNT_UTILS_SSTREAM_H

#include <utils/ostream.h>

namespace utils::io {

class sstream : public ostream {
public:
    ostream& flush() noexcept override;
    const char* c_str() const noexcept;
    size_t length() const noexcept;
};

} // namespace utils::io

#endif // TNT_UTILS_SSTREAM_H
