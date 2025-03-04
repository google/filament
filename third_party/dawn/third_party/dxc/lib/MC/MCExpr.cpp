//===- MCExpr.cpp - Assembly Level Expression Implementation --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCExpr.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "mcexpr"

namespace {
namespace stats {
STATISTIC(MCExprEvaluate, "Number of MCExpr evaluations");
}
}

void MCExpr::print(raw_ostream &OS, const MCAsmInfo *MAI) const {
  switch (getKind()) {
  case MCExpr::Target:
    return cast<MCTargetExpr>(this)->printImpl(OS, MAI);
  case MCExpr::Constant:
    OS << cast<MCConstantExpr>(*this).getValue();
    return;

  case MCExpr::SymbolRef: {
    const MCSymbolRefExpr &SRE = cast<MCSymbolRefExpr>(*this);
    const MCSymbol &Sym = SRE.getSymbol();
    // Parenthesize names that start with $ so that they don't look like
    // absolute names.
    bool UseParens = Sym.getName()[0] == '$';
    if (UseParens) {
      OS << '(';
      Sym.print(OS, MAI);
      OS << ')';
    } else
      Sym.print(OS, MAI);

    if (SRE.getKind() != MCSymbolRefExpr::VK_None)
      SRE.printVariantKind(OS);

    return;
  }

  case MCExpr::Unary: {
    const MCUnaryExpr &UE = cast<MCUnaryExpr>(*this);
    switch (UE.getOpcode()) {
    case MCUnaryExpr::LNot:  OS << '!'; break;
    case MCUnaryExpr::Minus: OS << '-'; break;
    case MCUnaryExpr::Not:   OS << '~'; break;
    case MCUnaryExpr::Plus:  OS << '+'; break;
    }
    UE.getSubExpr()->print(OS, MAI);
    return;
  }

  case MCExpr::Binary: {
    const MCBinaryExpr &BE = cast<MCBinaryExpr>(*this);

    // Only print parens around the LHS if it is non-trivial.
    if (isa<MCConstantExpr>(BE.getLHS()) || isa<MCSymbolRefExpr>(BE.getLHS())) {
      BE.getLHS()->print(OS, MAI);
    } else {
      OS << '(';
      BE.getLHS()->print(OS, MAI);
      OS << ')';
    }

    switch (BE.getOpcode()) {
    case MCBinaryExpr::Add:
      // Print "X-42" instead of "X+-42".
      if (const MCConstantExpr *RHSC = dyn_cast<MCConstantExpr>(BE.getRHS())) {
        if (RHSC->getValue() < 0) {
          OS << RHSC->getValue();
          return;
        }
      }

      OS <<  '+';
      break;
    case MCBinaryExpr::AShr: OS << ">>"; break;
    case MCBinaryExpr::And:  OS <<  '&'; break;
    case MCBinaryExpr::Div:  OS <<  '/'; break;
    case MCBinaryExpr::EQ:   OS << "=="; break;
    case MCBinaryExpr::GT:   OS <<  '>'; break;
    case MCBinaryExpr::GTE:  OS << ">="; break;
    case MCBinaryExpr::LAnd: OS << "&&"; break;
    case MCBinaryExpr::LOr:  OS << "||"; break;
    case MCBinaryExpr::LShr: OS << ">>"; break;
    case MCBinaryExpr::LT:   OS <<  '<'; break;
    case MCBinaryExpr::LTE:  OS << "<="; break;
    case MCBinaryExpr::Mod:  OS <<  '%'; break;
    case MCBinaryExpr::Mul:  OS <<  '*'; break;
    case MCBinaryExpr::NE:   OS << "!="; break;
    case MCBinaryExpr::Or:   OS <<  '|'; break;
    case MCBinaryExpr::Shl:  OS << "<<"; break;
    case MCBinaryExpr::Sub:  OS <<  '-'; break;
    case MCBinaryExpr::Xor:  OS <<  '^'; break;
    }

    // Only print parens around the LHS if it is non-trivial.
    if (isa<MCConstantExpr>(BE.getRHS()) || isa<MCSymbolRefExpr>(BE.getRHS())) {
      BE.getRHS()->print(OS, MAI);
    } else {
      OS << '(';
      BE.getRHS()->print(OS, MAI);
      OS << ')';
    }
    return;
  }
  }

  llvm_unreachable("Invalid expression kind!");
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MCExpr::dump() const {
  dbgs() << *this;
  dbgs() << '\n';
}
#endif

/* *** */

const MCBinaryExpr *MCBinaryExpr::create(Opcode Opc, const MCExpr *LHS,
                                         const MCExpr *RHS, MCContext &Ctx) {
  return new (Ctx) MCBinaryExpr(Opc, LHS, RHS);
}

const MCUnaryExpr *MCUnaryExpr::create(Opcode Opc, const MCExpr *Expr,
                                       MCContext &Ctx) {
  return new (Ctx) MCUnaryExpr(Opc, Expr);
}

const MCConstantExpr *MCConstantExpr::create(int64_t Value, MCContext &Ctx) {
  return new (Ctx) MCConstantExpr(Value);
}

/* *** */

MCSymbolRefExpr::MCSymbolRefExpr(const MCSymbol *Symbol, VariantKind Kind,
                                 const MCAsmInfo *MAI)
    : MCExpr(MCExpr::SymbolRef), Kind(Kind),
      UseParensForSymbolVariant(MAI->useParensForSymbolVariant()),
      HasSubsectionsViaSymbols(MAI->hasSubsectionsViaSymbols()),
      Symbol(Symbol) {
  assert(Symbol);
}

const MCSymbolRefExpr *MCSymbolRefExpr::create(const MCSymbol *Sym,
                                               VariantKind Kind,
                                               MCContext &Ctx) {
  return new (Ctx) MCSymbolRefExpr(Sym, Kind, Ctx.getAsmInfo());
}

const MCSymbolRefExpr *MCSymbolRefExpr::create(StringRef Name, VariantKind Kind,
                                               MCContext &Ctx) {
  return create(Ctx.getOrCreateSymbol(Name), Kind, Ctx);
}

StringRef MCSymbolRefExpr::getVariantKindName(VariantKind Kind) {
  switch (Kind) {
  case VK_Invalid: return "<<invalid>>";
  case VK_None: return "<<none>>";

  case VK_GOT: return "GOT";
  case VK_GOTOFF: return "GOTOFF";
  case VK_GOTPCREL: return "GOTPCREL";
  case VK_GOTTPOFF: return "GOTTPOFF";
  case VK_INDNTPOFF: return "INDNTPOFF";
  case VK_NTPOFF: return "NTPOFF";
  case VK_GOTNTPOFF: return "GOTNTPOFF";
  case VK_PLT: return "PLT";
  case VK_TLSGD: return "TLSGD";
  case VK_TLSLD: return "TLSLD";
  case VK_TLSLDM: return "TLSLDM";
  case VK_TPOFF: return "TPOFF";
  case VK_DTPOFF: return "DTPOFF";
  case VK_TLVP: return "TLVP";
  case VK_TLVPPAGE: return "TLVPPAGE";
  case VK_TLVPPAGEOFF: return "TLVPPAGEOFF";
  case VK_PAGE: return "PAGE";
  case VK_PAGEOFF: return "PAGEOFF";
  case VK_GOTPAGE: return "GOTPAGE";
  case VK_GOTPAGEOFF: return "GOTPAGEOFF";
  case VK_SECREL: return "SECREL32";
  case VK_SIZE: return "SIZE";
  case VK_WEAKREF: return "WEAKREF";
  case VK_ARM_NONE: return "none";
  case VK_ARM_TARGET1: return "target1";
  case VK_ARM_TARGET2: return "target2";
  case VK_ARM_PREL31: return "prel31";
  case VK_ARM_SBREL: return "sbrel";
  case VK_ARM_TLSLDO: return "tlsldo";
  case VK_ARM_TLSCALL: return "tlscall";
  case VK_ARM_TLSDESC: return "tlsdesc";
  case VK_ARM_TLSDESCSEQ: return "tlsdescseq";
  case VK_PPC_LO: return "l";
  case VK_PPC_HI: return "h";
  case VK_PPC_HA: return "ha";
  case VK_PPC_HIGHER: return "higher";
  case VK_PPC_HIGHERA: return "highera";
  case VK_PPC_HIGHEST: return "highest";
  case VK_PPC_HIGHESTA: return "highesta";
  case VK_PPC_GOT_LO: return "got@l";
  case VK_PPC_GOT_HI: return "got@h";
  case VK_PPC_GOT_HA: return "got@ha";
  case VK_PPC_TOCBASE: return "tocbase";
  case VK_PPC_TOC: return "toc";
  case VK_PPC_TOC_LO: return "toc@l";
  case VK_PPC_TOC_HI: return "toc@h";
  case VK_PPC_TOC_HA: return "toc@ha";
  case VK_PPC_DTPMOD: return "dtpmod";
  case VK_PPC_TPREL: return "tprel";
  case VK_PPC_TPREL_LO: return "tprel@l";
  case VK_PPC_TPREL_HI: return "tprel@h";
  case VK_PPC_TPREL_HA: return "tprel@ha";
  case VK_PPC_TPREL_HIGHER: return "tprel@higher";
  case VK_PPC_TPREL_HIGHERA: return "tprel@highera";
  case VK_PPC_TPREL_HIGHEST: return "tprel@highest";
  case VK_PPC_TPREL_HIGHESTA: return "tprel@highesta";
  case VK_PPC_DTPREL: return "dtprel";
  case VK_PPC_DTPREL_LO: return "dtprel@l";
  case VK_PPC_DTPREL_HI: return "dtprel@h";
  case VK_PPC_DTPREL_HA: return "dtprel@ha";
  case VK_PPC_DTPREL_HIGHER: return "dtprel@higher";
  case VK_PPC_DTPREL_HIGHERA: return "dtprel@highera";
  case VK_PPC_DTPREL_HIGHEST: return "dtprel@highest";
  case VK_PPC_DTPREL_HIGHESTA: return "dtprel@highesta";
  case VK_PPC_GOT_TPREL: return "got@tprel";
  case VK_PPC_GOT_TPREL_LO: return "got@tprel@l";
  case VK_PPC_GOT_TPREL_HI: return "got@tprel@h";
  case VK_PPC_GOT_TPREL_HA: return "got@tprel@ha";
  case VK_PPC_GOT_DTPREL: return "got@dtprel";
  case VK_PPC_GOT_DTPREL_LO: return "got@dtprel@l";
  case VK_PPC_GOT_DTPREL_HI: return "got@dtprel@h";
  case VK_PPC_GOT_DTPREL_HA: return "got@dtprel@ha";
  case VK_PPC_TLS: return "tls";
  case VK_PPC_GOT_TLSGD: return "got@tlsgd";
  case VK_PPC_GOT_TLSGD_LO: return "got@tlsgd@l";
  case VK_PPC_GOT_TLSGD_HI: return "got@tlsgd@h";
  case VK_PPC_GOT_TLSGD_HA: return "got@tlsgd@ha";
  case VK_PPC_TLSGD: return "tlsgd";
  case VK_PPC_GOT_TLSLD: return "got@tlsld";
  case VK_PPC_GOT_TLSLD_LO: return "got@tlsld@l";
  case VK_PPC_GOT_TLSLD_HI: return "got@tlsld@h";
  case VK_PPC_GOT_TLSLD_HA: return "got@tlsld@ha";
  case VK_PPC_TLSLD: return "tlsld";
  case VK_PPC_LOCAL: return "local";
  case VK_Mips_GPREL: return "GPREL";
  case VK_Mips_GOT_CALL: return "GOT_CALL";
  case VK_Mips_GOT16: return "GOT16";
  case VK_Mips_GOT: return "GOT";
  case VK_Mips_ABS_HI: return "ABS_HI";
  case VK_Mips_ABS_LO: return "ABS_LO";
  case VK_Mips_TLSGD: return "TLSGD";
  case VK_Mips_TLSLDM: return "TLSLDM";
  case VK_Mips_DTPREL_HI: return "DTPREL_HI";
  case VK_Mips_DTPREL_LO: return "DTPREL_LO";
  case VK_Mips_GOTTPREL: return "GOTTPREL";
  case VK_Mips_TPREL_HI: return "TPREL_HI";
  case VK_Mips_TPREL_LO: return "TPREL_LO";
  case VK_Mips_GPOFF_HI: return "GPOFF_HI";
  case VK_Mips_GPOFF_LO: return "GPOFF_LO";
  case VK_Mips_GOT_DISP: return "GOT_DISP";
  case VK_Mips_GOT_PAGE: return "GOT_PAGE";
  case VK_Mips_GOT_OFST: return "GOT_OFST";
  case VK_Mips_HIGHER:   return "HIGHER";
  case VK_Mips_HIGHEST:  return "HIGHEST";
  case VK_Mips_GOT_HI16: return "GOT_HI16";
  case VK_Mips_GOT_LO16: return "GOT_LO16";
  case VK_Mips_CALL_HI16: return "CALL_HI16";
  case VK_Mips_CALL_LO16: return "CALL_LO16";
  case VK_Mips_PCREL_HI16: return "PCREL_HI16";
  case VK_Mips_PCREL_LO16: return "PCREL_LO16";
  case VK_COFF_IMGREL32: return "IMGREL";
  case VK_Hexagon_PCREL: return "PCREL";
  case VK_Hexagon_LO16: return "LO16";
  case VK_Hexagon_HI16: return "HI16";
  case VK_Hexagon_GPREL: return "GPREL";
  case VK_Hexagon_GD_GOT: return "GDGOT";
  case VK_Hexagon_LD_GOT: return "LDGOT";
  case VK_Hexagon_GD_PLT: return "GDPLT";
  case VK_Hexagon_LD_PLT: return "LDPLT";
  case VK_Hexagon_IE: return "IE";
  case VK_Hexagon_IE_GOT: return "IEGOT";
  case VK_TPREL: return "tprel";
  case VK_DTPREL: return "dtprel";
  }
  llvm_unreachable("Invalid variant kind");
}

MCSymbolRefExpr::VariantKind
MCSymbolRefExpr::getVariantKindForName(StringRef Name) {
  return StringSwitch<VariantKind>(Name.lower())
    .Case("got", VK_GOT)
    .Case("gotoff", VK_GOTOFF)
    .Case("gotpcrel", VK_GOTPCREL)
    .Case("got_prel", VK_GOTPCREL)
    .Case("gottpoff", VK_GOTTPOFF)
    .Case("indntpoff", VK_INDNTPOFF)
    .Case("ntpoff", VK_NTPOFF)
    .Case("gotntpoff", VK_GOTNTPOFF)
    .Case("plt", VK_PLT)
    .Case("tlsgd", VK_TLSGD)
    .Case("tlsld", VK_TLSLD)
    .Case("tlsldm", VK_TLSLDM)
    .Case("tpoff", VK_TPOFF)
    .Case("dtpoff", VK_DTPOFF)
    .Case("tlvp", VK_TLVP)
    .Case("tlvppage", VK_TLVPPAGE)
    .Case("tlvppageoff", VK_TLVPPAGEOFF)
    .Case("page", VK_PAGE)
    .Case("pageoff", VK_PAGEOFF)
    .Case("gotpage", VK_GOTPAGE)
    .Case("gotpageoff", VK_GOTPAGEOFF)
    .Case("imgrel", VK_COFF_IMGREL32)
    .Case("secrel32", VK_SECREL)
    .Case("size", VK_SIZE)
    .Case("l", VK_PPC_LO)
    .Case("h", VK_PPC_HI)
    .Case("ha", VK_PPC_HA)
    .Case("higher", VK_PPC_HIGHER)
    .Case("highera", VK_PPC_HIGHERA)
    .Case("highest", VK_PPC_HIGHEST)
    .Case("highesta", VK_PPC_HIGHESTA)
    .Case("got@l", VK_PPC_GOT_LO)
    .Case("got@h", VK_PPC_GOT_HI)
    .Case("got@ha", VK_PPC_GOT_HA)
    .Case("local", VK_PPC_LOCAL)
    .Case("tocbase", VK_PPC_TOCBASE)
    .Case("toc", VK_PPC_TOC)
    .Case("toc@l", VK_PPC_TOC_LO)
    .Case("toc@h", VK_PPC_TOC_HI)
    .Case("toc@ha", VK_PPC_TOC_HA)
    .Case("tls", VK_PPC_TLS)
    .Case("dtpmod", VK_PPC_DTPMOD)
    .Case("tprel", VK_PPC_TPREL)
    .Case("tprel@l", VK_PPC_TPREL_LO)
    .Case("tprel@h", VK_PPC_TPREL_HI)
    .Case("tprel@ha", VK_PPC_TPREL_HA)
    .Case("tprel@higher", VK_PPC_TPREL_HIGHER)
    .Case("tprel@highera", VK_PPC_TPREL_HIGHERA)
    .Case("tprel@highest", VK_PPC_TPREL_HIGHEST)
    .Case("tprel@highesta", VK_PPC_TPREL_HIGHESTA)
    .Case("dtprel", VK_PPC_DTPREL)
    .Case("dtprel@l", VK_PPC_DTPREL_LO)
    .Case("dtprel@h", VK_PPC_DTPREL_HI)
    .Case("dtprel@ha", VK_PPC_DTPREL_HA)
    .Case("dtprel@higher", VK_PPC_DTPREL_HIGHER)
    .Case("dtprel@highera", VK_PPC_DTPREL_HIGHERA)
    .Case("dtprel@highest", VK_PPC_DTPREL_HIGHEST)
    .Case("dtprel@highesta", VK_PPC_DTPREL_HIGHESTA)
    .Case("got@tprel", VK_PPC_GOT_TPREL)
    .Case("got@tprel@l", VK_PPC_GOT_TPREL_LO)
    .Case("got@tprel@h", VK_PPC_GOT_TPREL_HI)
    .Case("got@tprel@ha", VK_PPC_GOT_TPREL_HA)
    .Case("got@dtprel", VK_PPC_GOT_DTPREL)
    .Case("got@dtprel@l", VK_PPC_GOT_DTPREL_LO)
    .Case("got@dtprel@h", VK_PPC_GOT_DTPREL_HI)
    .Case("got@dtprel@ha", VK_PPC_GOT_DTPREL_HA)
    .Case("got@tlsgd", VK_PPC_GOT_TLSGD)
    .Case("got@tlsgd@l", VK_PPC_GOT_TLSGD_LO)
    .Case("got@tlsgd@h", VK_PPC_GOT_TLSGD_HI)
    .Case("got@tlsgd@ha", VK_PPC_GOT_TLSGD_HA)
    .Case("got@tlsld", VK_PPC_GOT_TLSLD)
    .Case("got@tlsld@l", VK_PPC_GOT_TLSLD_LO)
    .Case("got@tlsld@h", VK_PPC_GOT_TLSLD_HI)
    .Case("got@tlsld@ha", VK_PPC_GOT_TLSLD_HA)
    .Case("none", VK_ARM_NONE)
    .Case("target1", VK_ARM_TARGET1)
    .Case("target2", VK_ARM_TARGET2)
    .Case("prel31", VK_ARM_PREL31)
    .Case("sbrel", VK_ARM_SBREL)
    .Case("tlsldo", VK_ARM_TLSLDO)
    .Case("tlscall", VK_ARM_TLSCALL)
    .Case("tlsdesc", VK_ARM_TLSDESC)
    .Default(VK_Invalid);
}

void MCSymbolRefExpr::printVariantKind(raw_ostream &OS) const {
  if (UseParensForSymbolVariant)
    OS << '(' << MCSymbolRefExpr::getVariantKindName(getKind()) << ')';
  else
    OS << '@' << MCSymbolRefExpr::getVariantKindName(getKind());
}

/* *** */

void MCTargetExpr::anchor() {}

/* *** */

bool MCExpr::evaluateAsAbsolute(int64_t &Res) const {
  return evaluateAsAbsolute(Res, nullptr, nullptr, nullptr);
}

bool MCExpr::evaluateAsAbsolute(int64_t &Res,
                                const MCAsmLayout &Layout) const {
  return evaluateAsAbsolute(Res, &Layout.getAssembler(), &Layout, nullptr);
}

bool MCExpr::evaluateAsAbsolute(int64_t &Res,
                                const MCAsmLayout &Layout,
                                const SectionAddrMap &Addrs) const {
  return evaluateAsAbsolute(Res, &Layout.getAssembler(), &Layout, &Addrs);
}

bool MCExpr::evaluateAsAbsolute(int64_t &Res, const MCAssembler &Asm) const {
  return evaluateAsAbsolute(Res, &Asm, nullptr, nullptr);
}

bool MCExpr::evaluateKnownAbsolute(int64_t &Res,
                                   const MCAsmLayout &Layout) const {
  return evaluateAsAbsolute(Res, &Layout.getAssembler(), &Layout, nullptr,
                            true);
}

bool MCExpr::evaluateAsAbsolute(int64_t &Res, const MCAssembler *Asm,
                                const MCAsmLayout *Layout,
                                const SectionAddrMap *Addrs) const {
  // FIXME: The use if InSet = Addrs is a hack. Setting InSet causes us
  // absolutize differences across sections and that is what the MachO writer
  // uses Addrs for.
  return evaluateAsAbsolute(Res, Asm, Layout, Addrs, Addrs);
}

bool MCExpr::evaluateAsAbsolute(int64_t &Res, const MCAssembler *Asm,
                                const MCAsmLayout *Layout,
                                const SectionAddrMap *Addrs, bool InSet) const {
  MCValue Value;

  // Fast path constants.
  if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(this)) {
    Res = CE->getValue();
    return true;
  }

  bool IsRelocatable =
      evaluateAsRelocatableImpl(Value, Asm, Layout, nullptr, Addrs, InSet);

  // Record the current value.
  Res = Value.getConstant();

  return IsRelocatable && Value.isAbsolute();
}

