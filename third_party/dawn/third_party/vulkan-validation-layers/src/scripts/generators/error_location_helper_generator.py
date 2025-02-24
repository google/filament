#!/usr/bin/python3 -i
#
# Copyright (c) 2023-2025 The Khronos Group Inc.
# Copyright (c) 2023-2025 Valve Corporation
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

class ErrorLocationHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.fields = set()
        self.pointer_fields = set()

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023-2025 The Khronos Group Inc.
            * Copyright (c) 2023-2025 Valve Corporation
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
            ****************************************************************************/
            ''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        # Build set of all field names found in all structs and commands
        for command in [x for x in self.vk.commands.values() if not x.alias]:
            for param in command.params:
                self.fields.add(param.name)
                if param.pointer:
                    self.pointer_fields.add(param.name)
        for struct in self.vk.structs.values():
            for member in struct.members:
                self.fields.add(member.name)
                if member.pointer:
                    self.pointer_fields.add(member.name)

        # Pointers in spec start with 'p' except a few cases when dealing with external items (etc WSI).
        # These are names that are also not pointers and removing them in effort to keep the code
        # simpler and just have a few cases where we use a 'dot' instead of an 'arrow' for the error messages.
        self.pointer_fields.remove('buffer') # VkImportAndroidHardwareBufferInfoANDROID
        self.pointer_fields.remove('display') # VkWaylandSurfaceCreateInfoKHR
        self.pointer_fields.remove('window') # VkAndroidSurfaceCreateInfoKHR (and few others)
        self.pointer_fields.remove('surface') # VkDirectFBSurfaceCreateInfoEXT

        # Sort alphabetically
        self.fields = sorted(self.fields)
        self.pointer_fields = sorted(self.pointer_fields)

        if self.filename == 'error_location_helper.h':
            self.generateHeader()
        elif self.filename == 'error_location_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <vulkan/vulkan.h>
            #include "containers/custom_containers.h"
            #include "generated/vk_api_version.h"

            namespace vvl {
            enum class Func {
                Empty = 0,
            ''')
        # Want alpha-sort for ease of look at list while debugging
        for command in sorted(self.vk.commands.values()):
            out.append(f'    {command.name},\n')
        out.append('};\n')

        out.append('\n')
        out.append('enum class Struct {\n')
        out.append('    Empty = 0,\n')
        # Want alpha-sort for ease of look at list while debugging
        for struct in sorted(self.vk.structs.values()):
            out.append(f'    {struct.name},\n')
        out.append('};\n')

        out.append('\n')
        out.append('enum class Field {\n')
        out.append('    Empty = 0,\n')
        # Already alpha-sorted
        for field in self.fields:
            out.append(f'    {field},\n')
        out.append('};\n')
        out.append('\n')

        out.append('enum class Enum {\n')
        out.append('    Empty = 0,\n')
        # Want alpha-sort for ease of look at list while debugging
        for enum in sorted(self.vk.enums.values()):
            out.append(f'    {enum.name},\n')
        out.append('};\n')

        out.append('enum class FlagBitmask {\n')
        out.append('    Empty = 0,\n')
        # Want alpha-sort for ease of look at list while debugging
        for bitmask in sorted(self.vk.bitmasks.values()):
            out.append(f'    {bitmask.name},\n')
        out.append('};\n')

        out.append('\n')
        out.append('// Need underscore prefix to not conflict with namespace, but still easy to match generation\n')
        out.append('enum class Extension {\n')
        out.append('    Empty = 0,\n')
        for extension in sorted(self.vk.extensions.values(), key=lambda x: x.name):
            out.append(f'    _{extension.name},\n')
        out.append('};\n')

        out.append('''

            // Sometimes you know the requirement list doesn't contain any version values
            typedef small_vector<vvl::Extension, 2, size_t> Extensions;

            struct Requirement {
                const vvl::Extension extension;
                const vvl::Version version;

                Requirement(vvl::Extension extension_) : extension(extension_), version(vvl::Version::Empty) {}
                Requirement(vvl::Version version_) : extension(vvl::Extension::Empty), version(version_) {}
            };
            typedef small_vector <Requirement, 2, size_t> Requirements;

            const char* String(Func func);
            const char* String(Struct structure);
            const char* String(Field field);
            const char* String(Enum value);
            const char* String(FlagBitmask value);
            const char* String(Extension extension);
            std::string String(const Extensions& extensions);
            std::string String(const Requirement& requirement);
            std::string String(const Requirements& requirements);

            bool IsFieldPointer(Field field);

            // Used for VUID maps were we only want the new function name
            Func FindAlias(Func func);
            }  // namespace vvl
            ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "error_location_helper.h"
            #include "generated/vk_api_version.h"
            #include <assert.h>
            #include <string_view>
            ''')

        out.append('''
// clang-format off
namespace vvl {
''')
        out.append('''
const char* String(Func func) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Func::Empty
''')
        # Need to be alpha-sort also to match array indexing
        for command in sorted(self.vk.commands.values()):
            out.append(f'    {{"{command.name}", {len(command.name) + 1}}},\n')
        out.append('''    };
    return table[(int)func].data();
}

