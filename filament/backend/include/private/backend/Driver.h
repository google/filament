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

#include <backend/PipelineState.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/BufferDescriptor.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/Handle.h"
#include "private/backend/DriverApiForward.h"
#include "private/backend/Program.h"
#include "private/backend/SamplerGroup.h"

#include <filament/backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <stdint.h>

namespace filament {
namespace driver {

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

} // namespace driver
} // namespace filament

#if !defined(NDEBUG)

utils::io::ostream& operator<<(utils::io::ostream& out, const filament::driver::AttributeArray& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::driver::FaceOffsets& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::driver::PolygonOffset& po);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::driver::PipelineState& ps);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::driver::RasterState& rs);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::driver::TargetBufferInfo& tbi);

utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::BufferDescriptor const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::BufferUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::CullingMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::ElementType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PixelBufferDescriptor const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PixelDataFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PixelDataType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::Precision precision);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PrimitiveType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::RenderPassParams const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerCompareFunc func);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerCompareMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerMagFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerMinFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerParams params);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerWrapMode wrap);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::ShaderModel model);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::TextureCubemapFace face);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::TextureFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::TextureUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::Viewport const& v);
#endif

#endif // TNT_FILAMENT_DRIVER_DRIVER_H
