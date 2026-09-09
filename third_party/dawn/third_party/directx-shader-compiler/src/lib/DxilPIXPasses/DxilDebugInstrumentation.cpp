///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilDebugInstrumentation.cpp                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Adds instrumentation that enables shader debugging in PIX                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <optional>
#include <vector>

#include "dxc/DXIL/DxilFunctionProps.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"
#include "dxc/DxilPIXPasses/DxilPIXVirtualRegisters.h"
#include "dxc/HLSL/DxilGenerationPass.h"
#include "dxc/Support/Global.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#include "PixPassHelpers.h"

using namespace llvm;
using namespace hlsl;

// Overview of instrumentation:
//
// In summary, instructions are added that cause a "trace" of the execution of
// the shader to be written out to a UAV. This trace is then used by a debugger
// application to provide a postmortem debugging experience that reconstructs
// the execution history of the shader. The caller specifies the power-of-two
// size of the UAV.
//
// The instrumentation is added per basic block, and each block will then write
// a contiguous sequence of values into the UAV.
//
// The trace is only required for particular shader instances of interest, and
// a branchless mechanism is used to write the trace either to an incrementing
// location within the UAV, or to a "dumping ground" area in the top half of the
// UAV if the instance is not of interest.
//
// In addition, each half of the UAV is further subdivided: the first quarter is
// the area in which blocks are permitted to start writing their sequence, and
// that sequence is constrained to be no longer than the size of the second
// quarter. This allows us to limit writes to the appropriate half of the UAV
// via a single AND at the beginning of the basic block. An additional OR
// provides the offset, either 0 for threads-of-interest, or UAVSize/2 for
// not-of-interest.
//
// Threads determine where to start writing their data by incrementing a DWORD
// that lives at the very top of that thread's half of the UAV. This is done
// because several threads may satisfy the selection criteria (e.g. a pixel
// shader may be invoked several times for a given pixel coordinate if the model
// has overlapping triangles).
//
// A picture of the UAV layout:
// <--------------power-of-two-size-of-UAV---------------->
// [1           ][2           ][3           ][4           ]
// <------A----->             ^                           ^
//                            B                           C
//                            <------D------>
//
// A: the size of the AND for interesting writes. Their payloads extend
// beyond this into area 2, but those payloads are limited to be small
// enough (1/4 UAV size -1) that they don't overwrite B.
// B: The interesting thread's counter.
// C: The uninteresting thread's counter.
// D: Size of the AND for uninteresting threads (same value as A)
//
// The following modifications are made by this pass:
//
// First, instructions are added to the top of the entry point function that
// implement the following:
// -  Examine the input variables that define the instance of the shader that is
//    running. This will be SV_Position for pixel shaders, SV_Vertex+SV_Instance
//    for vertex shaders, thread id for compute shaders etc. If these system
//    values need to be added to the shader, then they are also added to the
//    input signature, if appropriate.
// -  Compare the above variables with the instance of interest defined by the
//    invoker of this pass. If equal, create an OR value of zero that will
//    not affect the block's starting write offset. If not equal, the OR will
//    move the writes into the second half of the UAV.
// -  Calculate an "instance identifier". Even with the above instance
//    identification, several invocations may end up matching the selection
//    criteria. More on this below.
//
// As mentioned, a counter/offset is maintained at the top of the thread's
// half of the UAV. The very first value of this counter that
// is encountered by each invocation is used as the "instance identifier"
// mentioned above. That instance identifier is written out with each packet,
// since many threads executing in parallel will emit interleaved packets,
// and the debugger application uses the identifiers to gather packets from each
// separate invocation together.
//
// In addition to the above, this pass creates a text precis of the structure
// being written out for each basic block. This precis is passed back to the
// caller, and can be used to parse the UAV output later. The precis will
// contain notes about void-type instructions, which won't write anything to the
// UAV, allowing the caller to reconstruct those instructions.
// Some care has to be taken about whether to emit UAV writes after the
// corresponding instruction or before. Terminators must emit their UAV data
// before the terminator itself, of course. Phi instructions get special
// treatment also: their instrumentation has to come after (since phis must be
// the first instructions in the block), but also the instrumentation must
// execute in the same order as the precis specifies, or the caller will mix
// up the phi values. We achieve this by saying that phi instrumentation must
// come before the first non-phi instruction in the block.
// Some blocks will have all-void instructions, so that no debugging
// data is emitted at all. These blocks still produce a precis, and still
// need to be noticed during execution, so an empty block header is emitted
// into the UAV.
//
// Error conditions:
// Overflow of the debug output from the interesting threads will start to
// overwrite their own area of the UAV (after the AND limits those writes
// to the lower half of the UAV (thus, by the way, avoiding overwriting
// their counter value)). The caller must check the counter value after
// the debugging run is complete to see if this happened, and if so, increase
// the UAV size and try again.
// Uninteresting threads use an AND value that limits their writes to the
// upper half of the UAV and can be entirely ignored by the caller.
// Since a sufficiently-large block is guaranteed to overflow the UAV,
// the precis-creation can exit early and report this "static" overflow
// condition to the caller.
// In all overflow cases, the caller is expected to try to instrument again,
// with a larger UAV.

// These definitions echo those in the debugger application's
// debugshaderrecord.h file
enum DebugShaderModifierRecordType {
  DebugShaderModifierRecordTypeInvocationStartMarker,
  DebugShaderModifierRecordTypeStep,
  DebugShaderModifierRecordTypeEvent,
  DebugShaderModifierRecordTypeInputRegister,
  DebugShaderModifierRecordTypeReadRegister,
  DebugShaderModifierRecordTypeWrittenRegister,
  DebugShaderModifierRecordTypeRegisterRelativeIndex0,
  DebugShaderModifierRecordTypeRegisterRelativeIndex1,
  DebugShaderModifierRecordTypeRegisterRelativeIndex2,
  // Note that everything above this line is no longer used, but is kept
  // here in order to keep this file more in-sync with the debugger source.
  // (As of this writing, the debugger still supports older versions of this
  // pass which produced finer-grained debug packets.)
  DebugShaderModifierRecordTypeDXILStepBlock = 249,
  DebugShaderModifierRecordTypeDXILStepRet = 250,
  DebugShaderModifierRecordTypeDXILStepVoid = 251,
  DebugShaderModifierRecordTypeDXILStepFloat = 252,
  DebugShaderModifierRecordTypeDXILStepUint32 = 253,
  DebugShaderModifierRecordTypeDXILStepUint64 = 254,
  DebugShaderModifierRecordTypeDXILStepDouble = 255,
};

// These structs echo those in the debugger application's debugshaderrecord.h
// file, but are recapitulated here because the originals use unnamed unions
// which are disallowed by DXCompiler's build.
//
#pragma pack(push, 4)
struct DebugShaderModifierRecordHeader {
  union {
    struct {
      uint32_t SizeDwords : 4;
      uint32_t Flags : 4;
      uint32_t Type : 8;
      uint32_t HeaderPayload : 16;
    } Details;
    uint32_t u32Header;
  } Header;
  uint32_t UID;
};

struct DebugShaderModifierRecordDXILStepBase {
  union {
    struct {
      uint32_t SizeDwords : 4;
      uint32_t Flags : 4;
      uint32_t Type : 8;
      uint32_t Opcode : 16;
    } Details;
    uint32_t u32Header;
  } Header;
  uint32_t UID;
  uint32_t InstructionOffset;
};

struct DebugShaderModifierRecordDXILBlock {
  union {
    struct {
      uint32_t NotUsed0 : 4;
      uint32_t NotUsed1 : 4;
      uint32_t Type : 8;
      uint32_t CountOfInstructions : 16;
    } Details;
    uint32_t u32Header;
  } Header;
  uint32_t UID;
  uint32_t FirstInstructionOrdinal;
};

template <typename ReturnType>
struct DebugShaderModifierRecordDXILStep
    : public DebugShaderModifierRecordDXILStepBase {
  ReturnType ReturnValue;
  union {
    struct {
      uint32_t ValueOrdinalBase : 16;
      uint32_t ValueOrdinalIndex : 16;
    } Details;
    uint32_t u32ValueOrdinal;
  } ValueOrdinal;
};

template <>
struct DebugShaderModifierRecordDXILStep<void>
    : public DebugShaderModifierRecordDXILStepBase {};
#pragma pack(pop)

uint32_t
DebugShaderModifierRecordPayloadSizeDwords(size_t recordTotalSizeBytes) {
  return ((recordTotalSizeBytes - sizeof(DebugShaderModifierRecordHeader)) /
          sizeof(uint32_t));
}

