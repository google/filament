//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayWidgets.cpp:
//    Implements functions that interpret widget data.  Data formats and limits correspond to the
//    Vulkan implementation (as the only implementation).  They are generic enough so other backends
//    could respect them too, if they implement the overlay.
//

#include "libANGLE/Overlay.h"
#include "libANGLE/Overlay_font_autogen.h"

#include <functional>

namespace gl
{
namespace
{
// Internally, every widget is either Text or Graph.
enum class WidgetInternalType
{
    Text,
    Graph,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

// A map that says how the API-facing widget types map to internal types.
constexpr angle::PackedEnumMap<WidgetType, WidgetInternalType> kWidgetTypeToInternalMap = {
    {WidgetType::Count, WidgetInternalType::Text},
    {WidgetType::Text, WidgetInternalType::Text},
    {WidgetType::PerSecond, WidgetInternalType::Text},
    {WidgetType::RunningGraph, WidgetInternalType::Graph},
    {WidgetType::RunningHistogram, WidgetInternalType::Graph},
};

// Structures and limits matching uniform buffers in vulkan/shaders/src/OverlayDraw.comp.  The size
// of text and graph widgets is chosen such that they could fit in uniform buffers with minimum
// required Vulkan size.
constexpr size_t kMaxRenderableTextWidgets  = 32;
constexpr size_t kMaxRenderableGraphWidgets = 32;
constexpr size_t kMaxTextLength             = 256;
constexpr size_t kMaxGraphDataSize          = 256;

constexpr angle::PackedEnumMap<WidgetInternalType, size_t> kWidgetInternalTypeMaxWidgets = {
    {WidgetInternalType::Text, kMaxRenderableTextWidgets},
    {WidgetInternalType::Graph, kMaxRenderableGraphWidgets},
};

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

// Structure matching buffer in vulkan/shaders/src/OverlayCull.comp.
struct WidgetCoordinates
{
    uint32_t coordinates[kMaxRenderableTextWidgets + kMaxRenderableGraphWidgets][4];
};

// Structures matching buffers in vulkan/shaders/src/OverlayDraw.comp.
struct TextWidgetData
{
    uint32_t coordinates[4];
    float color[4];
    uint32_t fontSize[3];
    uint32_t padding;
    uint8_t text[kMaxTextLength];
};

struct GraphWidgetData
{
    uint32_t coordinates[4];
    float color[4];
    uint32_t valueWidth;
    uint32_t padding[3];
    uint32_t values[kMaxGraphDataSize];
};

struct TextWidgets
{
    TextWidgetData widgets[kMaxRenderableTextWidgets];
};

struct GraphWidgets
{
    GraphWidgetData widgets[kMaxRenderableGraphWidgets];
};

ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

uint32_t GetWidgetCoord(int32_t src, uint32_t extent)
{
    int32_t dst = src < 0 ? extent + src : src;

    return std::min<uint32_t>(std::max(dst, 0), extent - 1);
}

void GetWidgetCoordinates(const int32_t srcCoords[4],
                          const gl::Extents &imageExtent,
                          uint32_t dstCoordsOut[4])
{
    dstCoordsOut[0] = GetWidgetCoord(srcCoords[0], imageExtent.width);
    dstCoordsOut[1] = GetWidgetCoord(srcCoords[1], imageExtent.height);
    dstCoordsOut[2] = GetWidgetCoord(srcCoords[2], imageExtent.width);
    dstCoordsOut[3] = GetWidgetCoord(srcCoords[3], imageExtent.height);
}

void GetWidgetColor(const float srcColor[4], float dstColor[4])
{
    memcpy(dstColor, srcColor, 4 * sizeof(dstColor[0]));
}

void GetTextFontSize(int srcFontSize, uint32_t dstFontSize[3])
{
    // .xy contains the font glyph width/height
    dstFontSize[0] = overlay::kFontGlyphWidth >> srcFontSize;
    dstFontSize[1] = overlay::kFontGlyphHeight >> srcFontSize;
    // .z contains the mip
    dstFontSize[2] = srcFontSize;
}

void GetGraphValueWidth(const int32_t srcCoords[4], size_t valueCount, uint32_t *dstValueWidth)
{
    const int32_t graphWidth = std::abs(srcCoords[2] - srcCoords[0]);

    // If valueCount doesn't divide graphWidth, the graph bars won't fit well in its frame.
    // Fix initOverlayWidgets() in that case.
    ASSERT(graphWidth % valueCount == 0);

    *dstValueWidth = graphWidth / valueCount;
}

void GetTextString(const std::string &src, uint8_t textOut[kMaxTextLength])
{
    for (size_t i = 0; i < src.length() && i < kMaxTextLength; ++i)
    {
        // The font image has 95 ASCII characters starting from ' '.
        textOut[i] = src[i] - ' ';
    }
}

void GetGraphValues(const std::vector<uint64_t> srcValues,
                    size_t startIndex,
                    float scale,
                    uint32_t valuesOut[kMaxGraphDataSize])
{
    ASSERT(srcValues.size() <= kMaxGraphDataSize);

    for (size_t i = 0; i < srcValues.size(); ++i)
    {
        size_t index = (startIndex + i) % srcValues.size();
        valuesOut[i] = static_cast<uint32_t>(srcValues[index] * scale);
    }
}

std::vector<uint64_t> CreateHistogram(const std::vector<uint64_t> values)
{
    std::vector<uint64_t> histogram(values.size(), 0);

    for (uint64_t rank : values)
    {
        ++histogram[static_cast<size_t>(rank)];
    }

    return histogram;
}

using OverlayWidgetCounts  = angle::PackedEnumMap<WidgetInternalType, uint32_t>;
using AppendWidgetDataFunc = void (*)(const overlay::Widget *widget,
                                      const gl::Extents &imageExtent,
                                      TextWidgetData *textWidget,
                                      GraphWidgetData *graphWidget,
                                      OverlayWidgetCounts *widgetCounts);
}  // namespace

namespace overlay_impl
{
#define ANGLE_DECLARE_APPEND_WIDGET_PROC(WIDGET_ID)                                              \
    static void Append##WIDGET_ID(const overlay::Widget *widget, const gl::Extents &imageExtent, \
                                  TextWidgetData *textWidget, GraphWidgetData *graphWidget,      \
                                  OverlayWidgetCounts *widgetCounts);

// This class interprets the generic data collected in every element into a human-understandable
// widget.  This often means generating text specific to this item and scaling graph data to
// something sensible.
class AppendWidgetDataHelper
{
  public:
    ANGLE_WIDGET_ID_X(ANGLE_DECLARE_APPEND_WIDGET_PROC)

