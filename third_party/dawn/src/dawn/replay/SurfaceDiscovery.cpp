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

#include "src/dawn/replay/SurfaceDiscovery.h"

#include <algorithm>
#include <utility>

namespace dawn::replay {

VisitResult SurfaceDiscoveryVisitor::operator()(
    const schema::RootCommandSurfaceConfigureCmdData& data) {
    auto& info = mSurfaceInfos[data.surfaceId];
    info.width = data.config.width;
    info.height = data.config.height;
    return VisitStatus::Continue;
}

VisitResult SurfaceDiscoveryVisitor::operator()(
    const schema::RootCommandSurfaceUnconfigureCmdData& data) {
    mSurfaceInfos.try_emplace(data.surfaceId);
    return VisitStatus::Continue;
}

VisitResult SurfaceDiscoveryVisitor::operator()(
    const schema::RootCommandSurfacePresentCmdData& data) {
    mSurfaceInfos.try_emplace(data.surfaceId);
    return VisitStatus::Continue;
}

VisitResult SurfaceDiscoveryVisitor::operator()(
    const schema::RootCommandSurfaceGetCurrentTextureCmdData& data) {
    mSurfaceInfos.try_emplace(data.surfaceId);
    return VisitStatus::Continue;
}

VisitResult SurfaceDiscoveryVisitor::operator()(const schema::RootCommandSetLabelCmdData& data) {
    if (data.type == schema::ObjectType::Surface) {
        mSurfaceInfos.try_emplace(data.id);
    }
    return VisitStatus::Continue;
}

VisitResult SurfaceDiscoveryVisitor::operator()(const CreateResourceData& data) {
    if (data.resource.type == schema::ObjectType::Surface) {
        mSurfaceInfos.try_emplace(data.resource.id);
    }
    return std::visit(mResourceVisitor, data.data);
}

VisitResult SurfaceDiscoveryVisitor::operator()(const schema::RootCommandWriteBufferCmdData& data) {
    return VisitStatus::Continue;
}
VisitResult SurfaceDiscoveryVisitor::operator()(
    const schema::RootCommandWriteTextureCmdData& data) {
    return VisitStatus::Continue;
}
VisitResult SurfaceDiscoveryVisitor::operator()(const schema::RootCommandQueueSubmitCmdData& data) {
    return VisitStatus::Continue;
}
VisitResult SurfaceDiscoveryVisitor::operator()(const schema::RootCommandInitTextureCmdData& data) {
    return VisitStatus::Continue;
}
VisitResult SurfaceDiscoveryVisitor::operator()(const schema::RootCommandEndCmdData& data) {
    return VisitStatus::Continue;
}

ResourceVisitor& SurfaceDiscoveryVisitor::GetResourceVisitor() {
    return mResourceVisitor;
}

void SurfaceDiscoveryVisitor::SetContentReadHead(ReadHead* readHead) {}

std::vector<schema::ObjectId> SurfaceDiscoveryVisitor::GetSurfaceIds() const {
    std::vector<schema::ObjectId> ids;
    for (auto const& [id, info] : mSurfaceInfos) {
        ids.push_back(id);
    }
    return ids;
}

std::vector<SurfaceInfo> SurfaceDiscoveryVisitor::GetSurfaceInfos() const {
    std::vector<SurfaceInfo> infos;
    for (auto const& [id, info] : mSurfaceInfos) {
        infos.push_back(info);
    }
    return infos;
}

}  // namespace dawn::replay
