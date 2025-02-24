#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
# Copyright (c) 2023-2024 RasterGrid Kft.
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
#
# This script generates the dispatch portion of a factory layer which intercepts
# all Vulkan  functions. The resultant factory layer allows rapid development of
# layers and interceptors.

import os
from generators.vulkan_object import Command
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:
    # Generates source code for InitObjectDispatchVector
    @staticmethod
    def genInitObjectDispatchVectorSource(targetApiName: str) -> str:
        match targetApiName:

            # Vulkan specific InitObjectDispatchVector
            case 'vulkan':
                return '''
// Include layer validation object definitions
#include "generated/dispatch_vector.h"
#include "chassis/dispatch_object.h"
#include "thread_tracker/thread_safety_validation.h"
#include "stateless/stateless_validation.h"
#include "object_tracker/object_lifetime_validation.h"
#include "core_checks/core_validation.h"
#include "best_practices/best_practices_validation.h"
#include "gpuav/core/gpuav.h"
#include "sync/sync_validation.h"

namespace vvl {
namespace dispatch {

void Device::InitObjectDispatchVectors() {

#define BUILD_DISPATCH_VECTOR(name) \\
    init_object_dispatch_vector(InterceptId ## name, \\
                                typeid(&vvl::base::Device::name), \\
                                typeid(&threadsafety::Device::name), \\
                                typeid(&stateless::Device::name), \\
                                typeid(&object_lifetimes::Device::name), \\
                                typeid(&CoreChecks::name), \\
                                typeid(&BestPractices::name), \\
                                typeid(&gpuav::Validator::name), \\
                                typeid(&SyncValidator::name));

    auto init_object_dispatch_vector = [this](InterceptId id,
                                              const std::type_info& vo_typeid,
                                              const std::type_info& tt_typeid,
                                              const std::type_info& tpv_typeid,
                                              const std::type_info& tot_typeid,
                                              const std::type_info& tcv_typeid,
                                              const std::type_info& tbp_typeid,
                                              const std::type_info& tga_typeid,
                                              const std::type_info& tsv_typeid) {
        for (auto& vo: this->object_dispatch) {
            auto *item = vo.get();
            auto intercept_vector = &this->intercept_vectors[id];
            switch (item->container_type) {
            case LayerObjectTypeThreading:
                if (tt_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeParameterValidation:
                if (tpv_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeObjectTracker:
                if (tot_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeCoreValidation:
                if (tcv_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeBestPractices:
                if (tbp_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeGpuAssisted:
                if (tga_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeSyncValidation:
                if (tsv_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            default:
                /* Chassis codegen needs to be updated for unknown validation object type */
                assert(0);
            }
        }
    };

    intercept_vectors.resize(InterceptIdCount);
'''

class DispatchVectorGenerator(BaseGenerator):
    # will skip all 3 functions
    skip_intercept_id_functions = (
        'vkGetDeviceProcAddr',
        'vkDestroyDevice',
        'vkCreateValidationCacheEXT',
        'vkDestroyValidationCacheEXT',
        'vkMergeValidationCachesEXT',
        'vkGetValidationCacheDataEXT',
        # have all 3 calls have dual signatures being used
        'vkCreateShaderModule',
        'vkCreateShadersEXT',
        'vkCreateGraphicsPipelines',
        'vkCreateComputePipelines',
        'vkCreateRayTracingPipelinesNV',
        'vkCreateRayTracingPipelinesKHR',
    )

    # We need to skip any signatures that pass around chassis_modification_state structs
    # and therefore can't easily create the intercept id
    skip_intercept_id_pre_validate = (
        'vkAllocateDescriptorSets'
    )
    skip_intercept_id_pre_record = (
        'vkCreatePipelineLayout',
        'vkCreateBuffer',
    )
    skip_intercept_id_post_record = (
        'vkAllocateDescriptorSets'
    )

    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
            * Copyright (c) 2015-2024 Google Inc.
            * Copyright (c) 2023-2024 RasterGrid Kft.
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

        if self.filename == 'dispatch_vector.h':
            self.generateHeader()
        elif self.filename == 'dispatch_vector.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            // This source code creates dispatch vectors for each chassis api intercept,
            // i.e., PreCallValidateFoo, PreCallRecordFoo, PostCallRecordFoo, etc., ensuring that
            // each vector contains only the validation objects that override that particular base
            // class virtual function. Preventing non-overridden calls from reaching the default
            // functions saved about 5% in multithreaded applications.

            ''')

        out.append('typedef enum InterceptId{\n')
        for command in [x for x in self.vk.commands.values() if not x.instance and x.name not in self.skip_intercept_id_functions]:
            if command.name not in self.skip_intercept_id_pre_validate:
                out.append(f'    InterceptIdPreCallValidate{command.name[2:]},\n')
            if command.name not in self.skip_intercept_id_pre_record:
                out.append(f'    InterceptIdPreCallRecord{command.name[2:]},\n')
            if command.name not in self.skip_intercept_id_post_record:
                out.append(f'    InterceptIdPostCallRecord{command.name[2:]},\n')
        out.append('    InterceptIdCount,\n')
        out.append('} InterceptId;\n')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            // This source code creates dispatch vectors for each chassis api intercept,
            // i.e., PreCallValidateFoo, PreCallRecordFoo, PostCallRecordFoo, etc., ensuring that
            // each vector contains only the validation objects that override that particular base
            // class virtual function. Preventing non-overridden calls from reaching the default
            // functions saved about 5% in multithreaded applications.

            #include "generated/dispatch_vector.h"
            #include "chassis/dispatch_object.h"
            ''')

        out.append(APISpecific.genInitObjectDispatchVectorSource(self.targetApiName))

        guard_helper = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if not x.instance and x.name not in self.skip_intercept_id_functions]:
            out.extend(guard_helper.add_guard(command.protect))
            if command.name not in self.skip_intercept_id_pre_validate:
                out.append(f'    BUILD_DISPATCH_VECTOR(PreCallValidate{command.name[2:]});\n')
            if command.name not in self.skip_intercept_id_pre_record:
                out.append(f'    BUILD_DISPATCH_VECTOR(PreCallRecord{command.name[2:]});\n')
            if command.name not in self.skip_intercept_id_post_record:
                out.append(f'    BUILD_DISPATCH_VECTOR(PostCallRecord{command.name[2:]});\n')
        out.extend(guard_helper.add_guard(None))
        out.append('}\n')
        out.append('} // namespace dispatch\n')
        out.append('} // namespace vvl\n')
        self.write("".join(out))
