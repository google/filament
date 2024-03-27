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

#include <filagui/ImGuiExtensions.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <math/scalar.h>
#include <math/vec3.h>
#include <math/quat.h>

#include <algorithm>
#include <limits>

using namespace filament::math;

// Private helper class for ImGuiExt::DirectionWidget, heavily inspired by AntTweakBar and Chris
// Maughan's port to ImGui. Thanks Chris!
class ArrowWidget {
public:
    explicit ArrowWidget(float3 direction);
    bool draw();
    float3 getDirection() const;
private:
    quatf mDirectionQuat;
    enum EArrowParts { ARROW_CONE, ARROW_CONE_CAP, ARROW_CYL, ARROW_CYL_CAP };
    static void createArrow();
    void drawTriangles(ImDrawList* draw_list, const ImVec2& offset,
            const ImVector<ImVec2>& triProj, const ImVector<ImU32>& colLight, int numVertices);
    static float quatD(float w, float h) { return std::min(std::abs(w), std::abs(h)) - 4.0f; }
    static float quatPX(float x, float w, float h) { return (x*0.5f*quatD(w, h) + w*0.5f + 0.5f); }
    static float quatPY(float y, float w, float h) { return (-y*0.5f*quatD(w, h) + h*0.5f - 0.5f); }
    static float quatIX(int x, float w, float h) { return (2.0f*x - w - 1.0f) / quatD(w, h); }
    static float quatIY(int y, float w, float h) { return (-2.0f*y + h - 1.0f) / quatD(w, h); }
    static void quatFromDirection(quatf& quat, const float3& dir);
    static ImU32 blendColor(ImU32 c1, ImU32 c2, float t);
    const ImU32 DirColor = 0xffff0000;
    const int WidgetSize = 100;
};

static ImVector<float3> s_ArrowTri[4];
static ImVector<ImVec2> s_ArrowTriProj[4];
static ImVector<float3> s_ArrowNorm[4];
static ImVector<ImU32> s_ArrowColLight[4];

inline float ImVec2Cross(const ImVec2& left, const ImVec2& right) {
    return (left.x * right.y) - (left.y * right.x);
}

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x+rhs.x, lhs.y+rhs.y);
}

static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x-rhs.x, lhs.y-rhs.y);
}

namespace ImGuiExt {

bool DirectionWidget(const char* label, float v[3]) {
    ImGui::PushID(label);
    ImGui::BeginGroup();
    float3 dir = {v[0], v[1], v[2]};

    // DragFloat3 is nicer than SliderFloat3 because it supports double-clicking for keyboard entry.
    bool changed = ImGui::DragFloat3(label, &dir.x, 0.01f, -1.0f, 1.0f);
    if (changed) {
        v[0] = dir.x;
        v[1] = dir.y;
        v[2] = dir.z;
    }

    ArrowWidget widget(normalize(dir));
    if (widget.draw() && !changed) {
        changed = true;
        dir = widget.getDirection();
        v[0] = dir.x;
        v[1] = dir.y;
        v[2] = dir.z;
    }
    ImGui::EndGroup();
    ImGui::PopID();
    return changed;
}

static void PlotEx(ImGuiPlotType plot_type, const char* label, int series_count,
        void (*series_start)(int series),
        float (*values_getter)(int series, void* data, int idx),
        void (*series_end)(int series),
        void* data, int values_count, int values_offset, const char* overlay_text,
        float scale_min, float scale_max, ImVec2 graph_size) {

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (graph_size.x == 0.0f) graph_size.x = ImGui::CalcItemWidth();
    if (graph_size.y == 0.0f) graph_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb)) return;
    const bool hovered = ImGui::ItemHoverable(inner_bb, 0, g.LastItemData.InFlags);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX) {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int j = 0; j < series_count; j++) {
            for (int i = 0; i < values_count; i++) {
                const float v = values_getter(j, data, i);
                v_min = ImMin(v_min, v);
                v_max = ImMax(v_max, v);
            }
        }
        if (scale_min == FLT_MAX) scale_min = v_min;
        if (scale_max == FLT_MAX) scale_max = v_max;
    }

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    if (values_count > 0) {
        int res_w = ImMin((int)graph_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
        int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

        int v_hovered = -1;
        if (hovered) {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            v_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

        for (int s = 0; s < series_count; s++) {
            series_start(s);

            float v0 = values_getter(s, data, (0 + values_offset) % values_count);
            float t0 = 0.0f;
            ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle
            float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

            const ImU32 col_base = ImGui::GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
            const ImU32 col_hovered = ImGui::GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

            for (int n = 0; n < res_w; n++) {
                const float t1 = t0 + t_step;
                const int v1_idx = (int)(t0 * item_count + 0.5f);
                const float v1 = values_getter(s, data, (v1_idx + values_offset + 1) % values_count);
                const ImVec2 tp1 = ImVec2(t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale));

                // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
                ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
                ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max,
                        (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, histogram_zero_line_t));
                if (plot_type == ImGuiPlotType_Lines) {
                    window->DrawList->AddLine(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
                } else if (plot_type == ImGuiPlotType_Histogram) {
                    if (pos1.x >= pos0.x + 2.0f) {
                        pos1.x -= 1.0f;
                    }
                    window->DrawList->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
                }

                t0 = t1;
                tp0 = tp1;
            }

            series_end(s);
        }
    }

    // Text overlay
    if (overlay_text) {
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y),
                frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));
    }

    if (label_size.x > 0.0f) {
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
    }
}

