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

#ifndef SRC_DAWN_NATIVE_WEBGPU_RECORDABLE_OBJECT_H_
#define SRC_DAWN_NATIVE_WEBGPU_RECORDABLE_OBJECT_H_

#include "dawn/native/Error.h"
#include "dawn/native/webgpu/Serialization.h"

namespace dawn::native::webgpu {

class CaptureContext;
class RecordableObject {
  public:
    explicit RecordableObject(schema::ObjectType objectType);
    schema::ObjectType GetObjectType() const;

    virtual MaybeError AddReferenced(CaptureContext& captureContext) = 0;
    virtual MaybeError CaptureCreationParameters(CaptureContext& context) = 0;

    // This is called anytime a resource is used in the queue.
    // If newResource is true this is the first time a resource has been seen
    // during capture. The object should capture the contents unless it happens
    // to know it's all zeros. Otherwise, it's up to that object to decide.
    // For example a buffer will capture content if it's been mapped and written to
    // since the last time this function was called. The default implementation does nothing.
    virtual MaybeError CaptureContentIfNeeded(CaptureContext& context,
                                              schema::ObjectId id,
                                              bool newResource);

  private:
    schema::ObjectType mObjectType;
};

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_RECORDABLE_OBJECT_H_
