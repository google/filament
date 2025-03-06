//===----- LegalizeIntegerTypes.cpp - Legalization of integer types -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements integer type expansion and promotion for LegalizeTypes.
// Promotion is the act of changing a computation in an illegal type into a
// computation in a larger type.  For example, implementing i8 arithmetic in an
// i32 register (often needed on powerpc).
// Expansion is the act of changing a computation in an illegal type into a
// computation in two identical registers of a smaller type.  For example,
// implementing i64 arithmetic in two i32 registers (often needed on 32-bit
// targets).
//
//===----------------------------------------------------------------------===//

#include "LegalizeTypes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "legalize-types"

//===----------------------------------------------------------------------===//
//  Integer Result Promotion
//===----------------------------------------------------------------------===//

/// PromoteIntegerResult - This method is called when a result of a node is
/// found to be in need of promotion to a larger type.  At this point, the node
/// may also have invalid operands or may have other results that need
/// expansion, we just know that (at least) one result needs promotion.
void DAGTypeLegalizer::PromoteIntegerResult(SDNode *N, unsigned ResNo) {
  DEBUG(dbgs() << "Promote integer result: "; N->dump(&DAG); dbgs() << "\n");
  SDValue Res = SDValue();

  // See if the target wants to custom expand this node.
  if (CustomLowerNode(N, N->getValueType(ResNo), true))
    return;

  switch (N->getOpcode()) {
  default:
#ifndef NDEBUG
    dbgs() << "PromoteIntegerResult #" << ResNo << ": ";
    N->dump(&DAG); dbgs() << "\n";
#endif
    llvm_unreachable("Do not know how to promote this operator!");
  case ISD::MERGE_VALUES:Res = PromoteIntRes_MERGE_VALUES(N, ResNo); break;
  case ISD::AssertSext:  Res = PromoteIntRes_AssertSext(N); break;
  case ISD::AssertZext:  Res = PromoteIntRes_AssertZext(N); break;
  case ISD::BITCAST:     Res = PromoteIntRes_BITCAST(N); break;
  case ISD::BSWAP:       Res = PromoteIntRes_BSWAP(N); break;
  case ISD::BUILD_PAIR:  Res = PromoteIntRes_BUILD_PAIR(N); break;
  case ISD::Constant:    Res = PromoteIntRes_Constant(N); break;
  case ISD::CONVERT_RNDSAT:
                         Res = PromoteIntRes_CONVERT_RNDSAT(N); break;
  case ISD::CTLZ_ZERO_UNDEF:
  case ISD::CTLZ:        Res = PromoteIntRes_CTLZ(N); break;
  case ISD::CTPOP:       Res = PromoteIntRes_CTPOP(N); break;
  case ISD::CTTZ_ZERO_UNDEF:
  case ISD::CTTZ:        Res = PromoteIntRes_CTTZ(N); break;
  case ISD::EXTRACT_VECTOR_ELT:
                         Res = PromoteIntRes_EXTRACT_VECTOR_ELT(N); break;
  case ISD::LOAD:        Res = PromoteIntRes_LOAD(cast<LoadSDNode>(N));break;
  case ISD::MLOAD:       Res = PromoteIntRes_MLOAD(cast<MaskedLoadSDNode>(N));break;
  case ISD::SELECT:      Res = PromoteIntRes_SELECT(N); break;
  case ISD::VSELECT:     Res = PromoteIntRes_VSELECT(N); break;
  case ISD::SELECT_CC:   Res = PromoteIntRes_SELECT_CC(N); break;
  case ISD::SETCC:       Res = PromoteIntRes_SETCC(N); break;
  case ISD::SMIN:
  case ISD::SMAX:
  case ISD::UMIN:
  case ISD::UMAX:        Res = PromoteIntRes_SimpleIntBinOp(N); break;
  case ISD::SHL:         Res = PromoteIntRes_SHL(N); break;
  case ISD::SIGN_EXTEND_INREG:
                         Res = PromoteIntRes_SIGN_EXTEND_INREG(N); break;
  case ISD::SRA:         Res = PromoteIntRes_SRA(N); break;
  case ISD::SRL:         Res = PromoteIntRes_SRL(N); break;
  case ISD::TRUNCATE:    Res = PromoteIntRes_TRUNCATE(N); break;
  case ISD::UNDEF:       Res = PromoteIntRes_UNDEF(N); break;
  case ISD::VAARG:       Res = PromoteIntRes_VAARG(N); break;

  case ISD::EXTRACT_SUBVECTOR:
                         Res = PromoteIntRes_EXTRACT_SUBVECTOR(N); break;
  case ISD::VECTOR_SHUFFLE:
                         Res = PromoteIntRes_VECTOR_SHUFFLE(N); break;
  case ISD::INSERT_VECTOR_ELT:
                         Res = PromoteIntRes_INSERT_VECTOR_ELT(N); break;
  case ISD::BUILD_VECTOR:
                         Res = PromoteIntRes_BUILD_VECTOR(N); break;
  case ISD::SCALAR_TO_VECTOR:
                         Res = PromoteIntRes_SCALAR_TO_VECTOR(N); break;
  case ISD::CONCAT_VECTORS:
                         Res = PromoteIntRes_CONCAT_VECTORS(N); break;

  case ISD::SIGN_EXTEND:
  case ISD::ZERO_EXTEND:
  case ISD::ANY_EXTEND:  Res = PromoteIntRes_INT_EXTEND(N); break;

  case ISD::FP_TO_SINT:
  case ISD::FP_TO_UINT:  Res = PromoteIntRes_FP_TO_XINT(N); break;

  case ISD::FP_TO_FP16:  Res = PromoteIntRes_FP_TO_FP16(N); break;

  case ISD::AND:
  case ISD::OR:
  case ISD::XOR:
  case ISD::ADD:
  case ISD::SUB:
  case ISD::MUL:         Res = PromoteIntRes_SimpleIntBinOp(N); break;

  case ISD::SDIV:
  case ISD::SREM:        Res = PromoteIntRes_SDIV(N); break;

  case ISD::UDIV:
  case ISD::UREM:        Res = PromoteIntRes_UDIV(N); break;

  case ISD::SADDO:
  case ISD::SSUBO:       Res = PromoteIntRes_SADDSUBO(N, ResNo); break;
  case ISD::UADDO:
  case ISD::USUBO:       Res = PromoteIntRes_UADDSUBO(N, ResNo); break;
  case ISD::SMULO:
  case ISD::UMULO:       Res = PromoteIntRes_XMULO(N, ResNo); break;

  case ISD::ATOMIC_LOAD:
    Res = PromoteIntRes_Atomic0(cast<AtomicSDNode>(N)); break;

  case ISD::ATOMIC_LOAD_ADD:
  case ISD::ATOMIC_LOAD_SUB:
  case ISD::ATOMIC_LOAD_AND:
  case ISD::ATOMIC_LOAD_OR:
  case ISD::ATOMIC_LOAD_XOR:
  case ISD::ATOMIC_LOAD_NAND:
  case ISD::ATOMIC_LOAD_MIN:
  case ISD::ATOMIC_LOAD_MAX:
  case ISD::ATOMIC_LOAD_UMIN:
  case ISD::ATOMIC_LOAD_UMAX:
  case ISD::ATOMIC_SWAP:
    Res = PromoteIntRes_Atomic1(cast<AtomicSDNode>(N)); break;

  case ISD::ATOMIC_CMP_SWAP:
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS:
    Res = PromoteIntRes_AtomicCmpSwap(cast<AtomicSDNode>(N), ResNo);
    break;
  }

  // If the result is null then the sub-method took care of registering it.
  if (Res.getNode())
    SetPromotedInteger(SDValue(N, ResNo), Res);
}

SDValue DAGTypeLegalizer::PromoteIntRes_MERGE_VALUES(SDNode *N,
                                                     unsigned ResNo) {
  SDValue Op = DisintegrateMERGE_VALUES(N, ResNo);
  return GetPromotedInteger(Op);
}

SDValue DAGTypeLegalizer::PromoteIntRes_AssertSext(SDNode *N) {
  // Sign-extend the new bits, and continue the assertion.
  SDValue Op = SExtPromotedInteger(N->getOperand(0));
  return DAG.getNode(ISD::AssertSext, SDLoc(N),
                     Op.getValueType(), Op, N->getOperand(1));
}

SDValue DAGTypeLegalizer::PromoteIntRes_AssertZext(SDNode *N) {
  // Zero the new bits, and continue the assertion.
  SDValue Op = ZExtPromotedInteger(N->getOperand(0));
  return DAG.getNode(ISD::AssertZext, SDLoc(N),
                     Op.getValueType(), Op, N->getOperand(1));
}

SDValue DAGTypeLegalizer::PromoteIntRes_Atomic0(AtomicSDNode *N) {
  EVT ResVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDValue Res = DAG.getAtomic(N->getOpcode(), SDLoc(N),
                              N->getMemoryVT(), ResVT,
                              N->getChain(), N->getBasePtr(),
                              N->getMemOperand(), N->getOrdering(),
                              N->getSynchScope());
  // Legalized the chain result - switch anything that used the old chain to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Res.getValue(1));
  return Res;
}

SDValue DAGTypeLegalizer::PromoteIntRes_Atomic1(AtomicSDNode *N) {
  SDValue Op2 = GetPromotedInteger(N->getOperand(2));
  SDValue Res = DAG.getAtomic(N->getOpcode(), SDLoc(N),
                              N->getMemoryVT(),
                              N->getChain(), N->getBasePtr(),
                              Op2, N->getMemOperand(), N->getOrdering(),
                              N->getSynchScope());
  // Legalized the chain result - switch anything that used the old chain to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Res.getValue(1));
  return Res;
}

SDValue DAGTypeLegalizer::PromoteIntRes_AtomicCmpSwap(AtomicSDNode *N,
                                                      unsigned ResNo) {
  if (ResNo == 1) {
    assert(N->getOpcode() == ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS);
    EVT SVT = getSetCCResultType(N->getOperand(2).getValueType());
    EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(1));

    // Only use the result of getSetCCResultType if it is legal,
    // otherwise just use the promoted result type (NVT).
    if (!TLI.isTypeLegal(SVT))
      SVT = NVT;

    SDVTList VTs = DAG.getVTList(N->getValueType(0), SVT, MVT::Other);
    SDValue Res = DAG.getAtomicCmpSwap(
        ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS, SDLoc(N), N->getMemoryVT(), VTs,
        N->getChain(), N->getBasePtr(), N->getOperand(2), N->getOperand(3),
        N->getMemOperand(), N->getSuccessOrdering(), N->getFailureOrdering(),
        N->getSynchScope());
    ReplaceValueWith(SDValue(N, 0), Res.getValue(0));
    ReplaceValueWith(SDValue(N, 2), Res.getValue(2));
    return Res.getValue(1);
  }

  SDValue Op2 = GetPromotedInteger(N->getOperand(2));
  SDValue Op3 = GetPromotedInteger(N->getOperand(3));
  SDVTList VTs =
      DAG.getVTList(Op2.getValueType(), N->getValueType(1), MVT::Other);
  SDValue Res = DAG.getAtomicCmpSwap(
      N->getOpcode(), SDLoc(N), N->getMemoryVT(), VTs, N->getChain(),
      N->getBasePtr(), Op2, Op3, N->getMemOperand(), N->getSuccessOrdering(),
      N->getFailureOrdering(), N->getSynchScope());
  // Update the use to N with the newly created Res.
  for (unsigned i = 1, NumResults = N->getNumValues(); i < NumResults; ++i)
    ReplaceValueWith(SDValue(N, i), Res.getValue(i));
  return Res;
}

SDValue DAGTypeLegalizer::PromoteIntRes_BITCAST(SDNode *N) {
  SDValue InOp = N->getOperand(0);
  EVT InVT = InOp.getValueType();
  EVT NInVT = TLI.getTypeToTransformTo(*DAG.getContext(), InVT);
  EVT OutVT = N->getValueType(0);
  EVT NOutVT = TLI.getTypeToTransformTo(*DAG.getContext(), OutVT);
  SDLoc dl(N);

  switch (getTypeAction(InVT)) {
  case TargetLowering::TypeLegal:
    break;
  case TargetLowering::TypePromoteInteger:
    if (NOutVT.bitsEq(NInVT) && !NOutVT.isVector() && !NInVT.isVector())
      // The input promotes to the same size.  Convert the promoted value.
      return DAG.getNode(ISD::BITCAST, dl, NOutVT, GetPromotedInteger(InOp));
    break;
  case TargetLowering::TypeSoftenFloat:
    // Promote the integer operand by hand.
    return DAG.getNode(ISD::ANY_EXTEND, dl, NOutVT, GetSoftenedFloat(InOp));
  case TargetLowering::TypePromoteFloat: {
    // Convert the promoted float by hand.
    if (NOutVT.bitsEq(NInVT)) {
      SDValue PromotedOp = GetPromotedFloat(InOp);
      SDValue Trunc = DAG.getNode(ISD::FP_TO_FP16, dl, NOutVT, PromotedOp);
      return DAG.getNode(ISD::AssertZext, dl, NOutVT, Trunc,
                         DAG.getValueType(OutVT));
    }
    break;
  }
  case TargetLowering::TypeExpandInteger:
  case TargetLowering::TypeExpandFloat:
    break;
  case TargetLowering::TypeScalarizeVector:
    // Convert the element to an integer and promote it by hand.
    if (!NOutVT.isVector())
      return DAG.getNode(ISD::ANY_EXTEND, dl, NOutVT,
                         BitConvertToInteger(GetScalarizedVector(InOp)));
    break;
  case TargetLowering::TypeSplitVector: {
    // For example, i32 = BITCAST v2i16 on alpha.  Convert the split
    // pieces of the input into integers and reassemble in the final type.
    SDValue Lo, Hi;
    GetSplitVector(N->getOperand(0), Lo, Hi);
    Lo = BitConvertToInteger(Lo);
    Hi = BitConvertToInteger(Hi);

    if (DAG.getDataLayout().isBigEndian())
      std::swap(Lo, Hi);

    InOp = DAG.getNode(ISD::ANY_EXTEND, dl,
                       EVT::getIntegerVT(*DAG.getContext(),
                                         NOutVT.getSizeInBits()),
                       JoinIntegers(Lo, Hi));
    return DAG.getNode(ISD::BITCAST, dl, NOutVT, InOp);
  }
  case TargetLowering::TypeWidenVector:
    // The input is widened to the same size. Convert to the widened value.
    // Make sure that the outgoing value is not a vector, because this would
    // make us bitcast between two vectors which are legalized in different ways.
    if (NOutVT.bitsEq(NInVT) && !NOutVT.isVector())
      return DAG.getNode(ISD::BITCAST, dl, NOutVT, GetWidenedVector(InOp));
  }

  return DAG.getNode(ISD::ANY_EXTEND, dl, NOutVT,
                     CreateStackStoreLoad(InOp, OutVT));
}

SDValue DAGTypeLegalizer::PromoteIntRes_BSWAP(SDNode *N) {
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  EVT OVT = N->getValueType(0);
  EVT NVT = Op.getValueType();
  SDLoc dl(N);

  unsigned DiffBits = NVT.getScalarSizeInBits() - OVT.getScalarSizeInBits();
  return DAG.getNode(
      ISD::SRL, dl, NVT, DAG.getNode(ISD::BSWAP, dl, NVT, Op),
      DAG.getConstant(DiffBits, dl,
                      TLI.getShiftAmountTy(NVT, DAG.getDataLayout())));
}

SDValue DAGTypeLegalizer::PromoteIntRes_BUILD_PAIR(SDNode *N) {
  // The pair element type may be legal, or may not promote to the same type as
  // the result, for example i14 = BUILD_PAIR (i7, i7).  Handle all cases.
  return DAG.getNode(ISD::ANY_EXTEND, SDLoc(N),
                     TLI.getTypeToTransformTo(*DAG.getContext(),
                     N->getValueType(0)), JoinIntegers(N->getOperand(0),
                     N->getOperand(1)));
}

SDValue DAGTypeLegalizer::PromoteIntRes_Constant(SDNode *N) {
  EVT VT = N->getValueType(0);
  // FIXME there is no actual debug info here
  SDLoc dl(N);
  // Zero extend things like i1, sign extend everything else.  It shouldn't
  // matter in theory which one we pick, but this tends to give better code?
  unsigned Opc = VT.isByteSized() ? ISD::SIGN_EXTEND : ISD::ZERO_EXTEND;
  SDValue Result = DAG.getNode(Opc, dl,
                               TLI.getTypeToTransformTo(*DAG.getContext(), VT),
                               SDValue(N, 0));
  assert(isa<ConstantSDNode>(Result) && "Didn't constant fold ext?");
  return Result;
}

SDValue DAGTypeLegalizer::PromoteIntRes_CONVERT_RNDSAT(SDNode *N) {
  ISD::CvtCode CvtCode = cast<CvtRndSatSDNode>(N)->getCvtCode();
  assert ((CvtCode == ISD::CVT_SS || CvtCode == ISD::CVT_SU ||
           CvtCode == ISD::CVT_US || CvtCode == ISD::CVT_UU ||
           CvtCode == ISD::CVT_SF || CvtCode == ISD::CVT_UF) &&
          "can only promote integers");
  EVT OutVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  return DAG.getConvertRndSat(OutVT, SDLoc(N), N->getOperand(0),
                              N->getOperand(1), N->getOperand(2),
                              N->getOperand(3), N->getOperand(4), CvtCode);
}

SDValue DAGTypeLegalizer::PromoteIntRes_CTLZ(SDNode *N) {
  // Zero extend to the promoted type and do the count there.
  SDValue Op = ZExtPromotedInteger(N->getOperand(0));
  SDLoc dl(N);
  EVT OVT = N->getValueType(0);
  EVT NVT = Op.getValueType();
  Op = DAG.getNode(N->getOpcode(), dl, NVT, Op);
  // Subtract off the extra leading bits in the bigger type.
  return DAG.getNode(
      ISD::SUB, dl, NVT, Op,
      DAG.getConstant(NVT.getScalarSizeInBits() - OVT.getScalarSizeInBits(), dl,
                      NVT));
}

SDValue DAGTypeLegalizer::PromoteIntRes_CTPOP(SDNode *N) {
  // Zero extend to the promoted type and do the count there.
  SDValue Op = ZExtPromotedInteger(N->getOperand(0));
  return DAG.getNode(ISD::CTPOP, SDLoc(N), Op.getValueType(), Op);
}

SDValue DAGTypeLegalizer::PromoteIntRes_CTTZ(SDNode *N) {
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  EVT OVT = N->getValueType(0);
  EVT NVT = Op.getValueType();
  SDLoc dl(N);
  if (N->getOpcode() == ISD::CTTZ) {
    // The count is the same in the promoted type except if the original
    // value was zero.  This can be handled by setting the bit just off
    // the top of the original type.
    auto TopBit = APInt::getOneBitSet(NVT.getScalarSizeInBits(),
                                      OVT.getScalarSizeInBits());
    Op = DAG.getNode(ISD::OR, dl, NVT, Op, DAG.getConstant(TopBit, dl, NVT));
  }
  return DAG.getNode(N->getOpcode(), dl, NVT, Op);
}

