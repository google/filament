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

#ifndef sw_VertexRoutine_hpp
#define sw_VertexRoutine_hpp

#include "ShaderCore.hpp"
#include "SpirvShader.hpp"
#include "Device/VertexProcessor.hpp"

namespace vk {
class PipelineLayout;
}

namespace sw {

class VertexRoutinePrototype : public VertexRoutineFunction
{
public:
	VertexRoutinePrototype()
	    : device(Arg<0>())
	    , vertex(Arg<1>())
	    , batch(Arg<2>())
	    , task(Arg<3>())
	    , data(Arg<4>())
	{}
	virtual ~VertexRoutinePrototype() {}

protected:
	Pointer<Byte> device;
	Pointer<Byte> vertex;
	Pointer<UInt> batch;
	Pointer<Byte> task;
	Pointer<Byte> data;
};

class VertexRoutine : public VertexRoutinePrototype
{
public:
	VertexRoutine(
	    const VertexProcessor::State &state,
	    const vk::PipelineLayout *pipelineLayout,
	    const SpirvShader *spirvShader);
	virtual ~VertexRoutine();

	void generate();

protected:
	Pointer<Byte> constants;

	SIMD::Int clipFlags;
	Int cullMask;

	SpirvRoutine routine;

	const VertexProcessor::State &state;
	const SpirvShader *const spirvShader;

private:
	virtual void program(Pointer<UInt> &batch, UInt &vertexCount) = 0;

	typedef VertexProcessor::State::Input Stream;

	Vector4f readStream(Pointer<Byte> &buffer, UInt &stride, const Stream &stream, Pointer<UInt> &batch,
	                    bool robustBufferAccess, UInt &robustnessSize, Int baseVertex);
	void readInput(Pointer<UInt> &batch);
	void computeClipFlags();
	void computeCullMask();
	void writeCache(Pointer<Byte> &vertexCache, Pointer<UInt> &tagCache, Pointer<UInt> &batch);
	void writeVertex(const Pointer<Byte> &vertex, Pointer<Byte> &cacheEntry);
};

}  // namespace sw

#endif  // sw_VertexRoutine_hpp
