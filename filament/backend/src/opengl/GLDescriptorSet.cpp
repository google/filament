/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "GLDescriptorSet.h"

#include "GLBufferObject.h"
#include "GLDescriptorSetLayout.h"
#include "GLTexture.h"
#include "GLUtils.h"
#include "OpenGLDriver.h"
#include "OpenGLContext.h"
#include "OpenGLProgram.h"

#include "gl_headers.h"

#include <private/backend/HandleAllocator.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/BitmaskEnum.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <algorithm>

#include <type_traits>
#include <utility>
#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

GLDescriptorSet::GLDescriptorSet(OpenGLContext& gl, DescriptorSetLayoutHandle dslh,
        GLDescriptorSetLayout const* layout) noexcept
        : descriptors(layout->maxDescriptorBinding + 1),
          dslh(std::move(dslh)) {

    // We have allocated enough storage for all descriptors. Now allocate the empty descriptor
    // themselves.
    for (auto const& entry : layout->bindings) {
        size_t const index = entry.binding;

        // now we'll initialize the alternative for each way we can handle this descriptor.
        auto& desc = descriptors[index].desc;
        switch (entry.type) {
            case DescriptorType::UNIFORM_BUFFER: {
                // A uniform buffer can have dynamic offsets or not and have special handling for
                // ES2 (where we need to emulate it). That's four alternatives.
                bool const dynamicOffset = any(entry.flags & DescriptorFlags::DYNAMIC_OFFSET);
                dynamicBuffers.set(index, dynamicOffset);
                if (UTILS_UNLIKELY(gl.isES2())) {
                    dynamicBufferCount++;
                    desc.emplace<BufferGLES2>(dynamicOffset);
                } else {
                    auto const type = GLUtils::getBufferBindingType(BufferObjectBinding::UNIFORM);
                    if (dynamicOffset) {
                        dynamicBufferCount++;
                        desc.emplace<DynamicBuffer>(type);
                    } else {
                        desc.emplace<Buffer>(type);
                    }
                }
                break;
            }
            case DescriptorType::SHADER_STORAGE_BUFFER: {
                // shader storage buffers are not supported on ES2, So that's two alternatives.
                bool const dynamicOffset = any(entry.flags & DescriptorFlags::DYNAMIC_OFFSET);
                dynamicBuffers.set(index, dynamicOffset);
                auto const type = GLUtils::getBufferBindingType(BufferObjectBinding::SHADER_STORAGE);
                if (dynamicOffset) {
                    dynamicBufferCount++;
                    desc.emplace<DynamicBuffer>(type);
                } else {
                    desc.emplace<Buffer>(type);
                }
                break;
            }
            case DescriptorType::SAMPLER:
                if (UTILS_UNLIKELY(gl.isES2())) {
                    desc.emplace<SamplerGLES2>();
                } else {
                    const bool anisotropyWorkaround =
                            gl.ext.EXT_texture_filter_anisotropic &&
                            gl.bugs.texture_filter_anisotropic_broken_on_sampler;
                    if (anisotropyWorkaround) {
                        desc.emplace<SamplerWithAnisotropyWorkaround>();
                    } else {
                        desc.emplace<Sampler>();
                    }
                }
                break;
            case DescriptorType::INPUT_ATTACHMENT:
                break;
        }
    }
}

void GLDescriptorSet::update(OpenGLContext&,
        descriptor_binding_t binding, GLBufferObject* bo, size_t offset, size_t size) noexcept {
    assert_invariant(binding < descriptors.size());
    std::visit([=](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Buffer> || std::is_same_v<T, DynamicBuffer>) {
            assert_invariant(arg.target != 0);
            arg.id = bo ? bo->gl.id : 0;
            arg.offset = uint32_t(offset);
            arg.size = uint32_t(size);
            assert_invariant(arg.id || (!arg.size && !offset));
        } else if constexpr (std::is_same_v<T, BufferGLES2>) {
            arg.bo = bo;
            arg.offset = uint32_t(offset);
        } else {
            // API usage error. User asked to update the wrong type of descriptor.
            PANIC_PRECONDITION("descriptor %d is not a buffer", +binding);
        }
    }, descriptors[binding].desc);
}

