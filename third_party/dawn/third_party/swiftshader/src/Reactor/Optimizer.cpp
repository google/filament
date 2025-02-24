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

#include "Optimizer.hpp"

#include "src/IceCfg.h"
#include "src/IceCfgNode.h"

#include <unordered_map>
#include <vector>

namespace {

class Optimizer
{
public:
	Optimizer(rr::Nucleus::OptimizerReport *report)
	    : report(report)
	{
	}

	void run(Ice::Cfg *function);

private:
	void analyzeUses(Ice::Cfg *function);

	void eliminateDeadCode();
	void eliminateUnitializedLoads();
	void propagateAlloca();
	void performScalarReplacementOfAggregates();
	void optimizeSingleBasicBlockLoadsStores();

	void replace(Ice::Inst *instruction, Ice::Operand *newValue);
	void deleteInstruction(Ice::Inst *instruction);
	bool isDead(Ice::Inst *instruction);
	bool isStaticallyIndexedArray(Ice::Operand *allocaAddress);
	Ice::InstAlloca *allocaOf(Ice::Operand *address);

	static const Ice::InstIntrinsic *asLoadSubVector(const Ice::Inst *instruction);
	static const Ice::InstIntrinsic *asStoreSubVector(const Ice::Inst *instruction);
	static bool isLoad(const Ice::Inst &instruction);
	static bool isStore(const Ice::Inst &instruction);
	static bool loadTypeMatchesStore(const Ice::Inst *load, const Ice::Inst *store);
	static bool storeTypeMatchesStore(const Ice::Inst *store1, const Ice::Inst *store2);

	void collectDiagnostics();

	Ice::Cfg *function;
	Ice::GlobalContext *context;

	struct Uses : std::vector<Ice::Inst *>
	{
		bool areOnlyLoadStore() const;
		void insert(Ice::Operand *value, Ice::Inst *instruction);
		void erase(Ice::Inst *instruction);

		std::vector<Ice::Inst *> loads;
		std::vector<Ice::Inst *> stores;
	};

	struct LoadStoreInst
	{
		LoadStoreInst(Ice::Inst *inst, bool isStore)
		    : inst(inst)
		    , address(isStore ? inst->getStoreAddress() : inst->getLoadAddress())
		    , isStore(isStore)
		{
		}

		Ice::Inst *inst;
		Ice::Operand *address;
		bool isStore;
	};

	Optimizer::Uses *getUses(Ice::Operand *);
	void setUses(Ice::Operand *, Optimizer::Uses *);
	bool hasUses(Ice::Operand *) const;

	Ice::Inst *getDefinition(Ice::Variable *);
	void setDefinition(Ice::Variable *, Ice::Inst *);

	std::vector<Ice::Operand *> operandsWithUses;

