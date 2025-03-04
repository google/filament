// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/wire/server/Server.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire::server {

CallbackUserdata::CallbackUserdata(const std::weak_ptr<Server>& server) : server(server) {}

// static
std::shared_ptr<Server> Server::Create(const DawnProcTable& procs,
                                       CommandSerializer* serializer,
                                       MemoryTransferService* memoryTransferService) {
    auto server = std::shared_ptr<Server>(new Server(procs, serializer, memoryTransferService));
    server->mSelf = server;
    return server;
}

Server::Server(const DawnProcTable& procs,
               CommandSerializer* serializer,
               MemoryTransferService* memoryTransferService)
    : mSerializer(serializer), mProcs(procs), mMemoryTransferService(memoryTransferService) {
    if (mMemoryTransferService == nullptr) {
        // If a MemoryTransferService is not provided, fallback to inline memory.
        mOwnedMemoryTransferService = CreateInlineMemoryTransferService();
        mMemoryTransferService = mOwnedMemoryTransferService.get();
    }
}

Server::~Server() {
    // Un-set the error and lost callbacks since we cannot forward them
    // after the server has been destroyed.
    for (WGPUDevice device : Objects<WGPUDevice>().GetAllHandles()) {
        ClearDeviceCallbacks(device);
    }
    DestroyAllObjects(mProcs);
}

WireResult Server::InjectBuffer(WGPUBuffer buffer,
                                const Handle& handle,
                                const Handle& deviceHandle) {
    DAWN_ASSERT(buffer != nullptr);
    Known<WGPUDevice> device;
    WIRE_TRY(Objects<WGPUDevice>().Get(deviceHandle.id, &device));
    if (device->generation != deviceHandle.generation) {
        return WireResult::FatalError;
    }

    Reserved<WGPUBuffer> data;
    WIRE_TRY(Objects<WGPUBuffer>().Allocate(&data, handle));

    data->handle = buffer;
    data->generation = handle.generation;
    data->state = AllocationState::Allocated;

    // The Buffer is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.bufferAddRef(buffer);

    return WireResult::Success;
}

WireResult Server::InjectTexture(WGPUTexture texture,
                                 const Handle& handle,
                                 const Handle& deviceHandle) {
    DAWN_ASSERT(texture != nullptr);
    Known<WGPUDevice> device;
    WIRE_TRY(Objects<WGPUDevice>().Get(deviceHandle.id, &device));
    if (device->generation != deviceHandle.generation) {
        return WireResult::FatalError;
    }

    Reserved<WGPUTexture> data;
    WIRE_TRY(Objects<WGPUTexture>().Allocate(&data, handle));

    data->handle = texture;
    data->generation = handle.generation;
    data->state = AllocationState::Allocated;

    // The texture is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.textureAddRef(texture);

    return WireResult::Success;
}

WireResult Server::InjectSurface(WGPUSurface surface,
                                 const Handle& handle,
                                 const Handle& instanceHandle) {
    DAWN_ASSERT(surface != nullptr);
    Known<WGPUInstance> instance;
    WIRE_TRY(Objects<WGPUInstance>().Get(instanceHandle.id, &instance));
    if (instance->generation != instanceHandle.generation) {
        return WireResult::FatalError;
    }

    Reserved<WGPUSurface> data;
    WIRE_TRY(Objects<WGPUSurface>().Allocate(&data, handle));

    data->handle = surface;
    data->generation = handle.generation;
    data->state = AllocationState::Allocated;

    // The surface is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.surfaceAddRef(surface);

    return WireResult::Success;
}

WireResult Server::InjectInstance(WGPUInstance instance, const Handle& handle) {
    DAWN_ASSERT(instance != nullptr);
    Reserved<WGPUInstance> data;
    WIRE_TRY(Objects<WGPUInstance>().Allocate(&data, handle));

    data->handle = instance;
    data->generation = handle.generation;
    data->state = AllocationState::Allocated;

    // The instance is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.instanceAddRef(instance);

    return WireResult::Success;
}

WGPUDevice Server::GetDevice(uint32_t id, uint32_t generation) {
    Known<WGPUDevice> device;
    if (Objects<WGPUDevice>().Get(id, &device) != WireResult::Success ||
        device->generation != generation) {
        return nullptr;
    }
    return device->handle;
}

bool Server::IsDeviceKnown(WGPUDevice device) const {
    return Objects<WGPUDevice>().IsKnown(device);
}

namespace {
static constexpr WGPULoggingCallbackInfo kEmptyLoggingCallbackInfo = {nullptr, nullptr, nullptr,
                                                                      nullptr};
}  // namespace

void Server::SetForwardingDeviceCallbacks(Known<WGPUDevice> device) {
    // Note: these callbacks are manually inlined here since they do not acquire and
    // free their userdata. Also unlike other callbacks, these are cleared and unset when
    // the server is destroyed, so we don't need to check if the server is still alive
    // inside them.
    // Also, the device is special-cased in Server::DoUnregisterObject to call
    // ClearDeviceCallbacks. This ensures that callbacks will not fire after |deviceObject|
    // is freed.

    // Set callback to post warning and other information to client.
    mProcs.deviceSetLoggingCallback(
        device->handle, {nullptr,
                         [](WGPULoggingType type, WGPUStringView message, void* userdata, void*) {
                             DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
                             info->server->OnLogging(info->self, type, message);
                         },
                         device->info.get(), nullptr});
}

void Server::ClearDeviceCallbacks(WGPUDevice device) {
    // Un-set the logging callback since we cannot forward them after the server has been destroyed.
    mProcs.deviceSetLoggingCallback(device, kEmptyLoggingCallbackInfo);
}

}  // namespace dawn::wire::server
