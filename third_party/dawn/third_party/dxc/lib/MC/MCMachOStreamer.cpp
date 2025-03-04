//===-- MCMachOStreamer.cpp - MachO Streamer ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCStreamer.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCLinkerOptimizationHint.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCSymbolMachO.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class MCMachOStreamer : public MCObjectStreamer {
private:
  /// LabelSections - true if each section change should emit a linker local
  /// label for use in relocations for assembler local references. Obviates the
  /// need for local relocations. False by default.
  bool LabelSections;

  bool DWARFMustBeAtTheEnd;
  bool CreatedADWARFSection;

  /// HasSectionLabel - map of which sections have already had a non-local
  /// label emitted to them. Used so we don't emit extraneous linker local
  /// labels in the middle of the section.
  DenseMap<const MCSection*, bool> HasSectionLabel;

  void EmitInstToData(const MCInst &Inst, const MCSubtargetInfo &STI) override;

  void EmitDataRegion(DataRegionData::KindTy Kind);
  void EmitDataRegionEnd();

public:
  MCMachOStreamer(MCContext &Context, MCAsmBackend &MAB, raw_pwrite_stream &OS,
                  MCCodeEmitter *Emitter, bool DWARFMustBeAtTheEnd, bool label)
      : MCObjectStreamer(Context, MAB, OS, Emitter), LabelSections(label),
        DWARFMustBeAtTheEnd(DWARFMustBeAtTheEnd), CreatedADWARFSection(false) {}

  /// state management
  void reset() override {
    HasSectionLabel.clear();
    MCObjectStreamer::reset();
  }

  /// @name MCStreamer Interface
  /// @{

  void ChangeSection(MCSection *Sect, const MCExpr *Subsect) override;
  void EmitLabel(MCSymbol *Symbol) override;
  void EmitEHSymAttributes(const MCSymbol *Symbol, MCSymbol *EHSymbol) override;
  void EmitAssemblerFlag(MCAssemblerFlag Flag) override;
  void EmitLinkerOptions(ArrayRef<std::string> Options) override;
  void EmitDataRegion(MCDataRegionType Kind) override;
  void EmitVersionMin(MCVersionMinType Kind, unsigned Major,
                      unsigned Minor, unsigned Update) override;
  void EmitThumbFunc(MCSymbol *Func) override;
  bool EmitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override;
  void EmitSymbolDesc(MCSymbol *Symbol, unsigned DescValue) override;
  void EmitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        unsigned ByteAlignment) override;
  void BeginCOFFSymbolDef(const MCSymbol *Symbol) override {
    llvm_unreachable("macho doesn't support this directive");
  }
  void EmitCOFFSymbolStorageClass(int StorageClass) override {
    llvm_unreachable("macho doesn't support this directive");
  }
  void EmitCOFFSymbolType(int Type) override {
    llvm_unreachable("macho doesn't support this directive");
  }
  void EndCOFFSymbolDef() override {
    llvm_unreachable("macho doesn't support this directive");
  }
  void EmitLocalCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                             unsigned ByteAlignment) override;
  void EmitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                    uint64_t Size = 0, unsigned ByteAlignment = 0) override;
  void EmitTBSSSymbol(MCSection *Section, MCSymbol *Symbol, uint64_t Size,
                      unsigned ByteAlignment = 0) override;

  void EmitFileDirective(StringRef Filename) override {
    // FIXME: Just ignore the .file; it isn't important enough to fail the
    // entire assembly.

    // report_fatal_error("unsupported directive: '.file'");
  }

  void EmitIdent(StringRef IdentString) override {
    llvm_unreachable("macho doesn't support this directive");
  }

  void EmitLOHDirective(MCLOHType Kind, const MCLOHArgs &Args) override {
    getAssembler().getLOHContainer().addDirective(Kind, Args);
  }

  void FinishImpl() override;
};

} // end anonymous namespace.

static bool canGoAfterDWARF(const MCSectionMachO &MSec) {
  // These sections are created by the assembler itself after the end of
  // the .s file.
  StringRef SegName = MSec.getSegmentName();
  StringRef SecName = MSec.getSectionName();

  if (SegName == "__LD" && SecName == "__compact_unwind")
    return true;

  if (SegName == "__IMPORT") {
    if (SecName == "__jump_table")
      return true;

    if (SecName == "__pointers")
      return true;
  }

  if (SegName == "__TEXT" && SecName == "__eh_frame")
    return true;

  if (SegName == "__DATA" && SecName == "__nl_symbol_ptr")
    return true;

  return false;
}