  private:
    static std::ostream &OutputPerSecond(std::ostream &out, const overlay::PerSecond *perSecond);

    static std::ostream &OutputText(std::ostream &out, const overlay::Text *text);

    static std::ostream &OutputCount(std::ostream &out, const overlay::Count *count);

    static void AppendTextCommon(const overlay::Widget *widget,
                                 const gl::Extents &imageExtent,
                                 const std::string &text,
                                 TextWidgetData *textWidget,
                                 OverlayWidgetCounts *widgetCounts);

    using FormatGraphTitleFunc = std::function<std::string(uint64_t curValue, uint64_t maxValue)>;
    static void AppendRunningGraphCommon(const overlay::Widget *widget,
                                         const gl::Extents &imageExtent,
                                         TextWidgetData *textWidget,
                                         GraphWidgetData *graphWidget,
                                         OverlayWidgetCounts *widgetCounts,
                                         FormatGraphTitleFunc formatFunc);

    using FormatHistogramTitleFunc =
        std::function<std::string(size_t peakRange, size_t maxValueRange, size_t numRanges)>;
    static void AppendRunningHistogramCommon(const overlay::Widget *widget,
                                             const gl::Extents &imageExtent,
                                             TextWidgetData *textWidget,
                                             GraphWidgetData *graphWidget,
                                             OverlayWidgetCounts *widgetCounts,
                                             FormatHistogramTitleFunc formatFunc);

