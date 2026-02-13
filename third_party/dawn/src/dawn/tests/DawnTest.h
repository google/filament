// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_TESTS_DAWNTEST_H_
#define SRC_DAWN_TESTS_DAWNTEST_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/common/Mutex.h"
#include "dawn/common/Platform.h"
#include "dawn/common/Preprocessor.h"
#include "dawn/dawn_proc_table.h"
#include "dawn/native/DawnNative.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/tests/AdapterTestConfig.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/ParamGenerator.h"
#include "dawn/tests/ToggleParser.h"
#include "dawn/utils/ComboLimits.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/Timer.h"
#include "dawn/webgpu_cpp_print.h"
#include "partition_alloc/pointers/raw_ptr.h"

// Getting data back from Dawn is done in an async manners so all expectations are "deferred"
// until the end of the test. Also expectations use a copy to a MapRead buffer to get the data
// so resources should have the CopySrc allowed usage bit if you want to add expectations on
// them.

// AddBufferExpectation is defined in DawnTestBase as protected function. This ensures the macro can
// only be used in derivd class of DawnTestBase. Use "this" pointer to ensure the macro works with
// CRTP.
#define EXPECT_BUFFER(buffer, offset, size, expectation) \
    this->AddBufferExpectation(__FILE__, __LINE__, buffer, offset, size, expectation)

#define EXPECT_BUFFER_U8_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint8_t), new ::dawn::detail::ExpectEq<uint8_t>(expected))

#define EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint8_t) * (count),       \
                  new ::dawn::detail::ExpectEq<uint8_t>(expected, count))

#define EXPECT_BUFFER_U16_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint16_t),    \
                  new ::dawn::detail::ExpectEq<uint16_t>(expected))

#define EXPECT_BUFFER_U16_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint16_t) * (count),       \
                  new ::dawn::detail::ExpectEq<uint16_t>(expected, count))

#define EXPECT_BUFFER_U32_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint32_t),    \
                  new ::dawn::detail::ExpectEq<uint32_t>(expected))

#define EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint32_t) * (count),       \
                  new ::dawn::detail::ExpectEq<uint32_t>(expected, count))

#define EXPECT_BUFFER_U64_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint64_t),    \
                  new ::dawn::detail::ExpectEq<uint64_t>(expected))

#define EXPECT_BUFFER_U64_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint64_t) * (count),       \
                  new ::dawn::detail::ExpectEq<uint64_t>(expected, count))

#define EXPECT_BUFFER_FLOAT_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(float), new ::dawn::detail::ExpectEq<float>(expected))

#define EXPECT_BUFFER_FLOAT_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(float) * (count),            \
                  new ::dawn::detail::ExpectEq<float>(expected, count))

// Test a pixel of the mip level 0 of a 2D texture.
#define EXPECT_PIXEL_RGBA8_EQ(expected, texture, x, y) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, {x, y})

#define EXPECT_PIXEL_FLOAT_EQ(expected, texture, x, y) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, {x, y})

#define EXPECT_PIXEL_FLOAT16_EQ(expected, texture, x, y) \
    AddTextureExpectation<float, uint16_t>(__FILE__, __LINE__, expected, texture, {x, y})

#define EXPECT_PIXEL_RGBA8_BETWEEN(color0, color1, texture, x, y) \
    AddTextureBetweenColorsExpectation(__FILE__, __LINE__, color0, color1, texture, x, y)

#define EXPECT_TEXTURE_EQ(...) AddTextureExpectation(__FILE__, __LINE__, __VA_ARGS__)

#define EXPECT_TEXTURE_FLOAT16_EQ(...) \
    AddTextureExpectation<float, uint16_t>(__FILE__, __LINE__, __VA_ARGS__)

#define EXPECT_TEXTURE_NORM_BETWEEN(...) \
    AddSnormTextureBoundsExpectation(__FILE__, __LINE__, __VA_ARGS__)

// Matcher for C++ types to verify that their internal C-handles are identical.
MATCHER_P(CHandleIs, cType, "") {
    return arg.Get() == cType;
}

#define ASSERT_DEVICE_ERROR_MSG_ON(device, statement, matcher)                                  \
    FlushWire();                                                                                \
    EXPECT_CALL(mDeviceErrorCallback,                                                           \
                Call(CHandleIs(device.Get()), testing::Ne(wgpu::ErrorType::NoError), matcher)); \
    statement;                                                                                  \
    instance.ProcessEvents();                                                                   \
    FlushWire();                                                                                \
    testing::Mock::VerifyAndClearExpectations(&mDeviceErrorCallback);                           \
    do {                                                                                        \
    } while (0)

