#!/usr/bin/env python3 -i
#
# Copyright 2023-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

from dataclasses import dataclass, field
from enum import IntFlag, Enum, auto

@dataclass
class Extension:
    """<extension>"""
    name: str # ex) VK_KHR_SURFACE
    nameString: str # marco with string, ex) VK_KHR_SURFACE_EXTENSION_NAME
    specVersion: str # marco with string, ex) VK_KHR_SURFACE_SPEC_VERSION

    # Only one will be True, the other is False
    instance: bool
    device: bool

    depends: (str | None)
    vendorTag: (str | None)  # ex) EXT, KHR, etc
    platform: (str | None)   # ex) android
    protect: (str | None)    # ex) VK_USE_PLATFORM_ANDROID_KHR
    provisional: bool
    promotedTo: (str | None) # ex) VK_VERSION_1_1
    deprecatedBy: (str | None)
    obsoletedBy: (str | None)
    specialUse: list[str]

    # These are here to allow for easy reverse lookups
    # Quotes allow us to forward declare the dataclass
    commands: list['Command'] = field(default_factory=list, init=False)
    enums:    list['Enum']    = field(default_factory=list, init=False)
    bitmasks: list['Bitmask'] = field(default_factory=list, init=False)
    # Use the Enum name to see what fields are extended
    enumFields: dict[str, list['EnumField']] = field(default_factory=dict, init=False)
    # Use the Bitmaks name to see what flags are extended
    flags: dict[str, list['Flag']] = field(default_factory=dict, init=False)

@dataclass
class Version:
    """
    <feature> which represents a version
    This will NEVER be Version 1.0, since having 'no version' is same as being 1.0
    """
    name: str       # ex) VK_VERSION_1_1
    nameString: str # ex) "VK_VERSION_1_1" (no marco, so has quotes)
    nameApi: str    # ex) VK_API_VERSION_1_1

@dataclass
class Handle:
    """<type> which represents a dispatch handle"""
    name: str # ex) VkBuffer
    aliases: list[str] # ex) ['VkSamplerYcbcrConversionKHR']

    type: str # ex) VK_OBJECT_TYPE_BUFFER
    protect: (str | None) # ex) VK_USE_PLATFORM_ANDROID_KHR

    parent: 'Handle' # Chain of parent handles, can be None

    # Only one will be True, the other is False
    instance: bool
    device: bool

    dispatchable: bool

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class Param:
    """<command/param>"""
    name: str # ex) pCreateInfo, pAllocator, pBuffer
    alias: str

    # the "base type" - will not preserve the 'const' or pointer info
    # ex) void, uint32_t, VkFormat, VkBuffer, etc
    type: str
    # the "full type" - will be cDeclaration without the type name
    # ex) const void*, uint32_t, const VkFormat, VkBuffer*, etc
    # For arrays, this will only display the type, fixedSizeArray can be used to get the length
    fullType: str

    noAutoValidity: bool

    const: bool           # type contains 'const'
    length:  (str | None) # the known length of pointer, will never be 'null-terminated'
    nullTerminated: bool  # If a UTF-8 string, it will be null-terminated
    pointer: bool         # type contains a pointer (include 'PFN' function pointers)
    # Used to list how large an array of the type is
    # ex) lineWidthRange is ['2']
    # ex) memoryTypes is ['VK_MAX_MEMORY_TYPES']
    # ex) VkTransformMatrixKHR:matrix is ['3', '4']
    fixedSizeArray: list[str]

    optional: bool
    optionalPointer: bool # if type contains a pointer, is the pointer value optional

    externSync: bool
    externSyncPointer: list[str] # if type contains a pointer, might only specific members modified

    # C string of member, example:
    #   - const void* pNext
    #   - VkFormat format
    #   - VkStructureType sType
    cDeclaration: str

    def __lt__(self, other):
        return self.name < other.name

class Queues(IntFlag):
    TRANSFER       = auto() # VK_QUEUE_TRANSFER_BIT
    GRAPHICS       = auto() # VK_QUEUE_GRAPHICS_BIT
    COMPUTE        = auto() # VK_QUEUE_COMPUTE_BIT
    PROTECTED      = auto() # VK_QUEUE_PROTECTED_BIT
    SPARSE_BINDING = auto() # VK_QUEUE_SPARSE_BINDING_BIT
    OPTICAL_FLOW   = auto() # VK_QUEUE_OPTICAL_FLOW_BIT_NV
    DECODE         = auto() # VK_QUEUE_VIDEO_DECODE_BIT_KHR
    ENCODE         = auto() # VK_QUEUE_VIDEO_ENCODE_BIT_KHR
    ALL = TRANSFER | GRAPHICS | COMPUTE | PROTECTED | SPARSE_BINDING | OPTICAL_FLOW | DECODE | ENCODE

class CommandScope(Enum):
    NONE    = auto()
    INSIDE  = auto()
    OUTSIDE = auto()
    BOTH    = auto()

