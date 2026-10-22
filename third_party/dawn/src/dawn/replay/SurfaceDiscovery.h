// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_REPLAY_SURFACEDISCOVERY_H_
#define SRC_DAWN_REPLAY_SURFACEDISCOVERY_H_

#include <map>
#include <vector>

#include "dawn/replay/Replay.h"
#include "src/dawn/replay/CaptureWalker.h"

namespace dawn::replay {

class SurfaceDiscoveryVisitor : public RootCommandVisitor {
  public:
    using RootCommandVisitor::operator();
    VisitResult operator()(const schema::RootCommandSurfaceConfigureCmdData& data) override;
    VisitResult operator()(const schema::RootCommandSurfaceUnconfigureCmdData& data) override;
    VisitResult operator()(const schema::RootCommandSurfacePresentCmdData& data) override;
    VisitResult operator()(const schema::RootCommandSurfaceGetCurrentTextureCmdData& data) override;
    VisitResult operator()(const schema::RootCommandSetLabelCmdData& data) override;

    VisitResult operator()(const CreateResourceData& data) override;
    VisitResult operator()(const schema::RootCommandWriteBufferCmdData& data) override;
    VisitResult operator()(const schema::RootCommandWriteTextureCmdData& data) override;
    VisitResult operator()(const schema::RootCommandQueueSubmitCmdData& data) override;
    VisitResult operator()(const schema::RootCommandInitTextureCmdData& data) override;
    VisitResult operator()(const schema::RootCommandEndCmdData& data) override;

    ResourceVisitor& GetResourceVisitor() override;
    void SetContentReadHead(ReadHead* readHead) override;

    std::vector<schema::ObjectId> GetSurfaceIds() const;
    std::vector<SurfaceInfo> GetSurfaceInfos() const;

  private:
    struct NoopResourceVisitor : ResourceVisitor {
        using ResourceVisitor::operator();

        template <typename T>
        VisitResult SkipResourceData(const T& data) {
            if constexpr (std::is_same_v<T, CommandBufferData>) {
                return SkipEncoderCommands(data.readHead);
            } else if constexpr (std::is_same_v<T, RenderBundleData>) {
                return SkipRenderBundleCommands(data.readHead);
            } else {
                return VisitStatus::Continue;
            }
        }

#define DAWN_REPLAY_RESOURCE_VISITOR_OVERRIDE(ENUM, TYPE) \
    VisitResult operator()(const TYPE& data) override { return SkipResourceData(data); }
        DAWN_REPLAY_RESOURCE_DATA_MAP(DAWN_REPLAY_RESOURCE_VISITOR_OVERRIDE)
#undef DAWN_REPLAY_RESOURCE_VISITOR_OVERRIDE
        VisitResult operator()(const std::monostate&) override { return VisitStatus::Continue; }
    };

    NoopResourceVisitor mResourceVisitor;
    std::map<schema::ObjectId, SurfaceInfo> mSurfaceInfos;
};

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_SURFACEDISCOVERY_H_
