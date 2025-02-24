// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "spirv-tools/libspirv.h"

#include <spirv/unified1/spirv.hpp>

namespace sw {

const char *Spirv::OpcodeName(spv::Op opcode)
{
	return spvOpcodeString(opcode);
}

// This function is used by the shader debugger to determine whether an instruction is steppable.
bool Spirv::IsStatement(spv::Op opcode)
{
	switch(opcode)
	{
	default:
		// Most statement-like instructions produce a result which has a type.
		// Note OpType* instructions have a result but it is a type itself.
		{
			bool hasResult = false;
			bool hasResultType = false;
			spv::HasResultAndType(opcode, &hasResult, &hasResultType);

			return hasResult && hasResultType;
		}
		break;

	// Instructions without a result but potential side-effects.
	case spv::OpNop:
	case spv::OpStore:
	case spv::OpCopyMemory:
	case spv::OpCopyMemorySized:
	case spv::OpImageWrite:
	case spv::OpEmitVertex:
	case spv::OpEndPrimitive:
	case spv::OpEmitStreamVertex:
	case spv::OpEndStreamPrimitive:
	case spv::OpControlBarrier:
	case spv::OpMemoryBarrier:
	case spv::OpAtomicStore:
	case spv::OpBranch:
	case spv::OpBranchConditional:
	case spv::OpSwitch:
	case spv::OpKill:
	case spv::OpReturn:
	case spv::OpReturnValue:
	case spv::OpLifetimeStart:
	case spv::OpLifetimeStop:
	case spv::OpGroupWaitEvents:
	case spv::OpCommitReadPipe:
	case spv::OpCommitWritePipe:
	case spv::OpGroupCommitReadPipe:
	case spv::OpGroupCommitWritePipe:
	case spv::OpRetainEvent:
	case spv::OpReleaseEvent:
	case spv::OpSetUserEventStatus:
	case spv::OpCaptureEventProfilingInfo:
	case spv::OpAtomicFlagClear:
	case spv::OpMemoryNamedBarrier:
	case spv::OpTerminateInvocation:
	case spv::OpTraceRayKHR:
	case spv::OpExecuteCallableKHR:
	case spv::OpIgnoreIntersectionKHR:
	case spv::OpTerminateRayKHR:
	case spv::OpRayQueryInitializeKHR:
	case spv::OpRayQueryTerminateKHR:
	case spv::OpRayQueryGenerateIntersectionKHR:
	case spv::OpRayQueryConfirmIntersectionKHR:
	case spv::OpBeginInvocationInterlockEXT:
	case spv::OpEndInvocationInterlockEXT:
	case spv::OpDemoteToHelperInvocationEXT:
	case spv::OpAssumeTrueKHR:
		return true;
	}
}

bool Spirv::IsTerminator(spv::Op opcode)
{
	switch(opcode)
	{
	// Branch instructions
	case spv::OpBranch:
	case spv::OpBranchConditional:
	case spv::OpSwitch:
	// Function termination instructions
	case spv::OpReturn:
	case spv::OpReturnValue:
	case spv::OpKill:
	case spv::OpUnreachable:
	case spv::OpTerminateInvocation:
		return true;
	default:
		return false;
	}
}

}  // namespace sw