//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramImpl.cpp: Implements the class methods for ProgramImpl.

#include "libANGLE/renderer/ProgramImpl.h"

namespace rx
{
void LinkTask::link(const gl::ProgramLinkedResources &resources,
                    const gl::ProgramMergedVaryings &mergedVaryings,
                    std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
                    std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut)
{
    UNREACHABLE();
    return;
}
void LinkTask::load(std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
                    std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut)
{
    UNREACHABLE();
    return;
}
bool LinkTask::isLinkingInternally()
{
    return false;
}

angle::Result ProgramImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
