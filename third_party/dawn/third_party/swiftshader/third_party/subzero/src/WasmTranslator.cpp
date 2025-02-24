//===- subzero/src/WasmTranslator.cpp - WASM to Subzero Translation -------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines a driver for translating Wasm bitcode into PNaCl bitcode.
///
/// The translator uses V8's WebAssembly decoder to handle the binary Wasm
/// format but replaces the usual TurboFan builder with a new PNaCl builder.
///
//===----------------------------------------------------------------------===//

#if ALLOW_WASM

#include "WasmTranslator.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif // __clang__
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif // defined(__GNUC__) && !defined(__clang__)

#include "src/wasm/module-decoder.h"
#include "src/wasm/wasm-opcodes.h"
#include "src/zone.h"

#include "src/bit-vector.h"

#include "src/wasm/ast-decoder-impl.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__) && !defined(__clang__)

#include "IceCfgNode.h"
#include "IceGlobalInits.h"

using namespace std;
using namespace Ice;
using namespace v8::internal;
using namespace v8::internal::wasm;
using v8::internal::wasm::DecodeWasmModule;

#undef LOG
#define LOG(Expr) log([&](Ostream &out) { Expr; })

namespace {
// 64KB
const uint32_t WASM_PAGE_SIZE = 64 << 10;

std::string toStdString(WasmName Name) {
  return std::string(Name.name, Name.length);
}

Ice::Type toIceType(wasm::LocalType Type) {
  switch (Type) {
  case MachineRepresentation::kNone:
    llvm::report_fatal_error("kNone type not supported");
  case MachineRepresentation::kBit:
    return IceType_i1;
  case MachineRepresentation::kWord8:
    return IceType_i8;
  case MachineRepresentation::kWord16:
    return IceType_i16;
  case MachineRepresentation::kWord32:
    return IceType_i32;
  case MachineRepresentation::kWord64:
    return IceType_i64;
  case MachineRepresentation::kFloat32:
    return IceType_f32;
  case MachineRepresentation::kFloat64:
    return IceType_f64;
  case MachineRepresentation::kSimd128:
    llvm::report_fatal_error("ambiguous SIMD type");
  case MachineRepresentation::kTagged:
    llvm::report_fatal_error("kTagged type not supported");
  }
  llvm::report_fatal_error("unexpected type");
}

Ice::Type toIceType(v8::internal::MachineType Type) {
  // TODO (eholk): reorder these based on expected call frequency.
  if (Type == MachineType::Int32()) {
    return IceType_i32;
  }
  if (Type == MachineType::Uint32()) {
    return IceType_i32;
  }
  if (Type == MachineType::Int8()) {
    return IceType_i8;
  }
  if (Type == MachineType::Uint8()) {
    return IceType_i8;
  }
  if (Type == MachineType::Int16()) {
    return IceType_i16;
  }
  if (Type == MachineType::Uint16()) {
    return IceType_i16;
  }
  if (Type == MachineType::Int64()) {
    return IceType_i64;
  }
  if (Type == MachineType::Uint64()) {
    return IceType_i64;
  }
  if (Type == MachineType::Float32()) {
    return IceType_f32;
  }
  if (Type == MachineType::Float64()) {
    return IceType_f64;
  }
  llvm::report_fatal_error("Unsupported MachineType");
}

std::string fnNameFromId(uint32_t Id) {
  return std::string("fn") + to_string(Id);
}

std::string getFunctionName(const WasmModule *Module, uint32_t func_index) {
  // Try to find the function name in the export table
  for (const auto Export : Module->export_table) {
    if (Export.func_index == func_index) {
      return "__szwasm_" + toStdString(Module->GetName(Export.name_offset,
                                                       Export.name_length));
    }
  }
  return fnNameFromId(func_index);
}

} // end of anonymous namespace

/// This class wraps either an Operand or a CfgNode.
///
/// Turbofan's sea of nodes representation only has nodes for values, control
/// flow, etc. In Subzero these concepts are all separate. This class lets V8's
/// Wasm decoder treat Subzero objects as though they are all the same.
class OperandNode {
  static constexpr uintptr_t NODE_FLAG = 1;
  static constexpr uintptr_t UNDEF_PTR = (uintptr_t)-1;

  uintptr_t Data = UNDEF_PTR;

public:
  OperandNode() = default;
  explicit OperandNode(Operand *Operand)
      : Data(reinterpret_cast<uintptr_t>(Operand)) {}
  explicit OperandNode(CfgNode *Node)
      : Data(reinterpret_cast<uintptr_t>(Node) | NODE_FLAG) {}
  explicit OperandNode(nullptr_t) : Data(UNDEF_PTR) {}

  operator Operand *() const {
    if (UNDEF_PTR == Data) {
      return nullptr;
    }
    if (!isOperand()) {
      llvm::report_fatal_error("This OperandNode is not an Operand");
    }
    return reinterpret_cast<Operand *>(Data);
  }

  operator CfgNode *() const {
    if (UNDEF_PTR == Data) {
      return nullptr;
    }
    if (!isCfgNode()) {
      llvm::report_fatal_error("This OperandNode is not a CfgNode");
    }
    return reinterpret_cast<CfgNode *>(Data & ~NODE_FLAG);
  }

  explicit operator bool() const { return (Data != UNDEF_PTR) && Data; }
  bool operator==(const OperandNode &Rhs) const {
    return (Data == Rhs.Data) ||
           (UNDEF_PTR == Data && (Rhs.Data == 0 || Rhs.Data == NODE_FLAG)) ||
           (UNDEF_PTR == Rhs.Data && (Data == 0 || Data == NODE_FLAG));
  }
  bool operator!=(const OperandNode &Rhs) const { return !(*this == Rhs); }

  bool isOperand() const { return (Data != UNDEF_PTR) && !(Data & NODE_FLAG); }
  bool isCfgNode() const { return (Data != UNDEF_PTR) && (Data & NODE_FLAG); }

  Operand *toOperand() const { return static_cast<Operand *>(*this); }

  CfgNode *toCfgNode() const { return static_cast<CfgNode *>(*this); }
};

Ostream &operator<<(Ostream &Out, const OperandNode &Op) {
  if (Op.isOperand()) {
    const auto *Oper = Op.toOperand();
    Out << "(Operand*)" << Oper;
    if (Oper) {
      Out << "::" << Oper->getType();
    }
  } else if (Op.isCfgNode()) {
    Out << "(CfgNode*)" << Op.toCfgNode();
  } else {
    Out << "nullptr";
  }
  return Out;
}

