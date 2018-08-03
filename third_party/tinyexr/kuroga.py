#!/usr/bin/env python

#
# Kuroga, single python file meta-build system for ninja
# https://github.com/lighttransport/kuroga
#
# Requirements: python 2.6 or 2.7
#
# Usage: $ python kuroga.py input.py
#

import imp
import re
import textwrap
import glob
import os
import sys

# gcc preset
def add_gnu_rule(ninja):
    ninja.rule('gnucxx', description='CXX $out',
        command='$gnucxx -MMD -MF $out.d $gnudefines $gnuincludes $gnucxxflags -c $in -o $out',
        depfile='$out.d', deps='gcc')
    ninja.rule('gnucc', description='CC $out',
        command='$gnucc -MMD -MF $out.d $gnudefines $gnuincludes $gnucflags -c $in -o $out',
        depfile='$out.d', deps='gcc')
    ninja.rule('gnulink', description='LINK $out', pool='link_pool',
        command='$gnuld -o $out $in $libs $gnuldflags')
    ninja.rule('gnuar', description='AR $out', pool='link_pool',
        command='$gnuar rsc $out $in')
    ninja.rule('gnustamp', description='STAMP $out', command='touch $out')
    ninja.newline()

    ninja.variable('gnucxx', 'g++')
    ninja.variable('gnucc', 'gcc')
    ninja.variable('gnuld', '$gnucxx')
    ninja.variable('gnuar', 'ar')
    ninja.newline()

# clang preset
def add_clang_rule(ninja):
    ninja.rule('clangcxx', description='CXX $out',
        command='$clangcxx -MMD -MF $out.d $clangdefines $clangincludes $clangcxxflags -c $in -o $out',
        depfile='$out.d', deps='gcc')
    ninja.rule('clangcc', description='CC $out',
        command='$clangcc -MMD -MF $out.d $clangdefines $clangincludes $clangcflags -c $in -o $out',
        depfile='$out.d', deps='gcc')
    ninja.rule('clanglink', description='LINK $out', pool='link_pool',
        command='$clangld -o $out $in $libs $gnuldflags')
    ninja.rule('clangar', description='AR $out', pool='link_pool',
        command='$clangar rsc $out $in')
    ninja.rule('clangstamp', description='STAMP $out', command='touch $out')
    ninja.newline()

    ninja.variable('clangcxx', 'clang++')
    ninja.variable('clangcc', 'clang')
    ninja.variable('clangld', '$clangcxx')
    ninja.variable('clangar', 'ar')
    ninja.newline()

# msvc preset
def add_msvc_rule(ninja):
    ninja.rule('msvccxx', description='CXX $out',
        command='$msvccxx /TP /showIncludes $msvcdefines $msvcincludes $msvccxxflags -c $in /Fo$out',
        depfile='$out.d', deps='msvc')
    ninja.rule('msvccc', description='CC $out',
        command='$msvccc /TC /showIncludes $msvcdefines $msvcincludes $msvccflags -c $in /Fo$out',
        depfile='$out.d', deps='msvc')
    ninja.rule('msvclink', description='LINK $out', pool='link_pool',
        command='$msvcld $msvcldflags $in $libs /OUT:$out')
    ninja.rule('msvcar', description='AR $out', pool='link_pool',
        command='$msvcar $in /OUT:$out')
    #ninja.rule('msvcstamp', description='STAMP $out', command='touch $out')
    ninja.newline()

    ninja.variable('msvccxx', 'cl.exe')
    ninja.variable('msvccc', 'cl.exe')
    ninja.variable('msvcld', 'link.exe')
    ninja.variable('msvcar', 'lib.exe')
    ninja.newline()

# -- from ninja_syntax.py --
def escape_path(word):
    return word.replace('$ ', '$$ ').replace(' ', '$ ').replace(':', '$:')

