//===- subzero/src/IceASanInstrumentation.cpp - ASan ------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the AddressSanitizer instrumentation class.
///
//===----------------------------------------------------------------------===//

#include "IceASanInstrumentation.h"

#include "IceBuildDefs.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceGlobalInits.h"
#include "IceInst.h"
#include "IceTargetLowering.h"
#include "IceTypes.h"

#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Ice {

namespace {

constexpr SizeT BytesPerWord = sizeof(uint32_t);
constexpr SizeT RzSize = 32;
constexpr SizeT ShadowScaleLog2 = 3;
constexpr SizeT ShadowScale = 1 << ShadowScaleLog2;
constexpr SizeT ShadowLength32 = 1 << (32 - ShadowScaleLog2);
constexpr int32_t StackPoisonVal = -1;
constexpr const char *ASanPrefix = "__asan";
constexpr const char *RzPrefix = "__$rz";
constexpr const char *RzArrayName = "__$rz_array";
constexpr const char *RzSizesName = "__$rz_sizes";
const llvm::NaClBitcodeRecord::RecordVector RzContents =
    llvm::NaClBitcodeRecord::RecordVector(RzSize, 'R');

// In order to instrument the code correctly, the .pexe must not have had its
// symbols stripped.
using StringMap = std::unordered_map<std::string, std::string>;
using StringSet = std::unordered_set<std::string>;
// TODO(tlively): Handle all allocation functions
const StringMap FuncSubstitutions = {{"malloc", "__asan_malloc"},
                                     {"free", "__asan_free"},
                                     {"calloc", "__asan_calloc"},
                                     {"__asan_dummy_calloc", "__asan_calloc"},
                                     {"realloc", "__asan_realloc"}};
const StringSet FuncIgnoreList = {"_Balloc"};

llvm::NaClBitcodeRecord::RecordVector sizeToByteVec(SizeT Size) {
  llvm::NaClBitcodeRecord::RecordVector SizeContents;
  for (unsigned i = 0; i < sizeof(Size); ++i) {
    SizeContents.emplace_back(Size % (1 << CHAR_BIT));
    Size >>= CHAR_BIT;
  }
  return SizeContents;
}

} // end of anonymous namespace

ICE_TLS_DEFINE_FIELD(VarSizeMap *, ASanInstrumentation, LocalVars);
ICE_TLS_DEFINE_FIELD(std::vector<InstStore *> *, ASanInstrumentation,
                     LocalDtors);
ICE_TLS_DEFINE_FIELD(CfgNode *, ASanInstrumentation, CurNode);
ICE_TLS_DEFINE_FIELD(VarSizeMap *, ASanInstrumentation, CheckedVars);

bool ASanInstrumentation::isInstrumentable(Cfg *Func) {
  std::string FuncName = Func->getFunctionName().toStringOrEmpty();
  return FuncName == "" || (FuncIgnoreList.count(FuncName) == 0 &&
                            FuncName.find(ASanPrefix) != 0);
}