bool isComparison(wasm::WasmOpcode Opcode) {
  switch (Opcode) {
  case kExprI32Ne:
  case kExprI64Ne:
  case kExprI32Eq:
  case kExprI64Eq:
  case kExprI32LtS:
  case kExprI64LtS:
  case kExprI32LtU:
  case kExprI64LtU:
  case kExprI32GeS:
  case kExprI64GeS:
  case kExprI32GtS:
  case kExprI64GtS:
  case kExprI32GtU:
  case kExprI64GtU:
  case kExprF32Eq:
  case kExprF64Eq:
  case kExprF32Ne:
  case kExprF64Ne:
  case kExprF32Le:
  case kExprF64Le:
  case kExprF32Lt:
  case kExprF64Lt:
  case kExprF32Ge:
  case kExprF64Ge:
  case kExprF32Gt:
  case kExprF64Gt:
  case kExprI32LeS:
  case kExprI64LeS:
  case kExprI32GeU:
  case kExprI64GeU:
  case kExprI32LeU:
  case kExprI64LeU:
    return true;
  default:
    return false;
  }
}

class IceBuilder {
  using Node = OperandNode;
  using Variable = Ice::Variable;

  IceBuilder() = delete;
  IceBuilder(const IceBuilder &) = delete;
  IceBuilder &operator=(const IceBuilder &) = delete;

public:
  explicit IceBuilder(class Cfg *Func)
      : ControlPtr(nullptr), Func(Func), Ctx(Func->getContext()) {}

  /// Allocates a buffer of Nodes for use by V8.
  Node *Buffer(size_t Count) {
    LOG(out << "Buffer(" << Count << ")\n");
    return Func->allocateArrayOf<Node>(Count);
  }

