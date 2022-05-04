// Copyright (c) 2018 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

std::string LogStringForDecoration(uint32_t decoration) {
  switch (decoration) {
    case SpvDecorationRelaxedPrecision:
      return "RelaxedPrecision";
    case SpvDecorationSpecId:
      return "SpecId";
    case SpvDecorationBlock:
      return "Block";
    case SpvDecorationBufferBlock:
      return "BufferBlock";
    case SpvDecorationRowMajor:
      return "RowMajor";
    case SpvDecorationColMajor:
      return "ColMajor";
    case SpvDecorationArrayStride:
      return "ArrayStride";
    case SpvDecorationMatrixStride:
      return "MatrixStride";
    case SpvDecorationGLSLShared:
      return "GLSLShared";
    case SpvDecorationGLSLPacked:
      return "GLSLPacked";
    case SpvDecorationCPacked:
      return "CPacked";
    case SpvDecorationBuiltIn:
      return "BuiltIn";
    case SpvDecorationNoPerspective:
      return "NoPerspective";
    case SpvDecorationFlat:
      return "Flat";
    case SpvDecorationPatch:
      return "Patch";
    case SpvDecorationCentroid:
      return "Centroid";
    case SpvDecorationSample:
      return "Sample";
    case SpvDecorationInvariant:
      return "Invariant";
    case SpvDecorationRestrict:
      return "Restrict";
    case SpvDecorationAliased:
      return "Aliased";
    case SpvDecorationVolatile:
      return "Volatile";
    case SpvDecorationConstant:
      return "Constant";
    case SpvDecorationCoherent:
      return "Coherent";
    case SpvDecorationNonWritable:
      return "NonWritable";
    case SpvDecorationNonReadable:
      return "NonReadable";
    case SpvDecorationUniform:
      return "Uniform";
    case SpvDecorationSaturatedConversion:
      return "SaturatedConversion";
    case SpvDecorationStream:
      return "Stream";
    case SpvDecorationLocation:
      return "Location";
    case SpvDecorationComponent:
      return "Component";
    case SpvDecorationIndex:
      return "Index";
    case SpvDecorationBinding:
      return "Binding";
    case SpvDecorationDescriptorSet:
      return "DescriptorSet";
    case SpvDecorationOffset:
      return "Offset";
    case SpvDecorationXfbBuffer:
      return "XfbBuffer";
    case SpvDecorationXfbStride:
      return "XfbStride";
    case SpvDecorationFuncParamAttr:
      return "FuncParamAttr";
    case SpvDecorationFPRoundingMode:
      return "FPRoundingMode";
    case SpvDecorationFPFastMathMode:
      return "FPFastMathMode";
    case SpvDecorationLinkageAttributes:
      return "LinkageAttributes";
    case SpvDecorationNoContraction:
      return "NoContraction";
    case SpvDecorationInputAttachmentIndex:
      return "InputAttachmentIndex";
    case SpvDecorationAlignment:
      return "Alignment";
    case SpvDecorationMaxByteOffset:
      return "MaxByteOffset";
    case SpvDecorationAlignmentId:
      return "AlignmentId";
    case SpvDecorationMaxByteOffsetId:
      return "MaxByteOffsetId";
    case SpvDecorationNoSignedWrap:
      return "NoSignedWrap";
    case SpvDecorationNoUnsignedWrap:
      return "NoUnsignedWrap";
    case SpvDecorationExplicitInterpAMD:
      return "ExplicitInterpAMD";
    case SpvDecorationOverrideCoverageNV:
      return "OverrideCoverageNV";
    case SpvDecorationPassthroughNV:
      return "PassthroughNV";
    case SpvDecorationViewportRelativeNV:
      return "ViewportRelativeNV";
    case SpvDecorationSecondaryViewportRelativeNV:
      return "SecondaryViewportRelativeNV";
    case SpvDecorationPerPrimitiveNV:
      return "PerPrimitiveNV";
    case SpvDecorationPerViewNV:
      return "PerViewNV";
    case SpvDecorationPerTaskNV:
      return "PerTaskNV";
    case SpvDecorationPerVertexNV:
      return "PerVertexNV";
    case SpvDecorationNonUniform:
      return "NonUniform";
    case SpvDecorationRestrictPointer:
      return "RestrictPointer";
    case SpvDecorationAliasedPointer:
      return "AliasedPointer";
    case SpvDecorationCounterBuffer:
      return "CounterBuffer";
    case SpvDecorationHlslSemanticGOOGLE:
      return "HlslSemanticGOOGLE";
    default:
      break;
  }
  return "Unknown";
}

