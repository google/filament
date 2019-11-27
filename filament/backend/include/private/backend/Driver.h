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

#ifndef TNT_FILAMENT_DRIVER_DRIVER_H
#define TNT_FILAMENT_DRIVER_DRIVER_H

#include <backend/BufferDescriptor.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/PresentCallable.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/DriverApiForward.h"
#include "private/backend/Program.h"
#include "private/backend/SamplerGroup.h"

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <functional>

#include <stdint.h>

namespace filament {
namespace backend {

template<typename T>
class ConcreteDispatcher;
class Dispatcher;

class Driver {
public:
    static size_t getElementTypeSize(ElementType type) noexcept;

    virtual ~Driver() noexcept;

    // called from the main thread (NOT the render-thread) at various intervals, this
    // is where the driver can free resources consumed by previous commands.
    virtual void purge() noexcept = 0;

    virtual ShaderModel getShaderModel() const noexcept = 0;

    virtual Dispatcher& getDispatcher() noexcept = 0;

    // called from CommandStream::execute on the render-thread
    // the fn function will execute a batch of driver commands
    // this gives the driver a chance to wrap their execution in a meaningful manner
    // the default implementation simply calls fn
    virtual void execute(std::function<void(void)> fn) noexcept;

#ifndef NDEBUG
    virtual void debugCommand(const char* methodName) {}
#endif

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

} // namespace backend
} // namespace filament

#if !defined(NDEBUG)

utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::AttributeArray& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::FaceOffsets& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::PolygonOffset& po);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::PipelineState& ps);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::RasterState& rs);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::TargetBufferInfo& tbi);

utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::BufferDescriptor const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::BufferUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::CullingMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::ElementType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PixelBufferDescriptor const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PixelDataFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PixelDataType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::Precision precision);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PrimitiveType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TargetBufferFlags f);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::RenderPassParams const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerCompareFunc func);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerCompareMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerMagFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerMinFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerParams params);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerWrapMode wrap);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::ShaderModel model);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureCubemapFace face);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::Viewport const& v);
#endif

#endif // TNT_FILAMENT_DRIVER_DRIVER_H
