// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/tests/mocks/platform/CachingInterfaceMock.h"

using ::testing::Invoke;

CachingInterfaceMock::CachingInterfaceMock() {
    ON_CALL(*this, LoadData).WillByDefault(Invoke([this](auto&&... args) {
        return LoadDataDefault(args...);
    }));
    ON_CALL(*this, StoreData).WillByDefault(Invoke([this](auto&&... args) {
        return StoreDataDefault(args...);
    }));
}

void CachingInterfaceMock::Enable() {
    mEnabled = true;
}

void CachingInterfaceMock::Disable() {
    mEnabled = false;
}

size_t CachingInterfaceMock::GetHitCount() const {
    return mHitCount;
}

size_t CachingInterfaceMock::GetNumEntries() const {
    return mCache.size();
}

size_t CachingInterfaceMock::LoadDataDefault(const void* key,
                                             size_t keySize,
                                             void* value,
                                             size_t valueSize) {
    if (!mEnabled) {
        return 0;
    }

    const std::string keyStr(reinterpret_cast<const char*>(key), keySize);
    auto entry = mCache.find(keyStr);
    if (entry == mCache.end()) {
        return 0;
    }
    if (valueSize >= entry->second.size()) {
        // Only consider a cache-hit on the memcpy, since peeks are implementation detail.
        memcpy(value, entry->second.data(), entry->second.size());
        mHitCount++;
    }
    return entry->second.size();
}

void CachingInterfaceMock::StoreDataDefault(const void* key,
                                            size_t keySize,
                                            const void* value,
                                            size_t valueSize) {
    if (!mEnabled) {
        return;
    }

    const std::string keyStr(reinterpret_cast<const char*>(key), keySize);
    const uint8_t* it = reinterpret_cast<const uint8_t*>(value);
    std::vector<uint8_t> entry(it, it + valueSize);
    mCache.insert_or_assign(keyStr, entry);
}

DawnCachingMockPlatform::DawnCachingMockPlatform(dawn::platform::CachingInterface* cachingInterface)
    : mCachingInterface(cachingInterface) {}

dawn::platform::CachingInterface* DawnCachingMockPlatform::GetCachingInterface() {
    return mCachingInterface;
}