void MCMachOStreamer::ChangeSection(MCSection *Section,
                                    const MCExpr *Subsection) {
  // Change the section normally.
  bool Created = MCObjectStreamer::changeSectionImpl(Section, Subsection);
  const MCSectionMachO &MSec = *cast<MCSectionMachO>(Section);
  StringRef SegName = MSec.getSegmentName();
  if (SegName == "__DWARF")
    CreatedADWARFSection = true;
  else if (Created && DWARFMustBeAtTheEnd && !canGoAfterDWARF(MSec))
    assert(!CreatedADWARFSection && "Creating regular section after DWARF");

  // Output a linker-local symbol so we don't need section-relative local
  // relocations. The linker hates us when we do that.
  if (LabelSections && !HasSectionLabel[Section] &&
      !Section->getBeginSymbol()) {
    MCSymbol *Label = getContext().createLinkerPrivateTempSymbol();
    Section->setBeginSymbol(Label);
    HasSectionLabel[Section] = true;
  }
}

void MCMachOStreamer::EmitEHSymAttributes(const MCSymbol *Symbol,
                                          MCSymbol *EHSymbol) {
  getAssembler().registerSymbol(*Symbol);
  if (Symbol->isExternal())
    EmitSymbolAttribute(EHSymbol, MCSA_Global);
  if (cast<MCSymbolMachO>(Symbol)->isWeakDefinition())
    EmitSymbolAttribute(EHSymbol, MCSA_WeakDefinition);
  if (Symbol->isPrivateExtern())
    EmitSymbolAttribute(EHSymbol, MCSA_PrivateExtern);
}

void MCMachOStreamer::EmitLabel(MCSymbol *Symbol) {
  assert(Symbol->isUndefined() && "Cannot define a symbol twice!");

  // isSymbolLinkerVisible uses the section.
  AssignSection(Symbol, getCurrentSection().first);
  // We have to create a new fragment if this is an atom defining symbol,
  // fragments cannot span atoms.
  if (getAssembler().isSymbolLinkerVisible(*Symbol))
    insert(new MCDataFragment());

  MCObjectStreamer::EmitLabel(Symbol);

  // This causes the reference type flag to be cleared. Darwin 'as' was "trying"
  // to clear the weak reference and weak definition bits too, but the
  // implementation was buggy. For now we just try to match 'as', for
  // diffability.
  //
  // FIXME: Cleanup this code, these bits should be emitted based on semantic
  // properties, not on the order of definition, etc.
  cast<MCSymbolMachO>(Symbol)->clearReferenceType();
}

void MCMachOStreamer::EmitDataRegion(DataRegionData::KindTy Kind) {
  if (!getAssembler().getBackend().hasDataInCodeSupport())
    return;
  // Create a temporary label to mark the start of the data region.
  MCSymbol *Start = getContext().createTempSymbol();
  EmitLabel(Start);
  // Record the region for the object writer to use.
  DataRegionData Data = { Kind, Start, nullptr };
  std::vector<DataRegionData> &Regions = getAssembler().getDataRegions();
  Regions.push_back(Data);
}

void MCMachOStreamer::EmitDataRegionEnd() {
  if (!getAssembler().getBackend().hasDataInCodeSupport())
    return;
  std::vector<DataRegionData> &Regions = getAssembler().getDataRegions();
  assert(!Regions.empty() && "Mismatched .end_data_region!");
  DataRegionData &Data = Regions.back();
  assert(!Data.End && "Mismatched .end_data_region!");
  // Create a temporary label to mark the end of the data region.
  Data.End = getContext().createTempSymbol();
  EmitLabel(Data.End);
}

void MCMachOStreamer::EmitAssemblerFlag(MCAssemblerFlag Flag) {
  // Let the target do whatever target specific stuff it needs to do.
  getAssembler().getBackend().handleAssemblerFlag(Flag);
  // Do any generic stuff we need to do.
  switch (Flag) {
  case MCAF_SyntaxUnified: return; // no-op here.
  case MCAF_Code16: return; // Change parsing mode; no-op here.
  case MCAF_Code32: return; // Change parsing mode; no-op here.
  case MCAF_Code64: return; // Change parsing mode; no-op here.
  case MCAF_SubsectionsViaSymbols:
    getAssembler().setSubsectionsViaSymbols(true);
    return;
  }
}

void MCMachOStreamer::EmitLinkerOptions(ArrayRef<std::string> Options) {
  getAssembler().getLinkerOptions().push_back(Options);
}

void MCMachOStreamer::EmitDataRegion(MCDataRegionType Kind) {
  switch (Kind) {
  case MCDR_DataRegion:
    EmitDataRegion(DataRegionData::Data);
    return;
  case MCDR_DataRegionJT8:
    EmitDataRegion(DataRegionData::JumpTable8);
    return;
  case MCDR_DataRegionJT16:
    EmitDataRegion(DataRegionData::JumpTable16);
    return;
  case MCDR_DataRegionJT32:
    EmitDataRegion(DataRegionData::JumpTable32);
    return;
  case MCDR_DataRegionEnd:
    EmitDataRegionEnd();
    return;
  }
}