    static void AppendGraphCommon(const overlay::Widget *widget,
                                  const gl::Extents &imageExtent,
                                  const std::vector<uint64_t> runningValues,
                                  size_t startIndex,
                                  float scale,
                                  GraphWidgetData *graphWidget,
                                  OverlayWidgetCounts *widgetCounts);
};

void AppendWidgetDataHelper::AppendTextCommon(const overlay::Widget *widget,
                                              const gl::Extents &imageExtent,
                                              const std::string &text,
                                              TextWidgetData *textWidget,
                                              OverlayWidgetCounts *widgetCounts)
{
    GetWidgetCoordinates(widget->coords, imageExtent, textWidget->coordinates);
    GetWidgetColor(widget->color, textWidget->color);
    GetTextFontSize(widget->fontSize, textWidget->fontSize);
    GetTextString(text, textWidget->text);

    ++(*widgetCounts)[WidgetInternalType::Text];
}

void AppendWidgetDataHelper::AppendGraphCommon(const overlay::Widget *widget,
                                               const gl::Extents &imageExtent,
                                               const std::vector<uint64_t> runningValues,
                                               size_t startIndex,
                                               float scale,
                                               GraphWidgetData *graphWidget,
                                               OverlayWidgetCounts *widgetCounts)
{
    const overlay::RunningGraph *widgetAsGraph = static_cast<const overlay::RunningGraph *>(widget);

    GetWidgetCoordinates(widget->coords, imageExtent, graphWidget->coordinates);
    GetWidgetColor(widget->color, graphWidget->color);
    GetGraphValueWidth(widget->coords, widgetAsGraph->runningValues.size(),
                       &graphWidget->valueWidth);
    GetGraphValues(runningValues, startIndex, scale, graphWidget->values);

    ++(*widgetCounts)[WidgetInternalType::Graph];
}

void AppendWidgetDataHelper::AppendRunningGraphCommon(
    const overlay::Widget *widget,
    const gl::Extents &imageExtent,
    TextWidgetData *textWidget,
    GraphWidgetData *graphWidget,
    OverlayWidgetCounts *widgetCounts,
    AppendWidgetDataHelper::FormatGraphTitleFunc formatFunc)
{
    const overlay::RunningGraph *graph   = static_cast<const overlay::RunningGraph *>(widget);
    const overlay::Widget *matchToWidget = widget->matchToWidget;

    if (matchToWidget == nullptr)
    {
        matchToWidget = widget;
    }
    const overlay::RunningGraph *matchToGraph =
        static_cast<const overlay::RunningGraph *>(matchToWidget);

    const uint64_t maxValue =
        *std::max_element(graph->runningValues.begin(), graph->runningValues.end());
    const uint64_t maxValueInMatchToGraph =
        *std::max_element(matchToGraph->runningValues.begin(), matchToGraph->runningValues.end());
    const int32_t graphHeight = std::abs(widget->coords[3] - widget->coords[1]);
    const float graphScale    = static_cast<float>(graphHeight) / maxValueInMatchToGraph;

    const size_t graphSize  = graph->runningValues.size();
    const size_t currentIdx = graph->lastValueIndex - 1;

    const uint64_t curValue = graph->runningValues[(graphSize + currentIdx) % graphSize];

    AppendGraphCommon(widget, imageExtent, graph->runningValues, graph->lastValueIndex + 1,
                      graphScale, graphWidget, widgetCounts);

    if ((*widgetCounts)[WidgetInternalType::Text] <
        kWidgetInternalTypeMaxWidgets[WidgetInternalType::Text])
    {
        std::string text = formatFunc(curValue, maxValue);
        AppendTextCommon(&graph->description, imageExtent, text, textWidget, widgetCounts);
    }
}

// static
void AppendWidgetDataHelper::AppendRunningHistogramCommon(const overlay::Widget *widget,
                                                          const gl::Extents &imageExtent,
                                                          TextWidgetData *textWidget,
                                                          GraphWidgetData *graphWidget,
                                                          OverlayWidgetCounts *widgetCounts,
                                                          FormatHistogramTitleFunc formatFunc)
{
    const overlay::RunningHistogram *runningHistogram =
        static_cast<const overlay::RunningHistogram *>(widget);

    std::vector<uint64_t> histogram = CreateHistogram(runningHistogram->runningValues);
    auto peakRangeIt                = std::max_element(histogram.rbegin(), histogram.rend());
    const uint64_t peakRangeValue   = *peakRangeIt;
    const int32_t graphHeight       = std::abs(widget->coords[3] - widget->coords[1]);
    const float graphScale          = static_cast<float>(graphHeight) / peakRangeValue;
    auto maxValueIter               = std::find_if(histogram.rbegin(), histogram.rend(),
                                                   [](uint64_t value) { return value != 0; });

    AppendGraphCommon(widget, imageExtent, histogram, 0, graphScale, graphWidget, widgetCounts);

    if ((*widgetCounts)[WidgetInternalType::Text] <
        kWidgetInternalTypeMaxWidgets[WidgetInternalType::Text])
    {
        size_t peakRange     = std::distance(peakRangeIt, histogram.rend() - 1);
        size_t maxValueRange = std::distance(maxValueIter, histogram.rend() - 1);

        std::string text = formatFunc(peakRange, maxValueRange, histogram.size());
        AppendTextCommon(&runningHistogram->description, imageExtent, text, textWidget,
                         widgetCounts);
    }
}

void AppendWidgetDataHelper::AppendFPS(const overlay::Widget *widget,
                                       const gl::Extents &imageExtent,
                                       TextWidgetData *textWidget,
                                       GraphWidgetData *graphWidget,
                                       OverlayWidgetCounts *widgetCounts)
{
    const overlay::PerSecond *fps = static_cast<const overlay::PerSecond *>(widget);
    std::ostringstream text;
    text << "FPS: ";
    OutputPerSecond(text, fps);

    AppendTextCommon(widget, imageExtent, text.str(), textWidget, widgetCounts);
}

void AppendWidgetDataHelper::AppendVulkanLastValidationMessage(const overlay::Widget *widget,
                                                               const gl::Extents &imageExtent,
                                                               TextWidgetData *textWidget,
                                                               GraphWidgetData *graphWidget,
                                                               OverlayWidgetCounts *widgetCounts)
{
    const overlay::Text *lastValidationMessage = static_cast<const overlay::Text *>(widget);
    std::ostringstream text;
    text << "Last VVL Message: ";
    OutputText(text, lastValidationMessage);

    AppendTextCommon(widget, imageExtent, text.str(), textWidget, widgetCounts);
}

void AppendWidgetDataHelper::AppendVulkanValidationMessageCount(const overlay::Widget *widget,
                                                                const gl::Extents &imageExtent,
                                                                TextWidgetData *textWidget,
                                                                GraphWidgetData *graphWidget,
                                                                OverlayWidgetCounts *widgetCounts)
{
    const overlay::Count *validationMessageCount = static_cast<const overlay::Count *>(widget);
    std::ostringstream text;
    text << "VVL Message Count: ";
    OutputCount(text, validationMessageCount);

    AppendTextCommon(widget, imageExtent, text.str(), textWidget, widgetCounts);
}

void AppendWidgetDataHelper::AppendVulkanRenderPassCount(const overlay::Widget *widget,
                                                         const gl::Extents &imageExtent,
                                                         TextWidgetData *textWidget,
                                                         GraphWidgetData *graphWidget,
                                                         OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "RenderPass Count: " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanSecondaryCommandBufferPoolWaste(
    const overlay::Widget *widget,
    const gl::Extents &imageExtent,
    TextWidgetData *textWidget,
    GraphWidgetData *graphWidget,
    OverlayWidgetCounts *widgetCounts)
{
    auto format = [](size_t peakRange, size_t maxValueRange, size_t numRanges) {
        std::ostringstream text;
        size_t peakPercent = (peakRange * 100 + 50) / numRanges;
        text << "CB Pool Waste (Peak: " << peakPercent << "%)";
        return text.str();
    };

    AppendRunningHistogramCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts,
                                 format);
}

void AppendWidgetDataHelper::AppendVulkanWriteDescriptorSetCount(const overlay::Widget *widget,
                                                                 const gl::Extents &imageExtent,
                                                                 TextWidgetData *textWidget,
                                                                 GraphWidgetData *graphWidget,
                                                                 OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "WriteDescriptorSet Count: " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanDescriptorSetAllocations(const overlay::Widget *widget,
                                                                  const gl::Extents &imageExtent,
                                                                  TextWidgetData *textWidget,
                                                                  GraphWidgetData *graphWidget,
                                                                  OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Descriptor Set Allocations: " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanShaderResourceDSHitRate(const overlay::Widget *widget,
                                                                 const gl::Extents &imageExtent,
                                                                 TextWidgetData *textWidget,
                                                                 GraphWidgetData *graphWidget,
                                                                 OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Shader Resource DS Hit Rate (Max: " << maxValue << "%)";
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanDynamicBufferAllocations(const overlay::Widget *widget,
                                                                  const gl::Extents &imageExtent,
                                                                  TextWidgetData *textWidget,
                                                                  GraphWidgetData *graphWidget,
                                                                  OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "DynamicBuffer Allocations (Max: " << maxValue << ")";
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanTextureDescriptorCacheSize(
    const overlay::Widget *widget,
    const gl::Extents &imageExtent,
    TextWidgetData *textWidget,
    GraphWidgetData *graphWidget,
    OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Total Texture Descriptor Cache Size: " << curValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanUniformDescriptorCacheSize(
    const overlay::Widget *widget,
    const gl::Extents &imageExtent,
    TextWidgetData *textWidget,
    GraphWidgetData *graphWidget,
    OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Total Uniform Descriptor Cache Size: " << curValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanDescriptorCacheSize(const overlay::Widget *widget,
                                                             const gl::Extents &imageExtent,
                                                             TextWidgetData *textWidget,
                                                             GraphWidgetData *graphWidget,
                                                             OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Total Descriptor Cache Size: " << curValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanDescriptorCacheKeySize(const overlay::Widget *widget,
                                                                const gl::Extents &imageExtent,
                                                                TextWidgetData *textWidget,
                                                                GraphWidgetData *graphWidget,
                                                                OverlayWidgetCounts *widgetCounts)
{
    const overlay::Count *countWidget = static_cast<const overlay::Count *>(widget);
    std::ostringstream text;
    double kb = static_cast<double>(countWidget->count) / 1000.0;
    text << "DS Cache Key Size: " << std::fixed << std::setprecision(1) << kb << " kb";

    AppendTextCommon(widget, imageExtent, text.str(), textWidget, widgetCounts);
}

void AppendWidgetDataHelper::AppendVulkanAttemptedSubmissions(const overlay::Widget *widget,
                                                              const gl::Extents &imageExtent,
                                                              TextWidgetData *textWidget,
                                                              GraphWidgetData *graphWidget,
                                                              OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Attempted submissions (peak): " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanActualSubmissions(const overlay::Widget *widget,
                                                           const gl::Extents &imageExtent,
                                                           TextWidgetData *textWidget,
                                                           GraphWidgetData *graphWidget,
                                                           OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Actual submissions (peak): " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanPipelineCacheLookups(const overlay::Widget *widget,
                                                              const gl::Extents &imageExtent,
                                                              TextWidgetData *textWidget,
                                                              GraphWidgetData *graphWidget,
                                                              OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Pipeline Cache Lookups (peak): " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanPipelineCacheMisses(const overlay::Widget *widget,
                                                             const gl::Extents &imageExtent,
                                                             TextWidgetData *textWidget,
                                                             GraphWidgetData *graphWidget,
                                                             OverlayWidgetCounts *widgetCounts)
{
    auto format = [](uint64_t curValue, uint64_t maxValue) {
        std::ostringstream text;
        text << "Pipeline Cache Misses (peak): " << maxValue;
        return text.str();
    };

    AppendRunningGraphCommon(widget, imageExtent, textWidget, graphWidget, widgetCounts, format);
}

void AppendWidgetDataHelper::AppendVulkanTotalPipelineCacheHitTimeMs(
    const overlay::Widget *widget,
    const gl::Extents &imageExtent,
    TextWidgetData *textWidget,
    GraphWidgetData *graphWidget,
    OverlayWidgetCounts *widgetCounts)
{
    const overlay::Count *totalTime = static_cast<const overlay::Count *>(widget);
    std::ostringstream text;
    text << "Total Pipeline Cache Hit Time: ";
    OutputCount(text, totalTime);
    text << "ms";

    AppendTextCommon(widget, imageExtent, text.str(), textWidget, widgetCounts);
}

void AppendWidgetDataHelper::AppendVulkanTotalPipelineCacheMissTimeMs(
    const overlay::Widget *widget,
    const gl::Extents &imageExtent,
    TextWidgetData *textWidget,
    GraphWidgetData *graphWidget,
    OverlayWidgetCounts *widgetCounts)
{
    const overlay::Count *totalTime = static_cast<const overlay::Count *>(widget);
    std::ostringstream text;
    text << "Total Pipeline Cache Miss Time: ";
    OutputCount(text, totalTime);
    text << "ms";

    AppendTextCommon(widget, imageExtent, text.str(), textWidget, widgetCounts);
}

std::ostream &AppendWidgetDataHelper::OutputPerSecond(std::ostream &out,
                                                      const overlay::PerSecond *perSecond)
{
    return out << perSecond->lastPerSecondCount;
}

std::ostream &AppendWidgetDataHelper::OutputText(std::ostream &out, const overlay::Text *text)
{
    return out << text->text;
}

std::ostream &AppendWidgetDataHelper::OutputCount(std::ostream &out, const overlay::Count *count)
{
    return out << count->count;
}
}  // namespace overlay_impl

namespace
{
#define ANGLE_APPEND_WIDGET_MAP_PROC(WIDGET_ID) \
    {WidgetId::WIDGET_ID, overlay_impl::AppendWidgetDataHelper::Append##WIDGET_ID},

constexpr angle::PackedEnumMap<WidgetId, AppendWidgetDataFunc> kWidgetIdToAppendDataFuncMap = {
    ANGLE_WIDGET_ID_X(ANGLE_APPEND_WIDGET_MAP_PROC)};
}  // namespace

namespace overlay
{
const Text *Widget::getDescriptionWidget() const
{
    return nullptr;
}
RunningGraph::RunningGraph(size_t n) : runningValues(n, 0) {}
RunningGraph::~RunningGraph() = default;
const Text *RunningGraph::getDescriptionWidget() const
{
    return &description;
}
}  // namespace overlay

size_t OverlayState::getWidgetCoordinatesBufferSize() const
{
    return sizeof(WidgetCoordinates);
}

size_t OverlayState::getTextWidgetsBufferSize() const
{
    return sizeof(TextWidgets);
}

size_t OverlayState::getGraphWidgetsBufferSize() const
{
    return sizeof(GraphWidgets);
}

void OverlayState::fillWidgetData(const gl::Extents &imageExtents,
                                  uint8_t *textData,
                                  uint8_t *graphData,
                                  uint32_t *activeTextWidgetCountOut,
                                  uint32_t *activeGraphWidgetCountOut) const
{
    TextWidgets *textWidgets   = reinterpret_cast<TextWidgets *>(textData);
    GraphWidgets *graphWidgets = reinterpret_cast<GraphWidgets *>(graphData);

    memset(textWidgets, overlay::kFontCharacters, sizeof(*textWidgets));
    memset(graphWidgets, 0, sizeof(*graphWidgets));

    OverlayWidgetCounts widgetCounts = {};

    for (WidgetId id : angle::AllEnums<WidgetId>())
    {
        const std::unique_ptr<overlay::Widget> &widget = mOverlayWidgets[id];
        if (!widget->enabled)
        {
            continue;
        }

        WidgetInternalType internalType = kWidgetTypeToInternalMap[widget->type];
        ASSERT(internalType != WidgetInternalType::InvalidEnum);

        if (widgetCounts[internalType] >= kWidgetInternalTypeMaxWidgets[internalType])
        {
            continue;
        }

        AppendWidgetDataFunc appendFunc = kWidgetIdToAppendDataFuncMap[id];
        ASSERT(appendFunc);
        appendFunc(widget.get(), imageExtents,
                   &textWidgets->widgets[widgetCounts[WidgetInternalType::Text]],
                   &graphWidgets->widgets[widgetCounts[WidgetInternalType::Graph]], &widgetCounts);
    }

    *activeTextWidgetCountOut  = widgetCounts[WidgetInternalType::Text];
    *activeGraphWidgetCountOut = widgetCounts[WidgetInternalType::Graph];
}

}  // namespace gl
