//===--- PreciseVisitor.cpp ------- Precise Visitor --------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PreciseVisitor.h"
#include "clang/SPIRV/AstTypeProbe.h"
#include "clang/SPIRV/SpirvFunction.h"
#include "clang/SPIRV/SpirvType.h"

#include <stack>

namespace {

/// \brief Returns true if the given OpAccessChain instruction is accessing a
/// precise variable, or accessing a precise member of a structure. Returns
/// false otherwise.
bool isAccessingPrecise(clang::spirv::SpirvAccessChain *inst) {
  using namespace clang::spirv;

  // If the access chain base is another access chain and so on, first flatten
  // them (from the bottom to the top). For example:
  // %x = OpAccessChain <type> %obj %int_1 %int_2
  // %y = OpAccessChain <type> %x   %int_3 %int_4
  // %z = OpAccessChain <type> %y   %int_5 %int_6
  // Should be flattened to:
  // %z = OpAccessChain <type> %obj %int_1 %int_2 %int_3 %int_4 %int_5 %int_6
  std::stack<SpirvInstruction *> indexes;
  SpirvInstruction *base = inst;
  while (auto *accessChain = llvm::dyn_cast<SpirvAccessChain>(base)) {
    for (auto iter = accessChain->getIndexes().rbegin();
         iter != accessChain->getIndexes().rend(); ++iter) {
      indexes.push(*iter);
    }
    base = accessChain->getBase();

    // If we reach a 'precise' base at any level, return true.
    if (base->isPrecise())
      return true;
  }

  // Start from the lowest level base (%obj in the above example), and step
  // forward using the 'indexes'. If a 'precise' structure field is discovered
  // at any point, return true.
  const SpirvType *baseType = base->getResultType();
  while (baseType && !indexes.empty()) {
    if (auto *vecType = llvm::dyn_cast<VectorType>(baseType)) {
      indexes.pop();
      baseType = vecType->getElementType();
    } else if (auto *matType = llvm::dyn_cast<MatrixType>(baseType)) {
      indexes.pop();
      baseType = matType->getVecType();
    } else if (auto *arrType = llvm::dyn_cast<ArrayType>(baseType)) {
      indexes.pop();
      baseType = arrType->getElementType();
    } else if (auto *raType = llvm::dyn_cast<RuntimeArrayType>(baseType)) {
      indexes.pop();
      baseType = raType->getElementType();
    } else if (auto *npaType = llvm::dyn_cast<NodePayloadArrayType>(baseType)) {
      indexes.pop();
      baseType = npaType->getElementType();
    } else if (auto *structType = llvm::dyn_cast<StructType>(baseType)) {
      SpirvInstruction *index = indexes.top();
      if (auto *constInt = llvm::dyn_cast<SpirvConstantInteger>(index)) {
        uint32_t indexValue =
            static_cast<uint32_t>(constInt->getValue().getZExtValue());
        auto fields = structType->getFields();
        assert(indexValue < fields.size());
        auto &fieldInfo = fields[indexValue];
        if (fieldInfo.isPrecise) {
          return true;
        } else {
          baseType = fieldInfo.type;
          indexes.pop();
        }
      } else {
        // Trying to index into a structure using a variable? This shouldn't be
        // happening.
        assert(false && "indexing into a struct with variable value");
        return false;
      }
    } else if (auto *ptrType = llvm::dyn_cast<SpirvPointerType>(baseType)) {
      // Note: no need to pop the stack here.
      baseType = ptrType->getPointeeType();
    } else {
      return false;
    }
  }

  return false;
}

} // anonymous namespace

