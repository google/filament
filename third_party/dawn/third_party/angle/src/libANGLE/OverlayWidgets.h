//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayWidgets.h:
//    Defines the Overlay* widget classes and corresponding enums.
//

#ifndef LIBANGLE_OVERLAYWIDGETS_H_
#define LIBANGLE_OVERLAYWIDGETS_H_

#include "common/angleutils.h"
#include "libANGLE/Overlay_autogen.h"

namespace gl
{
class Overlay;
class OverlayState;

namespace overlay_impl
{
class AppendWidgetDataHelper;
}  // namespace overlay_impl

enum class WidgetType
{
    // Text types:

    // A total count of some event.
    Count,
    // A single line of ASCII text.  Retains content until changed.
    Text,
    // A per-second value.
    PerSecond,

    // Graph types:

    // A graph of the last N values.
    RunningGraph,
    // A histogram of the last N values (values between 0 and 1).
    RunningHistogram,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

namespace overlay
{
class Text;
class Widget
{
  public:
    virtual ~Widget() {}

    virtual const Text *getDescriptionWidget() const;

  protected:
    WidgetType type;
    // Whether this item should be drawn.
    bool enabled = false;

    // For text items, size of the font.  This is a value in [0, overlay::kFontMipCount) which
    // determines the font size to use.
    int fontSize;

    // The area covered by the item, predetermined by the overlay class.  Negative values
    // indicate offset from the left/bottom of the image.
    int32_t coords[4];
    float color[4];

    // In some cases, a widget may need to match its contents (e.g. graph height scaling) with
    // another related widget.  In such a case, this pointer will point to the widget it needs to
    // match to.
    Widget *matchToWidget;

    friend class gl::Overlay;
    friend class gl::OverlayState;
    friend class overlay_impl::AppendWidgetDataHelper;
};

class Count : public Widget
{
  public:
    ~Count() override {}
    void add(uint64_t n) { count += n; }
    void set(uint64_t n) { count = n; }
    void reset() { count = 0; }

  protected:
    uint64_t count = 0;

    friend class gl::Overlay;
    friend class overlay_impl::AppendWidgetDataHelper;
};

class PerSecond : public Count
{
  public:
    ~PerSecond() override {}

  protected:
    uint64_t lastPerSecondCount = 0;

    friend class gl::Overlay;
    friend class overlay_impl::AppendWidgetDataHelper;
};

class Text : public Widget
{
  public:
    ~Text() override {}
    void set(std::string &&str) { text = std::move(str); }

  protected:
    std::string text;

    friend class overlay_impl::AppendWidgetDataHelper;
};

class RunningGraph : public Widget
{
  public:
    // Out of line constructor to satisfy chromium-style.
    RunningGraph(size_t n);
    ~RunningGraph() override;

    void add(uint64_t n)
    {
        if (!ignoreFirstValue)
        {
            runningValues[lastValueIndex] += n;
        }
    }

    void next()
    {
        if (ignoreFirstValue)
        {
            ignoreFirstValue = false;
        }
        else
        {
            lastValueIndex                = (lastValueIndex + 1) % runningValues.size();
            runningValues[lastValueIndex] = 0;
        }
    }

    const Text *getDescriptionWidget() const override;

  protected:
    std::vector<uint64_t> runningValues;
    size_t lastValueIndex = 0;
    Text description;
    bool ignoreFirstValue = true;

    friend class gl::Overlay;
    friend class gl::OverlayState;
    friend class overlay_impl::AppendWidgetDataHelper;
};

class RunningHistogram : public RunningGraph
{
  public:
    RunningHistogram(size_t n) : RunningGraph(n) {}
    ~RunningHistogram() override {}

    void set(float n)
    {
        ASSERT(n >= 0.0f && n <= 1.0f);
        uint64_t rank =
            n == 1.0f ? runningValues.size() - 1 : static_cast<uint64_t>(n * runningValues.size());

        runningValues[lastValueIndex] = rank;
    }

  private:
    // Do not use the add() function from RunningGraph
    using RunningGraph::add;
};

// If overlay is disabled, all the above classes would be replaced with Mock, turning them into
// noop.
class Mock
{
  public:
    void reset() const {}
    template <typename T>
    void set(T) const
    {}
    template <typename T>
    void add(T) const
    {}
    void next() const {}
};

}  // namespace overlay

}  // namespace gl

#endif  // LIBANGLE_OVERLAYWIDGETS_H_
