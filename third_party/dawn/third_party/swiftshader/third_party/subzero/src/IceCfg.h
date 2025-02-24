//===- subzero/src/IceCfg.h - Control flow graph ----------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Cfg class, which represents the control flow graph and
/// the overall per-function compilation context.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECFG_H
#define SUBZERO_SRC_ICECFG_H

#include "IceAssembler.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceGlobalContext.h"
#include "IceLoopAnalyzer.h"
#include "IceStringPool.h"
#include "IceTypes.h"

namespace Ice {

class Cfg {
  Cfg() = delete;
  Cfg(const Cfg &) = delete;
  Cfg &operator=(const Cfg &) = delete;

  std::unique_ptr<ArenaAllocator> Allocator;

public:
  ~Cfg();

  static std::unique_ptr<Cfg> create(GlobalContext *Ctx,
                                     uint32_t SequenceNumber) {
    return std::unique_ptr<Cfg>(new Cfg(Ctx, SequenceNumber));
  }

  GlobalContext *getContext() const { return Ctx; }
  uint32_t getSequenceNumber() const { return SequenceNumber; }
  OptLevel getOptLevel() const { return OptimizationLevel; }

  static constexpr VerboseMask defaultVerboseMask() {
    return (IceV_NO_PER_PASS_DUMP_BEYOND << 1) - 1;
  }
  /// Returns true if any of the specified options in the verbose mask are set.
  /// If the argument is omitted, it checks if any verbose options at all are
  /// set.
  bool isVerbose(VerboseMask Mask = defaultVerboseMask()) const {
    return VMask & Mask;
  }
  void setVerbose(VerboseMask Mask) { VMask = Mask; }

  /// \name Manage the name and return type of the function being translated.
  /// @{
  void setFunctionName(GlobalString Name) { FunctionName = Name; }
  GlobalString getFunctionName() const { return FunctionName; }
  std::string getFunctionNameAndSize() const;
  void setReturnType(Type Ty) { ReturnType = Ty; }
  Type getReturnType() const { return ReturnType; }
  /// @}

  /// \name Manage the "internal" attribute of the function.
  /// @{
  void setInternal(bool Internal) { IsInternalLinkage = Internal; }
  bool getInternal() const { return IsInternalLinkage; }
  /// @}

  /// \name Manage errors.
  /// @{

  /// Translation error flagging. If support for some construct is known to be
  /// missing, instead of an assertion failure, setError() should be called and
  /// the error should be propagated back up. This way, we can gracefully fail
  /// to translate and let a fallback translator handle the function.
  void setError(const std::string &Message);
  bool hasError() const { return HasError; }
  std::string getError() const { return ErrorMessage; }
  /// @}

  /// \name Manage nodes (a.k.a. basic blocks, CfgNodes).
  /// @{
  void setEntryNode(CfgNode *EntryNode) { Entry = EntryNode; }
  CfgNode *getEntryNode() const { return Entry; }
  /// Create a node and append it to the end of the linearized list. The loop
  /// nest depth of the new node may not be valid if it is created after
  /// computeLoopNestDepth.
  CfgNode *makeNode();
  SizeT getNumNodes() const { return Nodes.size(); }
  const NodeList &getNodes() const { return Nodes; }
  /// Swap nodes of Cfg with given list of nodes.  The number of nodes must
  /// remain unchanged.
  void swapNodes(NodeList &NewNodes);
  /// @}

  /// String pool for CfgNode::Name values.
  StringPool *getNodeStrings() const { return NodeStrings.get(); }
  /// String pool for Variable::Name values.
  StringPool *getVarStrings() const { return VarStrings.get(); }

  /// \name Manage instruction numbering.
  /// @{
  InstNumberT newInstNumber() { return NextInstNumber++; }
  InstNumberT getNextInstNumber() const { return NextInstNumber; }
  /// @}

  /// \name Manage Variables.
  /// @{

  /// Create a new Variable with a particular type and an optional name. The
  /// Node argument is the node where the variable is defined.
  // TODO(jpp): untemplate this with separate methods: makeVariable and
  // makeStackVariable.
  template <typename T = Variable> T *makeVariable(Type Ty) {
    SizeT Index = Variables.size();
    auto *Var = T::create(this, Ty, Index);
    Variables.push_back(Var);
    return Var;
  }
  SizeT getNumVariables() const { return Variables.size(); }
  const VarList &getVariables() const { return Variables; }
  /// @}