#define ASSERT_DEVICE_ERROR_MSG(statement, matcher) \
    ASSERT_DEVICE_ERROR_MSG_ON(this->device, statement, matcher)

#define ASSERT_DEVICE_ERROR_ON(device, statement) \
    ASSERT_DEVICE_ERROR_MSG_ON(device, statement, testing::_)

#define ASSERT_DEVICE_ERROR(statement) ASSERT_DEVICE_ERROR_MSG(statement, testing::_)

struct GLFWwindow;

void InitDawnEnd2EndTestEnvironment(int argc, char** argv);

namespace dawn {
namespace utils {
class PlatformDebugLogger;
class TerribleCommandBuffer;
class WireHelper;
}  // namespace utils

namespace detail {
class Expectation;
class CustomTextureExpectation;

template <typename T>
class ExpectConstant;
template <typename T, typename U = T>
class ExpectEq;
template <typename T>
class ExpectBetweenColors;
template <typename T>
class ExpectBetweenSnormTextureBounds;
}  // namespace detail

namespace wire {
class CommandHandler;
class WireClient;
class WireServer;
}  // namespace wire

class DawnTestEnvironment : public testing::Environment {
  public:
    DawnTestEnvironment(int argc, char** argv);
    ~DawnTestEnvironment() override;

    static void SetEnvironment(DawnTestEnvironment* env);

    std::vector<AdapterTestParam> GetAvailableAdapterTestParamsForBackends(
        const BackendTestConfig* params,
        size_t numParams);

    void SetUp() override;
    void TearDown() override;

    bool UsesWire() const;
    bool IsImplicitDeviceSyncEnabled() const;
    native::BackendValidationLevel GetBackendValidationLevel() const;
    native::Instance* GetInstance() const;
    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;
    bool HasBackendTypeFilter() const;
    wgpu::BackendType GetBackendTypeFilter() const;
    const char* GetWireTraceDir() const;

    const std::vector<std::string>& GetEnabledToggles() const;
    const std::vector<std::string>& GetDisabledToggles() const;

    bool RunSuppressedTests() const;

  protected:
    std::unique_ptr<native::Instance> CreateInstance(platform::Platform* platform = nullptr);
    std::unique_ptr<native::Instance> mInstance;

  private:
    void ParseArgs(int argc, char** argv);
    void SelectPreferredAdapterProperties(const native::Instance* instance);
    void PrintTestConfigurationAndAdapterInfo(native::Instance* instance) const;

    /// @returns true if all the toggles are recognised, otherwise prints an error and returns
    /// false.
    bool ValidateToggles(native::Instance* instance) const;

    bool mUseWire = false;
    bool mEnableImplicitDeviceSync = false;
    native::BackendValidationLevel mBackendValidationLevel =
        native::BackendValidationLevel::Disabled;
    std::string mANGLEBackend;
    bool mBeginCaptureOnStartup = false;
    bool mHasVendorIdFilter = false;
    uint32_t mVendorIdFilter = 0;
    bool mHasBackendTypeFilter = false;
    wgpu::BackendType mBackendTypeFilter;
    std::string mWireTraceDir;
    bool mRunSuppressedTests = false;

    ToggleParser mToggleParser;

    std::vector<wgpu::AdapterType> mDevicePreferences;
    std::vector<TestAdapterProperties> mAdapterProperties;

    std::unique_ptr<utils::PlatformDebugLogger> mPlatformDebugLogger;
};

class DawnTestBase {
    friend class DawnPerfTestBase;

  public:
    explicit DawnTestBase(const AdapterTestParam& param);
    virtual ~DawnTestBase();

    void SetUp();
    void TearDown();

    bool IsD3D11() const;
    bool IsD3D12() const;
    bool IsMetal() const;
    bool IsNull() const;
    bool IsWebGPUOnWebGPU() const;
    bool IsOpenGL() const;
    bool IsOpenGLES() const;
    bool IsVulkan() const;

    bool IsAMD() const;
    bool IsApple() const;
    bool IsARM() const;
    bool IsImgTec() const;
    bool IsIntel() const;
    bool IsNvidia() const;
    bool IsQualcomm() const;
    bool IsSwiftshader() const;
    bool IsANGLE() const;
    bool IsANGLESwiftShader() const;
    bool IsANGLED3D11() const;
    bool IsWARP() const;
    bool IsMesaSoftware() const;

