// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NODE_BINDING_CONVERTER_H_
#define SRC_DAWN_NODE_BINDING_CONVERTER_H_

#include <webgpu/webgpu_cpp.h>

#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// ImplOfTraits is a traits helper that is used to associate the interop interface type to the
// binding implementation type.
template <typename T>
struct ImplOfTraits {};

// DECLARE_IMPL() is a macro that declares a specialization of ImplOfTraits so that
// `typename ImplOfTraits<interop::NAME>::type` is equivalent to `binding::NAME`.
#define DECLARE_IMPL(NAME)               \
    class NAME;                          \
    template <>                          \
    struct ImplOfTraits<interop::NAME> { \
        using type = binding::NAME;      \
    }

// Declare the interop interface to binding implementations
DECLARE_IMPL(GPUBindGroup);
DECLARE_IMPL(GPUBindGroupLayout);
DECLARE_IMPL(GPUBuffer);
DECLARE_IMPL(GPUPipelineLayout);
DECLARE_IMPL(GPUQuerySet);
DECLARE_IMPL(GPURenderBundle);
DECLARE_IMPL(GPURenderPipeline);
DECLARE_IMPL(GPUSampler);
DECLARE_IMPL(GPUShaderModule);
DECLARE_IMPL(GPUTexture);
DECLARE_IMPL(GPUTextureView);
#undef DECLARE_IMPL

// Helper for obtaining the binding implementation type from the interop interface type
template <typename T>
using ImplOf = typename ImplOfTraits<T>::type;

// Converter is a utility class for converting IDL generated interop types into Dawn types.
// As the Dawn C++ API uses raw C pointers for a number of its interfaces, Converter performs
// heap allocations for conversions of vector or optional types. These pointers are
// automatically freed when the Converter is destructed.
class Converter {
  public:
    explicit Converter(Napi::Env e) : env(e) {}
    Converter(Napi::Env e, wgpu::Device extensionDevice)
        : env(e), device(std::move(extensionDevice)) {}
    ~Converter();

    // Conversion function. Converts the interop type IN to the Dawn type OUT.
    // Returns true on success, false on failure.
    template <typename OUT, typename IN>
    [[nodiscard]] inline bool operator()(OUT&& out, IN&& in) {
        return Convert(std::forward<OUT>(out), std::forward<IN>(in));
    }

    // Vector conversion function. Converts the vector of interop type IN to a pointer of
    // elements of Dawn type OUT, which is assigned to 'out_els'.
    // out_count is assigned the number of elements in 'in'.
    // Returns true on success, false on failure.
    // The pointer assigned to 'out_els' is valid until the Converter is destructed.
    template <typename OUT, typename IN>
    [[nodiscard]] inline bool operator()(OUT*& out_els,
                                         size_t& out_count,
                                         const std::vector<IN>& in) {
        return Convert(out_els, out_count, in);
    }

    // Returns the Env that this Converter was constructed with.
    inline Napi::Env Env() const { return env; }

    // BufferSource is the converted type of interop::BufferSource.
    struct BufferSource {
        void* data;
        size_t size;             // in bytes
        size_t bytesPerElement;  // 1 for ArrayBuffers
    };

  private:
    // Below are the various overloads of Convert() used to convert the interop -> Dawn types.
    [[nodiscard]] bool Convert(wgpu::Extent3D& out, const interop::GPUExtent3D& in);

    [[nodiscard]] bool Convert(wgpu::Origin3D& out, const interop::GPUOrigin3DDict& in);

    [[nodiscard]] bool Convert(wgpu::Color& out, const interop::GPUColor& in);

    [[nodiscard]] bool Convert(wgpu::Origin3D& out,
                               const std::vector<interop::GPUIntegerCoordinate>& in);

    [[nodiscard]] bool Convert(wgpu::TextureAspect& out, const interop::GPUTextureAspect& in);

    [[nodiscard]] bool Convert(wgpu::TexelCopyTextureInfo& out,
                               const interop::GPUTexelCopyTextureInfo& in);

    [[nodiscard]] bool Convert(wgpu::TexelCopyBufferInfo& out,
                               const interop::GPUTexelCopyBufferInfo& in);

    [[nodiscard]] bool Convert(BufferSource& out, interop::BufferSource in);

    [[nodiscard]] bool Convert(wgpu::TexelCopyBufferLayout& out,
                               const interop::GPUTexelCopyBufferLayout& in);

    [[nodiscard]] bool Convert(wgpu::TextureFormat& out, const interop::GPUTextureFormat& in);

    [[nodiscard]] bool Convert(wgpu::TextureUsage& out, const interop::GPUTextureUsageFlags& in);

    [[nodiscard]] bool Convert(wgpu::ColorWriteMask& out, const interop::GPUColorWriteFlags& in);