  /// \name Manage arguments to the function.
  /// @{
  void addArg(Variable *Arg);
  const VarList &getArgs() const { return Args; }
  VarList &getArgs() { return Args; }
  void addImplicitArg(Variable *Arg);
  const VarList &getImplicitArgs() const { return ImplicitArgs; }
  /// @}

  /// \name Manage the jump tables.
  /// @{
  void addJumpTable(InstJumpTable *JumpTable) {
    JumpTables.emplace_back(JumpTable);
  }
  /// @}

  /// \name Manage the Globals used by this function.
  /// @{
  std::unique_ptr<VariableDeclarationList> getGlobalInits() {
    return std::move(GlobalInits);
  }
  void addGlobal(VariableDeclaration *Global) {
    if (GlobalInits == nullptr) {
      GlobalInits.reset(new VariableDeclarationList);
    }
    GlobalInits->push_back(Global);
  }
  VariableDeclarationList *getGlobalPool() {
    if (GlobalInits == nullptr) {
      GlobalInits.reset(new VariableDeclarationList);
    }
    return GlobalInits.get();
  }
  /// @}

  /// \name Miscellaneous accessors.
  /// @{
  TargetLowering *getTarget() const { return Target.get(); }
  VariablesMetadata *getVMetadata() const { return VMetadata.get(); }
  Liveness *getLiveness() const { return Live.get(); }
  template <typename T = Assembler> T *getAssembler() const {
    return llvm::dyn_cast<T>(TargetAssembler.get());
  }
  std::unique_ptr<Assembler> releaseAssembler() {
    return std::move(TargetAssembler);
  }
  bool hasComputedFrame() const;
  bool getFocusedTiming() const { return FocusedTiming; }
  void setFocusedTiming() { FocusedTiming = true; }
  /// @}

  /// Passes over the CFG.
  void translate();
  /// After the CFG is fully constructed, iterate over the nodes and compute the
  /// predecessor and successor edges, in the form of CfgNode::InEdges[] and
  /// CfgNode::OutEdges[].
  void computeInOutEdges();
  /// Renumber the non-deleted instructions in the Cfg.  This needs to be done
  /// in preparation for live range analysis.  The instruction numbers in a
  /// block must be monotonically increasing.  The range of instruction numbers
  /// in a block, from lowest to highest, must not overlap with the range of any
  /// other block.
  ///
  /// Also, if the configuration specifies to do so, remove/unlink all deleted
  /// instructions from the Cfg, to speed up later passes over the instructions.
  void renumberInstructions();
  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void advancedPhiLowering();
  void reorderNodes();
  void localCSE(bool AssumeSSA);
  void floatConstantCSE();
  void shortCircuitJumps();
  void loopInvariantCodeMotion();

  /// Scan allocas to determine whether we need to use a frame pointer.
  /// If SortAndCombine == true, merge all the fixed-size allocas in the
  /// entry block and emit stack or frame pointer-relative addressing.
  void processAllocas(bool SortAndCombine);
  void doAddressOpt();
  /// Find clusters of insertelement/extractelement instructions that can be
  /// replaced by a shufflevector instruction.
  void materializeVectorShuffles();
  void doArgLowering();
  void genCode();
  void genFrame();
  void generateLoopInfo();
  void livenessLightweight();
  void liveness(LivenessMode Mode);
  bool validateLiveness() const;
  void contractEmptyNodes();
  void doBranchOpt();

  /// \name  Manage the CurrentNode field.
  /// CurrentNode is used for validating the Variable::DefNode field during
  /// dumping/emitting.
  /// @{
  void setCurrentNode(const CfgNode *Node) { CurrentNode = Node; }
  void resetCurrentNode() { setCurrentNode(nullptr); }
  const CfgNode *getCurrentNode() const { return CurrentNode; }
  /// @}

  /// Get the total amount of memory held by the per-Cfg allocator.
  size_t getTotalMemoryMB() const;

  /// Get the current memory usage due to liveness data structures.
  size_t getLivenessMemoryMB() const;

  void emit();
  void emitIAS();
  static void emitTextHeader(GlobalString Name, GlobalContext *Ctx,
                             const Assembler *Asm);
  void dump(const char *Message = "");

  /// Allocate data of type T using the per-Cfg allocator.
  template <typename T> T *allocate() { return Allocator->Allocate<T>(); }

