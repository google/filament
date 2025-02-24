//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotator11.cpp: D3D11 helpers for adding trace annotations.
//

#include "libANGLE/renderer/d3d/d3d11/DebugAnnotator11.h"

#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/driver_utils.h"

#include "common/system_utils.h"

namespace rx
{

// DebugAnnotator11 implementation
DebugAnnotator11::DebugAnnotator11() {}

DebugAnnotator11::~DebugAnnotator11() {}

void DebugAnnotator11::beginEvent(gl::Context *context,
                                  angle::EntryPoint entryPoint,
                                  const char *eventName,
                                  const char *eventMessage)
{
    angle::LoggingAnnotator::beginEvent(context, entryPoint, eventName, eventMessage);
    if (!context)
    {
        return;
    }
    Renderer11 *renderer11 = GetImplAs<Context11>(context)->getRenderer();
    renderer11->getDebugAnnotatorContext()->beginEvent(entryPoint, eventName, eventMessage);
}

void DebugAnnotator11::endEvent(gl::Context *context,
                                const char *eventName,
                                angle::EntryPoint entryPoint)
{
    angle::LoggingAnnotator::endEvent(context, eventName, entryPoint);
    if (!context)
    {
        return;
    }
    Renderer11 *renderer11 = GetImplAs<Context11>(context)->getRenderer();
    renderer11->getDebugAnnotatorContext()->endEvent(eventName, entryPoint);
}

void DebugAnnotator11::setMarker(gl::Context *context, const char *markerName)
{
    angle::LoggingAnnotator::setMarker(context, markerName);
    if (!context)
    {
        return;
    }
    Renderer11 *renderer11 = GetImplAs<Context11>(context)->getRenderer();
    renderer11->getDebugAnnotatorContext()->setMarker(markerName);
}

bool DebugAnnotator11::getStatus(const gl::Context *context)
{
    if (!context)
    {
        return false;
    }
    Renderer11 *renderer11 = GetImplAs<Context11>(context)->getRenderer();
    return renderer11->getDebugAnnotatorContext()->getStatus();
}

// DebugAnnotatorContext11 implemenetation
DebugAnnotatorContext11::DebugAnnotatorContext11() = default;

DebugAnnotatorContext11::~DebugAnnotatorContext11() = default;

void DebugAnnotatorContext11::beginEvent(angle::EntryPoint entryPoint,
                                         const char *eventName,
                                         const char *eventMessage)
{
    if (loggingEnabledForThisThread())
    {
        std::mbstate_t state = std::mbstate_t();
        std::mbsrtowcs(mWCharMessage, &eventMessage, kMaxMessageLength, &state);
        mUserDefinedAnnotation->BeginEvent(mWCharMessage);
    }
}

void DebugAnnotatorContext11::endEvent(const char *eventName, angle::EntryPoint entryPoint)
{
    if (loggingEnabledForThisThread())
    {
        mUserDefinedAnnotation->EndEvent();
    }
}

void DebugAnnotatorContext11::setMarker(const char *markerName)
{
    if (loggingEnabledForThisThread())
    {
        std::mbstate_t state = std::mbstate_t();
        std::mbsrtowcs(mWCharMessage, &markerName, kMaxMessageLength, &state);
        mUserDefinedAnnotation->SetMarker(mWCharMessage);
    }
}

bool DebugAnnotatorContext11::getStatus() const
{
    if (loggingEnabledForThisThread())
    {
        return !!(mUserDefinedAnnotation->GetStatus());
    }

    return false;
}

bool DebugAnnotatorContext11::loggingEnabledForThisThread() const
{
    return mUserDefinedAnnotation != nullptr &&
           angle::GetCurrentThreadUniqueId() == mAnnotationThread;
}

void DebugAnnotatorContext11::initialize(ID3D11DeviceContext *context)
{
    // ID3DUserDefinedAnnotation.GetStatus only works on Windows10 or greater.
    // Returning true unconditionally from DebugAnnotatorContext11::getStatus() means
    // writing out all compiled shaders to temporary files even if debugging
    // tools are not attached. See rx::ShaderD3D::prepareSourceAndReturnOptions.
    // If you want debug annotations, you must use Windows 10.
    if (IsWindows10OrLater())
    {
        mAnnotationThread = angle::GetCurrentThreadUniqueId();
        mUserDefinedAnnotation.Attach(
            d3d11::DynamicCastComObject<ID3DUserDefinedAnnotation>(context));
    }
}

void DebugAnnotatorContext11::release()
{
    mUserDefinedAnnotation.Reset();
}

}  // namespace rx