struct InstructionAndType {
  Instruction *Inst;
  std::uint32_t InstructionOrdinal;
  DebugShaderModifierRecordType Type;
  std::uint32_t RegisterNumber;
  std::uint32_t AllocaBase;
  Value *AllocaWriteIndex = nullptr;
  std::optional<uint64_t> ConstantAllocaStoreValue;
};

class DxilDebugInstrumentation : public ModulePass {

private:
  union ParametersAllTogether {
    unsigned Parameters[3];
    struct PixelShaderParameters {
      unsigned X;
      unsigned Y;
    } PixelShader;
    struct VertexShaderParameters {
      unsigned VertexId;
      unsigned InstanceId;
    } VertexShader;
    struct ComputeShaderParameters {
      unsigned ThreadIdX;
      unsigned ThreadIdY;
      unsigned ThreadIdZ;
    } ComputeShader;
    struct GeometryShaderParameters {
      unsigned PrimitiveId;
      unsigned InstanceId;
    } GeometryShader;
    struct HullShaderParameters {
      unsigned PrimitiveId;
      unsigned ControlPointId;
    } HullShader;
    struct DomainShaderParameters {
      unsigned PrimitiveId;
    } DomainShader;
  } m_Parameters = {{0, 0, 0}};

  union SystemValueIndices {
    struct PixelShaderParameters {
      unsigned Position;
    } PixelShader;
    struct VertexShaderParameters {
      unsigned VertexId;
      unsigned InstanceId;
    } VertexShader;
  };
  unsigned m_FirstInstruction = 0;
  unsigned m_LastInstruction = static_cast<unsigned>(-1);

  uint64_t m_UAVSize = 1024 * 1024;
  unsigned m_upstreamSVPositionRow;

  struct PerFunctionValues {
    CallInst *UAVHandle = nullptr;
    Instruction *CounterOffset = nullptr;
    Value *InvocationId = nullptr;
    // Together these two values allow branchless writing to the UAV. An
    // invocation of the shader is either of interest or not (e.g. it writes to
    // the pixel the user selected for debugging or it doesn't). If not of
    // interest, debugging output will still occur, but it will be relegated to
    // the top half of the UAV. Invocations of interest, by contrast,
    // will be written to the UAV at sequentially increasing offsets.
    Value *OffsetMask = nullptr;
    Instruction *OffsetOr = nullptr;
    Value *SelectionCriterion = nullptr;
    Value *CurrentIndex = nullptr;
    std::vector<BasicBlock *> AddedBlocksToIgnoreForInstrumentation;
  };
  std::map<llvm::Function *, PerFunctionValues> m_FunctionToValues;

  struct BuilderContext {
    Module &M;
    DxilModule &DM;
    LLVMContext &Ctx;
    OP *HlslOP;
    IRBuilder<> &Builder;
  };

  uint32_t m_RemainingReservedSpaceInBytes = 0;

public:
  static char ID; // Pass identification, replacement for typeid
  explicit DxilDebugInstrumentation() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "Add PIX debug instrumentation";
  }
  void applyOptions(PassOptions O) override;
  bool runOnModule(Module &M) override;

  bool RunOnFunction(Module &M, DxilModule &DM, hlsl::DxilResource *uav,
                     llvm::Function *function);

private:
  SystemValueIndices addRequiredSystemValues(BuilderContext &BC,
                                             DXIL::ShaderKind shaderKind);
  void addInvocationSelectionProlog(BuilderContext &BC,
                                    SystemValueIndices SVIndices,
                                    DXIL::ShaderKind shaderKind);
  Value *addPixelShaderProlog(BuilderContext &BC, SystemValueIndices SVIndices);
  Value *addGeometryShaderProlog(BuilderContext &BC);
  Value *addDispatchedShaderProlog(BuilderContext &BC);
  Value *addRaygenShaderProlog(BuilderContext &BC);
  Value *addVertexShaderProlog(BuilderContext &BC,
                               SystemValueIndices SVIndices);
  Value *addHullhaderProlog(BuilderContext &BC);
  Value *addComparePrimitiveIdProlog(BuilderContext &BC, unsigned SVIndices);
  uint32_t addDebugEntryValue(BuilderContext &BC, Value *TheValue);
  void addInvocationStartMarker(BuilderContext &BC);
  void determineLimitANDAndInitializeCounter(BuilderContext &BC);
  void reserveDebugEntrySpace(BuilderContext &BC, uint32_t SpaceInDwords);
  std::optional<InstructionAndType> addStoreStepDebugEntry(BuilderContext *BC,
                                                           StoreInst *Inst);
  std::optional<InstructionAndType>
  addStepDebugEntry(BuilderContext *BC, Instruction *Inst,
                    llvm::SmallPtrSetImpl<Value *> const &RayQueryHandles);
  std::optional<DebugShaderModifierRecordType>
  addStepDebugEntryValue(BuilderContext *BC, std::uint32_t InstNum, Value *V,
                         std::uint32_t ValueOrdinal, Value *ValueOrdinalIndex);
  uint32_t UAVDumpingGroundOffset();
  template <typename ReturnType>
  void addStepEntryForType(DebugShaderModifierRecordType RecordType,
                           BuilderContext &BC, std::uint32_t InstNum, Value *V,
                           std::uint32_t ValueOrdinal,
                           Value *ValueOrdinalIndex);
  struct InstructionToInstrument {
    Value *ValueToWriteToDebugMemory;
    DebugShaderModifierRecordType ValueType;
    Instruction *InstructionAfterWhichToAddInstrumentation;
    Instruction *InstructionBeforeWhichToAddInstrumentation;
  };
  struct BlockInstrumentationData {
    uint32_t FirstInstructionOrdinalInBlock;
    std::vector<InstructionToInstrument> Instructions;
  };
  BlockInstrumentationData FindInstrumentableInstructionsInBlock(
      BasicBlock &BB, OP *HlslOP,
      llvm::SmallPtrSetImpl<Value *> const &RayQueryHandles);
  uint32_t
  CountBlockPayloadBytes(std::vector<InstructionToInstrument> const &IsAndTs);
};

void DxilDebugInstrumentation::applyOptions(PassOptions O) {
  GetPassOptionUnsigned(O, "FirstInstruction", &m_FirstInstruction, 0);
  GetPassOptionUnsigned(O, "LastInstruction", &m_LastInstruction,
                        static_cast<unsigned>(-1));
  GetPassOptionUnsigned(O, "parameter0", &m_Parameters.Parameters[0], 0);
  GetPassOptionUnsigned(O, "parameter1", &m_Parameters.Parameters[1], 0);
  GetPassOptionUnsigned(O, "parameter2", &m_Parameters.Parameters[2], 0);
  GetPassOptionUInt64(O, "UAVSize", &m_UAVSize, 1024 * 1024);
  GetPassOptionUnsigned(O, "upstreamSVPositionRow", &m_upstreamSVPositionRow,
                        0);
}

uint32_t DxilDebugInstrumentation::UAVDumpingGroundOffset() {
  return static_cast<uint32_t>(m_UAVSize / 2);
}

unsigned int GetNextEmptyRow(
    std::vector<std::unique_ptr<DxilSignatureElement>> const &Elements) {
  unsigned int Row = 0;
  for (auto const &Element : Elements) {
    Row = std::max<unsigned>(Row, Element->GetStartRow() + Element->GetRows());
  }
  return Row;
}

unsigned FindOrAddVSInSignatureElementForInstanceOrVertexID(
    hlsl::DxilSignature &InputSignature,
    hlsl::DXIL::SemanticKind semanticKind) {
  DXASSERT(InputSignature.GetSigPointKind() == DXIL::SigPointKind::VSIn,
           "Unexpected SigPointKind in input signature");
  DXASSERT(semanticKind == DXIL::SemanticKind::InstanceID ||
               semanticKind == DXIL::SemanticKind::VertexID,
           "This function only expects InstaceID or VertexID");

  auto const &InputElements = InputSignature.GetElements();

  auto ExistingElement =
      std::find_if(InputElements.begin(), InputElements.end(),
                   [&](const std::unique_ptr<DxilSignatureElement> &Element) {
                     return Element->GetSemantic()->GetKind() == semanticKind;
                   });

  if (ExistingElement == InputElements.end()) {
    auto AddedElement =
        llvm::make_unique<DxilSignatureElement>(DXIL::SigPointKind::VSIn);
    unsigned Row = GetNextEmptyRow(InputElements);
    AddedElement->Initialize(
        hlsl::Semantic::Get(semanticKind)->GetName(), hlsl::CompType::getU32(),
        hlsl::DXIL::InterpolationMode::Constant, 1, 1, Row, 0);
    AddedElement->AppendSemanticIndex(0);
    AddedElement->SetKind(semanticKind);
    AddedElement->SetUsageMask(1);
    // AppendElement sets the element's ID by default
    auto index = InputSignature.AppendElement(std::move(AddedElement));
    return InputElements[index]->GetID();
  } else {
    return ExistingElement->get()->GetID();
  }
}

