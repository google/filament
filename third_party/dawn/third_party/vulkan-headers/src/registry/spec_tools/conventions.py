#!/usr/bin/env python3 -i
#
# Copyright 2013-2025 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

# Base class for working-group-specific style conventions,
# used in generation.

from enum import Enum
import abc
import re

# Type categories that respond "False" to isStructAlwaysValid
# basetype is home to typedefs like ..Bool32
CATEGORIES_REQUIRING_VALIDATION = set(('handle',
                                       'enum',
                                       'bitmask',
                                       'basetype',
                                       None))

# These are basic C types pulled in via openxr_platform_defines.h
TYPES_KNOWN_ALWAYS_VALID = set(('char',
                                'float',
                                'int8_t', 'uint8_t',
                                'int16_t', 'uint16_t',
                                'int32_t', 'uint32_t',
                                'int64_t', 'uint64_t',
                                'size_t',
                                'intptr_t', 'uintptr_t',
                                'int',
                                ))

# Split an extension name into vendor ID and name portions
EXT_NAME_DECOMPOSE_RE = re.compile(r'(?P<prefix>[A-Za-z]+)_(?P<vendor>[A-Za-z]+)_(?P<name>[\w_]+)')

# Match an API version name.
# Match object includes API prefix, major, and minor version numbers.
# This could be refined further for specific APIs.
API_VERSION_NAME_RE = re.compile(r'(?P<apivariant>[A-Za-z]+)_VERSION_(?P<major>[0-9]+)_(?P<minor>[0-9]+)')

class ProseListFormats(Enum):
    """A connective, possibly with a quantifier."""
    AND = 0
    EACH_AND = 1
    OR = 2
    ANY_OR = 3

    @classmethod
    def from_string(cls, s):
        if s == 'or':
            return cls.OR
        if s == 'and':
            return cls.AND
        raise RuntimeError(f"Unrecognized string connective: {s}")

    @property
    def connective(self):
        if self in (ProseListFormats.OR, ProseListFormats.ANY_OR):
            return 'or'
        return 'and'

    def quantifier(self, n):
        """Return the desired quantifier for a list of a given length."""
        if self == ProseListFormats.ANY_OR:
            if n > 1:
                return 'any of '
        elif self == ProseListFormats.EACH_AND:
            if n > 2:
                return 'each of '
            if n == 2:
                return 'both of '
        return ''


