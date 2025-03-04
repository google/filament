//===-- llvm/CodeGen/SelectionDAGISel.h - Common Base Class------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the SelectionDAGISel class, which is used as the common
// base class for SelectionDAG-based instruction selectors.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_SELECTIONDAGISEL_H
#define LLVM_CODEGEN_SELECTIONDAGISEL_H

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetSubtargetInfo.h"

namespace llvm {
  class FastISel;
  class SelectionDAGBuilder;
  class SDValue;
  class MachineRegisterInfo;
  class MachineBasicBlock;
  class MachineFunction;
  class MachineInstr;
  class TargetLowering;
  class TargetLibraryInfo;
  class FunctionLoweringInfo;
  class ScheduleHazardRecognizer;
  class GCFunctionInfo;
  class ScheduleDAGSDNodes;
  class LoadInst;

/// SelectionDAGISel - This is the common base class used for SelectionDAG-based
/// pattern-matching instruction selectors.
class SelectionDAGISel : public MachineFunctionPass {
public:
  TargetMachine &TM;
  const TargetLibraryInfo *LibInfo;
  FunctionLoweringInfo *FuncInfo;
  MachineFunction *MF;
  MachineRegisterInfo *RegInfo;
  SelectionDAG *CurDAG;
  SelectionDAGBuilder *SDB;
  AliasAnalysis *AA;
  GCFunctionInfo *GFI;
  CodeGenOpt::Level OptLevel;
  const TargetInstrInfo *TII;
  const TargetLowering *TLI;

  static char ID;

  explicit SelectionDAGISel(TargetMachine &tm,
                            CodeGenOpt::Level OL = CodeGenOpt::Default);
  ~SelectionDAGISel() override;

  const TargetLowering *getTargetLowering() const { return TLI; }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnMachineFunction(MachineFunction &MF) override;

  virtual void EmitFunctionEntryCode() {}

  /// PreprocessISelDAG - This hook allows targets to hack on the graph before
  /// instruction selection starts.
  virtual void PreprocessISelDAG() {}

  /// PostprocessISelDAG() - This hook allows the target to hack on the graph
  /// right after selection.
  virtual void PostprocessISelDAG() {}

  /// Select - Main hook targets implement to select a node.
  virtual SDNode *Select(SDNode *N) = 0;

  /// SelectInlineAsmMemoryOperand - Select the specified address as a target
  /// addressing mode, according to the specified constraint.  If this does
  /// not match or is not implemented, return true.  The resultant operands
  /// (which will appear in the machine instruction) should be added to the
  /// OutOps vector.
  virtual bool SelectInlineAsmMemoryOperand(const SDValue &Op,
                                            unsigned ConstraintID,
                                            std::vector<SDValue> &OutOps) {
    return true;
  }

  /// IsProfitableToFold - Returns true if it's profitable to fold the specific
  /// operand node N of U during instruction selection that starts at Root.
  virtual bool IsProfitableToFold(SDValue N, SDNode *U, SDNode *Root) const;

  /// IsLegalToFold - Returns true if the specific operand node N of
  /// U can be folded during instruction selection that starts at Root.
  /// FIXME: This is a static member function because the MSP430/X86
  /// targets, which uses it during isel.  This could become a proper member.
  static bool IsLegalToFold(SDValue N, SDNode *U, SDNode *Root,
                            CodeGenOpt::Level OptLevel,
                            bool IgnoreChains = false);

