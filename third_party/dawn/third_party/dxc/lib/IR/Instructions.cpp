//===-- Instructions.cpp - Implement the LLVM instructions ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements all of the non-inline methods for the LLVM instruction
// classes.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Instructions.h"
#include "LLVMContextImpl.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
//                            CallSite Class
//===----------------------------------------------------------------------===//

User::op_iterator CallSite::getCallee() const {
  Instruction *II(getInstruction());
  return isCall()
    ? cast<CallInst>(II)->op_end() - 1 // Skip Callee
    : cast<InvokeInst>(II)->op_end() - 3; // Skip BB, BB, Callee
}

//===----------------------------------------------------------------------===//
//                            TerminatorInst Class
//===----------------------------------------------------------------------===//

// Out of line virtual method, so the vtable, etc has a home.
TerminatorInst::~TerminatorInst() {
}

//===----------------------------------------------------------------------===//
//                           UnaryInstruction Class
//===----------------------------------------------------------------------===//

// Out of line virtual method, so the vtable, etc has a home.
UnaryInstruction::~UnaryInstruction() {
}

//===----------------------------------------------------------------------===//
//                              SelectInst Class
//===----------------------------------------------------------------------===//

/// areInvalidOperands - Return a string if the specified operands are invalid
/// for a select operation, otherwise return null.
const char *SelectInst::areInvalidOperands(Value *Op0, Value *Op1, Value *Op2) {
  if (Op1->getType() != Op2->getType())
    return "both values to select must have same type";
  
  if (VectorType *VT = dyn_cast<VectorType>(Op0->getType())) {
    // Vector select.
    if (VT->getElementType() != Type::getInt1Ty(Op0->getContext()))
      return "vector select condition element type must be i1";
    VectorType *ET = dyn_cast<VectorType>(Op1->getType());
    if (!ET)
      return "selected values for vector select must be vectors";
    if (ET->getNumElements() != VT->getNumElements())
      return "vector select requires selected vectors to have "
                   "the same vector length as select condition";
  } else if (Op0->getType() != Type::getInt1Ty(Op0->getContext())) {
    return "select condition must be i1 or <n x i1>";
  }
  return nullptr;
}


//===----------------------------------------------------------------------===//
//                               PHINode Class
//===----------------------------------------------------------------------===//

PHINode::PHINode(const PHINode &PN)
    : Instruction(PN.getType(), Instruction::PHI, nullptr, PN.getNumOperands()),
      ReservedSpace(PN.getNumOperands()) {
  allocHungoffUses(PN.getNumOperands());
  std::copy(PN.op_begin(), PN.op_end(), op_begin());
  std::copy(PN.block_begin(), PN.block_end(), block_begin());
  SubclassOptionalData = PN.SubclassOptionalData;
}

// removeIncomingValue - Remove an incoming value.  This is useful if a
// predecessor basic block is deleted.
Value *PHINode::removeIncomingValue(unsigned Idx, bool DeletePHIIfEmpty) {
  Value *Removed = getIncomingValue(Idx);

  // Move everything after this operand down.
  //
  // FIXME: we could just swap with the end of the list, then erase.  However,
  // clients might not expect this to happen.  The code as it is thrashes the
  // use/def lists, which is kinda lame.
  std::copy(op_begin() + Idx + 1, op_end(), op_begin() + Idx);
  std::copy(block_begin() + Idx + 1, block_end(), block_begin() + Idx);

  // Nuke the last value.
  Op<-1>().set(nullptr);
  setNumHungOffUseOperands(getNumOperands() - 1);

  // If the PHI node is dead, because it has zero entries, nuke it now.
  if (getNumOperands() == 0 && DeletePHIIfEmpty) {
    // If anyone is using this PHI, make them use a dummy value instead...
    replaceAllUsesWith(UndefValue::get(getType()));
    eraseFromParent();
  }
  return Removed;
}

/// growOperands - grow operands - This grows the operand list in response
/// to a push_back style of operation.  This grows the number of ops by 1.5
/// times.
///
void PHINode::growOperands() {
  unsigned e = getNumOperands();
  unsigned NumOps = e + e / 2;
  if (NumOps < 2) NumOps = 2;      // 2 op PHI nodes are VERY common.

  ReservedSpace = NumOps;
  growHungoffUses(ReservedSpace, /* IsPhi */ true);
}

/// hasConstantValue - If the specified PHI node always merges together the same
/// value, return the value, otherwise return null.
Value *PHINode::hasConstantValue() const {
  // Exploit the fact that phi nodes always have at least one entry.
  Value *ConstantValue = getIncomingValue(0);
  for (unsigned i = 1, e = getNumIncomingValues(); i != e; ++i)
    if (getIncomingValue(i) != ConstantValue && getIncomingValue(i) != this) {
      if (ConstantValue != this)
        return nullptr; // Incoming values not all the same.
       // The case where the first value is this PHI.
      ConstantValue = getIncomingValue(i);
    }
  if (ConstantValue == this)
    return UndefValue::get(getType());
  return ConstantValue;
}

//===----------------------------------------------------------------------===//
//                       LandingPadInst Implementation
//===----------------------------------------------------------------------===//

LandingPadInst::LandingPadInst(Type *RetTy, unsigned NumReservedValues,
                               const Twine &NameStr, Instruction *InsertBefore)
    : Instruction(RetTy, Instruction::LandingPad, nullptr, 0, InsertBefore) {
  init(NumReservedValues, NameStr);
}

LandingPadInst::LandingPadInst(Type *RetTy, unsigned NumReservedValues,
                               const Twine &NameStr, BasicBlock *InsertAtEnd)
    : Instruction(RetTy, Instruction::LandingPad, nullptr, 0, InsertAtEnd) {
  init(NumReservedValues, NameStr);
}

LandingPadInst::LandingPadInst(const LandingPadInst &LP)
    : Instruction(LP.getType(), Instruction::LandingPad, nullptr,
                  LP.getNumOperands()),
      ReservedSpace(LP.getNumOperands()) {
  allocHungoffUses(LP.getNumOperands());
  Use *OL = getOperandList();
  const Use *InOL = LP.getOperandList();
  for (unsigned I = 0, E = ReservedSpace; I != E; ++I)
    OL[I] = InOL[I];

  setCleanup(LP.isCleanup());
}

LandingPadInst *LandingPadInst::Create(Type *RetTy, unsigned NumReservedClauses,
                                       const Twine &NameStr,
                                       Instruction *InsertBefore) {
  return new LandingPadInst(RetTy, NumReservedClauses, NameStr, InsertBefore);
}

LandingPadInst *LandingPadInst::Create(Type *RetTy, unsigned NumReservedClauses,
                                       const Twine &NameStr,
                                       BasicBlock *InsertAtEnd) {
  return new LandingPadInst(RetTy, NumReservedClauses, NameStr, InsertAtEnd);
}

void LandingPadInst::init(unsigned NumReservedValues, const Twine &NameStr) {
  ReservedSpace = NumReservedValues;
  setNumHungOffUseOperands(0);
  allocHungoffUses(ReservedSpace);
  setName(NameStr);
  setCleanup(false);
}

/// growOperands - grow operands - This grows the operand list in response to a
/// push_back style of operation. This grows the number of ops by 2 times.
void LandingPadInst::growOperands(unsigned Size) {
  unsigned e = getNumOperands();
  if (ReservedSpace >= e + Size) return;
  ReservedSpace = (std::max(e, 1U) + Size / 2) * 2;
  growHungoffUses(ReservedSpace);
}

void LandingPadInst::addClause(Constant *Val) {
  unsigned OpNo = getNumOperands();
  growOperands(1);
  assert(OpNo < ReservedSpace && "Growing didn't work!");
  setNumHungOffUseOperands(getNumOperands() + 1);
  getOperandList()[OpNo] = Val;
}

//===----------------------------------------------------------------------===//
//                        CallInst Implementation
//===----------------------------------------------------------------------===//

CallInst::~CallInst() {
}

void CallInst::init(FunctionType *FTy, Value *Func, ArrayRef<Value *> Args,
                    const Twine &NameStr) {
  this->FTy = FTy;
  assert(getNumOperands() == Args.size() + 1 && "NumOperands not set up?");
  Op<-1>() = Func;

#ifndef NDEBUG
  assert((Args.size() == FTy->getNumParams() ||
          (FTy->isVarArg() && Args.size() > FTy->getNumParams())) &&
         "Calling a function with bad signature!");

  for (unsigned i = 0; i != Args.size(); ++i)
    assert((i >= FTy->getNumParams() || 
            FTy->getParamType(i) == Args[i]->getType()) &&
           "Calling a function with a bad signature!");
#endif

  std::copy(Args.begin(), Args.end(), op_begin());
  setName(NameStr);
}

void CallInst::init(Value *Func, const Twine &NameStr) {
  FTy =
      cast<FunctionType>(cast<PointerType>(Func->getType())->getElementType());
  assert(getNumOperands() == 1 && "NumOperands not set up?");
  Op<-1>() = Func;

  assert(FTy->getNumParams() == 0 && "Calling a function with bad signature");

  setName(NameStr);
}

CallInst::CallInst(Value *Func, const Twine &Name,
                   Instruction *InsertBefore)
  : Instruction(cast<FunctionType>(cast<PointerType>(Func->getType())
                                   ->getElementType())->getReturnType(),
                Instruction::Call,
                OperandTraits<CallInst>::op_end(this) - 1,
                1, InsertBefore) {
  init(Func, Name);
}

CallInst::CallInst(Value *Func, const Twine &Name,
                   BasicBlock *InsertAtEnd)
  : Instruction(cast<FunctionType>(cast<PointerType>(Func->getType())
                                   ->getElementType())->getReturnType(),
                Instruction::Call,
                OperandTraits<CallInst>::op_end(this) - 1,
                1, InsertAtEnd) {
  init(Func, Name);
}

CallInst::CallInst(const CallInst &CI)
    : Instruction(CI.getType(), Instruction::Call,
                  OperandTraits<CallInst>::op_end(this) - CI.getNumOperands(),
                  CI.getNumOperands()),
      AttributeList(CI.AttributeList), FTy(CI.FTy) {
  setTailCallKind(CI.getTailCallKind());
  setCallingConv(CI.getCallingConv());
    
  std::copy(CI.op_begin(), CI.op_end(), op_begin());
  SubclassOptionalData = CI.SubclassOptionalData;
}

void CallInst::addAttribute(unsigned i, Attribute::AttrKind attr) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addAttribute(getContext(), i, attr);
  setAttributes(PAL);
}

void CallInst::addAttribute(unsigned i, StringRef Kind, StringRef Value) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addAttribute(getContext(), i, Kind, Value);
  setAttributes(PAL);
}

void CallInst::removeAttribute(unsigned i, Attribute attr) {
  AttributeSet PAL = getAttributes();
  AttrBuilder B(attr);
  LLVMContext &Context = getContext();
  PAL = PAL.removeAttributes(Context, i,
                             AttributeSet::get(Context, i, B));
  setAttributes(PAL);
}

void CallInst::addDereferenceableAttr(unsigned i, uint64_t Bytes) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addDereferenceableAttr(getContext(), i, Bytes);
  setAttributes(PAL);
}

void CallInst::addDereferenceableOrNullAttr(unsigned i, uint64_t Bytes) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addDereferenceableOrNullAttr(getContext(), i, Bytes);
  setAttributes(PAL);
}

bool CallInst::paramHasAttr(unsigned i, Attribute::AttrKind A) const {
  if (AttributeList.hasAttribute(i, A))
    return true;
  if (const Function *F = getCalledFunction())
    return F->getAttributes().hasAttribute(i, A);
  return false;
}

/// IsConstantOne - Return true only if val is constant int 1
static bool IsConstantOne(Value *val) {
  assert(val && "IsConstantOne does not work with nullptr val");
  const ConstantInt *CVal = dyn_cast<ConstantInt>(val);
  return CVal && CVal->isOne();
}

static Instruction *createMalloc(Instruction *InsertBefore,
                                 BasicBlock *InsertAtEnd, Type *IntPtrTy,
                                 Type *AllocTy, Value *AllocSize, 
                                 Value *ArraySize, Function *MallocF,
                                 const Twine &Name) {
  assert(((!InsertBefore && InsertAtEnd) || (InsertBefore && !InsertAtEnd)) &&
         "createMalloc needs either InsertBefore or InsertAtEnd");

  // malloc(type) becomes: 
  //       bitcast (i8* malloc(typeSize)) to type*
  // malloc(type, arraySize) becomes:
  //       bitcast (i8 *malloc(typeSize*arraySize)) to type*
  if (!ArraySize)
    ArraySize = ConstantInt::get(IntPtrTy, 1);
  else if (ArraySize->getType() != IntPtrTy) {
    if (InsertBefore)
      ArraySize = CastInst::CreateIntegerCast(ArraySize, IntPtrTy, false,
                                              "", InsertBefore);
    else
      ArraySize = CastInst::CreateIntegerCast(ArraySize, IntPtrTy, false,
                                              "", InsertAtEnd);
  }

  if (!IsConstantOne(ArraySize)) {
    if (IsConstantOne(AllocSize)) {
      AllocSize = ArraySize;         // Operand * 1 = Operand
    } else if (Constant *CO = dyn_cast<Constant>(ArraySize)) {
      Constant *Scale = ConstantExpr::getIntegerCast(CO, IntPtrTy,
                                                     false /*ZExt*/);
      // Malloc arg is constant product of type size and array size
      AllocSize = ConstantExpr::getMul(Scale, cast<Constant>(AllocSize));
    } else {
      // Multiply type size by the array size...
      if (InsertBefore)
        AllocSize = BinaryOperator::CreateMul(ArraySize, AllocSize,
                                              "mallocsize", InsertBefore);
      else
        AllocSize = BinaryOperator::CreateMul(ArraySize, AllocSize,
                                              "mallocsize", InsertAtEnd);
    }
  }

  assert(AllocSize->getType() == IntPtrTy && "malloc arg is wrong size");
  // Create the call to Malloc.
  BasicBlock* BB = InsertBefore ? InsertBefore->getParent() : InsertAtEnd;
  Module* M = BB->getParent()->getParent();
  Type *BPTy = Type::getInt8PtrTy(BB->getContext());
  Value *MallocFunc = MallocF;
  if (!MallocFunc)
    // prototype malloc as "void *malloc(size_t)"
    MallocFunc = M->getOrInsertFunction("malloc", BPTy, IntPtrTy, nullptr);
  PointerType *AllocPtrType = PointerType::getUnqual(AllocTy);
  CallInst *MCall = nullptr;
  Instruction *Result = nullptr;
  if (InsertBefore) {
    MCall = CallInst::Create(MallocFunc, AllocSize, "malloccall", InsertBefore);
    Result = MCall;
    if (Result->getType() != AllocPtrType)
      // Create a cast instruction to convert to the right type...
      Result = new BitCastInst(MCall, AllocPtrType, Name, InsertBefore);
  } else {
    MCall = CallInst::Create(MallocFunc, AllocSize, "malloccall");
    Result = MCall;
    if (Result->getType() != AllocPtrType) {
      InsertAtEnd->getInstList().push_back(MCall);
      // Create a cast instruction to convert to the right type...
      Result = new BitCastInst(MCall, AllocPtrType, Name);
    }
  }
  MCall->setTailCall();
  if (Function *F = dyn_cast<Function>(MallocFunc)) {
    MCall->setCallingConv(F->getCallingConv());
    if (!F->doesNotAlias(0)) F->setDoesNotAlias(0);
  }
  assert(!MCall->getType()->isVoidTy() && "Malloc has void return type");

  return Result;
}

/// CreateMalloc - Generate the IR for a call to malloc:
/// 1. Compute the malloc call's argument as the specified type's size,
///    possibly multiplied by the array size if the array size is not
///    constant 1.
/// 2. Call malloc with that argument.
/// 3. Bitcast the result of the malloc call to the specified type.
Instruction *CallInst::CreateMalloc(Instruction *InsertBefore,
                                    Type *IntPtrTy, Type *AllocTy,
                                    Value *AllocSize, Value *ArraySize,
                                    Function * MallocF,
                                    const Twine &Name) {
  return createMalloc(InsertBefore, nullptr, IntPtrTy, AllocTy, AllocSize,
                      ArraySize, MallocF, Name);
}

/// CreateMalloc - Generate the IR for a call to malloc:
/// 1. Compute the malloc call's argument as the specified type's size,
///    possibly multiplied by the array size if the array size is not
///    constant 1.
/// 2. Call malloc with that argument.
/// 3. Bitcast the result of the malloc call to the specified type.
/// Note: This function does not add the bitcast to the basic block, that is the
/// responsibility of the caller.
Instruction *CallInst::CreateMalloc(BasicBlock *InsertAtEnd,
                                    Type *IntPtrTy, Type *AllocTy,
                                    Value *AllocSize, Value *ArraySize, 
                                    Function *MallocF, const Twine &Name) {
  return createMalloc(nullptr, InsertAtEnd, IntPtrTy, AllocTy, AllocSize,
                      ArraySize, MallocF, Name);
}

static Instruction* createFree(Value* Source, Instruction *InsertBefore,
                               BasicBlock *InsertAtEnd) {
  assert(((!InsertBefore && InsertAtEnd) || (InsertBefore && !InsertAtEnd)) &&
         "createFree needs either InsertBefore or InsertAtEnd");
  assert(Source->getType()->isPointerTy() &&
         "Can not free something of nonpointer type!");

  BasicBlock* BB = InsertBefore ? InsertBefore->getParent() : InsertAtEnd;
  Module* M = BB->getParent()->getParent();

  Type *VoidTy = Type::getVoidTy(M->getContext());
  Type *IntPtrTy = Type::getInt8PtrTy(M->getContext());
  // prototype free as "void free(void*)"
  Value *FreeFunc = M->getOrInsertFunction("free", VoidTy, IntPtrTy, nullptr);
  CallInst* Result = nullptr;
  Value *PtrCast = Source;
  if (InsertBefore) {
    if (Source->getType() != IntPtrTy)
      PtrCast = new BitCastInst(Source, IntPtrTy, "", InsertBefore);
    Result = CallInst::Create(FreeFunc, PtrCast, "", InsertBefore);
  } else {
    if (Source->getType() != IntPtrTy)
      PtrCast = new BitCastInst(Source, IntPtrTy, "", InsertAtEnd);
    Result = CallInst::Create(FreeFunc, PtrCast, "");
  }
  Result->setTailCall();
  if (Function *F = dyn_cast<Function>(FreeFunc))
    Result->setCallingConv(F->getCallingConv());

  return Result;
}

