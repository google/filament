///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLOperations.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implementation of DXIL operations.                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/HLSL/HLOperations.h"
#include "dxc/HlslIntrinsicOp.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace hlsl;
using namespace llvm;

namespace hlsl {

const char HLPrefixStr[] = "dx.hl";
const char *const HLPrefix = HLPrefixStr;
static const char HLLowerStrategyStr[] = "dx.hlls";
static const char *const HLLowerStrategy = HLLowerStrategyStr;

static const char HLWaveSensitiveStr[] = "dx.wave-sensitive";
static const char *const HLWaveSensitive = HLWaveSensitiveStr;

static StringRef HLOpcodeGroupNames[]{
    "notHLDXIL",    // NotHL,
    "<ext>",        // HLExtIntrinsic - should always refer through extension
    "op",           // HLIntrinsic,
    "cast",         // HLCast,
    "init",         // HLInit,
    "binop",        // HLBinOp,
    "unop",         // HLUnOp,
    "subscript",    // HLSubscript,
    "matldst",      // HLMatLoadStore,
    "select",       // HLSelect,
    "createhandle", // HLCreateHandle,
    "createnodeoutputhandle",      // HLCreateNodeOutputHandle
    "indexnodehandle",             // HLIndexNodeHandle:
    "createnodeinputrecordhandle", // HLCreateNodeInputRecordHandle
    "annotatehandle",              // HLAnnotateHandle,
    "annotatenodehandle",          // HLAnnotateNodeHandle
    "annotatenoderecordhandle",    // HLAnnotateNodeRecordHandle
    "numOfHLDXIL",                 // NumOfHLOps
};
static_assert(_countof(HLOpcodeGroupNames) ==
                  1 + (size_t)HLOpcodeGroup::NumOfHLOps,
              "otherwise, tables out of sync");

static StringRef HLOpcodeGroupFullNames[]{
    "notHLDXIL",       // NotHL,
    "<ext>",           // HLExtIntrinsic - should aways refer through extension
    "dx.hl.op",        // HLIntrinsic,
    "dx.hl.cast",      // HLCast,
    "dx.hl.init",      // HLInit,
    "dx.hl.binop",     // HLBinOp,
    "dx.hl.unop",      // HLUnOp,
    "dx.hl.subscript", // HLSubscript,
    "dx.hl.matldst",   // HLMatLoadStore,
    "dx.hl.select",    // HLSelect,
    "dx.hl.createhandle",                // HLCreateHandle,
    "dx.hl.createnodeoutputhandle",      // HLCreateNodeHandle
    "dx.hl.indexnodehandle",             // HLIndexNodeHandle
    "dx.hl.createnodeinputrecordhandle", // HLCreateNodeInputRecordHandle
    "dx.hl.annotatehandle",              // HLAnnotateHandle,
    "dx.hl.annotatenodehandle",          // HLAnnotateNodeHandle,
    "dx.hl.annotatenoderecordhandle",    // HLAnnotateNodeRecordHandle
    "numOfHLDXIL",                       // NumOfHLOps
};
static_assert(_countof(HLOpcodeGroupFullNames) ==
                  1 + (size_t)HLOpcodeGroup::NumOfHLOps,
              "otherwise, tables out of sync");

static HLOpcodeGroup GetHLOpcodeGroupInternal(StringRef group) {
  return llvm::StringSwitch<HLOpcodeGroup>(group)
      .Case("op", HLOpcodeGroup::HLIntrinsic)
      .Case("cast", HLOpcodeGroup::HLCast)
      .Case("init", HLOpcodeGroup::HLInit)
      .Case("binop", HLOpcodeGroup::HLBinOp)
      .Case("unop", HLOpcodeGroup::HLUnOp)
      .Case("subscript", HLOpcodeGroup::HLSubscript)
      .Case("matldst", HLOpcodeGroup::HLMatLoadStore)
      .Case("select", HLOpcodeGroup::HLSelect)
      .Case("createhandle", HLOpcodeGroup::HLCreateHandle)
      .Case("createnodeoutputhandle", HLOpcodeGroup::HLCreateNodeOutputHandle)
      .Case("indexnodehandle", HLOpcodeGroup::HLIndexNodeHandle)
      .Case("createnodeinputrecordhandle",
            HLOpcodeGroup::HLCreateNodeInputRecordHandle)
      .Case("annotatehandle", HLOpcodeGroup::HLAnnotateHandle)
      .Case("annotatenodehandle", HLOpcodeGroup::HLAnnotateNodeHandle)
      .Case("annotatenoderecordhandle",
            HLOpcodeGroup::HLAnnotateNodeRecordHandle)
      .Default(HLOpcodeGroup::NotHL);
}

// GetHLOpGroup by function name.
HLOpcodeGroup GetHLOpcodeGroupByName(const Function *F) {
  StringRef name = F->getName();

  if (!name.startswith(HLPrefix)) {
    // This could be an external intrinsic, but this function
    // won't recognize those as such. Use GetHLOpcodeGroupByName
    // to make that distinction.
    return HLOpcodeGroup::NotHL;
  }

  const unsigned prefixSize = sizeof(HLPrefixStr);
  const unsigned groupEnd = name.find_first_of('.', prefixSize);

  StringRef group = name.substr(prefixSize, groupEnd - prefixSize);

  return GetHLOpcodeGroupInternal(group);
}

HLOpcodeGroup GetHLOpcodeGroup(llvm::Function *F) {
  llvm::StringRef name = GetHLOpcodeGroupNameByAttr(F);
  HLOpcodeGroup result = GetHLOpcodeGroupInternal(name);
  if (result == HLOpcodeGroup::NotHL) {
    result = name.empty() ? result : HLOpcodeGroup::HLExtIntrinsic;
  }
  if (result == HLOpcodeGroup::NotHL) {
    result = GetHLOpcodeGroupByName(F);
  }
  return result;
}

llvm::StringRef GetHLOpcodeGroupNameByAttr(llvm::Function *F) {
  Attribute groupAttr = F->getFnAttribute(hlsl::HLPrefix);
  StringRef group = groupAttr.getValueAsString();
  return group;
}

StringRef GetHLOpcodeGroupName(HLOpcodeGroup op) {
  switch (op) {
  case HLOpcodeGroup::HLCast:
  case HLOpcodeGroup::HLInit:
  case HLOpcodeGroup::HLBinOp:
  case HLOpcodeGroup::HLUnOp:
  case HLOpcodeGroup::HLIntrinsic:
  case HLOpcodeGroup::HLSubscript:
  case HLOpcodeGroup::HLMatLoadStore:
  case HLOpcodeGroup::HLSelect:
  case HLOpcodeGroup::HLCreateHandle:
  case HLOpcodeGroup::HLCreateNodeOutputHandle:
  case HLOpcodeGroup::HLIndexNodeHandle:
  case HLOpcodeGroup::HLCreateNodeInputRecordHandle:
  case HLOpcodeGroup::HLAnnotateHandle:
  case HLOpcodeGroup::HLAnnotateNodeHandle:
  case HLOpcodeGroup::HLAnnotateNodeRecordHandle:
    return HLOpcodeGroupNames[static_cast<unsigned>(op)];
  default:
    llvm_unreachable("invalid op");

    return "";
  }
}
StringRef GetHLOpcodeGroupFullName(HLOpcodeGroup op) {
  switch (op) {
  case HLOpcodeGroup::HLCast:
  case HLOpcodeGroup::HLInit:
  case HLOpcodeGroup::HLBinOp:
  case HLOpcodeGroup::HLUnOp:
  case HLOpcodeGroup::HLIntrinsic:
  case HLOpcodeGroup::HLSubscript:
  case HLOpcodeGroup::HLMatLoadStore:
  case HLOpcodeGroup::HLSelect:
  case HLOpcodeGroup::HLCreateHandle:
  case HLOpcodeGroup::HLCreateNodeOutputHandle:
  case HLOpcodeGroup::HLIndexNodeHandle:
  case HLOpcodeGroup::HLCreateNodeInputRecordHandle:
  case HLOpcodeGroup::HLAnnotateHandle:
  case HLOpcodeGroup::HLAnnotateNodeHandle:
  case HLOpcodeGroup::HLAnnotateNodeRecordHandle:
    return HLOpcodeGroupFullNames[static_cast<unsigned>(op)];
  default:
    llvm_unreachable("invalid op");
    return "";
  }
}

llvm::StringRef GetHLOpcodeName(HLUnaryOpcode Op) {
  switch (Op) {
  case HLUnaryOpcode::PostInc:
    return "++";
  case HLUnaryOpcode::PostDec:
    return "--";
  case HLUnaryOpcode::PreInc:
    return "++";
  case HLUnaryOpcode::PreDec:
    return "--";
  case HLUnaryOpcode::Plus:
    return "+";
  case HLUnaryOpcode::Minus:
    return "-";
  case HLUnaryOpcode::Not:
    return "~";
  case HLUnaryOpcode::LNot:
    return "!";
  case HLUnaryOpcode::Invalid:
  case HLUnaryOpcode::NumOfUO:
    // Invalid Unary Ops
    break;
  }
  llvm_unreachable("Unknown unary operator");
}

llvm::StringRef GetHLOpcodeName(HLBinaryOpcode Op) {
  switch (Op) {
  case HLBinaryOpcode::Mul:
    return "*";
  case HLBinaryOpcode::UDiv:
  case HLBinaryOpcode::Div:
    return "/";
  case HLBinaryOpcode::URem:
  case HLBinaryOpcode::Rem:
    return "%";
  case HLBinaryOpcode::Add:
    return "+";
  case HLBinaryOpcode::Sub:
    return "-";
  case HLBinaryOpcode::Shl:
    return "<<";
  case HLBinaryOpcode::UShr:
  case HLBinaryOpcode::Shr:
    return ">>";
  case HLBinaryOpcode::ULT:
  case HLBinaryOpcode::LT:
    return "<";
  case HLBinaryOpcode::UGT:
  case HLBinaryOpcode::GT:
    return ">";
  case HLBinaryOpcode::ULE:
  case HLBinaryOpcode::LE:
    return "<=";
  case HLBinaryOpcode::UGE:
  case HLBinaryOpcode::GE:
    return ">=";
  case HLBinaryOpcode::EQ:
    return "==";
  case HLBinaryOpcode::NE:
    return "!=";
  case HLBinaryOpcode::And:
    return "&";
  case HLBinaryOpcode::Xor:
    return "^";
  case HLBinaryOpcode::Or:
    return "|";
  case HLBinaryOpcode::LAnd:
    return "&&";
  case HLBinaryOpcode::LOr:
    return "||";
  case HLBinaryOpcode::Invalid:
  case HLBinaryOpcode::NumOfBO:
    // Invalid Binary Ops
    break;
  }

  llvm_unreachable("Invalid OpCode!");
}

llvm::StringRef GetHLOpcodeName(HLSubscriptOpcode Op) {
  switch (Op) {
  case HLSubscriptOpcode::DefaultSubscript:
    return "[]";
  case HLSubscriptOpcode::ColMatSubscript:
    return "colMajor[]";
  case HLSubscriptOpcode::RowMatSubscript:
    return "rowMajor[]";
  case HLSubscriptOpcode::ColMatElement:
    return "colMajor_m";
  case HLSubscriptOpcode::RowMatElement:
    return "rowMajor_m";
  case HLSubscriptOpcode::DoubleSubscript:
    return "[][]";
  case HLSubscriptOpcode::CBufferSubscript:
    return "cb";
  case HLSubscriptOpcode::VectorSubscript:
    return "vector[]";
  }
  return "";
}

llvm::StringRef GetHLOpcodeName(HLCastOpcode Op) {
  switch (Op) {
  case HLCastOpcode::DefaultCast:
    return "default";
  case HLCastOpcode::ToUnsignedCast:
    return "toUnsigned";
  case HLCastOpcode::FromUnsignedCast:
    return "fromUnsigned";
  case HLCastOpcode::UnsignedUnsignedCast:
    return "unsignedUnsigned";
  case HLCastOpcode::ColMatrixToVecCast:
    return "colMatToVec";
  case HLCastOpcode::RowMatrixToVecCast:
    return "rowMatToVec";
  case HLCastOpcode::ColMatrixToRowMatrix:
    return "colMatToRowMat";
  case HLCastOpcode::RowMatrixToColMatrix:
    return "rowMatToColMat";
  case HLCastOpcode::HandleToResCast:
    return "handleToRes";
  }
  return "";
}

llvm::StringRef GetHLOpcodeName(HLMatLoadStoreOpcode Op) {
  switch (Op) {
  case HLMatLoadStoreOpcode::ColMatLoad:
    return "colLoad";
  case HLMatLoadStoreOpcode::ColMatStore:
    return "colStore";
  case HLMatLoadStoreOpcode::RowMatLoad:
    return "rowLoad";
  case HLMatLoadStoreOpcode::RowMatStore:
    return "rowStore";
  }
  llvm_unreachable("invalid matrix load store operator");
}

StringRef GetHLLowerStrategy(Function *F) {
  llvm::Attribute A = F->getFnAttribute(HLLowerStrategy);
  llvm::StringRef LowerStrategy = A.getValueAsString();
  return LowerStrategy;
}

void SetHLLowerStrategy(Function *F, StringRef S) {
  F->addFnAttr(HLLowerStrategy, S);
}

// Set function attribute indicating wave-sensitivity
void SetHLWaveSensitive(Function *F) { F->addFnAttr(HLWaveSensitive, "y"); }

// Return if this Function is dependent on other wave members indicated by
// attribute
bool IsHLWaveSensitive(Function *F) {
  AttributeSet attrSet = F->getAttributes();
  return attrSet.hasAttribute(AttributeSet::FunctionIndex, HLWaveSensitive);
}

static std::string GetHLFunctionAttributeMangling(const AttributeSet &attribs);

std::string GetHLFullName(HLOpcodeGroup op, unsigned opcode,
                          const AttributeSet &attribs = AttributeSet()) {
  assert(op != HLOpcodeGroup::HLExtIntrinsic &&
         "else table name should be used");
  std::string opName = GetHLOpcodeGroupFullName(op).str() + ".";

  switch (op) {
  case HLOpcodeGroup::HLBinOp: {
    HLBinaryOpcode binOp = static_cast<HLBinaryOpcode>(opcode);
    return opName + GetHLOpcodeName(binOp).str();
  }
  case HLOpcodeGroup::HLUnOp: {
    HLUnaryOpcode unOp = static_cast<HLUnaryOpcode>(opcode);
    return opName + GetHLOpcodeName(unOp).str();
  }
  case HLOpcodeGroup::HLIntrinsic: {
    // intrinsic with same signature will share the funciton now
    // The opcode is in arg0.
    return opName + GetHLFunctionAttributeMangling(attribs);
  }
  case HLOpcodeGroup::HLMatLoadStore: {
    HLMatLoadStoreOpcode matOp = static_cast<HLMatLoadStoreOpcode>(opcode);
    return opName + GetHLOpcodeName(matOp).str();
  }
  case HLOpcodeGroup::HLSubscript: {
    HLSubscriptOpcode subOp = static_cast<HLSubscriptOpcode>(opcode);
    return opName + GetHLOpcodeName(subOp).str() + "." +
           GetHLFunctionAttributeMangling(attribs);
  }
  case HLOpcodeGroup::HLCast: {
    HLCastOpcode castOp = static_cast<HLCastOpcode>(opcode);
    return opName + GetHLOpcodeName(castOp).str();
  }
  case HLOpcodeGroup::HLCreateHandle:
  case HLOpcodeGroup::HLAnnotateHandle:
    return opName;
  default:
    return opName + GetHLFunctionAttributeMangling(attribs);
  }
}

// Get opcode from arg0 of function call.
unsigned GetHLOpcode(const CallInst *CI) {
  Value *idArg = CI->getArgOperand(HLOperandIndex::kOpcodeIdx);
  Constant *idConst = cast<Constant>(idArg);
  return idConst->getUniqueInteger().getLimitedValue();
}

unsigned GetRowMajorOpcode(HLOpcodeGroup group, unsigned opcode) {
  switch (group) {
  case HLOpcodeGroup::HLMatLoadStore: {
    HLMatLoadStoreOpcode matOp = static_cast<HLMatLoadStoreOpcode>(opcode);
    switch (matOp) {
    case HLMatLoadStoreOpcode::ColMatLoad:
      return static_cast<unsigned>(HLMatLoadStoreOpcode::RowMatLoad);
    case HLMatLoadStoreOpcode::ColMatStore:
      return static_cast<unsigned>(HLMatLoadStoreOpcode::RowMatStore);
    default:
      return opcode;
    }
  } break;
  case HLOpcodeGroup::HLSubscript: {
    HLSubscriptOpcode subOp = static_cast<HLSubscriptOpcode>(opcode);
    switch (subOp) {
    case HLSubscriptOpcode::ColMatElement:
      return static_cast<unsigned>(HLSubscriptOpcode::RowMatElement);
    case HLSubscriptOpcode::ColMatSubscript:
      return static_cast<unsigned>(HLSubscriptOpcode::RowMatSubscript);
    default:
      return opcode;
    }
  } break;
  default:
    return opcode;
  }
}

unsigned GetUnsignedOpcode(unsigned opcode) {
  return GetUnsignedIntrinsicOpcode(static_cast<IntrinsicOp>(opcode));
}

// For HLBinaryOpcode
bool HasUnsignedOpcode(HLBinaryOpcode opcode) {
  switch (opcode) {
  case HLBinaryOpcode::Div:
  case HLBinaryOpcode::Rem:
  case HLBinaryOpcode::Shr:
  case HLBinaryOpcode::LT:
  case HLBinaryOpcode::GT:
  case HLBinaryOpcode::LE:
  case HLBinaryOpcode::GE:
    return true;
  default:
    return false;
  }
}

HLBinaryOpcode GetUnsignedOpcode(HLBinaryOpcode opcode) {
  switch (opcode) {
  case HLBinaryOpcode::Div:
    return HLBinaryOpcode::UDiv;
  case HLBinaryOpcode::Rem:
    return HLBinaryOpcode::URem;
  case HLBinaryOpcode::Shr:
    return HLBinaryOpcode::UShr;
  case HLBinaryOpcode::LT:
    return HLBinaryOpcode::ULT;
  case HLBinaryOpcode::GT:
    return HLBinaryOpcode::UGT;
  case HLBinaryOpcode::LE:
    return HLBinaryOpcode::ULE;
  case HLBinaryOpcode::GE:
    return HLBinaryOpcode::UGE;
  default:
    return opcode;
  }
}

static AttributeSet GetHLFunctionAttributes(LLVMContext &C,
                                            FunctionType *funcTy,
                                            const AttributeSet &origAttribs,
                                            HLOpcodeGroup group,
                                            unsigned opcode) {
  // Always add nounwind
  AttributeSet attribs =
      AttributeSet::get(C, AttributeSet::FunctionIndex,
                        ArrayRef<Attribute::AttrKind>({Attribute::NoUnwind}));

  auto addAttr = [&](Attribute::AttrKind Attr) {
    if (!attribs.hasAttribute(AttributeSet::FunctionIndex, Attr))
      attribs = attribs.addAttribute(C, AttributeSet::FunctionIndex, Attr);
  };
  auto copyAttr = [&](Attribute::AttrKind Attr) {
    if (origAttribs.hasAttribute(AttributeSet::FunctionIndex, Attr))
      addAttr(Attr);
  };
  auto copyStrAttr = [&](StringRef Kind) {
    if (origAttribs.hasAttribute(AttributeSet::FunctionIndex, Kind))
      attribs = attribs.addAttribute(
          C, AttributeSet::FunctionIndex, Kind,
          origAttribs.getAttribute(AttributeSet::FunctionIndex, Kind)
              .getValueAsString());
  };

  // Copy attributes we preserve from the original function.
  copyAttr(Attribute::ReadOnly);
  copyAttr(Attribute::ReadNone);
  copyStrAttr(HLWaveSensitive);

  switch (group) {
  case HLOpcodeGroup::HLUnOp:
  case HLOpcodeGroup::HLBinOp:
  case HLOpcodeGroup::HLCast:
  case HLOpcodeGroup::HLSubscript:
    addAttr(Attribute::ReadNone);
    break;
  case HLOpcodeGroup::HLInit:
    if (!funcTy->getReturnType()->isVoidTy()) {
      addAttr(Attribute::ReadNone);
    }
    break;
  case HLOpcodeGroup::HLMatLoadStore: {
    HLMatLoadStoreOpcode matOp = static_cast<HLMatLoadStoreOpcode>(opcode);
    if (matOp == HLMatLoadStoreOpcode::ColMatLoad ||
        matOp == HLMatLoadStoreOpcode::RowMatLoad)
      addAttr(Attribute::ReadOnly);
  } break;
  case HLOpcodeGroup::HLCreateHandle: {
    addAttr(Attribute::ReadNone);
  } break;
  case HLOpcodeGroup::HLAnnotateHandle: {
    addAttr(Attribute::ReadNone);
  } break;
  case HLOpcodeGroup::HLIntrinsic: {
    IntrinsicOp intrinsicOp = static_cast<IntrinsicOp>(opcode);
    switch (intrinsicOp) {
    default:
      break;
    case IntrinsicOp::IOP_DeviceMemoryBarrierWithGroupSync:
    case IntrinsicOp::IOP_DeviceMemoryBarrier:
    case IntrinsicOp::IOP_GroupMemoryBarrierWithGroupSync:
    case IntrinsicOp::IOP_GroupMemoryBarrier:
    case IntrinsicOp::IOP_AllMemoryBarrierWithGroupSync:
    case IntrinsicOp::IOP_AllMemoryBarrier:
      addAttr(Attribute::NoDuplicate);
      break;
    }
  } break;
  case HLOpcodeGroup::NotHL:
  case HLOpcodeGroup::HLExtIntrinsic:
  case HLOpcodeGroup::HLSelect:
  case HLOpcodeGroup::NumOfHLOps:
    // No default attributes for these opcodes.
    break;
  }
  assert(!(attribs.hasAttribute(AttributeSet::FunctionIndex,
                                Attribute::ReadNone) &&
           attribs.hasAttribute(AttributeSet::FunctionIndex,
                                Attribute::ReadOnly)) &&
         "conflicting ReadNone and ReadOnly attributes");
  return attribs;
}

static std::string GetHLFunctionAttributeMangling(const AttributeSet &attribs) {
  std::string mangledName;
  raw_string_ostream mangledNameStr(mangledName);

  // Capture for adding in canonical order later.
  bool ReadNone = false;
  bool ReadOnly = false;
  bool ArgMemOnly = false;
  bool NoDuplicate = false;
  bool WaveSensitive = false;

  // Ensure every function attribute is recognized.
  for (unsigned Slot = 0; Slot < attribs.getNumSlots(); Slot++) {
    if (attribs.getSlotIndex(Slot) == AttributeSet::FunctionIndex) {
      for (auto it = attribs.begin(Slot), e = attribs.end(Slot); it != e;
           it++) {
        if (it->isEnumAttribute()) {
          switch (it->getKindAsEnum()) {
          case Attribute::ReadNone:
            ReadNone = true;
            break;
          case Attribute::ReadOnly:
            ReadOnly = true;
            break;
          case Attribute::ArgMemOnly:
            ArgMemOnly = true;
            break;
          case Attribute::NoDuplicate:
            NoDuplicate = true;
            break;
          case Attribute::NoUnwind:
            // All intrinsics have this attribute, so mangling is unaffected.
            break;
          default:
            assert(false && "unexpected attribute for HLOperation");
          }
        } else if (it->isStringAttribute()) {
          StringRef Kind = it->getKindAsString();
          if (Kind == HLWaveSensitive) {
            assert(it->getValueAsString() == "y" &&
                   "otherwise, unexpected value for WaveSensitive attribute");
            WaveSensitive = true;
          } else {
            assert(Kind == "dx.hlls" &&
                   "unexpected string function attribute for HLOperation");
          }
        }
      }
    }
  }

  // Validate attribute combinations.
  assert(!(ReadNone && ReadOnly && ArgMemOnly) &&
         "ReadNone, ReadOnly, and ArgMemOnly are mutually exclusive");

  // Add mangling in canonical order
  if (NoDuplicate)
    mangledNameStr << "nd";
  if (ReadNone)
    mangledNameStr << "rn";
  if (ReadOnly)
    mangledNameStr << "ro";
  if (WaveSensitive)
    mangledNameStr << "wave";
  return mangledName;
}

Function *GetOrCreateHLFunction(Module &M, FunctionType *funcTy,
                                HLOpcodeGroup group, unsigned opcode) {
  AttributeSet attribs;
  return GetOrCreateHLFunction(M, funcTy, group, nullptr, nullptr, opcode,
                               attribs);
}

Function *GetOrCreateHLFunction(Module &M, FunctionType *funcTy,
                                HLOpcodeGroup group, StringRef *groupName,
                                StringRef *fnName, unsigned opcode) {
  AttributeSet attribs;
  return GetOrCreateHLFunction(M, funcTy, group, groupName, fnName, opcode,
                               attribs);
}

Function *GetOrCreateHLFunction(Module &M, FunctionType *funcTy,
                                HLOpcodeGroup group, unsigned opcode,
                                const AttributeSet &attribs) {
  return GetOrCreateHLFunction(M, funcTy, group, nullptr, nullptr, opcode,
                               attribs);
}

Function *GetOrCreateHLFunction(Module &M, FunctionType *funcTy,
                                HLOpcodeGroup group, StringRef *groupName,
                                StringRef *fnName, unsigned opcode,
                                const AttributeSet &origAttribs) {
  // Set/transfer all common attributes
  AttributeSet attribs = GetHLFunctionAttributes(M.getContext(), funcTy,
                                                 origAttribs, group, opcode);

  std::string mangledName;
  raw_string_ostream mangledNameStr(mangledName);
  if (group == HLOpcodeGroup::HLExtIntrinsic) {
    assert(groupName && "else intrinsic should have been rejected");
    assert(fnName && "else intrinsic should have been rejected");
    mangledNameStr << *groupName;
    mangledNameStr << '.';
    mangledNameStr << *fnName;
    attribs = attribs.addAttribute(M.getContext(), AttributeSet::FunctionIndex,
                                   hlsl::HLPrefix, *groupName);
  } else {
    mangledNameStr << GetHLFullName(group, opcode, attribs);
    mangledNameStr << '.';
    funcTy->print(mangledNameStr);
  }

  mangledNameStr.flush();

  // Avoid getOrInsertFunction to verify attributes and type without casting.
  Function *F = cast_or_null<Function>(M.getNamedValue(mangledName));
  if (F) {
    assert(F->getFunctionType() == funcTy &&
           "otherwise, function type mismatch not captured by mangling");
    // Compare attribute mangling to ensure function attributes are as expected.
    assert(
        GetHLFunctionAttributeMangling(F->getAttributes().getFnAttributes()) ==
            GetHLFunctionAttributeMangling(attribs) &&
        "otherwise, function attribute mismatch not captured by mangling");
  } else {
    F = cast<Function>(M.getOrInsertFunction(mangledName, funcTy, attribs));
  }

  return F;
}

// HLFunction with body cannot share with HLFunction without body.
// So need add name.
Function *GetOrCreateHLFunctionWithBody(Module &M, FunctionType *funcTy,
                                        HLOpcodeGroup group, unsigned opcode,
                                        StringRef name) {
  // Set/transfer all common attributes
  AttributeSet attribs = GetHLFunctionAttributes(M.getContext(), funcTy,
                                                 AttributeSet(), group, opcode);

  std::string operatorName = GetHLFullName(group, opcode, attribs);
  std::string mangledName = operatorName + "." + name.str();
  raw_string_ostream mangledNameStr(mangledName);
  funcTy->print(mangledNameStr);
  mangledNameStr.flush();

  Function *F =
      cast<Function>(M.getOrInsertFunction(mangledName, funcTy, attribs));

  F->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

  return F;
}

Value *callHLFunction(Module &Module, HLOpcodeGroup OpcodeGroup,
                      unsigned Opcode, Type *RetTy, ArrayRef<Value *> Args,
                      IRBuilder<> &Builder) {
  AttributeSet attribs;
  return callHLFunction(Module, OpcodeGroup, Opcode, RetTy, Args, attribs,
                        Builder);
}

Value *callHLFunction(Module &Module, HLOpcodeGroup OpcodeGroup,
                      unsigned Opcode, Type *RetTy, ArrayRef<Value *> Args,
                      const AttributeSet &attribs, IRBuilder<> &Builder) {
  SmallVector<Type *, 4> ArgTys;
  ArgTys.reserve(Args.size());
  for (Value *Arg : Args)
    ArgTys.emplace_back(Arg->getType());

  FunctionType *FuncTy = FunctionType::get(RetTy, ArgTys, /* isVarArg */ false);
  Function *Func =
      GetOrCreateHLFunction(Module, FuncTy, OpcodeGroup, Opcode, attribs);

  return Builder.CreateCall(Func, Args);
}

} // namespace hlsl
