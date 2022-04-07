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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_DISPATCHER_H
#define TNT_FILAMENT_BACKEND_PRIVATE_DISPATCHER_H

#include <stdint.h>

namespace filament::backend {

class Driver;
class CommandBase;

/*
 * Dispatcher is a data structure containing only function pointers.
 * Each function pointer targets code that unpacks the arguments to the driver's method and
 * calls it.
 *
 * Dispatcher's function pointers are populated during initialization and no CommandStream calls
 * can be made before that.
 *
 * When a command is inserted into the stream, the corresponding function pointer is copied
 * directly into CommandBase from Dispatcher.
 */
class Dispatcher {
public:
    using Execute = void (*)(Driver& driver, CommandBase* self, intptr_t* next);
#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)
#define DECL_DRIVER_API(methodName, paramsDecl, params)                     Execute methodName##_;
#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)     Execute methodName##_;

#include "DriverAPI.inc"
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_PRIVATE_DISPATCHER_H
