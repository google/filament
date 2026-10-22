#!/usr/bin/env python
# -*- Coding: UTF-8 -*-

# ---------------------------------------------------------------------------
# Open Asset Import Library (ASSIMP)
# ---------------------------------------------------------------------------
#
# Copyright (c) 2006-2010, ASSIMP Development Team
#
# All rights reserved.
#
# Redistribution and use of this software in source and binary forms, 
# with or without modification, are permitted provided that the following 
# conditions are met:
# 
# * Redistributions of source code must retain the above
#   copyright notice, this list of conditions and the
#   following disclaimer.
# 
# * Redistributions in binary form must reproduce the above
#   copyright notice, this list of conditions and the
#   following disclaimer in the documentation and/or other
#   materials provided with the distribution.
# 
# * Neither the name of the ASSIMP team, nor the names of its
#   contributors may be used to endorse or promote products
#   derived from this software without specific prior
#   written permission of the ASSIMP Development Team.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ---------------------------------------------------------------------------

"""Update PyAssimp's texture type constants C/C++ headers.

This script is meant to be executed in the source tree, directly from
port/PyAssimp/gen
"""

import os
import re

REenumTextureType = re.compile(r''
                r'enum\saiTextureType'    # enum aiTextureType
                r'[^{]*?\{'               # {
                r'(?P<code>.*?)'          #   code 
                r'\};'                    # };
                , re.IGNORECASE + re.DOTALL + re.MULTILINE)

# Replace comments
RErpcom = re.compile(r''
                r'\s*(/\*+\s|\*+/|\B\*\s?|///?!?)'   # /**
                r'(?P<line>.*?)'                     #  * line 
                , re.IGNORECASE + re.DOTALL)

# Remove trailing commas
RErmtrailcom = re.compile(r',$', re.IGNORECASE + re.DOTALL)

# Remove #ifdef __cplusplus   
RErmifdef = re.compile(r''
                r'#ifndef SWIG'                       # #ifndef SWIG
                r'(?P<code>.*)'                       #   code 
                r'#endif(\s*//\s*!?\s*SWIG)*'  # #endif
                , re.IGNORECASE + re.DOTALL)

path = '../../../include/assimp'

files = os.listdir (path)
enumText = ''
for fileName in files:
    if fileName.endswith('.h'):
      text = open(os.path.join(path, fileName)).read()
      for enum in REenumTextureType.findall(text):
        enumText = enum

text = ''
for line in enumText.split('\n'):
  line = line.lstrip().rstrip()
  line = RErmtrailcom.sub('', line)
  text += RErpcom.sub('# \g<line>', line) + '\n'
text = RErmifdef.sub('', text)

file = open('material.py', 'w')
file.write(text)
file.close()

print("Generation done. You can now review the file 'material.py' and merge it.")
