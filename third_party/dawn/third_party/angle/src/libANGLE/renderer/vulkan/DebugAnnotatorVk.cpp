//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotatorVk.cpp: Vulkan helpers for adding trace annotations.
//

#include "libANGLE/renderer/vulkan/DebugAnnotatorVk.h"

#include "common/entry_points_enum_autogen.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

namespace rx
{

DebugAnnotatorVk::DebugAnnotatorVk() {}

DebugAnnotatorVk::~DebugAnnotatorVk() {}

void DebugAnnotatorVk::beginEvent(gl::Context *context,
                                  angle::EntryPoint entryPoint,
                                  const char *eventName,
                                  const char *eventMessage)
{
    angle::LoggingAnnotator::beginEvent(context, entryPoint, eventName, eventMessage);
    if (vkCmdBeginDebugUtilsLabelEXT && context)
    {
        ContextVk *contextVk = vk::GetImpl(static_cast<gl::Context *>(context));
        contextVk->logEvent(eventMessage);
    }
}

void DebugAnnotatorVk::endEvent(gl::Context *context,
                                const char *eventName,
                                angle::EntryPoint entryPoint)
{
    angle::LoggingAnnotator::endEvent(context, eventName, entryPoint);
    if (vkCmdBeginDebugUtilsLabelEXT && context)
    {
        ContextVk *contextVk = vk::GetImpl(static_cast<gl::Context *>(context));
        if (angle::IsDrawEntryPoint(entryPoint))
        {
            contextVk->endEventLog(entryPoint, PipelineType::Graphics);
        }
        else if (angle::IsDispatchEntryPoint(entryPoint))
        {
            contextVk->endEventLog(entryPoint, PipelineType::Compute);
        }
        else if (angle::IsClearEntryPoint(entryPoint) || angle::IsQueryEntryPoint(entryPoint))
        {
            contextVk->endEventLogForClearOrQuery();
        }
    }
}

bool DebugAnnotatorVk::getStatus(const gl::Context *context)
{
    return true;
}
}  // namespace rx
