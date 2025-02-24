#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
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
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class FeatureRequirementsGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Features of the VkPhysicalDeviceBufferDeviceAddressFeaturesEXT have
        # the same name as Vulkan 1.2 and
        # VkPhysicalDeviceBufferDeviceAddressFeaturesKHR features, but are
        # semantically different.  They are given a suffix to be distinguished.
        self.identical_but_different_features = {
            'VkPhysicalDeviceBufferDeviceAddressFeaturesEXT',
            'VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV',
        }

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023-2025 The Khronos Group Inc.
            * Copyright (c) 2023-2025 Valve Corporation
            * Copyright (c) 2023-2025 LunarG, Inc.
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
            ****************************************************************************/\n\n''')
        out.append('// NOLINTBEGIN\n\n') # Wrap for clang-tidy to ignore

        if self.filename == 'feature_requirements_helper.h':
            out.append(self.generateHeader())
        elif self.filename == 'feature_requirements_helper.cpp':
            out.append(self.generateSource())
        else:
            out.append(f'\nFile name {self.filename} has no code to generate\n')

        out.append('\n// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))

    def getFeaturesAndOrigins(self) -> dict:
        # Get all Vulkan Physical Device Features
        featureMap = dict()
        feature_structs = self.vk.structs['VkPhysicalDeviceFeatures2'].extendedBy
        feature_structs.append('VkPhysicalDeviceFeatures')
        for extending_struct_name in feature_structs:
            if extending_struct_name in self.identical_but_different_features:
                continue
            extending_struct = self.vk.structs[extending_struct_name]

            for feature in extending_struct.members:
                if feature.type == 'VkBool32':
                    if feature.name not in featureMap:
                        featureMap[feature.name] = set()
                    featureMap[feature.name].add(extending_struct_name)

        # Generate a comment for every feature regarding where it may be coming from, then sort the
        # features by that comment.  That ensures features of the same struct end up together.
        featuresAndOrigins = sorted([(sorted(structs), feature)
                                     for feature, structs in featureMap.items()])

        return featuresAndOrigins

    def generateHeader(self):
        out = []
        out.append('''
            #include "vk_api_version.h"

            #include <vulkan/vulkan.h>

            namespace vkt {
        ''')

        # Physical device features enum
        out.append('enum class Feature {\n')
        for origins, feature in self.getFeaturesAndOrigins():
            out.append(f'// {", ".join(origins)}\n')
            out.append(f'{feature},\n')
        out.append('};\n')

        # Functions declarations
        out.append('''
            struct FeatureAndName {
                VkBool32 *feature;
                const char *name;
            };

            // Find or add the correct VkPhysicalDeviceFeature struct in `pnext_chain` based on `feature`,
            // a vkt::Feature enum value, and set feature to VK_TRUE
            FeatureAndName AddFeature(APIVersion api_version, vkt::Feature feature, void **inout_pnext_chain);

            }// namespace vkt
        ''')
        return "".join(out)

    # Find the Vulkan version in a VkPhysicalDeviceVulkan<N>Features struct
    def getApiVersion(self, structs):
        if structs.find('11') != -1:
            return 'VK_API_VERSION_1_2'
        if structs.find('12') != -1:
            return 'VK_API_VERSION_1_2'
        if structs.find('13') != -1:
            return 'VK_API_VERSION_1_3'
        if structs.find('14') != -1:
            return 'VK_API_VERSION_1_4'
        else:
            assert False

    def generateSource(self):
        out = []
        out.append('#include "generated/feature_requirements_helper.h"\n\n')
        out.append('#include "generated/pnext_chain_extraction.h"\n\n')
        out.append('#include <vulkan/utility/vk_struct_helper.hpp>\n\n')
        out.append('namespace vkt {')

        # AddFeature
        guard_helper = PlatformGuardHelper()
        out.append('FeatureAndName AddFeature(APIVersion api_version, vkt::Feature feature, void **inout_pnext_chain) {\n')
        out.append('switch(feature) {\n')
        for origins, feature in self.getFeaturesAndOrigins():
            # Skip VkPhysicalDeviceFeatures, handled by hand
            if len(origins) == 1 and origins[0] == 'VkPhysicalDeviceFeatures':
                continue
            vulkan_feature_struct_i = -1
            for i,s in enumerate(origins):
                if s.find('Vulkan') != -1:
                    vulkan_feature_struct_i = i
                    break
            if len(origins) == 1 or vulkan_feature_struct_i == -1:
                    feature_struct_name = origins[0]
                    out.extend(guard_helper.add_guard(self.vk.structs[feature_struct_name].protect))
                    out.append(f'''
                        case Feature::{feature}: {{
                        auto vk_struct = const_cast<{feature_struct_name} *>(vku::FindStructInPNextChain<{feature_struct_name}>(*inout_pnext_chain));
                        if (!vk_struct) {{
                            vk_struct = new {feature_struct_name};
                            *vk_struct = vku::InitStructHelper();
                            if (*inout_pnext_chain) {{
                                vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                            }} else {{
                                *inout_pnext_chain = vk_struct;
                            }}
                        }}
                        return {{&vk_struct->{feature}, "{feature_struct_name}::{feature}"}};
                        }}
                        ''')
                    out.extend(guard_helper.add_guard(None))
            else:
                assert len(origins) == 2
                api_struct_name = origins[vulkan_feature_struct_i]
                feature_struct_name = origins[1 - vulkan_feature_struct_i]
                ref_api_version = self.getApiVersion(api_struct_name)
                out.extend(guard_helper.add_guard(self.vk.structs[feature_struct_name].protect))
                out.append(f'''
                    case Feature::{feature}:
                        if (api_version >= {ref_api_version}) {{
                            auto vk_struct = const_cast<{api_struct_name} *>(vku::FindStructInPNextChain<{api_struct_name}>(*inout_pnext_chain));
                            if (!vk_struct) {{
                                vk_struct = new {api_struct_name};
                                *vk_struct = vku::InitStructHelper();
                                if (*inout_pnext_chain) {{
                                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                                }} else {{
                                    *inout_pnext_chain = vk_struct;
                                }}
                            }}
                            return {{&vk_struct->{feature}, "{api_struct_name}::{feature}"}};
                        }} else {{
                            auto vk_struct = const_cast<{feature_struct_name} *>(vku::FindStructInPNextChain<{feature_struct_name}>(*inout_pnext_chain));
                            if (!vk_struct) {{
                                vk_struct = new {feature_struct_name};
                                *vk_struct = vku::InitStructHelper();
                                if (*inout_pnext_chain) {{
                                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                                }} else {{
                                    *inout_pnext_chain = vk_struct;
                                }}
                            }}
                            return {{&vk_struct->{feature}, "{feature_struct_name}::{feature}"}};
                        }}''')
                out.extend(guard_helper.add_guard(None))

        out.append('''default:
            assert(false);
            return {nullptr, ""};
            }''')

        out.append('}\n')

        out.append('}// namespace vkt')

        return "".join(out)
