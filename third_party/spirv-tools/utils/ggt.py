#!/usr/bin/env python3
# Copyright (c) 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Generates compressed grammar tables from SPIR-V JSON grammar."""

import errno
import json
import os.path
import re
import sys
from typing import Dict, List, Tuple, Any

# Find modules relative to the directory containing this script.
# This is needed for hermetic Bazel builds, where the Table files are bundled
# together with this script, while keeping their relative locations.
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from Table.Context import Context
from Table.IndexRange import IndexRange
from Table.Operand import Operand

class GrammarError(Exception):
    pass

# Extensions to recognize, but which don't necessarily come from the SPIR-V
# core or KHR grammar files.  Get this list from the SPIR-V registry web page.
# NOTE: Only put things on this list if it is not in those grammar files.
EXTENSIONS_FROM_SPIRV_REGISTRY_AND_NOT_FROM_GRAMMARS = """
SPV_AMD_gpu_shader_half_float
SPV_AMD_gpu_shader_int16
SPV_KHR_non_semantic_info
SPV_EXT_relaxed_printf_string_address_space
"""

class ExtInst():
    """
    An extended instruction set.

    Properties:
       prefix: the string prefix for operand enums. Often an empty string.
       file: the location of the JSON grammar file
       name: the name, can be used as an identifier
       enum_name: the enum name, e.g. SPV_EXT_INST_OPENCL_STD
       grammar: the JSON object for the grammar, loaded from the file.
    """
    def __init__(self,spec: str):
        matches = re.fullmatch('^([^,]*),(.*)',spec)
        if matches is None:
            raise Exception("Invalid prefix and path: {}".format(spec))
        self.prefix = matches[1]
        if self.prefix is None:
            self.prefix = ""
        self.file = matches[2]
        matches = re.match('.*extinst\\.(.*)\\.grammar.json', self.file)
        if matches is None:
            raise Exception("Invalid grammar file name: {}".format(self.file))
        self.name = matches[1].replace('-','_').replace('.','_')

        self.enum_name = 'SPV_EXT_INST_TYPE_{}'.format(self.name).upper()
        if self.enum_name == "SPV_EXT_INST_TYPE_OPENCL_STD_100":
            # Live with an old decision, by adjusting this name.
            self.enum_name = "SPV_EXT_INST_TYPE_OPENCL_STD"

        self.load()

    def load(self):
        """
        Populates self.grammar from the file.
        Applies the self.prefix to operand enums
        """
        with open(self.file) as json_file:
            self.grammar = json.loads(json_file.read())
            if len(self.prefix) > 0:
                prefix_operand_kind_names(self.prefix, self.grammar)


def convert_min_required_version(version): # (version: str | None) -> str
    """Converts the minimal required SPIR-V version encoded in the grammar to
    the symbol in SPIRV-Tools."""
    if version is None:
        return 'SPV_SPIRV_VERSION_WORD(1, 0)'
    if version == 'None':
        return '0xffffffffu'
    return 'SPV_SPIRV_VERSION_WORD({})'.format(version.replace('.', ','))


def convert_max_required_version(version): # (version: str | None) -> str
    """Converts the maximum required SPIR-V version encoded in the grammar to
    the symbol in SPIRV-Tools."""
    if version is None:
        return '0xffffffffu'
    return 'SPV_SPIRV_VERSION_WORD({})'.format(version.replace('.', ','))


def c_bool(b: bool) -> str:
    return 'true' if b else 'false'


def ctype(kind: str, quantifier: str) -> str:
    """Returns the corresponding operand type used in spirv-tools for the given
    operand kind and quantifier used in the JSON grammar.

    Arguments:
      - kind, e.g. 'IdRef'
      - quantifier, e.g. '', '?', '*'

    Returns:
      a string of the enumerant name in spv_operand_type_t
    """
    if kind == '':
        raise Error("operand JSON object missing a 'kind' field")
    # The following cases are where we differ between the JSON grammar and
    # spirv-tools.
    if kind == 'IdResultType':
        kind = 'TypeId'
    elif kind == 'IdResult':
        kind = 'ResultId'
    elif kind == 'IdMemorySemantics' or kind == 'MemorySemantics':
        kind = 'MemorySemanticsId'
    elif kind == 'IdScope' or kind == 'Scope':
        kind = 'ScopeId'
    elif kind == 'IdRef':
        kind = 'Id'

    elif kind == 'ImageOperands':
        kind = 'Image'
    elif kind == 'Dim':
        kind = 'Dimensionality'
    elif kind == 'ImageFormat':
        kind = 'SamplerImageFormat'
    elif kind == 'KernelEnqueueFlags':
        kind = 'KernelEnqFlags'

    elif kind == 'LiteralExtInstInteger':
        kind = 'ExtensionInstructionNumber'
    elif kind == 'LiteralSpecConstantOpInteger':
        kind = 'SpecConstantOpNumber'
    elif kind == 'LiteralContextDependentNumber':
        kind = 'TypedLiteralNumber'

    elif kind == 'PairLiteralIntegerIdRef':
        kind = 'LiteralIntegerId'
    elif kind == 'PairIdRefLiteralInteger':
        kind = 'IdLiteralInteger'
    elif kind == 'PairIdRefIdRef':  # Used by OpPhi in the grammar
        kind = 'Id'

    if kind == 'FPRoundingMode':
        kind = 'FpRoundingMode'
    elif kind == 'FPFastMathMode':
        kind = 'FpFastMathMode'

    if quantifier == '?':
        kind = 'Optional{}'.format(kind)
    elif quantifier == '*':
        kind = 'Variable{}'.format(kind)

    return 'SPV_OPERAND_TYPE_{}'.format(
        re.sub(r'([a-z])([A-Z])', r'\1_\2', kind).upper())


