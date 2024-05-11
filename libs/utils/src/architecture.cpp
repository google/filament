/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <utils/architecture.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#   include <unistd.h>
#elif defined(WIN32)
#   include <windows.h>
#endif

namespace utils::arch {

size_t getPageSize() noexcept {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
    return size_t(sysconf(_SC_PAGESIZE));
#elif defined(WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return size_t(sysInfo.dwPageSize);
#else
    return 4096u;
#endif
}

} // namespace utils::arch
