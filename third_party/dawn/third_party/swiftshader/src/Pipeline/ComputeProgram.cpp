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

#include "ComputeProgram.hpp"

#include "Constants.hpp"
#include "System/Debug.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkPipelineLayout.hpp"

#include "marl/defer.h"
#include "marl/trace.h"
#include "marl/waitgroup.h"

#include <queue>

namespace sw {

ComputeProgram::ComputeProgram(vk::Device *device, std::shared_ptr<SpirvShader> shader, const vk::PipelineLayout *pipelineLayout, const vk::DescriptorSet::Bindings &descriptorSets)
    : device(device)
    , shader(shader)
    , pipelineLayout(pipelineLayout)
    , descriptorSets(descriptorSets)
{
}

ComputeProgram::~ComputeProgram()
{
}

void ComputeProgram::generate()
{
	MARL_SCOPED_EVENT("ComputeProgram::generate");

	SpirvRoutine routine(pipelineLayout);
	shader->emitProlog(&routine);
	emit(&routine);
	shader->emitEpilog(&routine);
}

void ComputeProgram::setWorkgroupBuiltins(Pointer<Byte> data, SpirvRoutine *routine, Int workgroupID[3])
{
	// TODO(b/146486064): Consider only assigning these to the SpirvRoutine iff they are ever going to be read.
	routine->numWorkgroups = *Pointer<Int4>(data + OFFSET(Data, numWorkgroups));
	routine->workgroupID = Insert(Insert(Insert(Int4(0), workgroupID[0], 0), workgroupID[1], 1), workgroupID[2], 2);
	routine->workgroupSize = *Pointer<Int4>(data + OFFSET(Data, workgroupSize));
	routine->subgroupsPerWorkgroup = *Pointer<Int>(data + OFFSET(Data, subgroupsPerWorkgroup));
	routine->invocationsPerSubgroup = *Pointer<Int>(data + OFFSET(Data, invocationsPerSubgroup));

	routine->setInputBuiltin(shader.get(), spv::BuiltInNumWorkgroups, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		value[builtin.FirstComponent + 0] = As<SIMD::Float>(SIMD::Int(routine->numWorkgroups.x));
		value[builtin.FirstComponent + 1] = As<SIMD::Float>(SIMD::Int(routine->numWorkgroups.y));
		value[builtin.FirstComponent + 2] = As<SIMD::Float>(SIMD::Int(routine->numWorkgroups.z));
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInWorkgroupId, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		value[builtin.FirstComponent + 0] = As<SIMD::Float>(SIMD::Int(workgroupID[0]));
		value[builtin.FirstComponent + 1] = As<SIMD::Float>(SIMD::Int(workgroupID[1]));
		value[builtin.FirstComponent + 2] = As<SIMD::Float>(SIMD::Int(workgroupID[2]));
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInWorkgroupSize, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		value[builtin.FirstComponent + 0] = As<SIMD::Float>(SIMD::Int(routine->workgroupSize.x));
		value[builtin.FirstComponent + 1] = As<SIMD::Float>(SIMD::Int(routine->workgroupSize.y));
		value[builtin.FirstComponent + 2] = As<SIMD::Float>(SIMD::Int(routine->workgroupSize.z));
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInNumSubgroups, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		value[builtin.FirstComponent] = As<SIMD::Float>(SIMD::Int(routine->subgroupsPerWorkgroup));
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInSubgroupSize, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		value[builtin.FirstComponent] = As<SIMD::Float>(SIMD::Int(routine->invocationsPerSubgroup));
	});

	routine->setImmutableInputBuiltins(shader.get());
}