	rr::Nucleus::OptimizerReport *report = nullptr;
};

void Optimizer::run(Ice::Cfg *function)
{
	this->function = function;
	this->context = function->getContext();

	analyzeUses(function);

	// Start by eliminating any dead code, to avoid redundant work for the
	// subsequent optimization passes.
	eliminateDeadCode();
	eliminateUnitializedLoads();

	// Eliminate allocas which store the address of other allocas.
	propagateAlloca();

	// Replace arrays with individual elements if only statically indexed.
	performScalarReplacementOfAggregates();

	// Iterate through basic blocks to propagate loads following stores.
	optimizeSingleBasicBlockLoadsStores();

	for(auto operand : operandsWithUses)
	{
		// Deletes the Uses instance on the operand
		setUses(operand, nullptr);
	}
	operandsWithUses.clear();

	collectDiagnostics();
}

// Eliminates allocas which store the address of other allocas.
void Optimizer::propagateAlloca()
{
	Ice::CfgNode *entryBlock = function->getEntryNode();
	Ice::InstList &instList = entryBlock->getInsts();

	for(Ice::Inst &inst : instList)
	{
		if(inst.isDeleted())
		{
			continue;
		}

		auto *alloca = llvm::dyn_cast<Ice::InstAlloca>(&inst);

		if(!alloca)
		{
			break;  // Allocas are all at the top
		}

		// Look for stores of this alloca's address.
		Ice::Operand *address = alloca->getDest();
		Uses uses = *getUses(address);  // Hard copy

		for(auto *store : uses)
		{
			if(isStore(*store) && store->getData() == address)
			{
				Ice::Operand *dest = store->getStoreAddress();
				Ice::Variable *destVar = llvm::dyn_cast<Ice::Variable>(dest);
				Ice::Inst *def = destVar ? getDefinition(destVar) : nullptr;

				// If the address is stored into another stack variable, eliminate the latter.
				if(def && def->getKind() == Ice::Inst::Alloca)
				{
					Uses destUses = *getUses(dest);  // Hard copy

					// Make sure the only store into the stack variable is this address, and that all of its other uses are loads.
					// This prevents dynamic array loads/stores to be replaced by a scalar.
					if((destUses.stores.size() == 1) && (destUses.loads.size() == destUses.size() - 1))
					{
						for(auto *load : destUses.loads)
						{
							replace(load, address);
						}

						// The address is now only stored, never loaded, so the store can be eliminated, together with its alloca.
						assert(getUses(dest)->size() == 1);
						deleteInstruction(store);
						assert(def->isDeleted());
					}
				}
			}
		}
	}
}

Ice::Type pointerType()
{
	if(sizeof(void *) == 8)
	{
		return Ice::IceType_i64;
	}
	else
	{
		return Ice::IceType_i32;
	}
}

// Replace arrays with individual elements if only statically indexed.
void Optimizer::performScalarReplacementOfAggregates()
{
	std::vector<Ice::InstAlloca *> newAllocas;

	Ice::CfgNode *entryBlock = function->getEntryNode();
	Ice::InstList &instList = entryBlock->getInsts();

	for(Ice::Inst &inst : instList)
	{
		if(inst.isDeleted())
		{
			continue;
		}

		auto *alloca = llvm::dyn_cast<Ice::InstAlloca>(&inst);

		if(!alloca)
		{
			break;  // Allocas are all at the top
		}

		uint32_t sizeInBytes = llvm::cast<Ice::ConstantInteger32>(alloca->getSizeInBytes())->getValue();
		uint32_t alignInBytes = alloca->getAlignInBytes();

		// This pass relies on array elements to be naturally aligned (i.e. matches the type size).
		assert(sizeInBytes >= alignInBytes);
		assert(sizeInBytes % alignInBytes == 0);
		uint32_t elementCount = sizeInBytes / alignInBytes;

		Ice::Operand *address = alloca->getDest();

		if(isStaticallyIndexedArray(address))
		{
			// Delete the old array.
			alloca->setDeleted();

			// Allocate new stack slots for each element.
			std::vector<Ice::Variable *> newAddress(elementCount);
			auto *bytes = Ice::ConstantInteger32::create(context, Ice::IceType_i32, alignInBytes);

			for(uint32_t i = 0; i < elementCount; i++)
			{
				newAddress[i] = function->makeVariable(pointerType());
				auto *alloca = Ice::InstAlloca::create(function, newAddress[i], bytes, alignInBytes);
				setDefinition(newAddress[i], alloca);

				newAllocas.push_back(alloca);
			}

			Uses uses = *getUses(address);  // Hard copy

			for(auto *use : uses)
			{
				assert(!use->isDeleted());

				if(isLoad(*use))  // Direct use of base address
				{
					use->replaceSource(asLoadSubVector(use) ? 1 : 0, newAddress[0]);
					getUses(newAddress[0])->insert(newAddress[0], use);
				}
				else if(isStore(*use))  // Direct use of base address
				{
					use->replaceSource(asStoreSubVector(use) ? 2 : 1, newAddress[0]);
					getUses(newAddress[0])->insert(newAddress[0], use);
				}
				else  // Statically indexed use
				{
					auto *arithmetic = llvm::cast<Ice::InstArithmetic>(use);

					if(arithmetic->getOp() == Ice::InstArithmetic::Add)
					{
						auto *rhs = arithmetic->getSrc(1);
						int32_t offset = llvm::cast<Ice::ConstantInteger32>(rhs)->getValue();

						assert(offset % alignInBytes == 0);
						int32_t index = offset / alignInBytes;
						assert(static_cast<uint32_t>(index) < elementCount);

						replace(arithmetic, newAddress[index]);
					}
					else
						assert(false && "Mismatch between isStaticallyIndexedArray() and scalarReplacementOfAggregates()");
				}
			}
		}
	}

	// After looping over all the old allocas, add any new ones that replace them.
	// They're added to the front in reverse order, to retain their original order.
	for(size_t i = newAllocas.size(); i-- != 0;)
	{
		if(!isDead(newAllocas[i]))
		{
			instList.push_front(newAllocas[i]);
		}
	}
}

void Optimizer::eliminateDeadCode()
{
	bool modified;
	do
	{
		modified = false;
		for(Ice::CfgNode *basicBlock : function->getNodes())
		{
			for(Ice::Inst &inst : Ice::reverse_range(basicBlock->getInsts()))
			{
				if(inst.isDeleted())
				{
					continue;
				}

				if(isDead(&inst))
				{
					deleteInstruction(&inst);
					modified = true;
				}
			}
		}
	} while(modified);
}

void Optimizer::eliminateUnitializedLoads()
{
	Ice::CfgNode *entryBlock = function->getEntryNode();

	for(Ice::Inst &alloca : entryBlock->getInsts())
	{
		if(alloca.isDeleted())
		{
			continue;
		}

		if(!llvm::isa<Ice::InstAlloca>(alloca))
		{
			break;  // Allocas are all at the top
		}

		Ice::Operand *address = alloca.getDest();

		if(!hasUses(address))
		{
			continue;
		}

		const auto &addressUses = *getUses(address);

		if(!addressUses.areOnlyLoadStore())
		{
			continue;
		}

		if(addressUses.stores.empty())
		{
			for(Ice::Inst *load : addressUses.loads)
			{
				Ice::Variable *loadData = load->getDest();

				if(hasUses(loadData))
				{
					for(Ice::Inst *use : *getUses(loadData))
					{
						for(Ice::SizeT i = 0; i < use->getSrcSize(); i++)
						{
							if(use->getSrc(i) == loadData)
							{
								auto *undef = context->getConstantUndef(loadData->getType());

								use->replaceSource(i, undef);
							}
						}
					}

					setUses(loadData, nullptr);
				}

				load->setDeleted();
			}

			alloca.setDeleted();
			setUses(address, nullptr);
		}
	}
}

// Iterate through basic blocks to propagate stores to subsequent loads.
void Optimizer::optimizeSingleBasicBlockLoadsStores()
{
	for(Ice::CfgNode *block : function->getNodes())
	{
		// For each stack variable keep track of the last store instruction.
		// To eliminate a store followed by another store to the same alloca address
		// we must also know whether all loads have been replaced by the store value.
		struct LastStore
		{
			Ice::Inst *store;
			bool allLoadsReplaced = true;
		};

		// Use the (unique) index of the alloca's destination argument (i.e. the address
		// of the allocated variable), which is of type SizeT, as the key. Note we do not
		// use the pointer to the alloca instruction or its resulting address, to avoid
		// undeterministic unordered_map behavior.
		std::unordered_map<Ice::SizeT, LastStore> lastStoreTo;

		for(Ice::Inst &inst : block->getInsts())
		{
			if(inst.isDeleted())
			{
				continue;
			}

			if(isStore(inst))
			{
				Ice::Operand *address = inst.getStoreAddress();

				if(Ice::InstAlloca *alloca = allocaOf(address))
				{
					// Only consider this store for propagation if its address is not used as
					// a pointer which could be used for indirect stores.
					if(getUses(address)->areOnlyLoadStore())
					{
						Ice::SizeT addressIdx = alloca->getDest()->getIndex();

						// If there was a previous store to this address, and it was propagated
						// to all subsequent loads, it can be eliminated.
						if(auto entry = lastStoreTo.find(addressIdx); entry != lastStoreTo.end())
						{
							Ice::Inst *previousStore = entry->second.store;

							if(storeTypeMatchesStore(&inst, previousStore) &&
							   entry->second.allLoadsReplaced)
							{
								deleteInstruction(previousStore);
							}
						}

						lastStoreTo[addressIdx] = { &inst };
					}
				}
			}
			else if(isLoad(inst))
			{
				if(Ice::InstAlloca *alloca = allocaOf(inst.getLoadAddress()))
				{
					Ice::SizeT addressIdx = alloca->getDest()->getIndex();
					auto entry = lastStoreTo.find(addressIdx);
					if(entry != lastStoreTo.end())
					{
						const Ice::Inst *store = entry->second.store;

						if(loadTypeMatchesStore(&inst, store))
						{
							replace(&inst, store->getData());
						}
						else
						{
							entry->second.allLoadsReplaced = false;
						}
					}
				}
			}
		}
	}

	// This can leave some dead instructions. Specifically stores.
	// TODO(b/179668593): Check just for dead stores by iterating over allocas?
	eliminateDeadCode();
}

void Optimizer::analyzeUses(Ice::Cfg *function)
{
	for(Ice::CfgNode *basicBlock : function->getNodes())
	{
		for(Ice::Inst &instruction : basicBlock->getInsts())
		{
			if(instruction.isDeleted())
			{
				continue;
			}

			if(instruction.getDest())
			{
				setDefinition(instruction.getDest(), &instruction);
			}

			for(Ice::SizeT i = 0; i < instruction.getSrcSize(); i++)
			{
				Ice::SizeT unique = 0;
				for(; unique < i; unique++)
				{
					if(instruction.getSrc(i) == instruction.getSrc(unique))
					{
						break;
					}
				}

				if(i == unique)
				{
					Ice::Operand *src = instruction.getSrc(i);
					getUses(src)->insert(src, &instruction);
				}
			}
		}
	}
}

void Optimizer::replace(Ice::Inst *instruction, Ice::Operand *newValue)
{
	Ice::Variable *oldValue = instruction->getDest();

	if(!newValue)
	{
		newValue = context->getConstantUndef(oldValue->getType());
	}

	if(hasUses(oldValue))
	{
		for(Ice::Inst *use : *getUses(oldValue))
		{
			assert(!use->isDeleted());  // Should have been removed from uses already

			for(Ice::SizeT i = 0; i < use->getSrcSize(); i++)
			{
				if(use->getSrc(i) == oldValue)
				{
					use->replaceSource(i, newValue);
				}
			}

			getUses(newValue)->insert(newValue, use);
		}

		setUses(oldValue, nullptr);
	}

	deleteInstruction(instruction);
}

void Optimizer::deleteInstruction(Ice::Inst *instruction)
{
	if(!instruction || instruction->isDeleted())
	{
		return;
	}

	assert(!instruction->getDest() || getUses(instruction->getDest())->empty());
	instruction->setDeleted();

	for(Ice::SizeT i = 0; i < instruction->getSrcSize(); i++)
	{
		Ice::Operand *src = instruction->getSrc(i);

		if(hasUses(src))
		{
			auto &srcUses = *getUses(src);

			srcUses.erase(instruction);

			if(srcUses.empty())
			{
				setUses(src, nullptr);

				if(Ice::Variable *var = llvm::dyn_cast<Ice::Variable>(src))
				{
					deleteInstruction(getDefinition(var));
				}
			}
		}
	}
}

bool Optimizer::isDead(Ice::Inst *instruction)
{
	Ice::Variable *dest = instruction->getDest();

	if(dest)
	{
		return (!hasUses(dest) || getUses(dest)->empty()) && !instruction->hasSideEffects();
	}
	else if(isStore(*instruction))
	{
		if(Ice::Variable *address = llvm::dyn_cast<Ice::Variable>(instruction->getStoreAddress()))
		{
			Ice::Inst *def = getDefinition(address);

			if(def && llvm::isa<Ice::InstAlloca>(def))
			{
				if(hasUses(address))
				{
					Optimizer::Uses *uses = getUses(address);
					return uses->size() == uses->stores.size();  // Dead if all uses are stores
				}
				else
				{
					return true;  // No uses
				}
			}
		}
	}

	return false;
}

bool Optimizer::isStaticallyIndexedArray(Ice::Operand *allocaAddress)
{
	auto &uses = *getUses(allocaAddress);

	for(auto *use : uses)
	{
		// Direct load from base address.
		if(isLoad(*use) && use->getLoadAddress() == allocaAddress)
		{
			continue;  // This is fine.
		}

		if(isStore(*use))
		{
			// Can either be the address we're storing to, or the data we're storing.
			if(use->getStoreAddress() == allocaAddress)
			{
				continue;
			}
			else
			{
				// propagateAlloca() eliminates most of the stores of the address itself.
				// For the cases it doesn't handle, assume SRoA is not feasible.
				return false;
			}
		}

		// Pointer arithmetic is fine if it only uses constants.
		auto *arithmetic = llvm::dyn_cast<Ice::InstArithmetic>(use);
		if(arithmetic && arithmetic->getOp() == Ice::InstArithmetic::Add)
		{
			auto *rhs = arithmetic->getSrc(1);

			if(llvm::isa<Ice::Constant>(rhs))
			{
				continue;
			}
		}

		// If there's any other type of use, bail out.
		return false;
	}

	return true;
}

Ice::InstAlloca *Optimizer::allocaOf(Ice::Operand *address)
{
	Ice::Variable *addressVar = llvm::dyn_cast<Ice::Variable>(address);
	Ice::Inst *def = addressVar ? getDefinition(addressVar) : nullptr;
	Ice::InstAlloca *alloca = def ? llvm::dyn_cast<Ice::InstAlloca>(def) : nullptr;

	return alloca;
}

const Ice::InstIntrinsic *Optimizer::asLoadSubVector(const Ice::Inst *instruction)
{
	if(auto *instrinsic = llvm::dyn_cast<Ice::InstIntrinsic>(instruction))
	{
		if(instrinsic->getIntrinsicID() == Ice::Intrinsics::LoadSubVector)
		{
			return instrinsic;
		}
	}

	return nullptr;
}

const Ice::InstIntrinsic *Optimizer::asStoreSubVector(const Ice::Inst *instruction)
{
	if(auto *instrinsic = llvm::dyn_cast<Ice::InstIntrinsic>(instruction))
	{
		if(instrinsic->getIntrinsicID() == Ice::Intrinsics::StoreSubVector)
		{
			return instrinsic;
		}
	}

	return nullptr;
}

bool Optimizer::isLoad(const Ice::Inst &instruction)
{
	if(llvm::isa<Ice::InstLoad>(&instruction))
	{
		return true;
	}

	return asLoadSubVector(&instruction) != nullptr;
}

bool Optimizer::isStore(const Ice::Inst &instruction)
{
	if(llvm::isa<Ice::InstStore>(&instruction))
	{
		return true;
	}

	return asStoreSubVector(&instruction) != nullptr;
}

bool Optimizer::loadTypeMatchesStore(const Ice::Inst *load, const Ice::Inst *store)
{
	if(!load || !store)
	{
		return false;
	}

	assert(isLoad(*load) && isStore(*store));
	assert(load->getLoadAddress() == store->getStoreAddress());

	if(store->getData()->getType() != load->getDest()->getType())
	{
		return false;
	}

	if(auto *storeSubVector = asStoreSubVector(store))
	{
		if(auto *loadSubVector = asLoadSubVector(load))
		{
			// Check for matching sub-vector width.
			return llvm::cast<Ice::ConstantInteger32>(storeSubVector->getSrc(2))->getValue() ==
			       llvm::cast<Ice::ConstantInteger32>(loadSubVector->getSrc(1))->getValue();
		}
	}

	return true;
}

bool Optimizer::storeTypeMatchesStore(const Ice::Inst *store1, const Ice::Inst *store2)
{
	assert(isStore(*store1) && isStore(*store2));
	assert(store1->getStoreAddress() == store2->getStoreAddress());

	if(store1->getData()->getType() != store2->getData()->getType())
	{
		return false;
	}

	if(auto *storeSubVector1 = asStoreSubVector(store1))
	{
		if(auto *storeSubVector2 = asStoreSubVector(store2))
		{
			// Check for matching sub-vector width.
			return llvm::cast<Ice::ConstantInteger32>(storeSubVector1->getSrc(2))->getValue() ==
			       llvm::cast<Ice::ConstantInteger32>(storeSubVector2->getSrc(2))->getValue();
		}
	}

	return true;
}

void Optimizer::collectDiagnostics()
{
	if(report)
	{
		*report = {};

		for(auto *basicBlock : function->getNodes())
		{
			for(auto &inst : basicBlock->getInsts())
			{
				if(inst.isDeleted())
				{
					continue;
				}

				if(llvm::isa<Ice::InstAlloca>(inst))
				{
					report->allocas++;
				}
				else if(isLoad(inst))
				{
					report->loads++;
				}
				else if(isStore(inst))
				{
					report->stores++;
				}
			}
		}
	}
}

Optimizer::Uses *Optimizer::getUses(Ice::Operand *operand)
{
	Optimizer::Uses *uses = (Optimizer::Uses *)operand->Ice::Operand::getExternalData();
	if(!uses)
	{
		uses = new Optimizer::Uses;
		setUses(operand, uses);
		operandsWithUses.push_back(operand);
	}
	return uses;
}

void Optimizer::setUses(Ice::Operand *operand, Optimizer::Uses *uses)
{
	if(auto *oldUses = reinterpret_cast<Optimizer::Uses *>(operand->Ice::Operand::getExternalData()))
	{
		delete oldUses;
	}

	operand->Ice::Operand::setExternalData(uses);
}

bool Optimizer::hasUses(Ice::Operand *operand) const
{
	return operand->Ice::Operand::getExternalData() != nullptr;
}

Ice::Inst *Optimizer::getDefinition(Ice::Variable *var)
{
	return (Ice::Inst *)var->Ice::Variable::getExternalData();
}

void Optimizer::setDefinition(Ice::Variable *var, Ice::Inst *inst)
{
	var->Ice::Variable::setExternalData(inst);
}

bool Optimizer::Uses::areOnlyLoadStore() const
{
	return size() == (loads.size() + stores.size());
}

void Optimizer::Uses::insert(Ice::Operand *value, Ice::Inst *instruction)
{
	push_back(instruction);

	if(isLoad(*instruction))
	{
		if(value == instruction->getLoadAddress())
		{
			loads.push_back(instruction);
		}
	}
	else if(isStore(*instruction))
	{
		if(value == instruction->getStoreAddress())
		{
			stores.push_back(instruction);
		}
	}
}

void Optimizer::Uses::erase(Ice::Inst *instruction)
{
	auto &uses = *this;

	for(size_t i = 0; i < uses.size(); i++)
	{
		if(uses[i] == instruction)
		{
			uses[i] = back();
			pop_back();

			for(size_t i = 0; i < loads.size(); i++)
			{
				if(loads[i] == instruction)
				{
					loads[i] = loads.back();
					loads.pop_back();
					break;
				}
			}

			for(size_t i = 0; i < stores.size(); i++)
			{
				if(stores[i] == instruction)
				{
					stores[i] = stores.back();
					stores.pop_back();
					break;
				}
			}

			break;
		}
	}
}

}  // anonymous namespace

namespace rr {

void optimize(Ice::Cfg *function, Nucleus::OptimizerReport *report)
{
	Optimizer optimizer(report);

	optimizer.run(function);
}

}  // namespace rr
