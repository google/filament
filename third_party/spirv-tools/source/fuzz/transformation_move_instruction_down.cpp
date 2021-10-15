// Copyright (c) 2020 Vasyl Teliman
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

#include "source/fuzz/transformation_move_instruction_down.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "spirv/unified1/GLSL.std.450.h"

namespace spvtools {
namespace fuzz {
namespace {

const char* const kExtensionSetName = "GLSL.std.450";

std::string GetExtensionSet(opt::IRContext* ir_context,
                            const opt::Instruction& op_ext_inst) {
  assert(op_ext_inst.opcode() == SpvOpExtInst && "Wrong opcode");

  const auto* ext_inst_import = ir_context->get_def_use_mgr()->GetDef(
      op_ext_inst.GetSingleWordInOperand(0));
  assert(ext_inst_import && "Extension set is not imported");

  return ext_inst_import->GetInOperand(0).AsString();
}

}  // namespace

TransformationMoveInstructionDown::TransformationMoveInstructionDown(
    protobufs::TransformationMoveInstructionDown message)
    : message_(std::move(message)) {}

TransformationMoveInstructionDown::TransformationMoveInstructionDown(
    const protobufs::InstructionDescriptor& instruction) {
  *message_.mutable_instruction() = instruction;
}

bool TransformationMoveInstructionDown::IsApplicable(
    opt::IRContext* ir_context,
    const TransformationContext& transformation_context) const {
  // |instruction| must be valid.
  auto* inst = FindInstruction(message_.instruction(), ir_context);
  if (!inst) {
    return false;
  }

  // Instruction's opcode must be supported by this transformation.
  if (!IsInstructionSupported(ir_context, *inst)) {
    return false;
  }

  auto* inst_block = ir_context->get_instr_block(inst);
  assert(inst_block &&
         "Global instructions and function parameters are not supported");

  auto inst_it = fuzzerutil::GetIteratorForInstruction(inst_block, inst);
  assert(inst_it != inst_block->end() &&
         "Can't get an iterator for the instruction");

  // |instruction| can't be the last instruction in the block.
  auto successor_it = ++inst_it;
  if (successor_it == inst_block->end()) {
    return false;
  }

  // We don't risk swapping a memory instruction with an unsupported one.
  if (!IsSimpleInstruction(ir_context, *inst) &&
      !IsInstructionSupported(ir_context, *successor_it)) {
    return false;
  }

  // It must be safe to swap the instructions without changing the semantics of
  // the module.
  if (IsInstructionSupported(ir_context, *successor_it) &&
      !CanSafelySwapInstructions(ir_context, *inst, *successor_it,
                                 *transformation_context.GetFactManager())) {
    return false;
  }

  // Check that we can insert |instruction| after |inst_it|.
  auto successors_successor_it = ++inst_it;
  if (successors_successor_it == inst_block->end() ||
      !fuzzerutil::CanInsertOpcodeBeforeInstruction(inst->opcode(),
                                                    successors_successor_it)) {
    return false;
  }

  // Check that |instruction|'s successor doesn't depend on the |instruction|.
  if (inst->result_id()) {
    for (uint32_t i = 0; i < successor_it->NumInOperands(); ++i) {
      const auto& operand = successor_it->GetInOperand(i);
      if (spvIsInIdType(operand.type) &&
          operand.words[0] == inst->result_id()) {
        return false;
      }
    }
  }

  return true;
}

void TransformationMoveInstructionDown::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  auto* inst = FindInstruction(message_.instruction(), ir_context);
  assert(inst &&
         "The instruction should've been validated in the IsApplicable");

  auto inst_it = fuzzerutil::GetIteratorForInstruction(
      ir_context->get_instr_block(inst), inst);

  // Move the instruction down in the block.
  inst->InsertAfter(&*++inst_it);

  ir_context->InvalidateAnalyses(opt::IRContext::kAnalysisNone);
}

protobufs::Transformation TransformationMoveInstructionDown::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_move_instruction_down() = message_;
  return result;
}