// Create redzones around all global variables, ensuring that the initializer
// types of the redzones and their associated globals match so that they are
// laid out together in memory.
void ASanInstrumentation::instrumentGlobals(VariableDeclarationList &Globals) {
  std::unique_lock<std::mutex> _(GlobalsMutex);
  if (DidProcessGlobals)
    return;
  VariableDeclarationList NewGlobals;
  // Global holding pointers to all redzones
  auto *RzArray = VariableDeclaration::create(&NewGlobals);
  // Global holding sizes of all redzones
  auto *RzSizes = VariableDeclaration::create(&NewGlobals);

  RzArray->setName(Ctx, RzArrayName);
  RzSizes->setName(Ctx, RzSizesName);
  RzArray->setIsConstant(true);
  RzSizes->setIsConstant(true);
  NewGlobals.push_back(RzArray);
  NewGlobals.push_back(RzSizes);

  using PrototypeMap = std::unordered_map<std::string, FunctionDeclaration *>;
  PrototypeMap ProtoSubstitutions;
  for (VariableDeclaration *Global : Globals) {
    assert(Global->getAlignment() <= RzSize);
    VariableDeclaration *RzLeft = VariableDeclaration::create(&NewGlobals);
    VariableDeclaration *NewGlobal = Global;
    VariableDeclaration *RzRight = VariableDeclaration::create(&NewGlobals);
    RzLeft->setName(Ctx, nextRzName());
    RzRight->setName(Ctx, nextRzName());
    SizeT Alignment = std::max(RzSize, Global->getAlignment());
    SizeT RzLeftSize = Alignment;
    SizeT RzRightSize =
        RzSize + Utils::OffsetToAlignment(Global->getNumBytes(), Alignment);
    if (!Global->hasNonzeroInitializer()) {
      RzLeft->addInitializer(VariableDeclaration::ZeroInitializer::create(
          &NewGlobals, RzLeftSize));
      RzRight->addInitializer(VariableDeclaration::ZeroInitializer::create(
          &NewGlobals, RzRightSize));
    } else {
      RzLeft->addInitializer(VariableDeclaration::DataInitializer::create(
          &NewGlobals, llvm::NaClBitcodeRecord::RecordVector(RzLeftSize, 'R')));
      RzRight->addInitializer(VariableDeclaration::DataInitializer::create(
          &NewGlobals,
          llvm::NaClBitcodeRecord::RecordVector(RzRightSize, 'R')));

      // replace any pointers to allocator functions
      NewGlobal = VariableDeclaration::create(&NewGlobals);
      NewGlobal->setName(Global->getName());
      std::vector<VariableDeclaration::Initializer *> GlobalInits =
          Global->getInitializers();
      for (VariableDeclaration::Initializer *Init : GlobalInits) {
        auto *RelocInit =
            llvm::dyn_cast<VariableDeclaration::RelocInitializer>(Init);
        if (RelocInit == nullptr) {
          NewGlobal->addInitializer(Init);
          continue;
        }
        const GlobalDeclaration *TargetDecl = RelocInit->getDeclaration();
        const auto *TargetFunc =
            llvm::dyn_cast<FunctionDeclaration>(TargetDecl);
        if (TargetFunc == nullptr) {
          NewGlobal->addInitializer(Init);
          continue;
        }
        std::string TargetName = TargetDecl->getName().toStringOrEmpty();
        StringMap::const_iterator Subst = FuncSubstitutions.find(TargetName);
        if (Subst == FuncSubstitutions.end()) {
          NewGlobal->addInitializer(Init);
          continue;
        }
        std::string SubstName = Subst->second;
        PrototypeMap::iterator SubstProtoEntry =
            ProtoSubstitutions.find(SubstName);
        FunctionDeclaration *SubstProto;
        if (SubstProtoEntry != ProtoSubstitutions.end())
          SubstProto = SubstProtoEntry->second;
        else {
          constexpr bool IsProto = true;
          SubstProto = FunctionDeclaration::create(
              Ctx, TargetFunc->getSignature(), TargetFunc->getCallingConv(),
              llvm::GlobalValue::ExternalLinkage, IsProto);
          SubstProto->setName(Ctx, SubstName);
          ProtoSubstitutions.insert({SubstName, SubstProto});
        }

        NewGlobal->addInitializer(VariableDeclaration::RelocInitializer::create(
            &NewGlobals, SubstProto, RelocOffsetArray(0)));
      }
    }

    RzLeft->setIsConstant(Global->getIsConstant());
    NewGlobal->setIsConstant(Global->getIsConstant());
    RzRight->setIsConstant(Global->getIsConstant());
    RzLeft->setAlignment(Alignment);
    NewGlobal->setAlignment(Alignment);
    RzRight->setAlignment(1);
    RzArray->addInitializer(VariableDeclaration::RelocInitializer::create(
        &NewGlobals, RzLeft, RelocOffsetArray(0)));
    RzArray->addInitializer(VariableDeclaration::RelocInitializer::create(
        &NewGlobals, RzRight, RelocOffsetArray(0)));
    RzSizes->addInitializer(VariableDeclaration::DataInitializer::create(
        &NewGlobals, sizeToByteVec(RzLeftSize)));
    RzSizes->addInitializer(VariableDeclaration::DataInitializer::create(
        &NewGlobals, sizeToByteVec(RzRightSize)));

    NewGlobals.push_back(RzLeft);
    NewGlobals.push_back(NewGlobal);
    NewGlobals.push_back(RzRight);
    RzGlobalsNum += 2;

    GlobalSizes.insert({NewGlobal->getName(), NewGlobal->getNumBytes()});
  }

  // Replace old list of globals, without messing up arena allocators
  Globals.clear();
  Globals.merge(&NewGlobals);
  DidProcessGlobals = true;

  // Log the new set of globals
  if (BuildDefs::dump() && (getFlags().getVerbose() & IceV_GlobalInit)) {
    OstreamLocker _(Ctx);
    Ctx->getStrDump() << "========= Instrumented Globals =========\n";
    for (VariableDeclaration *Global : Globals) {
      Global->dump(Ctx->getStrDump());
    }
  }
}