  Node Error() { llvm::report_fatal_error("Error"); }
  Node Start(uint32_t Params) {
    LOG(out << "Start(" << Params << ") = ");
    auto *Entry = Func->getEntryNode();
    assert(Entry);
    LOG(out << Node(Entry) << "\n");

    // Load the WasmMemory address to make it available everywhere else in the
    // function.
    auto *WasmMemoryPtr =
        Ctx->getConstantExternSym(Ctx->getGlobalString("WASM_MEMORY"));
    assert(WasmMemory == nullptr);
    auto *WasmMemoryV = makeVariable(getPointerType());
    Entry->appendInst(InstLoad::create(Func, WasmMemoryV, WasmMemoryPtr));
    WasmMemory = WasmMemoryV;

    return OperandNode(Entry);
  }
  Node Param(uint32_t Index, wasm::LocalType Type) {
    LOG(out << "Param(" << Index << ") = ");
    auto *Arg = makeVariable(toIceType(Type));
    assert(Index == NextArg);
    Func->addArg(Arg);
    ++NextArg;
    LOG(out << Node(Arg) << "\n");
    return OperandNode(Arg);
  }
  Node Loop(CfgNode *Entry) {
    auto *Loop = Func->makeNode();
    LOG(out << "Loop(" << Entry << ") = " << Loop << "\n");
    Entry->appendInst(InstBr::create(Func, Loop));
    return OperandNode(Loop);
  }
  void Terminate(Node Effect, Node Control) {
    // TODO(eholk): this is almost certainly wrong
    LOG(out << "Terminate(" << Effect << ", " << Control << ")"
            << "\n");
  }
  Node Merge(uint32_t Count, Node *Controls) {
    LOG(out << "Merge(" << Count);
    for (uint32_t i = 0; i < Count; ++i) {
      LOG(out << ", " << Controls[i]);
    }
    LOG(out << ") = ");

    auto *MergedNode = Func->makeNode();

    for (uint32_t i = 0; i < Count; ++i) {
      CfgNode *Control = Controls[i];
      Control->appendInst(InstBr::create(Func, MergedNode));
    }
    LOG(out << (OperandNode)MergedNode << "\n");
    return OperandNode(MergedNode);
  }
  Node Phi(wasm::LocalType, uint32_t Count, Node *Vals, Node Control) {
    LOG(out << "Phi(" << Count << ", " << Control);
    for (uint32_t i = 0; i < Count; ++i) {
      LOG(out << ", " << Vals[i]);
    }
    LOG(out << ") = ");

    const auto &InEdges = Control.toCfgNode()->getInEdges();
    assert(Count == InEdges.size());

    assert(Count > 0);

    auto *Dest = makeVariable(Vals[0].toOperand()->getType(), Control);

    // Multiply by 200 in case more things get added later.

    // TODO(eholk): find a better way besides multiplying by some arbitrary
    // constant.
    auto *Phi = InstPhi::create(Func, Count * 200, Dest);
    for (uint32_t i = 0; i < Count; ++i) {
      auto *Op = Vals[i].toOperand();
      assert(Op);
      Phi->addArgument(Op, InEdges[i]);
    }
    setDefiningInst(Dest, Phi);
    Control.toCfgNode()->appendInst(Phi);
    LOG(out << Node(Dest) << "\n");
    return OperandNode(Dest);
  }
  Node EffectPhi(uint32_t Count, Node *Effects, Node Control) {
    // TODO(eholk): this function is almost certainly wrong.
    LOG(out << "EffectPhi(" << Count << ", " << Control << "):\n");
    for (uint32_t i = 0; i < Count; ++i) {
      LOG(out << "  " << Effects[i] << "\n");
    }
    return OperandNode(nullptr);
  }
  Node Int32Constant(int32_t Value) {
    LOG(out << "Int32Constant(" << Value << ") = ");
    auto *Const = Ctx->getConstantInt32(Value);
    assert(Const);
    assert(Control());
    LOG(out << Node(Const) << "\n");
    return OperandNode(Const);
  }
  Node Int64Constant(int64_t Value) {
    LOG(out << "Int64Constant(" << Value << ") = ");
    auto *Const = Ctx->getConstantInt64(Value);
    assert(Const);
    LOG(out << Node(Const) << "\n");
    return OperandNode(Const);
  }
  Node Float32Constant(float Value) {
    LOG(out << "Float32Constant(" << Value << ") = ");
    auto *Const = Ctx->getConstantFloat(Value);
    assert(Const);
    LOG(out << Node(Const) << "\n");
    return OperandNode(Const);
  }
  Node Float64Constant(double Value) {
    LOG(out << "Float64Constant(" << Value << ") = ");
    auto *Const = Ctx->getConstantDouble(Value);
    assert(Const);
    LOG(out << Node(Const) << "\n");
    return OperandNode(Const);
  }
  Node Binop(wasm::WasmOpcode Opcode, Node Left, Node Right) {
    LOG(out << "Binop(" << WasmOpcodes::OpcodeName(Opcode) << ", " << Left
            << ", " << Right << ") = ");
    BooleanVariable *BoolDest = nullptr;
    Variable *Dest = nullptr;
    if (isComparison(Opcode)) {
      BoolDest = makeVariable<BooleanVariable>(IceType_i32);
      Dest = BoolDest;
    } else {
      Dest = makeVariable(Left.toOperand()->getType());
    }
    switch (Opcode) {
    case kExprI32Add:
    case kExprI64Add:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Add, Dest, Left, Right));
      break;
    case kExprF32Add:
    case kExprF64Add:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Fadd,
                                                   Dest, Left, Right));
      break;
    case kExprI32Sub:
    case kExprI64Sub:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Sub, Dest, Left, Right));
      break;
    case kExprF32Sub:
    case kExprF64Sub:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Fsub,
                                                   Dest, Left, Right));
      break;
    case kExprI32Mul:
    case kExprI64Mul:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Mul, Dest, Left, Right));
      break;
    case kExprF32Mul:
    case kExprF64Mul:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Fmul,
                                                   Dest, Left, Right));
      break;
    case kExprI32DivS:
    case kExprI64DivS:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Sdiv,
                                                   Dest, Left, Right));
      break;
    case kExprI32DivU:
    case kExprI64DivU:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Udiv,
                                                   Dest, Left, Right));
      break;
    case kExprF32Div:
    case kExprF64Div:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Fdiv,
                                                   Dest, Left, Right));
      break;
    case kExprI32RemU:
    case kExprI64RemU:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Urem,
                                                   Dest, Left, Right));
      break;
    case kExprI32RemS:
    case kExprI64RemS:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Srem,
                                                   Dest, Left, Right));
      break;
    case kExprI32Ior:
    case kExprI64Ior:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Or, Dest, Left, Right));
      break;
    case kExprI32Xor:
    case kExprI64Xor:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Xor, Dest, Left, Right));
      break;
    case kExprI32Shl:
    case kExprI64Shl:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Shl, Dest, Left, Right));
      break;
    case kExprI32Rol:
    case kExprI64Rol: {
      // TODO(eholk): add rotate as an ICE instruction to make it easier to take
      // advantage of hardware support.

      const auto DestTy = Left.toOperand()->getType();
      const SizeT BitCount = typeWidthInBytes(DestTy) * CHAR_BIT;

      auto *Masked = makeVariable(DestTy);
      auto *Bottom = makeVariable(DestTy);
      auto *Top = makeVariable(DestTy);
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::And, Masked, Right,
                                 Ctx->getConstantInt(DestTy, BitCount - 1)));
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Shl, Top, Left, Masked));
      auto *RotShift = makeVariable(DestTy);
      Control()->appendInst(InstArithmetic::create(
          Func, InstArithmetic::Sub, RotShift,
          Ctx->getConstantInt(DestTy, BitCount), Masked));
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Lshr,
                                                   Bottom, Left, RotShift));
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Or, Dest, Top, Bottom));
      break;
    }
    case kExprI32Ror:
    case kExprI64Ror: {
      // TODO(eholk): add rotate as an ICE instruction to make it easier to take
      // advantage of hardware support.

      const auto DestTy = Left.toOperand()->getType();
      const SizeT BitCount = typeWidthInBytes(DestTy) * CHAR_BIT;

      auto *Masked = makeVariable(DestTy);
      auto *Bottom = makeVariable(DestTy);
      auto *Top = makeVariable(DestTy);
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::And, Masked, Right,
                                 Ctx->getConstantInt(DestTy, BitCount - 1)));
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Lshr,
                                                   Top, Left, Masked));
      auto *RotShift = makeVariable(DestTy);
      Control()->appendInst(InstArithmetic::create(
          Func, InstArithmetic::Sub, RotShift,
          Ctx->getConstantInt(DestTy, BitCount), Masked));
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Shl,
                                                   Bottom, Left, RotShift));
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::Or, Dest, Top, Bottom));
      break;
    }
    case kExprI32ShrU:
    case kExprI64ShrU:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Lshr,
                                                   Dest, Left, Right));
      break;
    case kExprI32ShrS:
    case kExprI64ShrS:
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Ashr,
                                                   Dest, Left, Right));
      break;
    case kExprI32And:
    case kExprI64And:
      Control()->appendInst(
          InstArithmetic::create(Func, InstArithmetic::And, Dest, Left, Right));
      break;
    case kExprI32Ne:
    case kExprI64Ne: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Ne, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32Eq:
    case kExprI64Eq: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Eq, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32LtS:
    case kExprI64LtS: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Slt, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32LeS:
    case kExprI64LeS: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Sle, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32GeU:
    case kExprI64GeU: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Uge, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32LeU:
    case kExprI64LeU: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Ule, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32LtU:
    case kExprI64LtU: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Ult, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32GeS:
    case kExprI64GeS: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Sge, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32GtS:
    case kExprI64GtS: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Sgt, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprI32GtU:
    case kExprI64GtU: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstIcmp::create(Func, InstIcmp::Ugt, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprF32Eq:
    case kExprF64Eq: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstFcmp::create(Func, InstFcmp::Ueq, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprF32Ne:
    case kExprF64Ne: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstFcmp::create(Func, InstFcmp::Une, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprF32Le:
    case kExprF64Le: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstFcmp::create(Func, InstFcmp::Ule, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprF32Lt:
    case kExprF64Lt: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstFcmp::create(Func, InstFcmp::Ult, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprF32Ge:
    case kExprF64Ge: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstFcmp::create(Func, InstFcmp::Uge, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    case kExprF32Gt:
    case kExprF64Gt: {
      auto *TmpDest = makeVariable(IceType_i1);
      Control()->appendInst(
          InstFcmp::create(Func, InstFcmp::Ugt, TmpDest, Left, Right));
      BoolDest->setBoolSource(TmpDest);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, TmpDest));
      break;
    }
    default:
      LOG(out << "Unknown binop: " << WasmOpcodes::OpcodeName(Opcode) << "\n");
      llvm::report_fatal_error("Uncovered or invalid binop.");
      return OperandNode(nullptr);
    }
    LOG(out << Dest << "\n");
    return OperandNode(Dest);
  }
  Node Unop(wasm::WasmOpcode Opcode, Node Input) {
    LOG(out << "Unop(" << WasmOpcodes::OpcodeName(Opcode) << ", " << Input
            << ") = ");
    Variable *Dest = nullptr;
    switch (Opcode) {
    // TODO (eholk): merge these next two cases using getConstantInteger
    case kExprI32Eqz: {
      auto *BoolDest = makeVariable<BooleanVariable>(IceType_i32);
      Dest = BoolDest;
      auto *Tmp = makeVariable(IceType_i1);
      Control()->appendInst(InstIcmp::create(Func, InstIcmp::Eq, Tmp, Input,
                                             Ctx->getConstantInt32(0)));
      BoolDest->setBoolSource(Tmp);
      Control()->appendInst(InstCast::create(Func, InstCast::Zext, Dest, Tmp));
      break;
    }
    case kExprI64Eqz: {
      auto *BoolDest = makeVariable<BooleanVariable>(IceType_i32);
      Dest = BoolDest;
      auto *Tmp = makeVariable(IceType_i1);
      Control()->appendInst(InstIcmp::create(Func, InstIcmp::Eq, Tmp, Input,
                                             Ctx->getConstantInt64(0)));
      BoolDest->setBoolSource(Tmp);
      Control()->appendInst(InstCast::create(Func, InstCast::Zext, Dest, Tmp));
      break;
    }
    case kExprI32Ctz: {
      Dest = makeVariable(IceType_i32);
      const auto FnName = Ctx->getGlobalString("llvm.cttz.i32");
      bool BadInstrinsic = false;
      const auto *Info = Ctx->getIntrinsicsInfo().find(FnName, BadInstrinsic);
      assert(!BadInstrinsic);
      assert(Info);

      auto *Call = InstIntrinsic::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), Info->Info);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprF32Neg: {
      Dest = makeVariable(IceType_f32);
      Control()->appendInst(InstArithmetic::create(
          Func, InstArithmetic::Fsub, Dest, Ctx->getConstantFloat(0), Input));
      break;
    }
    case kExprF64Neg: {
      Dest = makeVariable(IceType_f64);
      Control()->appendInst(InstArithmetic::create(
          Func, InstArithmetic::Fsub, Dest, Ctx->getConstantDouble(0), Input));
      break;
    }
    case kExprF32Abs: {
      Dest = makeVariable(IceType_f32);
      const auto FnName = Ctx->getGlobalString("llvm.fabs.f32");
      bool BadInstrinsic = false;
      const auto *Info = Ctx->getIntrinsicsInfo().find(FnName, BadInstrinsic);
      assert(!BadInstrinsic);
      assert(Info);

      auto *Call = InstIntrinsic::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), Info->Info);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprF64Abs: {
      Dest = makeVariable(IceType_f64);
      const auto FnName = Ctx->getGlobalString("llvm.fabs.f64");
      bool BadInstrinsic = false;
      const auto *Info = Ctx->getIntrinsicsInfo().find(FnName, BadInstrinsic);
      assert(!BadInstrinsic);
      assert(Info);

      auto *Call = InstIntrinsic::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), Info->Info);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprF32Floor: {
      Dest = makeVariable(IceType_f64);
      const auto FnName = Ctx->getGlobalString("env$$floor_f");
      constexpr bool HasTailCall = false;

      auto *Call = InstCall::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), HasTailCall);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprF64Floor: {
      Dest = makeVariable(IceType_f64);
      const auto FnName = Ctx->getGlobalString("env$$floor_d");
      constexpr bool HasTailCall = false;

      auto *Call = InstCall::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), HasTailCall);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprF32Sqrt: {
      Dest = makeVariable(IceType_f32);
      const auto FnName = Ctx->getGlobalString("llvm.sqrt.f32");
      bool BadInstrinsic = false;
      const auto *Info = Ctx->getIntrinsicsInfo().find(FnName, BadInstrinsic);
      assert(!BadInstrinsic);
      assert(Info);

      auto *Call = InstIntrinsic::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), Info->Info);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprF64Sqrt: {
      Dest = makeVariable(IceType_f64);
      const auto FnName = Ctx->getGlobalString("llvm.sqrt.f64");
      bool BadInstrinsic = false;
      const auto *Info = Ctx->getIntrinsicsInfo().find(FnName, BadInstrinsic);
      assert(!BadInstrinsic);
      assert(Info);

      auto *Call = InstIntrinsic::create(
          Func, 1, Dest, Ctx->getConstantExternSym(FnName), Info->Info);
      Call->addArg(Input);
      Control()->appendInst(Call);
      break;
    }
    case kExprI64UConvertI32:
      Dest = makeVariable(IceType_i64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Zext, Dest, Input));
      break;
    case kExprI64SConvertI32:
      Dest = makeVariable(IceType_i64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Sext, Dest, Input));
      break;
    case kExprI32SConvertF32:
      Dest = makeVariable(IceType_i32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Fptosi, Dest, Input));
      break;
    case kExprI32UConvertF32:
      Dest = makeVariable(IceType_i32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Fptoui, Dest, Input));
      break;
    case kExprI32SConvertF64:
      Dest = makeVariable(IceType_i32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Fptosi, Dest, Input));
      break;
    case kExprI32UConvertF64:
      Dest = makeVariable(IceType_i32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Fptoui, Dest, Input));
      break;
    case kExprI32ReinterpretF32:
      Dest = makeVariable(IceType_i32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Bitcast, Dest, Input));
      break;
    case kExprI64ReinterpretF64:
      Dest = makeVariable(IceType_i64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Bitcast, Dest, Input));
      break;
    case kExprF64ReinterpretI64:
      Dest = makeVariable(IceType_f64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Bitcast, Dest, Input));
      break;
    case kExprI32ConvertI64:
      Dest = makeVariable(IceType_i32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Trunc, Dest, Input));
      break;
    case kExprF64SConvertI32:
      Dest = makeVariable(IceType_f64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Sitofp, Dest, Input));
      break;
    case kExprF64UConvertI32:
      Dest = makeVariable(IceType_f64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Uitofp, Dest, Input));
      break;
    case kExprF64ConvertF32:
      Dest = makeVariable(IceType_f64);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Fpext, Dest, Input));
      break;
    case kExprF32SConvertI32:
      Dest = makeVariable(IceType_f32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Sitofp, Dest, Input));
      break;
    case kExprF32UConvertI32:
      Dest = makeVariable(IceType_f32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Uitofp, Dest, Input));
      break;
    case kExprF32ReinterpretI32:
      Dest = makeVariable(IceType_f32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Bitcast, Dest, Input));
      break;
    case kExprF32ConvertF64:
      Dest = makeVariable(IceType_f32);
      Control()->appendInst(
          InstCast::create(Func, InstCast::Fptrunc, Dest, Input));
      break;
    default:
      LOG(out << "Unknown unop: " << WasmOpcodes::OpcodeName(Opcode) << "\n");
      llvm::report_fatal_error("Uncovered or invalid unop.");
      return OperandNode(nullptr);
    }
    LOG(out << Dest << "\n");
    return OperandNode(Dest);
  }
  uint32_t InputCount(CfgNode *Node) const { return Node->getInEdges().size(); }
  bool IsPhiWithMerge(Node Phi, Node Merge) const {
    LOG(out << "IsPhiWithMerge(" << Phi << ", " << Merge << ")"
            << "\n");
    if (Phi && Phi.isOperand()) {
      LOG(out << "  ...is operand"
              << "\n");
      if (getDefiningInst(Phi)) {
        LOG(out << "  ...has defining instruction"
                << "\n");
        LOG(out << getDefNode(Phi) << "\n");
        LOG(out << "  ..." << (getDefNode(Phi) == Merge) << "\n");
        return getDefNode(Phi) == Merge;
      }
    }
    return false;
  }
  void AppendToMerge(CfgNode *Merge, CfgNode *From) const {
    From->appendInst(InstBr::create(Func, Merge));
  }
  void AppendToPhi(Node Merge, Node Phi, Node From) {
    LOG(out << "AppendToPhi(" << Merge << ", " << Phi << ", " << From << ")"
            << "\n");
    auto *Inst = getDefiningInst(Phi);
    assert(Inst->getDest()->getType() == From.toOperand()->getType());
    Inst->addArgument(From, getDefNode(From));
  }

  //-----------------------------------------------------------------------
  // Operations that read and/or write {control} and {effect}.
  //-----------------------------------------------------------------------
  Node Branch(Node Cond, Node *TrueNode, Node *FalseNode) {
    // true_node and false_node appear to be out parameters.
    LOG(out << "Branch(" << Cond << ", ");

    // save control here because true_node appears to alias control.
    auto *Ctrl = Control();

    *TrueNode = OperandNode(Func->makeNode());
    *FalseNode = OperandNode(Func->makeNode());

    LOG(out << *TrueNode << ", " << *FalseNode << ")"
            << "\n");

    auto *CondBool = Cond.toOperand()->asBoolean();
    if (CondBool == nullptr) {
      CondBool = makeVariable(IceType_i1);
      Ctrl->appendInst(InstIcmp::create(Func, InstIcmp::Ne, CondBool, Cond,
                                        Ctx->getConstantInt32(0)));
    }

    Ctrl->appendInst(InstBr::create(Func, CondBool, *TrueNode, *FalseNode));
    return OperandNode(nullptr);
  }
  InstSwitch *CurrentSwitch = nullptr;
  CfgNode *SwitchNode = nullptr;
  SizeT SwitchIndex = 0;
  Node Switch(uint32_t Count, Node Key) {
    LOG(out << "Switch(" << Count << ", " << Key << ")\n");

    assert(!CurrentSwitch);

    auto *Default = Func->makeNode();
    // Count - 1 because the decoder counts the default label but Subzero does
    // not.
    CurrentSwitch = InstSwitch::create(Func, Count - 1, Key, Default);
    SwitchIndex = 0;
    SwitchNode = Control();
    // We don't actually append the switch to the CfgNode here because not all
    // the branches are ready.
    return Node(nullptr);
  }
  Node IfValue(int32_t Value, Node) {
    LOG(out << "IfValue(" << Value << ") [Index = " << SwitchIndex << "]\n");
    assert(CurrentSwitch);
    auto *Target = Func->makeNode();
    CurrentSwitch->addBranch(SwitchIndex++, Value, Target);
    return Node(Target);
  }
  Node IfDefault(Node) {
    LOG(out << "IfDefault(...) [Index = " << SwitchIndex << "]\n");
    assert(CurrentSwitch);
    assert(CurrentSwitch->getLabelDefault());
    // Now we append the switch, since this should be the last edge.
    assert(SwitchIndex == CurrentSwitch->getNumCases());
    SwitchNode->appendInst(CurrentSwitch);
    SwitchNode = nullptr;
    auto Default = Node(CurrentSwitch->getLabelDefault());
    CurrentSwitch = nullptr;
    return Default;
  }
  Node Return(uint32_t Count, Node *Vals) {
    assert(1 >= Count);
    LOG(out << "Return(");
    if (Count > 0)
      LOG(out << Vals[0]);
    LOG(out << ")"
            << "\n");
    auto *Instr =
        1 == Count ? InstRet::create(Func, Vals[0]) : InstRet::create(Func);
    Control()->appendInst(Instr);
    Control()->setHasReturn();
    LOG(out << Node(nullptr) << "\n");
    return OperandNode(nullptr);
  }
  Node ReturnVoid() {
    LOG(out << "ReturnVoid() = ");
    auto *Instr = InstRet::create(Func);
    Control()->appendInst(Instr);
    Control()->setHasReturn();
    LOG(out << Node(nullptr) << "\n");
    return OperandNode(nullptr);
  }
  Node Unreachable() {
    LOG(out << "Unreachable() = ");
    auto *Instr = InstUnreachable::create(Func);
    Control()->appendInst(Instr);
    LOG(out << Node(nullptr) << "\n");
    return OperandNode(nullptr);
  }

  Node CallDirect(uint32_t Index, Node *Args) {
    LOG(out << "CallDirect(" << Index << ")"
            << "\n");
    assert(Module->IsValidFunction(Index));
    const auto *Module = this->Module->module;
    assert(Module);
    const auto &Target = Module->functions[Index];
    const auto *Sig = Target.sig;
    assert(Sig);
    const auto NumArgs = Sig->parameter_count();
    LOG(out << "  number of args: " << NumArgs << "\n");

    const auto TargetName = getFunctionName(Module, Index);
    LOG(out << "  target name: " << TargetName << "\n");

    assert(Sig->return_count() <= 1);

    auto TargetOperand =
        Ctx->getConstantSym(0, Ctx->getGlobalString(TargetName));

    auto *Dest = Sig->return_count() > 0
                     ? makeVariable(toIceType(Sig->GetReturn()))
                     : nullptr;
    auto *Call = InstCall::create(Func, NumArgs, Dest, TargetOperand,
                                  false /* HasTailCall */);
    for (uint32_t i = 0; i < NumArgs; ++i) {
      // The builder reserves the first argument for the code object.
      LOG(out << "  args[" << i << "] = " << Args[i + 1] << "\n");
      Call->addArg(Args[i + 1]);
    }

    Control()->appendInst(Call);
    LOG(out << "Call Result = " << Node(Dest) << "\n");
    return OperandNode(Dest);
  }
  Node CallImport(uint32_t Index, Node *Args) {
    LOG(out << "CallImport(" << Index << ")"
            << "\n");
    const auto *Module = this->Module->module;
    assert(Module);
    const auto *Sig = this->Module->GetImportSignature(Index);
    assert(Sig);
    const auto NumArgs = Sig->parameter_count();
    LOG(out << "  number of args: " << NumArgs << "\n");

    const auto &Target = Module->import_table[Index];
    const auto ModuleName = toStdString(
        Module->GetName(Target.module_name_offset, Target.module_name_length));
    const auto FnName = toStdString(Module->GetName(
        Target.function_name_offset, Target.function_name_length));

    const auto TargetName = Ctx->getGlobalString(ModuleName + "$$" + FnName);
    LOG(out << "  target name: " << TargetName << "\n");

    assert(Sig->return_count() <= 1);

    auto TargetOperand = Ctx->getConstantExternSym(TargetName);

    auto *Dest = Sig->return_count() > 0
                     ? makeVariable(toIceType(Sig->GetReturn()))
                     : nullptr;
    constexpr bool NoTailCall = false;
    auto *Call =
        InstCall::create(Func, NumArgs, Dest, TargetOperand, NoTailCall);
    for (uint32_t i = 0; i < NumArgs; ++i) {
      // The builder reserves the first argument for the code object.
      LOG(out << "  args[" << i << "] = " << Args[i + 1] << "\n");
      assert(Args[i + 1].toOperand()->getType() == toIceType(Sig->GetParam(i)));
      Call->addArg(Args[i + 1]);
    }

    Control()->appendInst(Call);
    LOG(out << "Call Result = " << Node(Dest) << "\n");
    return OperandNode(Dest);
  }
  Node CallIndirect(uint32_t SigIndex, Node *Args) {
    LOG(out << "CallIndirect(" << SigIndex << ")\n");
    // TODO(eholk): Compile to something better than a switch.
    const auto *Module = this->Module->module;
    assert(Module);
    const auto &IndirectTable = Module->function_table;

    auto *Abort = getIndirectFailTarget();

    assert(Args[0].toOperand());

    auto *Switch = InstSwitch::create(Func, IndirectTable.size(),
                                      Args[0].toOperand(), Abort);
    assert(Abort);

    const bool HasReturn = Module->signatures[SigIndex]->return_count() != 0;
    const Ice::Type DestTy =
        HasReturn ? toIceType(Module->signatures[SigIndex]->GetReturn())
                  : IceType_void;

    auto *Dest = HasReturn ? makeVariable(DestTy) : nullptr;

    auto *ExitNode = Func->makeNode();
    auto *PhiInst =
        HasReturn ? InstPhi::create(Func, IndirectTable.size(), Dest) : nullptr;

    for (uint32_t Index = 0; Index < IndirectTable.size(); ++Index) {
      const auto &Target = Module->functions[IndirectTable[Index]];

      if (SigIndex == Target.sig_index) {
        auto *CallNode = Func->makeNode();
        auto *SavedControl = Control();
        *ControlPtr = OperandNode(CallNode);
        auto *Tmp = CallDirect(Target.func_index, Args).toOperand();
        *ControlPtr = OperandNode(SavedControl);
        if (PhiInst) {
          PhiInst->addArgument(Tmp, CallNode);
        }
        CallNode->appendInst(InstBr::create(Func, ExitNode));
        Switch->addBranch(Index, Index, CallNode);
      } else {
        Switch->addBranch(Index, Index, Abort);
      }
    }

    if (PhiInst) {
      ExitNode->appendInst(PhiInst);
    }

    Control()->appendInst(Switch);
    *ControlPtr = OperandNode(ExitNode);
    return OperandNode(Dest);
  }
  Node Invert(Node Node) {
    (void)Node;
    llvm::report_fatal_error("Invert");
  }

  //-----------------------------------------------------------------------
  // Operations that concern the linear memory.
  //-----------------------------------------------------------------------
  Node MemSize(uint32_t Offset) {
    (void)Offset;
    llvm::report_fatal_error("MemSize");
  }
  Node LoadGlobal(uint32_t Index) {
    (void)Index;
    llvm::report_fatal_error("LoadGlobal");
  }
  Node StoreGlobal(uint32_t Index, Node Val) {
    (void)Index;
    (void)Val;
    llvm::report_fatal_error("StoreGlobal");
  }

  Node LoadMem(wasm::LocalType Type, MachineType MemType, Node Index,
               uint32_t Offset) {
    LOG(out << "LoadMem." << toIceType(MemType) << "(" << Index << "[" << Offset
            << "]) = ");

    auto *RealAddr = sanitizeAddress(Index, Offset);

    auto *LoadResult = makeVariable(toIceType(MemType));
    Control()->appendInst(InstLoad::create(Func, LoadResult, RealAddr));

    // and cast, if needed
    Variable *Result = nullptr;
    if (toIceType(Type) != toIceType(MemType)) {
      auto DestType = toIceType(Type);
      Result = makeVariable(DestType);
      // TODO(eholk): handle signs correctly.
      if (isScalarIntegerType(DestType)) {
        if (MemType.IsSigned()) {
          Control()->appendInst(
              InstCast::create(Func, InstCast::Sext, Result, LoadResult));
        } else {
          Control()->appendInst(
              InstCast::create(Func, InstCast::Zext, Result, LoadResult));
        }
      } else if (isScalarFloatingType(DestType)) {
        Control()->appendInst(
            InstCast::create(Func, InstCast::Sitofp, Result, LoadResult));
      } else {
        llvm::report_fatal_error("Unsupported type for memory load");
      }
    } else {
      Result = LoadResult;
    }

    LOG(out << Result << "\n");
    return OperandNode(Result);
  }
  void StoreMem(MachineType Type, Node Index, uint32_t Offset, Node Val) {
    LOG(out << "StoreMem." << toIceType(Type) << "(" << Index << "[" << Offset
            << "] = " << Val << ")"
            << "\n");

    auto *RealAddr = sanitizeAddress(Index, Offset);

    // cast the value to the right type, if needed
    Operand *StoreVal = nullptr;
    if (toIceType(Type) != Val.toOperand()->getType()) {
      auto *LocalStoreVal = makeVariable(toIceType(Type));
      Control()->appendInst(
          InstCast::create(Func, InstCast::Trunc, LocalStoreVal, Val));
      StoreVal = LocalStoreVal;
    } else {
      StoreVal = Val;
    }

    // then store the memory
    Control()->appendInst(InstStore::create(Func, StoreVal, RealAddr));
  }

  static void PrintDebugName(OperandNode Node) {
    (void)Node;
    llvm::report_fatal_error("PrintDebugName");
  }

  CfgNode *Control() {
    return ControlPtr ? ControlPtr->toCfgNode() : Func->getEntryNode();
  }
  Node Effect() { return *EffectPtr; }

  void set_module(wasm::ModuleEnv *Module) { this->Module = Module; }

  void set_control_ptr(Node *Control) { this->ControlPtr = Control; }

  void set_effect_ptr(Node *Effect) { this->EffectPtr = Effect; }

