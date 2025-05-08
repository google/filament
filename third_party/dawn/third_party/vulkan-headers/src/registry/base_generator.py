#!/usr/bin/env python3 -i
#
# Copyright 2023-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

import pickle
import os
import tempfile
from vulkan_object import (VulkanObject,
    Extension, Version, Handle, Param, Queues, CommandScope, Command,
    EnumField, Enum, Flag, Bitmask, Member, Struct,
    FormatComponent, FormatPlane, Format,
    SyncSupport, SyncEquivalent, SyncStage, SyncAccess, SyncPipelineStage, SyncPipeline,
    SpirvEnables, Spirv)

# These live in the Vulkan-Docs repo, but are pulled in via the
# Vulkan-Headers/registry folder
from generator import OutputGenerator, GeneratorOptions, write
from vkconventions import VulkanConventions

# An API style convention object
vulkanConventions = VulkanConventions()

# Helpers to keep things cleaner
def splitIfGet(elem, name):
    return elem.get(name).split(',') if elem.get(name) is not None and elem.get(name) != '' else None

def textIfFind(elem, name):
    return elem.find(name).text if elem.find(name) is not None else None

def intIfGet(elem, name):
    return None if elem.get(name) is None else int(elem.get(name), 0)

def boolGet(elem, name) -> bool:
    return elem.get(name) is not None and elem.get(name) == "true"

def getQueues(elem) -> Queues:
    queues = 0
    queues_list = splitIfGet(elem, 'queues')
    if queues_list is not None:
        queues |= Queues.TRANSFER if 'transfer' in queues_list else 0
        queues |= Queues.GRAPHICS if 'graphics' in queues_list else 0
        queues |= Queues.COMPUTE if 'compute' in queues_list else 0
        queues |= Queues.PROTECTED if 'protected' in queues_list else 0
        queues |= Queues.SPARSE_BINDING if 'sparse_binding' in queues_list else 0
        queues |= Queues.OPTICAL_FLOW if 'opticalflow' in queues_list else 0
        queues |= Queues.DECODE if 'decode' in queues_list else 0
        queues |= Queues.ENCODE if 'encode' in queues_list else 0
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


