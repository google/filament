#!/usr/bin/env python3 -i
#
# Copyright 2013-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

import os
import re

from generator import (GeneratorOptions,
                       MissingGeneratorOptionsConventionsError,
                       MissingGeneratorOptionsError, MissingRegistryError,
                       OutputGenerator, noneStr, regSortFeatures, write)

class CGeneratorOptions(GeneratorOptions):
    """CGeneratorOptions - subclass of GeneratorOptions.

    Adds options used by COutputGenerator objects during C language header
    generation."""

    def __init__(self,
                 prefixText='',
                 genFuncPointers=True,
                 protectFile=True,
                 protectFeature=True,
                 protectProto=None,
                 protectProtoStr=None,
                 protectExtensionProto=None,
                 protectExtensionProtoStr=None,
                 apicall='',
                 apientry='',
                 apientryp='',
                 indentFuncProto=True,
                 indentFuncPointer=False,
                 alignFuncParam=0,
                 genEnumBeginEndRange=False,
                 genAliasMacro=False,
                 genStructExtendsComment=False,
                 aliasMacro='',
                 misracstyle=False,
                 misracppstyle=False,
                 **kwargs
                 ):
        """Constructor.
        Additional parameters beyond parent class:

        - prefixText - list of strings to prefix generated header with
        (usually a copyright statement + calling convention macros)
        - protectFile - True if multiple inclusion protection should be
        generated (based on the filename) around the entire header
        - protectFeature - True if #ifndef..#endif protection should be
        generated around a feature interface in the header file
        - genFuncPointers - True if function pointer typedefs should be
        generated
        - protectProto - If conditional protection should be generated
        around prototype declarations, set to either '#ifdef'
        to require opt-in (#ifdef protectProtoStr) or '#ifndef'
        to require opt-out (#ifndef protectProtoStr). Otherwise
        set to None.
        - protectProtoStr - #ifdef/#ifndef symbol to use around prototype
        declarations, if protectProto is set
        - protectExtensionProto - If conditional protection should be generated
        around extension prototype declarations, set to either '#ifdef'
        to require opt-in (#ifdef protectExtensionProtoStr) or '#ifndef'
        to require opt-out (#ifndef protectExtensionProtoStr). Otherwise
        set to None
        - protectExtensionProtoStr - #ifdef/#ifndef symbol to use around
        extension prototype declarations, if protectExtensionProto is set
        - apicall - string to use for the function declaration prefix,
        such as APICALL on Windows
        - apientry - string to use for the calling convention macro,
        in typedefs, such as APIENTRY
        - apientryp - string to use for the calling convention macro
        in function pointer typedefs, such as APIENTRYP
        - indentFuncProto - True if prototype declarations should put each
        parameter on a separate line
        - indentFuncPointer - True if typedefed function pointers should put each
        parameter on a separate line
        - alignFuncParam - if nonzero and parameters are being put on a
        separate line, align parameter names at the specified column
        - genEnumBeginEndRange - True if BEGIN_RANGE / END_RANGE macros should
        be generated for enumerated types
        - genAliasMacro - True if the OpenXR alias macro should be generated
        for aliased types (unclear what other circumstances this is useful)
        - genStructExtendsComment - True if comments showing the structures
        whose pNext chain a structure extends are included before its
        definition
        - aliasMacro - alias macro to inject when genAliasMacro is True
        - misracstyle - generate MISRA C-friendly headers
        - misracppstyle - generate MISRA C++-friendly headers"""

        GeneratorOptions.__init__(self, **kwargs)

        self.prefixText = prefixText
        """list of strings to prefix generated header with (usually a copyright statement + calling convention macros)."""

        self.genFuncPointers = genFuncPointers
        """True if function pointer typedefs should be generated"""

        self.protectFile = protectFile
        """True if multiple inclusion protection should be generated (based on the filename) around the entire header."""

        self.protectFeature = protectFeature
        """True if #ifndef..#endif protection should be generated around a feature interface in the header file."""

        self.protectProto = protectProto
        """If conditional protection should be generated around prototype declarations, set to either '#ifdef' to require opt-in (#ifdef protectProtoStr) or '#ifndef' to require opt-out (#ifndef protectProtoStr). Otherwise set to None."""

        self.protectProtoStr = protectProtoStr
        """#ifdef/#ifndef symbol to use around prototype declarations, if protectProto is set"""

        self.protectExtensionProto = protectExtensionProto
        """If conditional protection should be generated around extension prototype declarations, set to either '#ifdef' to require opt-in (#ifdef protectExtensionProtoStr) or '#ifndef' to require opt-out (#ifndef protectExtensionProtoStr). Otherwise set to None."""

        self.protectExtensionProtoStr = protectExtensionProtoStr
        """#ifdef/#ifndef symbol to use around extension prototype declarations, if protectExtensionProto is set"""

        self.apicall = apicall
        """string to use for the function declaration prefix, such as APICALL on Windows."""

        self.apientry = apientry
        """string to use for the calling convention macro, in typedefs, such as APIENTRY."""

        self.apientryp = apientryp
        """string to use for the calling convention macro in function pointer typedefs, such as APIENTRYP."""

        self.indentFuncProto = indentFuncProto
        """True if prototype declarations should put each parameter on a separate line"""

        self.indentFuncPointer = indentFuncPointer
        """True if typedefed function pointers should put each parameter on a separate line"""

        self.alignFuncParam = alignFuncParam
        """if nonzero and parameters are being put on a separate line, align parameter names at the specified column"""

        self.genEnumBeginEndRange = genEnumBeginEndRange
        """True if BEGIN_RANGE / END_RANGE macros should be generated for enumerated types"""

        self.genAliasMacro = genAliasMacro
        """True if the OpenXR alias macro should be generated for aliased types (unclear what other circumstances this is useful)"""

        self.genStructExtendsComment = genStructExtendsComment
        """True if comments showing the structures whose pNext chain a structure extends are included before its definition"""

        self.aliasMacro = aliasMacro
        """alias macro to inject when genAliasMacro is True"""

        self.misracstyle = misracstyle
        """generate MISRA C-friendly headers"""

        self.misracppstyle = misracppstyle
        """generate MISRA C++-friendly headers"""

        self.codeGenerator = True
        """True if this generator makes compilable code"""


