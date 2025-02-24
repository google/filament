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
#include "Reactor/Assert.hpp"
#include "Vulkan/VkPipelineLayout.hpp"

#include <spirv/unified1/spirv.hpp>

namespace sw {

void SpirvEmitter::EmitLoad(InsnIterator insn)
{
	bool atomic = (insn.opcode() == spv::OpAtomicLoad);
	Object::ID resultId = insn.word(2);
	Object::ID pointerId = insn.word(3);
	auto &result = shader.getObject(resultId);
	auto &resultTy = shader.getType(result);
	auto &pointer = shader.getObject(pointerId);
	auto &pointerTy = shader.getType(pointer);
	std::memory_order memoryOrder = std::memory_order_relaxed;

	ASSERT(shader.getType(pointer).element == result.typeId());
	ASSERT(Type::ID(insn.word(1)) == result.typeId());
	ASSERT(!atomic || shader.getType(shader.getType(pointer).element).opcode() == spv::OpTypeInt);  // Vulkan 1.1: "Atomic instructions must declare a scalar 32-bit integer type, for the value pointed to by Pointer."

	if(pointerTy.storageClass == spv::StorageClassUniformConstant)
	{
		// Just propagate the pointer.
		auto &ptr = getPointer(pointerId);
		createPointer(resultId, ptr);
	}

	if(atomic)
	{
		Object::ID semanticsId = insn.word(5);
		auto memorySemantics = static_cast<spv::MemorySemanticsMask>(shader.getObject(semanticsId).constantValue[0]);
		memoryOrder = shader.MemoryOrder(memorySemantics);
	}

	auto ptr = GetPointerToData(pointerId, 0, false);
	auto robustness = shader.getOutOfBoundsBehavior(pointerId, routine->pipelineLayout);

	if(result.kind == Object::Kind::Pointer)
	{
		shader.VisitMemoryObject(pointerId, true, [&](const Spirv::MemoryElement &el) {
			ASSERT(el.index == 0);
			auto p = GetElementPointer(ptr, el.offset, pointerTy.storageClass);
			createPointer(resultId, p.Load<SIMD::Pointer>(robustness, activeLaneMask(), atomic, memoryOrder, sizeof(void *)));
		});

		SPIRV_SHADER_DBG("Load(atomic: {0}, order: {1}, ptr: {2}, mask: {3})", atomic, int(memoryOrder), ptr, activeLaneMask());
	}
	else
	{
		auto &dst = createIntermediate(resultId, resultTy.componentCount);
		shader.VisitMemoryObject(pointerId, false, [&](const Spirv::MemoryElement &el) {
			auto p = GetElementPointer(ptr, el.offset, pointerTy.storageClass);
			dst.move(el.index, p.Load<SIMD::Float>(robustness, activeLaneMask(), atomic, memoryOrder));
		});

		SPIRV_SHADER_DBG("Load(atomic: {0}, order: {1}, ptr: {2}, val: {3}, mask: {4})", atomic, int(memoryOrder), ptr, dst, activeLaneMask());
	}
}

void SpirvEmitter::EmitStore(InsnIterator insn)
{
	bool atomic = (insn.opcode() == spv::OpAtomicStore);
	Object::ID pointerId = insn.word(1);
	Object::ID objectId = insn.word(atomic ? 4 : 2);
	std::memory_order memoryOrder = std::memory_order_relaxed;

	if(atomic)
	{
		Object::ID semanticsId = insn.word(3);
		auto memorySemantics = static_cast<spv::MemorySemanticsMask>(shader.getObject(semanticsId).constantValue[0]);
		memoryOrder = shader.MemoryOrder(memorySemantics);
	}

	const auto &value = Operand(shader, *this, objectId);

	Store(pointerId, value, atomic, memoryOrder);
}

void SpirvEmitter::Store(Object::ID pointerId, const Operand &value, bool atomic, std::memory_order memoryOrder) const
{
	auto &pointer = shader.getObject(pointerId);
	auto &pointerTy = shader.getType(pointer);
	auto &elementTy = shader.getType(pointerTy.element);

	ASSERT(!atomic || elementTy.opcode() == spv::OpTypeInt);  // Vulkan 1.1: "Atomic instructions must declare a scalar 32-bit integer type, for the value pointed to by Pointer."

	auto ptr = GetPointerToData(pointerId, 0, false);
	auto robustness = shader.getOutOfBoundsBehavior(pointerId, routine->pipelineLayout);

	SIMD::Int mask = activeLaneMask();
	if(shader.StoresInHelperInvocationsHaveNoEffect(pointerTy.storageClass))
	{
		mask = mask & storesAndAtomicsMask();
	}

	SPIRV_SHADER_DBG("Store(atomic: {0}, order: {1}, ptr: {2}, val: {3}, mask: {4}", atomic, int(memoryOrder), ptr, value, mask);

	if(value.isPointer())
	{
		shader.VisitMemoryObject(pointerId, true, [&](const Spirv::MemoryElement &el) {
			ASSERT(el.index == 0);
			auto p = GetElementPointer(ptr, el.offset, pointerTy.storageClass);
			p.Store(value.Pointer(), robustness, mask, atomic, memoryOrder);
		});
	}
	else
	{
		shader.VisitMemoryObject(pointerId, false, [&](const Spirv::MemoryElement &el) {
			auto p = GetElementPointer(ptr, el.offset, pointerTy.storageClass);
			p.Store(value.Float(el.index), robustness, mask, atomic, memoryOrder);
		});
	}
}

void SpirvEmitter::EmitVariable(InsnIterator insn)
{
	Object::ID resultId = insn.word(2);
	auto &object = shader.getObject(resultId);
	auto &objectTy = shader.getType(object);

	switch(objectTy.storageClass)
	{
	case spv::StorageClassOutput:
	case spv::StorageClassPrivate:
	case spv::StorageClassFunction:
		{
			ASSERT(objectTy.opcode() == spv::OpTypePointer);
			auto base = &routine->getVariable(resultId)[0];
			auto elementTy = shader.getType(objectTy.element);
			auto size = elementTy.componentCount * static_cast<uint32_t>(sizeof(float)) * SIMD::Width;
			createPointer(resultId, SIMD::Pointer(base, size));
		}
		break;
	case spv::StorageClassWorkgroup:
		{
			ASSERT(objectTy.opcode() == spv::OpTypePointer);
			auto base = &routine->workgroupMemory[0];
			auto size = shader.workgroupMemory.size();
			createPointer(resultId, SIMD::Pointer(base, size, shader.workgroupMemory.offsetOf(resultId)));
		}
		break;
	case spv::StorageClassInput:
		{
			if(object.kind == Object::Kind::InterfaceVariable)
			{
				auto &dst = routine->getVariable(resultId);
				int offset = 0;
				shader.VisitInterface(resultId,
				                      [&](const Decorations &d, Spirv::AttribType type) {
					                      auto scalarSlot = d.Location << 2 | d.Component;
					                      dst[offset++] = routine->inputs[scalarSlot];
				                      });
			}
			ASSERT(objectTy.opcode() == spv::OpTypePointer);
			auto base = &routine->getVariable(resultId)[0];
			auto elementTy = shader.getType(objectTy.element);
			auto size = elementTy.componentCount * static_cast<uint32_t>(sizeof(float)) * SIMD::Width;
			createPointer(resultId, SIMD::Pointer(base, size));
		}
		break;
	case spv::StorageClassUniformConstant:
		{
			const auto &d = shader.descriptorDecorations.at(resultId);
			ASSERT(d.DescriptorSet >= 0);
			ASSERT(d.Binding >= 0);

			uint32_t bindingOffset = routine->pipelineLayout->getBindingOffset(d.DescriptorSet, d.Binding);
			Pointer<Byte> set = routine->descriptorSets[d.DescriptorSet];  // DescriptorSet*
			Pointer<Byte> binding = Pointer<Byte>(set + bindingOffset);    // vk::SampledImageDescriptor*
			auto size = 0;                                                 // Not required as this pointer is not directly used by SIMD::Read or SIMD::Write.
			createPointer(resultId, SIMD::Pointer(binding, size));
		}
		break;
	case spv::StorageClassUniform:
	case spv::StorageClassStorageBuffer:
	case spv::StorageClassPhysicalStorageBuffer:
		{
			const auto &d = shader.descriptorDecorations.at(resultId);
			ASSERT(d.DescriptorSet >= 0);
			auto size = 0;  // Not required as this pointer is not directly used by SIMD::Read or SIMD::Write.
			// Note: the module may contain descriptor set references that are not suitable for this implementation -- using a set index higher than the number
			// of descriptor set binding points we support. As long as the selected entrypoint doesn't actually touch the out of range binding points, this
			// is valid. In this case make the value nullptr to make it easier to diagnose an attempt to dereference it.
			if(static_cast<uint32_t>(d.DescriptorSet) < vk::MAX_BOUND_DESCRIPTOR_SETS)
			{
				createPointer(resultId, SIMD::Pointer(routine->descriptorSets[d.DescriptorSet], size));
			}
			else
			{
				createPointer(resultId, SIMD::Pointer(nullptr, 0));
			}
		}
		break;
	case spv::StorageClassPushConstant:
		{
			createPointer(resultId, SIMD::Pointer(routine->pushConstants, vk::MAX_PUSH_CONSTANT_SIZE));
		}
		break;
	default:
		UNREACHABLE("Storage class %d", objectTy.storageClass);
		break;
	}

	if(insn.wordCount() > 4)
	{
		Object::ID initializerId = insn.word(4);
		if(shader.getObject(initializerId).kind != Object::Kind::Constant)
		{
			UNIMPLEMENTED("b/148241854: Non-constant initializers not yet implemented");  // FIXME(b/148241854)
		}

		switch(objectTy.storageClass)
		{
		case spv::StorageClassOutput:
		case spv::StorageClassPrivate:
		case spv::StorageClassFunction:
		case spv::StorageClassWorkgroup:
			{
				auto ptr = GetPointerToData(resultId, 0, false);
				Operand initialValue(shader, *this, initializerId);

				shader.VisitMemoryObject(resultId, false, [&](const Spirv::MemoryElement &el) {
					auto p = GetElementPointer(ptr, el.offset, objectTy.storageClass);
					auto robustness = OutOfBoundsBehavior::UndefinedBehavior;  // Local variables are always within bounds.
					p.Store(initialValue.Float(el.index), robustness, activeLaneMask());
				});

				if(objectTy.storageClass == spv::StorageClassWorkgroup)
				{
					// Initialization of workgroup memory is done by each subgroup and requires waiting on a barrier.
					// TODO(b/221242292): Initialize just once per workgroup and eliminate the barrier.
					Yield(YieldResult::ControlBarrier);
				}
			}
			break;
		default:
			ASSERT_MSG(initializerId == 0, "Vulkan does not permit variables of storage class %d to have initializers", int(objectTy.storageClass));
		}
	}
}

void SpirvEmitter::EmitCopyMemory(InsnIterator insn)
{
	Object::ID dstPtrId = insn.word(1);
	Object::ID srcPtrId = insn.word(2);
	auto &dstPtrTy = shader.getObjectType(dstPtrId);
	auto &srcPtrTy = shader.getObjectType(srcPtrId);
	ASSERT(dstPtrTy.element == srcPtrTy.element);

	auto dstPtr = GetPointerToData(dstPtrId, 0, false);
	auto srcPtr = GetPointerToData(srcPtrId, 0, false);

	std::unordered_map<uint32_t, uint32_t> srcOffsets;

	shader.VisitMemoryObject(srcPtrId, false, [&](const Spirv::MemoryElement &el) { srcOffsets[el.index] = el.offset; });

	shader.VisitMemoryObject(dstPtrId, false, [&](const Spirv::MemoryElement &el) {
		auto it = srcOffsets.find(el.index);
		ASSERT(it != srcOffsets.end());
		auto srcOffset = it->second;
		auto dstOffset = el.offset;

		auto dst = GetElementPointer(dstPtr, dstOffset, dstPtrTy.storageClass);
		auto src = GetElementPointer(srcPtr, srcOffset, srcPtrTy.storageClass);

		// TODO(b/131224163): Optimize based on src/dst storage classes.
		auto robustness = OutOfBoundsBehavior::RobustBufferAccess;

		auto value = src.Load<SIMD::Float>(robustness, activeLaneMask());
		dst.Store(value, robustness, activeLaneMask());
	});
}

void SpirvEmitter::EmitMemoryBarrier(InsnIterator insn)
{
	auto semantics = spv::MemorySemanticsMask(shader.GetConstScalarInt(insn.word(2)));
	// TODO(b/176819536): We probably want to consider the memory scope here.
	// For now, just always emit the full fence.
	Fence(semantics);
}

void Spirv::VisitMemoryObjectInner(Type::ID id, Decorations d, uint32_t &index, uint32_t offset, bool resultIsPointer, const MemoryVisitor &f) const
{
	ApplyDecorationsForId(&d, id);
	const auto &type = getType(id);

	if(d.HasOffset)
	{
		offset += d.Offset;
		d.HasOffset = false;
	}

	switch(type.opcode())
	{
	case spv::OpTypePointer:
		if(resultIsPointer)
		{
			// Load/Store the pointer itself, rather than the structure pointed to by the pointer
			f(MemoryElement{ index++, offset, type });
		}
		else
		{
			VisitMemoryObjectInner(type.definition.word(3), d, index, offset, resultIsPointer, f);
		}
		break;
	case spv::OpTypeInt:
	case spv::OpTypeFloat:
	case spv::OpTypeRuntimeArray:
		f(MemoryElement{ index++, offset, type });
		break;
	case spv::OpTypeVector:
		{
			auto elemStride = (d.InsideMatrix && d.HasRowMajor && d.RowMajor) ? d.MatrixStride : static_cast<int32_t>(sizeof(float));
			for(auto i = 0u; i < type.definition.word(3); i++)
			{
				VisitMemoryObjectInner(type.definition.word(2), d, index, offset + elemStride * i, resultIsPointer, f);
			}
		}
		break;
	case spv::OpTypeMatrix:
		{
			auto columnStride = (d.HasRowMajor && d.RowMajor) ? static_cast<int32_t>(sizeof(float)) : d.MatrixStride;
			d.InsideMatrix = true;
			for(auto i = 0u; i < type.definition.word(3); i++)
			{
				ASSERT(d.HasMatrixStride);
				VisitMemoryObjectInner(type.definition.word(2), d, index, offset + columnStride * i, resultIsPointer, f);
			}
		}
		break;
	case spv::OpTypeStruct:
		for(auto i = 0u; i < type.definition.wordCount() - 2; i++)
		{
			ApplyDecorationsForIdMember(&d, id, i);
			VisitMemoryObjectInner(type.definition.word(i + 2), d, index, offset, resultIsPointer, f);
		}
		break;
	case spv::OpTypeArray:
		{
			auto arraySize = GetConstScalarInt(type.definition.word(3));
			for(auto i = 0u; i < arraySize; i++)
			{
				ASSERT(d.HasArrayStride);
				VisitMemoryObjectInner(type.definition.word(2), d, index, offset + i * d.ArrayStride, resultIsPointer, f);
			}
		}
		break;
	default:
		UNREACHABLE("%s", OpcodeName(type.opcode()));
	}
}

void Spirv::VisitMemoryObject(Object::ID id, bool resultIsPointer, const MemoryVisitor &f) const
{
	auto typeId = getObject(id).typeId();
	const auto &type = getType(typeId);

	if(IsExplicitLayout(type.storageClass))
	{
		Decorations d = GetDecorationsForId(id);
		uint32_t index = 0;
		VisitMemoryObjectInner(typeId, d, index, 0, resultIsPointer, f);
	}
	else
	{
		// Objects without explicit layout are tightly packed.
		auto &elType = getType(type.element);
		for(auto index = 0u; index < elType.componentCount; index++)
		{
			auto offset = static_cast<uint32_t>(index * sizeof(float));
			f({ index, offset, elType });
		}
	}
}

SIMD::Pointer SpirvEmitter::GetPointerToData(Object::ID id, SIMD::Int arrayIndices, bool nonUniform) const
{
	auto &object = shader.getObject(id);
	switch(object.kind)
	{
	case Object::Kind::Pointer:
	case Object::Kind::InterfaceVariable:
		return getPointer(id);

	case Object::Kind::DescriptorSet:
		{
			const auto &d = shader.descriptorDecorations.at(id);
			ASSERT(d.DescriptorSet >= 0 && static_cast<uint32_t>(d.DescriptorSet) < vk::MAX_BOUND_DESCRIPTOR_SETS);
			ASSERT(d.Binding >= 0);
			ASSERT(routine->pipelineLayout->getDescriptorCount(d.DescriptorSet, d.Binding) != 0);  // "If descriptorCount is zero this binding entry is reserved and the resource must not be accessed from any stage via this binding within any pipeline using the set layout."

			uint32_t bindingOffset = routine->pipelineLayout->getBindingOffset(d.DescriptorSet, d.Binding);
			uint32_t descriptorSize = routine->pipelineLayout->getDescriptorSize(d.DescriptorSet, d.Binding);

			auto set = getPointer(id);
			if(nonUniform)
			{
				SIMD::Int descriptorOffset = bindingOffset + descriptorSize * arrayIndices;
				auto robustness = shader.getOutOfBoundsBehavior(id, routine->pipelineLayout);
				ASSERT(routine->pipelineLayout->getDescriptorType(d.DescriptorSet, d.Binding) != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT);

				std::vector<Pointer<Byte>> pointers(SIMD::Width);
				for(int i = 0; i < SIMD::Width; i++)
				{
					pointers[i] = *Pointer<Pointer<Byte>>(set.getPointerForLane(i) + Extract(descriptorOffset, i) + OFFSET(vk::BufferDescriptor, ptr));
				}

				SIMD::Pointer ptr(pointers);

				if(routine->pipelineLayout->isDescriptorDynamic(d.DescriptorSet, d.Binding))
				{
					SIMD::Int dynamicOffsetIndex = SIMD::Int(routine->pipelineLayout->getDynamicOffsetIndex(d.DescriptorSet, d.Binding) + arrayIndices);
					SIMD::Pointer routineDynamicOffsets = SIMD::Pointer(routine->descriptorDynamicOffsets, 0, sizeof(int) * dynamicOffsetIndex);
					SIMD::Int dynamicOffsets = routineDynamicOffsets.Load<SIMD::Int>(robustness, activeLaneMask());
					ptr += dynamicOffsets;
				}
				return ptr;
			}
			else
			{
				rr::Int arrayIdx = Extract(arrayIndices, 0);
				rr::Int descriptorOffset = bindingOffset + descriptorSize * arrayIdx;
				Pointer<Byte> descriptor = set.getUniformPointer() + descriptorOffset;  // BufferDescriptor* or inline uniform block

				auto descriptorType = routine->pipelineLayout->getDescriptorType(d.DescriptorSet, d.Binding);
				if(descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT)
				{
					// Note: there is no bounds checking for inline uniform blocks.
					// MAX_INLINE_UNIFORM_BLOCK_SIZE represents the maximum size of
					// an inline uniform block, but this value should remain unused.
					return SIMD::Pointer(descriptor, vk::MAX_INLINE_UNIFORM_BLOCK_SIZE);
				}
				else
				{
					Pointer<Byte> data = *Pointer<Pointer<Byte>>(descriptor + OFFSET(vk::BufferDescriptor, ptr));  // void*
					rr::Int size = *Pointer<Int>(descriptor + OFFSET(vk::BufferDescriptor, sizeInBytes));

					if(routine->pipelineLayout->isDescriptorDynamic(d.DescriptorSet, d.Binding))
					{
						rr::Int dynamicOffsetIndex =
						    routine->pipelineLayout->getDynamicOffsetIndex(d.DescriptorSet, d.Binding) +
						    arrayIdx;
						rr::Int offset = routine->descriptorDynamicOffsets[dynamicOffsetIndex];
						rr::Int robustnessSize = *Pointer<rr::Int>(descriptor + OFFSET(vk::BufferDescriptor, robustnessSize));

						return SIMD::Pointer(data + offset, Min(size, robustnessSize - offset));
					}
					else
					{
						return SIMD::Pointer(data, size);
					}
				}
			}
		}

	default:
		UNREACHABLE("Invalid pointer kind %d", int(object.kind));
		return SIMD::Pointer(Pointer<Byte>(), 0);
	}
}

void SpirvEmitter::OffsetToElement(SIMD::Pointer &ptr, Object::ID elementId, int32_t arrayStride) const
{
	if(elementId != 0 && arrayStride != 0)
	{
		auto &elementObject = shader.getObject(elementId);
		ASSERT(elementObject.kind == Object::Kind::Constant || elementObject.kind == Object::Kind::Intermediate);

		if(elementObject.kind == Object::Kind::Constant)
		{
			ptr += shader.GetConstScalarInt(elementId) * arrayStride;
		}
		else
		{
			ptr += getIntermediate(elementId).Int(0) * arrayStride;
		}
	}
}

void SpirvEmitter::Fence(spv::MemorySemanticsMask semantics) const
{
	if(semantics != spv::MemorySemanticsMaskNone)
	{
		rr::Fence(shader.MemoryOrder(semantics));
	}
}

std::memory_order Spirv::MemoryOrder(spv::MemorySemanticsMask memorySemantics)
{
	uint32_t control = static_cast<uint32_t>(memorySemantics) & static_cast<uint32_t>(
	                                                                spv::MemorySemanticsAcquireMask |
	                                                                spv::MemorySemanticsReleaseMask |
	                                                                spv::MemorySemanticsAcquireReleaseMask |
	                                                                spv::MemorySemanticsSequentiallyConsistentMask);
	switch(control)
	{
	case spv::MemorySemanticsMaskNone: return std::memory_order_relaxed;
	case spv::MemorySemanticsAcquireMask: return std::memory_order_acquire;
	case spv::MemorySemanticsReleaseMask: return std::memory_order_release;
	case spv::MemorySemanticsAcquireReleaseMask: return std::memory_order_acq_rel;
	case spv::MemorySemanticsSequentiallyConsistentMask: return std::memory_order_acq_rel;  // Vulkan 1.1: "SequentiallyConsistent is treated as AcquireRelease"
	default:
		// "it is invalid for more than one of these four bits to be set:
		//  Acquire, Release, AcquireRelease, or SequentiallyConsistent."
		UNREACHABLE("MemorySemanticsMask: %x", int(control));
		return std::memory_order_acq_rel;
	}
}

bool Spirv::StoresInHelperInvocationsHaveNoEffect(spv::StorageClass storageClass)
{
	switch(storageClass)
	{
	// "Stores and atomics performed by helper invocations must not have any effect on memory..."
	default:
		return true;
	// "...except for the Function, Private and Output storage classes".
	case spv::StorageClassFunction:
	case spv::StorageClassPrivate:
	case spv::StorageClassOutput:
		return false;
	}
}

bool Spirv::IsExplicitLayout(spv::StorageClass storageClass)
{
	// From the Vulkan spec:
	// "Composite objects in the StorageBuffer, PhysicalStorageBuffer, Uniform,
	//  and PushConstant Storage Classes must be explicitly laid out."
	switch(storageClass)
	{
	case spv::StorageClassUniform:
	case spv::StorageClassStorageBuffer:
	case spv::StorageClassPhysicalStorageBuffer:
	case spv::StorageClassPushConstant:
		return true;
	default:
		return false;
	}
}

sw::SIMD::Pointer SpirvEmitter::GetElementPointer(sw::SIMD::Pointer structure, uint32_t offset, spv::StorageClass storageClass)
{
	if(IsStorageInterleavedByLane(storageClass))
	{
		for(int i = 0; i < SIMD::Width; i++)
		{
			structure.staticOffsets[i] += i * sizeof(float);
		}

		return structure + offset * sw::SIMD::Width;
	}
	else
	{
		return structure + offset;
	}
}

bool SpirvEmitter::IsStorageInterleavedByLane(spv::StorageClass storageClass)
{
	switch(storageClass)
	{
	case spv::StorageClassUniform:
	case spv::StorageClassStorageBuffer:
	case spv::StorageClassPhysicalStorageBuffer:
	case spv::StorageClassPushConstant:
	case spv::StorageClassWorkgroup:
	case spv::StorageClassImage:
		return false;
	default:
		return true;
	}
}

}  // namespace sw