/// CreateFree - Generate the IR for a call to the builtin free function.
Instruction * CallInst::CreateFree(Value* Source, Instruction *InsertBefore) {
  return createFree(Source, InsertBefore, nullptr);
}

/// CreateFree - Generate the IR for a call to the builtin free function.
/// Note: This function does not add the call to the basic block, that is the
/// responsibility of the caller.
Instruction* CallInst::CreateFree(Value* Source, BasicBlock *InsertAtEnd) {
  Instruction* FreeCall = createFree(Source, nullptr, InsertAtEnd);
  assert(FreeCall && "CreateFree did not create a CallInst");
  return FreeCall;
}

//===----------------------------------------------------------------------===//
//                        InvokeInst Implementation
//===----------------------------------------------------------------------===//

void InvokeInst::init(FunctionType *FTy, Value *Fn, BasicBlock *IfNormal,
                      BasicBlock *IfException, ArrayRef<Value *> Args,
                      const Twine &NameStr) {
  this->FTy = FTy;

  assert(getNumOperands() == 3 + Args.size() && "NumOperands not set up?");
  Op<-3>() = Fn;
  Op<-2>() = IfNormal;
  Op<-1>() = IfException;

#ifndef NDEBUG
  assert(((Args.size() == FTy->getNumParams()) ||
          (FTy->isVarArg() && Args.size() > FTy->getNumParams())) &&
         "Invoking a function with bad signature");

  for (unsigned i = 0, e = Args.size(); i != e; i++)
    assert((i >= FTy->getNumParams() || 
            FTy->getParamType(i) == Args[i]->getType()) &&
           "Invoking a function with a bad signature!");
#endif

  std::copy(Args.begin(), Args.end(), op_begin());
  setName(NameStr);
}

InvokeInst::InvokeInst(const InvokeInst &II)
    : TerminatorInst(II.getType(), Instruction::Invoke,
                     OperandTraits<InvokeInst>::op_end(this) -
                         II.getNumOperands(),
                     II.getNumOperands()),
      AttributeList(II.AttributeList), FTy(II.FTy) {
  setCallingConv(II.getCallingConv());
  std::copy(II.op_begin(), II.op_end(), op_begin());
  SubclassOptionalData = II.SubclassOptionalData;
}

BasicBlock *InvokeInst::getSuccessorV(unsigned idx) const {
  return getSuccessor(idx);
}
unsigned InvokeInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}
void InvokeInst::setSuccessorV(unsigned idx, BasicBlock *B) {
  return setSuccessor(idx, B);
}

bool InvokeInst::hasFnAttrImpl(Attribute::AttrKind A) const {
  if (AttributeList.hasAttribute(AttributeSet::FunctionIndex, A))
    return true;
  if (const Function *F = getCalledFunction())
    return F->getAttributes().hasAttribute(AttributeSet::FunctionIndex, A);
  return false;
}

bool InvokeInst::paramHasAttr(unsigned i, Attribute::AttrKind A) const {
  if (AttributeList.hasAttribute(i, A))
    return true;
  if (const Function *F = getCalledFunction())
    return F->getAttributes().hasAttribute(i, A);
  return false;
}

void InvokeInst::addAttribute(unsigned i, Attribute::AttrKind attr) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addAttribute(getContext(), i, attr);
  setAttributes(PAL);
}

void InvokeInst::removeAttribute(unsigned i, Attribute attr) {
  AttributeSet PAL = getAttributes();
  AttrBuilder B(attr);
  PAL = PAL.removeAttributes(getContext(), i,
                             AttributeSet::get(getContext(), i, B));
  setAttributes(PAL);
}

void InvokeInst::addDereferenceableAttr(unsigned i, uint64_t Bytes) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addDereferenceableAttr(getContext(), i, Bytes);
  setAttributes(PAL);
}

void InvokeInst::addDereferenceableOrNullAttr(unsigned i, uint64_t Bytes) {
  AttributeSet PAL = getAttributes();
  PAL = PAL.addDereferenceableOrNullAttr(getContext(), i, Bytes);
  setAttributes(PAL);
}

LandingPadInst *InvokeInst::getLandingPadInst() const {
  return cast<LandingPadInst>(getUnwindDest()->getFirstNonPHI());
}

//===----------------------------------------------------------------------===//
//                        ReturnInst Implementation
//===----------------------------------------------------------------------===//

ReturnInst::ReturnInst(const ReturnInst &RI)
  : TerminatorInst(Type::getVoidTy(RI.getContext()), Instruction::Ret,
                   OperandTraits<ReturnInst>::op_end(this) -
                     RI.getNumOperands(),
                   RI.getNumOperands()) {
  if (RI.getNumOperands())
    Op<0>() = RI.Op<0>();
  SubclassOptionalData = RI.SubclassOptionalData;
}

ReturnInst::ReturnInst(LLVMContext &C, Value *retVal, Instruction *InsertBefore)
  : TerminatorInst(Type::getVoidTy(C), Instruction::Ret,
                   OperandTraits<ReturnInst>::op_end(this) - !!retVal, !!retVal,
                   InsertBefore) {
  if (retVal)
    Op<0>() = retVal;
}
ReturnInst::ReturnInst(LLVMContext &C, Value *retVal, BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(C), Instruction::Ret,
                   OperandTraits<ReturnInst>::op_end(this) - !!retVal, !!retVal,
                   InsertAtEnd) {
  if (retVal)
    Op<0>() = retVal;
}
ReturnInst::ReturnInst(LLVMContext &Context, BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(Context), Instruction::Ret,
                   OperandTraits<ReturnInst>::op_end(this), 0, InsertAtEnd) {
}

unsigned ReturnInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}

/// Out-of-line ReturnInst method, put here so the C++ compiler can choose to
/// emit the vtable for the class in this translation unit.
void ReturnInst::setSuccessorV(unsigned idx, BasicBlock *NewSucc) {
  llvm_unreachable("ReturnInst has no successors!");
}

BasicBlock *ReturnInst::getSuccessorV(unsigned idx) const {
  llvm_unreachable("ReturnInst has no successors!");
}

ReturnInst::~ReturnInst() {
}

//===----------------------------------------------------------------------===//
//                        ResumeInst Implementation
//===----------------------------------------------------------------------===//

ResumeInst::ResumeInst(const ResumeInst &RI)
  : TerminatorInst(Type::getVoidTy(RI.getContext()), Instruction::Resume,
                   OperandTraits<ResumeInst>::op_begin(this), 1) {
  Op<0>() = RI.Op<0>();
}

ResumeInst::ResumeInst(Value *Exn, Instruction *InsertBefore)
  : TerminatorInst(Type::getVoidTy(Exn->getContext()), Instruction::Resume,
                   OperandTraits<ResumeInst>::op_begin(this), 1, InsertBefore) {
  Op<0>() = Exn;
}

ResumeInst::ResumeInst(Value *Exn, BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(Exn->getContext()), Instruction::Resume,
                   OperandTraits<ResumeInst>::op_begin(this), 1, InsertAtEnd) {
  Op<0>() = Exn;
}

unsigned ResumeInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}

void ResumeInst::setSuccessorV(unsigned idx, BasicBlock *NewSucc) {
  llvm_unreachable("ResumeInst has no successors!");
}

BasicBlock *ResumeInst::getSuccessorV(unsigned idx) const {
  llvm_unreachable("ResumeInst has no successors!");
}

//===----------------------------------------------------------------------===//
//                      UnreachableInst Implementation
//===----------------------------------------------------------------------===//

UnreachableInst::UnreachableInst(LLVMContext &Context, 
                                 Instruction *InsertBefore)
  : TerminatorInst(Type::getVoidTy(Context), Instruction::Unreachable,
                   nullptr, 0, InsertBefore) {
}
UnreachableInst::UnreachableInst(LLVMContext &Context, BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(Context), Instruction::Unreachable,
                   nullptr, 0, InsertAtEnd) {
}

unsigned UnreachableInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}

void UnreachableInst::setSuccessorV(unsigned idx, BasicBlock *NewSucc) {
  llvm_unreachable("UnreachableInst has no successors!");
}

BasicBlock *UnreachableInst::getSuccessorV(unsigned idx) const {
  llvm_unreachable("UnreachableInst has no successors!");
}

//===----------------------------------------------------------------------===//
//                        BranchInst Implementation
//===----------------------------------------------------------------------===//

void BranchInst::AssertOK() {
  if (isConditional())
    assert(getCondition()->getType()->isIntegerTy(1) &&
           "May only branch on boolean predicates!");
}

BranchInst::BranchInst(BasicBlock *IfTrue, Instruction *InsertBefore)
  : TerminatorInst(Type::getVoidTy(IfTrue->getContext()), Instruction::Br,
                   OperandTraits<BranchInst>::op_end(this) - 1,
                   1, InsertBefore) {
  assert(IfTrue && "Branch destination may not be null!");
  Op<-1>() = IfTrue;
}
BranchInst::BranchInst(BasicBlock *IfTrue, BasicBlock *IfFalse, Value *Cond,
                       Instruction *InsertBefore)
  : TerminatorInst(Type::getVoidTy(IfTrue->getContext()), Instruction::Br,
                   OperandTraits<BranchInst>::op_end(this) - 3,
                   3, InsertBefore) {
  Op<-1>() = IfTrue;
  Op<-2>() = IfFalse;
  Op<-3>() = Cond;
#ifndef NDEBUG
  AssertOK();
#endif
}

BranchInst::BranchInst(BasicBlock *IfTrue, BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(IfTrue->getContext()), Instruction::Br,
                   OperandTraits<BranchInst>::op_end(this) - 1,
                   1, InsertAtEnd) {
  assert(IfTrue && "Branch destination may not be null!");
  Op<-1>() = IfTrue;
}

BranchInst::BranchInst(BasicBlock *IfTrue, BasicBlock *IfFalse, Value *Cond,
           BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(IfTrue->getContext()), Instruction::Br,
                   OperandTraits<BranchInst>::op_end(this) - 3,
                   3, InsertAtEnd) {
  Op<-1>() = IfTrue;
  Op<-2>() = IfFalse;
  Op<-3>() = Cond;
#ifndef NDEBUG
  AssertOK();
#endif
}


BranchInst::BranchInst(const BranchInst &BI) :
  TerminatorInst(Type::getVoidTy(BI.getContext()), Instruction::Br,
                 OperandTraits<BranchInst>::op_end(this) - BI.getNumOperands(),
                 BI.getNumOperands()) {
  Op<-1>() = BI.Op<-1>();
  if (BI.getNumOperands() != 1) {
    assert(BI.getNumOperands() == 3 && "BR can have 1 or 3 operands!");
    Op<-3>() = BI.Op<-3>();
    Op<-2>() = BI.Op<-2>();
  }
  SubclassOptionalData = BI.SubclassOptionalData;
}

void BranchInst::swapSuccessors() {
  assert(isConditional() &&
         "Cannot swap successors of an unconditional branch");
  Op<-1>().swap(Op<-2>());

  // Update profile metadata if present and it matches our structural
  // expectations.
  MDNode *ProfileData = getMetadata(LLVMContext::MD_prof);
  if (!ProfileData || ProfileData->getNumOperands() != 3)
    return;

  // The first operand is the name. Fetch them backwards and build a new one.
  Metadata *Ops[] = {ProfileData->getOperand(0), ProfileData->getOperand(2),
                     ProfileData->getOperand(1)};
  setMetadata(LLVMContext::MD_prof,
              MDNode::get(ProfileData->getContext(), Ops));
}

BasicBlock *BranchInst::getSuccessorV(unsigned idx) const {
  return getSuccessor(idx);
}
unsigned BranchInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}
void BranchInst::setSuccessorV(unsigned idx, BasicBlock *B) {
  setSuccessor(idx, B);
}


//===----------------------------------------------------------------------===//
//                        AllocaInst Implementation
//===----------------------------------------------------------------------===//

static Value *getAISize(LLVMContext &Context, Value *Amt) {
  if (!Amt)
    Amt = ConstantInt::get(Type::getInt32Ty(Context), 1);
  else {
    assert(!isa<BasicBlock>(Amt) &&
           "Passed basic block into allocation size parameter! Use other ctor");
    assert(Amt->getType()->isIntegerTy() &&
           "Allocation array size is not an integer!");
  }
  return Amt;
}

AllocaInst::AllocaInst(Type *Ty, const Twine &Name, Instruction *InsertBefore)
    : AllocaInst(Ty, /*ArraySize=*/nullptr, Name, InsertBefore) {}

AllocaInst::AllocaInst(Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd)
    : AllocaInst(Ty, /*ArraySize=*/nullptr, Name, InsertAtEnd) {}

AllocaInst::AllocaInst(Type *Ty, Value *ArraySize, const Twine &Name,
                       Instruction *InsertBefore)
    : AllocaInst(Ty, ArraySize, /*Align=*/0, Name, InsertBefore) {}

AllocaInst::AllocaInst(Type *Ty, Value *ArraySize, const Twine &Name,
                       BasicBlock *InsertAtEnd)
    : AllocaInst(Ty, ArraySize, /*Align=*/0, Name, InsertAtEnd) {}

AllocaInst::AllocaInst(Type *Ty, Value *ArraySize, unsigned Align,
                       const Twine &Name, Instruction *InsertBefore)
    : UnaryInstruction(PointerType::getUnqual(Ty), Alloca,
                       getAISize(Ty->getContext(), ArraySize), InsertBefore),
      AllocatedType(Ty) {
  setAlignment(Align);
  assert(!Ty->isVoidTy() && "Cannot allocate void!");
  setName(Name);
}

AllocaInst::AllocaInst(Type *Ty, Value *ArraySize, unsigned Align,
                       const Twine &Name, BasicBlock *InsertAtEnd)
    : UnaryInstruction(PointerType::getUnqual(Ty), Alloca,
                       getAISize(Ty->getContext(), ArraySize), InsertAtEnd),
      AllocatedType(Ty) {
  setAlignment(Align);
  assert(!Ty->isVoidTy() && "Cannot allocate void!");
  setName(Name);
}

// Out of line virtual method, so the vtable, etc has a home.
AllocaInst::~AllocaInst() {
}

void AllocaInst::setAlignment(unsigned Align) {
  assert((Align & (Align-1)) == 0 && "Alignment is not a power of 2!");
  assert(Align <= MaximumAlignment &&
         "Alignment is greater than MaximumAlignment!");
  setInstructionSubclassData((getSubclassDataFromInstruction() & ~31) |
                             (Log2_32(Align) + 1));
  assert(getAlignment() == Align && "Alignment representation error!");
}

bool AllocaInst::isArrayAllocation() const {
  if (ConstantInt *CI = dyn_cast<ConstantInt>(getOperand(0)))
    return !CI->isOne();
  return true;
}

/// isStaticAlloca - Return true if this alloca is in the entry block of the
/// function and is a constant size.  If so, the code generator will fold it
/// into the prolog/epilog code, so it is basically free.
bool AllocaInst::isStaticAlloca() const {
  // Must be constant size.
  if (!isa<ConstantInt>(getArraySize())) return false;
  
  // Must be in the entry block.
  const BasicBlock *Parent = getParent();
  return Parent == &Parent->getParent()->front() && !isUsedWithInAlloca();
}

//===----------------------------------------------------------------------===//
//                           LoadInst Implementation
//===----------------------------------------------------------------------===//

void LoadInst::AssertOK() {
  assert(getOperand(0)->getType()->isPointerTy() &&
         "Ptr must have pointer type.");
  assert(!(isAtomic() && getAlignment() == 0) &&
         "Alignment required for atomic load");
}

LoadInst::LoadInst(Value *Ptr, const Twine &Name, Instruction *InsertBef)
    : LoadInst(Ptr, Name, /*isVolatile=*/false, InsertBef) {}

LoadInst::LoadInst(Value *Ptr, const Twine &Name, BasicBlock *InsertAE)
    : LoadInst(Ptr, Name, /*isVolatile=*/false, InsertAE) {}

LoadInst::LoadInst(Type *Ty, Value *Ptr, const Twine &Name, bool isVolatile,
                   Instruction *InsertBef)
    : LoadInst(Ty, Ptr, Name, isVolatile, /*Align=*/0, InsertBef) {}

LoadInst::LoadInst(Value *Ptr, const Twine &Name, bool isVolatile,
                   BasicBlock *InsertAE)
    : LoadInst(Ptr, Name, isVolatile, /*Align=*/0, InsertAE) {}

LoadInst::LoadInst(Type *Ty, Value *Ptr, const Twine &Name, bool isVolatile,
                   unsigned Align, Instruction *InsertBef)
    : LoadInst(Ty, Ptr, Name, isVolatile, Align, NotAtomic, CrossThread,
               InsertBef) {}

LoadInst::LoadInst(Value *Ptr, const Twine &Name, bool isVolatile,
                   unsigned Align, BasicBlock *InsertAE)
    : LoadInst(Ptr, Name, isVolatile, Align, NotAtomic, CrossThread, InsertAE) {
}

LoadInst::LoadInst(Type *Ty, Value *Ptr, const Twine &Name, bool isVolatile,
                   unsigned Align, AtomicOrdering Order,
                   SynchronizationScope SynchScope, Instruction *InsertBef)
    : UnaryInstruction(Ty, Load, Ptr, InsertBef) {
  assert(Ty == cast<PointerType>(Ptr->getType())->getElementType());
  setVolatile(isVolatile);
  setAlignment(Align);
  setAtomic(Order, SynchScope);
  AssertOK();
  setName(Name);
}

