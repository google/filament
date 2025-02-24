/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Register Bank Source Fragments                                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_REGBANK_DECLARATIONS
#undef GET_REGBANK_DECLARATIONS
namespace llvm {
namespace PPC {
enum : unsigned {
  InvalidRegBankID = ~0u,
  CRRegBankID = 0,
  FPRRegBankID = 1,
  GPRRegBankID = 2,
  NumRegisterBanks,
};
} // end namespace PPC
} // end namespace llvm
#endif // GET_REGBANK_DECLARATIONS

#ifdef GET_TARGET_REGBANK_CLASS
#undef GET_TARGET_REGBANK_CLASS
private:
  static RegisterBank *RegBanks[];

protected:
  PPCGenRegisterBankInfo();

#endif // GET_TARGET_REGBANK_CLASS

#ifdef GET_TARGET_REGBANK_IMPL
#undef GET_TARGET_REGBANK_IMPL
namespace llvm {
namespace PPC {
const uint32_t CRRegBankCoverageData[] = {
    // 0-31
    (1u << (PPC::CRRCRegClassID - 0)) |
    (1u << (PPC::CRBITRCRegClassID - 0)) |
    0,
    // 32-63
    0,
};
const uint32_t FPRRegBankCoverageData[] = {
    // 0-31
    (1u << (PPC::VSSRCRegClassID - 0)) |
    (1u << (PPC::F4RCRegClassID - 0)) |
    (1u << (PPC::F8RCRegClassID - 0)) |
    (1u << (PPC::SPILLTOVSRRC_and_F4RCRegClassID - 0)) |
    (1u << (PPC::VSFRCRegClassID - 0)) |
    (1u << (PPC::SPILLTOVSRRC_and_VSFRCRegClassID - 0)) |
    (1u << (PPC::SPILLTOVSRRC_and_VFRCRegClassID - 0)) |
    (1u << (PPC::VFRCRegClassID - 0)) |
    0,
    // 32-63
    0,
};
const uint32_t GPRRegBankCoverageData[] = {
    // 0-31
    (1u << (PPC::G8RCRegClassID - 0)) |
    (1u << (PPC::GPRCRegClassID - 0)) |
    (1u << (PPC::G8RC_and_G8RC_NOX0RegClassID - 0)) |
    (1u << (PPC::GPRC_NOR0RegClassID - 0)) |
    (1u << (PPC::GPRC_and_GPRC_NOR0RegClassID - 0)) |
    (1u << (PPC::G8RC_NOX0RegClassID - 0)) |
    0,
    // 32-63
    0,
};

RegisterBank CRRegBank(/* ID */ PPC::CRRegBankID, /* Name */ "CR", /* Size */ 32, /* CoveredRegClasses */ CRRegBankCoverageData, /* NumRegClasses */ 51);
RegisterBank FPRRegBank(/* ID */ PPC::FPRRegBankID, /* Name */ "FPR", /* Size */ 64, /* CoveredRegClasses */ FPRRegBankCoverageData, /* NumRegClasses */ 51);
RegisterBank GPRRegBank(/* ID */ PPC::GPRRegBankID, /* Name */ "GPR", /* Size */ 64, /* CoveredRegClasses */ GPRRegBankCoverageData, /* NumRegClasses */ 51);
} // end namespace PPC

RegisterBank *PPCGenRegisterBankInfo::RegBanks[] = {
    &PPC::CRRegBank,
    &PPC::FPRRegBank,
    &PPC::GPRRegBank,
};

PPCGenRegisterBankInfo::PPCGenRegisterBankInfo()
    : RegisterBankInfo(RegBanks, PPC::NumRegisterBanks) {
  // Assert that RegBank indices match their ID's
#ifndef NDEBUG
  for (auto RB : enumerate(RegBanks))
    assert(RB.index() == RB.value()->getID() && "Index != ID");
#endif // NDEBUG
}
} // end namespace llvm
#endif // GET_TARGET_REGBANK_IMPL