@dataclass
class Command:
    """<command>"""
    name: str # ex) vkCmdDraw
    alias: (str | None) # Because commands are interfaces into layers/drivers, we need all command alias
    protect: (str | None) # ex) 'VK_ENABLE_BETA_EXTENSIONS'

    extensions: list[Extension] # All extensions that enable the struct
    version: (Version | None) # None if Version 1.0

    returnType: str # ex) void, VkResult, etc

    params: list[Param] # Each parameter of the command

    # Only one will be True, the other is False
    instance: bool
    device: bool

    tasks: list[str]        # ex) [ action, state, synchronization ]
    queues: Queues          # zero == No Queues found
    successCodes: list[str] # ex) [ VK_SUCCESS, VK_INCOMPLETE ]
    errorCodes: list[str]   # ex) [ VK_ERROR_OUT_OF_HOST_MEMORY ]

    # Shows support if command can be in a primary and/or secondary command buffer
    primary: bool
    secondary: bool

    renderPass: CommandScope
    videoCoding: CommandScope

    implicitExternSyncParams: list[str]

    # C prototype string - ex:
    # VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
    #   const VkInstanceCreateInfo* pCreateInfo,
    #   const VkAllocationCallbacks* pAllocator,
    #   VkInstance* pInstance);
    cPrototype: str

    # function pointer typedef  - ex:
    # typedef VkResult (VKAPI_PTR *PFN_vkCreateInstance)
    #   (const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
    cFunctionPointer: str

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class Member:
    """<member>"""
    name: str # ex) sType, pNext, flags, size, usage

    # the "base type" - will not preserve the 'const' or pointer info
    # ex) void, uint32_t, VkFormat, VkBuffer, etc
    type: str
    # the "full type" - will be cDeclaration without the type name
    # ex) const void*, uint32_t, const VkFormat, VkBuffer*, etc
    # For arrays, this will only display the type, fixedSizeArray can be used to get the length
    fullType: str

    noAutoValidity: bool
    limitType: (str | None) # ex) 'max', 'bitmask', 'bits', 'min,mul'

    const: bool           # type contains 'const'
    length:  (str | None) # the known length of pointer, will never be 'null-terminated'
    nullTerminated: bool  # If a UTF-8 string, it will be null-terminated
    pointer: bool         # type contains a pointer (include 'PFN' function pointers)
    # Used to list how large an array of the type is
    # ex) lineWidthRange is ['2']
    # ex) memoryTypes is ['VK_MAX_MEMORY_TYPES']
    # ex) VkTransformMatrixKHR:matrix is ['3', '4']
    fixedSizeArray: list[str]

    optional: bool
    optionalPointer: bool # if type contains a pointer, is the pointer value optional

    externSync: bool

    # C string of member, example:
    #   - const void* pNext
    #   - VkFormat format
    #   - VkStructureType sType
    cDeclaration: str

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class Struct:
    """<type category="struct"> or <type category="union">"""
    name: str # ex) VkImageSubresource2
    aliases: list[str] # ex) ['VkImageSubresource2KHR', 'VkImageSubresource2EXT']

    extensions: list[Extension] # All extensions that enable the struct
    version: (Version | None) # None if Version 1.0
    protect: (str | None) # ex) VK_ENABLE_BETA_EXTENSIONS

    members: list[Member]

    union: bool # Unions are just a subset of a Structs
    returnedOnly: bool

    sType: (str | None) # ex) VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
    allowDuplicate: bool # can have a pNext point to itself

    # These use to be list['Struct'] but some circular loops occur and cause
    # pydevd warnings and made debugging slow (30 seconds to index a Struct)
    extends: list[str] # Struct names that this struct extends
    extendedBy: list[str] # Struct names that can be extended by this struct

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class EnumField:
    """<enum> of type enum"""
    name: str # ex) VK_DYNAMIC_STATE_SCISSOR
    protect: (str | None) # ex) VK_ENABLE_BETA_EXTENSIONS

    negative: bool # True if negative values are allowed (ex. VkResult)
    value: int
    valueStr: str # value as shown in spec (ex. "0", "2", "1000267000", "0x00000004")

    # some fields are enabled from 2 extensions (ex) VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
    extensions: list[Extension] # None if part of 1.0 core

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class Enum:
    """<enums> of type enum"""
    name: str # ex) VkLineRasterizationMode
    aliases: list[str] # ex) ['VkLineRasterizationModeKHR', 'VkLineRasterizationModeEXT']

    protect: (str | None) # ex) VK_ENABLE_BETA_EXTENSIONS

    bitWidth: int # 32 or 64 (currently all are 32, but field is to match with Bitmask)
    returnedOnly: bool

    fields: list[EnumField]

    extensions: list[Extension] # None if part of 1.0 core
    # Unique list of all extension that are involved in 'fields' (superset of 'extensions')
    fieldExtensions: list[Extension]

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class Flag:
    """<enum> of type bitmask"""
    name: str # ex) VK_ACCESS_2_SHADER_READ_BIT
    protect: (str | None) # ex) VK_ENABLE_BETA_EXTENSIONS

    value: int
    valueStr: str # value as shown in spec (ex. 0x00000000", "0x00000004", "0x0000000F", "0x800000000ULL")
    multiBit: bool # if true, more than one bit is set (ex) VK_SHADER_STAGE_ALL_GRAPHICS)
    zero: bool     # if true, the value is zero (ex) VK_PIPELINE_STAGE_NONE)

    # some fields are enabled from 2 extensions (ex) VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT)
    extensions: list[Extension] # None if part of 1.0 core

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class Bitmask:
    """<enums> of type bitmask"""
    name: str     # ex) VkAccessFlagBits2
    aliases: list[str] # ex) ['VkAccessFlagBits2KHR']

    flagName: str # ex) VkAccessFlags2
    protect: (str | None) # ex) VK_ENABLE_BETA_EXTENSIONS

    bitWidth: int # 32 or 64
    returnedOnly: bool

    flags: list[Flag]

    extensions: list[Extension] # None if part of 1.0 core
    # Unique list of all extension that are involved in 'flag' (superset of 'extensions')
    flagExtensions: list[Extension]

    def __lt__(self, other):
        return self.name < other.name