std::string ASanInstrumentation::nextRzName() {
  std::stringstream Name;
  Name << RzPrefix << RzNum++;
  return Name.str();
}

// Check for an alloca signaling the presence of local variables and add a
// redzone if it is found
void ASanInstrumentation::instrumentFuncStart(LoweringContext &Context) {
  if (ICE_TLS_GET_FIELD(LocalDtors) == nullptr) {
    ICE_TLS_SET_FIELD(LocalDtors, new std::vector<InstStore *>());
    ICE_TLS_SET_FIELD(LocalVars, new VarSizeMap());
  }
  Cfg *Func = Context.getNode()->getCfg();
  using Entry = std::pair<SizeT, int32_t>;
  std::vector<InstAlloca *> NewAllocas;
  std::vector<Entry> PoisonVals;
  Variable *FirstShadowLocVar;
  InstArithmetic *ShadowIndexCalc;
  InstArithmetic *ShadowLocCalc;
  InstAlloca *Cur;
  ConstantInteger32 *VarSizeOp;
  while (!Context.atEnd()) {
    Cur = llvm::dyn_cast<InstAlloca>(iteratorToInst(Context.getCur()));
    VarSizeOp = (Cur == nullptr)
                    ? nullptr
                    : llvm::dyn_cast<ConstantInteger32>(Cur->getSizeInBytes());
    if (Cur == nullptr || VarSizeOp == nullptr) {
      Context.advanceCur();
      Context.advanceNext();
      continue;
    }

    Cur->setDeleted();

    if (PoisonVals.empty()) {
      // insert leftmost redzone
      auto *LastRzVar = Func->makeVariable(IceType_i32);
      LastRzVar->setName(Func, nextRzName());
      auto *ByteCount = ConstantInteger32::create(Ctx, IceType_i32, RzSize);
      constexpr SizeT Alignment = 8;
      NewAllocas.emplace_back(
          InstAlloca::create(Func, LastRzVar, ByteCount, Alignment));
      PoisonVals.emplace_back(Entry{RzSize >> ShadowScaleLog2, StackPoisonVal});

      // Calculate starting address for poisoning
      FirstShadowLocVar = Func->makeVariable(IceType_i32);
      FirstShadowLocVar->setName(Func, "firstShadowLoc");
      auto *ShadowIndexVar = Func->makeVariable(IceType_i32);
      ShadowIndexVar->setName(Func, "shadowIndex");

      auto *ShadowScaleLog2Const =
          ConstantInteger32::create(Ctx, IceType_i32, ShadowScaleLog2);
      auto *ShadowMemLocConst =
          ConstantInteger32::create(Ctx, IceType_i32, ShadowLength32);

      ShadowIndexCalc =
          InstArithmetic::create(Func, InstArithmetic::Lshr, ShadowIndexVar,
                                 LastRzVar, ShadowScaleLog2Const);
      ShadowLocCalc =
          InstArithmetic::create(Func, InstArithmetic::Add, FirstShadowLocVar,
                                 ShadowIndexVar, ShadowMemLocConst);
    }

    // create the new alloca that includes a redzone
    SizeT VarSize = VarSizeOp->getValue();
    Variable *Dest = Cur->getDest();
    ICE_TLS_GET_FIELD(LocalVars)->insert({Dest, VarSize});
    SizeT RzPadding = RzSize + Utils::OffsetToAlignment(VarSize, RzSize);
    auto *ByteCount =
        ConstantInteger32::create(Ctx, IceType_i32, VarSize + RzPadding);
    constexpr SizeT Alignment = 8;
    NewAllocas.emplace_back(
        InstAlloca::create(Func, Dest, ByteCount, Alignment));

    const SizeT Zeros = VarSize >> ShadowScaleLog2;
    const SizeT Offset = VarSize % ShadowScale;
    const SizeT PoisonBytes =
        ((VarSize + RzPadding) >> ShadowScaleLog2) - Zeros - 1;
    if (Zeros > 0)
      PoisonVals.emplace_back(Entry{Zeros, 0});
    PoisonVals.emplace_back(Entry{1, (Offset == 0) ? StackPoisonVal : Offset});
    PoisonVals.emplace_back(Entry{PoisonBytes, StackPoisonVal});
    Context.advanceCur();
    Context.advanceNext();
  }

  Context.rewind();
  if (PoisonVals.empty()) {
    Context.advanceNext();
    return;
  }
  for (InstAlloca *RzAlloca : NewAllocas) {
    Context.insert(RzAlloca);
  }
  Context.insert(ShadowIndexCalc);
  Context.insert(ShadowLocCalc);

  // Poison redzones
  std::vector<Entry>::iterator Iter = PoisonVals.begin();
  for (SizeT Offset = 0; Iter != PoisonVals.end(); Offset += BytesPerWord) {
    int32_t CurVals[BytesPerWord] = {0};
    for (uint32_t i = 0; i < BytesPerWord; ++i) {
      if (Iter == PoisonVals.end())
        break;
      Entry Val = *Iter;
      CurVals[i] = Val.second;
      --Val.first;
      if (Val.first > 0)
        *Iter = Val;
      else
        ++Iter;
    }
    int32_t Poison = ((CurVals[3] & 0xff) << 24) | ((CurVals[2] & 0xff) << 16) |
                     ((CurVals[1] & 0xff) << 8) | (CurVals[0] & 0xff);
    if (Poison == 0)
      continue;
    auto *PoisonConst = ConstantInteger32::create(Ctx, IceType_i32, Poison);
    auto *ZeroConst = ConstantInteger32::create(Ctx, IceType_i32, 0);
    auto *OffsetConst = ConstantInteger32::create(Ctx, IceType_i32, Offset);
    auto *PoisonAddrVar = Func->makeVariable(IceType_i32);
    Context.insert(InstArithmetic::create(Func, InstArithmetic::Add,
                                          PoisonAddrVar, FirstShadowLocVar,
                                          OffsetConst));
    Context.insert(InstStore::create(Func, PoisonConst, PoisonAddrVar));
    ICE_TLS_GET_FIELD(LocalDtors)
        ->emplace_back(InstStore::create(Func, ZeroConst, PoisonAddrVar));
  }
  Context.advanceNext();
}

