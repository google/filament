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
#ifndef SRC_DAWN_NATIVE_D3D12_COMMANDRECORDINGCONTEXT_H_
#define SRC_DAWN_NATIVE_D3D12_COMMANDRECORDINGCONTEXT_H_

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d/KeyedMutex.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {
class Device;
class Heap;
class Texture;

class CommandRecordingContext {
  public:
    void AddToSharedTextureList(Texture* texture);
    void AddToSharedBufferList(Buffer* buffer);
    void Open(ComPtr<ID3D12GraphicsCommandList> commandList);

    ID3D12GraphicsCommandList* GetCommandList() const;
    ID3D12GraphicsCommandList1* GetCommandList1() const;
    ID3D12GraphicsCommandList4* GetCommandList4() const;
    void Release();
    bool NeedsSubmit() const;
    void SetNeedsSubmit();

    MaybeError ExecuteCommandList(Device* device, ID3D12CommandQueue* commandQueue);

    void TrackHeapUsage(Heap* heap, ExecutionSerial serial);

    void AddToTempBuffers(Ref<Buffer> tempBuffer);

    MaybeError AcquireKeyedMutex(Ref<d3d::KeyedMutex> keyedMutex);

  private:
    void ReleaseKeyedMutexes();

    ComPtr<ID3D12GraphicsCommandList> mD3d12CommandList;
    ComPtr<ID3D12GraphicsCommandList1> mD3d12CommandList1;
    ComPtr<ID3D12GraphicsCommandList4> mD3d12CommandList4;
    bool mNeedsSubmit = false;
    absl::flat_hash_set<Buffer*> mSharedBuffers;
    absl::flat_hash_set<Texture*> mSharedTextures;
    absl::flat_hash_set<Ref<d3d::KeyedMutex>> mAcquiredKeyedMutexes;
    std::vector<Heap*> mHeapsPendingUsage;
    std::vector<Ref<Buffer>> mTempBuffers;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_COMMANDRECORDINGCONTEXT_H_