  // Opcodes used by the DAG state machine:
  enum BuiltinOpcodes {
    OPC_Scope,
    OPC_RecordNode,
    OPC_RecordChild0, OPC_RecordChild1, OPC_RecordChild2, OPC_RecordChild3,
    OPC_RecordChild4, OPC_RecordChild5, OPC_RecordChild6, OPC_RecordChild7,
    OPC_RecordMemRef,
    OPC_CaptureGlueInput,
    OPC_MoveChild,
    OPC_MoveParent,
    OPC_CheckSame,
    OPC_CheckChild0Same, OPC_CheckChild1Same,
    OPC_CheckChild2Same, OPC_CheckChild3Same,
    OPC_CheckPatternPredicate,
    OPC_CheckPredicate,
    OPC_CheckOpcode,
    OPC_SwitchOpcode,
    OPC_CheckType,
    OPC_SwitchType,
    OPC_CheckChild0Type, OPC_CheckChild1Type, OPC_CheckChild2Type,
    OPC_CheckChild3Type, OPC_CheckChild4Type, OPC_CheckChild5Type,
    OPC_CheckChild6Type, OPC_CheckChild7Type,
    OPC_CheckInteger,
    OPC_CheckChild0Integer, OPC_CheckChild1Integer, OPC_CheckChild2Integer,
    OPC_CheckChild3Integer, OPC_CheckChild4Integer,
    OPC_CheckCondCode,
    OPC_CheckValueType,
    OPC_CheckComplexPat,
    OPC_CheckAndImm, OPC_CheckOrImm,
    OPC_CheckFoldableChainNode,

    OPC_EmitInteger,
    OPC_EmitRegister,
    OPC_EmitRegister2,
    OPC_EmitConvertToTarget,
    OPC_EmitMergeInputChains,
    OPC_EmitMergeInputChains1_0,
    OPC_EmitMergeInputChains1_1,
    OPC_EmitCopyToReg,
    OPC_EmitNodeXForm,
    OPC_EmitNode,
    OPC_MorphNodeTo,
    OPC_MarkGlueResults,
    OPC_CompleteMatch
  };

  enum {
    OPFL_None       = 0,  // Node has no chain or glue input and isn't variadic.
    OPFL_Chain      = 1,     // Node has a chain input.
    OPFL_GlueInput  = 2,     // Node has a glue input.
    OPFL_GlueOutput = 4,     // Node has a glue output.
    OPFL_MemRefs    = 8,     // Node gets accumulated MemRefs.
    OPFL_Variadic0  = 1<<4,  // Node is variadic, root has 0 fixed inputs.
    OPFL_Variadic1  = 2<<4,  // Node is variadic, root has 1 fixed inputs.
    OPFL_Variadic2  = 3<<4,  // Node is variadic, root has 2 fixed inputs.
    OPFL_Variadic3  = 4<<4,  // Node is variadic, root has 3 fixed inputs.
    OPFL_Variadic4  = 5<<4,  // Node is variadic, root has 4 fixed inputs.
    OPFL_Variadic5  = 6<<4,  // Node is variadic, root has 5 fixed inputs.
    OPFL_Variadic6  = 7<<4,  // Node is variadic, root has 6 fixed inputs.

    OPFL_VariadicInfo = OPFL_Variadic6
  };

  /// getNumFixedFromVariadicInfo - Transform an EmitNode flags word into the
  /// number of fixed arity values that should be skipped when copying from the
  /// root.
  static inline int getNumFixedFromVariadicInfo(unsigned Flags) {
    return ((Flags&OPFL_VariadicInfo) >> 4)-1;
  }


protected:
  /// DAGSize - Size of DAG being instruction selected.
  ///
  unsigned DAGSize;

  /// ReplaceUses - replace all uses of the old node F with the use
  /// of the new node T.
  void ReplaceUses(SDValue F, SDValue T) {
    CurDAG->ReplaceAllUsesOfValueWith(F, T);
  }

  /// ReplaceUses - replace all uses of the old nodes F with the use
  /// of the new nodes T.
  void ReplaceUses(const SDValue *F, const SDValue *T, unsigned Num) {
    CurDAG->ReplaceAllUsesOfValuesWith(F, T, Num);
  }

  /// ReplaceUses - replace all uses of the old node F with the use
  /// of the new node T.
  void ReplaceUses(SDNode *F, SDNode *T) {
    CurDAG->ReplaceAllUsesWith(F, T);
  }


  /// SelectInlineAsmMemoryOperands - Calls to this are automatically generated
  /// by tblgen.  Others should not call it.
  void SelectInlineAsmMemoryOperands(std::vector<SDValue> &Ops, SDLoc DL);


public:
  // Calls to these predicates are generated by tblgen.
  bool CheckAndMask(SDValue LHS, ConstantSDNode *RHS,
                    int64_t DesiredMaskS) const;
  bool CheckOrMask(SDValue LHS, ConstantSDNode *RHS,
                    int64_t DesiredMaskS) const;


