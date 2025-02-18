#!/usr/bin/env python3
# Copyright 2019 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os, json, sys
from collections import namedtuple
import xml.etree.ElementTree as etree

from generator_lib import Generator, run_generator, FileRender, GeneratorOutput


class ProcName:
    def __init__(self, gl_name, proc_name=None):
        assert gl_name.startswith('gl')
        if proc_name == None:
            proc_name = gl_name[2:]

        self.gl_name = gl_name
        self.proc_name = proc_name

    def glProcName(self):
        return self.gl_name

    def ProcName(self):
        return self.proc_name

    def PFNPROCNAME(self):
        return 'PFN' + self.gl_name.upper() + 'PROC'

    def __repr__(self):
        return 'Proc("{}", "{}")'.format(self.gl_name, self.proc_name)


ProcParam = namedtuple('ProcParam', ['name', 'type'])


class Proc:
    def __init__(self, element):
        # Type declaration for return values and arguments all have the same
        # (weird) format.
        # <element>[A][<ptype>B</ptype>][C]<other stuff.../></element>
        #
        # Some examples are:
        #   <proto>void <name>glFinish</name></proto>
        #   <proto><ptype>GLenum</ptype><name>glFenceSync</name></proto>
        #   <proto>const <ptype>GLubyte</ptype> *<name>glGetString</name></proto>
        #
        # This handles all the shapes found in gl.xml except for this one that
        # has an array specifier after </name>:
        #   <param><ptype>GLuint</ptype> <name>baseAndCount</name>[2]</param>
        def parse_type_declaration(element):
            result = ''
            if element.text != None:
                result += element.text
            ptype = element.find('ptype')
            if ptype != None:
                result += ptype.text
                if ptype.tail != None:
                    result += ptype.tail
            return result.strip()

        proto = element.find('proto')

        self.return_type = parse_type_declaration(proto)

        self.params = []
        for param in element.findall('./param'):
            self.params.append(
                ProcParam(
                    param.find('name').text, parse_type_declaration(param)))

        self.gl_name = proto.find('name').text
        self.alias = None
        if element.find('alias') != None:
            self.alias = element.find('alias').attrib['name']

    def glProcName(self):
        return self.gl_name

    def ProcName(self):
        assert self.gl_name.startswith('gl')
        return self.gl_name[2:]

    def PFNGLPROCNAME(self):
        return 'PFN' + self.gl_name.upper() + 'PROC'

    def __repr__(self):
        return 'Proc("{}")'.format(self.gl_name)


EnumDefine = namedtuple('EnumDefine', ['name', 'value'])
Version = namedtuple('Version', ['major', 'minor'])
VersionBlock = namedtuple('VersionBlock', ['version', 'procs', 'enums'])
HeaderBlock = namedtuple('HeaderBlock', ['description', 'procs', 'enums'])
ExtensionBlock = namedtuple('ExtensionBlock',
                            ['extension', 'procs', 'enums', 'supported_specs'])


def parse_version(version):
    return Version(*map(int, version.split('.')))


