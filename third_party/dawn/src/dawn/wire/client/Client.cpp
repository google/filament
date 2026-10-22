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

#include "src/dawn/wire/client/Client.h"

#include <vector>

#include "src/dawn/common/Compiler.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/client/Device.h"

namespace dawn::wire::client {

namespace {

class NoopCommandSerializer final : public CommandSerializer {
  public:
    static NoopCommandSerializer* GetInstance() {
        static NoopCommandSerializer gNoopCommandSerializer;
        return &gNoopCommandSerializer;
    }

    ~NoopCommandSerializer() override = default;

    size_t GetMaximumAllocationSize() const final {
        // Return SIZE_MAX so ChunkedCommandSerializer won't unnecessarily try to chunk commands.
        return SIZE_MAX;
    }
    std::optional<std::span<volatile std::byte>> GetCommandSpace(size_t size) final {
        return std::nullopt;
    }
    bool Flush() final { return false; }
};

}  // anonymous namespace

Client::Client(CommandSerializer* serializer, MemoryTransferService* memoryTransferService)
    : ClientBase(), mSerializer(serializer), mMemoryTransferService(memoryTransferService) {
    if (mMemoryTransferService == nullptr) {
        // If a MemoryTransferService is not provided, fall back to inline memory.
        mOwnedMemoryTransferService = CreateInlineMemoryTransferService();
        mMemoryTransferService = mOwnedMemoryTransferService.get();
    }
}

Client::~Client() {
    // Keep a strong reference to all Instance objects during event manager teardown and object
    // unregistration. This prevents Instance (and its EventManager) from being destructed while
    // event callbacks are executing or before UnregisterAllObjects has set mClient = nullptr on all
    // client objects.
    std::vector<Ref<Instance>> instances;
    for (auto object : mObjects[ObjectType::Instance].GetAllObjects()) {
        if (object != nullptr) {
            instances.push_back(static_cast<Instance*>(object));
        }
    }
    for (auto& instance : instances) {
        instance->GetEventManager().Destroy();
    }

    UnregisterAllObjects();
}

void Client::UnregisterAllObjects() {
    for (auto& objectList : mObjects) {
        for (auto object : objectList.GetAllObjects()) {
            if (object != nullptr) {
                object->Unregister();
            }
        }
    }
}

ReservedBuffer Client::ReserveBuffer(Device* device, const BufferDescriptor* descriptor) {
    Ref<Buffer> buffer = Make<Buffer>(device->GetInstance(), device, descriptor);

    ReservedBuffer result;
    result.handle = buffer->GetWireHandle(this);
    result.deviceHandle = device->GetWireHandle(this);
    result.buffer = ToAPI(ReturnToAPI2(std::move(buffer)));
    return result;
}

ReservedTexture Client::ReserveTexture(Device* device, const TextureDescriptor* descriptor) {
    Ref<Texture> texture = Make<Texture>(device, descriptor);

    ReservedTexture result;
    result.handle = texture->GetWireHandle(this);
    result.deviceHandle = device->GetWireHandle(this);
    result.texture = ToAPI(ReturnToAPI2(std::move(texture)));
    return result;
}

ReservedSurface Client::ReserveSurface(Instance* instance,
                                       const SurfaceCapabilities* capabilities) {
    Ref<Surface> surface = Make<Surface>(capabilities);

    ReservedSurface result;
    result.handle = surface->GetWireHandle(this);
    result.instanceHandle = instance->GetWireHandle(this);
    result.surface = ToAPI(ReturnToAPI2(std::move(surface)));
    return result;
}

ReservedInstance Client::ReserveInstance(const InstanceDescriptor* descriptor) {
    Ref<Instance> instance = Make<Instance>();

    if (instance->Initialize(descriptor) != WireResult::Success) {
        return {nullptr, {0, 0}};
    }

    ReservedInstance result;
    result.handle = instance->GetWireHandle(this);
    result.instance = ToAPI(ReturnToAPI2(std::move(instance)));
    return result;
}

void Client::ReclaimBufferReservation(const ReservedBuffer& reservation) {
    ReclaimReservation(FromAPI(reservation.buffer));
}

void Client::ReclaimTextureReservation(const ReservedTexture& reservation) {
    ReclaimReservation(FromAPI(reservation.texture));
}

void Client::ReclaimSurfaceReservation(const ReservedSurface& reservation) {
    ReclaimReservation(FromAPI(reservation.surface));
}

void Client::ReclaimInstanceReservation(const ReservedInstance& reservation) {
    ReclaimReservation(FromAPI(reservation.instance));
}

void Client::Disconnect() {
    mDisconnected = true;
    mSerializer.SetCommandSerializerForDisconnect(NoopCommandSerializer::GetInstance());

    // Destroy all event managers.
    for (auto object : mObjects[ObjectType::Instance].GetAllObjects()) {
        if (object != nullptr) {
            Ref<Instance> instance = static_cast<Instance*>(object);
            instance->GetEventManager().Destroy();
        }
    }

    for (auto object : mObjects[ObjectType::Device].GetAllObjects()) {
        if (object != nullptr) {
            static_cast<Device*>(object)->HandleDeviceLost(
                WGPUDeviceLostReason_Unknown, ToOutputStringView("GPU connection lost"));
        }
    }
}

bool Client::IsDisconnected() const {
    return mDisconnected;
}

void Client::Unregister(ObjectBase* obj, ObjectType type) {
    UnregisterObjectCmd cmd;
    cmd.objectType = type;
    cmd.objectId = obj->GetWireHandle(this).id;
    SerializeCommand(cmd);

    ReclaimReservation(obj, type);
}

void Client::ReclaimReservation(ObjectBase* obj, ObjectType type) {
    mObjects[type].Remove(obj, this);
}

}  // namespace dawn::wire::client