void ComputeProgram::setSubgroupBuiltins(Pointer<Byte> data, SpirvRoutine *routine, Int workgroupID[3], SIMD::Int localInvocationIndex, Int subgroupIndex)
{
	Int4 numWorkgroups = *Pointer<Int4>(data + OFFSET(Data, numWorkgroups));
	Int4 workgroupSize = *Pointer<Int4>(data + OFFSET(Data, workgroupSize));

	Int workgroupSizeX = workgroupSize.x;
	Int workgroupSizeY = workgroupSize.y;

	SIMD::Int localInvocationID[3];
	{
		SIMD::Int idx = localInvocationIndex;
		localInvocationID[2] = idx / SIMD::Int(workgroupSizeX * workgroupSizeY);
		idx -= localInvocationID[2] * SIMD::Int(workgroupSizeX * workgroupSizeY);  // modulo
		localInvocationID[1] = idx / SIMD::Int(workgroupSizeX);
		idx -= localInvocationID[1] * SIMD::Int(workgroupSizeX);  // modulo
		localInvocationID[0] = idx;
	}

	Int4 wgID = Insert(Insert(Insert(Int4(0), workgroupID[0], 0), workgroupID[1], 1), workgroupID[2], 2);
	auto localBase = workgroupSize * wgID;
	SIMD::Int globalInvocationID[3];
	globalInvocationID[0] = SIMD::Int(Extract(localBase, 0)) + localInvocationID[0];
	globalInvocationID[1] = SIMD::Int(Extract(localBase, 1)) + localInvocationID[1];
	globalInvocationID[2] = SIMD::Int(Extract(localBase, 2)) + localInvocationID[2];

	routine->localInvocationIndex = localInvocationIndex;
	routine->subgroupIndex = subgroupIndex;
	routine->localInvocationID[0] = localInvocationID[0];
	routine->localInvocationID[1] = localInvocationID[1];
	routine->localInvocationID[2] = localInvocationID[2];
	routine->globalInvocationID[0] = globalInvocationID[0];
	routine->globalInvocationID[1] = globalInvocationID[1];
	routine->globalInvocationID[2] = globalInvocationID[2];

	routine->setInputBuiltin(shader.get(), spv::BuiltInLocalInvocationIndex, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		ASSERT(builtin.SizeInComponents == 1);
		value[builtin.FirstComponent] = As<SIMD::Float>(localInvocationIndex);
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInSubgroupId, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		ASSERT(builtin.SizeInComponents == 1);
		value[builtin.FirstComponent] = As<SIMD::Float>(SIMD::Int(subgroupIndex));
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInLocalInvocationId, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		for(uint32_t component = 0; component < builtin.SizeInComponents; component++)
		{
			value[builtin.FirstComponent + component] =
			    As<SIMD::Float>(localInvocationID[component]);
		}
	});

	routine->setInputBuiltin(shader.get(), spv::BuiltInGlobalInvocationId, [&](const Spirv::BuiltinMapping &builtin, Array<SIMD::Float> &value) {
		for(uint32_t component = 0; component < builtin.SizeInComponents; component++)
		{
			value[builtin.FirstComponent + component] =
			    As<SIMD::Float>(globalInvocationID[component]);
		}
	});
}

void ComputeProgram::emit(SpirvRoutine *routine)
{
	Pointer<Byte> device = Arg<0>();
	Pointer<Byte> data = Arg<1>();
	Int workgroupX = Arg<2>();
	Int workgroupY = Arg<3>();
	Int workgroupZ = Arg<4>();
	Pointer<Byte> workgroupMemory = Arg<5>();
	Int firstSubgroup = Arg<6>();
	Int subgroupCount = Arg<7>();

	routine->device = device;
	routine->descriptorSets = data + OFFSET(Data, descriptorSets);
	routine->descriptorDynamicOffsets = data + OFFSET(Data, descriptorDynamicOffsets);
	routine->pushConstants = data + OFFSET(Data, pushConstants);
	routine->constants = device + OFFSET(vk::Device, constants);
	routine->workgroupMemory = workgroupMemory;

	Int invocationsPerWorkgroup = *Pointer<Int>(data + OFFSET(Data, invocationsPerWorkgroup));

	Int workgroupID[3] = { workgroupX, workgroupY, workgroupZ };
	setWorkgroupBuiltins(data, routine, workgroupID);

	For(Int i = 0, i < subgroupCount, i++)
	{
		auto subgroupIndex = firstSubgroup + i;

		// TODO: Replace SIMD::Int(0, 1, 2, 3) with SIMD-width equivalent
		auto localInvocationIndex = SIMD::Int(subgroupIndex * SIMD::Width) + SIMD::Int(0, 1, 2, 3);

		// Disable lanes where (invocationIDs >= invocationsPerWorkgroup)
		auto activeLaneMask = CmpLT(localInvocationIndex, SIMD::Int(invocationsPerWorkgroup));

		setSubgroupBuiltins(data, routine, workgroupID, localInvocationIndex, subgroupIndex);

		shader->emit(routine, activeLaneMask, activeLaneMask, descriptorSets);
	}
}