// Returns true if the decoration takes ID parameters.
// TODO(dneto): This can be generated from the grammar.
bool DecorationTakesIdParameters(SpvDecoration type) {
  switch (type) {
    case SpvDecorationUniformId:
    case SpvDecorationAlignmentId:
    case SpvDecorationMaxByteOffsetId:
    case SpvDecorationHlslCounterBufferGOOGLE:
      return true;
    default:
      break;
  }
  return false;
}

bool IsMemberDecorationOnly(SpvDecoration dec) {
  switch (dec) {
    case SpvDecorationRowMajor:
    case SpvDecorationColMajor:
    case SpvDecorationMatrixStride:
      // SPIR-V spec bug? Offset is generated on variables when dealing with
      // transform feedback.
      // case SpvDecorationOffset:
      return true;
    default:
      break;
  }
  return false;
}

bool IsNotMemberDecoration(SpvDecoration dec) {
  switch (dec) {
    case SpvDecorationSpecId:
    case SpvDecorationBlock:
    case SpvDecorationBufferBlock:
    case SpvDecorationArrayStride:
    case SpvDecorationGLSLShared:
    case SpvDecorationGLSLPacked:
    case SpvDecorationCPacked:
    // TODO: https://github.com/KhronosGroup/glslang/issues/703:
    // glslang applies Restrict to structure members.
    // case SpvDecorationRestrict:
    case SpvDecorationAliased:
    case SpvDecorationConstant:
    case SpvDecorationUniform:
    case SpvDecorationUniformId:
    case SpvDecorationSaturatedConversion:
    case SpvDecorationIndex:
    case SpvDecorationBinding:
    case SpvDecorationDescriptorSet:
    case SpvDecorationFuncParamAttr:
    case SpvDecorationFPRoundingMode:
    case SpvDecorationFPFastMathMode:
    case SpvDecorationLinkageAttributes:
    case SpvDecorationNoContraction:
    case SpvDecorationInputAttachmentIndex:
    case SpvDecorationAlignment:
    case SpvDecorationMaxByteOffset:
    case SpvDecorationAlignmentId:
    case SpvDecorationMaxByteOffsetId:
    case SpvDecorationNoSignedWrap:
    case SpvDecorationNoUnsignedWrap:
    case SpvDecorationNonUniform:
    case SpvDecorationRestrictPointer:
    case SpvDecorationAliasedPointer:
    case SpvDecorationCounterBuffer:
      return true;
    default:
      break;
  }
  return false;
}