    [[nodiscard]] bool Convert(wgpu::BufferUsage& out, const interop::GPUBufferUsageFlags& in);

    [[nodiscard]] bool Convert(wgpu::MapMode& out, const interop::GPUMapModeFlags& in);

    [[nodiscard]] bool Convert(wgpu::ShaderStage& out, const interop::GPUShaderStageFlags& in);

    [[nodiscard]] bool Convert(wgpu::TextureDimension& out, const interop::GPUTextureDimension& in);

    [[nodiscard]] bool Convert(wgpu::TextureViewDimension& out,
                               const interop::GPUTextureViewDimension& in);

    [[nodiscard]] bool Convert(wgpu::ComputeState& out, const interop::GPUProgrammableStage& in);

    [[nodiscard]] bool Convert(wgpu::ConstantEntry& out,
                               const std::string& in_name,
                               wgpu::interop::GPUPipelineConstantValue in_value);

    [[nodiscard]] bool Convert(wgpu::BlendComponent& out, const interop::GPUBlendComponent& in);

    [[nodiscard]] bool Convert(wgpu::BlendFactor& out, const interop::GPUBlendFactor& in);

    [[nodiscard]] bool Convert(wgpu::BlendOperation& out, const interop::GPUBlendOperation& in);

    [[nodiscard]] bool Convert(wgpu::BlendState& out, const interop::GPUBlendState& in);

    [[nodiscard]] bool Convert(wgpu::PrimitiveState& out, const interop::GPUPrimitiveState& in);

    [[nodiscard]] bool Convert(wgpu::ColorTargetState& out, const interop::GPUColorTargetState& in);

    [[nodiscard]] bool Convert(wgpu::DepthStencilState& out,
                               const interop::GPUDepthStencilState& in);

    [[nodiscard]] bool Convert(wgpu::MultisampleState& out, const interop::GPUMultisampleState& in);

    [[nodiscard]] bool Convert(wgpu::FragmentState& out, const interop::GPUFragmentState& in);

    [[nodiscard]] bool Convert(wgpu::PrimitiveTopology& out,
                               const interop::GPUPrimitiveTopology& in);

    [[nodiscard]] bool Convert(wgpu::FrontFace& out, const interop::GPUFrontFace& in);

    [[nodiscard]] bool Convert(wgpu::CullMode& out, const interop::GPUCullMode& in);

    [[nodiscard]] bool Convert(wgpu::CompareFunction& out, const interop::GPUCompareFunction& in);

    [[nodiscard]] bool Convert(wgpu::IndexFormat& out, const interop::GPUIndexFormat& in);

    [[nodiscard]] bool Convert(wgpu::StencilOperation& out, const interop::GPUStencilOperation& in);

    [[nodiscard]] bool Convert(wgpu::StencilFaceState& out, const interop::GPUStencilFaceState& in);

    [[nodiscard]] bool Convert(wgpu::VertexState& out, const interop::GPUVertexState& in);

    [[nodiscard]] bool Convert(wgpu::VertexBufferLayout& out,
                               const interop::GPUVertexBufferLayout& in);

    [[nodiscard]] bool Convert(wgpu::VertexStepMode& out, const interop::GPUVertexStepMode& in);

    [[nodiscard]] bool Convert(wgpu::VertexAttribute& out, const interop::GPUVertexAttribute& in);

    [[nodiscard]] bool Convert(wgpu::VertexFormat& out, const interop::GPUVertexFormat& in);

    [[nodiscard]] bool Convert(wgpu::RenderPassColorAttachment& out,
                               const interop::GPURenderPassColorAttachment& in);

    [[nodiscard]] bool Convert(wgpu::RenderPassDepthStencilAttachment& out,
                               const interop::GPURenderPassDepthStencilAttachment& in);

    [[nodiscard]] bool Convert(wgpu::PassTimestampWrites& out,
                               const interop::GPURenderPassTimestampWrites& in);

    [[nodiscard]] bool Convert(wgpu::PassTimestampWrites& out,
                               const interop::GPUComputePassTimestampWrites& in);

    [[nodiscard]] bool Convert(wgpu::LoadOp& out, const interop::GPULoadOp& in);

    [[nodiscard]] bool Convert(wgpu::StoreOp& out, const interop::GPUStoreOp& in);

    [[nodiscard]] bool Convert(wgpu::BindGroupEntry& out, const interop::GPUBindGroupEntry& in);

    [[nodiscard]] bool Convert(wgpu::BindGroupLayoutEntry& out,
                               const interop::GPUBindGroupLayoutEntry& in);

    [[nodiscard]] bool Convert(wgpu::BufferBindingLayout& out,
                               const interop::GPUBufferBindingLayout& in);

    [[nodiscard]] bool Convert(wgpu::SamplerBindingLayout& out,
                               const interop::GPUSamplerBindingLayout& in);

