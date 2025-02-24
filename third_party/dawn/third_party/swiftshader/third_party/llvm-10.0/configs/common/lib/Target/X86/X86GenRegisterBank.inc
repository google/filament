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
namespace X86 {
enum {
  GPRRegBankID,
  VECRRegBankID,
  NumRegisterBanks,
};
} // end namespace X86
} // end namespace llvm
#endif // GET_REGBANK_DECLARATIONS

#ifdef GET_TARGET_REGBANK_CLASS
#undef GET_TARGET_REGBANK_CLASS
private:
  static RegisterBank *RegBanks[];

protected:
  X86GenRegisterBankInfo();

#endif // GET_TARGET_REGBANK_CLASS

#ifdef GET_TARGET_REGBANK_IMPL
#undef GET_TARGET_REGBANK_IMPL
namespace llvm {
namespace X86 {
const uint32_t GPRRegBankCoverageData[] = {
    // 0-31
    (1u << (X86::GR8RegClassID - 0)) |
    (1u << (X86::GR16RegClassID - 0)) |
    (1u << (X86::LOW32_ADDR_ACCESS_RBPRegClassID - 0)) |
    (1u << (X86::LOW32_ADDR_ACCESSRegClassID - 0)) |
    (1u << (X86::LOW32_ADDR_ACCESS_RBP_with_sub_8bitRegClassID - 0)) |
    (1u << (X86::GR8_NOREXRegClassID - 0)) |
    (1u << (X86::GR8_ABCD_HRegClassID - 0)) |
    (1u << (X86::GR8_ABCD_LRegClassID - 0)) |
    (1u << (X86::GR16_NOREXRegClassID - 0)) |
    (1u << (X86::GR16_ABCDRegClassID - 0)) |
    0,
    // 32-63
    (1u << (X86::GR32RegClassID - 32)) |
    (1u << (X86::GR32_NOSPRegClassID - 32)) |
    (1u << (X86::LOW32_ADDR_ACCESS_RBP_with_sub_16bit_in_GR16_NOREXRegClassID - 32)) |
    (1u << (X86::GR32_NOREXRegClassID - 32)) |
    (1u << (X86::GR32_NOREX_NOSPRegClassID - 32)) |
    (1u << (X86::GR32_ABCDRegClassID - 32)) |
    (1u << (X86::GR32_TCRegClassID - 32)) |
    (1u << (X86::GR32_ABCD_and_GR32_TCRegClassID - 32)) |
    (1u << (X86::GR32_ADRegClassID - 32)) |
    (1u << (X86::GR32_DCRegClassID - 32)) |
    (1u << (X86::GR32_AD_and_GR32_DCRegClassID - 32)) |
    (1u << (X86::GR32_CBRegClassID - 32)) |
    (1u << (X86::GR32_CB_and_GR32_DCRegClassID - 32)) |
    (1u << (X86::GR32_SIDIRegClassID - 32)) |
    (1u << (X86::GR32_BSIRegClassID - 32)) |
    (1u << (X86::GR32_BSI_and_GR32_SIDIRegClassID - 32)) |
    (1u << (X86::GR32_DIBPRegClassID - 32)) |
    (1u << (X86::GR32_DIBP_and_GR32_SIDIRegClassID - 32)) |
    (1u << (X86::GR32_ABCD_and_GR32_BSIRegClassID - 32)) |
    (1u << (X86::GR32_BPSPRegClassID - 32)) |
    (1u << (X86::GR32_BPSP_and_GR32_DIBPRegClassID - 32)) |
    (1u << (X86::GR32_BPSP_and_GR32_TCRegClassID - 32)) |
    0,
    // 64-95
    (1u << (X86::GR64RegClassID - 64)) |
    (1u << (X86::GR64_with_sub_8bitRegClassID - 64)) |
    (1u << (X86::GR64_NOSPRegClassID - 64)) |
    (1u << (X86::GR64_NOSP_and_GR64_TCRegClassID - 64)) |
    (1u << (X86::GR64_TC_and_GR64_NOSP_and_GR64_TCW64RegClassID - 64)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_ABCD_and_GR32_TCRegClassID - 64)) |
    (1u << (X86::GR64_ADRegClassID - 64)) |
    (1u << (X86::GR64_NOREX_NOSP_and_GR64_TCRegClassID - 64)) |
    (1u << (X86::GR64_NOREX_NOSPRegClassID - 64)) |
    (1u << (X86::GR64_ABCDRegClassID - 64)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_BSIRegClassID - 64)) |
    (1u << (X86::GR64_NOSP_and_GR64_TCW64RegClassID - 64)) |
    (1u << (X86::GR64_TC_with_sub_8bitRegClassID - 64)) |
    (1u << (X86::GR64_TCW64_and_GR64_TC_with_sub_8bitRegClassID - 64)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_TCRegClassID - 64)) |
    (1u << (X86::GR64_TC_and_GR64_with_sub_16bit_in_GR16_NOREXRegClassID - 64)) |
    (1u << (X86::GR64_TCW64_with_sub_8bitRegClassID - 64)) |
    (1u << (X86::GR64_with_sub_16bit_in_GR16_NOREXRegClassID - 64)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_BPSPRegClassID - 64)) |
    (1u << (X86::GR64_TCRegClassID - 64)) |
    (1u << (X86::GR64_TC_and_GR64_TCW64RegClassID - 64)) |
    (1u << (X86::GR64_NOREX_and_GR64_TCW64RegClassID - 64)) |
    (1u << (X86::GR64_NOREX_and_GR64_TCRegClassID - 64)) |
    (1u << (X86::GR64_NOREXRegClassID - 64)) |
    (1u << (X86::GR64_and_LOW32_ADDR_ACCESS_RBPRegClassID - 64)) |
    (1u << (X86::GR64_TCW64RegClassID - 64)) |
    0,
    // 96-127
    (1u << (X86::GR64_with_sub_32bit_in_GR32_AD_and_GR32_DCRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_DCRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_CB_and_GR32_DCRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_SIDIRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_BSI_and_GR32_SIDIRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_DIBP_and_GR32_SIDIRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_CBRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_ABCD_and_GR32_BSIRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_DIBPRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_BPSP_and_GR32_DIBPRegClassID - 96)) |
    (1u << (X86::GR64_with_sub_32bit_in_GR32_BPSP_and_GR32_TCRegClassID - 96)) |
    (1u << (X86::GR64_and_LOW32_ADDR_ACCESSRegClassID - 96)) |
    0,
};
const uint32_t VECRRegBankCoverageData[] = {
    // 0-31
    (1u << (X86::FR32XRegClassID - 0)) |
    0,
    // 32-63
    (1u << (X86::FR32RegClassID - 32)) |
    0,
    // 64-95
    (1u << (X86::FR64XRegClassID - 64)) |
    (1u << (X86::FR64RegClassID - 64)) |
    0,
    // 96-127
    (1u << (X86::VR512RegClassID - 96)) |
    (1u << (X86::VR128XRegClassID - 96)) |
    (1u << (X86::VR256XRegClassID - 96)) |
    (1u << (X86::VR512_0_15RegClassID - 96)) |
    (1u << (X86::VR128RegClassID - 96)) |
    (1u << (X86::VR256RegClassID - 96)) |
    0,
};

RegisterBank GPRRegBank(/* ID */ X86::GPRRegBankID, /* Name */ "GPR", /* Size */ 64, /* CoveredRegClasses */ GPRRegBankCoverageData, /* NumRegClasses */ 118);
RegisterBank VECRRegBank(/* ID */ X86::VECRRegBankID, /* Name */ "VECR", /* Size */ 512, /* CoveredRegClasses */ VECRRegBankCoverageData, /* NumRegClasses */ 118);
} // end namespace X86

RegisterBank *X86GenRegisterBankInfo::RegBanks[] = {
    &X86::GPRRegBank,
    &X86::VECRRegBank,
};

X86GenRegisterBankInfo::X86GenRegisterBankInfo()
    : RegisterBankInfo(RegBanks, X86::NumRegisterBanks) {
  // Assert that RegBank indices match their ID's
#ifndef NDEBUG
  unsigned Index = 0;
  for (const auto &RB : RegBanks)
    assert(Index++ == RB->getID() && "Index != ID");
#endif // NDEBUG
}
} // end namespace llvm
#endif // GET_TARGET_REGBANK_IMPL
