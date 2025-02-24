// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SpirvShader.hpp"

#include <spirv/unified1/spirv.hpp>

namespace sw {

void Spirv::EvalSpecConstantOp(InsnIterator insn)
{
	auto opcode = static_cast<spv::Op>(insn.word(3));

	switch(opcode)
	{
	case spv::OpIAdd:
	case spv::OpISub:
	case spv::OpIMul:
	case spv::OpUDiv:
	case spv::OpSDiv:
	case spv::OpUMod:
	case spv::OpSMod:
	case spv::OpSRem:
	case spv::OpShiftRightLogical:
	case spv::OpShiftRightArithmetic:
	case spv::OpShiftLeftLogical:
	case spv::OpBitwiseOr:
	case spv::OpLogicalOr:
	case spv::OpBitwiseAnd:
	case spv::OpLogicalAnd:
	case spv::OpBitwiseXor:
	case spv::OpLogicalEqual:
	case spv::OpIEqual:
	case spv::OpLogicalNotEqual:
	case spv::OpINotEqual:
	case spv::OpULessThan:
	case spv::OpSLessThan:
	case spv::OpUGreaterThan:
	case spv::OpSGreaterThan:
	case spv::OpULessThanEqual:
	case spv::OpSLessThanEqual:
	case spv::OpUGreaterThanEqual:
	case spv::OpSGreaterThanEqual:
		EvalSpecConstantBinaryOp(insn);
		break;

	case spv::OpSConvert:
	case spv::OpFConvert:
	case spv::OpUConvert:
	case spv::OpSNegate:
	case spv::OpNot:
	case spv::OpLogicalNot:
	case spv::OpQuantizeToF16:
		EvalSpecConstantUnaryOp(insn);
		break;

	case spv::OpSelect:
		{
			auto &result = CreateConstant(insn);
			const auto &cond = getObject(insn.word(4));
			auto condIsScalar = (getType(cond).componentCount == 1);
			const auto &left = getObject(insn.word(5));
			const auto &right = getObject(insn.word(6));

			for(auto i = 0u; i < getType(result).componentCount; i++)
			{
				auto sel = cond.constantValue[condIsScalar ? 0 : i];
				result.constantValue[i] = sel ? left.constantValue[i] : right.constantValue[i];
			}
			break;
		}

	case spv::OpCompositeExtract:
		{
			auto &result = CreateConstant(insn);
			const auto &compositeObject = getObject(insn.word(4));
			auto firstComponent = WalkLiteralAccessChain(compositeObject.typeId(), Span(insn, 5, insn.wordCount() - 5));

			for(auto i = 0u; i < getType(result).componentCount; i++)
			{
				result.constantValue[i] = compositeObject.constantValue[firstComponent + i];
			}
			break;
		}

	case spv::OpCompositeInsert:
		{
			auto &result = CreateConstant(insn);
			const auto &newPart = getObject(insn.word(4));
			const auto &oldObject = getObject(insn.word(5));
			auto firstNewComponent = WalkLiteralAccessChain(result.typeId(), Span(insn, 6, insn.wordCount() - 6));

			// old components before
			for(auto i = 0u; i < firstNewComponent; i++)
			{
				result.constantValue[i] = oldObject.constantValue[i];
			}
			// new part
			for(auto i = 0u; i < getType(newPart).componentCount; i++)
			{
				result.constantValue[firstNewComponent + i] = newPart.constantValue[i];
			}
			// old components after
			for(auto i = firstNewComponent + getType(newPart).componentCount; i < getType(result).componentCount; i++)
			{
				result.constantValue[i] = oldObject.constantValue[i];
			}
			break;
		}

	case spv::OpVectorShuffle:
		{
			auto &result = CreateConstant(insn);
			const auto &firstHalf = getObject(insn.word(4));
			const auto &secondHalf = getObject(insn.word(5));

			for(auto i = 0u; i < getType(result).componentCount; i++)
			{
				auto selector = insn.word(6 + i);
				if(selector == static_cast<uint32_t>(-1))
				{
					// Undefined value, we'll use zero
					result.constantValue[i] = 0;
				}
				else if(selector < getType(firstHalf).componentCount)
				{
					result.constantValue[i] = firstHalf.constantValue[selector];
				}
				else
				{
					result.constantValue[i] = secondHalf.constantValue[selector - getType(firstHalf).componentCount];
				}
			}
			break;
		}

	default:
		// Other spec constant ops are possible, but require capabilities that are
		// not exposed in our Vulkan implementation (eg Kernel), so we should never
		// get here for correct shaders.
		UNSUPPORTED("EvalSpecConstantOp op: %s", OpcodeName(opcode));
	}
}

void Spirv::EvalSpecConstantUnaryOp(InsnIterator insn)
{
	auto &result = CreateConstant(insn);

	auto opcode = static_cast<spv::Op>(insn.word(3));
	const auto &lhs = getObject(insn.word(4));
	auto size = getType(lhs).componentCount;

	for(auto i = 0u; i < size; i++)
	{
		auto &v = result.constantValue[i];
		auto l = lhs.constantValue[i];

		switch(opcode)
		{
		case spv::OpSConvert:
		case spv::OpFConvert:
		case spv::OpUConvert:
			UNREACHABLE("Not possible until we have multiple bit widths");
			break;

		case spv::OpSNegate:
			v = -(int)l;
			break;
		case spv::OpNot:
		case spv::OpLogicalNot:
			v = ~l;
			break;

		case spv::OpQuantizeToF16:
			{
				// Can do this nicer with host code, but want to perfectly mirror the reactor code we emit.
				auto abs = bit_cast<float>(l & 0x7FFFFFFF);
				auto sign = l & 0x80000000;
				auto isZero = abs < 0.000061035f ? ~0u : 0u;
				auto isInf = abs > 65504.0f ? ~0u : 0u;
				auto isNaN = (abs != abs) ? ~0u : 0u;
				auto isInfOrNan = isInf | isNaN;
				v = l & 0xFFFFE000;
				v &= ~isZero | 0x80000000;
				v = sign | (isInfOrNan & 0x7F800000) | (~isInfOrNan & v);
				v |= isNaN & 0x400000;
			}
			break;
		default:
			UNREACHABLE("EvalSpecConstantUnaryOp op: %s", OpcodeName(opcode));
		}
	}
}

void Spirv::EvalSpecConstantBinaryOp(InsnIterator insn)
{
	auto &result = CreateConstant(insn);

	auto opcode = static_cast<spv::Op>(insn.word(3));
	const auto &lhs = getObject(insn.word(4));
	const auto &rhs = getObject(insn.word(5));
	auto size = getType(lhs).componentCount;

	for(auto i = 0u; i < size; i++)
	{
		auto &v = result.constantValue[i];
		auto l = lhs.constantValue[i];
		auto r = rhs.constantValue[i];

		switch(opcode)
		{
		case spv::OpIAdd:
			v = l + r;
			break;
		case spv::OpISub:
			v = l - r;
			break;
		case spv::OpIMul:
			v = l * r;
			break;
		case spv::OpUDiv:
			v = (r == 0) ? 0 : l / r;
			break;
		case spv::OpUMod:
			v = (r == 0) ? 0 : l % r;
			break;
		case spv::OpSDiv:
			if(r == 0) r = UINT32_MAX;
			if(l == static_cast<uint32_t>(INT32_MIN)) l = UINT32_MAX;
			v = static_cast<int32_t>(l) / static_cast<int32_t>(r);
			break;
		case spv::OpSRem:
			if(r == 0) r = UINT32_MAX;
			if(l == static_cast<uint32_t>(INT32_MIN)) l = UINT32_MAX;
			v = static_cast<int32_t>(l) % static_cast<int32_t>(r);
			break;
		case spv::OpSMod:
			if(r == 0) r = UINT32_MAX;
			if(l == static_cast<uint32_t>(INT32_MIN)) l = UINT32_MAX;
			// Test if a signed-multiply would be negative.
			v = static_cast<int32_t>(l) % static_cast<int32_t>(r);
			if((v & 0x80000000) != (r & 0x80000000))
				v += r;
			break;
		case spv::OpShiftRightLogical:
			v = l >> r;
			break;
		case spv::OpShiftRightArithmetic:
			v = static_cast<int32_t>(l) >> r;
			break;
		case spv::OpShiftLeftLogical:
			v = l << r;
			break;
		case spv::OpBitwiseOr:
		case spv::OpLogicalOr:
			v = l | r;
			break;
		case spv::OpBitwiseAnd:
		case spv::OpLogicalAnd:
			v = l & r;
			break;
		case spv::OpBitwiseXor:
			v = l ^ r;
			break;
		case spv::OpLogicalEqual:
		case spv::OpIEqual:
			v = (l == r) ? ~0u : 0u;
			break;
		case spv::OpLogicalNotEqual:
		case spv::OpINotEqual:
			v = (l != r) ? ~0u : 0u;
			break;
		case spv::OpULessThan:
			v = l < r ? ~0u : 0u;
			break;
		case spv::OpSLessThan:
			v = static_cast<int32_t>(l) < static_cast<int32_t>(r) ? ~0u : 0u;
			break;
		case spv::OpUGreaterThan:
			v = l > r ? ~0u : 0u;
			break;
		case spv::OpSGreaterThan:
			v = static_cast<int32_t>(l) > static_cast<int32_t>(r) ? ~0u : 0u;
			break;
		case spv::OpULessThanEqual:
			v = l <= r ? ~0u : 0u;
			break;
		case spv::OpSLessThanEqual:
			v = static_cast<int32_t>(l) <= static_cast<int32_t>(r) ? ~0u : 0u;
			break;
		case spv::OpUGreaterThanEqual:
			v = l >= r ? ~0u : 0u;
			break;
		case spv::OpSGreaterThanEqual:
			v = static_cast<int32_t>(l) >= static_cast<int32_t>(r) ? ~0u : 0u;
			break;
		default:
			UNREACHABLE("EvalSpecConstantBinaryOp op: %s", OpcodeName(opcode));
		}
	}
}

}  // namespace sw