SDValue DAGTypeLegalizer::PromoteIntRes_EXTRACT_VECTOR_ELT(SDNode *N) {
  SDLoc dl(N);
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  return DAG.getNode(ISD::EXTRACT_VECTOR_ELT, dl, NVT, N->getOperand(0),
                     N->getOperand(1));
}

SDValue DAGTypeLegalizer::PromoteIntRes_FP_TO_XINT(SDNode *N) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  unsigned NewOpc = N->getOpcode();
  SDLoc dl(N);

  // If we're promoting a UINT to a larger size and the larger FP_TO_UINT is
  // not Legal, check to see if we can use FP_TO_SINT instead.  (If both UINT
  // and SINT conversions are Custom, there is no way to tell which is
  // preferable. We choose SINT because that's the right thing on PPC.)
  if (N->getOpcode() == ISD::FP_TO_UINT &&
      !TLI.isOperationLegal(ISD::FP_TO_UINT, NVT) &&
      TLI.isOperationLegalOrCustom(ISD::FP_TO_SINT, NVT))
    NewOpc = ISD::FP_TO_SINT;

  SDValue Res = DAG.getNode(NewOpc, dl, NVT, N->getOperand(0));

  // Assert that the converted value fits in the original type.  If it doesn't
  // (eg: because the value being converted is too big), then the result of the
  // original operation was undefined anyway, so the assert is still correct.
  return DAG.getNode(N->getOpcode() == ISD::FP_TO_UINT ?
                     ISD::AssertZext : ISD::AssertSext, dl, NVT, Res,
                     DAG.getValueType(N->getValueType(0).getScalarType()));
}

SDValue DAGTypeLegalizer::PromoteIntRes_FP_TO_FP16(SDNode *N) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDLoc dl(N);

  SDValue Res = DAG.getNode(N->getOpcode(), dl, NVT, N->getOperand(0));

  return DAG.getNode(ISD::AssertZext, dl,
                     NVT, Res, DAG.getValueType(N->getValueType(0)));
}

SDValue DAGTypeLegalizer::PromoteIntRes_INT_EXTEND(SDNode *N) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDLoc dl(N);

  if (getTypeAction(N->getOperand(0).getValueType())
      == TargetLowering::TypePromoteInteger) {
    SDValue Res = GetPromotedInteger(N->getOperand(0));
    assert(Res.getValueType().bitsLE(NVT) && "Extension doesn't make sense!");

    // If the result and operand types are the same after promotion, simplify
    // to an in-register extension.
    if (NVT == Res.getValueType()) {
      // The high bits are not guaranteed to be anything.  Insert an extend.
      if (N->getOpcode() == ISD::SIGN_EXTEND)
        return DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, NVT, Res,
                           DAG.getValueType(N->getOperand(0).getValueType()));
      if (N->getOpcode() == ISD::ZERO_EXTEND)
        return DAG.getZeroExtendInReg(Res, dl,
                      N->getOperand(0).getValueType().getScalarType());
      assert(N->getOpcode() == ISD::ANY_EXTEND && "Unknown integer extension!");
      return Res;
    }
  }

  // Otherwise, just extend the original operand all the way to the larger type.
  return DAG.getNode(N->getOpcode(), dl, NVT, N->getOperand(0));
}

SDValue DAGTypeLegalizer::PromoteIntRes_LOAD(LoadSDNode *N) {
  assert(ISD::isUNINDEXEDLoad(N) && "Indexed load during type legalization!");
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  ISD::LoadExtType ExtType =
    ISD::isNON_EXTLoad(N) ? ISD::EXTLOAD : N->getExtensionType();
  SDLoc dl(N);
  SDValue Res = DAG.getExtLoad(ExtType, dl, NVT, N->getChain(), N->getBasePtr(),
                               N->getMemoryVT(), N->getMemOperand());

  // Legalized the chain result - switch anything that used the old chain to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Res.getValue(1));
  return Res;
}

SDValue DAGTypeLegalizer::PromoteIntRes_MLOAD(MaskedLoadSDNode *N) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDValue ExtSrc0 = GetPromotedInteger(N->getSrc0());

  SDValue Mask = N->getMask();
  EVT NewMaskVT = getSetCCResultType(NVT);
  if (NewMaskVT != N->getMask().getValueType())
    Mask = PromoteTargetBoolean(Mask, NewMaskVT);
  SDLoc dl(N);

  SDValue Res = DAG.getMaskedLoad(NVT, dl, N->getChain(), N->getBasePtr(),
                                  Mask, ExtSrc0, N->getMemoryVT(),
                                  N->getMemOperand(), ISD::SEXTLOAD);
  // Legalized the chain result - switch anything that used the old chain to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Res.getValue(1));
  return Res;
}
/// Promote the overflow flag of an overflowing arithmetic node.
SDValue DAGTypeLegalizer::PromoteIntRes_Overflow(SDNode *N) {
  // Simply change the return type of the boolean result.
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(1));
  EVT ValueVTs[] = { N->getValueType(0), NVT };
  SDValue Ops[] = { N->getOperand(0), N->getOperand(1) };
  SDValue Res = DAG.getNode(N->getOpcode(), SDLoc(N),
                            DAG.getVTList(ValueVTs), Ops);

  // Modified the sum result - switch anything that used the old sum to use
  // the new one.
  ReplaceValueWith(SDValue(N, 0), Res);

  return SDValue(Res.getNode(), 1);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SADDSUBO(SDNode *N, unsigned ResNo) {
  if (ResNo == 1)
    return PromoteIntRes_Overflow(N);

  // The operation overflowed iff the result in the larger type is not the
  // sign extension of its truncation to the original type.
  SDValue LHS = SExtPromotedInteger(N->getOperand(0));
  SDValue RHS = SExtPromotedInteger(N->getOperand(1));
  EVT OVT = N->getOperand(0).getValueType();
  EVT NVT = LHS.getValueType();
  SDLoc dl(N);

  // Do the arithmetic in the larger type.
  unsigned Opcode = N->getOpcode() == ISD::SADDO ? ISD::ADD : ISD::SUB;
  SDValue Res = DAG.getNode(Opcode, dl, NVT, LHS, RHS);

  // Calculate the overflow flag: sign extend the arithmetic result from
  // the original type.
  SDValue Ofl = DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, NVT, Res,
                            DAG.getValueType(OVT));
  // Overflowed if and only if this is not equal to Res.
  Ofl = DAG.getSetCC(dl, N->getValueType(1), Ofl, Res, ISD::SETNE);

  // Use the calculated overflow everywhere.
  ReplaceValueWith(SDValue(N, 1), Ofl);

  return Res;
}

