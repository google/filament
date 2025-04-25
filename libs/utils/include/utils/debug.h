/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_UTILS_DEBUG_H
#define TNT_UTILS_DEBUG_H

#include <utils/compiler.h>

namespace utils {
UTILS_PUBLIC
void panic(const char *func, const char * file, int line, const char *assertion) noexcept;
} // namespace filament

#ifdef NDEBUG
#   define	assert_invariant(e)	((void)0)
#else
#   define	assert_invariant(e) \
            (UTILS_VERY_LIKELY(e) ? ((void)0) : utils::panic(__func__, __FILE__, __LINE__, #e))
#endif // NDEBUG

#endif // TNT_UTILS_DEBUG_H