    [[nodiscard]] bool Convert(wgpu::TextureBindingLayout& out,
                               const interop::GPUTextureBindingLayout& in);

    [[nodiscard]] bool Convert(wgpu::StorageTextureBindingLayout& out,
                               const interop::GPUStorageTextureBindingLayout& in);

    [[nodiscard]] bool Convert(wgpu::BufferBindingType& out,
                               const interop::GPUBufferBindingType& in);

    [[nodiscard]] bool Convert(wgpu::SamplerBindingType& out,
                               const interop::GPUSamplerBindingType& in);

    [[nodiscard]] bool Convert(wgpu::TextureSampleType& out,
                               const interop::GPUTextureSampleType& in);

    [[nodiscard]] bool Convert(wgpu::StorageTextureAccess& out,
                               const interop::GPUStorageTextureAccess& in);

    [[nodiscard]] bool Convert(wgpu::QueryType& out, const interop::GPUQueryType& in);

    [[nodiscard]] bool Convert(wgpu::AddressMode& out, const interop::GPUAddressMode& in);

    [[nodiscard]] bool Convert(wgpu::FilterMode& out, const interop::GPUFilterMode& in);

    [[nodiscard]] bool Convert(wgpu::MipmapFilterMode& out, const interop::GPUMipmapFilterMode& in);

    [[nodiscard]] bool Convert(wgpu::ComputePipelineDescriptor& out,
                               const interop::GPUComputePipelineDescriptor& in);

    [[nodiscard]] bool Convert(wgpu::RenderPipelineDescriptor& out,
                               const interop::GPURenderPipelineDescriptor& in);

    [[nodiscard]] bool Convert(wgpu::PipelineLayout& out, const interop::GPUAutoLayoutMode& in);

    [[nodiscard]] bool Convert(wgpu::Bool& out, const bool& in);
    [[nodiscard]] bool Convert(wgpu::OptionalBool& out, const std::optional<bool>& in);

    // Below are the various overloads of Convert() used to convert the Dawn types -> interop.
    [[nodiscard]] bool Convert(interop::GPUTextureDimension& out, wgpu::TextureDimension in);

    [[nodiscard]] bool Convert(interop::GPUTextureFormat& out, wgpu::TextureFormat in);

    [[nodiscard]] bool Convert(interop::GPUTextureUsageFlags& out, wgpu::TextureUsage in);

    [[nodiscard]] bool Convert(interop::GPUBufferUsageFlags& out, wgpu::BufferUsage in);

    [[nodiscard]] bool Convert(interop::GPUQueryType& out, wgpu::QueryType in);

    [[nodiscard]] bool Convert(interop::GPUBufferMapState& out, wgpu::BufferMapState in);

    // These conversion methods don't generate an error when false is returned. That
    // responsibility is left to the caller if it is needed (it isn't always needed, see
    // https://gpuweb.github.io/gpuweb/#gpu-supportedfeatures)
    [[nodiscard]] bool Convert(wgpu::FeatureName& out, interop::GPUFeatureName in);
    [[nodiscard]] bool Convert(interop::GPUFeatureName& out, wgpu::FeatureName in);
    [[nodiscard]] bool Convert(wgpu::WGSLLanguageFeatureName& out,
                               interop::WGSLLanguageFeatureName in);
    [[nodiscard]] bool Convert(interop::WGSLLanguageFeatureName& out,
                               wgpu::WGSLLanguageFeatureName in);

    // std::string to C string / wgpu::StringView types
    [[nodiscard]] inline bool Convert(const char*& out, const std::string& in) {
        out = in.c_str();
        return true;
    }
    [[nodiscard]] inline bool Convert(wgpu::StringView& out, const std::string& in) {
        out = {in.data(), in.size()};
        return true;
    }

    // Floating point number conversion. IDL rules are that double/float that isn't "unrestricted"
    // must be finite.
    template <typename OUT,
              typename IN,
              std::enable_if_t<std::is_floating_point_v<IN> && std::is_floating_point_v<OUT>,
                               bool> = true>
    [[nodiscard]] inline bool Convert(OUT& out, const IN& in) {
        out = in;
        if (!std::isfinite(out)) {
            return Throw(Napi::TypeError::New(
                env, "Floating-point value (" + std::to_string(out) + ") is not finite"));
        }
        return true;
    }

    // Integral number conversion, with dynamic limit checking
    template <typename OUT,
              typename IN,
              std::enable_if_t<std::is_integral_v<IN> && std::is_integral_v<OUT>, bool> = true>
    [[nodiscard]] inline bool Convert(OUT& out, const IN& in) {
        out = static_cast<OUT>(in);
        if (static_cast<IN>(out) != in) {
            return Throw("Integer value (" + std::to_string(in) +
                         ") cannot be converted to the Dawn data type without "
                         "truncation of the value");
        }
        return true;
    }

