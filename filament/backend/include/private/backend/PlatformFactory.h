/*
 * Copyright (C) 2022 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_BACKEND_PLATFORM_FACTORY_H
#define TNT_FILAMENT_BACKEND_PLATFORM_FACTORY_H

#include <backend/DriverEnums.h>

#include <utils/compiler.h>

namespace filament::backend {

class Platform;

class UTILS_PUBLIC PlatformFactory  {
public:

    /**
     * Creates a Platform configured for the requested backend if available
     *
     * @param backendHint Preferred backend, if not available the backend most suitable for the
     *                    underlying platform is returned and \p backendHint is updated
     *                    accordingly. Can't be nullptr.
     *
     * @return A pointer to the Platform object.
     *
     * @see destroy
     */
    static Platform* create(backend::Backend* backendHint) noexcept;

    /**
     * Destroys a Platform object returned by create()
     *
     * @param platform a reference (as a pointer) to the Platform pointer to destroy.
     *                 \p platform is cleared upon return.
     *
     * @see create
     */
    static void destroy(Platform** platform) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_BACKEND_PLATFORM_FACTORY_H
