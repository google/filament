#!/usr/bin/env python3 -i
#
# Copyright 2013-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

# Working-group-specific style conventions,
# used in generation.

import re
import os

from spec_tools.conventions import ConventionsBase

# Modified from default implementation - see category_requires_validation() below
CATEGORIES_REQUIRING_VALIDATION = set(('handle', 'enum', 'bitmask'))

# Tokenize into "words" for structure types, approximately per spec "Implicit Valid Usage" section 2.7.2
# This first set is for things we recognize explicitly as words,
# as exceptions to the general regex.
# Ideally these would be listed in the spec as exceptions, as OpenXR does.
SPECIAL_WORDS = set((
    '16Bit',  # VkPhysicalDevice16BitStorageFeatures
    '2D',     # VkPhysicalDeviceImage2DViewOf3DFeaturesEXT
    '3D',     # VkPhysicalDeviceImage2DViewOf3DFeaturesEXT
    '8Bit',  # VkPhysicalDevice8BitStorageFeaturesKHR
    'AABB',  # VkGeometryAABBNV
    'ASTC',  # VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT
    'D3D12',  # VkD3D12FenceSubmitInfoKHR
    'Float16',  # VkPhysicalDeviceShaderFloat16Int8FeaturesKHR
    'Bfloat16',  # VkPhysicalDeviceShaderBfloat16FeaturesKHR
    'Float8',  # VkPhysicalDeviceShaderFloat8FeaturesEXT
    'ImagePipe',  # VkImagePipeSurfaceCreateInfoFUCHSIA
    'Int64',  # VkPhysicalDeviceShaderAtomicInt64FeaturesKHR
    'Int8',  # VkPhysicalDeviceShaderFloat16Int8FeaturesKHR
    'MacOS',  # VkMacOSSurfaceCreateInfoMVK
    'RGBA10X6', # VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT
    'Uint8',  # VkPhysicalDeviceIndexTypeUint8FeaturesEXT
    'Win32',  # VkWin32SurfaceCreateInfoKHR
))
# A regex to match any of the SPECIAL_WORDS
EXCEPTION_PATTERN = r'(?P<exception>{})'.format(
    '|'.join(f'({re.escape(w)})' for w in SPECIAL_WORDS))
MAIN_RE = re.compile(
    # the negative lookahead is to prevent the all-caps pattern from being too greedy.
    r'({}|([0-9]+)|([A-Z][a-z]+)|([A-Z][A-Z]*(?![a-z])))'.format(EXCEPTION_PATTERN))


