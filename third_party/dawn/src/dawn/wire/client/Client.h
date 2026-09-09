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

#ifndef SRC_DAWN_WIRE_CLIENT_CLIENT_H_
#define SRC_DAWN_WIRE_CLIENT_CLIENT_H_

#include <webgpu/webgpu.h>

#include <memory>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ClientBase_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/FutureUtils.h"
#include "src/dawn/common/LinkedList.h"
#include "src/dawn/wire/ChunkedCommandSerializer.h"
#include "src/dawn/wire/WireDeserializeAllocator.h"
#include "src/dawn/wire/WireResult.h"
#include "src/dawn/wire/client/EventManager.h"
#include "src/dawn/wire/client/ObjectStore.h"
#include "src/utils/non_copyable.h"

namespace dawn::wire::client {

class Device;
class MemoryTransferService;
class EventManager;

class Client : public ClientBase {
  public:
    Client(CommandSerializer* serializer, MemoryTransferService* memoryTransferService);
    ~Client() override;

    // Make<T>(arg1, arg2, arg3) creates a new T, calling a constructor of the form:
    //
    //   T::T(ObjectBaseParams, arg1, arg2, arg3)
    template <typename T, typename... Args>
    Ref<T> Make(Args&&... args) {
        constexpr ObjectType type = ObjectTypeToTypeEnum<T>;

        ObjectBaseParams params = {this, mObjects[type].ReserveHandle()};
        Ref<T> object = AcquireRef(new T(params, std::forward<Args>(args)...));

        mObjects[type].Insert(object.Get(), this);

        return object;
    }

    void Unregister(ObjectBase* obj, ObjectType type);

    template <typename T>
    T* Get(ObjectId id) {
        return static_cast<T*>(mObjects[ObjectTypeToTypeEnum<T>].Get(id));
    }

    // ChunkedCommandHandler implementation
    bool HandleCommands(Span<const volatile std::byte> commands) override;

    MemoryTransferService* GetMemoryTransferService() const { return mMemoryTransferService; }

    ReservedBuffer ReserveBuffer(Device* device, const BufferDescriptor* descriptor);
    ReservedTexture ReserveTexture(Device* device, const TextureDescriptor* descriptor);
    ReservedSurface ReserveSurface(Instance* instance, const SurfaceCapabilities* capabilities);
    ReservedInstance ReserveInstance(const InstanceDescriptor* descriptor);

    void ReclaimBufferReservation(const ReservedBuffer& reservation);
    void ReclaimTextureReservation(const ReservedTexture& reservation);
    void ReclaimSurfaceReservation(const ReservedSurface& reservation);
    void ReclaimInstanceReservation(const ReservedInstance& reservation);

    template <typename Cmd>
    void SerializeCommand(const Cmd& cmd) {
        mSerializer.SerializeCommand(cmd, *this);
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(const Cmd& cmd, Extensions&&... es) {
        mSerializer.SerializeCommand(cmd, *this, std::forward<Extensions>(es)...);
    }

    void Disconnect();
    bool IsDisconnected() const;

  private:
    void UnregisterAllObjects();
    void ReclaimReservation(ObjectBase* obj, ObjectType type);

    template <typename T>
    void ReclaimReservation(T* obj) {
        ReclaimReservation(obj, ObjectTypeToTypeEnum<T>);
    }

    template <typename Event, typename... ReadyArgs>
    WireResult SetFutureReady(ObjectId instanceId, FutureID futureID, ReadyArgs&&... readyArgs) {
        Instance* instance = Get<Instance>(instanceId);
        if (instance == nullptr) {
            return WireResult::FatalError;
        }

        // Validate that the future id is a valid id.
        if (futureID <= kNullFutureID) {
            return WireResult::FatalError;
        }

        return instance->GetEventManager().SetFutureReady<Event>(
            futureID, std::forward<ReadyArgs>(readyArgs)...);
    }

#include "dawn/wire/client/ClientPrototypes_autogen.inc"

    ChunkedCommandSerializer mSerializer;
    PerObjectType<ObjectStore> mObjects;
    std::unique_ptr<MemoryTransferService> mOwnedMemoryTransferService = nullptr;
    raw_ptr<MemoryTransferService> mMemoryTransferService = nullptr;
    bool mDisconnected = false;
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService();

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_CLIENT_H_