DxilDebugInstrumentation::SystemValueIndices
DxilDebugInstrumentation::addRequiredSystemValues(BuilderContext &BC,
                                                  DXIL::ShaderKind shaderKind) {
  SystemValueIndices SVIndices{};

  switch (shaderKind) {
  case DXIL::ShaderKind::Amplification:
  case DXIL::ShaderKind::Mesh:
  case DXIL::ShaderKind::Compute:
  case DXIL::ShaderKind::RayGeneration:
  case DXIL::ShaderKind::Intersection:
  case DXIL::ShaderKind::AnyHit:
  case DXIL::ShaderKind::ClosestHit:
  case DXIL::ShaderKind::Miss:
  case DXIL::ShaderKind::Node:
    // Dispatch* thread Id is not in the input signature
    break;
  case DXIL::ShaderKind::Vertex: {
    hlsl::DxilSignature &InputSignature = BC.DM.GetInputSignature();
    SVIndices.VertexShader.VertexId =
        FindOrAddVSInSignatureElementForInstanceOrVertexID(
            InputSignature, hlsl::DXIL::SemanticKind::VertexID);
    SVIndices.VertexShader.InstanceId =
        FindOrAddVSInSignatureElementForInstanceOrVertexID(
            InputSignature, hlsl::DXIL::SemanticKind::InstanceID);
  } break;
  case DXIL::ShaderKind::Geometry:
  case DXIL::ShaderKind::Hull:
  case DXIL::ShaderKind::Domain:
    // GS, HS, DS Primitive id, HS control point id, and GS Instance id are not
    // in the input signature
    break;
  case DXIL::ShaderKind::Pixel: {
    SVIndices.PixelShader.Position =
        PIXPassHelpers::FindOrAddSV_Position(BC.DM, m_upstreamSVPositionRow);
  } break;
  default:
    assert(false); // guaranteed by runOnModule
  }

  return SVIndices;
}

Value *DxilDebugInstrumentation::addDispatchedShaderProlog(BuilderContext &BC) {
  Constant *Zero32Arg = BC.HlslOP->GetU32Const(0);
  Constant *One32Arg = BC.HlslOP->GetU32Const(1);
  Constant *Two32Arg = BC.HlslOP->GetU32Const(2);

  auto ThreadIdFunc =
      BC.HlslOP->GetOpFunc(DXIL::OpCode::ThreadId, Type::getInt32Ty(BC.Ctx));
  Constant *Opcode = BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::ThreadId);
  auto ThreadIdX =
      BC.Builder.CreateCall(ThreadIdFunc, {Opcode, Zero32Arg}, "ThreadIdX");
  auto ThreadIdY =
      BC.Builder.CreateCall(ThreadIdFunc, {Opcode, One32Arg}, "ThreadIdY");
  auto ThreadIdZ =
      BC.Builder.CreateCall(ThreadIdFunc, {Opcode, Two32Arg}, "ThreadIdZ");

  // Compare to expected thread ID
  auto CompareToX = BC.Builder.CreateICmpEQ(
      ThreadIdX, BC.HlslOP->GetU32Const(m_Parameters.ComputeShader.ThreadIdX),
      "CompareToThreadIdX");
  auto CompareToY = BC.Builder.CreateICmpEQ(
      ThreadIdY, BC.HlslOP->GetU32Const(m_Parameters.ComputeShader.ThreadIdY),
      "CompareToThreadIdY");
  auto CompareToZ = BC.Builder.CreateICmpEQ(
      ThreadIdZ, BC.HlslOP->GetU32Const(m_Parameters.ComputeShader.ThreadIdZ),
      "CompareToThreadIdZ");

  auto CompareXAndY =
      BC.Builder.CreateAnd(CompareToX, CompareToY, "CompareXAndY");

  auto CompareAll =
      BC.Builder.CreateAnd(CompareXAndY, CompareToZ, "CompareAll");

  return CompareAll;
}

Value *DxilDebugInstrumentation::addRaygenShaderProlog(BuilderContext &BC) {
  auto DispatchRaysIndexOpFunc = BC.HlslOP->GetOpFunc(
      DXIL::OpCode::DispatchRaysIndex, Type::getInt32Ty(BC.Ctx));
  Constant *DispatchRaysIndexOpcode =
      BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::DispatchRaysIndex);
  auto RayX = BC.Builder.CreateCall(
      DispatchRaysIndexOpFunc,
      {DispatchRaysIndexOpcode, BC.HlslOP->GetI8Const(0)}, "RayX");
  auto RayY = BC.Builder.CreateCall(
      DispatchRaysIndexOpFunc,
      {DispatchRaysIndexOpcode, BC.HlslOP->GetI8Const(1)}, "RayY");
  auto RayZ = BC.Builder.CreateCall(
      DispatchRaysIndexOpFunc,
      {DispatchRaysIndexOpcode, BC.HlslOP->GetI8Const(2)}, "RayZ");

  auto CompareToX = BC.Builder.CreateICmpEQ(
      RayX, BC.HlslOP->GetU32Const(m_Parameters.ComputeShader.ThreadIdX),
      "CompareToThreadIdX");

  auto CompareToY = BC.Builder.CreateICmpEQ(
      RayY, BC.HlslOP->GetU32Const(m_Parameters.ComputeShader.ThreadIdY),
      "CompareToThreadIdY");

  auto CompareToZ = BC.Builder.CreateICmpEQ(
      RayZ, BC.HlslOP->GetU32Const(m_Parameters.ComputeShader.ThreadIdZ),
      "CompareToThreadIdZ");

  auto CompareXAndY =
      BC.Builder.CreateAnd(CompareToX, CompareToY, "CompareXAndY");

  auto CompareAll =
      BC.Builder.CreateAnd(CompareXAndY, CompareToZ, "CompareAll");
  return CompareAll;
}

Value *
DxilDebugInstrumentation::addVertexShaderProlog(BuilderContext &BC,
                                                SystemValueIndices SVIndices) {
  Constant *Zero32Arg = BC.HlslOP->GetU32Const(0);
  Constant *Zero8Arg = BC.HlslOP->GetI8Const(0);
  UndefValue *UndefArg = UndefValue::get(Type::getInt32Ty(BC.Ctx));

  auto LoadInputOpFunc =
      BC.HlslOP->GetOpFunc(DXIL::OpCode::LoadInput, Type::getInt32Ty(BC.Ctx));
  Constant *LoadInputOpcode =
      BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::LoadInput);
  Constant *SV_Vert_ID =
      BC.HlslOP->GetU32Const(SVIndices.VertexShader.VertexId);
  auto VertId =
      BC.Builder.CreateCall(LoadInputOpFunc,
                            {LoadInputOpcode, SV_Vert_ID, Zero32Arg /*row*/,
                             Zero8Arg /*column*/, UndefArg},
                            "VertId");

  Constant *SV_Instance_ID =
      BC.HlslOP->GetU32Const(SVIndices.VertexShader.InstanceId);
  auto InstanceId =
      BC.Builder.CreateCall(LoadInputOpFunc,
                            {LoadInputOpcode, SV_Instance_ID, Zero32Arg /*row*/,
                             Zero8Arg /*column*/, UndefArg},
                            "InstanceId");

  // Compare to expected vertex ID and instance ID
  auto CompareToVert = BC.Builder.CreateICmpEQ(
      VertId, BC.HlslOP->GetU32Const(m_Parameters.VertexShader.VertexId),
      "CompareToVertId");
  auto CompareToInstance = BC.Builder.CreateICmpEQ(
      InstanceId, BC.HlslOP->GetU32Const(m_Parameters.VertexShader.InstanceId),
      "CompareToInstanceId");
  auto CompareBoth =
      BC.Builder.CreateAnd(CompareToVert, CompareToInstance, "CompareBoth");

  return CompareBoth;
}

Value *DxilDebugInstrumentation::addHullhaderProlog(BuilderContext &BC) {
  auto LoadControlPointFunction = BC.HlslOP->GetOpFunc(
      DXIL::OpCode::OutputControlPointID, Type::getInt32Ty(BC.Ctx));
  Constant *LoadControlPointOpcode =
      BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::OutputControlPointID);
  auto ControlPointId = BC.Builder.CreateCall(
      LoadControlPointFunction, {LoadControlPointOpcode}, "ControlPointId");

  auto *CompareToPrimId =
      addComparePrimitiveIdProlog(BC, m_Parameters.HullShader.PrimitiveId);

  auto CompareToControlPoint = BC.Builder.CreateICmpEQ(
      ControlPointId,
      BC.HlslOP->GetU32Const(m_Parameters.HullShader.ControlPointId),
      "CompareToControlPointId");

  auto CompareBoth = BC.Builder.CreateAnd(CompareToControlPoint,
                                          CompareToPrimId, "CompareBoth");

  return CompareBoth;
}