LoadInst::LoadInst(Value *Ptr, const Twine &Name, bool isVolatile, 
                   unsigned Align, AtomicOrdering Order,
                   SynchronizationScope SynchScope,
                   BasicBlock *InsertAE)
  : UnaryInstruction(cast<PointerType>(Ptr->getType())->getElementType(),
                     Load, Ptr, InsertAE) {
  setVolatile(isVolatile);
  setAlignment(Align);
  setAtomic(Order, SynchScope);
  AssertOK();
  setName(Name);
}

LoadInst::LoadInst(Value *Ptr, const char *Name, Instruction *InsertBef)
  : UnaryInstruction(cast<PointerType>(Ptr->getType())->getElementType(),
                     Load, Ptr, InsertBef) {
  setVolatile(false);
  setAlignment(0);
  setAtomic(NotAtomic);
  AssertOK();
  if (Name && Name[0]) setName(Name);
}

LoadInst::LoadInst(Value *Ptr, const char *Name, BasicBlock *InsertAE)
  : UnaryInstruction(cast<PointerType>(Ptr->getType())->getElementType(),
                     Load, Ptr, InsertAE) {
  setVolatile(false);
  setAlignment(0);
  setAtomic(NotAtomic);
  AssertOK();
  if (Name && Name[0]) setName(Name);
}

LoadInst::LoadInst(Type *Ty, Value *Ptr, const char *Name, bool isVolatile,
                   Instruction *InsertBef)
    : UnaryInstruction(Ty, Load, Ptr, InsertBef) {
  assert(Ty == cast<PointerType>(Ptr->getType())->getElementType());
  setVolatile(isVolatile);
  setAlignment(0);
  setAtomic(NotAtomic);
  AssertOK();
  if (Name && Name[0]) setName(Name);
}

LoadInst::LoadInst(Value *Ptr, const char *Name, bool isVolatile,
                   BasicBlock *InsertAE)
  : UnaryInstruction(cast<PointerType>(Ptr->getType())->getElementType(),
                     Load, Ptr, InsertAE) {
  setVolatile(isVolatile);
  setAlignment(0);
  setAtomic(NotAtomic);
  AssertOK();
  if (Name && Name[0]) setName(Name);
}

void LoadInst::setAlignment(unsigned Align) {
  assert((Align & (Align-1)) == 0 && "Alignment is not a power of 2!");
  assert(Align <= MaximumAlignment &&
         "Alignment is greater than MaximumAlignment!");
  setInstructionSubclassData((getSubclassDataFromInstruction() & ~(31 << 1)) |
                             ((Log2_32(Align)+1)<<1));
  assert(getAlignment() == Align && "Alignment representation error!");
}

//===----------------------------------------------------------------------===//
//                           StoreInst Implementation
//===----------------------------------------------------------------------===//

void StoreInst::AssertOK() {
  assert(getOperand(0) && getOperand(1) && "Both operands must be non-null!");
  assert(getOperand(1)->getType()->isPointerTy() &&
         "Ptr must have pointer type!");
  assert(getOperand(0)->getType() ==
                 cast<PointerType>(getOperand(1)->getType())->getElementType()
         && "Ptr must be a pointer to Val type!");
  assert(!(isAtomic() && getAlignment() == 0) &&
         "Alignment required for atomic store");
}

StoreInst::StoreInst(Value *val, Value *addr, Instruction *InsertBefore)
    : StoreInst(val, addr, /*isVolatile=*/false, InsertBefore) {}

StoreInst::StoreInst(Value *val, Value *addr, BasicBlock *InsertAtEnd)
    : StoreInst(val, addr, /*isVolatile=*/false, InsertAtEnd) {}

StoreInst::StoreInst(Value *val, Value *addr, bool isVolatile,
                     Instruction *InsertBefore)
    : StoreInst(val, addr, isVolatile, /*Align=*/0, InsertBefore) {}

StoreInst::StoreInst(Value *val, Value *addr, bool isVolatile,
                     BasicBlock *InsertAtEnd)
    : StoreInst(val, addr, isVolatile, /*Align=*/0, InsertAtEnd) {}

StoreInst::StoreInst(Value *val, Value *addr, bool isVolatile, unsigned Align,
                     Instruction *InsertBefore)
    : StoreInst(val, addr, isVolatile, Align, NotAtomic, CrossThread,
                InsertBefore) {}

StoreInst::StoreInst(Value *val, Value *addr, bool isVolatile, unsigned Align,
                     BasicBlock *InsertAtEnd)
    : StoreInst(val, addr, isVolatile, Align, NotAtomic, CrossThread,
                InsertAtEnd) {}

StoreInst::StoreInst(Value *val, Value *addr, bool isVolatile,
                     unsigned Align, AtomicOrdering Order,
                     SynchronizationScope SynchScope,
                     Instruction *InsertBefore)
  : Instruction(Type::getVoidTy(val->getContext()), Store,
                OperandTraits<StoreInst>::op_begin(this),
                OperandTraits<StoreInst>::operands(this),
                InsertBefore) {
  Op<0>() = val;
  Op<1>() = addr;
  setVolatile(isVolatile);
  setAlignment(Align);
  setAtomic(Order, SynchScope);
  AssertOK();
}

StoreInst::StoreInst(Value *val, Value *addr, bool isVolatile,
                     unsigned Align, AtomicOrdering Order,
                     SynchronizationScope SynchScope,
                     BasicBlock *InsertAtEnd)
  : Instruction(Type::getVoidTy(val->getContext()), Store,
                OperandTraits<StoreInst>::op_begin(this),
                OperandTraits<StoreInst>::operands(this),
                InsertAtEnd) {
  Op<0>() = val;
  Op<1>() = addr;
  setVolatile(isVolatile);
  setAlignment(Align);
  setAtomic(Order, SynchScope);
  AssertOK();
}

void StoreInst::setAlignment(unsigned Align) {
  assert((Align & (Align-1)) == 0 && "Alignment is not a power of 2!");
  assert(Align <= MaximumAlignment &&
         "Alignment is greater than MaximumAlignment!");
  setInstructionSubclassData((getSubclassDataFromInstruction() & ~(31 << 1)) |
                             ((Log2_32(Align)+1) << 1));
  assert(getAlignment() == Align && "Alignment representation error!");
}

//===----------------------------------------------------------------------===//
//                       AtomicCmpXchgInst Implementation
//===----------------------------------------------------------------------===//

void AtomicCmpXchgInst::Init(Value *Ptr, Value *Cmp, Value *NewVal,
                             AtomicOrdering SuccessOrdering,
                             AtomicOrdering FailureOrdering,
                             SynchronizationScope SynchScope) {
  Op<0>() = Ptr;
  Op<1>() = Cmp;
  Op<2>() = NewVal;
  setSuccessOrdering(SuccessOrdering);
  setFailureOrdering(FailureOrdering);
  setSynchScope(SynchScope);

  assert(getOperand(0) && getOperand(1) && getOperand(2) &&
         "All operands must be non-null!");
  assert(getOperand(0)->getType()->isPointerTy() &&
         "Ptr must have pointer type!");
  assert(getOperand(1)->getType() ==
                 cast<PointerType>(getOperand(0)->getType())->getElementType()
         && "Ptr must be a pointer to Cmp type!");
  assert(getOperand(2)->getType() ==
                 cast<PointerType>(getOperand(0)->getType())->getElementType()
         && "Ptr must be a pointer to NewVal type!");
  assert(SuccessOrdering != NotAtomic &&
         "AtomicCmpXchg instructions must be atomic!");
  assert(FailureOrdering != NotAtomic &&
         "AtomicCmpXchg instructions must be atomic!");
  assert(SuccessOrdering >= FailureOrdering &&
         "AtomicCmpXchg success ordering must be at least as strong as fail");
  assert(FailureOrdering != Release && FailureOrdering != AcquireRelease &&
         "AtomicCmpXchg failure ordering cannot include release semantics");
}

AtomicCmpXchgInst::AtomicCmpXchgInst(Value *Ptr, Value *Cmp, Value *NewVal,
                                     AtomicOrdering SuccessOrdering,
                                     AtomicOrdering FailureOrdering,
                                     SynchronizationScope SynchScope,
                                     Instruction *InsertBefore)
    : Instruction(
          StructType::get(Cmp->getType(), Type::getInt1Ty(Cmp->getContext()),
                          nullptr),
          AtomicCmpXchg, OperandTraits<AtomicCmpXchgInst>::op_begin(this),
          OperandTraits<AtomicCmpXchgInst>::operands(this), InsertBefore) {
  Init(Ptr, Cmp, NewVal, SuccessOrdering, FailureOrdering, SynchScope);
}

AtomicCmpXchgInst::AtomicCmpXchgInst(Value *Ptr, Value *Cmp, Value *NewVal,
                                     AtomicOrdering SuccessOrdering,
                                     AtomicOrdering FailureOrdering,
                                     SynchronizationScope SynchScope,
                                     BasicBlock *InsertAtEnd)
    : Instruction(
          StructType::get(Cmp->getType(), Type::getInt1Ty(Cmp->getContext()),
                          nullptr),
          AtomicCmpXchg, OperandTraits<AtomicCmpXchgInst>::op_begin(this),
          OperandTraits<AtomicCmpXchgInst>::operands(this), InsertAtEnd) {
  Init(Ptr, Cmp, NewVal, SuccessOrdering, FailureOrdering, SynchScope);
}

//===----------------------------------------------------------------------===//
//                       AtomicRMWInst Implementation
//===----------------------------------------------------------------------===//

void AtomicRMWInst::Init(BinOp Operation, Value *Ptr, Value *Val,
                         AtomicOrdering Ordering,
                         SynchronizationScope SynchScope) {
  Op<0>() = Ptr;
  Op<1>() = Val;
  setOperation(Operation);
  setOrdering(Ordering);
  setSynchScope(SynchScope);

  assert(getOperand(0) && getOperand(1) &&
         "All operands must be non-null!");
  assert(getOperand(0)->getType()->isPointerTy() &&
         "Ptr must have pointer type!");
  assert(getOperand(1)->getType() ==
         cast<PointerType>(getOperand(0)->getType())->getElementType()
         && "Ptr must be a pointer to Val type!");
  assert(Ordering != NotAtomic &&
         "AtomicRMW instructions must be atomic!");
}

AtomicRMWInst::AtomicRMWInst(BinOp Operation, Value *Ptr, Value *Val,
                             AtomicOrdering Ordering,
                             SynchronizationScope SynchScope,
                             Instruction *InsertBefore)
  : Instruction(Val->getType(), AtomicRMW,
                OperandTraits<AtomicRMWInst>::op_begin(this),
                OperandTraits<AtomicRMWInst>::operands(this),
                InsertBefore) {
  Init(Operation, Ptr, Val, Ordering, SynchScope);
}

AtomicRMWInst::AtomicRMWInst(BinOp Operation, Value *Ptr, Value *Val,
                             AtomicOrdering Ordering,
                             SynchronizationScope SynchScope,
                             BasicBlock *InsertAtEnd)
  : Instruction(Val->getType(), AtomicRMW,
                OperandTraits<AtomicRMWInst>::op_begin(this),
                OperandTraits<AtomicRMWInst>::operands(this),
                InsertAtEnd) {
  Init(Operation, Ptr, Val, Ordering, SynchScope);
}

//===----------------------------------------------------------------------===//
//                       FenceInst Implementation
//===----------------------------------------------------------------------===//

FenceInst::FenceInst(LLVMContext &C, AtomicOrdering Ordering, 
                     SynchronizationScope SynchScope,
                     Instruction *InsertBefore)
  : Instruction(Type::getVoidTy(C), Fence, nullptr, 0, InsertBefore) {
  setOrdering(Ordering);
  setSynchScope(SynchScope);
}

FenceInst::FenceInst(LLVMContext &C, AtomicOrdering Ordering, 
                     SynchronizationScope SynchScope,
                     BasicBlock *InsertAtEnd)
  : Instruction(Type::getVoidTy(C), Fence, nullptr, 0, InsertAtEnd) {
  setOrdering(Ordering);
  setSynchScope(SynchScope);
}

//===----------------------------------------------------------------------===//
//                       GetElementPtrInst Implementation
//===----------------------------------------------------------------------===//

void GetElementPtrInst::init(Value *Ptr, ArrayRef<Value *> IdxList,
                             const Twine &Name) {
  assert(getNumOperands() == 1 + IdxList.size() &&
         "NumOperands not initialized?");
  Op<0>() = Ptr;
  std::copy(IdxList.begin(), IdxList.end(), op_begin() + 1);
  setName(Name);
}

GetElementPtrInst::GetElementPtrInst(const GetElementPtrInst &GEPI)
    : Instruction(GEPI.getType(), GetElementPtr,
                  OperandTraits<GetElementPtrInst>::op_end(this) -
                      GEPI.getNumOperands(),
                  GEPI.getNumOperands()),
      SourceElementType(GEPI.SourceElementType),
      ResultElementType(GEPI.ResultElementType) {
  std::copy(GEPI.op_begin(), GEPI.op_end(), op_begin());
  SubclassOptionalData = GEPI.SubclassOptionalData;
}

/// getIndexedType - Returns the type of the element that would be accessed with
/// a gep instruction with the specified parameters.
///
/// The Idxs pointer should point to a continuous piece of memory containing the
/// indices, either as Value* or uint64_t.
///
/// A null type is returned if the indices are invalid for the specified
/// pointer type.
///
template <typename IndexTy>
static Type *getIndexedTypeInternal(Type *Agg, ArrayRef<IndexTy> IdxList) {
  // Handle the special case of the empty set index set, which is always valid.
  if (IdxList.empty())
    return Agg;

  // If there is at least one index, the top level type must be sized, otherwise
  // it cannot be 'stepped over'.
  if (!Agg->isSized())
    return nullptr;

  unsigned CurIdx = 1;
  for (; CurIdx != IdxList.size(); ++CurIdx) {
    CompositeType *CT = dyn_cast<CompositeType>(Agg);
    if (!CT || CT->isPointerTy()) return nullptr;
    IndexTy Index = IdxList[CurIdx];
    if (!CT->indexValid(Index)) return nullptr;
    Agg = CT->getTypeAtIndex(Index);
  }
  return CurIdx == IdxList.size() ? Agg : nullptr;
}

Type *GetElementPtrInst::getIndexedType(Type *Ty, ArrayRef<Value *> IdxList) {
  return getIndexedTypeInternal(Ty, IdxList);
}

Type *GetElementPtrInst::getIndexedType(Type *Ty,
                                        ArrayRef<Constant *> IdxList) {
  return getIndexedTypeInternal(Ty, IdxList);
}

Type *GetElementPtrInst::getIndexedType(Type *Ty, ArrayRef<uint64_t> IdxList) {
  return getIndexedTypeInternal(Ty, IdxList);
}

/// hasAllZeroIndices - Return true if all of the indices of this GEP are
/// zeros.  If so, the result pointer and the first operand have the same
/// value, just potentially different types.
bool GetElementPtrInst::hasAllZeroIndices() const {
  for (unsigned i = 1, e = getNumOperands(); i != e; ++i) {
    if (ConstantInt *CI = dyn_cast<ConstantInt>(getOperand(i))) {
      if (!CI->isZero()) return false;
    } else {
      return false;
    }
  }
  return true;
}

/// hasAllConstantIndices - Return true if all of the indices of this GEP are
/// constant integers.  If so, the result pointer and the first operand have
/// a constant offset between them.
bool GetElementPtrInst::hasAllConstantIndices() const {
  for (unsigned i = 1, e = getNumOperands(); i != e; ++i) {
    if (!isa<ConstantInt>(getOperand(i)))
      return false;
  }
  return true;
}

void GetElementPtrInst::setIsInBounds(bool B) {
  cast<GEPOperator>(this)->setIsInBounds(B);
}

bool GetElementPtrInst::isInBounds() const {
  return cast<GEPOperator>(this)->isInBounds();
}

bool GetElementPtrInst::accumulateConstantOffset(const DataLayout &DL,
                                                 APInt &Offset) const {
  // Delegate to the generic GEPOperator implementation.
  return cast<GEPOperator>(this)->accumulateConstantOffset(DL, Offset);
}

//===----------------------------------------------------------------------===//
//                           ExtractElementInst Implementation
//===----------------------------------------------------------------------===//

ExtractElementInst::ExtractElementInst(Value *Val, Value *Index,
                                       const Twine &Name,
                                       Instruction *InsertBef)
  : Instruction(cast<VectorType>(Val->getType())->getElementType(),
                ExtractElement,
                OperandTraits<ExtractElementInst>::op_begin(this),
                2, InsertBef) {
  assert(isValidOperands(Val, Index) &&
         "Invalid extractelement instruction operands!");
  Op<0>() = Val;
  Op<1>() = Index;
  setName(Name);
}

ExtractElementInst::ExtractElementInst(Value *Val, Value *Index,
                                       const Twine &Name,
                                       BasicBlock *InsertAE)
  : Instruction(cast<VectorType>(Val->getType())->getElementType(),
                ExtractElement,
                OperandTraits<ExtractElementInst>::op_begin(this),
                2, InsertAE) {
  assert(isValidOperands(Val, Index) &&
         "Invalid extractelement instruction operands!");

  Op<0>() = Val;
  Op<1>() = Index;
  setName(Name);
}


bool ExtractElementInst::isValidOperands(const Value *Val, const Value *Index) {
  if (!Val->getType()->isVectorTy() || !Index->getType()->isIntegerTy())
    return false;
  return true;
}


//===----------------------------------------------------------------------===//
//                           InsertElementInst Implementation
//===----------------------------------------------------------------------===//

InsertElementInst::InsertElementInst(Value *Vec, Value *Elt, Value *Index,
                                     const Twine &Name,
                                     Instruction *InsertBef)
  : Instruction(Vec->getType(), InsertElement,
                OperandTraits<InsertElementInst>::op_begin(this),
                3, InsertBef) {
  assert(isValidOperands(Vec, Elt, Index) &&
         "Invalid insertelement instruction operands!");
  Op<0>() = Vec;
  Op<1>() = Elt;
  Op<2>() = Index;
  setName(Name);
}

