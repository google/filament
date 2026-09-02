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

#include "src/dawn/tests/mocks/platform/CachingInterfaceMock.h"

#include <algorithm>
#include <utility>

#include "src/utils/assert.h"
#include "src/utils/compiler.h"

CachingInterfaceMock::CachingInterfaceMock() {
    ON_CALL(*this, FindKey).WillByDefault([this](auto&&... args) {
        return FindKeyDefault(args...);
    });
    ON_CALL(*this, LoadData).WillByDefault([this](auto&&... args) {
        return LoadDataDefault(args...);
    });
    ON_CALL(*this, StoreData).WillByDefault([this](auto&&... args) {
        return StoreDataDefault(args...);
    });
}

void CachingInterfaceMock::Enable() {
    std::scoped_lock lock(mMutex);
    mEnabled = true;
}

void CachingInterfaceMock::Disable() {
    std::scoped_lock lock(mMutex);
    mEnabled = false;
}

size_t CachingInterfaceMock::GetHitCount() const {
    std::scoped_lock lock(mMutex);
    return mHitCount;
}

size_t CachingInterfaceMock::GetNumEntries() const {
    std::scoped_lock lock(mMutex);
    return mCache.size();
}

size_t CachingInterfaceMock::FindKeyDefault(std::span<const std::byte> key) {
    std::scoped_lock lock(mMutex);
    if (!mEnabled) {
        return 0;
    }

    std::vector<std::byte> keyVec(key.begin(), key.end());
    auto entry = mCache.find(keyVec);
    if (entry == mCache.end()) {
        return 0;
    }
    return entry->second.size();
}

size_t CachingInterfaceMock::LoadDataDefault(std::span<const std::byte> key,
                                             std::span<std::byte> dest) {
    std::scoped_lock lock(mMutex);
    if (!mEnabled) {
        return 0;
    }

    std::vector<std::byte> keyVec(key.begin(), key.end());
    auto entry = mCache.find(keyVec);
    if (entry == mCache.end()) {
        return 0;
    }
    DAWN_CHECK(dest.size() >= entry->second.size());
    std::ranges::copy(entry->second, dest.begin());
    mHitCount++;
    return entry->second.size();
}

void CachingInterfaceMock::StoreDataDefault(std::span<const std::byte> key,
                                            std::span<const std::byte> src) {
    std::scoped_lock lock(mMutex);
    if (!mEnabled) {
        return;
    }

    std::vector<std::byte> keyVec(key.begin(), key.end());
    std::vector<std::byte> entry(src.begin(), src.end());
    mCache.insert_or_assign(std::move(keyVec), std::move(entry));
}

DawnCachingMockPlatform::DawnCachingMockPlatform(dawn::platform::CachingInterface* cachingInterface)
    : mCachingInterface(cachingInterface) {}

dawn::platform::CachingInterface* DawnCachingMockPlatform::GetCachingInterface() {
    return mCachingInterface;
}