Value *DxilDebugInstrumentation::addComparePrimitiveIdProlog(BuilderContext &BC,
                                                             unsigned primId) {
  auto PrimitiveIdFunction =
      BC.HlslOP->GetOpFunc(DXIL::OpCode::PrimitiveID, Type::getInt32Ty(BC.Ctx));
  Constant *PrimitiveIdOpcode =
      BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::PrimitiveID);
  auto PrimId =
      BC.Builder.CreateCall(PrimitiveIdFunction, {PrimitiveIdOpcode}, "PrimId");

  return BC.Builder.CreateICmpEQ(PrimId, BC.HlslOP->GetU32Const(primId),
                                 "CompareToPrimId");
}

Value *DxilDebugInstrumentation::addGeometryShaderProlog(BuilderContext &BC) {
  auto CompareToPrim =
      addComparePrimitiveIdProlog(BC, m_Parameters.GeometryShader.PrimitiveId);

  if (BC.DM.GetGSInstanceCount() <= 1) {
    return CompareToPrim;
  }

  auto GSInstanceIdOpFunc = BC.HlslOP->GetOpFunc(DXIL::OpCode::GSInstanceID,
                                                 Type::getInt32Ty(BC.Ctx));
  Constant *GSInstanceIdOpcode =
      BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::GSInstanceID);
  auto GSInstanceId = BC.Builder.CreateCall(
      GSInstanceIdOpFunc, {GSInstanceIdOpcode}, "GSInstanceId");

  // Compare to expected vertex ID and instance ID
  auto CompareToInstance = BC.Builder.CreateICmpEQ(
      GSInstanceId,
      BC.HlslOP->GetU32Const(m_Parameters.GeometryShader.InstanceId),
      "CompareToInstanceId");
  auto CompareBoth =
      BC.Builder.CreateAnd(CompareToPrim, CompareToInstance, "CompareBoth");

  return CompareBoth;
}

Value *
DxilDebugInstrumentation::addPixelShaderProlog(BuilderContext &BC,
                                               SystemValueIndices SVIndices) {
  Constant *Zero32Arg = BC.HlslOP->GetU32Const(0);
  Constant *Zero8Arg = BC.HlslOP->GetI8Const(0);
  Constant *One8Arg = BC.HlslOP->GetI8Const(1);
  UndefValue *UndefArg = UndefValue::get(Type::getInt32Ty(BC.Ctx));

  // Convert SV_POSITION to UINT
  Value *XAsInt;
  Value *YAsInt;
  {
    auto LoadInputOpFunc =
        BC.HlslOP->GetOpFunc(DXIL::OpCode::LoadInput, Type::getFloatTy(BC.Ctx));
    Constant *LoadInputOpcode =
        BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::LoadInput);
    Constant *SV_Pos_ID =
        BC.HlslOP->GetU32Const(SVIndices.PixelShader.Position);
    auto XPos =
        BC.Builder.CreateCall(LoadInputOpFunc,
                              {LoadInputOpcode, SV_Pos_ID, Zero32Arg /*row*/,
                               Zero8Arg /*column*/, UndefArg},
                              "XPos");
    auto YPos =
        BC.Builder.CreateCall(LoadInputOpFunc,
                              {LoadInputOpcode, SV_Pos_ID, Zero32Arg /*row*/,
                               One8Arg /*column*/, UndefArg},
                              "YPos");

    XAsInt = BC.Builder.CreateCast(Instruction::CastOps::FPToUI, XPos,
                                   Type::getInt32Ty(BC.Ctx), "XIndex");
    YAsInt = BC.Builder.CreateCast(Instruction::CastOps::FPToUI, YPos,
                                   Type::getInt32Ty(BC.Ctx), "YIndex");
  }

  // Compare to expected pixel position and primitive ID
  auto CompareToX = BC.Builder.CreateICmpEQ(
      XAsInt, BC.HlslOP->GetU32Const(m_Parameters.PixelShader.X), "CompareToX");
  auto CompareToY = BC.Builder.CreateICmpEQ(
      YAsInt, BC.HlslOP->GetU32Const(m_Parameters.PixelShader.Y), "CompareToY");
  auto ComparePos = BC.Builder.CreateAnd(CompareToX, CompareToY, "ComparePos");

  return ComparePos;
}

void DxilDebugInstrumentation::addInvocationSelectionProlog(
    BuilderContext &BC, SystemValueIndices SVIndices,
    DXIL::ShaderKind shaderKind) {
  Value *ParameterTestResult = nullptr;
  switch (shaderKind) {
  case DXIL::ShaderKind::RayGeneration:
  case DXIL::ShaderKind::ClosestHit:
  case DXIL::ShaderKind::Intersection:
  case DXIL::ShaderKind::AnyHit:
  case DXIL::ShaderKind::Miss:
    ParameterTestResult = addRaygenShaderProlog(BC);
    break;
  case DXIL::ShaderKind::Node:
    ParameterTestResult = BC.HlslOP->GetI1Const(1);
    break;
  case DXIL::ShaderKind::Compute:
  case DXIL::ShaderKind::Amplification:
  case DXIL::ShaderKind::Mesh:
    ParameterTestResult = addDispatchedShaderProlog(BC);
    break;
  case DXIL::ShaderKind::Geometry:
    ParameterTestResult = addGeometryShaderProlog(BC);
    break;
  case DXIL::ShaderKind::Vertex:
    ParameterTestResult = addVertexShaderProlog(BC, SVIndices);
    break;
  case DXIL::ShaderKind::Hull:
    ParameterTestResult = addHullhaderProlog(BC);
    break;
  case DXIL::ShaderKind::Domain:
    ParameterTestResult =
        addComparePrimitiveIdProlog(BC, m_Parameters.DomainShader.PrimitiveId);
    break;
  case DXIL::ShaderKind::Pixel:
    ParameterTestResult = addPixelShaderProlog(BC, SVIndices);
    break;
  default:
    assert(false); // guaranteed by runOnModule
  }

  auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];
  values.SelectionCriterion = ParameterTestResult;
}

void DxilDebugInstrumentation::determineLimitANDAndInitializeCounter(
    BuilderContext &BC) {

  auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];

  // Split the block at the current insertion point. Insert a conditional
  // branch that will invoke one of two new blocks depending on if this
  // is a thread-of-interest. The two different classes of thread will
  // then be given different limiting AND values within these new
  // blocks.

  BasicBlock *RestOfMainBlock = BC.Builder.GetInsertBlock()->splitBasicBlock(
      *BC.Builder.GetInsertPoint());

  // Up to this split point is a new block that we don't need to instrument:
  values.AddedBlocksToIgnoreForInstrumentation.push_back(
      BC.Builder.GetInsertBlock());

  auto *InterestingInvocationBlock = BasicBlock::Create(
      BC.Ctx, "PIXInterestingBlock", BC.Builder.GetInsertBlock()->getParent(),
      RestOfMainBlock);
  values.AddedBlocksToIgnoreForInstrumentation.push_back(
      InterestingInvocationBlock);
  IRBuilder<> BuilderForInteresting(InterestingInvocationBlock);
  BuilderForInteresting.CreateBr(RestOfMainBlock);

  auto *NonInterestingInvocationBlock = BasicBlock::Create(
      BC.Ctx, "PIXNonInterestingBlock",
      BC.Builder.GetInsertBlock()->getParent(), RestOfMainBlock);
  values.AddedBlocksToIgnoreForInstrumentation.push_back(
      NonInterestingInvocationBlock);

  IRBuilder<> BuilderForNonInteresting(NonInterestingInvocationBlock);
  BuilderForNonInteresting.CreateBr(RestOfMainBlock);

  // Connect these new blocks as necessary:
  BC.Builder.SetInsertPoint(BC.Builder.GetInsertBlock()->getTerminator());
  BC.Builder.CreateCondBr(values.SelectionCriterion, InterestingInvocationBlock,
                          NonInterestingInvocationBlock);
  BC.Builder.GetInsertBlock()->getTerminator()->eraseFromParent();

  values.OffsetMask = BC.HlslOP->GetU32Const(m_UAVSize / 4 - 1);

  // Now add a phi that selects between two constant OR values based on
  // which branch the thread followed above (interesting or not).
  // The OR will either place the output in the lower half or the upper
  // half of the UAV.
  BC.Builder.SetInsertPoint(RestOfMainBlock->getFirstInsertionPt());
  auto *PHIForOr =
      BC.Builder.CreatePHI(Type::getInt32Ty(BC.Ctx), 2, "PIXOffsetOr");
  PHIForOr->addIncoming(BC.HlslOP->GetU32Const(0), InterestingInvocationBlock);
  PHIForOr->addIncoming(BC.HlslOP->GetU32Const(m_UAVSize / 2),
                        NonInterestingInvocationBlock);
  values.OffsetOr = PHIForOr;

  auto *PHIForCounterOffset =
      BC.Builder.CreatePHI(Type::getInt32Ty(BC.Ctx), 2, "PIXCounterLocation");
  const uint32_t InterestingCounterOffset =
      static_cast<uint32_t>(m_UAVSize / 2 - sizeof(uint32_t));
  PHIForCounterOffset->addIncoming(
      BC.HlslOP->GetU32Const(InterestingCounterOffset),
      InterestingInvocationBlock);
  const uint32_t UninterestingCounterOffsetValue =
      static_cast<uint32_t>(m_UAVSize - sizeof(uint32_t));
  PHIForCounterOffset->addIncoming(
      BC.HlslOP->GetU32Const(UninterestingCounterOffsetValue),
      NonInterestingInvocationBlock);
  values.CounterOffset = PHIForCounterOffset;

  // These are reported to the caller so there are fewer assumptions made by the
  // caller about these internal details:
  *OSOverride << "InterestingCounterOffset:"
              << std::to_string(InterestingCounterOffset) << "\n";
  *OSOverride << "OverflowThreshold:" << std::to_string(m_UAVSize / 4 - 1)
              << "\n";
}

