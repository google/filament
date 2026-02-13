#!/usr/bin/env python3 -i
#
# Copyright 2023-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

import pickle
import os
import tempfile
import copy
from vulkan_object import (VulkanObject,
    Extension, Version, Deprecate, Handle, Param, Queues, CommandScope, Command,
    EnumField, Enum, Flag, Bitmask, ExternSync, Flags, Member, Struct,
    Constant, FormatComponent, FormatPlane, Format,
    SyncSupport, SyncEquivalent, SyncStage, SyncAccess, SyncPipelineStage, SyncPipeline,
    SpirvEnables, Spirv,
    VideoCodec, VideoFormat, VideoProfiles, VideoProfileMember, VideoRequiredCapabilities,
    VideoStd, VideoStdHeader)

# These live in the Vulkan-Docs repo, but are pulled in via the
# Vulkan-Headers/registry folder
from generator import OutputGenerator, GeneratorOptions, write
from vkconventions import VulkanConventions
from reg import Registry
from xml.etree import ElementTree

# An API style convention object
vulkanConventions = VulkanConventions()

# Helpers to keep things cleaner
def splitIfGet(elem, name):
    return elem.get(name).split(',') if elem.get(name) is not None and elem.get(name) != '' else []

def textIfFind(elem, name):
    return elem.find(name).text if elem.find(name) is not None else None

def intIfGet(elem, name):
    return None if elem.get(name) is None else int(elem.get(name), 0)

def boolGet(elem, name) -> bool:
    return elem.get(name) is not None and elem.get(name) == "true"

def externSyncGet(elem):
    value = elem.get('externsync')
    if value is None:
        return (ExternSync.NONE, None)
    if value == 'true':
        return (ExternSync.ALWAYS, None)
    if value == 'maybe':
        return (ExternSync.MAYBE, None)

    # There are no cases where multiple members of the param are marked as
    # externsync.  Supporting that with maybe: requires more than
    # ExternSync.SUBTYPE_MAYBE (which is only one bit of information), which is
    # not currently done as there are no users.
    #
    # If this assert is hit, please consider simplifying the design such that
    # externsync can move to the struct itself and so external synchronization
    # requirements do not depend on the context.
    assert ',' not in value

    if value.startswith('maybe:'):
        return (ExternSync.SUBTYPE_MAYBE, value.removeprefix('maybe:'))
    return (ExternSync.SUBTYPE, value)


def getQueues(elem) -> Queues:
    queues = 0
    queues_list = splitIfGet(elem, 'queues')
    if len(queues_list) > 0:
        queues |= Queues.TRANSFER if 'transfer' in queues_list else 0
        queues |= Queues.GRAPHICS if 'graphics' in queues_list else 0
        queues |= Queues.COMPUTE if 'compute' in queues_list else 0
        queues |= Queues.PROTECTED if 'protected' in queues_list else 0
        queues |= Queues.SPARSE_BINDING if 'sparse_binding' in queues_list else 0
        queues |= Queues.OPTICAL_FLOW if 'opticalflow' in queues_list else 0
        queues |= Queues.DECODE if 'decode' in queues_list else 0
        queues |= Queues.ENCODE if 'encode' in queues_list else 0
        queues |= Queues.DATA_GRAPH if 'data_graph' in queues_list else 0
    return queues

# Shared object used by Sync elements that do not have ones
maxSyncSupport = SyncSupport(None, None, True)
maxSyncEquivalent = SyncEquivalent(None, None, True)

# Helpers to set GeneratorOptions options globally
def SetOutputFileName(fileName: str) -> None:
    global globalFileName
    globalFileName = fileName

def SetOutputDirectory(directory: str) -> None:
    global globalDirectory
    globalDirectory = directory

def SetTargetApiName(apiname: str) -> None:
    global globalApiName
    globalApiName = apiname

def SetMergedApiNames(names: str) -> None:
    global mergedApiNames
    mergedApiNames = names

cachingEnabled = False
def EnableCaching() -> None:
    global cachingEnabled
    cachingEnabled = True

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:
    # Version object factory method
    @staticmethod
    def createApiVersion(targetApiName: str, name: str) -> Version:
        match targetApiName:

            # Vulkan SC specific API version creation
            case 'vulkansc':
                nameApi = name.replace('VK_', 'VK_API_')
                nameApi = nameApi.replace('VKSC_', 'VKSC_API_')
                nameString = f'"{name}"'
                return Version(name, nameString, nameApi)

            # Vulkan specific API version creation
            case 'vulkan':
                nameApi = name.replace('VK_', 'VK_API_')
                nameString = f'"{name}"'
                return Version(name, nameString, nameApi)

    # TODO - Currently genType in reg.py does not provide a good way to get this string to apply the C-macro
    # We do our best to emulate the answer here the way the spec/headers will with goal to have a proper fix before these assumptions break
    @staticmethod
    def createHeaderVersion(targetApiName: str, vk: VulkanObject) -> str:
        match targetApiName:
            case 'vulkan':
                major_version = 1
                minor_version = 4
            case 'vulkansc':
                major_version = 1
                minor_version = 0
        return  f'{major_version}.{minor_version}.{vk.headerVersion}'


# This Generator Option is used across all generators.
# After years of use, it has shown that most the options are unified across each generator (file)
# as it is easier to modify things per-file that need the difference
class BaseGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 customFileName = None,
                 customDirectory = None,
                 customApiName = None,
                 videoXmlPath = None):
        GeneratorOptions.__init__(self,
                conventions = vulkanConventions,
                filename = customFileName if customFileName else globalFileName,
                directory = customDirectory if customDirectory else globalDirectory,
                apiname = customApiName if customApiName else globalApiName,
                mergeApiNames = mergedApiNames,
                defaultExtensions = customApiName if customApiName else globalApiName,
                emitExtensions = '.*',
                emitSpirv = '.*',
                emitFormats = '.*')
        # These are used by the generator.py script
        self.apicall         = 'VKAPI_ATTR '
        self.apientry        = 'VKAPI_CALL '
        self.apientryp       = 'VKAPI_PTR *'
        self.alignFuncParam  = 48

        # This is used to provide the video.xml to the private video XML generator
        self.videoXmlPath = videoXmlPath