const char* String(Struct structure) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Struct::Empty
''')
        # Need to be alpha-sort also to match array indexing
        for struct in sorted(self.vk.structs.values()):
            out.append(f'    {{"{struct.name}", {len(struct.name) + 1}}},\n')
        out.append('''    };
    return table[(int)structure].data();
}

const char* String(Field field) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Field::Empty
''')
        for field in self.fields:
            out.append(f'    {{"{field}", {len(field) + 1}}},\n')
        out.append('''    };
    return table[(int)field].data();
}

const char* String(Enum value) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Enum::Empty
''')
        # Need to be alpha-sort also to match array indexing
        for enum in sorted(self.vk.enums.values()):
            out.append(f'    {{"{enum.name}", {len(enum.name) + 1}}},\n')
        out.append('''    };
    return table[(int)value].data();
}

const char* String(FlagBitmask value) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // FlagBitmask::Empty
''')
        # Need to be alpha-sort also to match array indexing
        for bitmask in sorted(self.vk.bitmasks.values()):
            out.append(f'    {{"{bitmask.name}", {len(bitmask.name) + 1}}},\n')
        out.append('''    };
    return table[(int)value].data();
}

const char* String(Extension extension) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Extension::Empty
''')
        for extension in sorted(self.vk.extensions.values(), key=lambda x: x.name):
            out.append(f'    {{"{extension.name}", {len(extension.name) + 1}}},\n')
        out.append('''    };
    return table[(int)extension].data();
}

bool IsFieldPointer(Field field) {
    switch (field) {
''')
        for field in self.pointer_fields:
            out.append(f'    case Field::{field}:\n')
        out.append('''        return true;
    default:
        return false;
    }
}

Func FindAlias(Func func) {
    switch (func) {
''')
        for command in [x for x in self.vk.commands.values() if x.alias]:
            out.append(f'    case Func::{command.name}:\n')
            out.append(f'       return Func::{command.alias};\n')
        out.append('''
    default:
        break;
    }
    return func;
}
// clang-format on
''')

        out.append('''
            std::string String(const Extensions& extensions) {
                std::stringstream out;
                for (size_t i = 0; i < extensions.size(); i++) {
                    out << String(extensions[i]);
                    if (i + 1 != extensions.size()) {
                        out << " or ";
                    }
                }
                return out.str();
            }

            std::string String(const Requirement& requirement) {
                if (requirement.extension == Extension::Empty) {
                    APIVersion api_version(static_cast<uint32_t>(requirement.version));
                    return StringAPIVersion(api_version);
                } else {
                    return String(requirement.extension);
                }
            }

            std::string String(const Requirements& requirements) {
                std::stringstream out;
                for (size_t i = 0; i < requirements.size(); i++) {
                    out << String(requirements[i]);
                    if (i + 1 != requirements.size()) {
                        out << " or ";
                    }
                }
                return out.str();
            }

        }  // namespace vvl
        ''')

        self.write("".join(out))
