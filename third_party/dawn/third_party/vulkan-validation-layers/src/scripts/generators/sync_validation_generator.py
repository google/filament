#!/usr/bin/python3 -i
#
# Copyright (c) 2023-2025 The Khronos Group Inc.
# Copyright (c) 2023-2025 LunarG, Inc.
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

import sys
import os
from generators.vulkan_object import Flag
from generators.base_generator import BaseGenerator

separator = ' |\n        '

def BitSuffixed(name):
    alt_bit = ('_ANDROID', '_EXT', '_IMG', '_KHR', '_NV', '_NVX', '_SYNCVAL')
    bit_suf = name + '_BIT'
    # Since almost every bit ends with _KHR, just ignore it.
    # Otherwise some generated names end up with every other word being KHR.
    if name.endswith('_KHR') :
            bit_suf = name.replace('_KHR', '_BIT')
    else:
        for alt in alt_bit:
            if name.endswith(alt) :
                bit_suf = name.replace(alt, '_BIT' + alt)
                break
    return bit_suf

def SortSetBasedOnOrder(stage_set, stage_order):
    return [ stage for stage in stage_order if stage in stage_set]


class SyncValidationOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # List of all stages sorted according to enum numeric value.
        # This does not include stages similar to ALL_GRAPHICS that represent multiple stages
        self.stages = []

        # List of stages obtained from merge-sorting ordered stages from each pipeline type.
        # This defines how the stages are are ordered in the ealiest/latest stage bitmask
        self.logicallyOrderedStages = []

        # pipeline names from <syncpipeline>
        self.pipelineNames = []

        # < pipeline_name, [pipeline stages in logical order (exactly as defined in XML)] >
        self.pipelineStages = dict()

        # < pipeline_name, [{ stage : 'stage', ordered: True/False, after : [stages], before : [stages] }]  >
        # Each stage includes ordering info but also the stages itself are ordered according to
        # order/before/after directives. So, if you need iterate over stages from specific pipeline type
        # according to all ordering constrains just iterate over the list as asual.
        self.pipelineStagesOrdered = dict()

        # < queue type, [stages] >
        self.queueToStages = dict()

        self.stageAccessCombo = []

        # fake stages and accesses for acquire present support
        self.pipelineStagePresentEngine = Flag('VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL', 0, False, False, None, None)
        self.accessAcquireRead = Flag('VK_ACCESS_2_PRESENT_ACQUIRE_READ_BIT_SYNCVAL', 0, False, False, None, None)
        self.accessPresented = Flag('VK_ACCESS_2_PRESENT_PRESENTED_BIT_SYNCVAL', 0, False, False, None, None)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
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

        # Set value to be at end of bitmask
        self.pipelineStagePresentEngine.value = max([x.value for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags]) + 1
        self.accessAcquireRead.value = max([x.value for x in self.vk.bitmasks['VkAccessFlagBits2'].flags]) + 1
        self.accessPresented.value = max([x.value for x in self.vk.bitmasks['VkAccessFlagBits2'].flags]) + 2

        # Add into the VulkanObject so logic works as if they were in the XML
        self.vk.bitmasks['VkPipelineStageFlagBits2'].flags.append(self.pipelineStagePresentEngine)
        self.vk.bitmasks['VkAccessFlagBits2'].flags.append(self.accessAcquireRead)
        self.vk.bitmasks['VkAccessFlagBits2'].flags.append(self.accessPresented)

        present_stage = 'VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL'

        # Get stages in logical order
        self.logicallyOrderedStages = ['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT']
        for pipeline_name in self.pipelineNames:
            for index, stage_info in enumerate(self.pipelineStagesOrdered[pipeline_name]):
                stage = stage_info['stage']
                if stage not in self.logicallyOrderedStages:
                    later_stages =[s['stage'] for s in self.pipelineStagesOrdered[pipeline_name][index+1:]]
                    insert_loc = len(self.logicallyOrderedStages)
                    while insert_loc > 0:
                        if any(s in self.logicallyOrderedStages[:insert_loc] for s in later_stages):
                            insert_loc -= 1
                        else:
                            break
                    self.logicallyOrderedStages.insert(insert_loc, stage)
        self.logicallyOrderedStages.append('VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT')

        self.stages = [x.flag.name for x in self.vk.syncStage if x.equivalent.max]
        self.stages.append(present_stage)
        # sort self.stages based on VkPipelineStageFlagBits2 bit order
        sort_order = {y.name : y.value for y in sorted([x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags], key=lambda x: x.value)}
        sort_order['VK_PIPELINE_STAGE_2_NONE'] = -1
        self.stages.sort(key=lambda stage: sort_order[stage])

        self.stageAccessCombo = self.createStageAccessCombinations()

        if self.filename == 'sync_validation_types.h':
            self.generateHeader()
        elif self.filename == 'sync_validation_types.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <array>
            #include <bitset>
            #include <map>
            #include <stdint.h>
            #include <vulkan/vulkan.h>
            #include "containers/custom_containers.h"
            ''')
        out.append('// clang-format off\n')

        shader_read_access = next((a for a in self.vk.syncAccess if a.flag.name == 'VK_ACCESS_2_SHADER_READ_BIT'), None)
        shader_read_expansion = [e.name for e in shader_read_access.equivalent.accesses]
        out.append(f'static constexpr VkAccessFlags2 kShaderReadExpandBits = {"|".join(shader_read_expansion)};\n')

        shader_write_access = next((a for a in self.vk.syncAccess if a.flag.name == 'VK_ACCESS_2_SHADER_WRITE_BIT'), None)
        shader_write_expansion = [e.name for e in shader_write_access.equivalent.accesses]
        out.append(f'static constexpr VkAccessFlags2 kShaderWriteExpandBits = {"|".join(shader_write_expansion)};\n')

        all_transfer_stage = next((s for s in self.vk.syncStage if s.flag.name == 'VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT'), None)
        all_transfer_expansion = [e.name for e in all_transfer_stage.equivalent.stages]
        out.append(f'static constexpr VkPipelineStageFlags2 kAllTransferExpandBits = {"|".join(all_transfer_expansion)};\n')

        out.append(f'''
