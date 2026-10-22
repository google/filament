#!/usr/bin/env python3
#
# Copyright 2023-2026 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

import argparse
import xml.etree.ElementTree as etree
from reg import stripNonmatchingAPIs

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='stripAPI',
                formatter_class=argparse.RawDescriptionHelpFormatter,
                description='''\
Filters out elements with non-matching explicit 'api' attributes from API XML.
To remove Vulkan SC-only elements from the combined API XML:
    python3 scripts/stripAPI.py -input xml/vk.xml -output vulkan-only.xml -keepAPI vulkan
To remove Vulkan-only elements:
    python3 scripts/stripAPI.py -input xml/vk.xml -output vulkansc-only.xml -keepAPI vulkansc
If you are parsing the XML yourself but using the xml.etree package, the
equivalent runtime code is:
   import reg
   reg.stripNonmatchingAPIs(tree.getroot(), keepAPI, actuallyDelete=True)
where 'tree' is an ElementTree created from the XML file using
    etree.parse(filename)''')

    parser.add_argument('-input', action='store',
                        required=True,
                        help='Specify input registry XML')
    parser.add_argument('-output', action='store',
                        required=True,
                        help='Specify output registry XML')
    parser.add_argument('-keepAPI', action='store',
                        default=None,
                        help='Specify API name whose \'api\' tags are kept')

    args = parser.parse_args()

    tree = etree.parse(args.input)
    if args.keepAPI is not None:
        stripNonmatchingAPIs(tree.getroot(), args.keepAPI, actuallyDelete = True)
    tree.write(args.output)

