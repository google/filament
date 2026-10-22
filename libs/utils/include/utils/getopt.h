/*
 * Copyright (C) 2026 The Android Open Source Project
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


#ifndef UTILS_GETOPT_H
#define UTILS_GETOPT_H

// If system has getopt, we just include it
#if defined(HAS_SYSTEM_GETOPT)
#include <getopt.h>
#else
#include <getopt/getopt.h>
#endif

#ifdef __cplusplus

#undef no_argument
#undef required_argument
#undef optional_argument

// This is an aliasing of getopt to prevent compilation conflicts with the system getop (if
// it exists). Please refer to third_party/getopt for API details.
namespace utils {
namespace getopt {
    using ::getopt;
    using ::optarg;
    using ::optind;
    using ::opterr;
    using ::optopt;
    using ::option;
    using ::getopt_long;
    using ::getopt_long_only;

    constexpr int no_argument = 0;
    constexpr int required_argument = 1;
    constexpr int optional_argument = 2;
}
}
#endif // __cplusplus

#endif // UTILS_GETOPT_H
