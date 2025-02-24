///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilPIXDXRInvocationsLog.cpp                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilFunctionProps.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilUtil.h"

#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Transforms/Utils/Local.h"

#include "PixPassHelpers.h"

using namespace llvm;
using namespace hlsl;
using namespace PIXPassHelpers;

class DxilPIXDXRInvocationsLog : public ModulePass {
  uint64_t m_MaxNumEntriesInLog = 1;

public:
  static char ID;
  DxilPIXDXRInvocationsLog() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "DXIL Logs all non-RayGen DXR 1.0 invocations into a UAV";
  }

  void applyOptions(PassOptions O) override;
  bool runOnModule(Module &M) override;
};

static DXIL::ShaderKind GetShaderKind(DxilModule const &DM,
                                      llvm::Function const *entryFunction) {
  DXIL::ShaderKind ShaderKind = DXIL::ShaderKind::Invalid;
  if (!DM.HasDxilFunctionProps(entryFunction)) {
    auto ShaderModel = DM.GetShaderModel();
    ShaderKind = ShaderModel->GetKind();
  } else {
    auto const &Props = DM.GetDxilFunctionProps(entryFunction);
    ShaderKind = Props.shaderKind;
  }

  return ShaderKind;
}

void DxilPIXDXRInvocationsLog::applyOptions(PassOptions O) {
  GetPassOptionUInt64(
      O, "maxNumEntriesInLog", &m_MaxNumEntriesInLog,
      1); // Use a silly default value. PIX should set a better value here.
}