class ConventionsBase(abc.ABC):
    """WG-specific conventions."""

    def __init__(self):
        self._command_prefix = None
        self._type_prefix = None

    def formatVersionOrExtension(self, name):
        """Mark up an API version or extension name as a link in the spec."""

        # Is this a version name?
        match = API_VERSION_NAME_RE.match(name)
        if match is not None:
            return self.formatVersion(name,
                match.group('apivariant'),
                match.group('major'),
                match.group('minor'))
        else:
            # If not, assumed to be an extension name. Might be worth checking.
            return self.formatExtension(name)

    def formatVersion(self, name, apivariant, major, minor):
        """Mark up an API version name as a link in the spec."""
        return f'`<<{name}>>`'

    def formatExtension(self, name):
        """Mark up an extension name as a link in the spec."""
        return f'`<<{name}>>`'

    def formatSPIRVlink(self, name):
        """Mark up a SPIR-V extension name as an external link in the spec.
           Since these are external links, the formatting probably will be
           the same for all APIs creating such links, so long as they use
           the asciidoctor {spirv} attribute for the base path to the SPIR-V
           extensions."""

        (vendor, _) = self.extension_name_split(name)

        return f'{{spirv}}/{vendor}/{name}.html[{name}]'

    @property
    @abc.abstractmethod
    def null(self):
        """Preferred spelling of NULL."""
        raise NotImplementedError

    def makeProseList(self, elements, fmt=ProseListFormats.AND, with_verb=False, *args, **kwargs):
        """Make a (comma-separated) list for use in prose.

        Adds a connective (by default, 'and')
        before the last element if there are more than 1.

        Adds the right one of "is" or "are" to the end if with_verb is true.

        Optionally adds a quantifier (like 'any') before a list of 2 or more,
        if specified by fmt.

        Override with a different method or different call to
        _implMakeProseList if you want to add a comma for two elements,
        or not use a serial comma.
        """
        return self._implMakeProseList(elements, fmt, with_verb, *args, **kwargs)

    @property
    def struct_macro(self):
        """Get the appropriate format macro for a structure.

        May override.
        """
        return 'slink:'

    @property
    def external_macro(self):
        """Get the appropriate format macro for an external type like uint32_t.

        May override.
        """
        return 'code:'

    @property
    def allows_x_number_suffix(self):
        """Whether vendor tags can be suffixed with X and a number to mark experimental extensions."""
        return False

    @property
    @abc.abstractmethod
    def structtype_member_name(self):
        """Return name of the structure type member.

        Must implement.
        """
        raise NotImplementedError()

    @property
    @abc.abstractmethod
    def nextpointer_member_name(self):
        """Return name of the structure pointer chain member.

        Must implement.
        """
        raise NotImplementedError()

    @property
    @abc.abstractmethod
    def xml_api_name(self):
        """Return the name used in the default API XML registry for the default API"""
        raise NotImplementedError()

    @abc.abstractmethod
    def generate_structure_type_from_name(self, structname):
        """Generate a structure type name, like XR_TYPE_CREATE_INSTANCE_INFO.

        Must implement.
        """
        raise NotImplementedError()

    def makeStructName(self, name):
        """Prepend the appropriate format macro for a structure to a structure type name.

        Uses struct_macro, so just override that if you want to change behavior.
        """
        return self.struct_macro + name

    def makeExternalTypeName(self, name):
        """Prepend the appropriate format macro for an external type like uint32_t to a type name.

        Uses external_macro, so just override that if you want to change behavior.
        """
        return self.external_macro + name

    def _implMakeProseList(self, elements, fmt, with_verb, comma_for_two_elts=False, serial_comma=True):
        """Internal-use implementation to make a (comma-separated) list for use in prose.

        Adds a connective (by default, 'and')
        before the last element if there are more than 1,
        and only includes commas if there are more than 2
        (if comma_for_two_elts is False).

        Adds the right one of "is" or "are" to the end if with_verb is true.

        Optionally adds a quantifier (like 'any') before a list of 2 or more,
        if specified by fmt.

        Do not edit these defaults, override self.makeProseList().
        """
        assert serial_comma  # did not implement what we did not need
        if isinstance(fmt, str):
            fmt = ProseListFormats.from_string(fmt)

        my_elts = list(elements)
        if len(my_elts) > 1:
            my_elts[-1] = f'{fmt.connective} {my_elts[-1]}'

        if not comma_for_two_elts and len(my_elts) <= 2:
            prose = ' '.join(my_elts)
        else:
            prose = ', '.join(my_elts)

        quantifier = fmt.quantifier(len(my_elts))

        parts = [quantifier, prose]

        if with_verb:
            if len(my_elts) > 1:
                parts.append(' are')
            else:
                parts.append(' is')
        return ''.join(parts)

    @property
    @abc.abstractmethod
    def file_suffix(self):
        """Return suffix of generated Asciidoctor files"""
        raise NotImplementedError

    @abc.abstractmethod
    def api_name(self, spectype=None):
        """Return API or specification name for citations in ref pages.

        spectype is the spec this refpage is for.
        'api' (the default value) is the main API Specification.
        If an unrecognized spectype is given, returns None.

        Must implement."""
        raise NotImplementedError

    def should_insert_may_alias_macro(self, genOpts):
        """Return true if we should insert a "may alias" macro in this file.

        Only used by OpenXR right now."""
        return False

    @property
    def command_prefix(self):
        """Return the expected prefix of commands/functions.

        Implemented in terms of api_prefix."""
        if not self._command_prefix:
            self._command_prefix = self.api_prefix[:].replace('_', '').lower()
        return self._command_prefix

    @property
    def type_prefix(self):
        """Return the expected prefix of type names.

        Implemented in terms of command_prefix (and in turn, api_prefix)."""
        if not self._type_prefix:
            self._type_prefix = ''.join(
                (self.command_prefix[0:1].upper(), self.command_prefix[1:]))
        return self._type_prefix

    @property
    @abc.abstractmethod
    def api_prefix(self):
        """Return API token prefix.

        Typically two uppercase letters followed by an underscore.

        Must implement."""
        raise NotImplementedError

    @property
    def extension_name_prefix(self):
        """Return extension name prefix.

        Typically two uppercase letters followed by an underscore.

        Assumed to be the same as api_prefix, but some APIs use different
        case conventions."""

        return self.api_prefix

    def extension_short_description(self, elem):
        """Return a short description of an extension for use in refpages.

        elem is an ElementTree for the <extension> tag in the XML.
        The default behavior is to use the 'type' field of this tag, but not
        all APIs support this field."""

        ext_type = elem.get('type')

        if ext_type is not None:
            return f'{ext_type} extension'
        else:
            return ''

    @property
    def write_contacts(self):
        """Return whether contact list should be written to extension appendices"""
        return False

    @property
    def write_extension_type(self):
        """Return whether extension type should be written to extension appendices"""
        return True

    @property
    def write_extension_number(self):
        """Return whether extension number should be written to extension appendices"""
        return True

    @property
    def write_extension_revision(self):
        """Return whether extension revision number should be written to extension appendices"""
        return True

    @property
    def write_refpage_include(self):
        """Return whether refpage include should be written to extension appendices"""
        return True

    @property
    def api_version_prefix(self):
        """Return API core version token prefix.

        Implemented in terms of api_prefix.

        May override."""
        return f"{self.api_prefix}VERSION_"

    @property
    def KHR_prefix(self):
        """Return extension name prefix for KHR extensions.

        Implemented in terms of api_prefix.

        May override."""
        return f"{self.api_prefix}KHR_"

    @property
    def EXT_prefix(self):
        """Return extension name prefix for EXT extensions.

        Implemented in terms of api_prefix.

        May override."""
        return f"{self.api_prefix}EXT_"

    def writeFeature(self, featureName, featureExtraProtect, filename):
        """Return True if OutputGenerator.endFeature should write this feature.

        Defaults to always True.
        Used in COutputGenerator.

        May override."""
        return True

    def requires_error_validation(self, return_type):
        """Return True if the return_type element is an API result code
        requiring error validation.

        Defaults to always False.

        May override."""
        return False

    @property
    def required_errors(self):
        """Return a list of required error codes for validation.

        Defaults to an empty list.

        May override."""
        return []

    def is_voidpointer_alias(self, tag, text, tail):
        """Return True if the declaration components (tag,text,tail) of an
        element represents a void * type.

        Defaults to a reasonable implementation.

        May override."""
        return tag == 'type' and text == 'void' and tail.startswith('*')

    def make_voidpointer_alias(self, tail):
        """Reformat a void * declaration to include the API alias macro.

        Defaults to a no-op.

        Must override if you actually want to use this feature in your project."""
        return tail

    def category_requires_validation(self, category):
        """Return True if the given type 'category' always requires validation.

        Defaults to a reasonable implementation.

        May override."""
        return category in CATEGORIES_REQUIRING_VALIDATION

    def type_always_valid(self, typename):
        """Return True if the given type name is always valid (never requires validation).

        This is for things like integers.

        Defaults to a reasonable implementation.

        May override."""
        return typename in TYPES_KNOWN_ALWAYS_VALID

    @property
    def should_skip_checking_codes(self):
        """Return True if more than the basic validation of return codes should
        be skipped for a command."""

        return False

    @property
    def generate_index_terms(self):
        """Return True if asiidoctor index terms should be generated as part
           of an API interface from the docgenerator."""

        return False

    @property
    def generate_enum_table(self):
        """Return True if asciidoctor tables describing enumerants in a
           group should be generated as part of group generation."""
        return False

    @property
    def generate_max_enum_in_docs(self):
        """Return True if MAX_ENUM tokens should be generated in
           documentation includes."""
        return False

    def extension_name_split(self, name):
        """Split an extension name, returning (vendor, rest of name).
           The API prefix of the name is ignored."""

        match = EXT_NAME_DECOMPOSE_RE.match(name)
        vendor = match.group('vendor')
        bare_name = match.group('name')

        return (vendor, bare_name)

    @abc.abstractmethod
    def extension_file_path(self, name):
        """Return file path to an extension appendix relative to a directory
           containing all such appendices.
           - name - extension name

           Must implement."""
        raise NotImplementedError

    def extension_include_string(self, name):
        """Return format string for include:: line for an extension appendix
           file.
            - name - extension name"""

        return f'include::{{appendices}}/{self.extension_file_path(name)}[]'

    @property
    def provisional_extension_warning(self):
        """Return True if a warning should be included in extension
           appendices for provisional extensions."""
        return True

    @property
    def generated_include_path(self):
        """Return path relative to the generated reference pages, to the
           generated API include files."""

        return '{generated}'

    @property
    def include_extension_appendix_in_refpage(self):
        """Return True if generating extension refpages by embedding
           extension appendix content (default), False otherwise
           (OpenXR)."""

        return True

    def valid_flag_bit(self, bitpos):
        """Return True if bitpos is an allowed numeric bit position for
           an API flag.

           Behavior depends on the data type used for flags (which may be 32
           or 64 bits), and may depend on assumptions about compiler
           handling of sign bits in enumerated types, as well."""
        return True

    @property
    def duplicate_aliased_structs(self):
        """
        Should aliased structs have the original struct definition listed in the
        generated docs snippet?
        """
        return False

    @property
    def protectProtoComment(self):
        """Return True if generated #endif should have a comment matching
           the protection symbol used in the opening #ifdef/#ifndef."""
        return False

    @property
    def extra_refpage_headers(self):
        """Return any extra headers (preceding the title) for generated
           reference pages."""
        return ''

    @property
    def extra_refpage_body(self):
        """Return any extra text (following the title) for generated
           reference pages."""
        return ''

    def is_api_version_name(self, name):
        """Return True if name is an API version name."""

        return API_VERSION_NAME_RE.match(name) is not None

    @property
    def docgen_language(self):
        """Return the language to be used in docgenerator [source]
           blocks."""

        return 'c++'

    @property
    def docgen_source_options(self):
        """Return block options to be used in docgenerator [source] blocks,
           which are appended to the 'source' block type.
           Can be empty."""

        return '%unbreakable'