InsertElementInst::InsertElementInst(Value *Vec, Value *Elt, Value *Index,
                                     const Twine &Name,
                                     BasicBlock *InsertAE)
  : Instruction(Vec->getType(), InsertElement,
                OperandTraits<InsertElementInst>::op_begin(this),
                3, InsertAE) {
  assert(isValidOperands(Vec, Elt, Index) &&
         "Invalid insertelement instruction operands!");

  Op<0>() = Vec;
  Op<1>() = Elt;
  Op<2>() = Index;
  setName(Name);
}

bool InsertElementInst::isValidOperands(const Value *Vec, const Value *Elt, 
                                        const Value *Index) {
  if (!Vec->getType()->isVectorTy())
    return false;   // First operand of insertelement must be vector type.
  
  if (Elt->getType() != cast<VectorType>(Vec->getType())->getElementType())
    return false;// Second operand of insertelement must be vector element type.
    
  if (!Index->getType()->isIntegerTy())
    return false;  // Third operand of insertelement must be i32.
  return true;
}


//===----------------------------------------------------------------------===//
//                      ShuffleVectorInst Implementation
//===----------------------------------------------------------------------===//

ShuffleVectorInst::ShuffleVectorInst(Value *V1, Value *V2, Value *Mask,
                                     const Twine &Name,
                                     Instruction *InsertBefore)
: Instruction(VectorType::get(cast<VectorType>(V1->getType())->getElementType(),
                cast<VectorType>(Mask->getType())->getNumElements()),
              ShuffleVector,
              OperandTraits<ShuffleVectorInst>::op_begin(this),
              OperandTraits<ShuffleVectorInst>::operands(this),
              InsertBefore) {
  assert(isValidOperands(V1, V2, Mask) &&
         "Invalid shuffle vector instruction operands!");
  Op<0>() = V1;
  Op<1>() = V2;
  Op<2>() = Mask;
  setName(Name);
}

ShuffleVectorInst::ShuffleVectorInst(Value *V1, Value *V2, Value *Mask,
                                     const Twine &Name,
                                     BasicBlock *InsertAtEnd)
: Instruction(VectorType::get(cast<VectorType>(V1->getType())->getElementType(),
                cast<VectorType>(Mask->getType())->getNumElements()),
              ShuffleVector,
              OperandTraits<ShuffleVectorInst>::op_begin(this),
              OperandTraits<ShuffleVectorInst>::operands(this),
              InsertAtEnd) {
  assert(isValidOperands(V1, V2, Mask) &&
         "Invalid shuffle vector instruction operands!");

  Op<0>() = V1;
  Op<1>() = V2;
  Op<2>() = Mask;
  setName(Name);
}

bool ShuffleVectorInst::isValidOperands(const Value *V1, const Value *V2,
                                        const Value *Mask) {
  // V1 and V2 must be vectors of the same type.
  if (!V1->getType()->isVectorTy() || V1->getType() != V2->getType())
    return false;
  
  // Mask must be vector of i32.
  VectorType *MaskTy = dyn_cast<VectorType>(Mask->getType());
  if (!MaskTy || !MaskTy->getElementType()->isIntegerTy(32))
    return false;

  // Check to see if Mask is valid.
  if (isa<UndefValue>(Mask) || isa<ConstantAggregateZero>(Mask))
    return true;

  if (const ConstantVector *MV = dyn_cast<ConstantVector>(Mask)) {
    unsigned V1Size = cast<VectorType>(V1->getType())->getNumElements();
    for (Value *Op : MV->operands()) {
      if (ConstantInt *CI = dyn_cast<ConstantInt>(Op)) {
        if (CI->uge(V1Size*2))
          return false;
      } else if (!isa<UndefValue>(Op)) {
        return false;
      }
    }
    return true;
  }
  
  if (const ConstantDataSequential *CDS =
        dyn_cast<ConstantDataSequential>(Mask)) {
    unsigned V1Size = cast<VectorType>(V1->getType())->getNumElements();
    for (unsigned i = 0, e = MaskTy->getNumElements(); i != e; ++i)
      if (CDS->getElementAsInteger(i) >= V1Size*2)
        return false;
    return true;
  }
  
  // The bitcode reader can create a place holder for a forward reference
  // used as the shuffle mask. When this occurs, the shuffle mask will
  // fall into this case and fail. To avoid this error, do this bit of
  // ugliness to allow such a mask pass.
  if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(Mask))
    if (CE->getOpcode() == Instruction::UserOp1)
      return true;

  return false;
}

/// getMaskValue - Return the index from the shuffle mask for the specified
/// output result.  This is either -1 if the element is undef or a number less
/// than 2*numelements.
int ShuffleVectorInst::getMaskValue(Constant *Mask, unsigned i) {
  assert(i < Mask->getType()->getVectorNumElements() && "Index out of range");
  if (ConstantDataSequential *CDS =dyn_cast<ConstantDataSequential>(Mask))
    return CDS->getElementAsInteger(i);
  Constant *C = Mask->getAggregateElement(i);
  if (isa<UndefValue>(C))
    return -1;
  return cast<ConstantInt>(C)->getZExtValue();
}

/// getShuffleMask - Return the full mask for this instruction, where each
/// element is the element number and undef's are returned as -1.
void ShuffleVectorInst::getShuffleMask(Constant *Mask,
                                       SmallVectorImpl<int> &Result) {
  unsigned NumElts = Mask->getType()->getVectorNumElements();
  
  if (ConstantDataSequential *CDS=dyn_cast<ConstantDataSequential>(Mask)) {
    for (unsigned i = 0; i != NumElts; ++i)
      Result.push_back(CDS->getElementAsInteger(i));
    return;
  }    
  for (unsigned i = 0; i != NumElts; ++i) {
    Constant *C = Mask->getAggregateElement(i);
    Result.push_back(isa<UndefValue>(C) ? -1 :
                     cast<ConstantInt>(C)->getZExtValue());
  }
}


//===----------------------------------------------------------------------===//
//                             InsertValueInst Class
//===----------------------------------------------------------------------===//

void InsertValueInst::init(Value *Agg, Value *Val, ArrayRef<unsigned> Idxs, 
                           const Twine &Name) {
  assert(getNumOperands() == 2 && "NumOperands not initialized?");

  // There's no fundamental reason why we require at least one index
  // (other than weirdness with &*IdxBegin being invalid; see
  // getelementptr's init routine for example). But there's no
  // present need to support it.
  assert(Idxs.size() > 0 && "InsertValueInst must have at least one index");

  assert(ExtractValueInst::getIndexedType(Agg->getType(), Idxs) ==
         Val->getType() && "Inserted value must match indexed type!");
  Op<0>() = Agg;
  Op<1>() = Val;

  Indices.append(Idxs.begin(), Idxs.end());
  setName(Name);
}

InsertValueInst::InsertValueInst(const InsertValueInst &IVI)
  : Instruction(IVI.getType(), InsertValue,
                OperandTraits<InsertValueInst>::op_begin(this), 2),
    Indices(IVI.Indices) {
  Op<0>() = IVI.getOperand(0);
  Op<1>() = IVI.getOperand(1);
  SubclassOptionalData = IVI.SubclassOptionalData;
}

//===----------------------------------------------------------------------===//
//                             ExtractValueInst Class
//===----------------------------------------------------------------------===//

void ExtractValueInst::init(ArrayRef<unsigned> Idxs, const Twine &Name) {
  assert(getNumOperands() == 1 && "NumOperands not initialized?");

  // There's no fundamental reason why we require at least one index.
  // But there's no present need to support it.
  assert(Idxs.size() > 0 && "ExtractValueInst must have at least one index");

  Indices.append(Idxs.begin(), Idxs.end());
  setName(Name);
}

ExtractValueInst::ExtractValueInst(const ExtractValueInst &EVI)
  : UnaryInstruction(EVI.getType(), ExtractValue, EVI.getOperand(0)),
    Indices(EVI.Indices) {
  SubclassOptionalData = EVI.SubclassOptionalData;
}

// getIndexedType - Returns the type of the element that would be extracted
// with an extractvalue instruction with the specified parameters.
//
// A null type is returned if the indices are invalid for the specified
// pointer type.
//
Type *ExtractValueInst::getIndexedType(Type *Agg,
                                       ArrayRef<unsigned> Idxs) {
  for (unsigned Index : Idxs) {
    // We can't use CompositeType::indexValid(Index) here.
    // indexValid() always returns true for arrays because getelementptr allows
    // out-of-bounds indices. Since we don't allow those for extractvalue and
    // insertvalue we need to check array indexing manually.
    // Since the only other types we can index into are struct types it's just
    // as easy to check those manually as well.
    if (ArrayType *AT = dyn_cast<ArrayType>(Agg)) {
      if (Index >= AT->getNumElements())
        return nullptr;
    } else if (StructType *ST = dyn_cast<StructType>(Agg)) {
      if (Index >= ST->getNumElements())
        return nullptr;
    } else {
      // Not a valid type to index into.
      return nullptr;
    }

    Agg = cast<CompositeType>(Agg)->getTypeAtIndex(Index);
  }
  return const_cast<Type*>(Agg);
}

//===----------------------------------------------------------------------===//
//                             BinaryOperator Class
//===----------------------------------------------------------------------===//

BinaryOperator::BinaryOperator(BinaryOps iType, Value *S1, Value *S2,
                               Type *Ty, const Twine &Name,
                               Instruction *InsertBefore)
  : Instruction(Ty, iType,
                OperandTraits<BinaryOperator>::op_begin(this),
                OperandTraits<BinaryOperator>::operands(this),
                InsertBefore) {
  Op<0>() = S1;
  Op<1>() = S2;
  init(iType);
  setName(Name);
}

BinaryOperator::BinaryOperator(BinaryOps iType, Value *S1, Value *S2, 
                               Type *Ty, const Twine &Name,
                               BasicBlock *InsertAtEnd)
  : Instruction(Ty, iType,
                OperandTraits<BinaryOperator>::op_begin(this),
                OperandTraits<BinaryOperator>::operands(this),
                InsertAtEnd) {
  Op<0>() = S1;
  Op<1>() = S2;
  init(iType);
  setName(Name);
}


void BinaryOperator::init(BinaryOps iType) {
  Value *LHS = getOperand(0), *RHS = getOperand(1);
  (void)LHS; (void)RHS; // Silence warnings.
  assert(LHS->getType() == RHS->getType() &&
         "Binary operator operand types must match!");
#ifndef NDEBUG
  switch (iType) {
  case Add: case Sub:
  case Mul:
    assert(getType() == LHS->getType() &&
           "Arithmetic operation should return same type as operands!");
    assert(getType()->isIntOrIntVectorTy() &&
           "Tried to create an integer operation on a non-integer type!");
    break;
  case FAdd: case FSub:
  case FMul:
    assert(getType() == LHS->getType() &&
           "Arithmetic operation should return same type as operands!");
    assert(getType()->isFPOrFPVectorTy() &&
           "Tried to create a floating-point operation on a "
           "non-floating-point type!");
    break;
  case UDiv: 
  case SDiv: 
    assert(getType() == LHS->getType() &&
           "Arithmetic operation should return same type as operands!");
    assert((getType()->isIntegerTy() || (getType()->isVectorTy() && 
            cast<VectorType>(getType())->getElementType()->isIntegerTy())) &&
           "Incorrect operand type (not integer) for S/UDIV");
    break;
  case FDiv:
    assert(getType() == LHS->getType() &&
           "Arithmetic operation should return same type as operands!");
    assert(getType()->isFPOrFPVectorTy() &&
           "Incorrect operand type (not floating point) for FDIV");
    break;
  case URem: 
  case SRem: 
    assert(getType() == LHS->getType() &&
           "Arithmetic operation should return same type as operands!");
    assert((getType()->isIntegerTy() || (getType()->isVectorTy() && 
            cast<VectorType>(getType())->getElementType()->isIntegerTy())) &&
           "Incorrect operand type (not integer) for S/UREM");
    break;
  case FRem:
    assert(getType() == LHS->getType() &&
           "Arithmetic operation should return same type as operands!");
    assert(getType()->isFPOrFPVectorTy() &&
           "Incorrect operand type (not floating point) for FREM");
    break;
  case Shl:
  case LShr:
  case AShr:
    assert(getType() == LHS->getType() &&
           "Shift operation should return same type as operands!");
    assert((getType()->isIntegerTy() ||
            (getType()->isVectorTy() && 
             cast<VectorType>(getType())->getElementType()->isIntegerTy())) &&
           "Tried to create a shift operation on a non-integral type!");
    break;
  case And: case Or:
  case Xor:
    assert(getType() == LHS->getType() &&
           "Logical operation should return same type as operands!");
    assert((getType()->isIntegerTy() ||
            (getType()->isVectorTy() && 
             cast<VectorType>(getType())->getElementType()->isIntegerTy())) &&
           "Tried to create a logical operation on a non-integral type!");
    break;
  default:
    break;
  }
#endif
}

BinaryOperator *BinaryOperator::Create(BinaryOps Op, Value *S1, Value *S2,
                                       const Twine &Name,
                                       Instruction *InsertBefore) {
  assert(S1->getType() == S2->getType() &&
         "Cannot create binary operator with two operands of differing type!");
  return new BinaryOperator(Op, S1, S2, S1->getType(), Name, InsertBefore);
}

BinaryOperator *BinaryOperator::Create(BinaryOps Op, Value *S1, Value *S2,
                                       const Twine &Name,
                                       BasicBlock *InsertAtEnd) {
  BinaryOperator *Res = Create(Op, S1, S2, Name);
  InsertAtEnd->getInstList().push_back(Res);
  return Res;
}

BinaryOperator *BinaryOperator::CreateNeg(Value *Op, const Twine &Name,
                                          Instruction *InsertBefore) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return new BinaryOperator(Instruction::Sub,
                            zero, Op,
                            Op->getType(), Name, InsertBefore);
}

BinaryOperator *BinaryOperator::CreateNeg(Value *Op, const Twine &Name,
                                          BasicBlock *InsertAtEnd) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return new BinaryOperator(Instruction::Sub,
                            zero, Op,
                            Op->getType(), Name, InsertAtEnd);
}

BinaryOperator *BinaryOperator::CreateNSWNeg(Value *Op, const Twine &Name,
                                             Instruction *InsertBefore) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return BinaryOperator::CreateNSWSub(zero, Op, Name, InsertBefore);
}

BinaryOperator *BinaryOperator::CreateNSWNeg(Value *Op, const Twine &Name,
                                             BasicBlock *InsertAtEnd) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return BinaryOperator::CreateNSWSub(zero, Op, Name, InsertAtEnd);
}

BinaryOperator *BinaryOperator::CreateNUWNeg(Value *Op, const Twine &Name,
                                             Instruction *InsertBefore) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return BinaryOperator::CreateNUWSub(zero, Op, Name, InsertBefore);
}

BinaryOperator *BinaryOperator::CreateNUWNeg(Value *Op, const Twine &Name,
                                             BasicBlock *InsertAtEnd) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return BinaryOperator::CreateNUWSub(zero, Op, Name, InsertAtEnd);
}

BinaryOperator *BinaryOperator::CreateFNeg(Value *Op, const Twine &Name,
                                           Instruction *InsertBefore) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return new BinaryOperator(Instruction::FSub, zero, Op,
                            Op->getType(), Name, InsertBefore);
}

BinaryOperator *BinaryOperator::CreateFNeg(Value *Op, const Twine &Name,
                                           BasicBlock *InsertAtEnd) {
  Value *zero = ConstantFP::getZeroValueForNegation(Op->getType());
  return new BinaryOperator(Instruction::FSub, zero, Op,
                            Op->getType(), Name, InsertAtEnd);
}

BinaryOperator *BinaryOperator::CreateNot(Value *Op, const Twine &Name,
                                          Instruction *InsertBefore) {
  Constant *C = Constant::getAllOnesValue(Op->getType());
  return new BinaryOperator(Instruction::Xor, Op, C,
                            Op->getType(), Name, InsertBefore);
}

BinaryOperator *BinaryOperator::CreateNot(Value *Op, const Twine &Name,
                                          BasicBlock *InsertAtEnd) {
  Constant *AllOnes = Constant::getAllOnesValue(Op->getType());
  return new BinaryOperator(Instruction::Xor, Op, AllOnes,
                            Op->getType(), Name, InsertAtEnd);
}


// isConstantAllOnes - Helper function for several functions below
static inline bool isConstantAllOnes(const Value *V) {
  if (const Constant *C = dyn_cast<Constant>(V))
    return C->isAllOnesValue();
  return false;
}

bool BinaryOperator::isNeg(const Value *V) {
  if (const BinaryOperator *Bop = dyn_cast<BinaryOperator>(V))
    if (Bop->getOpcode() == Instruction::Sub)
      if (Constant* C = dyn_cast<Constant>(Bop->getOperand(0)))
        return C->isNegativeZeroValue();
  return false;
}

bool BinaryOperator::isFNeg(const Value *V, bool IgnoreZeroSign) {
  if (const BinaryOperator *Bop = dyn_cast<BinaryOperator>(V))
    if (Bop->getOpcode() == Instruction::FSub)
      if (Constant* C = dyn_cast<Constant>(Bop->getOperand(0))) {
        if (!IgnoreZeroSign)
          IgnoreZeroSign = cast<Instruction>(V)->hasNoSignedZeros();
        return !IgnoreZeroSign ? C->isNegativeZeroValue() : C->isZeroValue();
      }
  return false;
}

bool BinaryOperator::isNot(const Value *V) {
  if (const BinaryOperator *Bop = dyn_cast<BinaryOperator>(V))
    return (Bop->getOpcode() == Instruction::Xor &&
            (isConstantAllOnes(Bop->getOperand(1)) ||
             isConstantAllOnes(Bop->getOperand(0))));
  return false;
}

Value *BinaryOperator::getNegArgument(Value *BinOp) {
  return cast<BinaryOperator>(BinOp)->getOperand(1);
}