void DxilDebugInstrumentation::reserveDebugEntrySpace(BuilderContext &BC,
                                                      uint32_t SpaceInBytes) {
  auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];
  assert(values.CurrentIndex == nullptr);
  assert(m_RemainingReservedSpaceInBytes == 0);

  m_RemainingReservedSpaceInBytes = SpaceInBytes;

  // Insert the UAV increment instruction:
  Function *AtomicOpFunc =
      BC.HlslOP->GetOpFunc(OP::OpCode::AtomicBinOp, Type::getInt32Ty(BC.Ctx));
  Constant *AtomicBinOpcode =
      BC.HlslOP->GetU32Const((unsigned)OP::OpCode::AtomicBinOp);
  Constant *AtomicAdd =
      BC.HlslOP->GetU32Const((unsigned)DXIL::AtomicBinOpCode::Add);
  UndefValue *UndefArg = UndefValue::get(Type::getInt32Ty(BC.Ctx));

  Constant *Increment = BC.HlslOP->GetU32Const(SpaceInBytes);
  auto PreviousValue = BC.Builder.CreateCall(
      AtomicOpFunc,
      {
          AtomicBinOpcode,  // i32, ; opcode
          values.UAVHandle, // %dx.types.Handle, ; resource handle
          AtomicAdd, // i32, ; binary operation code : EXCHANGE, IADD, AND, OR,
                     // XOR, IMIN, IMAX, UMIN, UMAX
          values.CounterOffset, // i32, ; coordinate c0: index in bytes
          UndefArg,             // i32, ; coordinate c1 (unused)
          UndefArg,             // i32, ; coordinate c2 (unused)
          Increment,            // i32); increment value
      },
      "UAVIncResult");

  if (values.InvocationId == nullptr) {
    values.InvocationId = PreviousValue;
  }

  auto *Masked = BC.Builder.CreateAnd(PreviousValue, values.OffsetMask,
                                      "MaskedForUAVLimit");
  values.CurrentIndex =
      BC.Builder.CreateOr(Masked, values.OffsetOr, "ORedForUAVStart");
}

uint32_t DxilDebugInstrumentation::addDebugEntryValue(BuilderContext &BC,
                                                      Value *TheValue) {
  assert(m_RemainingReservedSpaceInBytes > 0);

  uint32_t BytesToBeEmitted = 0;

  auto TheValueTypeID = TheValue->getType()->getTypeID();
  if (TheValueTypeID == Type::TypeID::DoubleTyID) {
    Function *SplitDouble =
        BC.HlslOP->GetOpFunc(OP::OpCode::SplitDouble, TheValue->getType());
    Constant *SplitDoubleOpcode =
        BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::SplitDouble);
    auto SplitDoubleIntruction = BC.Builder.CreateCall(
        SplitDouble, {SplitDoubleOpcode, TheValue}, "SplitDouble");
    auto LowBits =
        BC.Builder.CreateExtractValue(SplitDoubleIntruction, 0, "LowBits");
    auto HighBits =
        BC.Builder.CreateExtractValue(SplitDoubleIntruction, 1, "HighBits");
    // addDebugEntryValue(BC, BC.HlslOP->GetU32Const(0)); // padding
    addDebugEntryValue(BC, LowBits);
    addDebugEntryValue(BC, HighBits);
    BytesToBeEmitted += 8;
  } else if (TheValueTypeID == Type::TypeID::IntegerTyID &&
             TheValue->getType()->getIntegerBitWidth() == 64) {
    auto LowBits =
        BC.Builder.CreateTrunc(TheValue, Type::getInt32Ty(BC.Ctx), "LowBits");
    auto ShiftedBits = BC.Builder.CreateLShr(TheValue, 32, "ShiftedBits");
    auto HighBits = BC.Builder.CreateTrunc(
        ShiftedBits, Type::getInt32Ty(BC.Ctx), "HighBits");
    // addDebugEntryValue(BC, BC.HlslOP->GetU32Const(0)); // padding
    addDebugEntryValue(BC, LowBits);
    addDebugEntryValue(BC, HighBits);
    BytesToBeEmitted += 8;
  } else if (TheValueTypeID == Type::TypeID::IntegerTyID &&
             (TheValue->getType()->getIntegerBitWidth() < 32)) {
    auto As32 =
        BC.Builder.CreateZExt(TheValue, Type::getInt32Ty(BC.Ctx), "As32");
    BytesToBeEmitted += addDebugEntryValue(BC, As32);
  } else if (TheValueTypeID == Type::TypeID::HalfTyID) {
    auto AsFloat =
        BC.Builder.CreateFPCast(TheValue, Type::getFloatTy(BC.Ctx), "AsFloat");
    BytesToBeEmitted += addDebugEntryValue(BC, AsFloat);
  } else {
    Function *StoreValue =
        BC.HlslOP->GetOpFunc(OP::OpCode::RawBufferStore,
                             TheValue->getType()); // Type::getInt32Ty(BC.Ctx));
    Constant *StoreValueOpcode =
        BC.HlslOP->GetU32Const((unsigned)DXIL::OpCode::RawBufferStore);
    UndefValue *Undef32Arg = UndefValue::get(Type::getInt32Ty(BC.Ctx));
    UndefValue *UndefArg = nullptr;
    if (TheValueTypeID == Type::TypeID::IntegerTyID) {
      UndefArg = UndefValue::get(Type::getInt32Ty(BC.Ctx));
    } else if (TheValueTypeID == Type::TypeID::FloatTyID) {
      UndefArg = UndefValue::get(Type::getFloatTy(BC.Ctx));
    } else {
      // The above are the only two valid types for a UAV store
      assert(false);
    }
    BytesToBeEmitted += 4;
    Constant *WriteMask_X = BC.HlslOP->GetI8Const(1);

    auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];
    Constant *RawBufferStoreAlignment = BC.HlslOP->GetU32Const(4);

    (void)BC.Builder.CreateCall(
        StoreValue, {StoreValueOpcode,    // i32 opcode
                     values.UAVHandle,    // %dx.types.Handle, ; resource handle
                     values.CurrentIndex, // i32 c0: index in bytes into UAV
                     Undef32Arg,          // i32 c1: unused
                     TheValue,
                     UndefArg, // unused values
                     UndefArg, // unused values
                     UndefArg, // unused values
                     WriteMask_X, RawBufferStoreAlignment});

    assert(m_RemainingReservedSpaceInBytes >= 4); // check for underflow
    m_RemainingReservedSpaceInBytes -= 4;

    if (m_RemainingReservedSpaceInBytes != 0) {
      values.CurrentIndex =
          BC.Builder.CreateAdd(values.CurrentIndex, BC.HlslOP->GetU32Const(4));
    } else {
      values.CurrentIndex = nullptr;
    }
  }

  return BytesToBeEmitted;
}

void DxilDebugInstrumentation::addInvocationStartMarker(BuilderContext &BC) {
  DebugShaderModifierRecordHeader marker{{{0, 0, 0, 0}}, 0};
  reserveDebugEntrySpace(BC, sizeof(marker));

  marker.Header.Details.SizeDwords =
      DebugShaderModifierRecordPayloadSizeDwords(sizeof(marker));
  marker.Header.Details.Flags = 0;
  marker.Header.Details.Type =
      DebugShaderModifierRecordTypeInvocationStartMarker;
  addDebugEntryValue(BC, BC.HlslOP->GetU32Const(marker.Header.u32Header));
  auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];
  addDebugEntryValue(BC, values.InvocationId);
}