  /// Allocate an array of data of type T using the per-Cfg allocator.
  template <typename T> T *allocateArrayOf(size_t NumElems) {
    return Allocator->Allocate<T>(NumElems);
  }

  /// Deallocate data that was allocated via allocate<T>().
  template <typename T> void deallocate(T *Object) {
    Allocator->Deallocate(Object);
  }

  /// Deallocate data that was allocated via allocateArrayOf<T>().
  template <typename T> void deallocateArrayOf(T *Array) {
    Allocator->Deallocate(Array);
  }

  /// Update Phi labels with InEdges.
  ///
  /// The WASM translator cannot always determine the right incoming edge for a
  /// value due to the CFG being built incrementally. The fixPhiNodes pass fills
  /// in the correct information once everything is known.
  void fixPhiNodes();

  void setStackSizeLimit(uint32_t Limit) { StackSizeLimit = Limit; }
  uint32_t getStackSizeLimit() const { return StackSizeLimit; }

private:
  friend class CfgAllocatorTraits; // for Allocator access.

  Cfg(GlobalContext *Ctx, uint32_t SequenceNumber);

  void createNodeNameDeclaration(const std::string &NodeAsmName);
  void
  createBlockProfilingInfoDeclaration(const std::string &NodeAsmName,
                                      VariableDeclaration *NodeNameDeclaration);

  /// Iterate through the registered jump tables and emit them.
  void emitJumpTables();

  enum AllocaBaseVariableType {
    BVT_StackPointer,
    BVT_FramePointer,
    BVT_UserPointer
  };
  void sortAndCombineAllocas(CfgVector<InstAlloca *> &Allocas,
                             uint32_t CombinedAlignment, InstList &Insts,
                             AllocaBaseVariableType BaseVariableType);
  void findRematerializable();
  CfgVector<Inst *>
  findLoopInvariantInstructions(const CfgUnorderedSet<SizeT> &Body);

  static ArenaAllocator *createAllocator();

  GlobalContext *Ctx;
  uint32_t SequenceNumber; /// output order for emission
  OptLevel OptimizationLevel = Opt_m1;
  VerboseMask VMask;
  GlobalString FunctionName;
  Type ReturnType = IceType_void;
  bool IsInternalLinkage = false;
  bool HasError = false;
  bool FocusedTiming = false;
  std::string ErrorMessage = "";
  CfgNode *Entry = nullptr; /// entry basic block
  NodeList Nodes;           /// linearized node list; Entry should be first
  InstNumberT NextInstNumber;
  VarList Variables;
  VarList Args;         /// subset of Variables, in argument order
  VarList ImplicitArgs; /// subset of Variables
  // Separate string pools for CfgNode and Variable names, due to a combination
  // of the uniqueness requirement, and assumptions in lit tests.
  std::unique_ptr<StringPool> NodeStrings;
  std::unique_ptr<StringPool> VarStrings;
  std::unique_ptr<Liveness> Live;
  std::unique_ptr<TargetLowering> Target;
  std::unique_ptr<VariablesMetadata> VMetadata;
  std::unique_ptr<Assembler> TargetAssembler;
  /// Globals required by this CFG.
  std::unique_ptr<VariableDeclarationList> GlobalInits;
  CfgVector<InstJumpTable *> JumpTables;
  /// CurrentNode is maintained during dumping/emitting just for validating
  /// Variable::DefNode. Normally, a traversal over CfgNodes maintains this, but
  /// before global operations like register allocation, resetCurrentNode()
  /// should be called to avoid spurious validation failures.
  const CfgNode *CurrentNode = nullptr;
  CfgVector<Loop> LoopInfo;
  uint32_t StackSizeLimit = 1 * 1024 * 1024; // 1 MiB

public:
  static void TlsInit() { CfgAllocatorTraits::init(); }
};

template <> Variable *Cfg::makeVariable<Variable>(Type Ty);

struct NodeStringPoolTraits {
  using OwnerType = Cfg;
  static StringPool *getStrings(const OwnerType *PoolOwner) {
    return PoolOwner->getNodeStrings();
  }
};
using NodeString = StringID<NodeStringPoolTraits>;

struct VariableStringPoolTraits {
  using OwnerType = Cfg;
  static StringPool *getStrings(const OwnerType *PoolOwner) {
    return PoolOwner->getVarStrings();
  }
};
using VariableString = StringID<VariableStringPoolTraits>;

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECFG_H
