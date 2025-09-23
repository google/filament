#!/usr/bin/env python3 -i
#
# Copyright 2013-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

"""Types and classes for manipulating an API registry."""

import copy
import re
import sys
import xml.etree.ElementTree as etree
from collections import defaultdict, deque, namedtuple

from generator import GeneratorOptions, OutputGenerator, noneStr, write
from apiconventions import APIConventions

def apiNameMatch(str, supported):
    """Return whether a required api name matches a pattern specified for an
    XML <feature> 'api' attribute or <extension> 'supported' attribute.

    - str - API name such as 'vulkan' or 'openxr'. May be None, in which
        case it never matches (this should not happen).
    - supported - comma-separated list of XML API names. May be None, in
        which case str always matches (this is the usual case)."""

    if str is not None:
        return supported is None or str in supported.split(',')

    # Fallthrough case - either str is None or the test failed
    return False

def matchAPIProfile(api, profile, elem):
    """Return whether an API and profile
    being generated matches an element's profile

    - api - string naming the API to match
    - profile - string naming the profile to match
    - elem - Element which (may) have 'api' and 'profile'
      attributes to match to.

    If a tag is not present in the Element, the corresponding API
      or profile always matches.

    Otherwise, the tag must exactly match the API or profile.

    Thus, if 'profile' = core:

    - `<remove>`  with no attribute will match
    - `<remove profile="core">` will match
    - `<remove profile="compatibility">` will not match

    Possible match conditions:

    ```
      Requested   Element
      Profile     Profile
      ---------   --------
      None        None        Always matches
      'string'    None        Always matches
      None        'string'    Does not match. Cannot generate multiple APIs
                              or profiles, so if an API/profile constraint
                              is present, it must be asked for explicitly.
      'string'    'string'    Strings must match
    ```

    ** In the future, we will allow regexes for the attributes,
    not just strings, so that `api="^(gl|gles2)"` will match. Even
    this is not really quite enough, we might prefer something
    like `"gl(core)|gles1(common-lite)"`."""
    # Match 'api', if present
    elem_api = elem.get('api')
    if elem_api:
        if api is None:
            raise UserWarning("No API requested, but 'api' attribute is present with value '"
                              + elem_api + "'")
        elif api != elem_api:
            # Requested API does not match attribute
            return False
    elem_profile = elem.get('profile')
    if elem_profile:
        if profile is None:
            raise UserWarning("No profile requested, but 'profile' attribute is present with value '"
                              + elem_profile + "'")
        elif profile != elem_profile:
            # Requested profile does not match attribute
            return False
    return True


def mergeAPIs(tree, fromApiNames, toApiName):
    """Merge multiple APIs using the precedence order specified in apiNames.
    Also deletes <remove> elements.

        tree - Element at the root of the hierarchy to merge.
        apiNames - list of strings of API names."""

    stack = deque()
    stack.append(tree)

    while len(stack) > 0:
        parent = stack.pop()

        for child in parent.findall('*'):
            if child.tag == 'remove':
                # Remove <remove> elements
                parent.remove(child)
            else:
                stack.append(child)

            supportedList = child.get('supported')
            if supportedList:
                supportedList = supportedList.split(',')
                for apiName in [toApiName] + fromApiNames:
                    if apiName in supportedList:
                        child.set('supported', toApiName)

            if child.get('api'):
                definitionName = None
                definitionVariants = []

                # Keep only one definition with the same name if there are multiple definitions
                if child.tag in ['type']:
                    if child.get('name') is not None:
                        definitionName = child.get('name')
                        definitionVariants = parent.findall(f"{child.tag}[@name='{definitionName}']")
                    else:
                        definitionName = child.find('name').text
                        definitionVariants = parent.findall(f"{child.tag}/name[.='{definitionName}']/..")
                elif child.tag in ['member', 'param']:
                    definitionName = child.find('name').text
                    definitionVariants = parent.findall(f"{child.tag}/name[.='{definitionName}']/..")
                elif child.tag in ['enum', 'feature']:
                    definitionName = child.get('name')
                    definitionVariants = parent.findall(f"{child.tag}[@name='{definitionName}']")
                elif child.tag in ['require']:
                    # No way to correlate require tags because they do not have a definite identifier in the way they
                    # are used in the latest forms of the XML so the best we can do is simply enable all of them
                    if child.get('api') in fromApiNames:
                        child.set('api', toApiName)
                elif child.tag in ['command']:
                    definitionName = child.find('proto/name').text
                    definitionVariants = parent.findall(f"{child.tag}/proto/name[.='{definitionName}']/../..")

                if definitionName:
                    bestMatchApi = None
                    requires = None
                    for apiName in [toApiName] + fromApiNames:
                        for variant in definitionVariants:
                            # Keep any requires attributes from the target API
                            if variant.get('requires') and variant.get('api') == apiName:
                                requires = variant.get('requires')
                            # Find the best matching definition
                            if apiName in variant.get('api').split(',') and bestMatchApi is None:
                                bestMatchApi = variant.get('api')

                    if bestMatchApi:
                        for variant in definitionVariants:
                            if variant.get('api') != bestMatchApi:
                                # Only keep best matching definition
                                parent.remove(variant)
                            else:
                                # Add requires attribute from the target API if it is not overridden
                                if requires is not None and variant.get('requires') is None:
                                    variant.set('requires', requires)
                                variant.set('api', toApiName)


def stripNonmatchingAPIs(tree, apiName, actuallyDelete = True):
    """Remove tree Elements with 'api' attributes matching apiName.

        tree - Element at the root of the hierarchy to strip. Only its
            children can actually be removed, not the tree itself.
        apiName - string which much match a command-separated component of
            the 'api' attribute.
        actuallyDelete - only delete matching elements if True."""

    stack = deque()
    stack.append(tree)

    while len(stack) > 0:
        parent = stack.pop()

        for child in parent.findall('*'):
            api = child.get('api')

            if apiNameMatch(apiName, api):
                # Add child to the queue
                stack.append(child)
            elif not apiNameMatch(apiName, api):
                # Child does not match requested api. Remove it.
                if actuallyDelete:
                    parent.remove(child)


class BaseInfo:
    """Base class for information about a registry feature
    (type/group/enum/command/API/extension).

    Represents the state of a registry feature, used during API generation.
    """

    def __init__(self, elem):
        self.required = False
        """should this feature be defined during header generation
        (has it been removed by a profile or version)?"""

        self.declared = False
        "has this feature been defined already?"

        self.elem = elem
        "etree Element for this feature"

        self.deprecatedbyversion = None
        self.deprecatedbyextensions = []
        self.deprecatedlink = None

    def resetState(self):
        """Reset required/declared to initial values. Used
        prior to generating a new API interface."""
        self.required = False
        self.declared = False

    def compareKeys(self, info, key, required = False):
        """Return True if self.elem and info.elem have the same attribute
           value for key.
           If 'required' is not True, also returns True if neither element
           has an attribute value for key."""

        if required and key not in self.elem.keys():
            return False
        return self.elem.get(key) == info.elem.get(key)

    def compareElem(self, info, infoName):
        """Return True if self.elem and info.elem have the same definition.
        info - the other object
        infoName - 'type' / 'group' / 'enum' / 'command' / 'feature' /
                   'extension'"""

        if infoName == 'enum':
            if self.compareKeys(info, 'extends'):
                # Either both extend the same type, or no type
                if (self.compareKeys(info, 'value', required = True) or
                    self.compareKeys(info, 'bitpos', required = True)):
                    # If both specify the same value or bit position,
                    # they are equal
                    return True
                elif (self.compareKeys(info, 'extnumber') and
                      self.compareKeys(info, 'offset') and
                      self.compareKeys(info, 'dir')):
                    # If both specify the same relative offset, they are equal
                    return True
                elif (self.compareKeys(info, 'alias')):
                    # If both are aliases of the same value
                    return True
                else:
                    return False
            else:
                # The same enum cannot extend two different types
                return False
        else:
            # Non-<enum>s should never be redefined
            return False


class TypeInfo(BaseInfo):
    """Registry information about a type. No additional state
      beyond BaseInfo is required."""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.additionalValidity = []
        self.removedValidity = []

    def getMembers(self):
        """Get a collection of all member elements for this type, if any."""
        return self.elem.findall('member')

    def resetState(self):
        BaseInfo.resetState(self)
        self.additionalValidity = []
        self.removedValidity = []


class GroupInfo(BaseInfo):
    """Registry information about a group of related enums
    in an <enums> block, generally corresponding to a C "enum" type."""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)


class EnumInfo(BaseInfo):
    """Registry information about an enum"""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.type = elem.get('type')
        """numeric type of the value of the <enum> tag
        ( '' for GLint, 'u' for GLuint, 'ull' for GLuint64 )"""
        if self.type is None:
            self.type = ''