template <typename ReturnType>
void DxilDebugInstrumentation::addStepEntryForType(
    DebugShaderModifierRecordType RecordType, BuilderContext &BC,
    std::uint32_t InstNum, Value *V, std::uint32_t ValueOrdinal,
    Value *ValueOrdinalIndex) {
  DebugShaderModifierRecordDXILStep<ReturnType> step = {};
  reserveDebugEntrySpace(BC, sizeof(step));

  auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];

  step.Header.Details.SizeDwords =
      DebugShaderModifierRecordPayloadSizeDwords(sizeof(step));
  step.Header.Details.Type = static_cast<uint8_t>(RecordType);
  addDebugEntryValue(BC, BC.HlslOP->GetU32Const(step.Header.u32Header));
  addDebugEntryValue(BC, values.InvocationId);
  addDebugEntryValue(BC, BC.HlslOP->GetU32Const(InstNum));
  if (RecordType != DebugShaderModifierRecordTypeDXILStepVoid &&
      RecordType != DebugShaderModifierRecordTypeDXILStepRet) {
    addDebugEntryValue(BC, V);
    IRBuilder<> &B = BC.Builder;

    Value *VO = BC.HlslOP->GetU32Const(ValueOrdinal << 16);
    Value *VOI = B.CreateAnd(ValueOrdinalIndex, BC.HlslOP->GetU32Const(0xFFFF),
                             "ValueOrdinalIndex");
    Value *EncodedValueOrdinalAndIndex =
        BC.Builder.CreateOr(VO, VOI, "ValueOrdinal");
    addDebugEntryValue(BC, EncodedValueOrdinalAndIndex);
  }
}

std::optional<InstructionAndType>
DxilDebugInstrumentation::addStoreStepDebugEntry(BuilderContext *BC,
                                                 StoreInst *Inst) {
  std::uint32_t ValueOrdinalBase;
  std::uint32_t UnusedValueOrdinalSize;
  llvm::Value *ValueOrdinalIndex;
  if (!pix_dxil::PixAllocaRegWrite::FromInst(Inst, &ValueOrdinalBase,
                                             &UnusedValueOrdinalSize,
                                             &ValueOrdinalIndex)) {
    return std::nullopt;
  }

  std::uint32_t InstNum;
  if (!pix_dxil::PixDxilInstNum::FromInst(Inst, &InstNum)) {
    return std::nullopt;
  }

  auto Type = addStepDebugEntryValue(BC, InstNum, Inst->getValueOperand(),
                                     ValueOrdinalBase, ValueOrdinalIndex);
  if (Type) {
    if (Instruction *ValueAsInst =
            dyn_cast<Instruction>(Inst->getValueOperand())) {
      uint32_t RegNum = 0;
      if (pix_dxil::PixDxilReg::FromInst(ValueAsInst, &RegNum)) {
        InstructionAndType ret{};
        ret.Inst = Inst;
        ret.InstructionOrdinal = InstNum;
        ret.Type = *Type;
        ret.RegisterNumber = RegNum;
        ret.AllocaBase = ValueOrdinalBase;
        ret.AllocaWriteIndex = ValueOrdinalIndex;
        return ret;
      }
    } else if (Constant *ValueAsConst =
                   dyn_cast<Constant>(Inst->getValueOperand())) {
      InstructionAndType ret{};
      ret.Inst = Inst;
      ret.InstructionOrdinal = InstNum;
      ret.Type = *Type;
      ret.AllocaBase = ValueOrdinalBase;
      ret.AllocaWriteIndex = ValueOrdinalIndex;

      switch (ValueAsConst->getType()->getTypeID()) {
      case Type::HalfTyID:
      case Type::FloatTyID:
      case Type::DoubleTyID:
        ret.ConstantAllocaStoreValue = dyn_cast<ConstantFP>(ValueAsConst)
                                           ->getValueAPF()
                                           .bitcastToAPInt()
                                           .getLimitedValue();
        break;
      case Type::IntegerTyID:
        ret.ConstantAllocaStoreValue =
            dyn_cast<ConstantInt>(ValueAsConst)->getLimitedValue();
        break;
      default:
        return std::nullopt;
      }
      return ret;
    }
  }
  return std::nullopt;
}

std::optional<InstructionAndType> DxilDebugInstrumentation::addStepDebugEntry(
    BuilderContext *BC, Instruction *Inst,
    llvm::SmallPtrSetImpl<Value *> const &RayQueryHandles) {

  std::uint32_t InstNum;
  if (!pix_dxil::PixDxilInstNum::FromInst(Inst, &InstNum)) {
    return std::nullopt;
  }

  if (RayQueryHandles.count(Inst) != 0) {
    InstructionAndType ret{};
    ret.Inst = Inst;
    ret.InstructionOrdinal = InstNum;
    ret.Type = DebugShaderModifierRecordTypeDXILStepVoid;
    return ret;
  }

  if (auto *St = llvm::dyn_cast<llvm::StoreInst>(Inst)) {
    return addStoreStepDebugEntry(BC, St);
  }

  if (auto *Ld = llvm::dyn_cast<llvm::LoadInst>(Inst)) {
    if (llvm::isa<ConstantExpr>(Ld->getPointerOperand())) {
      auto *constant = llvm::cast<ConstantExpr>(Ld->getPointerOperand());
      if (constant->getOpcode() == Instruction::GetElementPtr) {
        PIXPassHelpers::ScopedInstruction asInstr(constant->getAsInstruction());
        auto *GEP = llvm::cast<GetElementPtrInst>(asInstr.Get());
        if (GEP->getPointerOperand()->getName().equals("dx.nothing.a")) {
          // These debug-only loads are interesting as instructions to
          // step though where otherwise no step might exist for the
          // given HLSL lines, so we include them in the instrumentation:
          InstructionAndType ret{};
          ret.Inst = Inst;
          ret.InstructionOrdinal = InstNum;
          ret.Type = DebugShaderModifierRecordTypeDXILStepVoid;
          return ret;
        }
      }
    }
  }

  std::uint32_t RegNum;
  if (!pix_dxil::PixDxilReg::FromInst(Inst, &RegNum)) {
    if (Inst->getOpcode() == Instruction::Ret) {
      if (BC != nullptr)
        addStepEntryForType<void>(DebugShaderModifierRecordTypeDXILStepRet, *BC,
                                  InstNum, nullptr, 0, 0);
      InstructionAndType ret{};
      ret.Inst = Inst;
      ret.InstructionOrdinal = InstNum;
      ret.Type = DebugShaderModifierRecordTypeDXILStepRet;
      return ret;
    } else if (Inst->isTerminator()) {
      if (BC != nullptr)
        addStepEntryForType<void>(DebugShaderModifierRecordTypeDXILStepVoid,
                                  *BC, InstNum, nullptr, 0, 0);
      InstructionAndType ret{};
      ret.Inst = Inst;
      ret.InstructionOrdinal = InstNum;
      ret.Type = DebugShaderModifierRecordTypeDXILStepVoid;
      return ret;
    }
    return std::nullopt;
  }
  auto Type = addStepDebugEntryValue(BC, InstNum, Inst, RegNum,
                                     BC ? BC->Builder.getInt32(0) : nullptr);
  if (Type) {
    InstructionAndType ret{};
    ret.Inst = Inst;
    ret.InstructionOrdinal = InstNum;
    ret.Type = *Type;
    ret.RegisterNumber = RegNum;
    return ret;
  }
  return std::nullopt;
}