void MCMachOStreamer::EmitVersionMin(MCVersionMinType Kind, unsigned Major,
                                     unsigned Minor, unsigned Update) {
  getAssembler().setVersionMinInfo(Kind, Major, Minor, Update);
}

void MCMachOStreamer::EmitThumbFunc(MCSymbol *Symbol) {
  // Remember that the function is a thumb function. Fixup and relocation
  // values will need adjusted.
  getAssembler().setIsThumbFunc(Symbol);
  cast<MCSymbolMachO>(Symbol)->setThumbFunc();
}

bool MCMachOStreamer::EmitSymbolAttribute(MCSymbol *Sym,
                                          MCSymbolAttr Attribute) {
  MCSymbolMachO *Symbol = cast<MCSymbolMachO>(Sym);

  // Indirect symbols are handled differently, to match how 'as' handles
  // them. This makes writing matching .o files easier.
  if (Attribute == MCSA_IndirectSymbol) {
    // Note that we intentionally cannot use the symbol data here; this is
    // important for matching the string table that 'as' generates.
    IndirectSymbolData ISD;
    ISD.Symbol = Symbol;
    ISD.Section = getCurrentSectionOnly();
    getAssembler().getIndirectSymbols().push_back(ISD);
    return true;
  }

  // Adding a symbol attribute always introduces the symbol, note that an
  // important side effect of calling registerSymbol here is to register
  // the symbol with the assembler.
  getAssembler().registerSymbol(*Symbol);

  // The implementation of symbol attributes is designed to match 'as', but it
  // leaves much to desired. It doesn't really make sense to arbitrarily add and
  // remove flags, but 'as' allows this (in particular, see .desc).
  //
  // In the future it might be worth trying to make these operations more well
  // defined.
  switch (Attribute) {
  case MCSA_Invalid:
  case MCSA_ELF_TypeFunction:
  case MCSA_ELF_TypeIndFunction:
  case MCSA_ELF_TypeObject:
  case MCSA_ELF_TypeTLS:
  case MCSA_ELF_TypeCommon:
  case MCSA_ELF_TypeNoType:
  case MCSA_ELF_TypeGnuUniqueObject:
  case MCSA_Hidden:
  case MCSA_IndirectSymbol:
  case MCSA_Internal:
  case MCSA_Protected:
  case MCSA_Weak:
  case MCSA_Local:
    return false;

  case MCSA_Global:
    Symbol->setExternal(true);
    // This effectively clears the undefined lazy bit, in Darwin 'as', although
    // it isn't very consistent because it implements this as part of symbol
    // lookup.
    //
    // FIXME: Cleanup this code, these bits should be emitted based on semantic
    // properties, not on the order of definition, etc.
    Symbol->setReferenceTypeUndefinedLazy(false);
    break;

  case MCSA_LazyReference:
    // FIXME: This requires -dynamic.
    Symbol->setNoDeadStrip();
    if (Symbol->isUndefined())
      Symbol->setReferenceTypeUndefinedLazy(true);
    break;

    // Since .reference sets the no dead strip bit, it is equivalent to
    // .no_dead_strip in practice.
  case MCSA_Reference:
  case MCSA_NoDeadStrip:
    Symbol->setNoDeadStrip();
    break;

  case MCSA_SymbolResolver:
    Symbol->setSymbolResolver();
    break;

  case MCSA_PrivateExtern:
    Symbol->setExternal(true);
    Symbol->setPrivateExtern(true);
    break;

  case MCSA_WeakReference:
    // FIXME: This requires -dynamic.
    if (Symbol->isUndefined())
      Symbol->setWeakReference();
    break;

  case MCSA_WeakDefinition:
    // FIXME: 'as' enforces that this is defined and global. The manual claims
    // it has to be in a coalesced section, but this isn't enforced.
    Symbol->setWeakDefinition();
    break;

  case MCSA_WeakDefAutoPrivate:
    Symbol->setWeakDefinition();
    Symbol->setWeakReference();
    break;
  }

  return true;
}

void MCMachOStreamer::EmitSymbolDesc(MCSymbol *Symbol, unsigned DescValue) {
  // Encode the 'desc' value into the lowest implementation defined bits.
  getAssembler().registerSymbol(*Symbol);
  cast<MCSymbolMachO>(Symbol)->setDesc(DescValue);
}