const Value *BinaryOperator::getNegArgument(const Value *BinOp) {
  return getNegArgument(const_cast<Value*>(BinOp));
}

Value *BinaryOperator::getFNegArgument(Value *BinOp) {
  return cast<BinaryOperator>(BinOp)->getOperand(1);
}

const Value *BinaryOperator::getFNegArgument(const Value *BinOp) {
  return getFNegArgument(const_cast<Value*>(BinOp));
}

Value *BinaryOperator::getNotArgument(Value *BinOp) {
  assert(isNot(BinOp) && "getNotArgument on non-'not' instruction!");
  BinaryOperator *BO = cast<BinaryOperator>(BinOp);
  Value *Op0 = BO->getOperand(0);
  Value *Op1 = BO->getOperand(1);
  if (isConstantAllOnes(Op0)) return Op1;

  assert(isConstantAllOnes(Op1));
  return Op0;
}

const Value *BinaryOperator::getNotArgument(const Value *BinOp) {
  return getNotArgument(const_cast<Value*>(BinOp));
}


// swapOperands - Exchange the two operands to this instruction.  This
// instruction is safe to use on any binary instruction and does not
// modify the semantics of the instruction.  If the instruction is
// order dependent (SetLT f.e.) the opcode is changed.
//
bool BinaryOperator::swapOperands() {
  if (!isCommutative())
    return true; // Can't commute operands
  Op<0>().swap(Op<1>());
  return false;
}

void BinaryOperator::setHasNoUnsignedWrap(bool b) {
  cast<OverflowingBinaryOperator>(this)->setHasNoUnsignedWrap(b);
}

void BinaryOperator::setHasNoSignedWrap(bool b) {
  cast<OverflowingBinaryOperator>(this)->setHasNoSignedWrap(b);
}

void BinaryOperator::setIsExact(bool b) {
  cast<PossiblyExactOperator>(this)->setIsExact(b);
}

bool BinaryOperator::hasNoUnsignedWrap() const {
  return cast<OverflowingBinaryOperator>(this)->hasNoUnsignedWrap();
}

bool BinaryOperator::hasNoSignedWrap() const {
  return cast<OverflowingBinaryOperator>(this)->hasNoSignedWrap();
}

bool BinaryOperator::isExact() const {
  return cast<PossiblyExactOperator>(this)->isExact();
}

void BinaryOperator::copyIRFlags(const Value *V) {
  // Copy the wrapping flags.
  if (auto *OB = dyn_cast<OverflowingBinaryOperator>(V)) {
    setHasNoSignedWrap(OB->hasNoSignedWrap());
    setHasNoUnsignedWrap(OB->hasNoUnsignedWrap());
  }

  // Copy the exact flag.
  if (auto *PE = dyn_cast<PossiblyExactOperator>(V))
    setIsExact(PE->isExact());

  // Copy the fast-math flags.
  if (auto *FP = dyn_cast<FPMathOperator>(V))
    copyFastMathFlags(FP->getFastMathFlags());
}

void BinaryOperator::andIRFlags(const Value *V) {
  if (auto *OB = dyn_cast<OverflowingBinaryOperator>(V)) {
    setHasNoSignedWrap(hasNoSignedWrap() && OB->hasNoSignedWrap());
    setHasNoUnsignedWrap(hasNoUnsignedWrap() && OB->hasNoUnsignedWrap());
  }

  if (auto *PE = dyn_cast<PossiblyExactOperator>(V))
    setIsExact(isExact() && PE->isExact());

  if (auto *FP = dyn_cast<FPMathOperator>(V)) {
    FastMathFlags FM = getFastMathFlags();
    FM &= FP->getFastMathFlags();
    copyFastMathFlags(FM);
  }
}


//===----------------------------------------------------------------------===//
//                             FPMathOperator Class
//===----------------------------------------------------------------------===//

/// getFPAccuracy - Get the maximum error permitted by this operation in ULPs.
/// An accuracy of 0.0 means that the operation should be performed with the
/// default precision.
float FPMathOperator::getFPAccuracy() const {
  const MDNode *MD =
      cast<Instruction>(this)->getMetadata(LLVMContext::MD_fpmath);
  if (!MD)
    return 0.0;
  ConstantFP *Accuracy = mdconst::extract<ConstantFP>(MD->getOperand(0));
  return Accuracy->getValueAPF().convertToFloat();
}


//===----------------------------------------------------------------------===//
//                                CastInst Class
//===----------------------------------------------------------------------===//

void CastInst::anchor() {}

// Just determine if this cast only deals with integral->integral conversion.
bool CastInst::isIntegerCast() const {
  switch (getOpcode()) {
    default: return false;
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::Trunc:
      return true;
    case Instruction::BitCast:
      return getOperand(0)->getType()->isIntegerTy() &&
        getType()->isIntegerTy();
  }
}

bool CastInst::isLosslessCast() const {
  // Only BitCast can be lossless, exit fast if we're not BitCast
  if (getOpcode() != Instruction::BitCast)
    return false;

  // Identity cast is always lossless
  Type* SrcTy = getOperand(0)->getType();
  Type* DstTy = getType();
  if (SrcTy == DstTy)
    return true;
  
  // Pointer to pointer is always lossless.
  if (SrcTy->isPointerTy())
    return DstTy->isPointerTy();
  return false;  // Other types have no identity values
}

/// This function determines if the CastInst does not require any bits to be
/// changed in order to effect the cast. Essentially, it identifies cases where
/// no code gen is necessary for the cast, hence the name no-op cast.  For 
/// example, the following are all no-op casts:
/// # bitcast i32* %x to i8*
/// # bitcast <2 x i32> %x to <4 x i16> 
/// # ptrtoint i32* %x to i32     ; on 32-bit plaforms only
/// @brief Determine if the described cast is a no-op.
bool CastInst::isNoopCast(Instruction::CastOps Opcode,
                          Type *SrcTy,
                          Type *DestTy,
                          Type *IntPtrTy) {
  switch (Opcode) {
    default: llvm_unreachable("Invalid CastOp");
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt: 
    case Instruction::FPTrunc:
    case Instruction::FPExt:
    case Instruction::UIToFP:
    case Instruction::SIToFP:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::AddrSpaceCast:
      // TODO: Target informations may give a more accurate answer here.
      return false;
    case Instruction::BitCast:
      return true;  // BitCast never modifies bits.
    case Instruction::PtrToInt:
      return IntPtrTy->getScalarSizeInBits() ==
             DestTy->getScalarSizeInBits();
    case Instruction::IntToPtr:
      return IntPtrTy->getScalarSizeInBits() ==
             SrcTy->getScalarSizeInBits();
  }
}

/// @brief Determine if a cast is a no-op.
bool CastInst::isNoopCast(Type *IntPtrTy) const {
  return isNoopCast(getOpcode(), getOperand(0)->getType(), getType(), IntPtrTy);
}

bool CastInst::isNoopCast(const DataLayout &DL) const {
  Type *PtrOpTy = nullptr;
  if (getOpcode() == Instruction::PtrToInt)
    PtrOpTy = getOperand(0)->getType();
  else if (getOpcode() == Instruction::IntToPtr)
    PtrOpTy = getType();

  Type *IntPtrTy =
      PtrOpTy ? DL.getIntPtrType(PtrOpTy) : DL.getIntPtrType(getContext(), 0);

  return isNoopCast(getOpcode(), getOperand(0)->getType(), getType(), IntPtrTy);
}

/// This function determines if a pair of casts can be eliminated and what
/// opcode should be used in the elimination. This assumes that there are two
/// instructions like this:
/// *  %F = firstOpcode SrcTy %x to MidTy
/// *  %S = secondOpcode MidTy %F to DstTy
/// The function returns a resultOpcode so these two casts can be replaced with:
/// *  %Replacement = resultOpcode %SrcTy %x to DstTy
/// If no such cast is permited, the function returns 0.
unsigned CastInst::isEliminableCastPair(
  Instruction::CastOps firstOp, Instruction::CastOps secondOp,
  Type *SrcTy, Type *MidTy, Type *DstTy, Type *SrcIntPtrTy, Type *MidIntPtrTy,
  Type *DstIntPtrTy) {
  // Define the 144 possibilities for these two cast instructions. The values
  // in this matrix determine what to do in a given situation and select the
  // case in the switch below.  The rows correspond to firstOp, the columns 
  // correspond to secondOp.  In looking at the table below, keep in  mind
  // the following cast properties:
  //
  //          Size Compare       Source               Destination
  // Operator  Src ? Size   Type       Sign         Type       Sign
  // -------- ------------ -------------------   ---------------------
  // TRUNC         >       Integer      Any        Integral     Any
  // ZEXT          <       Integral   Unsigned     Integer      Any
  // SEXT          <       Integral    Signed      Integer      Any
  // FPTOUI       n/a      FloatPt      n/a        Integral   Unsigned
  // FPTOSI       n/a      FloatPt      n/a        Integral    Signed
  // UITOFP       n/a      Integral   Unsigned     FloatPt      n/a
  // SITOFP       n/a      Integral    Signed      FloatPt      n/a
  // FPTRUNC       >       FloatPt      n/a        FloatPt      n/a
  // FPEXT         <       FloatPt      n/a        FloatPt      n/a
  // PTRTOINT     n/a      Pointer      n/a        Integral   Unsigned
  // INTTOPTR     n/a      Integral   Unsigned     Pointer      n/a
  // BITCAST       =       FirstClass   n/a       FirstClass    n/a
  // ADDRSPCST    n/a      Pointer      n/a        Pointer      n/a
  //
  // NOTE: some transforms are safe, but we consider them to be non-profitable.
  // For example, we could merge "fptoui double to i32" + "zext i32 to i64",
  // into "fptoui double to i64", but this loses information about the range
  // of the produced value (we no longer know the top-part is all zeros).
  // Further this conversion is often much more expensive for typical hardware,
  // and causes issues when building libgcc.  We disallow fptosi+sext for the
  // same reason.
  const unsigned numCastOps =
    Instruction::CastOpsEnd - Instruction::CastOpsBegin;
  static const uint8_t CastResults[numCastOps][numCastOps] = {
    // T        F  F  U  S  F  F  P  I  B  A  -+
    // R  Z  S  P  P  I  I  T  P  2  N  T  S   |
    // U  E  E  2  2  2  2  R  E  I  T  C  C   +- secondOp
    // N  X  X  U  S  F  F  N  X  N  2  V  V   |
    // C  T  T  I  I  P  P  C  T  T  P  T  T  -+
    {  1, 0, 0,99,99, 0, 0,99,99,99, 0, 3, 0}, // Trunc         -+
    {  8, 1, 9,99,99, 2,17,99,99,99, 2, 3, 0}, // ZExt           |
    {  8, 0, 1,99,99, 0, 2,99,99,99, 0, 3, 0}, // SExt           |
    {  0, 0, 0,99,99, 0, 0,99,99,99, 0, 3, 0}, // FPToUI         |
    {  0, 0, 0,99,99, 0, 0,99,99,99, 0, 3, 0}, // FPToSI         |
    { 99,99,99, 0, 0,99,99, 0, 0,99,99, 4, 0}, // UIToFP         +- firstOp
    { 99,99,99, 0, 0,99,99, 0, 0,99,99, 4, 0}, // SIToFP         |
    { 99,99,99, 0, 0,99,99, 0, 0,99,99, 4, 0}, // FPTrunc        |
    { 99,99,99, 2, 2,99,99,10, 2,99,99, 4, 0}, // FPExt          |
    {  1, 0, 0,99,99, 0, 0,99,99,99, 7, 3, 0}, // PtrToInt       |
    { 99,99,99,99,99,99,99,99,99,11,99,15, 0}, // IntToPtr       |
    {  5, 5, 5, 6, 6, 5, 5, 6, 6,16, 5, 1,14}, // BitCast        |
    {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13,12}, // AddrSpaceCast -+
  };

  // If either of the casts are a bitcast from scalar to vector, disallow the
  // merging. However, bitcast of A->B->A are allowed.
  bool isFirstBitcast  = (firstOp == Instruction::BitCast);
  bool isSecondBitcast = (secondOp == Instruction::BitCast);
  bool chainedBitcast  = (SrcTy == DstTy && isFirstBitcast && isSecondBitcast);

  // Check if any of the bitcasts convert scalars<->vectors.
  if ((isFirstBitcast  && isa<VectorType>(SrcTy) != isa<VectorType>(MidTy)) ||
      (isSecondBitcast && isa<VectorType>(MidTy) != isa<VectorType>(DstTy)))
    // Unless we are bitcasing to the original type, disallow optimizations.
    if (!chainedBitcast) return 0;

  int ElimCase = CastResults[firstOp-Instruction::CastOpsBegin]
                            [secondOp-Instruction::CastOpsBegin];
  switch (ElimCase) {
    case 0: 
      // Categorically disallowed.
      return 0;
    case 1: 
      // Allowed, use first cast's opcode.
      return firstOp;
    case 2: 
      // Allowed, use second cast's opcode.
      return secondOp;
    case 3: 
      // No-op cast in second op implies firstOp as long as the DestTy
      // is integer and we are not converting between a vector and a
      // non-vector type.
      if (!SrcTy->isVectorTy() && DstTy->isIntegerTy())
        return firstOp;
      return 0;
    case 4:
      // No-op cast in second op implies firstOp as long as the DestTy
      // is floating point.
      if (DstTy->isFloatingPointTy())
        return firstOp;
      return 0;
    case 5: 
      // No-op cast in first op implies secondOp as long as the SrcTy
      // is an integer.
      if (SrcTy->isIntegerTy())
        return secondOp;
      return 0;
    case 6:
      // No-op cast in first op implies secondOp as long as the SrcTy
      // is a floating point.
      if (SrcTy->isFloatingPointTy())
        return secondOp;
      return 0;
    case 7: {
      // Cannot simplify if address spaces are different!
      if (SrcTy->getPointerAddressSpace() != DstTy->getPointerAddressSpace())
        return 0;

      unsigned MidSize = MidTy->getScalarSizeInBits();
      // We can still fold this without knowing the actual sizes as long we
      // know that the intermediate pointer is the largest possible
      // pointer size.
      // FIXME: Is this always true?
      if (MidSize == 64)
        return Instruction::BitCast;

      // ptrtoint, inttoptr -> bitcast (ptr -> ptr) if int size is >= ptr size.
      if (!SrcIntPtrTy || DstIntPtrTy != SrcIntPtrTy)
        return 0;
      unsigned PtrSize = SrcIntPtrTy->getScalarSizeInBits();
      if (MidSize >= PtrSize)
        return Instruction::BitCast;
      return 0;
    }
    case 8: {
      // ext, trunc -> bitcast,    if the SrcTy and DstTy are same size
      // ext, trunc -> ext,        if sizeof(SrcTy) < sizeof(DstTy)
      // ext, trunc -> trunc,      if sizeof(SrcTy) > sizeof(DstTy)
      unsigned SrcSize = SrcTy->getScalarSizeInBits();
      unsigned DstSize = DstTy->getScalarSizeInBits();
      if (SrcSize == DstSize)
        return Instruction::BitCast;
      else if (SrcSize < DstSize)
        return firstOp;
      return secondOp;
    }
    case 9:
      // zext, sext -> zext, because sext can't sign extend after zext
      return Instruction::ZExt;
    case 10:
      // fpext followed by ftrunc is allowed if the bit size returned to is
      // the same as the original, in which case its just a bitcast
      if (SrcTy == DstTy)
        return Instruction::BitCast;
      return 0; // If the types are not the same we can't eliminate it.
    case 11: {
      // inttoptr, ptrtoint -> bitcast if SrcSize<=PtrSize and SrcSize==DstSize
      if (!MidIntPtrTy)
        return 0;
      unsigned PtrSize = MidIntPtrTy->getScalarSizeInBits();
      unsigned SrcSize = SrcTy->getScalarSizeInBits();
      unsigned DstSize = DstTy->getScalarSizeInBits();
      if (SrcSize <= PtrSize && SrcSize == DstSize)
        return Instruction::BitCast;
      return 0;
    }
    case 12: {
      // addrspacecast, addrspacecast -> bitcast,       if SrcAS == DstAS
      // addrspacecast, addrspacecast -> addrspacecast, if SrcAS != DstAS
      if (SrcTy->getPointerAddressSpace() != DstTy->getPointerAddressSpace())
        return Instruction::AddrSpaceCast;
      return Instruction::BitCast;
    }
    case 13:
      // FIXME: this state can be merged with (1), but the following assert
      // is useful to check the correcteness of the sequence due to semantic
      // change of bitcast.
      assert(
        SrcTy->isPtrOrPtrVectorTy() &&
        MidTy->isPtrOrPtrVectorTy() &&
        DstTy->isPtrOrPtrVectorTy() &&
        SrcTy->getPointerAddressSpace() != MidTy->getPointerAddressSpace() &&
        MidTy->getPointerAddressSpace() == DstTy->getPointerAddressSpace() &&
        "Illegal addrspacecast, bitcast sequence!");
      // Allowed, use first cast's opcode
      return firstOp;
    case 14:
      // bitcast, addrspacecast -> addrspacecast if the element type of
      // bitcast's source is the same as that of addrspacecast's destination.
      if (SrcTy->getPointerElementType() == DstTy->getPointerElementType())
        return Instruction::AddrSpaceCast;
      return 0;

    case 15:
      // FIXME: this state can be merged with (1), but the following assert
      // is useful to check the correcteness of the sequence due to semantic
      // change of bitcast.
      assert(
        SrcTy->isIntOrIntVectorTy() &&
        MidTy->isPtrOrPtrVectorTy() &&
        DstTy->isPtrOrPtrVectorTy() &&
        MidTy->getPointerAddressSpace() == DstTy->getPointerAddressSpace() &&
        "Illegal inttoptr, bitcast sequence!");
      // Allowed, use first cast's opcode
      return firstOp;
    case 16:
      // FIXME: this state can be merged with (2), but the following assert
      // is useful to check the correcteness of the sequence due to semantic
      // change of bitcast.
      assert(
        SrcTy->isPtrOrPtrVectorTy() &&
        MidTy->isPtrOrPtrVectorTy() &&
        DstTy->isIntOrIntVectorTy() &&
        SrcTy->getPointerAddressSpace() == MidTy->getPointerAddressSpace() &&
        "Illegal bitcast, ptrtoint sequence!");
      // Allowed, use second cast's opcode
      return secondOp;
    case 17:
      // (sitofp (zext x)) -> (uitofp x)
      return Instruction::UIToFP;
    case 99: 
      // Cast combination can't happen (error in input). This is for all cases
      // where the MidTy is not the same for the two cast instructions.
      llvm_unreachable("Invalid Cast Combination");
    default:
      llvm_unreachable("Error in CastResults table!!!");
  }
}

