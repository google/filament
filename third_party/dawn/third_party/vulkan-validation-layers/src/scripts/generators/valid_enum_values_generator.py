#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
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
from collections import defaultdict
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class ValidEnumValuesOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)
        self.ignoreList = [
            'VkStructureType', # Structs are checked as there own thing
            'VkResult' # The spots it is used (VkBindMemoryStatusKHR, VkPresentInfoKHR) are really returrn values
        ]

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
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

        if self.filename == 'valid_enum_values.h':
            self.generateHeader()
        elif self.filename == 'valid_enum_values.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        guard_helper = PlatformGuardHelper()
        for enum in [x for x in self.vk.enums.values() if x.name not in self.ignoreList and not x.returnedOnly]:
            out.extend(guard_helper.add_guard(enum.protect))
            out.append(f'template<> ValidValue stateless::Context::IsValidEnumValue({enum.name} value) const;\n')
        out.extend(guard_helper.add_guard(None))

        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "stateless/stateless_validation.h"
            #include <vulkan/vk_enum_string_helper.h>

            //  Checking for values is a 2 part process
            //    1. Check if is valid at all
            //    2. If invalid, spend more time to figure out how and what info to report to the user
            //
            //  While this might not seem ideal to compute the enabled extensions every time this function is called, the
            //  other solution would be to build a list at vkCreateDevice time of all the valid values. This adds much higher
            //  memory overhead.
            //
            //  Another key point to consider is being able to tell the user a value is invalid because it "doesn't exist" vs
            //  "forgot to enable an extension" is VERY important

            ''')
        guard_helper = PlatformGuardHelper()

        for enum in [x for x in self.vk.enums.values() if x.name not in self.ignoreList and not x.returnedOnly]:
            out.extend(guard_helper.add_guard(enum.protect, extra_newline=True))
            out.append(f'template<> ValidValue stateless::Context::IsValidEnumValue({enum.name} value) const {{\n')
            out.append('    switch (value) {\n')
            # If the field has same/subset extensions as enum, we count it as "core" for the struct
            coreEnums = [x for x in enum.fields if not x.extensions or (x.extensions and all(e in enum.extensions for e in x.extensions))]
            for coreEnum in coreEnums:
                out.append(f'    case {coreEnum.name}:\n')
            out.append('    return ValidValue::Valid;\n')

            # Build up list of expressions so case statements with same expression can have fallthrough
            expressionMap = defaultdict(list)
            for field in [x for x in enum.fields if len(x.extensions) > 0]:
                expression = []
                # Ignore the base extensions needed to use the enum, only focus on the field specific extensions
                for extension in [x for x in field.extensions if x not in enum.extensions]:
                    expression.append(f'IsExtEnabled(extensions.{extension.name.lower()})')
                if (len(expression) == 0):
                    continue
                expression = " || ".join(expression)
                expressionMap[expression].append(field.name)

            for expression, names in expressionMap.items():
                for name in names:
                    out.append(f'    case {name}:\n')
                out.append(f'return {expression} ? ValidValue::Valid : ValidValue::NoExtension;\n')

            out.append('''default:
                            return ValidValue::NotFound;
                        };
                    }
                ''')
            out.extend(guard_helper.add_guard(None, extra_newline=True))

        # For those that had an extension on field, provide a way to get it to print a useful error message out
        for enum in [x for x in self.vk.enums.values() if x.name not in self.ignoreList and not x.returnedOnly]:
            out.extend(guard_helper.add_guard(enum.protect, extra_newline=True))

            # Need empty functions to resolve all template variations
            if len(enum.fieldExtensions) <= len(enum.extensions):
                out.append(f'template<> vvl::Extensions stateless::Context::GetEnumExtensions({enum.name} value) const {{ return {{}}; }}\n')
                out.append(f'template<> const char* stateless::Context::DescribeEnum({enum.name} value) const {{ return nullptr; }}\n')
                out.extend(guard_helper.add_guard(None, extra_newline=True))
                continue

            out.append(f'template<> vvl::Extensions stateless::Context::GetEnumExtensions({enum.name} value) const {{\n')
            out.append('    switch (value) {\n')

            expressionMap = defaultdict(list)
            for field in [x for x in enum.fields if len(x.extensions) > 0]:
                expression = []
                for extension in [x for x in field.extensions if x not in enum.extensions]:
                    expression.append(f'vvl::Extension::_{extension.name}')
                if (len(expression) == 0):
                    continue
                expression = ", ".join(expression)
                expressionMap[expression].append(field.name)

            for expression, names in expressionMap.items():
                for name in names:
                    out.append(f'    case {name}:\n')
                out.append(f'return {{{expression}}};\n')

            out.append('''default:
                            return {};
                        };
                    }
                ''')

            out.append(f'template<> const char* stateless::Context::DescribeEnum({enum.name} value) const {{\n')
            out.append(f'   return string_{enum.name}(value);\n')
            out.append('}\n')
            out.extend(guard_helper.add_guard(None, extra_newline=True))

        self.write(''.join(out))
