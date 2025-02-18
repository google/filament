// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_SYSTEMUTILS_H_
#define SRC_DAWN_COMMON_SYSTEMUTILS_H_

#include <optional>
#include <string>
#include <utility>

#include "dawn/common/Platform.h"

namespace dawn {

const char* GetPathSeparator();
// Returns a pair of the environment variable's value, and a boolean indicating whether the variable
// was present.
std::pair<std::string, bool> GetEnvironmentVar(const char* variableName);
bool SetEnvironmentVar(const char* variableName, const char* value);
// Directories are always returned with a trailing path separator.
// May return std::nullopt if the path is too long, there is no current
// module (statically linked into executable), or the function is not
// implemented on the platform.
std::optional<std::string> GetExecutableDirectory();
std::optional<std::string> GetModuleDirectory();

#if DAWN_PLATFORM_IS(MACOS)
void GetMacOSVersion(int32_t* majorVersion, int32_t* minorVersion = nullptr);
bool IsMacOSVersionAtLeast(uint32_t majorVersion, uint32_t minorVersion = 0);
#endif

class ScopedEnvironmentVar {
  public:
    ScopedEnvironmentVar();
    ScopedEnvironmentVar(const char* variableName, const char* value);
    ~ScopedEnvironmentVar();

    ScopedEnvironmentVar(const ScopedEnvironmentVar& rhs) = delete;
    ScopedEnvironmentVar& operator=(const ScopedEnvironmentVar& rhs) = delete;

    bool Set(const char* variableName, const char* value);

  private:
    std::string mName;
    std::pair<std::string, bool> mOriginalValue;
    bool mIsSet = false;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_SYSTEMUTILS_H_
