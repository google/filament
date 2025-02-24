// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_VertexProgram_hpp
#define sw_VertexProgram_hpp

#include "ShaderCore.hpp"
#include "VertexRoutine.hpp"

namespace sw {

struct Stream;

class VertexProgram : public VertexRoutine
{
public:
	VertexProgram(
	    const VertexProcessor::State &state,
	    const vk::PipelineLayout *pipelineLayout,
	    const SpirvShader *spirvShader,
	    const vk::DescriptorSet::Bindings &descriptorSets);

	virtual ~VertexProgram();

private:
	void program(Pointer<UInt> &batch, UInt &vertexCount) override;

	const vk::DescriptorSet::Bindings &descriptorSets;
};

}  // namespace sw

#endif  // sw_VertexProgram_hpp