std::optional<DebugShaderModifierRecordType>
DxilDebugInstrumentation::addStepDebugEntryValue(BuilderContext *BC,
                                                 std::uint32_t InstNum,
                                                 Value *V,
                                                 std::uint32_t ValueOrdinal,
                                                 Value *ValueOrdinalIndex) {
  const Type::TypeID ID = V->getType()->getTypeID();

  switch (ID) {
  case Type::TypeID::StructTyID:
  case Type::TypeID::VoidTyID:
    if (BC != nullptr)
      addStepEntryForType<void>(DebugShaderModifierRecordTypeDXILStepVoid, *BC,
                                InstNum, V, ValueOrdinal, ValueOrdinalIndex);
    return DebugShaderModifierRecordTypeDXILStepVoid;
  case Type::TypeID::FloatTyID:
    if (BC != nullptr)
      addStepEntryForType<float>(DebugShaderModifierRecordTypeDXILStepFloat,
                                 *BC, InstNum, V, ValueOrdinal,
                                 ValueOrdinalIndex);
    return DebugShaderModifierRecordTypeDXILStepFloat;
  case Type::TypeID::IntegerTyID:
    assert(V->getType()->getIntegerBitWidth() == 64 ||
           V->getType()->getIntegerBitWidth() <= 32);
    if (V->getType()->getIntegerBitWidth() > 64) {
      return std::nullopt;
    }
    if (V->getType()->getIntegerBitWidth() == 64) {
      if (BC != nullptr)
        addStepEntryForType<uint64_t>(
            DebugShaderModifierRecordTypeDXILStepUint64, *BC, InstNum, V,
            ValueOrdinal, ValueOrdinalIndex);
      return DebugShaderModifierRecordTypeDXILStepUint64;
    } else {
      if (V->getType()->getIntegerBitWidth() > 32) {
        return std::nullopt;
      }
      if (BC != nullptr)
        addStepEntryForType<uint32_t>(
            DebugShaderModifierRecordTypeDXILStepUint32, *BC, InstNum, V,
            ValueOrdinal, ValueOrdinalIndex);
      return DebugShaderModifierRecordTypeDXILStepUint32;
    }
  case Type::TypeID::DoubleTyID:
    if (BC != nullptr)
      addStepEntryForType<double>(DebugShaderModifierRecordTypeDXILStepDouble,
                                  *BC, InstNum, V, ValueOrdinal,
                                  ValueOrdinalIndex);
    return DebugShaderModifierRecordTypeDXILStepDouble;
  case Type::TypeID::HalfTyID:
    if (BC != nullptr)
      addStepEntryForType<float>(DebugShaderModifierRecordTypeDXILStepFloat,
                                 *BC, InstNum, V, ValueOrdinal,
                                 ValueOrdinalIndex);
    return DebugShaderModifierRecordTypeDXILStepFloat;
  case Type::TypeID::PointerTyID:
    // Skip pointer calculation instructions. They aren't particularly
    // meaningful to the user (being a mere implementation detail for lookup
    // tables, etc.), and their type is problematic from a UI point of view.
    // The subsequent instructions that dereference the pointer will be
    // properly instrumented and show the (meaningful) retrieved value.
    break;
  case Type::TypeID::VectorTyID:
    // Shows up in "insertelement" in raygen shader?
    break;
  case Type::TypeID::FP128TyID:
  case Type::TypeID::LabelTyID:
  case Type::TypeID::MetadataTyID:
  case Type::TypeID::FunctionTyID:
  case Type::TypeID::ArrayTyID:
  case Type::TypeID::X86_FP80TyID:
  case Type::TypeID::X86_MMXTyID:
  case Type::TypeID::PPC_FP128TyID:
    assert(false);
  }
  return std::nullopt;
}

bool DxilDebugInstrumentation::runOnModule(Module &M) {
  DxilModule &DM = M.GetOrCreateDxilModule();

  // There is no point running this pass if it can't return its report:
  if (OSOverride == nullptr)
    return false;

  auto ShaderModel = DM.GetShaderModel();
  auto shaderKind = ShaderModel->GetKind();
  auto HLSLBindId = 0;
  auto *uav = PIXPassHelpers::CreateGlobalUAVResource(DM, HLSLBindId, "PIXUAV");
  bool modified = false;
  if (shaderKind == DXIL::ShaderKind::Library) {
    auto instrumentableFunctions =
        PIXPassHelpers::GetAllInstrumentableFunctions(DM);
    for (auto *F : instrumentableFunctions) {
      if (RunOnFunction(M, DM, uav, F)) {
        modified = true;
      }
    }
  } else {
    llvm::Function *entryFunction = PIXPassHelpers::GetEntryFunction(DM);
    modified = RunOnFunction(M, DM, uav, entryFunction);
  }
  return modified;
}

struct RecordTypeDatum {
  DebugShaderModifierRecordType Type;
  uint32_t PayloadSize;
  const char *AsString;
};

static const RecordTypeDatum RecordTypeData[] = {
    {DebugShaderModifierRecordTypeDXILStepRet, 0, "r"},
    {DebugShaderModifierRecordTypeDXILStepVoid, 0, "v"},
    {DebugShaderModifierRecordTypeDXILStepFloat, 4, "f"},
    {DebugShaderModifierRecordTypeDXILStepUint32, 4, "3"},
    {DebugShaderModifierRecordTypeDXILStepUint64, 8, "6"},
    {DebugShaderModifierRecordTypeDXILStepDouble, 8, "d"}};

std::optional<RecordTypeDatum const *>
FindDatum(DebugShaderModifierRecordType RecordType) {
  for (auto const &datum : RecordTypeData) {
    if (datum.Type == RecordType) {
      return &datum;
    }
  }
  return std::nullopt;
}

uint32_t DxilDebugInstrumentation::CountBlockPayloadBytes(
    std::vector<InstructionToInstrument> const &IsAndTs) {
  uint32_t count = 0;
  for (auto const &IandT : IsAndTs) {
    auto datum = FindDatum(IandT.ValueType);
    if (datum)
      count += (*datum)->PayloadSize;
  }
  return count;
}

const char *TypeString(InstructionAndType const &IandT) {
  auto datum = FindDatum(IandT.Type);
  if (datum)
    return (*datum)->AsString;
  assert(false);
  return "v";
}

Instruction *FindFirstNonPhiInstruction(Instruction *I) {
  while (llvm::isa<llvm::PHINode>(I))
    I = I->getNextNode();
  return I;
}

// This function reports a textual representation of the format
// of the debug data that will be output by the instructions
// added by this pass.
// The string has one or more lines of the exemplary form
//      Block#3:5,f,22,a;7,f,22,s,20;9,f,22,s,20;10,f,23,a;12,f,23,s,21;
// The integer after the Block# is the first instruction number in the
// block.
// Instructions are delimited by ; The fields within the instruction
// (delimited by ,) are, in order:
// -instruction ordinal
// -data type (r=ret, v=void, f=float, 3=int32, 6=int64, d=double)
// -scalar register number
// -alloca/scalar indicator:
// r == ret instruction
// a == scalar is being created and assigned a value, and that
//      value is in the debug output.
// s == Existing scalar is being assigned via static alloca index.
//      Index is appended to this instruction record. No
//      corresponding data in the debug output.
// d == A dynamic index added to the static base index. Base index
//      is appended to this record. The corresponding debug entry is
//      the dynamic index into that alloca.
// v == A void terminator or other void-valued instruction. No
//      corresponding data in the debug output.
// If indicator is "a", a string of the form [base+index] for the alloca
// store location.
// If indicator is "d", a single integer denoting the base for the alloca
// store.
DxilDebugInstrumentation::BlockInstrumentationData
DxilDebugInstrumentation::FindInstrumentableInstructionsInBlock(
    BasicBlock &BB, OP *HlslOP,
    llvm::SmallPtrSetImpl<Value *> const &RayQueryHandles) {
  BlockInstrumentationData ret{};
  auto &Is = BB.getInstList();
  *OSOverride << "Block#";
  bool FoundFirstInstruction = false;
  for (auto &Inst : Is) {
    if (!FoundFirstInstruction) {
      std::uint32_t InstNum;
      if (pix_dxil::PixDxilInstNum::FromInst(&Inst, &InstNum)) {
        *OSOverride << std::to_string(InstNum) << ":";
        ret.FirstInstructionOrdinalInBlock = InstNum;
        FoundFirstInstruction = true;
      }
    }
    auto IandT = addStepDebugEntry(nullptr, &Inst, RayQueryHandles);
    if (IandT) {
      InstructionToInstrument DebugOutputForThisInstruction{};
      DebugOutputForThisInstruction.ValueType = IandT->Type;
      auto *InsertionPoint = FindFirstNonPhiInstruction(&Inst);
      if (InsertionPoint->isTerminator() || llvm::isa<llvm::PHINode>(Inst))
        DebugOutputForThisInstruction
            .InstructionBeforeWhichToAddInstrumentation = InsertionPoint;
      else
        DebugOutputForThisInstruction
            .InstructionAfterWhichToAddInstrumentation = InsertionPoint;

      const char *IndexingToken = nullptr;
      std::optional<std::string> RegisterOrStaticIndex;
      if (IandT->Type == DebugShaderModifierRecordTypeDXILStepRet) {
        IndexingToken = "r";
      } else if (IandT->Type == DebugShaderModifierRecordTypeDXILStepVoid) {
        IndexingToken = "v"; // void instruction, no debug output required
      } else if (IandT->AllocaWriteIndex != nullptr) {
        if (ConstantInt *IndexAsConstant =
                dyn_cast<ConstantInt>(IandT->AllocaWriteIndex)) {
          RegisterOrStaticIndex =
              std::to_string(IandT->AllocaBase) + "+" +
              std::to_string(IndexAsConstant->getLimitedValue());
          IndexingToken = "s"; // static indexing, no debug output required
        } else {
          IndexingToken = "d"; // dynamic indexing
          int MaxArraySize = 1;
          if (auto *Store = dyn_cast<StoreInst>(&Inst)) {
            if (auto *GEP =
                    dyn_cast<GetElementPtrInst>(Store->getPointerOperand())) {
              if (auto *Alloca =
                      dyn_cast<AllocaInst>(GEP->getPointerOperand())) {
                MaxArraySize =
                    Alloca->getAllocatedType()->getArrayNumElements();
              }
            }
          }
          RegisterOrStaticIndex = std::to_string(IandT->AllocaBase) + "-" +
                                  std::to_string(MaxArraySize);
          DebugOutputForThisInstruction.ValueToWriteToDebugMemory =
              IandT->AllocaWriteIndex;
        }
      } else {
        IndexingToken = "a"; // meaning an SSA assignment
        // todo: Can SSA Values be assigned a literal constant?
        DebugOutputForThisInstruction.ValueToWriteToDebugMemory = IandT->Inst;
      }

      *OSOverride << std::to_string(IandT->InstructionOrdinal) << ","
                  << TypeString(*IandT) << ","
                  << std::to_string(IandT->RegisterNumber) << ","
                  << IndexingToken;
      if (RegisterOrStaticIndex) {
        *OSOverride << "," << *RegisterOrStaticIndex;
      }
      if (IandT->ConstantAllocaStoreValue) {
        uint64_t value = IandT->ConstantAllocaStoreValue.value();
        *OSOverride << "," << std::to_string(value);
      }
      *OSOverride << ";";
      if (DebugOutputForThisInstruction.ValueToWriteToDebugMemory)
        ret.Instructions.push_back(std::move(DebugOutputForThisInstruction));
    }
  }
  *OSOverride << "\n";
  return ret;
}

