//===-- RuntimeDyldMachOAArch64.h -- MachO/AArch64 specific code. -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_TARGETS_RUNTIMEDYLDMACHOAARCH64_H
#define LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_TARGETS_RUNTIMEDYLDMACHOAARCH64_H

#include "../RuntimeDyldMachO.h"
#include "llvm/Support/Endian.h"

#define DEBUG_TYPE "dyld"

namespace llvm {

class RuntimeDyldMachOAArch64
    : public RuntimeDyldMachOCRTPBase<RuntimeDyldMachOAArch64> {
public:

  typedef uint64_t TargetPtrT;

  RuntimeDyldMachOAArch64(RuntimeDyld::MemoryManager &MM,
                          RuntimeDyld::SymbolResolver &Resolver)
      : RuntimeDyldMachOCRTPBase(MM, Resolver) {}

  unsigned getMaxStubSize() override { return 8; }

  unsigned getStubAlignment() override { return 8; }

  /// Extract the addend encoded in the instruction / memory location.
  int64_t decodeAddend(const RelocationEntry &RE) const {
    const SectionEntry &Section = Sections[RE.SectionID];
    uint8_t *LocalAddress = Section.Address + RE.Offset;
    unsigned NumBytes = 1 << RE.Size;
    int64_t Addend = 0;
    // Verify that the relocation has the correct size and alignment.
    switch (RE.RelType) {
    default:
      llvm_unreachable("Unsupported relocation type!");
    case MachO::ARM64_RELOC_UNSIGNED:
      assert((NumBytes == 4 || NumBytes == 8) && "Invalid relocation size.");
      break;
    case MachO::ARM64_RELOC_BRANCH26:
    case MachO::ARM64_RELOC_PAGE21:
    case MachO::ARM64_RELOC_PAGEOFF12:
    case MachO::ARM64_RELOC_GOT_LOAD_PAGE21:
    case MachO::ARM64_RELOC_GOT_LOAD_PAGEOFF12:
      assert(NumBytes == 4 && "Invalid relocation size.");
      assert((((uintptr_t)LocalAddress & 0x3) == 0) &&
             "Instruction address is not aligned to 4 bytes.");
      break;
    }

    switch (RE.RelType) {
    default:
      llvm_unreachable("Unsupported relocation type!");
    case MachO::ARM64_RELOC_UNSIGNED:
      // This could be an unaligned memory location.
      if (NumBytes == 4)
        Addend = *reinterpret_cast<support::ulittle32_t *>(LocalAddress);
      else
        Addend = *reinterpret_cast<support::ulittle64_t *>(LocalAddress);
      break;
    case MachO::ARM64_RELOC_BRANCH26: {
      // Verify that the relocation points to the expected branch instruction.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      assert((*p & 0xFC000000) == 0x14000000 && "Expected branch instruction.");

      // Get the 26 bit addend encoded in the branch instruction and sign-extend
      // to 64 bit. The lower 2 bits are always zeros and are therefore implicit
      // (<< 2).
      Addend = (*p & 0x03FFFFFF) << 2;
      Addend = SignExtend64(Addend, 28);
      break;
    }
    case MachO::ARM64_RELOC_GOT_LOAD_PAGE21:
    case MachO::ARM64_RELOC_PAGE21: {
      // Verify that the relocation points to the expected adrp instruction.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      assert((*p & 0x9F000000) == 0x90000000 && "Expected adrp instruction.");

      // Get the 21 bit addend encoded in the adrp instruction and sign-extend
      // to 64 bit. The lower 12 bits (4096 byte page) are always zeros and are
      // therefore implicit (<< 12).
      Addend = ((*p & 0x60000000) >> 29) | ((*p & 0x01FFFFE0) >> 3) << 12;
      Addend = SignExtend64(Addend, 33);
      break;
    }
    case MachO::ARM64_RELOC_GOT_LOAD_PAGEOFF12: {
      // Verify that the relocation points to one of the expected load / store
      // instructions.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      (void)p;
      assert((*p & 0x3B000000) == 0x39000000 &&
             "Only expected load / store instructions.");
    } // fall-through
    case MachO::ARM64_RELOC_PAGEOFF12: {
      // Verify that the relocation points to one of the expected load / store
      // or add / sub instructions.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      assert((((*p & 0x3B000000) == 0x39000000) ||
              ((*p & 0x11C00000) == 0x11000000)   ) &&
             "Expected load / store  or add/sub instruction.");

      // Get the 12 bit addend encoded in the instruction.
      Addend = (*p & 0x003FFC00) >> 10;

      // Check which instruction we are decoding to obtain the implicit shift
      // factor of the instruction.
      int ImplicitShift = 0;
      if ((*p & 0x3B000000) == 0x39000000) { // << load / store
        // For load / store instructions the size is encoded in bits 31:30.
        ImplicitShift = ((*p >> 30) & 0x3);
        if (ImplicitShift == 0) {
          // Check if this a vector op to get the correct shift value.
          if ((*p & 0x04800000) == 0x04800000)
            ImplicitShift = 4;
        }
      }
      // Compensate for implicit shift.
      Addend <<= ImplicitShift;
      break;
    }
    }
    return Addend;
  }

  /// Extract the addend encoded in the instruction.
  void encodeAddend(uint8_t *LocalAddress, unsigned NumBytes,
                    MachO::RelocationInfoType RelType, int64_t Addend) const {
    // Verify that the relocation has the correct alignment.
    switch (RelType) {
    default:
      llvm_unreachable("Unsupported relocation type!");
    case MachO::ARM64_RELOC_UNSIGNED:
      assert((NumBytes == 4 || NumBytes == 8) && "Invalid relocation size.");
      break;
    case MachO::ARM64_RELOC_BRANCH26:
    case MachO::ARM64_RELOC_PAGE21:
    case MachO::ARM64_RELOC_PAGEOFF12:
    case MachO::ARM64_RELOC_GOT_LOAD_PAGE21:
    case MachO::ARM64_RELOC_GOT_LOAD_PAGEOFF12:
      assert(NumBytes == 4 && "Invalid relocation size.");
      assert((((uintptr_t)LocalAddress & 0x3) == 0) &&
             "Instruction address is not aligned to 4 bytes.");
      break;
    }

    switch (RelType) {
    default:
      llvm_unreachable("Unsupported relocation type!");
    case MachO::ARM64_RELOC_UNSIGNED:
      // This could be an unaligned memory location.
      if (NumBytes == 4)
        *reinterpret_cast<support::ulittle32_t *>(LocalAddress) = Addend;
      else
        *reinterpret_cast<support::ulittle64_t *>(LocalAddress) = Addend;
      break;
    case MachO::ARM64_RELOC_BRANCH26: {
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      // Verify that the relocation points to the expected branch instruction.
      assert((*p & 0xFC000000) == 0x14000000 && "Expected branch instruction.");

      // Verify addend value.
      assert((Addend & 0x3) == 0 && "Branch target is not aligned");
      assert(isInt<28>(Addend) && "Branch target is out of range.");

      // Encode the addend as 26 bit immediate in the branch instruction.
      *p = (*p & 0xFC000000) | ((uint32_t)(Addend >> 2) & 0x03FFFFFF);
      break;
    }
    case MachO::ARM64_RELOC_GOT_LOAD_PAGE21:
    case MachO::ARM64_RELOC_PAGE21: {
      // Verify that the relocation points to the expected adrp instruction.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      assert((*p & 0x9F000000) == 0x90000000 && "Expected adrp instruction.");

      // Check that the addend fits into 21 bits (+ 12 lower bits).
      assert((Addend & 0xFFF) == 0 && "ADRP target is not page aligned.");
      assert(isInt<33>(Addend) && "Invalid page reloc value.");

      // Encode the addend into the instruction.
      uint32_t ImmLoValue = ((uint64_t)Addend << 17) & 0x60000000;
      uint32_t ImmHiValue = ((uint64_t)Addend >> 9) & 0x00FFFFE0;
      *p = (*p & 0x9F00001F) | ImmHiValue | ImmLoValue;
      break;
    }
    case MachO::ARM64_RELOC_GOT_LOAD_PAGEOFF12: {
      // Verify that the relocation points to one of the expected load / store
      // instructions.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      assert((*p & 0x3B000000) == 0x39000000 &&
             "Only expected load / store instructions.");
      (void)p;
    } // fall-through
    case MachO::ARM64_RELOC_PAGEOFF12: {
      // Verify that the relocation points to one of the expected load / store
      // or add / sub instructions.
      auto *p = reinterpret_cast<support::aligned_ulittle32_t *>(LocalAddress);
      assert((((*p & 0x3B000000) == 0x39000000) ||
              ((*p & 0x11C00000) == 0x11000000)   ) &&
             "Expected load / store  or add/sub instruction.");

      // Check which instruction we are decoding to obtain the implicit shift
      // factor of the instruction and verify alignment.
      int ImplicitShift = 0;
      if ((*p & 0x3B000000) == 0x39000000) { // << load / store
        // For load / store instructions the size is encoded in bits 31:30.
        ImplicitShift = ((*p >> 30) & 0x3);
        switch (ImplicitShift) {
        case 0:
          // Check if this a vector op to get the correct shift value.
          if ((*p & 0x04800000) == 0x04800000) {
            ImplicitShift = 4;
            assert(((Addend & 0xF) == 0) &&
                   "128-bit LDR/STR not 16-byte aligned.");
          }
          break;
        case 1:
          assert(((Addend & 0x1) == 0) && "16-bit LDR/STR not 2-byte aligned.");
          break;
        case 2:
          assert(((Addend & 0x3) == 0) && "32-bit LDR/STR not 4-byte aligned.");
          break;
        case 3:
          assert(((Addend & 0x7) == 0) && "64-bit LDR/STR not 8-byte aligned.");
          break;
        }
      }
      // Compensate for implicit shift.
      Addend >>= ImplicitShift;
      assert(isUInt<12>(Addend) && "Addend cannot be encoded.");

      // Encode the addend into the instruction.
      *p = (*p & 0xFFC003FF) | ((uint32_t)(Addend << 10) & 0x003FFC00);
      break;
    }
    }
  }

  relocation_iterator
  processRelocationRef(unsigned SectionID, relocation_iterator RelI,
                       const ObjectFile &BaseObjT,
                       ObjSectionToIDMap &ObjSectionToID,
                       StubMap &Stubs) override {
    const MachOObjectFile &Obj =
      static_cast<const MachOObjectFile &>(BaseObjT);
    MachO::any_relocation_info RelInfo =
        Obj.getRelocation(RelI->getRawDataRefImpl());

    assert(!Obj.isRelocationScattered(RelInfo) && "");

    // ARM64 has an ARM64_RELOC_ADDEND relocation type that carries an explicit
    // addend for the following relocation. If found: (1) store the associated
    // addend, (2) consume the next relocation, and (3) use the stored addend to
    // override the addend.
    int64_t ExplicitAddend = 0;
    if (Obj.getAnyRelocationType(RelInfo) == MachO::ARM64_RELOC_ADDEND) {
      assert(!Obj.getPlainRelocationExternal(RelInfo));
      assert(!Obj.getAnyRelocationPCRel(RelInfo));
      assert(Obj.getAnyRelocationLength(RelInfo) == 2);
      int64_t RawAddend = Obj.getPlainRelocationSymbolNum(RelInfo);
      // Sign-extend the 24-bit to 64-bit.
      ExplicitAddend = SignExtend64(RawAddend, 24);
      ++RelI;
      RelInfo = Obj.getRelocation(RelI->getRawDataRefImpl());
    }

    RelocationEntry RE(getRelocationEntry(SectionID, Obj, RelI));
    RE.Addend = decodeAddend(RE);
    RelocationValueRef Value(
        getRelocationValueRef(Obj, RelI, RE, ObjSectionToID));

    assert((ExplicitAddend == 0 || RE.Addend == 0) && "Relocation has "\
      "ARM64_RELOC_ADDEND and embedded addend in the instruction.");
    if (ExplicitAddend) {
      RE.Addend = ExplicitAddend;
      Value.Offset = ExplicitAddend;
    }

    bool IsExtern = Obj.getPlainRelocationExternal(RelInfo);
    if (!IsExtern && RE.IsPCRel)
      makeValueAddendPCRel(Value, RelI, 1 << RE.Size);

    RE.Addend = Value.Offset;

    if (RE.RelType == MachO::ARM64_RELOC_GOT_LOAD_PAGE21 ||
        RE.RelType == MachO::ARM64_RELOC_GOT_LOAD_PAGEOFF12)
      processGOTRelocation(RE, Value, Stubs);
    else {
      if (Value.SymbolName)
        addRelocationForSymbol(RE, Value.SymbolName);
      else
        addRelocationForSection(RE, Value.SectionID);
    }

    return ++RelI;
  }

  void resolveRelocation(const RelocationEntry &RE, uint64_t Value) override {
    DEBUG(dumpRelocationToResolve(RE, Value));

    const SectionEntry &Section = Sections[RE.SectionID];
    uint8_t *LocalAddress = Section.Address + RE.Offset;
    MachO::RelocationInfoType RelType =
      static_cast<MachO::RelocationInfoType>(RE.RelType);

    switch (RelType) {
    default:
      llvm_unreachable("Invalid relocation type!");
    case MachO::ARM64_RELOC_UNSIGNED: {
      assert(!RE.IsPCRel && "PCRel and ARM64_RELOC_UNSIGNED not supported");
      // Mask in the target value a byte at a time (we don't have an alignment
      // guarantee for the target address, so this is safest).
      if (RE.Size < 2)
        llvm_unreachable("Invalid size for ARM64_RELOC_UNSIGNED");

      encodeAddend(LocalAddress, 1 << RE.Size, RelType, Value + RE.Addend);
      break;
    }
    case MachO::ARM64_RELOC_BRANCH26: {
      assert(RE.IsPCRel && "not PCRel and ARM64_RELOC_BRANCH26 not supported");
      // Check if branch is in range.
      uint64_t FinalAddress = Section.LoadAddress + RE.Offset;
      int64_t PCRelVal = Value - FinalAddress + RE.Addend;
      encodeAddend(LocalAddress, /*Size=*/4, RelType, PCRelVal);
      break;
    }
    case MachO::ARM64_RELOC_GOT_LOAD_PAGE21:
    case MachO::ARM64_RELOC_PAGE21: {
      assert(RE.IsPCRel && "not PCRel and ARM64_RELOC_PAGE21 not supported");
      // Adjust for PC-relative relocation and offset.
      uint64_t FinalAddress = Section.LoadAddress + RE.Offset;
      int64_t PCRelVal =
        ((Value + RE.Addend) & (-4096)) - (FinalAddress & (-4096));
      encodeAddend(LocalAddress, /*Size=*/4, RelType, PCRelVal);
      break;
    }
    case MachO::ARM64_RELOC_GOT_LOAD_PAGEOFF12:
    case MachO::ARM64_RELOC_PAGEOFF12: {
      assert(!RE.IsPCRel && "PCRel and ARM64_RELOC_PAGEOFF21 not supported");
      // Add the offset from the symbol.
      Value += RE.Addend;
      // Mask out the page address and only use the lower 12 bits.
      Value &= 0xFFF;
      encodeAddend(LocalAddress, /*Size=*/4, RelType, Value);
      break;
    }
    case MachO::ARM64_RELOC_SUBTRACTOR:
    case MachO::ARM64_RELOC_POINTER_TO_GOT:
    case MachO::ARM64_RELOC_TLVP_LOAD_PAGE21:
    case MachO::ARM64_RELOC_TLVP_LOAD_PAGEOFF12:
      llvm_unreachable("Relocation type not yet implemented!");
    case MachO::ARM64_RELOC_ADDEND:
      llvm_unreachable("ARM64_RELOC_ADDEND should have been handeled by "
                       "processRelocationRef!");
    }
  }

  void finalizeSection(const ObjectFile &Obj, unsigned SectionID,
                       const SectionRef &Section) {}