bool TransformationMoveInstructionDown::IsInstructionSupported(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/3605):
  //  Add support for more instructions here.
  return IsSimpleInstruction(ir_context, inst) ||
         IsMemoryInstruction(ir_context, inst) || IsBarrierInstruction(inst);
}

bool TransformationMoveInstructionDown::IsSimpleInstruction(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  switch (inst.opcode()) {
    case SpvOpNop:
    case SpvOpUndef:
    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
      // OpAccessChain and OpInBoundsAccessChain are considered simple
      // instructions since they result in a pointer to the object in memory,
      // not the object itself.
    case SpvOpVectorExtractDynamic:
    case SpvOpVectorInsertDynamic:
    case SpvOpVectorShuffle:
    case SpvOpCompositeConstruct:
    case SpvOpCompositeExtract:
    case SpvOpCompositeInsert:
    case SpvOpCopyObject:
    case SpvOpTranspose:
    case SpvOpConvertFToU:
    case SpvOpConvertFToS:
    case SpvOpConvertSToF:
    case SpvOpConvertUToF:
    case SpvOpUConvert:
    case SpvOpSConvert:
    case SpvOpFConvert:
    case SpvOpQuantizeToF16:
    case SpvOpSatConvertSToU:
    case SpvOpSatConvertUToS:
    case SpvOpBitcast:
    case SpvOpSNegate:
    case SpvOpFNegate:
    case SpvOpIAdd:
    case SpvOpFAdd:
    case SpvOpISub:
    case SpvOpFSub:
    case SpvOpIMul:
    case SpvOpFMul:
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpFDiv:
    case SpvOpUMod:
    case SpvOpSRem:
    case SpvOpSMod:
    case SpvOpFRem:
    case SpvOpFMod:
    case SpvOpVectorTimesScalar:
    case SpvOpMatrixTimesScalar:
    case SpvOpVectorTimesMatrix:
    case SpvOpMatrixTimesVector:
    case SpvOpMatrixTimesMatrix:
    case SpvOpOuterProduct:
    case SpvOpDot:
    case SpvOpIAddCarry:
    case SpvOpISubBorrow:
    case SpvOpUMulExtended:
    case SpvOpSMulExtended:
    case SpvOpAny:
    case SpvOpAll:
    case SpvOpIsNan:
    case SpvOpIsInf:
    case SpvOpIsFinite:
    case SpvOpIsNormal:
    case SpvOpSignBitSet:
    case SpvOpLessOrGreater:
    case SpvOpOrdered:
    case SpvOpUnordered:
    case SpvOpLogicalEqual:
    case SpvOpLogicalNotEqual:
    case SpvOpLogicalOr:
    case SpvOpLogicalAnd:
    case SpvOpLogicalNot:
    case SpvOpSelect:
    case SpvOpIEqual:
    case SpvOpINotEqual:
    case SpvOpUGreaterThan:
    case SpvOpSGreaterThan:
    case SpvOpUGreaterThanEqual:
    case SpvOpSGreaterThanEqual:
    case SpvOpULessThan:
    case SpvOpSLessThan:
    case SpvOpULessThanEqual:
    case SpvOpSLessThanEqual:
    case SpvOpFOrdEqual:
    case SpvOpFUnordEqual:
    case SpvOpFOrdNotEqual:
    case SpvOpFUnordNotEqual:
    case SpvOpFOrdLessThan:
    case SpvOpFUnordLessThan:
    case SpvOpFOrdGreaterThan:
    case SpvOpFUnordGreaterThan:
    case SpvOpFOrdLessThanEqual:
    case SpvOpFUnordLessThanEqual:
    case SpvOpFOrdGreaterThanEqual:
    case SpvOpFUnordGreaterThanEqual:
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
    case SpvOpShiftLeftLogical:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
    case SpvOpBitwiseAnd:
    case SpvOpNot:
    case SpvOpBitFieldInsert:
    case SpvOpBitFieldSExtract:
    case SpvOpBitFieldUExtract:
    case SpvOpBitReverse:
    case SpvOpBitCount:
    case SpvOpCopyLogical:
      return true;
    case SpvOpExtInst: {
      const auto* ext_inst_import =
          ir_context->get_def_use_mgr()->GetDef(inst.GetSingleWordInOperand(0));

      if (ext_inst_import->GetInOperand(0).AsString() != kExtensionSetName) {
        return false;
      }

      switch (static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1))) {
        case GLSLstd450Round:
        case GLSLstd450RoundEven:
        case GLSLstd450Trunc:
        case GLSLstd450FAbs:
        case GLSLstd450SAbs:
        case GLSLstd450FSign:
        case GLSLstd450SSign:
        case GLSLstd450Floor:
        case GLSLstd450Ceil:
        case GLSLstd450Fract:
        case GLSLstd450Radians:
        case GLSLstd450Degrees:
        case GLSLstd450Sin:
        case GLSLstd450Cos:
        case GLSLstd450Tan:
        case GLSLstd450Asin:
        case GLSLstd450Acos:
        case GLSLstd450Atan:
        case GLSLstd450Sinh:
        case GLSLstd450Cosh:
        case GLSLstd450Tanh:
        case GLSLstd450Asinh:
        case GLSLstd450Acosh:
        case GLSLstd450Atanh:
        case GLSLstd450Atan2:
        case GLSLstd450Pow:
        case GLSLstd450Exp:
        case GLSLstd450Log:
        case GLSLstd450Exp2:
        case GLSLstd450Log2:
        case GLSLstd450Sqrt:
        case GLSLstd450InverseSqrt:
        case GLSLstd450Determinant:
        case GLSLstd450MatrixInverse:
        case GLSLstd450ModfStruct:
        case GLSLstd450FMin:
        case GLSLstd450UMin:
        case GLSLstd450SMin:
        case GLSLstd450FMax:
        case GLSLstd450UMax:
        case GLSLstd450SMax:
        case GLSLstd450FClamp:
        case GLSLstd450UClamp:
        case GLSLstd450SClamp:
        case GLSLstd450FMix:
        case GLSLstd450IMix:
        case GLSLstd450Step:
        case GLSLstd450SmoothStep:
        case GLSLstd450Fma:
        case GLSLstd450FrexpStruct:
        case GLSLstd450Ldexp:
        case GLSLstd450PackSnorm4x8:
        case GLSLstd450PackUnorm4x8:
        case GLSLstd450PackSnorm2x16:
        case GLSLstd450PackUnorm2x16:
        case GLSLstd450PackHalf2x16:
        case GLSLstd450PackDouble2x32:
        case GLSLstd450UnpackSnorm2x16:
        case GLSLstd450UnpackUnorm2x16:
        case GLSLstd450UnpackHalf2x16:
        case GLSLstd450UnpackSnorm4x8:
        case GLSLstd450UnpackUnorm4x8:
        case GLSLstd450UnpackDouble2x32:
        case GLSLstd450Length:
        case GLSLstd450Distance:
        case GLSLstd450Cross:
        case GLSLstd450Normalize:
        case GLSLstd450FaceForward:
        case GLSLstd450Reflect:
        case GLSLstd450Refract:
        case GLSLstd450FindILsb:
        case GLSLstd450FindSMsb:
        case GLSLstd450FindUMsb:
        case GLSLstd450NMin:
        case GLSLstd450NMax:
        case GLSLstd450NClamp:
          return true;
        default:
          return false;
      }
    }
    default:
      return false;
  }
}