bool DxilDebugInstrumentation::RunOnFunction(Module &M, DxilModule &DM,
                                             hlsl::DxilResource *uav,
                                             llvm::Function *function) {
  DXIL::ShaderKind shaderKind =
      PIXPassHelpers::GetFunctionShaderKind(DM, function);

  switch (shaderKind) {
  case DXIL::ShaderKind::Amplification:
  case DXIL::ShaderKind::Mesh:
  case DXIL::ShaderKind::Vertex:
  case DXIL::ShaderKind::Geometry:
  case DXIL::ShaderKind::Pixel:
  case DXIL::ShaderKind::Compute:
  case DXIL::ShaderKind::RayGeneration:
  case DXIL::ShaderKind::Hull:
  case DXIL::ShaderKind::Domain:
  case DXIL::ShaderKind::Intersection:
  case DXIL::ShaderKind::AnyHit:
  case DXIL::ShaderKind::ClosestHit:
  case DXIL::ShaderKind::Miss:
  case DXIL::ShaderKind::Node:
    break;
  default:
    return false;
  }
  llvm::SmallPtrSet<Value *, 16> RayQueryHandles;
  PIXPassHelpers::FindRayQueryHandlesForFunction(function, RayQueryHandles);

  Instruction *firstInsertionPt = dxilutil::FirstNonAllocaInsertionPt(function);
  IRBuilder<> Builder(firstInsertionPt);

  LLVMContext &Ctx = M.getContext();
  OP *HlslOP = DM.GetOP();

  BuilderContext BC{M, DM, Ctx, HlslOP, Builder};

  auto &values = m_FunctionToValues[BC.Builder.GetInsertBlock()->getParent()];

  // PIX binds two UAVs when running this instrumentation: one for raygen
  // shaders and another for the hitgroups and miss shaders. Since PIX invokes
  // this pass at the library level, which may contain examples of both types,
  // PIX can't really specify which UAV index to use per-shader. This pass
  // therefore just has to know this:
  constexpr unsigned int RayGenUAVRegister = 0;
  constexpr unsigned int HitGroupAndMissUAVRegister = 1;
  unsigned int UAVRegisterId = RayGenUAVRegister;
  switch (shaderKind) {
  case DXIL::ShaderKind::ClosestHit:
  case DXIL::ShaderKind::Intersection:
  case DXIL::ShaderKind::AnyHit:
  case DXIL::ShaderKind::Miss:
    UAVRegisterId = HitGroupAndMissUAVRegister;
    break;
  }

  values.UAVHandle = PIXPassHelpers::CreateHandleForResource(
      DM, Builder, uav, "PIX_DebugUAV_Handle");

  auto SystemValues = addRequiredSystemValues(BC, shaderKind);
  addInvocationSelectionProlog(BC, SystemValues, shaderKind);
  determineLimitANDAndInitializeCounter(BC);
  addInvocationStartMarker(BC);

  // Instrument original instructions:
  for (auto &BB : function->getBasicBlockList()) {
    if (std::find(values.AddedBlocksToIgnoreForInstrumentation.begin(),
                  values.AddedBlocksToIgnoreForInstrumentation.end(),
                  &BB) == values.AddedBlocksToIgnoreForInstrumentation.end()) {
      auto BlockInstrumentation =
          FindInstrumentableInstructionsInBlock(BB, BC.HlslOP, RayQueryHandles);
      if (BlockInstrumentation.FirstInstructionOrdinalInBlock <
              m_FirstInstruction ||
          BlockInstrumentation.FirstInstructionOrdinalInBlock >=
              m_LastInstruction)
        continue;
      uint32_t BlockPayloadBytes =
          CountBlockPayloadBytes(BlockInstrumentation.Instructions);
      // If the block has no instructions which require debug output,
      // we will still write an empty block header at the end of that
      // block (i.e. before the terminator) so that the instrumentation
      // at least indicates that flow control went through the block.
      Instruction *BlockInstrumentationStart = (BB).getTerminator();
      if (!BlockInstrumentation.Instructions.empty()) {
        auto const &First = BlockInstrumentation.Instructions[0];
        if (First.InstructionAfterWhichToAddInstrumentation != nullptr)
          BlockInstrumentationStart =
              First.InstructionAfterWhichToAddInstrumentation;
        else if (First.InstructionBeforeWhichToAddInstrumentation != nullptr)
          BlockInstrumentationStart =
              First.InstructionBeforeWhichToAddInstrumentation;
        else {
          assert(false);
          continue;
        }
      }
      IRBuilder<> Builder(BlockInstrumentationStart);
      BuilderContext BCForBlock{BC.M, BC.DM, BC.Ctx, BC.HlslOP, Builder};

      DebugShaderModifierRecordDXILBlock step = {};
      auto FullRecordSize =
          static_cast<uint32_t>(sizeof(step) + BlockPayloadBytes);
      if (FullRecordSize >= (m_UAVSize / 4) - 1) {
        *OSOverride << "StaticOverflow:" << std::to_string(FullRecordSize)
                    << "\n";
        break;
      }
      reserveDebugEntrySpace(BCForBlock, FullRecordSize);
      step.Header.Details.CountOfInstructions =
          static_cast<uint16_t>(BlockInstrumentation.Instructions.size());
      step.Header.Details.Type =
          static_cast<uint8_t>(DebugShaderModifierRecordTypeDXILStepBlock);
      addDebugEntryValue(BCForBlock,
                         BCForBlock.HlslOP->GetU32Const(step.Header.u32Header));
      addDebugEntryValue(BCForBlock, values.InvocationId);
      addDebugEntryValue(
          BCForBlock, BCForBlock.HlslOP->GetU32Const(
                          BlockInstrumentation.FirstInstructionOrdinalInBlock));
      for (auto &Inst : BlockInstrumentation.Instructions) {
        Instruction *BuilderInstruction;
        if (Inst.InstructionAfterWhichToAddInstrumentation != nullptr)
          BuilderInstruction =
              Inst.InstructionAfterWhichToAddInstrumentation->getNextNode();
        else if (Inst.InstructionBeforeWhichToAddInstrumentation != nullptr)
          BuilderInstruction = Inst.InstructionBeforeWhichToAddInstrumentation;
        else {
          assert(false);
          continue;
        }
        IRBuilder<> Builder(BuilderInstruction);
        BuilderContext BC2{BC.M, BC.DM, BC.Ctx, BC.HlslOP, Builder};
        addDebugEntryValue(BC2, Inst.ValueToWriteToDebugMemory);
      }
    }
  }

  return true;
}

char DxilDebugInstrumentation::ID = 0;

ModulePass *llvm::createDxilDebugInstrumentationPass() {
  return new DxilDebugInstrumentation();
}

INITIALIZE_PASS(DxilDebugInstrumentation, "hlsl-dxil-debug-instrumentation",
                "HLSL DXIL debug instrumentation for PIX", false, false)