void MCMachOStreamer::EmitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                       unsigned ByteAlignment) {
  // FIXME: Darwin 'as' does appear to allow redef of a .comm by itself.
  assert(Symbol->isUndefined() && "Cannot define a symbol twice!");

  AssignSection(Symbol, nullptr);

  getAssembler().registerSymbol(*Symbol);
  Symbol->setExternal(true);
  Symbol->setCommon(Size, ByteAlignment);
}

void MCMachOStreamer::EmitLocalCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                            unsigned ByteAlignment) {
  // '.lcomm' is equivalent to '.zerofill'.
  return EmitZerofill(getContext().getObjectFileInfo()->getDataBSSSection(),
                      Symbol, Size, ByteAlignment);
}

void MCMachOStreamer::EmitZerofill(MCSection *Section, MCSymbol *Symbol,
                                   uint64_t Size, unsigned ByteAlignment) {
  getAssembler().registerSection(*Section);

  // The symbol may not be present, which only creates the section.
  if (!Symbol)
    return;

  // On darwin all virtual sections have zerofill type.
  assert(Section->isVirtualSection() && "Section does not have zerofill type!");

  assert(Symbol->isUndefined() && "Cannot define a symbol twice!");

  getAssembler().registerSymbol(*Symbol);

  // Emit an align fragment if necessary.
  if (ByteAlignment != 1)
    new MCAlignFragment(ByteAlignment, 0, 0, ByteAlignment, Section);

  AssignSection(Symbol, Section);

  MCFragment *F = new MCFillFragment(0, 0, Size, Section);
  Symbol->setFragment(F);

  // Update the maximum alignment on the zero fill section if necessary.
  if (ByteAlignment > Section->getAlignment())
    Section->setAlignment(ByteAlignment);
}

// This should always be called with the thread local bss section.  Like the
// .zerofill directive this doesn't actually switch sections on us.
void MCMachOStreamer::EmitTBSSSymbol(MCSection *Section, MCSymbol *Symbol,
                                     uint64_t Size, unsigned ByteAlignment) {
  EmitZerofill(Section, Symbol, Size, ByteAlignment);
  return;
}

void MCMachOStreamer::EmitInstToData(const MCInst &Inst,
                                     const MCSubtargetInfo &STI) {
  MCDataFragment *DF = getOrCreateDataFragment();

  SmallVector<MCFixup, 4> Fixups;
  SmallString<256> Code;
  raw_svector_ostream VecOS(Code);
  getAssembler().getEmitter().encodeInstruction(Inst, VecOS, Fixups, STI);
  VecOS.flush();

  // Add the fixups and data.
  for (unsigned i = 0, e = Fixups.size(); i != e; ++i) {
    Fixups[i].setOffset(Fixups[i].getOffset() + DF->getContents().size());
    DF->getFixups().push_back(Fixups[i]);
  }
  DF->getContents().append(Code.begin(), Code.end());
}

void MCMachOStreamer::FinishImpl() {
  EmitFrames(&getAssembler().getBackend());

  // We have to set the fragment atom associations so we can relax properly for
  // Mach-O.

  // First, scan the symbol table to build a lookup table from fragments to
  // defining symbols.
  DenseMap<const MCFragment *, const MCSymbol *> DefiningSymbolMap;
  for (const MCSymbol &Symbol : getAssembler().symbols()) {
    if (getAssembler().isSymbolLinkerVisible(Symbol) && Symbol.getFragment()) {
      // An atom defining symbol should never be internal to a fragment.
      assert(Symbol.getOffset() == 0 &&
             "Invalid offset in atom defining symbol!");
      DefiningSymbolMap[Symbol.getFragment()] = &Symbol;
    }
  }

  // Set the fragment atom associations by tracking the last seen atom defining
  // symbol.
  for (MCAssembler::iterator it = getAssembler().begin(),
         ie = getAssembler().end(); it != ie; ++it) {
    const MCSymbol *CurrentAtom = nullptr;
    for (MCSection::iterator it2 = it->begin(), ie2 = it->end(); it2 != ie2;
         ++it2) {
      if (const MCSymbol *Symbol = DefiningSymbolMap.lookup(it2))
        CurrentAtom = Symbol;
      it2->setAtom(CurrentAtom);
    }
  }

  this->MCObjectStreamer::FinishImpl();
}

MCStreamer *llvm::createMachOStreamer(MCContext &Context, MCAsmBackend &MAB,
                                      raw_pwrite_stream &OS, MCCodeEmitter *CE,
                                      bool RelaxAll, bool DWARFMustBeAtTheEnd,
                                      bool LabelSections) {
  MCMachOStreamer *S = new MCMachOStreamer(Context, MAB, OS, CE,
                                           DWARFMustBeAtTheEnd, LabelSections);
  if (RelaxAll)
    S->getAssembler().setRelaxAll(true);
  return S;
}
