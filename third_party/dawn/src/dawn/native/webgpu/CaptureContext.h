// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_WEBGPU_CAPTURECONTEXT_H_
#define SRC_DAWN_NATIVE_WEBGPU_CAPTURECONTEXT_H_

#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/webgpu/Forward.h"
#include "dawn/native/webgpu/Serialization.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ref.h"

namespace dawn::native {

class BufferBase;
class DeviceBase;
struct Color;
struct Origin2D;
struct Extent2D;
struct BufferCopy;
struct ProgrammableStage;
struct TexelOrigin3D;
struct TexelExtent3D;
struct TextureCopy;
struct TimestampWrites;
struct TypedTexelBlockInfo;

}  // namespace dawn::native

namespace dawn::native::webgpu {

class Device;
class RecordableObject;

class CaptureContext {
  public:
    explicit CaptureContext(Device* device,
                            std::ostream& commandStream,
                            std::ostream& contentStream);
    ~CaptureContext();

    // Content is padded to 4 byte blocks. This class automates writing the
    // padding.
    class ScopedContentWriter {
      public:
        ScopedContentWriter(const ScopedContentWriter&) = delete;
        ScopedContentWriter& operator=(const ScopedContentWriter&) = delete;

        explicit ScopedContentWriter(CaptureContext& context);
        ~ScopedContentWriter();
        void WriteContentBytes(const void* data, size_t size);

      private:
        uint64_t mBytesWritten = 0;
        raw_ref<CaptureContext> mContext;
    };

    static constexpr uint64_t kCopyBufferSize = 1024 * 1024;

    // Add resources both, creates an id for the resource AND captures its
    // description if it has not already been captured which is effectively
    // capturing an implicit call to createXXX.
    template <typename T>
    ResultOrError<schema::ObjectId> AddResourceAndGetId(T* object) {
        assert(object != nullptr);
        schema::ObjectId id;
        Ref<ApiObjectBase> ref(object);
        auto it = mObjectIds.find(ref);
        bool newResource = it == mObjectIds.end();
        if (newResource) {
            DAWN_TRY(object->AddReferenced(*this));

            id = mNextObjectId++;
            mObjectIds[std::move(ref)] = id;
            DAWN_TRY(CaptureCreation(id, object->GetLabel(), object));
        } else {
            id = it->second;
        }
        DAWN_TRY(object->CaptureContentIfNeeded(*this, id, newResource));
        return {id};
    }

    template <typename T>
    MaybeError AddResource(T* object) {
        [[maybe_unused]] schema::ObjectId id;
        DAWN_TRY_ASSIGN(id, AddResourceAndGetId(object));
        return {};
    }

    // You must have called AddResource at some point before calling GetId.
    template <typename T>
    schema::ObjectId GetId(T ref) {
        if (ref == nullptr) {
            return 0;
        }

        auto it = mObjectIds.find(ref);
        DAWN_ASSERT(it != mObjectIds.end());
        return it->second;
    }

    template <typename T>
    schema::ObjectId GetId(T* object) {
        if (object == nullptr) {
            return 0;
        }

        auto it = mObjectIds.find(Ref<ApiObjectBase>(object));
        DAWN_ASSERT(it != mObjectIds.end());
        return it->second;
    }

    template <typename T>
    bool HasId(T* object) {
        return mObjectIds.find(Ref<ApiObjectBase>(object)) != mObjectIds.end();
    }

    template <typename T>
    void CaptureSetLabel(T* object, const std::string& label) {
        auto result = AddResourceAndGetId(object);
        if (result.IsSuccess()) {
            schema::RootCommandSetLabelCmd data{{
                .data{{
                    .id = result.AcquireSuccess(),
                    .type = object->GetObjectType(),
                    .label = label,
                }},
            }};
            Serialize(*this, data);
        }
    }

    // Special case for Device as it's not a RecordableObject
    // so we don't want to call AddResourceAndGetId on it.
    template <>
    void CaptureSetLabel(Device* object, const std::string& label) {
        schema::RootCommandSetLabelCmd data{{
            .data{{
                .id = schema::kDeviceId,
                .type = schema::ObjectType::Device,
                .label = label,
            }},
        }};
        Serialize(*this, data);
    }

    void WriteCommandBytes(const void* data, size_t size);

    MaybeError CaptureQueueWriteBuffer(Buffer* buffer,
                                       uint64_t bufferOffset,
                                       const void* data,
                                       size_t size);
    MaybeError CaptureQueueWriteTexture(const TexelCopyTextureInfo& destination,
                                        const void* data,
                                        size_t dataSize,
                                        const TexelCopyBufferLayout& dataLayout,
                                        const TexelExtent3D& writeSizePixel);

    WGPUBuffer GetCopyBuffer();
    WGPUBuffer GetBlitTextureToBufferBuffer();

  protected:
    void WriteContentBytes(const void* data, size_t size);

  private:
    MaybeError CaptureCreation(schema::ObjectId id,
                               const std::string& label,
                               RecordableObject* object);
    MaybeError CaptureContentIfNeeded(schema::ObjectId id,
                                      bool newResource,
                                      RecordableObject* object);

    // This is here for debugging. So that at debug time you can see what how many command bytes
    // have been written. and compare that to how many have been read when replaying.
    uint64_t mCommandBytesWritten = 0;

    // TODO(crbug.com/485825675): Investigate why one of the 3 raw_ptr/raw_ref
    // is/are dangling and if we can make them all non-dangling.
    raw_ptr<Device, DanglingUntriaged> mDevice;
    const raw_ref<std::ostream, DanglingUntriaged> mCommandStream;
    const raw_ref<std::ostream, DanglingUntriaged> mContentStream;
    absl::flat_hash_map<Ref<ApiObjectBase>, schema::ObjectId> mObjectIds;
    schema::ObjectId mNextObjectId = 2;  // 1 = the device itself.

    WGPUBuffer mCopyBuffer = nullptr;
};

wgpu::TextureAspect ToDawn(const Aspect aspect);
schema::Origin3D ToSchema(const TexelOrigin3D& origin);
schema::Origin2D ToSchema(const Origin2D& origin);
schema::Extent3D ToSchema(const TexelExtent3D& extent);
schema::Extent2D ToSchema(const Extent2D& extent);
schema::Color ToSchema(const Color& color);
schema::ProgrammableStage ToSchema(CaptureContext& captureContext, const ProgrammableStage& stage);
schema::TexelCopyBufferLayout ToSchema(const BufferCopy& bufferCopy,
                                       const TypedTexelBlockInfo& blockInfo);
schema::TexelCopyBufferInfo ToSchema(CaptureContext& captureContext,
                                     const BufferCopy& bufferCopy,
                                     const TypedTexelBlockInfo& blockInfo);
schema::TexelCopyTextureInfo ToSchema(CaptureContext& captureContext,
                                      const TextureCopy& textureCopy);
schema::TexelCopyBufferLayout ToSchema(const TexelCopyBufferLayout& layout);
schema::TexelCopyTextureInfo ToSchema(CaptureContext& captureContext,
                                      const TexelCopyTextureInfo& info);
schema::TimestampWrites ToSchema(CaptureContext& captureContext,
                                 const TimestampWrites& timestampWrites);

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_CAPTURECONTEXT_H_
