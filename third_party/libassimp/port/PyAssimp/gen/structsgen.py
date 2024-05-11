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

"""Update PyAssimp's data structures to keep up with the
C/C++ headers.

This script is meant to be executed in the source tree, directly from
port/PyAssimp/gen
"""

import os
import re

#==[regexps]=================================================

# Clean desc 
REdefine = re.compile(r''
                r'(?P<desc>)'                                       # /** *desc */
                r'#\s*define\s(?P<name>[^(\n]+?)\s(?P<code>.+)$'    #  #define name value
                , re.MULTILINE)

# Get structs
REstructs = re.compile(r''
                #r'//\s?[\-]*\s(?P<desc>.*?)\*/\s'           # /** *desc */
                #r'//\s?[\-]*(?P<desc>.*?)\*/(?:.*?)'            # garbage
                r'//\s?[\-]*\s(?P<desc>.*?)\*/\W*?'           # /** *desc */
                r'struct\s(?:ASSIMP_API\s)?(?P<name>[a-z][a-z0-9_]\w+\b)'    # struct name
                r'[^{]*?\{'                                 # {
                r'(?P<code>.*?)'                            #   code 
                r'\}\s*(PACK_STRUCT)?;'                      # };
                , re.IGNORECASE + re.DOTALL + re.MULTILINE)

# Clean desc 
REdesc = re.compile(r''
                r'^\s*?([*]|/\*\*)(?P<line>.*?)'            #  * line 
                , re.IGNORECASE + re.DOTALL + re.MULTILINE)

# Remove #ifdef __cplusplus   
RErmifdef = re.compile(r''
                r'#ifdef __cplusplus'                       # #ifdef __cplusplus
                r'(?P<code>.*)'                             #   code 
                r'#endif(\s*//\s*!?\s*__cplusplus)*'          # #endif
                , re.IGNORECASE + re.DOTALL)
                
# Replace comments
RErpcom = re.compile(r''
                r'\s*(/\*+\s|\*+/|\B\*\s|///?!?)'             # /**
                r'(?P<line>.*?)'                            #  * line 
                , re.IGNORECASE + re.DOTALL)
                
# Restructure 
def GetType(type, prefix='c_'):
    t = type
    while t.endswith('*'):
        t = t[:-1]
    if t[:5] == 'const':
        t = t[5:]

    # skip some types
    if t in skiplist:
           return None

    t = t.strip()
    types = {'unsigned int':'uint', 'unsigned char':'ubyte',}
    if t in types:
        t = types[t]
    t = prefix + t
    while type.endswith('*'):
        t = "POINTER(" + t + ")"
        type = type[:-1]
    return t
    
def restructure( match ):
    type = match.group("type")
    if match.group("struct") == "":
        type = GetType(type)
    elif match.group("struct") == "C_ENUM ":
        type = "c_uint"
    else:
        type = GetType(type[2:], '')
        if type is None:
           return ''
    if match.group("index"):
        type = type + "*" + match.group("index")
        
    result = ""
    for name in match.group("name").split(','):
        result += "(\"" + name.strip() + "\", "+ type + "),"
    
    return result

RErestruc = re.compile(r''
                r'(?P<struct>C_STRUCT\s|C_ENUM\s|)'                     #  [C_STRUCT]
                r'(?P<type>\w+\s?\w+?[*]*)\s'                           #  type
                #r'(?P<name>\w+)'                                       #  name
                r'(?P<name>\w+|[a-z0-9_, ]+)'                               #  name
                r'(:?\[(?P<index>\w+)\])?;'                             #  []; (optional)
                , re.DOTALL)
#==[template]================================================
template = """
class $NAME$(Structure):
    \"\"\"
$DESCRIPTION$
    \"\"\" 
$DEFINES$
    _fields_ = [
            $FIELDS$
        ]
"""

templateSR = """
class $NAME$(Structure):
    \"\"\"
$DESCRIPTION$
    \"\"\" 
$DEFINES$

$NAME$._fields_ = [
            $FIELDS$
        ]
"""

skiplist = ("FileIO", "File", "locateFromAssimpHeap",'LogStream','MeshAnim','AnimMesh')

#============================================================
def Structify(fileName):
    file = open(fileName, 'r')
    text = file.read()
    result = []
    
    # Get defines.
    defs = REdefine.findall(text)
    # Create defines
    defines = "\n"
    for define in defs:
        # Clean desc 
        desc = REdesc.sub('', define[0])
        # Replace comments
        desc = RErpcom.sub('#\g<line>', desc)
        defines += desc
	if len(define[2].strip()):
            # skip non-integral defines, we can support them right now
            try:
                int(define[2],0)
            except:
                continue
            defines += " "*4 + define[1] + " = " + define[2] + "\n"  
            
    
    # Get structs
    rs = REstructs.finditer(text)

    fileName = os.path.basename(fileName)
    print fileName
    for r in rs:
        name = r.group('name')[2:]
        desc = r.group('desc')
        
        # Skip some structs
        if name in skiplist:
            continue

        text = r.group('code')

        # Clean desc 
        desc = REdesc.sub('', desc)
        
        desc = "See '"+ fileName +"' for details." #TODO  
        
        # Remove #ifdef __cplusplus
        text = RErmifdef.sub('', text)
        
        # Whether the struct contains more than just POD
        primitive = text.find('C_STRUCT') == -1

        # Restructure
        text = RErestruc.sub(restructure, text)
        # Replace comments
        text = RErpcom.sub('# \g<line>', text)
        text = text.replace("),#", "),\n#")
        text = text.replace("#", "\n#")
        text = "".join([l for l in text.splitlines(True) if not l.strip().endswith("#")]) # remove empty comment lines
        
        # Whether it's selfreferencing: ex. struct Node { Node* parent; };
        selfreferencing = text.find('POINTER('+name+')') != -1
        
        complex = name == "Scene"
        
        # Create description
        description = ""
        for line in desc.split('\n'):
            description += " "*4 + line.strip() + "\n"  
        description = description.rstrip()

        # Create fields
        fields = ""
        for line in text.split('\n'):
            fields += " "*12 + line.strip() + "\n"
        fields = fields.strip()

        if selfreferencing:
            templ = templateSR
        else:
            templ = template
            
        # Put it all together
        text = templ.replace('$NAME$', name)
        text = text.replace('$DESCRIPTION$', description)
        text = text.replace('$FIELDS$', fields)
        
        if ((name.lower() == fileName.split('.')[0][2:].lower()) and (name != 'Material')) or name == "String":
            text = text.replace('$DEFINES$', defines)
        else:
            text = text.replace('$DEFINES$', '')

        
        result.append((primitive, selfreferencing, complex, text))
             
    return result   

text = "#-*- coding: UTF-8 -*-\n\n"
text += "from ctypes import POINTER, c_int, c_uint, c_size_t, c_char, c_float, Structure, c_char_p, c_double, c_ubyte\n\n"

structs1 = ""
structs2 = ""
structs3 = ""
structs4 = ""

path = '../../../include/assimp'
files = os.listdir (path)
#files = ["aiScene.h", "aiTypes.h"]
for fileName in files:
    if fileName.endswith('.h'):
        for struct in Structify(os.path.join(path, fileName)):
            primitive, sr, complex, struct = struct
            if primitive:
                structs1 += struct
            elif sr:
                structs2 += struct
            elif complex:
                structs4 += struct
            else:
                structs3 += struct

text += structs1 + structs2 + structs3 + structs4

file = open('structs.py', 'w')
file.write(text)
file.close()

print("Generation done. You can now review the file 'structs.py' and merge it.")