void GLDescriptorSet::update(OpenGLContext& gl,
        descriptor_binding_t binding, GLTexture* t, SamplerParams params) noexcept {
    assert_invariant(binding < descriptors.size());
    std::visit([=, &gl](auto&& arg) mutable {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Sampler> ||
                      std::is_same_v<T, SamplerWithAnisotropyWorkaround> ||
                      std::is_same_v<T, SamplerGLES2>) {
            if (UTILS_UNLIKELY(t && t->target == SamplerType::SAMPLER_EXTERNAL)) {
                // From OES_EGL_image_external spec:
                // "The default s and t wrap modes are CLAMP_TO_EDGE, and it is an INVALID_ENUM
                //  error to set the wrap mode to any other value."
                params.wrapS = SamplerWrapMode::CLAMP_TO_EDGE;
                params.wrapT = SamplerWrapMode::CLAMP_TO_EDGE;
                params.wrapR = SamplerWrapMode::CLAMP_TO_EDGE;
            }
            // GLES3.x specification forbids depth textures to be filtered.
            if (t && isDepthFormat(t->format)
                    && params.compareMode == SamplerCompareMode::NONE) {
                params.filterMag = SamplerMagFilter::NEAREST;
                switch (params.filterMin) {
                    case SamplerMinFilter::LINEAR:
                        params.filterMin = SamplerMinFilter::NEAREST;
                        break;
                    case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
                    case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
                    case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
                        params.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                        break;
                    default:
                        break;
                }
            }

            arg.target = t ? t->gl.target : 0;
            arg.id = t ? t->gl.id : 0;
            if constexpr (std::is_same_v<T, Sampler> ||
                          std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
                if constexpr (std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
                    arg.anisotropy = float(1u << params.anisotropyLog2);
                }
                if (t) {
                    arg.ref = t->ref;
                    arg.baseLevel = t->gl.baseLevel;
                    arg.maxLevel = t->gl.maxLevel;
                    arg.swizzle = t->gl.swizzle;
                }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                arg.sampler = gl.getSampler(params);
#else
                (void)gl;
#endif
            } else {
                arg.params = params;
            }
        } else {
            // API usage error. User asked to update the wrong type of descriptor.
            PANIC_PRECONDITION("descriptor %d is not a texture", +binding);
        }
    }, descriptors[binding].desc);
}

template<typename T>
void GLDescriptorSet::updateTextureView(OpenGLContext& gl,
        HandleAllocatorGL& handleAllocator, GLuint unit, T const& desc) noexcept {
    // The common case is that we don't have a ref handle (we only have one if
    // the texture ever had a View on it).
    assert_invariant(desc.ref);
    GLTextureRef* const ref = handleAllocator.handle_cast<GLTextureRef*>(desc.ref);
    if (UTILS_UNLIKELY((desc.baseLevel != ref->baseLevel || desc.maxLevel != ref->maxLevel))) {
        // If we have views, then it's still uncommon that we'll switch often
        // handle the case where we reset to the original texture
        GLint baseLevel = GLint(desc.baseLevel); // NOLINT(*-signed-char-misuse)
        GLint maxLevel = GLint(desc.maxLevel); // NOLINT(*-signed-char-misuse)
        if (baseLevel > maxLevel) {
            baseLevel = 0;
            maxLevel = 1000; // per OpenGL spec
        }
        // that is very unfortunate that we have to call activeTexture here
        gl.activeTexture(unit);
        glTexParameteri(desc.target, GL_TEXTURE_BASE_LEVEL, baseLevel);
        glTexParameteri(desc.target, GL_TEXTURE_MAX_LEVEL,  maxLevel);
        ref->baseLevel = desc.baseLevel;
        ref->maxLevel = desc.maxLevel;
    }
    if (UTILS_UNLIKELY(desc.swizzle != ref->swizzle)) {
        using namespace GLUtils;
        gl.activeTexture(unit);
#if !defined(__EMSCRIPTEN__)  && !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
        glTexParameteri(desc.target, GL_TEXTURE_SWIZZLE_R, (GLint)getSwizzleChannel(desc.swizzle[0]));
        glTexParameteri(desc.target, GL_TEXTURE_SWIZZLE_G, (GLint)getSwizzleChannel(desc.swizzle[1]));
        glTexParameteri(desc.target, GL_TEXTURE_SWIZZLE_B, (GLint)getSwizzleChannel(desc.swizzle[2]));
        glTexParameteri(desc.target, GL_TEXTURE_SWIZZLE_A, (GLint)getSwizzleChannel(desc.swizzle[3]));
#endif
        ref->swizzle = desc.swizzle;
    }
}