    // ClampedInteger<T>
    template <typename T>
    [[nodiscard]] inline bool Convert(T& out, const interop::ClampedInteger<T>& in) {
        out = in;
        return true;
    }

    // EnforceRangeInteger<T>
    template <typename T>
    [[nodiscard]] inline bool Convert(T& out, const interop::EnforceRangeInteger<T>& in) {
        out = in;
        return true;
    }

    template <typename OUT, typename... IN_TYPES>
    [[nodiscard]] inline bool Convert(OUT& out, const std::variant<IN_TYPES...>& in) {
        return std::visit([&](auto&& i) { return Convert(out, i); }, in);
    }

    // If the std::optional does not have a value, then Convert() simply returns true and 'out'
    // is not assigned a new value.
    template <typename OUT, typename IN>
    [[nodiscard]] inline bool Convert(OUT& out, const std::optional<IN>& in) {
        if (in.has_value()) {
            return Convert(out, in.value());
        }
        return true;
    }

    // std::optional -> T*
    // OUT* is assigned either a pointer to the converted value, or nullptr, depending on
    // whether 'in' has a value.
    template <typename OUT,
              typename IN,
              typename _ = std::enable_if_t<!std::is_same_v<IN, std::string>>>
    [[nodiscard]] inline bool Convert(OUT*& out, const std::optional<IN>& in) {
        if (in.has_value()) {
            auto* el = Allocate<std::remove_const_t<OUT>>();
            if (!Convert(*el, in.value())) {
                return false;
            }
            out = el;
        } else {
            out = nullptr;
        }
        return true;
    }

    // interop::Interface -> Dawn object
    template <typename OUT, typename IN>
    [[nodiscard]] inline bool Convert(OUT& out, const interop::Interface<IN>& in) {
        using Impl = ImplOf<IN>;
        out = *in.template As<Impl>();
        if (!out) {
            LOG("Dawn object has been destroyed. This should not happen");
            return false;
        }
        return true;
    }

    // vector -> raw pointer + count
    template <typename OUT, typename IN>
    [[nodiscard]] inline bool Convert(OUT*& out_els, size_t& out_count, const std::vector<IN>& in) {
        if (in.size() == 0) {
            out_els = nullptr;
            out_count = 0;
            return true;
        }
        auto* els = Allocate<std::remove_const_t<OUT>>(in.size());
        for (size_t i = 0; i < in.size(); i++) {
            if (!Convert(els[i], in[i])) {
                return false;
            }
        }
        out_els = els;
        return Convert(out_count, in.size());
    }

    // unordered_map -> raw pointer + count
    template <typename OUT, typename IN_KEY, typename IN_VALUE>
    [[nodiscard]] inline bool Convert(OUT*& out_els,
                                      size_t& out_count,
                                      const std::unordered_map<IN_KEY, IN_VALUE>& in) {
        if (in.size() == 0) {
            out_els = nullptr;
            out_count = 0;
            return true;
        }
        auto* els = Allocate<std::remove_const_t<OUT>>(in.size());
        size_t i = 0;
        for (auto& [key, value] : in) {
            if (!Convert(els[i++], key, value)) {
                return false;
            }
        }
        out_els = els;
        return Convert(out_count, in.size());
    }

    // std::optional<T> -> raw pointer + count
    template <typename OUT, typename IN>
    [[nodiscard]] inline bool Convert(OUT*& out_els,
                                      size_t& out_count,
                                      const std::optional<IN>& in) {
        if (!in.has_value()) {
            out_els = nullptr;
            out_count = 0;
            return true;
        }
        return Convert(out_els, out_count, in.value());
    }

    // JS strings can contain the null character, replace it with some other invalid character
    // to preserve the creation of errors in this case.
    char* ConvertStringReplacingNull(std::string_view in);

    Napi::Env env;
    wgpu::Device device = nullptr;

    bool HasFeature(wgpu::FeatureName feature);

    // Function to be used when throwing a JavaScript exception because of issues during the
    // conversion. Because the conversion should stop immediately, this method returns false,
    // so the pattern is:
    //
    //    if (some error case) {
    //        return Throw("Some error case description");
    //    }
    [[nodiscard]] bool Throw(std::string&& message);
    [[nodiscard]] bool Throw(Napi::Error&& error);

    // Allocate() allocates and constructs an array of 'n' elements, and returns a pointer to
    // the first element. The array is freed when the Converter is destructed.
    template <typename T>
    T* Allocate(size_t n = 1) {
        auto* ptr = new T[n]{};
        free_.emplace_back([ptr] { delete[] ptr; });
        return ptr;
    }

    std::vector<std::function<void()>> free_;
};

std::string CopyLabel(StringView label);

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_CONVERTER_H_
