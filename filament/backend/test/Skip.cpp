/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "Skip.h"

#include <sstream>

namespace test {

SkipEnvironment::SkipEnvironment(test::Backend backend) : backend(backend) {}
SkipEnvironment::SkipEnvironment(test::OperatingSystem os) : os(os) {}
SkipEnvironment::SkipEnvironment(test::OperatingSystem os, test::Backend backend)
    : backend(backend),
      os(os) {}

bool SkipEnvironment::matches() {
    bool backendMatches = !backend.has_value() || *backend == BackendTest::sBackend;
    bool osMatches = !os.has_value() || *os == BackendTest::sOperatingSystem;
    bool isMobileMatches = !isMobile.has_value() || *isMobile == BackendTest::sIsMobilePlatform;
    return backendMatches && osMatches && isMobileMatches;
}

std::string SkipEnvironment::describe() {
    std::stringstream result;
    if (matches()) {
        result << "environment matches because " << describe_actual_environment() << ".";
    } else {
        result << "environment does not match because " << describe_requirements() << " but "
               << describe_actual_environment() << ".";
    }
    return result.str();
}

std::string SkipEnvironment::describe_actual_environment() {
    bool resultWritten = false;
    std::stringstream reality;
    if (backend.has_value()) {
        reality << "backend was " << utils::to_string(BackendTest::sBackend).c_str();
        resultWritten = true;
    }
    if (os.has_value()) {
        if (resultWritten) {
            reality << ", and ";
        }
        reality << "operating system was "
                << utils::to_string(BackendTest::sOperatingSystem).c_str();
        resultWritten = true;
    }
    if (isMobile.has_value()) {
        if (resultWritten) {
            reality << ", and ";
        }
        reality << "device " << (BackendTest::sIsMobilePlatform ? "was" : "was not") << " mobile";
        resultWritten = true;
    }
    return reality.str();
}

std::string SkipEnvironment::describe_requirements() {
    bool resultWritten = false;
    std::stringstream requirement;
    if (backend.has_value()) {
        requirement << "backend needs to be " << utils::to_string(*backend).c_str();
        resultWritten = true;
    }
    if (os.has_value()) {
        if (resultWritten) {
            requirement << ", and ";
        }
        requirement << "operating system needs to be " << utils::to_string(*os).c_str();
        resultWritten = true;
    }
    if (isMobile.has_value() && BackendTest::sIsMobilePlatform != isMobile) {
        if (resultWritten) {
            requirement << ", and ";
        }
        requirement << "device needs to " << (*isMobile ? "be" : "not be") << " mobile";
        resultWritten = true;
    }
    return requirement.str();
}

} // namespace test