class VulkanConventions(ConventionsBase):
    @property
    def null(self):
        """Preferred spelling of NULL."""
        return '`NULL`'

    def formatVersion(self, name, apivariant, major, minor):
        """Mark up an API version name as a link in the spec."""
        version = f'{major}.{minor}'
        if apivariant == 'VKSC':
            # Vulkan SC has a different anchor pattern for version appendices
            if version == '1.0':
                return 'Vulkan SC 1.0'
            else:
                return f'<<versions-sc-{version}, Vulkan SC Version {version}>>'
        else:
            return f'<<versions-{version}, Vulkan Version {version}>>'

    def formatExtension(self, name):
        """Mark up an extension name as a link in the spec."""
        return f'apiext:{name}'

    @property
    def struct_macro(self):
        """Get the appropriate format macro for a structure.

        Primarily affects generated valid usage statements.
        """

        return 'slink:'

    @property
    def constFlagBits(self):
        """Returns True if static const flag bits should be generated, False if an enumerated type should be generated."""
        return False

    @property
    def structtype_member_name(self):
        """Return name of the structure type member"""
        return 'sType'

    @property
    def nextpointer_member_name(self):
        """Return name of the structure pointer chain member"""
        return 'pNext'

    @property
    def valid_pointer_prefix(self):
        """Return prefix to pointers which must themselves be valid"""
        return 'valid'

    def is_structure_type_member(self, paramtype, paramname):
        """Determine if member type and name match the structure type member."""
        return paramtype == 'VkStructureType' and paramname == self.structtype_member_name

    def is_nextpointer_member(self, paramtype, paramname):
        """Determine if member type and name match the next pointer chain member."""
        return paramtype == 'void' and paramname == self.nextpointer_member_name

    def generate_structure_type_from_name(self, structname):
        """Generate a structure type name, like VK_STRUCTURE_TYPE_CREATE_INSTANCE_INFO"""

        structure_type_parts = []
        # Tokenize into "words"
        for elem in MAIN_RE.findall(structname):
            word = elem[0]
            if word == 'Vk':
                structure_type_parts.append('VK_STRUCTURE_TYPE')
            else:
                structure_type_parts.append(word.upper())
        name = '_'.join(structure_type_parts)

        # The simple-minded rules need modification for some structure names
        subpats = [
            [ r'_H_(26[45])_',              r'_H\1_' ],
            [ r'_VP_9_',                    r'_VP9_' ],
            [ r'_AV_1_',                    r'_AV1_' ],
            [ r'_VULKAN_([0-9])([0-9])_',   r'_VULKAN_\1_\2_' ],
            [ r'_VULKAN_SC_([0-9])([0-9])_',r'_VULKAN_SC_\1_\2_' ],
            [ r'_DIRECT_FB_',               r'_DIRECTFB_' ],
            [ r'_VULKAN_SC_10',             r'_VULKAN_SC_1_0' ],

        ]

        for subpat in subpats:
            name = re.sub(subpat[0], subpat[1], name)
        return name

    @property
    def warning_comment(self):
        """Return warning comment to be placed in header of generated Asciidoctor files"""
        return '// WARNING: DO NOT MODIFY! This file is automatically generated from the vk.xml registry'

    @property
    def file_suffix(self):
        """Return suffix of generated Asciidoctor files"""
        return '.adoc'

    def api_name(self, spectype='api'):
        """Return API or specification name for citations in ref pages.ref
           pages should link to for

           spectype is the spec this refpage is for: 'api' is the Vulkan API
           Specification. Defaults to 'api'. If an unrecognized spectype is
           given, returns None.
        """
        if spectype == 'api' or spectype is None:
            return 'Vulkan'
        else:
            return None

    @property
    def api_prefix(self):
        """Return API token prefix"""
        return 'VK_'

    @property
    def write_contacts(self):
        """Return whether contact list should be written to extension appendices"""
        return True

    @property
    def write_refpage_include(self):
        """Return whether refpage include should be written to extension appendices"""
        return True

    @property
    def member_used_for_unique_vuid(self):
        """Return the member name used in the VUID-...-...-unique ID."""
        return self.structtype_member_name

    def is_externsync_command(self, protoname):
        """Returns True if the protoname element is an API command requiring
           external synchronization
        """
        return protoname is not None and 'vkCmd' in protoname

    def is_api_name(self, name):
        """Returns True if name is in the reserved API namespace.
        For Vulkan, these are names with a case-insensitive 'vk' prefix, or
        a 'PFN_vk' function pointer type prefix.
        """
        return name[0:2].lower() == 'vk' or name.startswith('PFN_vk')

    def specURL(self, spectype='api'):
        """Return public registry URL which ref pages should link to for the
           current all-extensions HTML specification, so xrefs in the
           asciidoc source that are not to ref pages can link into it
           instead. N.b. this may need to change on a per-refpage basis if
           there are multiple documents involved.
        """
        return 'https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html'

    @property
    def xml_api_name(self):
        """Return the name used in the default API XML registry for the default API"""
        return 'vulkan'

    @property
    def registry_path(self):
        """Return relpath to the default API XML registry in this project."""
        return 'xml/vk.xml'

    @property
    def specification_path(self):
        """Return relpath to the Asciidoctor specification sources in this project."""
        return '{generated}/meta'

    @property
    def special_use_section_anchor(self):
        """Return asciidoctor anchor name in the API Specification of the
        section describing extension special uses in detail."""
        return 'extendingvulkan-compatibility-specialuse'

    @property
    def extension_index_prefixes(self):
        """Return a list of extension prefixes used to group extension refpages."""
        return ['VK_KHR', 'VK_EXT', 'VK']

    @property
    def unified_flag_refpages(self):
        """Return True if Flags/FlagBits refpages are unified, False if
           they are separate.
        """
        return False

    @property
    def spec_reflow_path(self):
        """Return the path to the spec source folder to reflow"""
        return os.getcwd()

    @property
    def spec_no_reflow_dirs(self):
        """Return a set of directories not to automatically descend into
           when reflowing spec text
        """
        return ('scripts', 'style')

    @property
    def zero(self):
        return '`0`'

    def category_requires_validation(self, category):
        """Return True if the given type 'category' always requires validation.

        Overridden because Vulkan does not require "valid" text for basetype
        in the spec right now."""
        return category in CATEGORIES_REQUIRING_VALIDATION

    @property
    def should_skip_checking_codes(self):
        """Return True if more than the basic validation of return codes should
        be skipped for a command.

        Vulkan mostly relies on the validation layers rather than API
        builtin error checking, so these checks are not appropriate.

        For example, passing in a VkFormat parameter will not potentially
        generate a VK_ERROR_FORMAT_NOT_SUPPORTED code."""

        return True

    def extension_file_path(self, name):
        """Return file path to an extension appendix relative to a directory
           containing all such appendices.
           - name - extension name"""

        return f'{name}{self.file_suffix}'

    def valid_flag_bit(self, bitpos):
        """Return True if bitpos is an allowed numeric bit position for
           an API flag bit.

           Vulkan uses 32 bit Vk*Flags types, and assumes C compilers may
           cause Vk*FlagBits values with bit 31 set to result in a 64 bit
           enumerated type, so disallows such flags."""
        return bitpos >= 0 and bitpos < 31

    @property
    def extra_refpage_headers(self):
        """Return any extra text to add to refpage headers."""
        return 'include::{config}/attribs.adoc[]'

    @property
    def extra_refpage_body(self):
        """Return any extra text (following the title) for generated
           reference pages."""
        return 'include::{generated}/specattribs.adoc[]'


class VulkanSCConventions(VulkanConventions):

    def specURL(self, spectype='api'):
        """Return public registry URL which ref pages should link to for the
           current all-extensions HTML specification, so xrefs in the
           asciidoc source that are not to ref pages can link into it
           instead. N.b. this may need to change on a per-refpage basis if
           there are multiple documents involved.
        """
        return 'https://registry.khronos.org/vulkansc/specs/1.0-extensions/html/vkspec.html'

    @property
    def xml_api_name(self):
        """Return the name used in the default API XML registry for the default API"""
        return 'vulkansc'