class CmdInfo(BaseInfo):
    """Registry information about a command"""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.additionalValidity = []
        self.removedValidity = []

    def getParams(self):
        """Get a collection of all param elements for this command, if any."""
        return self.elem.findall('param')

    def resetState(self):
        BaseInfo.resetState(self)
        self.additionalValidity = []
        self.removedValidity = []


class FeatureInfo(BaseInfo):
    """Registry information about an API <feature>
    or <extension>."""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)
        self.name = elem.get('name')
        "feature name string (e.g. 'VK_KHR_surface')"

        self.emit = False
        "has this feature been defined already?"

        self.sortorder = int(elem.get('sortorder', 0))
        """explicit numeric sort key within feature and extension groups.
        Defaults to 0."""

        # Determine element category (vendor). Only works
        # for <extension> elements.
        if elem.tag == 'feature':
            # Element category (vendor) is meaningless for <feature>
            self.category = 'VERSION'
            """category, e.g. VERSION or khr/vendor tag"""

            self.version = elem.get('name')
            """feature name string"""

            self.versionNumber = elem.get('number')
            """versionNumber - API version number, taken from the 'number'
               attribute of <feature>. Extensions do not have API version
               numbers and are assigned number 0."""

            self.number = 0
            self.supported = None

            self.deprecates = elem.findall('deprecate')
        else:
            # Extract vendor portion of <APIprefix>_<vendor>_<name>
            self.category = self.name.split('_', 2)[1]
            self.version = "0"
            self.versionNumber = "0"

            self.number = int(elem.get('number','0'))
            """extension number, used for ordering and for assigning
            enumerant offsets. <feature> features do not have extension
            numbers and are assigned number 0, as are extensions without
            numbers, so sorting works."""

            self.supported = elem.get('supported', 'disabled')

class SpirvInfo(BaseInfo):
    """Registry information about an API <spirvextensions>
    or <spirvcapability>."""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)

class FormatInfo(BaseInfo):
    """Registry information about an API <format>."""

    def __init__(self, elem, condition):
        BaseInfo.__init__(self, elem)
        # Need to save the condition here when it is known
        self.condition = condition

class SyncStageInfo(BaseInfo):
    """Registry information about <syncstage>."""

    def __init__(self, elem, condition):
        BaseInfo.__init__(self, elem)
        # Need to save the condition here when it is known
        self.condition = condition

class SyncAccessInfo(BaseInfo):
    """Registry information about <syncaccess>."""

    def __init__(self, elem, condition):
        BaseInfo.__init__(self, elem)
        # Need to save the condition here when it is known
        self.condition = condition

class SyncPipelineInfo(BaseInfo):
    """Registry information about <syncpipeline>."""

    def __init__(self, elem):
        BaseInfo.__init__(self, elem)

