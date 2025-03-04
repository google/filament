#!/usr/bin/python3 -i
#
# Copyright 2013-2020 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

import io,os,re,string,sys
from lxml import etree

def write(*args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    end = kwargs.pop('end', '\n')
    file.write(' '.join([str(arg) for arg in args]))
    file.write(end)

# noneStr - returns string argument, or "" if argument is None.
# Used in converting lxml Elements into text.
#   str - string to convert
def noneStr(str):
    if (str):
        return str
    else:
        return ""

# matchAPIProfile - returns whether an API and profile
#   being generated matches an element's profile
# api - string naming the API to match
# profile - string naming the profile to match
# elem - Element which (may) have 'api' and 'profile'
#   attributes to match to.
# If a tag is not present in the Element, the corresponding API
#   or profile always matches.
# Otherwise, the tag must exactly match the API or profile.
# Thus, if 'profile' = core:
#   <remove> with no attribute will match
#   <remove profile='core'> will match
#   <remove profile='compatibility'> will not match
# Possible match conditions:
#   Requested   Element
#   Profile     Profile
#   ---------   --------
#   None        None        Always matches
#   'string'    None        Always matches
#   None        'string'    Does not match. Can't generate multiple APIs
#                           or profiles, so if an API/profile constraint
#                           is present, it must be asked for explicitly.
#   'string'    'string'    Strings must match
#
#   ** In the future, we will allow regexes for the attributes,
#   not just strings, so that api="^(gl|gles2)" will match. Even
#   this isn't really quite enough, we might prefer something
#   like "gl(core)|gles1(common-lite)".
def matchAPIProfile(api, profile, elem):
    """Match a requested API & profile name to a api & profile attributes of an Element"""
    match = True
    # Match 'api', if present
    if ('api' in elem.attrib):
        if (api == None):
            raise UserWarning("No API requested, but 'api' attribute is present with value '" +
                              elem.get('api') + "'")
        elif (api != elem.get('api')):
            # Requested API doesn't match attribute
            return False
    if ('profile' in elem.attrib):
        if (profile == None):
            raise UserWarning("No profile requested, but 'profile' attribute is present with value '" +
                elem.get('profile') + "'")
        elif (profile != elem.get('profile')):
            # Requested profile doesn't match attribute
            return False
    return True

# BaseInfo - base class for information about a registry feature
# (type/group/enum/command/API/extension).
#   required - should this feature be defined during header generation
#     (has it been removed by a profile or version)?
#   declared - has this feature been defined already?
#   elem - lxml.etree Element for this feature
#   resetState() - reset required/declared to initial values. Used
#     prior to generating a new API interface.
class BaseInfo:
    """Represents the state of a registry feature, used during API generation"""
    def __init__(self, elem):
        self.required = False
        self.declared = False
        self.elem = elem
    def resetState(self):
        self.required = False
        self.declared = False

# TypeInfo - registry information about a type. No additional state
#   beyond BaseInfo is required.
class TypeInfo(BaseInfo):
    """Represents the state of a registry type"""
    def __init__(self, elem):
        BaseInfo.__init__(self, elem)

# GroupInfo - registry information about a group of related enums.
#   enums - dictionary of enum names which are in the group
class GroupInfo(BaseInfo):
    """Represents the state of a registry enumerant group"""
    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.enums = {}

# EnumInfo - registry information about an enum
#   type - numeric type of the value of the <enum> tag
#     ( '' for GLint, 'u' for GLuint, 'ull' for GLuint64 )
class EnumInfo(BaseInfo):
    """Represents the state of a registry enum"""
    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.type = elem.get('type')
        if (self.type == None):
            self.type = ''

# CmdInfo - registry information about a command
#   glxtype - type of GLX protocol { None, 'render', 'single', 'vendor' }
#   glxopcode - GLX protocol opcode { None, number }
#   glxequiv - equivalent command at GLX dispatch level { None, string }
#   vecequiv - equivalent vector form of a command taking multiple scalar args
#     { None, string }
class CmdInfo(BaseInfo):
    """Represents the state of a registry command"""
    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.glxtype = None
        self.glxopcode = None
        self.glxequiv = None
        self.vecequiv = None

# FeatureInfo - registry information about an API <feature>
# or <extension>
#   name - feature name string (e.g. 'GL_ARB_multitexture')
#   number - feature version number (e.g. 1.2). <extension>
#     features are unversioned and assigned version number 0.
#   category - category, e.g. VERSION or ARB/KHR/OES/ETC/vendor
#   emit - has this feature been defined already?
class FeatureInfo(BaseInfo):
    """Represents the state of an API feature (version/extension)"""
    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.name = elem.get('name')
        # Determine element category (vendor). Only works
        # for <extension> elements.
        if (elem.tag == 'feature'):
            self.category = 'VERSION'
            self.number = elem.get('number')
        else:
            self.category = self.name.split('_', 2)[1]
            self.number = "0"
        self.emit = False

# Primary sort key for regSortFeatures.
# Sorts by category of the feature name string:
#   Core API features (those defined with a <feature> tag)
#   ARB/KHR/OES (Khronos extensions)
#   other       (EXT/vendor extensions)
def regSortCategoryKey(feature):
    if (feature.elem.tag == 'feature'):
        return 0
    elif (feature.category == 'ARB' or
          feature.category == 'KHR' or
          feature.category == 'OES'):
        return 1
    else:
        return 2

# Secondary sort key for regSortFeatures.
# Sorts by extension name.
def regSortNameKey(feature):
    return feature.name

# Tertiary sort key for regSortFeatures.
# Sorts by feature version number. <extension>
# elements all have version number "0"
def regSortNumberKey(feature):
    return feature.number

# regSortFeatures - default sort procedure for features.
# Sorts by primary key of feature category,
# then by feature name within the category,
# then by version number
def regSortFeatures(featureList):
    featureList.sort(key = regSortNumberKey)
    featureList.sort(key = regSortNameKey)
    featureList.sort(key = regSortCategoryKey)

# GeneratorOptions - base class for options used during header production
# These options are target language independent, and used by
# Registry.apiGen() and by base OutputGenerator objects.
#
# Members
#   filename - name of file to generate, or None to write to stdout.
#   apiname - string matching <api> 'apiname' attribute, e.g. 'gl'.
#   profile - string specifying API profile , e.g. 'core', or None.
#   versions - regex matching API versions to process interfaces for.
#     Normally '.*' or '[0-9]\.[0-9]' to match all defined versions.
#   emitversions - regex matching API versions to actually emit
#    interfaces for (though all requested versions are considered
#    when deciding which interfaces to generate). For GL 4.3 glext.h,
#     this might be '1\.[2-5]|[2-4]\.[0-9]'.
#   defaultExtensions - If not None, a string which must in its
#     entirety match the pattern in the "supported" attribute of
#     the <extension>. Defaults to None. Usually the same as apiname.
#   addExtensions - regex matching names of additional extensions
#     to include. Defaults to None.
#   removeExtensions - regex matching names of extensions to
#     remove (after defaultExtensions and addExtensions). Defaults
#     to None.
#   sortProcedure - takes a list of FeatureInfo objects and sorts
#     them in place to a preferred order in the generated output.
#     Default is core API versions, ARB/KHR/OES extensions, all
#     other extensions, alphabetically within each group.
# The regex patterns can be None or empty, in which case they match
#   nothing.
class GeneratorOptions:
    """Represents options during header production from an API registry"""
    def __init__(self,
                 filename = None,
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 sortProcedure = regSortFeatures):
        self.filename          = filename
        self.apiname           = apiname
        self.profile           = profile
        self.versions          = self.emptyRegex(versions)
        self.emitversions      = self.emptyRegex(emitversions)
        self.defaultExtensions = defaultExtensions
        self.addExtensions     = self.emptyRegex(addExtensions)
        self.removeExtensions  = self.emptyRegex(removeExtensions)
        self.sortProcedure     = sortProcedure
    #
    # Substitute a regular expression which matches no version
    # or extension names for None or the empty string.
    def emptyRegex(self,pat):
        if (pat == None or pat == ''):
            return '_nomatch_^'
        else:
            return pat

# CGeneratorOptions - subclass of GeneratorOptions.
#
# Adds options used by COutputGenerator objects during C language header
# generation.
#
# Additional members
#   prefixText - list of strings to prefix generated header with
#     (usually a copyright statement + calling convention macros).
#   protectFile - True if multiple inclusion protection should be
#     generated (based on the filename) around the entire header.
#   protectFeature - True if #ifndef..#endif protection should be
#     generated around a feature interface in the header file.
#   genFuncPointers - True if function pointer typedefs should be
#     generated
#   protectProto - Controls cpp protection around prototypes:
#     False - no protection
#     'nonzero' - protectProtoStr must be defined to a nonzero value
#     True - protectProtoStr must be defined
#   protectProtoStr - #ifdef symbol to use around prototype
#     declarations, if protected
#   apicall - string to use for the function declaration prefix,
#     such as APICALL on Windows.
#   apientry - string to use for the calling convention macro,
#     in typedefs, such as APIENTRY.
#   apientryp - string to use for the calling convention macro
#     in function pointer typedefs, such as APIENTRYP.
class CGeneratorOptions(GeneratorOptions):
    """Represents options during C header production from an API registry"""
    def __init__(self,
                 filename = None,
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 sortProcedure = regSortFeatures,
                 prefixText = "",
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 protectProto = True,
                 protectProtoStr = True,
                 apicall = '',
                 apientry = '',
                 apientryp = ''):
        GeneratorOptions.__init__(self, filename, apiname, profile,
                                  versions, emitversions, defaultExtensions,
                                  addExtensions, removeExtensions, sortProcedure)
        self.prefixText      = prefixText
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.protectProto    = protectProto
        self.protectProtoStr = protectProtoStr
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp

# OutputGenerator - base class for generating API interfaces.
# Manages basic logic, logging, and output file control
# Derived classes actually generate formatted output.
#
# ---- methods ----
# OutputGenerator(errFile, warnFile, diagFile)
#   errFile, warnFile, diagFile - file handles to write errors,
#     warnings, diagnostics to. May be None to not write.
# logMsg(level, *args) - log messages of different categories
#   level - 'error', 'warn', or 'diag'. 'error' will also
#     raise a UserWarning exception
#   *args - print()-style arguments
# beginFile(genOpts) - start a new interface file
#   genOpts - GeneratorOptions controlling what's generated and how
# endFile() - finish an interface file, closing it when done
# beginFeature(interface, emit) - write interface for a feature
# and tag generated features as having been done.
#   interface - element for the <version> / <extension> to generate
#   emit - actually write to the header only when True
# endFeature() - finish an interface.
# genType(typeinfo,name) - generate interface for a type
#   typeinfo - TypeInfo for a type
# genEnum(enuminfo, name) - generate interface for an enum
#   enuminfo - EnumInfo for an enum
#   name - enum name
# genCmd(cmdinfo) - generate interface for a command
#   cmdinfo - CmdInfo for a command
class OutputGenerator:
    """Generate specified API interfaces in a specific style, such as a C header"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        self.outFile = None
        self.errFile = errFile
        self.warnFile = warnFile
        self.diagFile = diagFile
        # Internal state
        self.featureName = None
        self.genOpts = None
    #
    # logMsg - write a message of different categories to different
    #   destinations.
    # level -
    #   'diag' (diagnostic, voluminous)
    #   'warn' (warning)
    #   'error' (fatal error - raises exception after logging)
    # *args - print()-style arguments to direct to corresponding log
    def logMsg(self, level, *args):
        """Log a message at the given level. Can be ignored or log to a file"""
        if (level == 'error'):
            strfile = io.StringIO()
            write('ERROR:', *args, file=strfile)
            if (self.errFile != None):
                write(strfile.getvalue(), file=self.errFile)
            raise UserWarning(strfile.getvalue())
        elif (level == 'warn'):
            if (self.warnFile != None):
                write('WARNING:', *args, file=self.warnFile)
        elif (level == 'diag'):
            if (self.diagFile != None):
                write('DIAG:', *args, file=self.diagFile)
        else:
            raise UserWarning(
                '*** FATAL ERROR in Generator.logMsg: unknown level:' + level)
    #
    def beginFile(self, genOpts):
        self.genOpts = genOpts
        #
        # Open specified output file. Not done in constructor since a
        # Generator can be used without writing to a file.
        if (self.genOpts.filename != None):
            self.outFile = open(self.genOpts.filename, 'w')
        else:
            self.outFile = sys.stdout
    def endFile(self):
        self.errFile and self.errFile.flush()
        self.warnFile and self.warnFile.flush()
        self.diagFile and self.diagFile.flush()
        self.outFile.flush()
        if (self.outFile != sys.stdout and self.outFile != sys.stderr):
            self.outFile.close()
        self.genOpts = None
    #
    def beginFeature(self, interface, emit):
        self.emit = emit
        self.featureName = interface.get('name')
        # If there's an additional 'protect' attribute in the feature, save it
        self.featureExtraProtect = interface.get('protect')
    def endFeature(self):
        # Derived classes responsible for emitting feature
        self.featureName = None
        self.featureExtraProtect = None
    #
    # Type generation
    def genType(self, typeinfo, name):
        if (self.featureName == None):
            raise UserWarning('Attempt to generate type', name,
                    'when not in feature')
    #
    # Enumerant generation
    def genEnum(self, enuminfo, name):
        if (self.featureName == None):
            raise UserWarning('Attempt to generate enum', name,
                    'when not in feature')
    #
    # Command generation
    def genCmd(self, cmd, name):
        if (self.featureName == None):
            raise UserWarning('Attempt to generate command', name,
                    'when not in feature')

# COutputGenerator - subclass of OutputGenerator.
# Generates C-language API interfaces.
#
# ---- methods ----
# COutputGenerator(errFile, warnFile, diagFile) - args as for
#   OutputGenerator. Defines additional internal state.
# makeCDecls(cmd) - return C prototype and function pointer typedef for a
#     <command> Element, as a list of two strings
#   cmd - Element for the <command>
# newline() - print a newline to the output file (utility function)
# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
# genType(typeinfo,name) - generate interface for a type
# genEnum(enuminfo, name)
# genCmd(cmdinfo)
class COutputGenerator(OutputGenerator):
    """Generate specified API interfaces in a specific style, such as a C header"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.typeBody = ''
        self.enumBody = ''
        self.cmdBody = ''
    #
    # makeCDecls - return C prototype and function pointer typedef for a
    #   command, as a two-element list of strings.
    # cmd - Element containing a <command> tag
    def makeCDecls(self, cmd):
        """Generate C function pointer typedef for <command> Element"""
        proto = cmd.find('proto')
        params = cmd.findall('param')
        # Begin accumulating prototype and typedef strings
        pdecl = self.genOpts.apicall
        tdecl = 'typedef '
        #
        # Insert the function return type/name.
        # For prototypes, add APIENTRY macro before the name
        # For typedefs, add (APIENTRYP <name>) around the name and
        #   use the PFNGLCMDNAMEPROC nameng convention.
        # Done by walking the tree for <proto> element by element.
        # lxml.etree has elem.text followed by (elem[i], elem[i].tail)
        #   for each child element and any following text
        # Leading text
        pdecl += noneStr(proto.text)
        tdecl += noneStr(proto.text)
        # For each child element, if it's a <name> wrap in appropriate
        # declaration. Otherwise append its contents and tail contents.
        for elem in proto:
            text = noneStr(elem.text)
            tail = noneStr(elem.tail)
            if (elem.tag == 'name'):
                pdecl += self.genOpts.apientry + text + tail
                tdecl += '(' + self.genOpts.apientryp + 'PFN' + text.upper() + 'PROC' + tail + ')'
            else:
                pdecl += text + tail
                tdecl += text + tail
        # Now add the parameter declaration list, which is identical
        # for prototypes and typedefs. Concatenate all the text from
        # a <param> node without the tags. No tree walking required
        # since all tags are ignored.
        n = len(params)
        paramdecl = ' ('
        if n > 0:
            for i in range(0,n):
                paramdecl += ''.join([t for t in params[i].itertext()])
                if (i < n - 1):
                    paramdecl += ', '
        else:
            paramdecl += 'void'
        paramdecl += ');\n';
        return [ pdecl + paramdecl, tdecl + paramdecl ]
    #
    def newline(self):
        write('', file=self.outFile)
    #
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        # C-specific
        #
        # Multiple inclusion protection & C++ wrappers.
        if (genOpts.protectFile and self.genOpts.filename):
            headerSym = '__' + self.genOpts.apiname + '_' + re.sub('\.h', '_h_', os.path.basename(self.genOpts.filename))
            write('#ifndef', headerSym, file=self.outFile)
            write('#define', headerSym, '1', file=self.outFile)
            self.newline()
        write('#ifdef __cplusplus', file=self.outFile)
        write('extern "C" {', file=self.outFile)
        write('#endif', file=self.outFile)
        self.newline()
        #
        # User-supplied prefix text, if any (list of strings)
        if (genOpts.prefixText):
            for s in genOpts.prefixText:
                write(s, file=self.outFile)
        #
        # Some boilerplate describing what was generated - this
        # will probably be removed later since the extensions
        # pattern may be very long.
        write('/* Generated C header for:', file=self.outFile)
        write(' * API:', genOpts.apiname, file=self.outFile)
        if (genOpts.profile):
            write(' * Profile:', genOpts.profile, file=self.outFile)
        write(' * Versions considered:', genOpts.versions, file=self.outFile)
        write(' * Versions emitted:', genOpts.emitversions, file=self.outFile)
        write(' * Default extensions included:', genOpts.defaultExtensions, file=self.outFile)
        write(' * Additional extensions included:', genOpts.addExtensions, file=self.outFile)
        write(' * Extensions removed:', genOpts.removeExtensions, file=self.outFile)
        write(' */', file=self.outFile)
    def endFile(self):
        # C-specific
        # Finish C++ wrapper and multiple inclusion protection
        self.newline()
        write('#ifdef __cplusplus', file=self.outFile)
        write('}', file=self.outFile)
        write('#endif', file=self.outFile)
        if (self.genOpts.protectFile and self.genOpts.filename):
            self.newline()
            write('#endif', file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        # C-specific
        # Accumulate types, enums, function pointer typedefs, end function
        # prototypes separately for this feature. They're only printed in
        # endFeature().
        self.typeBody = ''
        self.enumBody = ''
        self.cmdPointerBody = ''
        self.cmdBody = ''
    def endFeature(self):
        # C-specific
        # Actually write the interface to the output file.
        if (self.emit):
            self.newline()
            if (self.genOpts.protectFeature):
                write('#ifndef', self.featureName, file=self.outFile)
            write('#define', self.featureName, '1', file=self.outFile)
            if (self.typeBody != ''):
                write(self.typeBody, end='', file=self.outFile)
            #
            # Don't add additional protection for derived type declarations,
            # which may be needed by other features later on.
            if (self.featureExtraProtect != None):
                write('#ifdef', self.featureExtraProtect, file=self.outFile)
            if (self.enumBody != ''):
                write(self.enumBody, end='', file=self.outFile)
            if (self.genOpts.genFuncPointers and self.cmdPointerBody != ''):
                write(self.cmdPointerBody, end='', file=self.outFile)
            if (self.cmdBody != ''):
                if (self.genOpts.protectProto == True):
                    prefix = '#ifdef ' + self.genOpts.protectProtoStr + '\n'
                    suffix = '#endif\n'
                elif (self.genOpts.protectProto == 'nonzero'):
                    prefix = '#if ' + self.genOpts.protectProtoStr + '\n'
                    suffix = '#endif\n'
                elif (self.genOpts.protectProto == False):
                    prefix = ''
                    suffix = ''
                else:
                    self.gen.logMsg('warn',
                                    '*** Unrecognized value for protectProto:',
                                    self.genOpts.protectProto,
                                    'not generating prototype wrappers')
                    prefix = ''
                    suffix = ''

                write(prefix + self.cmdBody + suffix, end='', file=self.outFile)
            if (self.featureExtraProtect != None):
                write('#endif /*', self.featureExtraProtect, '*/', file=self.outFile)
            if (self.genOpts.protectFeature):
                write('#endif /*', self.featureName, '*/', file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFeature(self)
    #
    # Type generation
    def genType(self, typeinfo, name):
        OutputGenerator.genType(self, typeinfo, name)
        #
        # Replace <apientry /> tags with an APIENTRY-style string
        # (from self.genOpts). Copy other text through unchanged.
        # If the resulting text is an empty string, don't emit it.
        typeElem = typeinfo.elem
        s = noneStr(typeElem.text)
        for elem in typeElem:
            if (elem.tag == 'apientry'):
                s += self.genOpts.apientry + noneStr(elem.tail)
            else:
                s += noneStr(elem.text) + noneStr(elem.tail)
        if (len(s) > 0):
            self.typeBody += s + '\n'
    #
    # Enumerant generation
    def genEnum(self, enuminfo, name):
        OutputGenerator.genEnum(self, enuminfo, name)
        #
        # EnumInfo.type is a C value suffix (e.g. u, ull)
        self.enumBody += '#define ' + name.ljust(33) + ' ' + enuminfo.elem.get('value')
        #
        # Handle non-integer 'type' fields by using it as the C value suffix
        t = enuminfo.elem.get('type')
        if (t != '' and t != 'i'):
            self.enumBody += enuminfo.type
        self.enumBody += '\n'
    #
    # Command generation
    def genCmd(self, cmdinfo, name):
        OutputGenerator.genCmd(self, cmdinfo, name)
        #
        decls = self.makeCDecls(cmdinfo.elem)
        self.cmdBody += decls[0]
        if (self.genOpts.genFuncPointers):
            self.cmdPointerBody += decls[1]

# Registry - object representing an API registry, loaded from an XML file
# Members
#   tree - ElementTree containing the root <registry>
#   typedict - dictionary of TypeInfo objects keyed by type name
#   groupdict - dictionary of GroupInfo objects keyed by group name
#   enumdict - dictionary of EnumInfo objects keyed by enum name
#   cmddict - dictionary of CmdInfo objects keyed by command name
#   apidict - dictionary of <api> Elements keyed by API name
#   extensions - list of <extension> Elements
#   extdict - dictionary of <extension> Elements keyed by extension name
#   gen - OutputGenerator object used to write headers / messages
#   genOpts - GeneratorOptions object used to control which
#     fetures to write and how to format them
#   emitFeatures - True to actually emit features for a version / extension,
#     or False to just treat them as emitted
# Public methods
#   loadElementTree(etree) - load registry from specified ElementTree
#   loadFile(filename) - load registry from XML file
#   setGenerator(gen) - OutputGenerator to use
#   parseTree() - parse the registry once loaded & create dictionaries
#   dumpReg(maxlen, filehandle) - diagnostic to dump the dictionaries
#     to specified file handle (default stdout). Truncates type /
#     enum / command elements to maxlen characters (default 80)
#   generator(g) - specify the output generator object
#   apiGen(apiname, genOpts) - generate API headers for the API type
#     and profile specified in genOpts, but only for the versions and
#     extensions specified there.
#   apiReset() - call between calls to apiGen() to reset internal state
#   validateGroups() - call to verify that each <proto> or <param>
#     with a 'group' attribute matches an actual existing group.
# Private methods
#   addElementInfo(elem,info,infoName,dictionary) - add feature info to dict
#   lookupElementInfo(fname,dictionary) - lookup feature info in dict
class Registry:
    """Represents an API registry loaded from XML"""
    def __init__(self):
        self.tree         = None
        self.typedict     = {}
        self.groupdict    = {}
        self.enumdict     = {}
        self.cmddict      = {}
        self.apidict      = {}
        self.extensions   = []
        self.extdict      = {}
        # A default output generator, so commands prior to apiGen can report
        # errors via the generator object.
        self.gen          = OutputGenerator()
        self.genOpts      = None
        self.emitFeatures = False
    def loadElementTree(self, tree):
        """Load ElementTree into a Registry object and parse it"""
        self.tree = tree
        self.parseTree()
    def loadFile(self, file):
        """Load an API registry XML file into a Registry object and parse it"""
        self.tree = etree.parse(file)
        self.parseTree()
    def setGenerator(self, gen):
        """Specify output generator object. None restores the default generator"""
        self.gen = gen
    # addElementInfo - add information about an element to the
    # corresponding dictionary
    #   elem - <type>/<group>/<enum>/<command>/<feature>/<extension> Element
    #   info - corresponding {Type|Group|Enum|Cmd|Feature}Info object
    #   infoName - 'type' / 'group' / 'enum' / 'command' / 'feature' / 'extension'
    #   dictionary - self.{type|group|enum|cmd|api|ext}dict
    # If the Element has an 'api' attribute, the dictionary key is the
    # tuple (name,api). If not, the key is the name. 'name' is an
    # attribute of the Element
    def addElementInfo(self, elem, info, infoName, dictionary):
        if ('api' in elem.attrib):
            key = (elem.get('name'),elem.get('api'))
        else:
            key = elem.get('name')
        if key in dictionary:
            self.gen.logMsg('warn', '*** Attempt to redefine',
                            infoName, 'with key:', key)
        else:
            dictionary[key] = info
    #
    # lookupElementInfo - find a {Type|Enum|Cmd}Info object by name.
    # If an object qualified by API name exists, use that.
    #   fname - name of type / enum / command
    #   dictionary - self.{type|enum|cmd}dict
    def lookupElementInfo(self, fname, dictionary):
        key = (fname, self.genOpts.apiname)
        if (key in dictionary):
            # self.gen.logMsg('diag', 'Found API-specific element for feature', fname)
            return dictionary[key]
        elif (fname in dictionary):
            # self.gen.logMsg('diag', 'Found generic element for feature', fname)
            return dictionary[fname]
        else:
            return None
    def parseTree(self):
        """Parse the registry Element, once created"""
        # This must be the Element for the root <registry>
        self.reg = self.tree.getroot()
        #
        # Create dictionary of registry types from toplevel <types> tags
        # and add 'name' attribute to each <type> tag (where missing)
        # based on its <name> element.
        #
        # There's usually one <types> block; more are OK
        # Required <type> attributes: 'name' or nested <name> tag contents
        self.typedict = {}
        for type in self.reg.findall('types/type'):
            # If the <type> doesn't already have a 'name' attribute, set
            # it from contents of its <name> tag.
            if (type.get('name') == None):
                type.attrib['name'] = type.find('name').text
            self.addElementInfo(type, TypeInfo(type), 'type', self.typedict)
        #
        # Create dictionary of registry groups from toplevel <groups> tags.
        #
        # There's usually one <groups> block; more are OK.
        # Required <group> attributes: 'name'
        self.groupdict = {}
        for group in self.reg.findall('groups/group'):
            self.addElementInfo(group, GroupInfo(group), 'group', self.groupdict)
        #
        # Create dictionary of registry enums from toplevel <enums> tags
        #
        # There are usually many <enums> tags in different namespaces, but
        #   these are functional namespaces of the values, while the actual
        #   enum names all share the dictionary.
        # Required <enums> attributes: 'name', 'value'
        self.enumdict = {}
        for enum in self.reg.findall('enums/enum'):
            self.addElementInfo(enum, EnumInfo(enum), 'enum', self.enumdict)
        #
        # Create dictionary of registry commands from <command> tags
        # and add 'name' attribute to each <command> tag (where missing)
        # based on its <proto><name> element.
        #
        # There's usually only one <commands> block; more are OK.
        # Required <command> attributes: 'name' or <proto><name> tag contents
        self.cmddict = {}
        for cmd in self.reg.findall('commands/command'):
            # If the <command> doesn't already have a 'name' attribute, set
            # it from contents of its <proto><name> tag.
            if (cmd.get('name') == None):
                cmd.attrib['name'] = cmd.find('proto/name').text
            ci = CmdInfo(cmd)
            self.addElementInfo(cmd, ci, 'command', self.cmddict)
        #
        # Create dictionaries of API and extension interfaces
        #   from toplevel <api> and <extension> tags.
        #
        self.apidict = {}
        for feature in self.reg.findall('feature'):
            ai = FeatureInfo(feature)
            self.addElementInfo(feature, ai, 'feature', self.apidict)
        self.extensions = self.reg.findall('extensions/extension')
        self.extdict = {}
        for feature in self.extensions:
            ei = FeatureInfo(feature)
            self.addElementInfo(feature, ei, 'extension', self.extdict)
    def dumpReg(self, maxlen = 40, filehandle = sys.stdout):
        """Dump all the dictionaries constructed from the Registry object"""
        write('***************************************', file=filehandle)
        write('    ** Dumping Registry contents **',     file=filehandle)
        write('***************************************', file=filehandle)
        write('// Types', file=filehandle)
        for name in self.typedict:
            tobj = self.typedict[name]
            write('    Type', name, '->', etree.tostring(tobj.elem)[0:maxlen], file=filehandle)
        write('// Groups', file=filehandle)
        for name in self.groupdict:
            gobj = self.groupdict[name]
            write('    Group', name, '->', etree.tostring(gobj.elem)[0:maxlen], file=filehandle)
        write('// Enums', file=filehandle)
        for name in self.enumdict:
            eobj = self.enumdict[name]
            write('    Enum', name, '->', etree.tostring(eobj.elem)[0:maxlen], file=filehandle)
        write('// Commands', file=filehandle)
        for name in self.cmddict:
            cobj = self.cmddict[name]
            write('    Command', name, '->', etree.tostring(cobj.elem)[0:maxlen], file=filehandle)
        write('// APIs', file=filehandle)
        for key in self.apidict:
            write('    API Version ', key, '->',
                etree.tostring(self.apidict[key].elem)[0:maxlen], file=filehandle)
        write('// Extensions', file=filehandle)
        for key in self.extdict:
            write('    Extension', key, '->',
                etree.tostring(self.extdict[key].elem)[0:maxlen], file=filehandle)
        # write('***************************************', file=filehandle)
        # write('    ** Dumping XML ElementTree **', file=filehandle)
        # write('***************************************', file=filehandle)
        # write(etree.tostring(self.tree.getroot(),pretty_print=True), file=filehandle)
    #
    # typename - name of type
    # required - boolean (to tag features as required or not)
    def markTypeRequired(self, typename, required):
        """Require (along with its dependencies) or remove (but not its dependencies) a type"""
        self.gen.logMsg('diag', '*** tagging type:', typename, '-> required =', required)
        # Get TypeInfo object for <type> tag corresponding to typename
        type = self.lookupElementInfo(typename, self.typedict)
        if (type != None):
            # Tag required type dependencies as required.
            # This DOES NOT un-tag dependencies in a <remove> tag.
            # See comments in markRequired() below for the reason.
            if (required and ('requires' in type.elem.attrib)):
                depType = type.elem.get('requires')
                self.gen.logMsg('diag', '*** Generating dependent type',
                    depType, 'for type', typename)
                self.markTypeRequired(depType, required)
            type.required = required
        else:
            self.gen.logMsg('warn', '*** type:', typename , 'IS NOT DEFINED')
    #
    # features - Element for <require> or <remove> tag
    # required - boolean (to tag features as required or not)
    def markRequired(self, features, required):
        """Require or remove features specified in the Element"""
        self.gen.logMsg('diag', '*** markRequired (features = <too long to print>, required =', required, ')')
        # Loop over types, enums, and commands in the tag
        # @@ It would be possible to respect 'api' and 'profile' attributes
        #  in individual features, but that's not done yet.
        for typeElem in features.findall('type'):
            self.markTypeRequired(typeElem.get('name'), required)
        for enumElem in features.findall('enum'):
            name = enumElem.get('name')
            self.gen.logMsg('diag', '*** tagging enum:', name, '-> required =', required)
            enum = self.lookupElementInfo(name, self.enumdict)
            if (enum != None):
                enum.required = required
            else:
                self.gen.logMsg('warn', '*** enum:', name , 'IS NOT DEFINED')
        for cmdElem in features.findall('command'):
            name = cmdElem.get('name')
            self.gen.logMsg('diag', '*** tagging command:', name, '-> required =', required)
            cmd = self.lookupElementInfo(name, self.cmddict)
            if (cmd != None):
                cmd.required = required
                # Tag all parameter types of this command as required.
                # This DOES NOT remove types of commands in a <remove>
                # tag, because many other commands may use the same type.
                # We could be more clever and reference count types,
                # instead of using a boolean.
                if (required):
                    # Look for <ptype> in entire <command> tree,
                    # not just immediate children
                    for ptype in cmd.elem.findall('.//ptype'):
                        self.gen.logMsg('diag', '*** markRequired: command implicitly requires dependent type', ptype.text)
                        self.markTypeRequired(ptype.text, required)
            else:
                self.gen.logMsg('warn', '*** command:', name, 'IS NOT DEFINED')
    #
    # interface - Element for <version> or <extension>, containing
    #   <require> and <remove> tags
    # api - string specifying API name being generated
    # profile - string specifying API profile being generated
    def requireAndRemoveFeatures(self, interface, api, profile):
        """Process <recquire> and <remove> tags for a <version> or <extension>"""
        # <require> marks things that are required by this version/profile
        for feature in interface.findall('require'):
            if (matchAPIProfile(api, profile, feature)):
                self.markRequired(feature,True)
        # <remove> marks things that are removed by this version/profile
        for feature in interface.findall('remove'):
            if (matchAPIProfile(api, profile, feature)):
                self.markRequired(feature,False)
    #
    # generateFeature - generate a single type / enum / command,
    # and all its dependencies as needed.
    #   fname - name of feature (<type>/<enum>/<command>
    #   ftype - type of feature, 'type' | 'enum' | 'command'
    #   dictionary - of *Info objects - self.{type|enum|cmd}dict
    #   genProc - bound function pointer for self.gen.gen{Type|Enum|Cmd}
    def generateFeature(self, fname, ftype, dictionary, genProc):
        f = self.lookupElementInfo(fname, dictionary)
        if (f == None):
            # No such feature. This is an error, but reported earlier
            self.gen.logMsg('diag', '*** No entry found for feature', fname,
                            'returning!')
            return
        #
        # If feature isn't required, or has already been declared, return
        if (not f.required):
            self.gen.logMsg('diag', '*** Skipping', ftype, fname, '(not required)')
            return
        if (f.declared):
            self.gen.logMsg('diag', '*** Skipping', ftype, fname, '(already declared)')
            return
        #
        # Pull in dependent type declaration(s) of the feature.
        # For types, there may be one in the 'required' attribute of the element
        # For commands, there may be many in <ptype> tags within the element
        # For enums, no dependencies are allowed (though perhasps if you
        #   have a uint64 enum, it should require GLuint64)
        if (ftype == 'type'):
            if ('requires' in f.elem.attrib):
                depname = f.elem.get('requires')
                self.gen.logMsg('diag', '*** Generating required dependent type',
                                depname)
                self.generateFeature(depname, 'type', self.typedict,
                                     self.gen.genType)
        elif (ftype == 'command'):
            for ptype in f.elem.findall('.//ptype'):
                depname = ptype.text
                self.gen.logMsg('diag', '*** Generating required parameter type',
                                depname)
                self.generateFeature(depname, 'type', self.typedict,
                                     self.gen.genType)
        #
        # Actually generate the type only if emitting declarations
        if self.emitFeatures:
            self.gen.logMsg('diag', '*** Emitting', ftype, 'decl for', fname)
            genProc(f, fname)
        else:
            self.gen.logMsg('diag', '*** Skipping', ftype, fname,
                            '(not emitting this feature)')
        # Always mark feature declared, as though actually emitted
        f.declared = True
    #
    # generateRequiredInterface - generate all interfaces required
    # by an API version or extension
    #   interface - Element for <version> or <extension>
    def generateRequiredInterface(self, interface):
        """Generate required C interface for specified API version/extension"""
        #
        # Loop over all features inside all <require> tags.
        # <remove> tags are ignored (handled in pass 1).
        for features in interface.findall('require'):
            for t in features.findall('type'):
                self.generateFeature(t.get('name'), 'type', self.typedict,
                                     self.gen.genType)
            for e in features.findall('enum'):
                self.generateFeature(e.get('name'), 'enum', self.enumdict,
                                     self.gen.genEnum)
            for c in features.findall('command'):
                self.generateFeature(c.get('name'), 'command', self.cmddict,
                                     self.gen.genCmd)
    #
    # apiGen(genOpts) - generate interface for specified versions
    #   genOpts - GeneratorOptions object with parameters used
    #   by the Generator object.
    def apiGen(self, genOpts):
        """Generate interfaces for the specified API type and range of versions"""
        #
        self.gen.logMsg('diag', '*******************************************')
        self.gen.logMsg('diag', '  Registry.apiGen file:', genOpts.filename,
                        'api:', genOpts.apiname,
                        'profile:', genOpts.profile)
        self.gen.logMsg('diag', '*******************************************')
        #
        self.genOpts = genOpts
        # Reset required/declared flags for all features
        self.apiReset()
        #
        # Compile regexps used to select versions & extensions
        regVersions = re.compile(self.genOpts.versions)
        regEmitVersions = re.compile(self.genOpts.emitversions)
        regAddExtensions = re.compile(self.genOpts.addExtensions)
        regRemoveExtensions = re.compile(self.genOpts.removeExtensions)
        #
        # Get all matching API versions & add to list of FeatureInfo
        features = []
        apiMatch = False
        for key in self.apidict:
            fi = self.apidict[key]
            api = fi.elem.get('api')
            if (api == self.genOpts.apiname):
                apiMatch = True
                if (regVersions.match(fi.number)):
                    # Matches API & version #s being generated. Mark for
                    # emission and add to the features[] list .
                    # @@ Could use 'declared' instead of 'emit'?
                    fi.emit = (regEmitVersions.match(fi.number) != None)
                    features.append(fi)
                    if (not fi.emit):
                        self.gen.logMsg('diag', '*** NOT tagging feature api =', api,
                            'name =', fi.name, 'number =', fi.number,
                            'for emission (does not match emitversions pattern)')
                else:
                    self.gen.logMsg('diag', '*** NOT including feature api =', api,
                        'name =', fi.name, 'number =', fi.number,
                        '(does not match requested versions)')
            else:
                self.gen.logMsg('diag', '*** NOT including feature api =', api,
                    'name =', fi.name,
                    '(does not match requested API)')
        if (not apiMatch):
            self.gen.logMsg('warn', '*** No matching API versions found!')
        #
        # Get all matching extensions & add to the list.
        # Start with extensions tagged with 'api' pattern matching the API
        # being generated. Add extensions matching the pattern specified in
        # regExtensions, then remove extensions matching the pattern
        # specified in regRemoveExtensions
        for key in self.extdict:
            ei = self.extdict[key]
            extName = ei.name
            include = False
            #
            # Include extension if defaultExtensions is not None and if the
            # 'supported' attribute matches defaultExtensions. The regexp in
            # 'supported' must exactly match defaultExtensions, so bracket
            # it with ^(pat)$.
            pat = '^(' + ei.elem.get('supported') + ')$'
            if (self.genOpts.defaultExtensions and
                     re.match(pat, self.genOpts.defaultExtensions)):
                self.gen.logMsg('diag', '*** Including extension',
                    extName, "(defaultExtensions matches the 'supported' attribute)")
                include = True
            #
            # Include additional extensions if the extension name matches
            # the regexp specified in the generator options. This allows
            # forcing extensions into an interface even if they're not
            # tagged appropriately in the registry.
            if (regAddExtensions.match(extName) != None):
                self.gen.logMsg('diag', '*** Including extension',
                    extName, '(matches explicitly requested extensions to add)')
                include = True
            # Remove extensions if the name matches the regexp specified
            # in generator options. This allows forcing removal of
            # extensions from an interface even if they're tagged that
            # way in the registry.
            if (regRemoveExtensions.match(extName) != None):
                self.gen.logMsg('diag', '*** Removing extension',
                    extName, '(matches explicitly requested extensions to remove)')
                include = False
            #
            # If the extension is to be included, add it to the
            # extension features list.
            if (include):
                ei.emit = True
                features.append(ei)
            else:
                self.gen.logMsg('diag', '*** NOT including extension',
                    extName, '(does not match api attribute or explicitly requested extensions)')
        #
        # Sort the extension features list, if a sort procedure is defined
        if (self.genOpts.sortProcedure):
            self.genOpts.sortProcedure(features)
        #
        # Pass 1: loop over requested API versions and extensions tagging
        #   types/commands/features as required (in an <require> block) or no
        #   longer required (in an <exclude> block). It is possible to remove
        #   a feature in one version and restore it later by requiring it in
        #   a later version.
        # If a profile other than 'None' is being generated, it must
        #   match the profile attribute (if any) of the <require> and
        #   <remove> tags.
        self.gen.logMsg('diag', '*** PASS 1: TAG FEATURES ********************************************')
        for f in features:
            self.gen.logMsg('diag', '*** PASS 1: Tagging required and removed features for',
                f.name)
            self.requireAndRemoveFeatures(f.elem, self.genOpts.apiname, self.genOpts.profile)
        #
        # Pass 2: loop over specified API versions and extensions printing
        #   declarations for required things which haven't already been
        #   generated.
        self.gen.logMsg('diag', '*** PASS 2: GENERATE INTERFACES FOR FEATURES ************************')
        self.gen.beginFile(self.genOpts)
        for f in features:
            self.gen.logMsg('diag', '*** PASS 2: Generating interface for',
                f.name)
            emit = self.emitFeatures = f.emit
            if (not emit):
                self.gen.logMsg('diag', '*** PASS 2: NOT declaring feature',
                    f.elem.get('name'), 'because it is not tagged for emission')
            # Generate the interface (or just tag its elements as having been
            # emitted, if they haven't been).
            self.gen.beginFeature(f.elem, emit)
            self.generateRequiredInterface(f.elem)
            self.gen.endFeature()
        self.gen.endFile()
    #
    # apiReset - use between apiGen() calls to reset internal state
    #
    def apiReset(self):
        """Reset type/enum/command dictionaries before generating another API"""
        for type in self.typedict:
            self.typedict[type].resetState()
        for enum in self.enumdict:
            self.enumdict[enum].resetState()
        for cmd in self.cmddict:
            self.cmddict[cmd].resetState()
        for cmd in self.apidict:
            self.apidict[cmd].resetState()
    #
    # validateGroups - check that group= attributes match actual groups
    #
    def validateGroups(self):
        """Validate group= attributes on <param> and <proto> tags"""
        # Keep track of group names not in <group> tags
        badGroup = {}
        self.gen.logMsg('diag', '*** VALIDATING GROUP ATTRIBUTES ***')
        for cmd in self.reg.findall('commands/command'):
            proto = cmd.find('proto')
            funcname = cmd.find('proto/name').text
            if ('group' in proto.attrib.keys()):
                group = proto.get('group')
                # self.gen.logMsg('diag', '*** Command ', funcname, ' has return group ', group)
                if (group not in self.groupdict.keys()):
                    # self.gen.logMsg('diag', '*** Command ', funcname, ' has UNKNOWN return group ', group)
                    if (group not in badGroup.keys()):
                        badGroup[group] = 1
                    else:
                        badGroup[group] = badGroup[group] +  1
            for param in cmd.findall('param'):
                pname = param.find('name')
                if (pname != None):
                    pname = pname.text
                else:
                    pname = type.get('name')
                if ('group' in param.attrib.keys()):
                    group = param.get('group')
                    if (group not in self.groupdict.keys()):
                        # self.gen.logMsg('diag', '*** Command ', funcname, ' param ', pname, ' has UNKNOWN group ', group)
                        if (group not in badGroup.keys()):
                            badGroup[group] = 1
                        else:
                            badGroup[group] = badGroup[group] +  1
        if (len(badGroup.keys()) > 0):
            self.gen.logMsg('diag', '*** SUMMARY OF UNRECOGNIZED GROUPS ***')
            for key in sorted(badGroup.keys()):
                self.gen.logMsg('diag', '    ', key, ' occurred ', badGroup[key], ' times')
