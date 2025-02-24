/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

namespace vvl {
class CommandBuffer;
class RenderPass;
}  // namespace vvl

namespace gpuav {
class Validator;

void UpdateCmdBufImageLayouts(Validator &gpuav, const vvl::CommandBuffer &cb_state);
void TransitionSubpassLayouts(vvl::CommandBuffer &cb_state, const vvl::RenderPass &render_pass_state, const int subpass_index);
void TransitionBeginRenderPassLayouts(vvl::CommandBuffer &cb_state, const vvl::RenderPass &render_pass_state);
void TransitionFinalSubpassLayouts(vvl::CommandBuffer &cb_state);

}  // namespace gpuav
