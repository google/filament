//
// Copyright (C) 2016-2018 Google, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

//
// Post-processing for SPIR-V IR, in internal form, not standard binary form.
//

#include <cassert>
#include <cstdlib>

#include <unordered_set>
#include <algorithm>

#include "SpvBuilder.h"

#include "spirv.hpp"
#include "GlslangToSpv.h"
#include "SpvBuilder.h"
namespace spv {
    #include "GLSL.std.450.h"
    #include "GLSL.ext.KHR.h"
    #include "GLSL.ext.EXT.h"
#ifdef AMD_EXTENSIONS
    #include "GLSL.ext.AMD.h"
#endif
#ifdef NV_EXTENSIONS
    #include "GLSL.ext.NV.h"
#endif
}

namespace spv {

// Called for each instruction that resides in a block.
void Builder::postProcess(Instruction& inst)
{
    // Add capabilities based simply on the opcode.
    switch (inst.getOpCode()) {
    case OpExtInst:
        switch (inst.getImmediateOperand(1)) {
        case GLSLstd450InterpolateAtCentroid:
        case GLSLstd450InterpolateAtSample:
        case GLSLstd450InterpolateAtOffset:
            addCapability(CapabilityInterpolationFunction);
            break;
        default:
            break;
        }
        break;
    case OpDPdxFine:
    case OpDPdyFine:
    case OpFwidthFine:
    case OpDPdxCoarse:
    case OpDPdyCoarse:
    case OpFwidthCoarse:
        addCapability(CapabilityDerivativeControl);
        break;

    case OpImageQueryLod:
    case OpImageQuerySize:
    case OpImageQuerySizeLod:
    case OpImageQuerySamples:
    case OpImageQueryLevels:
        addCapability(CapabilityImageQuery);
        break;

#ifdef NV_EXTENSIONS
    case OpGroupNonUniformPartitionNV:
        addExtension(E_SPV_NV_shader_subgroup_partitioned);
        addCapability(CapabilityGroupNonUniformPartitionedNV);
        break;
#endif

    default:
        break;
    }
}

// Called for each instruction in a reachable block.
void Builder::postProcessReachable(Instruction& inst)
{
    // did have code here, but questionable to do so without deleting the instructions
}

// comment in header
void Builder::postProcess()
{
    std::unordered_set<const Block*> reachableBlocks;
    std::unordered_set<Id> unreachableDefinitions;
    // Collect IDs defined in unreachable blocks. For each function, label the
    // reachable blocks first. Then for each unreachable block, collect the
    // result IDs of the instructions in it.
    for (auto fi = module.getFunctions().cbegin(); fi != module.getFunctions().cend(); fi++) {
        Function* f = *fi;
        Block* entry = f->getEntryBlock();
        inReadableOrder(entry, [&reachableBlocks](const Block* b) { reachableBlocks.insert(b); });
        for (auto bi = f->getBlocks().cbegin(); bi != f->getBlocks().cend(); bi++) {
            Block* b = *bi;
            if (reachableBlocks.count(b) == 0) {
                for (auto ii = b->getInstructions().cbegin(); ii != b->getInstructions().cend(); ii++)
                    unreachableDefinitions.insert(ii->get()->getResultId());
            }
        }
    }

    // Remove unneeded decorations, for unreachable instructions
    decorations.erase(std::remove_if(decorations.begin(), decorations.end(),
        [&unreachableDefinitions](std::unique_ptr<Instruction>& I) -> bool {
            Id decoration_id = I.get()->getIdOperand(0);
            return unreachableDefinitions.count(decoration_id) != 0;
        }),
        decorations.end());

    // Add per-instruction capabilities, extensions, etc.,

    // process all reachable instructions...
    for (auto bi = reachableBlocks.cbegin(); bi != reachableBlocks.cend(); ++bi) {
        const Block* block = *bi;
        const auto function = [this](const std::unique_ptr<Instruction>& inst) { postProcessReachable(*inst.get()); };
        std::for_each(block->getInstructions().begin(), block->getInstructions().end(), function);
    }

    // process all block-contained instructions
    for (auto fi = module.getFunctions().cbegin(); fi != module.getFunctions().cend(); fi++) {
        Function* f = *fi;
        for (auto bi = f->getBlocks().cbegin(); bi != f->getBlocks().cend(); bi++) {
            Block* b = *bi;
            for (auto ii = b->getInstructions().cbegin(); ii != b->getInstructions().cend(); ii++)
                postProcess(*ii->get());
        }
    }
}

}; // end spv namespace