    bool IsIntelGen9() const;
    bool IsIntelGen12() const;
    bool IsIntelGen12OrLater() const;

    bool IsWindows() const;
    bool IsLinux() const;
    bool IsMacOS(int32_t majorVersion = -1, int32_t minorVersion = -1) const;
    bool IsAndroid() const;
    bool IsChromeOS() const;

    bool IsMesa(const std::string& mesaVersion = "") const;

    bool UsesWire() const;
    bool IsImplicitDeviceSyncEnabled() const;
    bool IsBackendValidationEnabled() const;
    bool IsFullBackendValidationEnabled() const;
    bool IsCompatibilityMode() const;
    bool IsCPU() const;
    bool RunSuppressedTests() const;

    bool IsDXC() const;

    static bool IsAsan();
    static bool IsTsan();

    bool HasToggleEnabled(const char* workaround) const;

    void DestroyDevice(wgpu::Device device = nullptr);
    void LoseDeviceForTesting(wgpu::Device device = nullptr);

    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;

    bool HasBackendTypeFilter() const;
    wgpu::BackendType GetBackendTypeFilter() const;

    const wgpu::Instance& GetInstance() const;
    native::Adapter GetAdapter() const;

    virtual std::unique_ptr<platform::Platform> CreateTestPlatform();

    struct PrintToStringParamName {
        explicit PrintToStringParamName(const char* test);
        std::string SanitizeParamName(std::string paramName,
                                      const TestAdapterProperties& properties,
                                      size_t index) const;

        template <class ParamType>
        std::string operator()(const ::testing::TestParamInfo<ParamType>& info) const {
            return SanitizeParamName(::testing::PrintToStringParamName()(info),
                                     info.param.adapterProperties, info.index);
        }

        std::string mTest;
    };

    // Resolve all the deferred expectations in mDeferredExpectations now to avoid letting
    // mDeferredExpectations get too big.
    void ResolveDeferredExpectationsNow();

    // Starts the internal timer for the test and sets the max expected time. This
    // 'max_expected_time' is not actually a test timeout as it simply checks an expectation at the
    // end of the test.
    void StartTestTimer(float expected_max_time);

  protected:
    wgpu::Instance instance;
    wgpu::Adapter adapter;
    dawn::utils::ComboLimits adapterLimits;
    wgpu::Device device;
    dawn::utils::ComboLimits deviceLimits;
    wgpu::Queue queue;

    DawnProcTable backendProcs = {};
    WGPUDevice backendDevice = nullptr;

    uint64_t mLastWarningCount = 0;
    std::unique_ptr<utils::Timer> mTimer;
    float mExpectedTimeMaxSec = 0.0f;

    // Mock callbacks tracking errors and destruction. These are strict mocks because any errors or
    // device loss that aren't expected should result in test failures and not just some warnings
    // printed to stdout.
    testing::StrictMock<testing::MockCppCallback<wgpu::UncapturedErrorCallback<void>*>>
        mDeviceErrorCallback;
    testing::StrictMock<testing::MockCppCallback<wgpu::DeviceLostCallback<void>*>>
        mDeviceLostCallback;

    // Helper methods to implement the EXPECT_ macros
    std::ostringstream& AddBufferExpectation(const char* file,
                                             int line,
                                             const wgpu::Buffer& buffer,
                                             uint64_t offset,
                                             uint64_t size,
                                             detail::Expectation* expectation);

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              wgpu::TextureFormat format,
                                              T tolerance = 0,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        // No device passed explicitly. Default it, and forward the rest of the args.
        return AddTextureExpectation<T, U>(file, line, this->device, expectedData, texture, origin,
                                           extent, format, tolerance, level, aspect, bytesPerRow);
    }

    // T - expected value Type
    // U - actual value Type (defaults = T)
    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              wgpu::Device targetDevice,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              wgpu::TextureFormat format,
                                              T tolerance = 0,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        uint32_t texelBlockSize = utils::GetTexelBlockSizeInBytes(format);
        uint32_t texelComponentCount = utils::GetTextureComponentCount(format);

