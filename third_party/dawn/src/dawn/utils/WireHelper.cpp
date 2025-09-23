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

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <system_error>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/utils/WireHelper.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::utils {

namespace {

class WireServerTraceLayer : public dawn::wire::CommandHandler {
  public:
    WireServerTraceLayer(const char* dir, dawn::wire::CommandHandler* handler)
        : dawn::wire::CommandHandler(), mDir(dir), mHandler(handler) {
        const char* sep = GetPathSeparator();
        if (mDir.size() > 0 && mDir.back() != *sep) {
            mDir += sep;
        }
    }

    void BeginWireTrace(const char* name) {
        std::string filename = name;
        // Replace slashes in gtest names with underscores so everything is in one
        // directory.
        std::replace(filename.begin(), filename.end(), '/', '_');
        std::replace(filename.begin(), filename.end(), '\\', '_');

        if (!std::filesystem::is_directory(mDir)) {
            std::error_code ec;
            std::filesystem::create_directories(mDir, ec);
            DAWN_ASSERT(ec.value() == 0);
            DAWN_ASSERT(std::filesystem::is_directory(mDir));
        }

        // Prepend the filename with the directory.
        filename = mDir + filename;

        DAWN_ASSERT(!mFile.is_open());
        mFile.open(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

        // Write the initial 8 bytes. This means the fuzzer should never inject an
        // error.
        const uint64_t injectedErrorIndex = 0xFFFF'FFFF'FFFF'FFFF;
        mFile.write(reinterpret_cast<const char*>(&injectedErrorIndex), sizeof(injectedErrorIndex));
    }

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override {
        if (mFile.is_open()) {
            mFile.write(const_cast<const char*>(commands), size);
        }
        return mHandler->HandleCommands(commands, size);
    }

  private:
    std::string mDir;
    raw_ptr<dawn::wire::CommandHandler> mHandler;
    std::ofstream mFile;
};

class WireHelperDirect : public WireHelper {
  public:
    explicit WireHelperDirect(const DawnProcTable& procs) { dawnProcSetProcs(&procs); }

    wgpu::Instance RegisterInstance(WGPUInstance backendInstance,
                                    const WGPUInstanceDescriptor* wireDesc) override {
        DAWN_ASSERT(backendInstance != nullptr);
        return wgpu::Instance(backendInstance);
    }

    void BeginWireTrace(const char* name) override {}

    bool FlushClient() override { return true; }

    bool FlushServer() override { return true; }

    bool IsIdle() override { return true; }
};

class WireHelperProxy : public WireHelper {
  public:
    explicit WireHelperProxy(const char* wireTraceDir, const DawnProcTable& procs) {
        mC2sBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();
        mS2cBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();

        dawn::wire::WireServerDescriptor serverDesc = {};
        serverDesc.procs = &procs;
        serverDesc.serializer = mS2cBuf.get();

        mWireServer.reset(new dawn::wire::WireServer(serverDesc));
        mC2sBuf->SetHandler(mWireServer.get());

        if (wireTraceDir != nullptr && strlen(wireTraceDir) > 0) {
            mWireServerTraceLayer.reset(new WireServerTraceLayer(wireTraceDir, mWireServer.get()));
            mC2sBuf->SetHandler(mWireServerTraceLayer.get());
        }

        dawn::wire::WireClientDescriptor clientDesc = {};
        clientDesc.serializer = mC2sBuf.get();

        mWireClient.reset(new dawn::wire::WireClient(clientDesc));
        mS2cBuf->SetHandler(mWireClient.get());
        dawnProcSetProcs(&dawn::wire::client::GetProcs());
    }

    ~WireHelperProxy() override {
        mC2sBuf->SetHandler(nullptr);
        mS2cBuf->SetHandler(nullptr);
    }

    wgpu::Instance RegisterInstance(WGPUInstance backendInstance,
                                    const WGPUInstanceDescriptor* wireDesc) override {
        DAWN_ASSERT(backendInstance != nullptr);

        auto reserved = mWireClient->ReserveInstance(wireDesc);
        mWireServer->InjectInstance(backendInstance, reserved.handle);

        return wgpu::Instance::Acquire(reserved.instance);
    }

    void BeginWireTrace(const char* name) override {
        if (mWireServerTraceLayer) {
            return mWireServerTraceLayer->BeginWireTrace(name);
        }
    }

    bool FlushClient() override { return mC2sBuf->Flush(); }

    bool FlushServer() override { return mS2cBuf->Flush(); }

    bool IsIdle() override { return mC2sBuf->Empty() && mS2cBuf->Empty(); }

  private:
    std::unique_ptr<dawn::utils::TerribleCommandBuffer> mC2sBuf;
    std::unique_ptr<dawn::utils::TerribleCommandBuffer> mS2cBuf;
    std::unique_ptr<dawn::wire::WireServer> mWireServer;
    std::unique_ptr<dawn::wire::WireClient> mWireClient;
    std::unique_ptr<WireServerTraceLayer> mWireServerTraceLayer;
};

}  // anonymous namespace

std::pair<wgpu::Instance, std::unique_ptr<dawn::native::Instance>> WireHelper::CreateInstances(
    const wgpu::InstanceDescriptor* nativeDesc,
    const wgpu::InstanceDescriptor* wireDesc) {
    auto nativeInstance = std::make_unique<dawn::native::Instance>(
        reinterpret_cast<const WGPUInstanceDescriptor*>(nativeDesc));

    return {RegisterInstance(nativeInstance->Get(),
                             reinterpret_cast<const WGPUInstanceDescriptor*>(wireDesc)),
            std::move(nativeInstance)};
}

std::unique_ptr<WireHelper> CreateWireHelper(const DawnProcTable& procs,
                                             bool useWire,
                                             const char* wireTraceDir) {
    if (useWire) {
        return std::unique_ptr<WireHelper>(new WireHelperProxy(wireTraceDir, procs));
    } else {
        return std::unique_ptr<WireHelper>(new WireHelperDirect(procs));
    }
}

WireHelper::~WireHelper() {
    dawnProcSetProcs(nullptr);
}

}  // namespace dawn::utils
