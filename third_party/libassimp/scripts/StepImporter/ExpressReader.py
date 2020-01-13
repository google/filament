#!/usr/bin/env python3
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

"""Parse an EXPRESS file and extract basic information on all
entities and data types contained"""

import sys
import re
from collections import OrderedDict

re_match_entity = re.compile(r"""
ENTITY\s+(\w+)\s*                                    # 'ENTITY foo'
.*?                                                  #  skip SUPERTYPE-of
(?:SUBTYPE\s+OF\s+\((\w+)\))?;                       # 'SUBTYPE OF (bar);' or simply ';'
(.*?)                                                # 'a : atype;' (0 or more lines like this)
(?:(?:INVERSE|UNIQUE|WHERE)\s*$.*?)?                 #  skip the INVERSE, UNIQUE, WHERE clauses and everything behind 
END_ENTITY;                                          
""",re.VERBOSE|re.DOTALL|re.MULTILINE) 

re_match_type = re.compile(r"""
TYPE\s+(\w+?)\s*=\s*((?:LIST|SET)\s*\[\d+:[\d?]+\]\s*OF)?(?:\s*UNIQUE)?\s*(\w+)   # TYPE foo = LIST[1:2] of blub
(?:(?<=ENUMERATION)\s*OF\s*\((.*?)\))?
.*?                                                                 #  skip the WHERE clause
END_TYPE;
""",re.VERBOSE|re.DOTALL)

re_match_field = re.compile(r"""
\s+(\w+?)\s*:\s*(OPTIONAL)?\s*((?:LIST|SET)\s*\[\d+:[\d?]+\]\s*OF)?(?:\s*UNIQUE)?\s*(\w+?);
""",re.VERBOSE|re.DOTALL)


class Schema:
    def __init__(self):
        self.entities = OrderedDict()
        self.types = OrderedDict()

class Entity:
    def __init__(self,name,parent,members):
        self.name = name
        self.parent = parent
        self.members = members

class Field:
    def __init__(self,name,type,optional,collection):
        self.name = name
        self.type = type
        self.optional = optional
        self.collection = collection
        self.fullspec = (self.collection+' ' if self.collection else '') + self.type 

class Type:
    def __init__(self,name,aggregate,equals,enums):
        self.name = name
        self.aggregate = aggregate
        self.equals = equals
        self.enums = enums
        

def read(filename, silent=False):
    schema = Schema()
    print( "Try to read EXPRESS schema file" + filename)
    with open(filename,'rt') as inp: 
        contents = inp.read()
        types = re.findall(re_match_type,contents)
        for name,aggregate,equals,enums in types:
            schema.types[name] = Type(name,aggregate,equals,enums)
            
        entities = re.findall(re_match_entity,contents)
        for name,parent,fields_raw in entities:
            print('process entity {0}, parent is {1}'.format(name,parent)) if not silent else None
            fields = re.findall(re_match_field,fields_raw)
            members = [Field(name,type,opt,coll) for name, opt, coll, type in fields]
            print('  got {0} fields'.format(len(members))) if not silent else None
            
            schema.entities[name] = Entity(name,parent,members)
    return schema

if __name__ == "__main__":
    sys.exit(read(sys.argv[1] if len(sys.argv)>1 else 'schema.exp'))




    