/// \brief Helper method for \see EvaluateSymbolAdd().
static void AttemptToFoldSymbolOffsetDifference(
    const MCAssembler *Asm, const MCAsmLayout *Layout,
    const SectionAddrMap *Addrs, bool InSet, const MCSymbolRefExpr *&A,
    const MCSymbolRefExpr *&B, int64_t &Addend) {
  if (!A || !B)
    return;

  const MCSymbol &SA = A->getSymbol();
  const MCSymbol &SB = B->getSymbol();

  if (SA.isUndefined() || SB.isUndefined())
    return;

  if (!Asm->getWriter().isSymbolRefDifferenceFullyResolved(*Asm, A, B, InSet))
    return;

  if (SA.getFragment() == SB.getFragment()) {
    Addend += (SA.getOffset() - SB.getOffset());

    // Pointers to Thumb symbols need to have their low-bit set to allow
    // for interworking.
    if (Asm->isThumbFunc(&SA))
      Addend |= 1;

    // Clear the symbol expr pointers to indicate we have folded these
    // operands.
    A = B = nullptr;
    return;
  }

  if (!Layout)
    return;

  const MCSection &SecA = *SA.getFragment()->getParent();
  const MCSection &SecB = *SB.getFragment()->getParent();

  if ((&SecA != &SecB) && !Addrs)
    return;

  // Eagerly evaluate.
  Addend += Layout->getSymbolOffset(A->getSymbol()) -
            Layout->getSymbolOffset(B->getSymbol());
  if (Addrs && (&SecA != &SecB))
    Addend += (Addrs->lookup(&SecA) - Addrs->lookup(&SecB));

  // Pointers to Thumb symbols need to have their low-bit set to allow
  // for interworking.
  if (Asm->isThumbFunc(&SA))
    Addend |= 1;

  // Clear the symbol expr pointers to indicate we have folded these
  // operands.
  A = B = nullptr;
}