void PlotLinesSeries(const char* label, int series_count,
        void (*series_start)(int series),
        float (*values_getter)(int series, void* data, int idx),
        void (*series_end)(int series),
        void* data, int values_count, int values_offset, const char* overlay_text,
        float scale_min, float scale_max, ImVec2 graph_size) {
    PlotEx(ImGuiPlotType_Lines, label, series_count, series_start, values_getter, series_end,
            data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

} // namespace ImGuiExt

ArrowWidget::ArrowWidget(float3 direction) {
    quatFromDirection(mDirectionQuat, direction);
}

float3 ArrowWidget::getDirection() const {
    float3 d = mDirectionQuat * float3(1, 0, 0);
    return d / length(d);
}

bool ArrowWidget::draw() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (s_ArrowTri[0].empty()) {
        ArrowWidget::createArrow();
    }

    bool value_changed = false;

    ImVec2 orient_pos = ImGui::GetCursorScreenPos();

    float sv_orient_size = std::min(ImGui::CalcItemWidth(), float(WidgetSize));
    float w = sv_orient_size;
    float h = sv_orient_size;

    // We want to generate quaternion rotations relative to the quaternion in the down press state.
    // This gives us cleaner control over rotation (it feels better)
    static quatf origQuat;
    static float3 coordOld;
    bool highlighted = false;
    ImGui::InvisibleButton("widget", ImVec2(sv_orient_size, sv_orient_size));
    if (ImGui::IsItemActive()) {
        highlighted = true;
        ImVec2 mouse = ImGui::GetMousePos() - orient_pos;
        if (ImGui::IsMouseClicked(0)) {
            origQuat = mDirectionQuat;
            coordOld = float3(quatIX((int)mouse.x, w, h), quatIY((int)mouse.y, w, h), 1.0f);
        } else if (ImGui::IsMouseDragging(0)) {
            float3 coord(quatIX((int)mouse.x, w, h), quatIY((int)mouse.y, w, h), 1.0f);
            float3 pVec = coord;
            float3 oVec = coordOld;
            coord.z = 0.0f;
            float n0 = length(oVec);
            float n1 = length(pVec);
            if (n0 > FLT_EPSILON && n1 > FLT_EPSILON) {
                float3 v0 = oVec / n0;
                float3 v1 = pVec / n1;
                float3 axis = cross(v0, v1);
                float sa = length(axis);
                float ca = dot(v0, v1);
                float angle = atan2(sa, ca);
                if (coord.x*coord.x + coord.y*coord.y > 1.0) {
                    angle *= 1.0f + 1.5f*(length(coord) - 1.0f);
                }
                quatf qrot, qres, qorig;
                qrot = quatf::fromAxisAngle(axis, angle);
                float nqorig = sqrt(origQuat.x * origQuat.x + origQuat.y * origQuat.y +
                        origQuat.z * origQuat.z + origQuat.w * origQuat.w);
                if (std::abs(nqorig) > FLT_EPSILON * FLT_EPSILON) {
                    qorig = origQuat / nqorig;
                    qres = qrot * qorig;
                    mDirectionQuat = qres;
                } else {
                    mDirectionQuat = qrot;
                }
                value_changed = true;
            }
        }
        draw_list->AddRectFilled(orient_pos, orient_pos + ImVec2(sv_orient_size, sv_orient_size),
                ImColor(style.Colors[ImGuiCol_FrameBgActive]), style.FrameRounding);
    } else {
        ImColor color(ImGui::IsItemHovered() ? style.Colors[ImGuiCol_FrameBgHovered] :
                style.Colors[ImGuiCol_FrameBg]);
        draw_list->AddRectFilled(orient_pos, orient_pos + ImVec2(sv_orient_size, sv_orient_size),
                color, style.FrameRounding);
    }

    ImVec2 inner_pos = orient_pos;
    quatf quat = normalize(mDirectionQuat);
    ImColor alpha(1.0f, 1.0f, 1.0f, highlighted ? 1.0f : 0.75f);
    float3 arrowDir = quat * float3(1, 0, 0);

    for (int k = 0; k < 4; ++k)  {
        int j = (arrowDir.z > 0) ? 3 - k : k;
        assert(s_ArrowTriProj[j].size() == (s_ArrowTri[j].size()) &&
                s_ArrowColLight[j].size() == s_ArrowTri[j].size() &&
                s_ArrowNorm[j].size() == s_ArrowTri[j].size());
        size_t ntri = s_ArrowTri[j].size();
        for (int i = 0; i < ntri; ++i) {
            float3 coord = s_ArrowTri[j][i];
            float3 norm = s_ArrowNorm[j][i];
            if (coord.x > 0) {
                coord.x = 2.5f * coord.x - 2.0f;
            } else {
                coord.x += 0.2f;
            }
            coord.y *= 1.5f;
            coord.z *= 1.5f;
            coord = quat * coord;
            norm = quat * norm;
            s_ArrowTriProj[j][i] = ImVec2(quatPX(coord.x, w, h), quatPY(coord.y, w, h));
            ImU32 col = (DirColor | 0xff000000) & alpha;
            s_ArrowColLight[j][i] = blendColor(0xff000000, col, std::abs(clamp(norm.z, -1.0f, 1.0f)));
        }
        drawTriangles(draw_list, inner_pos, s_ArrowTriProj[j], s_ArrowColLight[j], ntri);
    }

    return value_changed;
}