class Registry:
    """Object representing an API registry, loaded from an XML file."""

    def __init__(self, gen=None, genOpts=None):
        if gen is None:
            # If not specified, give a default object so messaging will work
            self.gen = OutputGenerator()
        else:
            self.gen = gen
        "Output generator used to write headers / messages"

        if genOpts is None:
            # If no generator is provided, we may still need the XML API name
            # (for example, in genRef.py).
            self.genOpts = GeneratorOptions(apiname = APIConventions().xml_api_name)
        else:
            self.genOpts = genOpts
        "Options controlling features to write and how to format them"

        self.gen.registry = self
        self.gen.genOpts = self.genOpts
        self.gen.genOpts.registry = self

        self.tree = None
        "ElementTree containing the root `<registry>`"

        self.typedict = {}
        "dictionary of TypeInfo objects keyed by type name"

        self.groupdict = {}
        "dictionary of GroupInfo objects keyed by group name"

        self.enumdict = {}
        "dictionary of EnumInfo objects keyed by enum name"

        self.cmddict = {}
        "dictionary of CmdInfo objects keyed by command name"

        self.aliasdict = {}
        "dictionary of type and command names mapped to their alias, such as VkFooKHR -> VkFoo"

        self.enumvaluedict = {}
        "dictionary of enum values mapped to their type, such as VK_FOO_VALUE -> VkFoo"

        self.apidict = {}
        "dictionary of FeatureInfo objects for `<feature>` elements keyed by API name"

        self.extensions = []
        "list of `<extension>` Elements"

        self.extdict = {}
        "dictionary of FeatureInfo objects for `<extension>` elements keyed by extension name"

        self.spirvextdict = {}
        "dictionary of FeatureInfo objects for `<spirvextension>` elements keyed by spirv extension name"

        self.spirvcapdict = {}
        "dictionary of FeatureInfo objects for `<spirvcapability>` elements keyed by spirv capability name"

        self.formatsdict = {}
        "dictionary of FeatureInfo objects for `<format>` elements keyed by VkFormat name"

        self.syncstagedict = {}
        "dictionary of Sync*Info objects for `<syncstage>` elements keyed by VkPipelineStageFlagBits2 name"

        self.syncaccessdict = {}
        "dictionary of Sync*Info objects for `<syncaccess>` elements keyed by VkAccessFlagBits2 name"

        self.syncpipelinedict = {}
        "dictionary of Sync*Info objects for `<syncpipeline>` elements keyed by pipeline type name"

        self.emitFeatures = False
        """True to actually emit features for a version / extension,
        or False to just treat them as emitted"""

        self.breakPat = None
        "regexp pattern to break on when generating names"
        # self.breakPat     = re.compile('VkFenceImportFlagBits.*')

        self.requiredextensions = []  # Hack - can remove it after validity generator goes away

        # ** Global types for automatic source generation **
        # Length Member data
        self.commandextensiontuple = namedtuple('commandextensiontuple',
                                                ['command',        # The name of the command being modified
                                                 'value',          # The value to append to the command
                                                 'extension'])     # The name of the extension that added it
        self.validextensionstructs = defaultdict(list)
        self.commandextensionsuccesses = []
        self.commandextensionerrors = []

        self.filename     = None

    def loadElementTree(self, tree):
        """Load ElementTree into a Registry object and parse it."""
        self.tree = tree
        self.parseTree()

    def loadFile(self, file):
        """Load an API registry XML file into a Registry object and parse it"""
        self.filename = file
        self.tree = etree.parse(file)
        self.parseTree()

    def setGenerator(self, gen):
        """Specify output generator object.

        `None` restores the default generator."""
        self.gen = gen
        self.gen.setRegistry(self)

    def addElementInfo(self, elem, info, infoName, dictionary):
        """Add information about an element to the corresponding dictionary.

        Intended for internal use only.

        - elem - `<type>`/`<enums>`/`<enum>`/`<command>`/`<feature>`/`<extension>`/`<spirvextension>`/`<spirvcapability>`/`<format>`/`<syncstage>`/`<syncaccess>`/`<syncpipeline>` Element
        - info - corresponding {Type|Group|Enum|Cmd|Feature|Spirv|Format|SyncStage|SyncAccess|SyncPipeline}Info object
        - infoName - 'type' / 'group' / 'enum' / 'command' / 'feature' / 'extension' / 'spirvextension' / 'spirvcapability' / 'format' / 'syncstage' / 'syncaccess' / 'syncpipeline'
        - dictionary - self.{type|group|enum|cmd|api|ext|format|spirvext|spirvcap|sync}dict

        The dictionary key is the element 'name' attribute."""

        # self.gen.logMsg('diag', 'Adding ElementInfo.required =',
        #     info.required, 'name =', elem.get('name'))
        key = elem.get('name')
        if key in dictionary:
            if not dictionary[key].compareElem(info, infoName):
                self.gen.logMsg('warn', 'Attempt to redefine', key,
                                '(this should not happen)')
        else:
            dictionary[key] = info

    def lookupElementInfo(self, fname, dictionary):
        """Find a {Type|Enum|Cmd}Info object by name.

        Intended for internal use only.

        If an object qualified by API name exists, use that.

        - fname - name of type / enum / command
        - dictionary - self.{type|enum|cmd}dict"""
        key = (fname, self.genOpts.apiname)
        if key in dictionary:
            # self.gen.logMsg('diag', 'Found API-specific element for feature', fname)
            return dictionary[key]
        if fname in dictionary:
            # self.gen.logMsg('diag', 'Found generic element for feature', fname)
            return dictionary[fname]

        return None

    def breakOnName(self, regexp):
        """Specify a feature name regexp to break on when generating features."""
        self.breakPat = re.compile(regexp)

    def addEnumValue(self, enum, type_name):
        """Track aliasing and map back from enum values to their type"""
        # Record alias, if any
        value = enum.get('name')
        alias = enum.get('alias')
        if alias:
            self.aliasdict[value] = alias
        # Map the value back to the type
        if type_name in self.aliasdict:
            type_name = self.aliasdict[type_name]
        if value in self.enumvaluedict:
            # Some times the same enum is defined by multiple extensions
            assert(type_name == self.enumvaluedict[value])
        else:
            self.enumvaluedict[value] = type_name

    def parseTree(self):
        """Parse the registry Element, once created"""
        # This must be the Element for the root <registry>
        if self.tree is None:
            raise RuntimeError("Tree not initialized!")
        self.reg = self.tree.getroot()

        # Preprocess the tree in one of the following ways:
        # - either merge a set of APIs to another API based on their 'api' attributes
        # - or remove all elements with non-matching 'api' attributes
        # The preprocessing happens through a breath-first tree traversal.
        # This is a blunt hammer, but eliminates the need to track and test
        # the apis deeper in processing to select the correct elements and
        # avoid duplicates.
        # Schema validation should prevent duplicate elements with
        # overlapping api attributes, or where one element has an api
        # attribute and the other does not.

        if self.genOpts.mergeApiNames:
            mergeAPIs(self.reg, self.genOpts.mergeApiNames.split(','), self.genOpts.apiname)
        else:
            stripNonmatchingAPIs(self.reg, self.genOpts.apiname, actuallyDelete = True)

        self.aliasdict = {}
        self.enumvaluedict = {}

        # Create dictionary of registry types from toplevel <types> tags
        # and add 'name' attribute to each <type> tag (where missing)
        # based on its <name> element.
        #
        # There is usually one <types> block; more are OK
        # Required <type> attributes: 'name' or nested <name> tag contents
        self.typedict = {}
        for type_elem in self.reg.findall('types/type'):
            # If the <type> does not already have a 'name' attribute, set
            # it from contents of its <name> tag.
            name = type_elem.get('name')
            if name is None:
                name_elem = type_elem.find('name')
                if name_elem is None or not name_elem.text:
                    raise RuntimeError("Type without a name!")
                name = name_elem.text
                type_elem.set('name', name)
            self.addElementInfo(type_elem, TypeInfo(type_elem), 'type', self.typedict)

            # Record alias, if any
            alias = type_elem.get('alias')
            if alias:
                self.aliasdict[name] = alias

        # Create dictionary of registry enum groups from <enums> tags.
        #
        # Required <enums> attributes: 'name'. If no name is given, one is
        # generated, but that group cannot be identified and turned into an
        # enum type definition - it is just a container for <enum> tags.
        self.groupdict = {}
        for group in self.reg.findall('enums'):
            self.addElementInfo(group, GroupInfo(group), 'group', self.groupdict)

        # Create dictionary of registry enums from <enum> tags
        #
        # <enums> tags usually define different namespaces for the values
        #   defined in those tags, but the actual names all share the
        #   same dictionary.
        # Required <enum> attributes: 'name', 'value'
        # For containing <enums> which have type="enum" or type="bitmask",
        # tag all contained <enum>s are required. This is a stopgap until
        # a better scheme for tagging core and extension enums is created.
        self.enumdict = {}
        for enums in self.reg.findall('enums'):
            required = (enums.get('type') is not None)
            type_name = enums.get('name')
            # Enum values are defined only for the type that is not aliased to something else.
            assert(type_name not in self.aliasdict)
            for enum in enums.findall('enum'):
                enumInfo = EnumInfo(enum)
                enumInfo.required = required
                self.addElementInfo(enum, enumInfo, 'enum', self.enumdict)
                self.addEnumValue(enum, type_name)

        # Create dictionary of registry commands from <command> tags
        # and add 'name' attribute to each <command> tag (where missing)
        # based on its <proto><name> element.
        #
        # There is usually only one <commands> block; more are OK.
        # Required <command> attributes: 'name' or <proto><name> tag contents
        self.cmddict = {}
        # List of commands which alias others. Contains
        #   [ name, aliasName, element ]
        # for each alias
        cmdAlias = []
        for cmd in self.reg.findall('commands/command'):
            # If the <command> does not already have a 'name' attribute, set
            # it from contents of its <proto><name> tag.
            name = cmd.get('name')
            if name is None:
                name_elem = cmd.find('proto/name')
                if name_elem is None or not name_elem.text:
                    raise RuntimeError("Command without a name!")
                name = cmd.set('name', name_elem.text)
            ci = CmdInfo(cmd)
            self.addElementInfo(cmd, ci, 'command', self.cmddict)
            alias = cmd.get('alias')
            if alias:
                cmdAlias.append([name, alias, cmd])
                self.aliasdict[name] = alias

        # Now loop over aliases, injecting a copy of the aliased command's
        # Element with the aliased prototype name replaced with the command
        # name - if it exists.
        # Copy the 'export' sttribute (whether it exists or not) from the
        # original, aliased command, since that can be different for a
        # command and its alias.
        for (name, alias, cmd) in cmdAlias:
            if alias in self.cmddict:
                aliasInfo = self.cmddict[alias]
                cmdElem = copy.deepcopy(aliasInfo.elem)
                cmdElem.find('proto/name').text = name
                cmdElem.set('name', name)
                cmdElem.set('alias', alias)
                export = cmd.get('export')
                if export is not None:
                    # Replicate the command's 'export' attribute
                    cmdElem.set('export', export)
                elif cmdElem.get('export') is not None:
                    # Remove the 'export' attribute, if the alias has one but
                    # the command does not.
                    del cmdElem.attrib['export']
                ci = CmdInfo(cmdElem)
                # Replace the dictionary entry for the CmdInfo element
                self.cmddict[name] = ci

                # @  newString = etree.tostring(base, encoding="unicode").replace(aliasValue, aliasName)
                # @elem.append(etree.fromstring(replacement))
            else:
                self.gen.logMsg('warn', 'No matching <command> found for command',
                                cmd.get('name'), 'alias', alias)

        # Create dictionaries of API and extension interfaces
        #   from toplevel <api> and <extension> tags.
        self.apidict = {}
        format_condition = dict()
        for feature in self.reg.findall('feature'):
            featureInfo = FeatureInfo(feature)
            self.addElementInfo(feature, featureInfo, 'feature', self.apidict)

            # Add additional enums defined only in <feature> tags
            # to the corresponding enumerated type.
            # When seen here, the <enum> element, processed to contain the
            # numeric enum value, is added to the corresponding <enums>
            # element, as well as adding to the enum dictionary. It is no
            # longer removed from the <require> element it is introduced in.
            # Instead, generateRequiredInterface ignores <enum> elements
            # that extend enumerated types.
            #
            # For <enum> tags which are actually just constants, if there is
            # no 'extends' tag but there is a 'value' or 'bitpos' tag, just
            # add an EnumInfo record to the dictionary. That works because
            # output generation of constants is purely dependency-based, and
            # does not need to iterate through the XML tags.
            for elem in feature.findall('require'):
                for enum in elem.findall('enum'):
                    addEnumInfo = False
                    groupName = enum.get('extends')
                    if groupName is not None:
                        # self.gen.logMsg('diag', 'Found extension enum',
                        #     enum.get('name'))
                        # Add version number attribute to the <enum> element
                        enum.set('version', featureInfo.version)
                        # Look up the GroupInfo with matching groupName
                        if groupName in self.groupdict:
                            # self.gen.logMsg('diag', 'Matching group',
                            #     groupName, 'found, adding element...')
                            gi = self.groupdict[groupName]
                            gi.elem.append(copy.deepcopy(enum))
                        else:
                            self.gen.logMsg('warn', 'NO matching group',
                                            groupName, 'for enum', enum.get('name'), 'found.')
                        if groupName == "VkFormat":
                            format_name = enum.get('name')
                            if enum.get('alias'):
                                format_name = enum.get('alias')
                            format_condition[format_name] = featureInfo.name
                        addEnumInfo = True
                    elif enum.get('value') or enum.get('bitpos') or enum.get('alias'):
                        # self.gen.logMsg('diag', 'Adding extension constant "enum"',
                        #     enum.get('name'))
                        addEnumInfo = True
                    if addEnumInfo:
                        enumInfo = EnumInfo(enum)
                        self.addElementInfo(enum, enumInfo, 'enum', self.enumdict)
                        self.addEnumValue(enum, groupName)

        sync_pipeline_stage_condition = dict()
        sync_access_condition = dict()

        self.extensions = self.reg.findall('extensions/extension')
        self.extdict = {}
        for feature in self.extensions:
            featureInfo = FeatureInfo(feature)
            self.addElementInfo(feature, featureInfo, 'extension', self.extdict)

            # Add additional enums defined only in <extension> tags
            # to the corresponding core type.
            # Algorithm matches that of enums in a "feature" tag as above.
            #
            # This code also adds a 'extnumber' attribute containing the
            # extension number, used for enumerant value calculation.
            for elem in feature.findall('require'):
                for enum in elem.findall('enum'):
                    addEnumInfo = False
                    groupName = enum.get('extends')
                    if groupName is not None:
                        # self.gen.logMsg('diag', 'Found extension enum',
                        #     enum.get('name'))

                        # Add <extension> block's extension number attribute to
                        # the <enum> element unless specified explicitly, such
                        # as when redefining an enum in another extension.
                        extnumber = enum.get('extnumber')
                        if not extnumber:
                            enum.set('extnumber', str(featureInfo.number))

                        enum.set('extname', featureInfo.name)
                        enum.set('supported', noneStr(featureInfo.supported))
                        # Look up the GroupInfo with matching groupName
                        if groupName in self.groupdict:
                            # self.gen.logMsg('diag', 'Matching group',
                            #     groupName, 'found, adding element...')
                            gi = self.groupdict[groupName]
                            gi.elem.append(copy.deepcopy(enum))
                        else:
                            self.gen.logMsg('warn', 'NO matching group',
                                            groupName, 'for enum', enum.get('name'), 'found.')
                        # This is Vulkan-specific
                        if groupName == "VkFormat":
                            format_name = enum.get('name')
                            if enum.get('alias'):
                                format_name = enum.get('alias')
                            if format_name in format_condition:
                                format_condition[format_name] += f",{featureInfo.name}"
                            else:
                                format_condition[format_name] = featureInfo.name
                        elif groupName == "VkPipelineStageFlagBits2":
                            stage_flag = enum.get('name')
                            if enum.get('alias'):
                                stage_flag = enum.get('alias')
                            featureName = elem.get('depends') if elem.get('depends') is not None else featureInfo.name
                            if stage_flag in sync_pipeline_stage_condition:
                                sync_pipeline_stage_condition[stage_flag] += f",{featureName}"
                            else:
                                sync_pipeline_stage_condition[stage_flag] = featureName
                        elif groupName == "VkAccessFlagBits2":
                            access_flag = enum.get('name')
                            if enum.get('alias'):
                                access_flag = enum.get('alias')
                            featureName = elem.get('depends') if elem.get('depends') is not None else featureInfo.name
                            if access_flag in sync_access_condition:
                                sync_access_condition[access_flag] += f",{featureName}"
                            else:
                                sync_access_condition[access_flag] = featureName

                        addEnumInfo = True
                    elif enum.get('value') or enum.get('bitpos') or enum.get('alias'):
                        # self.gen.logMsg('diag', 'Adding extension constant "enum"',
                        #     enum.get('name'))
                        addEnumInfo = True
                    if addEnumInfo:
                        enumInfo = EnumInfo(enum)
                        self.addElementInfo(enum, enumInfo, 'enum', self.enumdict)
                        self.addEnumValue(enum, groupName)

        # Parse out all spirv tags in dictionaries
        # Use addElementInfo to catch duplicates
        for spirv in self.reg.findall('spirvextensions/spirvextension'):
            spirvInfo = SpirvInfo(spirv)
            self.addElementInfo(spirv, spirvInfo, 'spirvextension', self.spirvextdict)
        for spirv in self.reg.findall('spirvcapabilities/spirvcapability'):
            spirvInfo = SpirvInfo(spirv)
            self.addElementInfo(spirv, spirvInfo, 'spirvcapability', self.spirvcapdict)

        for format in self.reg.findall('formats/format'):
            condition = None
            format_name = format.get('name')
            if format_name in format_condition:
                condition = format_condition[format_name]
            formatInfo = FormatInfo(format, condition)
            self.addElementInfo(format, formatInfo, 'format', self.formatsdict)

        for stage in self.reg.findall('sync/syncstage'):
            condition = None
            stage_flag = stage.get('name')
            if stage_flag in sync_pipeline_stage_condition:
                condition = sync_pipeline_stage_condition[stage_flag]
            syncInfo = SyncStageInfo(stage, condition)
            self.addElementInfo(stage, syncInfo, 'syncstage', self.syncstagedict)

        for access in self.reg.findall('sync/syncaccess'):
            condition = None
            access_flag = access.get('name')
            if access_flag in sync_access_condition:
                condition = sync_access_condition[access_flag]
            syncInfo = SyncAccessInfo(access, condition)
            self.addElementInfo(access, syncInfo, 'syncaccess', self.syncaccessdict)

        for pipeline in self.reg.findall('sync/syncpipeline'):
            syncInfo = SyncPipelineInfo(pipeline)
            self.addElementInfo(pipeline, syncInfo, 'syncpipeline', self.syncpipelinedict)

    def dumpReg(self, maxlen=120, filehandle=sys.stdout):
        """Dump all the dictionaries constructed from the Registry object.

        Diagnostic to dump the dictionaries to specified file handle (default stdout).
        Truncates type / enum / command elements to maxlen characters (default 120)"""
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
        write('// SPIR-V', file=filehandle)
        for key in self.spirvextdict:
            write('    SPIR-V Extension', key, '->',
                  etree.tostring(self.spirvextdict[key].elem)[0:maxlen], file=filehandle)
        for key in self.spirvcapdict:
            write('    SPIR-V Capability', key, '->',
                  etree.tostring(self.spirvcapdict[key].elem)[0:maxlen], file=filehandle)
        write('// VkFormat', file=filehandle)
        for key in self.formatsdict:
            write('    VkFormat', key, '->',
                  etree.tostring(self.formatsdict[key].elem)[0:maxlen], file=filehandle)

    def markTypeRequired(self, typename, required):
        """Require (along with its dependencies) or remove (but not its dependencies) a type.

        - typename - name of type
        - required - boolean (to tag features as required or not)
        """
        self.gen.logMsg('diag', 'tagging type:', typename, '-> required =', required)

        # Get TypeInfo object for <type> tag corresponding to typename
        typeinfo = self.lookupElementInfo(typename, self.typedict)
        if typeinfo is not None:
            if required:
                # Tag type dependencies in 'alias' and 'required' attributes as
                # required. This does not un-tag dependencies in a <remove>
                # tag. See comments in markRequired() below for the reason.
                for attrib_name in ['requires', 'alias']:
                    depname = typeinfo.elem.get(attrib_name)
                    if depname:
                        self.gen.logMsg('diag', 'Generating dependent type',
                                        depname, 'for', attrib_name, 'type', typename)
                        # Do not recurse on self-referential structures.
                        if typename != depname:
                            self.markTypeRequired(depname, required)
                        else:
                            self.gen.logMsg('diag', 'type', typename, 'is self-referential')
                # Tag types used in defining this type (e.g. in nested
                # <type> tags)
                # Look for <type> in entire <command> tree,
                # not just immediate children
                for subtype in typeinfo.elem.findall('.//type'):
                    self.gen.logMsg('diag', 'markRequired: type requires dependent <type>', subtype.text)
                    if typename != subtype.text:
                        self.markTypeRequired(subtype.text, required)
                    else:
                        self.gen.logMsg('diag', 'type', typename, 'is self-referential')
                # Tag enums used in defining this type, for example in
                #   <member><name>member</name>[<enum>MEMBER_SIZE</enum>]</member>
                for subenum in typeinfo.elem.findall('.//enum'):
                    self.gen.logMsg('diag', 'markRequired: type requires dependent <enum>', subenum.text)
                    self.markEnumRequired(subenum.text, required)
                # Tag type dependency in 'bitvalues' attributes as
                # required. This ensures that the bit values for a flag
                # are emitted
                depType = typeinfo.elem.get('bitvalues')
                if depType:
                    self.gen.logMsg('diag', 'Generating bitflag type',
                                    depType, 'for type', typename)
                    self.markTypeRequired(depType, required)
                    group = self.lookupElementInfo(depType, self.groupdict)
                    if group is not None:
                        group.flagType = typeinfo

            typeinfo.required = required
        elif '.h' not in typename:
            self.gen.logMsg('warn', 'type:', typename, 'IS NOT DEFINED')

    def markEnumRequired(self, enumname, required):
        """Mark an enum as required or not.

        - enumname - name of enum
        - required - boolean (to tag features as required or not)"""

        self.gen.logMsg('diag', 'markEnumRequired: tagging enum:', enumname, '-> required =', required)
        enum = self.lookupElementInfo(enumname, self.enumdict)
        if enum is not None:
            # If the enum is part of a group, and is being removed, then
            # look it up in that <enums> tag and remove the Element there,
            # so that it is not visible to generators (which traverse the
            # <enums> tag elements rather than using the dictionaries).
            if not required:
                groupName = enum.elem.get('extends')
                if groupName is not None:
                    self.gen.logMsg('diag', f'markEnumRequired: Removing extending enum {enum.elem.get("name")}')

                    # Look up the Info with matching groupName
                    if groupName in self.groupdict:
                        gi = self.groupdict[groupName]
                        gienum = gi.elem.find(f"enum[@name='{enumname}']")
                        if gienum is not None:
                            # Remove copy of this enum from the group
                            gi.elem.remove(gienum)
                        else:
                            self.gen.logMsg('warn', 'markEnumRequired: Cannot remove enum',
                                            enumname, 'not found in group',
                                            groupName)
                    else:
                        self.gen.logMsg('warn', 'markEnumRequired: Cannot remove enum',
                                        enumname, 'from nonexistent group',
                                        groupName)
                else:
                    # This enum is not an extending enum.
                    # The XML tree must be searched for all <enums> that
                    # might have it, so we know the parent to delete from.

                    enumName = enum.elem.get('name')

                    self.gen.logMsg('diag', f'markEnumRequired: Removing non-extending enum {enumName}')

                    count = 0
                    for enums in self.reg.findall('enums'):
                        for thisEnum in enums.findall('enum'):
                            if thisEnum.get('name') == enumName:
                                # Actually remove it
                                count = count + 1
                                enums.remove(thisEnum)

                    if count == 0:
                        self.gen.logMsg('warn', f'markEnumRequired: {enumName}) not found in any <enums> tag')

            enum.required = required
            # Tag enum dependencies in 'alias' attribute as required
            depname = enum.elem.get('alias')
            if depname:
                self.gen.logMsg('diag', 'markEnumRequired: Generating dependent enum',
                                depname, 'for alias', enumname, 'required =', enum.required)
                self.markEnumRequired(depname, required)
        else:
            self.gen.logMsg('warn', f'markEnumRequired: {enumname} IS NOT DEFINED')

    def markCmdRequired(self, cmdname, required):
        """Mark a command as required or not.

        - cmdname - name of command
        - required - boolean (to tag features as required or not)"""
        self.gen.logMsg('diag', 'tagging command:', cmdname, '-> required =', required)
        cmd = self.lookupElementInfo(cmdname, self.cmddict)
        if cmd is not None:
            cmd.required = required

            # Tag command dependencies in 'alias' attribute as required
            #
            # This is usually not done, because command 'aliases' are not
            # actual C language aliases like type and enum aliases. Instead
            # they are just duplicates of the function signature of the
            # alias. This means that there is no dependency of a command
            # alias on what it aliases. One exception is validity includes,
            # where the spec markup needs the promoted-to validity include
            # even if only the promoted-from command is being built.
            if self.genOpts.requireCommandAliases:
                depname = cmd.elem.get('alias')
                if depname:
                    self.gen.logMsg('diag', 'Generating dependent command',
                                    depname, 'for alias', cmdname)
                    self.markCmdRequired(depname, required)

            # Tag all parameter types of this command as required.
            # This does not remove types of commands in a <remove>
            # tag, because many other commands may use the same type.
            # We could be more clever and reference count types,
            # instead of using a boolean.
            if required:
                # Look for <type> in entire <command> tree,
                # not just immediate children
                for type_elem in cmd.elem.findall('.//type'):
                    self.gen.logMsg('diag', 'markRequired: command implicitly requires dependent type', type_elem.text)
                    self.markTypeRequired(type_elem.text, required)
        else:
            self.gen.logMsg('warn', 'command:', cmdname, 'IS NOT DEFINED')

    def markRequired(self, featurename, feature, required):
        """Require or remove features specified in the Element.

        - featurename - name of the feature
        - feature - Element for `<require>` or `<remove>` tag
        - required - boolean (to tag features as required or not)"""
        self.gen.logMsg('diag', 'markRequired (feature = <too long to print>, required =', required, ')')

        # Loop over types, enums, and commands in the tag
        # @@ It would be possible to respect 'api' and 'profile' attributes
        #  in individual features, but that is not done yet.
        for typeElem in feature.findall('type'):
            self.markTypeRequired(typeElem.get('name'), required)
        for enumElem in feature.findall('enum'):
            self.markEnumRequired(enumElem.get('name'), required)

        for cmdElem in feature.findall('command'):
            self.markCmdRequired(cmdElem.get('name'), required)

        # Extensions may need to extend existing commands or other items in the future.
        # So, look for extend tags.
        for extendElem in feature.findall('extend'):
            extendType = extendElem.get('type')
            if extendType == 'command':
                commandName = extendElem.get('name')
                successExtends = extendElem.get('successcodes')
                if successExtends is not None:
                    for success in successExtends.split(','):
                        self.commandextensionsuccesses.append(self.commandextensiontuple(command=commandName,
                                                                                         value=success,
                                                                                         extension=featurename))
                errorExtends = extendElem.get('errorcodes')
                if errorExtends is not None:
                    for error in errorExtends.split(','):
                        self.commandextensionerrors.append(self.commandextensiontuple(command=commandName,
                                                                                      value=error,
                                                                                      extension=featurename))
            else:
                self.gen.logMsg('warn', 'extend type:', extendType, 'IS NOT SUPPORTED')

    def getAlias(self, elem, dict):
        """Check for an alias in the same require block.

        - elem - Element to check for an alias"""

        # Try to find an alias
        alias = elem.get('alias')
        if alias is None:
            name = elem.get('name')
            typeinfo = self.lookupElementInfo(name, dict)
            if not typeinfo:
                self.gen.logMsg('error', name, 'is not a known name')
            alias = typeinfo.elem.get('alias')

        return alias

    def checkForCorrectionAliases(self, alias, require, tag):
        """Check for an alias in the same require block.

        - alias - String name of the alias
        - require -  `<require>` block from the registry
        - tag - tag to look for in the require block"""

        # For the time being, the code below is bypassed. It has the effect
        # of excluding "spelling aliases" created to comply with the style
        # guide, but this leaves references out of the specification and
        # causes broken internal links.
        #
        # if alias and require.findall(tag + "[@name='" + alias + "']"):
        #     return True

        return False

    def fillFeatureDictionary(self, interface, featurename, api, profile):
        """Capture added interfaces for a `<version>` or `<extension>`.

        - interface - Element for `<version>` or `<extension>`, containing
          `<require>` and `<remove>` tags
        - featurename - name of the feature
        - api - string specifying API name being generated
        - profile - string specifying API profile being generated"""

        # Explicitly initialize known types - errors for unhandled categories
        self.gen.featureDictionary[featurename] = {
            "enumconstant": {},
            "command": {},
            "enum": {},
            "struct": {},
            "handle": {},
            "basetype": {},
            "include": {},
            "define": {},
            "bitmask": {},
            "union": {},
            "funcpointer": {},
        }

        # <require> marks things that are required by this version/profile
        for require in interface.findall('require'):
            if matchAPIProfile(api, profile, require):

                # Determine the required extension or version needed for a require block
                # Assumes that only one of these is specified
                # 'extension', and therefore 'required_key', may be a boolean
                # expression of extension names.
                # 'required_key' is used only as a dictionary key at
                # present, and passed through to the script generators, so
                # they must be prepared to parse that boolean expression.
                required_key = require.get('depends')

                # Loop over types, enums, and commands in the tag
                for typeElem in require.findall('type'):
                    typename = typeElem.get('name')
                    typeinfo = self.lookupElementInfo(typename, self.typedict)

                    if typeinfo:
                        # Remove aliases in the same extension/feature; these are always added as a correction. Do not need the original to be visible.
                        alias = self.getAlias(typeElem, self.typedict)
                        if not self.checkForCorrectionAliases(alias, require, 'type'):
                            # Resolve the type info to the actual type, so we get an accurate read for 'structextends'
                            while alias:
                                typeinfo = self.lookupElementInfo(alias, self.typedict)
                                if not typeinfo:
                                    raise RuntimeError(f"Missing alias {alias}")
                                alias = typeinfo.elem.get('alias')

                            typecat = typeinfo.elem.get('category')
                            typeextends = typeinfo.elem.get('structextends')
                            if not required_key in self.gen.featureDictionary[featurename][typecat]:
                                self.gen.featureDictionary[featurename][typecat][required_key] = {}
                            if not typeextends in self.gen.featureDictionary[featurename][typecat][required_key]:
                                self.gen.featureDictionary[featurename][typecat][required_key][typeextends] = []
                            self.gen.featureDictionary[featurename][typecat][required_key][typeextends].append(typename)
                        else:
                            self.gen.logMsg('warn', f'fillFeatureDictionary: NOT filling for {typename}')


                for enumElem in require.findall('enum'):
                    enumname = enumElem.get('name')
                    typeinfo = self.lookupElementInfo(enumname, self.enumdict)

                    # Remove aliases in the same extension/feature; these are always added as a correction. Do not need the original to be visible.
                    alias = self.getAlias(enumElem, self.enumdict)
                    if not self.checkForCorrectionAliases(alias, require, 'enum'):
                        enumextends = enumElem.get('extends')
                        if not required_key in self.gen.featureDictionary[featurename]['enumconstant']:
                            self.gen.featureDictionary[featurename]['enumconstant'][required_key] = {}
                        if not enumextends in self.gen.featureDictionary[featurename]['enumconstant'][required_key]:
                            self.gen.featureDictionary[featurename]['enumconstant'][required_key][enumextends] = []
                        self.gen.featureDictionary[featurename]['enumconstant'][required_key][enumextends].append(enumname)
                    else:
                        self.gen.logMsg('warn', f'fillFeatureDictionary: NOT filling for {typename}')

                for cmdElem in require.findall('command'):
                    # Remove aliases in the same extension/feature; these are always added as a correction. Do not need the original to be visible.
                    alias = self.getAlias(cmdElem, self.cmddict)
                    if not self.checkForCorrectionAliases(alias, require, 'command'):
                        if not required_key in self.gen.featureDictionary[featurename]['command']:
                            self.gen.featureDictionary[featurename]['command'][required_key] = []
                        self.gen.featureDictionary[featurename]['command'][required_key].append(cmdElem.get('name'))
                    else:
                        self.gen.logMsg('warn', f'fillFeatureDictionary: NOT filling for {typename}')

    def requireFeatures(self, interface, featurename, api, profile):
        """Process `<require>` tags for a `<version>` or `<extension>`.

        - interface - Element for `<version>` or `<extension>`, containing
          `<require>` tags
        - featurename - name of the feature
        - api - string specifying API name being generated
        - profile - string specifying API profile being generated"""

        # <require> marks things that are required by this version/profile
        for feature in interface.findall('require'):
            if matchAPIProfile(api, profile, feature):
                self.markRequired(featurename, feature, True)

    def deprecateFeatures(self, interface, featurename, api, profile):
        """Process `<require>` tags for a `<version>` or `<extension>`.

        - interface - Element for `<version>` or `<extension>`, containing
          `<require>` tags
        - featurename - name of the feature
        - api - string specifying API name being generated
        - profile - string specifying API profile being generated"""

        versionmatch = APIConventions().is_api_version_name(featurename)

        # <deprecate> marks things that are deprecated by this version/profile
        for deprecation in interface.findall('deprecate'):
            if matchAPIProfile(api, profile, deprecation):
                for typeElem in deprecation.findall('type'):
                    type = self.lookupElementInfo(typeElem.get('name'), self.typedict)
                    if type:
                        if versionmatch is not False:
                            type.deprecatedbyversion = featurename
                        else:
                            type.deprecatedbyextensions.append(featurename)
                        type.deprecatedlink = deprecation.get('explanationlink')
                    else:
                        self.gen.logMsg('error', typeElem.get('name'), ' is tagged for deprecation but not present in registry')
                for enumElem in deprecation.findall('enum'):
                    enum = self.lookupElementInfo(enumElem.get('name'), self.enumdict)
                    if enum:
                        if versionmatch is not False:
                            enum.deprecatedbyversion = featurename
                        else:
                            enum.deprecatedbyextensions.append(featurename)
                        enum.deprecatedlink = deprecation.get('explanationlink')
                    else:
                        self.gen.logMsg('error', enumElem.get('name'), ' is tagged for deprecation but not present in registry')
                for cmdElem in deprecation.findall('command'):
                    cmd = self.lookupElementInfo(cmdElem.get('name'), self.cmddict)
                    if cmd:
                        if versionmatch is not False:
                            cmd.deprecatedbyversion = featurename
                        else:
                            cmd.deprecatedbyextensions.append(featurename)
                        cmd.deprecatedlink = deprecation.get('explanationlink')
                    else:
                        self.gen.logMsg('error', cmdElem.get('name'), ' is tagged for deprecation but not present in registry')

    def removeFeatures(self, interface, featurename, api, profile):
        """Process `<remove>` tags for a `<version>` or `<extension>`.

        - interface - Element for `<version>` or `<extension>`, containing
          `<remove>` tags
        - featurename - name of the feature
        - api - string specifying API name being generated
        - profile - string specifying API profile being generated"""

        # <remove> marks things that are removed by this version/profile
        for feature in interface.findall('remove'):
            if matchAPIProfile(api, profile, feature):
                self.markRequired(featurename, feature, False)

    def assignAdditionalValidity(self, interface, api, profile):
        # Loop over all usage inside all <require> tags.
        for feature in interface.findall('require'):
            if matchAPIProfile(api, profile, feature):
                for v in feature.findall('usage'):
                    if v.get('command'):
                        self.cmddict[v.get('command')].additionalValidity.append(copy.deepcopy(v))
                    if v.get('struct'):
                        self.typedict[v.get('struct')].additionalValidity.append(copy.deepcopy(v))

    def removeAdditionalValidity(self, interface, api, profile):
        # Loop over all usage inside all <remove> tags.
        for feature in interface.findall('remove'):
            if matchAPIProfile(api, profile, feature):
                for v in feature.findall('usage'):
                    if v.get('command'):
                        self.cmddict[v.get('command')].removedValidity.append(copy.deepcopy(v))
                    if v.get('struct'):
                        self.typedict[v.get('struct')].removedValidity.append(copy.deepcopy(v))

    def generateFeature(self, fname, ftype, dictionary, explicit=False):
        """Generate a single type / enum group / enum / command,
        and all its dependencies as needed.

        - fname - name of feature (`<type>`/`<enum>`/`<command>`)
        - ftype - type of feature, 'type' | 'enum' | 'command'
        - dictionary - of *Info objects - self.{type|enum|cmd}dict
        - explicit - True if this is explicitly required by the top-level
          XML <require> tag, False if it is a dependency of an explicit
          requirement."""

        self.gen.logMsg('diag', 'generateFeature: generating', ftype, fname)

        if not (explicit or self.genOpts.requireDepends):
            self.gen.logMsg('diag', 'generateFeature: NOT generating', ftype, fname, 'because generator does not require dependencies')
            return

        f = self.lookupElementInfo(fname, dictionary)
        if f is None:
            # No such feature. This is an error, but reported earlier
            self.gen.logMsg('diag', 'No entry found for feature', fname,
                            'returning!')
            return

        # If feature is not required, or has already been declared, return
        if not f.required:
            self.gen.logMsg('diag', 'Skipping', ftype, fname, '(not required)')
            return
        if f.declared:
            self.gen.logMsg('diag', 'Skipping', ftype, fname, '(already declared)')
            return
        # Always mark feature declared, as though actually emitted
        f.declared = True

        # Determine if this is an alias, and of what, if so
        alias = f.elem.get('alias')
        if alias:
            self.gen.logMsg('diag', fname, 'is an alias of', alias)

        # Pull in dependent declaration(s) of the feature.
        # For types, there may be one type in the 'requires' attribute of
        #   the element, one in the 'alias' attribute, and many in
        #   embedded <type> and <enum> tags within the element.
        # For commands, there may be many in <type> tags within the element.
        # For enums, no dependencies are allowed (though perhaps if you
        #   have a uint64 enum, it should require that type).
        genProc = None
        followupFeature = None
        if ftype == 'type':
            genProc = self.gen.genType

            # Generate type dependencies in 'alias' and 'requires' attributes
            if alias:
                self.generateFeature(alias, 'type', self.typedict)
            requires = f.elem.get('requires')
            if requires:
                self.gen.logMsg('diag', 'Generating required dependent type',
                                requires)
                self.generateFeature(requires, 'type', self.typedict)

            # Generate types used in defining this type (e.g. in nested
            # <type> tags)
            # Look for <type> in entire <command> tree,
            # not just immediate children
            for subtype in f.elem.findall('.//type'):
                self.gen.logMsg('diag', 'Generating required dependent <type>',
                                subtype.text)
                self.generateFeature(subtype.text, 'type', self.typedict)

            # Generate enums used in defining this type, for example in
            #   <member><name>member</name>[<enum>MEMBER_SIZE</enum>]</member>
            for subtype in f.elem.findall('.//enum'):
                self.gen.logMsg('diag', 'Generating required dependent <enum>',
                                subtype.text)
                self.generateFeature(subtype.text, 'enum', self.enumdict)

            # If the type is an enum group, look up the corresponding
            # group in the group dictionary and generate that instead.
            if f.elem.get('category') == 'enum':
                self.gen.logMsg('diag', 'Type', fname, 'is an enum group, so generate that instead')
                group = self.lookupElementInfo(fname, self.groupdict)
                if alias is not None:
                    # An alias of another group name.
                    # Pass to genGroup with 'alias' parameter = aliased name
                    self.gen.logMsg('diag', 'Generating alias', fname,
                                    'for enumerated type', alias)
                    # Now, pass the *aliased* GroupInfo to the genGroup, but
                    # with an additional parameter which is the alias name.
                    genProc = self.gen.genGroup
                    f = self.lookupElementInfo(alias, self.groupdict)
                elif group is None:
                    self.gen.logMsg('warn', 'Skipping enum type', fname,
                                    ': No matching enumerant group')
                    return
                else:
                    genProc = self.gen.genGroup
                    f = group

                    # @ The enum group is not ready for generation. At this
                    # @   point, it contains all <enum> tags injected by
                    # @   <extension> tags without any verification of whether
                    # @   they are required or not. It may also contain
                    # @   duplicates injected by multiple consistent
                    # @   definitions of an <enum>.

                    # @ Pass over each enum, marking its enumdict[] entry as
                    # @ required or not. Mark aliases of enums as required,
                    # @ too.

                    enums = group.elem.findall('enum')

                    self.gen.logMsg('diag', 'generateFeature: checking enums for group', fname)

                    # Check for required enums, including aliases
                    # LATER - Check for, report, and remove duplicates?
                    enumAliases = []
                    for elem in enums:
                        name = elem.get('name')

                        required = False

                        extname = elem.get('extname')
                        version = elem.get('version')
                        if extname is not None:
                            # 'supported' attribute was injected when the <enum> element was
                            # moved into the <enums> group in Registry.parseTree()
                            supported_list = elem.get('supported').split(",")
                            if self.genOpts.defaultExtensions in supported_list:
                                required = True
                            elif re.match(self.genOpts.addExtensions, extname) is not None:
                                required = True
                        elif version is not None:
                            required = re.match(self.genOpts.emitversions, version) is not None
                        else:
                            required = True

                        self.gen.logMsg('diag', '* required =', required, 'for', name)
                        if required:
                            # Mark this element as required (in the element, not the EnumInfo)
                            elem.set('required', 'true')
                            # If it is an alias, track that for later use
                            enumAlias = elem.get('alias')
                            if enumAlias:
                                enumAliases.append(enumAlias)
                    for elem in enums:
                        name = elem.get('name')
                        if name in enumAliases:
                            elem.set('required', 'true')
                            self.gen.logMsg('diag', '* also need to require alias', name)
            if f is None:
                raise RuntimeError("Should not get here")
            if f.elem.get('category') == 'bitmask':
                followupFeature = f.elem.get('bitvalues')
        elif ftype == 'command':
            # Generate command dependencies in 'alias' attribute
            if alias:
                self.generateFeature(alias, 'command', self.cmddict)

            genProc = self.gen.genCmd
            for type_elem in f.elem.findall('.//type'):
                depname = type_elem.text
                self.gen.logMsg('diag', 'Generating required parameter type',
                                depname)
                self.generateFeature(depname, 'type', self.typedict)
        elif ftype == 'enum':
            # Generate enum dependencies in 'alias' attribute
            if alias:
                self.generateFeature(alias, 'enum', self.enumdict)
            genProc = self.gen.genEnum

        # Actually generate the type only if emitting declarations
        if self.emitFeatures:
            self.gen.logMsg('diag', 'Emitting', ftype, 'decl for', fname)
            if genProc is None:
                raise RuntimeError("genProc is None when we should be emitting")
            genProc(f, fname, alias)
        else:
            self.gen.logMsg('diag', 'Skipping', ftype, fname,
                            '(should not be emitted)')

        if followupFeature:
            self.gen.logMsg('diag', 'Generating required bitvalues <enum>',
                            followupFeature)
            self.generateFeature(followupFeature, "type", self.typedict)

    def generateRequiredInterface(self, interface):
        """Generate all interfaces required by an API version or extension.

        - interface - Element for `<version>` or `<extension>`"""

        # Loop over all features inside all <require> tags.
        for features in interface.findall('require'):
            for t in features.findall('type'):
                self.generateFeature(t.get('name'), 'type', self.typedict, explicit=True)
            for e in features.findall('enum'):
                # If this is an enum extending an enumerated type, do not
                # generate it - this has already been done in reg.parseTree,
                # by copying this element into the enumerated type.
                enumextends = e.get('extends')
                if not enumextends:
                    self.generateFeature(e.get('name'), 'enum', self.enumdict, explicit=True)
            for c in features.findall('command'):
                self.generateFeature(c.get('name'), 'command', self.cmddict, explicit=True)

    def generateSpirv(self, spirv, dictionary):
        if spirv is None:
            self.gen.logMsg('diag', 'No entry found for element', name,
                            'returning!')
            return

        name = spirv.elem.get('name')
        # No known alias for spirv elements
        alias = None
        if spirv.emit:
            genProc = self.gen.genSpirv
            genProc(spirv, name, alias)

    def stripUnsupportedAPIs(self, dictionary, attribute, supportedDictionary):
        """Strip unsupported APIs from attributes of APIs.
           dictionary - *Info dictionary of APIs to be updated
           attribute - attribute name to look for in each API
           supportedDictionary - dictionary in which to look for supported
            API elements in the attribute"""

        for key in dictionary:
            eleminfo = dictionary[key]
            attribstring = eleminfo.elem.get(attribute)
            if attribstring is not None:
                apis = []
                stripped = False
                for api in attribstring.split(','):
                    ##print('Checking API {} referenced by {}'.format(api, key))
                    if api in supportedDictionary and supportedDictionary[api].required:
                        apis.append(api)
                    else:
                        stripped = True
                        ##print('\t**STRIPPING API {} from {}'.format(api, key))

                # Update the attribute after stripping stuff.
                # Could sort apis before joining, but it is not a clear win
                if stripped:
                    eleminfo.elem.set(attribute, ','.join(apis))

    def stripUnsupportedAPIsFromList(self, dictionary, supportedDictionary):
        """Strip unsupported APIs from attributes of APIs.
           dictionary - dictionary of list of structure name strings
           supportedDictionary - dictionary in which to look for supported
            API elements in the attribute"""

        for key in dictionary:
            attribstring = dictionary[key]
            if attribstring is not None:
                apis = []
                stripped = False
                for api in attribstring:
                    ##print('Checking API {} referenced by {}'.format(api, key))
                    if supportedDictionary[api].required:
                        apis.append(api)
                    else:
                        stripped = True
                        ##print('\t**STRIPPING API {} from {}'.format(api, key))

                # Update the attribute after stripping stuff.
                # Could sort apis before joining, but it is not a clear win
                if stripped:
                    dictionary[key] = apis

    def generateFormat(self, format, dictionary):
        if format is None:
            self.gen.logMsg('diag', 'No entry found for format element',
                            'returning!')
            return

        name = format.elem.get('name')
        # No known alias for VkFormat elements
        alias = None
        if format.emit:
            genProc = self.gen.genFormat
            genProc(format, name, alias)

    def generateSyncStage(self, sync):
        genProc = self.gen.genSyncStage
        genProc(sync)

    def generateSyncAccess(self, sync):
        genProc = self.gen.genSyncAccess
        genProc(sync)

    def generateSyncPipeline(self, sync):
        genProc = self.gen.genSyncPipeline
        genProc(sync)

    def tagValidExtensionStructs(self):
        """Construct a "validextensionstructs" list for parent structures
           based on "structextends" tags in child structures.
           Only do this for structures tagged as required."""

        for typeinfo in self.typedict.values():
            type_elem = typeinfo.elem
            if typeinfo.required and type_elem.get('category') == 'struct':
                struct_extends = type_elem.get('structextends')
                if struct_extends is not None:
                    for parent in struct_extends.split(','):
                        # self.gen.logMsg('diag', type_elem.get('name'), 'extends', parent)
                        self.validextensionstructs[parent].append(type_elem.get('name'))

        # Sort the lists so they do not depend on the XML order
        for parent in self.validextensionstructs:
            self.validextensionstructs[parent].sort()

    def apiGen(self):
        """Generate interface for specified versions using the current
        generator and generator options"""

        self.gen.logMsg('diag', '*******************************************')
        self.gen.logMsg('diag', '  Registry.apiGen file:', self.genOpts.filename,
                        'api:', self.genOpts.apiname,
                        'profile:', self.genOpts.profile)
        self.gen.logMsg('diag', '*******************************************')

        # Could reset required/declared flags for all features here.
        # This has been removed as never used. The initial motivation was
        # the idea of calling apiGen() repeatedly for different targets, but
        # this has never been done. The 20% or so build-time speedup that
        # might result is not worth the effort to make it actually work.
        #
        # self.apiReset()

        # Compile regexps used to select versions & extensions
        regVersions = re.compile(self.genOpts.versions)
        regEmitVersions = re.compile(self.genOpts.emitversions)
        regAddExtensions = re.compile(self.genOpts.addExtensions)
        regRemoveExtensions = re.compile(self.genOpts.removeExtensions)
        regEmitExtensions = re.compile(self.genOpts.emitExtensions)
        regEmitSpirv = re.compile(self.genOpts.emitSpirv)
        regEmitFormats = re.compile(self.genOpts.emitFormats)

        # Get all matching API feature names & add to list of FeatureInfo
        # Note we used to select on feature version attributes, not names.
        features = []
        apiMatch = False
        for key in self.apidict:
            fi = self.apidict[key]
            api = fi.elem.get('api')
            if apiNameMatch(self.genOpts.apiname, api):
                apiMatch = True
                if regVersions.match(fi.name):
                    # Matches API & version #s being generated. Mark for
                    # emission and add to the features[] list .
                    # @@ Could use 'declared' instead of 'emit'?
                    fi.emit = (regEmitVersions.match(fi.name) is not None)
                    features.append(fi)
                    if not fi.emit:
                        self.gen.logMsg('diag', 'NOT tagging feature api =', api,
                                        'name =', fi.name, 'version =', fi.version,
                                        'for emission (does not match emitversions pattern)')
                    else:
                        self.gen.logMsg('diag', 'Including feature api =', api,
                                        'name =', fi.name, 'version =', fi.version,
                                        'for emission (matches emitversions pattern)')
                else:
                    self.gen.logMsg('diag', 'NOT including feature api =', api,
                                    'name =', fi.name, 'version =', fi.version,
                                    '(does not match requested versions)')
            else:
                self.gen.logMsg('diag', 'NOT including feature api =', api,
                                'name =', fi.name,
                                '(does not match requested API)')
        if not apiMatch:
            self.gen.logMsg('warn', 'No matching API versions found!')

        # Get all matching extensions, in order by their extension number,
        # and add to the list of features.
        # Start with extensions whose 'supported' attributes match the API
        # being generated. Add extensions matching the pattern specified in
        # regExtensions, then remove extensions matching the pattern
        # specified in regRemoveExtensions
        for (extName, ei) in sorted(self.extdict.items(), key=lambda x: x[1].number if x[1].number is not None else '0'):
            extName = ei.name
            include = False

            # Include extension if defaultExtensions is not None and is
            # exactly matched by the 'supported' attribute.
            if apiNameMatch(self.genOpts.defaultExtensions,
                            ei.elem.get('supported')):
                self.gen.logMsg('diag', 'Including extension',
                                extName, "(defaultExtensions matches the 'supported' attribute)")
                include = True

            # Include additional extensions if the extension name matches
            # the regexp specified in the generator options. This allows
            # forcing extensions into an interface even if they are not
            # tagged appropriately in the registry.
            # However, we still respect the 'supported' attribute.
            if regAddExtensions.match(extName) is not None:
                if not apiNameMatch(self.genOpts.apiname, ei.elem.get('supported')):
                    self.gen.logMsg('diag', 'NOT including extension',
                                    extName, '(matches explicitly requested, but does not match the \'supported\' attribute)')
                    include = False
                else:
                    self.gen.logMsg('diag', 'Including extension',
                                    extName, '(matches explicitly requested extensions to add)')
                    include = True
            # Remove extensions if the name matches the regexp specified
            # in generator options. This allows forcing removal of
            # extensions from an interface even if they are tagged that
            # way in the registry.
            if regRemoveExtensions.match(extName) is not None:
                self.gen.logMsg('diag', 'Removing extension',
                                extName, '(matches explicitly requested extensions to remove)')
                include = False

            # If the extension is to be included, add it to the
            # extension features list.
            if include:
                ei.emit = (regEmitExtensions.match(extName) is not None)
                features.append(ei)
                if not ei.emit:
                    self.gen.logMsg('diag', 'NOT tagging extension',
                                    extName,
                                    'for emission (does not match emitextensions pattern)')

                # Hack - can be removed when validity generator goes away
                # (Jon) I am not sure what this does, or if it should
                # respect the ei.emit flag above.
                self.requiredextensions.append(extName)
            else:
                self.gen.logMsg('diag', 'NOT including extension',
                                extName, '(does not match api attribute or explicitly requested extensions)')

        # Add all spirv elements to list
        # generators decide to emit them all or not
        # Currently no filtering as no client of these elements needs filtering
        spirvexts = []
        for key in self.spirvextdict:
            si = self.spirvextdict[key]
            si.emit = (regEmitSpirv.match(key) is not None)
            spirvexts.append(si)
        spirvcaps = []
        for key in self.spirvcapdict:
            si = self.spirvcapdict[key]
            si.emit = (regEmitSpirv.match(key) is not None)
            spirvcaps.append(si)

        formats = []
        for key in self.formatsdict:
            si = self.formatsdict[key]
            si.emit = (regEmitFormats.match(key) is not None)
            formats.append(si)

        # Sort the features list, if a sort procedure is defined
        if self.genOpts.sortProcedure:
            self.genOpts.sortProcedure(features)

        # Passes 1+2: loop over requested API versions and extensions tagging
        #   types/commands/features as required (in an <require> block) or no
        #   longer required (in an <remove> block). <remove>s are processed
        #   after all <require>s, so removals win.
        # If a profile other than 'None' is being generated, it must
        #   match the profile attribute (if any) of the <require> and
        #   <remove> tags.
        self.gen.logMsg('diag', 'PASS 1: TAG FEATURES')
        for f in features:
            self.gen.logMsg('diag', 'PASS 1: Tagging required and features for', f.name)
            self.fillFeatureDictionary(f.elem, f.name, self.genOpts.apiname, self.genOpts.profile)
            self.requireFeatures(f.elem, f.name, self.genOpts.apiname, self.genOpts.profile)
            self.deprecateFeatures(f.elem, f.name, self.genOpts.apiname, self.genOpts.profile)
            self.assignAdditionalValidity(f.elem, self.genOpts.apiname, self.genOpts.profile)

        for f in features:
            self.gen.logMsg('diag', 'PASS 2: Tagging removed features for', f.name)
            self.removeFeatures(f.elem, f.name, self.genOpts.apiname, self.genOpts.profile)
            self.removeAdditionalValidity(f.elem, self.genOpts.apiname, self.genOpts.profile)

        # Now, strip references to APIs that are not required.
        # At present such references may occur in:
        #   Structs in <type category="struct"> 'structextends' attributes
        #   Enums in <command> 'successcodes' and 'errorcodes' attributes
        self.stripUnsupportedAPIs(self.typedict, 'structextends', self.typedict)
        self.stripUnsupportedAPIs(self.cmddict, 'successcodes', self.enumdict)
        self.stripUnsupportedAPIs(self.cmddict, 'errorcodes', self.enumdict)
        self.stripUnsupportedAPIsFromList(self.validextensionstructs, self.typedict)

        # Construct lists of valid extension structures
        self.tagValidExtensionStructs()

        # @@May need to strip <spirvcapability> / <spirvextension> <enable>
        # tags of these forms:
        #   <enable version="VK_API_VERSION_1_0"/>
        #   <enable struct="VkPhysicalDeviceFeatures" feature="geometryShader" requires="VK_VERSION_1_0"/>
        #   <enable extension="VK_KHR_shader_draw_parameters"/>
        #   <enable property="VkPhysicalDeviceVulkan12Properties" member="shaderDenormPreserveFloat16" value="VK_TRUE" requires="VK_VERSION_1_2,VK_KHR_shader_float_controls"/>

        # Pass 3: loop over specified API versions and extensions printing
        #   declarations for required things which have not already been
        #   generated.
        self.gen.logMsg('diag', 'PASS 3: GENERATE INTERFACES FOR FEATURES')
        self.gen.beginFile(self.genOpts)
        for f in features:
            self.gen.logMsg('diag', 'PASS 3: Generating interface for',
                            f.name)
            emit = self.emitFeatures = f.emit
            if not emit:
                self.gen.logMsg('diag', 'PASS 3: NOT declaring feature',
                                f.elem.get('name'), 'because it is not tagged for emission')
            # Generate the interface (or just tag its elements as having been
            # emitted, if they have not been).
            self.gen.beginFeature(f.elem, emit)
            self.generateRequiredInterface(f.elem)
            self.gen.endFeature()
        # Generate spirv elements
        for s in spirvexts:
            self.generateSpirv(s, self.spirvextdict)
        for s in spirvcaps:
            self.generateSpirv(s, self.spirvcapdict)
        for s in formats:
            self.generateFormat(s, self.formatsdict)
        for s in self.syncstagedict:
            self.generateSyncStage(self.syncstagedict[s])
        for s in self.syncaccessdict:
            self.generateSyncAccess(self.syncaccessdict[s])
        for s in self.syncpipelinedict:
            self.generateSyncPipeline(self.syncpipelinedict[s])
        self.gen.endFile()

    def apiReset(self):
        """Reset type/enum/command dictionaries before generating another API.

        Use between apiGen() calls to reset internal state."""
        for datatype in self.typedict:
            self.typedict[datatype].resetState()
        for enum in self.enumdict:
            self.enumdict[enum].resetState()
        for cmd in self.cmddict:
            self.cmddict[cmd].resetState()
        for cmd in self.apidict:
            self.apidict[cmd].resetState()