namespace clang {
namespace spirv {

bool PreciseVisitor::visit(SpirvFunction *fn, Phase phase) {
  // Before going through the function instructions
  if (phase == Visitor::Phase::Init) {
    curFnRetValPrecise = fn->isPrecise();
  }
  return true;
}

bool PreciseVisitor::visit(SpirvReturn *inst) {
  if (inst->hasReturnValue()) {
    inst->getReturnValue()->setPrecise(curFnRetValPrecise);
  }
  return true;
}

bool PreciseVisitor::visit(SpirvVariable *var) {
  if (var->hasInitializer())
    var->getInitializer()->setPrecise(var->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvSelect *inst) {
  inst->getTrueObject()->setPrecise(inst->isPrecise());
  inst->getFalseObject()->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvVectorShuffle *inst) {
  // If the result of a vector shuffle is 'precise', the vectors from which the
  // elements are chosen should also be 'precise'.
  if (inst->isPrecise()) {
    auto *vec1 = inst->getVec1();
    auto *vec2 = inst->getVec2();
    const auto vec1Type = vec1->getAstResultType();
    const auto vec2Type = vec2->getAstResultType();
    uint32_t vec1Size;
    uint32_t vec2Size;
    (void)isVectorType(vec1Type, nullptr, &vec1Size);
    (void)isVectorType(vec2Type, nullptr, &vec2Size);
    bool vec1ElemUsed = false;
    bool vec2ElemUsed = false;
    for (auto component : inst->getComponents()) {
      if (component < vec1Size)
        vec1ElemUsed = true;
      else
        vec2ElemUsed = true;
    }

    if (vec1ElemUsed)
      vec1->setPrecise();
    if (vec2ElemUsed)
      vec2->setPrecise();
  }
  return true;
}

bool PreciseVisitor::visit(SpirvBitFieldExtract *inst) {
  inst->getBase()->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvBitFieldInsert *inst) {
  inst->getBase()->setPrecise(inst->isPrecise());
  inst->getInsert()->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvAtomic *inst) {
  if (inst->isPrecise() && inst->hasValue())
    inst->getValue()->setPrecise();
  return true;
}

bool PreciseVisitor::visit(SpirvCompositeConstruct *inst) {
  if (inst->isPrecise())
    for (auto *consituent : inst->getConstituents())
      consituent->setPrecise();
  return true;
}

bool PreciseVisitor::visit(SpirvCompositeExtract *inst) {
  inst->getComposite()->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvCompositeInsert *inst) {
  inst->getComposite()->setPrecise(inst->isPrecise());
  inst->getObject()->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvLoad *inst) {
  // If the instruction result is precise, the pointer we're loading from should
  // also be marked as precise.
  if (inst->isPrecise())
    inst->getPointer()->setPrecise();

  return true;
}

bool PreciseVisitor::visit(SpirvStore *inst) {
  // If the 'pointer' to which we are storing is marked as 'precise', the object
  // we are storing should also be marked as 'precise'.
  // Note that the 'pointer' may either be an 'OpVariable' or it might be the
  // result of one or more access chains (in which case we should figure out if
  // the 'base' of the access chain is 'precise').
  auto *ptr = inst->getPointer();
  auto *obj = inst->getObject();

  // The simple case (target is a precise variable).
  if (ptr->isPrecise()) {
    obj->setPrecise();
    return true;
  }

  if (auto *accessChain = llvm::dyn_cast<SpirvAccessChain>(ptr)) {
    if (isAccessingPrecise(accessChain)) {
      obj->setPrecise();
      return true;
    }
  }

  return true;
}

bool PreciseVisitor::visit(SpirvBinaryOp *inst) {
  bool isPrecise = inst->isPrecise();
  inst->getOperand1()->setPrecise(isPrecise);
  inst->getOperand2()->setPrecise(isPrecise);
  return true;
}

bool PreciseVisitor::visit(SpirvUnaryOp *inst) {
  inst->getOperand()->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvGroupNonUniformOp *inst) {
  for (auto *operand : inst->getOperands())
    operand->setPrecise(inst->isPrecise());
  return true;
}

bool PreciseVisitor::visit(SpirvExtInst *inst) {
  if (inst->isPrecise())
    for (auto *operand : inst->getOperands())
      operand->setPrecise();
  return true;
}

bool PreciseVisitor::visit(SpirvFunctionCall *call) {
  // If a formal parameter for the function is precise, then the corresponding
  // actual parameter should be marked as precise.
  auto function = call->getFunction();
  for (uint32_t i = 0; i < call->getArgs().size(); ++i) {
    auto formalParameter = function->getParameters()[i];
    if (!formalParameter->isPrecise()) {
      continue;
    }
    call->getArgs()[i]->setPrecise();
  }
  return true;
}

} // end namespace spirv
} // end namespace clang
