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

#ifndef SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
#define SRC_DAWN_WIRE_CLIENT_INSTANCE_H_

#include <memory>

#include "absl/container/flat_hash_set.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "src/dawn/wire/client/EventManager.h"
#include "src/dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

void APISupportedWGSLLanguageFeaturesFreeMembers(
    WGPUSupportedWGSLLanguageFeatures supportedFeatures);
void APISupportedInstanceFeaturesFreeMembers(WGPUSupportedInstanceFeatures supportedFeatures);

class Instance final : public ObjectBase {
  public:
    explicit Instance(const ObjectBaseParams& params);

    ObjectType GetObjectType() const override;

    EventManager& GetEventManager() const;

    // Validate and initialize the client side state. Note the *actual* native
    // instance is not created via the wire, but gets injected separately.
    WireResult Initialize(const InstanceDescriptor* descriptor);

    Future APIRequestAdapter(const RequestAdapterOptions* options,
                             const WGPURequestAdapterCallbackInfo& callbackInfo);

    void APIProcessEvents();
    wgpu::WaitStatus APIWaitAny(Span<FutureWaitInfo> infos, uint64_t timeoutNS);

    bool APIHasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName feature) const;
    void APIGetWGSLLanguageFeatures(SupportedWGSLLanguageFeatures* features) const;

    Surface* APICreateSurface(const SurfaceDescriptor* desc) const;

  private:
    void GatherWGSLFeatures(const DawnWireWGSLControl* wgslControl,
                            const DawnWGSLBlocklist* wgslBlocklist);

    absl::flat_hash_set<wgpu::WGSLLanguageFeatureName> mWGSLFeatures;
    std::unique_ptr<EventManager> mEventManager;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