void ArrowWidget::drawTriangles(ImDrawList* draw_list, const ImVec2& offset,
        const ImVector<ImVec2>& triProj, const ImVector<ImU32>& colLight,
        int numVertices) {
    const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
    assert(numVertices % 3 == 0);
    draw_list->PrimReserve(numVertices, numVertices);
    for (int ii = 0; ii < numVertices / 3; ii++) {
        ImVec2 v1 = offset + triProj[ii * 3];
        ImVec2 v2 = offset + triProj[ii * 3 + 1];
        ImVec2 v3 = offset + triProj[ii * 3 + 2];

        // 2D cross product to do culling
        ImVec2 d1 = v2 - v1;
        ImVec2 d2 = v3 - v1;
        float c = ImVec2Cross(d1, d2);
        if (c > 0.0f) {
            v2 = v1;
            v3 = v1;
        }

        draw_list->PrimWriteIdx(ImDrawIdx(draw_list->_VtxCurrentIdx));
        draw_list->PrimWriteIdx(ImDrawIdx(draw_list->_VtxCurrentIdx + 1));
        draw_list->PrimWriteIdx(ImDrawIdx(draw_list->_VtxCurrentIdx + 2));
        draw_list->PrimWriteVtx(v1, uv, colLight[ii * 3]);
        draw_list->PrimWriteVtx(v2, uv, colLight[ii * 3 + 1]);
        draw_list->PrimWriteVtx(v3, uv, colLight[ii * 3 + 2]);
    }
}