bool TransformationMoveInstructionDown::IsMemoryReadInstruction(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  switch (inst.opcode()) {
      // Some simple instructions.
    case SpvOpLoad:
    case SpvOpCopyMemory:
      // Image instructions.
    case SpvOpImageSampleImplicitLod:
    case SpvOpImageSampleExplicitLod:
    case SpvOpImageSampleDrefImplicitLod:
    case SpvOpImageSampleDrefExplicitLod:
    case SpvOpImageSampleProjImplicitLod:
    case SpvOpImageSampleProjExplicitLod:
    case SpvOpImageSampleProjDrefImplicitLod:
    case SpvOpImageSampleProjDrefExplicitLod:
    case SpvOpImageFetch:
    case SpvOpImageGather:
    case SpvOpImageDrefGather:
    case SpvOpImageRead:
    case SpvOpImageSparseSampleImplicitLod:
    case SpvOpImageSparseSampleExplicitLod:
    case SpvOpImageSparseSampleDrefImplicitLod:
    case SpvOpImageSparseSampleDrefExplicitLod:
    case SpvOpImageSparseSampleProjImplicitLod:
    case SpvOpImageSparseSampleProjExplicitLod:
    case SpvOpImageSparseSampleProjDrefImplicitLod:
    case SpvOpImageSparseSampleProjDrefExplicitLod:
    case SpvOpImageSparseFetch:
    case SpvOpImageSparseGather:
    case SpvOpImageSparseDrefGather:
    case SpvOpImageSparseRead:
      // Atomic instructions.
    case SpvOpAtomicLoad:
    case SpvOpAtomicExchange:
    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
    case SpvOpAtomicFlagTestAndSet:
      return true;
      // Extensions.
    case SpvOpExtInst: {
      if (GetExtensionSet(ir_context, inst) != kExtensionSetName) {
        return false;
      }

      switch (static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1))) {
        case GLSLstd450InterpolateAtCentroid:
        case GLSLstd450InterpolateAtOffset:
        case GLSLstd450InterpolateAtSample:
          return true;
        default:
          return false;
      }
    }
    default:
      return false;
  }
}