        return AddTextureExpectationImpl(
            file, line, std::move(targetDevice),
            new detail::ExpectEq<T, U>(
                expectedData,
                texelComponentCount * extent.width * extent.height * extent.depthOrArrayLayers,
                tolerance),
            texture, origin, extent, level, aspect, texelBlockSize, bytesPerRow);
    }

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0,
                                              T tolerance = {}) {
        // No device passed explicitly. Default it, and forward the rest of the args.
        return AddTextureExpectation<T, U>(file, line, this->device, expectedData, texture, origin,
                                           extent, level, aspect, bytesPerRow, tolerance);
    }

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              wgpu::Device targetDevice,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0,
                                              T tolerance = {}) {
        return AddTextureExpectationImpl(
            file, line, std::move(targetDevice),
            new detail::ExpectEq<T, U>(
                expectedData, extent.width * extent.height * extent.depthOrArrayLayers, tolerance),
            texture, origin, extent, level, aspect, sizeof(U), bytesPerRow);
    }

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T& expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        // No device passed explicitly. Default it, and forward the rest of the args.
        return AddTextureExpectation<T, U>(file, line, this->device, expectedData, texture, origin,
                                           level, aspect, bytesPerRow);
    }

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              wgpu::Device targetDevice,
                                              const T& expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(file, line, std::move(targetDevice),
                                         new detail::ExpectEq<T, U>(expectedData), texture, origin,
                                         {1, 1}, level, aspect, sizeof(U), bytesPerRow);
    }

    template <typename E,
              typename = typename std::enable_if<
                  std::is_base_of<detail::CustomTextureExpectation, E>::value>::type>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              E* expectation,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        // No device passed explicitly. Default it, and forward the rest of the args.
        return AddTextureExpectation(file, line, this->device, expectation, texture, origin, extent,
                                     level, aspect, bytesPerRow);
    }

    template <typename E,
              typename = typename std::enable_if<
                  std::is_base_of<detail::CustomTextureExpectation, E>::value>::type>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              wgpu::Device targetDevice,
                                              E* expectation,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(file, line, std::move(targetDevice), expectation, texture,
                                         origin, extent, level, aspect, expectation->DataSize(),
                                         bytesPerRow);
    }

    template <typename T>
    std::ostringstream& AddTextureBetweenColorsExpectation(
        const char* file,
        int line,
        const T& color0,
        const T& color1,
        const wgpu::Texture& texture,
        uint32_t x,
        uint32_t y,
        uint32_t level = 0,
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
        uint32_t bytesPerRow = 0) {
        // No device passed explicitly. Default it, and forward the rest of the args.
        return AddTextureBetweenColorsExpectation(file, line, this->device, color0, color1, texture,
                                                  x, y, level, aspect, bytesPerRow);
    }

    template <typename T>
    std::ostringstream& AddTextureBetweenColorsExpectation(
        const char* file,
        int line,
        const wgpu::Device& targetDevice,
        const T& color0,
        const T& color1,
        const wgpu::Texture& texture,
        uint32_t x,
        uint32_t y,
        uint32_t level = 0,
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
        uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(
            file, line, std::move(targetDevice), new detail::ExpectBetweenColors<T>(color0, color1),
            texture, {x, y}, {1, 1}, level, aspect, sizeof(T), bytesPerRow);
    }

    template <typename T>
    std::ostringstream& AddSnormTextureBoundsExpectation(
        const char* file,
        int line,
        const std::vector<T>& expectedL,
        const std::vector<T>& expectedU,
        const wgpu::Texture& texture,
        wgpu::Origin3D origin,
        wgpu::Extent3D extent,
        wgpu::TextureFormat format,
        uint32_t level = 0,
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
        uint32_t bytesPerRow = 0) {
        // No device passed explicitly. Default it, and forward the rest of the args.
        return AddSnormTextureBoundsExpectation(file, line, this->device, expectedL, expectedU,
                                                texture, origin, extent, format, level, aspect,
                                                bytesPerRow);
    }

    template <typename T>
    std::ostringstream& AddSnormTextureBoundsExpectation(
        const char* file,
        int line,
        const wgpu::Device& targetDevice,
        const std::vector<T>& expectedL,
        const std::vector<T>& expectedU,
        const wgpu::Texture& texture,
        wgpu::Origin3D origin,
        wgpu::Extent3D extent,
        wgpu::TextureFormat format,
        uint32_t level = 0,
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
        uint32_t bytesPerRow = 0) {
        uint32_t texelBlockSize = utils::GetTexelBlockSizeInBytes(format);
        return AddTextureExpectationImpl(
            file, line, std::move(targetDevice),
            new detail::ExpectBetweenSnormTextureBounds<T>(expectedL, expectedU), texture, origin,
            extent, level, aspect, texelBlockSize, bytesPerRow);
    }

    std::ostringstream& ExpectSampledFloatData(wgpu::Texture texture,
                                               uint32_t width,
                                               uint32_t height,
                                               uint32_t componentCount,
                                               uint32_t arrayLayer,
                                               uint32_t mipLevel,
                                               detail::Expectation* expectation);

    std::ostringstream& ExpectMultisampledFloatData(wgpu::Texture texture,
                                                    uint32_t width,
                                                    uint32_t height,
                                                    uint32_t componentCount,
                                                    uint32_t sampleCount,
                                                    uint32_t arrayLayer,
                                                    uint32_t mipLevel,
                                                    detail::Expectation* expectation);

    std::ostringstream& ExpectSampledDepthData(wgpu::Texture depthTexture,
                                               uint32_t width,
                                               uint32_t height,
                                               uint32_t arrayLayer,
                                               uint32_t mipLevel,
                                               detail::Expectation* expectation);

    // Check depth by uploading expected data to a sampled texture, writing it out as a depth
    // attachment, and then using the "equals" depth test to check the contents are the same.
    // Check stencil by rendering a full screen quad and using the "equals" stencil test with
    // a stencil reference value. Note that checking stencil checks that the entire stencil
    // buffer is equal to the expected stencil value.
    std::ostringstream& ExpectAttachmentDepthStencilTestData(wgpu::Texture texture,
                                                             wgpu::TextureFormat format,
                                                             uint32_t width,
                                                             uint32_t height,
                                                             uint32_t arrayLayer,
                                                             uint32_t mipLevel,
                                                             std::vector<float> expectedDepth,
                                                             uint8_t* expectedStencil);

    std::ostringstream& ExpectAttachmentDepthTestData(wgpu::Texture texture,
                                                      wgpu::TextureFormat format,
                                                      uint32_t width,
                                                      uint32_t height,
                                                      uint32_t arrayLayer,
                                                      uint32_t mipLevel,
                                                      std::vector<float> expectedDepth) {
        return ExpectAttachmentDepthStencilTestData(texture, format, width, height, arrayLayer,
                                                    mipLevel, std::move(expectedDepth), nullptr);
    }

    std::ostringstream& ExpectAttachmentStencilTestData(wgpu::Texture texture,
                                                        wgpu::TextureFormat format,
                                                        uint32_t width,
                                                        uint32_t height,
                                                        uint32_t arrayLayer,
                                                        uint32_t mipLevel,
                                                        uint8_t expectedStencil) {
        return ExpectAttachmentDepthStencilTestData(texture, format, width, height, arrayLayer,
                                                    mipLevel, {}, &expectedStencil);
    }

    void MapAsyncAndWait(const wgpu::Buffer& buffer,
                         wgpu::MapMode mapMode,
                         uint64_t offset,
                         uint64_t size);

    void WaitABit(wgpu::Instance = nullptr);
    void FlushWire();
    void WaitForAllOperations();

    bool SupportsFeatures(const std::vector<wgpu::FeatureName>& features);

    // Exposed device creation helper for tests to use when needing more than 1 device.
    wgpu::Device CreateDevice(std::string isolationKey = "");

    // Called in SetUp() to get the features required to be enabled in the tests. The tests must
    // check if the required features are supported by the adapter in this function and guarantee
    // the returned features are all supported by the adapter. The tests may provide different
    // code path to handle the situation when not all features are supported.
    virtual std::vector<wgpu::FeatureName> GetRequiredFeatures();

    // Called in SetUp() to get the limits required to be enabled in the tests.
    // Note implementations of this can assume `required` starts as default-initialized.
    virtual void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                                   dawn::utils::ComboLimits& required);

    // Called in SetUp() to check if 'SetUseTieredLimits' should be set to true on the backend
    // adapter.
    virtual bool GetRequireUseTieredLimits();

    const TestAdapterProperties& GetAdapterProperties() const;

    const dawn::utils::ComboLimits& GetAdapterLimits();
    const dawn::utils::ComboLimits& GetSupportedLimits();

    uint64_t GetDeprecationWarningCountForTesting() const;

    // Helps compute expected deprecated warning count for creating device with given descriptor.
    uint32_t GetDeviceCreationDeprecationWarningExpectation(
        const wgpu::DeviceDescriptor& descriptor);

    void* GetUniqueUserdata();

  private:
    AdapterTestParam mParam;
    std::unique_ptr<utils::WireHelper> mWireHelper;

    // Helps generate unique userdata values passed to deviceLostUserdata.
    std::atomic<uintptr_t> mNextUniqueUserdata = 0;

    // Isolation keys are not exposed to the wire client. Device creation in the tests from
    // the client first push the key into this queue, which is then consumed by the server.
    std::queue<std::string> mNextIsolationKeyQueue;

    // Internal device creation function for default device creation with some optional overrides.
    WGPUDevice CreateDeviceImpl(std::string isolationKey, const WGPUDeviceDescriptor* descriptor);

    std::ostringstream& AddTextureExpectationImpl(const char* file,
                                                  int line,
                                                  wgpu::Device targetDevice,
                                                  detail::Expectation* expectation,
                                                  const wgpu::Texture& texture,
                                                  wgpu::Origin3D origin,
                                                  wgpu::Extent3D extent,
                                                  uint32_t level,
                                                  wgpu::TextureAspect aspect,
                                                  uint32_t dataSize,
                                                  uint32_t bytesPerRow);

    std::ostringstream& ExpectSampledFloatDataImpl(wgpu::Texture texture,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   uint32_t componentCount,
                                                   uint32_t arrayLayer,
                                                   uint32_t mipLevel,
                                                   uint32_t sampleCount,
                                                   wgpu::TextureAspect aspect,
                                                   detail::Expectation* expectation);

    // MapRead buffers used to get data for the expectations
    struct ReadbackSlot {
        wgpu::Device device;
        wgpu::Buffer buffer;
        uint64_t bufferSize;
        raw_ptr<const void> mappedData = nullptr;
    };
    std::vector<ReadbackSlot> mReadbackSlots;

    // Maps all the buffers and fill ReadbackSlot::mappedData
    void MapSlotsSynchronously();
    std::atomic<size_t> mNumPendingMapOperations = 0;

    // Reserve space where the data for an expectation can be copied
    struct ReadbackReservation {
        wgpu::Device device;
        wgpu::Buffer buffer;
        size_t slot;
        uint64_t offset;
    };
    ReadbackReservation ReserveReadback(wgpu::Device targetDevice, uint64_t readbackSize);

    struct DeferredExpectation {
        const char* file;
        int line;
        size_t readbackSlot;
        uint64_t readbackOffset;
        uint64_t size;
        uint32_t rowBytes = 0;
        uint32_t bytesPerRow = 0;
        std::unique_ptr<detail::Expectation> expectation;
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
        // Use unique_ptr because of missing move/copy constructors on std::basic_ostringstream
        std::unique_ptr<std::ostringstream> message;
    };
    std::vector<DeferredExpectation> mDeferredExpectations;

    // Assuming the data is mapped, checks all expectations
    void ResolveExpectations();

    bool mRequireUseTieredLimits = false;
    native::Adapter mBackendAdapter;
    WGPUDevice mLastCreatedBackendDevice;

    std::unique_ptr<platform::Platform> mTestPlatform;

    Mutex mMutex;
};