void ASanInstrumentation::instrumentCall(LoweringContext &Context,
                                         InstCall *Instr) {
  auto *CallTarget =
      llvm::dyn_cast<ConstantRelocatable>(Instr->getCallTarget());
  if (CallTarget == nullptr)
    return;

  std::string TargetName = CallTarget->getName().toStringOrEmpty();
  auto Subst = FuncSubstitutions.find(TargetName);
  if (Subst == FuncSubstitutions.end())
    return;

  std::string SubName = Subst->second;
  Constant *NewFunc = Ctx->getConstantExternSym(Ctx->getGlobalString(SubName));
  auto *NewCall =
      InstCall::create(Context.getNode()->getCfg(), Instr->getNumArgs(),
                       Instr->getDest(), NewFunc, Instr->isTailcall());
  for (SizeT I = 0, Args = Instr->getNumArgs(); I < Args; ++I)
    NewCall->addArg(Instr->getArg(I));
  Context.insert(NewCall);
  Instr->setDeleted();
}

void ASanInstrumentation::instrumentLoad(LoweringContext &Context,
                                         InstLoad *Instr) {
  Operand *Src = Instr->getLoadAddress();
  if (auto *Reloc = llvm::dyn_cast<ConstantRelocatable>(Src)) {
    auto *NewLoad = InstLoad::create(Context.getNode()->getCfg(),
                                     Instr->getDest(), instrumentReloc(Reloc));
    Instr->setDeleted();
    Context.insert(NewLoad);
    Instr = NewLoad;
  }
  Constant *Func =
      Ctx->getConstantExternSym(Ctx->getGlobalString("__asan_check_load"));
  instrumentAccess(Context, Instr->getLoadAddress(),
                   typeWidthInBytes(Instr->getDest()->getType()), Func);
}

void ASanInstrumentation::instrumentStore(LoweringContext &Context,
                                          InstStore *Instr) {
  Operand *Data = Instr->getData();
  if (auto *Reloc = llvm::dyn_cast<ConstantRelocatable>(Data)) {
    auto *NewStore =
        InstStore::create(Context.getNode()->getCfg(), instrumentReloc(Reloc),
                          Instr->getStoreAddress());
    Instr->setDeleted();
    Context.insert(NewStore);
    Instr = NewStore;
  }
  Constant *Func =
      Ctx->getConstantExternSym(Ctx->getGlobalString("__asan_check_store"));
  instrumentAccess(Context, Instr->getStoreAddress(),
                   typeWidthInBytes(Instr->getData()->getType()), Func);
}