uint32_t TransformationMoveInstructionDown::GetMemoryReadTarget(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  (void)ir_context;  // |ir_context| is only used in assertions.
  assert(IsMemoryReadInstruction(ir_context, inst) &&
         "|inst| is not a memory read instruction");

  switch (inst.opcode()) {
      // Simple instructions.
    case SpvOpLoad:
      // Image instructions.
    case SpvOpImageSampleImplicitLod:
    case SpvOpImageSampleExplicitLod:
    case SpvOpImageSampleDrefImplicitLod:
    case SpvOpImageSampleDrefExplicitLod:
    case SpvOpImageSampleProjImplicitLod:
    case SpvOpImageSampleProjExplicitLod:
    case SpvOpImageSampleProjDrefImplicitLod:
    case SpvOpImageSampleProjDrefExplicitLod:
    case SpvOpImageFetch:
    case SpvOpImageGather:
    case SpvOpImageDrefGather:
    case SpvOpImageRead:
    case SpvOpImageSparseSampleImplicitLod:
    case SpvOpImageSparseSampleExplicitLod:
    case SpvOpImageSparseSampleDrefImplicitLod:
    case SpvOpImageSparseSampleDrefExplicitLod:
    case SpvOpImageSparseSampleProjImplicitLod:
    case SpvOpImageSparseSampleProjExplicitLod:
    case SpvOpImageSparseSampleProjDrefImplicitLod:
    case SpvOpImageSparseSampleProjDrefExplicitLod:
    case SpvOpImageSparseFetch:
    case SpvOpImageSparseGather:
    case SpvOpImageSparseDrefGather:
    case SpvOpImageSparseRead:
      // Atomic instructions.
    case SpvOpAtomicLoad:
    case SpvOpAtomicExchange:
    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
    case SpvOpAtomicFlagTestAndSet:
      return inst.GetSingleWordInOperand(0);
    case SpvOpCopyMemory:
      return inst.GetSingleWordInOperand(1);
    case SpvOpExtInst: {
      assert(GetExtensionSet(ir_context, inst) == kExtensionSetName &&
             "Extension set is not supported");

      switch (static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1))) {
        case GLSLstd450InterpolateAtCentroid:
        case GLSLstd450InterpolateAtOffset:
        case GLSLstd450InterpolateAtSample:
          return inst.GetSingleWordInOperand(2);
        default:
          // This assertion will fail if not all memory read extension
          // instructions are handled in the switch.
          assert(false && "Not all memory opcodes are handled");
          return 0;
      }
    }
    default:
      // This assertion will fail if not all memory read opcodes are handled in
      // the switch.
      assert(false && "Not all memory opcodes are handled");
      return 0;
  }
}