// Fake stages and accesses for acquire present support
static const VkPipelineStageFlagBits2 VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL = 0x{(1 << self.pipelineStagePresentEngine.value):016X}ULL;
static const VkAccessFlagBits2 VK_ACCESS_2_PRESENT_ACQUIRE_READ_BIT_SYNCVAL = 0x{(1 << self.accessAcquireRead.value):016X}ULL;
static const VkAccessFlagBits2 VK_ACCESS_2_PRESENT_PRESENTED_BIT_SYNCVAL = 0x{(1 << self.accessPresented.value):016X}ULL;
''')

        out.append('// Unique number for each  stage/access combination\n')
        out.append('enum SyncAccessIndex {\n')
        for access in self.stageAccessCombo:
            out.append(f'    {access["stage_access"]} = {access["index"]},\n')
        out.append('};\n')

        out.append('\n')

        syncStageAccessFlagsSize = 192
        out.append(f'using SyncAccessFlags = std::bitset<{syncStageAccessFlagsSize}>;\n')
        out.append('// Unique bit for each stage/access combination\n')
        for access in [x for x in self.stageAccessCombo if x['access_bit'] is not None]:
            out.append(f'static const SyncAccessFlags {access["access_bit"]} = (SyncAccessFlags(1) << {access["stage_access"]});\n')

        if len(self.stageAccessCombo) > syncStageAccessFlagsSize:
            print("The bitset is too small, errors will occur, need to increase syncStageAccessFlagsSize\n")
            sys.exit(1)

        out.append(f'''
struct SyncAccessInfo {{
    const char *name;
    VkPipelineStageFlagBits2 stage_mask;
    VkAccessFlagBits2 access_mask;
    SyncAccessIndex access_index;
    SyncAccessFlags access_bit;
}};

// Array of text names and component masks for each stage/access index
const std::array<SyncAccessInfo, {len(self.stageAccessCombo)}>& syncAccessInfoByAccessIndex();

''')

        out.append('// Constants defining the mask of all read and write access states\n')
        out.append('static const SyncAccessFlags syncAccessReadMask = ( //  Mask of all read accesses\n')
        read_list = [x['access_bit'] for x in self.stageAccessCombo if x['is_read'] is not None and x['is_read'] == 'true']
        out.append('    ')
        out.append(' |\n    '.join(read_list))
        out.append('\n);')
        out.append('\n\n')

        out.append('static const SyncAccessFlags syncAccessWriteMask = ( //  Mask of all write accesses\n')
        write_list = [x['access_bit'] for x in self.stageAccessCombo if x['is_read'] is not None and x['is_read'] != 'true']
        out.append('    ')
        out.append(' |\n    '.join(write_list))
        out.append('\n);\n')

        out.append('''
