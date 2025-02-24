//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Overlay.cpp:
//    Implements the Overlay class.
//

#include "libANGLE/Overlay.h"

#include "common/string_utils.h"
#include "common/system_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Overlay_font_autogen.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/OverlayImpl.h"

#include <numeric>

namespace gl
{
namespace
{
#define ANGLE_WIDGET_NAME_PROC(WIDGET_ID) {ANGLE_STRINGIFY(WIDGET_ID), WidgetId::WIDGET_ID},

constexpr std::pair<const char *, WidgetId> kWidgetNames[] = {
    ANGLE_WIDGET_ID_X(ANGLE_WIDGET_NAME_PROC)};
}  // namespace

OverlayState::OverlayState() : mEnabledWidgetCount(0), mOverlayWidgets{} {}
OverlayState::~OverlayState() = default;

Overlay::Overlay(rx::GLImplFactory *factory)
    : mLastPerSecondUpdate(0), mImplementation(factory->createOverlay(mState))
{}
Overlay::~Overlay() = default;

void Overlay::init()
{
    initOverlayWidgets();
    mLastPerSecondUpdate = angle::GetCurrentSystemTime();

    ASSERT(std::all_of(
        mState.mOverlayWidgets.begin(), mState.mOverlayWidgets.end(),
        [](const std::unique_ptr<overlay::Widget> &widget) { return widget.get() != nullptr; }));

    enableOverlayWidgetsFromEnvironment();
}

void Overlay::destroy(const gl::Context *context)
{
    ASSERT(mImplementation);
    mImplementation->onDestroy(context);
}

void Overlay::enableOverlayWidgetsFromEnvironment()
{
    std::vector<std::string> enabledWidgets = angle::GetStringsFromEnvironmentVarOrAndroidProperty(
        "ANGLE_OVERLAY", "debug.angle.overlay", ":");

    for (const std::pair<const char *, WidgetId> &widgetName : kWidgetNames)
    {
        for (const std::string &enabledWidget : enabledWidgets)
        {
            if (angle::NamesMatchWithWildcard(enabledWidget.c_str(), widgetName.first))
            {
                mState.mOverlayWidgets[widgetName.second]->enabled = true;
                ++mState.mEnabledWidgetCount;
                break;
            }
        }
    }
}

void Overlay::onSwap() const
{
    // Increment FPS counter.
    getPerSecondWidget(WidgetId::FPS)->add(1);

    // Update per second values every second.
    double currentTime = angle::GetCurrentSystemTime();
    double timeDiff    = currentTime - mLastPerSecondUpdate;
    if (timeDiff >= 1.0)
    {
        for (const std::unique_ptr<overlay::Widget> &widget : mState.mOverlayWidgets)
        {
            if (widget->type == WidgetType::PerSecond)
            {
                overlay::PerSecond *perSecond =
                    reinterpret_cast<overlay::PerSecond *>(widget.get());
                perSecond->lastPerSecondCount = static_cast<size_t>(perSecond->count / timeDiff);
                perSecond->count              = 0;
            }
        }
        mLastPerSecondUpdate += 1.0;
    }
}

MockOverlay::MockOverlay(rx::GLImplFactory *implFactory) {}
MockOverlay::~MockOverlay() = default;

}  // namespace gl