private:
  wasm::ModuleEnv *Module;
  Node *ControlPtr;
  Node *EffectPtr;

  class Cfg *Func;
  GlobalContext *Ctx;

  CfgNode *BoundsFailTarget = nullptr;
  CfgNode *IndirectFailTarget = nullptr;

  SizeT NextArg = 0;

  CfgUnorderedMap<Operand *, InstPhi *> PhiMap;
  CfgUnorderedMap<Operand *, CfgNode *> DefNodeMap;

  Operand *WasmMemory = nullptr;

  InstPhi *getDefiningInst(Operand *Op) const {
    const auto &Iter = PhiMap.find(Op);
    if (Iter == PhiMap.end()) {
      return nullptr;
    }
    return Iter->second;
  }

  void setDefiningInst(Operand *Op, InstPhi *Phi) {
    LOG(out << "\n== setDefiningInst(" << Op << ", " << Phi << ") ==\n");
    PhiMap.emplace(Op, Phi);
  }

  template <typename T = Variable> T *makeVariable(Ice::Type Type) {
    return makeVariable<T>(Type, Control());
  }

  template <typename T = Variable>
  T *makeVariable(Ice::Type Type, CfgNode *DefNode) {
    auto *Var = Func->makeVariable<T>(Type);
    DefNodeMap.emplace(Var, DefNode);
    return Var;
  }

  CfgNode *getDefNode(Operand *Op) const {
    const auto &Iter = DefNodeMap.find(Op);
    if (Iter == DefNodeMap.end()) {
      return nullptr;
    }
    return Iter->second;
  }

  CfgNode *getBoundsFailTarget() {
    if (!BoundsFailTarget) {
      // TODO (eholk): Move this node to the end of the CFG, or even better,
      // have only one abort block for the whole module.
      BoundsFailTarget = Func->makeNode();
      BoundsFailTarget->appendInst(InstCall::create(
          Func, 0, nullptr,
          Ctx->getConstantExternSym(Ctx->getGlobalString("__Sz_bounds_fail")),
          false));
      BoundsFailTarget->appendInst(InstUnreachable::create(Func));
    }

    return BoundsFailTarget;
  }
  CfgNode *getIndirectFailTarget() {
    if (!IndirectFailTarget) {
      // TODO (eholk): Move this node to the end of the CFG, or even better,
      // have only one abort block for the whole module.
      IndirectFailTarget = Func->makeNode();
      IndirectFailTarget->appendInst(InstCall::create(
          Func, 0, nullptr,
          Ctx->getConstantExternSym(Ctx->getGlobalString("__Sz_indirect_fail")),
          false));
      IndirectFailTarget->appendInst(InstUnreachable::create(Func));
    }

    return IndirectFailTarget;
  }

  Operand *getWasmMemory() {
    assert(WasmMemory != nullptr);
    return WasmMemory;
  }

  Operand *sanitizeAddress(Operand *Base, uint32_t Offset) {
    SizeT MemSize = Module->module->min_mem_pages * WASM_PAGE_SIZE;

    bool ConstZeroBase = false;

    // first, add the index and the offset together.
    if (auto *ConstBase = llvm::dyn_cast<ConstantInteger32>(Base)) {
      uint32_t RealOffset = Offset + ConstBase->getValue();
      if (RealOffset >= MemSize) {
        // We've proven this will always be an out of bounds access, so insert
        // an unconditional trap.
        Control()->appendInst(InstUnreachable::create(Func));
        // It doesn't matter what we return here, so return something that will
        // allow the rest of code generation to happen.
        //
        // We might be tempted to just abort translation here, but out of bounds
        // memory access is a runtime trap, not a compile error.
        return Ctx->getConstantZero(getPointerType());
      }
      Base = Ctx->getConstantInt32(RealOffset);
      ConstZeroBase = (0 == RealOffset);
    } else if (0 != Offset) {
      auto *Addr = makeVariable(Ice::getPointerType());
      auto *OffsetConstant = Ctx->getConstantInt32(Offset);
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Add,
                                                   Addr, Base, OffsetConstant));

      Base = Addr;
    }

    // Do the bounds check if enabled
    if (getFlags().getWasmBoundsCheck() &&
        !llvm::isa<ConstantInteger32>(Base)) {
      // TODO (eholk): creating a new basic block on every memory access is
      // terrible (see https://goo.gl/Zj7DTr). Try adding a new instruction that
      // encapsulates this "abort if false" pattern.
      auto *CheckPassed = Func->makeNode();
      auto *CheckFailed = getBoundsFailTarget();

      auto *Check = makeVariable(IceType_i1);
      Control()->appendInst(InstIcmp::create(Func, InstIcmp::Ult, Check, Base,
                                             Ctx->getConstantInt32(MemSize)));
      Control()->appendInst(
          InstBr::create(Func, Check, CheckPassed, CheckFailed));

      *ControlPtr = OperandNode(CheckPassed);
    }

    Ice::Operand *RealAddr = nullptr;
    auto MemBase = getWasmMemory();
    if (!ConstZeroBase) {
      auto RealAddrV = Func->makeVariable(Ice::getPointerType());
      Control()->appendInst(InstArithmetic::create(Func, InstArithmetic::Add,
                                                   RealAddrV, Base, MemBase));

      RealAddr = RealAddrV;
    } else {
      RealAddr = MemBase;
    }
    return RealAddr;
  }

  template <typename F = std::function<void(Ostream &)>> void log(F Fn) const {
    if (BuildDefs::dump() && (getFlags().getVerbose() & IceV_Wasm)) {
      Fn(Ctx->getStrDump());
      Ctx->getStrDump().flush();
    }
  }
};