ConstantRelocatable *
ASanInstrumentation::instrumentReloc(ConstantRelocatable *Reloc) {
  std::string DataName = Reloc->getName().toString();
  StringMap::const_iterator DataSub = FuncSubstitutions.find(DataName);
  if (DataSub != FuncSubstitutions.end()) {
    return ConstantRelocatable::create(
        Ctx, Reloc->getType(),
        RelocatableTuple(Reloc->getOffset(), RelocOffsetArray(0),
                         Ctx->getGlobalString(DataSub->second),
                         Reloc->getEmitString()));
  }
  return Reloc;
}

void ASanInstrumentation::instrumentAccess(LoweringContext &Context,
                                           Operand *Op, SizeT Size,
                                           Constant *CheckFunc) {
  // Skip redundant checks within basic blocks
  VarSizeMap *Checked = ICE_TLS_GET_FIELD(CheckedVars);
  if (ICE_TLS_GET_FIELD(CurNode) != Context.getNode()) {
    ICE_TLS_SET_FIELD(CurNode, Context.getNode());
    if (Checked == NULL) {
      Checked = new VarSizeMap();
      ICE_TLS_SET_FIELD(CheckedVars, Checked);
    }
    Checked->clear();
  }
  VarSizeMap::iterator PrevCheck = Checked->find(Op);
  if (PrevCheck != Checked->end() && PrevCheck->second >= Size)
    return;
  else
    Checked->insert({Op, Size});

  // check for known good local access
  VarSizeMap::iterator LocalSize = ICE_TLS_GET_FIELD(LocalVars)->find(Op);
  if (LocalSize != ICE_TLS_GET_FIELD(LocalVars)->end() &&
      LocalSize->second >= Size)
    return;
  if (isOkGlobalAccess(Op, Size))
    return;
  constexpr SizeT NumArgs = 2;
  constexpr Variable *Void = nullptr;
  constexpr bool NoTailCall = false;
  auto *Call = InstCall::create(Context.getNode()->getCfg(), NumArgs, Void,
                                CheckFunc, NoTailCall);
  Call->addArg(Op);
  Call->addArg(ConstantInteger32::create(Ctx, IceType_i32, Size));
  // play games to insert the call before the access instruction
  InstList::iterator Next = Context.getNext();
  Context.setInsertPoint(Context.getCur());
  Context.insert(Call);
  Context.setNext(Next);
}

// TODO(tlively): Trace back load and store addresses to find their real offsets
bool ASanInstrumentation::isOkGlobalAccess(Operand *Op, SizeT Size) {
  auto *Reloc = llvm::dyn_cast<ConstantRelocatable>(Op);
  if (Reloc == nullptr)
    return false;
  RelocOffsetT Offset = Reloc->getOffset();
  GlobalSizeMap::iterator GlobalSize = GlobalSizes.find(Reloc->getName());
  return GlobalSize != GlobalSizes.end() && GlobalSize->second - Offset >= Size;
}

void ASanInstrumentation::instrumentRet(LoweringContext &Context, InstRet *) {
  Cfg *Func = Context.getNode()->getCfg();
  Context.setInsertPoint(Context.getCur());
  for (InstStore *RzUnpoison : *ICE_TLS_GET_FIELD(LocalDtors)) {
    Context.insert(InstStore::create(Func, RzUnpoison->getData(),
                                     RzUnpoison->getStoreAddress()));
  }
  Context.advanceCur();
  Context.advanceNext();
}

void ASanInstrumentation::instrumentStart(Cfg *Func) {
  Constant *ShadowMemInit =
      Ctx->getConstantExternSym(Ctx->getGlobalString("__asan_init"));
  constexpr SizeT NumArgs = 3;
  constexpr Variable *Void = nullptr;
  constexpr bool NoTailCall = false;
  auto *Call = InstCall::create(Func, NumArgs, Void, ShadowMemInit, NoTailCall);
  Func->getEntryNode()->getInsts().push_front(Call);

  instrumentGlobals(*getGlobals());

  Call->addArg(ConstantInteger32::create(Ctx, IceType_i32, RzGlobalsNum));
  Call->addArg(Ctx->getConstantSym(0, Ctx->getGlobalString(RzArrayName)));
  Call->addArg(Ctx->getConstantSym(0, Ctx->getGlobalString(RzSizesName)));
}

// TODO(tlively): make this more efficient with swap idiom
void ASanInstrumentation::finishFunc(Cfg *) {
  ICE_TLS_GET_FIELD(LocalVars)->clear();
  ICE_TLS_GET_FIELD(LocalDtors)->clear();
}

} // end of namespace Ice
