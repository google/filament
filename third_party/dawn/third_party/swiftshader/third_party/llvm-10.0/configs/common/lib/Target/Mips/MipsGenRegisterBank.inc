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
namespace Mips {
enum {
  FPRBRegBankID,
  GPRBRegBankID,
  NumRegisterBanks,
};
} // end namespace Mips
} // end namespace llvm
#endif // GET_REGBANK_DECLARATIONS

#ifdef GET_TARGET_REGBANK_CLASS
#undef GET_TARGET_REGBANK_CLASS
private:
  static RegisterBank *RegBanks[];

protected:
  MipsGenRegisterBankInfo();

#endif // GET_TARGET_REGBANK_CLASS

#ifdef GET_TARGET_REGBANK_IMPL
#undef GET_TARGET_REGBANK_IMPL
namespace llvm {
namespace Mips {
const uint32_t FPRBRegBankCoverageData[] = {
    // 0-31
    (1u << (Mips::FGR32RegClassID - 0)) |
    (1u << (Mips::FGRCCRegClassID - 0)) |
    0,
    // 32-63
    (1u << (Mips::FGR64RegClassID - 32)) |
    (1u << (Mips::AFGR64RegClassID - 32)) |
    0,
    // 64-95
    (1u << (Mips::MSA128DRegClassID - 64)) |
    (1u << (Mips::MSA128BRegClassID - 64)) |
    (1u << (Mips::MSA128HRegClassID - 64)) |
    (1u << (Mips::MSA128WRegClassID - 64)) |
    (1u << (Mips::MSA128WEvensRegClassID - 64)) |
    0,
};
const uint32_t GPRBRegBankCoverageData[] = {
    // 0-31
    (1u << (Mips::GPR32RegClassID - 0)) |
    (1u << (Mips::GPR32NONZERORegClassID - 0)) |
    (1u << (Mips::CPU16RegsPlusSPRegClassID - 0)) |
    (1u << (Mips::CPU16RegsRegClassID - 0)) |
    (1u << (Mips::GPRMM16RegClassID - 0)) |
    (1u << (Mips::CPU16Regs_and_GPRMM16ZeroRegClassID - 0)) |
    (1u << (Mips::CPU16Regs_and_GPRMM16MovePPairSecondRegClassID - 0)) |
    (1u << (Mips::GPRMM16MovePPairFirst_and_GPRMM16MovePPairSecondRegClassID - 0)) |
    (1u << (Mips::GPRMM16MovePPairFirstRegClassID - 0)) |
    (1u << (Mips::GPRMM16MoveP_and_CPU16Regs_and_GPRMM16ZeroRegClassID - 0)) |
    (1u << (Mips::CPU16Regs_and_GPRMM16MovePRegClassID - 0)) |
    (1u << (Mips::CPUSPRegRegClassID - 0)) |
    (1u << (Mips::GPR32NONZERO_and_GPRMM16MovePRegClassID - 0)) |
    (1u << (Mips::GPRMM16MovePPairSecondRegClassID - 0)) |
    (1u << (Mips::CPURARegRegClassID - 0)) |
    (1u << (Mips::GPRMM16MovePRegClassID - 0)) |
    (1u << (Mips::GPRMM16MoveP_and_GPRMM16ZeroRegClassID - 0)) |
    (1u << (Mips::GPRMM16ZeroRegClassID - 0)) |
    0,
    // 32-63
    (1u << (Mips::SP32RegClassID - 32)) |
    (1u << (Mips::GP32RegClassID - 32)) |
    (1u << (Mips::GPR32ZERORegClassID - 32)) |
    0,
    // 64-95
    0,
};

RegisterBank FPRBRegBank(/* ID */ Mips::FPRBRegBankID, /* Name */ "FPRB", /* Size */ 128, /* CoveredRegClasses */ FPRBRegBankCoverageData, /* NumRegClasses */ 70);
RegisterBank GPRBRegBank(/* ID */ Mips::GPRBRegBankID, /* Name */ "GPRB", /* Size */ 32, /* CoveredRegClasses */ GPRBRegBankCoverageData, /* NumRegClasses */ 70);
} // end namespace Mips

RegisterBank *MipsGenRegisterBankInfo::RegBanks[] = {
    &Mips::FPRBRegBank,
    &Mips::GPRBRegBank,
};

MipsGenRegisterBankInfo::MipsGenRegisterBankInfo()
    : RegisterBankInfo(RegBanks, Mips::NumRegisterBanks) {
  // Assert that RegBank indices match their ID's
#ifndef NDEBUG
  unsigned Index = 0;
  for (const auto &RB : RegBanks)
    assert(Index++ == RB->getID() && "Index != ID");
#endif // NDEBUG
}
} // end namespace llvm
#endif // GET_TARGET_REGBANK_IMPL
