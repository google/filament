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

#ifndef TNT_FILAMAT_PACKAGE_H
#define TNT_FILAMAT_PACKAGE_H

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include <cstddef>
#include <functional>

#include <utils/compiler.h>

namespace filamat {

class UTILS_PUBLIC Package {
public:
    Package() = default;

    // Regular constructor
    explicit Package(size_t size) : mSize(size) {
        mPayload = new uint8_t[size];
    }

    Package(const void* src, size_t size) : Package(size) {
        memcpy(mPayload, src, size);
    }

    // Move Constructor
    Package(Package&& other) noexcept : mPayload(other.mPayload), mSize(other.mSize) {
        other.mPayload = nullptr;
        other.mSize = 0;
    }

    // Move assignment
    Package& operator=(Package&& other) noexcept {
        std::swap(mPayload, other.mPayload);
        std::swap(mSize, other.mSize);
        return *this;
    }

    // Copy assignment operator disallowed.
    Package& operator=(const Package& other) = delete;

    // Copy constructor disallowed.
    Package(const Package& other) = delete;

    ~Package() {
        delete[] mPayload;
    }

    uint8_t* getData() const noexcept {
        return mPayload;
    }

    size_t getSize() const noexcept {
        return mSize;
    }

    uint8_t* getEnd() const noexcept {
        return mPayload + mSize;
    }

    void setValid(bool valid) noexcept {
        mValid = valid;
    }

    bool isValid() const noexcept {
        return mValid;
    }

    static Package invalidPackage() {
        Package package(0);
        package.setValid(false);
        return package;
    }

private:
    uint8_t* mPayload = nullptr;
    size_t mSize = 0;
    bool mValid = true;
};

} // namespace filamat
#endif
