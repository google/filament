#!/usr/bin/env python3
# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import os
import sys
import shutil
import common_ci
from xml.etree import ElementTree

def RunGenerators(api: str, registry: str, targetFilter: str) -> None:

    has_clang_format = shutil.which('clang-format') is not None
    if not has_clang_format:
        print("WARNING: Unable to find clang-format!")

    # These live in the Vulkan-Docs repo, but are pulled in via the
    # Vulkan-Headers/registry folder
    # At runtime we inject python path to find these helper scripts
    scripts = os.path.dirname(registry)
    scripts_directory_path = os.path.dirname(os.path.abspath(__file__))
    registry_headers_path = os.path.join(scripts_directory_path, scripts)
    sys.path.insert(0, registry_headers_path)
    try:
        from reg import Registry
    except:
        print("ModuleNotFoundError: No module named 'reg'") # normal python error message
        print(f'{registry_headers_path} is not pointing to the Vulkan-Headers registry directory.')
        print("Inside Vulkan-Headers there is a registry/reg.py file that is used.")
        sys.exit(1) # Return without call stack so easy to spot error

    from base_generator import BaseGeneratorOptions
    from generators.dispatch_table_generator import DispatchTableOutputGenerator
    from generators.enum_string_helper_generator import EnumStringHelperOutputGenerator
    from generators.format_utils_generator import FormatUtilsOutputGenerator
    from generators.struct_helper_generator import StructHelperOutputGenerator
    from generators.safe_struct_generator import SafeStructOutputGenerator

    # These set fields that are needed by both OutputGenerator and BaseGenerator,
    # but are uniform and don't need to be set at a per-generated file level
    from base_generator import (SetTargetApiName, SetMergedApiNames)
    SetTargetApiName(api)

    # Build up a list of all generators and custom options
    generators = {
        'vk_dispatch_table.h' : {
           'generator' : DispatchTableOutputGenerator,
           'genCombined': True,
           'directory' : f'include/vulkan/utility',
        },
        'vk_enum_string_helper.h' : {
            'generator' : EnumStringHelperOutputGenerator,
            'genCombined': True,
            'directory' : f'include/vulkan',
        },
        'vk_format_utils.h' : {
            'generator' : FormatUtilsOutputGenerator,
            'genCombined': True,
            'directory' : f'include/vulkan/utility',
        },
        'vk_struct_helper.hpp' : {
            'generator' : StructHelperOutputGenerator,
            'genCombined': True,
            'directory' : f'include/vulkan/utility',
        },
        'vk_safe_struct.hpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
            'directory' : f'include/vulkan/utility',
        },
        'vk_safe_struct_utils.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
            'directory' : f'src/vulkan',
        },
        'vk_safe_struct_core.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
            'regenerate' : True,
            'directory' : f'src/vulkan',
        },
        'vk_safe_struct_khr.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
            'directory' : f'src/vulkan',
        },
        'vk_safe_struct_ext.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
            'directory' : f'src/vulkan',
        },
        'vk_safe_struct_vendor.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
            'directory' : f'src/vulkan',
        },
    }

    unknownTargets = [x for x in (targetFilter if targetFilter else []) if x not in generators.keys()]
    if unknownTargets:
        print(f'ERROR: No generator options for unknown target(s): {", ".join(unknownTargets)}', file=sys.stderr)
        return 1

    # Filter if --target is passed in
    targets = [x for x in generators.keys() if not targetFilter or x in targetFilter]

    for index, target in enumerate(targets, start=1):
        print(f'[{index}|{len(targets)}] Generating {target}')

        # First grab a class contructor object and create an instance
        generator = generators[target]['generator']
        gen = generator()

        # This code and the 'genCombined' generator metadata is used by downstream
        # users to generate code with all Vulkan APIs merged into the target API variant
        # (e.g. Vulkan SC) when needed. The constructed apiList is also used to filter
        # out non-applicable extensions later below.
        apiList = [api]
        if api != 'vulkan' and generators[target]['genCombined']:
            SetMergedApiNames('vulkan')
            apiList.append('vulkan')
        else:
            SetMergedApiNames(None)

        outDirectory = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', generators[target]['directory']))
        options = BaseGeneratorOptions(
            customFileName  = target,
            customDirectory = outDirectory)

        # Create the registry object with the specified generator and generator
        # options. The options are set before XML loading as they may affect it.
        reg = Registry(gen, options)

        # Parse the specified registry XML into an ElementTree object
        tree = ElementTree.parse(registry)

        # Filter out extensions that are not on the API list
        [exts.remove(e) for exts in tree.findall('extensions') for e in exts.findall('extension') if (sup := e.get('supported')) is not None and all(api not in sup.split(',') for api in apiList)]

        # Load the XML tree into the registry object
        reg.loadElementTree(tree)

        # Finally, use the output generator to create the requested target
        reg.apiGen()

        # Run clang-format on the file
        if has_clang_format:
            common_ci.RunShellCmd(f'clang-format -i {os.path.join(outDirectory, target)}')

def main(argv):
    parser = argparse.ArgumentParser(description='Generate source code for this repository')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('registry', metavar='REGISTRY_PATH', help='path to the Vulkan-Headers registry directory')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--target', nargs='+', help='only generate file name passed in')
    args = parser.parse_args(argv)

    registry = os.path.abspath(os.path.join(args.registry,  'vk.xml'))
    if not os.path.isfile(registry):
        registry = os.path.abspath(os.path.join(args.registry, 'Vulkan-Headers/registry/vk.xml'))
        if not os.path.isfile(registry):
            print(f'cannot find vk.xml in {args.registry}')
            return -1

    RunGenerators(args.api, registry, args.target)

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