def convert_operand_kind(obj: Dict[str, str]) -> str:
    """Returns the corresponding operand type used in spirv-tools for the given
    operand kind and quantifier used in the JSON grammar.

    Arguments:
      - obj: an instruction operand, having keys:
          - 'kind', e.g. 'IdRef'
          - optionally, a quantifier: '?' or '*'

    Returns:
      a string of the enumerant name in spv_operand_type_t
    """
    kind = obj.get('kind', '')
    quantifier = obj.get('quantifier', '')
    return ctype(kind, quantifier)


def to_safe_identifier(s: str) -> str:
    """
    Returns a new string with all non-letters converted to underscores,
    and prepending 'k'.
    The result should be safe to use as a C identifier.
    """
    return 'k' + re.sub(r'[^a-zA-Z0-9]', '_', s)


class Grammar():
    """
    Accumulates string and enum tables.
    The extensions and operand kinds lists are fixed at creation time.
    Prints tables for instructions, operand kinds, and underlying string
    and enum tables.
    Assumes an index range is emitted by printing an IndexRange object.
    """
    def __init__(self, extensions: List[str], operand_kinds:List[dict], printing_classes: List[str]) -> None:
        self.context = Context()
        self.extensions = sorted(extensions)
        self.operand_kinds = sorted(operand_kinds, key = lambda ok: convert_operand_kind(ok))
        self.printing_classes = sorted([to_safe_identifier(x) for x in printing_classes])

        # The self.header_ignore_decls are only used to debug the flow.
        # They are copied into the C++ source code where they are more likely
        # to be seen by humans.
        self.header_ignore_decls: List[str] = [self.IndexRangeDecls()]

        # The self.header_decls content goes into core_tables_header.inc to be
        # included in a .h file.
        self.header_decls: List[str] = []
        # The self.body_decls content goes into core_tables_body.inc to be included
        # in a .cpp file.  It includes definitions of static variables and
        # hidden functions.
        self.body_decls: List[str] = []

        if len(self.operand_kinds) == 0:
            raise Exception("operand_kinds should be a non-empty list")
        if len(self.extensions) == 0:
            raise Exception("extensions should be a non-empty list")

        self.ComputePrintingClassDecls()
        self.ComputeExtensionDecls()

        # These operand kinds need to have their optional counterpart to also
        # be represented in the lookup tables, with the same content.
        self.operand_kinds_needing_optional_variant = [
                'ImageOperands',
                'AccessQualifier',
                'MemoryAccess',
                'PackedVectorFormat',
                'CooperativeMatrixOperands',
                'MatrixMultiplyAccumulateOperands',
                'RawAccessChainOperands',
                'FPEncoding',
                'TensorOperands']

    def dump(self) -> None:
        self.context.dump()

    def IndexRangeDecls(self) -> str:
        return """
struct IndexRange {
  uint32_t first = 0; // index of the first element in the range
  uint32_t count = 0; // number of elements in the range
};
constexpr inline IndexRange IR(uint32_t first, uint32_t count) {
  return {first, count};
}
"""

    def ComputePrintingClassDecls(self) -> str:
        parts: List[str] = []
        parts.append("enum class PrintingClass : uint32_t {");
        parts.extend(["  {},".format(x) for x in self.printing_classes])
        parts.append("};\n")
        self.header_decls.extend(parts)

    def ComputeExtensionDecls(self) -> None:
        parts: List[str] = []
        parts.append("enum Extension : uint32_t {");
        parts.extend(["  {},".format(to_safe_identifier(x)) for x in self.extensions])
        parts.append("};\n")
        self.header_decls.extend(parts)

        parts = []
        parts.append("// Returns the name of an extension, as an index into kStrings")
        parts.append("IndexRange ExtensionToIndexRange(Extension extension) {\n  switch(extension) {")
        for e in self.extensions:
            parts.append('    case Extension::k{}: return {};'.format(e,self.context.AddString(e)))
        parts.append("    default: break;");
        parts.append('  }\n  return {};\n}\n');
        self.body_decls.extend(parts)

        parts = []
        parts.append("""// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value""")
        parts.append("static const std::array<NameValue,{}> kExtensionNames{{{{".format(len(self.extensions)))
        for e in self.extensions:
            parts.append('    {{{}, static_cast<uint32_t>({})}},'.format(self.context.AddString(e), to_safe_identifier(e)))
        parts.append("}};\n")
        self.body_decls.extend(parts)

    def ComputeOperandTables(self) -> None:
        """
        Returns the string for the C definitions of the operand kind tables.

        An operand kind such as ImageOperands also has an associated
        operand kind that is an 'optional' variant.
        These are represented as two distinct operand kinds in spv_operand_type_t.
        For example, ImageOperands maps to both SPV_OPERAND_TYPE_IMAGE, and also
        to SPV_OPERAND_TYPE_OPTIONAL_IMAGE.

        The definitions are:
         - kOperandsByValue: a 1-dimensional array of all operand descriptions
           sorted first by operand kind, then by operand value.
           Only non-optional operand kinds are represented here.

         - kOperandsByValueRangeByKind: a function mapping from operand kind to
           the index range into kOperandByValue.
           This has mappings for both concrete and corresponding optional operand kinds.

         - kOperandNames: a 1-dimensional array of all operand NameIndex
           entries, sorted first by operand kinds, then by operand name.
           The name part is represented by an index range into the string table.
           The index part is the index of this name's entry into the by-value array.
           This can have more entries than the by-value array, because names
           can have string aliases. For example,the MemorySemantics value 0
           is named both "Relaxed" and "None".
           Only non-optional operand kinds are represented here.

         - kOperandNamesRangeByKind: a mapping from operand kind to the index
           range into kOperandNames.
           This has mappings for both concrete and corresponding optional operand kinds.
        """

        self.header_ignore_decls.append(
"""
struct NameIndex {
  // Location of the null-terminated name in the global string table.
  IndexRange name;
  // Index of this name's entry in in the associated by-value table.
  uint32_t index;
};
struct NameValue {
  // Location of the null-terminated name in the global string table.
  IndexRange name;
  // Enum value in the binary format.
  uint32_t value;
};
// Describes a SPIR-V operand.
struct OperandDesc {
  uint32_t value;
  IndexRange operands_range;      // Indexes kOperandSpans
  IndexRange name_range;          // Indexes kStrings
  IndexRange aliases_range;       // Indexes kAliasSpans
  IndexRange capabilities_range;  // Indexes kCapabilitySpans
  // A set of extensions that enable this feature. If empty then this operand
  // value is in core and its availability is subject to minVersion. The
  // assembler, binary parser, and disassembler ignore this rule, so you can
  // freely process invalid modules.
  IndexRange extensions_range;  // Indexes kExtensionSpans
  // Minimal core SPIR-V version required for this feature, if without
  // extensions. ~0u means reserved for future use. ~0u and non-empty
  // extension lists means only available in extensions.
  uint32_t minVersion;
  uint32_t lastVersion;
  utils::Span<spv_operand_type_t> operands() const;
  utils::Span<char> name() const;
  utils::Span<IndexRange> aliases() const;
  utils::Span<spv::Capability> capabilities() const;
  utils::Span<spvtools::Extension> extensions() const;
  OperandDesc(const OperandDesc&) = delete;
  OperandDesc(OperandDesc&&) = delete;
};
""")

        def ShouldEmit(operand_kind_json: Dict[str,any]):
            """ Returns true if we should emit a table for the given
            operand kind.
            """
            category = operand_kind_json.get('category')
            return category in ['ValueEnum', 'BitEnum']

        # Populate kOperandsByValue
        operands_by_value: List[str] = []
        operands_by_value_by_kind: Dict[str,IndexRange] = {}
        # Maps the operand kind and value to the index into kOperandsByValue
        index_by_kind_and_value: Dict[Tuple(str,int),int] = {}
        index = 0
        for operand_kind_json in self.operand_kinds:
            kind_key: str = convert_operand_kind(operand_kind_json)
            if ShouldEmit(operand_kind_json):
                operands = [Operand(o) for o in operand_kind_json['enumerants']]
                operand_descs: List[str] = []
                for o in sorted(operands, key = lambda o: o.value):
                    suboperands = [convert_operand_kind(p) for p in o.parameters]
                    desc = [
                        o.value,
                        self.context.AddStringList('operand', suboperands),
                        str(self.context.AddString(o.enumerant)) + '/* {} */'.format(o.enumerant),
                        self.context.AddStringList('alias', o.aliases),
                        self.context.AddStringList('capability', o.capabilities),
                        self.context.AddStringList('extension', o.extensions),
                        convert_min_required_version(o.version),
                        convert_max_required_version(o.lastVersion),
                    ]
                    operand_descs.append('{' + ','.join([str(d) for d in desc]) + '}}, // {}'.format(kind_key))
                    index_by_kind_and_value[(kind_key,o.value)] = index
                    index += 1
                operands_by_value_by_kind[kind_key] = IndexRange(len(operands_by_value), len(operand_descs))
                operands_by_value.extend(operand_descs)
            else:
                pass

        parts = []
        parts.append("""// Operand descriptions, ordered by (operand kind, operand enum value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   aliases, an IndexRange into kAliasSpans
//   capabilities, an IndexRange into kCapabilitySpans
//   extensions, as an IndexRange into kExtensionSpans
//   version, first version of SPIR-V that has it
//   lastVersion, last version of SPIR-V that has it""")
        parts.append("static const std::array<OperandDesc, {}> kOperandsByValue{{{{".format(len(operands_by_value)))
        parts.extend(['  ' + str(x) for x in operands_by_value])
        parts.append("}};\n")
        self.body_decls.extend(parts)

        parts = []
        parts.append("""// Maps an operand kind to possible operands for that kind.
// The result is an IndexRange into kOperandsByValue, and the operands
// are sorted by value within that span.
// An optional variant of a kind maps to the details for the corresponding
// concrete operand kind.""")
        parts.append("IndexRange OperandByValueRangeForKind(spv_operand_type_t type) {\n  switch(type) {")
        for kind_key, ir in operands_by_value_by_kind.items():
            parts.append("    case {}: return {};".format(
                kind_key,
                str(operands_by_value_by_kind[kind_key])))
        for kind in self.operand_kinds_needing_optional_variant:
            non_optional_kind = ctype(kind,'')
            if non_optional_kind in operands_by_value_by_kind:
                parts.append("    case {}: return {};".format(
                    ctype(kind, '?'),
                    str(operands_by_value_by_kind[ctype(kind,'')])))
            else:
                raise GrammarError(
                        "error: unknown operand type {}, from JSON grammar operand '{}':".format(non_optional_kind, kind) +
                        " consider updating spv_operand_type_t in spirv-tools/libspirv.h")
            
        parts.append("    default: break;");
        parts.append("  }\n  return IR(0,0);\n}\n")
        self.body_decls.extend(parts)

        # Populate kOperandNames
        operand_names: List[Tuple[IndexRange,int]] = []
        name_range_for_kind: Dict[str,IndexRange] = {}
        for operand_kind_json in self.operand_kinds:
            kind_key: str = convert_operand_kind(operand_kind_json)
            if ShouldEmit(operand_kind_json):
                operands = [Operand(o) for o in operand_kind_json['enumerants']]
                tuples: List[Tuple[str,int,str]] = []
                for o in operands:
                    tuples.append((o.enumerant, o.value, kind_key))
                    for a in o.aliases:
                        tuples.append((a, o.value, kind_key))
                tuples = sorted(tuples, key = lambda t: t[0])
                ir_tuples = [(self.context.AddString(t[0]),t[1],t[2]) for t in tuples]
                name_range_for_kind[kind_key] = IndexRange(len(operand_names), len(ir_tuples))
                operand_names.extend(ir_tuples)
            else:
                pass
        operand_name_strings: List[str] = []
        for i in range(0, len(operand_names)):
            ir, value, kind_key = operand_names[i]
            index = index_by_kind_and_value[(kind_key,value)]
            operand_name_strings.append('{{{}, {}}}, // {} {} in {}'.format(
                str(ir),index,i,self.context.GetString(ir),kind_key))

        parts: List[str] = []
        parts.append("""// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array""")
        parts.append("static const std::array<NameIndex, {}> kOperandNames{{{{".format(len(operand_name_strings)))
        parts.extend(['  ' + str(x) for x in operand_name_strings])
        parts.append("}};\n")
        self.body_decls.extend(parts)

        parts.append("""// Maps an operand kind to possible names for operands of that kind.
// The result is an IndexRange into kOperandNames, and the names
// are sorted by name within that span.
// An optional variant of a kind maps to the details for the corresponding
// concrete operand kind.""")
        parts = ["IndexRange OperandNameRangeForKind(spv_operand_type_t type) {\n  switch(type) {"]
        for kind_key, ir in name_range_for_kind.items():
            parts.append("    case {}: return {};".format(
                kind_key,
                str(name_range_for_kind[kind_key])))
        for kind in self.operand_kinds_needing_optional_variant:
            parts.append("    case {}: return {};".format(
                ctype(kind, '?'),
                str(name_range_for_kind[ctype(kind,'')])))
        parts.append("    default: break;");
        parts.append("  }\n  return IR(0,0);\n}\n")
        self.body_decls.extend(parts)


    def ComputeInstructionTables(self, insts) -> None:
        """
        Creates declarations for instruction tables.
        Populates self.header_ignore_decls, self.body_decls.

        Params:
            insts: an array of instructions objects using the JSON schema
        """
        self.header_ignore_decls.append("""
// Describes an Instruction
struct InstructionDesc {
  const spv::Op value;
  const bool hasResult;
  const bool hasType;
  const IndexRange operands_range;      // Indexes kOperandSpans
  const IndexRange name_range;          // Indexes kStrings
  const IndexRange aliases_range;       // Indexes kAliasSpans
  const IndexRange capabilities_range;  // Indexes kCapbilitySpans
  // A set of extensions that enable this feature. If empty then this operand
  // value is in core and its availability is subject to minVersion. The
  // assembler, binary parser, and disassembler ignore this rule, so you can
  // freely process invalid modules.
  const IndexRange extensions_range;    // Indexes kExtensionSpans
  // Minimal core SPIR-V version required for this feature, if without
  // extensions. ~0u means reserved for future use. ~0u and non-empty
  // extension lists means only available in extensions.
  uint32_t minVersion;
  uint32_t lastVersion;
  PrintingClass printingClass; // Section of SPIR-V spec. e.g. kComposite, kImage
  utils::Span<spv_operand_type_t> operands() const;
  utils::Span<char> name() const;
  utils::Span<IndexRange> aliases() const;
  utils::Span<spv::Capability> capabilities() const;
  utils::Span<spvtools::Extension> extensions() const;
  OperandDesc(const OperandDesc&) = delete;
  OperandDesc(OperandDesc&&) = delete;
};
""")

        # Create the array of InstructionDesc
        lines: List[str] = []
        # Maps the opcode name (without "Op" prefix) to its index in the table.
        index_by_opcode: Dict[int,int] = {}
        # Sort by opcode, so lookup can use binary search
        for inst in sorted(insts, key = lambda inst: int(inst['opcode'])):
            parts: List[str] = []

            opname: str = inst['opname']

            operand_kinds = [convert_operand_kind(o) for o in inst.get('operands',[])]
            if opname == 'OpExtInst' and operand_kinds[-1] == 'SPV_OPERAND_TYPE_VARIABLE_ID':
                # The published grammar uses 'sequence of ID' at the
                # end of the ExtInst operands. But SPIRV-Tools uses
                # a specific pattern based on the particular opcode.
                # Drop it here.
                # See https://github.com/KhronosGroup/SPIRV-Tools/issues/233
                operand_kinds.pop()

            hasResult = 'SPV_OPERAND_TYPE_RESULT_ID' in operand_kinds
            hasType = 'SPV_OPERAND_TYPE_TYPE_ID' in operand_kinds

            # Remove the "Op" prefix from opcode alias names
            aliases = [name[2:] for name in inst.get('aliases',[])]

            parts.extend([
                'spv::Op::' + opname,
                c_bool(hasResult),
                c_bool(hasType),
                self.context.AddStringList('operand', operand_kinds),
                self.context.AddString(opname[2:]),
                self.context.AddStringList('alias', aliases),
                self.context.AddStringList('capability', inst.get('capabilities',[])),
                self.context.AddStringList('extension', inst.get('extensions',[])),
                convert_min_required_version(inst.get('version', None)),
                convert_max_required_version(inst.get('lastVersion', None)),
                'PrintingClass::' + to_safe_identifier(inst.get('class','@exclude'))
            ])

            index_by_opcode[int(inst['opcode'])] = len(lines)
            lines.append('{{{}}},'.format(', '.join([str(x) for x in parts])))
        parts = []
        parts.append("""// Instruction descriptions, ordered by opcode.
// The fields in order are:
//   opcode
//   a boolean indicating if the instruction produces a result ID
//   a boolean indicating if the instruction result ID has a type
//   operands, an IndexRange into kOperandSpans
//   opcode name (without the 'Op' prefix), a character-counting IndexRange into kStrings
//   aliases, an IndexRange into kAliasSpans
//   capabilities, an IndexRange into kCapabilitySpans
//   extensions, as an IndexRange into kExtensionSpans
//   version, first version of SPIR-V that has it
//   lastVersion, last version of SPIR-V that has it""")
        parts.append("static const std::array<InstructionDesc, {}> kInstructionDesc{{{{".format(len(lines)));
        parts.extend(['  ' + l for l in lines])
        parts.append("}};\n");
        self.body_decls.extend(parts)

        # Create kInstructionNames.
        opcode_name_entries: List[str] = []
        name_value_pairs: List[Tuple[str,int]] = []
        for i in insts:
            name_value_pairs.append((i['opname'][2:], i['opcode']))
            for a in i.get('aliases',[]):
                name_value_pairs.append((a[2:], i['opcode']))
        name_value_pairs = sorted(name_value_pairs)
        inst_name_strings: List[str] = []
        for i in range(0, len(name_value_pairs)):
            name, value = name_value_pairs[i]
            ir = self.context.AddString(name)
            index = index_by_opcode[value]
            inst_name_strings.append('{{{}, {}}}, // {} {}'.format(str(ir),index,i,name))
        parts: List[str] = []
        parts.append("""// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc""")
        parts.append("static const std::array<NameIndex, {}> kInstructionNames{{{{".format(len(inst_name_strings)))
        parts.extend(['  ' + str(x) for x in inst_name_strings])
        parts.append("}};\n")
        self.body_decls.extend(parts)


    def ComputeExtendedInstructions(self, extinsts) -> None:
        """
        Generates tables for extended instructions

        Args:
            self
            extinsts: a list of extinst objects
        """

        """
            ExtInstDesc {
             value:         uint32_t
             name:          IndexRange
             operands:      IndexRange
             capabilities:  IndexRange
            }

        The definitions are:
         - kExtInstByValue: a 1-dimensional array of all operand descriptions
           sorted first by extended instruction enum, then by operand value.

         - ExtInstByValueRangeForKind: a function mapping from extinst enum to
           the index range into kExtInstByValue.

         - kExtInstNames: a 1-dimensional array of all extinst name-index pairs,
           sorted first by extinst enum, then by operand name.
           The name part is represented by an index range into the string table.
           The index part is the index of this name's entry in the kExtInstByValue
           array.

         - kExtInstNamesRangeByKind: a mapping from operand kind to the index
           range into kOperandNames.
           This has mappings for both concrete and corresponding optional operand kinds.
        """

        # Create kExtInstByValue
        by_value: List[List[Any]] = []
        by_value_by_kind: Dict[str,IndexRange] = {}
        index_by_kind_and_opcode: Dict[Tuple[str,int],int] = {}
        index = 0
        for e in extinsts:
            insts_in_set = []
            for inst in sorted(e.grammar['instructions'], key = lambda inst: inst['opcode']):
                operands = [convert_operand_kind(o) for o in inst.get('operands',[])]
                inst_parts = [
                    inst['opcode'],
                    self.context.AddStringList('operand', operands),
                    self.context.AddString(inst['opname']),
                    self.context.AddStringList('capability', inst.get('capabilities',[])),
                ]
                inst_parts = [str(x) for x in inst_parts]
                insts_in_set.append('    {{{}}}, // {} in {}'.format(
                        ','.join(inst_parts), inst['opname'], e.name))
                index_by_kind_and_opcode[(e.enum_name,int(inst['opcode']))] = index
                index += 1
            by_value_by_kind[e.enum_name] = IndexRange(len(by_value), len(insts_in_set))
            by_value.extend(insts_in_set)

        parts: List[str] = []
        parts.append("""// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans""")
        parts.append("static const std::array<ExtInstDesc, {}> kExtInstByValue{{{{".format(len(by_value)))
        parts.extend(by_value)
        parts.append('}};\n')
        self.body_decls.extend(parts)

        # Create kExtInstByValueRangeForKind
        parts = []
        parts.append("""// Maps an extended instruction enum to possible names for operands of that kind.
// The result is an IndexRange into kOperandNames, and the names
// are sorted by name within that span.
// An optional variant of a kind maps to the details for the corresponding
// concrete operand kind.""")
        parts = ["IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type) {\n  switch(type) {"]
        for name, ir in by_value_by_kind.items():
            parts.append("    case {}: return {};".format(name, ir))
        parts.append("    default: break;");
        parts.append("  }\n  return IR(0,0);\n}\n")
        self.body_decls.extend(parts)

        # Create kExtInstNames
        parts = []
        by_name: List[List[Any]] = []
        by_name_by_kind: Dict[str,IndexRange] = {}
        for e in extinsts:
            # Sort by name within a set
            insts_by_name = sorted(e.grammar['instructions'], key = lambda i: i['opname'])
            insts_in_set = []
            for inst in insts_by_name:
                index = index_by_kind_and_opcode[(e.enum_name,int(inst['opcode']))]
                insts_in_set.append(
                        '    {{{}, {}}}, // {} in {}'.format(
                                str(self.context.AddString(inst['opname'])),
                                index,
                                inst['opname'],
                                e.name))
            by_name_by_kind[e.enum_name] = IndexRange(len(by_name), len(insts_in_set))
            by_name.extend(insts_in_set)
        parts.append("""// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue""")
        parts.append("static const std::array<NameIndex, {}> kExtInstNames{{{{".format(len(by_name)))
        parts.extend(by_name)
        parts.append('}};\n')
        self.body_decls.extend(parts)

        # Create kExtInstNameRangeByKind
        parts = []
        parts.append("""// Maps an extended instruction kind to possible names for instructions of that kind.
// The result is an IndexRange into kExtInstNames, and the names
// are sorted by name within that span.""")
        parts = ["IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type) {\n  switch(type) {"]
        for name, ir in by_name_by_kind.items():
            parts.append("    case {}: return {};".format(name, str(ir)))
        parts.append("    default: break;");
        parts.append("  }\n  return IR(0,0);\n}\n")
        self.body_decls.extend(parts)

    def ComputeLeafTables(self) -> None:
        """
        Generates the tables that the instruction and operand tables point to.
        The tables are:
            - the string table
            - the table of sequences of:
               - capabilities
               - extensions
               - operands

        This method must be called after computing instruction and operand tables.
        """

        def c_str(s: str):
            """
            Returns the source for a C string literal or the given string, including
            the explicit null at the end
            """
            return '"{}\\0"'.format(json.dumps(s).strip('"'))

        parts: List[str] = []
        parts.append("// Array of characters, referenced by IndexRanges elsewhere.")
        parts.append("// Each IndexRange denotes a string.")
        parts.append('static const char kStrings[] =');
        parts.extend(['  {} // {}'.format(c_str(s), str(self.context.strings[s])) for s in self.context.string_buffer])
        parts.append(';\n');
        self.body_decls.extend(parts);

        parts: List[str] = []
        parts.append("""// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.""")
        parts.append('static const IndexRange kAliasSpans[] = {');
        ranges = self.context.range_buffer['alias']
        for i in range(0, len(ranges)):
            ir = ranges[i]
            parts.append('  {}, // {} {}'.format(str(ir), i, self.context.GetString(ir)))
        parts.append('};\n');
        self.body_decls.extend(parts);

        parts = []
        parts.append("// Array of capabilities, referenced by IndexRanges elsewhere.")
        parts.append("// Contains all sequences of capabilities used in the grammar.")
        parts.append('static const spv::Capability kCapabilitySpans[] = {');
        capability_ranges = self.context.range_buffer['capability']
        for i in range(0, len(capability_ranges)):
            ir = capability_ranges[i]
            cap = self.context.GetString(ir)
            parts.append('  spv::Capability::{}, // {}'.format(cap, i))
        parts.append('};\n');
        self.body_decls.extend(parts);

        parts = []
        parts.append("// Array of extensions, referenced by IndexRanges elsewhere.")
        parts.append("// Contains all sequences of extensions used in the grammar.")
        parts.append('static const spvtools::Extension kExtensionSpans[] = {');
        ranges = self.context.range_buffer['extension']
        for i in range(0, len(ranges)):
            ir = ranges[i]
            name = self.context.GetString(ir)
            parts.append('  spvtools::Extension::k{}, // {}'.format(name, i))
        parts.append('};\n');
        self.body_decls.extend(parts);

        parts = []
        parts.append("// Array of operand types, referenced by IndexRanges elsewhere.")
        parts.append("// Contains all sequences of operand types used in the grammar.")
        parts.append('static const spv_operand_type_t kOperandSpans[] = {');
        ranges = self.context.range_buffer['operand']
        for i in range(0, len(ranges)):
            ir = ranges[i]
            name = self.context.GetString(ir)
            parts.append('  {}, // {}'.format(name, i))
        parts.append('};\n');
        self.body_decls.extend(parts)