CastInst *CastInst::Create(Instruction::CastOps op, Value *S, Type *Ty, 
  const Twine &Name, Instruction *InsertBefore) {
  assert(castIsValid(op, S, Ty) && "Invalid cast!");
  // Construct and return the appropriate CastInst subclass
  switch (op) {
  case Trunc:         return new TruncInst         (S, Ty, Name, InsertBefore);
  case ZExt:          return new ZExtInst          (S, Ty, Name, InsertBefore);
  case SExt:          return new SExtInst          (S, Ty, Name, InsertBefore);
  case FPTrunc:       return new FPTruncInst       (S, Ty, Name, InsertBefore);
  case FPExt:         return new FPExtInst         (S, Ty, Name, InsertBefore);
  case UIToFP:        return new UIToFPInst        (S, Ty, Name, InsertBefore);
  case SIToFP:        return new SIToFPInst        (S, Ty, Name, InsertBefore);
  case FPToUI:        return new FPToUIInst        (S, Ty, Name, InsertBefore);
  case FPToSI:        return new FPToSIInst        (S, Ty, Name, InsertBefore);
  case PtrToInt:      return new PtrToIntInst      (S, Ty, Name, InsertBefore);
  case IntToPtr:      return new IntToPtrInst      (S, Ty, Name, InsertBefore);
  case BitCast:       return new BitCastInst       (S, Ty, Name, InsertBefore);
  case AddrSpaceCast: return new AddrSpaceCastInst (S, Ty, Name, InsertBefore);
  default: llvm_unreachable("Invalid opcode provided");
  }
}

CastInst *CastInst::Create(Instruction::CastOps op, Value *S, Type *Ty,
  const Twine &Name, BasicBlock *InsertAtEnd) {
  assert(castIsValid(op, S, Ty) && "Invalid cast!");
  // Construct and return the appropriate CastInst subclass
  switch (op) {
  case Trunc:         return new TruncInst         (S, Ty, Name, InsertAtEnd);
  case ZExt:          return new ZExtInst          (S, Ty, Name, InsertAtEnd);
  case SExt:          return new SExtInst          (S, Ty, Name, InsertAtEnd);
  case FPTrunc:       return new FPTruncInst       (S, Ty, Name, InsertAtEnd);
  case FPExt:         return new FPExtInst         (S, Ty, Name, InsertAtEnd);
  case UIToFP:        return new UIToFPInst        (S, Ty, Name, InsertAtEnd);
  case SIToFP:        return new SIToFPInst        (S, Ty, Name, InsertAtEnd);
  case FPToUI:        return new FPToUIInst        (S, Ty, Name, InsertAtEnd);
  case FPToSI:        return new FPToSIInst        (S, Ty, Name, InsertAtEnd);
  case PtrToInt:      return new PtrToIntInst      (S, Ty, Name, InsertAtEnd);
  case IntToPtr:      return new IntToPtrInst      (S, Ty, Name, InsertAtEnd);
  case BitCast:       return new BitCastInst       (S, Ty, Name, InsertAtEnd);
  case AddrSpaceCast: return new AddrSpaceCastInst (S, Ty, Name, InsertAtEnd);
  default: llvm_unreachable("Invalid opcode provided");
  }
}

CastInst *CastInst::CreateZExtOrBitCast(Value *S, Type *Ty, 
                                        const Twine &Name,
                                        Instruction *InsertBefore) {
  if (S->getType()->getScalarSizeInBits() == Ty->getScalarSizeInBits())
    return Create(Instruction::BitCast, S, Ty, Name, InsertBefore);
  return Create(Instruction::ZExt, S, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateZExtOrBitCast(Value *S, Type *Ty, 
                                        const Twine &Name,
                                        BasicBlock *InsertAtEnd) {
  if (S->getType()->getScalarSizeInBits() == Ty->getScalarSizeInBits())
    return Create(Instruction::BitCast, S, Ty, Name, InsertAtEnd);
  return Create(Instruction::ZExt, S, Ty, Name, InsertAtEnd);
}

CastInst *CastInst::CreateSExtOrBitCast(Value *S, Type *Ty, 
                                        const Twine &Name,
                                        Instruction *InsertBefore) {
  if (S->getType()->getScalarSizeInBits() == Ty->getScalarSizeInBits())
    return Create(Instruction::BitCast, S, Ty, Name, InsertBefore);
  return Create(Instruction::SExt, S, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateSExtOrBitCast(Value *S, Type *Ty, 
                                        const Twine &Name,
                                        BasicBlock *InsertAtEnd) {
  if (S->getType()->getScalarSizeInBits() == Ty->getScalarSizeInBits())
    return Create(Instruction::BitCast, S, Ty, Name, InsertAtEnd);
  return Create(Instruction::SExt, S, Ty, Name, InsertAtEnd);
}

CastInst *CastInst::CreateTruncOrBitCast(Value *S, Type *Ty,
                                         const Twine &Name,
                                         Instruction *InsertBefore) {
  if (S->getType()->getScalarSizeInBits() == Ty->getScalarSizeInBits())
    return Create(Instruction::BitCast, S, Ty, Name, InsertBefore);
  return Create(Instruction::Trunc, S, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateTruncOrBitCast(Value *S, Type *Ty,
                                         const Twine &Name, 
                                         BasicBlock *InsertAtEnd) {
  if (S->getType()->getScalarSizeInBits() == Ty->getScalarSizeInBits())
    return Create(Instruction::BitCast, S, Ty, Name, InsertAtEnd);
  return Create(Instruction::Trunc, S, Ty, Name, InsertAtEnd);
}

CastInst *CastInst::CreatePointerCast(Value *S, Type *Ty,
                                      const Twine &Name,
                                      BasicBlock *InsertAtEnd) {
  assert(S->getType()->isPtrOrPtrVectorTy() && "Invalid cast");
  assert((Ty->isIntOrIntVectorTy() || Ty->isPtrOrPtrVectorTy()) &&
         "Invalid cast");
  assert(Ty->isVectorTy() == S->getType()->isVectorTy() && "Invalid cast");
  assert((!Ty->isVectorTy() ||
          Ty->getVectorNumElements() == S->getType()->getVectorNumElements()) &&
         "Invalid cast");

  if (Ty->isIntOrIntVectorTy())
    return Create(Instruction::PtrToInt, S, Ty, Name, InsertAtEnd);

  return CreatePointerBitCastOrAddrSpaceCast(S, Ty, Name, InsertAtEnd);
}

/// @brief Create a BitCast or a PtrToInt cast instruction
CastInst *CastInst::CreatePointerCast(Value *S, Type *Ty,
                                      const Twine &Name,
                                      Instruction *InsertBefore) {
  assert(S->getType()->isPtrOrPtrVectorTy() && "Invalid cast");
  assert((Ty->isIntOrIntVectorTy() || Ty->isPtrOrPtrVectorTy()) &&
         "Invalid cast");
  assert(Ty->isVectorTy() == S->getType()->isVectorTy() && "Invalid cast");
  assert((!Ty->isVectorTy() ||
          Ty->getVectorNumElements() == S->getType()->getVectorNumElements()) &&
         "Invalid cast");

  if (Ty->isIntOrIntVectorTy())
    return Create(Instruction::PtrToInt, S, Ty, Name, InsertBefore);

  return CreatePointerBitCastOrAddrSpaceCast(S, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreatePointerBitCastOrAddrSpaceCast(
  Value *S, Type *Ty,
  const Twine &Name,
  BasicBlock *InsertAtEnd) {
  assert(S->getType()->isPtrOrPtrVectorTy() && "Invalid cast");
  assert(Ty->isPtrOrPtrVectorTy() && "Invalid cast");

  if (S->getType()->getPointerAddressSpace() != Ty->getPointerAddressSpace())
    return Create(Instruction::AddrSpaceCast, S, Ty, Name, InsertAtEnd);

  return Create(Instruction::BitCast, S, Ty, Name, InsertAtEnd);
}

CastInst *CastInst::CreatePointerBitCastOrAddrSpaceCast(
  Value *S, Type *Ty,
  const Twine &Name,
  Instruction *InsertBefore) {
  assert(S->getType()->isPtrOrPtrVectorTy() && "Invalid cast");
  assert(Ty->isPtrOrPtrVectorTy() && "Invalid cast");

  if (S->getType()->getPointerAddressSpace() != Ty->getPointerAddressSpace())
    return Create(Instruction::AddrSpaceCast, S, Ty, Name, InsertBefore);

  return Create(Instruction::BitCast, S, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateBitOrPointerCast(Value *S, Type *Ty,
                                           const Twine &Name,
                                           Instruction *InsertBefore) {
  if (S->getType()->isPointerTy() && Ty->isIntegerTy())
    return Create(Instruction::PtrToInt, S, Ty, Name, InsertBefore);
  if (S->getType()->isIntegerTy() && Ty->isPointerTy())
    return Create(Instruction::IntToPtr, S, Ty, Name, InsertBefore);

  return Create(Instruction::BitCast, S, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateIntegerCast(Value *C, Type *Ty,
                                      bool isSigned, const Twine &Name,
                                      Instruction *InsertBefore) {
  assert(C->getType()->isIntOrIntVectorTy() && Ty->isIntOrIntVectorTy() &&
         "Invalid integer cast");
  unsigned SrcBits = C->getType()->getScalarSizeInBits();
  unsigned DstBits = Ty->getScalarSizeInBits();
  Instruction::CastOps opcode =
    (SrcBits == DstBits ? Instruction::BitCast :
     (SrcBits > DstBits ? Instruction::Trunc :
      (isSigned ? Instruction::SExt : Instruction::ZExt)));
  return Create(opcode, C, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateIntegerCast(Value *C, Type *Ty, 
                                      bool isSigned, const Twine &Name,
                                      BasicBlock *InsertAtEnd) {
  assert(C->getType()->isIntOrIntVectorTy() && Ty->isIntOrIntVectorTy() &&
         "Invalid cast");
  unsigned SrcBits = C->getType()->getScalarSizeInBits();
  unsigned DstBits = Ty->getScalarSizeInBits();
  Instruction::CastOps opcode =
    (SrcBits == DstBits ? Instruction::BitCast :
     (SrcBits > DstBits ? Instruction::Trunc :
      (isSigned ? Instruction::SExt : Instruction::ZExt)));
  return Create(opcode, C, Ty, Name, InsertAtEnd);
}

CastInst *CastInst::CreateFPCast(Value *C, Type *Ty, 
                                 const Twine &Name, 
                                 Instruction *InsertBefore) {
  assert(C->getType()->isFPOrFPVectorTy() && Ty->isFPOrFPVectorTy() &&
         "Invalid cast");
  unsigned SrcBits = C->getType()->getScalarSizeInBits();
  unsigned DstBits = Ty->getScalarSizeInBits();
  Instruction::CastOps opcode =
    (SrcBits == DstBits ? Instruction::BitCast :
     (SrcBits > DstBits ? Instruction::FPTrunc : Instruction::FPExt));
  return Create(opcode, C, Ty, Name, InsertBefore);
}

CastInst *CastInst::CreateFPCast(Value *C, Type *Ty, 
                                 const Twine &Name, 
                                 BasicBlock *InsertAtEnd) {
  assert(C->getType()->isFPOrFPVectorTy() && Ty->isFPOrFPVectorTy() &&
         "Invalid cast");
  unsigned SrcBits = C->getType()->getScalarSizeInBits();
  unsigned DstBits = Ty->getScalarSizeInBits();
  Instruction::CastOps opcode =
    (SrcBits == DstBits ? Instruction::BitCast :
     (SrcBits > DstBits ? Instruction::FPTrunc : Instruction::FPExt));
  return Create(opcode, C, Ty, Name, InsertAtEnd);
}

// Check whether it is valid to call getCastOpcode for these types.
// This routine must be kept in sync with getCastOpcode.
bool CastInst::isCastable(Type *SrcTy, Type *DestTy) {
  if (!SrcTy->isFirstClassType() || !DestTy->isFirstClassType())
    return false;

  if (SrcTy == DestTy)
    return true;

  if (VectorType *SrcVecTy = dyn_cast<VectorType>(SrcTy))
    if (VectorType *DestVecTy = dyn_cast<VectorType>(DestTy))
      if (SrcVecTy->getNumElements() == DestVecTy->getNumElements()) {
        // An element by element cast.  Valid if casting the elements is valid.
        SrcTy = SrcVecTy->getElementType();
        DestTy = DestVecTy->getElementType();
      }

  // Get the bit sizes, we'll need these
  unsigned SrcBits = SrcTy->getPrimitiveSizeInBits();   // 0 for ptr
  unsigned DestBits = DestTy->getPrimitiveSizeInBits(); // 0 for ptr

  // Run through the possibilities ...
  if (DestTy->isIntegerTy()) {               // Casting to integral
    if (SrcTy->isIntegerTy())                // Casting from integral
        return true;
    if (SrcTy->isFloatingPointTy())   // Casting from floating pt
      return true;
    if (SrcTy->isVectorTy())          // Casting from vector
      return DestBits == SrcBits;
                                      // Casting from something else
    return SrcTy->isPointerTy();
  } 
  if (DestTy->isFloatingPointTy()) {  // Casting to floating pt
    if (SrcTy->isIntegerTy())                // Casting from integral
      return true;
    if (SrcTy->isFloatingPointTy())   // Casting from floating pt
      return true;
    if (SrcTy->isVectorTy())          // Casting from vector
      return DestBits == SrcBits;
                                    // Casting from something else
    return false;
  }
  if (DestTy->isVectorTy())         // Casting to vector
    return DestBits == SrcBits;
  if (DestTy->isPointerTy()) {        // Casting to pointer
    if (SrcTy->isPointerTy())                // Casting from pointer
      return true;
    return SrcTy->isIntegerTy();             // Casting from integral
  } 
  if (DestTy->isX86_MMXTy()) {
    if (SrcTy->isVectorTy())
      return DestBits == SrcBits;       // 64-bit vector to MMX
    return false;
  }                                    // Casting to something else
  return false;
}

bool CastInst::isBitCastable(Type *SrcTy, Type *DestTy) {
  if (!SrcTy->isFirstClassType() || !DestTy->isFirstClassType())
    return false;

  if (SrcTy == DestTy)
    return true;

  if (VectorType *SrcVecTy = dyn_cast<VectorType>(SrcTy)) {
    if (VectorType *DestVecTy = dyn_cast<VectorType>(DestTy)) {
      if (SrcVecTy->getNumElements() == DestVecTy->getNumElements()) {
        // An element by element cast. Valid if casting the elements is valid.
        SrcTy = SrcVecTy->getElementType();
        DestTy = DestVecTy->getElementType();
      }
    }
  }

  if (PointerType *DestPtrTy = dyn_cast<PointerType>(DestTy)) {
    if (PointerType *SrcPtrTy = dyn_cast<PointerType>(SrcTy)) {
      return SrcPtrTy->getAddressSpace() == DestPtrTy->getAddressSpace();
    }
  }

  unsigned SrcBits = SrcTy->getPrimitiveSizeInBits();   // 0 for ptr
  unsigned DestBits = DestTy->getPrimitiveSizeInBits(); // 0 for ptr

  // Could still have vectors of pointers if the number of elements doesn't
  // match
  if (SrcBits == 0 || DestBits == 0)
    return false;

  if (SrcBits != DestBits)
    return false;

  if (DestTy->isX86_MMXTy() || SrcTy->isX86_MMXTy())
    return false;

  return true;
}

bool CastInst::isBitOrNoopPointerCastable(Type *SrcTy, Type *DestTy,
                                          const DataLayout &DL) {
  if (auto *PtrTy = dyn_cast<PointerType>(SrcTy))
    if (auto *IntTy = dyn_cast<IntegerType>(DestTy))
      return IntTy->getBitWidth() == DL.getPointerTypeSizeInBits(PtrTy);
  if (auto *PtrTy = dyn_cast<PointerType>(DestTy))
    if (auto *IntTy = dyn_cast<IntegerType>(SrcTy))
      return IntTy->getBitWidth() == DL.getPointerTypeSizeInBits(PtrTy);

  return isBitCastable(SrcTy, DestTy);
}

// Provide a way to get a "cast" where the cast opcode is inferred from the
// types and size of the operand. This, basically, is a parallel of the
// logic in the castIsValid function below.  This axiom should hold:
//   castIsValid( getCastOpcode(Val, Ty), Val, Ty)
// should not assert in castIsValid. In other words, this produces a "correct"
// casting opcode for the arguments passed to it.
// This routine must be kept in sync with isCastable.
Instruction::CastOps
CastInst::getCastOpcode(
  const Value *Src, bool SrcIsSigned, Type *DestTy, bool DestIsSigned) {
  Type *SrcTy = Src->getType();

  assert(SrcTy->isFirstClassType() && DestTy->isFirstClassType() &&
         "Only first class types are castable!");

  if (SrcTy == DestTy)
    return BitCast;

  // FIXME: Check address space sizes here
  if (VectorType *SrcVecTy = dyn_cast<VectorType>(SrcTy))
    if (VectorType *DestVecTy = dyn_cast<VectorType>(DestTy))
      if (SrcVecTy->getNumElements() == DestVecTy->getNumElements()) {
        // An element by element cast.  Find the appropriate opcode based on the
        // element types.
        SrcTy = SrcVecTy->getElementType();
        DestTy = DestVecTy->getElementType();
      }

  // Get the bit sizes, we'll need these
  unsigned SrcBits = SrcTy->getPrimitiveSizeInBits();   // 0 for ptr
  unsigned DestBits = DestTy->getPrimitiveSizeInBits(); // 0 for ptr

  // Run through the possibilities ...
  if (DestTy->isIntegerTy()) {                      // Casting to integral
    if (SrcTy->isIntegerTy()) {                     // Casting from integral
      if (DestBits < SrcBits)
        return Trunc;                               // int -> smaller int
      else if (DestBits > SrcBits) {                // its an extension
        if (SrcIsSigned)
          return SExt;                              // signed -> SEXT
        else
          return ZExt;                              // unsigned -> ZEXT
      } else {
        return BitCast;                             // Same size, No-op cast
      }
    } else if (SrcTy->isFloatingPointTy()) {        // Casting from floating pt
      if (DestIsSigned) 
        return FPToSI;                              // FP -> sint
      else
        return FPToUI;                              // FP -> uint 
    } else if (SrcTy->isVectorTy()) {
      assert(DestBits == SrcBits &&
             "Casting vector to integer of different width");
      return BitCast;                             // Same size, no-op cast
    } else {
      assert(SrcTy->isPointerTy() &&
             "Casting from a value that is not first-class type");
      return PtrToInt;                              // ptr -> int
    }
  } else if (DestTy->isFloatingPointTy()) {         // Casting to floating pt
    if (SrcTy->isIntegerTy()) {                     // Casting from integral
      if (SrcIsSigned)
        return SIToFP;                              // sint -> FP
      else
        return UIToFP;                              // uint -> FP
    } else if (SrcTy->isFloatingPointTy()) {        // Casting from floating pt
      if (DestBits < SrcBits) {
        return FPTrunc;                             // FP -> smaller FP
      } else if (DestBits > SrcBits) {
        return FPExt;                               // FP -> larger FP
      } else  {
        return BitCast;                             // same size, no-op cast
      }
    } else if (SrcTy->isVectorTy()) {
      assert(DestBits == SrcBits &&
             "Casting vector to floating point of different width");
      return BitCast;                             // same size, no-op cast
    }
    llvm_unreachable("Casting pointer or non-first class to float");
  } else if (DestTy->isVectorTy()) {
    assert(DestBits == SrcBits &&
           "Illegal cast to vector (wrong type or size)");
    return BitCast;
  } else if (DestTy->isPointerTy()) {
    if (SrcTy->isPointerTy()) {
      if (DestTy->getPointerAddressSpace() != SrcTy->getPointerAddressSpace())
        return AddrSpaceCast;
      return BitCast;                               // ptr -> ptr
    } else if (SrcTy->isIntegerTy()) {
      return IntToPtr;                              // int -> ptr
    }
    llvm_unreachable("Casting pointer to other than pointer or int");
  } else if (DestTy->isX86_MMXTy()) {
    if (SrcTy->isVectorTy()) {
      assert(DestBits == SrcBits && "Casting vector of wrong width to X86_MMX");
      return BitCast;                               // 64-bit vector to MMX
    }
    llvm_unreachable("Illegal cast to X86_MMX");
  }
  llvm_unreachable("Casting to type that is not first-class");
}

//===----------------------------------------------------------------------===//
//                    CastInst SubClass Constructors
//===----------------------------------------------------------------------===//

/// Check that the construction parameters for a CastInst are correct. This
/// could be broken out into the separate constructors but it is useful to have
/// it in one place and to eliminate the redundant code for getting the sizes
/// of the types involved.
bool 
CastInst::castIsValid(Instruction::CastOps op, Value *S, Type *DstTy) {

  // Check for type sanity on the arguments
  Type *SrcTy = S->getType();

  if (!SrcTy->isFirstClassType() || !DstTy->isFirstClassType() ||
      SrcTy->isAggregateType() || DstTy->isAggregateType())
    return false;

  // Get the size of the types in bits, we'll need this later
  unsigned SrcBitSize = SrcTy->getScalarSizeInBits();
  unsigned DstBitSize = DstTy->getScalarSizeInBits();

  // If these are vector types, get the lengths of the vectors (using zero for
  // scalar types means that checking that vector lengths match also checks that
  // scalars are not being converted to vectors or vectors to scalars).
  unsigned SrcLength = SrcTy->isVectorTy() ?
    cast<VectorType>(SrcTy)->getNumElements() : 0;
  unsigned DstLength = DstTy->isVectorTy() ?
    cast<VectorType>(DstTy)->getNumElements() : 0;

  // Switch on the opcode provided
  switch (op) {
  default: return false; // This is an input error
  case Instruction::Trunc:
    return SrcTy->isIntOrIntVectorTy() && DstTy->isIntOrIntVectorTy() &&
      SrcLength == DstLength && SrcBitSize > DstBitSize;
  case Instruction::ZExt:
    return SrcTy->isIntOrIntVectorTy() && DstTy->isIntOrIntVectorTy() &&
      SrcLength == DstLength && SrcBitSize < DstBitSize;
  case Instruction::SExt: 
    return SrcTy->isIntOrIntVectorTy() && DstTy->isIntOrIntVectorTy() &&
      SrcLength == DstLength && SrcBitSize < DstBitSize;
  case Instruction::FPTrunc:
    return SrcTy->isFPOrFPVectorTy() && DstTy->isFPOrFPVectorTy() &&
      SrcLength == DstLength && SrcBitSize > DstBitSize;
  case Instruction::FPExt:
    return SrcTy->isFPOrFPVectorTy() && DstTy->isFPOrFPVectorTy() &&
      SrcLength == DstLength && SrcBitSize < DstBitSize;
  case Instruction::UIToFP:
  case Instruction::SIToFP:
    return SrcTy->isIntOrIntVectorTy() && DstTy->isFPOrFPVectorTy() &&
      SrcLength == DstLength;
  case Instruction::FPToUI:
  case Instruction::FPToSI:
    return SrcTy->isFPOrFPVectorTy() && DstTy->isIntOrIntVectorTy() &&
      SrcLength == DstLength;
  case Instruction::PtrToInt:
    if (isa<VectorType>(SrcTy) != isa<VectorType>(DstTy))
      return false;
    if (VectorType *VT = dyn_cast<VectorType>(SrcTy))
      if (VT->getNumElements() != cast<VectorType>(DstTy)->getNumElements())
        return false;
    return SrcTy->getScalarType()->isPointerTy() &&
           DstTy->getScalarType()->isIntegerTy();
  case Instruction::IntToPtr:
    if (isa<VectorType>(SrcTy) != isa<VectorType>(DstTy))
      return false;
    if (VectorType *VT = dyn_cast<VectorType>(SrcTy))
      if (VT->getNumElements() != cast<VectorType>(DstTy)->getNumElements())
        return false;
    return SrcTy->getScalarType()->isIntegerTy() &&
           DstTy->getScalarType()->isPointerTy();
  case Instruction::BitCast: {
    PointerType *SrcPtrTy = dyn_cast<PointerType>(SrcTy->getScalarType());
    PointerType *DstPtrTy = dyn_cast<PointerType>(DstTy->getScalarType());

    // BitCast implies a no-op cast of type only. No bits change.
    // However, you can't cast pointers to anything but pointers.
    if (!SrcPtrTy != !DstPtrTy)
      return false;

    // For non-pointer cases, the cast is okay if the source and destination bit
    // widths are identical.
    if (!SrcPtrTy)
      return SrcTy->getPrimitiveSizeInBits() == DstTy->getPrimitiveSizeInBits();

    // If both are pointers then the address spaces must match.
    if (SrcPtrTy->getAddressSpace() != DstPtrTy->getAddressSpace())
      return false;

    // A vector of pointers must have the same number of elements.
    if (VectorType *SrcVecTy = dyn_cast<VectorType>(SrcTy)) {
      if (VectorType *DstVecTy = dyn_cast<VectorType>(DstTy))
        return (SrcVecTy->getNumElements() == DstVecTy->getNumElements());

      return false;
    }

    return true;
  }
  case Instruction::AddrSpaceCast: {
    PointerType *SrcPtrTy = dyn_cast<PointerType>(SrcTy->getScalarType());
    if (!SrcPtrTy)
      return false;

    PointerType *DstPtrTy = dyn_cast<PointerType>(DstTy->getScalarType());
    if (!DstPtrTy)
      return false;

    if (SrcPtrTy->getAddressSpace() == DstPtrTy->getAddressSpace())
      return false;

    if (VectorType *SrcVecTy = dyn_cast<VectorType>(SrcTy)) {
      if (VectorType *DstVecTy = dyn_cast<VectorType>(DstTy))
        return (SrcVecTy->getNumElements() == DstVecTy->getNumElements());

      return false;
    }

    return true;
  }
  }
}

TruncInst::TruncInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, Trunc, S, Name, InsertBefore) {
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal Trunc");
}

TruncInst::TruncInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, Trunc, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal Trunc");
}

ZExtInst::ZExtInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
)  : CastInst(Ty, ZExt, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal ZExt");
}

ZExtInst::ZExtInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
)  : CastInst(Ty, ZExt, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal ZExt");
}
SExtInst::SExtInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, SExt, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal SExt");
}

SExtInst::SExtInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
)  : CastInst(Ty, SExt, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal SExt");
}

FPTruncInst::FPTruncInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, FPTrunc, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPTrunc");
}

FPTruncInst::FPTruncInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, FPTrunc, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPTrunc");
}

FPExtInst::FPExtInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, FPExt, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPExt");
}

FPExtInst::FPExtInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, FPExt, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPExt");
}

UIToFPInst::UIToFPInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, UIToFP, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal UIToFP");
}

UIToFPInst::UIToFPInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, UIToFP, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal UIToFP");
}

SIToFPInst::SIToFPInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, SIToFP, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal SIToFP");
}

SIToFPInst::SIToFPInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, SIToFP, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal SIToFP");
}