private:
  void processGOTRelocation(const RelocationEntry &RE,
                            RelocationValueRef &Value, StubMap &Stubs) {
    assert(RE.Size == 2);
    SectionEntry &Section = Sections[RE.SectionID];
    StubMap::const_iterator i = Stubs.find(Value);
    int64_t Offset;
    if (i != Stubs.end())
      Offset = static_cast<int64_t>(i->second);
    else {
      // FIXME: There must be a better way to do this then to check and fix the
      // alignment every time!!!
      uintptr_t BaseAddress = uintptr_t(Section.Address);
      uintptr_t StubAlignment = getStubAlignment();
      uintptr_t StubAddress =
          (BaseAddress + Section.StubOffset + StubAlignment - 1) &
          -StubAlignment;
      unsigned StubOffset = StubAddress - BaseAddress;
      Stubs[Value] = StubOffset;
      assert(((StubAddress % getStubAlignment()) == 0) &&
             "GOT entry not aligned");
      RelocationEntry GOTRE(RE.SectionID, StubOffset,
                            MachO::ARM64_RELOC_UNSIGNED, Value.Offset,
                            /*IsPCRel=*/false, /*Size=*/3);
      if (Value.SymbolName)
        addRelocationForSymbol(GOTRE, Value.SymbolName);
      else
        addRelocationForSection(GOTRE, Value.SectionID);
      Section.StubOffset = StubOffset + getMaxStubSize();
      Offset = static_cast<int64_t>(StubOffset);
    }
    RelocationEntry TargetRE(RE.SectionID, RE.Offset, RE.RelType, Offset,
                             RE.IsPCRel, RE.Size);
    addRelocationForSection(TargetRE, RE.SectionID);
  }
};
}

#undef DEBUG_TYPE

#endif
