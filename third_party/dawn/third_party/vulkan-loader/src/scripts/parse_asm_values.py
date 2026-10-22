#!/usr/bin/python3 -i
#
# Copyright (c) 2022 The Khronos Group Inc.
# Copyright (c) 2022 LunarG, Inc.

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
# Author: Charles Giessen <charles@lunarg.com>

# This script reads in the 'intermediate output' of a compiler to look for sizeof/offsetof information
# necessary for the assembler portions of the loader. This is achieved by forcing the compiler to output
# the intermediate assembly output and looking for specific patterns which contain the relevant information

import sys
import os.path
from os.path import exists
import re
import subprocess
import traceback


# Where to write the "gen_defines.asm" file
destination_file = sys.argv[1]
# The location the build system puts the intermediate asm file which depends on the compiler
source_asm_file = sys.argv[2]
# Whether we are using "MASM", "MARMASM" or "GAS" for the assembler
assembler_type = sys.argv[3]
# Whether we are using gcc, clang, or msvc
compiler = sys.argv[4]
# taken from CMAKE_SYSTEM_PROCESSOR - x86_64, aarch64|arm64, x86, aarch32|armhf
# Only used with GAS/MARMASM - MASM doesn't need this, as it has its own way to determine x86 vs x64
arch = sys.argv[5]

POSIX_COMPILERS = ["GNU", "Clang", "AppleClang"]

if destination_file is None or source_asm_file is None or assembler_type is None or compiler is None or arch is None:
    print("Required command line arguments were not provided")
    sys.exit(1)

defines = ["VULKAN_LOADER_ERROR_BIT",
            "PTR_SIZE",
            "CHAR_PTR_SIZE",
            "FUNCTION_OFFSET_INSTANCE",
            "PHYS_DEV_OFFSET_INST_DISPATCH",
            "PHYS_DEV_OFFSET_PHYS_DEV_TRAMP",
            "ICD_TERM_OFFSET_PHYS_DEV_TERM",
            "PHYS_DEV_OFFSET_PHYS_DEV_TERM",
            "INSTANCE_OFFSET_ICD_TERM",
            "DISPATCH_OFFSET_ICD_TERM",
            "EXT_OFFSET_DEVICE_DISPATCH" ]

if os.path.splitext(source_asm_file)[1] == ".a":
    try:
        ar_path = sys.argv[6]
        asm_archive_member = sys.argv[7]
        subprocess_result = subprocess.Popen([ar_path, "p", source_asm_file, asm_archive_member], stdout=subprocess.PIPE)
        asm_intermediate_file = subprocess_result.stdout.read().decode("utf-8")
    except IOError:
        print("Could not open assembler archive file:", source_asm_file)
        traceback.print_exc()
        sys.exit(1)
else:
    try:
        with open(source_asm_file, 'r') as f:
            asm_intermediate_file = f.read()
    except IOError:
        print("Could not open assembler file:", source_asm_file)
        sys.exit(1)

with open(destination_file, "w", encoding="utf-8") as dest:
    # special case vulkan error bit due to it not appearing in the asm - its defined in the header as 8 so it shouldn't change
    if assembler_type == "MASM":
        dest.write("VULKAN_LOADER_ERROR_BIT equ 8;\n")
    elif assembler_type == 'MARMASM':
        dest.write('    AREA    loader_structs_details, DATA,READONLY\n')
        if arch == "aarch64" or arch == "arm64":
            dest.write("AARCH_64 EQU 1\n")
        elif arch in ["aarch32", "armhf", "arm"]:
            dest.write("AARCH_64 EQU 0\n")
        else:
            print('The parameter "arch" has value of ', arch, ' which is not recognized')
        dest.write("VULKAN_LOADER_ERROR_BIT EQU 8\n")
    elif assembler_type == "GAS":
        # let the assembler know which platform to use
        if arch == "x86_64":
            dest.write(".set X86_64, 1\n")
        elif arch == "aarch64" or arch == "arm64":
            dest.write(".set AARCH_64, 1\n")
        elif arch in ["aarch32", "armhf", "arm"]:
            dest.write(".set AARCH_64, 0\n")
        else:
            print('The parameter "arch" has value of ', arch, ' which is not recognized')
        # Nothing to write in the x86 case

    for d in defines:
        match = None
        if compiler == "MSVC":
            if d == "VULKAN_LOADER_ERROR_BIT":
                continue # skip due to special case
            if 'arm' in arch.lower():
                match = re.search('\\|'+ d + '\\| DCD[\t ]*0x([0-9a-f]+)', asm_intermediate_file)
            else:
                match = re.search(d + " DD [ ]*([0-9a-f]+)H", asm_intermediate_file)
        elif compiler in POSIX_COMPILERS:
            match = re.search(d + " = ([0-9]+)", asm_intermediate_file)

        if len(match.groups()) > 0:
            if compiler == "MSVC":
                value = str(int(match.group(1), 16))
            elif compiler in POSIX_COMPILERS:
                value = match.group(1)

            # MASM uses hex values, decode them here
            if assembler_type == "MASM":
                dest.write(d + " equ " + value +";\n")
            elif assembler_type == 'MARMASM':
                dest.write(d + ' EQU ' + value +'\n')
            elif assembler_type == "GAS":
                dest.write(".set " + d + ", " + value + "\n")
        else:
            print("Couldn't find ", d)
            sys.exit(1)
    if assembler_type == 'MARMASM':
        dest.write("    END\n")