class COutputGenerator(OutputGenerator):
    """Generates C-language API interfaces."""

    # This is an ordered list of sections in the header file.
    TYPE_SECTIONS = ['include', 'define', 'basetype', 'handle', 'enum',
                     'group', 'bitmask', 'funcpointer', 'struct']
    ALL_SECTIONS = TYPE_SECTIONS + ['commandPointer', 'command']

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # Internal state - accumulators for different inner block text
        self.sections = {section: [] for section in self.ALL_SECTIONS}
        self.feature_not_empty = False
        self.may_alias = None

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        # C-specific
        #
        # Multiple inclusion protection & C++ wrappers.
        if self.genOpts.protectFile and self.genOpts.filename:
            headerSym = re.sub(r'\.h', '_h_',
                               os.path.basename(self.genOpts.filename)).upper()
            write('#ifndef', headerSym, file=self.outFile)
            write('#define', headerSym, '1', file=self.outFile)
            self.newline()

        # User-supplied prefix text, if any (list of strings)
        if genOpts.prefixText:
            for s in genOpts.prefixText:
                write(s, file=self.outFile)

        # C++ extern wrapper - after prefix lines so they can add includes.
        self.newline()
        write('#ifdef __cplusplus', file=self.outFile)
        write('extern "C" {', file=self.outFile)
        write('#endif', file=self.outFile)
        self.newline()

    def endFile(self):
        # C-specific
        # Finish C++ wrapper and multiple inclusion protection
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        self.newline()
        write('#ifdef __cplusplus', file=self.outFile)
        write('}', file=self.outFile)
        write('#endif', file=self.outFile)
        if self.genOpts.protectFile and self.genOpts.filename:
            self.newline()
            write('#endif', file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        # C-specific
        # Accumulate includes, defines, types, enums, function pointer typedefs,
        # end function prototypes separately for this feature. They are only
        # printed in endFeature().
        self.sections = {section: [] for section in self.ALL_SECTIONS}
        self.feature_not_empty = False

    def _endProtectComment(self, protect_str, protect_directive='#ifdef'):
        if protect_directive is None or protect_str is None:
            raise RuntimeError('Should not call in here without something to protect')

        # Do not put comments after #endif closing blocks if this is not set
        if not self.genOpts.conventions.protectProtoComment:
            return ''
        elif 'ifdef' in protect_directive:
            return f' /* {protect_str} */'
        else:
            return f' /* !{protect_str} */'

    def endFeature(self):
        "Actually write the interface to the output file."
        # C-specific
        if self.emit:
            if self.feature_not_empty:
                if self.genOpts is None:
                    raise MissingGeneratorOptionsError()
                if self.genOpts.conventions is None:
                    raise MissingGeneratorOptionsConventionsError()
                is_core = self.featureName and self.featureName.startswith(f"{self.conventions.api_prefix}VERSION_")
                if self.genOpts.conventions.writeFeature(self.featureName, self.featureExtraProtect, self.genOpts.filename):
                    self.newline()
                    if self.genOpts.protectFeature:
                        write('#ifndef', self.featureName, file=self.outFile)

                    # If type declarations are needed by other features based on
                    # this one, it may be necessary to suppress the ExtraProtect,
                    # or move it below the 'for section...' loop.
                    if self.featureExtraProtect is not None:
                        write('#ifdef', self.featureExtraProtect, file=self.outFile)
                    self.newline()

                    # Generate warning of possible use in IDEs
                    write(f'// {self.featureName} is a preprocessor guard. Do not pass it to API calls.', file=self.outFile)
                    write('#define', self.featureName, '1', file=self.outFile)
                    for section in self.TYPE_SECTIONS:
                        contents = self.sections[section]
                        if contents:
                            write('\n'.join(contents), file=self.outFile)

                    if self.genOpts.genFuncPointers and self.sections['commandPointer']:
                        write('\n'.join(self.sections['commandPointer']), file=self.outFile)
                        self.newline()

                    if self.sections['command']:
                        if self.genOpts.protectProto:
                            write(self.genOpts.protectProto,
                                  self.genOpts.protectProtoStr, file=self.outFile)
                        if self.genOpts.protectExtensionProto and not is_core:
                            write(self.genOpts.protectExtensionProto,
                                  self.genOpts.protectExtensionProtoStr, file=self.outFile)
                        write('\n'.join(self.sections['command']), end='', file=self.outFile)
                        if self.genOpts.protectExtensionProto and not is_core:
                            write('#endif' +
                                  self._endProtectComment(protect_directive=self.genOpts.protectExtensionProto,
                                                          protect_str=self.genOpts.protectExtensionProtoStr),
                                  file=self.outFile)
                        if self.genOpts.protectProto:
                            write('#endif' +
                                  self._endProtectComment(protect_directive=self.genOpts.protectProto,
                                                          protect_str=self.genOpts.protectProtoStr),
                                  file=self.outFile)
                        else:
                            self.newline()

                    if self.featureExtraProtect is not None:
                        write('#endif' +
                              self._endProtectComment(protect_str=self.featureExtraProtect),
                              file=self.outFile)

                    if self.genOpts.protectFeature:
                        write('#endif' +
                              self._endProtectComment(protect_str=self.featureName),
                              file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFeature(self)

    def appendSection(self, section, text):
        "Append a definition to the specified section"

        if section is None:
            self.logMsg('error', 'Missing section in appendSection (probably a <type> element missing its \'category\' attribute. Text:', text)
            exit(1)

        self.sections[section].append(text)
        self.feature_not_empty = True

    def genType(self, typeinfo, name, alias):
        "Generate type."
        OutputGenerator.genType(self, typeinfo, name, alias)
        typeElem = typeinfo.elem

        # Vulkan:
        # Determine the category of the type, and the type section to add
        # its definition to.
        # 'funcpointer' is added to the 'struct' section as a workaround for
        # internal issue #877, since structures and function pointer types
        # can have cross-dependencies.
        category = typeElem.get('category')
        if category == 'funcpointer':
            section = 'struct'
        else:
            section = category

        if category in ('struct', 'union'):
            # If the type is a struct type, generate it using the
            # special-purpose generator.
            self.genStruct(typeinfo, name, alias)
        else:
            if self.genOpts is None:
                raise MissingGeneratorOptionsError()

            body = self.deprecationComment(typeElem)

            # OpenXR: this section was not under 'else:' previously, just fell through
            if alias:
                # If the type is an alias, just emit a typedef declaration
                body += f"typedef {alias} {name};\n"
            else:
                # Replace <apientry /> tags with an APIENTRY-style string
                # (from self.genOpts). Copy other text through unchanged.
                # If the resulting text is an empty string, do not emit it.
                body += noneStr(typeElem.text)
                for elem in typeElem:
                    if elem.tag == 'apientry':
                        body += self.genOpts.apientry + noneStr(elem.tail)
                    else:
                        body += noneStr(elem.text) + noneStr(elem.tail)
                if category == 'define' and self.misracppstyle():
                    body = body.replace("(uint32_t)", "static_cast<uint32_t>")
            if body:
                # Add extra newline after multi-line entries.
                if '\n' in body[0:-1]:
                    body += '\n'
                self.appendSection(section, body)

    def genProtectString(self, protect_str):
        """Generate protection string.

        Protection strings are the strings defining the OS/Platform/Graphics
        requirements for a given API command.  When generating the
        language header files, we need to make sure the items specific to a
        graphics API or OS platform are properly wrapped in #ifs."""
        protect_if_str = ''
        protect_end_str = ''
        if not protect_str:
            return (protect_if_str, protect_end_str)

        if ',' in protect_str:
            protect_list = protect_str.split(',')
            protect_defs = (f'defined({d})' for d in protect_list)
            protect_def_str = ' && '.join(protect_defs)
            protect_if_str = f'#if {protect_def_str}\n'
            protect_end_str = f'#endif // {protect_def_str}\n'
        else:
            protect_if_str = f'#ifdef {protect_str}\n'
            protect_end_str = f'#endif // {protect_str}\n'

        return (protect_if_str, protect_end_str)

    def typeMayAlias(self, typeName):
        if not self.may_alias:
            if self.registry is None:
                raise MissingRegistryError()
            # First time we have asked if a type may alias.
            # So, populate the set of all names of types that may.

            # Everyone with an explicit mayalias="true"
            self.may_alias = set(typeName
                                 for typeName, data in self.registry.typedict.items()
                                 if data.elem.get('mayalias') == 'true')

            # Every type mentioned in some other type's parentstruct attribute.
            polymorphic_bases = (otherType.elem.get('parentstruct')
                                 for otherType in self.registry.typedict.values())
            self.may_alias.update(set(x for x in polymorphic_bases
                                      if x is not None))
        return typeName in self.may_alias

    def genStruct(self, typeinfo, typeName, alias):
        """Generate struct (e.g. C "struct" type).

        This is a special case of the <type> tag where the contents are
        interpreted as a set of <member> tags instead of freeform C
        C type declarations. The <member> tags are just like <param>
        tags - they are a declaration of a struct or union member.
        Only simple member declarations are supported (no nested
        structs etc.)

        If alias is not None, then this struct aliases another; just
        generate a typedef of that alias."""
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)

        if self.genOpts is None:
            raise MissingGeneratorOptionsError()

        typeElem = typeinfo.elem
        body = self.deprecationComment(typeElem)

        if alias:
            body += f"typedef {alias} {typeName};\n"
        else:
            (protect_begin, protect_end) = self.genProtectString(typeElem.get('protect'))
            if protect_begin:
                body += protect_begin

            if self.genOpts.genStructExtendsComment:
                structextends = typeElem.get('structextends')
                body += f"// {typeName} extends {structextends}\n" if structextends else ''

            body += f"typedef {typeElem.get('category')}"

            # This is an OpenXR-specific alternative where aliasing refers
            # to an inheritance hierarchy of types rather than C-level type
            # aliases.
            if self.genOpts.genAliasMacro and self.typeMayAlias(typeName):
                body += f" {self.genOpts.aliasMacro}"

            body += f" {typeName} {{\n"

            targetLen = self.getMaxCParamTypeLength(typeinfo)
            for member in typeElem.findall('.//member'):
                body += self.deprecationComment(member, indent = 4)
                body += self.makeCParamDecl(member, targetLen + 4)
                body += ';\n'
            body += f"}} {typeName};\n"
            if protect_end:
                body += protect_end

        self.appendSection('struct', body)

    def genGroup(self, groupinfo, groupName, alias=None):
        """Generate groups (e.g. C "enum" type).

        These are concatenated together with other types.

        If alias is not None, it is the name of another group type
        which aliases this type; just generate that alias."""
        OutputGenerator.genGroup(self, groupinfo, groupName, alias)
        groupElem = groupinfo.elem

        # After either enumerated type or alias paths, add the declaration
        # to the appropriate section for the group being defined.
        if groupElem.get('type') == 'bitmask':
            section = 'bitmask'
        else:
            section = 'group'

        if alias:
            # If the group name is aliased, just emit a typedef declaration
            # for the alias.
            body = f"typedef {alias} {groupName};\n"
            self.appendSection(section, body)
        else:
            if self.genOpts is None:
                raise MissingGeneratorOptionsError()
            (section, body) = self.buildEnumCDecl(self.genOpts.genEnumBeginEndRange, groupinfo, groupName)
            self.appendSection(section, f"\n{body}")

    def genEnum(self, enuminfo, name, alias):
        """Generate the C declaration for a constant (a single <enum> value).

        <enum> tags may specify their values in several ways, but are usually
        just integers."""

        OutputGenerator.genEnum(self, enuminfo, name, alias)

        body = self.deprecationComment(enuminfo.elem)
        body += self.buildConstantCDecl(enuminfo, name, alias)
        self.appendSection('enum', body)

    def genCmd(self, cmdinfo, name, alias):
        "Command generation"
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        # if alias:
        #     prefix = '// ' + name + ' is an alias of command ' + alias + '\n'
        # else:
        #     prefix = ''
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()

        prefix = ''
        decls = self.makeCDecls(cmdinfo.elem)
        self.appendSection('command', f"{prefix + decls[0]}\n")
        if self.genOpts.genFuncPointers:
            self.appendSection('commandPointer', decls[1])

    def misracstyle(self):
        return self.genOpts.misracstyle

    def misracppstyle(self):
        return self.genOpts.misracppstyle