#define DAWN_SKIP_TEST_IF_BASE(condition, type, reason) \
    do {                                                \
        if (condition) {                                \
            InfoLog() << "Test " type ": " #reason;     \
            GTEST_SKIP();                               \
            return;                                     \
        }                                               \
    } while (0)

// Skip a test which requires a feature or a toggle to be present / not present or some WIP
// features.
#define DAWN_TEST_UNSUPPORTED_IF(condition) \
    DAWN_SKIP_TEST_IF_BASE(condition, "unsupported", condition)

// Skip a test when the test failing on a specific HW / backend / OS combination. We can disable
// this macro with the command line parameter "--run-suppressed-tests".
#define DAWN_SUPPRESS_TEST_IF(condition) \
    DAWN_SKIP_TEST_IF_BASE(!RunSuppressedTests() && condition, "suppressed", condition)

#define EXPECT_DEPRECATION_WARNINGS(statement, n)                             \
    do {                                                                      \
        if (UsesWire()) {                                                     \
            statement;                                                        \
        } else {                                                              \
            uint64_t warningsBefore = GetDeprecationWarningCountForTesting(); \
            statement;                                                        \
            uint64_t warningsAfter = GetDeprecationWarningCountForTesting();  \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                     \
            if (!HasToggleEnabled("skip_validation")) {                       \
                EXPECT_EQ(warningsAfter, warningsBefore + n);                 \
            }                                                                 \
            mLastWarningCount = warningsAfter;                                \
        }                                                                     \
    } while (0)
