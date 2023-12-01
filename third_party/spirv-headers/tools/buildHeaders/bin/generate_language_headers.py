#!/usr/bin/env python3
# Copyright (c) 2017-2020 Google LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
# KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
# SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
#    https://www.khronos.org/registry/
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

"""Generates a C language headers from a SPIR-V JSON grammar file"""

import errno
import json
import os.path
import re

DEFAULT_COPYRIGHT="""Copyright (c) 2020 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Materials.

MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
   https://www.khronos.org/registry/

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
""".split('\n')

def make_path_to_file(f):
    """Makes all ancestor directories to the given file, if they
    don't yet exist.

    Arguments:
        f: The file whose ancestor directories are to be created.
    """
    dir = os.path.dirname(os.path.abspath(f))
    try:
        os.makedirs(dir)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(dir):
            pass
        else:
            raise

class ExtInstGrammar:
    """The grammar for an extended instruction set"""

    def __init__(self, name, copyright, instructions, operand_kinds, version = None, revision = None):
       self.name = name
       self.copyright = copyright
       self.instructions = instructions
       self.operand_kinds = operand_kinds
       self.version = version
       self.revision = revision


class LangGenerator:
    """A language-specific generator"""

    def __init__(self):
        self.upper_case_initial = re.compile('^[A-Z]')
        pass

    def comment_prefix(self):
        return ""

    def namespace_prefix(self):
        return ""

    def uses_guards(self):
        return False

    def cpp_guard_preamble(self):
        return ""

    def cpp_guard_postamble(self):
        return ""

    def enum_value(self, prefix, name, value):
        if self.upper_case_initial.match(name):
            use_name = name
        else:
            use_name = '_' + name

        return "    {}{} = {},".format(prefix, use_name, value)

    def generate(self, grammar):
        """Returns a string that is the language-specific header for the given grammar"""

        parts = []
        if grammar.copyright:
            parts.extend(["{}{}".format(self.comment_prefix(), f) for f in grammar.copyright])
        parts.append('')

        guard = 'SPIRV_UNIFIED1_{}_H_'.format(grammar.name)
        if self.uses_guards:
            parts.append('#ifndef {}'.format(guard))
            parts.append('#define {}'.format(guard))
        parts.append('')

        parts.append(self.cpp_guard_preamble())

        if grammar.version:
            parts.append(self.const_definition(grammar.name, 'Version', grammar.version))

        if grammar.revision is not None:
            parts.append(self.const_definition(grammar.name, 'Revision', grammar.revision))

        parts.append('')

        if grammar.instructions:
            parts.append(self.enum_prefix(grammar.name, 'Instructions'))
            for inst in grammar.instructions:
                parts.append(self.enum_value(grammar.name, inst['opname'], inst['opcode']))
            parts.append(self.enum_end(grammar.name, 'Instructions'))
            parts.append('')

        if grammar.operand_kinds:
            for kind in grammar.operand_kinds:
                parts.append(self.enum_prefix(grammar.name, kind['kind']))
                for e in kind['enumerants']:
                    parts.append(self.enum_value(grammar.name, e['enumerant'], e['value']))
                parts.append(self.enum_end(grammar.name, kind['kind']))
            parts.append('')

        parts.append(self.cpp_guard_postamble())

        if self.uses_guards:
            parts.append('#endif // {}'.format(guard))

        # Ensre the file ends in an end of line
        parts.append('')

        return '\n'.join(parts)


class CLikeGenerator(LangGenerator):
    def uses_guards(self):
        return True

    def comment_prefix(self):
        return "// "

    def const_definition(self, prefix, var, value):
        # Use an anonymous enum.  Don't use a static const int variable because
        # that can bloat binary size.
        return 'enum {0}{1}{2}{3} = {4},{1}{2}{3}_BitWidthPadding = 0x7fffffff{5};'.format(
               '{', '\n    ', prefix, var, value, '\n}')

    def enum_prefix(self, prefix, name):
        return 'enum {}{} {}'.format(prefix, name, '{')

    def enum_end(self, prefix, enum):
        return '    {}{}Max = 0x7fffffff\n{};\n'.format(prefix, enum, '}')

    def cpp_guard_preamble(self):
        return '#ifdef __cplusplus\nextern "C" {\n#endif\n'

    def cpp_guard_postamble(self):
        return '#ifdef __cplusplus\n}\n#endif\n'


class CGenerator(CLikeGenerator):
    pass


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Generate language headers from a JSON grammar')

    parser.add_argument('--extinst-name',
                        type=str, required=True,
                        help='The name to use in tokens')
    parser.add_argument('--extinst-grammar', metavar='<path>',
                        type=str, required=True,
                        help='input JSON grammar file for extended instruction set')
    parser.add_argument('--extinst-output-base', metavar='<path>',
                        type=str, required=True,
                        help='Basename of the language-specific output file.')
    args = parser.parse_args()

    with open(args.extinst_grammar) as json_file:
        grammar_json = json.loads(json_file.read())
        if 'copyright' in grammar_json:
          copyright = grammar_json['copyright']
        else:
          copyright = DEFAULT_COPYRIGHT
        if 'version' in grammar_json:
          version = grammar_json['version']
        else:
          version = 0
        if 'operand_kinds' in grammar_json:
          operand_kinds = grammar_json['operand_kinds']
        else:
          operand_kinds = []

        grammar = ExtInstGrammar(name = args.extinst_name,
                                 copyright = copyright,
                                 instructions = grammar_json['instructions'],
                                 operand_kinds = operand_kinds,
                                 version = version,
                                 revision = grammar_json['revision'])
        make_path_to_file(args.extinst_output_base)
        with open(args.extinst_output_base + '.h', 'w') as f:
            f.write(CGenerator().generate(grammar))


if __name__ == '__main__':
    main()