class Writer(object):
    def __init__(self, output, width=78):
        self.output = output
        self.width = width

    def newline(self):
        self.output.write('\n')

    def comment(self, text, has_path=False):
        for line in textwrap.wrap(text, self.width - 2, break_long_words=False,
                                  break_on_hyphens=False):
            self.output.write('# ' + line + '\n')

    def variable(self, key, value, indent=0):
        if value is None:
            return
        if isinstance(value, list):
            value = ' '.join(filter(None, value))  # Filter out empty strings.
        self._line('%s = %s' % (key, value), indent)

    def pool(self, name, depth):
        self._line('pool %s' % name)
        self.variable('depth', depth, indent=1)

    def rule(self, name, command, description=None, depfile=None,
             generator=False, pool=None, restat=False, rspfile=None,
             rspfile_content=None, deps=None):
        self._line('rule %s' % name)
        self.variable('command', command, indent=1)
        if description:
            self.variable('description', description, indent=1)
        if depfile:
            self.variable('depfile', depfile, indent=1)
        if generator:
            self.variable('generator', '1', indent=1)
        if pool:
            self.variable('pool', pool, indent=1)
        if restat:
            self.variable('restat', '1', indent=1)
        if rspfile:
            self.variable('rspfile', rspfile, indent=1)
        if rspfile_content:
            self.variable('rspfile_content', rspfile_content, indent=1)
        if deps:
            self.variable('deps', deps, indent=1)

    def build(self, outputs, rule, inputs=None, implicit=None, order_only=None,
              variables=None):
        outputs = as_list(outputs)
        out_outputs = [escape_path(x) for x in outputs]
        all_inputs = [escape_path(x) for x in as_list(inputs)]

        if implicit:
            implicit = [escape_path(x) for x in as_list(implicit)]
            all_inputs.append('|')
            all_inputs.extend(implicit)
        if order_only:
            order_only = [escape_path(x) for x in as_list(order_only)]
            all_inputs.append('||')
            all_inputs.extend(order_only)

        self._line('build %s: %s' % (' '.join(out_outputs),
                                     ' '.join([rule] + all_inputs)))

        if variables:
            if isinstance(variables, dict):
                iterator = iter(variables.items())
            else:
                iterator = iter(variables)

            for key, val in iterator:
                self.variable(key, val, indent=1)

        return outputs

    def include(self, path):
        self._line('include %s' % path)

    def subninja(self, path):
        self._line('subninja %s' % path)

    def default(self, paths):
        self._line('default %s' % ' '.join(as_list(paths)))

    def _count_dollars_before_index(self, s, i):
        """Returns the number of '$' characters right in front of s[i]."""
        dollar_count = 0
        dollar_index = i - 1
        while dollar_index > 0 and s[dollar_index] == '$':
            dollar_count += 1
            dollar_index -= 1
        return dollar_count

    def _line(self, text, indent=0):
        """Write 'text' word-wrapped at self.width characters."""
        leading_space = '  ' * indent
        while len(leading_space) + len(text) > self.width:
            # The text is too wide; wrap if possible.

            # Find the rightmost space that would obey our width constraint and
            # that's not an escaped space.
            available_space = self.width - len(leading_space) - len(' $')
            space = available_space
            while True:
                space = text.rfind(' ', 0, space)
                if (space < 0 or
                    self._count_dollars_before_index(text, space) % 2 == 0):
                    break

            if space < 0:
                # No such space; just use the first unescaped space we can find.
                space = available_space - 1
                while True:
                    space = text.find(' ', space + 1)
                    if (space < 0 or
                        self._count_dollars_before_index(text, space) % 2 == 0):
                        break
            if space < 0:
                # Give up on breaking.
                break

            self.output.write(leading_space + text[0:space] + ' $\n')
            text = text[space+1:]

            # Subsequent lines are continuations, so indent them.
            leading_space = '  ' * (indent+2)

        self.output.write(leading_space + text + '\n')

    def close(self):
        self.output.close()


def as_list(input):
    if input is None:
        return []
    if isinstance(input, list):
        return input
    return [input]

# -- end from ninja_syntax.py --

def gen(ninja, toolchain, config):
    
    ninja.variable('ninja_required_version', '1.4')
    ninja.newline()

    if hasattr(config, "builddir"):
        builddir = config.builddir[toolchain]
        ninja.variable(toolchain + 'builddir', builddir)
    else:
        builddir = ''

    ninja.variable(toolchain + 'defines', config.defines[toolchain] or [])
    ninja.variable(toolchain + 'includes', config.includes[toolchain] or [])
    ninja.variable(toolchain + 'cflags', config.cflags[toolchain] or [])
    ninja.variable(toolchain + 'cxxflags', config.cxxflags[toolchain] or [])
    ninja.variable(toolchain + 'ldflags', config.ldflags[toolchain] or [])
    ninja.newline()

    if hasattr(config, "link_pool_depth"):
        ninja.pool('link_pool', depth=config.link_pool_depth)
    else:
        ninja.pool('link_pool', depth=4)
    ninja.newline()

    # Add default toolchain(gnu, clang and msvc)
    add_gnu_rule(ninja)
    add_clang_rule(ninja)
    add_msvc_rule(ninja)

    obj_files = []

    cc = toolchain + 'cc'
    cxx = toolchain + 'cxx'
    link = toolchain + 'link'
    ar = toolchain + 'ar'
    
    if hasattr(config, "cxx_files"):
        for src in config.cxx_files:
            srcfile = src
            obj = os.path.splitext(srcfile)[0] + '.o'
            obj = os.path.join(builddir, obj);
            obj_files.append(obj)
            ninja.build(obj, cxx, srcfile)
        ninja.newline()

    if hasattr(config, "c_files"):
        for src in config.c_files:
            srcfile = src
            obj = os.path.splitext(srcfile)[0] + '.o'
            obj = os.path.join(builddir, obj);
            obj_files.append(obj)
            ninja.build(obj, cc, srcfile)
        ninja.newline()

    targetlist = []
    if hasattr(config, "exe"):
        ninja.build(config.exe, link, obj_files)
        targetlist.append(config.exe)

    if hasattr(config, "staticlib"):
        ninja.build(config.staticlib, ar, obj_files)
        targetlist.append(config.staticlib)

    ninja.build('all', 'phony', targetlist)
    ninja.newline()
        
    ninja.default('all')

def main():
    if len(sys.argv) < 2:
        print("Usage: python kuroga.py config.py")
        sys.exit(1)

    config = imp.load_source("config", sys.argv[1])

    f = open('build.ninja', 'w')
    ninja = Writer(f)

    if hasattr(config, "register_toolchain"):
        config.register_toolchain(ninja)
        
    gen(ninja, config.toolchain, config)
    f.close()

main()
