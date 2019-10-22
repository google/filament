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

#include "noop/NoopDriver.h"
#include "CommandStreamDispatcher.h"

namespace filament {

using namespace backend;

Driver* NoopDriver::create() {
    return new NoopDriver();
}

NoopDriver::NoopDriver() noexcept : DriverBase(new ConcreteDispatcher<NoopDriver>()) {
}

NoopDriver::~NoopDriver() noexcept = default;

backend::ShaderModel NoopDriver::getShaderModel() const noexcept {
#if defined(GLES31_HEADERS)
    return backend::ShaderModel::GL_ES_30;
#else
    return backend::ShaderModel::GL_CORE_41;
#endif
}

// explicit instantiation of the Dispatcher
template class backend::ConcreteDispatcher<NoopDriver>;

} // namespace filament