def make_path_to_file(f: str) -> None:
    """Makes all ancestor directories to the given file, if they don't yet
    exist.

    Arguments:
        f: The file whose ancestor directories are to be created.
    """
    dir = os.path.dirname(os.path.abspath(f))
    try:
        os.makedirs(dir)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(dir):
            pass
        else:
            raise


def get_extension_list(instructions, operand_kinds):
    """Returns extensions as an alphabetically sorted list of strings.

    Args:
      instructions: list of instruction objects, using the JSON grammar file schema
      operand_kinds: list of operand_kind objects, using the JSON grammar file schema
    """

    things_with_an_extensions_field = [item for item in instructions]

    enumerants = sum([item.get('enumerants', [])
                      for item in operand_kinds], [])

    things_with_an_extensions_field.extend(enumerants)

    extensions = sum([item.get('extensions', [])
                      for item in things_with_an_extensions_field
                      if item.get('extensions')], [])

    for item in EXTENSIONS_FROM_SPIRV_REGISTRY_AND_NOT_FROM_GRAMMARS.split():
            # If it's already listed in a grammar, then don't put it in the
            # special exceptions list.
        assert item not in extensions, 'Extension %s is already in a grammar file' % item

    extensions.extend(
        EXTENSIONS_FROM_SPIRV_REGISTRY_AND_NOT_FROM_GRAMMARS.split())

    # Validator would ignore type declaration unique check. Should only be used
    # for legacy autogenerated test files containing multiple instances of the
    # same type declaration, if fixing the test by other methods is too
    # difficult. Shouldn't be used for any other reasons.
    extensions.append('SPV_VALIDATOR_ignore_type_decl_unique')

    return sorted(set(extensions))


