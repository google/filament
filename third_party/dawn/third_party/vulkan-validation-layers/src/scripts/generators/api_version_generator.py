#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2024 The Khronos Group Inc.
# Copyright (c) 2015-2024 Valve Corporation
# Copyright (c) 2015-2024 LunarG, Inc.
# Copyright (c) 2015-2024 Google Inc.
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

import os
from generators.base_generator import BaseGenerator

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:
    @staticmethod
    def genAPIVersionEnum(targetApiName: str) -> str:
        match targetApiName:
            case 'vulkan':
                return '''
                    _VK_VERSION_1_0 = (int)VK_API_VERSION_1_0,
                    '''

    @staticmethod
    def genAPIVersionSource(targetApiName: str) -> str:
        match targetApiName:

            # Vulkan specific APIVersion class and related utilities
            case 'vulkan':
                return '''
                    class APIVersion {
                    public:
                        APIVersion() : api_version_(VVL_UNRECOGNIZED_API_VERSION) {}
                        APIVersion(uint32_t api_version) : api_version_(api_version) {}
                        APIVersion& operator=(uint32_t api_version) {
                            api_version_ = api_version;
                            return *this;
                        }
                        bool Valid() const { return api_version_ != VVL_UNRECOGNIZED_API_VERSION; }
                        uint32_t Value() const { return api_version_; }
                        uint32_t Major() const { return VK_API_VERSION_MAJOR(api_version_); }
                        uint32_t Minor() const { return VK_API_VERSION_MINOR(api_version_); }
                        uint32_t Patch() const { return VK_API_VERSION_PATCH(api_version_); }
                        bool operator<(APIVersion api_version) const { return api_version_ < api_version.api_version_; }
                        bool operator<=(APIVersion api_version) const { return api_version_ <= api_version.api_version_; }
                        bool operator>(APIVersion api_version) const { return api_version_ > api_version.api_version_; }
                        bool operator>=(APIVersion api_version) const { return api_version_ >= api_version.api_version_; }
                        bool operator==(APIVersion api_version) const { return api_version_ == api_version.api_version_; }
                        bool operator!=(APIVersion api_version) const { return api_version_ != api_version.api_version_; }
                    private:
                        uint32_t api_version_;
                    };

                    static inline APIVersion NormalizeApiVersion(APIVersion specified_version) {
                        if (specified_version < VK_API_VERSION_1_1)
                            return VK_API_VERSION_1_0;
                        else if (specified_version < VK_API_VERSION_1_2)
                            return VK_API_VERSION_1_1;
                        else if (specified_version < VK_API_VERSION_1_3)
                            return VK_API_VERSION_1_2;
                        else if (specified_version < VK_API_VERSION_1_4)
                            return VK_API_VERSION_1_3;
                        else
                            return VK_API_VERSION_1_4;
                                        }
                    '''

class ApiVersionOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2024 The Khronos Group Inc.
            * Copyright (c) 2015-2024 Valve Corporation
            * Copyright (c) 2015-2024 LunarG, Inc.
            * Copyright (c) 2015-2024 Google Inc.
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
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        out.append('''
            #pragma once
            #include <vulkan/vulkan.h>
            #include <sstream>
            #include <iomanip>


            #define VVL_UNRECOGNIZED_API_VERSION 0xFFFFFFFF

            namespace vvl {
            // Need underscore prefix to not conflict with namespace, but still easy to match generation
            enum class Version {
                Empty = 0,''')
        out.append(APISpecific.genAPIVersionEnum(self.targetApiName))
        for version in self.vk.versions.values():
            out.append(f'    _{version.name} = (int){version.nameApi},')
        out.append('};\n')
        out.append('}  // namespace vvl\n')

        out.append(APISpecific.genAPIVersionSource(self.targetApiName))

        out.append('''
            // Convert integer API version to a string
            static inline std::string StringAPIVersion(APIVersion version) {
                std::stringstream version_name;
                if (!version.Valid()) {
                    return "<unrecognized>";
                }
                version_name << version.Major() << "." << version.Minor() << "." << version.Patch() << " (0x" << std::setfill('0')
                            << std::setw(8) << std::hex << version.Value() << ")";
                return version_name.str();
            }
            ''')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write(''.join(out))
