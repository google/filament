#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
# Copyright (c) 2023-2025 RasterGrid Kft.
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
import re
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

# Need pyparsing because the Vulkan-Headers use it in dependencyBNF
from pyparsing import ParseResults
# From the Vulkan-Headers
from parse_dependency import dependencyBNF

def parseExpr(expr): return dependencyBNF().parseString(expr, parseAll=True)

def dependCheck(pr: ParseResults, token, op, start_group, end_group) -> None:
    """
    Run a set of callbacks on a boolean expression.

    token: run on a non-operator, non-parenthetical token
    op: run on an operator token
    start_group: run on a '(' token
    end_group: run on a ')' token
    """

    for r in pr:
        if isinstance(r, ParseResults):
            start_group()
            dependCheck(r, token, op, start_group, end_group)
            end_group()
        elif r in ',+':
            op(r)
        else:
            token(r)

def exprValues(pr: ParseResults) -> list:
    """
    Return a list of all "values" (i.e., non-operators) in the parsed expression.
    """

    values = []
    dependCheck(pr, lambda x: values.append(x), lambda x: None, lambda: None, lambda: None)
    return values

def exprToCpp(pr: ParseResults, opt = lambda x: x) -> str:
    r = []
    printExt = lambda x: r.append(opt(x))
    printOp = lambda x: r.append(' && ' if x == '+' else ' || ')
    openParen = lambda: r.append('(')
    closeParen = lambda: r.append(')')
    dependCheck(pr, printExt, printOp, openParen, closeParen)
    return ''.join(r)


class ExtensionHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)
        # [ Feature name | name in struct InstanceExtensions ]
        self.fieldName = dict()
        # [ Extension name : List[Extension | Version] ]
        self.requiredExpression = dict()

    def generate(self):
        for extension in self.vk.extensions.values():
            self.fieldName[extension.name] = extension.name.lower()
            self.requiredExpression[extension.name] = list()
            if extension.depends is not None:
                # This is a work around for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5372
                temp = re.sub(r',VK_VERSION_1_\d+', '', extension.depends)
                # It can look like (VK_KHR_timeline_semaphore,VK_VERSION_1_2) or (VK_VERSION_1_2,VK_KHR_timeline_semaphore)
                temp = re.sub(r'VK_VERSION_1_\d+,', '', temp)
                for reqs in exprValues(parseExpr(temp)):
                    feature = self.vk.extensions[reqs] if reqs in self.vk.extensions else self.vk.versions[reqs]
                    self.requiredExpression[extension.name].append(feature)
        for version in self.vk.versions.keys():
            self.fieldName[version] = version.lower().replace('version', 'feature_version')

        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
            * Copyright (c) 2015-2025 Google Inc.
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

        if self.filename == 'vk_extension_helper.h':
            self.generateHeader()
        elif self.filename == 'vk_extension_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        guard_helper = PlatformGuardHelper()
        out = []
        out.append('''
            #pragma once

            #include <string>
            #include <utility>
            #include <vector>
            #include <cassert>

            #include <vulkan/vulkan.h>
            #include "containers/custom_containers.h"
            #include "generated/vk_api_version.h"
            #include "generated/error_location_helper.h"

            // Extensions (unlike functions, struct, etc) are passed in as strings.
            // The goal is to turn the string to a enum and pass that around the layers.
            // Only when we need to print an error do we try and turn it back into a string
            vvl::Extension GetExtension(std::string extension);

            enum ExtEnabled : unsigned char {
                kNotEnabled,
                kEnabledByCreateinfo,  // Extension is passed at vkCreateDevice/vkCreateInstance time
                kEnabledByApiLevel,  // the API version implicitly enabled it
                kEnabledByInteraction,  // is implicity enabled by anthoer extension
            };

            // Map of promoted extension information per version (a separate map exists for instance and device extensions).
            // The map is keyed by the version number (e.g. VK_API_VERSION_1_1) and each value is a pair consisting of the
            // version string (e.g. "VK_VERSION_1_1") and the set of name of the promoted extensions.
            typedef vvl::unordered_map<uint32_t, std::pair<const char*, vvl::unordered_set<vvl::Extension>>> PromotedExtensionInfoMap;
            const PromotedExtensionInfoMap& GetInstancePromotionInfoMap();
            const PromotedExtensionInfoMap& GetDevicePromotionInfoMap();

            /*
            This function is a helper to know if the extension is enabled.

            Times to use it
            - To determine the VUID
            - The VU mentions the use of the extension
            - Extension exposes property limits being validated
            - Checking not enabled
                - if (!IsExtEnabled(...)) { }
            - Special extensions that being EXPOSED alters the VUs
                - IsExtEnabled(extensions.vk_khr_portability_subset)
            - Special extensions that alter behaviour of enabled
                - IsExtEnabled(extensions.vk_khr_maintenance*)

            Times to NOT use it
                - If checking if a struct or enum is being used. There are a stateless checks
                  to make sure the new Structs/Enums are not being used without this enabled.
                - If checking if the extension's feature enable status, because if the feature
                  is enabled, then we already validated that extension is enabled.
                - Some variables (ex. viewMask) require the extension to be used if non-zero
            */
            [[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) { return (extension != kNotEnabled); }

            [[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) { return (extension == kEnabledByCreateinfo); }
            ''')

        out.append('\nstruct InstanceExtensions {\n')
        out.append('    APIVersion api_version{};\n')
        for version in self.vk.versions.keys():
            out.append(f'    ExtEnabled {self.fieldName[version]}{{kNotEnabled}};\n')

        out.extend([f'    ExtEnabled {ext.name.lower()}{{kNotEnabled}};\n' for ext in self.vk.extensions.values() if ext.instance])

        out.append('''
            struct Requirement {
                const ExtEnabled InstanceExtensions::*enabled;
                const char *name;
            };
            typedef std::vector<Requirement> RequirementVec;
            struct Info {
                Info(ExtEnabled InstanceExtensions::*state_, const RequirementVec requirements_)
                    : state(state_), requirements(requirements_) {}
                ExtEnabled InstanceExtensions::*state;
                RequirementVec requirements;
            };

            typedef vvl::unordered_map<vvl::Extension, Info> InfoMap;
            static const InfoMap &GetInfoMap() {
                static const InfoMap info_map = {
            ''')
        for extension in [x for x in self.vk.extensions.values() if x.instance]:
            out.extend(guard_helper.add_guard(extension.protect))
            reqs = ''
            # This is only done to match whitespace from before code we refactored
            if self.requiredExpression[extension.name]:
                reqs += '{\n'
                reqs += ',\n'.join([f'{{&InstanceExtensions::{self.fieldName[feature.name]}, {feature.nameString}}}' for feature in self.requiredExpression[extension.name]])
                reqs += '}'
            out.append(f'{{vvl::Extension::_{extension.name}, Info(&InstanceExtensions::{extension.name.lower()}, {{{reqs}}})}},\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
                };
                return info_map;
            }

            static const Info &GetInfo(vvl::Extension extension_name) {
                static const Info empty_info{nullptr, RequirementVec()};
                const auto &ext_map = InstanceExtensions::GetInfoMap();
                const auto info = ext_map.find(extension_name);
                return (info != ext_map.cend()) ? info->second : empty_info;
            }

            InstanceExtensions() = default;
            InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo *pCreateInfo);

            };
            ''')

        out.append('\nstruct DeviceExtensions : public InstanceExtensions {\n')
        for version in self.vk.versions.keys():
            out.append(f'    ExtEnabled {self.fieldName[version]}{{kNotEnabled}};\n')

        out.extend([f'    ExtEnabled {ext.name.lower()}{{kNotEnabled}};\n' for ext in self.vk.extensions.values() if ext.device])

        out.append('''
            struct Requirement {
                const ExtEnabled DeviceExtensions::*enabled;
                const char *name;
            };
            typedef std::vector<Requirement> RequirementVec;
            struct Info {
                Info(ExtEnabled DeviceExtensions::*state_, const RequirementVec requirements_)
                    : state(state_), requirements(requirements_) {}
                ExtEnabled DeviceExtensions::*state;
                RequirementVec requirements;
            };

            typedef vvl::unordered_map<vvl::Extension, Info> InfoMap;
            static const InfoMap &GetInfoMap() {
                static const InfoMap info_map = {
            ''')

        for extension in [x for x in self.vk.extensions.values() if x.device]:
            out.extend(guard_helper.add_guard(extension.protect))
            reqs = ''
            # This is only done to match whitespace from before code we refactored
            if self.requiredExpression[extension.name]:
                reqs += '{\n'
                reqs += ',\n'.join([f'{{&DeviceExtensions::{self.fieldName[feature.name]}, {feature.nameString}}}' for feature in self.requiredExpression[extension.name]])
                reqs += '}'
            out.append(f'{{vvl::Extension::_{extension.name}, Info(&DeviceExtensions::{extension.name.lower()}, {{{reqs}}})}},\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
                };

                return info_map;
            }

            static const Info &GetInfo(vvl::Extension extension_name) {
                static const Info empty_info{nullptr, RequirementVec()};
                const auto &ext_map = DeviceExtensions::GetInfoMap();
                const auto info = ext_map.find(extension_name);
                return (info != ext_map.cend()) ? info->second : empty_info;
            }

            DeviceExtensions() = default;
            DeviceExtensions(const InstanceExtensions &instance_ext) : InstanceExtensions(instance_ext) {}

            DeviceExtensions(const InstanceExtensions &instance_extensions, APIVersion requested_api_version,
                                                const VkDeviceCreateInfo *pCreateInfo = nullptr);
            DeviceExtensions(const InstanceExtensions &instance_ext, APIVersion requested_api_version, const std::vector<VkExtensionProperties> &props);
            };

            const InstanceExtensions::Info &GetInstanceVersionMap(const char* version);
            const DeviceExtensions::Info &GetDeviceVersionMap(const char* version);

            ''')

        out.append('''
            constexpr bool IsInstanceExtension(vvl::Extension extension) {
                switch (extension) {
            ''')
        out.extend([f'case vvl::Extension::_{x.name}:\n' for x in self.vk.extensions.values() if x.instance])
        out.append('''    return true;''')
        out.append('''default: return false;
            }
        }\n''')

        out.append('''
            constexpr bool IsDeviceExtension(vvl::Extension extension) {
                switch (extension) {
            ''')
        out.extend([f'case vvl::Extension::_{x.name}:\n' for x in self.vk.extensions.values() if x.device])
        out.append('''    return true;''')
        out.append('''default: return false;
            }
        }\n''')

        self.write(''.join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "vk_extension_helper.h"

            vvl::Extension GetExtension(std::string extension) {
                static const vvl::unordered_map<std::string, vvl::Extension> extension_map {
            ''')
        for extension in self.vk.extensions.values():
            out.append(f'    {{"{extension.name}", vvl::Extension::_{extension.name}}},\n')
        out.append('''    };
                const auto it = extension_map.find(extension);
                return (it == extension_map.end()) ? vvl::Extension::Empty : it->second;
            }

            const PromotedExtensionInfoMap &GetInstancePromotionInfoMap() {
                static const PromotedExtensionInfoMap promoted_map = {
            ''')

        for version in self.vk.versions.keys():
            promoted_ext_list = [x for x in self.vk.extensions.values() if x.promotedTo == version and getattr(x, 'instance')]
            if len(promoted_ext_list) > 0:
                out.append(f'{{{version.replace("VERSION", "API_VERSION")},{{"{version}",{{')
                out.extend(['    %s,\n' % f'vvl::Extension::_{ext.name}' for ext in promoted_ext_list])
                out.append('}}},\n')

        out.append('''
                };
                return promoted_map;
            }

            const PromotedExtensionInfoMap &GetDevicePromotionInfoMap() {
                static const PromotedExtensionInfoMap promoted_map = {
            ''')

        for version in self.vk.versions.keys():
            promoted_ext_list = [x for x in self.vk.extensions.values() if x.promotedTo == version and getattr(x, 'device')]
            if len(promoted_ext_list) > 0:
                out.append(f'{{{version.replace("VERSION", "API_VERSION")},{{"{version}",{{')
                out.extend(['    %s,\n' % f'vvl::Extension::_{ext.name}' for ext in promoted_ext_list])
                out.append('}}},\n')

        out.append('''
                };
                return promoted_map;
            }

            const InstanceExtensions::Info &GetInstanceVersionMap(const char* version) {
                static const InstanceExtensions::Info empty_info{nullptr, InstanceExtensions::RequirementVec()};
                static const vvl::unordered_map<std::string_view, InstanceExtensions::Info> version_map = {
            ''')
        for version in self.vk.versions.keys():
            out.append(f'{{"{version}", InstanceExtensions::Info(&InstanceExtensions::{self.fieldName[version]}, {{}})}},\n')
        out.append('''};
                const auto info = version_map.find(version);
                return (info != version_map.cend()) ? info->second : empty_info;
            }

            const DeviceExtensions::Info &GetDeviceVersionMap(const char* version) {
                static const DeviceExtensions::Info empty_info{nullptr, DeviceExtensions::RequirementVec()};
                static const vvl::unordered_map<std::string_view, DeviceExtensions::Info> version_map = {
            ''')
        for version in self.vk.versions.keys():
            out.append(f'{{"{version}", DeviceExtensions::Info(&DeviceExtensions::{self.fieldName[version]}, {{}})}},\n')
        out.append('''};
                const auto info = version_map.find(version);
                return (info != version_map.cend()) ? info->second : empty_info;
            }

            InstanceExtensions::InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo* pCreateInfo) {
                // Initialize struct data, robust to invalid pCreateInfo
                api_version = NormalizeApiVersion(requested_api_version);
                if (!api_version.Valid()) return;

                const auto promotion_info_map = GetInstancePromotionInfoMap();
                for (const auto& version_it : promotion_info_map) {
                    auto info = GetInstanceVersionMap(version_it.second.first);
                    if (api_version >= version_it.first) {
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                        for (const auto& extension : version_it.second.second) {
                            info = GetInfo(extension);
                            assert(info.state);
                            if (info.state) this->*(info.state) = kEnabledByApiLevel;
                        }
                    }
                }

                // CreateInfo takes precedence over promoted
                if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
                    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                        if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
                        auto info = GetInfo(extension);
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                    }
                }
            }

            DeviceExtensions::DeviceExtensions(const InstanceExtensions& instance_ext,
                                               APIVersion requested_api_version,
                                               const VkDeviceCreateInfo* pCreateInfo)
                : InstanceExtensions(instance_ext) {

                auto api_version = NormalizeApiVersion(requested_api_version);
                if (!api_version.Valid()) return;

                const auto promotion_info_map = GetDevicePromotionInfoMap();
                for (const auto& version_it : promotion_info_map) {
                    auto info = GetDeviceVersionMap(version_it.second.first);
                    if (api_version >= version_it.first) {
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                        for (const auto& extension : version_it.second.second) {
                            info = GetInfo(extension);
                            assert(info.state);
                            if (info.state) this->*(info.state) = kEnabledByApiLevel;
                        }
                    }
                }

                // CreateInfo takes precedence over promoted
                if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
                    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                        if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
                        auto info = GetInfo(extension);
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                    }
                }

                // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
                // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
                {
                    constexpr std::array shader_object_interactions = {
                        vvl::Extension::_VK_EXT_extended_dynamic_state,
                        vvl::Extension::_VK_EXT_extended_dynamic_state2,
                        vvl::Extension::_VK_EXT_extended_dynamic_state3,
                        vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
                    };
                    auto info = GetInfo(vvl::Extension::_VK_EXT_shader_object);
                    if (info.state) {
                        if (this->*(info.state) != kNotEnabled) {
                            for (auto interaction_ext : shader_object_interactions) {
                                info = GetInfo(interaction_ext);
                                assert(info.state);
                                if (this->*(info.state) != kEnabledByCreateinfo) {
                                    this->*(info.state) = kEnabledByInteraction;
                                }
                            }
                        }
                    }
                }
            }

            DeviceExtensions::DeviceExtensions(const InstanceExtensions& instance_ext,
                                               APIVersion requested_api_version,
                                               const std::vector<VkExtensionProperties> &props)
                : InstanceExtensions(instance_ext) {

                auto api_version = NormalizeApiVersion(requested_api_version);
                if (!api_version.Valid()) return;

                const auto promotion_info_map = GetDevicePromotionInfoMap();
                for (const auto& version_it : promotion_info_map) {
                    auto info = GetDeviceVersionMap(version_it.second.first);
                    if (api_version >= version_it.first) {
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                        for (const auto& extension : version_it.second.second) {
                            info = GetInfo(extension);
                            assert(info.state);
                            if (info.state) this->*(info.state) = kEnabledByApiLevel;
                        }
                    }
                }
                for (const auto &prop : props) {
                    vvl::Extension extension = GetExtension(prop.extensionName);
                    auto info = GetInfo(extension);
                    if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                }

                // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
                // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
                {
                    constexpr std::array shader_object_interactions = {
                        vvl::Extension::_VK_EXT_extended_dynamic_state,
                        vvl::Extension::_VK_EXT_extended_dynamic_state2,
                        vvl::Extension::_VK_EXT_extended_dynamic_state3,
                        vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
                    };
                    auto info = GetInfo(vvl::Extension::_VK_EXT_shader_object);
                    if (info.state) {
                        if (this->*(info.state) != kNotEnabled) {
                            for (auto interaction_ext : shader_object_interactions) {
                                info = GetInfo(interaction_ext);
                                assert(info.state);
                                if (this->*(info.state) != kEnabledByCreateinfo) {
                                    this->*(info.state) = kEnabledByInteraction;
                                }
                            }
                        }
                    }
                }
            }
    ''')

        self.write(''.join(out))
