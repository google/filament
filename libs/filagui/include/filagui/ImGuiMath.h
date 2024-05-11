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

#ifndef FILAGUI_IMGUIMATH_H_
#define FILAGUI_IMGUIMATH_H_

#include <imgui.h>

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return { lhs.x+rhs.x, lhs.y+rhs.y };
}

static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
    return { lhs.x-rhs.x, lhs.y-rhs.y };
}

#endif /* FILAGUI_IMGUIMATH_H_ */