void ArrowWidget::createArrow() {
    const int SUBDIV = 15;
    const float CYL_RADIUS = 0.08f;
    const float CONE_RADIUS = 0.16f;
    const float CONE_LENGTH = 0.25f;
    const float ARROW_BGN = -1.1f;
    const float ARROW_END = 1.15f;

    for (int i = 0; i < 4; ++i) {
        s_ArrowTri[i].clear();
        s_ArrowNorm[i].clear();
    }

    float x0, x1, y0, y1, z0, z1, a0, a1, nx, nn;
    for (int i = 0; i < SUBDIV; ++i) {
        a0 = 2.0f*float(F_PI)*(float(i)) / SUBDIV;
        a1 = 2.0f*float(F_PI)*(float(i + 1)) / SUBDIV;
        x0 = ARROW_BGN;
        x1 = ARROW_END - CONE_LENGTH;
        y0 = cosf(a0);
        z0 = sinf(a0);
        y1 = cosf(a1);
        z1 = sinf(a1);
        s_ArrowTri[ARROW_CYL].push_back(float3(x1, CYL_RADIUS*y0, CYL_RADIUS*z0));
        s_ArrowTri[ARROW_CYL].push_back(float3(x0, CYL_RADIUS*y0, CYL_RADIUS*z0));
        s_ArrowTri[ARROW_CYL].push_back(float3(x0, CYL_RADIUS*y1, CYL_RADIUS*z1));
        s_ArrowTri[ARROW_CYL].push_back(float3(x1, CYL_RADIUS*y0, CYL_RADIUS*z0));
        s_ArrowTri[ARROW_CYL].push_back(float3(x0, CYL_RADIUS*y1, CYL_RADIUS*z1));
        s_ArrowTri[ARROW_CYL].push_back(float3(x1, CYL_RADIUS*y1, CYL_RADIUS*z1));
        s_ArrowNorm[ARROW_CYL].push_back(float3(0, y0, z0));
        s_ArrowNorm[ARROW_CYL].push_back(float3(0, y0, z0));
        s_ArrowNorm[ARROW_CYL].push_back(float3(0, y1, z1));
        s_ArrowNorm[ARROW_CYL].push_back(float3(0, y0, z0));
        s_ArrowNorm[ARROW_CYL].push_back(float3(0, y1, z1));
        s_ArrowNorm[ARROW_CYL].push_back(float3(0, y1, z1));
        s_ArrowTri[ARROW_CYL_CAP].push_back(float3(x0, 0, 0));
        s_ArrowTri[ARROW_CYL_CAP].push_back(float3(x0, CYL_RADIUS*y1, CYL_RADIUS*z1));
        s_ArrowTri[ARROW_CYL_CAP].push_back(float3(x0, CYL_RADIUS*y0, CYL_RADIUS*z0));
        s_ArrowNorm[ARROW_CYL_CAP].push_back(float3(-1, 0, 0));
        s_ArrowNorm[ARROW_CYL_CAP].push_back(float3(-1, 0, 0));
        s_ArrowNorm[ARROW_CYL_CAP].push_back(float3(-1, 0, 0));
        x0 = ARROW_END - CONE_LENGTH;
        x1 = ARROW_END;
        nx = CONE_RADIUS / (x1 - x0);
        nn = 1.0f / sqrtf(nx*nx + 1);
        s_ArrowTri[ARROW_CONE].push_back(float3(x1, 0, 0));
        s_ArrowTri[ARROW_CONE].push_back(float3(x0, CONE_RADIUS*y0, CONE_RADIUS*z0));
        s_ArrowTri[ARROW_CONE].push_back(float3(x0, CONE_RADIUS*y1, CONE_RADIUS*z1));
        s_ArrowTri[ARROW_CONE].push_back(float3(x1, 0, 0));
        s_ArrowTri[ARROW_CONE].push_back(float3(x0, CONE_RADIUS*y1, CONE_RADIUS*z1));
        s_ArrowTri[ARROW_CONE].push_back(float3(x1, 0, 0));
        s_ArrowNorm[ARROW_CONE].push_back(float3(nn*nx, nn*y0, nn*z0));
        s_ArrowNorm[ARROW_CONE].push_back(float3(nn*nx, nn*y0, nn*z0));
        s_ArrowNorm[ARROW_CONE].push_back(float3(nn*nx, nn*y1, nn*z1));
        s_ArrowNorm[ARROW_CONE].push_back(float3(nn*nx, nn*y0, nn*z0));
        s_ArrowNorm[ARROW_CONE].push_back(float3(nn*nx, nn*y1, nn*z1));
        s_ArrowNorm[ARROW_CONE].push_back(float3(nn*nx, nn*y1, nn*z1));
        s_ArrowTri[ARROW_CONE_CAP].push_back(float3(x0, 0, 0));
        s_ArrowTri[ARROW_CONE_CAP].push_back(float3(x0, CONE_RADIUS*y1, CONE_RADIUS*z1));
        s_ArrowTri[ARROW_CONE_CAP].push_back(float3(x0, CONE_RADIUS*y0, CONE_RADIUS*z0));
        s_ArrowNorm[ARROW_CONE_CAP].push_back(float3(-1, 0, 0));
        s_ArrowNorm[ARROW_CONE_CAP].push_back(float3(-1, 0, 0));
        s_ArrowNorm[ARROW_CONE_CAP].push_back(float3(-1, 0, 0));
    }

    for (int i = 0; i < 4; ++i) {
        s_ArrowTriProj[i].clear();
        s_ArrowTriProj[i].resize(s_ArrowTri[i].size());
        s_ArrowColLight[i].clear();
        s_ArrowColLight[i].resize(s_ArrowTri[i].size());
    }
}

ImU32 ArrowWidget::blendColor(ImU32 c1, ImU32 c2, float t) {
    ImColor color1(c1);
    ImColor color2(c2);
    float invt = 1.0f - t;
    color1 = ImColor((color1.Value.x * invt) + (color2.Value.x * t),
        (color1.Value.y * invt) + (color2.Value.y * t),
        (color1.Value.z * invt) + (color2.Value.z * t),
        (color1.Value.w * invt) + (color2.Value.w * t));
    return color1;
}

// Generate a quaternion that rotates (1,0,0) to (dx,dy,dz).
void ArrowWidget::quatFromDirection(quatf& out, const float3& dir) {
    float dn = length(dir);
    if (dn < FLT_EPSILON * FLT_EPSILON) {
        out.x = out.y = out.z = 0;
        out.w = 1;
    } else {
        float3 rotAxis = { 0, -dir.z, dir.y };
        if (dot(rotAxis, rotAxis) < FLT_EPSILON * FLT_EPSILON) {
            rotAxis.x = rotAxis.y = 0; rotAxis.z = 1;
        }
        float rotAngle = acos(clamp(dir.x / dn, -1.0f, 1.0f));
        out = quatf::fromAxisAngle(rotAxis, rotAngle);
    }
}