FPToUIInst::FPToUIInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, FPToUI, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPToUI");
}

FPToUIInst::FPToUIInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, FPToUI, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPToUI");
}

FPToSIInst::FPToSIInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, FPToSI, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPToSI");
}

FPToSIInst::FPToSIInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, FPToSI, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal FPToSI");
}

PtrToIntInst::PtrToIntInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, PtrToInt, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal PtrToInt");
}

PtrToIntInst::PtrToIntInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, PtrToInt, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal PtrToInt");
}

IntToPtrInst::IntToPtrInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, IntToPtr, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal IntToPtr");
}

IntToPtrInst::IntToPtrInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, IntToPtr, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal IntToPtr");
}

BitCastInst::BitCastInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, BitCast, S, Name, InsertBefore) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal BitCast");
}

BitCastInst::BitCastInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, BitCast, S, Name, InsertAtEnd) { 
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal BitCast");
}

AddrSpaceCastInst::AddrSpaceCastInst(
  Value *S, Type *Ty, const Twine &Name, Instruction *InsertBefore
) : CastInst(Ty, AddrSpaceCast, S, Name, InsertBefore) {
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal AddrSpaceCast");
}

AddrSpaceCastInst::AddrSpaceCastInst(
  Value *S, Type *Ty, const Twine &Name, BasicBlock *InsertAtEnd
) : CastInst(Ty, AddrSpaceCast, S, Name, InsertAtEnd) {
  assert(castIsValid(getOpcode(), S, Ty) && "Illegal AddrSpaceCast");
}

//===----------------------------------------------------------------------===//
//                               CmpInst Classes
//===----------------------------------------------------------------------===//

void CmpInst::anchor() {}

CmpInst::CmpInst(Type *ty, OtherOps op, unsigned short predicate,
                 Value *LHS, Value *RHS, const Twine &Name,
                 Instruction *InsertBefore)
  : Instruction(ty, op,
                OperandTraits<CmpInst>::op_begin(this),
                OperandTraits<CmpInst>::operands(this),
                InsertBefore) {
    Op<0>() = LHS;
    Op<1>() = RHS;
  setPredicate((Predicate)predicate);
  setName(Name);
}

CmpInst::CmpInst(Type *ty, OtherOps op, unsigned short predicate,
                 Value *LHS, Value *RHS, const Twine &Name,
                 BasicBlock *InsertAtEnd)
  : Instruction(ty, op,
                OperandTraits<CmpInst>::op_begin(this),
                OperandTraits<CmpInst>::operands(this),
                InsertAtEnd) {
  Op<0>() = LHS;
  Op<1>() = RHS;
  setPredicate((Predicate)predicate);
  setName(Name);
}

CmpInst *
CmpInst::Create(OtherOps Op, unsigned short predicate,
                Value *S1, Value *S2, 
                const Twine &Name, Instruction *InsertBefore) {
  if (Op == Instruction::ICmp) {
    if (InsertBefore)
      return new ICmpInst(InsertBefore, CmpInst::Predicate(predicate),
                          S1, S2, Name);
    else
      return new ICmpInst(CmpInst::Predicate(predicate),
                          S1, S2, Name);
  }
  
  if (InsertBefore)
    return new FCmpInst(InsertBefore, CmpInst::Predicate(predicate),
                        S1, S2, Name);
  else
    return new FCmpInst(CmpInst::Predicate(predicate),
                        S1, S2, Name);
}

CmpInst *
CmpInst::Create(OtherOps Op, unsigned short predicate, Value *S1, Value *S2, 
                const Twine &Name, BasicBlock *InsertAtEnd) {
  if (Op == Instruction::ICmp) {
    return new ICmpInst(*InsertAtEnd, CmpInst::Predicate(predicate),
                        S1, S2, Name);
  }
  return new FCmpInst(*InsertAtEnd, CmpInst::Predicate(predicate),
                      S1, S2, Name);
}

void CmpInst::swapOperands() {
  if (ICmpInst *IC = dyn_cast<ICmpInst>(this))
    IC->swapOperands();
  else
    cast<FCmpInst>(this)->swapOperands();
}

bool CmpInst::isCommutative() const {
  if (const ICmpInst *IC = dyn_cast<ICmpInst>(this))
    return IC->isCommutative();
  return cast<FCmpInst>(this)->isCommutative();
}

bool CmpInst::isEquality() const {
  if (const ICmpInst *IC = dyn_cast<ICmpInst>(this))
    return IC->isEquality();
  return cast<FCmpInst>(this)->isEquality();
}


CmpInst::Predicate CmpInst::getInversePredicate(Predicate pred) {
  switch (pred) {
    default: llvm_unreachable("Unknown cmp predicate!");
    case ICMP_EQ: return ICMP_NE;
    case ICMP_NE: return ICMP_EQ;
    case ICMP_UGT: return ICMP_ULE;
    case ICMP_ULT: return ICMP_UGE;
    case ICMP_UGE: return ICMP_ULT;
    case ICMP_ULE: return ICMP_UGT;
    case ICMP_SGT: return ICMP_SLE;
    case ICMP_SLT: return ICMP_SGE;
    case ICMP_SGE: return ICMP_SLT;
    case ICMP_SLE: return ICMP_SGT;

    case FCMP_OEQ: return FCMP_UNE;
    case FCMP_ONE: return FCMP_UEQ;
    case FCMP_OGT: return FCMP_ULE;
    case FCMP_OLT: return FCMP_UGE;
    case FCMP_OGE: return FCMP_ULT;
    case FCMP_OLE: return FCMP_UGT;
    case FCMP_UEQ: return FCMP_ONE;
    case FCMP_UNE: return FCMP_OEQ;
    case FCMP_UGT: return FCMP_OLE;
    case FCMP_ULT: return FCMP_OGE;
    case FCMP_UGE: return FCMP_OLT;
    case FCMP_ULE: return FCMP_OGT;
    case FCMP_ORD: return FCMP_UNO;
    case FCMP_UNO: return FCMP_ORD;
    case FCMP_TRUE: return FCMP_FALSE;
    case FCMP_FALSE: return FCMP_TRUE;
  }
}

ICmpInst::Predicate ICmpInst::getSignedPredicate(Predicate pred) {
  switch (pred) {
    default: llvm_unreachable("Unknown icmp predicate!");
    case ICMP_EQ: case ICMP_NE: 
    case ICMP_SGT: case ICMP_SLT: case ICMP_SGE: case ICMP_SLE: 
       return pred;
    case ICMP_UGT: return ICMP_SGT;
    case ICMP_ULT: return ICMP_SLT;
    case ICMP_UGE: return ICMP_SGE;
    case ICMP_ULE: return ICMP_SLE;
  }
}

ICmpInst::Predicate ICmpInst::getUnsignedPredicate(Predicate pred) {
  switch (pred) {
    default: llvm_unreachable("Unknown icmp predicate!");
    case ICMP_EQ: case ICMP_NE: 
    case ICMP_UGT: case ICMP_ULT: case ICMP_UGE: case ICMP_ULE: 
       return pred;
    case ICMP_SGT: return ICMP_UGT;
    case ICMP_SLT: return ICMP_ULT;
    case ICMP_SGE: return ICMP_UGE;
    case ICMP_SLE: return ICMP_ULE;
  }
}

