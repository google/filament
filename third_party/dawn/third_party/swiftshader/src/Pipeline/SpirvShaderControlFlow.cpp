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

#include "Reactor/Coroutine.hpp"  // rr::Yield

#include "ShaderCore.hpp"

#include <spirv/unified1/spirv.hpp>

#include <queue>

#include <fstream>
#include <iostream>

namespace sw {

Spirv::Block::Block(InsnIterator begin, InsnIterator end)
    : begin_(begin)
    , end_(end)
{
	// Default to a Simple, this may change later.
	kind = Block::Simple;

	// Walk the instructions to find the last two of the block.
	InsnIterator insns[2];
	for(auto insn : *this)
	{
		insns[0] = insns[1];
		insns[1] = insn;
	}

	switch(insns[1].opcode())
	{
	case spv::OpBranch:
		branchInstruction = insns[1];
		outs.emplace(Block::ID(branchInstruction.word(1)));

		switch(insns[0].opcode())
		{
		case spv::OpLoopMerge:
			kind = Loop;
			mergeInstruction = insns[0];
			mergeBlock = Block::ID(mergeInstruction.word(1));
			continueTarget = Block::ID(mergeInstruction.word(2));
			break;

		default:
			kind = Block::Simple;
			break;
		}
		break;

	case spv::OpBranchConditional:
		branchInstruction = insns[1];
		outs.emplace(Block::ID(branchInstruction.word(2)));
		outs.emplace(Block::ID(branchInstruction.word(3)));

		switch(insns[0].opcode())
		{
		case spv::OpSelectionMerge:
			kind = StructuredBranchConditional;
			mergeInstruction = insns[0];
			mergeBlock = Block::ID(mergeInstruction.word(1));
			break;

		case spv::OpLoopMerge:
			kind = Loop;
			mergeInstruction = insns[0];
			mergeBlock = Block::ID(mergeInstruction.word(1));
			continueTarget = Block::ID(mergeInstruction.word(2));
			break;

		default:
			kind = UnstructuredBranchConditional;
			break;
		}
		break;

	case spv::OpSwitch:
		branchInstruction = insns[1];
		outs.emplace(Block::ID(branchInstruction.word(2)));
		for(uint32_t w = 4; w < branchInstruction.wordCount(); w += 2)
		{
			outs.emplace(Block::ID(branchInstruction.word(w)));
		}

		switch(insns[0].opcode())
		{
		case spv::OpSelectionMerge:
			kind = StructuredSwitch;
			mergeInstruction = insns[0];
			mergeBlock = Block::ID(mergeInstruction.word(1));
			break;

		default:
			kind = UnstructuredSwitch;
			break;
		}
		break;

	default:
		break;
	}
}

void Spirv::Function::TraverseReachableBlocks(Block::ID id, Block::Set &reachable) const
{
	if(reachable.count(id) == 0)
	{
		reachable.emplace(id);
		for(auto out : getBlock(id).outs)
		{
			TraverseReachableBlocks(out, reachable);
		}
	}
}

void Spirv::Function::AssignBlockFields()
{
	Block::Set reachable;
	TraverseReachableBlocks(entry, reachable);

	for(auto &it : blocks)
	{
		auto &blockId = it.first;
		auto &block = it.second;
		if(reachable.count(blockId) > 0)
		{
			for(auto &outId : it.second.outs)
			{
				auto outIt = blocks.find(outId);
				ASSERT_MSG(outIt != blocks.end(), "Block %d has a non-existent out %d", blockId.value(), outId.value());
				auto &out = outIt->second;
				out.ins.emplace(blockId);
			}
			if(block.kind == Block::Loop)
			{
				auto mergeIt = blocks.find(block.mergeBlock);
				ASSERT_MSG(mergeIt != blocks.end(), "Loop block %d has a non-existent merge block %d", blockId.value(), block.mergeBlock.value());
				mergeIt->second.isLoopMerge = true;
			}
		}
	}
}

void Spirv::Function::ForeachBlockDependency(Block::ID blockId, std::function<void(Block::ID)> f) const
{
	auto block = getBlock(blockId);
	for(auto dep : block.ins)
	{
		if(block.kind != Block::Loop ||                  // if not a loop...
		   !ExistsPath(blockId, dep, block.mergeBlock))  // or a loop and not a loop back edge
		{
			f(dep);
		}
	}
}

bool Spirv::Function::ExistsPath(Block::ID from, Block::ID to, Block::ID notPassingThrough) const
{
	// TODO: Optimize: This can be cached on the block.
	Block::Set seen;
	seen.emplace(notPassingThrough);

	std::queue<Block::ID> pending;
	pending.emplace(from);

	while(pending.size() > 0)
	{
		auto id = pending.front();
		pending.pop();
		for(auto out : getBlock(id).outs)
		{
			if(seen.count(out) != 0) { continue; }
			if(out == to) { return true; }
			pending.emplace(out);
		}
		seen.emplace(id);
	}

	return false;
}

void SpirvEmitter::addOutputActiveLaneMaskEdge(Block::ID to, RValue<SIMD::Int> mask)
{
	addActiveLaneMaskEdge(block, to, mask & activeLaneMask());
}

void SpirvEmitter::addActiveLaneMaskEdge(Block::ID from, Block::ID to, RValue<SIMD::Int> mask)
{
	auto edge = Block::Edge{ from, to };
	auto it = edgeActiveLaneMasks.find(edge);
	if(it == edgeActiveLaneMasks.end())
	{
		edgeActiveLaneMasks.emplace(edge, mask);
	}
	else
	{
		auto combined = it->second | mask;
		edgeActiveLaneMasks.erase(edge);
		edgeActiveLaneMasks.emplace(edge, combined);
	}
}

RValue<SIMD::Int> SpirvEmitter::GetActiveLaneMaskEdge(Block::ID from, Block::ID to) const
{
	auto edge = Block::Edge{ from, to };
	auto it = edgeActiveLaneMasks.find(edge);
	ASSERT_MSG(it != edgeActiveLaneMasks.end(), "Could not find edge %d -> %d", from.value(), to.value());
	return it->second;
}

void SpirvEmitter::EmitBlocks(Block::ID id, Block::ID ignore /* = 0 */)
{
	auto oldPending = this->pending;
	auto &function = shader.getFunction(this->function);

	std::deque<Block::ID> pending;
	this->pending = &pending;
	pending.push_front(id);
	while(pending.size() > 0)
	{
		auto id = pending.front();

		const auto &block = function.getBlock(id);
		if(id == ignore)
		{
			pending.pop_front();
			continue;
		}

		// Ensure all dependency blocks have been generated.
		auto depsDone = true;
		function.ForeachBlockDependency(id, [&](Block::ID dep) {
			if(visited.count(dep) == 0)
			{
				this->pending->push_front(dep);
				depsDone = false;
			}
		});

		if(!depsDone)
		{
			continue;
		}

		pending.pop_front();

		this->block = id;

		switch(block.kind)
		{
		case Block::Simple:
		case Block::StructuredBranchConditional:
		case Block::UnstructuredBranchConditional:
		case Block::StructuredSwitch:
		case Block::UnstructuredSwitch:
			EmitNonLoop();
			break;

		case Block::Loop:
			EmitLoop();
			break;

		default:
			UNREACHABLE("Unexpected Block Kind: %d", int(block.kind));
		}
	}

	this->pending = oldPending;
}

void SpirvEmitter::EmitNonLoop()
{
	auto &function = shader.getFunction(this->function);
	auto blockId = block;
	auto block = function.getBlock(blockId);

	if(!visited.emplace(blockId).second)
	{
		return;  // Already generated this block.
	}

	if(blockId != function.entry)
	{
		// Set the activeLaneMask.
		SIMD::Int activeLaneMask(0);
		for(auto in : block.ins)
		{
			auto inMask = GetActiveLaneMaskEdge(in, blockId);
			SPIRV_SHADER_DBG("Block {0} -> {1} mask: {2}", in, blockId, inMask);
			activeLaneMask |= inMask;
		}
		SPIRV_SHADER_DBG("Block {0} mask: {1}", blockId, activeLaneMask);
		SetActiveLaneMask(activeLaneMask);
	}

	EmitInstructions(block.begin(), block.end());

	for(auto out : block.outs)
	{
		if(visited.count(out) == 0)
		{
			pending->push_back(out);
		}
	}

	SPIRV_SHADER_DBG("Block {0} done", blockId);
}

void SpirvEmitter::EmitLoop()
{
	auto &function = shader.getFunction(this->function);
	auto blockId = block;
	auto &block = function.getBlock(blockId);
	auto mergeBlockId = block.mergeBlock;
	auto &mergeBlock = function.getBlock(mergeBlockId);

	if(!visited.emplace(blockId).second)
	{
		return;  // Already emitted this loop.
	}

	SPIRV_SHADER_DBG("*** LOOP HEADER ***");

	// Gather all the blocks that make up the loop.
	std::unordered_set<Block::ID> loopBlocks;
	loopBlocks.emplace(block.mergeBlock);  // Stop traversal at mergeBlock.
	function.TraverseReachableBlocks(blockId, loopBlocks);

	// incomingBlocks are block ins that are not back-edges.
	std::unordered_set<Block::ID> incomingBlocks;
	for(auto in : block.ins)
	{
		if(loopBlocks.count(in) == 0)
		{
			incomingBlocks.emplace(in);
		}
	}

	// Emit the loop phi instructions, and initialize them with a value from
	// the incoming blocks.
	for(auto insn = block.begin(); insn != block.mergeInstruction; insn++)
	{
		if(insn.opcode() == spv::OpPhi)
		{
			StorePhi(blockId, insn, incomingBlocks);
		}
	}

	// loopActiveLaneMask is the mask of lanes that are continuing to loop.
	// This is initialized with the incoming active lane masks.
	SIMD::Int loopActiveLaneMask = SIMD::Int(0);
	for(auto in : incomingBlocks)
	{
		loopActiveLaneMask |= GetActiveLaneMaskEdge(in, blockId);
	}

	// mergeActiveLaneMasks contains edge lane masks for the merge block.
	// This is the union of all edge masks across all iterations of the loop.
	std::unordered_map<Block::ID, SIMD::Int> mergeActiveLaneMasks;
	for(auto in : function.getBlock(mergeBlockId).ins)
	{
		mergeActiveLaneMasks.emplace(in, SIMD::Int(0));
	}

	// Create the loop basic blocks
	auto headerBasicBlock = Nucleus::createBasicBlock();
	auto mergeBasicBlock = Nucleus::createBasicBlock();

	// Start emitting code inside the loop.
	Nucleus::createBr(headerBasicBlock);
	Nucleus::setInsertBlock(headerBasicBlock);

	SPIRV_SHADER_DBG("*** LOOP START (mask: {0}) ***", loopActiveLaneMask);

	// Load the active lane mask.
	SetActiveLaneMask(loopActiveLaneMask);

	// Emit the non-phi loop header block's instructions.
	for(auto insn = block.begin(); insn != block.end(); insn++)
	{
		if(insn.opcode() == spv::OpPhi)
		{
			LoadPhi(insn);
		}
		else
		{
			EmitInstruction(insn);
		}
	}

	// Emit all blocks between the loop header and the merge block, but
	// don't emit the merge block yet.
	for(auto out : block.outs)
	{
		EmitBlocks(out, mergeBlockId);
	}

	// Restore current block id after emitting loop blocks.
	this->block = blockId;

	// Rebuild the loopActiveLaneMask from the loop back edges.
	loopActiveLaneMask = SIMD::Int(0);
	for(auto in : block.ins)
	{
		if(function.ExistsPath(blockId, in, mergeBlockId))
		{
			loopActiveLaneMask |= GetActiveLaneMaskEdge(in, blockId);
		}
	}

	// Add active lanes to the merge lane mask.
	for(auto in : function.getBlock(mergeBlockId).ins)
	{
		auto edge = Block::Edge{ in, mergeBlockId };
		auto it = edgeActiveLaneMasks.find(edge);

		if(it != edgeActiveLaneMasks.end())
		{
			mergeActiveLaneMasks[in] |= it->second;
		}
	}

	// Update loop phi values.
	for(auto insn = block.begin(); insn != block.mergeInstruction; insn++)
	{
		if(insn.opcode() == spv::OpPhi)
		{
			StorePhi(blockId, insn, loopBlocks);
		}
	}

	SPIRV_SHADER_DBG("*** LOOP END (mask: {0}) ***", loopActiveLaneMask);

	// Use the [loop -> merge] active lane masks to update the phi values in
	// the merge block. We need to do this to handle divergent control flow
	// in the loop.
	//
	// Consider the following:
	//
	//     int phi_source = 0;
	//     for(uint i = 0; i < 4; i++)
	//     {
	//         phi_source = 0;
	//         if(gl_GlobalInvocationID.x % 4 == i) // divergent control flow
	//         {
	//             phi_source = 42; // single lane assignment.
	//             break; // activeLaneMask for [loop->merge] is active for a single lane.
	//         }
	//         // -- we are here --
	//     }
	//     // merge block
	//     int phi = phi_source; // OpPhi
	//
	// In this example, with each iteration of the loop, phi_source will
	// only have a single lane assigned. However the 'phi' value in the merge
	// block needs to be assigned the union of all the per-lane assignments
	// of phi_source when that lane exited the loop.
	for(auto insn = mergeBlock.begin(); insn != mergeBlock.end(); insn++)
	{
		if(insn.opcode() == spv::OpPhi)
		{
			StorePhi(mergeBlockId, insn, loopBlocks);
		}
	}

	// Loop body now done.
	// If any lanes are still active, jump back to the loop header,
	// otherwise jump to the merge block.
	Nucleus::createCondBr(AnyTrue(loopActiveLaneMask).value(), headerBasicBlock, mergeBasicBlock);

	// Continue emitting from the merge block.
	Nucleus::setInsertBlock(mergeBasicBlock);
	pending->push_back(mergeBlockId);

	for(const auto &it : mergeActiveLaneMasks)
	{
		addActiveLaneMaskEdge(it.first, mergeBlockId, it.second);
	}
}

void SpirvEmitter::EmitBranch(InsnIterator insn)
{
	auto target = Block::ID(insn.word(1));
	addActiveLaneMaskEdge(block, target, activeLaneMask());
}

void SpirvEmitter::EmitBranchConditional(InsnIterator insn)
{
	auto &function = shader.getFunction(this->function);
	auto block = function.getBlock(this->block);
	ASSERT(block.branchInstruction == insn);

	auto condId = Object::ID(block.branchInstruction.word(1));
	auto trueBlockId = Block::ID(block.branchInstruction.word(2));
	auto falseBlockId = Block::ID(block.branchInstruction.word(3));

	auto cond = Operand(shader, *this, condId);
	ASSERT_MSG(shader.getObjectType(condId).componentCount == 1, "Condition must be a Boolean type scalar");

	// TODO: Optimize for case where all lanes take same path.

	addOutputActiveLaneMaskEdge(trueBlockId, cond.Int(0));
	addOutputActiveLaneMaskEdge(falseBlockId, ~cond.Int(0));
}

void SpirvEmitter::EmitSwitch(InsnIterator insn)
{
	auto &function = shader.getFunction(this->function);
	auto block = function.getBlock(this->block);
	ASSERT(block.branchInstruction == insn);

	auto selId = Object::ID(block.branchInstruction.word(1));

	auto sel = Operand(shader, *this, selId);
	ASSERT_MSG(sel.componentCount == 1, "Selector must be a scalar");
	SPIRV_SHADER_DBG("switch({0})", sel);

	auto numCases = (block.branchInstruction.wordCount() - 3) / 2;

	// TODO: Optimize for case where all lanes take same path.

	SIMD::Int defaultLaneMask = activeLaneMask();

	// Gather up the case label matches and calculate defaultLaneMask.
	std::vector<RValue<SIMD::Int>> caseLabelMatches;
	caseLabelMatches.reserve(numCases);

	for(uint32_t i = 0; i < numCases; i++)
	{
		auto label = block.branchInstruction.word(i * 2 + 3);
		auto caseBlockId = Block::ID(block.branchInstruction.word(i * 2 + 4));
		auto caseLabelMatch = CmpEQ(sel.Int(0), SIMD::Int(label));
		SPIRV_SHADER_DBG("case {0}: {1}", label, caseLabelMatch & activeLaneMask());
		addOutputActiveLaneMaskEdge(caseBlockId, caseLabelMatch);
		defaultLaneMask &= ~caseLabelMatch;
	}

	auto defaultBlockId = Block::ID(block.branchInstruction.word(2));
	SPIRV_SHADER_DBG("default: {0}", defaultLaneMask);
	addOutputActiveLaneMaskEdge(defaultBlockId, defaultLaneMask);
}

void SpirvEmitter::EmitUnreachable(InsnIterator insn)
{
	// TODO: Log something in this case?
	SetActiveLaneMask(SIMD::Int(0));
}

void SpirvEmitter::EmitReturn(InsnIterator insn)
{
	SetActiveLaneMask(SIMD::Int(0));
}

void SpirvEmitter::EmitTerminateInvocation(InsnIterator insn)
{
	routine->discardMask |= SignMask(activeLaneMask());
	SetActiveLaneMask(SIMD::Int(0));
}

void SpirvEmitter::EmitDemoteToHelperInvocation(InsnIterator insn)
{
	routine->helperInvocation |= activeLaneMask();
	routine->discardMask |= SignMask(activeLaneMask());
	SetStoresAndAtomicsMask(storesAndAtomicsMask() & ~activeLaneMask());
}

void SpirvEmitter::EmitIsHelperInvocation(InsnIterator insn)
{
	auto &type = shader.getType(insn.resultTypeId());
	auto &dst = createIntermediate(insn.resultId(), type.componentCount);
	dst.move(0, routine->helperInvocation);
}

void SpirvEmitter::EmitFunctionCall(InsnIterator insn)
{
	auto functionId = Spirv::Function::ID(insn.word(3));
	const auto &functionIt = shader.functions.find(functionId);
	ASSERT(functionIt != shader.functions.end());
	auto &function = functionIt->second;

	// TODO(b/141246700): Add full support for spv::OpFunctionCall
	// The only supported function is a single OpKill wrapped in a
	// function, as a result of the "wrap OpKill" SPIRV-Tools pass
	ASSERT(function.blocks.size() == 1);
	spv::Op wrapOpKill[] = { spv::OpLabel, spv::OpKill };

	for(const auto &block : function.blocks)
	{
		int insnNumber = 0;
		for(auto blockInsn : block.second)
		{
			if(insnNumber > 1)
			{
				UNIMPLEMENTED("b/141246700: Function block number of instructions: %d", insnNumber);  // FIXME(b/141246700)
			}

			if(blockInsn.opcode() != wrapOpKill[insnNumber++])
			{
				UNIMPLEMENTED("b/141246700: Function block instruction %d : %s", insnNumber - 1, shader.OpcodeName(blockInsn.opcode()));  // FIXME(b/141246700)
			}

			if(blockInsn.opcode() == spv::OpKill)
			{
				EmitInstruction(blockInsn);
			}
		}
	}
}

void SpirvEmitter::EmitControlBarrier(InsnIterator insn)
{
	auto executionScope = spv::Scope(shader.GetConstScalarInt(insn.word(1)));
	auto semantics = spv::MemorySemanticsMask(shader.GetConstScalarInt(insn.word(3)));
	// TODO(b/176819536): We probably want to consider the memory scope here.
	// For now, just always emit the full fence.
	Fence(semantics);

	switch(executionScope)
	{
	case spv::ScopeWorkgroup:
		Yield(YieldResult::ControlBarrier);
		break;
	case spv::ScopeSubgroup:
		break;
	default:
		// See Vulkan 1.1 spec, Appendix A, Validation Rules within a Module.
		UNREACHABLE("Scope for execution must be limited to Workgroup or Subgroup");
		break;
	}
}

void SpirvEmitter::EmitPhi(InsnIterator insn)
{
	auto &function = shader.getFunction(this->function);
	auto currentBlock = function.getBlock(block);

	if(!currentBlock.isLoopMerge)
	{
		// If this is a loop merge block, then don't attempt to update the
		// phi values from the ins. EmitLoop() has had to take special care
		// of this phi in order to correctly deal with divergent lanes.
		StorePhi(block, insn, currentBlock.ins);
	}

	LoadPhi(insn);
}

void SpirvEmitter::LoadPhi(InsnIterator insn)
{
	auto typeId = Type::ID(insn.word(1));
	auto type = shader.getType(typeId);
	auto objectId = Object::ID(insn.word(2));

	auto storageIt = phis.find(objectId);
	ASSERT(storageIt != phis.end());
	const auto &storage = storageIt->second;

	auto &dst = createIntermediate(objectId, type.componentCount);

	for(uint32_t i = 0; i < type.componentCount; i++)
	{
		dst.move(i, storage[i]);
		SPIRV_SHADER_DBG("LoadPhi({0}.{1}): {2}", objectId, i, storage[i]);
	}
}

void SpirvEmitter::StorePhi(Block::ID currentBlock, InsnIterator insn, const std::unordered_set<Block::ID> &filter)
{
	auto typeId = Type::ID(insn.word(1));
	auto type = shader.getType(typeId);
	auto objectId = Object::ID(insn.word(2));

	auto storageIt = phis.find(objectId);
	ASSERT(storageIt != phis.end());
	auto &storage = storageIt->second;

	for(uint32_t w = 3; w < insn.wordCount(); w += 2)
	{
		auto varId = Object::ID(insn.word(w + 0));
		auto blockId = Block::ID(insn.word(w + 1));

		if(filter.count(blockId) == 0)
		{
			continue;
		}

		auto mask = GetActiveLaneMaskEdge(blockId, currentBlock);
		auto in = Operand(shader, *this, varId);

		for(uint32_t i = 0; i < type.componentCount; i++)
		{
			storage[i] = As<SIMD::Float>((As<SIMD::Int>(storage[i]) & ~mask) | (in.Int(i) & mask));
			SPIRV_SHADER_DBG("StorePhi({0}.{1}): [{2} <- {3}] {4}: {5}, mask: {6}",
			                 objectId, i, currentBlock, blockId, varId, in.UInt(i), mask);
		}
	}

	for(uint32_t i = 0; i < type.componentCount; i++)
	{
		SPIRV_SHADER_DBG("StorePhi({0}.{1}): {2}", objectId, i, As<SIMD::UInt>(storage[i]));
	}
}

void SpirvEmitter::Yield(YieldResult res) const
{
	rr::Yield(RValue<Int>(int(res)));
}

void SpirvEmitter::SetActiveLaneMask(RValue<SIMD::Int> mask)
{
	activeLaneMaskValue = mask.value();
}

void SpirvEmitter::SetStoresAndAtomicsMask(RValue<SIMD::Int> mask)
{
	storesAndAtomicsMaskValue = mask.value();
}

void Spirv::WriteCFGGraphVizDotFile(const char *path) const
{
	std::ofstream file(path);
	file << "digraph D {" << std::endl;
	for(auto &func : functions)
	{
		file << "  subgraph cluster_function_" << func.first.value() << " {"
		     << std::endl;

		file << "    label = \"function<" << func.first.value() << ">"
		     << (func.first == entryPoint ? " (entry point)" : "")
		     << "\"" << std::endl;

		for(auto &block : func.second.blocks)
		{
			file << "    block_" << block.first.value() << " ["
			     << "shape=circle "
			     << "label=\"" << block.first.value() << "\""
			     << "]" << std::endl;
		}
		file << std::endl;
		for(auto &block : func.second.blocks)
		{
			file << "    block_" << block.first.value() << " -> {";
			bool first = true;
			for(auto outs : block.second.outs)
			{
				if(!first) { file << ", "; }
				file << "block_" << outs.value();
				first = false;
			}
			file << "}" << std::endl;
		}
		file << std::endl;
		for(auto &block : func.second.blocks)
		{
			if(block.second.kind == Block::Loop)
			{
				if(block.second.mergeBlock != 0)
				{
					file << "    block_" << block.first.value() << " -> "
					     << "block_" << block.second.mergeBlock.value()
					     << "[label=\"M\" style=dashed color=blue]"
					     << std::endl;
				}

				if(block.second.continueTarget != 0)
				{
					file << "    block_" << block.first.value() << " -> "
					     << "block_" << block.second.continueTarget.value()
					     << "[label=\"C\" style=dashed color=green]"
					     << std::endl;
				}
			}
		}

		file << "  }" << std::endl;
	}

	for(auto &func : functions)
	{
		for(auto &block : func.second.blocks)
		{
			for(auto insn : block.second)
			{
				if(insn.opcode() == spv::OpFunctionCall)
				{
					auto target = getFunction(insn.word(3)).entry;
					file << "    block_" << block.first.value() << " -> "
					     << "block_" << target.value()
					     << "[color=\"#00008050\"]"
					     << std::endl;
				}
			}
		}
	}

	file << "}" << std::endl;
}

}  // namespace sw
