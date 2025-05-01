/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "ResourceList.h"

#include <absl/log/log.h>

#include <algorithm>

namespace filament {

ResourceListBase::ResourceListBase(const char* typeName)
#ifndef NDEBUG
        : mTypeName(typeName)
#endif
{
}

ResourceListBase::~ResourceListBase() noexcept {
#ifndef NDEBUG
    if (!mList.empty()) {
        DLOG(INFO) << "leaked " << mList.size() << " " << mTypeName;
    }
#endif
}

void ResourceListBase::insert(void* item) {
    mList.insert(item);
}

bool ResourceListBase::remove(void const* item) {
    return mList.erase(const_cast<void*>(item)) > 0;
}

auto ResourceListBase::find(void const* item) -> iterator {
    return mList.find(const_cast<void*>(item));
}

void ResourceListBase::clear() noexcept {
    mList.clear();
}

// this is not inlined, so we don't pay the code-size cost of iterating the list
void ResourceListBase::forEach(void (* f)(void*, void*), void* user) const noexcept {
    std::for_each(mList.begin(), mList.end(), [=](void* p) {
        f(user, p);
    });
}

} // namespace filament
