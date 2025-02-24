# Copyright 2017-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# printreg(reg, varname)
# Prints a registry Python data structure (see registry.py) in a consistent
# fashion.

def tab():
    return '        '

def quote(str):
    return '\'' + str + '\''

def printKey(key, value):
    print(tab() + quote(key), ':', value + ',')

def printNum(ext, key):
    if (key in ext.keys()):
        printKey(key, str(ext[key]))

def printSet(ext, key):
    if (key in ext.keys()):
        value = ( '{ ' +
                  ', '.join([quote(str(tag)) for tag in sorted(ext[key])]) +
                  ' }' )
        printKey(key, value)

def printStr(ext, key):
    if (key in ext.keys()):
        printKey(key, quote(str(ext[key])))

def striplibs(s):
    return ( s.replace('GL_','').
               replace('GLU_','').
               replace('GLX_','').
               replace('WGL_','') )

def printreg(reg, varname):
    print(varname, '= {')

    # print('keys in registry =', len(reg.keys()))

    print('# OpenGL extension number and name registry')
    print('')

    for key in sorted(reg.keys(), key = striplibs):
        ext = reg[key]

        print('    ' + quote(key), ': {')
        printNum(ext, 'arbnumber')
        printNum(ext, 'number')
        printNum(ext, 'esnumber')
        printNum(ext, 'scnumber')
        printSet(ext, 'flags')
        printSet(ext, 'supporters')
        printStr(ext, 'url')
        printStr(ext, 'esurl')
        printSet(ext, 'alias')
        printStr(ext, 'comments')
        print('    },')

    print('}')

