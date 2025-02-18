// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/tests/ToggleParser.h"

#include <cstring>
#include <sstream>

ToggleParser::ToggleParser() = default;
ToggleParser::~ToggleParser() = default;

bool ToggleParser::ParseEnabledToggles(char* arg) {
    constexpr const char kEnableTogglesSwitch[] = "--enable-toggles=";
    size_t argLen = sizeof(kEnableTogglesSwitch) - 1;
    if (strncmp(arg, kEnableTogglesSwitch, argLen) == 0) {
        std::string toggle;
        std::stringstream toggles(arg + argLen);
        while (getline(toggles, toggle, ',')) {
            mEnabledToggles.push_back(toggle);
        }
        return true;
    }
    return false;
}

bool ToggleParser::ParseDisabledToggles(char* arg) {
    constexpr const char kDisableTogglesSwitch[] = "--disable-toggles=";
    size_t argLDis = sizeof(kDisableTogglesSwitch) - 1;
    if (strncmp(arg, kDisableTogglesSwitch, argLDis) == 0) {
        std::string toggle;
        std::stringstream toggles(arg + argLDis);
        while (getline(toggles, toggle, ',')) {
            mDisabledToggles.push_back(toggle);
        }
        return true;
    }
    return false;
}

const std::vector<std::string>& ToggleParser::GetEnabledToggles() const {
    return mEnabledToggles;
}

const std::vector<std::string>& ToggleParser::GetDisabledToggles() const {
    return mDisabledToggles;
}
