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

#include <utils/string.h>

#include <stdlib.h>
#if defined(WIN32)
#    include <locale.h>
#elif defined(__linux__)
#    include <locale.h>
#else
#    include <xlocale.h>
#endif

#include <mutex>

namespace utils {

#if defined(WIN32)

_locale_t sLocaleC;
std::once_flag sLocaleCOnceFlag;

_locale_t get_c_locale() {
    std::call_once(sLocaleCOnceFlag, []() {
        sLocaleC = _create_locale(LC_ALL, "C");
    });
    return sLocaleC;
}

float strtof_c(const char* start, char** end) {
    return _strtof_l(start, end, get_c_locale());
}

#else // if defined(WIN32)

locale_t sLocaleC;
std::once_flag sLocaleCOnceFlag;

locale_t get_c_locale() {
    std::call_once(sLocaleCOnceFlag, []() {
        sLocaleC = newlocale(LC_ALL_MASK, "C", nullptr);
    });
    return sLocaleC;
}

float strtof_c(const char* start, char** end) {
    return strtof_l(start, end, get_c_locale());
}

#endif

} // namespace utils
