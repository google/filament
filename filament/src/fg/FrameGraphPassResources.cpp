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

#include "fg/ResourceNode.h"
#include "fg/PassNode.h"

#include <utils/Panic.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {

using namespace backend;
using namespace fg;

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

} // namespace filament