SDValue DAGTypeLegalizer::PromoteIntRes_SDIV(SDNode *N) {
  // Sign extend the input.
  SDValue LHS = SExtPromotedInteger(N->getOperand(0));
  SDValue RHS = SExtPromotedInteger(N->getOperand(1));
  return DAG.getNode(N->getOpcode(), SDLoc(N),
                     LHS.getValueType(), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SELECT(SDNode *N) {
  SDValue LHS = GetPromotedInteger(N->getOperand(1));
  SDValue RHS = GetPromotedInteger(N->getOperand(2));
  return DAG.getSelect(SDLoc(N),
                       LHS.getValueType(), N->getOperand(0), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_VSELECT(SDNode *N) {
  SDValue Mask = N->getOperand(0);
  EVT OpTy = N->getOperand(1).getValueType();

  // Promote all the way up to the canonical SetCC type.
  Mask = PromoteTargetBoolean(Mask, OpTy);
  SDValue LHS = GetPromotedInteger(N->getOperand(1));
  SDValue RHS = GetPromotedInteger(N->getOperand(2));
  return DAG.getNode(ISD::VSELECT, SDLoc(N),
                     LHS.getValueType(), Mask, LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SELECT_CC(SDNode *N) {
  SDValue LHS = GetPromotedInteger(N->getOperand(2));
  SDValue RHS = GetPromotedInteger(N->getOperand(3));
  return DAG.getNode(ISD::SELECT_CC, SDLoc(N),
                     LHS.getValueType(), N->getOperand(0),
                     N->getOperand(1), LHS, RHS, N->getOperand(4));
}

SDValue DAGTypeLegalizer::PromoteIntRes_SETCC(SDNode *N) {
  EVT SVT = getSetCCResultType(N->getOperand(0).getValueType());

  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));

  // Only use the result of getSetCCResultType if it is legal,
  // otherwise just use the promoted result type (NVT).
  if (!TLI.isTypeLegal(SVT))
    SVT = NVT;

  SDLoc dl(N);
  assert(SVT.isVector() == N->getOperand(0).getValueType().isVector() &&
         "Vector compare must return a vector result!");

  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  if (LHS.getValueType() != RHS.getValueType()) {
    if (getTypeAction(LHS.getValueType()) == TargetLowering::TypePromoteInteger &&
        !LHS.getValueType().isVector())
      LHS = GetPromotedInteger(LHS);
    if (getTypeAction(RHS.getValueType()) == TargetLowering::TypePromoteInteger &&
        !RHS.getValueType().isVector())
      RHS = GetPromotedInteger(RHS);
  }

  // Get the SETCC result using the canonical SETCC type.
  SDValue SetCC = DAG.getNode(N->getOpcode(), dl, SVT, LHS, RHS,
                              N->getOperand(2));

  assert(NVT.bitsLE(SVT) && "Integer type overpromoted?");
  // Convert to the expected type.
  return DAG.getNode(ISD::TRUNCATE, dl, NVT, SetCC);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SHL(SDNode *N) {
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  if (getTypeAction(LHS.getValueType()) == TargetLowering::TypePromoteInteger)
    LHS = GetPromotedInteger(LHS);
  if (getTypeAction(RHS.getValueType()) == TargetLowering::TypePromoteInteger)
    RHS = ZExtPromotedInteger(RHS);
  return DAG.getNode(ISD::SHL, SDLoc(N), LHS.getValueType(), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SIGN_EXTEND_INREG(SDNode *N) {
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  return DAG.getNode(ISD::SIGN_EXTEND_INREG, SDLoc(N),
                     Op.getValueType(), Op, N->getOperand(1));
}

SDValue DAGTypeLegalizer::PromoteIntRes_SimpleIntBinOp(SDNode *N) {
  // The input may have strange things in the top bits of the registers, but
  // these operations don't care.  They may have weird bits going out, but
  // that too is okay if they are integer operations.
  SDValue LHS = GetPromotedInteger(N->getOperand(0));
  SDValue RHS = GetPromotedInteger(N->getOperand(1));
  return DAG.getNode(N->getOpcode(), SDLoc(N),
                     LHS.getValueType(), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SRA(SDNode *N) {
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  // The input value must be properly sign extended.
  if (getTypeAction(LHS.getValueType()) == TargetLowering::TypePromoteInteger)
    LHS = SExtPromotedInteger(LHS);
  if (getTypeAction(RHS.getValueType()) == TargetLowering::TypePromoteInteger)
    RHS = ZExtPromotedInteger(RHS);
  return DAG.getNode(ISD::SRA, SDLoc(N), LHS.getValueType(), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SRL(SDNode *N) {
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  // The input value must be properly zero extended.
  if (getTypeAction(LHS.getValueType()) == TargetLowering::TypePromoteInteger)
    LHS = ZExtPromotedInteger(LHS);
  if (getTypeAction(RHS.getValueType()) == TargetLowering::TypePromoteInteger)
    RHS = ZExtPromotedInteger(RHS);
  return DAG.getNode(ISD::SRL, SDLoc(N), LHS.getValueType(), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_TRUNCATE(SDNode *N) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDValue Res;
  SDValue InOp = N->getOperand(0);
  SDLoc dl(N);

  switch (getTypeAction(InOp.getValueType())) {
  default: llvm_unreachable("Unknown type action!");
  case TargetLowering::TypeLegal:
  case TargetLowering::TypeExpandInteger:
    Res = InOp;
    break;
  case TargetLowering::TypePromoteInteger:
    Res = GetPromotedInteger(InOp);
    break;
  case TargetLowering::TypeSplitVector:
    EVT InVT = InOp.getValueType();
    assert(InVT.isVector() && "Cannot split scalar types");
    unsigned NumElts = InVT.getVectorNumElements();
    assert(NumElts == NVT.getVectorNumElements() &&
           "Dst and Src must have the same number of elements");
    assert(isPowerOf2_32(NumElts) &&
           "Promoted vector type must be a power of two");

    SDValue EOp1, EOp2;
    GetSplitVector(InOp, EOp1, EOp2);

    EVT HalfNVT = EVT::getVectorVT(*DAG.getContext(), NVT.getScalarType(),
                                   NumElts/2);
    EOp1 = DAG.getNode(ISD::TRUNCATE, dl, HalfNVT, EOp1);
    EOp2 = DAG.getNode(ISD::TRUNCATE, dl, HalfNVT, EOp2);

    return DAG.getNode(ISD::CONCAT_VECTORS, dl, NVT, EOp1, EOp2);
  }

  // Truncate to NVT instead of VT
  return DAG.getNode(ISD::TRUNCATE, dl, NVT, Res);
}

SDValue DAGTypeLegalizer::PromoteIntRes_UADDSUBO(SDNode *N, unsigned ResNo) {
  if (ResNo == 1)
    return PromoteIntRes_Overflow(N);

  // The operation overflowed iff the result in the larger type is not the
  // zero extension of its truncation to the original type.
  SDValue LHS = ZExtPromotedInteger(N->getOperand(0));
  SDValue RHS = ZExtPromotedInteger(N->getOperand(1));
  EVT OVT = N->getOperand(0).getValueType();
  EVT NVT = LHS.getValueType();
  SDLoc dl(N);

  // Do the arithmetic in the larger type.
  unsigned Opcode = N->getOpcode() == ISD::UADDO ? ISD::ADD : ISD::SUB;
  SDValue Res = DAG.getNode(Opcode, dl, NVT, LHS, RHS);

  // Calculate the overflow flag: zero extend the arithmetic result from
  // the original type.
  SDValue Ofl = DAG.getZeroExtendInReg(Res, dl, OVT);
  // Overflowed if and only if this is not equal to Res.
  Ofl = DAG.getSetCC(dl, N->getValueType(1), Ofl, Res, ISD::SETNE);

  // Use the calculated overflow everywhere.
  ReplaceValueWith(SDValue(N, 1), Ofl);

  return Res;
}

SDValue DAGTypeLegalizer::PromoteIntRes_XMULO(SDNode *N, unsigned ResNo) {
  // Promote the overflow bit trivially.
  if (ResNo == 1)
    return PromoteIntRes_Overflow(N);

  SDValue LHS = N->getOperand(0), RHS = N->getOperand(1);
  SDLoc DL(N);
  EVT SmallVT = LHS.getValueType();

  // To determine if the result overflowed in a larger type, we extend the
  // input to the larger type, do the multiply (checking if it overflows),
  // then also check the high bits of the result to see if overflow happened
  // there.
  if (N->getOpcode() == ISD::SMULO) {
    LHS = SExtPromotedInteger(LHS);
    RHS = SExtPromotedInteger(RHS);
  } else {
    LHS = ZExtPromotedInteger(LHS);
    RHS = ZExtPromotedInteger(RHS);
  }
  SDVTList VTs = DAG.getVTList(LHS.getValueType(), N->getValueType(1));
  SDValue Mul = DAG.getNode(N->getOpcode(), DL, VTs, LHS, RHS);

  // Overflow occurred if it occurred in the larger type, or if the high part
  // of the result does not zero/sign-extend the low part.  Check this second
  // possibility first.
  SDValue Overflow;
  if (N->getOpcode() == ISD::UMULO) {
    // Unsigned overflow occurred if the high part is non-zero.
    SDValue Hi = DAG.getNode(ISD::SRL, DL, Mul.getValueType(), Mul,
                             DAG.getIntPtrConstant(SmallVT.getSizeInBits(),
                                                   DL));
    Overflow = DAG.getSetCC(DL, N->getValueType(1), Hi,
                            DAG.getConstant(0, DL, Hi.getValueType()),
                            ISD::SETNE);
  } else {
    // Signed overflow occurred if the high part does not sign extend the low.
    SDValue SExt = DAG.getNode(ISD::SIGN_EXTEND_INREG, DL, Mul.getValueType(),
                               Mul, DAG.getValueType(SmallVT));
    Overflow = DAG.getSetCC(DL, N->getValueType(1), SExt, Mul, ISD::SETNE);
  }

  // The only other way for overflow to occur is if the multiplication in the
  // larger type itself overflowed.
  Overflow = DAG.getNode(ISD::OR, DL, N->getValueType(1), Overflow,
                         SDValue(Mul.getNode(), 1));

  // Use the calculated overflow everywhere.
  ReplaceValueWith(SDValue(N, 1), Overflow);
  return Mul;
}

SDValue DAGTypeLegalizer::PromoteIntRes_UDIV(SDNode *N) {
  // Zero extend the input.
  SDValue LHS = ZExtPromotedInteger(N->getOperand(0));
  SDValue RHS = ZExtPromotedInteger(N->getOperand(1));
  return DAG.getNode(N->getOpcode(), SDLoc(N),
                     LHS.getValueType(), LHS, RHS);
}

SDValue DAGTypeLegalizer::PromoteIntRes_UNDEF(SDNode *N) {
  return DAG.getUNDEF(TLI.getTypeToTransformTo(*DAG.getContext(),
                                               N->getValueType(0)));
}

SDValue DAGTypeLegalizer::PromoteIntRes_VAARG(SDNode *N) {
  SDValue Chain = N->getOperand(0); // Get the chain.
  SDValue Ptr = N->getOperand(1); // Get the pointer.
  EVT VT = N->getValueType(0);
  SDLoc dl(N);

  MVT RegVT = TLI.getRegisterType(*DAG.getContext(), VT);
  unsigned NumRegs = TLI.getNumRegisters(*DAG.getContext(), VT);
  // The argument is passed as NumRegs registers of type RegVT.

  SmallVector<SDValue, 8> Parts(NumRegs);
  for (unsigned i = 0; i < NumRegs; ++i) {
    Parts[i] = DAG.getVAArg(RegVT, dl, Chain, Ptr, N->getOperand(2),
                            N->getConstantOperandVal(3));
    Chain = Parts[i].getValue(1);
  }

  // Handle endianness of the load.
  if (DAG.getDataLayout().isBigEndian())
    std::reverse(Parts.begin(), Parts.end());

  // Assemble the parts in the promoted type.
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDValue Res = DAG.getNode(ISD::ZERO_EXTEND, dl, NVT, Parts[0]);
  for (unsigned i = 1; i < NumRegs; ++i) {
    SDValue Part = DAG.getNode(ISD::ZERO_EXTEND, dl, NVT, Parts[i]);
    // Shift it to the right position and "or" it in.
    Part = DAG.getNode(ISD::SHL, dl, NVT, Part,
                       DAG.getConstant(i * RegVT.getSizeInBits(), dl,
                                       TLI.getPointerTy(DAG.getDataLayout())));
    Res = DAG.getNode(ISD::OR, dl, NVT, Res, Part);
  }

  // Modified the chain result - switch anything that used the old chain to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Chain);

  return Res;
}

//===----------------------------------------------------------------------===//
//  Integer Operand Promotion
//===----------------------------------------------------------------------===//

/// PromoteIntegerOperand - This method is called when the specified operand of
/// the specified node is found to need promotion.  At this point, all of the
/// result types of the node are known to be legal, but other operands of the
/// node may need promotion or expansion as well as the specified one.
bool DAGTypeLegalizer::PromoteIntegerOperand(SDNode *N, unsigned OpNo) {
  DEBUG(dbgs() << "Promote integer operand: "; N->dump(&DAG); dbgs() << "\n");
  SDValue Res = SDValue();

  if (CustomLowerNode(N, N->getOperand(OpNo).getValueType(), false))
    return false;

  switch (N->getOpcode()) {
    default:
  #ifndef NDEBUG
    dbgs() << "PromoteIntegerOperand Op #" << OpNo << ": ";
    N->dump(&DAG); dbgs() << "\n";
  #endif
    llvm_unreachable("Do not know how to promote this operator's operand!");

  case ISD::ANY_EXTEND:   Res = PromoteIntOp_ANY_EXTEND(N); break;
  case ISD::ATOMIC_STORE:
    Res = PromoteIntOp_ATOMIC_STORE(cast<AtomicSDNode>(N));
    break;
  case ISD::BITCAST:      Res = PromoteIntOp_BITCAST(N); break;
  case ISD::BR_CC:        Res = PromoteIntOp_BR_CC(N, OpNo); break;
  case ISD::BRCOND:       Res = PromoteIntOp_BRCOND(N, OpNo); break;
  case ISD::BUILD_PAIR:   Res = PromoteIntOp_BUILD_PAIR(N); break;
  case ISD::BUILD_VECTOR: Res = PromoteIntOp_BUILD_VECTOR(N); break;
  case ISD::CONCAT_VECTORS: Res = PromoteIntOp_CONCAT_VECTORS(N); break;
  case ISD::EXTRACT_VECTOR_ELT: Res = PromoteIntOp_EXTRACT_VECTOR_ELT(N); break;
  case ISD::CONVERT_RNDSAT:
                          Res = PromoteIntOp_CONVERT_RNDSAT(N); break;
  case ISD::INSERT_VECTOR_ELT:
                          Res = PromoteIntOp_INSERT_VECTOR_ELT(N, OpNo);break;
  case ISD::SCALAR_TO_VECTOR:
                          Res = PromoteIntOp_SCALAR_TO_VECTOR(N); break;
  case ISD::VSELECT:
  case ISD::SELECT:       Res = PromoteIntOp_SELECT(N, OpNo); break;
  case ISD::SELECT_CC:    Res = PromoteIntOp_SELECT_CC(N, OpNo); break;
  case ISD::SETCC:        Res = PromoteIntOp_SETCC(N, OpNo); break;
  case ISD::SIGN_EXTEND:  Res = PromoteIntOp_SIGN_EXTEND(N); break;
  case ISD::SINT_TO_FP:   Res = PromoteIntOp_SINT_TO_FP(N); break;
  case ISD::STORE:        Res = PromoteIntOp_STORE(cast<StoreSDNode>(N),
                                                   OpNo); break;
  case ISD::MSTORE:       Res = PromoteIntOp_MSTORE(cast<MaskedStoreSDNode>(N),
                                                    OpNo); break;
  case ISD::MLOAD:        Res = PromoteIntOp_MLOAD(cast<MaskedLoadSDNode>(N),
                                                    OpNo); break;
  case ISD::TRUNCATE:     Res = PromoteIntOp_TRUNCATE(N); break;
  case ISD::FP16_TO_FP:
  case ISD::UINT_TO_FP:   Res = PromoteIntOp_UINT_TO_FP(N); break;
  case ISD::ZERO_EXTEND:  Res = PromoteIntOp_ZERO_EXTEND(N); break;
  case ISD::EXTRACT_SUBVECTOR: Res = PromoteIntOp_EXTRACT_SUBVECTOR(N); break;

  case ISD::SHL:
  case ISD::SRA:
  case ISD::SRL:
  case ISD::ROTL:
  case ISD::ROTR: Res = PromoteIntOp_Shift(N); break;
  }

  // If the result is null, the sub-method took care of registering results etc.
  if (!Res.getNode()) return false;

  // If the result is N, the sub-method updated N in place.  Tell the legalizer
  // core about this.
  if (Res.getNode() == N)
    return true;

  assert(Res.getValueType() == N->getValueType(0) && N->getNumValues() == 1 &&
         "Invalid operand expansion");

  ReplaceValueWith(SDValue(N, 0), Res);
  return false;
}

/// PromoteSetCCOperands - Promote the operands of a comparison.  This code is
/// shared among BR_CC, SELECT_CC, and SETCC handlers.
void DAGTypeLegalizer::PromoteSetCCOperands(SDValue &NewLHS,SDValue &NewRHS,
                                            ISD::CondCode CCCode) {
  // We have to insert explicit sign or zero extends.  Note that we could
  // insert sign extends for ALL conditions, but zero extend is cheaper on
  // many machines (an AND instead of two shifts), so prefer it.
  switch (CCCode) {
  default: llvm_unreachable("Unknown integer comparison!");
  case ISD::SETEQ:
  case ISD::SETNE: {
    SDValue OpL = GetPromotedInteger(NewLHS);
    SDValue OpR = GetPromotedInteger(NewRHS);

    // We would prefer to promote the comparison operand with sign extension,
    // if we find the operand is actually to truncate an AssertSext. With this
    // optimization, we can avoid inserting real truncate instruction, which
    // is redudant eventually.
    if (OpL->getOpcode() == ISD::AssertSext &&
        cast<VTSDNode>(OpL->getOperand(1))->getVT() == NewLHS.getValueType() &&
        OpR->getOpcode() == ISD::AssertSext &&
        cast<VTSDNode>(OpR->getOperand(1))->getVT() == NewRHS.getValueType()) {
      NewLHS = OpL;
      NewRHS = OpR;
    } else {
      NewLHS = ZExtPromotedInteger(NewLHS);
      NewRHS = ZExtPromotedInteger(NewRHS);
    }
    break;
  }
  case ISD::SETUGE:
  case ISD::SETUGT:
  case ISD::SETULE:
  case ISD::SETULT:
    // ALL of these operations will work if we either sign or zero extend
    // the operands (including the unsigned comparisons!).  Zero extend is
    // usually a simpler/cheaper operation, so prefer it.
    NewLHS = ZExtPromotedInteger(NewLHS);
    NewRHS = ZExtPromotedInteger(NewRHS);
    break;
  case ISD::SETGE:
  case ISD::SETGT:
  case ISD::SETLT:
  case ISD::SETLE:
    NewLHS = SExtPromotedInteger(NewLHS);
    NewRHS = SExtPromotedInteger(NewRHS);
    break;
  }
}

SDValue DAGTypeLegalizer::PromoteIntOp_ANY_EXTEND(SDNode *N) {
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  return DAG.getNode(ISD::ANY_EXTEND, SDLoc(N), N->getValueType(0), Op);
}

SDValue DAGTypeLegalizer::PromoteIntOp_ATOMIC_STORE(AtomicSDNode *N) {
  SDValue Op2 = GetPromotedInteger(N->getOperand(2));
  return DAG.getAtomic(N->getOpcode(), SDLoc(N), N->getMemoryVT(),
                       N->getChain(), N->getBasePtr(), Op2, N->getMemOperand(),
                       N->getOrdering(), N->getSynchScope());
}

SDValue DAGTypeLegalizer::PromoteIntOp_BITCAST(SDNode *N) {
  // This should only occur in unusual situations like bitcasting to an
  // x86_fp80, so just turn it into a store+load
  return CreateStackStoreLoad(N->getOperand(0), N->getValueType(0));
}

SDValue DAGTypeLegalizer::PromoteIntOp_BR_CC(SDNode *N, unsigned OpNo) {
  assert(OpNo == 2 && "Don't know how to promote this operand!");

  SDValue LHS = N->getOperand(2);
  SDValue RHS = N->getOperand(3);
  PromoteSetCCOperands(LHS, RHS, cast<CondCodeSDNode>(N->getOperand(1))->get());

  // The chain (Op#0), CC (#1) and basic block destination (Op#4) are always
  // legal types.
  return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0),
                                N->getOperand(1), LHS, RHS, N->getOperand(4)),
                 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_BRCOND(SDNode *N, unsigned OpNo) {
  assert(OpNo == 1 && "only know how to promote condition");

  // Promote all the way up to the canonical SetCC type.
  SDValue Cond = PromoteTargetBoolean(N->getOperand(1), MVT::Other);

  // The chain (Op#0) and basic block destination (Op#2) are always legal types.
  return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0), Cond,
                                        N->getOperand(2)), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_BUILD_PAIR(SDNode *N) {
  // Since the result type is legal, the operands must promote to it.
  EVT OVT = N->getOperand(0).getValueType();
  SDValue Lo = ZExtPromotedInteger(N->getOperand(0));
  SDValue Hi = GetPromotedInteger(N->getOperand(1));
  assert(Lo.getValueType() == N->getValueType(0) && "Operand over promoted?");
  SDLoc dl(N);

  Hi = DAG.getNode(ISD::SHL, dl, N->getValueType(0), Hi,
                   DAG.getConstant(OVT.getSizeInBits(), dl,
                                   TLI.getPointerTy(DAG.getDataLayout())));
  return DAG.getNode(ISD::OR, dl, N->getValueType(0), Lo, Hi);
}

SDValue DAGTypeLegalizer::PromoteIntOp_BUILD_VECTOR(SDNode *N) {
  // The vector type is legal but the element type is not.  This implies
  // that the vector is a power-of-two in length and that the element
  // type does not have a strange size (eg: it is not i1).
  EVT VecVT = N->getValueType(0);
  unsigned NumElts = VecVT.getVectorNumElements();
  assert(!((NumElts & 1) && (!TLI.isTypeLegal(VecVT))) &&
         "Legal vector of one illegal element?");

  // Promote the inserted value.  The type does not need to match the
  // vector element type.  Check that any extra bits introduced will be
  // truncated away.
  assert(N->getOperand(0).getValueType().getSizeInBits() >=
         N->getValueType(0).getVectorElementType().getSizeInBits() &&
         "Type of inserted value narrower than vector element type!");

  SmallVector<SDValue, 16> NewOps;
  for (unsigned i = 0; i < NumElts; ++i)
    NewOps.push_back(GetPromotedInteger(N->getOperand(i)));

  return SDValue(DAG.UpdateNodeOperands(N, NewOps), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_CONVERT_RNDSAT(SDNode *N) {
  ISD::CvtCode CvtCode = cast<CvtRndSatSDNode>(N)->getCvtCode();
  assert ((CvtCode == ISD::CVT_SS || CvtCode == ISD::CVT_SU ||
           CvtCode == ISD::CVT_US || CvtCode == ISD::CVT_UU ||
           CvtCode == ISD::CVT_FS || CvtCode == ISD::CVT_FU) &&
           "can only promote integer arguments");
  SDValue InOp = GetPromotedInteger(N->getOperand(0));
  return DAG.getConvertRndSat(N->getValueType(0), SDLoc(N), InOp,
                              N->getOperand(1), N->getOperand(2),
                              N->getOperand(3), N->getOperand(4), CvtCode);
}

SDValue DAGTypeLegalizer::PromoteIntOp_INSERT_VECTOR_ELT(SDNode *N,
                                                         unsigned OpNo) {
  if (OpNo == 1) {
    // Promote the inserted value.  This is valid because the type does not
    // have to match the vector element type.

    // Check that any extra bits introduced will be truncated away.
    assert(N->getOperand(1).getValueType().getSizeInBits() >=
           N->getValueType(0).getVectorElementType().getSizeInBits() &&
           "Type of inserted value narrower than vector element type!");
    return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0),
                                  GetPromotedInteger(N->getOperand(1)),
                                  N->getOperand(2)),
                   0);
  }

  assert(OpNo == 2 && "Different operand and result vector types?");

  // Promote the index.
  SDValue Idx = DAG.getZExtOrTrunc(N->getOperand(2), SDLoc(N),
                                   TLI.getVectorIdxTy(DAG.getDataLayout()));
  return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0),
                                N->getOperand(1), Idx), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_SCALAR_TO_VECTOR(SDNode *N) {
  // Integer SCALAR_TO_VECTOR operands are implicitly truncated, so just promote
  // the operand in place.
  return SDValue(DAG.UpdateNodeOperands(N,
                                GetPromotedInteger(N->getOperand(0))), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_SELECT(SDNode *N, unsigned OpNo) {
  assert(OpNo == 0 && "Only know how to promote the condition!");
  SDValue Cond = N->getOperand(0);
  EVT OpTy = N->getOperand(1).getValueType();

  // Promote all the way up to the canonical SetCC type.
  EVT OpVT = N->getOpcode() == ISD::SELECT ? OpTy.getScalarType() : OpTy;
  Cond = PromoteTargetBoolean(Cond, OpVT);

  return SDValue(DAG.UpdateNodeOperands(N, Cond, N->getOperand(1),
                                        N->getOperand(2)), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_SELECT_CC(SDNode *N, unsigned OpNo) {
  assert(OpNo == 0 && "Don't know how to promote this operand!");

  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  PromoteSetCCOperands(LHS, RHS, cast<CondCodeSDNode>(N->getOperand(4))->get());

  // The CC (#4) and the possible return values (#2 and #3) have legal types.
  return SDValue(DAG.UpdateNodeOperands(N, LHS, RHS, N->getOperand(2),
                                N->getOperand(3), N->getOperand(4)), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_SETCC(SDNode *N, unsigned OpNo) {
  assert(OpNo == 0 && "Don't know how to promote this operand!");

  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  PromoteSetCCOperands(LHS, RHS, cast<CondCodeSDNode>(N->getOperand(2))->get());

  // The CC (#2) is always legal.
  return SDValue(DAG.UpdateNodeOperands(N, LHS, RHS, N->getOperand(2)), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_Shift(SDNode *N) {
  return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0),
                                ZExtPromotedInteger(N->getOperand(1))), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_SIGN_EXTEND(SDNode *N) {
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  SDLoc dl(N);
  Op = DAG.getNode(ISD::ANY_EXTEND, dl, N->getValueType(0), Op);
  return DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, Op.getValueType(),
                     Op, DAG.getValueType(N->getOperand(0).getValueType()));
}

SDValue DAGTypeLegalizer::PromoteIntOp_SINT_TO_FP(SDNode *N) {
  return SDValue(DAG.UpdateNodeOperands(N,
                                SExtPromotedInteger(N->getOperand(0))), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_STORE(StoreSDNode *N, unsigned OpNo){
  assert(ISD::isUNINDEXEDStore(N) && "Indexed store during type legalization!");
  SDValue Ch = N->getChain(), Ptr = N->getBasePtr();
  SDLoc dl(N);

  SDValue Val = GetPromotedInteger(N->getValue());  // Get promoted value.

  // Truncate the value and store the result.
  return DAG.getTruncStore(Ch, dl, Val, Ptr,
                           N->getMemoryVT(), N->getMemOperand());
}

SDValue DAGTypeLegalizer::PromoteIntOp_MSTORE(MaskedStoreSDNode *N, unsigned OpNo){

  SDValue DataOp = N->getValue();
  EVT DataVT = DataOp.getValueType();
  SDValue Mask = N->getMask();
  EVT MaskVT = Mask.getValueType();
  SDLoc dl(N);

  bool TruncateStore = false;
  if (!TLI.isTypeLegal(DataVT)) {
    if (getTypeAction(DataVT) == TargetLowering::TypePromoteInteger) {
      DataOp = GetPromotedInteger(DataOp);
      if (!TLI.isTypeLegal(MaskVT))
        Mask = PromoteTargetBoolean(Mask, DataOp.getValueType());
      TruncateStore = true;
    }
    else {
      assert(getTypeAction(DataVT) == TargetLowering::TypeWidenVector &&
             "Unexpected data legalization in MSTORE");
      DataOp = GetWidenedVector(DataOp);

      if (getTypeAction(MaskVT) == TargetLowering::TypeWidenVector)
        Mask = GetWidenedVector(Mask);
      else {
        EVT BoolVT = getSetCCResultType(DataOp.getValueType());

        // We can't use ModifyToType() because we should fill the mask with
        // zeroes
        unsigned WidenNumElts = BoolVT.getVectorNumElements();
        unsigned MaskNumElts = MaskVT.getVectorNumElements();

        unsigned NumConcat = WidenNumElts / MaskNumElts;
        SmallVector<SDValue, 16> Ops(NumConcat);
        SDValue ZeroVal = DAG.getConstant(0, dl, MaskVT);
        Ops[0] = Mask;
        for (unsigned i = 1; i != NumConcat; ++i)
          Ops[i] = ZeroVal;

        Mask = DAG.getNode(ISD::CONCAT_VECTORS, dl, BoolVT, Ops);
      }
    }
  }
  else
    Mask = PromoteTargetBoolean(N->getMask(), DataOp.getValueType());
  return DAG.getMaskedStore(N->getChain(), dl, DataOp, N->getBasePtr(), Mask,
                            N->getMemoryVT(), N->getMemOperand(),
                            TruncateStore);
}

SDValue DAGTypeLegalizer::PromoteIntOp_MLOAD(MaskedLoadSDNode *N, unsigned OpNo){
  assert(OpNo == 2 && "Only know how to promote the mask!");
  EVT DataVT = N->getValueType(0);
  SDValue Mask = PromoteTargetBoolean(N->getOperand(OpNo), DataVT);
  SmallVector<SDValue, 4> NewOps(N->op_begin(), N->op_end());
  NewOps[OpNo] = Mask;
  return SDValue(DAG.UpdateNodeOperands(N, NewOps), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_TRUNCATE(SDNode *N) {
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  return DAG.getNode(ISD::TRUNCATE, SDLoc(N), N->getValueType(0), Op);
}

SDValue DAGTypeLegalizer::PromoteIntOp_UINT_TO_FP(SDNode *N) {
  return SDValue(DAG.UpdateNodeOperands(N,
                                ZExtPromotedInteger(N->getOperand(0))), 0);
}

SDValue DAGTypeLegalizer::PromoteIntOp_ZERO_EXTEND(SDNode *N) {
  SDLoc dl(N);
  SDValue Op = GetPromotedInteger(N->getOperand(0));
  Op = DAG.getNode(ISD::ANY_EXTEND, dl, N->getValueType(0), Op);
  return DAG.getZeroExtendInReg(Op, dl,
                                N->getOperand(0).getValueType().getScalarType());
}


//===----------------------------------------------------------------------===//
//  Integer Result Expansion
//===----------------------------------------------------------------------===//

/// ExpandIntegerResult - This method is called when the specified result of the
/// specified node is found to need expansion.  At this point, the node may also
/// have invalid operands or may have other results that need promotion, we just
/// know that (at least) one result needs expansion.
void DAGTypeLegalizer::ExpandIntegerResult(SDNode *N, unsigned ResNo) {
  DEBUG(dbgs() << "Expand integer result: "; N->dump(&DAG); dbgs() << "\n");
  SDValue Lo, Hi;
  Lo = Hi = SDValue();

  // See if the target wants to custom expand this node.
  if (CustomLowerNode(N, N->getValueType(ResNo), true))
    return;

  switch (N->getOpcode()) {
  default:
#ifndef NDEBUG
    dbgs() << "ExpandIntegerResult #" << ResNo << ": ";
    N->dump(&DAG); dbgs() << "\n";
#endif
    llvm_unreachable("Do not know how to expand the result of this operator!");

  case ISD::MERGE_VALUES: SplitRes_MERGE_VALUES(N, ResNo, Lo, Hi); break;
  case ISD::SELECT:       SplitRes_SELECT(N, Lo, Hi); break;
  case ISD::SELECT_CC:    SplitRes_SELECT_CC(N, Lo, Hi); break;
  case ISD::UNDEF:        SplitRes_UNDEF(N, Lo, Hi); break;

  case ISD::BITCAST:            ExpandRes_BITCAST(N, Lo, Hi); break;
  case ISD::BUILD_PAIR:         ExpandRes_BUILD_PAIR(N, Lo, Hi); break;
  case ISD::EXTRACT_ELEMENT:    ExpandRes_EXTRACT_ELEMENT(N, Lo, Hi); break;
  case ISD::EXTRACT_VECTOR_ELT: ExpandRes_EXTRACT_VECTOR_ELT(N, Lo, Hi); break;
  case ISD::VAARG:              ExpandRes_VAARG(N, Lo, Hi); break;

  case ISD::ANY_EXTEND:  ExpandIntRes_ANY_EXTEND(N, Lo, Hi); break;
  case ISD::AssertSext:  ExpandIntRes_AssertSext(N, Lo, Hi); break;
  case ISD::AssertZext:  ExpandIntRes_AssertZext(N, Lo, Hi); break;
  case ISD::BSWAP:       ExpandIntRes_BSWAP(N, Lo, Hi); break;
  case ISD::Constant:    ExpandIntRes_Constant(N, Lo, Hi); break;
  case ISD::CTLZ_ZERO_UNDEF:
  case ISD::CTLZ:        ExpandIntRes_CTLZ(N, Lo, Hi); break;
  case ISD::CTPOP:       ExpandIntRes_CTPOP(N, Lo, Hi); break;
  case ISD::CTTZ_ZERO_UNDEF:
  case ISD::CTTZ:        ExpandIntRes_CTTZ(N, Lo, Hi); break;
  case ISD::FP_TO_SINT:  ExpandIntRes_FP_TO_SINT(N, Lo, Hi); break;
  case ISD::FP_TO_UINT:  ExpandIntRes_FP_TO_UINT(N, Lo, Hi); break;
  case ISD::LOAD:        ExpandIntRes_LOAD(cast<LoadSDNode>(N), Lo, Hi); break;
  case ISD::MUL:         ExpandIntRes_MUL(N, Lo, Hi); break;
  case ISD::SDIV:        ExpandIntRes_SDIV(N, Lo, Hi); break;
  case ISD::SIGN_EXTEND: ExpandIntRes_SIGN_EXTEND(N, Lo, Hi); break;
  case ISD::SIGN_EXTEND_INREG: ExpandIntRes_SIGN_EXTEND_INREG(N, Lo, Hi); break;
  case ISD::SREM:        ExpandIntRes_SREM(N, Lo, Hi); break;
  case ISD::TRUNCATE:    ExpandIntRes_TRUNCATE(N, Lo, Hi); break;
  case ISD::UDIV:        ExpandIntRes_UDIV(N, Lo, Hi); break;
  case ISD::UREM:        ExpandIntRes_UREM(N, Lo, Hi); break;
  case ISD::ZERO_EXTEND: ExpandIntRes_ZERO_EXTEND(N, Lo, Hi); break;
  case ISD::ATOMIC_LOAD: ExpandIntRes_ATOMIC_LOAD(N, Lo, Hi); break;

  case ISD::ATOMIC_LOAD_ADD:
  case ISD::ATOMIC_LOAD_SUB:
  case ISD::ATOMIC_LOAD_AND:
  case ISD::ATOMIC_LOAD_OR:
  case ISD::ATOMIC_LOAD_XOR:
  case ISD::ATOMIC_LOAD_NAND:
  case ISD::ATOMIC_LOAD_MIN:
  case ISD::ATOMIC_LOAD_MAX:
  case ISD::ATOMIC_LOAD_UMIN:
  case ISD::ATOMIC_LOAD_UMAX:
  case ISD::ATOMIC_SWAP:
  case ISD::ATOMIC_CMP_SWAP: {
    std::pair<SDValue, SDValue> Tmp = ExpandAtomic(N);
    SplitInteger(Tmp.first, Lo, Hi);
    ReplaceValueWith(SDValue(N, 1), Tmp.second);
    break;
  }
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS: {
    AtomicSDNode *AN = cast<AtomicSDNode>(N);
    SDVTList VTs = DAG.getVTList(N->getValueType(0), MVT::Other);
    SDValue Tmp = DAG.getAtomicCmpSwap(
        ISD::ATOMIC_CMP_SWAP, SDLoc(N), AN->getMemoryVT(), VTs,
        N->getOperand(0), N->getOperand(1), N->getOperand(2), N->getOperand(3),
        AN->getMemOperand(), AN->getSuccessOrdering(), AN->getFailureOrdering(),
        AN->getSynchScope());

    // Expanding to the strong ATOMIC_CMP_SWAP node means we can determine
    // success simply by comparing the loaded value against the ingoing
    // comparison.
    SDValue Success = DAG.getSetCC(SDLoc(N), N->getValueType(1), Tmp,
                                   N->getOperand(2), ISD::SETEQ);

    SplitInteger(Tmp, Lo, Hi);
    ReplaceValueWith(SDValue(N, 1), Success);
    ReplaceValueWith(SDValue(N, 2), Tmp.getValue(1));
    break;
  }

  case ISD::AND:
  case ISD::OR:
  case ISD::XOR: ExpandIntRes_Logical(N, Lo, Hi); break;

  case ISD::ADD:
  case ISD::SUB: ExpandIntRes_ADDSUB(N, Lo, Hi); break;

  case ISD::ADDC:
  case ISD::SUBC: ExpandIntRes_ADDSUBC(N, Lo, Hi); break;

  case ISD::ADDE:
  case ISD::SUBE: ExpandIntRes_ADDSUBE(N, Lo, Hi); break;

  case ISD::SHL:
  case ISD::SRA:
  case ISD::SRL: ExpandIntRes_Shift(N, Lo, Hi); break;

  case ISD::SADDO:
  case ISD::SSUBO: ExpandIntRes_SADDSUBO(N, Lo, Hi); break;
  case ISD::UADDO:
  case ISD::USUBO: ExpandIntRes_UADDSUBO(N, Lo, Hi); break;
  case ISD::UMULO:
  case ISD::SMULO: ExpandIntRes_XMULO(N, Lo, Hi); break;
  }

  // If Lo/Hi is null, the sub-method took care of registering results etc.
  if (Lo.getNode())
    SetExpandedInteger(SDValue(N, ResNo), Lo, Hi);
}

/// Lower an atomic node to the appropriate builtin call.
std::pair <SDValue, SDValue> DAGTypeLegalizer::ExpandAtomic(SDNode *Node) {
  unsigned Opc = Node->getOpcode();
  MVT VT = cast<AtomicSDNode>(Node)->getMemoryVT().getSimpleVT();
  RTLIB::Libcall LC = RTLIB::getATOMIC(Opc, VT);
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unexpected atomic op or value type!");

  return ExpandChainLibCall(LC, Node, false);
}

/// N is a shift by a value that needs to be expanded,
/// and the shift amount is a constant 'Amt'.  Expand the operation.
void DAGTypeLegalizer::ExpandShiftByConstant(SDNode *N, const APInt &Amt,
                                             SDValue &Lo, SDValue &Hi) {
  SDLoc DL(N);
  // Expand the incoming operand to be shifted, so that we have its parts
  SDValue InL, InH;
  GetExpandedInteger(N->getOperand(0), InL, InH);

  // Though Amt shouldn't usually be 0, it's possible. E.g. when legalization
  // splitted a vector shift, like this: <op1, op2> SHL <0, 2>.
  if (!Amt) {
    Lo = InL;
    Hi = InH;
    return;
  }

  EVT NVT = InL.getValueType();
  unsigned VTBits = N->getValueType(0).getSizeInBits();
  unsigned NVTBits = NVT.getSizeInBits();
  EVT ShTy = N->getOperand(1).getValueType();

  if (N->getOpcode() == ISD::SHL) {
    if (Amt.ugt(VTBits)) {
      Lo = Hi = DAG.getConstant(0, DL, NVT);
    } else if (Amt.ugt(NVTBits)) {
      Lo = DAG.getConstant(0, DL, NVT);
      Hi = DAG.getNode(ISD::SHL, DL,
                       NVT, InL, DAG.getConstant(Amt - NVTBits, DL, ShTy));
    } else if (Amt == NVTBits) {
      Lo = DAG.getConstant(0, DL, NVT);
      Hi = InL;
    } else if (Amt == 1 &&
               TLI.isOperationLegalOrCustom(ISD::ADDC,
                              TLI.getTypeToExpandTo(*DAG.getContext(), NVT))) {
      // Emit this X << 1 as X+X.
      SDVTList VTList = DAG.getVTList(NVT, MVT::Glue);
      SDValue LoOps[2] = { InL, InL };
      Lo = DAG.getNode(ISD::ADDC, DL, VTList, LoOps);
      SDValue HiOps[3] = { InH, InH, Lo.getValue(1) };
      Hi = DAG.getNode(ISD::ADDE, DL, VTList, HiOps);
    } else {
      Lo = DAG.getNode(ISD::SHL, DL, NVT, InL, DAG.getConstant(Amt, DL, ShTy));
      Hi = DAG.getNode(ISD::OR, DL, NVT,
                       DAG.getNode(ISD::SHL, DL, NVT, InH,
                                   DAG.getConstant(Amt, DL, ShTy)),
                       DAG.getNode(ISD::SRL, DL, NVT, InL,
                                   DAG.getConstant(-Amt + NVTBits, DL, ShTy)));
    }
    return;
  }

  if (N->getOpcode() == ISD::SRL) {
    if (Amt.ugt(VTBits)) {
      Lo = Hi = DAG.getConstant(0, DL, NVT);
    } else if (Amt.ugt(NVTBits)) {
      Lo = DAG.getNode(ISD::SRL, DL,
                       NVT, InH, DAG.getConstant(Amt - NVTBits, DL, ShTy));
      Hi = DAG.getConstant(0, DL, NVT);
    } else if (Amt == NVTBits) {
      Lo = InH;
      Hi = DAG.getConstant(0, DL, NVT);
    } else {
      Lo = DAG.getNode(ISD::OR, DL, NVT,
                       DAG.getNode(ISD::SRL, DL, NVT, InL,
                                   DAG.getConstant(Amt, DL, ShTy)),
                       DAG.getNode(ISD::SHL, DL, NVT, InH,
                                   DAG.getConstant(-Amt + NVTBits, DL, ShTy)));
      Hi = DAG.getNode(ISD::SRL, DL, NVT, InH, DAG.getConstant(Amt, DL, ShTy));
    }
    return;
  }

  assert(N->getOpcode() == ISD::SRA && "Unknown shift!");
  if (Amt.ugt(VTBits)) {
    Hi = Lo = DAG.getNode(ISD::SRA, DL, NVT, InH,
                          DAG.getConstant(NVTBits - 1, DL, ShTy));
  } else if (Amt.ugt(NVTBits)) {
    Lo = DAG.getNode(ISD::SRA, DL, NVT, InH,
                     DAG.getConstant(Amt - NVTBits, DL, ShTy));
    Hi = DAG.getNode(ISD::SRA, DL, NVT, InH,
                     DAG.getConstant(NVTBits - 1, DL, ShTy));
  } else if (Amt == NVTBits) {
    Lo = InH;
    Hi = DAG.getNode(ISD::SRA, DL, NVT, InH,
                     DAG.getConstant(NVTBits - 1, DL, ShTy));
  } else {
    Lo = DAG.getNode(ISD::OR, DL, NVT,
                     DAG.getNode(ISD::SRL, DL, NVT, InL,
                                 DAG.getConstant(Amt, DL, ShTy)),
                     DAG.getNode(ISD::SHL, DL, NVT, InH,
                                 DAG.getConstant(-Amt + NVTBits, DL, ShTy)));
    Hi = DAG.getNode(ISD::SRA, DL, NVT, InH, DAG.getConstant(Amt, DL, ShTy));
  }
}

/// ExpandShiftWithKnownAmountBit - Try to determine whether we can simplify
/// this shift based on knowledge of the high bit of the shift amount.  If we
/// can tell this, we know that it is >= 32 or < 32, without knowing the actual
/// shift amount.
bool DAGTypeLegalizer::
ExpandShiftWithKnownAmountBit(SDNode *N, SDValue &Lo, SDValue &Hi) {
  SDValue Amt = N->getOperand(1);
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  EVT ShTy = Amt.getValueType();
  unsigned ShBits = ShTy.getScalarType().getSizeInBits();
  unsigned NVTBits = NVT.getScalarType().getSizeInBits();
  assert(isPowerOf2_32(NVTBits) &&
         "Expanded integer type size not a power of two!");
  SDLoc dl(N);

  APInt HighBitMask = APInt::getHighBitsSet(ShBits, ShBits - Log2_32(NVTBits));
  APInt KnownZero, KnownOne;
  DAG.computeKnownBits(N->getOperand(1), KnownZero, KnownOne);

  // If we don't know anything about the high bits, exit.
  if (((KnownZero|KnownOne) & HighBitMask) == 0)
    return false;

  // Get the incoming operand to be shifted.
  SDValue InL, InH;
  GetExpandedInteger(N->getOperand(0), InL, InH);

  // If we know that any of the high bits of the shift amount are one, then we
  // can do this as a couple of simple shifts.
  if (KnownOne.intersects(HighBitMask)) {
    // Mask out the high bit, which we know is set.
    Amt = DAG.getNode(ISD::AND, dl, ShTy, Amt,
                      DAG.getConstant(~HighBitMask, dl, ShTy));

    switch (N->getOpcode()) {
    default: llvm_unreachable("Unknown shift");
    case ISD::SHL:
      Lo = DAG.getConstant(0, dl, NVT);              // Low part is zero.
      Hi = DAG.getNode(ISD::SHL, dl, NVT, InL, Amt); // High part from Lo part.
      return true;
    case ISD::SRL:
      Hi = DAG.getConstant(0, dl, NVT);              // Hi part is zero.
      Lo = DAG.getNode(ISD::SRL, dl, NVT, InH, Amt); // Lo part from Hi part.
      return true;
    case ISD::SRA:
      Hi = DAG.getNode(ISD::SRA, dl, NVT, InH,       // Sign extend high part.
                       DAG.getConstant(NVTBits - 1, dl, ShTy));
      Lo = DAG.getNode(ISD::SRA, dl, NVT, InH, Amt); // Lo part from Hi part.
      return true;
    }
  }

  // If we know that all of the high bits of the shift amount are zero, then we
  // can do this as a couple of simple shifts.
  if ((KnownZero & HighBitMask) == HighBitMask) {
    // Calculate 31-x. 31 is used instead of 32 to avoid creating an undefined
    // shift if x is zero.  We can use XOR here because x is known to be smaller
    // than 32.
    SDValue Amt2 = DAG.getNode(ISD::XOR, dl, ShTy, Amt,
                               DAG.getConstant(NVTBits - 1, dl, ShTy));

    unsigned Op1, Op2;
    switch (N->getOpcode()) {
    default: llvm_unreachable("Unknown shift");
    case ISD::SHL:  Op1 = ISD::SHL; Op2 = ISD::SRL; break;
    case ISD::SRL:
    case ISD::SRA:  Op1 = ISD::SRL; Op2 = ISD::SHL; break;
    }

    // When shifting right the arithmetic for Lo and Hi is swapped.
    if (N->getOpcode() != ISD::SHL)
      std::swap(InL, InH);

    // Use a little trick to get the bits that move from Lo to Hi. First
    // shift by one bit.
    SDValue Sh1 = DAG.getNode(Op2, dl, NVT, InL, DAG.getConstant(1, dl, ShTy));
    // Then compute the remaining shift with amount-1.
    SDValue Sh2 = DAG.getNode(Op2, dl, NVT, Sh1, Amt2);

    Lo = DAG.getNode(N->getOpcode(), dl, NVT, InL, Amt);
    Hi = DAG.getNode(ISD::OR, dl, NVT, DAG.getNode(Op1, dl, NVT, InH, Amt),Sh2);

    if (N->getOpcode() != ISD::SHL)
      std::swap(Hi, Lo);
    return true;
  }

  return false;
}

/// ExpandShiftWithUnknownAmountBit - Fully general expansion of integer shift
/// of any size.
bool DAGTypeLegalizer::
ExpandShiftWithUnknownAmountBit(SDNode *N, SDValue &Lo, SDValue &Hi) {
  SDValue Amt = N->getOperand(1);
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  EVT ShTy = Amt.getValueType();
  unsigned NVTBits = NVT.getSizeInBits();
  assert(isPowerOf2_32(NVTBits) &&
         "Expanded integer type size not a power of two!");
  SDLoc dl(N);

  // Get the incoming operand to be shifted.
  SDValue InL, InH;
  GetExpandedInteger(N->getOperand(0), InL, InH);

  SDValue NVBitsNode = DAG.getConstant(NVTBits, dl, ShTy);
  SDValue AmtExcess = DAG.getNode(ISD::SUB, dl, ShTy, Amt, NVBitsNode);
  SDValue AmtLack = DAG.getNode(ISD::SUB, dl, ShTy, NVBitsNode, Amt);
  SDValue isShort = DAG.getSetCC(dl, getSetCCResultType(ShTy),
                                 Amt, NVBitsNode, ISD::SETULT);
  SDValue isZero = DAG.getSetCC(dl, getSetCCResultType(ShTy),
                                Amt, DAG.getConstant(0, dl, ShTy),
                                ISD::SETEQ);

  SDValue LoS, HiS, LoL, HiL;
  switch (N->getOpcode()) {
  default: llvm_unreachable("Unknown shift");
  case ISD::SHL:
    // Short: ShAmt < NVTBits
    LoS = DAG.getNode(ISD::SHL, dl, NVT, InL, Amt);
    HiS = DAG.getNode(ISD::OR, dl, NVT,
                      DAG.getNode(ISD::SHL, dl, NVT, InH, Amt),
                      DAG.getNode(ISD::SRL, dl, NVT, InL, AmtLack));

    // Long: ShAmt >= NVTBits
    LoL = DAG.getConstant(0, dl, NVT);                    // Lo part is zero.
    HiL = DAG.getNode(ISD::SHL, dl, NVT, InL, AmtExcess); // Hi from Lo part.

    Lo = DAG.getSelect(dl, NVT, isShort, LoS, LoL);
    Hi = DAG.getSelect(dl, NVT, isZero, InH,
                       DAG.getSelect(dl, NVT, isShort, HiS, HiL));
    return true;
  case ISD::SRL:
    // Short: ShAmt < NVTBits
    HiS = DAG.getNode(ISD::SRL, dl, NVT, InH, Amt);
    LoS = DAG.getNode(ISD::OR, dl, NVT,
                      DAG.getNode(ISD::SRL, dl, NVT, InL, Amt),
    // FIXME: If Amt is zero, the following shift generates an undefined result
    // on some architectures.
                      DAG.getNode(ISD::SHL, dl, NVT, InH, AmtLack));

    // Long: ShAmt >= NVTBits
    HiL = DAG.getConstant(0, dl, NVT);                    // Hi part is zero.
    LoL = DAG.getNode(ISD::SRL, dl, NVT, InH, AmtExcess); // Lo from Hi part.

    Lo = DAG.getSelect(dl, NVT, isZero, InL,
                       DAG.getSelect(dl, NVT, isShort, LoS, LoL));
    Hi = DAG.getSelect(dl, NVT, isShort, HiS, HiL);
    return true;
  case ISD::SRA:
    // Short: ShAmt < NVTBits
    HiS = DAG.getNode(ISD::SRA, dl, NVT, InH, Amt);
    LoS = DAG.getNode(ISD::OR, dl, NVT,
                      DAG.getNode(ISD::SRL, dl, NVT, InL, Amt),
                      DAG.getNode(ISD::SHL, dl, NVT, InH, AmtLack));

    // Long: ShAmt >= NVTBits
    HiL = DAG.getNode(ISD::SRA, dl, NVT, InH,             // Sign of Hi part.
                      DAG.getConstant(NVTBits - 1, dl, ShTy));
    LoL = DAG.getNode(ISD::SRA, dl, NVT, InH, AmtExcess); // Lo from Hi part.

    Lo = DAG.getSelect(dl, NVT, isZero, InL,
                       DAG.getSelect(dl, NVT, isShort, LoS, LoL));
    Hi = DAG.getSelect(dl, NVT, isShort, HiS, HiL);
    return true;
  }
}

void DAGTypeLegalizer::ExpandIntRes_ADDSUB(SDNode *N,
                                           SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  // Expand the subcomponents.
  SDValue LHSL, LHSH, RHSL, RHSH;
  GetExpandedInteger(N->getOperand(0), LHSL, LHSH);
  GetExpandedInteger(N->getOperand(1), RHSL, RHSH);

  EVT NVT = LHSL.getValueType();
  SDValue LoOps[2] = { LHSL, RHSL };
  SDValue HiOps[3] = { LHSH, RHSH };

  // Do not generate ADDC/ADDE or SUBC/SUBE if the target does not support
  // them.  TODO: Teach operation legalization how to expand unsupported
  // ADDC/ADDE/SUBC/SUBE.  The problem is that these operations generate
  // a carry of type MVT::Glue, but there doesn't seem to be any way to
  // generate a value of this type in the expanded code sequence.
  bool hasCarry =
    TLI.isOperationLegalOrCustom(N->getOpcode() == ISD::ADD ?
                                   ISD::ADDC : ISD::SUBC,
                                 TLI.getTypeToExpandTo(*DAG.getContext(), NVT));

  if (hasCarry) {
    SDVTList VTList = DAG.getVTList(NVT, MVT::Glue);
    if (N->getOpcode() == ISD::ADD) {
      Lo = DAG.getNode(ISD::ADDC, dl, VTList, LoOps);
      HiOps[2] = Lo.getValue(1);
      Hi = DAG.getNode(ISD::ADDE, dl, VTList, HiOps);
    } else {
      Lo = DAG.getNode(ISD::SUBC, dl, VTList, LoOps);
      HiOps[2] = Lo.getValue(1);
      Hi = DAG.getNode(ISD::SUBE, dl, VTList, HiOps);
    }
    return;
  }

  bool hasOVF =
    TLI.isOperationLegalOrCustom(N->getOpcode() == ISD::ADD ?
                                   ISD::UADDO : ISD::USUBO,
                                 TLI.getTypeToExpandTo(*DAG.getContext(), NVT));
  if (hasOVF) {
    SDVTList VTList = DAG.getVTList(NVT, NVT);
    TargetLoweringBase::BooleanContent BoolType = TLI.getBooleanContents(NVT);
    int RevOpc;
    if (N->getOpcode() == ISD::ADD) {
      RevOpc = ISD::SUB;
      Lo = DAG.getNode(ISD::UADDO, dl, VTList, LoOps);
      Hi = DAG.getNode(ISD::ADD, dl, NVT, makeArrayRef(HiOps, 2));
    } else {
      RevOpc = ISD::ADD;
      Lo = DAG.getNode(ISD::USUBO, dl, VTList, LoOps);
      Hi = DAG.getNode(ISD::SUB, dl, NVT, makeArrayRef(HiOps, 2));
    }
    SDValue OVF = Lo.getValue(1);

    switch (BoolType) {
    case TargetLoweringBase::UndefinedBooleanContent:
      OVF = DAG.getNode(ISD::AND, dl, NVT, DAG.getConstant(1, dl, NVT), OVF);
      // Fallthrough
    case TargetLoweringBase::ZeroOrOneBooleanContent:
      Hi = DAG.getNode(N->getOpcode(), dl, NVT, Hi, OVF);
      break;
    case TargetLoweringBase::ZeroOrNegativeOneBooleanContent:
      Hi = DAG.getNode(RevOpc, dl, NVT, Hi, OVF);
    }
    return;
  }

  if (N->getOpcode() == ISD::ADD) {
    Lo = DAG.getNode(ISD::ADD, dl, NVT, LoOps);
    Hi = DAG.getNode(ISD::ADD, dl, NVT, makeArrayRef(HiOps, 2));
    SDValue Cmp1 = DAG.getSetCC(dl, getSetCCResultType(NVT), Lo, LoOps[0],
                                ISD::SETULT);
    SDValue Carry1 = DAG.getSelect(dl, NVT, Cmp1,
                                   DAG.getConstant(1, dl, NVT),
                                   DAG.getConstant(0, dl, NVT));
    SDValue Cmp2 = DAG.getSetCC(dl, getSetCCResultType(NVT), Lo, LoOps[1],
                                ISD::SETULT);
    SDValue Carry2 = DAG.getSelect(dl, NVT, Cmp2,
                                   DAG.getConstant(1, dl, NVT), Carry1);
    Hi = DAG.getNode(ISD::ADD, dl, NVT, Hi, Carry2);
  } else {
    Lo = DAG.getNode(ISD::SUB, dl, NVT, LoOps);
    Hi = DAG.getNode(ISD::SUB, dl, NVT, makeArrayRef(HiOps, 2));
    SDValue Cmp =
      DAG.getSetCC(dl, getSetCCResultType(LoOps[0].getValueType()),
                   LoOps[0], LoOps[1], ISD::SETULT);
    SDValue Borrow = DAG.getSelect(dl, NVT, Cmp,
                                   DAG.getConstant(1, dl, NVT),
                                   DAG.getConstant(0, dl, NVT));
    Hi = DAG.getNode(ISD::SUB, dl, NVT, Hi, Borrow);
  }
}

void DAGTypeLegalizer::ExpandIntRes_ADDSUBC(SDNode *N,
                                            SDValue &Lo, SDValue &Hi) {
  // Expand the subcomponents.
  SDValue LHSL, LHSH, RHSL, RHSH;
  SDLoc dl(N);
  GetExpandedInteger(N->getOperand(0), LHSL, LHSH);
  GetExpandedInteger(N->getOperand(1), RHSL, RHSH);
  SDVTList VTList = DAG.getVTList(LHSL.getValueType(), MVT::Glue);
  SDValue LoOps[2] = { LHSL, RHSL };
  SDValue HiOps[3] = { LHSH, RHSH };

  if (N->getOpcode() == ISD::ADDC) {
    Lo = DAG.getNode(ISD::ADDC, dl, VTList, LoOps);
    HiOps[2] = Lo.getValue(1);
    Hi = DAG.getNode(ISD::ADDE, dl, VTList, HiOps);
  } else {
    Lo = DAG.getNode(ISD::SUBC, dl, VTList, LoOps);
    HiOps[2] = Lo.getValue(1);
    Hi = DAG.getNode(ISD::SUBE, dl, VTList, HiOps);
  }

  // Legalized the flag result - switch anything that used the old flag to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Hi.getValue(1));
}

void DAGTypeLegalizer::ExpandIntRes_ADDSUBE(SDNode *N,
                                            SDValue &Lo, SDValue &Hi) {
  // Expand the subcomponents.
  SDValue LHSL, LHSH, RHSL, RHSH;
  SDLoc dl(N);
  GetExpandedInteger(N->getOperand(0), LHSL, LHSH);
  GetExpandedInteger(N->getOperand(1), RHSL, RHSH);
  SDVTList VTList = DAG.getVTList(LHSL.getValueType(), MVT::Glue);
  SDValue LoOps[3] = { LHSL, RHSL, N->getOperand(2) };
  SDValue HiOps[3] = { LHSH, RHSH };

  Lo = DAG.getNode(N->getOpcode(), dl, VTList, LoOps);
  HiOps[2] = Lo.getValue(1);
  Hi = DAG.getNode(N->getOpcode(), dl, VTList, HiOps);

  // Legalized the flag result - switch anything that used the old flag to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Hi.getValue(1));
}

void DAGTypeLegalizer::ExpandIntRes_MERGE_VALUES(SDNode *N, unsigned ResNo,
                                                 SDValue &Lo, SDValue &Hi) {
  SDValue Res = DisintegrateMERGE_VALUES(N, ResNo);
  SplitInteger(Res, Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_ANY_EXTEND(SDNode *N,
                                               SDValue &Lo, SDValue &Hi) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDLoc dl(N);
  SDValue Op = N->getOperand(0);
  if (Op.getValueType().bitsLE(NVT)) {
    // The low part is any extension of the input (which degenerates to a copy).
    Lo = DAG.getNode(ISD::ANY_EXTEND, dl, NVT, Op);
    Hi = DAG.getUNDEF(NVT);   // The high part is undefined.
  } else {
    // For example, extension of an i48 to an i64.  The operand type necessarily
    // promotes to the result type, so will end up being expanded too.
    assert(getTypeAction(Op.getValueType()) ==
           TargetLowering::TypePromoteInteger &&
           "Only know how to promote this result!");
    SDValue Res = GetPromotedInteger(Op);
    assert(Res.getValueType() == N->getValueType(0) &&
           "Operand over promoted?");
    // Split the promoted operand.  This will simplify when it is expanded.
    SplitInteger(Res, Lo, Hi);
  }
}

void DAGTypeLegalizer::ExpandIntRes_AssertSext(SDNode *N,
                                               SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  EVT NVT = Lo.getValueType();
  EVT EVT = cast<VTSDNode>(N->getOperand(1))->getVT();
  unsigned NVTBits = NVT.getSizeInBits();
  unsigned EVTBits = EVT.getSizeInBits();

  if (NVTBits < EVTBits) {
    Hi = DAG.getNode(ISD::AssertSext, dl, NVT, Hi,
                     DAG.getValueType(EVT::getIntegerVT(*DAG.getContext(),
                                                        EVTBits - NVTBits)));
  } else {
    Lo = DAG.getNode(ISD::AssertSext, dl, NVT, Lo, DAG.getValueType(EVT));
    // The high part replicates the sign bit of Lo, make it explicit.
    Hi = DAG.getNode(ISD::SRA, dl, NVT, Lo,
                     DAG.getConstant(NVTBits - 1, dl,
                                     TLI.getPointerTy(DAG.getDataLayout())));
  }
}

void DAGTypeLegalizer::ExpandIntRes_AssertZext(SDNode *N,
                                               SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  EVT NVT = Lo.getValueType();
  EVT EVT = cast<VTSDNode>(N->getOperand(1))->getVT();
  unsigned NVTBits = NVT.getSizeInBits();
  unsigned EVTBits = EVT.getSizeInBits();

  if (NVTBits < EVTBits) {
    Hi = DAG.getNode(ISD::AssertZext, dl, NVT, Hi,
                     DAG.getValueType(EVT::getIntegerVT(*DAG.getContext(),
                                                        EVTBits - NVTBits)));
  } else {
    Lo = DAG.getNode(ISD::AssertZext, dl, NVT, Lo, DAG.getValueType(EVT));
    // The high part must be zero, make it explicit.
    Hi = DAG.getConstant(0, dl, NVT);
  }
}

void DAGTypeLegalizer::ExpandIntRes_BSWAP(SDNode *N,
                                          SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  GetExpandedInteger(N->getOperand(0), Hi, Lo);  // Note swapped operands.
  Lo = DAG.getNode(ISD::BSWAP, dl, Lo.getValueType(), Lo);
  Hi = DAG.getNode(ISD::BSWAP, dl, Hi.getValueType(), Hi);
}

void DAGTypeLegalizer::ExpandIntRes_Constant(SDNode *N,
                                             SDValue &Lo, SDValue &Hi) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  unsigned NBitWidth = NVT.getSizeInBits();
  auto Constant = cast<ConstantSDNode>(N);
  const APInt &Cst = Constant->getAPIntValue();
  bool IsTarget = Constant->isTargetOpcode();
  bool IsOpaque = Constant->isOpaque();
  SDLoc dl(N);
  Lo = DAG.getConstant(Cst.trunc(NBitWidth), dl, NVT, IsTarget, IsOpaque);
  Hi = DAG.getConstant(Cst.lshr(NBitWidth).trunc(NBitWidth), dl, NVT, IsTarget,
                       IsOpaque);
}

void DAGTypeLegalizer::ExpandIntRes_CTLZ(SDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  // ctlz (HiLo) -> Hi != 0 ? ctlz(Hi) : (ctlz(Lo)+32)
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  EVT NVT = Lo.getValueType();

  SDValue HiNotZero = DAG.getSetCC(dl, getSetCCResultType(NVT), Hi,
                                   DAG.getConstant(0, dl, NVT), ISD::SETNE);

  SDValue LoLZ = DAG.getNode(N->getOpcode(), dl, NVT, Lo);
  SDValue HiLZ = DAG.getNode(ISD::CTLZ_ZERO_UNDEF, dl, NVT, Hi);

  Lo = DAG.getSelect(dl, NVT, HiNotZero, HiLZ,
                     DAG.getNode(ISD::ADD, dl, NVT, LoLZ,
                                 DAG.getConstant(NVT.getSizeInBits(), dl,
                                                 NVT)));
  Hi = DAG.getConstant(0, dl, NVT);
}

void DAGTypeLegalizer::ExpandIntRes_CTPOP(SDNode *N,
                                          SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  // ctpop(HiLo) -> ctpop(Hi)+ctpop(Lo)
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  EVT NVT = Lo.getValueType();
  Lo = DAG.getNode(ISD::ADD, dl, NVT, DAG.getNode(ISD::CTPOP, dl, NVT, Lo),
                   DAG.getNode(ISD::CTPOP, dl, NVT, Hi));
  Hi = DAG.getConstant(0, dl, NVT);
}

void DAGTypeLegalizer::ExpandIntRes_CTTZ(SDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  // cttz (HiLo) -> Lo != 0 ? cttz(Lo) : (cttz(Hi)+32)
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  EVT NVT = Lo.getValueType();

  SDValue LoNotZero = DAG.getSetCC(dl, getSetCCResultType(NVT), Lo,
                                   DAG.getConstant(0, dl, NVT), ISD::SETNE);

  SDValue LoLZ = DAG.getNode(ISD::CTTZ_ZERO_UNDEF, dl, NVT, Lo);
  SDValue HiLZ = DAG.getNode(N->getOpcode(), dl, NVT, Hi);

  Lo = DAG.getSelect(dl, NVT, LoNotZero, LoLZ,
                     DAG.getNode(ISD::ADD, dl, NVT, HiLZ,
                                 DAG.getConstant(NVT.getSizeInBits(), dl,
                                                 NVT)));
  Hi = DAG.getConstant(0, dl, NVT);
}

void DAGTypeLegalizer::ExpandIntRes_FP_TO_SINT(SDNode *N, SDValue &Lo,
                                               SDValue &Hi) {
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  SDValue Op = N->getOperand(0);
  if (getTypeAction(Op.getValueType()) == TargetLowering::TypePromoteFloat)
    Op = GetPromotedFloat(Op);

  RTLIB::Libcall LC = RTLIB::getFPTOSINT(Op.getValueType(), VT);
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unexpected fp-to-sint conversion!");
  SplitInteger(TLI.makeLibCall(DAG, LC, VT, &Op, 1, true/*irrelevant*/,
                               dl).first,
               Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_FP_TO_UINT(SDNode *N, SDValue &Lo,
                                               SDValue &Hi) {
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  SDValue Op = N->getOperand(0);
  if (getTypeAction(Op.getValueType()) == TargetLowering::TypePromoteFloat)
    Op = GetPromotedFloat(Op);

  RTLIB::Libcall LC = RTLIB::getFPTOUINT(Op.getValueType(), VT);
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unexpected fp-to-uint conversion!");
  SplitInteger(TLI.makeLibCall(DAG, LC, VT, &Op, 1, false/*irrelevant*/,
                               dl).first,
               Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_LOAD(LoadSDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  if (ISD::isNormalLoad(N)) {
    ExpandRes_NormalLoad(N, Lo, Hi);
    return;
  }

  assert(ISD::isUNINDEXEDLoad(N) && "Indexed load during type legalization!");

  EVT VT = N->getValueType(0);
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), VT);
  SDValue Ch  = N->getChain();
  SDValue Ptr = N->getBasePtr();
  ISD::LoadExtType ExtType = N->getExtensionType();
  unsigned Alignment = N->getAlignment();
  bool isVolatile = N->isVolatile();
  bool isNonTemporal = N->isNonTemporal();
  bool isInvariant = N->isInvariant();
  AAMDNodes AAInfo = N->getAAInfo();
  SDLoc dl(N);

  assert(NVT.isByteSized() && "Expanded type not byte sized!");

  if (N->getMemoryVT().bitsLE(NVT)) {
    EVT MemVT = N->getMemoryVT();

    Lo = DAG.getExtLoad(ExtType, dl, NVT, Ch, Ptr, N->getPointerInfo(),
                        MemVT, isVolatile, isNonTemporal, isInvariant,
                        Alignment, AAInfo);

    // Remember the chain.
    Ch = Lo.getValue(1);

    if (ExtType == ISD::SEXTLOAD) {
      // The high part is obtained by SRA'ing all but one of the bits of the
      // lo part.
      unsigned LoSize = Lo.getValueType().getSizeInBits();
      Hi = DAG.getNode(ISD::SRA, dl, NVT, Lo,
                       DAG.getConstant(LoSize - 1, dl,
                                       TLI.getPointerTy(DAG.getDataLayout())));
    } else if (ExtType == ISD::ZEXTLOAD) {
      // The high part is just a zero.
      Hi = DAG.getConstant(0, dl, NVT);
    } else {
      assert(ExtType == ISD::EXTLOAD && "Unknown extload!");
      // The high part is undefined.
      Hi = DAG.getUNDEF(NVT);
    }
  } else if (DAG.getDataLayout().isLittleEndian()) {
    // Little-endian - low bits are at low addresses.
    Lo = DAG.getLoad(NVT, dl, Ch, Ptr, N->getPointerInfo(),
                     isVolatile, isNonTemporal, isInvariant, Alignment,
                     AAInfo);

    unsigned ExcessBits =
      N->getMemoryVT().getSizeInBits() - NVT.getSizeInBits();
    EVT NEVT = EVT::getIntegerVT(*DAG.getContext(), ExcessBits);

    // Increment the pointer to the other half.
    unsigned IncrementSize = NVT.getSizeInBits()/8;
    Ptr = DAG.getNode(ISD::ADD, dl, Ptr.getValueType(), Ptr,
                      DAG.getConstant(IncrementSize, dl, Ptr.getValueType()));
    Hi = DAG.getExtLoad(ExtType, dl, NVT, Ch, Ptr,
                        N->getPointerInfo().getWithOffset(IncrementSize), NEVT,
                        isVolatile, isNonTemporal, isInvariant,
                        MinAlign(Alignment, IncrementSize), AAInfo);

    // Build a factor node to remember that this load is independent of the
    // other one.
    Ch = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Lo.getValue(1),
                     Hi.getValue(1));
  } else {
    // Big-endian - high bits are at low addresses.  Favor aligned loads at
    // the cost of some bit-fiddling.
    EVT MemVT = N->getMemoryVT();
    unsigned EBytes = MemVT.getStoreSize();
    unsigned IncrementSize = NVT.getSizeInBits()/8;
    unsigned ExcessBits = (EBytes - IncrementSize)*8;

    // Load both the high bits and maybe some of the low bits.
    Hi = DAG.getExtLoad(ExtType, dl, NVT, Ch, Ptr, N->getPointerInfo(),
                        EVT::getIntegerVT(*DAG.getContext(),
                                          MemVT.getSizeInBits() - ExcessBits),
                        isVolatile, isNonTemporal, isInvariant, Alignment,
                        AAInfo);

    // Increment the pointer to the other half.
    Ptr = DAG.getNode(ISD::ADD, dl, Ptr.getValueType(), Ptr,
                      DAG.getConstant(IncrementSize, dl, Ptr.getValueType()));
    // Load the rest of the low bits.
    Lo = DAG.getExtLoad(ISD::ZEXTLOAD, dl, NVT, Ch, Ptr,
                        N->getPointerInfo().getWithOffset(IncrementSize),
                        EVT::getIntegerVT(*DAG.getContext(), ExcessBits),
                        isVolatile, isNonTemporal, isInvariant,
                        MinAlign(Alignment, IncrementSize), AAInfo);

    // Build a factor node to remember that this load is independent of the
    // other one.
    Ch = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Lo.getValue(1),
                     Hi.getValue(1));

    if (ExcessBits < NVT.getSizeInBits()) {
      // Transfer low bits from the bottom of Hi to the top of Lo.
      Lo = DAG.getNode(
          ISD::OR, dl, NVT, Lo,
          DAG.getNode(ISD::SHL, dl, NVT, Hi,
                      DAG.getConstant(ExcessBits, dl,
                                      TLI.getPointerTy(DAG.getDataLayout()))));
      // Move high bits to the right position in Hi.
      Hi = DAG.getNode(ExtType == ISD::SEXTLOAD ? ISD::SRA : ISD::SRL, dl, NVT,
                       Hi,
                       DAG.getConstant(NVT.getSizeInBits() - ExcessBits, dl,
                                       TLI.getPointerTy(DAG.getDataLayout())));
    }
  }

  // Legalized the chain result - switch anything that used the old chain to
  // use the new one.
  ReplaceValueWith(SDValue(N, 1), Ch);
}

void DAGTypeLegalizer::ExpandIntRes_Logical(SDNode *N,
                                            SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  SDValue LL, LH, RL, RH;
  GetExpandedInteger(N->getOperand(0), LL, LH);
  GetExpandedInteger(N->getOperand(1), RL, RH);
  Lo = DAG.getNode(N->getOpcode(), dl, LL.getValueType(), LL, RL);
  Hi = DAG.getNode(N->getOpcode(), dl, LL.getValueType(), LH, RH);
}

void DAGTypeLegalizer::ExpandIntRes_MUL(SDNode *N,
                                        SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), VT);
  SDLoc dl(N);

  SDValue LL, LH, RL, RH;
  GetExpandedInteger(N->getOperand(0), LL, LH);
  GetExpandedInteger(N->getOperand(1), RL, RH);

  if (TLI.expandMUL(N, Lo, Hi, NVT, DAG, LL, LH, RL, RH))
    return;

  // If nothing else, we can make a libcall.
  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i16)
    LC = RTLIB::MUL_I16;
  else if (VT == MVT::i32)
    LC = RTLIB::MUL_I32;
  else if (VT == MVT::i64)
    LC = RTLIB::MUL_I64;
  else if (VT == MVT::i128)
    LC = RTLIB::MUL_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported MUL!");

  SDValue Ops[2] = { N->getOperand(0), N->getOperand(1) };
  SplitInteger(TLI.makeLibCall(DAG, LC, VT, Ops, 2, true/*irrelevant*/,
                               dl).first,
               Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_SADDSUBO(SDNode *Node,
                                             SDValue &Lo, SDValue &Hi) {
  SDValue LHS = Node->getOperand(0);
  SDValue RHS = Node->getOperand(1);
  SDLoc dl(Node);

  // Expand the result by simply replacing it with the equivalent
  // non-overflow-checking operation.
  SDValue Sum = DAG.getNode(Node->getOpcode() == ISD::SADDO ?
                            ISD::ADD : ISD::SUB, dl, LHS.getValueType(),
                            LHS, RHS);
  SplitInteger(Sum, Lo, Hi);

  // Compute the overflow.
  //
  //   LHSSign -> LHS >= 0
  //   RHSSign -> RHS >= 0
  //   SumSign -> Sum >= 0
  //
  //   Add:
  //   Overflow -> (LHSSign == RHSSign) && (LHSSign != SumSign)
  //   Sub:
  //   Overflow -> (LHSSign != RHSSign) && (LHSSign != SumSign)
  //
  EVT OType = Node->getValueType(1);
  SDValue Zero = DAG.getConstant(0, dl, LHS.getValueType());

  SDValue LHSSign = DAG.getSetCC(dl, OType, LHS, Zero, ISD::SETGE);
  SDValue RHSSign = DAG.getSetCC(dl, OType, RHS, Zero, ISD::SETGE);
  SDValue SignsMatch = DAG.getSetCC(dl, OType, LHSSign, RHSSign,
                                    Node->getOpcode() == ISD::SADDO ?
                                    ISD::SETEQ : ISD::SETNE);

  SDValue SumSign = DAG.getSetCC(dl, OType, Sum, Zero, ISD::SETGE);
  SDValue SumSignNE = DAG.getSetCC(dl, OType, LHSSign, SumSign, ISD::SETNE);

  SDValue Cmp = DAG.getNode(ISD::AND, dl, OType, SignsMatch, SumSignNE);

  // Use the calculated overflow everywhere.
  ReplaceValueWith(SDValue(Node, 1), Cmp);
}

void DAGTypeLegalizer::ExpandIntRes_SDIV(SDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  SDLoc dl(N);
  SDValue Ops[2] = { N->getOperand(0), N->getOperand(1) };

  if (TLI.getOperationAction(ISD::SDIVREM, VT) == TargetLowering::Custom) {
    SDValue Res = DAG.getNode(ISD::SDIVREM, dl, DAG.getVTList(VT, VT), Ops);
    SplitInteger(Res.getValue(0), Lo, Hi);
    return;
  }

  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i16)
    LC = RTLIB::SDIV_I16;
  else if (VT == MVT::i32)
    LC = RTLIB::SDIV_I32;
  else if (VT == MVT::i64)
    LC = RTLIB::SDIV_I64;
  else if (VT == MVT::i128)
    LC = RTLIB::SDIV_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported SDIV!");

  SplitInteger(TLI.makeLibCall(DAG, LC, VT, Ops, 2, true, dl).first, Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_Shift(SDNode *N,
                                          SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  SDLoc dl(N);

  // If we can emit an efficient shift operation, do so now.  Check to see if
  // the RHS is a constant.
  if (ConstantSDNode *CN = dyn_cast<ConstantSDNode>(N->getOperand(1)))
    return ExpandShiftByConstant(N, CN->getAPIntValue(), Lo, Hi);

  // If we can determine that the high bit of the shift is zero or one, even if
  // the low bits are variable, emit this shift in an optimized form.
  if (ExpandShiftWithKnownAmountBit(N, Lo, Hi))
    return;

  // If this target supports shift_PARTS, use it.  First, map to the _PARTS opc.
  unsigned PartsOpc;
  if (N->getOpcode() == ISD::SHL) {
    PartsOpc = ISD::SHL_PARTS;
  } else if (N->getOpcode() == ISD::SRL) {
    PartsOpc = ISD::SRL_PARTS;
  } else {
    assert(N->getOpcode() == ISD::SRA && "Unknown shift!");
    PartsOpc = ISD::SRA_PARTS;
  }

  // Next check to see if the target supports this SHL_PARTS operation or if it
  // will custom expand it.
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), VT);
  TargetLowering::LegalizeAction Action = TLI.getOperationAction(PartsOpc, NVT);
  if ((Action == TargetLowering::Legal && TLI.isTypeLegal(NVT)) ||
      Action == TargetLowering::Custom) {
    // Expand the subcomponents.
    SDValue LHSL, LHSH;
    GetExpandedInteger(N->getOperand(0), LHSL, LHSH);
    EVT VT = LHSL.getValueType();

    // If the shift amount operand is coming from a vector legalization it may
    // have an illegal type.  Fix that first by casting the operand, otherwise
    // the new SHL_PARTS operation would need further legalization.
    SDValue ShiftOp = N->getOperand(1);
    EVT ShiftTy = TLI.getShiftAmountTy(VT, DAG.getDataLayout());
    assert(ShiftTy.getScalarType().getSizeInBits() >=
           Log2_32_Ceil(VT.getScalarType().getSizeInBits()) &&
           "ShiftAmountTy is too small to cover the range of this type!");
    if (ShiftOp.getValueType() != ShiftTy)
      ShiftOp = DAG.getZExtOrTrunc(ShiftOp, dl, ShiftTy);

    SDValue Ops[] = { LHSL, LHSH, ShiftOp };
    Lo = DAG.getNode(PartsOpc, dl, DAG.getVTList(VT, VT), Ops);
    Hi = Lo.getValue(1);
    return;
  }

  // Otherwise, emit a libcall.
  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  bool isSigned;
  if (N->getOpcode() == ISD::SHL) {
    isSigned = false; /*sign irrelevant*/
    if (VT == MVT::i16)
      LC = RTLIB::SHL_I16;
    else if (VT == MVT::i32)
      LC = RTLIB::SHL_I32;
    else if (VT == MVT::i64)
      LC = RTLIB::SHL_I64;
    else if (VT == MVT::i128)
      LC = RTLIB::SHL_I128;
  } else if (N->getOpcode() == ISD::SRL) {
    isSigned = false;
    if (VT == MVT::i16)
      LC = RTLIB::SRL_I16;
    else if (VT == MVT::i32)
      LC = RTLIB::SRL_I32;
    else if (VT == MVT::i64)
      LC = RTLIB::SRL_I64;
    else if (VT == MVT::i128)
      LC = RTLIB::SRL_I128;
  } else {
    assert(N->getOpcode() == ISD::SRA && "Unknown shift!");
    isSigned = true;
    if (VT == MVT::i16)
      LC = RTLIB::SRA_I16;
    else if (VT == MVT::i32)
      LC = RTLIB::SRA_I32;
    else if (VT == MVT::i64)
      LC = RTLIB::SRA_I64;
    else if (VT == MVT::i128)
      LC = RTLIB::SRA_I128;
  }

  if (LC != RTLIB::UNKNOWN_LIBCALL && TLI.getLibcallName(LC)) {
    SDValue Ops[2] = { N->getOperand(0), N->getOperand(1) };
    SplitInteger(TLI.makeLibCall(DAG, LC, VT, Ops, 2, isSigned, dl).first, Lo,
                 Hi);
    return;
  }

  if (!ExpandShiftWithUnknownAmountBit(N, Lo, Hi))
    llvm_unreachable("Unsupported shift!");
}

void DAGTypeLegalizer::ExpandIntRes_SIGN_EXTEND(SDNode *N,
                                                SDValue &Lo, SDValue &Hi) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDLoc dl(N);
  SDValue Op = N->getOperand(0);
  if (Op.getValueType().bitsLE(NVT)) {
    // The low part is sign extension of the input (degenerates to a copy).
    Lo = DAG.getNode(ISD::SIGN_EXTEND, dl, NVT, N->getOperand(0));
    // The high part is obtained by SRA'ing all but one of the bits of low part.
    unsigned LoSize = NVT.getSizeInBits();
    Hi = DAG.getNode(
        ISD::SRA, dl, NVT, Lo,
        DAG.getConstant(LoSize - 1, dl, TLI.getPointerTy(DAG.getDataLayout())));
  } else {
    // For example, extension of an i48 to an i64.  The operand type necessarily
    // promotes to the result type, so will end up being expanded too.
    assert(getTypeAction(Op.getValueType()) ==
           TargetLowering::TypePromoteInteger &&
           "Only know how to promote this result!");
    SDValue Res = GetPromotedInteger(Op);
    assert(Res.getValueType() == N->getValueType(0) &&
           "Operand over promoted?");
    // Split the promoted operand.  This will simplify when it is expanded.
    SplitInteger(Res, Lo, Hi);
    unsigned ExcessBits =
      Op.getValueType().getSizeInBits() - NVT.getSizeInBits();
    Hi = DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, Hi.getValueType(), Hi,
                     DAG.getValueType(EVT::getIntegerVT(*DAG.getContext(),
                                                        ExcessBits)));
  }
}

void DAGTypeLegalizer::
ExpandIntRes_SIGN_EXTEND_INREG(SDNode *N, SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  EVT EVT = cast<VTSDNode>(N->getOperand(1))->getVT();

  if (EVT.bitsLE(Lo.getValueType())) {
    // sext_inreg the low part if needed.
    Lo = DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, Lo.getValueType(), Lo,
                     N->getOperand(1));

    // The high part gets the sign extension from the lo-part.  This handles
    // things like sextinreg V:i64 from i8.
    Hi = DAG.getNode(ISD::SRA, dl, Hi.getValueType(), Lo,
                     DAG.getConstant(Hi.getValueType().getSizeInBits() - 1, dl,
                                     TLI.getPointerTy(DAG.getDataLayout())));
  } else {
    // For example, extension of an i48 to an i64.  Leave the low part alone,
    // sext_inreg the high part.
    unsigned ExcessBits =
      EVT.getSizeInBits() - Lo.getValueType().getSizeInBits();
    Hi = DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, Hi.getValueType(), Hi,
                     DAG.getValueType(EVT::getIntegerVT(*DAG.getContext(),
                                                        ExcessBits)));
  }
}

void DAGTypeLegalizer::ExpandIntRes_SREM(SDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  SDLoc dl(N);
  SDValue Ops[2] = { N->getOperand(0), N->getOperand(1) };

  if (TLI.getOperationAction(ISD::SDIVREM, VT) == TargetLowering::Custom) {
    SDValue Res = DAG.getNode(ISD::SDIVREM, dl, DAG.getVTList(VT, VT), Ops);
    SplitInteger(Res.getValue(1), Lo, Hi);
    return;
  }

  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i16)
    LC = RTLIB::SREM_I16;
  else if (VT == MVT::i32)
    LC = RTLIB::SREM_I32;
  else if (VT == MVT::i64)
    LC = RTLIB::SREM_I64;
  else if (VT == MVT::i128)
    LC = RTLIB::SREM_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported SREM!");

  SplitInteger(TLI.makeLibCall(DAG, LC, VT, Ops, 2, true, dl).first, Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_TRUNCATE(SDNode *N,
                                             SDValue &Lo, SDValue &Hi) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDLoc dl(N);
  Lo = DAG.getNode(ISD::TRUNCATE, dl, NVT, N->getOperand(0));
  Hi = DAG.getNode(ISD::SRL, dl, N->getOperand(0).getValueType(),
                   N->getOperand(0),
                   DAG.getConstant(NVT.getSizeInBits(), dl,
                                   TLI.getPointerTy(DAG.getDataLayout())));
  Hi = DAG.getNode(ISD::TRUNCATE, dl, NVT, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_UADDSUBO(SDNode *N,
                                             SDValue &Lo, SDValue &Hi) {
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  SDLoc dl(N);

  // Expand the result by simply replacing it with the equivalent
  // non-overflow-checking operation.
  SDValue Sum = DAG.getNode(N->getOpcode() == ISD::UADDO ?
                            ISD::ADD : ISD::SUB, dl, LHS.getValueType(),
                            LHS, RHS);
  SplitInteger(Sum, Lo, Hi);

  // Calculate the overflow: addition overflows iff a + b < a, and subtraction
  // overflows iff a - b > a.
  SDValue Ofl = DAG.getSetCC(dl, N->getValueType(1), Sum, LHS,
                             N->getOpcode () == ISD::UADDO ?
                             ISD::SETULT : ISD::SETUGT);

  // Use the calculated overflow everywhere.
  ReplaceValueWith(SDValue(N, 1), Ofl);
}

void DAGTypeLegalizer::ExpandIntRes_XMULO(SDNode *N,
                                          SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  SDLoc dl(N);

  // A divide for UMULO should be faster than a function call.
  if (N->getOpcode() == ISD::UMULO) {
    SDValue LHS = N->getOperand(0), RHS = N->getOperand(1);

    SDValue MUL = DAG.getNode(ISD::MUL, dl, LHS.getValueType(), LHS, RHS);
    SplitInteger(MUL, Lo, Hi);

    // A divide for UMULO will be faster than a function call. Select to
    // make sure we aren't using 0.
    SDValue isZero = DAG.getSetCC(dl, getSetCCResultType(VT),
                                  RHS, DAG.getConstant(0, dl, VT), ISD::SETEQ);
    SDValue NotZero = DAG.getSelect(dl, VT, isZero,
                                    DAG.getConstant(1, dl, VT), RHS);
    SDValue DIV = DAG.getNode(ISD::UDIV, dl, VT, MUL, NotZero);
    SDValue Overflow = DAG.getSetCC(dl, N->getValueType(1), DIV, LHS,
                                    ISD::SETNE);
    Overflow = DAG.getSelect(dl, N->getValueType(1), isZero,
                             DAG.getConstant(0, dl, N->getValueType(1)),
                             Overflow);
    ReplaceValueWith(SDValue(N, 1), Overflow);
    return;
  }

  Type *RetTy = VT.getTypeForEVT(*DAG.getContext());
  EVT PtrVT = TLI.getPointerTy(DAG.getDataLayout());
  Type *PtrTy = PtrVT.getTypeForEVT(*DAG.getContext());

  // Replace this with a libcall that will check overflow.
  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i32)
    LC = RTLIB::MULO_I32;
  else if (VT == MVT::i64)
    LC = RTLIB::MULO_I64;
  else if (VT == MVT::i128)
    LC = RTLIB::MULO_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported XMULO!");

  SDValue Temp = DAG.CreateStackTemporary(PtrVT);
  // Temporary for the overflow value, default it to zero.
  SDValue Chain = DAG.getStore(DAG.getEntryNode(), dl,
                               DAG.getConstant(0, dl, PtrVT), Temp,
                               MachinePointerInfo(), false, false, 0);

  TargetLowering::ArgListTy Args;
  TargetLowering::ArgListEntry Entry;
  for (const SDValue &Op : N->op_values()) {
    EVT ArgVT = Op.getValueType();
    Type *ArgTy = ArgVT.getTypeForEVT(*DAG.getContext());
    Entry.Node = Op;
    Entry.Ty = ArgTy;
    Entry.isSExt = true;
    Entry.isZExt = false;
    Args.push_back(Entry);
  }

  // Also pass the address of the overflow check.
  Entry.Node = Temp;
  Entry.Ty = PtrTy->getPointerTo();
  Entry.isSExt = true;
  Entry.isZExt = false;
  Args.push_back(Entry);

  SDValue Func = DAG.getExternalSymbol(TLI.getLibcallName(LC), PtrVT);

  TargetLowering::CallLoweringInfo CLI(DAG);
  CLI.setDebugLoc(dl).setChain(Chain)
    .setCallee(TLI.getLibcallCallingConv(LC), RetTy, Func, std::move(Args), 0)
    .setSExtResult();

  std::pair<SDValue, SDValue> CallInfo = TLI.LowerCallTo(CLI);

  SplitInteger(CallInfo.first, Lo, Hi);
  SDValue Temp2 = DAG.getLoad(PtrVT, dl, CallInfo.second, Temp,
                              MachinePointerInfo(), false, false, false, 0);
  SDValue Ofl = DAG.getSetCC(dl, N->getValueType(1), Temp2,
                             DAG.getConstant(0, dl, PtrVT),
                             ISD::SETNE);
  // Use the overflow from the libcall everywhere.
  ReplaceValueWith(SDValue(N, 1), Ofl);
}

void DAGTypeLegalizer::ExpandIntRes_UDIV(SDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  SDLoc dl(N);
  SDValue Ops[2] = { N->getOperand(0), N->getOperand(1) };

  if (TLI.getOperationAction(ISD::UDIVREM, VT) == TargetLowering::Custom) {
    SDValue Res = DAG.getNode(ISD::UDIVREM, dl, DAG.getVTList(VT, VT), Ops);
    SplitInteger(Res.getValue(0), Lo, Hi);
    return;
  }

  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i16)
    LC = RTLIB::UDIV_I16;
  else if (VT == MVT::i32)
    LC = RTLIB::UDIV_I32;
  else if (VT == MVT::i64)
    LC = RTLIB::UDIV_I64;
  else if (VT == MVT::i128)
    LC = RTLIB::UDIV_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported UDIV!");

  SplitInteger(TLI.makeLibCall(DAG, LC, VT, Ops, 2, false, dl).first, Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_UREM(SDNode *N,
                                         SDValue &Lo, SDValue &Hi) {
  EVT VT = N->getValueType(0);
  SDLoc dl(N);
  SDValue Ops[2] = { N->getOperand(0), N->getOperand(1) };

  if (TLI.getOperationAction(ISD::UDIVREM, VT) == TargetLowering::Custom) {
    SDValue Res = DAG.getNode(ISD::UDIVREM, dl, DAG.getVTList(VT, VT), Ops);
    SplitInteger(Res.getValue(1), Lo, Hi);
    return;
  }

  RTLIB::Libcall LC = RTLIB::UNKNOWN_LIBCALL;
  if (VT == MVT::i16)
    LC = RTLIB::UREM_I16;
  else if (VT == MVT::i32)
    LC = RTLIB::UREM_I32;
  else if (VT == MVT::i64)
    LC = RTLIB::UREM_I64;
  else if (VT == MVT::i128)
    LC = RTLIB::UREM_I128;
  assert(LC != RTLIB::UNKNOWN_LIBCALL && "Unsupported UREM!");

  SplitInteger(TLI.makeLibCall(DAG, LC, VT, Ops, 2, false, dl).first, Lo, Hi);
}

void DAGTypeLegalizer::ExpandIntRes_ZERO_EXTEND(SDNode *N,
                                                SDValue &Lo, SDValue &Hi) {
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), N->getValueType(0));
  SDLoc dl(N);
  SDValue Op = N->getOperand(0);
  if (Op.getValueType().bitsLE(NVT)) {
    // The low part is zero extension of the input (degenerates to a copy).
    Lo = DAG.getNode(ISD::ZERO_EXTEND, dl, NVT, N->getOperand(0));
    Hi = DAG.getConstant(0, dl, NVT);   // The high part is just a zero.
  } else {
    // For example, extension of an i48 to an i64.  The operand type necessarily
    // promotes to the result type, so will end up being expanded too.
    assert(getTypeAction(Op.getValueType()) ==
           TargetLowering::TypePromoteInteger &&
           "Only know how to promote this result!");
    SDValue Res = GetPromotedInteger(Op);
    assert(Res.getValueType() == N->getValueType(0) &&
           "Operand over promoted?");
    // Split the promoted operand.  This will simplify when it is expanded.
    SplitInteger(Res, Lo, Hi);
    unsigned ExcessBits =
      Op.getValueType().getSizeInBits() - NVT.getSizeInBits();
    Hi = DAG.getZeroExtendInReg(Hi, dl,
                                EVT::getIntegerVT(*DAG.getContext(),
                                                  ExcessBits));
  }
}

void DAGTypeLegalizer::ExpandIntRes_ATOMIC_LOAD(SDNode *N,
                                                SDValue &Lo, SDValue &Hi) {
  SDLoc dl(N);
  EVT VT = cast<AtomicSDNode>(N)->getMemoryVT();
  SDVTList VTs = DAG.getVTList(VT, MVT::i1, MVT::Other);
  SDValue Zero = DAG.getConstant(0, dl, VT);
  SDValue Swap = DAG.getAtomicCmpSwap(
      ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS, dl,
      cast<AtomicSDNode>(N)->getMemoryVT(), VTs, N->getOperand(0),
      N->getOperand(1), Zero, Zero, cast<AtomicSDNode>(N)->getMemOperand(),
      cast<AtomicSDNode>(N)->getOrdering(),
      cast<AtomicSDNode>(N)->getOrdering(),
      cast<AtomicSDNode>(N)->getSynchScope());

  ReplaceValueWith(SDValue(N, 0), Swap.getValue(0));
  ReplaceValueWith(SDValue(N, 1), Swap.getValue(2));
}

//===----------------------------------------------------------------------===//
//  Integer Operand Expansion
//===----------------------------------------------------------------------===//

/// ExpandIntegerOperand - This method is called when the specified operand of
/// the specified node is found to need expansion.  At this point, all of the
/// result types of the node are known to be legal, but other operands of the
/// node may need promotion or expansion as well as the specified one.
bool DAGTypeLegalizer::ExpandIntegerOperand(SDNode *N, unsigned OpNo) {
  DEBUG(dbgs() << "Expand integer operand: "; N->dump(&DAG); dbgs() << "\n");
  SDValue Res = SDValue();

  if (CustomLowerNode(N, N->getOperand(OpNo).getValueType(), false))
    return false;

  switch (N->getOpcode()) {
  default:
  #ifndef NDEBUG
    dbgs() << "ExpandIntegerOperand Op #" << OpNo << ": ";
    N->dump(&DAG); dbgs() << "\n";
  #endif
    llvm_unreachable("Do not know how to expand this operator's operand!");

  case ISD::BITCAST:           Res = ExpandOp_BITCAST(N); break;
  case ISD::BR_CC:             Res = ExpandIntOp_BR_CC(N); break;
  case ISD::BUILD_VECTOR:      Res = ExpandOp_BUILD_VECTOR(N); break;
  case ISD::EXTRACT_ELEMENT:   Res = ExpandOp_EXTRACT_ELEMENT(N); break;
  case ISD::INSERT_VECTOR_ELT: Res = ExpandOp_INSERT_VECTOR_ELT(N); break;
  case ISD::SCALAR_TO_VECTOR:  Res = ExpandOp_SCALAR_TO_VECTOR(N); break;
  case ISD::SELECT_CC:         Res = ExpandIntOp_SELECT_CC(N); break;
  case ISD::SETCC:             Res = ExpandIntOp_SETCC(N); break;
  case ISD::SINT_TO_FP:        Res = ExpandIntOp_SINT_TO_FP(N); break;
  case ISD::STORE:   Res = ExpandIntOp_STORE(cast<StoreSDNode>(N), OpNo); break;
  case ISD::TRUNCATE:          Res = ExpandIntOp_TRUNCATE(N); break;
  case ISD::UINT_TO_FP:        Res = ExpandIntOp_UINT_TO_FP(N); break;

  case ISD::SHL:
  case ISD::SRA:
  case ISD::SRL:
  case ISD::ROTL:
  case ISD::ROTR:              Res = ExpandIntOp_Shift(N); break;
  case ISD::RETURNADDR:
  case ISD::FRAMEADDR:         Res = ExpandIntOp_RETURNADDR(N); break;

  case ISD::ATOMIC_STORE:      Res = ExpandIntOp_ATOMIC_STORE(N); break;
  }

  // If the result is null, the sub-method took care of registering results etc.
  if (!Res.getNode()) return false;

  // If the result is N, the sub-method updated N in place.  Tell the legalizer
  // core about this.
  if (Res.getNode() == N)
    return true;

  assert(Res.getValueType() == N->getValueType(0) && N->getNumValues() == 1 &&
         "Invalid operand expansion");

  ReplaceValueWith(SDValue(N, 0), Res);
  return false;
}

/// IntegerExpandSetCCOperands - Expand the operands of a comparison.  This code
/// is shared among BR_CC, SELECT_CC, and SETCC handlers.
void DAGTypeLegalizer::IntegerExpandSetCCOperands(SDValue &NewLHS,
                                                  SDValue &NewRHS,
                                                  ISD::CondCode &CCCode,
                                                  SDLoc dl) {
  SDValue LHSLo, LHSHi, RHSLo, RHSHi;
  GetExpandedInteger(NewLHS, LHSLo, LHSHi);
  GetExpandedInteger(NewRHS, RHSLo, RHSHi);

  if (CCCode == ISD::SETEQ || CCCode == ISD::SETNE) {
    if (RHSLo == RHSHi) {
      if (ConstantSDNode *RHSCST = dyn_cast<ConstantSDNode>(RHSLo)) {
        if (RHSCST->isAllOnesValue()) {
          // Equality comparison to -1.
          NewLHS = DAG.getNode(ISD::AND, dl,
                               LHSLo.getValueType(), LHSLo, LHSHi);
          NewRHS = RHSLo;
          return;
        }
      }
    }

    NewLHS = DAG.getNode(ISD::XOR, dl, LHSLo.getValueType(), LHSLo, RHSLo);
    NewRHS = DAG.getNode(ISD::XOR, dl, LHSLo.getValueType(), LHSHi, RHSHi);
    NewLHS = DAG.getNode(ISD::OR, dl, NewLHS.getValueType(), NewLHS, NewRHS);
    NewRHS = DAG.getConstant(0, dl, NewLHS.getValueType());
    return;
  }

  // If this is a comparison of the sign bit, just look at the top part.
  // X > -1,  x < 0
  if (ConstantSDNode *CST = dyn_cast<ConstantSDNode>(NewRHS))
    if ((CCCode == ISD::SETLT && CST->isNullValue()) ||     // X < 0
        (CCCode == ISD::SETGT && CST->isAllOnesValue())) {  // X > -1
      NewLHS = LHSHi;
      NewRHS = RHSHi;
      return;
    }

  // FIXME: This generated code sucks.
  ISD::CondCode LowCC;
  switch (CCCode) {
  default: llvm_unreachable("Unknown integer setcc!");
  case ISD::SETLT:
  case ISD::SETULT: LowCC = ISD::SETULT; break;
  case ISD::SETGT:
  case ISD::SETUGT: LowCC = ISD::SETUGT; break;
  case ISD::SETLE:
  case ISD::SETULE: LowCC = ISD::SETULE; break;
  case ISD::SETGE:
  case ISD::SETUGE: LowCC = ISD::SETUGE; break;
  }

  // Tmp1 = lo(op1) < lo(op2)   // Always unsigned comparison
  // Tmp2 = hi(op1) < hi(op2)   // Signedness depends on operands
  // dest = hi(op1) == hi(op2) ? Tmp1 : Tmp2;

  // NOTE: on targets without efficient SELECT of bools, we can always use
  // this identity: (B1 ? B2 : B3) --> (B1 & B2)|(!B1&B3)
  TargetLowering::DAGCombinerInfo DagCombineInfo(DAG, AfterLegalizeTypes, true,
                                                 nullptr);
  SDValue Tmp1, Tmp2;
  if (TLI.isTypeLegal(LHSLo.getValueType()) &&
      TLI.isTypeLegal(RHSLo.getValueType()))
    Tmp1 = TLI.SimplifySetCC(getSetCCResultType(LHSLo.getValueType()),
                             LHSLo, RHSLo, LowCC, false, DagCombineInfo, dl);
  if (!Tmp1.getNode())
    Tmp1 = DAG.getSetCC(dl, getSetCCResultType(LHSLo.getValueType()),
                        LHSLo, RHSLo, LowCC);
  if (TLI.isTypeLegal(LHSHi.getValueType()) &&
      TLI.isTypeLegal(RHSHi.getValueType()))
    Tmp2 = TLI.SimplifySetCC(getSetCCResultType(LHSHi.getValueType()),
                             LHSHi, RHSHi, CCCode, false, DagCombineInfo, dl);
  if (!Tmp2.getNode())
    Tmp2 = DAG.getNode(ISD::SETCC, dl,
                       getSetCCResultType(LHSHi.getValueType()),
                       LHSHi, RHSHi, DAG.getCondCode(CCCode));

  ConstantSDNode *Tmp1C = dyn_cast<ConstantSDNode>(Tmp1.getNode());
  ConstantSDNode *Tmp2C = dyn_cast<ConstantSDNode>(Tmp2.getNode());
  if ((Tmp1C && Tmp1C->isNullValue()) ||
      (Tmp2C && Tmp2C->isNullValue() &&
       (CCCode == ISD::SETLE || CCCode == ISD::SETGE ||
        CCCode == ISD::SETUGE || CCCode == ISD::SETULE)) ||
      (Tmp2C && Tmp2C->getAPIntValue() == 1 &&
       (CCCode == ISD::SETLT || CCCode == ISD::SETGT ||
        CCCode == ISD::SETUGT || CCCode == ISD::SETULT))) {
    // low part is known false, returns high part.
    // For LE / GE, if high part is known false, ignore the low part.
    // For LT / GT, if high part is known true, ignore the low part.
    NewLHS = Tmp2;
    NewRHS = SDValue();
    return;
  }

  NewLHS = TLI.SimplifySetCC(getSetCCResultType(LHSHi.getValueType()),
                             LHSHi, RHSHi, ISD::SETEQ, false,
                             DagCombineInfo, dl);
  if (!NewLHS.getNode())
    NewLHS = DAG.getSetCC(dl, getSetCCResultType(LHSHi.getValueType()),
                          LHSHi, RHSHi, ISD::SETEQ);
  NewLHS = DAG.getSelect(dl, Tmp1.getValueType(),
                         NewLHS, Tmp1, Tmp2);
  NewRHS = SDValue();
}

SDValue DAGTypeLegalizer::ExpandIntOp_BR_CC(SDNode *N) {
  SDValue NewLHS = N->getOperand(2), NewRHS = N->getOperand(3);
  ISD::CondCode CCCode = cast<CondCodeSDNode>(N->getOperand(1))->get();
  IntegerExpandSetCCOperands(NewLHS, NewRHS, CCCode, SDLoc(N));

  // If ExpandSetCCOperands returned a scalar, we need to compare the result
  // against zero to select between true and false values.
  if (!NewRHS.getNode()) {
    NewRHS = DAG.getConstant(0, SDLoc(N), NewLHS.getValueType());
    CCCode = ISD::SETNE;
  }

  // Update N to have the operands specified.
  return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0),
                                DAG.getCondCode(CCCode), NewLHS, NewRHS,
                                N->getOperand(4)), 0);
}

SDValue DAGTypeLegalizer::ExpandIntOp_SELECT_CC(SDNode *N) {
  SDValue NewLHS = N->getOperand(0), NewRHS = N->getOperand(1);
  ISD::CondCode CCCode = cast<CondCodeSDNode>(N->getOperand(4))->get();
  IntegerExpandSetCCOperands(NewLHS, NewRHS, CCCode, SDLoc(N));

  // If ExpandSetCCOperands returned a scalar, we need to compare the result
  // against zero to select between true and false values.
  if (!NewRHS.getNode()) {
    NewRHS = DAG.getConstant(0, SDLoc(N), NewLHS.getValueType());
    CCCode = ISD::SETNE;
  }

  // Update N to have the operands specified.
  return SDValue(DAG.UpdateNodeOperands(N, NewLHS, NewRHS,
                                N->getOperand(2), N->getOperand(3),
                                DAG.getCondCode(CCCode)), 0);
}

SDValue DAGTypeLegalizer::ExpandIntOp_SETCC(SDNode *N) {
  SDValue NewLHS = N->getOperand(0), NewRHS = N->getOperand(1);
  ISD::CondCode CCCode = cast<CondCodeSDNode>(N->getOperand(2))->get();
  IntegerExpandSetCCOperands(NewLHS, NewRHS, CCCode, SDLoc(N));

  // If ExpandSetCCOperands returned a scalar, use it.
  if (!NewRHS.getNode()) {
    assert(NewLHS.getValueType() == N->getValueType(0) &&
           "Unexpected setcc expansion!");
    return NewLHS;
  }

  // Otherwise, update N to have the operands specified.
  return SDValue(DAG.UpdateNodeOperands(N, NewLHS, NewRHS,
                                DAG.getCondCode(CCCode)), 0);
}

SDValue DAGTypeLegalizer::ExpandIntOp_Shift(SDNode *N) {
  // The value being shifted is legal, but the shift amount is too big.
  // It follows that either the result of the shift is undefined, or the
  // upper half of the shift amount is zero.  Just use the lower half.
  SDValue Lo, Hi;
  GetExpandedInteger(N->getOperand(1), Lo, Hi);
  return SDValue(DAG.UpdateNodeOperands(N, N->getOperand(0), Lo), 0);
}

SDValue DAGTypeLegalizer::ExpandIntOp_RETURNADDR(SDNode *N) {
  // The argument of RETURNADDR / FRAMEADDR builtin is 32 bit contant.  This
  // surely makes pretty nice problems on 8/16 bit targets. Just truncate this
  // constant to valid type.
  SDValue Lo, Hi;
  GetExpandedInteger(N->getOperand(0), Lo, Hi);
  return SDValue(DAG.UpdateNodeOperands(N, Lo), 0);
}

SDValue DAGTypeLegalizer::ExpandIntOp_SINT_TO_FP(SDNode *N) {
  SDValue Op = N->getOperand(0);
  EVT DstVT = N->getValueType(0);
  RTLIB::Libcall LC = RTLIB::getSINTTOFP(Op.getValueType(), DstVT);
  assert(LC != RTLIB::UNKNOWN_LIBCALL &&
         "Don't know how to expand this SINT_TO_FP!");
  return TLI.makeLibCall(DAG, LC, DstVT, &Op, 1, true, SDLoc(N)).first;
}

SDValue DAGTypeLegalizer::ExpandIntOp_STORE(StoreSDNode *N, unsigned OpNo) {
  if (ISD::isNormalStore(N))
    return ExpandOp_NormalStore(N, OpNo);

  assert(ISD::isUNINDEXEDStore(N) && "Indexed store during type legalization!");
  assert(OpNo == 1 && "Can only expand the stored value so far");

  EVT VT = N->getOperand(1).getValueType();
  EVT NVT = TLI.getTypeToTransformTo(*DAG.getContext(), VT);
  SDValue Ch  = N->getChain();
  SDValue Ptr = N->getBasePtr();
  unsigned Alignment = N->getAlignment();
  bool isVolatile = N->isVolatile();
  bool isNonTemporal = N->isNonTemporal();
  AAMDNodes AAInfo = N->getAAInfo();
  SDLoc dl(N);
  SDValue Lo, Hi;

  assert(NVT.isByteSized() && "Expanded type not byte sized!");

  if (N->getMemoryVT().bitsLE(NVT)) {
    GetExpandedInteger(N->getValue(), Lo, Hi);
    return DAG.getTruncStore(Ch, dl, Lo, Ptr, N->getPointerInfo(),
                             N->getMemoryVT(), isVolatile, isNonTemporal,
                             Alignment, AAInfo);
  }

  if (DAG.getDataLayout().isLittleEndian()) {
    // Little-endian - low bits are at low addresses.
    GetExpandedInteger(N->getValue(), Lo, Hi);

    Lo = DAG.getStore(Ch, dl, Lo, Ptr, N->getPointerInfo(),
                      isVolatile, isNonTemporal, Alignment, AAInfo);

    unsigned ExcessBits =
      N->getMemoryVT().getSizeInBits() - NVT.getSizeInBits();
    EVT NEVT = EVT::getIntegerVT(*DAG.getContext(), ExcessBits);

    // Increment the pointer to the other half.
    unsigned IncrementSize = NVT.getSizeInBits()/8;
    Ptr = DAG.getNode(ISD::ADD, dl, Ptr.getValueType(), Ptr,
                      DAG.getConstant(IncrementSize, dl, Ptr.getValueType()));
    Hi = DAG.getTruncStore(Ch, dl, Hi, Ptr,
                           N->getPointerInfo().getWithOffset(IncrementSize),
                           NEVT, isVolatile, isNonTemporal,
                           MinAlign(Alignment, IncrementSize), AAInfo);
    return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Lo, Hi);
  }

  // Big-endian - high bits are at low addresses.  Favor aligned stores at
  // the cost of some bit-fiddling.
  GetExpandedInteger(N->getValue(), Lo, Hi);

  EVT ExtVT = N->getMemoryVT();
  unsigned EBytes = ExtVT.getStoreSize();
  unsigned IncrementSize = NVT.getSizeInBits()/8;
  unsigned ExcessBits = (EBytes - IncrementSize)*8;
  EVT HiVT = EVT::getIntegerVT(*DAG.getContext(),
                               ExtVT.getSizeInBits() - ExcessBits);

  if (ExcessBits < NVT.getSizeInBits()) {
    // Transfer high bits from the top of Lo to the bottom of Hi.
    Hi = DAG.getNode(ISD::SHL, dl, NVT, Hi,
                     DAG.getConstant(NVT.getSizeInBits() - ExcessBits, dl,
                                     TLI.getPointerTy(DAG.getDataLayout())));
    Hi = DAG.getNode(
        ISD::OR, dl, NVT, Hi,
        DAG.getNode(ISD::SRL, dl, NVT, Lo,
                    DAG.getConstant(ExcessBits, dl,
                                    TLI.getPointerTy(DAG.getDataLayout()))));
  }

  // Store both the high bits and maybe some of the low bits.
  Hi = DAG.getTruncStore(Ch, dl, Hi, Ptr, N->getPointerInfo(),
                         HiVT, isVolatile, isNonTemporal, Alignment, AAInfo);

  // Increment the pointer to the other half.
  Ptr = DAG.getNode(ISD::ADD, dl, Ptr.getValueType(), Ptr,
                    DAG.getConstant(IncrementSize, dl, Ptr.getValueType()));
  // Store the lowest ExcessBits bits in the second half.
  Lo = DAG.getTruncStore(Ch, dl, Lo, Ptr,
                         N->getPointerInfo().getWithOffset(IncrementSize),
                         EVT::getIntegerVT(*DAG.getContext(), ExcessBits),
                         isVolatile, isNonTemporal,
                         MinAlign(Alignment, IncrementSize), AAInfo);
  return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Lo, Hi);
}

SDValue DAGTypeLegalizer::ExpandIntOp_TRUNCATE(SDNode *N) {
  SDValue InL, InH;
  GetExpandedInteger(N->getOperand(0), InL, InH);
  // Just truncate the low part of the source.
  return DAG.getNode(ISD::TRUNCATE, SDLoc(N), N->getValueType(0), InL);
}

SDValue DAGTypeLegalizer::ExpandIntOp_UINT_TO_FP(SDNode *N) {
  SDValue Op = N->getOperand(0);
  EVT SrcVT = Op.getValueType();
  EVT DstVT = N->getValueType(0);
  SDLoc dl(N);

  // The following optimization is valid only if every value in SrcVT (when
  // treated as signed) is representable in DstVT.  Check that the mantissa
  // size of DstVT is >= than the number of bits in SrcVT -1.
  const fltSemantics &sem = DAG.EVTToAPFloatSemantics(DstVT);
  if (APFloat::semanticsPrecision(sem) >= SrcVT.getSizeInBits()-1 &&
      TLI.getOperationAction(ISD::SINT_TO_FP, SrcVT) == TargetLowering::Custom){
    // Do a signed conversion then adjust the result.
    SDValue SignedConv = DAG.getNode(ISD::SINT_TO_FP, dl, DstVT, Op);
    SignedConv = TLI.LowerOperation(SignedConv, DAG);

    // The result of the signed conversion needs adjusting if the 'sign bit' of
    // the incoming integer was set.  To handle this, we dynamically test to see
    // if it is set, and, if so, add a fudge factor.

    const uint64_t F32TwoE32  = 0x4F800000ULL;
    const uint64_t F32TwoE64  = 0x5F800000ULL;
    const uint64_t F32TwoE128 = 0x7F800000ULL;

    APInt FF(32, 0);
    if (SrcVT == MVT::i32)
      FF = APInt(32, F32TwoE32);
    else if (SrcVT == MVT::i64)
      FF = APInt(32, F32TwoE64);
    else if (SrcVT == MVT::i128)
      FF = APInt(32, F32TwoE128);
    else
      llvm_unreachable("Unsupported UINT_TO_FP!");

    // Check whether the sign bit is set.
    SDValue Lo, Hi;
    GetExpandedInteger(Op, Lo, Hi);
    SDValue SignSet = DAG.getSetCC(dl,
                                   getSetCCResultType(Hi.getValueType()),
                                   Hi,
                                   DAG.getConstant(0, dl, Hi.getValueType()),
                                   ISD::SETLT);

    // Build a 64 bit pair (0, FF) in the constant pool, with FF in the lo bits.
    SDValue FudgePtr =
        DAG.getConstantPool(ConstantInt::get(*DAG.getContext(), FF.zext(64)),
                            TLI.getPointerTy(DAG.getDataLayout()));

    // Get a pointer to FF if the sign bit was set, or to 0 otherwise.
    SDValue Zero = DAG.getIntPtrConstant(0, dl);
    SDValue Four = DAG.getIntPtrConstant(4, dl);
    if (DAG.getDataLayout().isBigEndian())
      std::swap(Zero, Four);
    SDValue Offset = DAG.getSelect(dl, Zero.getValueType(), SignSet,
                                   Zero, Four);
    unsigned Alignment = cast<ConstantPoolSDNode>(FudgePtr)->getAlignment();
    FudgePtr = DAG.getNode(ISD::ADD, dl, FudgePtr.getValueType(),
                           FudgePtr, Offset);
    Alignment = std::min(Alignment, 4u);

    // Load the value out, extending it from f32 to the destination float type.
    // FIXME: Avoid the extend by constructing the right constant pool?
    SDValue Fudge = DAG.getExtLoad(ISD::EXTLOAD, dl, DstVT, DAG.getEntryNode(),
                                   FudgePtr,
                                   MachinePointerInfo::getConstantPool(),
                                   MVT::f32,
                                   false, false, false, Alignment);
    return DAG.getNode(ISD::FADD, dl, DstVT, SignedConv, Fudge);
  }

  // Otherwise, use a libcall.
  RTLIB::Libcall LC = RTLIB::getUINTTOFP(SrcVT, DstVT);
  assert(LC != RTLIB::UNKNOWN_LIBCALL &&
         "Don't know how to expand this UINT_TO_FP!");
  return TLI.makeLibCall(DAG, LC, DstVT, &Op, 1, true, dl).first;
}

SDValue DAGTypeLegalizer::ExpandIntOp_ATOMIC_STORE(SDNode *N) {
  SDLoc dl(N);
  SDValue Swap = DAG.getAtomic(ISD::ATOMIC_SWAP, dl,
                               cast<AtomicSDNode>(N)->getMemoryVT(),
                               N->getOperand(0),
                               N->getOperand(1), N->getOperand(2),
                               cast<AtomicSDNode>(N)->getMemOperand(),
                               cast<AtomicSDNode>(N)->getOrdering(),
                               cast<AtomicSDNode>(N)->getSynchScope());
  return Swap.getValue(1);
}


SDValue DAGTypeLegalizer::PromoteIntRes_EXTRACT_SUBVECTOR(SDNode *N) {
  SDValue InOp0 = N->getOperand(0);
  EVT InVT = InOp0.getValueType();

  EVT OutVT = N->getValueType(0);
  EVT NOutVT = TLI.getTypeToTransformTo(*DAG.getContext(), OutVT);
  assert(NOutVT.isVector() && "This type must be promoted to a vector type");
  unsigned OutNumElems = OutVT.getVectorNumElements();
  EVT NOutVTElem = NOutVT.getVectorElementType();

  SDLoc dl(N);
  SDValue BaseIdx = N->getOperand(1);

  SmallVector<SDValue, 8> Ops;
  Ops.reserve(OutNumElems);
  for (unsigned i = 0; i != OutNumElems; ++i) {

    // Extract the element from the original vector.
    SDValue Index = DAG.getNode(ISD::ADD, dl, BaseIdx.getValueType(),
      BaseIdx, DAG.getConstant(i, dl, BaseIdx.getValueType()));
    SDValue Ext = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, dl,
      InVT.getVectorElementType(), N->getOperand(0), Index);

    SDValue Op = DAG.getNode(ISD::ANY_EXTEND, dl, NOutVTElem, Ext);
    // Insert the converted element to the new vector.
    Ops.push_back(Op);
  }

  return DAG.getNode(ISD::BUILD_VECTOR, dl, NOutVT, Ops);
}


SDValue DAGTypeLegalizer::PromoteIntRes_VECTOR_SHUFFLE(SDNode *N) {
  ShuffleVectorSDNode *SV = cast<ShuffleVectorSDNode>(N);
  EVT VT = N->getValueType(0);
  SDLoc dl(N);

  ArrayRef<int> NewMask = SV->getMask().slice(0, VT.getVectorNumElements());

  SDValue V0 = GetPromotedInteger(N->getOperand(0));
  SDValue V1 = GetPromotedInteger(N->getOperand(1));
  EVT OutVT = V0.getValueType();

  return DAG.getVectorShuffle(OutVT, dl, V0, V1, NewMask);
}


SDValue DAGTypeLegalizer::PromoteIntRes_BUILD_VECTOR(SDNode *N) {
  EVT OutVT = N->getValueType(0);
  EVT NOutVT = TLI.getTypeToTransformTo(*DAG.getContext(), OutVT);
  assert(NOutVT.isVector() && "This type must be promoted to a vector type");
  unsigned NumElems = N->getNumOperands();
  EVT NOutVTElem = NOutVT.getVectorElementType();

  SDLoc dl(N);

  SmallVector<SDValue, 8> Ops;
  Ops.reserve(NumElems);
  for (unsigned i = 0; i != NumElems; ++i) {
    SDValue Op;
    // BUILD_VECTOR integer operand types are allowed to be larger than the
    // result's element type. This may still be true after the promotion. For
    // example, we might be promoting (<v?i1> = BV <i32>, <i32>, ...) to
    // (v?i16 = BV <i32>, <i32>, ...), and we can't any_extend <i32> to <i16>.
    if (N->getOperand(i).getValueType().bitsLT(NOutVTElem))
      Op = DAG.getNode(ISD::ANY_EXTEND, dl, NOutVTElem, N->getOperand(i));
    else
      Op = N->getOperand(i);
    Ops.push_back(Op);
  }

  return DAG.getNode(ISD::BUILD_VECTOR, dl, NOutVT, Ops);
}

SDValue DAGTypeLegalizer::PromoteIntRes_SCALAR_TO_VECTOR(SDNode *N) {

  SDLoc dl(N);

  assert(!N->getOperand(0).getValueType().isVector() &&
         "Input must be a scalar");

  EVT OutVT = N->getValueType(0);
  EVT NOutVT = TLI.getTypeToTransformTo(*DAG.getContext(), OutVT);
  assert(NOutVT.isVector() && "This type must be promoted to a vector type");
  EVT NOutVTElem = NOutVT.getVectorElementType();

  SDValue Op = DAG.getNode(ISD::ANY_EXTEND, dl, NOutVTElem, N->getOperand(0));

  return DAG.getNode(ISD::SCALAR_TO_VECTOR, dl, NOutVT, Op);
}

SDValue DAGTypeLegalizer::PromoteIntRes_CONCAT_VECTORS(SDNode *N) {
  SDLoc dl(N);

  EVT OutVT = N->getValueType(0);
  EVT NOutVT = TLI.getTypeToTransformTo(*DAG.getContext(), OutVT);
  assert(NOutVT.isVector() && "This type must be promoted to a vector type");

  EVT InElemTy = OutVT.getVectorElementType();
  EVT OutElemTy = NOutVT.getVectorElementType();

  unsigned NumElem = N->getOperand(0).getValueType().getVectorNumElements();
  unsigned NumOutElem = NOutVT.getVectorNumElements();
  unsigned NumOperands = N->getNumOperands();
  assert(NumElem * NumOperands == NumOutElem &&
         "Unexpected number of elements");

  // Take the elements from the first vector.
  SmallVector<SDValue, 8> Ops(NumOutElem);
  for (unsigned i = 0; i < NumOperands; ++i) {
    SDValue Op = N->getOperand(i);
    for (unsigned j = 0; j < NumElem; ++j) {
      SDValue Ext = DAG.getNode(
          ISD::EXTRACT_VECTOR_ELT, dl, InElemTy, Op,
          DAG.getConstant(j, dl, TLI.getVectorIdxTy(DAG.getDataLayout())));
      Ops[i * NumElem + j] = DAG.getNode(ISD::ANY_EXTEND, dl, OutElemTy, Ext);
    }
  }

  return DAG.getNode(ISD::BUILD_VECTOR, dl, NOutVT, Ops);
}

SDValue DAGTypeLegalizer::PromoteIntRes_INSERT_VECTOR_ELT(SDNode *N) {
  EVT OutVT = N->getValueType(0);
  EVT NOutVT = TLI.getTypeToTransformTo(*DAG.getContext(), OutVT);
  assert(NOutVT.isVector() && "This type must be promoted to a vector type");

  EVT NOutVTElem = NOutVT.getVectorElementType();

  SDLoc dl(N);
  SDValue V0 = GetPromotedInteger(N->getOperand(0));

  SDValue ConvElem = DAG.getNode(ISD::ANY_EXTEND, dl,
    NOutVTElem, N->getOperand(1));
  return DAG.getNode(ISD::INSERT_VECTOR_ELT, dl, NOutVT,
    V0, ConvElem, N->getOperand(2));
}

SDValue DAGTypeLegalizer::PromoteIntOp_EXTRACT_VECTOR_ELT(SDNode *N) {
  SDLoc dl(N);
  SDValue V0 = GetPromotedInteger(N->getOperand(0));
  SDValue V1 = DAG.getZExtOrTrunc(N->getOperand(1), dl,
                                  TLI.getVectorIdxTy(DAG.getDataLayout()));
  SDValue Ext = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, dl,
    V0->getValueType(0).getScalarType(), V0, V1);

  // EXTRACT_VECTOR_ELT can return types which are wider than the incoming
  // element types. If this is the case then we need to expand the outgoing
  // value and not truncate it.
  return DAG.getAnyExtOrTrunc(Ext, dl, N->getValueType(0));
}

SDValue DAGTypeLegalizer::PromoteIntOp_EXTRACT_SUBVECTOR(SDNode *N) {
  SDLoc dl(N);
  SDValue V0 = GetPromotedInteger(N->getOperand(0));
  MVT InVT = V0.getValueType().getSimpleVT();
  MVT OutVT = MVT::getVectorVT(InVT.getVectorElementType(),
                               N->getValueType(0).getVectorNumElements());
  SDValue Ext = DAG.getNode(ISD::EXTRACT_SUBVECTOR, dl, OutVT, V0, N->getOperand(1));
  return DAG.getNode(ISD::TRUNCATE, dl, N->getValueType(0), Ext);
}

SDValue DAGTypeLegalizer::PromoteIntOp_CONCAT_VECTORS(SDNode *N) {
  SDLoc dl(N);
  unsigned NumElems = N->getNumOperands();

  EVT RetSclrTy = N->getValueType(0).getVectorElementType();

  SmallVector<SDValue, 8> NewOps;
  NewOps.reserve(NumElems);

  // For each incoming vector
  for (unsigned VecIdx = 0; VecIdx != NumElems; ++VecIdx) {
    SDValue Incoming = GetPromotedInteger(N->getOperand(VecIdx));
    EVT SclrTy = Incoming->getValueType(0).getVectorElementType();
    unsigned NumElem = Incoming->getValueType(0).getVectorNumElements();

    for (unsigned i=0; i<NumElem; ++i) {
      // Extract element from incoming vector
      SDValue Ex = DAG.getNode(
          ISD::EXTRACT_VECTOR_ELT, dl, SclrTy, Incoming,
          DAG.getConstant(i, dl, TLI.getVectorIdxTy(DAG.getDataLayout())));
      SDValue Tr = DAG.getNode(ISD::TRUNCATE, dl, RetSclrTy, Ex);
      NewOps.push_back(Tr);
    }
  }

  return DAG.getNode(ISD::BUILD_VECTOR, dl,  N->getValueType(0), NewOps);
}
