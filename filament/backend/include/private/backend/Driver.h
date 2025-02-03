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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_DRIVER_H
#define TNT_FILAMENT_BACKEND_PRIVATE_DRIVER_H

#include <backend/CallbackHandler.h>
#include <backend/DescriptorSetOffsetArray.h>
#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>
#include <backend/TargetBufferInfo.h>

#include <utils/CString.h>
#include <utils/compiler.h>

#include <functional>

#include <stddef.h>
#include <stdint.h>

// Command debugging off. debugging virtuals are not called.
// This is automatically enabled in DEBUG builds.
#define FILAMENT_DEBUG_COMMANDS_NONE         0x0
// Command debugging enabled. No logging by default.
#define FILAMENT_DEBUG_COMMANDS_ENABLE       0x1
// Command debugging enabled. Every command logged to slog.d
#define FILAMENT_DEBUG_COMMANDS_LOG          0x2
// Command debugging enabled. Every command logged to systrace
#define FILAMENT_DEBUG_COMMANDS_SYSTRACE     0x4

#define FILAMENT_DEBUG_COMMANDS              FILAMENT_DEBUG_COMMANDS_NONE

namespace filament::backend {

class BufferDescriptor;
class CallbackHandler;
class PixelBufferDescriptor;
class Program;

template<typename T>
class ConcreteDispatcher;
class Dispatcher;
class CommandStream;

class Driver {
public:
    virtual ~Driver() noexcept;

    static size_t getElementTypeSize(ElementType type) noexcept;

    // called from the main thread (NOT the render-thread) at various intervals, this
    // is where the driver can execute user callbacks.
    virtual void purge() noexcept = 0;

    virtual ShaderModel getShaderModel() const noexcept = 0;

    // The shader language used for shaders for this driver, used to inform matdbg.
    //
    // For OpenGL, this distinguishes whether the driver's shaders are powered by ESSL1 or ESSL3.
    // This information is used by matdbg to display the correct shader code to the web UI and patch
    // the correct chunk when rebuilding shaders live.
    //
    // Metal shaders can either be MSL or Metal libraries, but at time of writing, matdbg can only
    // interface with MSL.
    virtual ShaderLanguage getShaderLanguage() const noexcept = 0;

    // Returns the dispatcher. This is only called once during initialization of the CommandStream,
    // so it doesn't matter that it's virtual.
    virtual Dispatcher getDispatcher() const noexcept = 0;

    // called from CommandStream::execute on the render-thread
    // the fn function will execute a batch of driver commands
    // this gives the driver a chance to wrap their execution in a meaningful manner
    // the default implementation simply calls fn
    virtual void execute(std::function<void(void)> const& fn);

    // This is called on debug build, or when enabled manually on the backend thread side.
    virtual void debugCommandBegin(CommandStream* cmds,
            bool synchronous, const char* methodName) noexcept = 0;

    virtual void debugCommandEnd(CommandStream* cmds,
            bool synchronous, const char* methodName) noexcept = 0;

    /*
     * Asynchronous calls here only to provide a type to CommandStream. They must be non-virtual
     * so that calling the concrete implementation won't go through a vtable.
     *
     * Synchronous calls are virtual and are called directly by CommandStream.
     */

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    void methodName(paramsDecl) {}

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    virtual RetType methodName(paramsDecl) = 0;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    virtual RetType methodName##S() noexcept = 0; \
    void methodName##R(RetType, paramsDecl) {}

#include "private/backend/DriverAPI.inc"
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_DRIVER_H