  /// CheckPatternPredicate - This function is generated by tblgen in the
  /// target.  It runs the specified pattern predicate and returns true if it
  /// succeeds or false if it fails.  The number is a private implementation
  /// detail to the code tblgen produces.
  virtual bool CheckPatternPredicate(unsigned PredNo) const {
    llvm_unreachable("Tblgen should generate the implementation of this!");
  }

  /// CheckNodePredicate - This function is generated by tblgen in the target.
  /// It runs node predicate number PredNo and returns true if it succeeds or
  /// false if it fails.  The number is a private implementation
  /// detail to the code tblgen produces.
  virtual bool CheckNodePredicate(SDNode *N, unsigned PredNo) const {
    llvm_unreachable("Tblgen should generate the implementation of this!");
  }

  virtual bool CheckComplexPattern(SDNode *Root, SDNode *Parent, SDValue N,
                                   unsigned PatternNo,
                        SmallVectorImpl<std::pair<SDValue, SDNode*> > &Result) {
    llvm_unreachable("Tblgen should generate the implementation of this!");
  }

  virtual SDValue RunSDNodeXForm(SDValue V, unsigned XFormNo) {
    llvm_unreachable("Tblgen should generate this!");
  }

  SDNode *SelectCodeCommon(SDNode *NodeToMatch,
                           const unsigned char *MatcherTable,
                           unsigned TableSize);

  /// \brief Return true if complex patterns for this target can mutate the
  /// DAG.
  virtual bool ComplexPatternFuncMutatesDAG() const {
    return false;
  }

private:

  // Calls to these functions are generated by tblgen.
  SDNode *Select_INLINEASM(SDNode *N);
  SDNode *Select_READ_REGISTER(SDNode *N);
  SDNode *Select_WRITE_REGISTER(SDNode *N);
  SDNode *Select_UNDEF(SDNode *N);
  void CannotYetSelect(SDNode *N);

private:
  void DoInstructionSelection();
  SDNode *MorphNode(SDNode *Node, unsigned TargetOpc, SDVTList VTs,
                    ArrayRef<SDValue> Ops, unsigned EmitNodeInfo);

  /// Prepares the landing pad to take incoming values or do other EH
  /// personality specific tasks. Returns true if the block should be
  /// instruction selected, false if no code should be emitted for it.
  bool PrepareEHLandingPad();

  /// \brief Perform instruction selection on all basic blocks in the function.
  void SelectAllBasicBlocks(const Function &Fn);

  /// \brief Perform instruction selection on a single basic block, for
  /// instructions between \p Begin and \p End.  \p HadTailCall will be set
  /// to true if a call in the block was translated as a tail call.
  void SelectBasicBlock(BasicBlock::const_iterator Begin,
                        BasicBlock::const_iterator End,
                        bool &HadTailCall);
  void FinishBasicBlock();

  void CodeGenAndEmitDAG();

  /// \brief Generate instructions for lowering the incoming arguments of the
  /// given function.
  void LowerArguments(const Function &F);

  void ComputeLiveOutVRegInfo();

  /// Create the scheduler. If a specific scheduler was specified
  /// via the SchedulerRegistry, use it, otherwise select the
  /// one preferred by the target.
  ///
  ScheduleDAGSDNodes *CreateScheduler();

  /// OpcodeOffset - This is a cache used to dispatch efficiently into isel
  /// state machines that start with a OPC_SwitchOpcode node.
  std::vector<unsigned> OpcodeOffset;

  void UpdateChainsAndGlue(SDNode *NodeToMatch, SDValue InputChain,
                           const SmallVectorImpl<SDNode*> &ChainNodesMatched,
                           SDValue InputGlue, const SmallVectorImpl<SDNode*> &F,
                           bool isMorphNodeTo);

};

}

#endif /* LLVM_CODEGEN_SELECTIONDAGISEL_H */
