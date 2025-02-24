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

// Template function to perform a binary group operation.
// |TYPE| should be the type of the binary operation (as a SIMD::<ScalarType>).
// |I| should be a type suitable to initialize the identity value.
// |APPLY| should be a callable object that takes two RValue<TYPE> parameters
// and returns a new RValue<TYPE> corresponding to the operation's result.
template<typename TYPE, typename I, typename APPLY>
static RValue<TYPE> BinaryOperation(
    spv::GroupOperation operation,
    RValue<SIMD::UInt> value,
    RValue<SIMD::UInt> mask,
    const I identityValue,
    APPLY &&apply)
{
	auto identity = TYPE(identityValue);
	SIMD::UInt v_uint = (value & mask) | (As<SIMD::UInt>(identity) & ~mask);
	TYPE v = As<TYPE>(v_uint);

	switch(operation)
	{
	case spv::GroupOperationReduce:
		{
			// NOTE: floating-point add and multiply are not really commutative so
			//       ensure that all values in the final lanes are identical
			TYPE v2 = apply(v.xxzz, v.yyww);  // [xy]   [xy]   [zw]   [zw]
			return apply(v2.xxxx, v2.zzzz);   // [xyzw] [xyzw] [xyzw] [xyzw]
		}
		break;
	case spv::GroupOperationInclusiveScan:
		{
			TYPE v2 = apply(v, Shuffle(v, identity, 0x4012) /* [id, v.y, v.z, v.w] */);   // [x] [xy] [yz]  [zw]
			return apply(v2, Shuffle(v2, identity, 0x4401) /* [id,  id, v2.x, v2.y] */);  // [x] [xy] [xyz] [xyzw]
		}
		break;
	case spv::GroupOperationExclusiveScan:
		{
			TYPE v2 = apply(v, Shuffle(v, identity, 0x4012) /* [id, v.y, v.z, v.w] */);      // [x] [xy] [yz]  [zw]
			TYPE v3 = apply(v2, Shuffle(v2, identity, 0x4401) /* [id,  id, v2.x, v2.y] */);  // [x] [xy] [xyz] [xyzw]
			return Shuffle(v3, identity, 0x4012 /* [id, v3.x, v3.y, v3.z] */);               // [i] [x]  [xy]  [xyz]
		}
		break;
	default:
		UNSUPPORTED("Group operation: %d", operation);
		return identity;
	}
}