#
# This object handles all the parsing from reg.py generator scripts in the Vulkan-Headers
# It will grab all the data and form it into a single object the rest of the generators will use
class BaseGenerator(OutputGenerator):
    def __init__(self):
        OutputGenerator.__init__(self, None, None, None)
        self.vk = VulkanObject()
        self.targetApiName = globalApiName

        # reg.py has a `self.featureName` but this is nicer because
        # it will be either the Version or Extension object
        self.currentExtension = None
        self.currentVersion = None

        # We need to flag extensions that we ignore because they are disabled or not
        # supported in the target API(s)
        self.unsupportedExtension = False

        # Will map alias to promoted name
        #   ex. ['VK_FILTER_CUBIC_IMG' : 'VK_FILTER_CUBIC_EXT']
        # When generating any code, there is no reason so use the old name
        self.enumAliasMap = dict()
        self.enumFieldAliasMap = dict()
        self.bitmaskAliasMap = dict()
        self.flagAliasMap = dict()
        self.flagsAliasMap = dict()
        self.structAliasMap = dict()
        self.handleAliasMap = dict()

        # We track all enum constants and flag bits so that we can apply their aliases in the end
        self.enumFieldMap: dict[str, EnumField] = dict()
        self.flagMap: dict[str, Flag] = dict()

    # De-aliases a definition name based on the specified alias map.
    # There are aliases of aliases.
    # e.g. VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR aliases
    # VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES_KHR which itself aliases
    # But it is also common for EXT types promoted to KHR then to core.
    # We should not make assumptions about the nesting level of aliases, instead we resolve any
    # level of alias aliasing.
    def dealias(self, name: str, aliasMap: dict):
        while name in aliasMap:
            name = aliasMap[name]
        return name

    def write(self, data):
        # Prevents having to check before writing
        if data is not None and data != "":
            write(data, file=self.outFile)

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.filename = genOpts.filename

        # No gen*() command to get these, so do it manually
        for platform in self.registry.tree.findall('platforms/platform'):
            self.vk.platforms[platform.get('name')] = platform.get('protect')

        for tags in self.registry.tree.findall('tags'):
            for tag in tags.findall('tag'):
                self.vk.vendorTags.append(tag.get('name'))

        # No way known to get this from the XML
        self.vk.queueBits[Queues.TRANSFER]       = 'VK_QUEUE_TRANSFER_BIT'
        self.vk.queueBits[Queues.GRAPHICS]       = 'VK_QUEUE_GRAPHICS_BIT'
        self.vk.queueBits[Queues.COMPUTE]        = 'VK_QUEUE_COMPUTE_BIT'
        self.vk.queueBits[Queues.PROTECTED]      = 'VK_QUEUE_PROTECTED_BIT'
        self.vk.queueBits[Queues.SPARSE_BINDING] = 'VK_QUEUE_SPARSE_BINDING_BIT'
        self.vk.queueBits[Queues.OPTICAL_FLOW]   = 'VK_QUEUE_OPTICAL_FLOW_BIT_NV'
        self.vk.queueBits[Queues.DECODE]         = 'VK_QUEUE_VIDEO_DECODE_BIT_KHR'
        self.vk.queueBits[Queues.ENCODE]         = 'VK_QUEUE_VIDEO_ENCODE_BIT_KHR'

        # If the video.xml path is provided then we need to load and parse it using
        # the private video std generator
        if genOpts.videoXmlPath is not None:
            videoStdGenerator = _VideoStdGenerator()
            videoRegistry = Registry(videoStdGenerator, genOpts)
            videoRegistry.loadElementTree(ElementTree.parse(genOpts.videoXmlPath))
            videoRegistry.apiGen()
            self.vk.videoStd = videoStdGenerator.vk.videoStd

    # This function should be overloaded
    def generate(self):
        print("WARNING: This should not be called from the child class")
        return

    # This function is dense, it does all the magic to set the right extensions dependencies!
    #
    # The issue is if 2 extension expose a command, genCmd() will only
    # show one of the extension, at endFile() we can finally go through
    # and update which things depend on which extensions
    #
    # self.featureDictionary is built for use in the reg.py framework
    # Details found in Vulkan-Docs/scripts/scriptgenerator.py
    def applyExtensionDependency(self):
        for extension in self.vk.extensions.values():
            # dict.key() can be None, so need to double loop
            dict = self.featureDictionary[extension.name]['command']

            # "required" == None
            #         or
            #  an additional feature dependency, which is a boolean expression of
            #  one or more extension and/or core version names
            for required in dict:
                for commandName in dict[required]:
                    # Skip commands removed in the target API
                    # This check is needed because parts of the base generator code bypass the
                    # dependency resolution logic in the registry tooling and thus the generator
                    # may attempt to generate code for commands which are not supported in the
                    # target API variant, thus this check needs to happen even if any specific
                    # target API variant may not specifically need it
                    if not commandName in self.vk.commands:
                        continue

                    command = self.vk.commands[commandName]
                    # Make sure list is unique
                    command.extensions.extend([extension.name] if extension.name not in command.extensions else [])
                    extension.commands.extend([command] if command not in extension.commands else [])

            # While genGroup() will call twice with aliased value, it does not provide all the information we need
            dict = self.featureDictionary[extension.name]['enumconstant']
            for required in dict:
                # group can be a Enum or Bitmask
                for group in dict[required]:
                    if group in self.vk.handles:
                        handle = self.vk.handles[group]
                        # Make sure list is unique
                        handle.extensions.extend([extension.name] if extension.name not in handle.extensions else [])
                        extension.handles[group].extend([handle] if handle not in extension.handles[group] else [])
                    if group in self.vk.enums:
                        if group not in extension.enumFields:
                            extension.enumFields[group] = [] # Dict needs init
                        enum = self.vk.enums[group]
                        # Need to convert all alias so they match what is in EnumField
                        enumList = list(map(lambda x: x if x not in self.enumFieldAliasMap else self.dealias(x, self.enumFieldAliasMap), dict[required][group]))

                        for enumField in [x for x in enum.fields if x.name in enumList]:
                            # Make sure list is unique
                            enum.fieldExtensions.extend([extension.name] if extension.name not in enum.fieldExtensions else [])
                            enumField.extensions.extend([extension.name] if extension.name not in enumField.extensions else [])
                            extension.enumFields[group].extend([enumField] if enumField not in extension.enumFields[group] else [])
                    if group in self.vk.bitmasks:
                        if group not in extension.flagBits:
                            extension.flagBits[group] = [] # Dict needs init
                        bitmask = self.vk.bitmasks[group]
                        # Need to convert all alias so they match what is in Flags
                        flagList = list(map(lambda x: x if x not in self.flagAliasMap else self.dealias(x, self.flagAliasMap), dict[required][group]))

                        for flags in [x for x in bitmask.flags if x.name in flagList]:
                            # Make sure list is unique
                            bitmask.flagExtensions.extend([extension.name] if extension.name not in bitmask.flagExtensions else [])
                            flags.extensions.extend([extension.name] if extension.name not in flags.extensions else [])
                            extension.flagBits[group].extend([flags] if flags not in extension.flagBits[group] else [])
                    if group in self.vk.flags:
                        flags = self.vk.flags[group]
                        # Make sure list is unique
                        flags.extensions.extend([extension.name] if extension.name not in flags.extensions else [])
                        extension.flags.extend([flags] if flags not in extension.flags[group] else [])

        # Need to do 'enum'/'bitmask' after 'enumconstant' has applied everything so we can add implicit extensions
        #
        # Sometimes two extensions enable an Enum, but the newer extension version has extra flags allowed
        # This information seems to be implicit, so need to update it here
        # Go through each Flag and append the Enum extension to it
        #
        # ex. VkAccelerationStructureTypeKHR where GENERIC_KHR is not allowed with just VK_NV_ray_tracing
        # This only works because the values are aliased as well, making the KHR a superset enum
        for extension in self.vk.extensions.values():
            dict = self.featureDictionary[extension.name]['enum']
            for required in dict:
                for group in dict[required]:
                    for enumName in dict[required][group]:
                        isAlias = enumName in self.enumAliasMap
                        enumName = self.dealias(enumName, self.enumAliasMap)
                        if enumName in self.vk.enums:
                            enum = self.vk.enums[enumName]
                            enum.extensions.extend([extension.name] if extension.name not in enum.extensions else [])
                            extension.enums.extend([enum] if enum not in extension.enums else [])
                            # Update fields with implicit base extension
                            if isAlias:
                                continue
                            enum.fieldExtensions.extend([extension.name] if extension.name not in enum.fieldExtensions else [])
                            for enumField in [x for x in enum.fields if (not x.extensions or (x.extensions and all(e in enum.extensions for e in x.extensions)))]:
                                enumField.extensions.extend([extension.name] if extension.name not in enumField.extensions else [])
                                if enumName not in extension.enumFields:
                                    extension.enumFields[enumName] = [] # Dict needs init
                                extension.enumFields[enumName].extend([enumField] if enumField not in extension.enumFields[enumName] else [])

            dict = self.featureDictionary[extension.name]['bitmask']
            for required in dict:
                for group in dict[required]:
                    for bitmaskName in dict[required][group]:
                        bitmaskName = bitmaskName.replace('Flags', 'FlagBits') # Works since Flags is not repeated in name
                        isAlias = bitmaskName in self.bitmaskAliasMap
                        bitmaskName = self.dealias(bitmaskName, self.bitmaskAliasMap)
                        if bitmaskName in self.vk.bitmasks:
                            bitmask = self.vk.bitmasks[bitmaskName]
                            bitmask.extensions.extend([extension.name] if extension.name not in bitmask.extensions else [])
                            extension.bitmasks.extend([bitmask] if bitmask not in extension.bitmasks else [])
                            # Update flags with implicit base extension
                            if isAlias:
                                continue
                            bitmask.flagExtensions.extend([extension.name] if extension.name not in bitmask.flagExtensions else [])
                            for flag in [x for x in bitmask.flags if (not x.extensions or (x.extensions and all(e in bitmask.extensions for e in x.extensions)))]:
                                flag.extensions.extend([extension.name] if extension.name not in flag.extensions else [])
                                if bitmaskName not in extension.flagBits:
                                    extension.flagBits[bitmaskName] = [] # Dict needs init
                                extension.flagBits[bitmaskName].extend([flag] if flag not in extension.flagBits[bitmaskName] else [])

        # Some structs (ex VkAttachmentSampleCountInfoAMD) can have multiple alias pointing to same extension
        for extension in self.vk.extensions.values():
            dict = self.featureDictionary[extension.name]['struct']
            for required in dict:
                for group in dict[required]:
                    for structName in dict[required][group]:
                        isAlias = structName in self.structAliasMap
                        structName = self.dealias(structName, self.structAliasMap)
                        if structName in self.vk.structs:
                            struct = self.vk.structs[structName]
                            struct.extensions.extend([extension.name] if extension.name not in struct.extensions else [])

        # While we update struct alias inside other structs, the command itself might have the struct as a first level param.
        # We use this time to update params to have the promoted name
        # Example - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9322
        # TODO: It is unclear why only structs need dealiasing here, but not other types, so this probably needs revisiting
        for command in self.vk.commands.values():
            for member in command.params:
                if member.type in self.structAliasMap:
                    member.type = self.dealias(member.type, self.structAliasMap)
            # Replace string with Version class now we have all version created
            if command.deprecate and command.deprecate.version:
                command.deprecate.version = self.vk.versions[command.deprecate.version]

        # Could build up a reverse lookup map, but since these are not too large of list, just do here
        # (Need to be done after we have found all the aliases)
        for key, value in self.structAliasMap.items():
            self.vk.structs[self.dealias(value, self.structAliasMap)].aliases.append(key)
        for key, value in self.enumFieldAliasMap.items():
            self.enumFieldMap[self.dealias(value, self.enumFieldAliasMap)].aliases.append(key)
        for key, value in self.enumAliasMap.items():
            self.vk.enums[self.dealias(value, self.enumAliasMap)].aliases.append(key)
        for key, value in self.flagAliasMap.items():
            self.flagMap[self.dealias(value, self.flagAliasMap)].aliases.append(key)
        for key, value in self.bitmaskAliasMap.items():
            self.vk.bitmasks[self.dealias(value, self.bitmaskAliasMap)].aliases.append(key)
        for key, value in self.flagsAliasMap.items():
            self.vk.flags[self.dealias(value, self.flagsAliasMap)].aliases.append(key)
        for key, value in self.handleAliasMap.items():
            self.vk.handles[self.dealias(value, self.handleAliasMap)].aliases.append(key)

    def addConstants(self, constantNames: list[str]):
        for constantName in constantNames:
            enumInfo = self.registry.enumdict[constantName]
            typeName = enumInfo.type
            valueStr = enumInfo.elem.get('value')
            # These values are represented in c-style
            isHex = valueStr.upper().startswith('0X')
            intBase = 16 if isHex else 10
            if valueStr.upper().endswith('F') and not isHex:
                value = float(valueStr[:-1])
            elif valueStr.upper().endswith('U)'):
                inner_number = int(valueStr.removeprefix("(~").removesuffix(")")[:-1], intBase)
                value = (~inner_number) & ((1 << 32) - 1)
            elif valueStr.upper().endswith('ULL)'):
                inner_number = int(valueStr.removeprefix("(~").removesuffix(")")[:-3], intBase)
                value = (~0) & ((1 << 64) - 1)
            else:
                value = int(valueStr, intBase)
            self.vk.constants[constantName] = Constant(constantName, typeName, value, valueStr)

    def addVideoCodecs(self):
        for xmlVideoCodec in self.registry.tree.findall('videocodecs/videocodec'):
            name = xmlVideoCodec.get('name')
            extend = xmlVideoCodec.get('extend')
            value = xmlVideoCodec.get('value')

            profiles: dict[str, VideoProfiles] = {}
            capabilities: dict[str, str] = {}
            formats: dict[str, VideoFormat] = {}

            if extend is not None:
                # Inherit base profiles, capabilities, and formats
                profiles = copy.deepcopy(self.vk.videoCodecs[extend].profiles)
                capabilities = copy.deepcopy(self.vk.videoCodecs[extend].capabilities)
                formats = copy.deepcopy(self.vk.videoCodecs[extend].formats)

            for xmlVideoProfiles in xmlVideoCodec.findall('videoprofiles'):
                videoProfileStructName = xmlVideoProfiles.get('struct')
                videoProfileStructMembers : dict[str, VideoProfileMember] = {}

                for xmlVideoProfileMember in xmlVideoProfiles.findall('videoprofilemember'):
                    memberName = xmlVideoProfileMember.get('name')
                    memberValues: dict[str, str] = {}

                    for xmlVideoProfile in xmlVideoProfileMember.findall('videoprofile'):
                        memberValues[xmlVideoProfile.get('value')] = xmlVideoProfile.get('name')

                    videoProfileStructMembers[memberName] = VideoProfileMember(memberName, memberValues)

                profiles[videoProfileStructName] = VideoProfiles(videoProfileStructName, videoProfileStructMembers)

            for xmlVideoCapabilities in xmlVideoCodec.findall('videocapabilities'):
                capabilities[xmlVideoCapabilities.get('struct')] = xmlVideoCapabilities.get('struct')

            for xmlVideoFormat in xmlVideoCodec.findall('videoformat'):
                videoFormatName = xmlVideoFormat.get('name')
                videoFormatExtend = xmlVideoFormat.get('extend')

                videoFormatRequiredCaps: list[VideoRequiredCapabilities] = []
                videoFormatProps: dict[str, str] = {}

                if videoFormatName is not None:
                    # This is a new video format category
                    videoFormatUsage = xmlVideoFormat.get('usage')
                    videoFormat = VideoFormat(videoFormatName, videoFormatUsage, videoFormatRequiredCaps, videoFormatProps)
                    formats[videoFormatName] = videoFormat
                else:
                    # This is an extension to an already defined video format category
                    videoFormat = formats[videoFormatExtend]
                    videoFormatRequiredCaps = videoFormat.requiredCaps
                    videoFormatProps = videoFormat.properties

                for xmlVideoFormatRequiredCap in xmlVideoFormat.findall('videorequirecapabilities'):
                    requiredCap = VideoRequiredCapabilities(xmlVideoFormatRequiredCap.get('struct'),
                                                            xmlVideoFormatRequiredCap.get('member'),
                                                            xmlVideoFormatRequiredCap.get('value'))
                    videoFormatRequiredCaps.append(requiredCap)

                for xmlVideoFormatProperties in xmlVideoFormat.findall('videoformatproperties'):
                    videoFormatProps[xmlVideoFormatProperties.get('struct')] = xmlVideoFormatProperties.get('struct')

            self.vk.videoCodecs[name] = VideoCodec(name, value, profiles, capabilities, formats)

    def endFile(self):
        # This is the point were reg.py has ran, everything is collected
        # We do some post processing now
        self.applyExtensionDependency()

        self.addConstants([k for k,v in self.registry.enumvaluedict.items() if v == 'API Constants'])
        self.addVideoCodecs()

        self.vk.headerVersionComplete = APISpecific.createHeaderVersion(self.targetApiName, self.vk)

        # Use structs and commands to find which things are returnedOnly
        for struct in [x for x in self.vk.structs.values() if not x.returnedOnly]:
            for enum in [self.vk.enums[x.type] for x in struct.members if x.type in self.vk.enums]:
                enum.returnedOnly = False
            for bitmask in [self.vk.bitmasks[x.type] for x in struct.members if x.type in self.vk.bitmasks]:
                bitmask.returnedOnly = False
            for flags in [self.vk.flags[x.type] for x in struct.members if x.type in self.vk.flags]:
                flags.returnedOnly = False
                if flags.bitmaskName is not None:
                    self.vk.bitmasks[flags.bitmaskName].returnedOnly = False
        for command in self.vk.commands.values():
            for enum in [self.vk.enums[x.type] for x in command.params if x.type in self.vk.enums]:
                enum.returnedOnly = False
            for bitmask in [self.vk.bitmasks[x.type] for x in command.params if x.type in self.vk.bitmasks]:
                bitmask.returnedOnly = False
            for flags in [self.vk.flags[x.type] for x in command.params if x.type in self.vk.flags]:
                flags.returnedOnly = False
                if flags.bitmaskName is not None:
                    self.vk.bitmasks[flags.bitmaskName].returnedOnly = False

        # Turn handle parents into pointers to classes
        for handle in [x for x in self.vk.handles.values() if x.parent is not None]:
            handle.parent = self.vk.handles[handle.parent]
        # search up parent chain to see if instance or device
        for handle in [x for x in self.vk.handles.values()]:
            next_parent = handle.parent
            while (not handle.instance and not handle.device):
                handle.instance = next_parent.name == 'VkInstance'
                handle.device = next_parent.name == 'VkDevice'
                next_parent = next_parent.parent

        maxSyncSupport.stages = self.vk.bitmasks['VkPipelineStageFlagBits2'].flags
        maxSyncEquivalent.accesses = self.vk.bitmasks['VkAccessFlagBits2'].flags
        maxSyncEquivalent.stages = self.vk.bitmasks['VkPipelineStageFlagBits2'].flags

        # All inherited generators should run from here
        self.generate()

        if cachingEnabled:
            cachePath = os.path.join(tempfile.gettempdir(), f'vkobject_{os.getpid()}')
            if not os.path.isfile(cachePath):
                cacheFile = open(cachePath, 'wb')
                pickle.dump(self.vk, cacheFile)
                cacheFile.close()

        # This should not have to do anything but call into OutputGenerator
        OutputGenerator.endFile(self)

    #
    # Bypass the entire processing and load in the VkObject data
    # Still need to handle the beingFile/endFile for reg.py
    def generateFromCache(self, cacheVkObjectData, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.filename = genOpts.filename
        self.vk = cacheVkObjectData
        self.generate()
        OutputGenerator.endFile(self)

    #
    # Processing point at beginning of each extension definition
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        platform = interface.get('platform')
        self.featureExtraProtec = self.vk.platforms[platform] if platform in self.vk.platforms else None
        protect = self.vk.platforms[platform] if platform in self.vk.platforms else None
        name = interface.get('name')

        if interface.tag == 'extension':
            # Generator scripts built on BaseGenerator do not handle the `supported` attribute of extensions
            # therefore historically the `generate_source.py` in individual ecosystem components hacked the
            # registry by removing non-applicable or disabled extensions from the loaded XML already before
            # reg.py parsed it. That broke the general behavior of reg.py for certain use cases so we now
            # filter extensions here instead (after parsing) in order to no longer need the filtering hack
            # in downstream `generate_source.py` scripts.
            enabledApiList = [ globalApiName ] + ([] if mergedApiNames is None else mergedApiNames.split(','))
            if (sup := interface.get('supported')) is not None and all(api not in sup.split(',') for api in enabledApiList):
                self.unsupportedExtension = True
                return

            instance = interface.get('type') == 'instance'
            device = not instance
            depends = interface.get('depends')
            vendorTag = interface.get('author')
            platform = interface.get('platform')
            provisional = boolGet(interface, 'provisional')
            promotedto = interface.get('promotedto')
            deprecatedby = interface.get('deprecatedby')
            obsoletedby = interface.get('obsoletedby')
            specialuse = splitIfGet(interface, 'specialuse')
            ratifiedApis = splitIfGet(interface, 'ratified')
            ratified = True if len(ratifiedApis) > 0 and self.genOpts.apiname in ratifiedApis else False

            # Not sure if better way to get this info
            specVersion = self.featureDictionary[name]['enumconstant'][None][None][0]
            nameString = self.featureDictionary[name]['enumconstant'][None][None][1]

            self.currentExtension = Extension(name, nameString, specVersion, instance, device, depends, vendorTag,
                                            platform, protect, provisional, promotedto, deprecatedby,
                                            obsoletedby, specialuse, ratified)
            self.vk.extensions[name] = self.currentExtension
        else: # version
            number = interface.get('number')
            if number != '1.0':
                self.currentVersion = APISpecific.createApiVersion(self.targetApiName, name)
                self.vk.versions[name] = self.currentVersion

    def endFeature(self):
        OutputGenerator.endFeature(self)
        self.currentExtension = None
        self.currentVersion = None
        self.unsupportedExtension = False

    #
    # All <command> from XML
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        # Do not include APIs from unsupported extensions
        if self.unsupportedExtension:
            return

        params = []
        for param in cmdinfo.elem.findall('param'):
            paramName = param.find('name').text
            paramType = textIfFind(param, 'type')
            paramAlias = param.get('alias')

            cdecl = self.makeCParamDecl(param, 0)
            paramFullType = ' '.join(cdecl.split()[:-1])
            pointer = '*' in cdecl or paramType.startswith('PFN_')
            paramConst = 'const' in cdecl
            fixedSizeArray = [x[:-1] for x in cdecl.split('[') if x.endswith(']')]

            paramNoautovalidity = boolGet(param, 'noautovalidity')

            nullTerminated = False
            length = param.get('altlen') if param.get('altlen') is not None else param.get('len')
            if length:
                # we will either find it like "null-terminated" or "enabledExtensionCount,null-terminated"
                # This finds both
                nullTerminated = 'null-terminated' in length
                length = length.replace(',null-terminated', '') if 'null-terminated' in length else length
                length = None if length == 'null-terminated' else length

            if fixedSizeArray and not length:
                length = ','.join(fixedSizeArray)

            # See Member::optional code for details of this
            optionalValues = splitIfGet(param, 'optional')
            optional = len(optionalValues) > 0 and optionalValues[0].lower() == "true"
            optionalPointer = len(optionalValues) > 1 and optionalValues[1].lower() == "true"

            # externsync will be 'true', 'maybe', '<expression>' or 'maybe:<expression>'
            (externSync, externSyncPointer) = externSyncGet(param)

            params.append(Param(paramName, paramAlias, paramType, paramFullType, paramNoautovalidity,
                                paramConst, length, nullTerminated, pointer, fixedSizeArray,
                                optional, optionalPointer,
                                externSync, externSyncPointer, cdecl))

        attrib = cmdinfo.elem.attrib
        alias = attrib.get('alias')
        tasks = splitIfGet(attrib, 'tasks')

        queues = getQueues(attrib)
        allowNoQueues = boolGet(attrib, 'allownoqueues')
        successcodes = splitIfGet(attrib, 'successcodes')
        errorcodes = splitIfGet(attrib, 'errorcodes')
        cmdbufferlevel = attrib.get('cmdbufferlevel')
        primary = cmdbufferlevel is not None and 'primary' in cmdbufferlevel
        secondary = cmdbufferlevel is not None and 'secondary' in cmdbufferlevel

        renderpass = attrib.get('renderpass')
        renderpass = CommandScope.NONE if renderpass is None else getattr(CommandScope, renderpass.upper())
        videocoding = attrib.get('videocoding')
        videocoding = CommandScope.NONE if videocoding is None else getattr(CommandScope, videocoding.upper())

        protoElem = cmdinfo.elem.find('proto')
        returnType = textIfFind(protoElem, 'type')

        decls = self.makeCDecls(cmdinfo.elem)
        cPrototype = decls[0]
        cFunctionPointer = decls[1]

        deprecate = None
        if cmdinfo.deprecatedlink:
            deprecate = Deprecate(cmdinfo.deprecatedlink,
                                  cmdinfo.deprecatedbyversion, # is just the string, will update to class later
                                  cmdinfo.deprecatedbyextensions)

        protect = self.currentExtension.protect if self.currentExtension is not None else None

        # These coammds have no way from the XML to detect they would be an instance command
        specialInstanceCommand = ['vkCreateInstance', 'vkEnumerateInstanceExtensionProperties','vkEnumerateInstanceLayerProperties', 'vkEnumerateInstanceVersion']
        instance = len(params) > 0 and (params[0].type == 'VkInstance' or params[0].type == 'VkPhysicalDevice' or name in specialInstanceCommand)
        device = not instance

        implicitElem = cmdinfo.elem.find('implicitexternsyncparams')
        implicitExternSyncParams = [x.text for x in implicitElem.findall('param')] if implicitElem else []

        self.vk.commands[name] = Command(name, alias, protect, [], self.currentVersion,
                                         returnType, params, instance, device,
                                         tasks, queues, allowNoQueues, successcodes, errorcodes,
                                         primary, secondary, renderpass, videocoding,
                                         implicitExternSyncParams, deprecate, cPrototype, cFunctionPointer)

    #
    # List the enum for the commands
    # TODO - Seems empty groups like `VkDeviceDeviceMemoryReportCreateInfoEXT` do not show up in here
    def genGroup(self, groupinfo, groupName, alias):
        # Do not include APIs from unsupported extensions
        if self.unsupportedExtension:
            return

        # There can be case where the Enum/Bitmask is in a protect, but the individual
        # fields also have their own protect
        groupProtect = self.currentExtension.protect if hasattr(self.currentExtension, 'protect') and self.currentExtension.protect is not None else None
        enumElem = groupinfo.elem
        bitwidth = 32 if enumElem.get('bitwidth') is None else int(enumElem.get('bitwidth'))
        fields = []
        if enumElem.get('type') == "enum":
            if alias is not None:
                self.enumAliasMap[groupName] = alias
                return

            for elem in enumElem.findall('enum'):
                fieldName = elem.get('name')

                # Do not include non-required enum constants
                # reg.py emits the enum constants of the entire type, even constants that are part of unsupported
                # extensions or those that are removed by <remove> elements in a given API. reg.py correctly tracks
                # down these and also alias dependencies and marks the enum constants that are actually required
                # with the 'required' attribute. Therefore we also have to verify that here to make sure we only
                # include enum constants that are actually required in the target API(s).
                if elem.get('required') is None:
                    continue

                if elem.get('alias') is not None:
                    self.enumFieldAliasMap[fieldName] = elem.get('alias')
                    continue

                negative = elem.get('dir') is not None
                protect = elem.get('protect')
                (valueInt, valueStr) = self.enumToValue(elem, True, bitwidth)

                # Some values have multiple extensions (ex VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
                # genGroup() lists them twice
                if next((x for x in fields if x.name == fieldName), None) is None:
                    self.enumFieldMap[fieldName] = EnumField(fieldName, [], protect, negative, valueInt, valueStr, [])
                    fields.append(self.enumFieldMap[fieldName])

            self.vk.enums[groupName] = Enum(groupName, [], groupProtect, bitwidth, True, fields, [], [])

        else: # "bitmask"
            if alias is not None:
                self.bitmaskAliasMap[groupName] = alias
                return

            for elem in enumElem.findall('enum'):
                flagName = elem.get('name')

                # Do not include non-required enum constants
                # reg.py emits the enum constants of the entire type, even constants that are part of unsupported
                # extensions or those that are removed by <remove> elements in a given API. reg.py correctly tracks
                # down these and also alias dependencies and marks the enum constants that are actually required
                # with the 'required' attribute. Therefore we also have to verify that here to make sure we only
                # include enum constants that are actually required in the target API(s).
                if elem.get('required') is None:
                    continue

                if elem.get('alias') is not None:
                    self.flagAliasMap[flagName] = elem.get('alias')
                    continue

                protect = elem.get('protect')

                (valueInt, valueStr) = self.enumToValue(elem, True, bitwidth)
                flagZero = valueInt == 0
                flagMultiBit = False
                # if flag uses 'value' instead of 'bitpos', will be zero or a mask
                if elem.get('bitpos') is None and elem.get('value'):
                    flagMultiBit = valueInt != 0

                # Some values have multiple extensions (ex VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT)
                # genGroup() lists them twice
                if next((x for x in fields if x.name == flagName), None) is None:
                    self.flagMap[flagName] = Flag(flagName, [], protect, valueInt, valueStr, flagMultiBit, flagZero, [])
                    fields.append(self.flagMap[flagName])

            flagName = groupName.replace('FlagBits', 'Flags')
            self.vk.bitmasks[groupName] = Bitmask(groupName, [], flagName, groupProtect, bitwidth, True, fields, [], [])

    def genType(self, typeInfo, typeName, alias):
        OutputGenerator.genType(self, typeInfo, typeName, alias)

        # Do not include APIs from unsupported extensions
        if self.unsupportedExtension:
            return

        typeElem = typeInfo.elem
        protect = self.currentExtension.protect if hasattr(self.currentExtension, 'protect') and self.currentExtension.protect is not None else None
        extension = [self.currentExtension.name] if self.currentExtension is not None else []
        category = typeElem.get('category')
        if (category == 'struct' or category == 'union'):
            if alias is not None:
                self.structAliasMap[typeName] = alias
                return

            union = category == 'union'

            returnedOnly = boolGet(typeElem, 'returnedonly')
            allowDuplicate = boolGet(typeElem, 'allowduplicate')

            extends = splitIfGet(typeElem, 'structextends')
            extendedBy = self.registry.validextensionstructs[typeName] if len(self.registry.validextensionstructs[typeName]) > 0 else []

            membersElem = typeInfo.elem.findall('.//member')
            members = []
            sType = None

            for member in membersElem:
                for comment in member.findall('comment'):
                    member.remove(comment)

                name = textIfFind(member, 'name')
                type = textIfFind(member, 'type')
                sType = member.get('values') if member.get('values') is not None else sType
                noautovalidity = boolGet(member, 'noautovalidity')
                limittype = member.get('limittype')

                (externSync, externSyncPointer) = externSyncGet(member)
                # No cases currently where a subtype of a struct is marked as externally synchronized.
                assert externSyncPointer is None

                nullTerminated = False
                length = member.get('altlen') if member.get('altlen') is not None else member.get('len')
                if length:
                    # we will either find it like "null-terminated" or "enabledExtensionCount,null-terminated"
                    # This finds both
                    nullTerminated = 'null-terminated' in length
                    length = length.replace(',null-terminated', '') if 'null-terminated' in length else length
                    length = None if length == 'null-terminated' else length

                cdecl = self.makeCParamDecl(member, 0)
                fullType = ' '.join(cdecl[:cdecl.rfind(name)].split())
                pointer = '*' in cdecl or type.startswith('PFN_')
                const = 'const' in cdecl
                # Some structs like VkTransformMatrixKHR have a 2D array
                fixedSizeArray = [x[:-1] for x in cdecl.split('[') if x.endswith(']')]

                if fixedSizeArray and not length:
                    length = ','.join(fixedSizeArray)

                # Handle C bit field members
                bitFieldWidth = int(cdecl.split(':')[1]) if ':' in cdecl else None

                # if a pointer, this can be a something like:
                #     optional="true,false" for ppGeometries
                #     optional="false,true" for pPhysicalDeviceCount
                # the first is if the variable itself is optional
                # the second is the value of the pointer is optional;
                optionalValues = splitIfGet(member, 'optional')
                optional = len(optionalValues) > 0 and optionalValues[0].lower() == "true"
                optionalPointer = len(optionalValues) > 1 and optionalValues[1].lower() == "true"

                members.append(Member(name, type, fullType, noautovalidity, limittype,
                                      const, length, nullTerminated, pointer, fixedSizeArray,
                                      optional, optionalPointer,
                                      externSync, cdecl, bitFieldWidth))

            self.vk.structs[typeName] = Struct(typeName, [], extension, self.currentVersion, protect, members,
                                               union, returnedOnly, sType, allowDuplicate, extends, extendedBy)

        elif category == 'handle':
            if alias is not None:
                self.handleAliasMap[typeName] = alias
                return
            type = typeElem.get('objtypeenum')

            # will resolve these later, the VulkanObjectType does not list things in dependent order
            parent = typeElem.get('parent')
            instance = typeName == 'VkInstance'
            device = typeName == 'VkDevice'

            dispatchable = typeElem.find('type').text == 'VK_DEFINE_HANDLE'

            self.vk.handles[typeName] = Handle(typeName, [], type, protect, parent, instance, device, dispatchable, extension)

        elif category == 'define':
            if typeName == 'VK_HEADER_VERSION':
                self.vk.headerVersion = typeElem.find('name').tail.strip()

        elif category == 'bitmask':
            if alias is not None:
                self.flagsAliasMap[typeName] = alias
                return

            # Bitmask types, i.e. flags
            baseFlagsType = typeElem.find('type').text
            bitWidth = 64 if baseFlagsType == 'VkFlags64' else 32

            # Bitmask enum type is either in the 'requires' or 'bitvalues' attribute
            # (for some reason there are two conventions)
            bitmaskName = typeElem.get('bitvalues')
            if bitmaskName is None:
                bitmaskName = typeElem.get('requires')

            self.vk.flags[typeName] = Flags(typeName, [], bitmaskName, protect, baseFlagsType, bitWidth, True, extension)

        else:
            # not all categories are used
            #   'group'/'enum' are routed to genGroup instead
            #   'basetype'/'include' are only for headers
            #   'funcpointer` ignore until needed
            return

    def genSpirv(self, spirvinfo, spirvName, alias):
        OutputGenerator.genSpirv(self, spirvinfo, spirvName, alias)
        spirvElem = spirvinfo.elem
        name = spirvElem.get('name')
        extension = True if spirvElem.tag == 'spirvextension' else False
        capability = not extension

        enables = []
        for elem in spirvElem:
            version = elem.attrib.get('version')
            extensionEnable = elem.attrib.get('extension')
            struct = elem.attrib.get('struct')
            feature = elem.attrib.get('feature')
            requires = elem.attrib.get('requires')
            propertyEnable = elem.attrib.get('property')
            member = elem.attrib.get('member')
            value = elem.attrib.get('value')
            enables.append(SpirvEnables(version, extensionEnable, struct, feature,
                                        requires, propertyEnable, member, value))

        self.vk.spirv.append(Spirv(name, extension, capability, enables))

    def genFormat(self, format, formatinfo, alias):
        OutputGenerator.genFormat(self, format, formatinfo, alias)
        formatElem = format.elem
        name = formatElem.get('name')

        components = []
        for component in formatElem.iterfind('component'):
            type = component.get('name')
            bits = component.get('bits')
            numericFormat = component.get('numericFormat')
            planeIndex = intIfGet(component, 'planeIndex')
            components.append(FormatComponent(type, bits, numericFormat, planeIndex))

        planes = []
        for plane in formatElem.iterfind('plane'):
            index = int(plane.get('index'))
            widthDivisor = int(plane.get('widthDivisor'))
            heightDivisor = int(plane.get('heightDivisor'))
            compatible = plane.get('compatible')
            planes.append(FormatPlane(index, widthDivisor, heightDivisor, compatible))

        className = formatElem.get('class')
        blockSize = int(formatElem.get('blockSize'))
        texelsPerBlock = int(formatElem.get('texelsPerBlock'))
        blockExtent = splitIfGet(formatElem, 'blockExtent')
        packed = intIfGet(formatElem, 'packed')
        chroma = formatElem.get('chroma')
        compressed = formatElem.get('compressed')
        spirvImageFormat = formatElem.find('spirvimageformat')
        if spirvImageFormat is not None:
            spirvImageFormat = spirvImageFormat.get('name')

        self.vk.formats[name] = Format(name, className, blockSize, texelsPerBlock,
                                       blockExtent, packed, chroma, compressed,
                                       components, planes, spirvImageFormat)

    def genSyncStage(self, sync):
        OutputGenerator.genSyncStage(self, sync)
        syncElem = sync.elem

        support = maxSyncSupport
        supportElem = syncElem.find('syncsupport')
        if supportElem is not None:
            queues = getQueues(supportElem)
            stageNames = splitIfGet(supportElem, 'stage')
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if len(stageNames) > 0 else None
            support = SyncSupport(queues, stages, False)

        equivalent = maxSyncEquivalent
        equivalentElem = syncElem.find('syncequivalent')
        if equivalentElem is not None:
            stageNames = splitIfGet(equivalentElem, 'stage')
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if len(stageNames) > 0 else None
            accessNames = splitIfGet(equivalentElem, 'access')
            accesses = [x for x in self.vk.bitmasks['VkAccessFlagBits2'].flags if x.name in accessNames] if len(accessNames) > 0 else None
            equivalent = SyncEquivalent(stages, accesses, False)

        flagName = syncElem.get('name')
        flag = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name == flagName]
        # This check is needed because not all API variants have VK_KHR_synchronization2
        if flag:
            self.vk.syncStage.append(SyncStage(flag[0], support, equivalent))

    def genSyncAccess(self, sync):
        OutputGenerator.genSyncAccess(self, sync)
        syncElem = sync.elem

        support = maxSyncSupport
        supportElem = syncElem.find('syncsupport')
        if supportElem is not None:
            queues = getQueues(supportElem)
            stageNames = splitIfGet(supportElem, 'stage')
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if len(stageNames) > 0 else None
            support = SyncSupport(queues, stages, False)

        equivalent = maxSyncEquivalent
        equivalentElem = syncElem.find('syncequivalent')
        if equivalentElem is not None:
            stageNames = splitIfGet(equivalentElem, 'stage')
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if len(stageNames) > 0 else None
            accessNames = splitIfGet(equivalentElem, 'access')
            accesses = [x for x in self.vk.bitmasks['VkAccessFlagBits2'].flags if x.name in accessNames] if len(accessNames) > 0 else None
            equivalent = SyncEquivalent(stages, accesses, False)

        flagName = syncElem.get('name')
        flag = [x for x in self.vk.bitmasks['VkAccessFlagBits2'].flags if x.name == flagName]
        # This check is needed because not all API variants have VK_KHR_synchronization2
        if flag:
            self.vk.syncAccess.append(SyncAccess(flag[0], support, equivalent))

    def genSyncPipeline(self, sync):
        OutputGenerator.genSyncPipeline(self, sync)
        syncElem = sync.elem
        name = syncElem.get('name')
        depends = splitIfGet(syncElem, 'depends')
        stages = []
        for stageElem in syncElem.findall('syncpipelinestage'):
            order = stageElem.get('order')
            before = stageElem.get('before')
            after = stageElem.get('after')
            value = stageElem.text
            stages.append(SyncPipelineStage(order, before, after, value))

        self.vk.syncPipeline.append(SyncPipeline(name, depends, stages))

