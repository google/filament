//===---- LiveRangeEdit.h - Basic tools for split and spill -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The LiveRangeEdit class represents changes done to a virtual register when it
// is spilled or split.
//
// The parent register is never changed. Instead, a number of new virtual
// registers are created and added to the newRegs vector.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_LIVERANGEEDIT_H
#define LLVM_CODEGEN_LIVERANGEEDIT_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"

namespace llvm {

class AliasAnalysis;
class LiveIntervals;
class MachineBlockFrequencyInfo;
class MachineLoopInfo;
class VirtRegMap;

class LiveRangeEdit : private MachineRegisterInfo::Delegate {
public:
  /// Callback methods for LiveRangeEdit owners.
  class Delegate {
    virtual void anchor();
  public:
    /// Called immediately before erasing a dead machine instruction.
    virtual void LRE_WillEraseInstruction(MachineInstr *MI) {}

    /// Called when a virtual register is no longer used. Return false to defer
    /// its deletion from LiveIntervals.
    virtual bool LRE_CanEraseVirtReg(unsigned) { return true; }

    /// Called before shrinking the live range of a virtual register.
    virtual void LRE_WillShrinkVirtReg(unsigned) {}

    /// Called after cloning a virtual register.
    /// This is used for new registers representing connected components of Old.
    virtual void LRE_DidCloneVirtReg(unsigned New, unsigned Old) {}

    virtual ~Delegate() {}
  };

private:
  LiveInterval *Parent;
  SmallVectorImpl<unsigned> &NewRegs;
  MachineRegisterInfo &MRI;
  LiveIntervals &LIS;
  VirtRegMap *VRM;
  const TargetInstrInfo &TII;
  Delegate *const TheDelegate;

  /// FirstNew - Index of the first register added to NewRegs.
  const unsigned FirstNew;

  /// ScannedRemattable - true when remattable values have been identified.
  bool ScannedRemattable;

  /// Remattable - Values defined by remattable instructions as identified by
  /// tii.isTriviallyReMaterializable().
  SmallPtrSet<const VNInfo*,4> Remattable;

  /// Rematted - Values that were actually rematted, and so need to have their
  /// live range trimmed or entirely removed.
  SmallPtrSet<const VNInfo*,4> Rematted;

  /// scanRemattable - Identify the Parent values that may rematerialize.
  void scanRemattable(AliasAnalysis *aa);

  /// allUsesAvailableAt - Return true if all registers used by OrigMI at
  /// OrigIdx are also available with the same value at UseIdx.
  bool allUsesAvailableAt(const MachineInstr *OrigMI, SlotIndex OrigIdx,
                          SlotIndex UseIdx) const;

  /// foldAsLoad - If LI has a single use and a single def that can be folded as
  /// a load, eliminate the register by folding the def into the use.
  bool foldAsLoad(LiveInterval *LI, SmallVectorImpl<MachineInstr*> &Dead);

  typedef SetVector<LiveInterval*,
                    SmallVector<LiveInterval*, 8>,
                    SmallPtrSet<LiveInterval*, 8> > ToShrinkSet;
  /// Helper for eliminateDeadDefs.
  void eliminateDeadDef(MachineInstr *MI, ToShrinkSet &ToShrink);

  /// MachineRegisterInfo callback to notify when new virtual
  /// registers are created.
  void MRI_NoteNewVirtualRegister(unsigned VReg) override;

  /// \brief Check if MachineOperand \p MO is a last use/kill either in the
  /// main live range of \p LI or in one of the matching subregister ranges.
  bool useIsKill(const LiveInterval &LI, const MachineOperand &MO) const;

public:
  /// Create a LiveRangeEdit for breaking down parent into smaller pieces.
  /// @param parent The register being spilled or split.
  /// @param newRegs List to receive any new registers created. This needn't be
  ///                empty initially, any existing registers are ignored.
  /// @param MF The MachineFunction the live range edit is taking place in.
  /// @param lis The collection of all live intervals in this function.
  /// @param vrm Map of virtual registers to physical registers for this
  ///            function.  If NULL, no virtual register map updates will
  ///            be done.  This could be the case if called before Regalloc.
  LiveRangeEdit(LiveInterval *parent, SmallVectorImpl<unsigned> &newRegs,
                MachineFunction &MF, LiveIntervals &lis, VirtRegMap *vrm,
                Delegate *delegate = nullptr)
      : Parent(parent), NewRegs(newRegs), MRI(MF.getRegInfo()), LIS(lis),
        VRM(vrm), TII(*MF.getSubtarget().getInstrInfo()),
        TheDelegate(delegate), FirstNew(newRegs.size()),
        ScannedRemattable(false) {
    MRI.setDelegate(this);
  }