spv_result_t ValidateDecorationTarget(ValidationState_t& _, SpvDecoration dec,
                                      const Instruction* inst,
                                      const Instruction* target) {
  auto fail = [&_, dec, inst, target](uint32_t vuid) -> DiagnosticStream {
    DiagnosticStream ds = std::move(
        _.diag(SPV_ERROR_INVALID_ID, inst)
        << _.VkErrorID(vuid) << LogStringForDecoration(dec)
        << " decoration on target <id> '" << _.getIdName(target->id()) << "' ");
    return ds;
  };
  switch (dec) {
    case SpvDecorationSpecId:
      if (!spvOpcodeIsScalarSpecConstant(target->opcode())) {
        return fail(0) << "must be a scalar specialization constant";
      }
      break;
    case SpvDecorationBlock:
    case SpvDecorationBufferBlock:
    case SpvDecorationGLSLShared:
    case SpvDecorationGLSLPacked:
    case SpvDecorationCPacked:
      if (target->opcode() != SpvOpTypeStruct) {
        return fail(0) << "must be a structure type";
      }
      break;
    case SpvDecorationArrayStride:
      if (target->opcode() != SpvOpTypeArray &&
          target->opcode() != SpvOpTypeRuntimeArray &&
          target->opcode() != SpvOpTypePointer) {
        return fail(0) << "must be an array or pointer type";
      }
      break;
    case SpvDecorationBuiltIn:
      if (target->opcode() != SpvOpVariable &&
          !spvOpcodeIsConstant(target->opcode())) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "BuiltIns can only target variables, structure members or "
                  "constants";
      }
      if (_.HasCapability(SpvCapabilityShader) &&
          inst->GetOperandAs<SpvBuiltIn>(2) == SpvBuiltInWorkgroupSize) {
        if (!spvOpcodeIsConstant(target->opcode())) {
          return fail(0) << "must be a constant for WorkgroupSize";
        }
      } else if (target->opcode() != SpvOpVariable) {
        return fail(0) << "must be a variable";
      }
      break;
    case SpvDecorationNoPerspective:
    case SpvDecorationFlat:
    case SpvDecorationPatch:
    case SpvDecorationCentroid:
    case SpvDecorationSample:
    case SpvDecorationRestrict:
    case SpvDecorationAliased:
    case SpvDecorationVolatile:
    case SpvDecorationCoherent:
    case SpvDecorationNonWritable:
    case SpvDecorationNonReadable:
    case SpvDecorationXfbBuffer:
    case SpvDecorationXfbStride:
    case SpvDecorationComponent:
    case SpvDecorationStream:
    case SpvDecorationRestrictPointer:
    case SpvDecorationAliasedPointer:
      if (target->opcode() != SpvOpVariable &&
          target->opcode() != SpvOpFunctionParameter) {
        return fail(0) << "must be a memory object declaration";
      }
      if (_.GetIdOpcode(target->type_id()) != SpvOpTypePointer) {
        return fail(0) << "must be a pointer type";
      }
      break;
    case SpvDecorationInvariant:
    case SpvDecorationConstant:
    case SpvDecorationLocation:
    case SpvDecorationIndex:
    case SpvDecorationBinding:
    case SpvDecorationDescriptorSet:
    case SpvDecorationInputAttachmentIndex:
      if (target->opcode() != SpvOpVariable) {
        return fail(0) << "must be a variable";
      }
      break;
    default:
      break;
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    // The following were all checked as pointer types above.
    SpvStorageClass sc = SpvStorageClassUniform;
    const auto type = _.FindDef(target->type_id());
    if (type && type->operands().size() > 2) {
      sc = type->GetOperandAs<SpvStorageClass>(1);
    }
    switch (dec) {
      case SpvDecorationLocation:
      case SpvDecorationComponent:
        // Location is used for input, output and ray tracing stages.
        if (sc != SpvStorageClassInput && sc != SpvStorageClassOutput &&
            sc != SpvStorageClassRayPayloadKHR &&
            sc != SpvStorageClassIncomingRayPayloadKHR &&
            sc != SpvStorageClassHitAttributeKHR &&
            sc != SpvStorageClassCallableDataKHR &&
            sc != SpvStorageClassIncomingCallableDataKHR &&
            sc != SpvStorageClassShaderRecordBufferKHR) {
          return _.diag(SPV_ERROR_INVALID_ID, target)
                 << _.VkErrorID(6672) << LogStringForDecoration(dec)
                 << " decoration must not be applied to this storage class";
        }
        break;
      case SpvDecorationIndex:
        // Langauge from SPIR-V definition of Index
        if (sc != SpvStorageClassOutput) {
          return fail(0) << "must be in the Output storage class";
        }
        break;
      case SpvDecorationBinding:
      case SpvDecorationDescriptorSet:
        if (sc != SpvStorageClassStorageBuffer &&
            sc != SpvStorageClassUniform &&
            sc != SpvStorageClassUniformConstant) {
          return fail(6491) << "must be in the StorageBuffer, Uniform, or "
                               "UniformConstant storage class";
        }
        break;
      case SpvDecorationInputAttachmentIndex:
        if (sc != SpvStorageClassUniformConstant) {
          return fail(6678) << "must be in the UniformConstant storage class";
        }
        break;
      case SpvDecorationFlat:
      case SpvDecorationNoPerspective:
      case SpvDecorationCentroid:
      case SpvDecorationSample:
        if (sc != SpvStorageClassInput && sc != SpvStorageClassOutput) {
          return fail(4670) << "storage class must be Input or Output";
        }
        break;
      default:
        break;
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateDecorate(ValidationState_t& _, const Instruction* inst) {
  const auto decoration = inst->GetOperandAs<SpvDecoration>(1);
  const auto target_id = inst->GetOperandAs<uint32_t>(0);
  const auto target = _.FindDef(target_id);
  if (!target) {
    return _.diag(SPV_ERROR_INVALID_ID, inst) << "target is not defined";
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    if ((decoration == SpvDecorationGLSLShared) ||
        (decoration == SpvDecorationGLSLPacked)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << _.VkErrorID(4669) << "OpDecorate decoration '"
             << LogStringForDecoration(decoration)
             << "' is not valid for the Vulkan execution environment.";
    }
  }

  if (DecorationTakesIdParameters(decoration)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Decorations taking ID parameters may not be used with "
              "OpDecorateId";
  }

  if (target->opcode() != SpvOpDecorationGroup) {
    if (IsMemberDecorationOnly(decoration)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << LogStringForDecoration(decoration)
             << " can only be applied to structure members";
    }

    if (auto error = ValidateDecorationTarget(_, decoration, inst, target)) {
      return error;
    }
  }

  // TODO: Add validations for all decorations.
  return SPV_SUCCESS;
}

spv_result_t ValidateDecorateId(ValidationState_t& _, const Instruction* inst) {
  const auto decoration = inst->GetOperandAs<SpvDecoration>(1);
  if (!DecorationTakesIdParameters(decoration)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Decorations that don't take ID parameters may not be used with "
              "OpDecorateId";
  }

  // No member decorations take id parameters, so we don't bother checking if
  // we are using a member only decoration here.

  // TODO: Add validations for these decorations.
  // UniformId is covered elsewhere.
  return SPV_SUCCESS;
}

spv_result_t ValidateMemberDecorate(ValidationState_t& _,
                                    const Instruction* inst) {
  const auto struct_type_id = inst->GetOperandAs<uint32_t>(0);
  const auto struct_type = _.FindDef(struct_type_id);
  if (!struct_type || SpvOpTypeStruct != struct_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpMemberDecorate Structure type <id> '"
           << _.getIdName(struct_type_id) << "' is not a struct type.";
  }
  const auto member = inst->GetOperandAs<uint32_t>(1);
  const auto member_count =
      static_cast<uint32_t>(struct_type->words().size() - 2);
  if (member_count <= member) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Index " << member
           << " provided in OpMemberDecorate for struct <id> "
           << _.getIdName(struct_type_id)
           << " is out of bounds. The structure has " << member_count
           << " members. Largest valid index is " << member_count - 1 << ".";
  }

  const auto decoration = inst->GetOperandAs<SpvDecoration>(2);
  if (IsNotMemberDecoration(decoration)) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << LogStringForDecoration(decoration)
           << " cannot be applied to structure members";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateDecorationGroup(ValidationState_t& _,
                                     const Instruction* inst) {
  const auto decoration_group_id = inst->GetOperandAs<uint32_t>(0);
  const auto decoration_group = _.FindDef(decoration_group_id);
  for (auto pair : decoration_group->uses()) {
    auto use = pair.first;
    if (use->opcode() != SpvOpDecorate && use->opcode() != SpvOpGroupDecorate &&
        use->opcode() != SpvOpGroupMemberDecorate &&
        use->opcode() != SpvOpName && use->opcode() != SpvOpDecorateId &&
        !use->IsNonSemantic()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Result id of OpDecorationGroup can only "
             << "be targeted by OpName, OpGroupDecorate, "
             << "OpDecorate, OpDecorateId, and OpGroupMemberDecorate";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupDecorate(ValidationState_t& _,
                                   const Instruction* inst) {
  const auto decoration_group_id = inst->GetOperandAs<uint32_t>(0);
  auto decoration_group = _.FindDef(decoration_group_id);
  if (!decoration_group || SpvOpDecorationGroup != decoration_group->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpGroupDecorate Decoration group <id> '"
           << _.getIdName(decoration_group_id)
           << "' is not a decoration group.";
  }
  for (unsigned i = 1; i < inst->operands().size(); ++i) {
    auto target_id = inst->GetOperandAs<uint32_t>(i);
    auto target = _.FindDef(target_id);
    if (!target || target->opcode() == SpvOpDecorationGroup) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpGroupDecorate may not target OpDecorationGroup <id> '"
             << _.getIdName(target_id) << "'";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupMemberDecorate(ValidationState_t& _,
                                         const Instruction* inst) {
  const auto decoration_group_id = inst->GetOperandAs<uint32_t>(0);
  const auto decoration_group = _.FindDef(decoration_group_id);
  if (!decoration_group || SpvOpDecorationGroup != decoration_group->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpGroupMemberDecorate Decoration group <id> '"
           << _.getIdName(decoration_group_id)
           << "' is not a decoration group.";
  }
  // Grammar checks ensures that the number of arguments to this instruction
  // is an odd number: 1 decoration group + (id,literal) pairs.
  for (size_t i = 1; i + 1 < inst->operands().size(); i += 2) {
    const uint32_t struct_id = inst->GetOperandAs<uint32_t>(i);
    const uint32_t index = inst->GetOperandAs<uint32_t>(i + 1);
    auto struct_instr = _.FindDef(struct_id);
    if (!struct_instr || SpvOpTypeStruct != struct_instr->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpGroupMemberDecorate Structure type <id> '"
             << _.getIdName(struct_id) << "' is not a struct type.";
    }
    const uint32_t num_struct_members =
        static_cast<uint32_t>(struct_instr->words().size() - 2);
    if (index >= num_struct_members) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Index " << index
             << " provided in OpGroupMemberDecorate for struct <id> "
             << _.getIdName(struct_id)
             << " is out of bounds. The structure has " << num_struct_members
             << " members. Largest valid index is " << num_struct_members - 1
             << ".";
    }
  }
  return SPV_SUCCESS;
}

// Registers necessary decoration(s) for the appropriate IDs based on the
// instruction.
spv_result_t RegisterDecorations(ValidationState_t& _,
                                 const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpDecorate:
    case SpvOpDecorateId: {
      const uint32_t target_id = inst->word(1);
      const SpvDecoration dec_type = static_cast<SpvDecoration>(inst->word(2));
      std::vector<uint32_t> dec_params;
      if (inst->words().size() > 3) {
        dec_params.insert(dec_params.end(), inst->words().begin() + 3,
                          inst->words().end());
      }
      _.RegisterDecorationForId(target_id, Decoration(dec_type, dec_params));
      break;
    }
    case SpvOpMemberDecorate: {
      const uint32_t struct_id = inst->word(1);
      const uint32_t index = inst->word(2);
      const SpvDecoration dec_type = static_cast<SpvDecoration>(inst->word(3));
      std::vector<uint32_t> dec_params;
      if (inst->words().size() > 4) {
        dec_params.insert(dec_params.end(), inst->words().begin() + 4,
                          inst->words().end());
      }
      _.RegisterDecorationForId(struct_id,
                                Decoration(dec_type, dec_params, index));
      break;
    }
    case SpvOpDecorationGroup: {
      // We don't need to do anything right now. Assigning decorations to groups
      // will be taken care of via OpGroupDecorate.
      break;
    }
    case SpvOpGroupDecorate: {
      // Word 1 is the group <id>. All subsequent words are target <id>s that
      // are going to be decorated with the decorations.
      const uint32_t decoration_group_id = inst->word(1);
      std::vector<Decoration>& group_decorations =
          _.id_decorations(decoration_group_id);
      for (size_t i = 2; i < inst->words().size(); ++i) {
        const uint32_t target_id = inst->word(i);
        _.RegisterDecorationsForId(target_id, group_decorations.begin(),
                                   group_decorations.end());
      }
      break;
    }
    case SpvOpGroupMemberDecorate: {
      // Word 1 is the Decoration Group <id> followed by (struct<id>,literal)
      // pairs. All decorations of the group should be applied to all the struct
      // members that are specified in the instructions.
      const uint32_t decoration_group_id = inst->word(1);
      std::vector<Decoration>& group_decorations =
          _.id_decorations(decoration_group_id);
      // Grammar checks ensures that the number of arguments to this instruction
      // is an odd number: 1 decoration group + (id,literal) pairs.
      for (size_t i = 2; i + 1 < inst->words().size(); i = i + 2) {
        const uint32_t struct_id = inst->word(i);
        const uint32_t index = inst->word(i + 1);
        // ID validation phase ensures this is in fact a struct instruction and
        // that the index is not out of bound.
        _.RegisterDecorationsForStructMember(struct_id, index,
                                             group_decorations.begin(),
                                             group_decorations.end());
      }
      break;
    }
    default:
      break;
  }
  return SPV_SUCCESS;
}

}  // namespace

spv_result_t AnnotationPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpDecorate:
      if (auto error = ValidateDecorate(_, inst)) return error;
      break;
    case SpvOpDecorateId:
      if (auto error = ValidateDecorateId(_, inst)) return error;
      break;
    // TODO(dneto): SpvOpDecorateStringGOOGLE
    // See https://github.com/KhronosGroup/SPIRV-Tools/issues/2253
    case SpvOpMemberDecorate:
      if (auto error = ValidateMemberDecorate(_, inst)) return error;
      break;
    case SpvOpDecorationGroup:
      if (auto error = ValidateDecorationGroup(_, inst)) return error;
      break;
    case SpvOpGroupDecorate:
      if (auto error = ValidateGroupDecorate(_, inst)) return error;
      break;
    case SpvOpGroupMemberDecorate:
      if (auto error = ValidateGroupMemberDecorate(_, inst)) return error;
      break;
    default:
      break;
  }

  // In order to validate decoration rules, we need to know all the decorations
  // that are applied to any given <id>.
  RegisterDecorations(_, inst);

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