/// \brief Evaluate the result of an add between (conceptually) two MCValues.
///
/// This routine conceptually attempts to construct an MCValue:
///   Result = (Result_A - Result_B + Result_Cst)
/// from two MCValue's LHS and RHS where
///   Result = LHS + RHS
/// and
///   Result = (LHS_A - LHS_B + LHS_Cst) + (RHS_A - RHS_B + RHS_Cst).
///
/// This routine attempts to aggresively fold the operands such that the result
/// is representable in an MCValue, but may not always succeed.
///
/// \returns True on success, false if the result is not representable in an
/// MCValue.

/// NOTE: It is really important to have both the Asm and Layout arguments.
/// They might look redundant, but this function can be used before layout
/// is done (see the object streamer for example) and having the Asm argument
/// lets us avoid relaxations early.
static bool
EvaluateSymbolicAdd(const MCAssembler *Asm, const MCAsmLayout *Layout,
                    const SectionAddrMap *Addrs, bool InSet, const MCValue &LHS,
                    const MCSymbolRefExpr *RHS_A, const MCSymbolRefExpr *RHS_B,
                    int64_t RHS_Cst, MCValue &Res) {
  // FIXME: This routine (and other evaluation parts) are *incredibly* sloppy
  // about dealing with modifiers. This will ultimately bite us, one day.
  const MCSymbolRefExpr *LHS_A = LHS.getSymA();
  const MCSymbolRefExpr *LHS_B = LHS.getSymB();
  int64_t LHS_Cst = LHS.getConstant();

  // Fold the result constant immediately.
  int64_t Result_Cst = LHS_Cst + RHS_Cst;

  assert((!Layout || Asm) &&
         "Must have an assembler object if layout is given!");

  // If we have a layout, we can fold resolved differences.
  if (Asm) {
    // First, fold out any differences which are fully resolved. By
    // reassociating terms in
    //   Result = (LHS_A - LHS_B + LHS_Cst) + (RHS_A - RHS_B + RHS_Cst).
    // we have the four possible differences:
    //   (LHS_A - LHS_B),
    //   (LHS_A - RHS_B),
    //   (RHS_A - LHS_B),
    //   (RHS_A - RHS_B).
    // Since we are attempting to be as aggressive as possible about folding, we
    // attempt to evaluate each possible alternative.
    AttemptToFoldSymbolOffsetDifference(Asm, Layout, Addrs, InSet, LHS_A, LHS_B,
                                        Result_Cst);
    AttemptToFoldSymbolOffsetDifference(Asm, Layout, Addrs, InSet, LHS_A, RHS_B,
                                        Result_Cst);
    AttemptToFoldSymbolOffsetDifference(Asm, Layout, Addrs, InSet, RHS_A, LHS_B,
                                        Result_Cst);
    AttemptToFoldSymbolOffsetDifference(Asm, Layout, Addrs, InSet, RHS_A, RHS_B,
                                        Result_Cst);
  }

  // We can't represent the addition or subtraction of two symbols.
  if ((LHS_A && RHS_A) || (LHS_B && RHS_B))
    return false;

  // At this point, we have at most one additive symbol and one subtractive
  // symbol -- find them.
  const MCSymbolRefExpr *A = LHS_A ? LHS_A : RHS_A;
  const MCSymbolRefExpr *B = LHS_B ? LHS_B : RHS_B;

  // If we have a negated symbol, then we must have also have a non-negated
  // symbol in order to encode the expression.
  if (B && !A)
    return false;

  Res = MCValue::get(A, B, Result_Cst);
  return true;
}

