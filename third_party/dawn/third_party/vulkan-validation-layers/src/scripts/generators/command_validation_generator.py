#!/usr/bin/python3 -i
#
# Copyright (c) 2021-2025 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
from generators.generator_utils import buildListVUID, getVUID
from generators.vulkan_object import Queues, CommandScope
from generators.base_generator import BaseGenerator
#
# CommandValidationOutputGenerator - Generate implicit vkCmd validation for CoreChecks
class CommandValidationOutputGenerator(BaseGenerator):
    def __init__(self,
                 valid_usage_file):
        BaseGenerator.__init__(self)
        self.valid_vuids = buildListVUID(valid_usage_file)
    #
    # Called at beginning of processing as file is opened
    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2021-2025 Valve Corporation
            * Copyright (c) 2021-2025 LunarG, Inc.
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
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'command_validation.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateSource(self):
        out = []
        out.append('''
            #include "error_message/logging.h"
            #include "core_checks/core_validation.h"

            enum CMD_SCOPE_TYPE { CMD_SCOPE_INSIDE, CMD_SCOPE_OUTSIDE, CMD_SCOPE_BOTH };

            struct CommandValidationInfo {
                const char* recording_vuid;
                const char* buffer_level_vuid;

                VkQueueFlags queue_flags;
                const char* queue_vuid;

                CMD_SCOPE_TYPE render_pass;
                const char* render_pass_vuid;

                CMD_SCOPE_TYPE video_coding;
                const char* video_coding_vuid;
            };

            using Func = vvl::Func;
            ''')
        out.append('// clang-format off\n')
        out.append('const auto &GetCommandValidationTable() {\n')
        out.append('static const vvl::unordered_map<Func, CommandValidationInfo> kCommandValidationTable {\n')
        for command in [x for x in self.vk.commands.values() if x.name.startswith('vkCmd')]:
            out.append(f'{{Func::{command.name}, {{\n')
            # recording_vuid
            alias_name = command.name if command.alias is None else command.alias
            vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-commandBuffer-recording')
            out.append(f'    {vuid},\n')

            # buffer_level_vuid
            if command.primary and command.secondary:
                out.append('    nullptr,\n')
            elif command.primary:
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-bufferlevel')
                out.append(f'    {vuid},\n')
            else:
                # Currently there is only "primary" or "primary,secondary" in XML
                # Hard to predict what might change, so will error out instead if assumption breaks
                print('cmdbufferlevel attribute was and not known, need to update generation code')
                sys.exit(1)

            # queue_flags / queue_vuid
            queue_flags = []
            queue_flags.extend(["VK_QUEUE_GRAPHICS_BIT"] if Queues.GRAPHICS & command.queues else [])
            queue_flags.extend(["VK_QUEUE_COMPUTE_BIT"] if Queues.COMPUTE & command.queues else [])
            queue_flags.extend(["VK_QUEUE_TRANSFER_BIT"] if Queues.TRANSFER & command.queues else [])
            queue_flags.extend(["VK_QUEUE_SPARSE_BINDING_BIT"] if Queues.SPARSE_BINDING & command.queues else [])
            queue_flags.extend(["VK_QUEUE_PROTECTED_BIT"] if Queues.PROTECTED & command.queues else [])
            queue_flags.extend(["VK_QUEUE_VIDEO_DECODE_BIT_KHR"] if Queues.DECODE & command.queues else [])
            queue_flags.extend(["VK_QUEUE_VIDEO_ENCODE_BIT_KHR"] if Queues.ENCODE & command.queues else [])
            queue_flags.extend(["VK_QUEUE_OPTICAL_FLOW_BIT_NV"] if Queues.OPTICAL_FLOW & command.queues else [])
            queue_flags = ' | '.join(queue_flags)

            vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-commandBuffer-cmdpool')
            out.append(f'    {queue_flags}, {vuid},\n')

            # render_pass / render_pass_vuid
            renderPassType = 'CMD_SCOPE_BOTH'
            vuid = '"kVUIDUndefined"' # Only will be a VUID if not BOTH
            if command.renderPass is CommandScope.INSIDE:
                renderPassType = 'CMD_SCOPE_INSIDE'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-renderpass')
            elif command.renderPass is CommandScope.OUTSIDE:
                renderPassType = 'CMD_SCOPE_OUTSIDE'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-renderpass')
            out.append(f'    {renderPassType}, {vuid},\n')

            # video_coding / video_coding_vuid
            videoCodingType = 'CMD_SCOPE_BOTH'
            vuid = '"kVUIDUndefined"' # Only will be a VUID if not BOTH
            if command.videoCoding is CommandScope.INSIDE:
                videoCodingType = 'CMD_SCOPE_INSIDE'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-videocoding')
            elif command.videoCoding is CommandScope.OUTSIDE or command.videoCoding is CommandScope.NONE:
                videoCodingType = 'CMD_SCOPE_OUTSIDE'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-videocoding')
            out.append(f'    {videoCodingType}, {vuid},\n')

            out.append('}},\n')
        out.append('};\n')
        out.append('return kCommandValidationTable;\n')
        out.append('}\n')
        out.append('// clang-format on\n')

        #
        # The main function to validate all the commands
        # TODO - Remove C++ code from being a single python string
        out.append('''
            // Ran on all vkCmd* commands
            // Because it validate the implicit VUs that stateless can't, if this fails, it is likely
            // the input is very bad and other checks will crash dereferencing null pointers
            bool CoreChecks::ValidateCmd(const vvl::CommandBuffer& cb_state, const Location& loc) const {
                bool skip = false;

                auto info_it = GetCommandValidationTable().find(loc.function);
                if (info_it == GetCommandValidationTable().end()) {
                    assert(false);
                }
                const auto& info = info_it->second;

                // Validate the given command being added to the specified cmd buffer,
                // flagging errors if CB is not in the recording state or if there's an issue with the Cmd ordering
                switch (cb_state.state) {
                    case CbState::Recording:
                        skip |= ValidateCmdSubpassState(cb_state, loc, info.recording_vuid);
                        break;

                    case CbState::InvalidComplete:
                    case CbState::InvalidIncomplete:
                        skip |= ReportInvalidCommandBuffer(cb_state, loc, info.recording_vuid);
                        break;

                    default:
                        assert(loc.function != Func::Empty);
                        skip |= LogError(info.recording_vuid, cb_state.Handle(), loc, "was called before vkBeginCommandBuffer().");
                }

                // Validate the command pool from which the command buffer is from that the command is allowed for queue type
                if (!HasRequiredQueueFlags(cb_state, *physical_device_state, info.queue_flags)) {
                    const LogObjectList objlist(cb_state.Handle(), cb_state.command_pool->Handle());
                    skip |= LogError(info.queue_vuid, objlist, loc,
                                "%s", DescribeRequiredQueueFlag(cb_state, *physical_device_state, info.queue_flags).c_str());
                }

                // Validate if command is inside or outside a render pass if applicable
                if (info.render_pass == CMD_SCOPE_INSIDE) {
                    skip |= OutsideRenderPass(cb_state, loc, info.render_pass_vuid);
                } else if (info.render_pass == CMD_SCOPE_OUTSIDE) {
                    skip |= InsideRenderPass(cb_state, loc, info.render_pass_vuid);
                }

                // Validate if command is inside or outside a video coding scope if applicable
                if (info.video_coding == CMD_SCOPE_INSIDE) {
                    skip |= OutsideVideoCodingScope(cb_state, loc, info.video_coding_vuid);
                } else if (info.video_coding == CMD_SCOPE_OUTSIDE) {
                    skip |= InsideVideoCodingScope(cb_state, loc, info.video_coding_vuid);
                }

                // Validate if command has to be recorded in a primary command buffer
                if (info.buffer_level_vuid != nullptr) {
                    skip |= ValidatePrimaryCommandBuffer(cb_state, loc, info.buffer_level_vuid);
                }

                return skip;
            }''')
        self.write("".join(out))