/// Initialize a set of values that all satisfy the condition with C.
///
ConstantRange 
ICmpInst::makeConstantRange(Predicate pred, const APInt &C) {
  APInt Lower(C);
  APInt Upper(C);
  uint32_t BitWidth = C.getBitWidth();
  switch (pred) {
  default: llvm_unreachable("Invalid ICmp opcode to ConstantRange ctor!");
  case ICmpInst::ICMP_EQ: ++Upper; break;
  case ICmpInst::ICMP_NE: ++Lower; break;
  case ICmpInst::ICMP_ULT:
    Lower = APInt::getMinValue(BitWidth);
    // Check for an empty-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/false);
    break;
  case ICmpInst::ICMP_SLT:
    Lower = APInt::getSignedMinValue(BitWidth);
    // Check for an empty-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/false);
    break;
  case ICmpInst::ICMP_UGT: 
    ++Lower; Upper = APInt::getMinValue(BitWidth);        // Min = Next(Max)
    // Check for an empty-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/false);
    break;
  case ICmpInst::ICMP_SGT:
    ++Lower; Upper = APInt::getSignedMinValue(BitWidth);  // Min = Next(Max)
    // Check for an empty-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/false);
    break;
  case ICmpInst::ICMP_ULE: 
    Lower = APInt::getMinValue(BitWidth); ++Upper; 
    // Check for a full-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/true);
    break;
  case ICmpInst::ICMP_SLE: 
    Lower = APInt::getSignedMinValue(BitWidth); ++Upper; 
    // Check for a full-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/true);
    break;
  case ICmpInst::ICMP_UGE:
    Upper = APInt::getMinValue(BitWidth);        // Min = Next(Max)
    // Check for a full-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/true);
    break;
  case ICmpInst::ICMP_SGE:
    Upper = APInt::getSignedMinValue(BitWidth);  // Min = Next(Max)
    // Check for a full-set condition.
    if (Lower == Upper)
      return ConstantRange(BitWidth, /*isFullSet=*/true);
    break;
  }
  return ConstantRange(Lower, Upper);
}

CmpInst::Predicate CmpInst::getSwappedPredicate(Predicate pred) {
  switch (pred) {
    default: llvm_unreachable("Unknown cmp predicate!");
    case ICMP_EQ: case ICMP_NE:
      return pred;
    case ICMP_SGT: return ICMP_SLT;
    case ICMP_SLT: return ICMP_SGT;
    case ICMP_SGE: return ICMP_SLE;
    case ICMP_SLE: return ICMP_SGE;
    case ICMP_UGT: return ICMP_ULT;
    case ICMP_ULT: return ICMP_UGT;
    case ICMP_UGE: return ICMP_ULE;
    case ICMP_ULE: return ICMP_UGE;
  
    case FCMP_FALSE: case FCMP_TRUE:
    case FCMP_OEQ: case FCMP_ONE:
    case FCMP_UEQ: case FCMP_UNE:
    case FCMP_ORD: case FCMP_UNO:
      return pred;
    case FCMP_OGT: return FCMP_OLT;
    case FCMP_OLT: return FCMP_OGT;
    case FCMP_OGE: return FCMP_OLE;
    case FCMP_OLE: return FCMP_OGE;
    case FCMP_UGT: return FCMP_ULT;
    case FCMP_ULT: return FCMP_UGT;
    case FCMP_UGE: return FCMP_ULE;
    case FCMP_ULE: return FCMP_UGE;
  }
}

bool CmpInst::isUnsigned(unsigned short predicate) {
  switch (predicate) {
    default: return false;
    case ICmpInst::ICMP_ULT: case ICmpInst::ICMP_ULE: case ICmpInst::ICMP_UGT: 
    case ICmpInst::ICMP_UGE: return true;
  }
}

bool CmpInst::isSigned(unsigned short predicate) {
  switch (predicate) {
    default: return false;
    case ICmpInst::ICMP_SLT: case ICmpInst::ICMP_SLE: case ICmpInst::ICMP_SGT: 
    case ICmpInst::ICMP_SGE: return true;
  }
}

bool CmpInst::isOrdered(unsigned short predicate) {
  switch (predicate) {
    default: return false;
    case FCmpInst::FCMP_OEQ: case FCmpInst::FCMP_ONE: case FCmpInst::FCMP_OGT: 
    case FCmpInst::FCMP_OLT: case FCmpInst::FCMP_OGE: case FCmpInst::FCMP_OLE: 
    case FCmpInst::FCMP_ORD: return true;
  }
}
      
bool CmpInst::isUnordered(unsigned short predicate) {
  switch (predicate) {
    default: return false;
    case FCmpInst::FCMP_UEQ: case FCmpInst::FCMP_UNE: case FCmpInst::FCMP_UGT: 
    case FCmpInst::FCMP_ULT: case FCmpInst::FCMP_UGE: case FCmpInst::FCMP_ULE: 
    case FCmpInst::FCMP_UNO: return true;
  }
}

bool CmpInst::isTrueWhenEqual(unsigned short predicate) {
  switch(predicate) {
    default: return false;
    case ICMP_EQ:   case ICMP_UGE: case ICMP_ULE: case ICMP_SGE: case ICMP_SLE:
    case FCMP_TRUE: case FCMP_UEQ: case FCMP_UGE: case FCMP_ULE: return true;
  }
}

bool CmpInst::isFalseWhenEqual(unsigned short predicate) {
  switch(predicate) {
  case ICMP_NE:    case ICMP_UGT: case ICMP_ULT: case ICMP_SGT: case ICMP_SLT:
  case FCMP_FALSE: case FCMP_ONE: case FCMP_OGT: case FCMP_OLT: return true;
  default: return false;
  }
}


//===----------------------------------------------------------------------===//
//                        SwitchInst Implementation
//===----------------------------------------------------------------------===//

void SwitchInst::init(Value *Value, BasicBlock *Default, unsigned NumReserved) {
  assert(Value && Default && NumReserved);
  ReservedSpace = NumReserved;
  setNumHungOffUseOperands(2);
  allocHungoffUses(ReservedSpace);

  Op<0>() = Value;
  Op<1>() = Default;
}

/// SwitchInst ctor - Create a new switch instruction, specifying a value to
/// switch on and a default destination.  The number of additional cases can
/// be specified here to make memory allocation more efficient.  This
/// constructor can also autoinsert before another instruction.
SwitchInst::SwitchInst(Value *Value, BasicBlock *Default, unsigned NumCases,
                       Instruction *InsertBefore)
  : TerminatorInst(Type::getVoidTy(Value->getContext()), Instruction::Switch,
                   nullptr, 0, InsertBefore) {
  init(Value, Default, 2+NumCases*2);
}

/// SwitchInst ctor - Create a new switch instruction, specifying a value to
/// switch on and a default destination.  The number of additional cases can
/// be specified here to make memory allocation more efficient.  This
/// constructor also autoinserts at the end of the specified BasicBlock.
SwitchInst::SwitchInst(Value *Value, BasicBlock *Default, unsigned NumCases,
                       BasicBlock *InsertAtEnd)
  : TerminatorInst(Type::getVoidTy(Value->getContext()), Instruction::Switch,
                   nullptr, 0, InsertAtEnd) {
  init(Value, Default, 2+NumCases*2);
}

SwitchInst::SwitchInst(const SwitchInst &SI)
  : TerminatorInst(SI.getType(), Instruction::Switch, nullptr, 0) {
  init(SI.getCondition(), SI.getDefaultDest(), SI.getNumOperands());
  setNumHungOffUseOperands(SI.getNumOperands());
  Use *OL = getOperandList();
  const Use *InOL = SI.getOperandList();
  for (unsigned i = 2, E = SI.getNumOperands(); i != E; i += 2) {
    OL[i] = InOL[i];
    OL[i+1] = InOL[i+1];
  }
  SubclassOptionalData = SI.SubclassOptionalData;
}


/// addCase - Add an entry to the switch instruction...
///
void SwitchInst::addCase(ConstantInt *OnVal, BasicBlock *Dest) {
  unsigned NewCaseIdx = getNumCases();
  unsigned OpNo = getNumOperands();
  if (OpNo+2 > ReservedSpace)
    growOperands();  // Get more space!
  // Initialize some new operands.
  assert(OpNo+1 < ReservedSpace && "Growing didn't work!");
  setNumHungOffUseOperands(OpNo+2);
  CaseIt Case(this, NewCaseIdx);
  Case.setValue(OnVal);
  Case.setSuccessor(Dest);
}

/// removeCase - This method removes the specified case and its successor
/// from the switch instruction.
void SwitchInst::removeCase(CaseIt i) {
  unsigned idx = i.getCaseIndex();
  
  assert(2 + idx*2 < getNumOperands() && "Case index out of range!!!");

  unsigned NumOps = getNumOperands();
  Use *OL = getOperandList();

  // Overwrite this case with the end of the list.
  if (2 + (idx + 1) * 2 != NumOps) {
    OL[2 + idx * 2] = OL[NumOps - 2];
    OL[2 + idx * 2 + 1] = OL[NumOps - 1];
  }

  // Nuke the last value.
  OL[NumOps-2].set(nullptr);
  OL[NumOps-2+1].set(nullptr);
  setNumHungOffUseOperands(NumOps-2);
}

/// growOperands - grow operands - This grows the operand list in response
/// to a push_back style of operation.  This grows the number of ops by 3 times.
///
void SwitchInst::growOperands() {
  unsigned e = getNumOperands();
  unsigned NumOps = e*3;

  ReservedSpace = NumOps;
  growHungoffUses(ReservedSpace);
}


BasicBlock *SwitchInst::getSuccessorV(unsigned idx) const {
  return getSuccessor(idx);
}
unsigned SwitchInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}
void SwitchInst::setSuccessorV(unsigned idx, BasicBlock *B) {
  setSuccessor(idx, B);
}

//===----------------------------------------------------------------------===//
//                        IndirectBrInst Implementation
//===----------------------------------------------------------------------===//

void IndirectBrInst::init(Value *Address, unsigned NumDests) {
  assert(Address && Address->getType()->isPointerTy() &&
         "Address of indirectbr must be a pointer");
  ReservedSpace = 1+NumDests;
  setNumHungOffUseOperands(1);
  allocHungoffUses(ReservedSpace);

  Op<0>() = Address;
}


/// growOperands - grow operands - This grows the operand list in response
/// to a push_back style of operation.  This grows the number of ops by 2 times.
///
void IndirectBrInst::growOperands() {
  unsigned e = getNumOperands();
  unsigned NumOps = e*2;
  
  ReservedSpace = NumOps;
  growHungoffUses(ReservedSpace);
}

IndirectBrInst::IndirectBrInst(Value *Address, unsigned NumCases,
                               Instruction *InsertBefore)
: TerminatorInst(Type::getVoidTy(Address->getContext()),Instruction::IndirectBr,
                 nullptr, 0, InsertBefore) {
  init(Address, NumCases);
}

IndirectBrInst::IndirectBrInst(Value *Address, unsigned NumCases,
                               BasicBlock *InsertAtEnd)
: TerminatorInst(Type::getVoidTy(Address->getContext()),Instruction::IndirectBr,
                 nullptr, 0, InsertAtEnd) {
  init(Address, NumCases);
}

IndirectBrInst::IndirectBrInst(const IndirectBrInst &IBI)
    : TerminatorInst(Type::getVoidTy(IBI.getContext()), Instruction::IndirectBr,
                     nullptr, IBI.getNumOperands()) {
  allocHungoffUses(IBI.getNumOperands());
  Use *OL = getOperandList();
  const Use *InOL = IBI.getOperandList();
  for (unsigned i = 0, E = IBI.getNumOperands(); i != E; ++i)
    OL[i] = InOL[i];
  SubclassOptionalData = IBI.SubclassOptionalData;
}

/// addDestination - Add a destination.
///
void IndirectBrInst::addDestination(BasicBlock *DestBB) {
  unsigned OpNo = getNumOperands();
  if (OpNo+1 > ReservedSpace)
    growOperands();  // Get more space!
  // Initialize some new operands.
  assert(OpNo < ReservedSpace && "Growing didn't work!");
  setNumHungOffUseOperands(OpNo+1);
  getOperandList()[OpNo] = DestBB;
}

/// removeDestination - This method removes the specified successor from the
/// indirectbr instruction.
void IndirectBrInst::removeDestination(unsigned idx) {
  assert(idx < getNumOperands()-1 && "Successor index out of range!");
  
  unsigned NumOps = getNumOperands();
  Use *OL = getOperandList();

  // Replace this value with the last one.
  OL[idx+1] = OL[NumOps-1];
  
  // Nuke the last value.
  OL[NumOps-1].set(nullptr);
  setNumHungOffUseOperands(NumOps-1);
}

BasicBlock *IndirectBrInst::getSuccessorV(unsigned idx) const {
  return getSuccessor(idx);
}
unsigned IndirectBrInst::getNumSuccessorsV() const {
  return getNumSuccessors();
}
void IndirectBrInst::setSuccessorV(unsigned idx, BasicBlock *B) {
  setSuccessor(idx, B);
}

//===----------------------------------------------------------------------===//
//                           cloneImpl() implementations
//===----------------------------------------------------------------------===//

// Define these methods here so vtables don't get emitted into every translation
// unit that uses these classes.

GetElementPtrInst *GetElementPtrInst::cloneImpl() const {
  return new (getNumOperands()) GetElementPtrInst(*this);
}

BinaryOperator *BinaryOperator::cloneImpl() const {
  return Create(getOpcode(), Op<0>(), Op<1>());
}

FCmpInst *FCmpInst::cloneImpl() const {
  return new FCmpInst(getPredicate(), Op<0>(), Op<1>());
}

ICmpInst *ICmpInst::cloneImpl() const {
  return new ICmpInst(getPredicate(), Op<0>(), Op<1>());
}

ExtractValueInst *ExtractValueInst::cloneImpl() const {
  return new ExtractValueInst(*this);
}

InsertValueInst *InsertValueInst::cloneImpl() const {
  return new InsertValueInst(*this);
}

AllocaInst *AllocaInst::cloneImpl() const {
  AllocaInst *Result = new AllocaInst(getAllocatedType(),
                                      (Value *)getOperand(0), getAlignment());
  Result->setUsedWithInAlloca(isUsedWithInAlloca());
  return Result;
}

LoadInst *LoadInst::cloneImpl() const {
  return new LoadInst(getOperand(0), Twine(), isVolatile(),
                      getAlignment(), getOrdering(), getSynchScope());
}

StoreInst *StoreInst::cloneImpl() const {
  return new StoreInst(getOperand(0), getOperand(1), isVolatile(),
                       getAlignment(), getOrdering(), getSynchScope());
  
}

AtomicCmpXchgInst *AtomicCmpXchgInst::cloneImpl() const {
  AtomicCmpXchgInst *Result =
    new AtomicCmpXchgInst(getOperand(0), getOperand(1), getOperand(2),
                          getSuccessOrdering(), getFailureOrdering(),
                          getSynchScope());
  Result->setVolatile(isVolatile());
  Result->setWeak(isWeak());
  return Result;
}

AtomicRMWInst *AtomicRMWInst::cloneImpl() const {
  AtomicRMWInst *Result =
    new AtomicRMWInst(getOperation(),getOperand(0), getOperand(1),
                      getOrdering(), getSynchScope());
  Result->setVolatile(isVolatile());
  return Result;
}

FenceInst *FenceInst::cloneImpl() const {
  return new FenceInst(getContext(), getOrdering(), getSynchScope());
}

TruncInst *TruncInst::cloneImpl() const {
  return new TruncInst(getOperand(0), getType());
}

ZExtInst *ZExtInst::cloneImpl() const {
  return new ZExtInst(getOperand(0), getType());
}

SExtInst *SExtInst::cloneImpl() const {
  return new SExtInst(getOperand(0), getType());
}

FPTruncInst *FPTruncInst::cloneImpl() const {
  return new FPTruncInst(getOperand(0), getType());
}

FPExtInst *FPExtInst::cloneImpl() const {
  return new FPExtInst(getOperand(0), getType());
}

UIToFPInst *UIToFPInst::cloneImpl() const {
  return new UIToFPInst(getOperand(0), getType());
}

SIToFPInst *SIToFPInst::cloneImpl() const {
  return new SIToFPInst(getOperand(0), getType());
}

FPToUIInst *FPToUIInst::cloneImpl() const {
  return new FPToUIInst(getOperand(0), getType());
}

FPToSIInst *FPToSIInst::cloneImpl() const {
  return new FPToSIInst(getOperand(0), getType());
}

PtrToIntInst *PtrToIntInst::cloneImpl() const {
  return new PtrToIntInst(getOperand(0), getType());
}

IntToPtrInst *IntToPtrInst::cloneImpl() const {
  return new IntToPtrInst(getOperand(0), getType());
}

BitCastInst *BitCastInst::cloneImpl() const {
  return new BitCastInst(getOperand(0), getType());
}

AddrSpaceCastInst *AddrSpaceCastInst::cloneImpl() const {
  return new AddrSpaceCastInst(getOperand(0), getType());
}

CallInst *CallInst::cloneImpl() const {
  return  new(getNumOperands()) CallInst(*this);
}

SelectInst *SelectInst::cloneImpl() const {
  return SelectInst::Create(getOperand(0), getOperand(1), getOperand(2));
}

VAArgInst *VAArgInst::cloneImpl() const {
  return new VAArgInst(getOperand(0), getType());
}

ExtractElementInst *ExtractElementInst::cloneImpl() const {
  return ExtractElementInst::Create(getOperand(0), getOperand(1));
}

InsertElementInst *InsertElementInst::cloneImpl() const {
  return InsertElementInst::Create(getOperand(0), getOperand(1), getOperand(2));
}

ShuffleVectorInst *ShuffleVectorInst::cloneImpl() const {
  return new ShuffleVectorInst(getOperand(0), getOperand(1), getOperand(2));
}

PHINode *PHINode::cloneImpl() const { return new PHINode(*this); }

LandingPadInst *LandingPadInst::cloneImpl() const {
  return new LandingPadInst(*this);
}

ReturnInst *ReturnInst::cloneImpl() const {
  return new(getNumOperands()) ReturnInst(*this);
}

BranchInst *BranchInst::cloneImpl() const {
  return new(getNumOperands()) BranchInst(*this);
}

SwitchInst *SwitchInst::cloneImpl() const { return new SwitchInst(*this); }

IndirectBrInst *IndirectBrInst::cloneImpl() const {
  return new IndirectBrInst(*this);
}

InvokeInst *InvokeInst::cloneImpl() const {
  return new(getNumOperands()) InvokeInst(*this);
}

ResumeInst *ResumeInst::cloneImpl() const { return new (1) ResumeInst(*this); }

UnreachableInst *UnreachableInst::cloneImpl() const {
  LLVMContext &Context = getContext();
  return new UnreachableInst(Context);
}
