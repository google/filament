/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <backend/PresentCallable.h>

#include <utils/Panic.h>

namespace filament {
namespace backend {

PresentCallable::PresentCallable(PresentFn fn, void* user) noexcept
    : mPresentFn(fn), mUser(user) {
    assert(fn != nullptr);
}

void PresentCallable::operator()(bool presentFrame) noexcept {
    ASSERT_PRECONDITION(mPresentFn, "This PresentCallable was already called. " \
            "PresentCallables should be called exactly once.");
    mPresentFn(presentFrame, mUser);
    // Set mPresentFn to nullptr to denote that the callable has been called.
    mPresentFn = nullptr;
}

} // namespace backend
} // namespace filament