std::unique_ptr<Cfg> WasmTranslator::translateFunction(Zone *Zone,
                                                       FunctionBody &Body) {
  OstreamLocker L1(Ctx);
  auto Func = Cfg::create(Ctx, getNextSequenceNumber());
  TimerMarker T(TimerStack::TT_wasmGenIce, Func.get());
  Ice::CfgLocalAllocatorScope L2(Func.get());

  // TODO(eholk): parse the function signature...

  Func->setEntryNode(Func->makeNode());

  IceBuilder Builder(Func.get());
  SR_WasmDecoder<OperandNode, IceBuilder> Decoder(Zone, &Builder, Body);

  LOG(out << getFlags().getDefaultGlobalPrefix() << "\n");
  Decoder.Decode();

  // We don't always know where the incoming branches are in phi nodes, so this
  // function finds them.
  Func->fixPhiNodes();

  Func->computeInOutEdges();

  return Func;
}

constexpr SizeT InitialBufferSize = 16 << 10; // 16KB

WasmTranslator::WasmTranslator(GlobalContext *Ctx)
    : Translator(Ctx), Buffer(InitialBufferSize) {}

void WasmTranslator::translate(
    const std::string &IRFilename,
    std::unique_ptr<llvm::DataStreamer> InputStream) {
  TimerMarker T(TimerStack::TT_wasm, Ctx);

  LOG(out << "Initializing v8/wasm stuff..."
          << "\n");
  Zone Zone;
  ZoneScope _(&Zone);

  SizeT BytesRead = 0;
  while (true) {
    BytesRead +=
        InputStream->GetBytes(&Buffer[BytesRead], Buffer.size() - BytesRead);
    LOG(out << "Read " << BytesRead << " bytes"
            << "\n");
    if (BytesRead < Buffer.size())
      break;
    Buffer.resize(Buffer.size() * 2);
  }

  LOG(out << "Decoding module " << IRFilename << "\n");

  constexpr v8::internal::Isolate *NoIsolate = nullptr;
  auto Result = DecodeWasmModule(NoIsolate, &Zone, Buffer.data(),
                                 Buffer.data() + BytesRead, false, kWasmOrigin);

  auto Module = Result.val;

  LOG(out << "Module info:"
          << "\n");
  LOG(out << "  min_mem_pages:           " << Module->min_mem_pages << "\n");
  LOG(out << "  max_mem_pages:           " << Module->max_mem_pages << "\n");
  LOG(out << "  number of globals:       " << Module->globals.size() << "\n");
  LOG(out << "  number of signatures:    " << Module->signatures.size()
          << "\n");
  LOG(out << "  number of functions:     " << Module->functions.size() << "\n");
  LOG(out << "  number of data_segments: " << Module->data_segments.size()
          << "\n");
  LOG(out << "  function table size:     " << Module->function_table.size()
          << "\n");
  LOG(out << "  import table size:       " << Module->import_table.size()
          << "\n");
  LOG(out << "  export table size:       " << Module->export_table.size()
          << "\n");

  LOG(out << "\n"
          << "Data segment information:"
          << "\n");
  uint32_t Id = 0;
  for (const auto Seg : Module->data_segments) {
    LOG(out << Id << ":  (" << Seg.source_offset << ", " << Seg.source_size
            << ") => " << Seg.dest_addr);
    if (Seg.init) {
      LOG(out << " init\n");
    } else {
      LOG(out << "\n");
    }
    Id++;
  }

  LOG(out << "\n"
          << "Import information:"
          << "\n");
  for (const auto Import : Module->import_table) {
    auto ModuleName = toStdString(
        Module->GetName(Import.module_name_offset, Import.module_name_length));
    auto FnName = toStdString(Module->GetName(Import.function_name_offset,
                                              Import.function_name_length));
    LOG(out << "  " << Import.sig_index << ": " << ModuleName << "::" << FnName
            << "\n");
  }

  LOG(out << "\n"
          << "Export information:"
          << "\n");
  for (const auto Export : Module->export_table) {
    LOG(out << "  " << Export.func_index << ": "
            << toStdString(
                   Module->GetName(Export.name_offset, Export.name_length))
            << " (" << Export.name_offset << ", " << Export.name_length << ")");
    LOG(out << "\n");
  }

  LOG(out << "\n"
          << "Function information:"
          << "\n");
  for (const auto F : Module->functions) {
    LOG(out << "  " << F.func_index << ": "
            << toStdString(Module->GetName(F.name_offset, F.name_length))
            << " (" << F.name_offset << ", " << F.name_length << ")");
    if (F.exported)
      LOG(out << " export");
    if (F.external)
      LOG(out << " extern");
    LOG(out << "\n");
  }

  LOG(out << "\n"
          << "Indirect table:"
          << "\n");
  for (uint32_t F : Module->function_table) {
    LOG(out << "  " << F << ": " << getFunctionName(Module, F) << "\n");
  }

  ModuleEnv ModuleEnv;
  ModuleEnv.module = Module;

  FunctionBody Body;
  Body.module = &ModuleEnv;

  LOG(out << "Translating " << IRFilename << "\n");

  {
    unique_ptr<VariableDeclarationList> Globals =
        makeUnique<VariableDeclarationList>();

    // Global variables, etc go here.
    auto *WasmMemory = VariableDeclaration::createExternal(Globals.get());
    WasmMemory->setName(Ctx->getGlobalString("WASM_DATA_INIT"));

    // Fill in the segments
    SizeT WritePtr = 0;
    for (const auto Seg : Module->data_segments) {
      // fill in gaps with zero.
      if (Seg.dest_addr > WritePtr) {
        WasmMemory->addInitializer(VariableDeclaration::ZeroInitializer::create(
            Globals.get(), Seg.dest_addr - WritePtr));
        WritePtr = Seg.dest_addr;
      }

      // Add the data
      WasmMemory->addInitializer(VariableDeclaration::DataInitializer::create(
          Globals.get(),
          reinterpret_cast<const char *>(Module->module_start) +
              Seg.source_offset,
          Seg.source_size));

      WritePtr += Seg.source_size;
    }

    // Save the size of the initialized data in a global variable so the runtime
    // can use it to determine the initial heap break.
    auto *GlobalDataSize = VariableDeclaration::createExternal(Globals.get());
    GlobalDataSize->setName(Ctx->getGlobalString("WASM_DATA_SIZE"));
    GlobalDataSize->addInitializer(VariableDeclaration::DataInitializer::create(
        Globals.get(), reinterpret_cast<const char *>(&WritePtr),
        sizeof(WritePtr)));

    // Save the number of pages for the runtime
    auto *GlobalNumPages = VariableDeclaration::createExternal(Globals.get());
    GlobalNumPages->setName(Ctx->getGlobalString("WASM_NUM_PAGES"));
    GlobalNumPages->addInitializer(VariableDeclaration::DataInitializer::create(
        Globals.get(), reinterpret_cast<const char *>(&Module->min_mem_pages),
        sizeof(Module->min_mem_pages)));

    Globals->push_back(WasmMemory);
    Globals->push_back(GlobalDataSize);
    Globals->push_back(GlobalNumPages);

    lowerGlobals(std::move(Globals));
  }

  // Translate each function.
  for (const auto Fn : Module->functions) {
    const auto FnName = getFunctionName(Module, Fn.func_index);

    LOG(out << "  " << Fn.func_index << ": " << FnName << "...");

    Body.sig = Fn.sig;
    Body.base = Buffer.data();
    Body.start = Buffer.data() + Fn.code_start_offset;
    Body.end = Buffer.data() + Fn.code_end_offset;

    std::unique_ptr<Cfg> Func = nullptr;
    {
      TimerMarker T_func(getContext(), FnName);
      Func = translateFunction(&Zone, Body);
      Func->setFunctionName(Ctx->getGlobalString(FnName));
    }
    Ctx->optQueueBlockingPush(makeUnique<CfgOptWorkItem>(std::move(Func)));
    LOG(out << "done.\n");
  }

  return;
}

#endif // ALLOW_WASM