#define EXPECT_DEPRECATION_WARNING(statement) EXPECT_DEPRECATION_WARNINGS(statement, 1)

template <typename Params = AdapterTestParam>
class DawnTestWithParams : public DawnTestBase, public ::testing::TestWithParam<Params> {
  protected:
    DawnTestWithParams();
    ~DawnTestWithParams() override = default;

    void SetUp() override { DawnTestBase::SetUp(); }

    void TearDown() override { DawnTestBase::TearDown(); }
};

template <typename Params>
DawnTestWithParams<Params>::DawnTestWithParams() : DawnTestBase(this->GetParam()) {}

using DawnTest = DawnTestWithParams<>;

// Instantiate the test once for each backend provided after the first argument. Use it like this:
//     DAWN_INSTANTIATE_TEST(MyTestFixture, MetalBackend, OpenGLBackend)
#define DAWN_INSTANTIATE_TEST(testName, ...)                                            \
    const decltype(DAWN_PP_GET_HEAD(__VA_ARGS__)) testName##params[] = {__VA_ARGS__};   \
    INSTANTIATE_TEST_SUITE_P(                                                           \
        , testName,                                                                     \
        testing::ValuesIn(::dawn::detail::GetAvailableAdapterTestParamsForBackends(     \
            testName##params, sizeof(testName##params) / sizeof(testName##params[0]))), \
        DawnTestBase::PrintToStringParamName(#testName));                               \
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(testName)

#define DAWN_INSTANTIATE_PREFIXED_TEST_P(prefix, testName, ...)                    \
    INSTANTIATE_TEST_SUITE_P(                                                      \
        prefix, testName,                                                          \
        ::testing::ValuesIn(MakeParamGenerator<testName::ParamType>(__VA_ARGS__)), \
        DawnTestBase::PrintToStringParamName(#testName))

// Instantiate the test once for each backend provided in the first param list.
// The test will be parameterized over the following param lists.
// Use it like this:
//     DAWN_INSTANTIATE_TEST_P(MyTestFixture, {MetalBackend(), OpenGLBackend()}, {A, B}, {1, 2})
// MyTestFixture must extend DawnTestWithParams<Param> where Param is a struct that extends
// AdapterTestParam, and whose constructor looks like:
//     Param(AdapterTestParam, ABorC, 12or3, ..., otherParams... )
//     You must also teach GTest how to print this struct.
//     https://github.com/google/googletest/blob/main/docs/advanced.md#teaching-googletest-how-to-print-your-values
// Macro DAWN_TEST_PARAM_STRUCT can help generate this struct.
#define DAWN_INSTANTIATE_TEST_P(testName, ...)                                                 \
    INSTANTIATE_TEST_SUITE_P(                                                                  \
        , testName, ::testing::ValuesIn(MakeParamGenerator<testName::ParamType>(__VA_ARGS__)), \
        DawnTestBase::PrintToStringParamName(#testName));                                      \
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(testName)

// Basically same as DAWN_INSTANTIATE_TEST_P, except that each backend is provided in the
// 'backends' param list.
#define DAWN_INSTANTIATE_TEST_B(testName, backends, ...)                                     \
    INSTANTIATE_TEST_SUITE_P(                                                                \
        , testName,                                                                          \
        ::testing::ValuesIn(MakeParamGenerator<testName::ParamType>(backends, __VA_ARGS__)), \
        DawnTestBase::PrintToStringParamName(#testName));                                    \
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(testName)

// Usage: DAWN_TEST_PARAM_STRUCT(Foo, TypeA, TypeB, ...)
// Generate a test param struct called Foo which extends AdapterTestParam and generated
// struct _Dawn_Foo. _Dawn_Foo has members of types TypeA, TypeB, etc. which are named mTypeA,
// mTypeB, etc. in the order they are placed in the macro argument list. Struct Foo should be
// constructed with an AdapterTestParam as the first argument, followed by a list of values
// to initialize the base _Dawn_Foo struct.
// It is recommended to use alias declarations so that stringified types are more readable.
// Example:
//   using MyParam = unsigned int;
//   DAWN_TEST_PARAM_STRUCT(FooParams, MyParam);
#define DAWN_TEST_PARAM_STRUCT(StructName, ...) \
    DAWN_TEST_PARAM_STRUCT_BASE(AdapterTestParam, StructName, __VA_ARGS__)

namespace detail {
// Helper functions used for DAWN_INSTANTIATE_TEST
std::vector<AdapterTestParam> GetAvailableAdapterTestParamsForBackends(
    const BackendTestConfig* params,
    size_t numParams);

// All classes used to implement the deferred expectations should inherit from this.
class Expectation {
  public:
    virtual ~Expectation() = default;

    // Will be called with the buffer or texture data the expectation should check.
    virtual testing::AssertionResult Check(const void* data, size_t size) = 0;
};

template <typename T>
class ExpectConstant : public Expectation {
  public:
    explicit ExpectConstant(T constant);
    uint32_t DataSize();
    testing::AssertionResult Check(const void* data, size_t size) override;

  private:
    T mConstant;
};

extern template class ExpectConstant<float>;

// Expectation that checks the data is equal to some expected values.
// T - expected value Type
// U - actual value Type (defaults = T)
// This is expanded for float16 mostly where T=float, U=uint16_t
template <typename T, typename U>
class ExpectEq : public Expectation {
  public:
    explicit ExpectEq(T singleValue, T tolerance = {});
    ExpectEq(const T* values, const unsigned int count, T tolerance = {});

    testing::AssertionResult Check(const void* data, size_t size) override;

  private:
    std::vector<T> mExpected;
    T mTolerance;
};
extern template class ExpectEq<uint8_t>;
extern template class ExpectEq<int16_t>;
extern template class ExpectEq<uint32_t>;
extern template class ExpectEq<uint64_t>;
extern template class ExpectEq<int32_t>;
extern template class ExpectEq<utils::RGBA8>;
extern template class ExpectEq<float>;
extern template class ExpectEq<float, uint16_t>;

template <typename T>
class ExpectBetweenColors : public Expectation {
  public:
    // Inclusive for now
    ExpectBetweenColors(T value0, T value1);
    testing::AssertionResult Check(const void* data, size_t size) override;

  private:
    std::vector<T> mLowerColorChannels;
    std::vector<T> mHigherColorChannels;

    // used for printing error
    std::vector<T> mValues0;
    std::vector<T> mValues1;
};
// A color is considered between color0 and color1 when all channel values are within range of
// each counterparts. It doesn't matter which value is higher or lower. Essentially color =
// lerp(color0, color1, t) where t is [0,1]. But I don't want to be too strict here.
extern template class ExpectBetweenColors<utils::RGBA8>;

template <typename T>
class ExpectBetweenSnormTextureBounds : public Expectation {
  public:
    // Inclusive for now
    ExpectBetweenSnormTextureBounds(const std::vector<T>& expectedL,
                                    const std::vector<T>& expectedU)
        : expectedLower(expectedL), expectedUpper(expectedU) {}
    testing::AssertionResult Check(const void* data, size_t size) override;

  private:
    std::vector<T> expectedLower;
    std::vector<T> expectedUpper;
};
template <typename T>
ExpectBetweenSnormTextureBounds(const std::vector<T>&, const std::vector<T>&)
    -> ExpectBetweenSnormTextureBounds<T>;
extern template class ExpectBetweenSnormTextureBounds<int8_t>;
extern template class ExpectBetweenSnormTextureBounds<int16_t>;
extern template class ExpectBetweenSnormTextureBounds<uint16_t>;

class CustomTextureExpectation : public Expectation {
  public:
    ~CustomTextureExpectation() override = default;
    virtual uint32_t DataSize() = 0;
};

}  // namespace detail

template <typename Param, typename... Params>
auto MakeParamGenerator(std::vector<BackendTestConfig>&& first,
                        std::initializer_list<Params>&&... params) {
    return ParamGenerator<Param, AdapterTestParam, Params...>(
        ::dawn::detail::GetAvailableAdapterTestParamsForBackends(first.data(), first.size()),
        std::forward<std::initializer_list<Params>&&>(params)...);
}
template <typename Param, typename... Params>
auto MakeParamGenerator(std::vector<BackendTestConfig>&& first, std::vector<Params>&&... params) {
    return ParamGenerator<Param, AdapterTestParam, Params...>(
        ::dawn::detail::GetAvailableAdapterTestParamsForBackends(first.data(), first.size()),
        std::forward<std::vector<Params>&&>(params)...);
}

}  // namespace dawn
#endif  // SRC_DAWN_TESTS_DAWNTEST_H_