def compute_params(root, supported_extensions):
    # Parse all the commands and enums
    all_procs = {}
    for command in root.findall('''commands[@namespace='GL']/command'''):
        proc = Proc(command)
        assert proc.gl_name not in all_procs
        all_procs[proc.gl_name] = proc

    all_enums = {}
    for enum in root.findall('''enums[@namespace='GL']/enum'''):
        enum_name = enum.attrib['name']
        # Special case an enum we'll never use that has different values in GL and GLES
        if enum_name == 'GL_ACTIVE_PROGRAM_EXT':
            continue

        assert enum_name not in all_enums
        all_enums[enum_name] = EnumDefine(enum_name, enum.attrib['value'])

    # Get the list of all Desktop OpenGL function removed by the Core Profile.
    core_removed_procs = set()
    for proc in root.findall('''feature/remove[@profile='core']/command'''):
        core_removed_procs.add(proc.attrib['name'])

    # Get list of enums and procs per OpenGL ES/Desktop OpenGL version
    def parse_version_blocks(api, removed_procs=set()):
        blocks = []
        for section in root.findall('''feature[@api='{}']'''.format(api)):
            section_procs = []
            for command in section.findall('./require/command'):
                proc_name = command.attrib['name']
                assert all_procs[proc_name].alias == None
                if proc_name not in removed_procs:
                    section_procs.append(all_procs[proc_name])

            section_enums = []
            for enum in section.findall('./require/enum'):
                section_enums.append(all_enums[enum.attrib['name']])

            blocks.append(
                VersionBlock(parse_version(section.attrib['number']),
                             section_procs, section_enums))

        return blocks

    gles_blocks = parse_version_blocks('gles2')
    desktop_gl_blocks = parse_version_blocks('gl', core_removed_procs)

    def parse_extension_block(extension):
        section = root.find(
            '''extensions/extension[@name='{}']'''.format(extension))
        supported_specs = section.attrib['supported'].split('|')
        section_procs = []
        for command in section.findall('./require/command'):
            proc_name = command.attrib['name']
            assert all_procs[proc_name].alias == None
            section_procs.append(all_procs[proc_name])

        section_enums = []
        for enum in section.findall('./require/enum'):
            section_enums.append(all_enums[enum.attrib['name']])

        return ExtensionBlock(extension, section_procs, section_enums,
                              supported_specs)

    extension_desktop_gl_blocks = []
    extension_gles_blocks = []
    for extension in supported_extensions:
        extension_block = parse_extension_block(extension)
        if 'gl' in extension_block.supported_specs:
            extension_desktop_gl_blocks.append(extension_block)
        if 'gles2' in extension_block.supported_specs:
            extension_gles_blocks.append(extension_block)

    # Compute the blocks for headers such that there is no duplicate definition
    already_added_header_procs = set()
    already_added_header_enums = set()
    header_blocks = []

    def add_header_block(description, block):
        block_procs = []
        for proc in block.procs:
            if not proc.glProcName() in already_added_header_procs:
                already_added_header_procs.add(proc.glProcName())
                block_procs.append(proc)

        block_enums = []
        for enum in block.enums:
            if not enum.name in already_added_header_enums:
                already_added_header_enums.add(enum.name)
                block_enums.append(enum)

        if len(block_procs) > 0 or len(block_enums) > 0:
            header_blocks.append(
                HeaderBlock(description, block_procs, block_enums))

    for block in gles_blocks:
        add_header_block(
            'OpenGL ES {}.{}'.format(block.version.major, block.version.minor),
            block)

    for block in desktop_gl_blocks:
        add_header_block(
            'Desktop OpenGL {}.{}'.format(block.version.major,
                                          block.version.minor), block)

    for block in extension_desktop_gl_blocks:
        add_header_block(block.extension, block)

    for block in extension_gles_blocks:
        add_header_block(block.extension, block)

    return {
        'gles_blocks': gles_blocks,
        'desktop_gl_blocks': desktop_gl_blocks,
        'extension_desktop_gl_blocks': extension_desktop_gl_blocks,
        'extension_gles_blocks': extension_gles_blocks,
        'header_blocks': header_blocks,
    }


class OpenGLLoaderGenerator(Generator):
    def get_description(self):
        return 'Generates code to load OpenGL function pointers'

    def add_commandline_arguments(self, parser):
        parser.add_argument('--gl-xml',
                            required=True,
                            type=str,
                            help='The Khronos gl.xml to use.')
        parser.add_argument(
            '--supported-extensions',
            required=True,
            type=str,
            help=
            'The JSON file that defines the OpenGL and GLES extensions to use.'
        )

    def get_outputs(self, args):
        supported_extensions = []
        with open(args.supported_extensions) as f:
            supported_extensions_json = json.loads(f.read())
            supported_extensions = supported_extensions_json[
                'supported_extensions']

        params = compute_params(
            etree.parse(args.gl_xml).getroot(), supported_extensions)

        renders = [
            FileRender(
                'opengl/OpenGLFunctionsBase.cpp',
                'src/dawn/native/opengl/OpenGLFunctionsBase_autogen.cpp',
                [params]),
            FileRender('opengl/OpenGLFunctionsBase.h',
                       'src/dawn/native/opengl/OpenGLFunctionsBase_autogen.h',
                       [params]),
            FileRender('opengl/opengl_platform.h',
                       'src/dawn/native/opengl/opengl_platform_autogen.h',
                       [params]),
        ]
        return GeneratorOutput(renders=renders, imported_templates=[])

    def get_dependencies(self, args):
        return [
            os.path.abspath(args.gl_xml),
            os.path.abspath(args.supported_extensions)
        ]


if __name__ == '__main__':
    sys.exit(run_generator(OpenGLLoaderGenerator()))
