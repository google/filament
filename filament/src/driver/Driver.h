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

#include <memory>

#include <stdint.h>

#include <math/vec4.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <filament/driver/PixelBufferDescriptor.h>
#include <filament/driver/BufferDescriptor.h>
#include <filament/driver/ExternalContext.h>
#include <filament/driver/DriverEnums.h>

#include "driver/Handle.h"
#include "driver/DriverApiForward.h"
#include "driver/Program.h"
#include "driver/SamplerBuffer.h"
#include "driver/UniformBuffer.h"

namespace filament {

template<typename T>
class ConcreteDispatcher;
class Dispatcher;

/* ------------------------------------------------------------------------------------------------
 * Driver interface and factory
 * ------------------------------------------------------------------------------------------------
 */

class Driver {
public:
    // constants
    static constexpr size_t MAX_ATTRIBUTE_BUFFER_COUNT = 8;

    /*
     * Driver types...
     */
    using ShaderModel = driver::ShaderModel;

    // remap the public types into the Driver class
    using PrimitiveType = driver::PrimitiveType;
    using UniformType = driver::UniformType;
    using ElementType = driver::ElementType;
    using Usage = driver::Usage;
    using TextureFormat = driver::TextureFormat;
    using TextureUsage = driver::TextureUsage;
    using TextureCubemapFace = driver::TextureCubemapFace;
    using SamplerType = driver::SamplerType;
    using SamplerPrecision = driver::Precision;
    using SamplerWrapMode = driver::SamplerWrapMode;
    using SamplerMinFilter = driver::SamplerMinFilter;
    using SamplerMagFilter = driver::SamplerMagFilter;
    using SamplerCompareMode = driver::SamplerCompareMode;
    using SamplerCompareFunc = driver::SamplerCompareFunc;
    using SamplerParams = driver::SamplerParams;
    using PixelDataFormat = driver::PixelDataFormat;
    using PixelDataType = driver::PixelDataType;
    using BufferDescriptor = driver::BufferDescriptor;
    using PixelBufferDescriptor = driver::PixelBufferDescriptor;
    using FaceOffsets = driver::FaceOffsets;
    using FenceStatus = driver::FenceStatus;
    using TargetBufferFlags = driver::TargetBufferFlags;
    using RenderPassParams = driver::RenderPassParams;

    static constexpr uint64_t FENCE_WAIT_FOR_EVER = driver::FENCE_WAIT_FOR_EVER;

    // Types used by the command stream
    // (we use this renaming because the macro-system doesn't deal well with "<" and ">")
    using VertexBufferHandle    = Handle<HwVertexBuffer>;
    using IndexBufferHandle     = Handle<HwIndexBuffer>;
    using RenderPrimitiveHandle = Handle<HwRenderPrimitive>;
    using ProgramHandle         = Handle<HwProgram>;
    using SamplerBufferHandle   = Handle<HwSamplerBuffer>;
    using UniformBufferHandle   = Handle<HwUniformBuffer>;
    using TextureHandle         = Handle<HwTexture>;
    using RenderTargetHandle    = Handle<HwRenderTarget>;
    using FenceHandle           = Handle<HwFence>;
    using SwapChainHandle       = Handle<HwSwapChain>;
    using StreamHandle          = Handle<HwStream>;

    struct Attribute {
        static constexpr uint8_t FLAG_NORMALIZED     = 0x1;
        static constexpr uint8_t FLAG_INTEGER_TARGET = 0x2;
        uint32_t offset = 0;
        uint8_t stride = 0;
        uint8_t buffer = 0xFF;
        ElementType type = ElementType::BYTE;
        uint8_t flags = 0x0;
    };

    using AttributeArray = std::array<Attribute, MAX_ATTRIBUTE_BUFFER_COUNT>;

    // types of the data returned by samplers in the shaders
    enum class SamplerFormat : uint8_t {
        // don't change values of enums (used w/ UniformInterfaceBlock::Type)
        INT = 0,
        UINT = 1,
        FLOAT = 2,
        SHADOW = 3,
    };

    struct TargetBufferInfo {
        // ctor for 2D textures
        TargetBufferInfo(TextureHandle h, uint8_t level = 0) noexcept
                : handle(h), level(level) { }
        // ctor for cubemaps
        TargetBufferInfo(TextureHandle h, uint8_t level, TextureCubemapFace face) noexcept
                : handle(h), level(level), face(face) { }
        // ctor for 3D textures
        TargetBufferInfo(TextureHandle h, uint8_t level, uint16_t layer) noexcept
                : handle(h), level(level), layer(layer) { }

