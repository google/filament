/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_TEST_MOCK_DRIVER_H
#define TNT_FILAMENT_TEST_MOCK_DRIVER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "private/backend/CommandStream.h"
#include "private/backend/Driver.h"

using namespace filament;
using namespace filament::backend;

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

// A mock implementation of the backend::Driver for unit testing.
//
// This mock is primarily designed for testing synchronous APIs and functions that
// return a value immediately (those with the "S" suffix, like `createBufferObjectS`).
// It does not fully support asynchronous command stream behavior, so its use for
// testing complex rendering logic is limited.
class MockDriver : public Driver {
public:
    MockDriver() = default;
    ~MockDriver() override = default;

    uint64_t nextFakeHandle = 1;

    MOCK_METHOD(void, terminate, (), (override));
    MOCK_METHOD(backend::StreamHandle, createStreamNative,
            (void* stream, utils::ImmutableCString tag), (override));
    MOCK_METHOD(backend::StreamHandle, createStreamAcquired, (utils::ImmutableCString tag),
            (override));
    MOCK_METHOD(void, setAcquiredImage,
            (backend::StreamHandle stream, void* image, const math::mat3f& transform,
                    backend::CallbackHandler* handler, backend::StreamCallback cb, void* userData),
            (override));
    MOCK_METHOD(void, setStreamDimensions,
            (backend::StreamHandle stream, uint32_t width, uint32_t height), (override));
    MOCK_METHOD(int64_t, getStreamTimestamp, (backend::StreamHandle stream), (override));
    MOCK_METHOD(void, updateStreams, (backend::DriverApi * driver), (override));
    MOCK_METHOD(backend::FenceStatus, getFenceStatus, (backend::FenceHandle fh), (override));
    MOCK_METHOD(backend::FenceStatus, fenceWait, (backend::FenceHandle fh, uint64_t timeout),
            (override));
    MOCK_METHOD(void, getPlatformSync,
            (backend::SyncHandle sh, backend::CallbackHandler* handler,
                    backend::Platform::SyncCallback cb, void* userData),
            (override));
    MOCK_METHOD(bool, isTextureFormatSupported, (backend::TextureFormat format), (override));
    MOCK_METHOD(bool, isTextureSwizzleSupported, (), (override));
    MOCK_METHOD(bool, isTextureFormatMipmappable, (backend::TextureFormat format), (override));
    MOCK_METHOD(bool, isRenderTargetFormatSupported, (backend::TextureFormat format), (override));
    MOCK_METHOD(bool, isFrameBufferFetchSupported, (), (override));
    MOCK_METHOD(bool, isFrameBufferFetchMultiSampleSupported, (), (override));
    MOCK_METHOD(bool, isFrameTimeSupported, (), (override));
    MOCK_METHOD(bool, isAutoDepthResolveSupported, (), (override));
    MOCK_METHOD(bool, isSRGBSwapChainSupported, (), (override));
    MOCK_METHOD(bool, isMSAASwapChainSupported, (uint32_t samples), (override));
    MOCK_METHOD(bool, isProtectedContentSupported, (), (override));
    MOCK_METHOD(bool, isStereoSupported, (), (override));
    MOCK_METHOD(bool, isParallelShaderCompileSupported, (), (override));
    MOCK_METHOD(bool, isDepthStencilResolveSupported, (), (override));
    MOCK_METHOD(bool, isDepthStencilBlitSupported, (backend::TextureFormat format), (override));
    MOCK_METHOD(bool, isProtectedTexturesSupported, (), (override));
    MOCK_METHOD(bool, isDepthClampSupported, (), (override));
    MOCK_METHOD(uint8_t, getMaxDrawBuffers, (), (override));
    MOCK_METHOD(size_t, getMaxUniformBufferSize, (), (override));
    MOCK_METHOD(size_t, getMaxTextureSize, (backend::SamplerType target), (override));
    MOCK_METHOD(size_t, getMaxArrayTextureLayers, (), (override));
    MOCK_METHOD(math::float2, getClipSpaceParams, (), (override));
    MOCK_METHOD(void, setupExternalImage2, (backend::Platform::ExternalImageHandleRef image),
            (override));
    MOCK_METHOD(void, setupExternalImage, (void* image), (override));
    MOCK_METHOD(backend::TimerQueryResult, getTimerQueryValue,
            (backend::TimerQueryHandle tqh, uint64_t* elapsedTime), (override));
    MOCK_METHOD(bool, isWorkaroundNeeded, (backend::Workaround workaround), (override));
    MOCK_METHOD(backend::FeatureLevel, getFeatureLevel, (), (override));
    MOCK_METHOD(size_t, getUniformBufferOffsetAlignment, (), (override));
    MOCK_METHOD(bool, isCompositorTimingSupported, (), (override));
    MOCK_METHOD(bool, queryFrameTimestamps, (SwapChainHandle, uint64_t, FrameTimestamps*), (override));
    MOCK_METHOD(bool, queryCompositorTiming, (SwapChainHandle, CompositorTiming*), (override));
    MOCK_METHOD(void, fenceCancel, (backend::FenceHandle), (override));


    ShaderModel getShaderModel() const noexcept final { return ShaderModel::DESKTOP; }
    utils::FixedCapacityVector<ShaderLanguage> getShaderLanguages(
            ShaderLanguage preferredLanguage) const noexcept final {
        return {
            ShaderLanguage::ESSL3,
            ShaderLanguage::ESSL1,
            ShaderLanguage::SPIRV,
            ShaderLanguage::MSL,
            ShaderLanguage::METAL_LIBRARY,
            ShaderLanguage::WGSL,
        };
    }
    void debugCommandBegin(CommandStream* cmds, bool synchronous,
            const char* methodName) noexcept override {}
    void debugCommandEnd(CommandStream* cmds, bool synchronous,
            const char* methodName) noexcept override {}
    void purge() noexcept override {}

    template<typename T>
    friend class ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params)                                            \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl) {}
#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)
#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)                            \
    RetType methodName##S() noexcept override {                                                    \
        return RetType((RetType::HandleId) nextFakeHandle++);                                      \
    }                                                                                              \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl) {}

#include "private/backend/DriverAPI.inc"

    Dispatcher getDispatcher() const noexcept override {
        Dispatcher dispatcher;
        // Currently we don't have a good way to mock the functions for asynchronous backend API.
        // We set all API to nullptr to let the users know it doesn't work.
#define DECL_DRIVER_API(methodName, paramsDecl, params) dispatcher.methodName##_ = nullptr;
#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)                            \
    dispatcher.methodName##_ = nullptr;
#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)
#include "private/backend/DriverAPI.inc"

        return dispatcher;
    }
};

#endif // TNT_FILAMENT_TEST_MOCK_DRIVER_H
