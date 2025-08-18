#!/usr/bin/env python3 -i
#
# Copyright 2013-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0
"""Base class for source/header/doc generators, as well as some utility functions."""

from __future__ import unicode_literals

import io
import os
import pdb
import re
import shutil
import sys
import tempfile
try:
    from pathlib import Path
except ImportError:
    from pathlib2 import Path  # type: ignore

from spec_tools.util import getElemName, getElemType


def write(*args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    end = kwargs.pop('end', '\n')
    file.write(' '.join(str(arg) for arg in args))
    file.write(end)


def noneStr(s):
    """Return string argument, or "" if argument is None.

    Used in converting etree Elements into text.
    s - string to convert"""
    if s:
        return s
    return ""


def enquote(s):
    """Return string argument with surrounding quotes,
      for serialization into Python code."""
    if s:
        if isinstance(s, str):
            return f"'{s}'"
        else:
            return s
    return None


def regSortCategoryKey(feature):
    """Sort key for regSortFeatures.
    Sorts by category of the feature name string:

    - Core API features (those defined with a `<feature>` tag)
        - (sort VKSC after VK - this is Vulkan-specific)
    - ARB/KHR/OES (Khronos extensions)
    - other       (EXT/vendor extensions)"""

    if feature.elem.tag == 'feature':
        if feature.name.startswith('VKSC'):
            return 0.5
        else:
            return 0

    if feature.category.upper() in ('ARB', 'KHR', 'OES'):
        return 1

    return 2


def regSortOrderKey(feature):
    """Sort key for regSortFeatures - key is the sortorder attribute."""

    return feature.sortorder


def regSortNameKey(feature):
    """Sort key for regSortFeatures - key is the extension name."""

    return feature.name


def regSortFeatureVersionKey(feature):
    """Sort key for regSortFeatures - key is the feature version.
    `<extension>` elements all have version number 0."""

    return float(feature.versionNumber)


def regSortExtensionNumberKey(feature):
    """Sort key for regSortFeatures - key is the extension number.
    `<feature>` elements all have extension number 0."""

    return int(feature.number)


def regSortFeatures(featureList):
    """Default sort procedure for features.

    - Sorts by explicit sort order (default 0) relative to other features
    - then by feature category ('feature' or 'extension'),
    - then by version number (for features)
    - then by extension number (for extensions)"""
    featureList.sort(key=regSortExtensionNumberKey)
    featureList.sort(key=regSortFeatureVersionKey)
    featureList.sort(key=regSortCategoryKey)
    featureList.sort(key=regSortOrderKey)


class MissingGeneratorOptionsError(RuntimeError):
    """Error raised when a Generator tries to do something that requires GeneratorOptions but it is None."""

    def __init__(self, msg=None):
        full_msg = 'Missing generator options object self.genOpts'
        if msg:
            full_msg += f": {msg}"
        super().__init__(full_msg)


class MissingRegistryError(RuntimeError):
    """Error raised when a Generator tries to do something that requires a Registry object but it is None."""

    def __init__(self, msg=None):
        full_msg = 'Missing Registry object self.registry'
        if msg:
            full_msg += f": {msg}"
        super().__init__(full_msg)


class MissingGeneratorOptionsConventionsError(RuntimeError):
    """Error raised when a Generator tries to do something that requires a Conventions object but it is None."""

    def __init__(self, msg=None):
        full_msg = 'Missing Conventions object self.genOpts.conventions'
        if msg:
            full_msg += f": {msg}"
        super().__init__(full_msg)


class GeneratorOptions:
    """Base class for options used during header/documentation production.

    These options are target language independent, and used by
    Registry.apiGen() and by base OutputGenerator objects."""

    def __init__(self,
                 conventions=None,
                 filename=None,
                 directory='.',
                 genpath=None,
                 apiname=None,
                 mergeApiNames=None,
                 profile=None,
                 versions='.*',
                 emitversions='.*',
                 defaultExtensions=None,
                 addExtensions=None,
                 removeExtensions=None,
                 emitExtensions=None,
                 emitSpirv=None,
                 emitFormats=None,
                 reparentEnums=True,
                 sortProcedure=regSortFeatures,
                 requireCommandAliases=False,
                 requireDepends=True,
                ):
        """Constructor.

        Arguments:

        - conventions - may be mandatory for some generators:
        an object that implements ConventionsBase
        - filename - basename of file to generate, or None to write to stdout.
        - directory - directory in which to generate filename
        - genpath - path to previously generated files, such as apimap.py
        - apiname - string matching `<api>` 'apiname' attribute, e.g. 'gl'.
        - mergeApiNames - If not None, a comma separated list of API names
          to merge into the API specified by 'apiname'
        - profile - string specifying API profile , e.g. 'core', or None.
        - versions - regex matching API versions to process interfaces for.
        Normally `'.*'` or `'[0-9][.][0-9]'` to match all defined versions.
        - emitversions - regex matching API versions to actually emit
        interfaces for (though all requested versions are considered
        when deciding which interfaces to generate). For GL 4.3 glext.h,
        this might be `'1[.][2-5]|[2-4][.][0-9]'`.
        - defaultExtensions - If not None, a string which must in its
        entirety match the pattern in the "supported" attribute of
        the `<extension>`. Defaults to None. Usually the same as apiname.
        - addExtensions - regex matching names of additional extensions
        to include. Defaults to None.
        - removeExtensions - regex matching names of extensions to
        remove (after defaultExtensions and addExtensions). Defaults
        to None.
        - emitExtensions - regex matching names of extensions to actually emit
        interfaces for (though all requested versions are considered when
        deciding which interfaces to generate). Defaults to None.
        - emitSpirv - regex matching names of extensions and capabilities
        to actually emit interfaces for.
        - emitFormats - regex matching names of formats to actually emit
        interfaces for.
        - reparentEnums - move <enum> elements which extend an enumerated
        type from <feature> or <extension> elements to the target <enums>
        element. This is required for almost all purposes, but the
        InterfaceGenerator relies on the list of interfaces in the <feature>
        or <extension> being complete. Defaults to True.
        - sortProcedure - takes a list of FeatureInfo objects and sorts
        them in place to a preferred order in the generated output.
        - requireCommandAliases - if True, treat command aliases
        as required dependencies.
        - requireDepends - whether to follow API dependencies when emitting
        APIs.

        Default is
          - core API versions
          - Khronos (ARB/KHR/OES) extensions
          - All other extensions
          - By core API version number or extension number in each group.

        The regex patterns can be None or empty, in which case they match
        nothing."""
        self.conventions = conventions
        """may be mandatory for some generators:
        an object that implements ConventionsBase"""

        self.filename = filename
        "basename of file to generate, or None to write to stdout."

        self.genpath = genpath
        """path to previously generated files, such as apimap.py"""

        self.directory = directory
        "directory in which to generate filename"

        self.apiname = apiname
        "string matching `<api>` 'apiname' attribute, e.g. 'gl'."

        self.mergeApiNames = mergeApiNames
        "comma separated list of API names to merge into the API specified by 'apiname'"

        self.profile = profile
        "string specifying API profile , e.g. 'core', or None."

        self.versions = self.emptyRegex(versions)
        """regex matching API versions to process interfaces for.
        Normally `'.*'` or `'[0-9][.][0-9]'` to match all defined versions."""

        self.emitversions = self.emptyRegex(emitversions)
        """regex matching API versions to actually emit
        interfaces for (though all requested versions are considered
        when deciding which interfaces to generate). For GL 4.3 glext.h,
        this might be `'1[.][2-5]|[2-4][.][0-9]'`."""

        self.defaultExtensions = defaultExtensions
        """If not None, a string which must in its
        entirety match the pattern in the "supported" attribute of
        the `<extension>`. Defaults to None. Usually the same as apiname."""

        self.addExtensions = self.emptyRegex(addExtensions)
        """regex matching names of additional extensions
        to include. Defaults to None."""

        self.removeExtensions = self.emptyRegex(removeExtensions)
        """regex matching names of extensions to
        remove (after defaultExtensions and addExtensions). Defaults
        to None."""

        self.emitExtensions = self.emptyRegex(emitExtensions)
        """regex matching names of extensions to actually emit
        interfaces for (though all requested versions are considered when
        deciding which interfaces to generate)."""

        self.emitSpirv = self.emptyRegex(emitSpirv)
        """regex matching names of extensions and capabilities
        to actually emit interfaces for."""

        self.emitFormats = self.emptyRegex(emitFormats)
        """regex matching names of formats
        to actually emit interfaces for."""

        self.reparentEnums = reparentEnums
        """boolean specifying whether to remove <enum> elements from
        <feature> or <extension> when extending an <enums> type."""

        self.sortProcedure = sortProcedure
        """takes a list of FeatureInfo objects and sorts
        them in place to a preferred order in the generated output.
        Default is core API versions, ARB/KHR/OES extensions, all
        other extensions, alphabetically within each group."""

        self.codeGenerator = False
        """True if this generator makes compilable code"""

        self.registry = None
        """Populated later with the registry object."""

        self.requireCommandAliases = requireCommandAliases
        """True if alias= attributes of <command> tags are transitively
        required."""

        self.requireDepends = requireDepends
        """True if dependencies of API tags are transitively required."""

    def emptyRegex(self, pat):
        """Substitute a regular expression which matches no version
        or extension names for None or the empty string."""
        if not pat:
            return '_nomatch_^'

        return pat


class OutputGenerator:
    """Generate specified API interfaces in a specific style, such as a C header.

    Base class for generating API interfaces.
    Manages basic logic, logging, and output file control.
    Derived classes actually generate formatted output.
    """

    # categoryToPath - map XML 'category' to include file directory name
    categoryToPath = {
        'bitmask': 'flags',
        'enum': 'enums',
        'funcpointer': 'funcpointers',
        'handle': 'handles',
        'define': 'defines',
        'basetype': 'basetypes',
    }

    def breakName(self, name, msg):
        """Break into debugger if this is a special name"""

        # List of string names to break on
        bad = (
        )

        if name in bad and True:
            print(f'breakName {name}: {msg}')
            pdb.set_trace()

    def __init__(self, errFile=sys.stderr, warnFile=sys.stderr, diagFile=sys.stdout):
        """Constructor

        - errFile, warnFile, diagFile - file handles to write errors,
          warnings, diagnostics to. May be None to not write."""
        self.outFile = None
        self.errFile = errFile
        self.warnFile = warnFile
        self.diagFile = diagFile
        # Internal state
        self.featureName = None
        """The current feature name being generated."""

        self.genOpts = None
        """The GeneratorOptions subclass instance."""

        self.registry = None
        """The specification registry object."""

        self.featureDictionary = {}
        """The dictionary of dictionaries of API features."""

        # Used for extension enum value generation
        self.extBase = 1000000000
        self.extBlockSize = 1000
        self.madeDirs = {}

        # API dictionary, which may be loaded by the beginFile method of
        # derived generators.
        self.apidict = None

        # File suffix for generated files, set in beginFile below.
        self.file_suffix = ''

    def logMsg(self, level, *args):
        """Write a message of different categories to different
        destinations.

        - `level`
          - 'diag' (diagnostic, voluminous)
          - 'warn' (warning)
          - 'error' (fatal error - raises exception after logging)

        - `*args` - print()-style arguments to direct to corresponding log"""
        if level == 'error':
            strfile = io.StringIO()
            write('ERROR:', *args, file=strfile)
            if self.errFile is not None:
                write(strfile.getvalue(), file=self.errFile)
            raise UserWarning(strfile.getvalue())
        elif level == 'warn':
            if self.warnFile is not None:
                write('WARNING:', *args, file=self.warnFile)
        elif level == 'diag':
            if self.diagFile is not None:
                write('DIAG:', *args, file=self.diagFile)
        else:
            raise UserWarning(
                f"*** FATAL ERROR in Generator.logMsg: unknown level:{level}")

    def enumToValue(self, elem, needsNum, bitwidth = 32,
                    forceSuffix = False, parent_for_alias_dereference=None):
        """Parse and convert an `<enum>` tag into a value.

        - elem - <enum> Element
        - needsNum - generate a numeric representation of the element value
        - bitwidth - size of the numeric representation in bits (32 or 64)
        - forceSuffix - if True, always use a 'U' / 'ULL' suffix on integers
        - parent_for_alias_dereference - if not None, an Element containing
          the parent of elem, used to look for elements this is an alias of

        Returns a list:

        - first element - integer representation of the value, or None
          if needsNum is False. The value must be a legal number
          if needsNum is True.
        - second element - string representation of the value

        There are several possible representations of values.

        - A 'value' attribute simply contains the value.
        - A 'bitpos' attribute defines a value by specifying the bit
          position which is set in that value.
        - An 'offset','extbase','extends' triplet specifies a value
          as an offset to a base value defined by the specified
          'extbase' extension name, which is then cast to the
          typename specified by 'extends'. This requires probing
          the registry database, and imbeds knowledge of the
          API extension enum scheme in this function.
        - An 'alias' attribute contains the name of another enum
          which this is an alias of. The other enum must be
          declared first when emitting this enum."""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        if self.genOpts.conventions is None:
            raise MissingGeneratorOptionsConventionsError()

        name = elem.get('name')
        numVal = None
        if 'value' in elem.keys():
            value = elem.get('value')
            # print('About to translate value =', value, 'type =', type(value))
            if needsNum:
                numVal = int(value, 0)
            # If there is a non-integer, numeric 'type' attribute (e.g. 'u' or
            # 'ull'), append it to the string value.
            # t = enuminfo.elem.get('type')
            # if t is not None and t != '' and t != 'i' and t != 's':
            #     value += enuminfo.type
            if forceSuffix:
              if bitwidth == 64:
                value = f"{value}ULL"
              else:
                value = f"{value}U"
            self.logMsg('diag', 'Enum', name, '-> value [', numVal, ',', value, ']')
            return [numVal, value]
        if 'bitpos' in elem.keys():
            value = elem.get('bitpos')
            bitpos = int(value, 0)
            numVal = 1 << bitpos
            value = f'0x{numVal:08x}'
            if bitwidth == 64 or bitpos >= 32:
              value = f"{value}ULL"
            elif forceSuffix:
              value = f"{value}U"
            self.logMsg('diag', 'Enum', name, '-> bitpos [', numVal, ',', value, ']')
            return [numVal, value]
        if 'offset' in elem.keys():
            # Obtain values in the mapping from the attributes
            enumNegative = False
            offset = int(elem.get('offset'), 0)
            extnumber = int(elem.get('extnumber'), 0)
            extends = elem.get('extends')
            if 'dir' in elem.keys():
                enumNegative = True
            self.logMsg('diag', 'Enum', name, 'offset =', offset,
                        'extnumber =', extnumber, 'extends =', extends,
                        'enumNegative =', enumNegative)
            # Now determine the actual enumerant value, as defined
            # in the "Layers and Extensions" appendix of the spec.
            numVal = self.extBase + (extnumber - 1) * self.extBlockSize + offset
            if enumNegative:
                numVal *= -1
            value = '%d' % numVal
            # More logic needed!
            self.logMsg('diag', 'Enum', name, '-> offset [', numVal, ',', value, ']')
            return [numVal, value]
        if 'alias' in elem.keys():
            alias_of = elem.get('alias')
            if parent_for_alias_dereference is None:
                return (None, alias_of)
            siblings = parent_for_alias_dereference.findall('enum')
            for sib in siblings:
                sib_name = sib.get('name')
                if sib_name == alias_of:
                    return self.enumToValue(sib, needsNum)
            raise RuntimeError("Could not find the aliased enum value")
        return [None, None]

    def checkDuplicateEnums(self, enums):
        """Check enumerated values for duplicates.

        -  enums - list of `<enum>` Elements

        returns the list with duplicates stripped"""
        # Dictionaries indexed by name and numeric value.
        # Entries are [ Element, numVal, strVal ] matching name or value

        nameMap = {}
        valueMap = {}

        stripped = []
        for elem in enums:
            name = elem.get('name')
            (numVal, strVal) = self.enumToValue(elem, True)

            if name in nameMap:
                # Duplicate name found; check values
                (name2, numVal2, strVal2) = nameMap[name]

                # Duplicate enum values for the same name are benign. This
                # happens when defining the same enum conditionally in
                # several extension blocks.
                if (strVal2 == strVal or (numVal is not None
                                          and numVal == numVal2)):
                    True
                    # self.logMsg('info', 'checkDuplicateEnums: Duplicate enum (' + name +
                    #             ') found with the same value:' + strVal)
                else:
                    self.logMsg('warn', 'checkDuplicateEnums: Duplicate enum (' + name
                                + ') found with different values:' + strVal
                                + ' and ' + strVal2)

                # Do not add the duplicate to the returned list
                continue
            elif numVal in valueMap:
                # Duplicate value found (such as an alias); report it, but
                # still add this enum to the list.
                (name2, numVal2, strVal2) = valueMap[numVal]

                msg = 'Two enums found with the same value: {} = {} = {}'.format(
                    name, name2.get('name'), strVal)
                self.logMsg('error', msg)

            # Track this enum to detect followon duplicates
            nameMap[name] = [elem, numVal, strVal]
            if numVal is not None:
                valueMap[numVal] = [elem, numVal, strVal]

            # Add this enum to the list
            stripped.append(elem)

        # Return the list
        return stripped

    def misracstyle(self):
        return False;

    def misracppstyle(self):
        return False;

    def deprecationComment(self, elem, indent = 0):
        """If an API element is marked deprecated, return a brief comment
           describing why.
           Otherwise, return an empty string.

          - elem - Element of the API.
            API name is determined depending on the element tag.
          - indent - number of spaces to indent the comment"""

        reason = elem.get('deprecated')

        # This is almost always the path taken.
        if reason == None:
            return ''

        # There is actually a deprecated attribute.
        padding = indent * ' '

        # Determine the API name.
        if elem.tag == 'member' or elem.tag == 'param':
            name = elem.find('.//name').text
        else:
            name = elem.get('name')

        if reason == 'aliased':
            return f'{padding}// {name} is a deprecated alias\n'
        elif reason == 'ignored':
            return f'{padding}// {name} is deprecated and should not be used\n'
        elif reason == 'true':
            return f'{padding}// {name} is deprecated, but no reason was given in the API XML\n'
        else:
            # This can be caught by schema validation
            self.logMsg('error', f"{name} has an unknown deprecation attribute value '{reason}'")
            exit(1)

    def buildEnumCDecl(self, expand, groupinfo, groupName):
        """Generate the C declaration for an enum"""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        if self.genOpts.conventions is None:
            raise MissingGeneratorOptionsConventionsError()

        groupElem = groupinfo.elem

        # Determine the required bit width for the enum group.
        # 32 is the default, which generates C enum types for the values.
        bitwidth = 32

        # If the constFlagBits preference is set, 64 is the default for bitmasks
        if self.genOpts.conventions.constFlagBits and groupElem.get('type') == 'bitmask':
            bitwidth = 64

        # Check for an explicitly defined bitwidth, which will override any defaults.
        if groupElem.get('bitwidth'):
            try:
                bitwidth = int(groupElem.get('bitwidth'))
            except ValueError as ve:
                self.logMsg('error', 'Invalid value for bitwidth attribute (', groupElem.get('bitwidth'), ') for ', groupName, ' - must be an integer value\n')
                exit(1)

        usebitmask = False
        usedefine = False

        # Bitmask flags can be generated as either "static const uint{32,64}_t" values,
        # or as 32-bit C enums. 64-bit types must use uint64_t values.
        if groupElem.get('type') == 'bitmask':
            if bitwidth > 32 or self.misracppstyle():
                usebitmask = True
            if self.misracstyle():
                usedefine = True

        if usedefine or usebitmask:
            # Validate the bitwidth and generate values appropriately
            if bitwidth > 64:
                self.logMsg('error', 'Invalid value for bitwidth attribute (', groupElem.get('bitwidth'), ') for bitmask type ', groupName, ' - must be less than or equal to 64\n')
                exit(1)
            else:
                return self.buildEnumCDecl_BitmaskOrDefine(groupinfo, groupName, bitwidth, usedefine)
        else:
            # Validate the bitwidth and generate values appropriately
            if bitwidth > 32:
                self.logMsg('error', 'Invalid value for bitwidth attribute (', groupElem.get('bitwidth'), ') for enum type ', groupName, ' - must be less than or equal to 32\n')
                exit(1)
            else:
                return self.buildEnumCDecl_Enum(expand, groupinfo, groupName)

    def buildEnumCDecl_BitmaskOrDefine(self, groupinfo, groupName, bitwidth, usedefine):
        """Generate the C declaration for an "enum" that is actually a
        set of flag bits"""
        groupElem = groupinfo.elem
        flagTypeName = groupElem.get('name')

        # Prefix
        body = f"// Flag bits for {flagTypeName}\n"

        if bitwidth == 64:
            body += f"typedef VkFlags64 {flagTypeName};\n";
        else:
            body += f"typedef VkFlags {flagTypeName};\n";

        # Maximum allowable value for a flag (unsigned 64-bit integer)
        maxValidValue = 2**(64) - 1
        minValidValue = 0

        # Get a list of nested 'enum' tags.
        enums = groupElem.findall('enum')

        # Check for and report duplicates, and return a list with them
        # removed.
        enums = self.checkDuplicateEnums(enums)

        # Accumulate non-numeric enumerant values separately and append
        # them following the numeric values, to allow for aliases.
        # NOTE: this does not do a topological sort yet, so aliases of
        # aliases can still get in the wrong order.
        aliasText = ''

        # Loop over the nested 'enum' tags.
        for elem in enums:
            # Convert the value to an integer and use that to track min/max.
            # Values of form -(number) are accepted but nothing more complex.
            # Should catch exceptions here for more complex constructs. Not yet.
            (numVal, strVal) = self.enumToValue(elem, True, bitwidth, True)
            name = elem.get('name')

            # Range check for the enum value
            if numVal is not None and (numVal > maxValidValue or numVal < minValidValue):
                self.logMsg('error', 'Allowable range for flag types in C is [', minValidValue, ',', maxValidValue, '], but', name, 'flag has a value outside of this (', strVal, ')\n')
                exit(1)

            decl = self.genRequirements(name, mustBeFound = False)

            if self.isEnumRequired(elem):
                protect = elem.get('protect')
                if protect is not None:
                    body += f'#ifdef {protect}\n'

                body += self.deprecationComment(elem, indent = 0)

                if usedefine:
                    decl += f"#define {name} {strVal}\n"
                elif self.misracppstyle():
                    decl += f"static constexpr {flagTypeName} {name} {{{strVal}}};\n"
                else:
                    # Some C compilers only allow initializing a 'static const' variable with a literal value.
                    # So initializing an alias from another 'static const' value would fail to compile.
                    # Work around this by chasing the aliases to get the actual value.
                    while numVal is None:
                        alias = self.registry.tree.find(f"enums/enum[@name='{strVal}']")
                        if alias is not None:
                            (numVal, strVal) = self.enumToValue(alias, True, bitwidth, True)
                        else:
                            self.logMsg('error', f'No such alias {strVal} for enum {name}')
                    decl += f"static const {flagTypeName} {name} = {strVal};\n"

                if numVal is not None:
                    body += decl
                else:
                    aliasText += decl

                if protect is not None:
                    body += '#endif\n'

        # Now append the non-numeric enumerant values
        body += aliasText

        # Postfix

        return ("bitmask", body)

    def buildEnumCDecl_Enum(self, expand, groupinfo, groupName):
        """Generate the C declaration for an enumerated type"""
        groupElem = groupinfo.elem

        # Break the group name into prefix and suffix portions for range
        # enum generation
        expandName = re.sub(r'([0-9]+|[a-z_])([A-Z0-9])', r'\1_\2', groupName).upper()
        expandPrefix = expandName
        expandSuffix = ''
        expandSuffixMatch = re.search(r'[A-Z][A-Z]+$', groupName)
        if expandSuffixMatch:
            expandSuffix = f"_{expandSuffixMatch.group()}"
            # Strip off the suffix from the prefix
            expandPrefix = expandName.rsplit(expandSuffix, 1)[0]

        # Prefix
        body = ["typedef enum %s {" % groupName]

        # @@ Should use the type="bitmask" attribute instead
        isEnum = ('FLAG_BITS' not in expandPrefix)

        # Allowable range for a C enum - which is that of a signed 32-bit integer
        maxValidValue = 2**(32 - 1) - 1
        minValidValue = (maxValidValue * -1) - 1

        # Get a list of nested 'enum' tags.
        enums = groupElem.findall('enum')

        # Check for and report duplicates, and return a list with them
        # removed.
        enums = self.checkDuplicateEnums(enums)

        # Loop over the nested 'enum' tags. Keep track of the minimum and
        # maximum numeric values, if they can be determined; but only for
        # core API enumerants, not extension enumerants. This is inferred
        # by looking for 'extends' attributes.
        minName = None

        # Accumulate non-numeric enumerant values separately and append
        # them following the numeric values, to allow for aliases.
        # NOTE: this does not do a topological sort yet, so aliases of
        # aliases can still get in the wrong order.
        aliasText = []

        maxName = None
        minValue = None
        maxValue = None
        for elem in enums:
            # Convert the value to an integer and use that to track min/max.
            # Values of form -(number) are accepted but nothing more complex.
            # Should catch exceptions here for more complex constructs. Not yet.
            (numVal, strVal) = self.enumToValue(elem, True)
            name = elem.get('name')

            # Extension enumerants are only included if they are required
            if self.isEnumRequired(elem):
                decl = ''

                protect = elem.get('protect')
                if protect is not None:
                    decl += f'#ifdef {protect}\n'


                decl += self.genRequirements(name, mustBeFound = False, indent = 2)
                decl += self.deprecationComment(elem, indent = 2)
                decl += f'    {name} = {strVal},'

                if protect is not None:
                    decl += '\n#endif'

                if numVal is not None:
                    body.append(decl)
                else:
                    aliasText.append(decl)

            # Range check for the enum value
            if numVal is not None and (numVal > maxValidValue or numVal < minValidValue):
                self.logMsg('error', 'Allowable range for C enum types is [', minValidValue, ',', maxValidValue, '], but', name, 'has a value outside of this (', strVal, ')\n')
                exit(1)

            # Do not track min/max for non-numbers (numVal is None)
            if isEnum and numVal is not None and elem.get('extends') is None:
                if minName is None:
                    minName = maxName = name
                    minValue = maxValue = numVal
                elif minValue is None or numVal < minValue:
                    minName = name
                    minValue = numVal
                elif maxValue is None or numVal > maxValue:
                    maxName = name
                    maxValue = numVal

        # Now append the non-numeric enumerant values
        body.extend(aliasText)

        # Generate min/max value tokens - legacy use case.
        if isEnum and expand:
            body.extend((f'    {expandPrefix}_BEGIN_RANGE{expandSuffix} = {minName},',
                         f'    {expandPrefix}_END_RANGE{expandSuffix} = {maxName},',
                         f'    {expandPrefix}_RANGE_SIZE{expandSuffix} = ({maxName} - {minName} + 1),'))

        # Generate a range-padding value to ensure the enum is 32 bits, but
        # only in code generators, so it does not appear in documentation
        if (self.genOpts.codeGenerator or
            self.conventions.generate_max_enum_in_docs):
            body.append(f'    {expandPrefix}_MAX_ENUM{expandSuffix} = 0x7FFFFFFF')

        # Postfix
        body.append("} %s;" % groupName)

        # Determine appropriate section for this declaration
        if groupElem.get('type') == 'bitmask':
            section = 'bitmask'
        else:
            section = 'group'

        return (section, '\n'.join(body))

    def buildConstantCDecl(self, enuminfo, name, alias):
        """Generate the C declaration for a constant (a single <enum>
        value).

        <enum> tags may specify their values in several ways, but are
        usually just integers or floating-point numbers."""

        (_, strVal) = self.enumToValue(enuminfo.elem, False)

        if self.misracppstyle() and enuminfo.elem.get('type') and not alias:
            # Generate e.g.: static constexpr uint32_t x = ~static_cast<uint32_t>(1U);
            # This appeases MISRA "underlying type" rules.
            typeStr = enuminfo.elem.get('type');
            invert = '~' in strVal
            number = strVal.strip("()~UL")
            if typeStr != "float":
                number += 'U'
            strVal = "~" if invert else ""
            strVal += f"static_cast<{typeStr}>({number})"
            body = f"static constexpr {typeStr.ljust(9)}{name.ljust(33)} {{{strVal}}};"
        elif enuminfo.elem.get('type') and not alias:
            # Generate e.g.: #define x (~0ULL)
            typeStr = enuminfo.elem.get('type');
            invert = '~' in strVal
            paren = '(' in strVal
            number = strVal.strip("()~UL")
            if typeStr != "float":
                if typeStr == "uint64_t":
                    number += 'ULL'
                else:
                    number += 'U'
            strVal = "~" if invert else ""
            strVal += number
            if paren:
                strVal = f"({strVal})";
            body = f"#define {name.ljust(33)} {strVal}";
        else:
            body = f"#define {name.ljust(33)} {strVal}"

        return body

    def makeDir(self, path):
        """Create a directory, if not already done.

        Generally called from derived generators creating hierarchies."""
        self.logMsg('diag', 'OutputGenerator::makeDir(', path, ')')
        if path not in self.madeDirs:
            # This can get race conditions with multiple writers, see
            # https://stackoverflow.com/questions/273192/
            if not os.path.exists(path):
                os.makedirs(path)
            self.madeDirs[path] = None

    def beginFile(self, genOpts):
        """Start a new interface file

        - genOpts - GeneratorOptions controlling what is generated and how"""

        self.genOpts = genOpts
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        if self.genOpts.conventions is None:
            raise MissingGeneratorOptionsConventionsError()
        self.should_insert_may_alias_macro = \
            self.genOpts.conventions.should_insert_may_alias_macro(self.genOpts)
        self.file_suffix = self.genOpts.conventions.file_suffix

        # Try to import the API dictionary, apimap.py, if it exists. Nothing
        # in apimap.py cannot be extracted directly from the XML, and in the
        # future we should do that.
        if self.genOpts.genpath is not None:
            try:
                sys.path.insert(0, self.genOpts.genpath)
                import apimap
                self.apidict = apimap
            except ImportError:
                self.apidict = None

        self.conventions = genOpts.conventions

        # Open a temporary file for accumulating output.
        if self.genOpts.filename is not None:
            self.outFile = tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', newline='\n', delete=False)
        else:
            self.outFile = sys.stdout

    def endFile(self):
        if self.errFile:
            self.errFile.flush()
        if self.warnFile:
            self.warnFile.flush()
        if self.diagFile:
            self.diagFile.flush()
        if self.outFile:
            self.outFile.flush()
            if self.outFile != sys.stdout and self.outFile != sys.stderr:
                self.outFile.close()

            if self.genOpts is None:
                raise MissingGeneratorOptionsError()

            # On successfully generating output, move the temporary file to the
            # target file.
            if self.genOpts.filename is not None:
                directory = Path(self.genOpts.directory)
                if sys.platform == 'win32':
                    if not Path.exists(directory):
                        os.makedirs(directory)
                shutil.copy(self.outFile.name, directory / self.genOpts.filename)
                os.remove(self.outFile.name)
        self.genOpts = None

    def beginFeature(self, interface, emit):
        """Write interface for a feature and tag generated features as having been done.

        - interface - element for the `<version>` / `<extension>` to generate
        - emit - actually write to the header only when True"""
        self.emit = emit
        self.featureName = interface.get('name')
        # If there is an additional 'protect' attribute in the feature, save it
        self.featureExtraProtect = interface.get('protect')

    def endFeature(self):
        """Finish an interface file, closing it when done.

        Derived classes responsible for emitting feature"""
        self.featureName = None
        self.featureExtraProtect = None

    def genRequirements(self, name, mustBeFound = True, indent = 0):
        """Generate text showing what core versions and extensions introduce
        an API. This exists in the base Generator class because it is used by
        the shared enumerant-generating interfaces (buildEnumCDecl, etc.).
        Here it returns an empty string for most generators, but can be
        overridden by e.g. DocGenerator.

        - name - name of the API
        - mustBeFound - If True, when requirements for 'name' cannot be
          determined, a warning comment is generated.
        """

        return ''

    def validateFeature(self, featureType, featureName):
        """Validate we are generating something only inside a `<feature>` tag"""
        if self.featureName is None:
            raise UserWarning('Attempt to generate', featureType,
                              featureName, 'when not in feature')

    def genType(self, typeinfo, name, alias):
        """Generate interface for a type

        - typeinfo - TypeInfo for a type

        Extend to generate as desired in your derived class."""
        self.validateFeature('type', name)

    def genStruct(self, typeinfo, typeName, alias):
        """Generate interface for a C "struct" type.

        - typeinfo - TypeInfo for a type interpreted as a struct

        Extend to generate as desired in your derived class."""
        self.validateFeature('struct', typeName)

        # The mixed-mode <member> tags may contain no-op <comment> tags.
        # It is convenient to remove them here where all output generators
        # will benefit.
        for member in typeinfo.elem.findall('.//member'):
            for comment in member.findall('comment'):
                member.remove(comment)

    def genGroup(self, groupinfo, groupName, alias):
        """Generate interface for a group of enums (C "enum")

        - groupinfo - GroupInfo for a group.

        Extend to generate as desired in your derived class."""

        self.validateFeature('group', groupName)

    def genEnum(self, enuminfo, typeName, alias):
        """Generate interface for an enum (constant).

        - enuminfo - EnumInfo for an enum
        - name - enum name

        Extend to generate as desired in your derived class."""
        self.validateFeature('enum', typeName)

    def genCmd(self, cmd, cmdinfo, alias):
        """Generate interface for a command.

        - cmdinfo - CmdInfo for a command

        Extend to generate as desired in your derived class."""
        self.validateFeature('command', cmdinfo)

    def genSpirv(self, spirv, spirvinfo, alias):
        """Generate interface for a spirv element.

        - spirvinfo - SpirvInfo for a command

        Extend to generate as desired in your derived class."""
        return

    def genFormat(self, format, formatinfo, alias):
        """Generate interface for a format element.

        - formatinfo - FormatInfo

        Extend to generate as desired in your derived class."""
        return

    def genSyncStage(self, stageinfo):
        """Generate interface for a sync stage element.

        - stageinfo - SyncStageInfo

        Extend to generate as desired in your derived class."""
        return

    def genSyncAccess(self, accessinfo):
        """Generate interface for a sync stage element.

        - accessinfo - AccessInfo

        Extend to generate as desired in your derived class."""
        return

    def genSyncPipeline(self, pipelineinfo):
        """Generate interface for a sync stage element.

        - pipelineinfo - SyncPipelineInfo

        Extend to generate as desired in your derived class."""
        return

    def makeProtoName(self, name, tail):
        """Turn a `<proto>` `<name>` into C-language prototype
        and typedef declarations for that name.

        - name - contents of `<name>` tag
        - tail - whatever text follows that tag in the Element"""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        return self.genOpts.apientry + name + tail

    def makeTypedefName(self, name, tail):
        """Make the function-pointer typedef name for a command."""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        return f"({self.genOpts.apientryp}PFN_{name}{tail})"

    def makeCParamDecl(self, param, aligncol):
        """Return a string which is an indented, formatted
        declaration for a `<param>` or `<member>` block (e.g. function parameter
        or structure/union member).

        - param - Element (`<param>` or `<member>`) to format
        - aligncol - if non-zero, attempt to align the nested `<name>` element
          at this column"""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        if self.genOpts.conventions is None:
            raise MissingGeneratorOptionsConventionsError()
        indent = '    '
        paramdecl = indent
        prefix = noneStr(param.text)

        for elem in param:
            text = noneStr(elem.text)
            tail = noneStr(elem.tail)

            if self.should_insert_may_alias_macro and self.genOpts.conventions.is_voidpointer_alias(elem.tag, text, tail):
                # OpenXR-specific macro insertion - but not in apiinc for the spec
                tail = self.genOpts.conventions.make_voidpointer_alias(tail)
            if elem.tag == 'name' and aligncol > 0:
                self.logMsg('diag', 'Aligning parameter', elem.text, 'to column', self.genOpts.alignFuncParam)
                # Align at specified column, if possible
                paramdecl = paramdecl.rstrip()
                oldLen = len(paramdecl)
                # This works around a problem where very long type names -
                # longer than the alignment column - would run into the tail
                # text.
                paramdecl = f"{paramdecl.ljust(aligncol - 1)} "
                newLen = len(paramdecl)
                self.logMsg('diag', 'Adjust length of parameter decl from', oldLen, 'to', newLen, ':', paramdecl)

            if (self.misracppstyle() and prefix.find('const ') != -1):
                # Change pointer type order from e.g. "const void *" to "void const *".
                # If the string starts with 'const', reorder it to be after the first type.
                paramdecl += f"{prefix.replace('const ', '') + text} const{tail}"
            else:
                paramdecl += prefix + text + tail

            # Clear prefix for subsequent iterations
            prefix = ''

        paramdecl = paramdecl + prefix

        if aligncol == 0:
            # Squeeze out multiple spaces other than the indentation
            paramdecl = indent + ' '.join(paramdecl.split())
        return paramdecl

    def getCParamTypeLength(self, param):
        """Return the length of the type field is an indented, formatted
        declaration for a `<param>` or `<member>` block (e.g. function parameter
        or structure/union member).

        - param - Element (`<param>` or `<member>`) to identify"""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        if self.genOpts.conventions is None:
            raise MissingGeneratorOptionsConventionsError()

        # Allow for missing <name> tag
        newLen = 0
        paramdecl = f"    {noneStr(param.text)}"
        for elem in param:
            text = noneStr(elem.text)
            tail = noneStr(elem.tail)

            if self.should_insert_may_alias_macro and self.genOpts.conventions.is_voidpointer_alias(elem.tag, text, tail):
                # OpenXR-specific macro insertion
                tail = self.genOpts.conventions.make_voidpointer_alias(tail)
            if elem.tag == 'name':
                # Align at specified column, if possible
                newLen = len(paramdecl.rstrip())
                self.logMsg('diag', 'Identifying length of', elem.text, 'as', newLen)
            paramdecl += text + tail

        return newLen

    def getMaxCParamTypeLength(self, info):
        """Return the length of the longest type field for a member/parameter.

        - info - TypeInfo or CommandInfo.
        """
        lengths = (self.getCParamTypeLength(member)
                   for member in info.getMembers())
        return max(lengths)

    def getHandleParent(self, typename):
        """Get the parent of a handle object."""
        if self.registry is None:
            raise MissingRegistryError()

        info = self.registry.typedict.get(typename)
        if info is None:
            return None

        elem = info.elem
        if elem is not None:
            return elem.get('parent')

        return None

    def iterateHandleAncestors(self, typename):
        """Iterate through the ancestors of a handle type."""
        current = self.getHandleParent(typename)
        while current is not None:
            yield current
            current = self.getHandleParent(current)

    def getHandleAncestors(self, typename):
        """Get the ancestors of a handle object."""
        return list(self.iterateHandleAncestors(typename))

    def getTypeCategory(self, typename):
        """Get the category of a type."""
        if self.registry is None:
            raise MissingRegistryError()

        info = self.registry.typedict.get(typename)
        if info is None:
            return None

        elem = info.elem
        if elem is not None:
            return elem.get('category')
        return None

    def isStructAlwaysValid(self, structname):
        """Try to do check if a structure is always considered valid (i.e. there is no rules to its acceptance)."""
        # A conventions object is required for this call.
        if not self.conventions:
            raise RuntimeError("To use isStructAlwaysValid, be sure your options include a Conventions object.")
        if self.registry is None:
            raise MissingRegistryError()

        if self.conventions.type_always_valid(structname):
            return True

        category = self.getTypeCategory(structname)
        if self.conventions.category_requires_validation(category):
            return False

        info = self.registry.typedict.get(structname)
        if info is None:
            self.logMsg('error', f'isStructAlwaysValid({structname}) - structure not found in typedict')

        members = info.getMembers()

        for member in members:
            member_name = getElemName(member)
            if member_name in (self.conventions.structtype_member_name,
                               self.conventions.nextpointer_member_name):
                return False

            if member.get('noautovalidity'):
                return False

            member_type = getElemType(member)

            if member_type in ('void', 'char') or self.paramIsArray(member) or self.paramIsPointer(member):
                return False

            if self.conventions.type_always_valid(member_type):
                continue

            member_category = self.getTypeCategory(member_type)

            if self.conventions.category_requires_validation(member_category):
                return False

            if member_category in ('struct', 'union'):
                if self.isStructAlwaysValid(member_type) is False:
                    return False

        return True

    def paramIsArray(self, param):
        """Check if the parameter passed in is a pointer to an array.

        param           the XML information for the param
        """
        return param.get('len') is not None

    def paramIsPointer(self, param):
        """Check if the parameter passed in is a pointer.

        param           the XML information for the param
        """
        tail = param.find('type').tail
        return tail is not None and '*' in tail

    def isEnumRequired(self, elem):
        """Return True if this `<enum>` element is
        required, False otherwise

        - elem - `<enum>` element to test"""
        required = elem.get('required') is not None
        self.logMsg('diag', 'isEnumRequired:', elem.get('name'),
                    '->', required)
        return required

        # @@@ This code is overridden by equivalent code now run in
        # @@@ Registry.generateFeature

        required = False

        extname = elem.get('extname')
        if extname is not None:
            # 'supported' attribute was injected when the <enum> element was
            # moved into the <enums> group in Registry.parseTree()
            if self.genOpts.defaultExtensions == elem.get('supported'):
                required = True
            elif re.match(self.genOpts.addExtensions, extname) is not None:
                required = True
        elif elem.get('version') is not None:
            required = re.match(self.genOpts.emitversions, elem.get('version')) is not None
        else:
            required = True

        return required

    def makeCDecls(self, cmd):
        """Return C prototype and function pointer typedef for a
        `<command>` Element, as a two-element list of strings.

        - cmd - Element containing a `<command>` tag"""
        if self.genOpts is None:
            raise MissingGeneratorOptionsError()
        proto = cmd.find('proto')
        params = cmd.findall('param')
        # Begin accumulating prototype and typedef strings
        pdecl = self.genOpts.apicall
        tdecl = 'typedef '

        # Insert the function return type/name.
        # For prototypes, add APIENTRY macro before the name
        # For typedefs, add (APIENTRY *<name>) around the name and
        #   use the PFN_cmdnameproc naming convention.
        # Done by walking the tree for <proto> element by element.
        # etree has elem.text followed by (elem[i], elem[i].tail)
        #   for each child element and any following text
        # Leading text
        pdecl += noneStr(proto.text)
        tdecl += noneStr(proto.text)
        # For each child element, if it is a <name> wrap in appropriate
        # declaration. Otherwise append its contents and tail contents.
        for elem in proto:
            text = noneStr(elem.text)
            tail = noneStr(elem.tail)
            if elem.tag == 'name':
                pdecl += self.makeProtoName(text, tail)
                tdecl += self.makeTypedefName(text, tail)
            else:
                pdecl += text + tail
                tdecl += text + tail

        if self.genOpts.alignFuncParam == 0:
            # Squeeze out multiple spaces - there is no indentation
            pdecl = ' '.join(pdecl.split())
            tdecl = ' '.join(tdecl.split())

        # Now add the parameter declaration list, which is identical
        # for prototypes and typedefs. Concatenate all the text from
        # a <param> node without the tags. No tree walking required
        # since all tags are ignored.
        # Uses: self.indentFuncProto
        # self.indentFuncPointer
        # self.alignFuncParam
        n = len(params)
        # Indented parameters
        if n > 0:
            indentdecl = '(\n'
            indentdecl += ',\n'.join(self.makeCParamDecl(p, self.genOpts.alignFuncParam)
                                     for p in params)
            indentdecl += ');'
        else:
            indentdecl = '(void);'
        # Non-indented parameters
        paramdecl = '('
        if n > 0:
            paramnames = []
            if self.misracppstyle():
                for p in params:
                    param = ''
                    firstIter = True;
                    for t in p.itertext():
                        if (firstIter):
                            prefix = t
                            firstIter = False
                        else:
                            # Change pointer type order from e.g. "const void *" to "void const *".
                            # If the string starts with 'const', reorder it to be after the first type.
                            if (prefix.find('const ') != -1):
                                param += f"{prefix.replace('const ', '') + t} const "
                            else:
                                param += prefix + t
                            # Clear prefix for subsequent iterations
                            prefix = ''
                    paramnames.append(param);
            else:
                paramnames = (''.join(t for t in p.itertext())
                              for p in params)
            paramdecl += ', '.join(paramnames)
        else:
            paramdecl += 'void'
        paramdecl += ");"

        return [pdecl + indentdecl, tdecl + paramdecl]

    def newline(self):
        """Print a newline to the output file (utility function)"""
        write('', file=self.outFile)

    def setRegistry(self, registry):
        self.registry = registry
