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
enum : unsigned {
  InvalidRegBankID = ~0u,
  GPRRegBankID = 0,
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
    (1u << (RISCV::GPRF16RegClassID - 0)) |
    (1u << (RISCV::GPRF32RegClassID - 0)) |
    (1u << (RISCV::GPRF64RegClassID - 0)) |
    (1u << (RISCV::GPRNoX0RegClassID - 0)) |
    (1u << (RISCV::GPRNoX0X2RegClassID - 0)) |
    (1u << (RISCV::GPRJALRRegClassID - 0)) |
    (1u << (RISCV::GPRTCRegClassID - 0)) |
    (1u << (RISCV::GPRC_and_GPRTCRegClassID - 0)) |
    (1u << (RISCV::GPRCRegClassID - 0)) |
    (1u << (RISCV::SPRegClassID - 0)) |
    (1u << (RISCV::GPRX0RegClassID - 0)) |
    0,
    // 32-63
    0,
    // 64-95
    0,
};

RegisterBank GPRRegBank(/* ID */ RISCV::GPRRegBankID, /* Name */ "GPRB", /* Size */ 32, /* CoveredRegClasses */ GPRRegBankCoverageData, /* NumRegClasses */ 76);
} // end namespace RISCV

RegisterBank *RISCVGenRegisterBankInfo::RegBanks[] = {
    &RISCV::GPRRegBank,
};

RISCVGenRegisterBankInfo::RISCVGenRegisterBankInfo()
    : RegisterBankInfo(RegBanks, RISCV::NumRegisterBanks) {
  // Assert that RegBank indices match their ID's
#ifndef NDEBUG
  for (auto RB : enumerate(RegBanks))
    assert(RB.index() == RB.value()->getID() && "Index != ID");
#endif // NDEBUG
}
} // end namespace llvm
#endif // GET_TARGET_REGBANK_IMPL