        // texture to be used as render target
        TextureHandle handle;
        // level to be used
        uint8_t level = 0;
        union {
            // face if texture is a cubemap
            TextureCubemapFace face;
            // for 3D textures
            uint16_t layer = 0;
        };
        TargetBufferInfo() noexcept { }
    };

    struct RasterState {
        using CullingMode = driver::CullingMode;
        using DepthFunc = driver::SamplerCompareFunc;
        using BlendEquation = driver::BlendEquation;
        using BlendFunction = driver::BlendFunction;

        RasterState() noexcept {
            static_assert(sizeof(RasterState) == sizeof(uint32_t),
                    "RasterState size not what was intended");
            culling = CullingMode::BACK;
            blendEquationRGB = BlendEquation::ADD;
            blendEquationAlpha = BlendEquation::ADD;
            blendFunctionSrcRGB = BlendFunction::ONE;
            blendFunctionSrcAlpha = BlendFunction::ONE;
            blendFunctionDstRGB = BlendFunction::ZERO;
            blendFunctionDstAlpha = BlendFunction::ZERO;
        }

        bool operator == (RasterState rhs) const noexcept { return u == rhs.u; }
        bool operator != (RasterState rhs) const noexcept { return u != rhs.u; }

        void disableBlending() noexcept {
            blendEquationRGB = BlendEquation::ADD;
            blendEquationAlpha = BlendEquation::ADD;
            blendFunctionSrcRGB = BlendFunction::ONE;
            blendFunctionSrcAlpha = BlendFunction::ONE;
            blendFunctionDstRGB = BlendFunction::ZERO;
            blendFunctionDstAlpha = BlendFunction::ZERO;
        }

        // note: clang reduces this entire function to a simple load/mask/compare
        bool hasBlending() const noexcept {
            // there could be other cases where blending would end-up being disabled,
            // but this is common and easy to check
            return !(blendEquationRGB == BlendEquation::ADD &&
                     blendEquationAlpha == BlendEquation::ADD &&
                     blendFunctionSrcRGB == BlendFunction::ONE &&
                     blendFunctionSrcAlpha == BlendFunction::ONE &&
                     blendFunctionDstRGB == BlendFunction::ZERO &&
                     blendFunctionDstAlpha == BlendFunction::ZERO);
        }

        union {
            struct {
                CullingMode culling                 : 2;        //  2
                BlendEquation blendEquationRGB      : 3;        //  5
                BlendEquation blendEquationAlpha    : 3;        //  8
                BlendFunction blendFunctionSrcRGB   : 4;        // 12
                BlendFunction blendFunctionSrcAlpha : 4;        // 16
                BlendFunction blendFunctionDstRGB   : 4;        // 20
                BlendFunction blendFunctionDstAlpha : 4;        // 24
                bool depthWrite                     : 1;        // 25
                DepthFunc depthFunc                 : 3;        // 28
                bool colorWrite                     : 1;        // 29
                bool alphaToCoverage                : 1;        // 30
                uint8_t padding                     : 2;        // 32
            };
            uint32_t u = 0;
        };
    };

    static SamplerFormat getSamplerFormat(TextureFormat format) noexcept;
    static SamplerPrecision getSamplerPrecision(TextureFormat format) noexcept;
    static size_t getElementTypeSize(ElementType type) noexcept;

    // This is here to be compatible with CommandStream (nice for debugging)
    inline void queueCommand(const std::function<void()>& command) {
        command();
    }

    // --------------------------------------------------------------------------------------------
    // DriverAPI interface
    // --------------------------------------------------------------------------------------------

public:
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
     *
     * Asynchronous calls here only to provide a type to CommandStream. They must be non-virtual
     * so that calling the concrete implementation won't go through a vtable.
     *
     * Synchronous calls are virtual and are called directly by CommandStream.
     *
     */

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    void methodName(paramsDecl) {}

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    virtual RetType methodName(paramsDecl) = 0;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    virtual RetType methodName##Synchronous() noexcept = 0; \
    void methodName(RetType, paramsDecl) {}

#include "driver/DriverAPI.inc"
};

} // namespace filament

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::Driver::AttributeArray& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::Driver::FaceOffsets& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::Driver::RasterState& rs);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::Driver::TargetBufferInfo& tbi);

utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::ShaderModel model);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PrimitiveType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::ElementType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::Usage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::CullingMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::Precision precision);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::TextureFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::TextureUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::TextureCubemapFace face);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerWrapMode wrap);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerMinFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerMagFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerCompareMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerCompareFunc func);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::SamplerParams params);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PixelDataFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PixelDataType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::BufferDescriptor const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::PixelBufferDescriptor const& b);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::driver::RenderPassParams const& b);
#endif

#endif // TNT_FILAMENT_DRIVER_DRIVER_H