// Bit order mask of accesses for each stage. Order matters, don't try to use vvl::unordered_map
const std::map<VkPipelineStageFlagBits2, SyncAccessFlags>& syncAccessMaskByStageBit();

// Bit order mask of accesses for each VkAccess. Order matters, don't try to use vvl::unordered_map
const std::map<VkAccessFlagBits2, SyncAccessFlags>& syncAccessMaskByAccessBit();

// Direct VkPipelineStageFlags to valid VkAccessFlags lookup table
const vvl::unordered_map<VkPipelineStageFlagBits2, VkAccessFlags2>& syncDirectStageToAccessMask();

// Pipeline stages corresponding to VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT for each VkQueueFlagBits
const vvl::unordered_map<VkQueueFlagBits, VkPipelineStageFlags2>& syncAllCommandStagesByQueueFlags();

// Masks of logically earlier stage flags for a given stage flag
const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2>& syncLogicallyEarlierStages();

// Masks of logically later stage flags for a given stage flag
const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2>& syncLogicallyLaterStages();
''')

        out.append('// clang-format on\n')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "sync_validation_types.h"
            ''')

        # syncAccessInfoByAccessIndex
        out.append('// clang-format off\n')
        out.append(f'const std::array<SyncAccessInfo, {len(self.stageAccessCombo)}>& syncAccessInfoByAccessIndex() {{\n')
        out.append(f'static const std::array<SyncAccessInfo, {len(self.stageAccessCombo)}> variable = {{ {{\n')
        for stageAccess in self.stageAccessCombo:
            out.append(f'''    {{
        {stageAccess["stage_access_string"]},
        {stageAccess["stage"]},
        {stageAccess["access"]},
        {stageAccess["stage_access"]},
        {stageAccess["access_bit"] if stageAccess["access_bit"] is not None else "SyncAccessFlags(0)"}
    }},
''')
        out.append('}};\n')
        out.append('return variable;\n')
        out.append('}\n')

        # syncAccessMaskByStageBit
        out.append('const std::map<VkPipelineStageFlagBits2, SyncAccessFlags>& syncAccessMaskByStageBit() {\n')
        out.append('    static const std::map<VkPipelineStageFlagBits2, SyncAccessFlags> variable = {\n')
        stage_to_stageAccess = {}
        for stageAccess_info in self.stageAccessCombo:
            stage = stageAccess_info['stage']
            if stage == 'VK_PIPELINE_STAGE_2_NONE': continue
            stageAccess_bit = stageAccess_info['access_bit']
            stage_to_stageAccess[stage] = stage_to_stageAccess.get(stage, []) + [stageAccess_bit]
        stages_in_bit_order = sorted([x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags], key=lambda x: x.value)
        for flag in [x for x in stages_in_bit_order if x.name in stage_to_stageAccess]:
                out.append(f'    {{ {flag.name}, (\n        {separator.join(stage_to_stageAccess[flag.name])}\n    )}},\n')
        out.append('    };\n')
        out.append('    return variable;\n')
        out.append('}\n\n')

        # syncAccessMaskByAccessBit
        out.append('const std::map<VkAccessFlagBits2, SyncAccessFlags>& syncAccessMaskByAccessBit() {\n')
        out.append('    static const std::map<VkAccessFlagBits2, SyncAccessFlags> variable = {\n')
        access_to_stageAccess = {}
        for stageAccess_info in self.stageAccessCombo:
            access = stageAccess_info['access']
            if access == 'VK_ACCESS_2_NONE':
                continue
            stageAccess_bit = stageAccess_info['access_bit']
            access_to_stageAccess[access] = access_to_stageAccess.get(access, []) + [stageAccess_bit]

        accesses_in_bit_order = sorted([x for x in self.vk.bitmasks['VkAccessFlagBits2'].flags], key=lambda x: x.value)
        for flag in [x for x in accesses_in_bit_order if x.name in access_to_stageAccess]:
            out.append(f'    {{ {flag.name}, (\n        {separator.join(access_to_stageAccess[flag.name])}\n    )}},\n')
        out.append('    { VK_ACCESS_2_MEMORY_READ_BIT, (\n        syncAccessReadMask\n    )},\n')
        out.append('    { VK_ACCESS_2_MEMORY_WRITE_BIT, (\n        syncAccessWriteMask\n    )},\n')
        out.append('    };\n')
        out.append('    return variable;\n')
        out.append('}\n\n')

        # syncDirectStageToAccessMask
        out.append('const vvl::unordered_map<VkPipelineStageFlagBits2, VkAccessFlags2>& syncDirectStageToAccessMask() {\n')
        out.append('    static const vvl::unordered_map<VkPipelineStageFlagBits2, VkAccessFlags2> variable = {\n')
        stage_to_access = {}
        for stageAccess_info in self.stageAccessCombo:
            stage = stageAccess_info['stage']
            if stage == 'VK_PIPELINE_STAGE_2_NONE':
                continue
            stage_to_access[stage] = stage_to_access.get(stage, []) + [stageAccess_info['access']]

        stages_in_bit_order = sorted([x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags], key=lambda x: x.value)
        for flag in [x for x in stages_in_bit_order if x.name in stage_to_access]:
            out.append(f'    {{ {flag.name}, (\n        {separator.join(stage_to_access[flag.name])}\n    )}},\n')
        out.append('    };\n')
        out.append('    return variable;\n')
        out.append('}\n\n')

        # syncAllCommandStagesByQueueFlags
        out.append('const vvl::unordered_map<VkQueueFlagBits, VkPipelineStageFlags2>& syncAllCommandStagesByQueueFlags() {\n')
        out.append('    static const vvl::unordered_map<VkQueueFlagBits, VkPipelineStageFlags2> variable = {\n')
        ignoreQueueFlag = [
            'VK_PIPELINE_STAGE_2_NONE',
            'VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT',
            'VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT',
            'VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT',
            'VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT',
        ]
        for queue in [x for x in self.vk.queueBits.keys()]:
            stages = [x.flag.name for x in self.vk.syncStage if x.support.queues & queue and x.flag.name not in ignoreQueueFlag and x.equivalent.max]
            out.append(f'    {{ {self.vk.queueBits[queue]}, (\n        {separator.join(stages)}\n    )}},\n')
        out.append('    };\n')
        out.append('    return variable;\n')
        out.append('}\n\n')

        # syncLogicallyEarlierStages
        out.append('const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2>& syncLogicallyEarlierStages() {\n')
        out.append('    static const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2> variable = {\n')
        earlier_stages = {}
        earlier_stages['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'])
        earlier_stages['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT', 'VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
        for stages in self.pipelineStagesOrdered.values():
            for i in range(len(stages)):
                stage_order = stages[i]
                stage = stage_order['stage']
                if stage == 'VK_PIPELINE_STAGE_2_HOST_BIT':
                    continue
                if stage not in earlier_stages:
                    earlier_stages[stage] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'])
                    earlier_stages['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'].update([stage])

                if not stage_order['ordered']:
                    earlier_stages[stage] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'])
                else:
                    earlier_stages[stage].update([s['stage'] for s in stages[:i]])
        earlier_stages = { key:SortSetBasedOnOrder(values, self.logicallyOrderedStages) for key, values in earlier_stages.items() }
        for stage in self.stages:
            if stage in earlier_stages and len(earlier_stages[stage]) > 0:
                out.append(f'    {{ {stage}, (\n        {separator.join(earlier_stages[stage])}\n    )}},\n')
        out.append('    };\n')
        out.append('    return variable;\n')
        out.append('}\n\n')

        # syncLogicallyLaterStages
        out.append('const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2>& syncLogicallyLaterStages() {\n')
        out.append('    static const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2> variable = {\n')
        later_stages = {}
        later_stages['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
        later_stages['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT', 'VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
        for stages in self.pipelineStagesOrdered.values():
            for i in range(len(stages)):
                stage_order = stages[i]
                stage = stage_order['stage']
                if stage == 'VK_PIPELINE_STAGE_2_HOST_BIT':
                    continue
                if stage not in later_stages:
                    later_stages[stage] = set(['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
                    later_stages['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'].update([stage])

                if not stage_order['ordered']:
                    later_stages[stage] = set(['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
                else:
                    later_stages[stage].update([s['stage'] for s in stages[i+1:] if s['ordered']])
        later_stages = { key:SortSetBasedOnOrder(values, self.logicallyOrderedStages) for key, values in later_stages.items() }
        for stage in self.stages:
            if stage in later_stages and len(later_stages[stage]) > 0:
                out.append(f'    {{ {stage}, (\n        {separator.join(later_stages[stage])}\n    )}},\n')
        out.append('    };\n')
        out.append('    return variable;\n')
        out.append('}\n\n')
        out.append('// clang-format on\n')

        self.write("".join(out))

    # Gets all <syncpipeline>
    # TODO - Use the BaseGenerator's VulkanObject
    def genSyncPipeline(self, sync):
        BaseGenerator.genSyncPipeline(self, sync)
        name = sync.elem.get('name').replace(' ', '_')

        # special case for trasfer stage: expand it to primitive transfer operations
        if name == 'transfer':
            transferExpansion = [
                ('transfer copy', 'VK_PIPELINE_STAGE_2_COPY_BIT'),
                ('transfer_resolve', 'VK_PIPELINE_STAGE_2_RESOLVE_BIT'),
                ('transfer_blit', 'VK_PIPELINE_STAGE_2_BLIT_BIT'),
                ('transfer_clear', 'VK_PIPELINE_STAGE_2_CLEAR_BIT'),
                ('transfer_acceleration_structure_copy', 'VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR'),
            ]
            for transfer_pipeline_name, transfer_stage in transferExpansion:
                self.pipelineNames.append(transfer_pipeline_name)
                self.pipelineStages[transfer_pipeline_name] = []
                self.pipelineStagesOrdered[transfer_pipeline_name] = []
                for elem in sync.elem.findall('syncpipelinestage'):
                    stage = elem.text
                    if stage == 'VK_PIPELINE_STAGE_2_TRANSFER_BIT':
                        stage = transfer_stage
                    self.pipelineStages[transfer_pipeline_name].append(stage)
                    self.pipelineStagesOrdered[transfer_pipeline_name].append({'stage' : stage, 'ordered' : True, 'before' : None, 'after' : None })
        # regular case (non-expandable stages)
        else:
            self.pipelineNames.append(name)
            self.pipelineStages[name] = []
            self.pipelineStagesOrdered[name] = []
            before_list = []
            after_list = []
            unordered_list = []
            for elem in sync.elem.findall('syncpipelinestage'):
                stage = elem.text
                self.pipelineStages[name].append(stage)
                order = elem.get('order')
                before = elem.get('before')
                after = elem.get('after')
                stage_order = {'stage' : stage, 'ordered' : order != 'None', 'before' : None, 'after' : None }
                if before is not None:
                    stage_order['before'] = before
                    before_list.append(stage_order)
                elif after is not None:
                    stage_order['after'] = after
                    after_list.append(stage_order)
                elif order == 'None':
                    unordered_list.append(stage_order)
                else:
                    self.pipelineStagesOrdered[name].append(stage_order)

            # process ordering directives
            for stage_order in before_list:
                before_stage = stage_order['before']
                before_index = next((i for i, s in enumerate(self.pipelineStagesOrdered[name]) if s['stage'] == before_stage))
                self.pipelineStagesOrdered[name].insert(before_index, stage_order)

            for stage_order in after_list:
                after_stage = stage_order['after']
                after_index = next((i for i, s in enumerate(self.pipelineStagesOrdered[name]) if s['stage'] == after_stage))
                self.pipelineStagesOrdered[name].insert(after_index + 1, stage_order)

            for stage_order in unordered_list:
                self.pipelineStagesOrdered[name].append(stage_order)

    # Create the stage/access combination from the legal uses of access with stages
    def createStageAccessCombinations(self):
        index = 1
        enum_prefix = 'SYNC_'
        stage_accesses = []
        none_stage_access = enum_prefix + 'ACCESS_INDEX_NONE'
        stage_accesses.append({
                        'stage_access': none_stage_access,
                        'stage_access_string' : '"' + none_stage_access + '"',
                        'access_bit': None,
                        'index': 0,
                        'stage': 'VK_PIPELINE_STAGE_2_NONE',
                        'access': 'VK_ACCESS_2_NONE',
                        'is_read': None}) #tri-state logic hack...

        # < stage, [accesses] >
        stageToAccessMap = dict()
        stageToAccessMap['VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL'] = ['VK_ACCESS_2_PRESENT_ACQUIRE_READ_BIT_SYNCVAL', 'VK_ACCESS_2_PRESENT_PRESENTED_BIT_SYNCVAL']

        # In general, syncval algorithms work only with 'base' accesses
        # and skip aliases/multi-accesses, or expands multi-accesses when necessary
        # Create a subset
        for access in [x for x in self.vk.syncAccess if x.equivalent.max and not x.support.max]:
            for flag in access.support.stages:
                if flag.name not in stageToAccessMap:
                    stageToAccessMap[flag.name] = []
                stageToAccessMap[flag.name].append(access.flag.name)

        # ACCELERATION_STRUCTURE_BUILD accesses input geometry buffers using SHADER_READ access.
        # The core of validation logic works with expanded stages and SHADER_STORAGE_READ was
        # choosen to represent SHADER_READ access. Then SHADER_READ itself is handled correctly:
        # validation logic automatically includes SHADER_READ when its sub-access is used.
        #
        # This new stage-access pair should be added manually because vk.xml does not define
        # ACCELERATION_STRUCTURE_BUILD for SHADER_READ's sub-accesses.
        #
        # TODO: with this change SHADER_STORAGE_READ also passes validation (in addition to SHADER_READ),
        # but other sub-accesses are not allowed. Update this logic if there is a confirmation
        # how ACCELERATION_STRUCTURE_BUILD should work with sub-components of SHADER_READ:
        # https://gitlab.khronos.org/vulkan/vulkan/-/issues/3640#note_434212
        stageToAccessMap['VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR'].append('VK_ACCESS_2_SHADER_STORAGE_READ_BIT')
        # Micro map, like AS Build, is a non *_STAGE_BIT that can have SHADER_READ
        if 'VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT' in stageToAccessMap:
            stageToAccessMap['VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT'].append('VK_ACCESS_2_SHADER_STORAGE_READ_BIT')

        for stage in [x for x in self.stages if x in stageToAccessMap]:
            mini_stage = stage.lstrip()
            if mini_stage.startswith(enum_prefix):
                mini_stage = mini_stage.replace(enum_prefix,'')
            else:
                mini_stage = mini_stage.replace('VK_PIPELINE_STAGE_2_', '')
            mini_stage = mini_stage.replace('_BIT_KHR', '')
            mini_stage = mini_stage.replace('_BIT', '')

            # Because access_stage_table's elements order might be different sometimes.
            # It causes the generator creates different code. It needs to be sorted.
            stageToAccessMap[stage].sort()
            for access in stageToAccessMap[stage]:
                mini_access = access.replace('VK_ACCESS_2_', '').replace('_BIT_KHR', '')
                mini_access = mini_access.replace('_BIT', '')
                stage_access = '_'.join((mini_stage,mini_access))
                stage_access = enum_prefix + stage_access
                access_bit = BitSuffixed(stage_access)
                is_read = stage_access.endswith('_READ') or ( '_READ_' in stage_access)
                stage_accesses.append({
                        'stage_access': stage_access,
                        'stage_access_string' : '"' + stage_access + '"',
                        'access_bit': access_bit,
                        'index': index,
                        'stage': stage,
                        'access': access,
                        'is_read': 'true' if is_read else 'false' })
                index += 1

        # Add synthetic stage/access
        synth_stage_access = [ 'IMAGE_LAYOUT_TRANSITION', 'QUEUE_FAMILY_OWNERSHIP_TRANSFER']
        stage = 'VK_PIPELINE_STAGE_2_NONE'
        access = 'VK_ACCESS_2_NONE'

        for synth in synth_stage_access :
            stage_access = enum_prefix + synth
            access_bit = BitSuffixed(stage_access)
            is_read = False # both ILT and QFO are R/W operations
            stage_accesses.append({
                        'stage_access': stage_access,
                        'stage_access_string' : '"' + stage_access + '"',
                        'access_bit': access_bit,
                        'index': index,
                        'stage': stage,
                        'access': access,
                        'is_read': 'true' if is_read else 'false' })
            index += 1

        return stage_accesses