bool TransformationMoveInstructionDown::IsMemoryWriteInstruction(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  switch (inst.opcode()) {
      // Simple Instructions.
    case SpvOpStore:
    case SpvOpCopyMemory:
      // Image instructions.
    case SpvOpImageWrite:
      // Atomic instructions.
    case SpvOpAtomicStore:
    case SpvOpAtomicExchange:
    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
    case SpvOpAtomicFlagTestAndSet:
    case SpvOpAtomicFlagClear:
      return true;
      // Extensions.
    case SpvOpExtInst: {
      if (GetExtensionSet(ir_context, inst) != kExtensionSetName) {
        return false;
      }

      auto extension = static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1));
      return extension == GLSLstd450Modf || extension == GLSLstd450Frexp;
    }
    default:
      return false;
  }
}

uint32_t TransformationMoveInstructionDown::GetMemoryWriteTarget(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  (void)ir_context;  // |ir_context| is only used in assertions.
  assert(IsMemoryWriteInstruction(ir_context, inst) &&
         "|inst| is not a memory write instruction");

  switch (inst.opcode()) {
    case SpvOpStore:
    case SpvOpCopyMemory:
    case SpvOpImageWrite:
    case SpvOpAtomicStore:
    case SpvOpAtomicExchange:
    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
    case SpvOpAtomicFlagTestAndSet:
    case SpvOpAtomicFlagClear:
      return inst.GetSingleWordInOperand(0);
    case SpvOpExtInst: {
      assert(GetExtensionSet(ir_context, inst) == kExtensionSetName &&
             "Extension set is not supported");

      switch (static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1))) {
        case GLSLstd450Modf:
        case GLSLstd450Frexp:
          return inst.GetSingleWordInOperand(3);
        default:
          // This assertion will fail if not all memory write extension
          // instructions are handled in the switch.
          assert(false && "Not all opcodes are handled");
          return 0;
      }
    }
    default:
      // This assertion will fail if not all memory write opcodes are handled in
      // the switch.
      assert(false && "Not all opcodes are handled");
      return 0;
  }
}

bool TransformationMoveInstructionDown::IsMemoryInstruction(
    opt::IRContext* ir_context, const opt::Instruction& inst) {
  return IsMemoryReadInstruction(ir_context, inst) ||
         IsMemoryWriteInstruction(ir_context, inst);
}

bool TransformationMoveInstructionDown::IsBarrierInstruction(
    const opt::Instruction& inst) {
  switch (inst.opcode()) {
    case SpvOpMemoryBarrier:
    case SpvOpControlBarrier:
    case SpvOpMemoryNamedBarrier:
      return true;
    default:
      return false;
  }
}

