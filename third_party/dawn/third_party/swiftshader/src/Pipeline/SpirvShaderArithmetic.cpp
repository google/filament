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
#include "SpirvShaderDebug.hpp"

#include "ShaderCore.hpp"

#include <spirv/unified1/spirv.hpp>

#include <limits>

namespace sw {

void SpirvEmitter::EmitVectorTimesScalar(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	for(auto i = 0u; i < type.componentCount; i++)
	{
		dst.move(i, lhs.Float(i) * rhs.Float(0));
	}
}

void SpirvEmitter::EmitMatrixTimesVector(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	for(auto i = 0u; i < type.componentCount; i++)
	{
		SIMD::Float v = lhs.Float(i) * rhs.Float(0);
		for(auto j = 1u; j < rhs.componentCount; j++)
		{
			v = MulAdd(lhs.Float(i + type.componentCount * j), rhs.Float(j), v);
		}
		dst.move(i, v);
	}
}

void SpirvEmitter::EmitVectorTimesMatrix(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	for(auto i = 0u; i < type.componentCount; i++)
	{
		SIMD::Float v = lhs.Float(0) * rhs.Float(i * lhs.componentCount);
		for(auto j = 1u; j < lhs.componentCount; j++)
		{
			v = MulAdd(lhs.Float(j), rhs.Float(i * lhs.componentCount + j), v);
		}
		dst.move(i, v);
	}
}

void SpirvEmitter::EmitMatrixTimesMatrix(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	auto numColumns = type.definition.word(3);
	auto numRows = shader.getType(type.definition.word(2)).definition.word(3);
	auto numAdds = shader.getObjectType(insn.word(3)).definition.word(3);

	for(auto row = 0u; row < numRows; row++)
	{
		for(auto col = 0u; col < numColumns; col++)
		{
			SIMD::Float v = lhs.Float(row) * rhs.Float(col * numAdds);
			for(auto i = 1u; i < numAdds; i++)
			{
				v = MulAdd(lhs.Float(i * numRows + row), rhs.Float(col * numAdds + i), v);
			}
			dst.move(numRows * col + row, v);
		}
	}
}

void SpirvEmitter::EmitOuterProduct(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	auto numRows = lhs.componentCount;
	auto numCols = rhs.componentCount;

	for(auto col = 0u; col < numCols; col++)
	{
		for(auto row = 0u; row < numRows; row++)
		{
			dst.move(col * numRows + row, lhs.Float(row) * rhs.Float(col));
		}
	}
}

void SpirvEmitter::EmitTranspose(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto mat = Operand(shader, *this, insn.word(3));

	auto numCols = type.definition.word(3);
	auto numRows = shader.getType(type.definition.word(2)).componentCount;

	for(auto col = 0u; col < numCols; col++)
	{
		for(auto row = 0u; row < numRows; row++)
		{
			dst.move(col * numRows + row, mat.Float(row * numCols + col));
		}
	}
}

void SpirvEmitter::EmitBitcastPointer(Spirv::Object::ID resultID, Operand &src)
{
	if(src.isPointer())  // Pointer -> Integer bits
	{
		if(sizeof(void *) == 4)  // 32-bit pointers
		{
			SIMD::UInt bits;
			src.Pointer().castTo(bits);

			auto &dst = createIntermediate(resultID, 1);
			dst.move(0, bits);
		}
		else  // 64-bit pointers
		{
			ASSERT(sizeof(void *) == 8);
			// Casting a 64 bit pointer into 2 32bit integers
			auto &ptr = src.Pointer();
			SIMD::UInt lowerBits, upperBits;
			ptr.castTo(lowerBits, upperBits);

			auto &dst = createIntermediate(resultID, 2);
			dst.move(0, lowerBits);
			dst.move(1, upperBits);
		}
	}
	else  // Integer bits -> Pointer
	{
		if(sizeof(void *) == 4)  // 32-bit pointers
		{
			createPointer(resultID, SIMD::Pointer(src.UInt(0)));
		}
		else  // 64-bit pointers
		{
			ASSERT(sizeof(void *) == 8);
			// Casting two 32-bit integers into a 64-bit pointer
			createPointer(resultID, SIMD::Pointer(src.UInt(0), src.UInt(1)));
		}
	}
}

void SpirvEmitter::EmitUnaryOp(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto src = Operand(shader, *this, insn.word(3));

	bool dstIsPointer = shader.getObject(insn.resultId()).kind == Spirv::Object::Kind::Pointer;
	bool srcIsPointer = src.isPointer();
	if(srcIsPointer || dstIsPointer)
	{
		ASSERT(insn.opcode() == spv::OpBitcast);
		ASSERT(srcIsPointer || (type.componentCount == 1));  // When the ouput is a pointer, it's a single pointer

		return EmitBitcastPointer(insn.resultId(), src);
	}

	auto &dst = createIntermediate(insn.resultId(), type.componentCount);

	for(auto i = 0u; i < type.componentCount; i++)
	{
		switch(insn.opcode())
		{
		case spv::OpNot:
		case spv::OpLogicalNot:  // logical not == bitwise not due to all-bits boolean representation
			dst.move(i, ~src.UInt(i));
			break;
		case spv::OpBitFieldInsert:
			{
				auto insert = Operand(shader, *this, insn.word(4)).UInt(i);
				auto offset = Operand(shader, *this, insn.word(5)).UInt(0);
				auto count = Operand(shader, *this, insn.word(6)).UInt(0);
				auto one = SIMD::UInt(1);
				auto v = src.UInt(i);
				auto mask = Bitmask32(offset + count) ^ Bitmask32(offset);
				dst.move(i, (v & ~mask) | ((insert << offset) & mask));
			}
			break;
		case spv::OpBitFieldSExtract:
		case spv::OpBitFieldUExtract:
			{
				auto offset = Operand(shader, *this, insn.word(4)).UInt(0);
				auto count = Operand(shader, *this, insn.word(5)).UInt(0);
				auto one = SIMD::UInt(1);
				auto v = src.UInt(i);
				SIMD::UInt out = (v >> offset) & Bitmask32(count);
				if(insn.opcode() == spv::OpBitFieldSExtract)
				{
					auto sign = out & NthBit32(count - one);
					auto sext = ~(sign - one);
					out |= sext;
				}
				dst.move(i, out);
			}
			break;
		case spv::OpBitReverse:
			{
				// TODO: Add an intrinsic to reactor. Even if there isn't a
				// single vector instruction, there may be target-dependent
				// ways to make this faster.
				// https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
				SIMD::UInt v = src.UInt(i);
				v = ((v >> 1) & SIMD::UInt(0x55555555)) | ((v & SIMD::UInt(0x55555555)) << 1);
				v = ((v >> 2) & SIMD::UInt(0x33333333)) | ((v & SIMD::UInt(0x33333333)) << 2);
				v = ((v >> 4) & SIMD::UInt(0x0F0F0F0F)) | ((v & SIMD::UInt(0x0F0F0F0F)) << 4);
				v = ((v >> 8) & SIMD::UInt(0x00FF00FF)) | ((v & SIMD::UInt(0x00FF00FF)) << 8);
				v = (v >> 16) | (v << 16);
				dst.move(i, v);
			}
			break;
		case spv::OpBitCount:
			dst.move(i, CountBits(src.UInt(i)));
			break;
		case spv::OpSNegate:
			dst.move(i, -src.Int(i));
			break;
		case spv::OpFNegate:
			dst.move(i, -src.Float(i));
			break;
		case spv::OpConvertFToU:
			dst.move(i, SIMD::UInt(src.Float(i)));
			break;
		case spv::OpConvertFToS:
			dst.move(i, SIMD::Int(src.Float(i)));
			break;
		case spv::OpConvertSToF:
			dst.move(i, SIMD::Float(src.Int(i)));
			break;
		case spv::OpConvertUToF:
			dst.move(i, SIMD::Float(src.UInt(i)));
			break;
		case spv::OpBitcast:
			dst.move(i, src.Float(i));
			break;
		case spv::OpIsInf:
			dst.move(i, IsInf(src.Float(i)));
			break;
		case spv::OpIsNan:
			dst.move(i, IsNan(src.Float(i)));
			break;
		case spv::OpDPdx:
		case spv::OpDPdxCoarse:
			// Derivative instructions: FS invocations are laid out like so:
			//    0 1
			//    2 3
			ASSERT(SIMD::Width == 4);  // All cross-lane instructions will need care when using a different width
			dst.move(i, SIMD::Float(Extract(src.Float(i), 1) - Extract(src.Float(i), 0)));
			break;
		case spv::OpDPdy:
		case spv::OpDPdyCoarse:
			dst.move(i, SIMD::Float(Extract(src.Float(i), 2) - Extract(src.Float(i), 0)));
			break;
		case spv::OpFwidth:
		case spv::OpFwidthCoarse:
			dst.move(i, SIMD::Float(Abs(Extract(src.Float(i), 1) - Extract(src.Float(i), 0)) + Abs(Extract(src.Float(i), 2) - Extract(src.Float(i), 0))));
			break;
		case spv::OpDPdxFine:
			{
				auto firstRow = Extract(src.Float(i), 1) - Extract(src.Float(i), 0);
				auto secondRow = Extract(src.Float(i), 3) - Extract(src.Float(i), 2);
				SIMD::Float v = SIMD::Float(firstRow);
				v = Insert(v, secondRow, 2);
				v = Insert(v, secondRow, 3);
				dst.move(i, v);
			}
			break;
		case spv::OpDPdyFine:
			{
				auto firstColumn = Extract(src.Float(i), 2) - Extract(src.Float(i), 0);
				auto secondColumn = Extract(src.Float(i), 3) - Extract(src.Float(i), 1);
				SIMD::Float v = SIMD::Float(firstColumn);
				v = Insert(v, secondColumn, 1);
				v = Insert(v, secondColumn, 3);
				dst.move(i, v);
			}
			break;
		case spv::OpFwidthFine:
			{
				auto firstRow = Extract(src.Float(i), 1) - Extract(src.Float(i), 0);
				auto secondRow = Extract(src.Float(i), 3) - Extract(src.Float(i), 2);
				SIMD::Float dpdx = SIMD::Float(firstRow);
				dpdx = Insert(dpdx, secondRow, 2);
				dpdx = Insert(dpdx, secondRow, 3);
				auto firstColumn = Extract(src.Float(i), 2) - Extract(src.Float(i), 0);
				auto secondColumn = Extract(src.Float(i), 3) - Extract(src.Float(i), 1);
				SIMD::Float dpdy = SIMD::Float(firstColumn);
				dpdy = Insert(dpdy, secondColumn, 1);
				dpdy = Insert(dpdy, secondColumn, 3);
				dst.move(i, Abs(dpdx) + Abs(dpdy));
			}
			break;
		case spv::OpQuantizeToF16:
			{
				// Note: keep in sync with the specialization constant version in EvalSpecConstantUnaryOp
				auto abs = Abs(src.Float(i));
				auto sign = src.Int(i) & SIMD::Int(0x80000000);
				auto isZero = CmpLT(abs, SIMD::Float(0.000061035f));
				auto isInf = CmpGT(abs, SIMD::Float(65504.0f));
				auto isNaN = IsNan(abs);
				auto isInfOrNan = isInf | isNaN;
				SIMD::Int v = src.Int(i) & SIMD::Int(0xFFFFE000);
				v &= ~isZero | SIMD::Int(0x80000000);
				v = sign | (isInfOrNan & SIMD::Int(0x7F800000)) | (~isInfOrNan & v);
				v |= isNaN & SIMD::Int(0x400000);
				dst.move(i, v);
			}
			break;
		default:
			UNREACHABLE("%s", shader.OpcodeName(insn.opcode()));
		}
	}
}

void SpirvEmitter::EmitBinaryOp(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto &lhsType = shader.getObjectType(insn.word(3));
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	for(auto i = 0u; i < lhsType.componentCount; i++)
	{
		switch(insn.opcode())
		{
		case spv::OpIAdd:
			dst.move(i, lhs.Int(i) + rhs.Int(i));
			break;
		case spv::OpISub:
			dst.move(i, lhs.Int(i) - rhs.Int(i));
			break;
		case spv::OpIMul:
			dst.move(i, lhs.Int(i) * rhs.Int(i));
			break;
		case spv::OpSDiv:
			{
				SIMD::Int a = lhs.Int(i);
				SIMD::Int b = rhs.Int(i);
				b = b | CmpEQ(b, SIMD::Int(0));                                       // prevent divide-by-zero
				a = a | (CmpEQ(a, SIMD::Int(0x80000000)) & CmpEQ(b, SIMD::Int(-1)));  // prevent integer overflow
				dst.move(i, a / b);
			}
			break;
		case spv::OpUDiv:
			{
				auto zeroMask = As<SIMD::UInt>(CmpEQ(rhs.Int(i), SIMD::Int(0)));
				dst.move(i, lhs.UInt(i) / (rhs.UInt(i) | zeroMask));
			}
			break;
		case spv::OpSRem:
			{
				SIMD::Int a = lhs.Int(i);
				SIMD::Int b = rhs.Int(i);
				b = b | CmpEQ(b, SIMD::Int(0));                                       // prevent divide-by-zero
				a = a | (CmpEQ(a, SIMD::Int(0x80000000)) & CmpEQ(b, SIMD::Int(-1)));  // prevent integer overflow
				dst.move(i, a % b);
			}
			break;
		case spv::OpSMod:
			{
				SIMD::Int a = lhs.Int(i);
				SIMD::Int b = rhs.Int(i);
				b = b | CmpEQ(b, SIMD::Int(0));                                       // prevent divide-by-zero
				a = a | (CmpEQ(a, SIMD::Int(0x80000000)) & CmpEQ(b, SIMD::Int(-1)));  // prevent integer overflow
				auto mod = a % b;
				// If a and b have opposite signs, the remainder operation takes
				// the sign from a but OpSMod is supposed to take the sign of b.
				// Adding b will ensure that the result has the correct sign and
				// that it is still congruent to a modulo b.
				//
				// See also http://mathforum.org/library/drmath/view/52343.html
				auto signDiff = CmpNEQ(CmpGE(a, SIMD::Int(0)), CmpGE(b, SIMD::Int(0)));
				auto fixedMod = mod + (b & CmpNEQ(mod, SIMD::Int(0)) & signDiff);
				dst.move(i, As<SIMD::Float>(fixedMod));
			}
			break;
		case spv::OpUMod:
			{
				auto zeroMask = As<SIMD::UInt>(CmpEQ(rhs.Int(i), SIMD::Int(0)));
				dst.move(i, lhs.UInt(i) % (rhs.UInt(i) | zeroMask));
			}
			break;
		case spv::OpIEqual:
		case spv::OpLogicalEqual:
			dst.move(i, CmpEQ(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpINotEqual:
		case spv::OpLogicalNotEqual:
			dst.move(i, CmpNEQ(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpUGreaterThan:
			dst.move(i, CmpGT(lhs.UInt(i), rhs.UInt(i)));
			break;
		case spv::OpSGreaterThan:
			dst.move(i, CmpGT(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpUGreaterThanEqual:
			dst.move(i, CmpGE(lhs.UInt(i), rhs.UInt(i)));
			break;
		case spv::OpSGreaterThanEqual:
			dst.move(i, CmpGE(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpULessThan:
			dst.move(i, CmpLT(lhs.UInt(i), rhs.UInt(i)));
			break;
		case spv::OpSLessThan:
			dst.move(i, CmpLT(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpULessThanEqual:
			dst.move(i, CmpLE(lhs.UInt(i), rhs.UInt(i)));
			break;
		case spv::OpSLessThanEqual:
			dst.move(i, CmpLE(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpFAdd:
			dst.move(i, lhs.Float(i) + rhs.Float(i));
			break;
		case spv::OpFSub:
			dst.move(i, lhs.Float(i) - rhs.Float(i));
			break;
		case spv::OpFMul:
			dst.move(i, lhs.Float(i) * rhs.Float(i));
			break;
		case spv::OpFDiv:
			// TODO(b/169760262): Optimize using reciprocal instructions (2.5 ULP).
			// TODO(b/222218659): Optimize for RelaxedPrecision (2.5 ULP).
			dst.move(i, lhs.Float(i) / rhs.Float(i));
			break;
		case spv::OpFMod:
			// TODO(b/126873455): Inaccurate for values greater than 2^24.
			// TODO(b/169760262): Optimize using reciprocal instructions.
			// TODO(b/222218659): Optimize for RelaxedPrecision.
			dst.move(i, lhs.Float(i) - rhs.Float(i) * Floor(lhs.Float(i) / rhs.Float(i)));
			break;
		case spv::OpFRem:
			// TODO(b/169760262): Optimize using reciprocal instructions.
			// TODO(b/222218659): Optimize for RelaxedPrecision.
			dst.move(i, lhs.Float(i) % rhs.Float(i));
			break;
		case spv::OpFOrdEqual:
			dst.move(i, CmpEQ(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFUnordEqual:
			dst.move(i, CmpUEQ(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFOrdNotEqual:
			dst.move(i, CmpNEQ(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFUnordNotEqual:
			dst.move(i, CmpUNEQ(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFOrdLessThan:
			dst.move(i, CmpLT(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFUnordLessThan:
			dst.move(i, CmpULT(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFOrdGreaterThan:
			dst.move(i, CmpGT(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFUnordGreaterThan:
			dst.move(i, CmpUGT(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFOrdLessThanEqual:
			dst.move(i, CmpLE(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFUnordLessThanEqual:
			dst.move(i, CmpULE(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFOrdGreaterThanEqual:
			dst.move(i, CmpGE(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpFUnordGreaterThanEqual:
			dst.move(i, CmpUGE(lhs.Float(i), rhs.Float(i)));
			break;
		case spv::OpShiftRightLogical:
			dst.move(i, lhs.UInt(i) >> rhs.UInt(i));
			break;
		case spv::OpShiftRightArithmetic:
			dst.move(i, lhs.Int(i) >> rhs.Int(i));
			break;
		case spv::OpShiftLeftLogical:
			dst.move(i, lhs.UInt(i) << rhs.UInt(i));
			break;
		case spv::OpBitwiseOr:
		case spv::OpLogicalOr:
			dst.move(i, lhs.UInt(i) | rhs.UInt(i));
			break;
		case spv::OpBitwiseXor:
			dst.move(i, lhs.UInt(i) ^ rhs.UInt(i));
			break;
		case spv::OpBitwiseAnd:
		case spv::OpLogicalAnd:
			dst.move(i, lhs.UInt(i) & rhs.UInt(i));
			break;
		case spv::OpSMulExtended:
			// Extended ops: result is a structure containing two members of the same type as lhs & rhs.
			// In our flat view then, component i is the i'th component of the first member;
			// component i + N is the i'th component of the second member.
			dst.move(i, lhs.Int(i) * rhs.Int(i));
			dst.move(i + lhsType.componentCount, MulHigh(lhs.Int(i), rhs.Int(i)));
			break;
		case spv::OpUMulExtended:
			dst.move(i, lhs.UInt(i) * rhs.UInt(i));
			dst.move(i + lhsType.componentCount, MulHigh(lhs.UInt(i), rhs.UInt(i)));
			break;
		case spv::OpIAddCarry:
			dst.move(i, lhs.UInt(i) + rhs.UInt(i));
			dst.move(i + lhsType.componentCount, CmpLT(dst.UInt(i), lhs.UInt(i)) >> 31);
			break;
		case spv::OpISubBorrow:
			dst.move(i, lhs.UInt(i) - rhs.UInt(i));
			dst.move(i + lhsType.componentCount, CmpLT(lhs.UInt(i), rhs.UInt(i)) >> 31);
			break;
		default:
			UNREACHABLE("%s", shader.OpcodeName(insn.opcode()));
		}
	}

	SPIRV_SHADER_DBG("{0}: {1}", insn.word(2), dst);
	SPIRV_SHADER_DBG("{0}: {1}", insn.word(3), lhs);
	SPIRV_SHADER_DBG("{0}: {1}", insn.word(4), rhs);
}

void SpirvEmitter::EmitDot(Spirv::InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	ASSERT(type.componentCount == 1);
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	auto &lhsType = shader.getObjectType(insn.word(3));
	auto lhs = Operand(shader, *this, insn.word(3));
	auto rhs = Operand(shader, *this, insn.word(4));

	auto opcode = insn.opcode();
	switch(opcode)
	{
	case spv::OpDot:
		dst.move(0, FDot(lhsType.componentCount, lhs, rhs));
		break;
	case spv::OpSDot:
		dst.move(0, SDot(lhsType.componentCount, lhs, rhs, nullptr));
		break;
	case spv::OpUDot:
		dst.move(0, UDot(lhsType.componentCount, lhs, rhs, nullptr));
		break;
	case spv::OpSUDot:
		dst.move(0, SUDot(lhsType.componentCount, lhs, rhs, nullptr));
		break;
	case spv::OpSDotAccSat:
		{
			auto accum = Operand(shader, *this, insn.word(5));
			dst.move(0, SDot(lhsType.componentCount, lhs, rhs, &accum));
		}
		break;
	case spv::OpUDotAccSat:
		{
			auto accum = Operand(shader, *this, insn.word(5));
			dst.move(0, UDot(lhsType.componentCount, lhs, rhs, &accum));
		}
		break;
	case spv::OpSUDotAccSat:
		{
			auto accum = Operand(shader, *this, insn.word(5));
			dst.move(0, SUDot(lhsType.componentCount, lhs, rhs, &accum));
		}
		break;
	default:
		UNREACHABLE("%s", shader.OpcodeName(opcode));
		break;
	}

	SPIRV_SHADER_DBG("{0}: {1}", insn.resultId(), dst);
	SPIRV_SHADER_DBG("{0}: {1}", insn.word(3), lhs);
	SPIRV_SHADER_DBG("{0}: {1}", insn.word(4), rhs);
}

SIMD::Float SpirvEmitter::FDot(unsigned numComponents, const Operand &x, const Operand &y)
{
	SIMD::Float d = x.Float(0) * y.Float(0);

	for(auto i = 1u; i < numComponents; i++)
	{
		d = MulAdd(x.Float(i), y.Float(i), d);
	}

	return d;
}

SIMD::Int SpirvEmitter::SDot(unsigned numComponents, const Operand &x, const Operand &y, const Operand *accum)
{
	SIMD::Int d(0);

	if(numComponents == 1)  // 4x8bit packed
	{
		numComponents = 4;
		for(auto i = 0u; i < numComponents; i++)
		{
			Int4 xs(As<SByte4>(Extract(x.Int(0), i)));
			Int4 ys(As<SByte4>(Extract(y.Int(0), i)));

			Int4 xy = xs * ys;
			rr::Int sum = Extract(xy, 0) + Extract(xy, 1) + Extract(xy, 2) + Extract(xy, 3);

			d = Insert(d, sum, i);
		}
	}
	else
	{
		d = x.Int(0) * y.Int(0);

		for(auto i = 1u; i < numComponents; i++)
		{
			d += x.Int(i) * y.Int(i);
		}
	}

	if(accum)
	{
		d = AddSat(d, accum->Int(0));
	}

	return d;
}

SIMD::UInt SpirvEmitter::UDot(unsigned numComponents, const Operand &x, const Operand &y, const Operand *accum)
{
	SIMD::UInt d(0);

	if(numComponents == 1)  // 4x8bit packed
	{
		numComponents = 4;
		for(auto i = 0u; i < numComponents; i++)
		{
			Int4 xs(As<Byte4>(Extract(x.Int(0), i)));
			Int4 ys(As<Byte4>(Extract(y.Int(0), i)));

			UInt4 xy = xs * ys;
			rr::UInt sum = Extract(xy, 0) + Extract(xy, 1) + Extract(xy, 2) + Extract(xy, 3);

			d = Insert(d, sum, i);
		}
	}
	else
	{
		d = x.UInt(0) * y.UInt(0);

		for(auto i = 1u; i < numComponents; i++)
		{
			d += x.UInt(i) * y.UInt(i);
		}
	}

	if(accum)
	{
		d = AddSat(d, accum->UInt(0));
	}

	return d;
}

SIMD::Int SpirvEmitter::SUDot(unsigned numComponents, const Operand &x, const Operand &y, const Operand *accum)
{
	SIMD::Int d(0);

	if(numComponents == 1)  // 4x8bit packed
	{
		numComponents = 4;
		for(auto i = 0u; i < numComponents; i++)
		{
			Int4 xs(As<SByte4>(Extract(x.Int(0), i)));
			Int4 ys(As<Byte4>(Extract(y.Int(0), i)));

			Int4 xy = xs * ys;
			rr::Int sum = Extract(xy, 0) + Extract(xy, 1) + Extract(xy, 2) + Extract(xy, 3);

			d = Insert(d, sum, i);
		}
	}
	else
	{
		d = x.Int(0) * As<SIMD::Int>(y.UInt(0));

		for(auto i = 1u; i < numComponents; i++)
		{
			d += x.Int(i) * As<SIMD::Int>(y.UInt(i));
		}
	}

	if(accum)
	{
		d = AddSat(d, accum->Int(0));
	}

	return d;
}

SIMD::Int SpirvEmitter::AddSat(RValue<SIMD::Int> a, RValue<SIMD::Int> b)
{
	SIMD::Int sum = a + b;
	SIMD::Int sSign = sum >> 31;
	SIMD::Int aSign = a >> 31;
	SIMD::Int bSign = b >> 31;

	// Overflow happened if both numbers added have the same sign and the sum has a different sign
	SIMD::Int oob = ~(aSign ^ bSign) & (aSign ^ sSign);
	SIMD::Int overflow = oob & sSign;
	SIMD::Int underflow = oob & aSign;

	return (overflow & std::numeric_limits<int32_t>::max()) |
	       (underflow & std::numeric_limits<int32_t>::min()) |
	       (~oob & sum);
}

SIMD::UInt SpirvEmitter::AddSat(RValue<SIMD::UInt> a, RValue<SIMD::UInt> b)
{
	SIMD::UInt sum = a + b;

	// Overflow happened if the sum of unsigned integers is smaller than either of the 2 numbers being added
	// Note: CmpLT()'s return value is automatically set to UINT_MAX when true
	return CmpLT(sum, a) | sum;
}

}  // namespace sw