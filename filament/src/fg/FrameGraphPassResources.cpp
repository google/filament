/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FrameGraphPassResources.h"

#include "FrameGraph.h"
#include "FrameGraphHandle.h"

#include "fg/RenderTargetResource.h"
#include "fg/ResourceNode.h"
#include "fg/RenderTarget.h"
#include "fg/PassNode.h"

#include <utils/Panic.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {

using namespace backend;
using namespace fg;
using namespace details;

FrameGraphPassResources::FrameGraphPassResources(FrameGraph& fg, fg::PassNode const& pass) noexcept
        : mFrameGraph(fg), mPass(pass) {
}

const char* FrameGraphPassResources::getPassName() const noexcept {
    return mPass.name;
}

fg::ResourceEntryBase const& FrameGraphPassResources::getResourceEntryBase(FrameGraphHandle r) const noexcept {
    ResourceNode& node = mFrameGraph.getResourceNodeUnchecked(r);

    fg::ResourceEntryBase const* const pResource = node.resource;
    assert(pResource);

// TODO: we should check for write to
//    // check that this FrameGraphHandle is indeed used by this pass
//    ASSERT_POSTCONDITION_NON_FATAL(mPass.isReadingFrom(r),
//            "Pass \"%s\" doesn't declare reads to resource \"%s\" -- expect graphic corruptions",
//            mPass.name, pResource->name);

    return *pResource;
}

FrameGraphPassResources::RenderTargetInfo
FrameGraphPassResources::getRenderTarget(FrameGraphRenderTargetHandle handle, uint8_t level) const noexcept {
    FrameGraphPassResources::RenderTargetInfo info{};
    FrameGraph& fg = mFrameGraph;

    fg::RenderTarget& renderTarget = fg.mRenderTargets[handle];
    assert(renderTarget.cache);

    info = renderTarget.cache->targetInfo;
    assert(info.target);

    // overwrite discard flags with the per-rendertarget (per-pass) computed value
    info.params.flags.discardStart = TargetBufferFlags::NONE;
    info.params.flags.discardEnd   = TargetBufferFlags::NONE;
    info.params.flags.clear        = renderTarget.userClearFlags;

    static constexpr TargetBufferFlags flags[] = {
            TargetBufferFlags::COLOR,
            TargetBufferFlags::DEPTH,
            TargetBufferFlags::STENCIL };

    auto& resourceNodes = fg.mResourceNodes;
    for (size_t i = 0; i <renderTarget.desc.attachments.textures.size(); i++) {
        FrameGraphHandle attachment = renderTarget.desc.attachments.textures[i];
        if (attachment.isValid()) {
            if (resourceNodes[attachment.index].resource->discardStart) {
                info.params.flags.discardStart |= flags[i];
            }
            if (resourceNodes[attachment.index].resource->discardEnd) {
                info.params.flags.discardEnd |= flags[i];
            }
        }
    }

    // clear implies discarding the content of the buffer
    info.params.flags.discardStart |= (renderTarget.userClearFlags & TargetBufferFlags::ALL);
    if (renderTarget.cache->imported) {
        // we never discard more than the user flags
        info.params.flags.discardStart &= renderTarget.cache->discardStart;
        info.params.flags.discardEnd   &= renderTarget.cache->discardEnd;
    }

    // check that this FrameGraphRenderTarget is indeed declared by this pass
    ASSERT_POSTCONDITION_NON_FATAL(info.target,
            "Pass \"%s\" doesn't declare rendertarget \"%s\" -- expect graphic corruptions",
            mPass.name, renderTarget.name);

//    slog.d << mPass.name << ": resource = \"" << renderTarget.name << "\", flags = "
//        << io::hex
//        << renderTarget.targetInfo.params.discardStart << ", "
//        << renderTarget.targetInfo.params.discardEnd << io::endl;

    return info;
}

} // namespace filament
