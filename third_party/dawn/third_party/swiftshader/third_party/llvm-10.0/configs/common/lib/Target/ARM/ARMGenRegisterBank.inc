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
namespace ARM {
enum {
  FPRRegBankID,
  GPRRegBankID,
  NumRegisterBanks,
};
} // end namespace ARM
} // end namespace llvm
#endif // GET_REGBANK_DECLARATIONS

#ifdef GET_TARGET_REGBANK_CLASS
#undef GET_TARGET_REGBANK_CLASS
private:
  static RegisterBank *RegBanks[];

protected:
  ARMGenRegisterBankInfo();

#endif // GET_TARGET_REGBANK_CLASS

#ifdef GET_TARGET_REGBANK_IMPL
#undef GET_TARGET_REGBANK_IMPL
namespace llvm {
namespace ARM {
const uint32_t FPRRegBankCoverageData[] = {
    // 0-31
    (1u << (ARM::HPRRegClassID - 0)) |
    (1u << (ARM::SPRRegClassID - 0)) |
    (1u << (ARM::SPR_8RegClassID - 0)) |
    (1u << (ARM::FPWithVPRRegClassID - 0)) |
    (1u << (ARM::FPWithVPR_with_ssub_0RegClassID - 0)) |
    (1u << (ARM::FPWithVPR_with_ssub_0_with_ssub_0_in_SPR_8RegClassID - 0)) |
    0,
    // 32-63
    (1u << (ARM::DPRRegClassID - 32)) |
    (1u << (ARM::DPR_VFP2RegClassID - 32)) |
    (1u << (ARM::DPR_8RegClassID - 32)) |
    (1u << (ARM::QPRRegClassID - 32)) |
    (1u << (ARM::MQPRRegClassID - 32)) |
    (1u << (ARM::QPR_VFP2RegClassID - 32)) |
    (1u << (ARM::QPR_8RegClassID - 32)) |
    0,
    // 64-95
    0,
    // 96-127
    0,
};
const uint32_t GPRRegBankCoverageData[] = {
    // 0-31
    (1u << (ARM::GPRRegClassID - 0)) |
    (1u << (ARM::GPRnopcRegClassID - 0)) |
    (1u << (ARM::rGPRRegClassID - 0)) |
    (1u << (ARM::tGPRRegClassID - 0)) |
    (1u << (ARM::tGPR_and_tGPREvenRegClassID - 0)) |
    (1u << (ARM::tGPREven_and_tGPR_and_tcGPRRegClassID - 0)) |
    (1u << (ARM::tGPR_and_tGPROddRegClassID - 0)) |
    (1u << (ARM::tGPROdd_and_tcGPRRegClassID - 0)) |
    (1u << (ARM::tGPR_and_tcGPRRegClassID - 0)) |
    (1u << (ARM::tGPREvenRegClassID - 0)) |
    (1u << (ARM::hGPR_and_tGPREvenRegClassID - 0)) |
    (1u << (ARM::GPRlrRegClassID - 0)) |
    (1u << (ARM::tGPREven_and_tcGPRRegClassID - 0)) |
    (1u << (ARM::GPRwithAPSRnosp_and_hGPRRegClassID - 0)) |
    (1u << (ARM::hGPR_and_tGPROddRegClassID - 0)) |
    (1u << (ARM::tGPROddRegClassID - 0)) |
    (1u << (ARM::tcGPRRegClassID - 0)) |
    (1u << (ARM::GPRnopc_and_hGPRRegClassID - 0)) |
    (1u << (ARM::GPRspRegClassID - 0)) |
    (1u << (ARM::tGPRwithpcRegClassID - 0)) |
    (1u << (ARM::hGPRRegClassID - 0)) |
    (1u << (ARM::GPRwithAPSRRegClassID - 0)) |
    0,
    // 32-63
    (1u << (ARM::hGPR_and_tcGPRRegClassID - 32)) |
    (1u << (ARM::hGPR_and_tGPRwithpcRegClassID - 32)) |
    0,
    // 64-95
    0,
    // 96-127
    0,
};

RegisterBank FPRRegBank(/* ID */ ARM::FPRRegBankID, /* Name */ "FPRB", /* Size */ 128, /* CoveredRegClasses */ FPRRegBankCoverageData, /* NumRegClasses */ 122);
RegisterBank GPRRegBank(/* ID */ ARM::GPRRegBankID, /* Name */ "GPRB", /* Size */ 32, /* CoveredRegClasses */ GPRRegBankCoverageData, /* NumRegClasses */ 122);
} // end namespace ARM

RegisterBank *ARMGenRegisterBankInfo::RegBanks[] = {
    &ARM::FPRRegBank,
    &ARM::GPRRegBank,
};

ARMGenRegisterBankInfo::ARMGenRegisterBankInfo()
    : RegisterBankInfo(RegBanks, ARM::NumRegisterBanks) {
  // Assert that RegBank indices match their ID's
#ifndef NDEBUG
  unsigned Index = 0;
  for (const auto &RB : RegBanks)
    assert(Index++ == RB->getID() && "Index != ID");
#endif // NDEBUG
}
} // end namespace llvm
#endif // GET_TARGET_REGBANK_IMPL