void GLDescriptorSet::bind(
        OpenGLContext& gl,
        HandleAllocatorGL& handleAllocator,
        OpenGLProgram const& p,
        descriptor_set_t set, uint32_t const* offsets, bool offsetsOnly) const noexcept {
    // TODO: check that offsets is sized correctly
    size_t dynamicOffsetIndex = 0;

    utils::bitset64 activeDescriptorBindings = p.getActiveDescriptors(set);
    if (offsetsOnly) {
        activeDescriptorBindings &= dynamicBuffers;
    }

    // loop only over the active indices for this program
    activeDescriptorBindings.forEachSetBit(
            [this,&gl, &handleAllocator, &p, set, offsets, &dynamicOffsetIndex]
            (size_t binding) {

        // This would fail here if we're trying to set a descriptor that doesn't exist in the
        // program. In other words, a mismatch between the program's layout and this descriptor-set.
        assert_invariant(binding < descriptors.size());

        auto const& entry = descriptors[binding];
        std::visit(
                [&gl, &handleAllocator, &p, &dynamicOffsetIndex, set, binding, offsets]
                (auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Buffer>) {
                GLuint const bindingPoint = p.getBufferBinding(set, binding);
                GLintptr const offset = arg.offset;
                assert_invariant(arg.id || (!arg.size && !offset));
                gl.bindBufferRange(arg.target, bindingPoint, arg.id, offset, arg.size);
            } else if constexpr (std::is_same_v<T, DynamicBuffer>) {
                GLuint const bindingPoint = p.getBufferBinding(set, binding);
                GLintptr const offset = arg.offset + offsets[dynamicOffsetIndex++];
                assert_invariant(arg.id || (!arg.size && !offset));
                gl.bindBufferRange(arg.target, bindingPoint, arg.id, offset, arg.size);
            } else if constexpr (std::is_same_v<T, BufferGLES2>) {
                GLuint const bindingPoint = p.getBufferBinding(set, binding);
                GLintptr offset = arg.offset;
                if (arg.dynamicOffset) {
                    offset += offsets[dynamicOffsetIndex++];
                }
                if (arg.bo) {
                    auto buffer = static_cast<char const*>(arg.bo->gl.buffer) + offset;
                    p.updateUniforms(bindingPoint, arg.bo->gl.id, buffer, arg.bo->age);
                }
            } else if constexpr (std::is_same_v<T, Sampler>) {
                GLuint const unit = p.getTextureUnit(set, binding);
                if (arg.target) {
                    gl.bindTexture(unit, arg.target, arg.id);
                    gl.bindSampler(unit, arg.sampler);
                    if (UTILS_UNLIKELY(arg.ref)) {
                        updateTextureView(gl, handleAllocator, unit, arg);
                    }
                } else {
                    gl.unbindTextureUnit(unit);
                }
            } else if constexpr (std::is_same_v<T, SamplerWithAnisotropyWorkaround>) {
                GLuint const unit = p.getTextureUnit(set, binding);
                if (arg.target) {
                    gl.bindTexture(unit, arg.target, arg.id);
                    gl.bindSampler(unit, arg.sampler);
                    if (UTILS_UNLIKELY(arg.ref)) {
                        updateTextureView(gl, handleAllocator, unit, arg);
                    }
#if defined(GL_EXT_texture_filter_anisotropic)
                    // Driver claims to support anisotropic filtering, but it fails when set on
                    // the sampler, we have to set it on the texture instead.
                    glTexParameterf(arg.target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                            std::min(gl.gets.max_anisotropy, float(arg.anisotropy)));
#endif
                } else {
                    gl.unbindTextureUnit(unit);
                }
            } else if constexpr (std::is_same_v<T, SamplerGLES2>) {
                // in ES2 the sampler parameters need to be set on the texture itself
                GLuint const unit = p.getTextureUnit(set, binding);
                if (arg.target) {
                    gl.bindTexture(unit, arg.target, arg.id);
                    SamplerParams const params = arg.params;
                    glTexParameteri(arg.target, GL_TEXTURE_MIN_FILTER,
                            (GLint)GLUtils::getTextureFilter(params.filterMin));
                    glTexParameteri(arg.target, GL_TEXTURE_MAG_FILTER,
                            (GLint)GLUtils::getTextureFilter(params.filterMag));
                    glTexParameteri(arg.target, GL_TEXTURE_WRAP_S,
                            (GLint)GLUtils::getWrapMode(params.wrapS));
                    glTexParameteri(arg.target, GL_TEXTURE_WRAP_T,
                            (GLint)GLUtils::getWrapMode(params.wrapT));
#if defined(GL_EXT_texture_filter_anisotropic)
                    glTexParameterf(arg.target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                            std::min(gl.gets.max_anisotropy, arg.anisotropy));
#endif
                } else {
                    gl.unbindTextureUnit(unit);
                }
            }
        }, entry.desc);
    });
    CHECK_GL_ERROR(utils::slog.e)
}

void GLDescriptorSet::validate(HandleAllocatorGL& allocator,
        DescriptorSetLayoutHandle pipelineLayout) const {

    if (UTILS_UNLIKELY(dslh != pipelineLayout)) {
        auto* const dsl = allocator.handle_cast < GLDescriptorSetLayout const * > (dslh);
        auto* const cur = allocator.handle_cast < GLDescriptorSetLayout const * > (pipelineLayout);

        UTILS_UNUSED_IN_RELEASE
        bool const pipelineLayoutMatchesDescriptorSetLayout = std::equal(
                dsl->bindings.begin(), dsl->bindings.end(),
                cur->bindings.begin(),
                [](DescriptorSetLayoutBinding const& lhs,
                        DescriptorSetLayoutBinding const& rhs) {
                    return lhs.type == rhs.type &&
                           lhs.stageFlags == rhs.stageFlags &&
                           lhs.binding == rhs.binding &&
                           lhs.flags == rhs.flags &&
                           lhs.count == rhs.count;
                });

        assert_invariant(pipelineLayoutMatchesDescriptorSetLayout);
    }
}

} // namespace filament::backend