  ~LiveRangeEdit() override { MRI.resetDelegate(this); }

  LiveInterval &getParent() const {
   assert(Parent && "No parent LiveInterval");
   return *Parent;
  }
  unsigned getReg() const { return getParent().reg; }

  /// Iterator for accessing the new registers added by this edit.
  typedef SmallVectorImpl<unsigned>::const_iterator iterator;
  iterator begin() const { return NewRegs.begin()+FirstNew; }
  iterator end() const { return NewRegs.end(); }
  unsigned size() const { return NewRegs.size()-FirstNew; }
  bool empty() const { return size() == 0; }
  unsigned get(unsigned idx) const { return NewRegs[idx+FirstNew]; }

  ArrayRef<unsigned> regs() const {
    return makeArrayRef(NewRegs).slice(FirstNew);
  }

  /// createEmptyIntervalFrom - Create a new empty interval based on OldReg.
  LiveInterval &createEmptyIntervalFrom(unsigned OldReg);

  /// createFrom - Create a new virtual register based on OldReg.
  unsigned createFrom(unsigned OldReg);

  /// create - Create a new register with the same class and original slot as
  /// parent.
  LiveInterval &createEmptyInterval() {
    return createEmptyIntervalFrom(getReg());
  }

  unsigned create() {
    return createFrom(getReg());
  }

  /// anyRematerializable - Return true if any parent values may be
  /// rematerializable.
  /// This function must be called before any rematerialization is attempted.
  bool anyRematerializable(AliasAnalysis*);

  /// checkRematerializable - Manually add VNI to the list of rematerializable
  /// values if DefMI may be rematerializable.
  bool checkRematerializable(VNInfo *VNI, const MachineInstr *DefMI,
                             AliasAnalysis*);

  /// Remat - Information needed to rematerialize at a specific location.
  struct Remat {
    VNInfo *ParentVNI;      // parent_'s value at the remat location.
    MachineInstr *OrigMI;   // Instruction defining ParentVNI.
    explicit Remat(VNInfo *ParentVNI) : ParentVNI(ParentVNI), OrigMI(nullptr) {}
  };

  /// canRematerializeAt - Determine if ParentVNI can be rematerialized at
  /// UseIdx. It is assumed that parent_.getVNINfoAt(UseIdx) == ParentVNI.
  /// When cheapAsAMove is set, only cheap remats are allowed.
  bool canRematerializeAt(Remat &RM,
                          SlotIndex UseIdx,
                          bool cheapAsAMove);

  /// rematerializeAt - Rematerialize RM.ParentVNI into DestReg by inserting an
  /// instruction into MBB before MI. The new instruction is mapped, but
  /// liveness is not updated.
  /// Return the SlotIndex of the new instruction.
  SlotIndex rematerializeAt(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            unsigned DestReg,
                            const Remat &RM,
                            const TargetRegisterInfo&,
                            bool Late = false);

  /// markRematerialized - explicitly mark a value as rematerialized after doing
  /// it manually.
  void markRematerialized(const VNInfo *ParentVNI) {
    Rematted.insert(ParentVNI);
  }

  /// didRematerialize - Return true if ParentVNI was rematerialized anywhere.
  bool didRematerialize(const VNInfo *ParentVNI) const {
    return Rematted.count(ParentVNI);
  }

  /// eraseVirtReg - Notify the delegate that Reg is no longer in use, and try
  /// to erase it from LIS.
  void eraseVirtReg(unsigned Reg);

  /// eliminateDeadDefs - Try to delete machine instructions that are now dead
  /// (allDefsAreDead returns true). This may cause live intervals to be trimmed
  /// and further dead efs to be eliminated.
  /// RegsBeingSpilled lists registers currently being spilled by the register
  /// allocator.  These registers should not be split into new intervals
  /// as currently those new intervals are not guaranteed to spill.
  void eliminateDeadDefs(SmallVectorImpl<MachineInstr*> &Dead,
                         ArrayRef<unsigned> RegsBeingSpilled = None);

  /// calculateRegClassAndHint - Recompute register class and hint for each new
  /// register.
  void calculateRegClassAndHint(MachineFunction&,
                                const MachineLoopInfo&,
                                const MachineBlockFrequencyInfo&);
};

}

#endif
