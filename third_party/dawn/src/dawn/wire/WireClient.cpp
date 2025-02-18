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

#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire {

WireClient::WireClient(const WireClientDescriptor& descriptor)
    : mImpl(new client::Client(descriptor.serializer, descriptor.memoryTransferService)) {}

WireClient::~WireClient() {
    mImpl.reset();
}

const volatile char* WireClient::HandleCommands(const volatile char* commands, size_t size) {
    return mImpl->HandleCommands(commands, size);
}

ReservedBuffer WireClient::ReserveBuffer(WGPUDevice device,
                                         const WGPUBufferDescriptor* descriptor) {
    return mImpl->ReserveBuffer(device, descriptor);
}

ReservedTexture WireClient::ReserveTexture(WGPUDevice device,
                                           const WGPUTextureDescriptor* descriptor) {
    return mImpl->ReserveTexture(device, descriptor);
}

ReservedSurface WireClient::ReserveSurface(WGPUInstance instance,
                                           const WGPUSurfaceCapabilities* capabilities) {
    return mImpl->ReserveSurface(instance, capabilities);
}

ReservedInstance WireClient::ReserveInstance(const WGPUInstanceDescriptor* descriptor) {
    return mImpl->ReserveInstance(descriptor);
}

void WireClient::ReclaimBufferReservation(const ReservedBuffer& reservation) {
    mImpl->ReclaimBufferReservation(reservation);
}

void WireClient::ReclaimTextureReservation(const ReservedTexture& reservation) {
    mImpl->ReclaimTextureReservation(reservation);
}

void WireClient::ReclaimSurfaceReservation(const ReservedSurface& reservation) {
    mImpl->ReclaimSurfaceReservation(reservation);
}

void WireClient::ReclaimInstanceReservation(const ReservedInstance& reservation) {
    mImpl->ReclaimInstanceReservation(reservation);
}

void WireClient::Disconnect() {
    mImpl->Disconnect();
}

namespace client {
MemoryTransferService::MemoryTransferService() = default;

MemoryTransferService::~MemoryTransferService() = default;

MemoryTransferService::ReadHandle::ReadHandle() = default;

MemoryTransferService::ReadHandle::~ReadHandle() = default;

MemoryTransferService::WriteHandle::WriteHandle() = default;

MemoryTransferService::WriteHandle::~WriteHandle() = default;
}  // namespace client

}  // namespace dawn::wire