bool TransformationMoveInstructionDown::CanSafelySwapInstructions(
    opt::IRContext* ir_context, const opt::Instruction& a,
    const opt::Instruction& b, const FactManager& fact_manager) {
  assert(IsInstructionSupported(ir_context, a) &&
         IsInstructionSupported(ir_context, b) &&
         "Both opcodes must be supported");

  // One of opcodes is simple - we can swap them without any side-effects.
  if (IsSimpleInstruction(ir_context, a) ||
      IsSimpleInstruction(ir_context, b)) {
    return true;
  }

  // Both parameters are either memory instruction or barriers.

  // One of the opcodes is a barrier - can't swap them.
  if (IsBarrierInstruction(a) || IsBarrierInstruction(b)) {
    return false;
  }

  // Both parameters are memory instructions.

  // Both parameters only read from memory - it's OK to swap them.
  if (!IsMemoryWriteInstruction(ir_context, a) &&
      !IsMemoryWriteInstruction(ir_context, b)) {
    return true;
  }

  // At least one of parameters is a memory read instruction.

  // In theory, we can swap two memory instructions, one of which reads
  // from the memory, if the read target (the pointer the memory is read from)
  // and the write target (the memory is written into):
  // - point to different memory regions
  // - point to the same region with irrelevant value
  // - point to the same region and the region is not used anymore.
  //
  // However, we can't currently determine if two pointers point to two
  // different memory regions. That being said, if two pointers are not
  // synonymous, they still might point to the same memory region. For example:
  //   %1 = OpVariable ...
  //   %2 = OpAccessChain %1 0
  //   %3 = OpAccessChain %1 0
  // In this pseudo-code, %2 and %3 are not synonymous but point to the same
  // memory location. This implies that we can't determine if some memory
  // location is not used in the block.
  //
  // With this in mind, consider two cases (we will build a table for each one):
  // - one instruction only reads from memory, the other one only writes to it.
  //   S - both point to the same memory region.
  //   D - both point to different memory regions.
  //   0, 1, 2 - neither, one of or both of the memory regions are irrelevant.
  //   |-| - can't swap; |+| - can swap.
  //     | 0 | 1 | 2 |
  //   S : -   +   +
  //   D : +   +   +
  // - both instructions write to memory. Notation is the same.
  //     | 0 | 1 | 2 |
  //   S : *   +   +
  //   D : +   +   +
  //   * - we can swap two instructions that write into the same non-irrelevant
  //   memory region if the written value is the same.
  //
  // Note that we can't always distinguish between S and D. Also note that
  // in case of S, if one of the instructions is marked with
  // PointeeValueIsIrrelevant, then the pointee of the other one is irrelevant
  // as well even if the instruction is not marked with that fact.
  //
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/3723):
  //  This procedure can be improved when we can determine if two pointers point
  //  to different memory regions.

  // From now on we will denote an instruction that:
  // - only reads from memory - R
  // - only writes into memory - W
  // - reads and writes - RW
  //
  // Both |a| and |b| can be either W or RW at this point. Additionally, at most
  // one of them can be R. The procedure below checks all possible combinations
  // of R, W and RW according to the tables above. We conservatively assume that
  // both |a| and |b| point to the same memory region.

  auto memory_is_irrelevant = [ir_context, &fact_manager](uint32_t id) {
    const auto* inst = ir_context->get_def_use_mgr()->GetDef(id);
    if (!inst->type_id()) {
      return false;
    }

    const auto* type = ir_context->get_type_mgr()->GetType(inst->type_id());
    assert(type && "|id| has invalid type");

    if (!type->AsPointer()) {
      return false;
    }

    return fact_manager.PointeeValueIsIrrelevant(id);
  };

  if (IsMemoryWriteInstruction(ir_context, a) &&
      IsMemoryWriteInstruction(ir_context, b) &&
      (memory_is_irrelevant(GetMemoryWriteTarget(ir_context, a)) ||
       memory_is_irrelevant(GetMemoryWriteTarget(ir_context, b)))) {
    // We ignore the case when the written value is the same. This is because
    // the written value might not be equal to any of the instruction's
    // operands.
    return true;
  }

  if (IsMemoryReadInstruction(ir_context, a) &&
      IsMemoryWriteInstruction(ir_context, b) &&
      !memory_is_irrelevant(GetMemoryReadTarget(ir_context, a)) &&
      !memory_is_irrelevant(GetMemoryWriteTarget(ir_context, b))) {
    return false;
  }

  if (IsMemoryWriteInstruction(ir_context, a) &&
      IsMemoryReadInstruction(ir_context, b) &&
      !memory_is_irrelevant(GetMemoryWriteTarget(ir_context, a)) &&
      !memory_is_irrelevant(GetMemoryReadTarget(ir_context, b))) {
    return false;
  }

  return IsMemoryReadInstruction(ir_context, a) ||
         IsMemoryReadInstruction(ir_context, b);
}

std::unordered_set<uint32_t> TransformationMoveInstructionDown::GetFreshIds()
    const {
  return std::unordered_set<uint32_t>();
}

}  // namespace fuzz
}  // namespace spvtools