#
# This object handles all the parsing from the video.xml (i.e. Video Std header definitions)
# It will fill in video standard definitions into the VulkanObject
class _VideoStdGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)
        self.vk.videoStd = VideoStd()

        # Track the current Video Std header we are processing
        self.currentVideoStdHeader = None

    def write(self, data):
        # We do not write anything here
        return

    def beginFile(self, genOpts):
        # We intentionally skip default BaseGenerator behavior
        OutputGenerator.beginFile(self, genOpts)

    def endFile(self):
        # Move parsed definitions to the Video Std definitions
        self.vk.videoStd.enums = self.vk.enums
        self.vk.videoStd.structs = self.vk.structs
        self.vk.videoStd.constants = self.vk.constants

        # We intentionally skip default BaseGenerator behavior
        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        # We intentionally skip default BaseGenerator behavior
        OutputGenerator.beginFeature(self, interface, emit)

        # Only "extension" is possible in the video.xml, identifying the Video Std header
        assert interface.tag == 'extension'
        name = interface.get('name')
        version: (str | None) = None
        depends: list[str] = []

        # Handle Video Std header version constant
        for enum in interface.findall('require/enum[@value]'):
            enumName = enum.get('name')
            if enumName.endswith('_SPEC_VERSION'):
                version = enum.get('value')

        # Handle dependencies on other Video Std headers
        for type in interface.findall('require/type[@name]'):
            typeName = type.get('name')
            if typeName.startswith('vk_video/'):
                depends.append(typeName[len('vk_video/'):-len('.h')])

        headerFile = f'vk_video/{name}.h'

        self.vk.videoStd.headers[name] = VideoStdHeader(name, version, headerFile, depends)

        self.currentVideoStdHeader = self.vk.videoStd.headers[name]

        # Handle constants here as that seems the most straightforward
        constantNames = []
        for enum in interface.findall('require/enum[@type]'):
            constantNames.append(enum.get('name'))
        self.addConstants(constantNames)
        for constantName in constantNames:
            self.vk.constants[constantName].videoStdHeader = self.currentVideoStdHeader.name

    def endFeature(self):
        self.currentVideoStdHeader = None

        # We intentionally skip default BaseGenerator behavior
        OutputGenerator.endFeature(self)

    def genCmd(self, cmdinfo, name, alias):
        # video.xml should not contain any commands
        assert False

    def genGroup(self, groupinfo, groupName, alias):
        BaseGenerator.genGroup(self, groupinfo, groupName, alias)

        # We are supposed to be inside a video std header
        assert self.currentVideoStdHeader is not None

        # Mark the enum with the Video Std header it comes from
        if groupinfo.elem.get('type') == 'enum':
            assert alias is None
            self.vk.enums[groupName].videoStdHeader = self.currentVideoStdHeader.name

    def genType(self, typeInfo, typeName, alias):
        BaseGenerator.genType(self, typeInfo, typeName, alias)

        # We are supposed to be inside a video std header
        assert self.currentVideoStdHeader is not None

        # Mark the struct with the Video Std header it comes from
        if typeInfo.elem.get('category') == 'struct':
            assert alias is None
            self.vk.structs[typeName].videoStdHeader = self.currentVideoStdHeader.name

    def genSpirv(self, spirvinfo, spirvName, alias):
        # video.xml should not contain any SPIR-V info
        assert False

    def genFormat(self, format, formatinfo, alias):
        # video.xml should not contain any format info
        assert False

    def genSyncStage(self, sync):
        # video.xml should not contain any sync stage info
        assert False

    def genSyncAccess(self, sync):
        # video.xml should not contain any sync access info
        assert False

    def genSyncPipeline(self, sync):
        # video.xml should not contain any sync pipeline info
        assert False