bool MCExpr::evaluateAsRelocatable(MCValue &Res,
                                   const MCAsmLayout *Layout,
                                   const MCFixup *Fixup) const {
  MCAssembler *Assembler = Layout ? &Layout->getAssembler() : nullptr;
  return evaluateAsRelocatableImpl(Res, Assembler, Layout, Fixup, nullptr,
                                   false);
}

bool MCExpr::evaluateAsValue(MCValue &Res, const MCAsmLayout &Layout) const {
  MCAssembler *Assembler = &Layout.getAssembler();
  return evaluateAsRelocatableImpl(Res, Assembler, &Layout, nullptr, nullptr,
                                   true);
}

static bool canExpand(const MCSymbol &Sym, const MCAssembler *Asm, bool InSet) {
  const MCExpr *Expr = Sym.getVariableValue();
  const auto *Inner = dyn_cast<MCSymbolRefExpr>(Expr);
  if (Inner) {
    if (Inner->getKind() == MCSymbolRefExpr::VK_WEAKREF)
      return false;
  }

  if (InSet)
    return true;
  if (!Asm)
    return false;
  return !Asm->getWriter().isWeak(Sym);
}

bool MCExpr::evaluateAsRelocatableImpl(MCValue &Res, const MCAssembler *Asm,
                                       const MCAsmLayout *Layout,
                                       const MCFixup *Fixup,
                                       const SectionAddrMap *Addrs,
                                       bool InSet) const {
  ++stats::MCExprEvaluate;

  switch (getKind()) {
  case Target:
    return cast<MCTargetExpr>(this)->evaluateAsRelocatableImpl(Res, Layout,
                                                               Fixup);

  case Constant:
    Res = MCValue::get(cast<MCConstantExpr>(this)->getValue());
    return true;

  case SymbolRef: {
    const MCSymbolRefExpr *SRE = cast<MCSymbolRefExpr>(this);
    const MCSymbol &Sym = SRE->getSymbol();

    // Evaluate recursively if this is a variable.
    if (Sym.isVariable() && SRE->getKind() == MCSymbolRefExpr::VK_None &&
        canExpand(Sym, Asm, InSet)) {
      bool IsMachO = SRE->hasSubsectionsViaSymbols();
      if (Sym.getVariableValue()->evaluateAsRelocatableImpl(
              Res, Asm, Layout, Fixup, Addrs, InSet || IsMachO)) {
        if (!IsMachO)
          return true;

        const MCSymbolRefExpr *A = Res.getSymA();
        const MCSymbolRefExpr *B = Res.getSymB();
        // FIXME: This is small hack. Given
        // a = b + 4
        // .long a
        // the OS X assembler will completely drop the 4. We should probably
        // include it in the relocation or produce an error if that is not
        // possible.
        if (!A && !B)
          return true;
      }
    }

    Res = MCValue::get(SRE, nullptr, 0);
    return true;
  }

  case Unary: {
    const MCUnaryExpr *AUE = cast<MCUnaryExpr>(this);
    MCValue Value;

    if (!AUE->getSubExpr()->evaluateAsRelocatableImpl(Value, Asm, Layout, Fixup,
                                                      Addrs, InSet))
      return false;

    switch (AUE->getOpcode()) {
    case MCUnaryExpr::LNot:
      if (!Value.isAbsolute())
        return false;
      Res = MCValue::get(!Value.getConstant());
      break;
    case MCUnaryExpr::Minus:
      /// -(a - b + const) ==> (b - a - const)
      if (Value.getSymA() && !Value.getSymB())
        return false;
      Res = MCValue::get(Value.getSymB(), Value.getSymA(),
                         -Value.getConstant());
      break;
    case MCUnaryExpr::Not:
      if (!Value.isAbsolute())
        return false;
      Res = MCValue::get(~Value.getConstant());
      break;
    case MCUnaryExpr::Plus:
      Res = Value;
      break;
    }

    return true;
  }

  case Binary: {
    const MCBinaryExpr *ABE = cast<MCBinaryExpr>(this);
    MCValue LHSValue, RHSValue;

    if (!ABE->getLHS()->evaluateAsRelocatableImpl(LHSValue, Asm, Layout, Fixup,
                                                  Addrs, InSet) ||
        !ABE->getRHS()->evaluateAsRelocatableImpl(RHSValue, Asm, Layout, Fixup,
                                                  Addrs, InSet))
      return false;

    // We only support a few operations on non-constant expressions, handle
    // those first.
    if (!LHSValue.isAbsolute() || !RHSValue.isAbsolute()) {
      switch (ABE->getOpcode()) {
      default:
        return false;
      case MCBinaryExpr::Sub:
        // Negate RHS and add.
        return EvaluateSymbolicAdd(Asm, Layout, Addrs, InSet, LHSValue,
                                   RHSValue.getSymB(), RHSValue.getSymA(),
                                   -RHSValue.getConstant(), Res);

      case MCBinaryExpr::Add:
        return EvaluateSymbolicAdd(Asm, Layout, Addrs, InSet, LHSValue,
                                   RHSValue.getSymA(), RHSValue.getSymB(),
                                   RHSValue.getConstant(), Res);
      }
    }

    // FIXME: We need target hooks for the evaluation. It may be limited in
    // width, and gas defines the result of comparisons differently from
    // Apple as.
    int64_t LHS = LHSValue.getConstant(), RHS = RHSValue.getConstant();
    int64_t Result = 0;
    switch (ABE->getOpcode()) {
    case MCBinaryExpr::AShr: Result = LHS >> RHS; break;
    case MCBinaryExpr::Add:  Result = LHS + RHS; break;
    case MCBinaryExpr::And:  Result = LHS & RHS; break;
    case MCBinaryExpr::Div:  Result = LHS / RHS; break;
    case MCBinaryExpr::EQ:   Result = LHS == RHS; break;
    case MCBinaryExpr::GT:   Result = LHS > RHS; break;
    case MCBinaryExpr::GTE:  Result = LHS >= RHS; break;
    case MCBinaryExpr::LAnd: Result = LHS && RHS; break;
    case MCBinaryExpr::LOr:  Result = LHS || RHS; break;
    case MCBinaryExpr::LShr: Result = uint64_t(LHS) >> uint64_t(RHS); break;
    case MCBinaryExpr::LT:   Result = LHS < RHS; break;
    case MCBinaryExpr::LTE:  Result = LHS <= RHS; break;
    case MCBinaryExpr::Mod:  Result = LHS % RHS; break;
    case MCBinaryExpr::Mul:  Result = LHS * RHS; break;
    case MCBinaryExpr::NE:   Result = LHS != RHS; break;
    case MCBinaryExpr::Or:   Result = LHS | RHS; break;
    case MCBinaryExpr::Shl:  Result = uint64_t(LHS) << uint64_t(RHS); break;
    case MCBinaryExpr::Sub:  Result = LHS - RHS; break;
    case MCBinaryExpr::Xor:  Result = LHS ^ RHS; break;
    }

    Res = MCValue::get(Result);
    return true;
  }
  }

  llvm_unreachable("Invalid assembly expression kind!");
}

