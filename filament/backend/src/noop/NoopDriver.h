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

#ifndef TNT_FILAMENT_BACKEND_NOOPDRIVER_H
#define TNT_FILAMENT_BACKEND_NOOPDRIVER_H

#include "private/backend/Driver.h"
#include "DriverBase.h"

#include <utils/compiler.h>

namespace filament::backend {

class NoopDriver final : public DriverBase {
    NoopDriver() noexcept;
    ~NoopDriver() noexcept override;
    Dispatcher getDispatcher() const noexcept final;

public:
    static Driver* create();

private:
    ShaderModel getShaderModel() const noexcept final;
    ShaderLanguage getShaderLanguage() const noexcept final;

    uint64_t nextFakeHandle = 1;

    /*
     * Driver interface
     */

    template<typename T>
    friend class ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override { \
        return RetType((RetType::HandleId)nextFakeHandle++); } \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl) { }

#include "private/backend/DriverAPI.inc"
};

} // namespace filament

#endif // TNT_FILAMENT_BACKEND_NOOPDRIVER_H
