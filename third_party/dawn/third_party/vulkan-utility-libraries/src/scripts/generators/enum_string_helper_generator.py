#!/usr/bin/python3 -i
#
# Copyright 2023-2025 The Khronos Group Inc.
# Copyright 2023-2025 Valve Corporation
# Copyright 2023-2025 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

import os
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class EnumStringHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications
// Copyright 2023-2025 The Khronos Group Inc.
// Copyright 2023-2025 Valve Corporation
// Copyright 2023-2025 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
''')
        out.append('''
#pragma once

// clang-format off

#ifdef __cplusplus
#include <string>
#endif
#include <vulkan/vulkan.h>
''')
        guard_helper = PlatformGuardHelper()

        # If there are no fields (empty enum) ignore
        for enum in [x for x in self.vk.enums.values() if len(x.fields) > 0]:
            groupType = enum.name if enum.bitWidth == 32 else 'uint64_t'
            out.extend(guard_helper.add_guard(enum.protect))
            out.append(f'static inline const char* string_{enum.name}({groupType} input_value) {{\n')
            out.append('    switch (input_value) {\n')
            enum_field_guard_helper = PlatformGuardHelper()
            for field in enum.fields:
                out.extend(enum_field_guard_helper.add_guard(field.protect))
                out.append(f'        case {field.name}:\n')
                out.append(f'            return "{field.name}";\n')
            out.extend(enum_field_guard_helper.add_guard(None))
            out.append('        default:\n')
            out.append(f'            return "Unhandled {enum.name}";\n')
            out.append('    }\n')
            out.append('}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('\n')

        # For bitmask, first create a string for FlagBits, then a Flags version that calls into it
        # If there are no flags (empty bitmask) ignore
        for bitmask in [x for x in self.vk.bitmasks.values() if len(x.flags) > 0]:
            groupType = bitmask.name if bitmask.bitWidth == 32 else 'uint64_t'

            # switch labels must be constant expressions. In C a const-qualified variable is not a constant expression.
            use_switch_statement = True
            if groupType == 'uint64_t':
                use_switch_statement = False

            out.extend(guard_helper.add_guard(bitmask.protect))
            out.append(f'static inline const char* string_{bitmask.name}({groupType} input_value) {{\n')

            bitmask_field_guard_helper = PlatformGuardHelper()
            if use_switch_statement:
                out.append('    switch (input_value) {\n')
                for flag in [x for x in bitmask.flags if not x.multiBit]:
                    out.extend(bitmask_field_guard_helper.add_guard(flag.protect))
                    out.append(f'        case {flag.name}:\n')
                    out.append(f'            return "{flag.name}";\n')
                out.extend(bitmask_field_guard_helper.add_guard(None))
                out.append('        default:\n')
                out.append(f'            return "Unhandled {bitmask.name}";\n')
                out.append('    }\n')
            else:
                # We need to use if statements
                for flag in [x for x in bitmask.flags if not x.multiBit]:
                    out.extend(bitmask_field_guard_helper.add_guard(flag.protect))
                    out.append(f'    if (input_value == {flag.name}) return "{flag.name}";\n')
                out.extend(bitmask_field_guard_helper.add_guard(None))
                out.append(f'    return "Unhandled {bitmask.name}";\n')
            out.append('}\n')

            mulitBitChecks = ''
            for flag in [x for x in bitmask.flags if x.multiBit]:
                mulitBitChecks += f'    if (input_value == {flag.name}) {{ return "{flag.name}"; }}\n'
            intSuffix = 'U' if bitmask.bitWidth == 32 else 'ULL'

            out.append('\n#ifdef __cplusplus')
            out.append(f'''
static inline std::string string_{bitmask.flagName}({bitmask.flagName} input_value) {{
{mulitBitChecks}    std::string ret;
    int index = 0;
    while(input_value) {{
        if (input_value & 1) {{
            if( !ret.empty()) ret.append("|");
            ret.append(string_{bitmask.name}(static_cast<{groupType}>(1{intSuffix} << index)));
        }}
        ++index;
        input_value >>= 1;
    }}
    if (ret.empty()) ret.append("{bitmask.flagName}(0)");
    return ret;
}}\n''')
            out.append('#endif // __cplusplus\n')
        out.extend(guard_helper.add_guard(None))
        out.append('// clang-format on')

        # Special lookup for struct name of VkStructureType (since the sType name is not always that helpful)
        out.append('''\n
                   // Same thing as string_VkStructureType, but prints out the API name
                   static inline const char* string_VkStructureName(VkStructureType input_value) {
                        switch (input_value) {\n''')
        struct_guard_helper = PlatformGuardHelper()
        for struct in [x for x in self.vk.structs.values() if x.sType]:
            out.extend(struct_guard_helper.add_guard(struct.protect))
            out.append(f'        case {struct.sType}:\n')
            out.append(f'            return "{struct.name}";\n')
        out.append('''    default:
                            return "Unhandled VkStructureType";
                        }
                    }
                   ''')

        self.write("".join(out))
