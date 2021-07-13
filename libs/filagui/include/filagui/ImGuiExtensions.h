/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef FILAGUI_IMGUIEXTENSIONS_H
#define FILAGUI_IMGUIEXTENSIONS_H

#include <utils/compiler.h>

struct ImVec2;

/**
 * Namespace for custom widgets that follow the API conventions used by ImGui.
 * For example, the prototype for ImGuiExt::DirectionWidget is similar to ImGui::DragFloat3.
 */
namespace ImGuiExt {
    /*
     * Draws an arrow widget for manipulating a unit vector (inspired by AntTweakBar).
     *
     * This adds a draggable 3D arrow widget to the current ImGui window, as well as a label and
     * three spin boxes that can be dragged or double-clicked for manual entry.
     *
     * The widget allow users to enter arbitrary vectors, so clients should copy the UI value
     * then normalize it. Clients should not directly normalize the UI value as this would cause
     * surprises for users who attempt to type in individual components.
     */
    UTILS_PUBLIC
    bool DirectionWidget(const char* label, float v[3]);

    /**
     * Draws a plot with multiple series. The parameters are the same as for ImGui::ImPlotLines
     * except for the following:
     * - series_start: called when a new series starts rendering, this can be used to customize
     *   the series' style for instance
     * - series_end: called when a series is done rendering
     * - values_getter: the first parameter indicates which series is being rendered
     */
    UTILS_PUBLIC
    void PlotLinesSeries(const char* label, int series_count,
            void (*series_start)(int series),
            float (*values_getter)(int series, void* data, int idx),
            void (*series_end)(int series),
            void* data, int values_count, int values_offset, const char* overlay_text,
            float scale_min, float scale_max, ImVec2 graph_size);
}

#endif // FILAGUI_IMGUIEXTENSIONS_H