void ComputeProgram::run(
    const vk::DescriptorSet::Array &descriptorSetObjects,
    const vk::DescriptorSet::Bindings &descriptorSets,
    const vk::DescriptorSet::DynamicOffsets &descriptorDynamicOffsets,
    const vk::Pipeline::PushConstantStorage &pushConstants,
    uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	uint32_t workgroupSizeX = shader->getWorkgroupSizeX();
	uint32_t workgroupSizeY = shader->getWorkgroupSizeY();
	uint32_t workgroupSizeZ = shader->getWorkgroupSizeZ();

	auto invocationsPerSubgroup = SIMD::Width;
	auto invocationsPerWorkgroup = workgroupSizeX * workgroupSizeY * workgroupSizeZ;
	auto subgroupsPerWorkgroup = (invocationsPerWorkgroup + invocationsPerSubgroup - 1) / invocationsPerSubgroup;

	Data data;
	data.descriptorSets = descriptorSets;
	data.descriptorDynamicOffsets = descriptorDynamicOffsets;
	data.numWorkgroups[0] = groupCountX;
	data.numWorkgroups[1] = groupCountY;
	data.numWorkgroups[2] = groupCountZ;
	data.workgroupSize[0] = workgroupSizeX;
	data.workgroupSize[1] = workgroupSizeY;
	data.workgroupSize[2] = workgroupSizeZ;
	data.invocationsPerSubgroup = invocationsPerSubgroup;
	data.invocationsPerWorkgroup = invocationsPerWorkgroup;
	data.subgroupsPerWorkgroup = subgroupsPerWorkgroup;
	data.pushConstants = pushConstants;

	marl::WaitGroup wg;
	constexpr uint32_t batchCount = 16;

	auto groupCount = groupCountX * groupCountY * groupCountZ;

	for(uint32_t batchID = 0; batchID < batchCount && batchID < groupCount; batchID++)
	{
		wg.add(1);
		marl::schedule([this, batchID, groupCount, groupCountX, groupCountY,
		                baseGroupZ, baseGroupY, baseGroupX, wg, subgroupsPerWorkgroup,
		                &data] {
			// Workaround for the fact that some compilers don't allow batchCount to be captured.
			constexpr uint32_t batchCount = 16;
			defer(wg.done());
			std::vector<uint8_t> workgroupMemory(shader->workgroupMemory.size());

			for(uint32_t groupIndex = batchID; groupIndex < groupCount; groupIndex += batchCount)
			{
				auto modulo = groupIndex;
				auto groupOffsetZ = modulo / (groupCountX * groupCountY);
				modulo -= groupOffsetZ * (groupCountX * groupCountY);
				auto groupOffsetY = modulo / groupCountX;
				modulo -= groupOffsetY * groupCountX;
				auto groupOffsetX = modulo;

				auto groupZ = baseGroupZ + groupOffsetZ;
				auto groupY = baseGroupY + groupOffsetY;
				auto groupX = baseGroupX + groupOffsetX;
				MARL_SCOPED_EVENT("groupX: %d, groupY: %d, groupZ: %d", groupX, groupY, groupZ);

				using Coroutine = std::unique_ptr<rr::Stream<SpirvEmitter::YieldResult>>;
				std::queue<Coroutine> coroutines;

				if(shader->getAnalysis().ContainsControlBarriers)
				{
					// Make a function call per subgroup so each subgroup
					// can yield, bringing all subgroups to the barrier
					// together.
					for(uint32_t subgroupIndex = 0; subgroupIndex < subgroupsPerWorkgroup; subgroupIndex++)
					{
						auto coroutine = (*this)(device, &data, groupX, groupY, groupZ, workgroupMemory.data(), subgroupIndex, 1);
						coroutines.push(std::move(coroutine));
					}
				}
				else
				{
					auto coroutine = (*this)(device, &data, groupX, groupY, groupZ, workgroupMemory.data(), 0, subgroupsPerWorkgroup);
					coroutines.push(std::move(coroutine));
				}

				while(coroutines.size() > 0)
				{
					auto coroutine = std::move(coroutines.front());
					coroutines.pop();

					SpirvEmitter::YieldResult result;
					if(coroutine->await(result))
					{
						// TODO: Consider result (when the enum is more than 1 entry).
						coroutines.push(std::move(coroutine));
					}
				}
			}
		});
	}

	wg.wait();

	if(shader->containsImageWrite())
	{
		vk::DescriptorSet::ContentsChanged(descriptorSetObjects, pipelineLayout, device);
	}
}

}  // namespace sw