@dataclass
class FormatComponent:
    """<format/component>"""
    type: str # ex) R, G, B, A, D, S, etc
    bits: str # will be an INT or 'compressed'
    numericFormat: str # ex) UNORM, SINT, etc
    planeIndex: (int | None) # None if no planeIndex in format

@dataclass
class FormatPlane:
    """<format/plane>"""
    index: int
    widthDivisor: int
    heightDivisor: int
    compatible: str

@dataclass
class Format:
    """<format>"""
    name: str
    className: str
    blockSize: int
    texelsPerBlock: int
    blockExtent: list[str]
    packed: (int | None) # None == not-packed
    chroma: (str | None)
    compressed: (str | None)
    components: list[FormatComponent] # <format/component>
    planes: list[FormatPlane]  # <format/plane>
    spirvImageFormat: (str | None)

@dataclass
class SyncSupport:
    """<syncsupport>"""
    queues: Queues
    stages: list[Flag] # VkPipelineStageFlagBits2
    max: bool # If this supports max values

@dataclass
class SyncEquivalent:
    """<syncequivalent>"""
    stages: list[Flag] # VkPipelineStageFlagBits2
    accesses: list[Flag] # VkAccessFlagBits2
    max: bool # If this equivalent to everything

@dataclass
class SyncStage:
    """<syncstage>"""
    flag: Flag # VkPipelineStageFlagBits2
    support: SyncSupport
    equivalent: SyncEquivalent

@dataclass
class SyncAccess:
    """<syncaccess>"""
    flag: Flag # VkAccessFlagBits2
    support: SyncSupport
    equivalent: SyncEquivalent

@dataclass
class SyncPipelineStage:
    """<syncpipelinestage>"""
    order: (str | None)
    before: (str | None)
    after: (str | None)
    value: str

@dataclass
class SyncPipeline:
    """<syncpipeline>"""
    name: str
    depends: list[str]
    stages: list[SyncPipelineStage]

@dataclass
class SpirvEnables:
    """What is needed to enable the SPIR-V element"""
    version: (str | None)
    extension: (str | None)
    struct: (str | None)
    feature: (str | None)
    requires: (str | None)
    property: (str | None)
    member: (str | None)
    value: (str | None)

@dataclass
class Spirv:
    """<spirvextension> and <spirvcapability>"""
    name: str
    # Only one will be True, the other is False
    extension: bool
    capability: bool
    enable: list[SpirvEnables]

# This is the global Vulkan Object that holds all the information from parsing the XML
# This class is designed so all generator scripts can use this to obtain data
@dataclass
class VulkanObject():
    headerVersion: int = 0 # value of VK_HEADER_VERSION

    extensions: dict[str, Extension] = field(default_factory=dict, init=False)
    versions:   dict[str, Version]   = field(default_factory=dict, init=False)

    handles:  dict[str, Handle]      = field(default_factory=dict, init=False)
    commands: dict[str, Command]     = field(default_factory=dict, init=False)
    structs:  dict[str, Struct]      = field(default_factory=dict, init=False)
    enums:    dict[str, Enum]        = field(default_factory=dict, init=False)
    bitmasks: dict[str, Bitmask]     = field(default_factory=dict, init=False)
    formats:  dict[str, Format]      = field(default_factory=dict, init=False)

    syncStage:    list[SyncStage]    = field(default_factory=list, init=False)
    syncAccess:   list[SyncAccess]   = field(default_factory=list, init=False)
    syncPipeline: list[SyncPipeline] = field(default_factory=list, init=False)

    spirv: list[Spirv]               = field(default_factory=list, init=False)

    # ex) [ xlib : VK_USE_PLATFORM_XLIB_KHR ]
    platforms: dict[str, str]        = field(default_factory=dict, init=False)
    # list of all vendor Suffix names (KHR, EXT, etc. )
    vendorTags: list[str]            = field(default_factory=list, init=False)
    # ex) [ Queues.COMPUTE : VK_QUEUE_COMPUTE_BIT ]
    queueBits: dict[IntFlag, str]    = field(default_factory=dict, init=False)
