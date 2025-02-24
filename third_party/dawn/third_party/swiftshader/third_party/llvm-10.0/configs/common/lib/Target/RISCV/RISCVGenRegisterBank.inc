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
namespace RISCV {
enum {
  GPRRegBankID,
  NumRegisterBanks,
};
} // end namespace RISCV
} // end namespace llvm
#endif // GET_REGBANK_DECLARATIONS

#ifdef GET_TARGET_REGBANK_CLASS
#undef GET_TARGET_REGBANK_CLASS
private:
  static RegisterBank *RegBanks[];

protected:
  RISCVGenRegisterBankInfo();

#endif // GET_TARGET_REGBANK_CLASS

#ifdef GET_TARGET_REGBANK_IMPL
#undef GET_TARGET_REGBANK_IMPL
namespace llvm {
namespace RISCV {
const uint32_t GPRRegBankCoverageData[] = {
    // 0-31
    (1u << (RISCV::GPRRegClassID - 0)) |
    (1u << (RISCV::GPRNoX0RegClassID - 0)) |
    (1u << (RISCV::GPRNoX0X2RegClassID - 0)) |
    (1u << (RISCV::GPRTCRegClassID - 0)) |
    (1u << (RISCV::GPRC_and_GPRTCRegClassID - 0)) |
    (1u << (RISCV::GPRCRegClassID - 0)) |
    (1u << (RISCV::SPRegClassID - 0)) |
    (1u << (RISCV::GPRX0RegClassID - 0)) |
    0,
};

RegisterBank GPRRegBank(/* ID */ RISCV::GPRRegBankID, /* Name */ "GPRB", /* Size */ 32, /* CoveredRegClasses */ GPRRegBankCoverageData, /* NumRegClasses */ 12);
} // end namespace RISCV

RegisterBank *RISCVGenRegisterBankInfo::RegBanks[] = {
    &RISCV::GPRRegBank,
};

RISCVGenRegisterBankInfo::RISCVGenRegisterBankInfo()
    : RegisterBankInfo(RegBanks, RISCV::NumRegisterBanks) {
  // Assert that RegBank indices match their ID's
#ifndef NDEBUG
  unsigned Index = 0;
  for (const auto &RB : RegBanks)
    assert(Index++ == RB->getID() && "Index != ID");
#endif // NDEBUG
}
} // end namespace llvm
#endif // GET_TARGET_REGBANK_IMPL