def prefix_operand_kind_names(prefix, json_dict):
    """Modifies json_dict, by prefixing all the operand kind names
    with the given prefix.  Also modifies their uses in the instructions
    to match.
    """

    old_to_new = {}
    for operand_kind in json_dict["operand_kinds"]:
        old_name = operand_kind["kind"]
        new_name = prefix + old_name
        operand_kind["kind"] = new_name
        old_to_new[old_name] = new_name

    for instruction in json_dict["instructions"]:
        for operand in instruction.get("operands", []):
            replacement = old_to_new.get(operand["kind"])
            if replacement is not None:
                operand["kind"] = replacement


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Generate SPIR-V info tables')

    parser.add_argument('--spirv-core-grammar', metavar='<path>',
                        type=str, required=False,
                        help='input JSON grammar file for core SPIR-V '
                        'instructions')
    parser.add_argument('--extinst', metavar='<path>',
                        type=str, action='append', required=False, default=None,
                        help='extended instruction info: an enum prefix, then a comma, then'
                        ' the file location of the JSON grammar')

    parser.add_argument('--core-tables-body-output', metavar='<path>',
                        type=str, required=False, default=None,
                        help='output file for core SPIR-V grammar tables to be included in .cpp')
    parser.add_argument('--core-tables-header-output', metavar='<path>',
                        type=str, required=False, default=None,
                        help='output file for core SPIR-V grammar tables to be included in .h')

    args = parser.parse_args()

    if args.spirv_core_grammar is None:
        print('error: missing --spirv-core-grammar ')
        sys.exit(1)
    if (args.core_tables_body_output is None) and (args.core_tables_header_output is None):
        print('error: need at least one of --core-tables-body-output --core-tables-header-output ')
        sys.exit(1)
    if len(args.extinst) < 1:
        print('error: missing --extinst ')
        sys.exit(1)

    # Load the JSON grammar files.
    extinsts = sorted([ExtInst(e) for e in args.extinst], key = lambda e: e.name)
    with open(args.spirv_core_grammar) as json_file:
        core_grammar = json.loads(json_file.read())
        printing_class: List[str] = [e['tag'] for e in core_grammar['instruction_printing_class']]

    # Collect all operand kinds and instructions, so we can generate
    # extension lists, capability lists, and alias lists.
    # Make a copy to avoid polluting the instruction list.
    instructions = [x for x in core_grammar['instructions']]
    operand_kinds = [x for x in core_grammar['operand_kinds']]
    for e in extinsts:
        instructions.extend(e.grammar.get('instructions',[]))
        operand_kinds.extend(e.grammar.get('operand_kinds',[]))

    extensions = get_extension_list(instructions, operand_kinds)

    g = Grammar(extensions, operand_kinds, printing_class)
    g.ComputeOperandTables()
    g.ComputeInstructionTables(core_grammar['instructions'])
    g.ComputeExtendedInstructions(extinsts)
    g.ComputeLeafTables()

    if args.core_tables_body_output is not None:
        make_path_to_file(args.core_tables_body_output)
        with open(args.core_tables_body_output, 'w') as f:
            f.write('\n'.join(g.body_decls))
    if args.core_tables_header_output is not None:
        make_path_to_file(args.core_tables_header_output)
        with open(args.core_tables_header_output, 'w') as f:
            f.write('\n'.join(g.header_decls))
    sys.exit(0)


if __name__ == '__main__':
  try:
    main()
  except GrammarError as ge:
    print(ge)
    sys.exit(1)