# This Generator Option is used across all generators.
# After years of use, it has shown that most the options are unified across each generator (file)
# as it is easier to modify things per-file that need the difference
class BaseGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 customFileName = None,
                 customDirectory = None,
                 customApiName = None):
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

        # Will map alias to promoted name
        #   ex. ['VK_FILTER_CUBIC_IMG' : 'VK_FILTER_CUBIC_EXT']
        # When generating any code, there is no reason so use the old name
        self.enumAliasMap = dict()
        self.enumFieldAliasMap = dict()
        self.bitmaskAliasMap = dict()
        self.flagAliasMap = dict()
        self.structAliasMap = dict()
        self.handleAliasMap = dict()

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
                    command.extensions.extend([extension] if extension not in command.extensions else [])
                    extension.commands.extend([command] if command not in extension.commands else [])

            # While genGroup() will call twice with aliased value, it does not provide all the information we need
            dict = self.featureDictionary[extension.name]['enumconstant']
            for required in dict:
                # group can be a Enum or Bitmask
                for group in dict[required]:
                    if group in self.vk.enums:
                        if group not in extension.enumFields:
                            extension.enumFields[group] = [] # Dict needs init
                        enum = self.vk.enums[group]
                        # Need to convert all alias so they match what is in EnumField
                        enumList = list(map(lambda x: x if x not in self.enumFieldAliasMap else self.enumFieldAliasMap[x], dict[required][group]))

                        for enumField in [x for x in enum.fields if x.name in enumList]:
                            # Make sure list is unique
                            enum.fieldExtensions.extend([extension] if extension not in enum.fieldExtensions else [])
                            enumField.extensions.extend([extension] if extension not in enumField.extensions else [])
                            extension.enumFields[group].extend([enumField] if enumField not in extension.enumFields[group] else [])
                    if group in self.vk.bitmasks:
                        if group not in extension.flags:
                            extension.flags[group] = [] # Dict needs init
                        bitmask = self.vk.bitmasks[group]
                        # Need to convert all alias so they match what is in Flags
                        flagList = list(map(lambda x: x if x not in self.flagAliasMap else self.flagAliasMap[x], dict[required][group]))

                        for flags in [x for x in bitmask.flags if x.name in flagList]:
                            # Make sure list is unique
                            bitmask.flagExtensions.extend([extension] if extension not in bitmask.flagExtensions else [])
                            flags.extensions.extend([extension] if extension not in flags.extensions else [])
                            extension.flags[group].extend([flags] if flags not in extension.flags[group] else [])

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
                        enumName = self.enumAliasMap[enumName] if isAlias else enumName
                        if enumName in self.vk.enums:
                            enum = self.vk.enums[enumName]
                            enum.extensions.extend([extension] if extension not in enum.extensions else [])
                            extension.enums.extend([enum] if enum not in extension.enums else [])
                            # Update fields with implicit base extension
                            if isAlias:
                                continue
                            enum.fieldExtensions.extend([extension] if extension not in enum.fieldExtensions else [])
                            for enumField in [x for x in enum.fields if (not x.extensions or (x.extensions and all(e in enum.extensions for e in x.extensions)))]:
                                enumField.extensions.extend([extension] if extension not in enumField.extensions else [])
                                if enumName not in extension.enumFields:
                                    extension.enumFields[enumName] = [] # Dict needs init
                                extension.enumFields[enumName].extend([enumField] if enumField not in extension.enumFields[enumName] else [])

            dict = self.featureDictionary[extension.name]['bitmask']
            for required in dict:
                for group in dict[required]:
                    for bitmaskName in dict[required][group]:
                        bitmaskName = bitmaskName.replace('Flags', 'FlagBits') # Works since Flags is not repeated in name
                        isAlias = bitmaskName in self.bitmaskAliasMap
                        bitmaskName = self.bitmaskAliasMap[bitmaskName] if isAlias else bitmaskName
                        if bitmaskName in self.vk.bitmasks:
                            bitmask = self.vk.bitmasks[bitmaskName]
                            bitmask.extensions.extend([extension] if extension not in bitmask.extensions else [])
                            extension.bitmasks.extend([bitmask] if bitmask not in extension.bitmasks else [])
                            # Update flags with implicit base extension
                            if isAlias:
                                continue
                            bitmask.flagExtensions.extend([extension] if extension not in bitmask.flagExtensions else [])
                            for flag in [x for x in bitmask.flags if (not x.extensions or (x.extensions and all(e in bitmask.extensions for e in x.extensions)))]:
                                flag.extensions.extend([extension] if extension not in flag.extensions else [])
                                if bitmaskName not in extension.flags:
                                    extension.flags[bitmaskName] = [] # Dict needs init
                                extension.flags[bitmaskName].extend([flag] if flag not in extension.flags[bitmaskName] else [])

        # Some structs (ex VkAttachmentSampleCountInfoAMD) can have multiple alias pointing to same extension
        for extension in self.vk.extensions.values():
            dict = self.featureDictionary[extension.name]['struct']
            for required in dict:
                for group in dict[required]:
                    for structName in dict[required][group]:
                        isAlias = structName in self.structAliasMap
                        structName = self.structAliasMap[structName] if isAlias else structName
                        # An EXT struct can alias a KHR struct,
                        # that in turns aliaes a core struct
                        # => Try to propagate aliasing, it can safely result in a no-op
                        isAlias = structName in self.structAliasMap
                        structName = self.structAliasMap[structName] if isAlias else structName
                        if structName in self.vk.structs:
                            struct = self.vk.structs[structName]
                            struct.extensions.extend([extension] if extension not in struct.extensions else [])

        # While we update struct alias inside other structs, the command itself might have the struct as a first level param.
        # We use this time to update params to have the promoted name
        # Example - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9322
        for command in self.vk.commands.values():
            for member in command.params:
                if member.type in self.structAliasMap:
                    member.type = self.structAliasMap[member.type]

        # Could build up a reverse lookup map, but since these are not too large of list, just do here
        # (Need to be done after we have found all the aliases)
        for key, value in self.structAliasMap.items():
            self.vk.structs[value].aliases.append(key)
        for key, value in self.enumAliasMap.items():
            self.vk.enums[value].aliases.append(key)
        for key, value in self.bitmaskAliasMap.items():
            self.vk.bitmasks[value].aliases.append(key)
        for key, value in self.handleAliasMap.items():
            self.vk.handles[value].aliases.append(key)

    def endFile(self):
        # This is the point were reg.py has ran, everything is collected
        # We do some post processing now
        self.applyExtensionDependency()

        # Use structs and commands to find which things are returnedOnly
        for struct in [x for x in self.vk.structs.values() if not x.returnedOnly]:
            for enum in [self.vk.enums[x.type] for x in struct.members if x.type in self.vk.enums]:
                enum.returnedOnly = False
            for bitmask in [self.vk.bitmasks[x.type] for x in struct.members if x.type in self.vk.bitmasks]:
                bitmask.returnedOnly = False
            for bitmask in [self.vk.bitmasks[x.type.replace('Flags', 'FlagBits')] for x in struct.members if x.type.replace('Flags', 'FlagBits') in self.vk.bitmasks]:
                bitmask.returnedOnly = False
        for command in self.vk.commands.values():
            for enum in [self.vk.enums[x.type] for x in command.params if x.type in self.vk.enums]:
                enum.returnedOnly = False
            for bitmask in [self.vk.bitmasks[x.type] for x in command.params if x.type in self.vk.bitmasks]:
                bitmask.returnedOnly = False
            for bitmask in [self.vk.bitmasks[x.type.replace('Flags', 'FlagBits')] for x in command.params if x.type.replace('Flags', 'FlagBits') in self.vk.bitmasks]:
                bitmask.returnedOnly = False

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

        maxSyncSupport.queues = Queues.ALL
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
            # Not sure if better way to get this info
            specVersion = self.featureDictionary[name]['enumconstant'][None][None][0]
            nameString = self.featureDictionary[name]['enumconstant'][None][None][1]

            self.currentExtension = Extension(name, nameString, specVersion, instance, device, depends, vendorTag,
                                            platform, protect, provisional, promotedto, deprecatedby,
                                            obsoletedby, specialuse)
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

    #
    # All <command> from XML
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

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
            optional = optionalValues is not None and optionalValues[0].lower() == "true"
            optionalPointer = optionalValues is not None and len(optionalValues) > 1 and optionalValues[1].lower() == "true"

            # externsync will be 'true' or expression
            # if expression, it should be same as 'true'
            externSync = boolGet(param, 'externsync')
            externSyncPointer = None if externSync else splitIfGet(param, 'externsync')
            if not externSync and externSyncPointer is not None:
                externSync = True

            params.append(Param(paramName, paramAlias, paramType, paramFullType, paramNoautovalidity,
                                paramConst, length, nullTerminated, pointer, fixedSizeArray,
                                optional, optionalPointer,
                                externSync, externSyncPointer, cdecl))

        attrib = cmdinfo.elem.attrib
        alias = attrib.get('alias')
        tasks = splitIfGet(attrib, 'tasks')

        queues = getQueues(attrib)
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

        protect = self.currentExtension.protect if self.currentExtension is not None else None

        # These coammds have no way from the XML to detect they would be an instance command
        specialInstanceCommand = ['vkCreateInstance', 'vkEnumerateInstanceExtensionProperties','vkEnumerateInstanceLayerProperties', 'vkEnumerateInstanceVersion']
        instance = len(params) > 0 and (params[0].type == 'VkInstance' or params[0].type == 'VkPhysicalDevice' or name in specialInstanceCommand)
        device = not instance

        implicitElem = cmdinfo.elem.find('implicitexternsyncparams')
        implicitExternSyncParams = [x.text for x in implicitElem.findall('param')] if implicitElem else []

        self.vk.commands[name] = Command(name, alias, protect, [], self.currentVersion,
                                         returnType, params, instance, device,
                                         tasks, queues, successcodes, errorcodes,
                                         primary, secondary, renderpass, videocoding,
                                         implicitExternSyncParams, cPrototype, cFunctionPointer)

    #
    # List the enum for the commands
    # TODO - Seems empty groups like `VkDeviceDeviceMemoryReportCreateInfoEXT` do not show up in here
    def genGroup(self, groupinfo, groupName, alias):
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

                if elem.get('alias') is not None:
                    self.enumFieldAliasMap[fieldName] = elem.get('alias')
                    continue

                negative = elem.get('dir') is not None
                protect = elem.get('protect')
                (valueInt, valueStr) = self.enumToValue(elem, True, bitwidth)

                # Some values have multiple extensions (ex VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
                # genGroup() lists them twice
                if next((x for x in fields if x.name == fieldName), None) is None:
                    fields.append(EnumField(fieldName, protect, negative, valueInt, valueStr, []))

            self.vk.enums[groupName] = Enum(groupName, [], groupProtect, bitwidth, True, fields, [], [])

        else: # "bitmask"
            if alias is not None:
                self.bitmaskAliasMap[groupName] = alias
                return

            for elem in enumElem.findall('enum'):
                flagName = elem.get('name')

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
                    fields.append(Flag(flagName, protect, valueInt, valueStr, flagMultiBit, flagZero, []))

            flagName = groupName.replace('FlagBits', 'Flags')
            self.vk.bitmasks[groupName] = Bitmask(groupName, [], flagName, groupProtect, bitwidth, True, fields, [], [])

    def genType(self, typeInfo, typeName, alias):
        OutputGenerator.genType(self, typeInfo, typeName, alias)
        typeElem = typeInfo.elem
        protect = self.currentExtension.protect if hasattr(self.currentExtension, 'protect') and self.currentExtension.protect is not None else None
        category = typeElem.get('category')
        if (category == 'struct' or category == 'union'):
            extension = [self.currentExtension] if self.currentExtension is not None else []
            if alias is not None:
                self.structAliasMap[typeName] = alias
                return

            union = category == 'union'

            returnedOnly = boolGet(typeElem, 'returnedonly')
            allowDuplicate = boolGet(typeElem, 'allowduplicate')

            extends = splitIfGet(typeElem, 'structextends')
            extendedBy = self.registry.validextensionstructs[typeName] if len(self.registry.validextensionstructs[typeName]) > 0 else None

            membersElem = typeInfo.elem.findall('.//member')
            members = []
            sType = None

            for member in membersElem:
                for comment in member.findall('comment'):
                    member.remove(comment)

                name = textIfFind(member, 'name')
                type = textIfFind(member, 'type')
                sType = member.get('values') if member.get('values') is not None else sType
                externSync = boolGet(member, 'externsync')
                noautovalidity = boolGet(member, 'noautovalidity')
                limittype = member.get('limittype')

                nullTerminated = False
                length = member.get('altlen') if member.get('altlen') is not None else member.get('len')
                if length:
                    # we will either find it like "null-terminated" or "enabledExtensionCount,null-terminated"
                    # This finds both
                    nullTerminated = 'null-terminated' in length
                    length = length.replace(',null-terminated', '') if 'null-terminated' in length else length
                    length = None if length == 'null-terminated' else length

                cdecl = self.makeCParamDecl(member, 0)
                fullType = ' '.join(cdecl.split()[:-1])
                pointer = '*' in cdecl or type.startswith('PFN_')
                const = 'const' in cdecl
                # Some structs like VkTransformMatrixKHR have a 2D array
                fixedSizeArray = [x[:-1] for x in cdecl.split('[') if x.endswith(']')]

                if fixedSizeArray and not length:
                    length = ','.join(fixedSizeArray)

                # if a pointer, this can be a something like:
                #     optional="true,false" for ppGeometries
                #     optional="false,true" for pPhysicalDeviceCount
                # the first is if the variable itself is optional
                # the second is the value of the pointer is optional;
                optionalValues = splitIfGet(member, 'optional')
                optional = optionalValues is not None and optionalValues[0].lower() == "true"
                optionalPointer = optionalValues is not None and len(optionalValues) > 1 and optionalValues[1].lower() == "true"

                members.append(Member(name, type, fullType, noautovalidity, limittype,
                                      const, length, nullTerminated, pointer, fixedSizeArray,
                                      optional, optionalPointer,
                                      externSync, cdecl))

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

            self.vk.handles[typeName] = Handle(typeName, [], type, protect, parent, instance, device, dispatchable)

        elif category == 'define':
            if typeName == 'VK_HEADER_VERSION':
                self.vk.headerVersion = typeElem.find('name').tail.strip()

        else:
            # not all categories are used
            #   'group'/'enum'/'bitmask' are routed to genGroup instead
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
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if stageNames is not None else None
            support = SyncSupport(queues, stages, False)

        equivalent = maxSyncEquivalent
        equivalentElem = syncElem.find('syncequivalent')
        if equivalentElem is not None:
            stageNames = splitIfGet(equivalentElem, 'stage')
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if stageNames is not None else None
            accessNames = splitIfGet(equivalentElem, 'access')
            accesses = [x for x in self.vk.bitmasks['VkAccessFlagBits2'].flags if x.name in accessNames] if accessNames is not None else None
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
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if stageNames is not None else None
            support = SyncSupport(queues, stages, False)

        equivalent = maxSyncEquivalent
        equivalentElem = syncElem.find('syncequivalent')
        if equivalentElem is not None:
            stageNames = splitIfGet(equivalentElem, 'stage')
            stages = [x for x in self.vk.bitmasks['VkPipelineStageFlagBits2'].flags if x.name in stageNames] if stageNames is not None else None
            accessNames = splitIfGet(equivalentElem, 'access')
            accesses = [x for x in self.vk.bitmasks['VkAccessFlagBits2'].flags if x.name in accessNames] if accessNames is not None else None
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