bool DxilPIXDXRInvocationsLog::runOnModule(Module &M) {

  DxilModule &DM = M.GetOrCreateDxilModule();
  LLVMContext &Ctx = M.getContext();
  OP *HlslOP = DM.GetOP();

  bool Modified = false;

  for (auto entryFunction : DM.GetExportedFunctions()) {

    DXIL::ShaderKind ShaderKind = GetShaderKind(DM, entryFunction);

    switch (ShaderKind) {
    case DXIL::ShaderKind::Intersection:
    case DXIL::ShaderKind::AnyHit:
    case DXIL::ShaderKind::ClosestHit:
    case DXIL::ShaderKind::Miss:
      break;

    default:
      continue;
    }

    Modified = true;

    IRBuilder<> Builder(dxilutil::FirstNonAllocaInsertionPt(entryFunction));

    // Add the UAVs that we're going to write to
    CallInst *HandleForCountUAV = PIXPassHelpers::CreateUAVOnceForModule(
        DM, Builder, /* registerID */ 0, "PIX_CountUAV_Handle");
    CallInst *HandleForUAV = PIXPassHelpers::CreateUAVOnceForModule(
        DM, Builder, /* registerID */ 1, "PIX_UAV_Handle");

    DM.ReEmitDxilResources();

    auto DispatchRaysIndexOpFunc = HlslOP->GetOpFunc(
        DXIL::OpCode::DispatchRaysIndex, Type::getInt32Ty(Ctx));
    auto WorldRayOriginOpFunc =
        HlslOP->GetOpFunc(DXIL::OpCode::WorldRayOrigin, Type::getFloatTy(Ctx));
    auto WorldRayDirectionOpFunc = HlslOP->GetOpFunc(
        DXIL::OpCode::WorldRayDirection, Type::getFloatTy(Ctx));
    auto CurrentRayTFunc =
        HlslOP->GetOpFunc(DXIL::OpCode::RayTCurrent, Type::getFloatTy(Ctx));
    auto MinRayTFunc =
        HlslOP->GetOpFunc(DXIL::OpCode::RayTMin, Type::getFloatTy(Ctx));
    auto RayFlagsFunc =
        HlslOP->GetOpFunc(DXIL::OpCode::RayFlags, Type::getInt32Ty(Ctx));

    auto *DispatchRaysIndexOpcode =
        HlslOP->GetU32Const((unsigned)DXIL::OpCode::DispatchRaysIndex);
    auto *WorldRayOriginOpcode =
        HlslOP->GetU32Const((unsigned)DXIL::OpCode::WorldRayOrigin);
    auto *WorldRayDirectionOpcode =
        HlslOP->GetU32Const((unsigned)DXIL::OpCode::WorldRayDirection);
    auto *CurrentRayTOpcode =
        HlslOP->GetU32Const((unsigned)DXIL::OpCode::RayTCurrent);
    auto *MinRayTOpcode = HlslOP->GetU32Const((unsigned)DXIL::OpCode::RayTMin);
    auto *RayFlagsOpcode =
        HlslOP->GetU32Const((unsigned)DXIL::OpCode::RayFlags);

    auto DispatchRaysX = Builder.CreateCall(
        DispatchRaysIndexOpFunc,
        {DispatchRaysIndexOpcode, HlslOP->GetI8Const(0)}, "DispatchRaysX");
    auto DispatchRaysY = Builder.CreateCall(
        DispatchRaysIndexOpFunc,
        {DispatchRaysIndexOpcode, HlslOP->GetI8Const(1)}, "DispatchRaysY");
    auto DispatchRaysZ = Builder.CreateCall(
        DispatchRaysIndexOpFunc,
        {DispatchRaysIndexOpcode, HlslOP->GetI8Const(2)}, "DispatchRaysZ");

    auto WorldRayOriginX = Builder.CreateCall(
        WorldRayOriginOpFunc, {WorldRayOriginOpcode, HlslOP->GetI8Const(0)},
        "WorldRayOriginX");
    auto WorldRayOriginY = Builder.CreateCall(
        WorldRayOriginOpFunc, {WorldRayOriginOpcode, HlslOP->GetI8Const(1)},
        "WorldRayOriginY");
    auto WorldRayOriginZ = Builder.CreateCall(
        WorldRayOriginOpFunc, {WorldRayOriginOpcode, HlslOP->GetI8Const(2)},
        "WorldRayOriginZ");

    auto WorldRayDirectionX = Builder.CreateCall(
        WorldRayDirectionOpFunc,
        {WorldRayDirectionOpcode, HlslOP->GetI8Const(0)}, "WorldRayDirectionX");
    auto WorldRayDirectionY = Builder.CreateCall(
        WorldRayDirectionOpFunc,
        {WorldRayDirectionOpcode, HlslOP->GetI8Const(1)}, "WorldRayDirectionY");
    auto WorldRayDirectionZ = Builder.CreateCall(
        WorldRayDirectionOpFunc,
        {WorldRayDirectionOpcode, HlslOP->GetI8Const(2)}, "WorldRayDirectionZ");

    auto CurrentRayT =
        Builder.CreateCall(CurrentRayTFunc, {CurrentRayTOpcode}, "CurrentRayT");
    auto MinRayT = Builder.CreateCall(MinRayTFunc, {MinRayTOpcode}, "MinRayT");
    auto RayFlags =
        Builder.CreateCall(RayFlagsFunc, {RayFlagsOpcode}, "RayFlags");

    Function *AtomicOpFunc =
        HlslOP->GetOpFunc(OP::OpCode::AtomicBinOp, Type::getInt32Ty(Ctx));
    Constant *AtomicBinOpcode =
        HlslOP->GetU32Const((unsigned)OP::OpCode::AtomicBinOp);
    Constant *AtomicAdd =
        HlslOP->GetU32Const((unsigned)DXIL::AtomicBinOpCode::Add);

    Function *UMinOpFunc =
        HlslOP->GetOpFunc(OP::OpCode::UMin, Type::getInt32Ty(Ctx));
    Constant *UMinOpCode = HlslOP->GetU32Const((unsigned)OP::OpCode::UMin);

    Function *StoreFuncFloat =
        HlslOP->GetOpFunc(OP::OpCode::BufferStore, Type::getFloatTy(Ctx));
    Function *StoreFuncInt =
        HlslOP->GetOpFunc(OP::OpCode::BufferStore, Type::getInt32Ty(Ctx));
    Constant *StoreOpcode =
        HlslOP->GetU32Const((unsigned)OP::OpCode::BufferStore);

    Constant *WriteMask_XYZW = HlslOP->GetI8Const(15);
    Constant *WriteMask_X = HlslOP->GetI8Const(1);
    Constant *ShaderKindAsConstant = HlslOP->GetU32Const((uint32_t)ShaderKind);
    Constant *MaxEntryIndexAsConstant =
        HlslOP->GetU32Const((uint32_t)m_MaxNumEntriesInLog - 1u);
    Constant *Zero32Arg = HlslOP->GetU32Const(0);
    Constant *One32Arg = HlslOP->GetU32Const(1);
    UndefValue *UndefArg = UndefValue::get(Type::getInt32Ty(Ctx));

    // Firstly we read this invocation's index within the invocations log
    // buffer, and atomically increment it for the next invocation
    auto *EntryIndex = Builder.CreateCall(
        AtomicOpFunc,
        {
            AtomicBinOpcode,   // i32, ; opcode
            HandleForCountUAV, // %dx.types.Handle, ; resource handle
            AtomicAdd,         // i32, ; binary operation code
            Zero32Arg,         // i32, ; coordinate c0: byte offset
            UndefArg,          // i32, ; coordinate c1 (unused)
            UndefArg,          // i32, ; coordinate c2 (unused)
            One32Arg           // i32); increment value
        },
        "EntryIndexResult");

    // Clamp the index so that we don't write off the end of the UAV. If we
    // clamp, then it's up to PIX to replay the work again with a larger log
    // buffer.
    auto *EntryIndexClamped = Builder.CreateCall(
        UMinOpFunc, {UMinOpCode, EntryIndex, MaxEntryIndexAsConstant});

    const auto numBytesPerEntry =
        4 + (3 * 4) + (3 * 4) + (3 * 4) + 4 + 4 +
        4; // See number of bytes we store per shader invocation below

    auto EntryOffset =
        Builder.CreateMul(EntryIndexClamped,
                          HlslOP->GetU32Const(numBytesPerEntry), "EntryOffset");
    auto EntryOffsetPlus16 = Builder.CreateAdd(
        EntryOffset, HlslOP->GetU32Const(16), "EntryOffsetPlus16");
    auto EntryOffsetPlus32 = Builder.CreateAdd(
        EntryOffset, HlslOP->GetU32Const(32), "EntryOffsetPlus32");
    auto EntryOffsetPlus48 = Builder.CreateAdd(
        EntryOffset, HlslOP->GetU32Const(48), "EntryOffsetPlus48");

    // Then we start storing the invocation's info into the main UAV buffer
    (void)Builder.CreateCall(
        StoreFuncInt,
        {
            StoreOpcode,          // i32, ; opcode
            HandleForUAV,         // %dx.types.Handle, ; resource handle
            EntryOffset,          // i32, ; coordinate c0: byte offset
            UndefArg,             // i32, ; coordinate c1 (unused)
            ShaderKindAsConstant, // i32, ; value v0
            DispatchRaysX,        // i32, ; value v1
            DispatchRaysY,        // i32, ; value v2
            DispatchRaysZ,        // i32, ; value v3
            WriteMask_XYZW        // i8 ;
        });

    (void)Builder.CreateCall(
        StoreFuncFloat,
        {
            StoreOpcode,        // i32, ; opcode
            HandleForUAV,       // %dx.types.Handle, ; resource handle
            EntryOffsetPlus16,  // i32, ; coordinate c0: byte offset
            UndefArg,           // i32, ; coordinate c1 (unused)
            WorldRayOriginX,    // f32, ; value v0
            WorldRayOriginY,    // f32, ; value v1
            WorldRayOriginZ,    // f32, ; value v2
            WorldRayDirectionX, // f32, ; value v3
            WriteMask_XYZW      // i8 ;
        });

    (void)Builder.CreateCall(
        StoreFuncFloat,
        {
            StoreOpcode,        // i32, ; opcode
            HandleForUAV,       // %dx.types.Handle, ; resource handle
            EntryOffsetPlus32,  // i32, ; coordinate c0: byte offset
            UndefArg,           // i32, ; coordinate c1 (unused)
            WorldRayDirectionY, // f32, ; value v0
            WorldRayDirectionZ, // f32, ; value v1
            MinRayT,            // f32, ; value v2
            CurrentRayT,        // f32, ; value v3
            WriteMask_XYZW      // i8 ;
        });

    (void)Builder.CreateCall(
        StoreFuncInt,
        {
            StoreOpcode,       // i32, ; opcode
            HandleForUAV,      // %dx.types.Handle, ; resource handle
            EntryOffsetPlus48, // i32, ; coordinate c0: byte offset
            UndefArg,          // i32, ; coordinate c1 (unused)
            RayFlags,          // i32, ; value v0
            UndefArg,          // i32, ; value v1
            UndefArg,          // i32, ; value v2
            UndefArg,          // i32, ; value v3
            WriteMask_X        // i8 ;
        });
  }

  return Modified;
}

char DxilPIXDXRInvocationsLog::ID = 0;

ModulePass *llvm::createDxilPIXDXRInvocationsLogPass() {
  return new DxilPIXDXRInvocationsLog();
}

INITIALIZE_PASS(DxilPIXDXRInvocationsLog, "hlsl-dxil-pix-dxr-invocations-log",
                "HLSL DXIL Logs all non-RayGen DXR 1.0 invocations into a UAV",
                false, false)