void SpirvEmitter::EmitGroupNonUniform(InsnIterator insn)
{
	ASSERT(SIMD::Width == 4);  // EmitGroupNonUniform makes many assumptions that the SIMD vector width is 4

	auto &type = shader.getType(Type::ID(insn.word(1)));
	Object::ID resultId = insn.word(2);
	auto scope = spv::Scope(shader.GetConstScalarInt(insn.word(3)));
	ASSERT_MSG(scope == spv::ScopeSubgroup, "Scope for Non Uniform Group Operations must be Subgroup for Vulkan 1.1");

	auto &dst = createIntermediate(resultId, type.componentCount);

	switch(insn.opcode())
	{
	case spv::OpGroupNonUniformElect:
		{
			// Result is true only in the active invocation with the lowest id
			// in the group, otherwise result is false.
			SIMD::Int active = activeLaneMask();  // Considers helper invocations active. See b/151137030
			// TODO: Would be nice if we could write this as:
			//   elect = active & ~(active.Oxyz | active.OOxy | active.OOOx)
			auto v0111 = SIMD::Int(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
			auto elect = active & ~(v0111 & (active.xxyz | active.xxxy | active.xxxx));
			dst.move(0, elect);
		}
		break;

	case spv::OpGroupNonUniformAll:
		{
			Operand predicate(shader, *this, insn.word(4));
			dst.move(0, AndAll(predicate.UInt(0) | ~As<SIMD::UInt>(activeLaneMask())));  // Considers helper invocations active. See b/151137030
		}
		break;

	case spv::OpGroupNonUniformAny:
		{
			Operand predicate(shader, *this, insn.word(4));
			dst.move(0, OrAll(predicate.UInt(0) & As<SIMD::UInt>(activeLaneMask())));  // Considers helper invocations active. See b/151137030
		}
		break;

	case spv::OpGroupNonUniformAllEqual:
		{
			Operand value(shader, *this, insn.word(4));
			auto res = SIMD::UInt(0xffffffff);
			SIMD::UInt active = As<SIMD::UInt>(activeLaneMask());  // Considers helper invocations active. See b/151137030
			SIMD::UInt inactive = ~active;
			for(auto i = 0u; i < type.componentCount; i++)
			{
				SIMD::UInt v = value.UInt(i) & active;
				SIMD::UInt filled = v;
				for(int j = 0; j < SIMD::Width - 1; j++)
				{
					filled |= filled.yzwx & inactive;  // Populate inactive 'holes' with a live value
				}
				res &= AndAll(CmpEQ(filled.xyzw, filled.yzwx));
			}
			dst.move(0, res);
		}
		break;

	case spv::OpGroupNonUniformBroadcast:
		{
			auto valueId = Object::ID(insn.word(4));
			auto idId = Object::ID(insn.word(5));
			Operand value(shader, *this, valueId);

			// Decide between the fast path for constants and the slow path for
			// intermediates.
			if(shader.getObject(idId).kind == Object::Kind::Constant)
			{
				auto id = SIMD::Int(shader.GetConstScalarInt(insn.word(5)));
				auto mask = CmpEQ(id, SIMD::Int(0, 1, 2, 3));
				for(auto i = 0u; i < type.componentCount; i++)
				{
					dst.move(i, OrAll(value.Int(i) & mask));
				}
			}
			else
			{
				Operand id(shader, *this, idId);

				SIMD::UInt active = As<SIMD::UInt>(activeLaneMask());  // Considers helper invocations active. See b/151137030
				SIMD::UInt inactive = ~active;
				SIMD::UInt filled = id.UInt(0) & active;

				for(int j = 0; j < SIMD::Width - 1; j++)
				{
					filled |= filled.yzwx & inactive;  // Populate inactive 'holes' with a live value
				}

				auto mask = CmpEQ(filled, SIMD::UInt(0, 1, 2, 3));

				for(uint32_t i = 0u; i < type.componentCount; i++)
				{
					dst.move(i, OrAll(value.UInt(i) & mask));
				}
			}
		}
		break;

	case spv::OpGroupNonUniformBroadcastFirst:
		{
			auto valueId = Object::ID(insn.word(4));
			Operand value(shader, *this, valueId);
			// Result is true only in the active invocation with the lowest id
			// in the group, otherwise result is false.
			SIMD::Int active = activeLaneMask();  // Considers helper invocations active. See b/151137030
			// TODO: Would be nice if we could write this as:
			//   elect = active & ~(active.Oxyz | active.OOxy | active.OOOx)
			auto v0111 = SIMD::Int(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
			auto elect = active & ~(v0111 & (active.xxyz | active.xxxy | active.xxxx));
			for(auto i = 0u; i < type.componentCount; i++)
			{
				dst.move(i, OrAll(value.Int(i) & elect));
			}
		}
		break;

	case spv::OpGroupNonUniformQuadBroadcast:
		{
			auto valueId = Object::ID(insn.word(4));
			Operand value(shader, *this, valueId);

			ASSERT(shader.getType(shader.getObject(insn.word(5))).componentCount == 1);
			auto indexId = Object::ID(insn.word(5));
			SIMD::Int index = Operand(shader, *this, indexId).Int(0);

			SIMD::Int active = activeLaneMask();
			// Populate all lanes in index with the same value. Index is required to be
			// uniform per the SPIR-V spec, so all active lanes should be identical.
			index = OrAll(active & index);
			SIMD::Int mask = CmpEQ(index, SIMD::Int(0, 1, 2, 3));

			for(auto i = 0u; i < type.componentCount; i++)
			{
				dst.move(i, OrAll(value.Int(i) & mask));
			}
		}
		break;

	case spv::OpGroupNonUniformQuadSwap:
		{
			auto valueId = Object::ID(insn.word(4));
			// SPIR-V spec: Drection must be a scalar of integer type and come from a constant instruction
			int direction = shader.GetConstScalarInt(insn.word(5));

			Operand value(shader, *this, valueId);
			for(auto i = 0u; i < type.componentCount; i++)
			{
				SIMD::Int v = value.Int(i);
				switch(direction)
				{
				case 0:  // Horizontal
					dst.move(i, v.yxwz);
					break;
				case 1:  // Vertical
					dst.move(i, v.zwxy);
					break;
				case 2:  // Diagonal
					dst.move(i, v.wzyx);
					break;
				default:
					// The SPIR-V spec doesn't define what happens in this case,
					// so the result in undefined.
					UNSUPPORTED("SPIR-V does not define a OpGroupNonUniformQuadSwap result for a direction of %d", direction);
					break;
				}
			}
		}
		break;

	case spv::OpGroupNonUniformBallot:
		{
			ASSERT(type.componentCount == 4);
			Operand predicate(shader, *this, insn.word(4));
			dst.move(0, SIMD::Int(SignMask(activeLaneMask() & predicate.Int(0))));  // Considers helper invocations active. See b/151137030
			dst.move(1, SIMD::Int(0));
			dst.move(2, SIMD::Int(0));
			dst.move(3, SIMD::Int(0));
		}
		break;

	case spv::OpGroupNonUniformInverseBallot:
		{
			auto valueId = Object::ID(insn.word(4));
			ASSERT(type.componentCount == 1);
			ASSERT(shader.getObjectType(valueId).componentCount == 4);
			Operand value(shader, *this, valueId);
			auto bit = (value.Int(0) >> SIMD::Int(0, 1, 2, 3)) & SIMD::Int(1);
			dst.move(0, -bit);
		}
		break;

	case spv::OpGroupNonUniformBallotBitExtract:
		{
			auto valueId = Object::ID(insn.word(4));
			auto indexId = Object::ID(insn.word(5));
			ASSERT(type.componentCount == 1);
			ASSERT(shader.getObjectType(valueId).componentCount == 4);
			ASSERT(shader.getObjectType(indexId).componentCount == 1);
			Operand value(shader, *this, valueId);
			Operand index(shader, *this, indexId);
			auto vecIdx = index.Int(0) / SIMD::Int(32);
			auto bitIdx = index.Int(0) & SIMD::Int(31);
			auto bits = (value.Int(0) & CmpEQ(vecIdx, SIMD::Int(0))) |
			            (value.Int(1) & CmpEQ(vecIdx, SIMD::Int(1))) |
			            (value.Int(2) & CmpEQ(vecIdx, SIMD::Int(2))) |
			            (value.Int(3) & CmpEQ(vecIdx, SIMD::Int(3)));
			dst.move(0, -((bits >> bitIdx) & SIMD::Int(1)));
		}
		break;

	case spv::OpGroupNonUniformBallotBitCount:
		{
			auto operation = spv::GroupOperation(insn.word(4));
			auto valueId = Object::ID(insn.word(5));
			ASSERT(type.componentCount == 1);
			ASSERT(shader.getObjectType(valueId).componentCount == 4);
			Operand value(shader, *this, valueId);
			switch(operation)
			{
			case spv::GroupOperationReduce:
				dst.move(0, CountBits(value.UInt(0) & SIMD::UInt(15)));
				break;
			case spv::GroupOperationInclusiveScan:
				dst.move(0, CountBits(value.UInt(0) & SIMD::UInt(1, 3, 7, 15)));
				break;
			case spv::GroupOperationExclusiveScan:
				dst.move(0, CountBits(value.UInt(0) & SIMD::UInt(0, 1, 3, 7)));
				break;
			default:
				UNSUPPORTED("GroupOperation %d", int(operation));
			}
		}
		break;

	case spv::OpGroupNonUniformBallotFindLSB:
		{
			auto valueId = Object::ID(insn.word(4));
			ASSERT(type.componentCount == 1);
			ASSERT(shader.getObjectType(valueId).componentCount == 4);
			Operand value(shader, *this, valueId);
			dst.move(0, Cttz(value.UInt(0) & SIMD::UInt(15), false));
		}
		break;

	case spv::OpGroupNonUniformBallotFindMSB:
		{
			auto valueId = Object::ID(insn.word(4));
			ASSERT(type.componentCount == 1);
			ASSERT(shader.getObjectType(valueId).componentCount == 4);
			Operand value(shader, *this, valueId);
			dst.move(0, SIMD::UInt(31) - Ctlz(value.UInt(0) & SIMD::UInt(15), false));
		}
		break;

	case spv::OpGroupNonUniformShuffle:
		{
			Operand value(shader, *this, insn.word(4));
			Operand id(shader, *this, insn.word(5));
			auto x = CmpEQ(SIMD::Int(0), id.Int(0));
			auto y = CmpEQ(SIMD::Int(1), id.Int(0));
			auto z = CmpEQ(SIMD::Int(2), id.Int(0));
			auto w = CmpEQ(SIMD::Int(3), id.Int(0));
			for(auto i = 0u; i < type.componentCount; i++)
			{
				SIMD::Int v = value.Int(i);
				dst.move(i, (x & v.xxxx) | (y & v.yyyy) | (z & v.zzzz) | (w & v.wwww));
			}
		}
		break;

	case spv::OpGroupNonUniformShuffleXor:
		{
			Operand value(shader, *this, insn.word(4));
			Operand mask(shader, *this, insn.word(5));
			auto x = CmpEQ(SIMD::Int(0), SIMD::Int(0, 1, 2, 3) ^ mask.Int(0));
			auto y = CmpEQ(SIMD::Int(1), SIMD::Int(0, 1, 2, 3) ^ mask.Int(0));
			auto z = CmpEQ(SIMD::Int(2), SIMD::Int(0, 1, 2, 3) ^ mask.Int(0));
			auto w = CmpEQ(SIMD::Int(3), SIMD::Int(0, 1, 2, 3) ^ mask.Int(0));
			for(auto i = 0u; i < type.componentCount; i++)
			{
				SIMD::Int v = value.Int(i);
				dst.move(i, (x & v.xxxx) | (y & v.yyyy) | (z & v.zzzz) | (w & v.wwww));
			}
		}
		break;

	case spv::OpGroupNonUniformShuffleUp:
		{
			Operand value(shader, *this, insn.word(4));
			Operand delta(shader, *this, insn.word(5));
			auto d0 = CmpEQ(SIMD::Int(0), delta.Int(0));
			auto d1 = CmpEQ(SIMD::Int(1), delta.Int(0));
			auto d2 = CmpEQ(SIMD::Int(2), delta.Int(0));
			auto d3 = CmpEQ(SIMD::Int(3), delta.Int(0));
			for(auto i = 0u; i < type.componentCount; i++)
			{
				SIMD::Int v = value.Int(i);
				dst.move(i, (d0 & v.xyzw) | (d1 & v.xxyz) | (d2 & v.xxxy) | (d3 & v.xxxx));
			}
		}
		break;

	case spv::OpGroupNonUniformShuffleDown:
		{
			Operand value(shader, *this, insn.word(4));
			Operand delta(shader, *this, insn.word(5));
			auto d0 = CmpEQ(SIMD::Int(0), delta.Int(0));
			auto d1 = CmpEQ(SIMD::Int(1), delta.Int(0));
			auto d2 = CmpEQ(SIMD::Int(2), delta.Int(0));
			auto d3 = CmpEQ(SIMD::Int(3), delta.Int(0));
			for(auto i = 0u; i < type.componentCount; i++)
			{
				SIMD::Int v = value.Int(i);
				dst.move(i, (d0 & v.xyzw) | (d1 & v.yzww) | (d2 & v.zwww) | (d3 & v.wwww));
			}
		}
		break;

	// The remaining instructions are GroupNonUniformArithmetic operations
	default:
		auto &type = shader.getType(Type::ID(insn.word(1)));
		auto operation = static_cast<spv::GroupOperation>(insn.word(4));
		Operand value(shader, *this, insn.word(5));
		auto mask = As<SIMD::UInt>(activeLaneMask());  // Considers helper invocations active. See b/151137030

		for(uint32_t i = 0; i < type.componentCount; i++)
		{
			switch(insn.opcode())
			{
			case spv::OpGroupNonUniformIAdd:
				dst.move(i, BinaryOperation<SIMD::Int>(
				                operation, value.UInt(i), mask, 0,
				                [](auto a, auto b) { return a + b; }));
				break;
			case spv::OpGroupNonUniformFAdd:
				dst.move(i, BinaryOperation<SIMD::Float>(
				                operation, value.UInt(i), mask, 0.0f,
				                [](auto a, auto b) { return a + b; }));
				break;

			case spv::OpGroupNonUniformIMul:
				dst.move(i, BinaryOperation<SIMD::Int>(
				                operation, value.UInt(i), mask, 1,
				                [](auto a, auto b) { return a * b; }));
				break;

			case spv::OpGroupNonUniformFMul:
				dst.move(i, BinaryOperation<SIMD::Float>(
				                operation, value.UInt(i), mask, 1.0f,
				                [](auto a, auto b) { return a * b; }));
				break;

			case spv::OpGroupNonUniformBitwiseAnd:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, ~0u,
				                [](auto a, auto b) { return a & b; }));
				break;

			case spv::OpGroupNonUniformBitwiseOr:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, 0,
				                [](auto a, auto b) { return a | b; }));
				break;

			case spv::OpGroupNonUniformBitwiseXor:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, 0,
				                [](auto a, auto b) { return a ^ b; }));
				break;

			case spv::OpGroupNonUniformSMin:
				dst.move(i, BinaryOperation<SIMD::Int>(
				                operation, value.UInt(i), mask, INT32_MAX,
				                [](auto a, auto b) { return Min(a, b); }));
				break;

			case spv::OpGroupNonUniformUMin:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, ~0u,
				                [](auto a, auto b) { return Min(a, b); }));
				break;

			case spv::OpGroupNonUniformFMin:
				dst.move(i, BinaryOperation<SIMD::Float>(
				                operation, value.UInt(i), mask, SIMD::Float::infinity(),
				                [](auto a, auto b) { return NMin(a, b); }));
				break;

			case spv::OpGroupNonUniformSMax:
				dst.move(i, BinaryOperation<SIMD::Int>(
				                operation, value.UInt(i), mask, INT32_MIN,
				                [](auto a, auto b) { return Max(a, b); }));
				break;

			case spv::OpGroupNonUniformUMax:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, 0,
				                [](auto a, auto b) { return Max(a, b); }));
				break;

			case spv::OpGroupNonUniformFMax:
				dst.move(i, BinaryOperation<SIMD::Float>(
				                operation, value.UInt(i), mask, -SIMD::Float::infinity(),
				                [](auto a, auto b) { return NMax(a, b); }));
				break;

			case spv::OpGroupNonUniformLogicalAnd:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, ~0u,
				                [](auto a, auto b) {
					                SIMD::UInt zero = SIMD::UInt(0);
					                return CmpNEQ(a, zero) & CmpNEQ(b, zero);
				                }));
				break;

			case spv::OpGroupNonUniformLogicalOr:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, 0,
				                [](auto a, auto b) {
					                SIMD::UInt zero = SIMD::UInt(0);
					                return CmpNEQ(a, zero) | CmpNEQ(b, zero);
				                }));
				break;

			case spv::OpGroupNonUniformLogicalXor:
				dst.move(i, BinaryOperation<SIMD::UInt>(
				                operation, value.UInt(i), mask, 0,
				                [](auto a, auto b) {
					                SIMD::UInt zero = SIMD::UInt(0);
					                return CmpNEQ(a, zero) ^ CmpNEQ(b, zero);
				                }));
				break;

			default:
				UNSUPPORTED("EmitGroupNonUniform op: %s", shader.OpcodeName(type.opcode()));
			}
		}
		break;
	}
}

}  // namespace sw