MCSection *MCExpr::findAssociatedSection() const {
  switch (getKind()) {
  case Target:
    // We never look through target specific expressions.
    return cast<MCTargetExpr>(this)->findAssociatedSection();

  case Constant:
    return MCSymbol::AbsolutePseudoSection;

  case SymbolRef: {
    const MCSymbolRefExpr *SRE = cast<MCSymbolRefExpr>(this);
    const MCSymbol &Sym = SRE->getSymbol();

    if (Sym.isDefined())
      return &Sym.getSection();

    return nullptr;
  }

  case Unary:
    return cast<MCUnaryExpr>(this)->getSubExpr()->findAssociatedSection();

  case Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(this);
    MCSection *LHS_S = BE->getLHS()->findAssociatedSection();
    MCSection *RHS_S = BE->getRHS()->findAssociatedSection();

    // If either section is absolute, return the other.
    if (LHS_S == MCSymbol::AbsolutePseudoSection)
      return RHS_S;
    if (RHS_S == MCSymbol::AbsolutePseudoSection)
      return LHS_S;

    // Not always correct, but probably the best we can do without more context.
    if (BE->getOpcode() == MCBinaryExpr::Sub)
      return MCSymbol::AbsolutePseudoSection;

    // Otherwise, return the first non-null section.
    return LHS_S ? LHS_S : RHS_S;
  }
  }

  llvm_unreachable("Invalid assembly expression kind!");
}
