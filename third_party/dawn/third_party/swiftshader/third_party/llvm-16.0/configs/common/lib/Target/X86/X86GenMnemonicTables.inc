/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* X86 Mnemonic tables                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

namespace llvm {
namespace X86 {

#ifdef GET_X86_MNEMONIC_TABLES_H
#undef GET_X86_MNEMONIC_TABLES_H

bool isPCMPISTRM(unsigned Opcode);
bool isVALIGND(unsigned Opcode);
bool isVFMULCPH(unsigned Opcode);
bool isVPDPBSSD(unsigned Opcode);
bool isVFIXUPIMMPS(unsigned Opcode);
bool isVPMOVQ2M(unsigned Opcode);
bool isLDTILECFG(unsigned Opcode);
bool isFADD(unsigned Opcode);
bool isVALIGNQ(unsigned Opcode);
bool isAESENC128KL(unsigned Opcode);
bool isVPMAXUB(unsigned Opcode);
bool isFMULP(unsigned Opcode);
bool isVPMAXUD(unsigned Opcode);
bool isCMPCC(unsigned Opcode);
bool isVSHUFI32X4(unsigned Opcode);
bool isLOOPNE(unsigned Opcode);
bool isVPMAXUQ(unsigned Opcode);
bool isVCOMPRESSPD(unsigned Opcode);
bool isVPMAXUW(unsigned Opcode);
bool isVPMOVB2M(unsigned Opcode);
bool isVANDPD(unsigned Opcode);
bool isVCOMPRESSPS(unsigned Opcode);
bool isVPDPBSUD(unsigned Opcode);
bool isPFRCPIT1(unsigned Opcode);
bool isPFRCPIT2(unsigned Opcode);
bool isWRPKRU(unsigned Opcode);
bool isVANDPS(unsigned Opcode);
bool isWRUSSD(unsigned Opcode);
bool isVMPTRLD(unsigned Opcode);
bool isWRUSSQ(unsigned Opcode);
bool isAESDECLAST(unsigned Opcode);
bool isSYSCALL(unsigned Opcode);
bool isVFIXUPIMMSD(unsigned Opcode);
bool isVPRORD(unsigned Opcode);
bool isTEST(unsigned Opcode);
bool isSHA1MSG1(unsigned Opcode);
bool isSHA1MSG2(unsigned Opcode);
bool isVFMULCSH(unsigned Opcode);
bool isMOVNTDQA(unsigned Opcode);
bool isVFIXUPIMMSS(unsigned Opcode);
bool isADOX(unsigned Opcode);
bool isVPRORQ(unsigned Opcode);
bool isVSCATTERPF1DPD(unsigned Opcode);
bool isVPSRLDQ(unsigned Opcode);
bool isVPMOVUSWB(unsigned Opcode);
bool isVSCATTERPF1DPS(unsigned Opcode);
bool isFICOMP(unsigned Opcode);
bool isFBSTP(unsigned Opcode);
bool isVPSHUFLW(unsigned Opcode);
bool isVPSCATTERDD(unsigned Opcode);
bool isFNINIT(unsigned Opcode);
bool isMOVNTPD(unsigned Opcode);
bool isUIRET(unsigned Opcode);
bool isPINSRB(unsigned Opcode);
bool isPINSRD(unsigned Opcode);
bool isSHRD(unsigned Opcode);
bool isVPSCATTERDQ(unsigned Opcode);
bool isMOVNTPS(unsigned Opcode);
bool isVGETEXPPD(unsigned Opcode);
bool isVRANGEPD(unsigned Opcode);
bool isPFRCP(unsigned Opcode);
bool isVGETEXPPH(unsigned Opcode);
bool isPINSRQ(unsigned Opcode);
bool isVPROTB(unsigned Opcode);
bool isVPROTD(unsigned Opcode);
bool isSEAMCALL(unsigned Opcode);
bool isPINSRW(unsigned Opcode);
bool isSHRX(unsigned Opcode);
bool isVGETEXPPS(unsigned Opcode);
bool isVRANGEPS(unsigned Opcode);
bool isPABSB(unsigned Opcode);
bool isPABSD(unsigned Opcode);
bool isVPROTQ(unsigned Opcode);
bool isVPROTW(unsigned Opcode);
bool isVCVTTPS2UDQ(unsigned Opcode);
bool isFXRSTOR(unsigned Opcode);
bool isVMOVDQU16(unsigned Opcode);
bool isPABSW(unsigned Opcode);
bool isCVTDQ2PD(unsigned Opcode);
bool isCVTDQ2PS(unsigned Opcode);
bool isXOR(unsigned Opcode);
bool isINC(unsigned Opcode);
bool isVMCALL(unsigned Opcode);
bool isPACKSSDW(unsigned Opcode);
bool isPSUBUSB(unsigned Opcode);
bool isINT(unsigned Opcode);
bool isMOVNTSD(unsigned Opcode);
bool isVPMOVZXBD(unsigned Opcode);
bool isVCVTUDQ2PD(unsigned Opcode);
bool isVCVTUDQ2PH(unsigned Opcode);
bool isVMOVDQU32(unsigned Opcode);
bool isPSUBUSW(unsigned Opcode);
bool isMOVNTSS(unsigned Opcode);
bool isVGETEXPSD(unsigned Opcode);
bool isVPMOVZXBQ(unsigned Opcode);
bool isVGETEXPSH(unsigned Opcode);
bool isVCVTNEEBF162PS(unsigned Opcode);
bool isVRANGESD(unsigned Opcode);
bool isVCVTUDQ2PS(unsigned Opcode);
bool isVPMOVZXBW(unsigned Opcode);
bool isVGETEXPSS(unsigned Opcode);
bool isSHUFPD(unsigned Opcode);
bool isVCVTTSS2SI(unsigned Opcode);
bool isVINSERTF128(unsigned Opcode);
bool isKANDNB(unsigned Opcode);
bool isPADDSB(unsigned Opcode);
bool isKANDND(unsigned Opcode);
bool isVPCMPISTRI(unsigned Opcode);
bool isRDPID(unsigned Opcode);
bool isVPCMPISTRM(unsigned Opcode);
bool isVRANGESS(unsigned Opcode);
bool isSHUFPS(unsigned Opcode);
bool isVPDPBSSDS(unsigned Opcode);
bool isKANDNQ(unsigned Opcode);
bool isKANDNW(unsigned Opcode);
bool isPADDSW(unsigned Opcode);
bool isVCVTTPD2UDQ(unsigned Opcode);
bool isVANDNPD(unsigned Opcode);
bool isXEND(unsigned Opcode);
bool isVUNPCKHPD(unsigned Opcode);
bool isPSRAD(unsigned Opcode);
bool isVANDNPS(unsigned Opcode);
bool isVPMOVZXDQ(unsigned Opcode);
bool isVMPTRST(unsigned Opcode);
bool isVUNPCKHPS(unsigned Opcode);
bool isPSRAW(unsigned Opcode);
bool isVPLZCNTD(unsigned Opcode);
bool isWRMSRLIST(unsigned Opcode);
bool isVPADDUSB(unsigned Opcode);
bool isVMOVDQU64(unsigned Opcode);
bool isSUBPD(unsigned Opcode);
bool isMOVDDUP(unsigned Opcode);
bool isVPLZCNTQ(unsigned Opcode);
bool isSTAC(unsigned Opcode);
bool isSUBPS(unsigned Opcode);
bool isVPADDUSW(unsigned Opcode);
bool isFCMOVNBE(unsigned Opcode);
bool isSHA1RNDS4(unsigned Opcode);
bool isPAUSE(unsigned Opcode);
bool isSAHF(unsigned Opcode);
bool isVREDUCEPD(unsigned Opcode);
bool isVREDUCEPH(unsigned Opcode);
bool isFXAM(unsigned Opcode);
bool isLGDTD(unsigned Opcode);
bool isPMULHRW(unsigned Opcode);
bool isVREDUCEPS(unsigned Opcode);
bool isRDPMC(unsigned Opcode);
bool isVGATHERPF1QPD(unsigned Opcode);
bool isLGDTW(unsigned Opcode);
bool isVAESKEYGENASSIST(unsigned Opcode);
bool isRDFSBASE(unsigned Opcode);
bool isVGATHERPF1QPS(unsigned Opcode);
bool isSUBSD(unsigned Opcode);
bool isVCVTNEOBF162PS(unsigned Opcode);
bool isAESENCWIDE128KL(unsigned Opcode);
bool isFXCH(unsigned Opcode);
bool isSUBSS(unsigned Opcode);
bool isPANDN(unsigned Opcode);
bool isVPERMT2B(unsigned Opcode);
bool isLJMP(unsigned Opcode);
bool isCMPPD(unsigned Opcode);
bool isVPERMT2D(unsigned Opcode);
bool isVPMADD52HUQ(unsigned Opcode);
bool isVPERMT2Q(unsigned Opcode);
bool isCMPPS(unsigned Opcode);
bool isVCVTPH2W(unsigned Opcode);
bool isVREDUCESD(unsigned Opcode);
bool isVPERMT2W(unsigned Opcode);
bool isWBNOINVD(unsigned Opcode);
bool isVREDUCESH(unsigned Opcode);
bool isTILEZERO(unsigned Opcode);
bool isPMULHUW(unsigned Opcode);
bool isVREDUCESS(unsigned Opcode);
bool isVCVTUW2PH(unsigned Opcode);
bool isVPBLENDMB(unsigned Opcode);
bool isVPBLENDMD(unsigned Opcode);
bool isVFMSUB132PD(unsigned Opcode);
bool isVFMSUB132PH(unsigned Opcode);
bool isMWAIT(unsigned Opcode);
bool isSALC(unsigned Opcode);
bool isPMADDUBSW(unsigned Opcode);
bool isVFCMULCPH(unsigned Opcode);
bool isVPBLENDMQ(unsigned Opcode);
bool isRORX(unsigned Opcode);
bool isVFMSUB132PS(unsigned Opcode);
bool isVPBLENDMW(unsigned Opcode);
bool isMOV(unsigned Opcode);
bool isFXSAVE64(unsigned Opcode);
bool isRMPADJUST(unsigned Opcode);
bool isAADD(unsigned Opcode);
bool isVLDDQU(unsigned Opcode);
bool isVPSCATTERQD(unsigned Opcode);
bool isVPHADDUBD(unsigned Opcode);
bool isCMPSB(unsigned Opcode);
bool isCMPSD(unsigned Opcode);
bool isSTGI(unsigned Opcode);
bool isVUNPCKLPD(unsigned Opcode);
bool isVPSCATTERQQ(unsigned Opcode);
bool isFADDP(unsigned Opcode);
bool isVPHADDUBQ(unsigned Opcode);
bool isCMPSQ(unsigned Opcode);
bool isVPHADDUBW(unsigned Opcode);
bool isCMPSS(unsigned Opcode);
bool isVUNPCKLPS(unsigned Opcode);
bool isLCALL(unsigned Opcode);
bool isPSHUFB(unsigned Opcode);
bool isCMPSW(unsigned Opcode);
bool isPSHUFD(unsigned Opcode);
bool isRDPRU(unsigned Opcode);
bool isFRNDINT(unsigned Opcode);
bool isVPACKUSWB(unsigned Opcode);
bool isDIVPD(unsigned Opcode);
bool isVAESDEC(unsigned Opcode);
bool isPSHUFW(unsigned Opcode);
bool isVPDPBUUDS(unsigned Opcode);
bool isKMOVB(unsigned Opcode);
bool isVFMSUB132SD(unsigned Opcode);
bool isKMOVD(unsigned Opcode);
bool isVCVTTPS2UQQ(unsigned Opcode);
bool isVFMSUB132SH(unsigned Opcode);
bool isDIVPS(unsigned Opcode);
bool isVFCMULCSH(unsigned Opcode);
bool isFICOM(unsigned Opcode);
bool isKMOVQ(unsigned Opcode);
bool isVFMSUB132SS(unsigned Opcode);
bool isENCODEKEY128(unsigned Opcode);
bool isKMOVW(unsigned Opcode);
bool isPREFETCHT0(unsigned Opcode);
bool isPREFETCHT1(unsigned Opcode);
bool isPREFETCHT2(unsigned Opcode);
bool isSWAPGS(unsigned Opcode);
bool isVPTESTMD(unsigned Opcode);
bool isVPTESTMB(unsigned Opcode);
bool isVPHADDUDQ(unsigned Opcode);
bool isVPTESTMQ(unsigned Opcode);
bool isXRSTORS(unsigned Opcode);
bool isVPMULDQ(unsigned Opcode);
bool isUD1(unsigned Opcode);
bool isUD2(unsigned Opcode);
bool isVPTESTMW(unsigned Opcode);
bool isSEAMOPS(unsigned Opcode);
bool isMWAITX(unsigned Opcode);
bool isVFMADD132PD(unsigned Opcode);
bool isWRMSRNS(unsigned Opcode);
bool isVMOVNTDQ(unsigned Opcode);
bool isVFMADD132PH(unsigned Opcode);
bool isPSRLD(unsigned Opcode);
bool isVBROADCASTI64X2(unsigned Opcode);
bool isVBROADCASTI64X4(unsigned Opcode);
bool isVFMADD132PS(unsigned Opcode);
bool isIDIV(unsigned Opcode);
bool isPREFETCHWT1(unsigned Opcode);
bool isVPSRLVD(unsigned Opcode);
bool isPSRLQ(unsigned Opcode);
bool isPSRLW(unsigned Opcode);
bool isDIVSD(unsigned Opcode);
bool isVPMOVDB(unsigned Opcode);
bool isVPSRLVQ(unsigned Opcode);
bool isVPSRLVW(unsigned Opcode);
bool isDIVSS(unsigned Opcode);
bool isMUL(unsigned Opcode);
bool isVPMOVDW(unsigned Opcode);
bool isVCVTTPD2UQQ(unsigned Opcode);
bool isFSINCOS(unsigned Opcode);
bool isVPMADD52LUQ(unsigned Opcode);
bool isLWPINS(unsigned Opcode);
bool isMOVDIR64B(unsigned Opcode);
bool isLOOPE(unsigned Opcode);
bool isPUSH(unsigned Opcode);
bool isPSADBW(unsigned Opcode);
bool isFTST(unsigned Opcode);
bool isSETSSBSY(unsigned Opcode);
bool isSARX(unsigned Opcode);
bool isVADDSUBPD(unsigned Opcode);
bool isVADDSUBPS(unsigned Opcode);
bool isVMINPD(unsigned Opcode);
bool isCLZERO(unsigned Opcode);
bool isVMINPH(unsigned Opcode);
bool isXCRYPTCBC(unsigned Opcode);
bool isFXTRACT(unsigned Opcode);
bool isVMINPS(unsigned Opcode);
bool isVFMADD132SD(unsigned Opcode);
bool isVFMADD132SH(unsigned Opcode);
bool isVPERMI2B(unsigned Opcode);
bool isVPERMI2D(unsigned Opcode);
bool isVFMADD132SS(unsigned Opcode);
bool isVRSQRT28PD(unsigned Opcode);
bool isVPMULHW(unsigned Opcode);
bool isSIDT(unsigned Opcode);
bool isTDCALL(unsigned Opcode);
bool isVPERMI2Q(unsigned Opcode);
bool isVPERMI2W(unsigned Opcode);
bool isVPERM2F128(unsigned Opcode);
bool isVMLAUNCH(unsigned Opcode);
bool isFILD(unsigned Opcode);
bool isVRSQRT28PS(unsigned Opcode);
bool isVPBLENDVB(unsigned Opcode);
bool isVPMADDUBSW(unsigned Opcode);
bool isVSTMXCSR(unsigned Opcode);
bool isVCVTTPH2UDQ(unsigned Opcode);
bool isXSHA256(unsigned Opcode);
bool isWAIT(unsigned Opcode);
bool isPACKSSWB(unsigned Opcode);
bool isPMULHRSW(unsigned Opcode);
bool isMASKMOVQ(unsigned Opcode);
bool isVMINSD(unsigned Opcode);
bool isVMINSH(unsigned Opcode);
bool isRDTSC(unsigned Opcode);
bool isVMINSS(unsigned Opcode);
bool isVRSQRT28SD(unsigned Opcode);
bool isAAND(unsigned Opcode);
bool isXCRYPTCFB(unsigned Opcode);
bool isVSCALEFPD(unsigned Opcode);
bool isVPBROADCASTB(unsigned Opcode);
bool isVSCALEFPH(unsigned Opcode);
bool isVPBROADCASTD(unsigned Opcode);
bool isVRSQRT28SS(unsigned Opcode);
bool isVPMULLD(unsigned Opcode);
bool isVSCALEFPS(unsigned Opcode);
bool isMOVQ2DQ(unsigned Opcode);
bool isVPBROADCASTQ(unsigned Opcode);
bool isPALIGNR(unsigned Opcode);
bool isPUSHAL(unsigned Opcode);
bool isVPBROADCASTW(unsigned Opcode);
bool isVPMULLQ(unsigned Opcode);
bool isFINCSTP(unsigned Opcode);
bool isPUSHAW(unsigned Opcode);
bool isVPMULLW(unsigned Opcode);
bool isPFPNACC(unsigned Opcode);
bool isTESTUI(unsigned Opcode);
bool isVPMOVZXWD(unsigned Opcode);
bool isVPDPWSSDS(unsigned Opcode);
bool isINVLPG(unsigned Opcode);
bool isJCC(unsigned Opcode);
bool isVPMOVZXWQ(unsigned Opcode);
bool isBSF(unsigned Opcode);
bool isROUNDPD(unsigned Opcode);
bool isSAVEPREVSSP(unsigned Opcode);
bool isVSCATTERDPD(unsigned Opcode);
bool isBSR(unsigned Opcode);
bool isROUNDPS(unsigned Opcode);
bool isCVTPI2PD(unsigned Opcode);
bool isKTESTB(unsigned Opcode);
bool isKTESTD(unsigned Opcode);
bool isVSCATTERDPS(unsigned Opcode);
bool isFLD(unsigned Opcode);
bool isBTC(unsigned Opcode);
bool isVBCSTNEBF162PS(unsigned Opcode);
bool isCVTPI2PS(unsigned Opcode);
bool isKTESTQ(unsigned Opcode);
bool isVSCALEFSD(unsigned Opcode);
bool isVSCALEFSH(unsigned Opcode);
bool isKTESTW(unsigned Opcode);
bool isXSTORE(unsigned Opcode);
bool isVFMADDSUB132PD(unsigned Opcode);
bool isVPSRAVD(unsigned Opcode);
bool isBTR(unsigned Opcode);
bool isBTS(unsigned Opcode);
bool isVFMADDSUB132PH(unsigned Opcode);
bool isLGDT(unsigned Opcode);
bool isVSCALEFSS(unsigned Opcode);
bool isVEXTRACTF128(unsigned Opcode);
bool isPMOVSXBD(unsigned Opcode);
bool isKXNORB(unsigned Opcode);
bool isVFMADDSUB132PS(unsigned Opcode);
bool isKXNORD(unsigned Opcode);
bool isXRSTOR(unsigned Opcode);
bool isVPSRAVQ(unsigned Opcode);
bool isVPSRAVW(unsigned Opcode);
bool isVPDPWSSD(unsigned Opcode);
bool isSQRTPD(unsigned Opcode);
bool isVDPPD(unsigned Opcode);
bool isPMOVSXBQ(unsigned Opcode);
bool isPMADDWD(unsigned Opcode);
bool isKXNORQ(unsigned Opcode);
bool isSTUI(unsigned Opcode);
bool isINSERTQ(unsigned Opcode);
bool isPMOVSXBW(unsigned Opcode);
bool isVCVTSI2SD(unsigned Opcode);
bool isKXNORW(unsigned Opcode);
bool isVSCATTERPF0DPD(unsigned Opcode);
bool isVCVTSI2SH(unsigned Opcode);
bool isSQRTPS(unsigned Opcode);
bool isVDPPS(unsigned Opcode);
bool isPSUBSB(unsigned Opcode);
bool isVPSHRDVD(unsigned Opcode);
bool isFIST(unsigned Opcode);
bool isVCVTSI2SS(unsigned Opcode);
bool isVSCATTERPF0DPS(unsigned Opcode);
bool isVMOVNTPD(unsigned Opcode);
bool isROUNDSD(unsigned Opcode);
bool isVPSHRDVQ(unsigned Opcode);
bool isPSIGNB(unsigned Opcode);
bool isJECXZ(unsigned Opcode);
bool isPSIGND(unsigned Opcode);
bool isPSUBSW(unsigned Opcode);
bool isMOVDQ2Q(unsigned Opcode);
bool isPUSHFD(unsigned Opcode);
bool isVMOVNTPS(unsigned Opcode);
bool isROUNDSS(unsigned Opcode);
bool isVAESDECLAST(unsigned Opcode);
bool isPAVGB(unsigned Opcode);
bool isVPSUBB(unsigned Opcode);
bool isVPSHRDVW(unsigned Opcode);
bool isVPSUBD(unsigned Opcode);
bool isPUSHFQ(unsigned Opcode);
bool isPSIGNW(unsigned Opcode);
bool isVBROADCASTSD(unsigned Opcode);
bool isVPSUBQ(unsigned Opcode);
bool isPMOVSXDQ(unsigned Opcode);
bool isPAVGW(unsigned Opcode);
bool isVFMSUBADD132PD(unsigned Opcode);
bool isVPSUBW(unsigned Opcode);
bool isVCVTPS2PHX(unsigned Opcode);
bool isMOVHLPS(unsigned Opcode);
bool isVFMSUBADD132PH(unsigned Opcode);
bool isVSHUFF64X2(unsigned Opcode);
bool isVBROADCASTSS(unsigned Opcode);
bool isVFMSUBADD132PS(unsigned Opcode);
bool isMAXPD(unsigned Opcode);
bool isSQRTSD(unsigned Opcode);
bool isCVTPD2DQ(unsigned Opcode);
bool isMAXPS(unsigned Opcode);
bool isVPMOVQB(unsigned Opcode);
bool isINVPCID(unsigned Opcode);
bool isVCVTPS2DQ(unsigned Opcode);
bool isSQRTSS(unsigned Opcode);
bool isVADDPD(unsigned Opcode);
bool isLODSB(unsigned Opcode);
bool isLODSD(unsigned Opcode);
bool isV4FNMADDPS(unsigned Opcode);
bool isIRETD(unsigned Opcode);
bool isVADDPH(unsigned Opcode);
bool isVHADDPD(unsigned Opcode);
bool isVPMOVQD(unsigned Opcode);
bool isVPSUBUSB(unsigned Opcode);
bool isVPMOVQW(unsigned Opcode);
bool isVADDPS(unsigned Opcode);
bool isLODSQ(unsigned Opcode);
bool isFIDIVR(unsigned Opcode);
bool isIRETQ(unsigned Opcode);
bool isLODSW(unsigned Opcode);
bool isVHADDPS(unsigned Opcode);
bool isCVTTPS2DQ(unsigned Opcode);
bool isVPORD(unsigned Opcode);
bool isVP2INTERSECTD(unsigned Opcode);
bool isVPSUBUSW(unsigned Opcode);
bool isVPERMILPD(unsigned Opcode);
bool isVPSHLDD(unsigned Opcode);
bool isMOVD(unsigned Opcode);
bool isVPORQ(unsigned Opcode);
bool isVP2INTERSECTQ(unsigned Opcode);
bool isCRC32(unsigned Opcode);
bool isVPERMILPS(unsigned Opcode);
bool isMOVQ(unsigned Opcode);
bool isVPSHLDQ(unsigned Opcode);
bool isVPMASKMOVD(unsigned Opcode);
bool isVPSHLDW(unsigned Opcode);
bool isSLWPCB(unsigned Opcode);
bool isVORPD(unsigned Opcode);
bool isPCMPGTB(unsigned Opcode);
bool isPCMPGTD(unsigned Opcode);
bool isVPMASKMOVQ(unsigned Opcode);
bool isPFRSQIT1(unsigned Opcode);
bool isVORPS(unsigned Opcode);
bool isPCMPGTQ(unsigned Opcode);
bool isBLCIC(unsigned Opcode);
bool isMAXSD(unsigned Opcode);
bool isPCMPGTW(unsigned Opcode);
bool isVBCSTNESH2PS(unsigned Opcode);
bool isMOVLHPS(unsigned Opcode);
bool isXSAVEC(unsigned Opcode);
bool isMAXSS(unsigned Opcode);
bool isFST(unsigned Opcode);
bool isVCVTSS2USI(unsigned Opcode);
bool isVADDSD(unsigned Opcode);
bool isVRNDSCALEPD(unsigned Opcode);
bool isVCVTTPH2UQQ(unsigned Opcode);
bool isV4FNMADDSS(unsigned Opcode);
bool isVADDSH(unsigned Opcode);
bool isVRNDSCALEPH(unsigned Opcode);
bool isXSAVES(unsigned Opcode);
bool isXRESLDTRK(unsigned Opcode);
bool isVADDSS(unsigned Opcode);
bool isVRNDSCALEPS(unsigned Opcode);
bool isVPHADDUWD(unsigned Opcode);
bool isRDSSPD(unsigned Opcode);
bool isVPHADDUWQ(unsigned Opcode);
bool isVMOVDDUP(unsigned Opcode);
bool isINSERTPS(unsigned Opcode);
bool isNEG(unsigned Opcode);
bool isMOVUPD(unsigned Opcode);
bool isRDSSPQ(unsigned Opcode);
bool isVFRCZPD(unsigned Opcode);
bool isPHMINPOSUW(unsigned Opcode);
bool isVBROADCASTI32X2(unsigned Opcode);
bool isVBROADCASTI32X4(unsigned Opcode);
bool isJMP(unsigned Opcode);
bool isVBROADCASTI32X8(unsigned Opcode);
bool isMOVUPS(unsigned Opcode);
bool isVPANDND(unsigned Opcode);
bool isVCVTNE2PS2BF16(unsigned Opcode);
bool isVPROLVD(unsigned Opcode);
bool isVFRCZPS(unsigned Opcode);
bool isJRCXZ(unsigned Opcode);
bool isFNSTCW(unsigned Opcode);
bool isFFREEP(unsigned Opcode);
bool isVPANDNQ(unsigned Opcode);
bool isVPROLVQ(unsigned Opcode);
bool isAESDECWIDE128KL(unsigned Opcode);
bool isSTTILECFG(unsigned Opcode);
bool isVPMOVWB(unsigned Opcode);
bool isVRNDSCALESD(unsigned Opcode);
bool isVGATHERPF0QPD(unsigned Opcode);
bool isVPHSUBD(unsigned Opcode);
bool isVCVTSD2USI(unsigned Opcode);
bool isWRGSBASE(unsigned Opcode);
bool isVRNDSCALESH(unsigned Opcode);
bool isAESENC256KL(unsigned Opcode);
bool isVRNDSCALESS(unsigned Opcode);
bool isVGATHERPF0QPS(unsigned Opcode);
bool isVPHSUBW(unsigned Opcode);
bool isSHA1NEXTE(unsigned Opcode);
bool isXCRYPTCTR(unsigned Opcode);
bool isFMUL(unsigned Opcode);
bool isFBLD(unsigned Opcode);
bool isVMXON(unsigned Opcode);
bool isPUNPCKHQDQ(unsigned Opcode);
bool isVFRCZSD(unsigned Opcode);
bool isVFRCZSS(unsigned Opcode);
bool isRMPUPDATE(unsigned Opcode);
bool isVFNMADDPD(unsigned Opcode);
bool isVCOMISD(unsigned Opcode);
bool isVCOMISH(unsigned Opcode);
bool isAXOR(unsigned Opcode);
bool isVFNMADDPS(unsigned Opcode);
bool isVCOMISS(unsigned Opcode);
bool isPHADDD(unsigned Opcode);
bool isVPMULHUW(unsigned Opcode);
bool isVFNMSUB132PD(unsigned Opcode);
bool isPMOVMSKB(unsigned Opcode);
bool isVFNMSUB132PH(unsigned Opcode);
bool isPHADDW(unsigned Opcode);
bool isVFNMSUB132PS(unsigned Opcode);
bool isVMOVDQA(unsigned Opcode);
bool isSENDUIPI(unsigned Opcode);
bool isVCVTTPS2DQ(unsigned Opcode);
bool isRCPPS(unsigned Opcode);
bool isRDMSR(unsigned Opcode);
bool isVCVTQQ2PD(unsigned Opcode);
bool isVRSQRT14PD(unsigned Opcode);
bool isXORPD(unsigned Opcode);
bool isVCVTQQ2PH(unsigned Opcode);
bool isVMOVDQU(unsigned Opcode);
bool isBLENDPD(unsigned Opcode);
bool isVCVTQQ2PS(unsigned Opcode);
bool isVRSQRT14PS(unsigned Opcode);
bool isXORPS(unsigned Opcode);
bool isRCL(unsigned Opcode);
bool isRCR(unsigned Opcode);
bool isBLENDPS(unsigned Opcode);
bool isVPEXPANDB(unsigned Opcode);
bool isVEXPANDPD(unsigned Opcode);
bool isVPEXPANDD(unsigned Opcode);
bool isBLSMSK(unsigned Opcode);
bool isXSUSLDTRK(unsigned Opcode);
bool isGF2P8MULB(unsigned Opcode);
bool isSIDTD(unsigned Opcode);
bool isVEXPANDPS(unsigned Opcode);
bool isVFNMADDSD(unsigned Opcode);
bool isCBW(unsigned Opcode);
bool isVPEXPANDW(unsigned Opcode);
bool isVPEXPANDQ(unsigned Opcode);
bool isFXRSTOR64(unsigned Opcode);
bool isVFNMADDSS(unsigned Opcode);
bool isSIDTW(unsigned Opcode);
bool isCVTPD2PI(unsigned Opcode);
bool isVCVTPS2PD(unsigned Opcode);
bool isVFNMSUB132SD(unsigned Opcode);
bool isVCVTPS2PH(unsigned Opcode);
bool isFIDIV(unsigned Opcode);
bool isVFNMSUB132SH(unsigned Opcode);
bool isCVTPD2PS(unsigned Opcode);
bool isINVEPT(unsigned Opcode);
bool isVPERMI2PD(unsigned Opcode);
bool isVFMSUB213PD(unsigned Opcode);
bool isVFNMSUB132SS(unsigned Opcode);
bool isINVVPID(unsigned Opcode);
bool isVFMSUB213PH(unsigned Opcode);
bool isVPCOMB(unsigned Opcode);
bool isVPERMI2PS(unsigned Opcode);
bool isVPCOMD(unsigned Opcode);
bool isSMSW(unsigned Opcode);
bool isRCPSS(unsigned Opcode);
bool isRET(unsigned Opcode);
bool isVFMSUB213PS(unsigned Opcode);
bool isVRSQRT14SD(unsigned Opcode);
bool isCVTTPS2PI(unsigned Opcode);
bool isVMCLEAR(unsigned Opcode);
bool isVPCOMQ(unsigned Opcode);
bool isCDQ(unsigned Opcode);
bool isFLDL2E(unsigned Opcode);
bool isVCVTPS2QQ(unsigned Opcode);
bool isVPCOMW(unsigned Opcode);
bool isVRSQRT14SS(unsigned Opcode);
bool isVP4DPWSSDS(unsigned Opcode);
bool isVPSLLDQ(unsigned Opcode);
bool isVCVTSD2SH(unsigned Opcode);
bool isFLDL2T(unsigned Opcode);
bool isVEXP2PD(unsigned Opcode);
bool isVFNMADD132PD(unsigned Opcode);
bool isVCVTSD2SI(unsigned Opcode);
bool isVPOPCNTB(unsigned Opcode);
bool isVFNMADD132PH(unsigned Opcode);
bool isVINSERTI128(unsigned Opcode);
bool isVPOPCNTD(unsigned Opcode);
bool isSETCC(unsigned Opcode);
bool isVCVTSD2SS(unsigned Opcode);
bool isVEXP2PS(unsigned Opcode);
bool isVFNMADD132PS(unsigned Opcode);
bool isMOVDIRI(unsigned Opcode);
bool isVPOPCNTQ(unsigned Opcode);
bool isVFMSUB231PD(unsigned Opcode);
bool isVFMSUB231PH(unsigned Opcode);
bool isFCMOVBE(unsigned Opcode);
bool isVPOPCNTW(unsigned Opcode);
bool isCVTTSD2SI(unsigned Opcode);
bool isVFMSUB231PS(unsigned Opcode);
bool isNOP(unsigned Opcode);
bool isNOT(unsigned Opcode);
bool isTPAUSE(unsigned Opcode);
bool isVCVTNEPS2BF16(unsigned Opcode);
bool isVFMSUB213SD(unsigned Opcode);
bool isVFMSUB213SH(unsigned Opcode);
bool isVFMADDCPH(unsigned Opcode);
bool isVEXTRACTI64X2(unsigned Opcode);
bool isVFMSUB213SS(unsigned Opcode);
bool isVEXTRACTI64X4(unsigned Opcode);
bool isXSAVEC64(unsigned Opcode);
bool isBLCFILL(unsigned Opcode);
bool isXBEGIN(unsigned Opcode);
bool isXCRYPTOFB(unsigned Opcode);
bool isFUCOMI(unsigned Opcode);
bool isVMOVHPD(unsigned Opcode);
bool isMASKMOVDQU(unsigned Opcode);
bool isFUCOMP(unsigned Opcode);
bool isVFNMADD132SD(unsigned Opcode);
bool isMFENCE(unsigned Opcode);
bool isVFNMADD132SH(unsigned Opcode);
bool isVMOVHPS(unsigned Opcode);
bool isVPBROADCASTMW2D(unsigned Opcode);
bool isVFMADD213PD(unsigned Opcode);
bool isVFNMADD132SS(unsigned Opcode);
bool isVFMADD213PH(unsigned Opcode);
bool isVFMSUB231SD(unsigned Opcode);
bool isVPMULTISHIFTQB(unsigned Opcode);
bool isFNSAVE(unsigned Opcode);
bool isVFMSUB231SH(unsigned Opcode);
bool isVSHUFF32X4(unsigned Opcode);
bool isPMOVSXWD(unsigned Opcode);
bool isVFMADD213PS(unsigned Opcode);
bool isVFMSUB231SS(unsigned Opcode);
bool isVPCOMPRESSB(unsigned Opcode);
bool isPMOVSXWQ(unsigned Opcode);
bool isVPCOMPRESSD(unsigned Opcode);
bool isVPABSB(unsigned Opcode);
bool isVMOVNTDQA(unsigned Opcode);
bool isVPABSD(unsigned Opcode);
bool isVPCOMPRESSQ(unsigned Opcode);
bool isVINSERTI64X2(unsigned Opcode);
bool isVPCOMPRESSW(unsigned Opcode);
bool isVINSERTI64X4(unsigned Opcode);
bool isVPABSQ(unsigned Opcode);
bool isVFMADDCSH(unsigned Opcode);
bool isVPABSW(unsigned Opcode);
bool isPVALIDATE(unsigned Opcode);
bool isVFMADD231PD(unsigned Opcode);
bool isVFMADD231PH(unsigned Opcode);
bool isPUNPCKLQDQ(unsigned Opcode);
bool isVCVTNEEPH2PS(unsigned Opcode);
bool isVFMADD231PS(unsigned Opcode);
bool isAESENCWIDE256KL(unsigned Opcode);
bool isVGF2P8MULB(unsigned Opcode);
bool isWRSSD(unsigned Opcode);
bool isMOVSHDUP(unsigned Opcode);
bool isTZMSK(unsigned Opcode);
bool isFPATAN(unsigned Opcode);
bool isFUCOM(unsigned Opcode);
bool isVFMADD213SD(unsigned Opcode);
bool isWRSSQ(unsigned Opcode);
bool isVFMADD213SH(unsigned Opcode);
bool isVPTESTNMB(unsigned Opcode);
bool isVPTESTNMD(unsigned Opcode);
bool isPMULUDQ(unsigned Opcode);
bool isHRESET(unsigned Opcode);
bool isPMAXSB(unsigned Opcode);
bool isPMAXSD(unsigned Opcode);
bool isVFMADD213SS(unsigned Opcode);
bool isCLC(unsigned Opcode);
bool isCLD(unsigned Opcode);
bool isENCODEKEY256(unsigned Opcode);
bool isVPTESTNMQ(unsigned Opcode);
bool isVGATHERQPD(unsigned Opcode);
bool isBOUND(unsigned Opcode);
bool isCLI(unsigned Opcode);
bool isVPTESTNMW(unsigned Opcode);
bool isPREFETCHIT0(unsigned Opcode);
bool isPREFETCHIT1(unsigned Opcode);
bool isPMAXSW(unsigned Opcode);
bool isPFSUB(unsigned Opcode);
bool isVCVTDQ2PH(unsigned Opcode);
bool isVPALIGNR(unsigned Opcode);
bool isVCVTDQ2PD(unsigned Opcode);
bool isFNSTSW(unsigned Opcode);
bool isFISTP(unsigned Opcode);
bool isVFMADDPD(unsigned Opcode);
bool isVGATHERQPS(unsigned Opcode);
bool isPBLENDW(unsigned Opcode);
bool isVCVTDQ2PS(unsigned Opcode);
bool isPREFETCH(unsigned Opcode);
bool isSKINIT(unsigned Opcode);
bool isCMC(unsigned Opcode);
bool isVFMADDPS(unsigned Opcode);
bool isVPACKSSDW(unsigned Opcode);
bool isFISUB(unsigned Opcode);
bool isPADDB(unsigned Opcode);
bool isPOPAL(unsigned Opcode);
bool isPADDD(unsigned Opcode);
bool isVFMADD231SD(unsigned Opcode);
bool isCMP(unsigned Opcode);
bool isKANDB(unsigned Opcode);
bool isKANDD(unsigned Opcode);
bool isVFMADD231SH(unsigned Opcode);
bool isVCVTSH2USI(unsigned Opcode);
bool isPCONFIG(unsigned Opcode);
bool isPOPAW(unsigned Opcode);
bool isPADDQ(unsigned Opcode);
bool isPMAXUB(unsigned Opcode);
bool isROL(unsigned Opcode);
bool isKANDQ(unsigned Opcode);
bool isPADDW(unsigned Opcode);
bool isVCVTTPS2QQ(unsigned Opcode);
bool isPMAXUD(unsigned Opcode);
bool isROR(unsigned Opcode);
bool isVFMADD231SS(unsigned Opcode);
bool isKANDW(unsigned Opcode);
bool isVPSRAD(unsigned Opcode);
bool isVROUNDPD(unsigned Opcode);
bool isMOVBE(unsigned Opcode);
bool isLOOP(unsigned Opcode);
bool isVCVTTSD2SI(unsigned Opcode);
bool isVPSRAQ(unsigned Opcode);
bool isVROUNDPS(unsigned Opcode);
bool isPMAXUW(unsigned Opcode);
bool isVPSRAW(unsigned Opcode);
bool isVPOR(unsigned Opcode);
bool isVPEXTRB(unsigned Opcode);
bool isVPEXTRD(unsigned Opcode);
bool isXGETBV(unsigned Opcode);
bool isVSUBPD(unsigned Opcode);
bool isENCLS(unsigned Opcode);
bool isENCLU(unsigned Opcode);
bool isENCLV(unsigned Opcode);
bool isVSUBPH(unsigned Opcode);
bool isVPEXTRQ(unsigned Opcode);
bool isVHSUBPD(unsigned Opcode);
bool isVPEXTRW(unsigned Opcode);
bool isVSUBPS(unsigned Opcode);
bool isVGF2P8AFFINEINVQB(unsigned Opcode);
bool isVMOVLPD(unsigned Opcode);
bool isVFMADDSD(unsigned Opcode);
bool isVHSUBPS(unsigned Opcode);
bool isPSRLDQ(unsigned Opcode);
bool isVMOVLPS(unsigned Opcode);
bool isVFMADDSS(unsigned Opcode);
bool isVRCP28PD(unsigned Opcode);
bool isFPREM(unsigned Opcode);
bool isVPMADDWD(unsigned Opcode);
bool isVCVTSH2SD(unsigned Opcode);
bool isSERIALIZE(unsigned Opcode);
bool isV4FMADDPS(unsigned Opcode);
bool isVRCP28PS(unsigned Opcode);
bool isVCVTSH2SI(unsigned Opcode);
bool isRETF(unsigned Opcode);
bool isVROUNDSD(unsigned Opcode);
bool isVCVTSH2SS(unsigned Opcode);
bool isVSCATTERPF1QPD(unsigned Opcode);
bool isVPCONFLICTD(unsigned Opcode);
bool isMOVNTI(unsigned Opcode);
bool isCQO(unsigned Opcode);
bool isVROUNDSS(unsigned Opcode);
bool isMOVNTQ(unsigned Opcode);
bool isVAESENC(unsigned Opcode);
bool isVSCATTERPF1QPS(unsigned Opcode);
bool isVPCONFLICTQ(unsigned Opcode);
bool isFCMOVNB(unsigned Opcode);
bool isLZCNT(unsigned Opcode);
bool isFCMOVNE(unsigned Opcode);
bool isRSM(unsigned Opcode);
bool isPOPCNT(unsigned Opcode);
bool isVSUBSD(unsigned Opcode);
bool isPOPFD(unsigned Opcode);
bool isVSUBSH(unsigned Opcode);
bool isVPANDD(unsigned Opcode);
bool isFCMOVNU(unsigned Opcode);
bool isVMOVHLPS(unsigned Opcode);
bool isPOPFQ(unsigned Opcode);
bool isVPANDN(unsigned Opcode);
bool isVFMADDSUB213PD(unsigned Opcode);
bool isVCMPPD(unsigned Opcode);
bool isVFMADDSUB213PH(unsigned Opcode);
bool isVSUBSS(unsigned Opcode);
bool isVPANDQ(unsigned Opcode);
bool isVCMPPH(unsigned Opcode);
bool isVP4DPWSSD(unsigned Opcode);
bool isENDBR32(unsigned Opcode);
bool isEMMS(unsigned Opcode);
bool isXCHG(unsigned Opcode);
bool isVFMADDSUB213PS(unsigned Opcode);
bool isTDPBUSD(unsigned Opcode);
bool isRDSEED(unsigned Opcode);
bool isVCMPPS(unsigned Opcode);
bool isVRCP28SD(unsigned Opcode);
bool isRDMSRLIST(unsigned Opcode);
bool isVRCP28SS(unsigned Opcode);
bool isV4FMADDSS(unsigned Opcode);
bool isAESKEYGENASSIST(unsigned Opcode);
bool isFUCOMPI(unsigned Opcode);
bool isTDPBF16PS(unsigned Opcode);
bool isFUCOMPP(unsigned Opcode);
bool isVFMADDSUB231PD(unsigned Opcode);
bool isVCVTPH2PSX(unsigned Opcode);
bool isVFMADDSUB231PH(unsigned Opcode);
bool isBLENDVPD(unsigned Opcode);
bool isPSWAPD(unsigned Opcode);
bool isVMSAVE(unsigned Opcode);
bool isVFMADDSUB231PS(unsigned Opcode);
bool isTDPBUUD(unsigned Opcode);
bool isVBLENDMPD(unsigned Opcode);
bool isPFACC(unsigned Opcode);
bool isBLENDVPS(unsigned Opcode);
bool isVPERM2I128(unsigned Opcode);
bool isVPCMPGTB(unsigned Opcode);
bool isLLWPCB(unsigned Opcode);
bool isVPCMPGTD(unsigned Opcode);
bool isVFMSUBADD213PD(unsigned Opcode);
bool isVFMSUBADD213PH(unsigned Opcode);
bool isVBLENDMPS(unsigned Opcode);
bool isVCMPSD(unsigned Opcode);
bool isVCMPSH(unsigned Opcode);
bool isVPCMPGTQ(unsigned Opcode);
bool isANDNPD(unsigned Opcode);
bool isENDBR64(unsigned Opcode);
bool isVFMSUBADD213PS(unsigned Opcode);
bool isVPCMPGTW(unsigned Opcode);
bool isVCMPSS(unsigned Opcode);
bool isPFADD(unsigned Opcode);
bool isVMOVLHPS(unsigned Opcode);
bool isVPMINSD(unsigned Opcode);
bool isVPMINSB(unsigned Opcode);
bool isANDNPS(unsigned Opcode);
bool isPHADDSW(unsigned Opcode);
bool isVPSLLVD(unsigned Opcode);
bool isVDIVPD(unsigned Opcode);
bool isVPMINSQ(unsigned Opcode);
bool isVDIVPH(unsigned Opcode);
bool isVPMINSW(unsigned Opcode);
bool isVFNMSUBPD(unsigned Opcode);
bool isLWPVAL(unsigned Opcode);
bool isAESDEC128KL(unsigned Opcode);
bool isFIADD(unsigned Opcode);
bool isVPAND(unsigned Opcode);
bool isMOVSLDUP(unsigned Opcode);
bool isVPSLLVQ(unsigned Opcode);
bool isVDIVPS(unsigned Opcode);
bool isCWD(unsigned Opcode);
bool isVPSLLVW(unsigned Opcode);
bool isCWDE(unsigned Opcode);
bool isVFNMSUBPS(unsigned Opcode);
bool isVFMSUBADD231PD(unsigned Opcode);
bool isVPDPBUSD(unsigned Opcode);
bool isVFMSUBADD231PH(unsigned Opcode);
bool isPFNACC(unsigned Opcode);
bool isPFRSQRT(unsigned Opcode);
bool isVPMACSDD(unsigned Opcode);
bool isVFMSUBADD231PS(unsigned Opcode);
bool isFRSTOR(unsigned Opcode);
bool isVPMINUB(unsigned Opcode);
bool isVPMINUD(unsigned Opcode);
bool isKUNPCKBW(unsigned Opcode);
bool isDPPD(unsigned Opcode);
bool isVAESIMC(unsigned Opcode);
bool isPTEST(unsigned Opcode);
bool isVPMINUQ(unsigned Opcode);
bool isUCOMISD(unsigned Opcode);
bool isVPMINUW(unsigned Opcode);
bool isDPPS(unsigned Opcode);
bool isFLDLG2(unsigned Opcode);
bool isVPMOVD2M(unsigned Opcode);
bool isVMOVAPD(unsigned Opcode);
bool isVPSRLD(unsigned Opcode);
bool isTLBSYNC(unsigned Opcode);
bool isXLATB(unsigned Opcode);
bool isUCOMISS(unsigned Opcode);
bool isVPSRLQ(unsigned Opcode);
bool isPDEP(unsigned Opcode);
bool isVPDPBUUD(unsigned Opcode);
bool isVEXTRACTI32X4(unsigned Opcode);
bool isPFCMPEQ(unsigned Opcode);
bool isBLSIC(unsigned Opcode);
bool isVPSRLW(unsigned Opcode);
bool isVEXTRACTI32X8(unsigned Opcode);
bool isVDIVSD(unsigned Opcode);
bool isVFCMADDCPH(unsigned Opcode);
bool isVMOVAPS(unsigned Opcode);
bool isFNOP(unsigned Opcode);
bool isVDIVSH(unsigned Opcode);
bool isBT(unsigned Opcode);
bool isVFNMSUBSD(unsigned Opcode);
bool isVPHMINPOSUW(unsigned Opcode);
bool isVDIVSS(unsigned Opcode);
bool isVCVTTSH2SI(unsigned Opcode);
bool isKUNPCKDQ(unsigned Opcode);
bool isMULPD(unsigned Opcode);
bool isBEXTR(unsigned Opcode);
bool isVFNMSUBSS(unsigned Opcode);
bool isMPSADBW(unsigned Opcode);
bool isMULPS(unsigned Opcode);
bool isSHA256MSG1(unsigned Opcode);
bool isSHA256MSG2(unsigned Opcode);
bool isPOPF(unsigned Opcode);
bool isVERR(unsigned Opcode);
bool isPFCMPGE(unsigned Opcode);
bool isVCVTPS2UDQ(unsigned Opcode);
bool isVERW(unsigned Opcode);
bool isVFMSUBADDPD(unsigned Opcode);
bool isPFCMPGT(unsigned Opcode);
bool isVEXTRACTI128(unsigned Opcode);
bool isVGF2P8AFFINEQB(unsigned Opcode);
bool isPSLLD(unsigned Opcode);
bool isFSUBP(unsigned Opcode);
bool isFSUBR(unsigned Opcode);
bool isVBROADCASTF64X2(unsigned Opcode);
bool isFCHS(unsigned Opcode);
bool isCMPXCHG8B(unsigned Opcode);
bool isVBROADCASTF64X4(unsigned Opcode);
bool isVINSERTI32X4(unsigned Opcode);
bool isVFMSUBADDPS(unsigned Opcode);
bool isVBROADCASTF128(unsigned Opcode);
bool isVPERMIL2PD(unsigned Opcode);
bool isPSLLQ(unsigned Opcode);
bool isVINSERTI32X8(unsigned Opcode);
bool isLLDT(unsigned Opcode);
bool isMOVMSKPD(unsigned Opcode);
bool isPSLLW(unsigned Opcode);
bool isVFCMADDCSH(unsigned Opcode);
bool isVPERMIL2PS(unsigned Opcode);
bool isPF2ID(unsigned Opcode);
bool isVPUNPCKHQDQ(unsigned Opcode);
bool isMOVMSKPS(unsigned Opcode);
bool isMULSD(unsigned Opcode);
bool isPF2IW(unsigned Opcode);
bool isVBLENDPD(unsigned Opcode);
bool isCLAC(unsigned Opcode);
bool isMULSS(unsigned Opcode);
bool isORPD(unsigned Opcode);
bool isCDQE(unsigned Opcode);
bool isVBLENDPS(unsigned Opcode);
bool isTILESTORED(unsigned Opcode);
bool isORPS(unsigned Opcode);
bool isVPINSRB(unsigned Opcode);
bool isVPINSRD(unsigned Opcode);
bool isBZHI(unsigned Opcode);
bool isPUNPCKHBW(unsigned Opcode);
bool isVCVTPD2UDQ(unsigned Opcode);
bool isVPINSRQ(unsigned Opcode);
bool isVPINSRW(unsigned Opcode);
bool isVPMACSSDQH(unsigned Opcode);
bool isGETSEC(unsigned Opcode);
bool isCVTSS2SD(unsigned Opcode);
bool isVPMACSSDQL(unsigned Opcode);
bool isANDN(unsigned Opcode);
bool isCVTSS2SI(unsigned Opcode);
bool isAESDEC(unsigned Opcode);
bool isMOVSB(unsigned Opcode);
bool isMOVSD(unsigned Opcode);
bool isVFNMSUB213PD(unsigned Opcode);
bool isVPMOVW2M(unsigned Opcode);
bool isVFNMSUB213PH(unsigned Opcode);
bool isVPACKSSWB(unsigned Opcode);
bool isMOVSQ(unsigned Opcode);
bool isMOVSS(unsigned Opcode);
bool isVPMULHRSW(unsigned Opcode);
bool isVFNMSUB213PS(unsigned Opcode);
bool isCMOVCC(unsigned Opcode);
bool isMOVSW(unsigned Opcode);
bool isMOVSX(unsigned Opcode);
bool isVPCOMUD(unsigned Opcode);
bool isVPCOMUB(unsigned Opcode);
bool isVPDPBSUDS(unsigned Opcode);
bool isFLDLN2(unsigned Opcode);
bool isPACKUSDW(unsigned Opcode);
bool isVPCOMUQ(unsigned Opcode);
bool isMONTMUL(unsigned Opcode);
bool isPUNPCKHDQ(unsigned Opcode);
bool isVPCOMUW(unsigned Opcode);
bool isPMULDQ(unsigned Opcode);
bool isT1MSKC(unsigned Opcode);
bool isIN(unsigned Opcode);
bool isVPHADDBD(unsigned Opcode);
bool isSAR(unsigned Opcode);
bool isVPHADDBQ(unsigned Opcode);
bool isVFNMSUB231PD(unsigned Opcode);
bool isVPSHLDVD(unsigned Opcode);
bool isFSCALE(unsigned Opcode);
bool isVFNMSUB231PH(unsigned Opcode);
bool isVPHADDBW(unsigned Opcode);
bool isSBB(unsigned Opcode);
bool isVPSHLDVQ(unsigned Opcode);
bool isVFNMSUB231PS(unsigned Opcode);
bool isVPDPBUSDS(unsigned Opcode);
bool isFCOMPI(unsigned Opcode);
bool isRSQRTPS(unsigned Opcode);
bool isVSHUFPD(unsigned Opcode);
bool isVPSHLDVW(unsigned Opcode);
bool isVPADDSB(unsigned Opcode);
bool isFCOMPP(unsigned Opcode);
bool isDAA(unsigned Opcode);
bool isVFNMSUB213SD(unsigned Opcode);
bool isVSHUFPS(unsigned Opcode);
bool isINVLPGA(unsigned Opcode);
bool isINVLPGB(unsigned Opcode);
bool isVFNMSUB213SH(unsigned Opcode);
bool isDAS(unsigned Opcode);
bool isVPADDSW(unsigned Opcode);
bool isVFNMSUB213SS(unsigned Opcode);
bool isFCOM(unsigned Opcode);
bool isKXORB(unsigned Opcode);
bool isKXORD(unsigned Opcode);
bool isFCOS(unsigned Opcode);
bool isVPHADDDQ(unsigned Opcode);
bool isCLDEMOTE(unsigned Opcode);
bool isKXORQ(unsigned Opcode);
bool isKXORW(unsigned Opcode);
bool isVDPBF16PS(unsigned Opcode);
bool isCLGI(unsigned Opcode);
bool isVMREAD(unsigned Opcode);
bool isANDPD(unsigned Opcode);
bool isVFMSUBPD(unsigned Opcode);
bool isVFNMADD213PD(unsigned Opcode);
bool isVFNMADD213PH(unsigned Opcode);
bool isVFNMSUB231SD(unsigned Opcode);
bool isFSQRT(unsigned Opcode);
bool isVFNMSUB231SH(unsigned Opcode);
bool isPCLMULQDQ(unsigned Opcode);
bool isVRCP14PD(unsigned Opcode);
bool isANDPS(unsigned Opcode);
bool isVFMSUBPS(unsigned Opcode);
bool isVFNMADD213PS(unsigned Opcode);
bool isPMULHW(unsigned Opcode);
bool isVFNMSUB231SS(unsigned Opcode);
bool isAESDECWIDE256KL(unsigned Opcode);
bool isRSQRTSS(unsigned Opcode);
bool isVRCP14PS(unsigned Opcode);
bool isVZEROUPPER(unsigned Opcode);
bool isVPAVGB(unsigned Opcode);
bool isVPMOVSXBD(unsigned Opcode);
bool isRDGSBASE(unsigned Opcode);
bool isFLDCW(unsigned Opcode);
bool isLIDTD(unsigned Opcode);
bool isVGATHERPF1DPD(unsigned Opcode);
bool isSFENCE(unsigned Opcode);
bool isVPMOVSXBQ(unsigned Opcode);
bool isVPAVGW(unsigned Opcode);
bool isVPMOVSXBW(unsigned Opcode);
bool isFCMOVB(unsigned Opcode);
bool isVMASKMOVDQU(unsigned Opcode);
bool isVGATHERPF1DPS(unsigned Opcode);
bool isFCMOVE(unsigned Opcode);
bool isVMLOAD(unsigned Opcode);
bool isLIDTW(unsigned Opcode);
bool isVFNMADD231PD(unsigned Opcode);
bool isDEC(unsigned Opcode);
bool isVFNMADD231PH(unsigned Opcode);
bool isFCMOVU(unsigned Opcode);
bool isRSTORSSP(unsigned Opcode);
bool isVMAXPD(unsigned Opcode);
bool isVMAXPH(unsigned Opcode);
bool isPUNPCKLBW(unsigned Opcode);
bool isIMUL(unsigned Opcode);
bool isTILELOADDT1(unsigned Opcode);
bool isVFNMADD231PS(unsigned Opcode);
bool isVMAXPS(unsigned Opcode);
bool isVPXOR(unsigned Opcode);
bool isOR(unsigned Opcode);
bool isMOVZX(unsigned Opcode);
bool isXSAVES64(unsigned Opcode);
bool isVPUNPCKHBW(unsigned Opcode);
bool isSYSRET(unsigned Opcode);
bool isVFMSUBSD(unsigned Opcode);
bool isVFNMADD213SD(unsigned Opcode);
bool isVFNMADD213SH(unsigned Opcode);
bool isSGDT(unsigned Opcode);
bool isVRCP14SD(unsigned Opcode);
bool isVPMOVSXDQ(unsigned Opcode);
bool isSYSEXIT(unsigned Opcode);
bool isPMOVZXBD(unsigned Opcode);
bool isVFNMADD213SS(unsigned Opcode);
bool isVPMADCSSWD(unsigned Opcode);
bool isVPMULUDQ(unsigned Opcode);
bool isVFMSUBSS(unsigned Opcode);
bool isVPXORD(unsigned Opcode);
bool isXSAVEOPT64(unsigned Opcode);
bool isXSHA1(unsigned Opcode);
bool isVRCP14SS(unsigned Opcode);
bool isPMOVZXBQ(unsigned Opcode);
bool isSHL(unsigned Opcode);
bool isPMOVZXBW(unsigned Opcode);
bool isPMULLD(unsigned Opcode);
bool isVCVTPS2UQQ(unsigned Opcode);
bool isVPBLENDD(unsigned Opcode);
bool isVPXORQ(unsigned Opcode);
bool isPUNPCKLDQ(unsigned Opcode);
bool isSHR(unsigned Opcode);
bool isVPUNPCKLQDQ(unsigned Opcode);
bool isVCVTPD2DQ(unsigned Opcode);
bool isPMULLW(unsigned Opcode);
bool isVPBLENDW(unsigned Opcode);
bool isAESENCLAST(unsigned Opcode);
bool isVPUNPCKHDQ(unsigned Opcode);
bool isVFNMADD231SD(unsigned Opcode);
bool isVFNMADD231SH(unsigned Opcode);
bool isVPMOVSDB(unsigned Opcode);
bool isVMAXSD(unsigned Opcode);
bool isVMAXSH(unsigned Opcode);
bool isLAHF(unsigned Opcode);
bool isCVTTPD2DQ(unsigned Opcode);
bool isUNPCKHPD(unsigned Opcode);
bool isVFNMADD231SS(unsigned Opcode);
bool isVMAXSS(unsigned Opcode);
bool isVPMACSSDD(unsigned Opcode);
bool isVPMOVSDW(unsigned Opcode);
bool isPMOVZXDQ(unsigned Opcode);
bool isUNPCKHPS(unsigned Opcode);
bool isFCOMI(unsigned Opcode);
bool isCLFLUSH(unsigned Opcode);
bool isTILELOADD(unsigned Opcode);
bool isFCOMP(unsigned Opcode);
bool isDIV(unsigned Opcode);
bool isVPMACSWD(unsigned Opcode);
bool isFPREM1(unsigned Opcode);
bool isVSCATTERQPD(unsigned Opcode);
bool isFYL2X(unsigned Opcode);
bool isVPMACSWW(unsigned Opcode);
bool isPFMAX(unsigned Opcode);
bool isVPSHUFB(unsigned Opcode);
bool isVCVTPD2UQQ(unsigned Opcode);
bool isVPSHUFD(unsigned Opcode);
bool isVSCATTERQPS(unsigned Opcode);
bool isGF2P8AFFINEINVQB(unsigned Opcode);
bool isFEMMS(unsigned Opcode);
bool isKUNPCKWD(unsigned Opcode);
bool isVPCLMULQDQ(unsigned Opcode);
bool isKORB(unsigned Opcode);
bool isVINSERTPS(unsigned Opcode);
bool isKORD(unsigned Opcode);
bool isVMFUNC(unsigned Opcode);
bool isCPUID(unsigned Opcode);
bool isVSCATTERPF0QPD(unsigned Opcode);
bool isOUT(unsigned Opcode);
bool isKORQ(unsigned Opcode);
bool isKORW(unsigned Opcode);
bool isVSCATTERPF0QPS(unsigned Opcode);
bool isPHSUBSW(unsigned Opcode);
bool isPFSUBR(unsigned Opcode);
bool isVCVTPH2UDQ(unsigned Opcode);
bool isVBROADCASTF32X2(unsigned Opcode);
bool isVBROADCASTF32X4(unsigned Opcode);
bool isSHA256RNDS2(unsigned Opcode);
bool isVBROADCASTF32X8(unsigned Opcode);
bool isVPERMB(unsigned Opcode);
bool isVPUNPCKLBW(unsigned Opcode);
bool isVPERMD(unsigned Opcode);
bool isXCRYPTECB(unsigned Opcode);
bool isVPERMQ(unsigned Opcode);
bool isEXTRACTPS(unsigned Opcode);
bool isVPERMW(unsigned Opcode);
bool isHADDPD(unsigned Opcode);
bool isFXSAVE(unsigned Opcode);
bool isVRCPPH(unsigned Opcode);
bool isHADDPS(unsigned Opcode);
bool isVPSADBW(unsigned Opcode);
bool isRDRAND(unsigned Opcode);
bool isVRCPPS(unsigned Opcode);
bool isVXORPD(unsigned Opcode);
bool isFFREE(unsigned Opcode);
bool isPCMPEQB(unsigned Opcode);
bool isPCMPEQD(unsigned Opcode);
bool isVPUNPCKLDQ(unsigned Opcode);
bool isVTESTPD(unsigned Opcode);
bool isVXORPS(unsigned Opcode);
bool isTZCNT(unsigned Opcode);
bool isCLTS(unsigned Opcode);
bool isPCMPEQQ(unsigned Opcode);
bool isVFPCLASSPD(unsigned Opcode);
bool isVFPCLASSPH(unsigned Opcode);
bool isVTESTPS(unsigned Opcode);
bool isPCMPEQW(unsigned Opcode);
bool isUNPCKLPD(unsigned Opcode);
bool isVMOVDQA32(unsigned Opcode);
bool isVFPCLASSPS(unsigned Opcode);
bool isVPMOVMSKB(unsigned Opcode);
bool isFDECSTP(unsigned Opcode);
bool isCLUI(unsigned Opcode);
bool isFLDPI(unsigned Opcode);
bool isUNPCKLPS(unsigned Opcode);
bool isVCVTTPD2DQ(unsigned Opcode);
bool isPAVGUSB(unsigned Opcode);
bool isCALL(unsigned Opcode);
bool isFLDENV(unsigned Opcode);
bool isPACKUSWB(unsigned Opcode);
bool isVPHADDSW(unsigned Opcode);
bool isLAR(unsigned Opcode);
bool isCLFLUSHOPT(unsigned Opcode);
bool isVMMCALL(unsigned Opcode);
bool isARPL(unsigned Opcode);
bool isXABORT(unsigned Opcode);
bool isPUNPCKHWD(unsigned Opcode);
bool isVRCPSH(unsigned Opcode);
bool isLDDQU(unsigned Opcode);
bool isPFMIN(unsigned Opcode);
bool isSYSRETQ(unsigned Opcode);
bool isVRCPSS(unsigned Opcode);
bool isCLWB(unsigned Opcode);
bool isSTC(unsigned Opcode);
bool isSTD(unsigned Opcode);
bool isVMOVDQU8(unsigned Opcode);
bool isSTI(unsigned Opcode);
bool isSTR(unsigned Opcode);
bool isVFPCLASSSD(unsigned Opcode);
bool isLDMXCSR(unsigned Opcode);
bool isVFPCLASSSH(unsigned Opcode);
bool isVCVTPD2PH(unsigned Opcode);
bool isVCVTPH2DQ(unsigned Opcode);
bool isVFPCLASSSS(unsigned Opcode);
bool isVMOVDQA64(unsigned Opcode);
bool isSUB(unsigned Opcode);
bool isVUCOMISD(unsigned Opcode);
bool isVCVTPD2PS(unsigned Opcode);
bool isLEAVE(unsigned Opcode);
bool isOUTSB(unsigned Opcode);
bool isOUTSD(unsigned Opcode);
bool isVUCOMISH(unsigned Opcode);
bool isEXTRQ(unsigned Opcode);
bool isVUCOMISS(unsigned Opcode);
bool isCVTTPD2PI(unsigned Opcode);
bool isOUTSW(unsigned Opcode);
bool isLDS(unsigned Opcode);
bool isPSHUFHW(unsigned Opcode);
bool isPHSUBD(unsigned Opcode);
bool isHLT(unsigned Opcode);
bool isVCVTPD2QQ(unsigned Opcode);
bool isLIDT(unsigned Opcode);
bool isVPTERNLOGD(unsigned Opcode);
bool isLEA(unsigned Opcode);
bool isVPHADDWD(unsigned Opcode);
bool isPREFETCHNTA(unsigned Opcode);
bool isVPTERNLOGQ(unsigned Opcode);
bool isKSHIFTLB(unsigned Opcode);
bool isKSHIFTLD(unsigned Opcode);
bool isPHSUBW(unsigned Opcode);
bool isVPMOVSQB(unsigned Opcode);
bool isVPHADDWQ(unsigned Opcode);
bool isVCVTTSS2USI(unsigned Opcode);
bool isVPMOVSQD(unsigned Opcode);
bool isLES(unsigned Opcode);
bool isVMPSADBW(unsigned Opcode);
bool isKSHIFTLQ(unsigned Opcode);
bool isADDSUBPD(unsigned Opcode);
bool isKSHIFTLW(unsigned Opcode);
bool isXSAVE64(unsigned Opcode);
bool isVPMOVSQW(unsigned Opcode);
bool isADDSUBPS(unsigned Opcode);
bool isENQCMD(unsigned Opcode);
bool isVCVTTPH2W(unsigned Opcode);
bool isVMRUN(unsigned Opcode);
bool isWRFSBASE(unsigned Opcode);
bool isCOMISD(unsigned Opcode);
bool isLFS(unsigned Opcode);
bool isSTOSB(unsigned Opcode);
bool isSTOSD(unsigned Opcode);
bool isCOMISS(unsigned Opcode);
bool isVZEROALL(unsigned Opcode);
bool isVFMADDSUBPD(unsigned Opcode);
bool isVEXTRACTPS(unsigned Opcode);
bool isKADDB(unsigned Opcode);
bool isKADDD(unsigned Opcode);
bool isXTEST(unsigned Opcode);
bool isFISTTP(unsigned Opcode);
bool isSTOSQ(unsigned Opcode);
bool isJCXZ(unsigned Opcode);
bool isSTOSW(unsigned Opcode);
bool isVFMADDSUBPS(unsigned Opcode);
bool isVPSHAB(unsigned Opcode);
bool isKADDQ(unsigned Opcode);
bool isLGS(unsigned Opcode);
bool isVPMACSDQH(unsigned Opcode);
bool isMOVDQA(unsigned Opcode);
bool isVPSHAD(unsigned Opcode);
bool isKADDW(unsigned Opcode);
bool isPSMASH(unsigned Opcode);
bool isPBLENDVB(unsigned Opcode);
bool isVPMACSDQL(unsigned Opcode);
bool isVPHSUBBW(unsigned Opcode);
bool isVPSHAQ(unsigned Opcode);
bool isVSQRTPD(unsigned Opcode);
bool isCLRSSBSY(unsigned Opcode);
bool isMINPD(unsigned Opcode);
bool isVPSHAW(unsigned Opcode);
bool isVSQRTPH(unsigned Opcode);
bool isMOVDQU(unsigned Opcode);
bool isVSQRTPS(unsigned Opcode);
bool isVPSUBSB(unsigned Opcode);
bool isMULX(unsigned Opcode);
bool isMINPS(unsigned Opcode);
bool isPSHUFLW(unsigned Opcode);
bool isVEXTRACTF64X2(unsigned Opcode);
bool isVEXTRACTF64X4(unsigned Opcode);
bool isVCVTTSD2USI(unsigned Opcode);
bool isINCSSPD(unsigned Opcode);
bool isVPSIGNB(unsigned Opcode);
bool isVPSUBSW(unsigned Opcode);
bool isVPSIGND(unsigned Opcode);
bool isXSAVEOPT(unsigned Opcode);
bool isVPMOVSXWD(unsigned Opcode);
bool isINCSSPQ(unsigned Opcode);
bool isGF2P8AFFINEQB(unsigned Opcode);
bool isVPMOVSXWQ(unsigned Opcode);
bool isVPHSUBDQ(unsigned Opcode);
bool isVPSIGNW(unsigned Opcode);
bool isSGDTD(unsigned Opcode);
bool isPUNPCKLWD(unsigned Opcode);
bool isVPPERM(unsigned Opcode);
bool isAAA(unsigned Opcode);
bool isPAND(unsigned Opcode);
bool isVCVTPH2UQQ(unsigned Opcode);
bool isAAD(unsigned Opcode);
bool isSGDTW(unsigned Opcode);
bool isVPUNPCKHWD(unsigned Opcode);
bool isAAM(unsigned Opcode);
bool isVCVTNEOPH2PS(unsigned Opcode);
bool isAAS(unsigned Opcode);
bool isVSQRTSD(unsigned Opcode);
bool isBLCI(unsigned Opcode);
bool isMINSD(unsigned Opcode);
bool isVPSHUFBITQMB(unsigned Opcode);
bool isKSHIFTRB(unsigned Opcode);
bool isUMONITOR(unsigned Opcode);
bool isKSHIFTRD(unsigned Opcode);
bool isFNCLEX(unsigned Opcode);
bool isVSQRTSH(unsigned Opcode);
bool isBLCS(unsigned Opcode);
bool isVINSERTF64X2(unsigned Opcode);
bool isVSQRTSS(unsigned Opcode);
bool isVINSERTF64X4(unsigned Opcode);
bool isMINSS(unsigned Opcode);
bool isVPBROADCASTMB2Q(unsigned Opcode);
bool isKSHIFTRQ(unsigned Opcode);
bool isVMOVSHDUP(unsigned Opcode);
bool isVPMOVSWB(unsigned Opcode);
bool isPMOVZXWD(unsigned Opcode);
bool isFSIN(unsigned Opcode);
bool isPSLLDQ(unsigned Opcode);
bool isKSHIFTRW(unsigned Opcode);
bool isVPADDD(unsigned Opcode);
bool isVPADDB(unsigned Opcode);
bool isVPMACSSWD(unsigned Opcode);
bool isPMOVZXWQ(unsigned Opcode);
bool isVPADDQ(unsigned Opcode);
bool isVPADDW(unsigned Opcode);
bool isVRSQRTPH(unsigned Opcode);
bool isVPMACSSWW(unsigned Opcode);
bool isVRSQRTPS(unsigned Opcode);
bool isVCVTTPH2DQ(unsigned Opcode);
bool isWRMSR(unsigned Opcode);
bool isXSETBV(unsigned Opcode);
bool isMOVSXD(unsigned Opcode);
bool isADC(unsigned Opcode);
bool isADD(unsigned Opcode);
bool isFDIV(unsigned Opcode);
bool isAESDEC256KL(unsigned Opcode);
bool isVPCMPUB(unsigned Opcode);
bool isVCVTTPD2QQ(unsigned Opcode);
bool isVPCMPUD(unsigned Opcode);
bool isPFMUL(unsigned Opcode);
bool isPREFETCHW(unsigned Opcode);
bool isVPCMPUQ(unsigned Opcode);
bool isKORTESTB(unsigned Opcode);
bool isMOVHPD(unsigned Opcode);
bool isKORTESTD(unsigned Opcode);
bool isCVTSI2SD(unsigned Opcode);
bool isFSUBRP(unsigned Opcode);
bool isIRET(unsigned Opcode);
bool isPTWRITE(unsigned Opcode);
bool isVPCMPUW(unsigned Opcode);
bool isKORTESTQ(unsigned Opcode);
bool isMOVHPS(unsigned Opcode);
bool isFIMUL(unsigned Opcode);
bool isCVTSI2SS(unsigned Opcode);
bool isVCVTPH2PD(unsigned Opcode);
bool isKORTESTW(unsigned Opcode);
bool isPADDUSB(unsigned Opcode);
bool isVSHUFI64X2(unsigned Opcode);
bool isVRSQRTSH(unsigned Opcode);
bool isVMWRITE(unsigned Opcode);
bool isTILERELEASE(unsigned Opcode);
bool isVCVTPH2PS(unsigned Opcode);
bool isVMOVUPD(unsigned Opcode);
bool isPADDUSW(unsigned Opcode);
bool isVRSQRTSS(unsigned Opcode);
bool isVPCMOV(unsigned Opcode);
bool isVCVTUSI2SD(unsigned Opcode);
bool isVMOVUPS(unsigned Opcode);
bool isVCVTUSI2SH(unsigned Opcode);
bool isVPCMPB(unsigned Opcode);
bool isVPGATHERDD(unsigned Opcode);
bool isVPCMPD(unsigned Opcode);
bool isVCVTUSI2SS(unsigned Opcode);
bool isLFENCE(unsigned Opcode);
bool isVCVTPH2QQ(unsigned Opcode);
bool isVGATHERPF0DPD(unsigned Opcode);
bool isSEAMRET(unsigned Opcode);
bool isPI2FD(unsigned Opcode);
bool isVPGATHERDQ(unsigned Opcode);
bool isPCMPESTRI(unsigned Opcode);
bool isVPCMPQ(unsigned Opcode);
bool isPCMPESTRM(unsigned Opcode);
bool isVPCMPW(unsigned Opcode);
bool isVGATHERPF0DPS(unsigned Opcode);
bool isVPMOVUSDB(unsigned Opcode);
bool isPI2FW(unsigned Opcode);
bool isSYSEXITQ(unsigned Opcode);
bool isCVTPS2DQ(unsigned Opcode);
bool isRDPKRU(unsigned Opcode);
bool isVPMOVUSDW(unsigned Opcode);
bool isPSUBB(unsigned Opcode);
bool isPSUBD(unsigned Opcode);
bool isVPSHRDD(unsigned Opcode);
bool isRETFQ(unsigned Opcode);
bool isVPERMT2PD(unsigned Opcode);
bool isMOVABS(unsigned Opcode);
bool isVPSHRDQ(unsigned Opcode);
bool isPSUBQ(unsigned Opcode);
bool isVPSHRDW(unsigned Opcode);
bool isVPSHLB(unsigned Opcode);
bool isPSUBW(unsigned Opcode);
bool isVPSHLD(unsigned Opcode);
bool isVPERMT2PS(unsigned Opcode);
bool isVPUNPCKLWD(unsigned Opcode);
bool isVPSHLQ(unsigned Opcode);
bool isVPSHLW(unsigned Opcode);
bool isLSL(unsigned Opcode);
bool isVBROADCASTI128(unsigned Opcode);
bool isLSS(unsigned Opcode);
bool isVPHADDD(unsigned Opcode);
bool isADDPD(unsigned Opcode);
bool isVMASKMOVPD(unsigned Opcode);
bool isADDPS(unsigned Opcode);
bool isINSB(unsigned Opcode);
bool isINSD(unsigned Opcode);
bool isVPHADDW(unsigned Opcode);
bool isLTR(unsigned Opcode);
bool isVMASKMOVPS(unsigned Opcode);
bool isVCVTPH2UW(unsigned Opcode);
bool isINT3(unsigned Opcode);
bool isKNOTB(unsigned Opcode);
bool isKNOTD(unsigned Opcode);
bool isINSW(unsigned Opcode);
bool isVBLENDVPD(unsigned Opcode);
bool isBLSFILL(unsigned Opcode);
bool isMONITOR(unsigned Opcode);
bool isKNOTQ(unsigned Opcode);
bool isCMPXCHG16B(unsigned Opcode);
bool isKNOTW(unsigned Opcode);
bool isPEXTRB(unsigned Opcode);
bool isVPRORVD(unsigned Opcode);
bool isPEXTRD(unsigned Opcode);
bool isVAESENCLAST(unsigned Opcode);
bool isINTO(unsigned Opcode);
bool isVBLENDVPS(unsigned Opcode);
bool isVPRORVQ(unsigned Opcode);
bool isPEXTRQ(unsigned Opcode);
bool isHSUBPD(unsigned Opcode);
bool isPEXTRW(unsigned Opcode);
bool isFDIVRP(unsigned Opcode);
bool isSCASB(unsigned Opcode);
bool isF2XM1(unsigned Opcode);
bool isSCASD(unsigned Opcode);
bool isFISUBR(unsigned Opcode);
bool isMOVLPD(unsigned Opcode);
bool isHSUBPS(unsigned Opcode);
bool isSCASQ(unsigned Opcode);
bool isFSTP(unsigned Opcode);
bool isVDBPSADBW(unsigned Opcode);
bool isADDSD(unsigned Opcode);
bool isMOVLPS(unsigned Opcode);
bool isSCASW(unsigned Opcode);
bool isVCVTW2PH(unsigned Opcode);
bool isVPTEST(unsigned Opcode);
bool isFLD1(unsigned Opcode);
bool isWBINVD(unsigned Opcode);
bool isADDSS(unsigned Opcode);
bool isPOP(unsigned Opcode);
bool isINVD(unsigned Opcode);
bool isPOR(unsigned Opcode);
bool isAND(unsigned Opcode);
bool isFYL2XP1(unsigned Opcode);
bool isFSUB(unsigned Opcode);
bool isENTER(unsigned Opcode);
bool isVMOVSLDUP(unsigned Opcode);
bool isADCX(unsigned Opcode);
bool isXADD(unsigned Opcode);
bool isAESENC(unsigned Opcode);
bool isFLDZ(unsigned Opcode);
bool isXRSTORS64(unsigned Opcode);
bool isVCVTTSH2USI(unsigned Opcode);
bool isVMULPD(unsigned Opcode);
bool isFDIVP(unsigned Opcode);
bool isVGETMANTPD(unsigned Opcode);
bool isFDIVR(unsigned Opcode);
bool isVPMOVM2D(unsigned Opcode);
bool isVMULPH(unsigned Opcode);
bool isVGETMANTPH(unsigned Opcode);
bool isVPMOVM2B(unsigned Opcode);
bool isAOR(unsigned Opcode);
bool isVPHSUBSW(unsigned Opcode);
bool isVMULPS(unsigned Opcode);
bool isVPMOVM2Q(unsigned Opcode);
bool isMOVNTDQ(unsigned Opcode);
bool isVGETMANTPS(unsigned Opcode);
bool isVPCMPESTRI(unsigned Opcode);
bool isVPMOVM2W(unsigned Opcode);
bool isVPCMPESTRM(unsigned Opcode);
bool isSYSENTER(unsigned Opcode);
bool isVPERMPD(unsigned Opcode);
bool isVCVTTPH2QQ(unsigned Opcode);
bool isPUSHF(unsigned Opcode);
bool isPXOR(unsigned Opcode);
bool isCMPXCHG(unsigned Opcode);
bool isVPERMPS(unsigned Opcode);
bool isVMRESUME(unsigned Opcode);
bool isVPSLLD(unsigned Opcode);
bool isVPSLLQ(unsigned Opcode);
bool isVEXTRACTF32X4(unsigned Opcode);
bool isVPSLLW(unsigned Opcode);
bool isBLSI(unsigned Opcode);
bool isVEXTRACTF32X8(unsigned Opcode);
bool isBLSR(unsigned Opcode);
bool isVMULSD(unsigned Opcode);
bool isVGETMANTSD(unsigned Opcode);
bool isVPCMPEQB(unsigned Opcode);
bool isVMULSH(unsigned Opcode);
bool isVPCMPEQD(unsigned Opcode);
bool isVGETMANTSH(unsigned Opcode);
bool isLMSW(unsigned Opcode);
bool isFNSTENV(unsigned Opcode);
bool isVMULSS(unsigned Opcode);
bool isVGETMANTSS(unsigned Opcode);
bool isVPCMPEQQ(unsigned Opcode);
bool isVPCMPEQW(unsigned Opcode);
bool isTDPBSSD(unsigned Opcode);
bool isVPHSUBWD(unsigned Opcode);
bool isUMWAIT(unsigned Opcode);
bool isBSWAP(unsigned Opcode);
bool isVCVTUQQ2PD(unsigned Opcode);
bool isPEXT(unsigned Opcode);
bool isVCVTUQQ2PH(unsigned Opcode);
bool isVMOVMSKPD(unsigned Opcode);
bool isLOADIWKEY(unsigned Opcode);
bool isPMINSB(unsigned Opcode);
bool isPMINSD(unsigned Opcode);
bool isVCVTUQQ2PS(unsigned Opcode);
bool isSTMXCSR(unsigned Opcode);
bool isCVTPS2PD(unsigned Opcode);
bool isVMOVMSKPS(unsigned Opcode);
bool isVPROLD(unsigned Opcode);
bool isFPTAN(unsigned Opcode);
bool isCVTPS2PI(unsigned Opcode);
bool isVMXOFF(unsigned Opcode);
bool isXRSTOR64(unsigned Opcode);
bool isPMINSW(unsigned Opcode);
bool isVMOVSD(unsigned Opcode);
bool isVPGATHERQD(unsigned Opcode);
bool isVINSERTF32X4(unsigned Opcode);
bool isVMOVSH(unsigned Opcode);
bool isVPGATHERQQ(unsigned Opcode);
bool isVPROLQ(unsigned Opcode);
bool isVINSERTF32X8(unsigned Opcode);
bool isXSAVE(unsigned Opcode);
bool isTDPFP16PS(unsigned Opcode);
bool isVCVTTPH2UW(unsigned Opcode);
bool isVMOVSS(unsigned Opcode);
bool isRDTSCP(unsigned Opcode);
bool isVPMOVUSQB(unsigned Opcode);
bool isVPMOVUSQD(unsigned Opcode);
bool isTDPBSUD(unsigned Opcode);
bool isBLCMSK(unsigned Opcode);
bool isVPMOVUSQW(unsigned Opcode);
bool isVPMADCSWD(unsigned Opcode);
bool isVGATHERDPD(unsigned Opcode);
bool isSHLD(unsigned Opcode);
bool isPMINUB(unsigned Opcode);
bool isPMINUD(unsigned Opcode);
bool isAESIMC(unsigned Opcode);
bool isCVTSD2SI(unsigned Opcode);
bool isVLDMXCSR(unsigned Opcode);
bool isVCVTSS2SD(unsigned Opcode);
bool isVCVTSS2SH(unsigned Opcode);
bool isSLDT(unsigned Opcode);
bool isVCVTSS2SI(unsigned Opcode);
bool isVGATHERDPS(unsigned Opcode);
bool isFABS(unsigned Opcode);
bool isCVTSD2SS(unsigned Opcode);
bool isSHLX(unsigned Opcode);
bool isMONITORX(unsigned Opcode);
bool isPMINUW(unsigned Opcode);
bool isVPMAXSB(unsigned Opcode);
bool isVPMAXSD(unsigned Opcode);
bool isMOVAPD(unsigned Opcode);
bool isENQCMDS(unsigned Opcode);
bool isVMOVD(unsigned Opcode);
bool isVPMAXSQ(unsigned Opcode);
bool isCVTTSS2SI(unsigned Opcode);
bool isVPMAXSW(unsigned Opcode);
bool isMOVAPS(unsigned Opcode);
bool isVPACKUSDW(unsigned Opcode);
bool isVFIXUPIMMPD(unsigned Opcode);
bool isVMOVQ(unsigned Opcode);
bool isVPSHUFHW(unsigned Opcode);
bool isPCMPISTRI(unsigned Opcode);
bool isVMOVW(unsigned Opcode);
#endif // GET_X86_MNEMONIC_TABLES_H

#ifdef GET_X86_MNEMONIC_TABLES_CPP
#undef GET_X86_MNEMONIC_TABLES_CPP

bool isPCMPISTRM(unsigned Opcode) {
	switch (Opcode) {
	case PCMPISTRMrm:
	case PCMPISTRMrr:
		return true;
	}
	return false;
}

bool isVALIGND(unsigned Opcode) {
	switch (Opcode) {
	case VALIGNDZ128rmbi:
	case VALIGNDZ128rmbik:
	case VALIGNDZ128rmbikz:
	case VALIGNDZ128rmi:
	case VALIGNDZ128rmik:
	case VALIGNDZ128rmikz:
	case VALIGNDZ128rri:
	case VALIGNDZ128rrik:
	case VALIGNDZ128rrikz:
	case VALIGNDZ256rmbi:
	case VALIGNDZ256rmbik:
	case VALIGNDZ256rmbikz:
	case VALIGNDZ256rmi:
	case VALIGNDZ256rmik:
	case VALIGNDZ256rmikz:
	case VALIGNDZ256rri:
	case VALIGNDZ256rrik:
	case VALIGNDZ256rrikz:
	case VALIGNDZrmbi:
	case VALIGNDZrmbik:
	case VALIGNDZrmbikz:
	case VALIGNDZrmi:
	case VALIGNDZrmik:
	case VALIGNDZrmikz:
	case VALIGNDZrri:
	case VALIGNDZrrik:
	case VALIGNDZrrikz:
		return true;
	}
	return false;
}

bool isVFMULCPH(unsigned Opcode) {
	switch (Opcode) {
	case VFMULCPHZ128rm:
	case VFMULCPHZ128rmb:
	case VFMULCPHZ128rmbk:
	case VFMULCPHZ128rmbkz:
	case VFMULCPHZ128rmk:
	case VFMULCPHZ128rmkz:
	case VFMULCPHZ128rr:
	case VFMULCPHZ128rrk:
	case VFMULCPHZ128rrkz:
	case VFMULCPHZ256rm:
	case VFMULCPHZ256rmb:
	case VFMULCPHZ256rmbk:
	case VFMULCPHZ256rmbkz:
	case VFMULCPHZ256rmk:
	case VFMULCPHZ256rmkz:
	case VFMULCPHZ256rr:
	case VFMULCPHZ256rrk:
	case VFMULCPHZ256rrkz:
	case VFMULCPHZrm:
	case VFMULCPHZrmb:
	case VFMULCPHZrmbk:
	case VFMULCPHZrmbkz:
	case VFMULCPHZrmk:
	case VFMULCPHZrmkz:
	case VFMULCPHZrr:
	case VFMULCPHZrrb:
	case VFMULCPHZrrbk:
	case VFMULCPHZrrbkz:
	case VFMULCPHZrrk:
	case VFMULCPHZrrkz:
		return true;
	}
	return false;
}

bool isVPDPBSSD(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBSSDYrm:
	case VPDPBSSDYrr:
	case VPDPBSSDrm:
	case VPDPBSSDrr:
		return true;
	}
	return false;
}

bool isVFIXUPIMMPS(unsigned Opcode) {
	switch (Opcode) {
	case VFIXUPIMMPSZ128rmbi:
	case VFIXUPIMMPSZ128rmbik:
	case VFIXUPIMMPSZ128rmbikz:
	case VFIXUPIMMPSZ128rmi:
	case VFIXUPIMMPSZ128rmik:
	case VFIXUPIMMPSZ128rmikz:
	case VFIXUPIMMPSZ128rri:
	case VFIXUPIMMPSZ128rrik:
	case VFIXUPIMMPSZ128rrikz:
	case VFIXUPIMMPSZ256rmbi:
	case VFIXUPIMMPSZ256rmbik:
	case VFIXUPIMMPSZ256rmbikz:
	case VFIXUPIMMPSZ256rmi:
	case VFIXUPIMMPSZ256rmik:
	case VFIXUPIMMPSZ256rmikz:
	case VFIXUPIMMPSZ256rri:
	case VFIXUPIMMPSZ256rrik:
	case VFIXUPIMMPSZ256rrikz:
	case VFIXUPIMMPSZrmbi:
	case VFIXUPIMMPSZrmbik:
	case VFIXUPIMMPSZrmbikz:
	case VFIXUPIMMPSZrmi:
	case VFIXUPIMMPSZrmik:
	case VFIXUPIMMPSZrmikz:
	case VFIXUPIMMPSZrri:
	case VFIXUPIMMPSZrrib:
	case VFIXUPIMMPSZrribk:
	case VFIXUPIMMPSZrribkz:
	case VFIXUPIMMPSZrrik:
	case VFIXUPIMMPSZrrikz:
		return true;
	}
	return false;
}

bool isVPMOVQ2M(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVQ2MZ128rr:
	case VPMOVQ2MZ256rr:
	case VPMOVQ2MZrr:
		return true;
	}
	return false;
}

bool isLDTILECFG(unsigned Opcode) {
	return Opcode == LDTILECFG;
}

bool isFADD(unsigned Opcode) {
	switch (Opcode) {
	case ADD_F32m:
	case ADD_F64m:
	case ADD_FST0r:
	case ADD_FrST0:
		return true;
	}
	return false;
}

bool isVALIGNQ(unsigned Opcode) {
	switch (Opcode) {
	case VALIGNQZ128rmbi:
	case VALIGNQZ128rmbik:
	case VALIGNQZ128rmbikz:
	case VALIGNQZ128rmi:
	case VALIGNQZ128rmik:
	case VALIGNQZ128rmikz:
	case VALIGNQZ128rri:
	case VALIGNQZ128rrik:
	case VALIGNQZ128rrikz:
	case VALIGNQZ256rmbi:
	case VALIGNQZ256rmbik:
	case VALIGNQZ256rmbikz:
	case VALIGNQZ256rmi:
	case VALIGNQZ256rmik:
	case VALIGNQZ256rmikz:
	case VALIGNQZ256rri:
	case VALIGNQZ256rrik:
	case VALIGNQZ256rrikz:
	case VALIGNQZrmbi:
	case VALIGNQZrmbik:
	case VALIGNQZrmbikz:
	case VALIGNQZrmi:
	case VALIGNQZrmik:
	case VALIGNQZrmikz:
	case VALIGNQZrri:
	case VALIGNQZrrik:
	case VALIGNQZrrikz:
		return true;
	}
	return false;
}

bool isAESENC128KL(unsigned Opcode) {
	return Opcode == AESENC128KL;
}

bool isVPMAXUB(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXUBYrm:
	case VPMAXUBYrr:
	case VPMAXUBZ128rm:
	case VPMAXUBZ128rmk:
	case VPMAXUBZ128rmkz:
	case VPMAXUBZ128rr:
	case VPMAXUBZ128rrk:
	case VPMAXUBZ128rrkz:
	case VPMAXUBZ256rm:
	case VPMAXUBZ256rmk:
	case VPMAXUBZ256rmkz:
	case VPMAXUBZ256rr:
	case VPMAXUBZ256rrk:
	case VPMAXUBZ256rrkz:
	case VPMAXUBZrm:
	case VPMAXUBZrmk:
	case VPMAXUBZrmkz:
	case VPMAXUBZrr:
	case VPMAXUBZrrk:
	case VPMAXUBZrrkz:
	case VPMAXUBrm:
	case VPMAXUBrr:
		return true;
	}
	return false;
}

bool isFMULP(unsigned Opcode) {
	return Opcode == MUL_FPrST0;
}

bool isVPMAXUD(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXUDYrm:
	case VPMAXUDYrr:
	case VPMAXUDZ128rm:
	case VPMAXUDZ128rmb:
	case VPMAXUDZ128rmbk:
	case VPMAXUDZ128rmbkz:
	case VPMAXUDZ128rmk:
	case VPMAXUDZ128rmkz:
	case VPMAXUDZ128rr:
	case VPMAXUDZ128rrk:
	case VPMAXUDZ128rrkz:
	case VPMAXUDZ256rm:
	case VPMAXUDZ256rmb:
	case VPMAXUDZ256rmbk:
	case VPMAXUDZ256rmbkz:
	case VPMAXUDZ256rmk:
	case VPMAXUDZ256rmkz:
	case VPMAXUDZ256rr:
	case VPMAXUDZ256rrk:
	case VPMAXUDZ256rrkz:
	case VPMAXUDZrm:
	case VPMAXUDZrmb:
	case VPMAXUDZrmbk:
	case VPMAXUDZrmbkz:
	case VPMAXUDZrmk:
	case VPMAXUDZrmkz:
	case VPMAXUDZrr:
	case VPMAXUDZrrk:
	case VPMAXUDZrrkz:
	case VPMAXUDrm:
	case VPMAXUDrr:
		return true;
	}
	return false;
}

bool isCMPCC(unsigned Opcode) {
	switch (Opcode) {
	case CMPCCXADDmr32:
	case CMPCCXADDmr64:
		return true;
	}
	return false;
}

bool isVSHUFI32X4(unsigned Opcode) {
	switch (Opcode) {
	case VSHUFI32X4Z256rmbi:
	case VSHUFI32X4Z256rmbik:
	case VSHUFI32X4Z256rmbikz:
	case VSHUFI32X4Z256rmi:
	case VSHUFI32X4Z256rmik:
	case VSHUFI32X4Z256rmikz:
	case VSHUFI32X4Z256rri:
	case VSHUFI32X4Z256rrik:
	case VSHUFI32X4Z256rrikz:
	case VSHUFI32X4Zrmbi:
	case VSHUFI32X4Zrmbik:
	case VSHUFI32X4Zrmbikz:
	case VSHUFI32X4Zrmi:
	case VSHUFI32X4Zrmik:
	case VSHUFI32X4Zrmikz:
	case VSHUFI32X4Zrri:
	case VSHUFI32X4Zrrik:
	case VSHUFI32X4Zrrikz:
		return true;
	}
	return false;
}

bool isLOOPNE(unsigned Opcode) {
	return Opcode == LOOPNE;
}

bool isVPMAXUQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXUQZ128rm:
	case VPMAXUQZ128rmb:
	case VPMAXUQZ128rmbk:
	case VPMAXUQZ128rmbkz:
	case VPMAXUQZ128rmk:
	case VPMAXUQZ128rmkz:
	case VPMAXUQZ128rr:
	case VPMAXUQZ128rrk:
	case VPMAXUQZ128rrkz:
	case VPMAXUQZ256rm:
	case VPMAXUQZ256rmb:
	case VPMAXUQZ256rmbk:
	case VPMAXUQZ256rmbkz:
	case VPMAXUQZ256rmk:
	case VPMAXUQZ256rmkz:
	case VPMAXUQZ256rr:
	case VPMAXUQZ256rrk:
	case VPMAXUQZ256rrkz:
	case VPMAXUQZrm:
	case VPMAXUQZrmb:
	case VPMAXUQZrmbk:
	case VPMAXUQZrmbkz:
	case VPMAXUQZrmk:
	case VPMAXUQZrmkz:
	case VPMAXUQZrr:
	case VPMAXUQZrrk:
	case VPMAXUQZrrkz:
		return true;
	}
	return false;
}

bool isVCOMPRESSPD(unsigned Opcode) {
	switch (Opcode) {
	case VCOMPRESSPDZ128mr:
	case VCOMPRESSPDZ128mrk:
	case VCOMPRESSPDZ128rr:
	case VCOMPRESSPDZ128rrk:
	case VCOMPRESSPDZ128rrkz:
	case VCOMPRESSPDZ256mr:
	case VCOMPRESSPDZ256mrk:
	case VCOMPRESSPDZ256rr:
	case VCOMPRESSPDZ256rrk:
	case VCOMPRESSPDZ256rrkz:
	case VCOMPRESSPDZmr:
	case VCOMPRESSPDZmrk:
	case VCOMPRESSPDZrr:
	case VCOMPRESSPDZrrk:
	case VCOMPRESSPDZrrkz:
		return true;
	}
	return false;
}

bool isVPMAXUW(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXUWYrm:
	case VPMAXUWYrr:
	case VPMAXUWZ128rm:
	case VPMAXUWZ128rmk:
	case VPMAXUWZ128rmkz:
	case VPMAXUWZ128rr:
	case VPMAXUWZ128rrk:
	case VPMAXUWZ128rrkz:
	case VPMAXUWZ256rm:
	case VPMAXUWZ256rmk:
	case VPMAXUWZ256rmkz:
	case VPMAXUWZ256rr:
	case VPMAXUWZ256rrk:
	case VPMAXUWZ256rrkz:
	case VPMAXUWZrm:
	case VPMAXUWZrmk:
	case VPMAXUWZrmkz:
	case VPMAXUWZrr:
	case VPMAXUWZrrk:
	case VPMAXUWZrrkz:
	case VPMAXUWrm:
	case VPMAXUWrr:
		return true;
	}
	return false;
}

bool isVPMOVB2M(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVB2MZ128rr:
	case VPMOVB2MZ256rr:
	case VPMOVB2MZrr:
		return true;
	}
	return false;
}

bool isVANDPD(unsigned Opcode) {
	switch (Opcode) {
	case VANDPDYrm:
	case VANDPDYrr:
	case VANDPDZ128rm:
	case VANDPDZ128rmb:
	case VANDPDZ128rmbk:
	case VANDPDZ128rmbkz:
	case VANDPDZ128rmk:
	case VANDPDZ128rmkz:
	case VANDPDZ128rr:
	case VANDPDZ128rrk:
	case VANDPDZ128rrkz:
	case VANDPDZ256rm:
	case VANDPDZ256rmb:
	case VANDPDZ256rmbk:
	case VANDPDZ256rmbkz:
	case VANDPDZ256rmk:
	case VANDPDZ256rmkz:
	case VANDPDZ256rr:
	case VANDPDZ256rrk:
	case VANDPDZ256rrkz:
	case VANDPDZrm:
	case VANDPDZrmb:
	case VANDPDZrmbk:
	case VANDPDZrmbkz:
	case VANDPDZrmk:
	case VANDPDZrmkz:
	case VANDPDZrr:
	case VANDPDZrrk:
	case VANDPDZrrkz:
	case VANDPDrm:
	case VANDPDrr:
		return true;
	}
	return false;
}

bool isVCOMPRESSPS(unsigned Opcode) {
	switch (Opcode) {
	case VCOMPRESSPSZ128mr:
	case VCOMPRESSPSZ128mrk:
	case VCOMPRESSPSZ128rr:
	case VCOMPRESSPSZ128rrk:
	case VCOMPRESSPSZ128rrkz:
	case VCOMPRESSPSZ256mr:
	case VCOMPRESSPSZ256mrk:
	case VCOMPRESSPSZ256rr:
	case VCOMPRESSPSZ256rrk:
	case VCOMPRESSPSZ256rrkz:
	case VCOMPRESSPSZmr:
	case VCOMPRESSPSZmrk:
	case VCOMPRESSPSZrr:
	case VCOMPRESSPSZrrk:
	case VCOMPRESSPSZrrkz:
		return true;
	}
	return false;
}

bool isVPDPBSUD(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBSUDYrm:
	case VPDPBSUDYrr:
	case VPDPBSUDrm:
	case VPDPBSUDrr:
		return true;
	}
	return false;
}

bool isPFRCPIT1(unsigned Opcode) {
	switch (Opcode) {
	case PFRCPIT1rm:
	case PFRCPIT1rr:
		return true;
	}
	return false;
}

bool isPFRCPIT2(unsigned Opcode) {
	switch (Opcode) {
	case PFRCPIT2rm:
	case PFRCPIT2rr:
		return true;
	}
	return false;
}

bool isWRPKRU(unsigned Opcode) {
	return Opcode == WRPKRUr;
}

bool isVANDPS(unsigned Opcode) {
	switch (Opcode) {
	case VANDPSYrm:
	case VANDPSYrr:
	case VANDPSZ128rm:
	case VANDPSZ128rmb:
	case VANDPSZ128rmbk:
	case VANDPSZ128rmbkz:
	case VANDPSZ128rmk:
	case VANDPSZ128rmkz:
	case VANDPSZ128rr:
	case VANDPSZ128rrk:
	case VANDPSZ128rrkz:
	case VANDPSZ256rm:
	case VANDPSZ256rmb:
	case VANDPSZ256rmbk:
	case VANDPSZ256rmbkz:
	case VANDPSZ256rmk:
	case VANDPSZ256rmkz:
	case VANDPSZ256rr:
	case VANDPSZ256rrk:
	case VANDPSZ256rrkz:
	case VANDPSZrm:
	case VANDPSZrmb:
	case VANDPSZrmbk:
	case VANDPSZrmbkz:
	case VANDPSZrmk:
	case VANDPSZrmkz:
	case VANDPSZrr:
	case VANDPSZrrk:
	case VANDPSZrrkz:
	case VANDPSrm:
	case VANDPSrr:
		return true;
	}
	return false;
}

bool isWRUSSD(unsigned Opcode) {
	return Opcode == WRUSSD;
}

bool isVMPTRLD(unsigned Opcode) {
	return Opcode == VMPTRLDm;
}

bool isWRUSSQ(unsigned Opcode) {
	return Opcode == WRUSSQ;
}

bool isAESDECLAST(unsigned Opcode) {
	switch (Opcode) {
	case AESDECLASTrm:
	case AESDECLASTrr:
		return true;
	}
	return false;
}

bool isSYSCALL(unsigned Opcode) {
	return Opcode == SYSCALL;
}

bool isVFIXUPIMMSD(unsigned Opcode) {
	switch (Opcode) {
	case VFIXUPIMMSDZrmi:
	case VFIXUPIMMSDZrmik:
	case VFIXUPIMMSDZrmikz:
	case VFIXUPIMMSDZrri:
	case VFIXUPIMMSDZrrib:
	case VFIXUPIMMSDZrribk:
	case VFIXUPIMMSDZrribkz:
	case VFIXUPIMMSDZrrik:
	case VFIXUPIMMSDZrrikz:
		return true;
	}
	return false;
}

bool isVPRORD(unsigned Opcode) {
	switch (Opcode) {
	case VPRORDZ128mbi:
	case VPRORDZ128mbik:
	case VPRORDZ128mbikz:
	case VPRORDZ128mi:
	case VPRORDZ128mik:
	case VPRORDZ128mikz:
	case VPRORDZ128ri:
	case VPRORDZ128rik:
	case VPRORDZ128rikz:
	case VPRORDZ256mbi:
	case VPRORDZ256mbik:
	case VPRORDZ256mbikz:
	case VPRORDZ256mi:
	case VPRORDZ256mik:
	case VPRORDZ256mikz:
	case VPRORDZ256ri:
	case VPRORDZ256rik:
	case VPRORDZ256rikz:
	case VPRORDZmbi:
	case VPRORDZmbik:
	case VPRORDZmbikz:
	case VPRORDZmi:
	case VPRORDZmik:
	case VPRORDZmikz:
	case VPRORDZri:
	case VPRORDZrik:
	case VPRORDZrikz:
		return true;
	}
	return false;
}

bool isTEST(unsigned Opcode) {
	switch (Opcode) {
	case TEST16i16:
	case TEST16mi:
	case TEST16mr:
	case TEST16ri:
	case TEST16rr:
	case TEST32i32:
	case TEST32mi:
	case TEST32mr:
	case TEST32ri:
	case TEST32rr:
	case TEST64i32:
	case TEST64mi32:
	case TEST64mr:
	case TEST64ri32:
	case TEST64rr:
	case TEST8i8:
	case TEST8mi:
	case TEST8mr:
	case TEST8ri:
	case TEST8rr:
		return true;
	}
	return false;
}

bool isSHA1MSG1(unsigned Opcode) {
	switch (Opcode) {
	case SHA1MSG1rm:
	case SHA1MSG1rr:
		return true;
	}
	return false;
}

bool isSHA1MSG2(unsigned Opcode) {
	switch (Opcode) {
	case SHA1MSG2rm:
	case SHA1MSG2rr:
		return true;
	}
	return false;
}

bool isVFMULCSH(unsigned Opcode) {
	switch (Opcode) {
	case VFMULCSHZrm:
	case VFMULCSHZrmk:
	case VFMULCSHZrmkz:
	case VFMULCSHZrr:
	case VFMULCSHZrrb:
	case VFMULCSHZrrbk:
	case VFMULCSHZrrbkz:
	case VFMULCSHZrrk:
	case VFMULCSHZrrkz:
		return true;
	}
	return false;
}

bool isMOVNTDQA(unsigned Opcode) {
	return Opcode == MOVNTDQArm;
}

bool isVFIXUPIMMSS(unsigned Opcode) {
	switch (Opcode) {
	case VFIXUPIMMSSZrmi:
	case VFIXUPIMMSSZrmik:
	case VFIXUPIMMSSZrmikz:
	case VFIXUPIMMSSZrri:
	case VFIXUPIMMSSZrrib:
	case VFIXUPIMMSSZrribk:
	case VFIXUPIMMSSZrribkz:
	case VFIXUPIMMSSZrrik:
	case VFIXUPIMMSSZrrikz:
		return true;
	}
	return false;
}

bool isADOX(unsigned Opcode) {
	switch (Opcode) {
	case ADOX32rm:
	case ADOX32rr:
	case ADOX64rm:
	case ADOX64rr:
		return true;
	}
	return false;
}

bool isVPRORQ(unsigned Opcode) {
	switch (Opcode) {
	case VPRORQZ128mbi:
	case VPRORQZ128mbik:
	case VPRORQZ128mbikz:
	case VPRORQZ128mi:
	case VPRORQZ128mik:
	case VPRORQZ128mikz:
	case VPRORQZ128ri:
	case VPRORQZ128rik:
	case VPRORQZ128rikz:
	case VPRORQZ256mbi:
	case VPRORQZ256mbik:
	case VPRORQZ256mbikz:
	case VPRORQZ256mi:
	case VPRORQZ256mik:
	case VPRORQZ256mikz:
	case VPRORQZ256ri:
	case VPRORQZ256rik:
	case VPRORQZ256rikz:
	case VPRORQZmbi:
	case VPRORQZmbik:
	case VPRORQZmbikz:
	case VPRORQZmi:
	case VPRORQZmik:
	case VPRORQZmikz:
	case VPRORQZri:
	case VPRORQZrik:
	case VPRORQZrikz:
		return true;
	}
	return false;
}

bool isVSCATTERPF1DPD(unsigned Opcode) {
	return Opcode == VSCATTERPF1DPDm;
}

bool isVPSRLDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLDQYri:
	case VPSRLDQZ128mi:
	case VPSRLDQZ128ri:
	case VPSRLDQZ256mi:
	case VPSRLDQZ256ri:
	case VPSRLDQZmi:
	case VPSRLDQZri:
	case VPSRLDQri:
		return true;
	}
	return false;
}

bool isVPMOVUSWB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVUSWBZ128mr:
	case VPMOVUSWBZ128mrk:
	case VPMOVUSWBZ128rr:
	case VPMOVUSWBZ128rrk:
	case VPMOVUSWBZ128rrkz:
	case VPMOVUSWBZ256mr:
	case VPMOVUSWBZ256mrk:
	case VPMOVUSWBZ256rr:
	case VPMOVUSWBZ256rrk:
	case VPMOVUSWBZ256rrkz:
	case VPMOVUSWBZmr:
	case VPMOVUSWBZmrk:
	case VPMOVUSWBZrr:
	case VPMOVUSWBZrrk:
	case VPMOVUSWBZrrkz:
		return true;
	}
	return false;
}

bool isVSCATTERPF1DPS(unsigned Opcode) {
	return Opcode == VSCATTERPF1DPSm;
}

bool isFICOMP(unsigned Opcode) {
	switch (Opcode) {
	case FICOMP16m:
	case FICOMP32m:
		return true;
	}
	return false;
}

bool isFBSTP(unsigned Opcode) {
	return Opcode == FBSTPm;
}

bool isVPSHUFLW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHUFLWYmi:
	case VPSHUFLWYri:
	case VPSHUFLWZ128mi:
	case VPSHUFLWZ128mik:
	case VPSHUFLWZ128mikz:
	case VPSHUFLWZ128ri:
	case VPSHUFLWZ128rik:
	case VPSHUFLWZ128rikz:
	case VPSHUFLWZ256mi:
	case VPSHUFLWZ256mik:
	case VPSHUFLWZ256mikz:
	case VPSHUFLWZ256ri:
	case VPSHUFLWZ256rik:
	case VPSHUFLWZ256rikz:
	case VPSHUFLWZmi:
	case VPSHUFLWZmik:
	case VPSHUFLWZmikz:
	case VPSHUFLWZri:
	case VPSHUFLWZrik:
	case VPSHUFLWZrikz:
	case VPSHUFLWmi:
	case VPSHUFLWri:
		return true;
	}
	return false;
}

bool isVPSCATTERDD(unsigned Opcode) {
	switch (Opcode) {
	case VPSCATTERDDZ128mr:
	case VPSCATTERDDZ256mr:
	case VPSCATTERDDZmr:
		return true;
	}
	return false;
}

bool isFNINIT(unsigned Opcode) {
	return Opcode == FNINIT;
}

bool isMOVNTPD(unsigned Opcode) {
	return Opcode == MOVNTPDmr;
}

bool isUIRET(unsigned Opcode) {
	return Opcode == UIRET;
}

bool isPINSRB(unsigned Opcode) {
	switch (Opcode) {
	case PINSRBrm:
	case PINSRBrr:
		return true;
	}
	return false;
}

bool isPINSRD(unsigned Opcode) {
	switch (Opcode) {
	case PINSRDrm:
	case PINSRDrr:
		return true;
	}
	return false;
}

bool isSHRD(unsigned Opcode) {
	switch (Opcode) {
	case SHRD16mrCL:
	case SHRD16mri8:
	case SHRD16rrCL:
	case SHRD16rri8:
	case SHRD32mrCL:
	case SHRD32mri8:
	case SHRD32rrCL:
	case SHRD32rri8:
	case SHRD64mrCL:
	case SHRD64mri8:
	case SHRD64rrCL:
	case SHRD64rri8:
		return true;
	}
	return false;
}

bool isVPSCATTERDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSCATTERDQZ128mr:
	case VPSCATTERDQZ256mr:
	case VPSCATTERDQZmr:
		return true;
	}
	return false;
}

bool isMOVNTPS(unsigned Opcode) {
	return Opcode == MOVNTPSmr;
}

bool isVGETEXPPD(unsigned Opcode) {
	switch (Opcode) {
	case VGETEXPPDZ128m:
	case VGETEXPPDZ128mb:
	case VGETEXPPDZ128mbk:
	case VGETEXPPDZ128mbkz:
	case VGETEXPPDZ128mk:
	case VGETEXPPDZ128mkz:
	case VGETEXPPDZ128r:
	case VGETEXPPDZ128rk:
	case VGETEXPPDZ128rkz:
	case VGETEXPPDZ256m:
	case VGETEXPPDZ256mb:
	case VGETEXPPDZ256mbk:
	case VGETEXPPDZ256mbkz:
	case VGETEXPPDZ256mk:
	case VGETEXPPDZ256mkz:
	case VGETEXPPDZ256r:
	case VGETEXPPDZ256rk:
	case VGETEXPPDZ256rkz:
	case VGETEXPPDZm:
	case VGETEXPPDZmb:
	case VGETEXPPDZmbk:
	case VGETEXPPDZmbkz:
	case VGETEXPPDZmk:
	case VGETEXPPDZmkz:
	case VGETEXPPDZr:
	case VGETEXPPDZrb:
	case VGETEXPPDZrbk:
	case VGETEXPPDZrbkz:
	case VGETEXPPDZrk:
	case VGETEXPPDZrkz:
		return true;
	}
	return false;
}

bool isVRANGEPD(unsigned Opcode) {
	switch (Opcode) {
	case VRANGEPDZ128rmbi:
	case VRANGEPDZ128rmbik:
	case VRANGEPDZ128rmbikz:
	case VRANGEPDZ128rmi:
	case VRANGEPDZ128rmik:
	case VRANGEPDZ128rmikz:
	case VRANGEPDZ128rri:
	case VRANGEPDZ128rrik:
	case VRANGEPDZ128rrikz:
	case VRANGEPDZ256rmbi:
	case VRANGEPDZ256rmbik:
	case VRANGEPDZ256rmbikz:
	case VRANGEPDZ256rmi:
	case VRANGEPDZ256rmik:
	case VRANGEPDZ256rmikz:
	case VRANGEPDZ256rri:
	case VRANGEPDZ256rrik:
	case VRANGEPDZ256rrikz:
	case VRANGEPDZrmbi:
	case VRANGEPDZrmbik:
	case VRANGEPDZrmbikz:
	case VRANGEPDZrmi:
	case VRANGEPDZrmik:
	case VRANGEPDZrmikz:
	case VRANGEPDZrri:
	case VRANGEPDZrrib:
	case VRANGEPDZrribk:
	case VRANGEPDZrribkz:
	case VRANGEPDZrrik:
	case VRANGEPDZrrikz:
		return true;
	}
	return false;
}

bool isPFRCP(unsigned Opcode) {
	switch (Opcode) {
	case PFRCPrm:
	case PFRCPrr:
		return true;
	}
	return false;
}

bool isVGETEXPPH(unsigned Opcode) {
	switch (Opcode) {
	case VGETEXPPHZ128m:
	case VGETEXPPHZ128mb:
	case VGETEXPPHZ128mbk:
	case VGETEXPPHZ128mbkz:
	case VGETEXPPHZ128mk:
	case VGETEXPPHZ128mkz:
	case VGETEXPPHZ128r:
	case VGETEXPPHZ128rk:
	case VGETEXPPHZ128rkz:
	case VGETEXPPHZ256m:
	case VGETEXPPHZ256mb:
	case VGETEXPPHZ256mbk:
	case VGETEXPPHZ256mbkz:
	case VGETEXPPHZ256mk:
	case VGETEXPPHZ256mkz:
	case VGETEXPPHZ256r:
	case VGETEXPPHZ256rk:
	case VGETEXPPHZ256rkz:
	case VGETEXPPHZm:
	case VGETEXPPHZmb:
	case VGETEXPPHZmbk:
	case VGETEXPPHZmbkz:
	case VGETEXPPHZmk:
	case VGETEXPPHZmkz:
	case VGETEXPPHZr:
	case VGETEXPPHZrb:
	case VGETEXPPHZrbk:
	case VGETEXPPHZrbkz:
	case VGETEXPPHZrk:
	case VGETEXPPHZrkz:
		return true;
	}
	return false;
}

bool isPINSRQ(unsigned Opcode) {
	switch (Opcode) {
	case PINSRQrm:
	case PINSRQrr:
		return true;
	}
	return false;
}

bool isVPROTB(unsigned Opcode) {
	switch (Opcode) {
	case VPROTBmi:
	case VPROTBmr:
	case VPROTBri:
	case VPROTBrm:
	case VPROTBrr:
	case VPROTBrr_REV:
		return true;
	}
	return false;
}

bool isVPROTD(unsigned Opcode) {
	switch (Opcode) {
	case VPROTDmi:
	case VPROTDmr:
	case VPROTDri:
	case VPROTDrm:
	case VPROTDrr:
	case VPROTDrr_REV:
		return true;
	}
	return false;
}

bool isSEAMCALL(unsigned Opcode) {
	return Opcode == SEAMCALL;
}

bool isPINSRW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PINSRWrm:
	case MMX_PINSRWrr:
	case PINSRWrm:
	case PINSRWrr:
		return true;
	}
	return false;
}

bool isSHRX(unsigned Opcode) {
	switch (Opcode) {
	case SHRX32rm:
	case SHRX32rr:
	case SHRX64rm:
	case SHRX64rr:
		return true;
	}
	return false;
}

bool isVGETEXPPS(unsigned Opcode) {
	switch (Opcode) {
	case VGETEXPPSZ128m:
	case VGETEXPPSZ128mb:
	case VGETEXPPSZ128mbk:
	case VGETEXPPSZ128mbkz:
	case VGETEXPPSZ128mk:
	case VGETEXPPSZ128mkz:
	case VGETEXPPSZ128r:
	case VGETEXPPSZ128rk:
	case VGETEXPPSZ128rkz:
	case VGETEXPPSZ256m:
	case VGETEXPPSZ256mb:
	case VGETEXPPSZ256mbk:
	case VGETEXPPSZ256mbkz:
	case VGETEXPPSZ256mk:
	case VGETEXPPSZ256mkz:
	case VGETEXPPSZ256r:
	case VGETEXPPSZ256rk:
	case VGETEXPPSZ256rkz:
	case VGETEXPPSZm:
	case VGETEXPPSZmb:
	case VGETEXPPSZmbk:
	case VGETEXPPSZmbkz:
	case VGETEXPPSZmk:
	case VGETEXPPSZmkz:
	case VGETEXPPSZr:
	case VGETEXPPSZrb:
	case VGETEXPPSZrbk:
	case VGETEXPPSZrbkz:
	case VGETEXPPSZrk:
	case VGETEXPPSZrkz:
		return true;
	}
	return false;
}

bool isVRANGEPS(unsigned Opcode) {
	switch (Opcode) {
	case VRANGEPSZ128rmbi:
	case VRANGEPSZ128rmbik:
	case VRANGEPSZ128rmbikz:
	case VRANGEPSZ128rmi:
	case VRANGEPSZ128rmik:
	case VRANGEPSZ128rmikz:
	case VRANGEPSZ128rri:
	case VRANGEPSZ128rrik:
	case VRANGEPSZ128rrikz:
	case VRANGEPSZ256rmbi:
	case VRANGEPSZ256rmbik:
	case VRANGEPSZ256rmbikz:
	case VRANGEPSZ256rmi:
	case VRANGEPSZ256rmik:
	case VRANGEPSZ256rmikz:
	case VRANGEPSZ256rri:
	case VRANGEPSZ256rrik:
	case VRANGEPSZ256rrikz:
	case VRANGEPSZrmbi:
	case VRANGEPSZrmbik:
	case VRANGEPSZrmbikz:
	case VRANGEPSZrmi:
	case VRANGEPSZrmik:
	case VRANGEPSZrmikz:
	case VRANGEPSZrri:
	case VRANGEPSZrrib:
	case VRANGEPSZrribk:
	case VRANGEPSZrribkz:
	case VRANGEPSZrrik:
	case VRANGEPSZrrikz:
		return true;
	}
	return false;
}

bool isPABSB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PABSBrm:
	case MMX_PABSBrr:
	case PABSBrm:
	case PABSBrr:
		return true;
	}
	return false;
}

bool isPABSD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PABSDrm:
	case MMX_PABSDrr:
	case PABSDrm:
	case PABSDrr:
		return true;
	}
	return false;
}

bool isVPROTQ(unsigned Opcode) {
	switch (Opcode) {
	case VPROTQmi:
	case VPROTQmr:
	case VPROTQri:
	case VPROTQrm:
	case VPROTQrr:
	case VPROTQrr_REV:
		return true;
	}
	return false;
}

bool isVPROTW(unsigned Opcode) {
	switch (Opcode) {
	case VPROTWmi:
	case VPROTWmr:
	case VPROTWri:
	case VPROTWrm:
	case VPROTWrr:
	case VPROTWrr_REV:
		return true;
	}
	return false;
}

bool isVCVTTPS2UDQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPS2UDQZ128rm:
	case VCVTTPS2UDQZ128rmb:
	case VCVTTPS2UDQZ128rmbk:
	case VCVTTPS2UDQZ128rmbkz:
	case VCVTTPS2UDQZ128rmk:
	case VCVTTPS2UDQZ128rmkz:
	case VCVTTPS2UDQZ128rr:
	case VCVTTPS2UDQZ128rrk:
	case VCVTTPS2UDQZ128rrkz:
	case VCVTTPS2UDQZ256rm:
	case VCVTTPS2UDQZ256rmb:
	case VCVTTPS2UDQZ256rmbk:
	case VCVTTPS2UDQZ256rmbkz:
	case VCVTTPS2UDQZ256rmk:
	case VCVTTPS2UDQZ256rmkz:
	case VCVTTPS2UDQZ256rr:
	case VCVTTPS2UDQZ256rrk:
	case VCVTTPS2UDQZ256rrkz:
	case VCVTTPS2UDQZrm:
	case VCVTTPS2UDQZrmb:
	case VCVTTPS2UDQZrmbk:
	case VCVTTPS2UDQZrmbkz:
	case VCVTTPS2UDQZrmk:
	case VCVTTPS2UDQZrmkz:
	case VCVTTPS2UDQZrr:
	case VCVTTPS2UDQZrrb:
	case VCVTTPS2UDQZrrbk:
	case VCVTTPS2UDQZrrbkz:
	case VCVTTPS2UDQZrrk:
	case VCVTTPS2UDQZrrkz:
		return true;
	}
	return false;
}

bool isFXRSTOR(unsigned Opcode) {
	return Opcode == FXRSTOR;
}

bool isVMOVDQU16(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQU16Z128mr:
	case VMOVDQU16Z128mrk:
	case VMOVDQU16Z128rm:
	case VMOVDQU16Z128rmk:
	case VMOVDQU16Z128rmkz:
	case VMOVDQU16Z128rr:
	case VMOVDQU16Z128rr_REV:
	case VMOVDQU16Z128rrk:
	case VMOVDQU16Z128rrk_REV:
	case VMOVDQU16Z128rrkz:
	case VMOVDQU16Z128rrkz_REV:
	case VMOVDQU16Z256mr:
	case VMOVDQU16Z256mrk:
	case VMOVDQU16Z256rm:
	case VMOVDQU16Z256rmk:
	case VMOVDQU16Z256rmkz:
	case VMOVDQU16Z256rr:
	case VMOVDQU16Z256rr_REV:
	case VMOVDQU16Z256rrk:
	case VMOVDQU16Z256rrk_REV:
	case VMOVDQU16Z256rrkz:
	case VMOVDQU16Z256rrkz_REV:
	case VMOVDQU16Zmr:
	case VMOVDQU16Zmrk:
	case VMOVDQU16Zrm:
	case VMOVDQU16Zrmk:
	case VMOVDQU16Zrmkz:
	case VMOVDQU16Zrr:
	case VMOVDQU16Zrr_REV:
	case VMOVDQU16Zrrk:
	case VMOVDQU16Zrrk_REV:
	case VMOVDQU16Zrrkz:
	case VMOVDQU16Zrrkz_REV:
		return true;
	}
	return false;
}

bool isPABSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PABSWrm:
	case MMX_PABSWrr:
	case PABSWrm:
	case PABSWrr:
		return true;
	}
	return false;
}

bool isCVTDQ2PD(unsigned Opcode) {
	switch (Opcode) {
	case CVTDQ2PDrm:
	case CVTDQ2PDrr:
		return true;
	}
	return false;
}

bool isCVTDQ2PS(unsigned Opcode) {
	switch (Opcode) {
	case CVTDQ2PSrm:
	case CVTDQ2PSrr:
		return true;
	}
	return false;
}

bool isXOR(unsigned Opcode) {
	switch (Opcode) {
	case XOR16i16:
	case XOR16mi:
	case XOR16mi8:
	case XOR16mr:
	case XOR16ri:
	case XOR16ri8:
	case XOR16rm:
	case XOR16rr:
	case XOR16rr_REV:
	case XOR32i32:
	case XOR32mi:
	case XOR32mi8:
	case XOR32mr:
	case XOR32ri:
	case XOR32ri8:
	case XOR32rm:
	case XOR32rr:
	case XOR32rr_REV:
	case XOR64i32:
	case XOR64mi32:
	case XOR64mi8:
	case XOR64mr:
	case XOR64ri32:
	case XOR64ri8:
	case XOR64rm:
	case XOR64rr:
	case XOR64rr_REV:
	case XOR8i8:
	case XOR8mi:
	case XOR8mi8:
	case XOR8mr:
	case XOR8ri:
	case XOR8ri8:
	case XOR8rm:
	case XOR8rr:
	case XOR8rr_REV:
		return true;
	}
	return false;
}

bool isINC(unsigned Opcode) {
	switch (Opcode) {
	case INC16m:
	case INC16r:
	case INC16r_alt:
	case INC32m:
	case INC32r:
	case INC32r_alt:
	case INC64m:
	case INC64r:
	case INC8m:
	case INC8r:
		return true;
	}
	return false;
}

bool isVMCALL(unsigned Opcode) {
	return Opcode == VMCALL;
}

bool isPACKSSDW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PACKSSDWrm:
	case MMX_PACKSSDWrr:
	case PACKSSDWrm:
	case PACKSSDWrr:
		return true;
	}
	return false;
}

bool isPSUBUSB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBUSBrm:
	case MMX_PSUBUSBrr:
	case PSUBUSBrm:
	case PSUBUSBrr:
		return true;
	}
	return false;
}

bool isINT(unsigned Opcode) {
	return Opcode == INT;
}

bool isMOVNTSD(unsigned Opcode) {
	return Opcode == MOVNTSD;
}

bool isVPMOVZXBD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVZXBDYrm:
	case VPMOVZXBDYrr:
	case VPMOVZXBDZ128rm:
	case VPMOVZXBDZ128rmk:
	case VPMOVZXBDZ128rmkz:
	case VPMOVZXBDZ128rr:
	case VPMOVZXBDZ128rrk:
	case VPMOVZXBDZ128rrkz:
	case VPMOVZXBDZ256rm:
	case VPMOVZXBDZ256rmk:
	case VPMOVZXBDZ256rmkz:
	case VPMOVZXBDZ256rr:
	case VPMOVZXBDZ256rrk:
	case VPMOVZXBDZ256rrkz:
	case VPMOVZXBDZrm:
	case VPMOVZXBDZrmk:
	case VPMOVZXBDZrmkz:
	case VPMOVZXBDZrr:
	case VPMOVZXBDZrrk:
	case VPMOVZXBDZrrkz:
	case VPMOVZXBDrm:
	case VPMOVZXBDrr:
		return true;
	}
	return false;
}

bool isVCVTUDQ2PD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUDQ2PDZ128rm:
	case VCVTUDQ2PDZ128rmb:
	case VCVTUDQ2PDZ128rmbk:
	case VCVTUDQ2PDZ128rmbkz:
	case VCVTUDQ2PDZ128rmk:
	case VCVTUDQ2PDZ128rmkz:
	case VCVTUDQ2PDZ128rr:
	case VCVTUDQ2PDZ128rrk:
	case VCVTUDQ2PDZ128rrkz:
	case VCVTUDQ2PDZ256rm:
	case VCVTUDQ2PDZ256rmb:
	case VCVTUDQ2PDZ256rmbk:
	case VCVTUDQ2PDZ256rmbkz:
	case VCVTUDQ2PDZ256rmk:
	case VCVTUDQ2PDZ256rmkz:
	case VCVTUDQ2PDZ256rr:
	case VCVTUDQ2PDZ256rrk:
	case VCVTUDQ2PDZ256rrkz:
	case VCVTUDQ2PDZrm:
	case VCVTUDQ2PDZrmb:
	case VCVTUDQ2PDZrmbk:
	case VCVTUDQ2PDZrmbkz:
	case VCVTUDQ2PDZrmk:
	case VCVTUDQ2PDZrmkz:
	case VCVTUDQ2PDZrr:
	case VCVTUDQ2PDZrrk:
	case VCVTUDQ2PDZrrkz:
		return true;
	}
	return false;
}

bool isVCVTUDQ2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUDQ2PHZ128rm:
	case VCVTUDQ2PHZ128rmb:
	case VCVTUDQ2PHZ128rmbk:
	case VCVTUDQ2PHZ128rmbkz:
	case VCVTUDQ2PHZ128rmk:
	case VCVTUDQ2PHZ128rmkz:
	case VCVTUDQ2PHZ128rr:
	case VCVTUDQ2PHZ128rrk:
	case VCVTUDQ2PHZ128rrkz:
	case VCVTUDQ2PHZ256rm:
	case VCVTUDQ2PHZ256rmb:
	case VCVTUDQ2PHZ256rmbk:
	case VCVTUDQ2PHZ256rmbkz:
	case VCVTUDQ2PHZ256rmk:
	case VCVTUDQ2PHZ256rmkz:
	case VCVTUDQ2PHZ256rr:
	case VCVTUDQ2PHZ256rrk:
	case VCVTUDQ2PHZ256rrkz:
	case VCVTUDQ2PHZrm:
	case VCVTUDQ2PHZrmb:
	case VCVTUDQ2PHZrmbk:
	case VCVTUDQ2PHZrmbkz:
	case VCVTUDQ2PHZrmk:
	case VCVTUDQ2PHZrmkz:
	case VCVTUDQ2PHZrr:
	case VCVTUDQ2PHZrrb:
	case VCVTUDQ2PHZrrbk:
	case VCVTUDQ2PHZrrbkz:
	case VCVTUDQ2PHZrrk:
	case VCVTUDQ2PHZrrkz:
		return true;
	}
	return false;
}

bool isVMOVDQU32(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQU32Z128mr:
	case VMOVDQU32Z128mrk:
	case VMOVDQU32Z128rm:
	case VMOVDQU32Z128rmk:
	case VMOVDQU32Z128rmkz:
	case VMOVDQU32Z128rr:
	case VMOVDQU32Z128rr_REV:
	case VMOVDQU32Z128rrk:
	case VMOVDQU32Z128rrk_REV:
	case VMOVDQU32Z128rrkz:
	case VMOVDQU32Z128rrkz_REV:
	case VMOVDQU32Z256mr:
	case VMOVDQU32Z256mrk:
	case VMOVDQU32Z256rm:
	case VMOVDQU32Z256rmk:
	case VMOVDQU32Z256rmkz:
	case VMOVDQU32Z256rr:
	case VMOVDQU32Z256rr_REV:
	case VMOVDQU32Z256rrk:
	case VMOVDQU32Z256rrk_REV:
	case VMOVDQU32Z256rrkz:
	case VMOVDQU32Z256rrkz_REV:
	case VMOVDQU32Zmr:
	case VMOVDQU32Zmrk:
	case VMOVDQU32Zrm:
	case VMOVDQU32Zrmk:
	case VMOVDQU32Zrmkz:
	case VMOVDQU32Zrr:
	case VMOVDQU32Zrr_REV:
	case VMOVDQU32Zrrk:
	case VMOVDQU32Zrrk_REV:
	case VMOVDQU32Zrrkz:
	case VMOVDQU32Zrrkz_REV:
		return true;
	}
	return false;
}

bool isPSUBUSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBUSWrm:
	case MMX_PSUBUSWrr:
	case PSUBUSWrm:
	case PSUBUSWrr:
		return true;
	}
	return false;
}

bool isMOVNTSS(unsigned Opcode) {
	return Opcode == MOVNTSS;
}

bool isVGETEXPSD(unsigned Opcode) {
	switch (Opcode) {
	case VGETEXPSDZm:
	case VGETEXPSDZmk:
	case VGETEXPSDZmkz:
	case VGETEXPSDZr:
	case VGETEXPSDZrb:
	case VGETEXPSDZrbk:
	case VGETEXPSDZrbkz:
	case VGETEXPSDZrk:
	case VGETEXPSDZrkz:
		return true;
	}
	return false;
}

bool isVPMOVZXBQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVZXBQYrm:
	case VPMOVZXBQYrr:
	case VPMOVZXBQZ128rm:
	case VPMOVZXBQZ128rmk:
	case VPMOVZXBQZ128rmkz:
	case VPMOVZXBQZ128rr:
	case VPMOVZXBQZ128rrk:
	case VPMOVZXBQZ128rrkz:
	case VPMOVZXBQZ256rm:
	case VPMOVZXBQZ256rmk:
	case VPMOVZXBQZ256rmkz:
	case VPMOVZXBQZ256rr:
	case VPMOVZXBQZ256rrk:
	case VPMOVZXBQZ256rrkz:
	case VPMOVZXBQZrm:
	case VPMOVZXBQZrmk:
	case VPMOVZXBQZrmkz:
	case VPMOVZXBQZrr:
	case VPMOVZXBQZrrk:
	case VPMOVZXBQZrrkz:
	case VPMOVZXBQrm:
	case VPMOVZXBQrr:
		return true;
	}
	return false;
}

bool isVGETEXPSH(unsigned Opcode) {
	switch (Opcode) {
	case VGETEXPSHZm:
	case VGETEXPSHZmk:
	case VGETEXPSHZmkz:
	case VGETEXPSHZr:
	case VGETEXPSHZrb:
	case VGETEXPSHZrbk:
	case VGETEXPSHZrbkz:
	case VGETEXPSHZrk:
	case VGETEXPSHZrkz:
		return true;
	}
	return false;
}

bool isVCVTNEEBF162PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTNEEBF162PSYrm:
	case VCVTNEEBF162PSrm:
		return true;
	}
	return false;
}

bool isVRANGESD(unsigned Opcode) {
	switch (Opcode) {
	case VRANGESDZrmi:
	case VRANGESDZrmik:
	case VRANGESDZrmikz:
	case VRANGESDZrri:
	case VRANGESDZrrib:
	case VRANGESDZrribk:
	case VRANGESDZrribkz:
	case VRANGESDZrrik:
	case VRANGESDZrrikz:
		return true;
	}
	return false;
}

bool isVCVTUDQ2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUDQ2PSZ128rm:
	case VCVTUDQ2PSZ128rmb:
	case VCVTUDQ2PSZ128rmbk:
	case VCVTUDQ2PSZ128rmbkz:
	case VCVTUDQ2PSZ128rmk:
	case VCVTUDQ2PSZ128rmkz:
	case VCVTUDQ2PSZ128rr:
	case VCVTUDQ2PSZ128rrk:
	case VCVTUDQ2PSZ128rrkz:
	case VCVTUDQ2PSZ256rm:
	case VCVTUDQ2PSZ256rmb:
	case VCVTUDQ2PSZ256rmbk:
	case VCVTUDQ2PSZ256rmbkz:
	case VCVTUDQ2PSZ256rmk:
	case VCVTUDQ2PSZ256rmkz:
	case VCVTUDQ2PSZ256rr:
	case VCVTUDQ2PSZ256rrk:
	case VCVTUDQ2PSZ256rrkz:
	case VCVTUDQ2PSZrm:
	case VCVTUDQ2PSZrmb:
	case VCVTUDQ2PSZrmbk:
	case VCVTUDQ2PSZrmbkz:
	case VCVTUDQ2PSZrmk:
	case VCVTUDQ2PSZrmkz:
	case VCVTUDQ2PSZrr:
	case VCVTUDQ2PSZrrb:
	case VCVTUDQ2PSZrrbk:
	case VCVTUDQ2PSZrrbkz:
	case VCVTUDQ2PSZrrk:
	case VCVTUDQ2PSZrrkz:
		return true;
	}
	return false;
}

bool isVPMOVZXBW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVZXBWYrm:
	case VPMOVZXBWYrr:
	case VPMOVZXBWZ128rm:
	case VPMOVZXBWZ128rmk:
	case VPMOVZXBWZ128rmkz:
	case VPMOVZXBWZ128rr:
	case VPMOVZXBWZ128rrk:
	case VPMOVZXBWZ128rrkz:
	case VPMOVZXBWZ256rm:
	case VPMOVZXBWZ256rmk:
	case VPMOVZXBWZ256rmkz:
	case VPMOVZXBWZ256rr:
	case VPMOVZXBWZ256rrk:
	case VPMOVZXBWZ256rrkz:
	case VPMOVZXBWZrm:
	case VPMOVZXBWZrmk:
	case VPMOVZXBWZrmkz:
	case VPMOVZXBWZrr:
	case VPMOVZXBWZrrk:
	case VPMOVZXBWZrrkz:
	case VPMOVZXBWrm:
	case VPMOVZXBWrr:
		return true;
	}
	return false;
}

bool isVGETEXPSS(unsigned Opcode) {
	switch (Opcode) {
	case VGETEXPSSZm:
	case VGETEXPSSZmk:
	case VGETEXPSSZmkz:
	case VGETEXPSSZr:
	case VGETEXPSSZrb:
	case VGETEXPSSZrbk:
	case VGETEXPSSZrbkz:
	case VGETEXPSSZrk:
	case VGETEXPSSZrkz:
		return true;
	}
	return false;
}

bool isSHUFPD(unsigned Opcode) {
	switch (Opcode) {
	case SHUFPDrmi:
	case SHUFPDrri:
		return true;
	}
	return false;
}

bool isVCVTTSS2SI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTSS2SI64Zrm_Int:
	case VCVTTSS2SI64Zrr_Int:
	case VCVTTSS2SI64Zrrb_Int:
	case VCVTTSS2SI64rm_Int:
	case VCVTTSS2SI64rr_Int:
	case VCVTTSS2SIZrm_Int:
	case VCVTTSS2SIZrr_Int:
	case VCVTTSS2SIZrrb_Int:
	case VCVTTSS2SIrm_Int:
	case VCVTTSS2SIrr_Int:
		return true;
	}
	return false;
}

bool isVINSERTF128(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTF128rm:
	case VINSERTF128rr:
		return true;
	}
	return false;
}

bool isKANDNB(unsigned Opcode) {
	return Opcode == KANDNBrr;
}

bool isPADDSB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDSBrm:
	case MMX_PADDSBrr:
	case PADDSBrm:
	case PADDSBrr:
		return true;
	}
	return false;
}

bool isKANDND(unsigned Opcode) {
	return Opcode == KANDNDrr;
}

bool isVPCMPISTRI(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPISTRIrm:
	case VPCMPISTRIrr:
		return true;
	}
	return false;
}

bool isRDPID(unsigned Opcode) {
	switch (Opcode) {
	case RDPID32:
	case RDPID64:
		return true;
	}
	return false;
}

bool isVPCMPISTRM(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPISTRMrm:
	case VPCMPISTRMrr:
		return true;
	}
	return false;
}

bool isVRANGESS(unsigned Opcode) {
	switch (Opcode) {
	case VRANGESSZrmi:
	case VRANGESSZrmik:
	case VRANGESSZrmikz:
	case VRANGESSZrri:
	case VRANGESSZrrib:
	case VRANGESSZrribk:
	case VRANGESSZrribkz:
	case VRANGESSZrrik:
	case VRANGESSZrrikz:
		return true;
	}
	return false;
}

bool isSHUFPS(unsigned Opcode) {
	switch (Opcode) {
	case SHUFPSrmi:
	case SHUFPSrri:
		return true;
	}
	return false;
}

bool isVPDPBSSDS(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBSSDSYrm:
	case VPDPBSSDSYrr:
	case VPDPBSSDSrm:
	case VPDPBSSDSrr:
		return true;
	}
	return false;
}

bool isKANDNQ(unsigned Opcode) {
	return Opcode == KANDNQrr;
}

bool isKANDNW(unsigned Opcode) {
	return Opcode == KANDNWrr;
}

bool isPADDSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDSWrm:
	case MMX_PADDSWrr:
	case PADDSWrm:
	case PADDSWrr:
		return true;
	}
	return false;
}

bool isVCVTTPD2UDQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPD2UDQZ128rm:
	case VCVTTPD2UDQZ128rmb:
	case VCVTTPD2UDQZ128rmbk:
	case VCVTTPD2UDQZ128rmbkz:
	case VCVTTPD2UDQZ128rmk:
	case VCVTTPD2UDQZ128rmkz:
	case VCVTTPD2UDQZ128rr:
	case VCVTTPD2UDQZ128rrk:
	case VCVTTPD2UDQZ128rrkz:
	case VCVTTPD2UDQZ256rm:
	case VCVTTPD2UDQZ256rmb:
	case VCVTTPD2UDQZ256rmbk:
	case VCVTTPD2UDQZ256rmbkz:
	case VCVTTPD2UDQZ256rmk:
	case VCVTTPD2UDQZ256rmkz:
	case VCVTTPD2UDQZ256rr:
	case VCVTTPD2UDQZ256rrk:
	case VCVTTPD2UDQZ256rrkz:
	case VCVTTPD2UDQZrm:
	case VCVTTPD2UDQZrmb:
	case VCVTTPD2UDQZrmbk:
	case VCVTTPD2UDQZrmbkz:
	case VCVTTPD2UDQZrmk:
	case VCVTTPD2UDQZrmkz:
	case VCVTTPD2UDQZrr:
	case VCVTTPD2UDQZrrb:
	case VCVTTPD2UDQZrrbk:
	case VCVTTPD2UDQZrrbkz:
	case VCVTTPD2UDQZrrk:
	case VCVTTPD2UDQZrrkz:
		return true;
	}
	return false;
}

bool isVANDNPD(unsigned Opcode) {
	switch (Opcode) {
	case VANDNPDYrm:
	case VANDNPDYrr:
	case VANDNPDZ128rm:
	case VANDNPDZ128rmb:
	case VANDNPDZ128rmbk:
	case VANDNPDZ128rmbkz:
	case VANDNPDZ128rmk:
	case VANDNPDZ128rmkz:
	case VANDNPDZ128rr:
	case VANDNPDZ128rrk:
	case VANDNPDZ128rrkz:
	case VANDNPDZ256rm:
	case VANDNPDZ256rmb:
	case VANDNPDZ256rmbk:
	case VANDNPDZ256rmbkz:
	case VANDNPDZ256rmk:
	case VANDNPDZ256rmkz:
	case VANDNPDZ256rr:
	case VANDNPDZ256rrk:
	case VANDNPDZ256rrkz:
	case VANDNPDZrm:
	case VANDNPDZrmb:
	case VANDNPDZrmbk:
	case VANDNPDZrmbkz:
	case VANDNPDZrmk:
	case VANDNPDZrmkz:
	case VANDNPDZrr:
	case VANDNPDZrrk:
	case VANDNPDZrrkz:
	case VANDNPDrm:
	case VANDNPDrr:
		return true;
	}
	return false;
}

bool isXEND(unsigned Opcode) {
	return Opcode == XEND;
}

bool isVUNPCKHPD(unsigned Opcode) {
	switch (Opcode) {
	case VUNPCKHPDYrm:
	case VUNPCKHPDYrr:
	case VUNPCKHPDZ128rm:
	case VUNPCKHPDZ128rmb:
	case VUNPCKHPDZ128rmbk:
	case VUNPCKHPDZ128rmbkz:
	case VUNPCKHPDZ128rmk:
	case VUNPCKHPDZ128rmkz:
	case VUNPCKHPDZ128rr:
	case VUNPCKHPDZ128rrk:
	case VUNPCKHPDZ128rrkz:
	case VUNPCKHPDZ256rm:
	case VUNPCKHPDZ256rmb:
	case VUNPCKHPDZ256rmbk:
	case VUNPCKHPDZ256rmbkz:
	case VUNPCKHPDZ256rmk:
	case VUNPCKHPDZ256rmkz:
	case VUNPCKHPDZ256rr:
	case VUNPCKHPDZ256rrk:
	case VUNPCKHPDZ256rrkz:
	case VUNPCKHPDZrm:
	case VUNPCKHPDZrmb:
	case VUNPCKHPDZrmbk:
	case VUNPCKHPDZrmbkz:
	case VUNPCKHPDZrmk:
	case VUNPCKHPDZrmkz:
	case VUNPCKHPDZrr:
	case VUNPCKHPDZrrk:
	case VUNPCKHPDZrrkz:
	case VUNPCKHPDrm:
	case VUNPCKHPDrr:
		return true;
	}
	return false;
}

bool isPSRAD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSRADri:
	case MMX_PSRADrm:
	case MMX_PSRADrr:
	case PSRADri:
	case PSRADrm:
	case PSRADrr:
		return true;
	}
	return false;
}

bool isVANDNPS(unsigned Opcode) {
	switch (Opcode) {
	case VANDNPSYrm:
	case VANDNPSYrr:
	case VANDNPSZ128rm:
	case VANDNPSZ128rmb:
	case VANDNPSZ128rmbk:
	case VANDNPSZ128rmbkz:
	case VANDNPSZ128rmk:
	case VANDNPSZ128rmkz:
	case VANDNPSZ128rr:
	case VANDNPSZ128rrk:
	case VANDNPSZ128rrkz:
	case VANDNPSZ256rm:
	case VANDNPSZ256rmb:
	case VANDNPSZ256rmbk:
	case VANDNPSZ256rmbkz:
	case VANDNPSZ256rmk:
	case VANDNPSZ256rmkz:
	case VANDNPSZ256rr:
	case VANDNPSZ256rrk:
	case VANDNPSZ256rrkz:
	case VANDNPSZrm:
	case VANDNPSZrmb:
	case VANDNPSZrmbk:
	case VANDNPSZrmbkz:
	case VANDNPSZrmk:
	case VANDNPSZrmkz:
	case VANDNPSZrr:
	case VANDNPSZrrk:
	case VANDNPSZrrkz:
	case VANDNPSrm:
	case VANDNPSrr:
		return true;
	}
	return false;
}

bool isVPMOVZXDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVZXDQYrm:
	case VPMOVZXDQYrr:
	case VPMOVZXDQZ128rm:
	case VPMOVZXDQZ128rmk:
	case VPMOVZXDQZ128rmkz:
	case VPMOVZXDQZ128rr:
	case VPMOVZXDQZ128rrk:
	case VPMOVZXDQZ128rrkz:
	case VPMOVZXDQZ256rm:
	case VPMOVZXDQZ256rmk:
	case VPMOVZXDQZ256rmkz:
	case VPMOVZXDQZ256rr:
	case VPMOVZXDQZ256rrk:
	case VPMOVZXDQZ256rrkz:
	case VPMOVZXDQZrm:
	case VPMOVZXDQZrmk:
	case VPMOVZXDQZrmkz:
	case VPMOVZXDQZrr:
	case VPMOVZXDQZrrk:
	case VPMOVZXDQZrrkz:
	case VPMOVZXDQrm:
	case VPMOVZXDQrr:
		return true;
	}
	return false;
}

bool isVMPTRST(unsigned Opcode) {
	return Opcode == VMPTRSTm;
}

bool isVUNPCKHPS(unsigned Opcode) {
	switch (Opcode) {
	case VUNPCKHPSYrm:
	case VUNPCKHPSYrr:
	case VUNPCKHPSZ128rm:
	case VUNPCKHPSZ128rmb:
	case VUNPCKHPSZ128rmbk:
	case VUNPCKHPSZ128rmbkz:
	case VUNPCKHPSZ128rmk:
	case VUNPCKHPSZ128rmkz:
	case VUNPCKHPSZ128rr:
	case VUNPCKHPSZ128rrk:
	case VUNPCKHPSZ128rrkz:
	case VUNPCKHPSZ256rm:
	case VUNPCKHPSZ256rmb:
	case VUNPCKHPSZ256rmbk:
	case VUNPCKHPSZ256rmbkz:
	case VUNPCKHPSZ256rmk:
	case VUNPCKHPSZ256rmkz:
	case VUNPCKHPSZ256rr:
	case VUNPCKHPSZ256rrk:
	case VUNPCKHPSZ256rrkz:
	case VUNPCKHPSZrm:
	case VUNPCKHPSZrmb:
	case VUNPCKHPSZrmbk:
	case VUNPCKHPSZrmbkz:
	case VUNPCKHPSZrmk:
	case VUNPCKHPSZrmkz:
	case VUNPCKHPSZrr:
	case VUNPCKHPSZrrk:
	case VUNPCKHPSZrrkz:
	case VUNPCKHPSrm:
	case VUNPCKHPSrr:
		return true;
	}
	return false;
}

bool isPSRAW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSRAWri:
	case MMX_PSRAWrm:
	case MMX_PSRAWrr:
	case PSRAWri:
	case PSRAWrm:
	case PSRAWrr:
		return true;
	}
	return false;
}

bool isVPLZCNTD(unsigned Opcode) {
	switch (Opcode) {
	case VPLZCNTDZ128rm:
	case VPLZCNTDZ128rmb:
	case VPLZCNTDZ128rmbk:
	case VPLZCNTDZ128rmbkz:
	case VPLZCNTDZ128rmk:
	case VPLZCNTDZ128rmkz:
	case VPLZCNTDZ128rr:
	case VPLZCNTDZ128rrk:
	case VPLZCNTDZ128rrkz:
	case VPLZCNTDZ256rm:
	case VPLZCNTDZ256rmb:
	case VPLZCNTDZ256rmbk:
	case VPLZCNTDZ256rmbkz:
	case VPLZCNTDZ256rmk:
	case VPLZCNTDZ256rmkz:
	case VPLZCNTDZ256rr:
	case VPLZCNTDZ256rrk:
	case VPLZCNTDZ256rrkz:
	case VPLZCNTDZrm:
	case VPLZCNTDZrmb:
	case VPLZCNTDZrmbk:
	case VPLZCNTDZrmbkz:
	case VPLZCNTDZrmk:
	case VPLZCNTDZrmkz:
	case VPLZCNTDZrr:
	case VPLZCNTDZrrk:
	case VPLZCNTDZrrkz:
		return true;
	}
	return false;
}

bool isWRMSRLIST(unsigned Opcode) {
	return Opcode == WRMSRLIST;
}

bool isVPADDUSB(unsigned Opcode) {
	switch (Opcode) {
	case VPADDUSBYrm:
	case VPADDUSBYrr:
	case VPADDUSBZ128rm:
	case VPADDUSBZ128rmk:
	case VPADDUSBZ128rmkz:
	case VPADDUSBZ128rr:
	case VPADDUSBZ128rrk:
	case VPADDUSBZ128rrkz:
	case VPADDUSBZ256rm:
	case VPADDUSBZ256rmk:
	case VPADDUSBZ256rmkz:
	case VPADDUSBZ256rr:
	case VPADDUSBZ256rrk:
	case VPADDUSBZ256rrkz:
	case VPADDUSBZrm:
	case VPADDUSBZrmk:
	case VPADDUSBZrmkz:
	case VPADDUSBZrr:
	case VPADDUSBZrrk:
	case VPADDUSBZrrkz:
	case VPADDUSBrm:
	case VPADDUSBrr:
		return true;
	}
	return false;
}

bool isVMOVDQU64(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQU64Z128mr:
	case VMOVDQU64Z128mrk:
	case VMOVDQU64Z128rm:
	case VMOVDQU64Z128rmk:
	case VMOVDQU64Z128rmkz:
	case VMOVDQU64Z128rr:
	case VMOVDQU64Z128rr_REV:
	case VMOVDQU64Z128rrk:
	case VMOVDQU64Z128rrk_REV:
	case VMOVDQU64Z128rrkz:
	case VMOVDQU64Z128rrkz_REV:
	case VMOVDQU64Z256mr:
	case VMOVDQU64Z256mrk:
	case VMOVDQU64Z256rm:
	case VMOVDQU64Z256rmk:
	case VMOVDQU64Z256rmkz:
	case VMOVDQU64Z256rr:
	case VMOVDQU64Z256rr_REV:
	case VMOVDQU64Z256rrk:
	case VMOVDQU64Z256rrk_REV:
	case VMOVDQU64Z256rrkz:
	case VMOVDQU64Z256rrkz_REV:
	case VMOVDQU64Zmr:
	case VMOVDQU64Zmrk:
	case VMOVDQU64Zrm:
	case VMOVDQU64Zrmk:
	case VMOVDQU64Zrmkz:
	case VMOVDQU64Zrr:
	case VMOVDQU64Zrr_REV:
	case VMOVDQU64Zrrk:
	case VMOVDQU64Zrrk_REV:
	case VMOVDQU64Zrrkz:
	case VMOVDQU64Zrrkz_REV:
		return true;
	}
	return false;
}

bool isSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case SUBPDrm:
	case SUBPDrr:
		return true;
	}
	return false;
}

bool isMOVDDUP(unsigned Opcode) {
	switch (Opcode) {
	case MOVDDUPrm:
	case MOVDDUPrr:
		return true;
	}
	return false;
}

bool isVPLZCNTQ(unsigned Opcode) {
	switch (Opcode) {
	case VPLZCNTQZ128rm:
	case VPLZCNTQZ128rmb:
	case VPLZCNTQZ128rmbk:
	case VPLZCNTQZ128rmbkz:
	case VPLZCNTQZ128rmk:
	case VPLZCNTQZ128rmkz:
	case VPLZCNTQZ128rr:
	case VPLZCNTQZ128rrk:
	case VPLZCNTQZ128rrkz:
	case VPLZCNTQZ256rm:
	case VPLZCNTQZ256rmb:
	case VPLZCNTQZ256rmbk:
	case VPLZCNTQZ256rmbkz:
	case VPLZCNTQZ256rmk:
	case VPLZCNTQZ256rmkz:
	case VPLZCNTQZ256rr:
	case VPLZCNTQZ256rrk:
	case VPLZCNTQZ256rrkz:
	case VPLZCNTQZrm:
	case VPLZCNTQZrmb:
	case VPLZCNTQZrmbk:
	case VPLZCNTQZrmbkz:
	case VPLZCNTQZrmk:
	case VPLZCNTQZrmkz:
	case VPLZCNTQZrr:
	case VPLZCNTQZrrk:
	case VPLZCNTQZrrkz:
		return true;
	}
	return false;
}

bool isSTAC(unsigned Opcode) {
	return Opcode == STAC;
}

bool isSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case SUBPSrm:
	case SUBPSrr:
		return true;
	}
	return false;
}

bool isVPADDUSW(unsigned Opcode) {
	switch (Opcode) {
	case VPADDUSWYrm:
	case VPADDUSWYrr:
	case VPADDUSWZ128rm:
	case VPADDUSWZ128rmk:
	case VPADDUSWZ128rmkz:
	case VPADDUSWZ128rr:
	case VPADDUSWZ128rrk:
	case VPADDUSWZ128rrkz:
	case VPADDUSWZ256rm:
	case VPADDUSWZ256rmk:
	case VPADDUSWZ256rmkz:
	case VPADDUSWZ256rr:
	case VPADDUSWZ256rrk:
	case VPADDUSWZ256rrkz:
	case VPADDUSWZrm:
	case VPADDUSWZrmk:
	case VPADDUSWZrmkz:
	case VPADDUSWZrr:
	case VPADDUSWZrrk:
	case VPADDUSWZrrkz:
	case VPADDUSWrm:
	case VPADDUSWrr:
		return true;
	}
	return false;
}

bool isFCMOVNBE(unsigned Opcode) {
	return Opcode == CMOVNBE_F;
}

bool isSHA1RNDS4(unsigned Opcode) {
	switch (Opcode) {
	case SHA1RNDS4rmi:
	case SHA1RNDS4rri:
		return true;
	}
	return false;
}

bool isPAUSE(unsigned Opcode) {
	return Opcode == PAUSE;
}

bool isSAHF(unsigned Opcode) {
	return Opcode == SAHF;
}

bool isVREDUCEPD(unsigned Opcode) {
	switch (Opcode) {
	case VREDUCEPDZ128rmbi:
	case VREDUCEPDZ128rmbik:
	case VREDUCEPDZ128rmbikz:
	case VREDUCEPDZ128rmi:
	case VREDUCEPDZ128rmik:
	case VREDUCEPDZ128rmikz:
	case VREDUCEPDZ128rri:
	case VREDUCEPDZ128rrik:
	case VREDUCEPDZ128rrikz:
	case VREDUCEPDZ256rmbi:
	case VREDUCEPDZ256rmbik:
	case VREDUCEPDZ256rmbikz:
	case VREDUCEPDZ256rmi:
	case VREDUCEPDZ256rmik:
	case VREDUCEPDZ256rmikz:
	case VREDUCEPDZ256rri:
	case VREDUCEPDZ256rrik:
	case VREDUCEPDZ256rrikz:
	case VREDUCEPDZrmbi:
	case VREDUCEPDZrmbik:
	case VREDUCEPDZrmbikz:
	case VREDUCEPDZrmi:
	case VREDUCEPDZrmik:
	case VREDUCEPDZrmikz:
	case VREDUCEPDZrri:
	case VREDUCEPDZrrib:
	case VREDUCEPDZrribk:
	case VREDUCEPDZrribkz:
	case VREDUCEPDZrrik:
	case VREDUCEPDZrrikz:
		return true;
	}
	return false;
}

bool isVREDUCEPH(unsigned Opcode) {
	switch (Opcode) {
	case VREDUCEPHZ128rmbi:
	case VREDUCEPHZ128rmbik:
	case VREDUCEPHZ128rmbikz:
	case VREDUCEPHZ128rmi:
	case VREDUCEPHZ128rmik:
	case VREDUCEPHZ128rmikz:
	case VREDUCEPHZ128rri:
	case VREDUCEPHZ128rrik:
	case VREDUCEPHZ128rrikz:
	case VREDUCEPHZ256rmbi:
	case VREDUCEPHZ256rmbik:
	case VREDUCEPHZ256rmbikz:
	case VREDUCEPHZ256rmi:
	case VREDUCEPHZ256rmik:
	case VREDUCEPHZ256rmikz:
	case VREDUCEPHZ256rri:
	case VREDUCEPHZ256rrik:
	case VREDUCEPHZ256rrikz:
	case VREDUCEPHZrmbi:
	case VREDUCEPHZrmbik:
	case VREDUCEPHZrmbikz:
	case VREDUCEPHZrmi:
	case VREDUCEPHZrmik:
	case VREDUCEPHZrmikz:
	case VREDUCEPHZrri:
	case VREDUCEPHZrrib:
	case VREDUCEPHZrribk:
	case VREDUCEPHZrribkz:
	case VREDUCEPHZrrik:
	case VREDUCEPHZrrikz:
		return true;
	}
	return false;
}

bool isFXAM(unsigned Opcode) {
	return Opcode == XAM_F;
}

bool isLGDTD(unsigned Opcode) {
	return Opcode == LGDT32m;
}

bool isPMULHRW(unsigned Opcode) {
	switch (Opcode) {
	case PMULHRWrm:
	case PMULHRWrr:
		return true;
	}
	return false;
}

bool isVREDUCEPS(unsigned Opcode) {
	switch (Opcode) {
	case VREDUCEPSZ128rmbi:
	case VREDUCEPSZ128rmbik:
	case VREDUCEPSZ128rmbikz:
	case VREDUCEPSZ128rmi:
	case VREDUCEPSZ128rmik:
	case VREDUCEPSZ128rmikz:
	case VREDUCEPSZ128rri:
	case VREDUCEPSZ128rrik:
	case VREDUCEPSZ128rrikz:
	case VREDUCEPSZ256rmbi:
	case VREDUCEPSZ256rmbik:
	case VREDUCEPSZ256rmbikz:
	case VREDUCEPSZ256rmi:
	case VREDUCEPSZ256rmik:
	case VREDUCEPSZ256rmikz:
	case VREDUCEPSZ256rri:
	case VREDUCEPSZ256rrik:
	case VREDUCEPSZ256rrikz:
	case VREDUCEPSZrmbi:
	case VREDUCEPSZrmbik:
	case VREDUCEPSZrmbikz:
	case VREDUCEPSZrmi:
	case VREDUCEPSZrmik:
	case VREDUCEPSZrmikz:
	case VREDUCEPSZrri:
	case VREDUCEPSZrrib:
	case VREDUCEPSZrribk:
	case VREDUCEPSZrribkz:
	case VREDUCEPSZrrik:
	case VREDUCEPSZrrikz:
		return true;
	}
	return false;
}

bool isRDPMC(unsigned Opcode) {
	return Opcode == RDPMC;
}

bool isVGATHERPF1QPD(unsigned Opcode) {
	return Opcode == VGATHERPF1QPDm;
}

bool isLGDTW(unsigned Opcode) {
	return Opcode == LGDT16m;
}

bool isVAESKEYGENASSIST(unsigned Opcode) {
	switch (Opcode) {
	case VAESKEYGENASSIST128rm:
	case VAESKEYGENASSIST128rr:
		return true;
	}
	return false;
}

bool isRDFSBASE(unsigned Opcode) {
	switch (Opcode) {
	case RDFSBASE:
	case RDFSBASE64:
		return true;
	}
	return false;
}

bool isVGATHERPF1QPS(unsigned Opcode) {
	return Opcode == VGATHERPF1QPSm;
}

bool isSUBSD(unsigned Opcode) {
	switch (Opcode) {
	case SUBSDrm_Int:
	case SUBSDrr_Int:
		return true;
	}
	return false;
}

bool isVCVTNEOBF162PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTNEOBF162PSYrm:
	case VCVTNEOBF162PSrm:
		return true;
	}
	return false;
}

bool isAESENCWIDE128KL(unsigned Opcode) {
	return Opcode == AESENCWIDE128KL;
}

bool isFXCH(unsigned Opcode) {
	return Opcode == XCH_F;
}

bool isSUBSS(unsigned Opcode) {
	switch (Opcode) {
	case SUBSSrm_Int:
	case SUBSSrr_Int:
		return true;
	}
	return false;
}

bool isPANDN(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PANDNrm:
	case MMX_PANDNrr:
	case PANDNrm:
	case PANDNrr:
		return true;
	}
	return false;
}

bool isVPERMT2B(unsigned Opcode) {
	switch (Opcode) {
	case VPERMT2B128rm:
	case VPERMT2B128rmk:
	case VPERMT2B128rmkz:
	case VPERMT2B128rr:
	case VPERMT2B128rrk:
	case VPERMT2B128rrkz:
	case VPERMT2B256rm:
	case VPERMT2B256rmk:
	case VPERMT2B256rmkz:
	case VPERMT2B256rr:
	case VPERMT2B256rrk:
	case VPERMT2B256rrkz:
	case VPERMT2Brm:
	case VPERMT2Brmk:
	case VPERMT2Brmkz:
	case VPERMT2Brr:
	case VPERMT2Brrk:
	case VPERMT2Brrkz:
		return true;
	}
	return false;
}

bool isLJMP(unsigned Opcode) {
	switch (Opcode) {
	case FARJMP16i:
	case FARJMP16m:
	case FARJMP32i:
	case FARJMP64m:
		return true;
	}
	return false;
}

bool isCMPPD(unsigned Opcode) {
	switch (Opcode) {
	case CMPPDrmi:
	case CMPPDrri:
		return true;
	}
	return false;
}

bool isVPERMT2D(unsigned Opcode) {
	switch (Opcode) {
	case VPERMT2D128rm:
	case VPERMT2D128rmb:
	case VPERMT2D128rmbk:
	case VPERMT2D128rmbkz:
	case VPERMT2D128rmk:
	case VPERMT2D128rmkz:
	case VPERMT2D128rr:
	case VPERMT2D128rrk:
	case VPERMT2D128rrkz:
	case VPERMT2D256rm:
	case VPERMT2D256rmb:
	case VPERMT2D256rmbk:
	case VPERMT2D256rmbkz:
	case VPERMT2D256rmk:
	case VPERMT2D256rmkz:
	case VPERMT2D256rr:
	case VPERMT2D256rrk:
	case VPERMT2D256rrkz:
	case VPERMT2Drm:
	case VPERMT2Drmb:
	case VPERMT2Drmbk:
	case VPERMT2Drmbkz:
	case VPERMT2Drmk:
	case VPERMT2Drmkz:
	case VPERMT2Drr:
	case VPERMT2Drrk:
	case VPERMT2Drrkz:
		return true;
	}
	return false;
}

bool isVPMADD52HUQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMADD52HUQYrm:
	case VPMADD52HUQYrr:
	case VPMADD52HUQZ128m:
	case VPMADD52HUQZ128mb:
	case VPMADD52HUQZ128mbk:
	case VPMADD52HUQZ128mbkz:
	case VPMADD52HUQZ128mk:
	case VPMADD52HUQZ128mkz:
	case VPMADD52HUQZ128r:
	case VPMADD52HUQZ128rk:
	case VPMADD52HUQZ128rkz:
	case VPMADD52HUQZ256m:
	case VPMADD52HUQZ256mb:
	case VPMADD52HUQZ256mbk:
	case VPMADD52HUQZ256mbkz:
	case VPMADD52HUQZ256mk:
	case VPMADD52HUQZ256mkz:
	case VPMADD52HUQZ256r:
	case VPMADD52HUQZ256rk:
	case VPMADD52HUQZ256rkz:
	case VPMADD52HUQZm:
	case VPMADD52HUQZmb:
	case VPMADD52HUQZmbk:
	case VPMADD52HUQZmbkz:
	case VPMADD52HUQZmk:
	case VPMADD52HUQZmkz:
	case VPMADD52HUQZr:
	case VPMADD52HUQZrk:
	case VPMADD52HUQZrkz:
	case VPMADD52HUQrm:
	case VPMADD52HUQrr:
		return true;
	}
	return false;
}

bool isVPERMT2Q(unsigned Opcode) {
	switch (Opcode) {
	case VPERMT2Q128rm:
	case VPERMT2Q128rmb:
	case VPERMT2Q128rmbk:
	case VPERMT2Q128rmbkz:
	case VPERMT2Q128rmk:
	case VPERMT2Q128rmkz:
	case VPERMT2Q128rr:
	case VPERMT2Q128rrk:
	case VPERMT2Q128rrkz:
	case VPERMT2Q256rm:
	case VPERMT2Q256rmb:
	case VPERMT2Q256rmbk:
	case VPERMT2Q256rmbkz:
	case VPERMT2Q256rmk:
	case VPERMT2Q256rmkz:
	case VPERMT2Q256rr:
	case VPERMT2Q256rrk:
	case VPERMT2Q256rrkz:
	case VPERMT2Qrm:
	case VPERMT2Qrmb:
	case VPERMT2Qrmbk:
	case VPERMT2Qrmbkz:
	case VPERMT2Qrmk:
	case VPERMT2Qrmkz:
	case VPERMT2Qrr:
	case VPERMT2Qrrk:
	case VPERMT2Qrrkz:
		return true;
	}
	return false;
}

bool isCMPPS(unsigned Opcode) {
	switch (Opcode) {
	case CMPPSrmi:
	case CMPPSrri:
		return true;
	}
	return false;
}

bool isVCVTPH2W(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2WZ128rm:
	case VCVTPH2WZ128rmb:
	case VCVTPH2WZ128rmbk:
	case VCVTPH2WZ128rmbkz:
	case VCVTPH2WZ128rmk:
	case VCVTPH2WZ128rmkz:
	case VCVTPH2WZ128rr:
	case VCVTPH2WZ128rrk:
	case VCVTPH2WZ128rrkz:
	case VCVTPH2WZ256rm:
	case VCVTPH2WZ256rmb:
	case VCVTPH2WZ256rmbk:
	case VCVTPH2WZ256rmbkz:
	case VCVTPH2WZ256rmk:
	case VCVTPH2WZ256rmkz:
	case VCVTPH2WZ256rr:
	case VCVTPH2WZ256rrk:
	case VCVTPH2WZ256rrkz:
	case VCVTPH2WZrm:
	case VCVTPH2WZrmb:
	case VCVTPH2WZrmbk:
	case VCVTPH2WZrmbkz:
	case VCVTPH2WZrmk:
	case VCVTPH2WZrmkz:
	case VCVTPH2WZrr:
	case VCVTPH2WZrrb:
	case VCVTPH2WZrrbk:
	case VCVTPH2WZrrbkz:
	case VCVTPH2WZrrk:
	case VCVTPH2WZrrkz:
		return true;
	}
	return false;
}

bool isVREDUCESD(unsigned Opcode) {
	switch (Opcode) {
	case VREDUCESDZrmi:
	case VREDUCESDZrmik:
	case VREDUCESDZrmikz:
	case VREDUCESDZrri:
	case VREDUCESDZrrib:
	case VREDUCESDZrribk:
	case VREDUCESDZrribkz:
	case VREDUCESDZrrik:
	case VREDUCESDZrrikz:
		return true;
	}
	return false;
}

bool isVPERMT2W(unsigned Opcode) {
	switch (Opcode) {
	case VPERMT2W128rm:
	case VPERMT2W128rmk:
	case VPERMT2W128rmkz:
	case VPERMT2W128rr:
	case VPERMT2W128rrk:
	case VPERMT2W128rrkz:
	case VPERMT2W256rm:
	case VPERMT2W256rmk:
	case VPERMT2W256rmkz:
	case VPERMT2W256rr:
	case VPERMT2W256rrk:
	case VPERMT2W256rrkz:
	case VPERMT2Wrm:
	case VPERMT2Wrmk:
	case VPERMT2Wrmkz:
	case VPERMT2Wrr:
	case VPERMT2Wrrk:
	case VPERMT2Wrrkz:
		return true;
	}
	return false;
}

bool isWBNOINVD(unsigned Opcode) {
	return Opcode == WBNOINVD;
}

bool isVREDUCESH(unsigned Opcode) {
	switch (Opcode) {
	case VREDUCESHZrmi:
	case VREDUCESHZrmik:
	case VREDUCESHZrmikz:
	case VREDUCESHZrri:
	case VREDUCESHZrrib:
	case VREDUCESHZrribk:
	case VREDUCESHZrribkz:
	case VREDUCESHZrrik:
	case VREDUCESHZrrikz:
		return true;
	}
	return false;
}

bool isTILEZERO(unsigned Opcode) {
	return Opcode == TILEZERO;
}

bool isPMULHUW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMULHUWrm:
	case MMX_PMULHUWrr:
	case PMULHUWrm:
	case PMULHUWrr:
		return true;
	}
	return false;
}

bool isVREDUCESS(unsigned Opcode) {
	switch (Opcode) {
	case VREDUCESSZrmi:
	case VREDUCESSZrmik:
	case VREDUCESSZrmikz:
	case VREDUCESSZrri:
	case VREDUCESSZrrib:
	case VREDUCESSZrribk:
	case VREDUCESSZrribkz:
	case VREDUCESSZrrik:
	case VREDUCESSZrrikz:
		return true;
	}
	return false;
}

bool isVCVTUW2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUW2PHZ128rm:
	case VCVTUW2PHZ128rmb:
	case VCVTUW2PHZ128rmbk:
	case VCVTUW2PHZ128rmbkz:
	case VCVTUW2PHZ128rmk:
	case VCVTUW2PHZ128rmkz:
	case VCVTUW2PHZ128rr:
	case VCVTUW2PHZ128rrk:
	case VCVTUW2PHZ128rrkz:
	case VCVTUW2PHZ256rm:
	case VCVTUW2PHZ256rmb:
	case VCVTUW2PHZ256rmbk:
	case VCVTUW2PHZ256rmbkz:
	case VCVTUW2PHZ256rmk:
	case VCVTUW2PHZ256rmkz:
	case VCVTUW2PHZ256rr:
	case VCVTUW2PHZ256rrk:
	case VCVTUW2PHZ256rrkz:
	case VCVTUW2PHZrm:
	case VCVTUW2PHZrmb:
	case VCVTUW2PHZrmbk:
	case VCVTUW2PHZrmbkz:
	case VCVTUW2PHZrmk:
	case VCVTUW2PHZrmkz:
	case VCVTUW2PHZrr:
	case VCVTUW2PHZrrb:
	case VCVTUW2PHZrrbk:
	case VCVTUW2PHZrrbkz:
	case VCVTUW2PHZrrk:
	case VCVTUW2PHZrrkz:
		return true;
	}
	return false;
}

bool isVPBLENDMB(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDMBZ128rm:
	case VPBLENDMBZ128rmk:
	case VPBLENDMBZ128rmkz:
	case VPBLENDMBZ128rr:
	case VPBLENDMBZ128rrk:
	case VPBLENDMBZ128rrkz:
	case VPBLENDMBZ256rm:
	case VPBLENDMBZ256rmk:
	case VPBLENDMBZ256rmkz:
	case VPBLENDMBZ256rr:
	case VPBLENDMBZ256rrk:
	case VPBLENDMBZ256rrkz:
	case VPBLENDMBZrm:
	case VPBLENDMBZrmk:
	case VPBLENDMBZrmkz:
	case VPBLENDMBZrr:
	case VPBLENDMBZrrk:
	case VPBLENDMBZrrkz:
		return true;
	}
	return false;
}

bool isVPBLENDMD(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDMDZ128rm:
	case VPBLENDMDZ128rmb:
	case VPBLENDMDZ128rmbk:
	case VPBLENDMDZ128rmbkz:
	case VPBLENDMDZ128rmk:
	case VPBLENDMDZ128rmkz:
	case VPBLENDMDZ128rr:
	case VPBLENDMDZ128rrk:
	case VPBLENDMDZ128rrkz:
	case VPBLENDMDZ256rm:
	case VPBLENDMDZ256rmb:
	case VPBLENDMDZ256rmbk:
	case VPBLENDMDZ256rmbkz:
	case VPBLENDMDZ256rmk:
	case VPBLENDMDZ256rmkz:
	case VPBLENDMDZ256rr:
	case VPBLENDMDZ256rrk:
	case VPBLENDMDZ256rrkz:
	case VPBLENDMDZrm:
	case VPBLENDMDZrmb:
	case VPBLENDMDZrmbk:
	case VPBLENDMDZrmbkz:
	case VPBLENDMDZrmk:
	case VPBLENDMDZrmkz:
	case VPBLENDMDZrr:
	case VPBLENDMDZrrk:
	case VPBLENDMDZrrkz:
		return true;
	}
	return false;
}

bool isVFMSUB132PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB132PDYm:
	case VFMSUB132PDYr:
	case VFMSUB132PDZ128m:
	case VFMSUB132PDZ128mb:
	case VFMSUB132PDZ128mbk:
	case VFMSUB132PDZ128mbkz:
	case VFMSUB132PDZ128mk:
	case VFMSUB132PDZ128mkz:
	case VFMSUB132PDZ128r:
	case VFMSUB132PDZ128rk:
	case VFMSUB132PDZ128rkz:
	case VFMSUB132PDZ256m:
	case VFMSUB132PDZ256mb:
	case VFMSUB132PDZ256mbk:
	case VFMSUB132PDZ256mbkz:
	case VFMSUB132PDZ256mk:
	case VFMSUB132PDZ256mkz:
	case VFMSUB132PDZ256r:
	case VFMSUB132PDZ256rk:
	case VFMSUB132PDZ256rkz:
	case VFMSUB132PDZm:
	case VFMSUB132PDZmb:
	case VFMSUB132PDZmbk:
	case VFMSUB132PDZmbkz:
	case VFMSUB132PDZmk:
	case VFMSUB132PDZmkz:
	case VFMSUB132PDZr:
	case VFMSUB132PDZrb:
	case VFMSUB132PDZrbk:
	case VFMSUB132PDZrbkz:
	case VFMSUB132PDZrk:
	case VFMSUB132PDZrkz:
	case VFMSUB132PDm:
	case VFMSUB132PDr:
		return true;
	}
	return false;
}

bool isVFMSUB132PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB132PHZ128m:
	case VFMSUB132PHZ128mb:
	case VFMSUB132PHZ128mbk:
	case VFMSUB132PHZ128mbkz:
	case VFMSUB132PHZ128mk:
	case VFMSUB132PHZ128mkz:
	case VFMSUB132PHZ128r:
	case VFMSUB132PHZ128rk:
	case VFMSUB132PHZ128rkz:
	case VFMSUB132PHZ256m:
	case VFMSUB132PHZ256mb:
	case VFMSUB132PHZ256mbk:
	case VFMSUB132PHZ256mbkz:
	case VFMSUB132PHZ256mk:
	case VFMSUB132PHZ256mkz:
	case VFMSUB132PHZ256r:
	case VFMSUB132PHZ256rk:
	case VFMSUB132PHZ256rkz:
	case VFMSUB132PHZm:
	case VFMSUB132PHZmb:
	case VFMSUB132PHZmbk:
	case VFMSUB132PHZmbkz:
	case VFMSUB132PHZmk:
	case VFMSUB132PHZmkz:
	case VFMSUB132PHZr:
	case VFMSUB132PHZrb:
	case VFMSUB132PHZrbk:
	case VFMSUB132PHZrbkz:
	case VFMSUB132PHZrk:
	case VFMSUB132PHZrkz:
		return true;
	}
	return false;
}

bool isMWAIT(unsigned Opcode) {
	return Opcode == MWAITrr;
}

bool isSALC(unsigned Opcode) {
	return Opcode == SALC;
}

bool isPMADDUBSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMADDUBSWrm:
	case MMX_PMADDUBSWrr:
	case PMADDUBSWrm:
	case PMADDUBSWrr:
		return true;
	}
	return false;
}

bool isVFCMULCPH(unsigned Opcode) {
	switch (Opcode) {
	case VFCMULCPHZ128rm:
	case VFCMULCPHZ128rmb:
	case VFCMULCPHZ128rmbk:
	case VFCMULCPHZ128rmbkz:
	case VFCMULCPHZ128rmk:
	case VFCMULCPHZ128rmkz:
	case VFCMULCPHZ128rr:
	case VFCMULCPHZ128rrk:
	case VFCMULCPHZ128rrkz:
	case VFCMULCPHZ256rm:
	case VFCMULCPHZ256rmb:
	case VFCMULCPHZ256rmbk:
	case VFCMULCPHZ256rmbkz:
	case VFCMULCPHZ256rmk:
	case VFCMULCPHZ256rmkz:
	case VFCMULCPHZ256rr:
	case VFCMULCPHZ256rrk:
	case VFCMULCPHZ256rrkz:
	case VFCMULCPHZrm:
	case VFCMULCPHZrmb:
	case VFCMULCPHZrmbk:
	case VFCMULCPHZrmbkz:
	case VFCMULCPHZrmk:
	case VFCMULCPHZrmkz:
	case VFCMULCPHZrr:
	case VFCMULCPHZrrb:
	case VFCMULCPHZrrbk:
	case VFCMULCPHZrrbkz:
	case VFCMULCPHZrrk:
	case VFCMULCPHZrrkz:
		return true;
	}
	return false;
}

bool isVPBLENDMQ(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDMQZ128rm:
	case VPBLENDMQZ128rmb:
	case VPBLENDMQZ128rmbk:
	case VPBLENDMQZ128rmbkz:
	case VPBLENDMQZ128rmk:
	case VPBLENDMQZ128rmkz:
	case VPBLENDMQZ128rr:
	case VPBLENDMQZ128rrk:
	case VPBLENDMQZ128rrkz:
	case VPBLENDMQZ256rm:
	case VPBLENDMQZ256rmb:
	case VPBLENDMQZ256rmbk:
	case VPBLENDMQZ256rmbkz:
	case VPBLENDMQZ256rmk:
	case VPBLENDMQZ256rmkz:
	case VPBLENDMQZ256rr:
	case VPBLENDMQZ256rrk:
	case VPBLENDMQZ256rrkz:
	case VPBLENDMQZrm:
	case VPBLENDMQZrmb:
	case VPBLENDMQZrmbk:
	case VPBLENDMQZrmbkz:
	case VPBLENDMQZrmk:
	case VPBLENDMQZrmkz:
	case VPBLENDMQZrr:
	case VPBLENDMQZrrk:
	case VPBLENDMQZrrkz:
		return true;
	}
	return false;
}

bool isRORX(unsigned Opcode) {
	switch (Opcode) {
	case RORX32mi:
	case RORX32ri:
	case RORX64mi:
	case RORX64ri:
		return true;
	}
	return false;
}

bool isVFMSUB132PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB132PSYm:
	case VFMSUB132PSYr:
	case VFMSUB132PSZ128m:
	case VFMSUB132PSZ128mb:
	case VFMSUB132PSZ128mbk:
	case VFMSUB132PSZ128mbkz:
	case VFMSUB132PSZ128mk:
	case VFMSUB132PSZ128mkz:
	case VFMSUB132PSZ128r:
	case VFMSUB132PSZ128rk:
	case VFMSUB132PSZ128rkz:
	case VFMSUB132PSZ256m:
	case VFMSUB132PSZ256mb:
	case VFMSUB132PSZ256mbk:
	case VFMSUB132PSZ256mbkz:
	case VFMSUB132PSZ256mk:
	case VFMSUB132PSZ256mkz:
	case VFMSUB132PSZ256r:
	case VFMSUB132PSZ256rk:
	case VFMSUB132PSZ256rkz:
	case VFMSUB132PSZm:
	case VFMSUB132PSZmb:
	case VFMSUB132PSZmbk:
	case VFMSUB132PSZmbkz:
	case VFMSUB132PSZmk:
	case VFMSUB132PSZmkz:
	case VFMSUB132PSZr:
	case VFMSUB132PSZrb:
	case VFMSUB132PSZrbk:
	case VFMSUB132PSZrbkz:
	case VFMSUB132PSZrk:
	case VFMSUB132PSZrkz:
	case VFMSUB132PSm:
	case VFMSUB132PSr:
		return true;
	}
	return false;
}

bool isVPBLENDMW(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDMWZ128rm:
	case VPBLENDMWZ128rmk:
	case VPBLENDMWZ128rmkz:
	case VPBLENDMWZ128rr:
	case VPBLENDMWZ128rrk:
	case VPBLENDMWZ128rrkz:
	case VPBLENDMWZ256rm:
	case VPBLENDMWZ256rmk:
	case VPBLENDMWZ256rmkz:
	case VPBLENDMWZ256rr:
	case VPBLENDMWZ256rrk:
	case VPBLENDMWZ256rrkz:
	case VPBLENDMWZrm:
	case VPBLENDMWZrmk:
	case VPBLENDMWZrmkz:
	case VPBLENDMWZrr:
	case VPBLENDMWZrrk:
	case VPBLENDMWZrrkz:
		return true;
	}
	return false;
}

bool isMOV(unsigned Opcode) {
	switch (Opcode) {
	case MOV16ao16:
	case MOV16ao32:
	case MOV16mi:
	case MOV16mr:
	case MOV16ms:
	case MOV16o16a:
	case MOV16o32a:
	case MOV16ri:
	case MOV16ri_alt:
	case MOV16rm:
	case MOV16rr:
	case MOV16rr_REV:
	case MOV16rs:
	case MOV16sm:
	case MOV16sr:
	case MOV32ao16:
	case MOV32ao32:
	case MOV32cr:
	case MOV32dr:
	case MOV32mi:
	case MOV32mr:
	case MOV32o16a:
	case MOV32o32a:
	case MOV32rc:
	case MOV32rd:
	case MOV32ri:
	case MOV32ri_alt:
	case MOV32rm:
	case MOV32rr:
	case MOV32rr_REV:
	case MOV32rs:
	case MOV32sr:
	case MOV64ao32:
	case MOV64cr:
	case MOV64dr:
	case MOV64mi32:
	case MOV64mr:
	case MOV64o32a:
	case MOV64rc:
	case MOV64rd:
	case MOV64ri32:
	case MOV64rm:
	case MOV64rr:
	case MOV64rr_REV:
	case MOV64rs:
	case MOV64sr:
	case MOV8ao16:
	case MOV8ao32:
	case MOV8mi:
	case MOV8mr:
	case MOV8o16a:
	case MOV8o32a:
	case MOV8ri:
	case MOV8ri_alt:
	case MOV8rm:
	case MOV8rr:
	case MOV8rr_REV:
		return true;
	}
	return false;
}

bool isFXSAVE64(unsigned Opcode) {
	return Opcode == FXSAVE64;
}

bool isRMPADJUST(unsigned Opcode) {
	return Opcode == RMPADJUST;
}

bool isAADD(unsigned Opcode) {
	switch (Opcode) {
	case AADD32mr:
	case AADD64mr:
		return true;
	}
	return false;
}

bool isVLDDQU(unsigned Opcode) {
	switch (Opcode) {
	case VLDDQUYrm:
	case VLDDQUrm:
		return true;
	}
	return false;
}

bool isVPSCATTERQD(unsigned Opcode) {
	switch (Opcode) {
	case VPSCATTERQDZ128mr:
	case VPSCATTERQDZ256mr:
	case VPSCATTERQDZmr:
		return true;
	}
	return false;
}

bool isVPHADDUBD(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDUBDrm:
	case VPHADDUBDrr:
		return true;
	}
	return false;
}

bool isCMPSB(unsigned Opcode) {
	return Opcode == CMPSB;
}

bool isCMPSD(unsigned Opcode) {
	switch (Opcode) {
	case CMPSDrm_Int:
	case CMPSDrr_Int:
	case CMPSL:
		return true;
	}
	return false;
}

bool isSTGI(unsigned Opcode) {
	return Opcode == STGI;
}

bool isVUNPCKLPD(unsigned Opcode) {
	switch (Opcode) {
	case VUNPCKLPDYrm:
	case VUNPCKLPDYrr:
	case VUNPCKLPDZ128rm:
	case VUNPCKLPDZ128rmb:
	case VUNPCKLPDZ128rmbk:
	case VUNPCKLPDZ128rmbkz:
	case VUNPCKLPDZ128rmk:
	case VUNPCKLPDZ128rmkz:
	case VUNPCKLPDZ128rr:
	case VUNPCKLPDZ128rrk:
	case VUNPCKLPDZ128rrkz:
	case VUNPCKLPDZ256rm:
	case VUNPCKLPDZ256rmb:
	case VUNPCKLPDZ256rmbk:
	case VUNPCKLPDZ256rmbkz:
	case VUNPCKLPDZ256rmk:
	case VUNPCKLPDZ256rmkz:
	case VUNPCKLPDZ256rr:
	case VUNPCKLPDZ256rrk:
	case VUNPCKLPDZ256rrkz:
	case VUNPCKLPDZrm:
	case VUNPCKLPDZrmb:
	case VUNPCKLPDZrmbk:
	case VUNPCKLPDZrmbkz:
	case VUNPCKLPDZrmk:
	case VUNPCKLPDZrmkz:
	case VUNPCKLPDZrr:
	case VUNPCKLPDZrrk:
	case VUNPCKLPDZrrkz:
	case VUNPCKLPDrm:
	case VUNPCKLPDrr:
		return true;
	}
	return false;
}

bool isVPSCATTERQQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSCATTERQQZ128mr:
	case VPSCATTERQQZ256mr:
	case VPSCATTERQQZmr:
		return true;
	}
	return false;
}

bool isFADDP(unsigned Opcode) {
	return Opcode == ADD_FPrST0;
}

bool isVPHADDUBQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDUBQrm:
	case VPHADDUBQrr:
		return true;
	}
	return false;
}

bool isCMPSQ(unsigned Opcode) {
	return Opcode == CMPSQ;
}

bool isVPHADDUBW(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDUBWrm:
	case VPHADDUBWrr:
		return true;
	}
	return false;
}

bool isCMPSS(unsigned Opcode) {
	switch (Opcode) {
	case CMPSSrm_Int:
	case CMPSSrr_Int:
		return true;
	}
	return false;
}

bool isVUNPCKLPS(unsigned Opcode) {
	switch (Opcode) {
	case VUNPCKLPSYrm:
	case VUNPCKLPSYrr:
	case VUNPCKLPSZ128rm:
	case VUNPCKLPSZ128rmb:
	case VUNPCKLPSZ128rmbk:
	case VUNPCKLPSZ128rmbkz:
	case VUNPCKLPSZ128rmk:
	case VUNPCKLPSZ128rmkz:
	case VUNPCKLPSZ128rr:
	case VUNPCKLPSZ128rrk:
	case VUNPCKLPSZ128rrkz:
	case VUNPCKLPSZ256rm:
	case VUNPCKLPSZ256rmb:
	case VUNPCKLPSZ256rmbk:
	case VUNPCKLPSZ256rmbkz:
	case VUNPCKLPSZ256rmk:
	case VUNPCKLPSZ256rmkz:
	case VUNPCKLPSZ256rr:
	case VUNPCKLPSZ256rrk:
	case VUNPCKLPSZ256rrkz:
	case VUNPCKLPSZrm:
	case VUNPCKLPSZrmb:
	case VUNPCKLPSZrmbk:
	case VUNPCKLPSZrmbkz:
	case VUNPCKLPSZrmk:
	case VUNPCKLPSZrmkz:
	case VUNPCKLPSZrr:
	case VUNPCKLPSZrrk:
	case VUNPCKLPSZrrkz:
	case VUNPCKLPSrm:
	case VUNPCKLPSrr:
		return true;
	}
	return false;
}

bool isLCALL(unsigned Opcode) {
	switch (Opcode) {
	case FARCALL16i:
	case FARCALL16m:
	case FARCALL32i:
	case FARCALL64m:
		return true;
	}
	return false;
}

bool isPSHUFB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSHUFBrm:
	case MMX_PSHUFBrr:
	case PSHUFBrm:
	case PSHUFBrr:
		return true;
	}
	return false;
}

bool isCMPSW(unsigned Opcode) {
	return Opcode == CMPSW;
}

bool isPSHUFD(unsigned Opcode) {
	switch (Opcode) {
	case PSHUFDmi:
	case PSHUFDri:
		return true;
	}
	return false;
}

bool isRDPRU(unsigned Opcode) {
	return Opcode == RDPRU;
}

bool isFRNDINT(unsigned Opcode) {
	return Opcode == FRNDINT;
}

bool isVPACKUSWB(unsigned Opcode) {
	switch (Opcode) {
	case VPACKUSWBYrm:
	case VPACKUSWBYrr:
	case VPACKUSWBZ128rm:
	case VPACKUSWBZ128rmk:
	case VPACKUSWBZ128rmkz:
	case VPACKUSWBZ128rr:
	case VPACKUSWBZ128rrk:
	case VPACKUSWBZ128rrkz:
	case VPACKUSWBZ256rm:
	case VPACKUSWBZ256rmk:
	case VPACKUSWBZ256rmkz:
	case VPACKUSWBZ256rr:
	case VPACKUSWBZ256rrk:
	case VPACKUSWBZ256rrkz:
	case VPACKUSWBZrm:
	case VPACKUSWBZrmk:
	case VPACKUSWBZrmkz:
	case VPACKUSWBZrr:
	case VPACKUSWBZrrk:
	case VPACKUSWBZrrkz:
	case VPACKUSWBrm:
	case VPACKUSWBrr:
		return true;
	}
	return false;
}

bool isDIVPD(unsigned Opcode) {
	switch (Opcode) {
	case DIVPDrm:
	case DIVPDrr:
		return true;
	}
	return false;
}

bool isVAESDEC(unsigned Opcode) {
	switch (Opcode) {
	case VAESDECYrm:
	case VAESDECYrr:
	case VAESDECZ128rm:
	case VAESDECZ128rr:
	case VAESDECZ256rm:
	case VAESDECZ256rr:
	case VAESDECZrm:
	case VAESDECZrr:
	case VAESDECrm:
	case VAESDECrr:
		return true;
	}
	return false;
}

bool isPSHUFW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSHUFWmi:
	case MMX_PSHUFWri:
		return true;
	}
	return false;
}

bool isVPDPBUUDS(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBUUDSYrm:
	case VPDPBUUDSYrr:
	case VPDPBUUDSrm:
	case VPDPBUUDSrr:
		return true;
	}
	return false;
}

bool isKMOVB(unsigned Opcode) {
	switch (Opcode) {
	case KMOVBkk:
	case KMOVBkm:
	case KMOVBkr:
	case KMOVBmk:
	case KMOVBrk:
		return true;
	}
	return false;
}

bool isVFMSUB132SD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB132SDZm_Int:
	case VFMSUB132SDZm_Intk:
	case VFMSUB132SDZm_Intkz:
	case VFMSUB132SDZr_Int:
	case VFMSUB132SDZr_Intk:
	case VFMSUB132SDZr_Intkz:
	case VFMSUB132SDZrb_Int:
	case VFMSUB132SDZrb_Intk:
	case VFMSUB132SDZrb_Intkz:
	case VFMSUB132SDm_Int:
	case VFMSUB132SDr_Int:
		return true;
	}
	return false;
}

bool isKMOVD(unsigned Opcode) {
	switch (Opcode) {
	case KMOVDkk:
	case KMOVDkm:
	case KMOVDkr:
	case KMOVDmk:
	case KMOVDrk:
		return true;
	}
	return false;
}

bool isVCVTTPS2UQQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPS2UQQZ128rm:
	case VCVTTPS2UQQZ128rmb:
	case VCVTTPS2UQQZ128rmbk:
	case VCVTTPS2UQQZ128rmbkz:
	case VCVTTPS2UQQZ128rmk:
	case VCVTTPS2UQQZ128rmkz:
	case VCVTTPS2UQQZ128rr:
	case VCVTTPS2UQQZ128rrk:
	case VCVTTPS2UQQZ128rrkz:
	case VCVTTPS2UQQZ256rm:
	case VCVTTPS2UQQZ256rmb:
	case VCVTTPS2UQQZ256rmbk:
	case VCVTTPS2UQQZ256rmbkz:
	case VCVTTPS2UQQZ256rmk:
	case VCVTTPS2UQQZ256rmkz:
	case VCVTTPS2UQQZ256rr:
	case VCVTTPS2UQQZ256rrk:
	case VCVTTPS2UQQZ256rrkz:
	case VCVTTPS2UQQZrm:
	case VCVTTPS2UQQZrmb:
	case VCVTTPS2UQQZrmbk:
	case VCVTTPS2UQQZrmbkz:
	case VCVTTPS2UQQZrmk:
	case VCVTTPS2UQQZrmkz:
	case VCVTTPS2UQQZrr:
	case VCVTTPS2UQQZrrb:
	case VCVTTPS2UQQZrrbk:
	case VCVTTPS2UQQZrrbkz:
	case VCVTTPS2UQQZrrk:
	case VCVTTPS2UQQZrrkz:
		return true;
	}
	return false;
}

bool isVFMSUB132SH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB132SHZm_Int:
	case VFMSUB132SHZm_Intk:
	case VFMSUB132SHZm_Intkz:
	case VFMSUB132SHZr_Int:
	case VFMSUB132SHZr_Intk:
	case VFMSUB132SHZr_Intkz:
	case VFMSUB132SHZrb_Int:
	case VFMSUB132SHZrb_Intk:
	case VFMSUB132SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isDIVPS(unsigned Opcode) {
	switch (Opcode) {
	case DIVPSrm:
	case DIVPSrr:
		return true;
	}
	return false;
}

bool isVFCMULCSH(unsigned Opcode) {
	switch (Opcode) {
	case VFCMULCSHZrm:
	case VFCMULCSHZrmk:
	case VFCMULCSHZrmkz:
	case VFCMULCSHZrr:
	case VFCMULCSHZrrb:
	case VFCMULCSHZrrbk:
	case VFCMULCSHZrrbkz:
	case VFCMULCSHZrrk:
	case VFCMULCSHZrrkz:
		return true;
	}
	return false;
}

bool isFICOM(unsigned Opcode) {
	switch (Opcode) {
	case FICOM16m:
	case FICOM32m:
		return true;
	}
	return false;
}

bool isKMOVQ(unsigned Opcode) {
	switch (Opcode) {
	case KMOVQkk:
	case KMOVQkm:
	case KMOVQkr:
	case KMOVQmk:
	case KMOVQrk:
		return true;
	}
	return false;
}

bool isVFMSUB132SS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB132SSZm_Int:
	case VFMSUB132SSZm_Intk:
	case VFMSUB132SSZm_Intkz:
	case VFMSUB132SSZr_Int:
	case VFMSUB132SSZr_Intk:
	case VFMSUB132SSZr_Intkz:
	case VFMSUB132SSZrb_Int:
	case VFMSUB132SSZrb_Intk:
	case VFMSUB132SSZrb_Intkz:
	case VFMSUB132SSm_Int:
	case VFMSUB132SSr_Int:
		return true;
	}
	return false;
}

bool isENCODEKEY128(unsigned Opcode) {
	return Opcode == ENCODEKEY128;
}

bool isKMOVW(unsigned Opcode) {
	switch (Opcode) {
	case KMOVWkk:
	case KMOVWkm:
	case KMOVWkr:
	case KMOVWmk:
	case KMOVWrk:
		return true;
	}
	return false;
}

bool isPREFETCHT0(unsigned Opcode) {
	return Opcode == PREFETCHT0;
}

bool isPREFETCHT1(unsigned Opcode) {
	return Opcode == PREFETCHT1;
}

bool isPREFETCHT2(unsigned Opcode) {
	return Opcode == PREFETCHT2;
}

bool isSWAPGS(unsigned Opcode) {
	return Opcode == SWAPGS;
}

bool isVPTESTMD(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTMDZ128rm:
	case VPTESTMDZ128rmb:
	case VPTESTMDZ128rmbk:
	case VPTESTMDZ128rmk:
	case VPTESTMDZ128rr:
	case VPTESTMDZ128rrk:
	case VPTESTMDZ256rm:
	case VPTESTMDZ256rmb:
	case VPTESTMDZ256rmbk:
	case VPTESTMDZ256rmk:
	case VPTESTMDZ256rr:
	case VPTESTMDZ256rrk:
	case VPTESTMDZrm:
	case VPTESTMDZrmb:
	case VPTESTMDZrmbk:
	case VPTESTMDZrmk:
	case VPTESTMDZrr:
	case VPTESTMDZrrk:
		return true;
	}
	return false;
}

bool isVPTESTMB(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTMBZ128rm:
	case VPTESTMBZ128rmk:
	case VPTESTMBZ128rr:
	case VPTESTMBZ128rrk:
	case VPTESTMBZ256rm:
	case VPTESTMBZ256rmk:
	case VPTESTMBZ256rr:
	case VPTESTMBZ256rrk:
	case VPTESTMBZrm:
	case VPTESTMBZrmk:
	case VPTESTMBZrr:
	case VPTESTMBZrrk:
		return true;
	}
	return false;
}

bool isVPHADDUDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDUDQrm:
	case VPHADDUDQrr:
		return true;
	}
	return false;
}

bool isVPTESTMQ(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTMQZ128rm:
	case VPTESTMQZ128rmb:
	case VPTESTMQZ128rmbk:
	case VPTESTMQZ128rmk:
	case VPTESTMQZ128rr:
	case VPTESTMQZ128rrk:
	case VPTESTMQZ256rm:
	case VPTESTMQZ256rmb:
	case VPTESTMQZ256rmbk:
	case VPTESTMQZ256rmk:
	case VPTESTMQZ256rr:
	case VPTESTMQZ256rrk:
	case VPTESTMQZrm:
	case VPTESTMQZrmb:
	case VPTESTMQZrmbk:
	case VPTESTMQZrmk:
	case VPTESTMQZrr:
	case VPTESTMQZrrk:
		return true;
	}
	return false;
}

bool isXRSTORS(unsigned Opcode) {
	return Opcode == XRSTORS;
}

bool isVPMULDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMULDQYrm:
	case VPMULDQYrr:
	case VPMULDQZ128rm:
	case VPMULDQZ128rmb:
	case VPMULDQZ128rmbk:
	case VPMULDQZ128rmbkz:
	case VPMULDQZ128rmk:
	case VPMULDQZ128rmkz:
	case VPMULDQZ128rr:
	case VPMULDQZ128rrk:
	case VPMULDQZ128rrkz:
	case VPMULDQZ256rm:
	case VPMULDQZ256rmb:
	case VPMULDQZ256rmbk:
	case VPMULDQZ256rmbkz:
	case VPMULDQZ256rmk:
	case VPMULDQZ256rmkz:
	case VPMULDQZ256rr:
	case VPMULDQZ256rrk:
	case VPMULDQZ256rrkz:
	case VPMULDQZrm:
	case VPMULDQZrmb:
	case VPMULDQZrmbk:
	case VPMULDQZrmbkz:
	case VPMULDQZrmk:
	case VPMULDQZrmkz:
	case VPMULDQZrr:
	case VPMULDQZrrk:
	case VPMULDQZrrkz:
	case VPMULDQrm:
	case VPMULDQrr:
		return true;
	}
	return false;
}

bool isUD1(unsigned Opcode) {
	switch (Opcode) {
	case UD1Lm:
	case UD1Lr:
	case UD1Qm:
	case UD1Qr:
	case UD1Wm:
	case UD1Wr:
		return true;
	}
	return false;
}

bool isUD2(unsigned Opcode) {
	return Opcode == TRAP;
}

bool isVPTESTMW(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTMWZ128rm:
	case VPTESTMWZ128rmk:
	case VPTESTMWZ128rr:
	case VPTESTMWZ128rrk:
	case VPTESTMWZ256rm:
	case VPTESTMWZ256rmk:
	case VPTESTMWZ256rr:
	case VPTESTMWZ256rrk:
	case VPTESTMWZrm:
	case VPTESTMWZrmk:
	case VPTESTMWZrr:
	case VPTESTMWZrrk:
		return true;
	}
	return false;
}

bool isSEAMOPS(unsigned Opcode) {
	return Opcode == SEAMOPS;
}

bool isMWAITX(unsigned Opcode) {
	return Opcode == MWAITXrrr;
}

bool isVFMADD132PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD132PDYm:
	case VFMADD132PDYr:
	case VFMADD132PDZ128m:
	case VFMADD132PDZ128mb:
	case VFMADD132PDZ128mbk:
	case VFMADD132PDZ128mbkz:
	case VFMADD132PDZ128mk:
	case VFMADD132PDZ128mkz:
	case VFMADD132PDZ128r:
	case VFMADD132PDZ128rk:
	case VFMADD132PDZ128rkz:
	case VFMADD132PDZ256m:
	case VFMADD132PDZ256mb:
	case VFMADD132PDZ256mbk:
	case VFMADD132PDZ256mbkz:
	case VFMADD132PDZ256mk:
	case VFMADD132PDZ256mkz:
	case VFMADD132PDZ256r:
	case VFMADD132PDZ256rk:
	case VFMADD132PDZ256rkz:
	case VFMADD132PDZm:
	case VFMADD132PDZmb:
	case VFMADD132PDZmbk:
	case VFMADD132PDZmbkz:
	case VFMADD132PDZmk:
	case VFMADD132PDZmkz:
	case VFMADD132PDZr:
	case VFMADD132PDZrb:
	case VFMADD132PDZrbk:
	case VFMADD132PDZrbkz:
	case VFMADD132PDZrk:
	case VFMADD132PDZrkz:
	case VFMADD132PDm:
	case VFMADD132PDr:
		return true;
	}
	return false;
}

bool isWRMSRNS(unsigned Opcode) {
	return Opcode == WRMSRNS;
}

bool isVMOVNTDQ(unsigned Opcode) {
	switch (Opcode) {
	case VMOVNTDQYmr:
	case VMOVNTDQZ128mr:
	case VMOVNTDQZ256mr:
	case VMOVNTDQZmr:
	case VMOVNTDQmr:
		return true;
	}
	return false;
}

bool isVFMADD132PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD132PHZ128m:
	case VFMADD132PHZ128mb:
	case VFMADD132PHZ128mbk:
	case VFMADD132PHZ128mbkz:
	case VFMADD132PHZ128mk:
	case VFMADD132PHZ128mkz:
	case VFMADD132PHZ128r:
	case VFMADD132PHZ128rk:
	case VFMADD132PHZ128rkz:
	case VFMADD132PHZ256m:
	case VFMADD132PHZ256mb:
	case VFMADD132PHZ256mbk:
	case VFMADD132PHZ256mbkz:
	case VFMADD132PHZ256mk:
	case VFMADD132PHZ256mkz:
	case VFMADD132PHZ256r:
	case VFMADD132PHZ256rk:
	case VFMADD132PHZ256rkz:
	case VFMADD132PHZm:
	case VFMADD132PHZmb:
	case VFMADD132PHZmbk:
	case VFMADD132PHZmbkz:
	case VFMADD132PHZmk:
	case VFMADD132PHZmkz:
	case VFMADD132PHZr:
	case VFMADD132PHZrb:
	case VFMADD132PHZrbk:
	case VFMADD132PHZrbkz:
	case VFMADD132PHZrk:
	case VFMADD132PHZrkz:
		return true;
	}
	return false;
}

bool isPSRLD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSRLDri:
	case MMX_PSRLDrm:
	case MMX_PSRLDrr:
	case PSRLDri:
	case PSRLDrm:
	case PSRLDrr:
		return true;
	}
	return false;
}

bool isVBROADCASTI64X2(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTI64X2Z128rm:
	case VBROADCASTI64X2Z128rmk:
	case VBROADCASTI64X2Z128rmkz:
	case VBROADCASTI64X2rm:
	case VBROADCASTI64X2rmk:
	case VBROADCASTI64X2rmkz:
		return true;
	}
	return false;
}

bool isVBROADCASTI64X4(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTI64X4rm:
	case VBROADCASTI64X4rmk:
	case VBROADCASTI64X4rmkz:
		return true;
	}
	return false;
}

bool isVFMADD132PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD132PSYm:
	case VFMADD132PSYr:
	case VFMADD132PSZ128m:
	case VFMADD132PSZ128mb:
	case VFMADD132PSZ128mbk:
	case VFMADD132PSZ128mbkz:
	case VFMADD132PSZ128mk:
	case VFMADD132PSZ128mkz:
	case VFMADD132PSZ128r:
	case VFMADD132PSZ128rk:
	case VFMADD132PSZ128rkz:
	case VFMADD132PSZ256m:
	case VFMADD132PSZ256mb:
	case VFMADD132PSZ256mbk:
	case VFMADD132PSZ256mbkz:
	case VFMADD132PSZ256mk:
	case VFMADD132PSZ256mkz:
	case VFMADD132PSZ256r:
	case VFMADD132PSZ256rk:
	case VFMADD132PSZ256rkz:
	case VFMADD132PSZm:
	case VFMADD132PSZmb:
	case VFMADD132PSZmbk:
	case VFMADD132PSZmbkz:
	case VFMADD132PSZmk:
	case VFMADD132PSZmkz:
	case VFMADD132PSZr:
	case VFMADD132PSZrb:
	case VFMADD132PSZrbk:
	case VFMADD132PSZrbkz:
	case VFMADD132PSZrk:
	case VFMADD132PSZrkz:
	case VFMADD132PSm:
	case VFMADD132PSr:
		return true;
	}
	return false;
}

bool isIDIV(unsigned Opcode) {
	switch (Opcode) {
	case IDIV16m:
	case IDIV16r:
	case IDIV32m:
	case IDIV32r:
	case IDIV64m:
	case IDIV64r:
	case IDIV8m:
	case IDIV8r:
		return true;
	}
	return false;
}

bool isPREFETCHWT1(unsigned Opcode) {
	return Opcode == PREFETCHWT1;
}

bool isVPSRLVD(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLVDYrm:
	case VPSRLVDYrr:
	case VPSRLVDZ128rm:
	case VPSRLVDZ128rmb:
	case VPSRLVDZ128rmbk:
	case VPSRLVDZ128rmbkz:
	case VPSRLVDZ128rmk:
	case VPSRLVDZ128rmkz:
	case VPSRLVDZ128rr:
	case VPSRLVDZ128rrk:
	case VPSRLVDZ128rrkz:
	case VPSRLVDZ256rm:
	case VPSRLVDZ256rmb:
	case VPSRLVDZ256rmbk:
	case VPSRLVDZ256rmbkz:
	case VPSRLVDZ256rmk:
	case VPSRLVDZ256rmkz:
	case VPSRLVDZ256rr:
	case VPSRLVDZ256rrk:
	case VPSRLVDZ256rrkz:
	case VPSRLVDZrm:
	case VPSRLVDZrmb:
	case VPSRLVDZrmbk:
	case VPSRLVDZrmbkz:
	case VPSRLVDZrmk:
	case VPSRLVDZrmkz:
	case VPSRLVDZrr:
	case VPSRLVDZrrk:
	case VPSRLVDZrrkz:
	case VPSRLVDrm:
	case VPSRLVDrr:
		return true;
	}
	return false;
}

bool isPSRLQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSRLQri:
	case MMX_PSRLQrm:
	case MMX_PSRLQrr:
	case PSRLQri:
	case PSRLQrm:
	case PSRLQrr:
		return true;
	}
	return false;
}

bool isPSRLW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSRLWri:
	case MMX_PSRLWrm:
	case MMX_PSRLWrr:
	case PSRLWri:
	case PSRLWrm:
	case PSRLWrr:
		return true;
	}
	return false;
}

bool isDIVSD(unsigned Opcode) {
	switch (Opcode) {
	case DIVSDrm_Int:
	case DIVSDrr_Int:
		return true;
	}
	return false;
}

bool isVPMOVDB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVDBZ128mr:
	case VPMOVDBZ128mrk:
	case VPMOVDBZ128rr:
	case VPMOVDBZ128rrk:
	case VPMOVDBZ128rrkz:
	case VPMOVDBZ256mr:
	case VPMOVDBZ256mrk:
	case VPMOVDBZ256rr:
	case VPMOVDBZ256rrk:
	case VPMOVDBZ256rrkz:
	case VPMOVDBZmr:
	case VPMOVDBZmrk:
	case VPMOVDBZrr:
	case VPMOVDBZrrk:
	case VPMOVDBZrrkz:
		return true;
	}
	return false;
}

bool isVPSRLVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLVQYrm:
	case VPSRLVQYrr:
	case VPSRLVQZ128rm:
	case VPSRLVQZ128rmb:
	case VPSRLVQZ128rmbk:
	case VPSRLVQZ128rmbkz:
	case VPSRLVQZ128rmk:
	case VPSRLVQZ128rmkz:
	case VPSRLVQZ128rr:
	case VPSRLVQZ128rrk:
	case VPSRLVQZ128rrkz:
	case VPSRLVQZ256rm:
	case VPSRLVQZ256rmb:
	case VPSRLVQZ256rmbk:
	case VPSRLVQZ256rmbkz:
	case VPSRLVQZ256rmk:
	case VPSRLVQZ256rmkz:
	case VPSRLVQZ256rr:
	case VPSRLVQZ256rrk:
	case VPSRLVQZ256rrkz:
	case VPSRLVQZrm:
	case VPSRLVQZrmb:
	case VPSRLVQZrmbk:
	case VPSRLVQZrmbkz:
	case VPSRLVQZrmk:
	case VPSRLVQZrmkz:
	case VPSRLVQZrr:
	case VPSRLVQZrrk:
	case VPSRLVQZrrkz:
	case VPSRLVQrm:
	case VPSRLVQrr:
		return true;
	}
	return false;
}

bool isVPSRLVW(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLVWZ128rm:
	case VPSRLVWZ128rmk:
	case VPSRLVWZ128rmkz:
	case VPSRLVWZ128rr:
	case VPSRLVWZ128rrk:
	case VPSRLVWZ128rrkz:
	case VPSRLVWZ256rm:
	case VPSRLVWZ256rmk:
	case VPSRLVWZ256rmkz:
	case VPSRLVWZ256rr:
	case VPSRLVWZ256rrk:
	case VPSRLVWZ256rrkz:
	case VPSRLVWZrm:
	case VPSRLVWZrmk:
	case VPSRLVWZrmkz:
	case VPSRLVWZrr:
	case VPSRLVWZrrk:
	case VPSRLVWZrrkz:
		return true;
	}
	return false;
}

bool isDIVSS(unsigned Opcode) {
	switch (Opcode) {
	case DIVSSrm_Int:
	case DIVSSrr_Int:
		return true;
	}
	return false;
}

bool isMUL(unsigned Opcode) {
	switch (Opcode) {
	case MUL16m:
	case MUL16r:
	case MUL32m:
	case MUL32r:
	case MUL64m:
	case MUL64r:
	case MUL8m:
	case MUL8r:
		return true;
	}
	return false;
}

bool isVPMOVDW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVDWZ128mr:
	case VPMOVDWZ128mrk:
	case VPMOVDWZ128rr:
	case VPMOVDWZ128rrk:
	case VPMOVDWZ128rrkz:
	case VPMOVDWZ256mr:
	case VPMOVDWZ256mrk:
	case VPMOVDWZ256rr:
	case VPMOVDWZ256rrk:
	case VPMOVDWZ256rrkz:
	case VPMOVDWZmr:
	case VPMOVDWZmrk:
	case VPMOVDWZrr:
	case VPMOVDWZrrk:
	case VPMOVDWZrrkz:
		return true;
	}
	return false;
}

bool isVCVTTPD2UQQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPD2UQQZ128rm:
	case VCVTTPD2UQQZ128rmb:
	case VCVTTPD2UQQZ128rmbk:
	case VCVTTPD2UQQZ128rmbkz:
	case VCVTTPD2UQQZ128rmk:
	case VCVTTPD2UQQZ128rmkz:
	case VCVTTPD2UQQZ128rr:
	case VCVTTPD2UQQZ128rrk:
	case VCVTTPD2UQQZ128rrkz:
	case VCVTTPD2UQQZ256rm:
	case VCVTTPD2UQQZ256rmb:
	case VCVTTPD2UQQZ256rmbk:
	case VCVTTPD2UQQZ256rmbkz:
	case VCVTTPD2UQQZ256rmk:
	case VCVTTPD2UQQZ256rmkz:
	case VCVTTPD2UQQZ256rr:
	case VCVTTPD2UQQZ256rrk:
	case VCVTTPD2UQQZ256rrkz:
	case VCVTTPD2UQQZrm:
	case VCVTTPD2UQQZrmb:
	case VCVTTPD2UQQZrmbk:
	case VCVTTPD2UQQZrmbkz:
	case VCVTTPD2UQQZrmk:
	case VCVTTPD2UQQZrmkz:
	case VCVTTPD2UQQZrr:
	case VCVTTPD2UQQZrrb:
	case VCVTTPD2UQQZrrbk:
	case VCVTTPD2UQQZrrbkz:
	case VCVTTPD2UQQZrrk:
	case VCVTTPD2UQQZrrkz:
		return true;
	}
	return false;
}

bool isFSINCOS(unsigned Opcode) {
	return Opcode == FSINCOS;
}

bool isVPMADD52LUQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMADD52LUQYrm:
	case VPMADD52LUQYrr:
	case VPMADD52LUQZ128m:
	case VPMADD52LUQZ128mb:
	case VPMADD52LUQZ128mbk:
	case VPMADD52LUQZ128mbkz:
	case VPMADD52LUQZ128mk:
	case VPMADD52LUQZ128mkz:
	case VPMADD52LUQZ128r:
	case VPMADD52LUQZ128rk:
	case VPMADD52LUQZ128rkz:
	case VPMADD52LUQZ256m:
	case VPMADD52LUQZ256mb:
	case VPMADD52LUQZ256mbk:
	case VPMADD52LUQZ256mbkz:
	case VPMADD52LUQZ256mk:
	case VPMADD52LUQZ256mkz:
	case VPMADD52LUQZ256r:
	case VPMADD52LUQZ256rk:
	case VPMADD52LUQZ256rkz:
	case VPMADD52LUQZm:
	case VPMADD52LUQZmb:
	case VPMADD52LUQZmbk:
	case VPMADD52LUQZmbkz:
	case VPMADD52LUQZmk:
	case VPMADD52LUQZmkz:
	case VPMADD52LUQZr:
	case VPMADD52LUQZrk:
	case VPMADD52LUQZrkz:
	case VPMADD52LUQrm:
	case VPMADD52LUQrr:
		return true;
	}
	return false;
}

bool isLWPINS(unsigned Opcode) {
	switch (Opcode) {
	case LWPINS32rmi:
	case LWPINS32rri:
	case LWPINS64rmi:
	case LWPINS64rri:
		return true;
	}
	return false;
}

bool isMOVDIR64B(unsigned Opcode) {
	switch (Opcode) {
	case MOVDIR64B16:
	case MOVDIR64B32:
	case MOVDIR64B64:
		return true;
	}
	return false;
}

bool isLOOPE(unsigned Opcode) {
	return Opcode == LOOPE;
}

bool isPUSH(unsigned Opcode) {
	switch (Opcode) {
	case PUSH16i8:
	case PUSH16r:
	case PUSH16rmm:
	case PUSH16rmr:
	case PUSH32i8:
	case PUSH32r:
	case PUSH32rmm:
	case PUSH32rmr:
	case PUSH64i32:
	case PUSH64i8:
	case PUSH64r:
	case PUSH64rmm:
	case PUSH64rmr:
	case PUSHCS16:
	case PUSHCS32:
	case PUSHDS16:
	case PUSHDS32:
	case PUSHES16:
	case PUSHES32:
	case PUSHFS16:
	case PUSHFS32:
	case PUSHFS64:
	case PUSHGS16:
	case PUSHGS32:
	case PUSHGS64:
	case PUSHSS16:
	case PUSHSS32:
	case PUSHi16:
	case PUSHi32:
		return true;
	}
	return false;
}

bool isPSADBW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSADBWrm:
	case MMX_PSADBWrr:
	case PSADBWrm:
	case PSADBWrr:
		return true;
	}
	return false;
}

bool isFTST(unsigned Opcode) {
	return Opcode == TST_F;
}

bool isSETSSBSY(unsigned Opcode) {
	return Opcode == SETSSBSY;
}

bool isSARX(unsigned Opcode) {
	switch (Opcode) {
	case SARX32rm:
	case SARX32rr:
	case SARX64rm:
	case SARX64rr:
		return true;
	}
	return false;
}

bool isVADDSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case VADDSUBPDYrm:
	case VADDSUBPDYrr:
	case VADDSUBPDrm:
	case VADDSUBPDrr:
		return true;
	}
	return false;
}

bool isVADDSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case VADDSUBPSYrm:
	case VADDSUBPSYrr:
	case VADDSUBPSrm:
	case VADDSUBPSrr:
		return true;
	}
	return false;
}

bool isVMINPD(unsigned Opcode) {
	switch (Opcode) {
	case VMINPDYrm:
	case VMINPDYrr:
	case VMINPDZ128rm:
	case VMINPDZ128rmb:
	case VMINPDZ128rmbk:
	case VMINPDZ128rmbkz:
	case VMINPDZ128rmk:
	case VMINPDZ128rmkz:
	case VMINPDZ128rr:
	case VMINPDZ128rrk:
	case VMINPDZ128rrkz:
	case VMINPDZ256rm:
	case VMINPDZ256rmb:
	case VMINPDZ256rmbk:
	case VMINPDZ256rmbkz:
	case VMINPDZ256rmk:
	case VMINPDZ256rmkz:
	case VMINPDZ256rr:
	case VMINPDZ256rrk:
	case VMINPDZ256rrkz:
	case VMINPDZrm:
	case VMINPDZrmb:
	case VMINPDZrmbk:
	case VMINPDZrmbkz:
	case VMINPDZrmk:
	case VMINPDZrmkz:
	case VMINPDZrr:
	case VMINPDZrrb:
	case VMINPDZrrbk:
	case VMINPDZrrbkz:
	case VMINPDZrrk:
	case VMINPDZrrkz:
	case VMINPDrm:
	case VMINPDrr:
		return true;
	}
	return false;
}

bool isCLZERO(unsigned Opcode) {
	switch (Opcode) {
	case CLZERO32r:
	case CLZERO64r:
		return true;
	}
	return false;
}

bool isVMINPH(unsigned Opcode) {
	switch (Opcode) {
	case VMINPHZ128rm:
	case VMINPHZ128rmb:
	case VMINPHZ128rmbk:
	case VMINPHZ128rmbkz:
	case VMINPHZ128rmk:
	case VMINPHZ128rmkz:
	case VMINPHZ128rr:
	case VMINPHZ128rrk:
	case VMINPHZ128rrkz:
	case VMINPHZ256rm:
	case VMINPHZ256rmb:
	case VMINPHZ256rmbk:
	case VMINPHZ256rmbkz:
	case VMINPHZ256rmk:
	case VMINPHZ256rmkz:
	case VMINPHZ256rr:
	case VMINPHZ256rrk:
	case VMINPHZ256rrkz:
	case VMINPHZrm:
	case VMINPHZrmb:
	case VMINPHZrmbk:
	case VMINPHZrmbkz:
	case VMINPHZrmk:
	case VMINPHZrmkz:
	case VMINPHZrr:
	case VMINPHZrrb:
	case VMINPHZrrbk:
	case VMINPHZrrbkz:
	case VMINPHZrrk:
	case VMINPHZrrkz:
		return true;
	}
	return false;
}

bool isXCRYPTCBC(unsigned Opcode) {
	return Opcode == XCRYPTCBC;
}

bool isFXTRACT(unsigned Opcode) {
	return Opcode == FXTRACT;
}

bool isVMINPS(unsigned Opcode) {
	switch (Opcode) {
	case VMINPSYrm:
	case VMINPSYrr:
	case VMINPSZ128rm:
	case VMINPSZ128rmb:
	case VMINPSZ128rmbk:
	case VMINPSZ128rmbkz:
	case VMINPSZ128rmk:
	case VMINPSZ128rmkz:
	case VMINPSZ128rr:
	case VMINPSZ128rrk:
	case VMINPSZ128rrkz:
	case VMINPSZ256rm:
	case VMINPSZ256rmb:
	case VMINPSZ256rmbk:
	case VMINPSZ256rmbkz:
	case VMINPSZ256rmk:
	case VMINPSZ256rmkz:
	case VMINPSZ256rr:
	case VMINPSZ256rrk:
	case VMINPSZ256rrkz:
	case VMINPSZrm:
	case VMINPSZrmb:
	case VMINPSZrmbk:
	case VMINPSZrmbkz:
	case VMINPSZrmk:
	case VMINPSZrmkz:
	case VMINPSZrr:
	case VMINPSZrrb:
	case VMINPSZrrbk:
	case VMINPSZrrbkz:
	case VMINPSZrrk:
	case VMINPSZrrkz:
	case VMINPSrm:
	case VMINPSrr:
		return true;
	}
	return false;
}

bool isVFMADD132SD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD132SDZm_Int:
	case VFMADD132SDZm_Intk:
	case VFMADD132SDZm_Intkz:
	case VFMADD132SDZr_Int:
	case VFMADD132SDZr_Intk:
	case VFMADD132SDZr_Intkz:
	case VFMADD132SDZrb_Int:
	case VFMADD132SDZrb_Intk:
	case VFMADD132SDZrb_Intkz:
	case VFMADD132SDm_Int:
	case VFMADD132SDr_Int:
		return true;
	}
	return false;
}

bool isVFMADD132SH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD132SHZm_Int:
	case VFMADD132SHZm_Intk:
	case VFMADD132SHZm_Intkz:
	case VFMADD132SHZr_Int:
	case VFMADD132SHZr_Intk:
	case VFMADD132SHZr_Intkz:
	case VFMADD132SHZrb_Int:
	case VFMADD132SHZrb_Intk:
	case VFMADD132SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVPERMI2B(unsigned Opcode) {
	switch (Opcode) {
	case VPERMI2B128rm:
	case VPERMI2B128rmk:
	case VPERMI2B128rmkz:
	case VPERMI2B128rr:
	case VPERMI2B128rrk:
	case VPERMI2B128rrkz:
	case VPERMI2B256rm:
	case VPERMI2B256rmk:
	case VPERMI2B256rmkz:
	case VPERMI2B256rr:
	case VPERMI2B256rrk:
	case VPERMI2B256rrkz:
	case VPERMI2Brm:
	case VPERMI2Brmk:
	case VPERMI2Brmkz:
	case VPERMI2Brr:
	case VPERMI2Brrk:
	case VPERMI2Brrkz:
		return true;
	}
	return false;
}

bool isVPERMI2D(unsigned Opcode) {
	switch (Opcode) {
	case VPERMI2D128rm:
	case VPERMI2D128rmb:
	case VPERMI2D128rmbk:
	case VPERMI2D128rmbkz:
	case VPERMI2D128rmk:
	case VPERMI2D128rmkz:
	case VPERMI2D128rr:
	case VPERMI2D128rrk:
	case VPERMI2D128rrkz:
	case VPERMI2D256rm:
	case VPERMI2D256rmb:
	case VPERMI2D256rmbk:
	case VPERMI2D256rmbkz:
	case VPERMI2D256rmk:
	case VPERMI2D256rmkz:
	case VPERMI2D256rr:
	case VPERMI2D256rrk:
	case VPERMI2D256rrkz:
	case VPERMI2Drm:
	case VPERMI2Drmb:
	case VPERMI2Drmbk:
	case VPERMI2Drmbkz:
	case VPERMI2Drmk:
	case VPERMI2Drmkz:
	case VPERMI2Drr:
	case VPERMI2Drrk:
	case VPERMI2Drrkz:
		return true;
	}
	return false;
}

bool isVFMADD132SS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD132SSZm_Int:
	case VFMADD132SSZm_Intk:
	case VFMADD132SSZm_Intkz:
	case VFMADD132SSZr_Int:
	case VFMADD132SSZr_Intk:
	case VFMADD132SSZr_Intkz:
	case VFMADD132SSZrb_Int:
	case VFMADD132SSZrb_Intk:
	case VFMADD132SSZrb_Intkz:
	case VFMADD132SSm_Int:
	case VFMADD132SSr_Int:
		return true;
	}
	return false;
}

bool isVRSQRT28PD(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT28PDZm:
	case VRSQRT28PDZmb:
	case VRSQRT28PDZmbk:
	case VRSQRT28PDZmbkz:
	case VRSQRT28PDZmk:
	case VRSQRT28PDZmkz:
	case VRSQRT28PDZr:
	case VRSQRT28PDZrb:
	case VRSQRT28PDZrbk:
	case VRSQRT28PDZrbkz:
	case VRSQRT28PDZrk:
	case VRSQRT28PDZrkz:
		return true;
	}
	return false;
}

bool isVPMULHW(unsigned Opcode) {
	switch (Opcode) {
	case VPMULHWYrm:
	case VPMULHWYrr:
	case VPMULHWZ128rm:
	case VPMULHWZ128rmk:
	case VPMULHWZ128rmkz:
	case VPMULHWZ128rr:
	case VPMULHWZ128rrk:
	case VPMULHWZ128rrkz:
	case VPMULHWZ256rm:
	case VPMULHWZ256rmk:
	case VPMULHWZ256rmkz:
	case VPMULHWZ256rr:
	case VPMULHWZ256rrk:
	case VPMULHWZ256rrkz:
	case VPMULHWZrm:
	case VPMULHWZrmk:
	case VPMULHWZrmkz:
	case VPMULHWZrr:
	case VPMULHWZrrk:
	case VPMULHWZrrkz:
	case VPMULHWrm:
	case VPMULHWrr:
		return true;
	}
	return false;
}

bool isSIDT(unsigned Opcode) {
	return Opcode == SIDT64m;
}

bool isTDCALL(unsigned Opcode) {
	return Opcode == TDCALL;
}

bool isVPERMI2Q(unsigned Opcode) {
	switch (Opcode) {
	case VPERMI2Q128rm:
	case VPERMI2Q128rmb:
	case VPERMI2Q128rmbk:
	case VPERMI2Q128rmbkz:
	case VPERMI2Q128rmk:
	case VPERMI2Q128rmkz:
	case VPERMI2Q128rr:
	case VPERMI2Q128rrk:
	case VPERMI2Q128rrkz:
	case VPERMI2Q256rm:
	case VPERMI2Q256rmb:
	case VPERMI2Q256rmbk:
	case VPERMI2Q256rmbkz:
	case VPERMI2Q256rmk:
	case VPERMI2Q256rmkz:
	case VPERMI2Q256rr:
	case VPERMI2Q256rrk:
	case VPERMI2Q256rrkz:
	case VPERMI2Qrm:
	case VPERMI2Qrmb:
	case VPERMI2Qrmbk:
	case VPERMI2Qrmbkz:
	case VPERMI2Qrmk:
	case VPERMI2Qrmkz:
	case VPERMI2Qrr:
	case VPERMI2Qrrk:
	case VPERMI2Qrrkz:
		return true;
	}
	return false;
}

bool isVPERMI2W(unsigned Opcode) {
	switch (Opcode) {
	case VPERMI2W128rm:
	case VPERMI2W128rmk:
	case VPERMI2W128rmkz:
	case VPERMI2W128rr:
	case VPERMI2W128rrk:
	case VPERMI2W128rrkz:
	case VPERMI2W256rm:
	case VPERMI2W256rmk:
	case VPERMI2W256rmkz:
	case VPERMI2W256rr:
	case VPERMI2W256rrk:
	case VPERMI2W256rrkz:
	case VPERMI2Wrm:
	case VPERMI2Wrmk:
	case VPERMI2Wrmkz:
	case VPERMI2Wrr:
	case VPERMI2Wrrk:
	case VPERMI2Wrrkz:
		return true;
	}
	return false;
}

bool isVPERM2F128(unsigned Opcode) {
	switch (Opcode) {
	case VPERM2F128rm:
	case VPERM2F128rr:
		return true;
	}
	return false;
}

bool isVMLAUNCH(unsigned Opcode) {
	return Opcode == VMLAUNCH;
}

bool isFILD(unsigned Opcode) {
	switch (Opcode) {
	case ILD_F16m:
	case ILD_F32m:
	case ILD_F64m:
		return true;
	}
	return false;
}

bool isVRSQRT28PS(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT28PSZm:
	case VRSQRT28PSZmb:
	case VRSQRT28PSZmbk:
	case VRSQRT28PSZmbkz:
	case VRSQRT28PSZmk:
	case VRSQRT28PSZmkz:
	case VRSQRT28PSZr:
	case VRSQRT28PSZrb:
	case VRSQRT28PSZrbk:
	case VRSQRT28PSZrbkz:
	case VRSQRT28PSZrk:
	case VRSQRT28PSZrkz:
		return true;
	}
	return false;
}

bool isVPBLENDVB(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDVBYrm:
	case VPBLENDVBYrr:
	case VPBLENDVBrm:
	case VPBLENDVBrr:
		return true;
	}
	return false;
}

bool isVPMADDUBSW(unsigned Opcode) {
	switch (Opcode) {
	case VPMADDUBSWYrm:
	case VPMADDUBSWYrr:
	case VPMADDUBSWZ128rm:
	case VPMADDUBSWZ128rmk:
	case VPMADDUBSWZ128rmkz:
	case VPMADDUBSWZ128rr:
	case VPMADDUBSWZ128rrk:
	case VPMADDUBSWZ128rrkz:
	case VPMADDUBSWZ256rm:
	case VPMADDUBSWZ256rmk:
	case VPMADDUBSWZ256rmkz:
	case VPMADDUBSWZ256rr:
	case VPMADDUBSWZ256rrk:
	case VPMADDUBSWZ256rrkz:
	case VPMADDUBSWZrm:
	case VPMADDUBSWZrmk:
	case VPMADDUBSWZrmkz:
	case VPMADDUBSWZrr:
	case VPMADDUBSWZrrk:
	case VPMADDUBSWZrrkz:
	case VPMADDUBSWrm:
	case VPMADDUBSWrr:
		return true;
	}
	return false;
}

bool isVSTMXCSR(unsigned Opcode) {
	return Opcode == VSTMXCSR;
}

bool isVCVTTPH2UDQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPH2UDQZ128rm:
	case VCVTTPH2UDQZ128rmb:
	case VCVTTPH2UDQZ128rmbk:
	case VCVTTPH2UDQZ128rmbkz:
	case VCVTTPH2UDQZ128rmk:
	case VCVTTPH2UDQZ128rmkz:
	case VCVTTPH2UDQZ128rr:
	case VCVTTPH2UDQZ128rrk:
	case VCVTTPH2UDQZ128rrkz:
	case VCVTTPH2UDQZ256rm:
	case VCVTTPH2UDQZ256rmb:
	case VCVTTPH2UDQZ256rmbk:
	case VCVTTPH2UDQZ256rmbkz:
	case VCVTTPH2UDQZ256rmk:
	case VCVTTPH2UDQZ256rmkz:
	case VCVTTPH2UDQZ256rr:
	case VCVTTPH2UDQZ256rrk:
	case VCVTTPH2UDQZ256rrkz:
	case VCVTTPH2UDQZrm:
	case VCVTTPH2UDQZrmb:
	case VCVTTPH2UDQZrmbk:
	case VCVTTPH2UDQZrmbkz:
	case VCVTTPH2UDQZrmk:
	case VCVTTPH2UDQZrmkz:
	case VCVTTPH2UDQZrr:
	case VCVTTPH2UDQZrrb:
	case VCVTTPH2UDQZrrbk:
	case VCVTTPH2UDQZrrbkz:
	case VCVTTPH2UDQZrrk:
	case VCVTTPH2UDQZrrkz:
		return true;
	}
	return false;
}

bool isXSHA256(unsigned Opcode) {
	return Opcode == XSHA256;
}

bool isWAIT(unsigned Opcode) {
	return Opcode == WAIT;
}

bool isPACKSSWB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PACKSSWBrm:
	case MMX_PACKSSWBrr:
	case PACKSSWBrm:
	case PACKSSWBrr:
		return true;
	}
	return false;
}

bool isPMULHRSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMULHRSWrm:
	case MMX_PMULHRSWrr:
	case PMULHRSWrm:
	case PMULHRSWrr:
		return true;
	}
	return false;
}

bool isMASKMOVQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_MASKMOVQ:
	case MMX_MASKMOVQ64:
		return true;
	}
	return false;
}

bool isVMINSD(unsigned Opcode) {
	switch (Opcode) {
	case VMINSDZrm_Int:
	case VMINSDZrm_Intk:
	case VMINSDZrm_Intkz:
	case VMINSDZrr_Int:
	case VMINSDZrr_Intk:
	case VMINSDZrr_Intkz:
	case VMINSDZrrb_Int:
	case VMINSDZrrb_Intk:
	case VMINSDZrrb_Intkz:
	case VMINSDrm_Int:
	case VMINSDrr_Int:
		return true;
	}
	return false;
}

bool isVMINSH(unsigned Opcode) {
	switch (Opcode) {
	case VMINSHZrm_Int:
	case VMINSHZrm_Intk:
	case VMINSHZrm_Intkz:
	case VMINSHZrr_Int:
	case VMINSHZrr_Intk:
	case VMINSHZrr_Intkz:
	case VMINSHZrrb_Int:
	case VMINSHZrrb_Intk:
	case VMINSHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isRDTSC(unsigned Opcode) {
	return Opcode == RDTSC;
}

bool isVMINSS(unsigned Opcode) {
	switch (Opcode) {
	case VMINSSZrm_Int:
	case VMINSSZrm_Intk:
	case VMINSSZrm_Intkz:
	case VMINSSZrr_Int:
	case VMINSSZrr_Intk:
	case VMINSSZrr_Intkz:
	case VMINSSZrrb_Int:
	case VMINSSZrrb_Intk:
	case VMINSSZrrb_Intkz:
	case VMINSSrm_Int:
	case VMINSSrr_Int:
		return true;
	}
	return false;
}

bool isVRSQRT28SD(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT28SDZm:
	case VRSQRT28SDZmk:
	case VRSQRT28SDZmkz:
	case VRSQRT28SDZr:
	case VRSQRT28SDZrb:
	case VRSQRT28SDZrbk:
	case VRSQRT28SDZrbkz:
	case VRSQRT28SDZrk:
	case VRSQRT28SDZrkz:
		return true;
	}
	return false;
}

bool isAAND(unsigned Opcode) {
	switch (Opcode) {
	case AAND32mr:
	case AAND64mr:
		return true;
	}
	return false;
}

bool isXCRYPTCFB(unsigned Opcode) {
	return Opcode == XCRYPTCFB;
}

bool isVSCALEFPD(unsigned Opcode) {
	switch (Opcode) {
	case VSCALEFPDZ128rm:
	case VSCALEFPDZ128rmb:
	case VSCALEFPDZ128rmbk:
	case VSCALEFPDZ128rmbkz:
	case VSCALEFPDZ128rmk:
	case VSCALEFPDZ128rmkz:
	case VSCALEFPDZ128rr:
	case VSCALEFPDZ128rrk:
	case VSCALEFPDZ128rrkz:
	case VSCALEFPDZ256rm:
	case VSCALEFPDZ256rmb:
	case VSCALEFPDZ256rmbk:
	case VSCALEFPDZ256rmbkz:
	case VSCALEFPDZ256rmk:
	case VSCALEFPDZ256rmkz:
	case VSCALEFPDZ256rr:
	case VSCALEFPDZ256rrk:
	case VSCALEFPDZ256rrkz:
	case VSCALEFPDZrm:
	case VSCALEFPDZrmb:
	case VSCALEFPDZrmbk:
	case VSCALEFPDZrmbkz:
	case VSCALEFPDZrmk:
	case VSCALEFPDZrmkz:
	case VSCALEFPDZrr:
	case VSCALEFPDZrrb:
	case VSCALEFPDZrrbk:
	case VSCALEFPDZrrbkz:
	case VSCALEFPDZrrk:
	case VSCALEFPDZrrkz:
		return true;
	}
	return false;
}

bool isVPBROADCASTB(unsigned Opcode) {
	switch (Opcode) {
	case VPBROADCASTBYrm:
	case VPBROADCASTBYrr:
	case VPBROADCASTBZ128rm:
	case VPBROADCASTBZ128rmk:
	case VPBROADCASTBZ128rmkz:
	case VPBROADCASTBZ128rr:
	case VPBROADCASTBZ128rrk:
	case VPBROADCASTBZ128rrkz:
	case VPBROADCASTBZ256rm:
	case VPBROADCASTBZ256rmk:
	case VPBROADCASTBZ256rmkz:
	case VPBROADCASTBZ256rr:
	case VPBROADCASTBZ256rrk:
	case VPBROADCASTBZ256rrkz:
	case VPBROADCASTBZrm:
	case VPBROADCASTBZrmk:
	case VPBROADCASTBZrmkz:
	case VPBROADCASTBZrr:
	case VPBROADCASTBZrrk:
	case VPBROADCASTBZrrkz:
	case VPBROADCASTBrZ128rr:
	case VPBROADCASTBrZ128rrk:
	case VPBROADCASTBrZ128rrkz:
	case VPBROADCASTBrZ256rr:
	case VPBROADCASTBrZ256rrk:
	case VPBROADCASTBrZ256rrkz:
	case VPBROADCASTBrZrr:
	case VPBROADCASTBrZrrk:
	case VPBROADCASTBrZrrkz:
	case VPBROADCASTBrm:
	case VPBROADCASTBrr:
		return true;
	}
	return false;
}

bool isVSCALEFPH(unsigned Opcode) {
	switch (Opcode) {
	case VSCALEFPHZ128rm:
	case VSCALEFPHZ128rmb:
	case VSCALEFPHZ128rmbk:
	case VSCALEFPHZ128rmbkz:
	case VSCALEFPHZ128rmk:
	case VSCALEFPHZ128rmkz:
	case VSCALEFPHZ128rr:
	case VSCALEFPHZ128rrk:
	case VSCALEFPHZ128rrkz:
	case VSCALEFPHZ256rm:
	case VSCALEFPHZ256rmb:
	case VSCALEFPHZ256rmbk:
	case VSCALEFPHZ256rmbkz:
	case VSCALEFPHZ256rmk:
	case VSCALEFPHZ256rmkz:
	case VSCALEFPHZ256rr:
	case VSCALEFPHZ256rrk:
	case VSCALEFPHZ256rrkz:
	case VSCALEFPHZrm:
	case VSCALEFPHZrmb:
	case VSCALEFPHZrmbk:
	case VSCALEFPHZrmbkz:
	case VSCALEFPHZrmk:
	case VSCALEFPHZrmkz:
	case VSCALEFPHZrr:
	case VSCALEFPHZrrb:
	case VSCALEFPHZrrbk:
	case VSCALEFPHZrrbkz:
	case VSCALEFPHZrrk:
	case VSCALEFPHZrrkz:
		return true;
	}
	return false;
}

bool isVPBROADCASTD(unsigned Opcode) {
	switch (Opcode) {
	case VPBROADCASTDYrm:
	case VPBROADCASTDYrr:
	case VPBROADCASTDZ128rm:
	case VPBROADCASTDZ128rmk:
	case VPBROADCASTDZ128rmkz:
	case VPBROADCASTDZ128rr:
	case VPBROADCASTDZ128rrk:
	case VPBROADCASTDZ128rrkz:
	case VPBROADCASTDZ256rm:
	case VPBROADCASTDZ256rmk:
	case VPBROADCASTDZ256rmkz:
	case VPBROADCASTDZ256rr:
	case VPBROADCASTDZ256rrk:
	case VPBROADCASTDZ256rrkz:
	case VPBROADCASTDZrm:
	case VPBROADCASTDZrmk:
	case VPBROADCASTDZrmkz:
	case VPBROADCASTDZrr:
	case VPBROADCASTDZrrk:
	case VPBROADCASTDZrrkz:
	case VPBROADCASTDrZ128rr:
	case VPBROADCASTDrZ128rrk:
	case VPBROADCASTDrZ128rrkz:
	case VPBROADCASTDrZ256rr:
	case VPBROADCASTDrZ256rrk:
	case VPBROADCASTDrZ256rrkz:
	case VPBROADCASTDrZrr:
	case VPBROADCASTDrZrrk:
	case VPBROADCASTDrZrrkz:
	case VPBROADCASTDrm:
	case VPBROADCASTDrr:
		return true;
	}
	return false;
}

bool isVRSQRT28SS(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT28SSZm:
	case VRSQRT28SSZmk:
	case VRSQRT28SSZmkz:
	case VRSQRT28SSZr:
	case VRSQRT28SSZrb:
	case VRSQRT28SSZrbk:
	case VRSQRT28SSZrbkz:
	case VRSQRT28SSZrk:
	case VRSQRT28SSZrkz:
		return true;
	}
	return false;
}

bool isVPMULLD(unsigned Opcode) {
	switch (Opcode) {
	case VPMULLDYrm:
	case VPMULLDYrr:
	case VPMULLDZ128rm:
	case VPMULLDZ128rmb:
	case VPMULLDZ128rmbk:
	case VPMULLDZ128rmbkz:
	case VPMULLDZ128rmk:
	case VPMULLDZ128rmkz:
	case VPMULLDZ128rr:
	case VPMULLDZ128rrk:
	case VPMULLDZ128rrkz:
	case VPMULLDZ256rm:
	case VPMULLDZ256rmb:
	case VPMULLDZ256rmbk:
	case VPMULLDZ256rmbkz:
	case VPMULLDZ256rmk:
	case VPMULLDZ256rmkz:
	case VPMULLDZ256rr:
	case VPMULLDZ256rrk:
	case VPMULLDZ256rrkz:
	case VPMULLDZrm:
	case VPMULLDZrmb:
	case VPMULLDZrmbk:
	case VPMULLDZrmbkz:
	case VPMULLDZrmk:
	case VPMULLDZrmkz:
	case VPMULLDZrr:
	case VPMULLDZrrk:
	case VPMULLDZrrkz:
	case VPMULLDrm:
	case VPMULLDrr:
		return true;
	}
	return false;
}

bool isVSCALEFPS(unsigned Opcode) {
	switch (Opcode) {
	case VSCALEFPSZ128rm:
	case VSCALEFPSZ128rmb:
	case VSCALEFPSZ128rmbk:
	case VSCALEFPSZ128rmbkz:
	case VSCALEFPSZ128rmk:
	case VSCALEFPSZ128rmkz:
	case VSCALEFPSZ128rr:
	case VSCALEFPSZ128rrk:
	case VSCALEFPSZ128rrkz:
	case VSCALEFPSZ256rm:
	case VSCALEFPSZ256rmb:
	case VSCALEFPSZ256rmbk:
	case VSCALEFPSZ256rmbkz:
	case VSCALEFPSZ256rmk:
	case VSCALEFPSZ256rmkz:
	case VSCALEFPSZ256rr:
	case VSCALEFPSZ256rrk:
	case VSCALEFPSZ256rrkz:
	case VSCALEFPSZrm:
	case VSCALEFPSZrmb:
	case VSCALEFPSZrmbk:
	case VSCALEFPSZrmbkz:
	case VSCALEFPSZrmk:
	case VSCALEFPSZrmkz:
	case VSCALEFPSZrr:
	case VSCALEFPSZrrb:
	case VSCALEFPSZrrbk:
	case VSCALEFPSZrrbkz:
	case VSCALEFPSZrrk:
	case VSCALEFPSZrrkz:
		return true;
	}
	return false;
}

bool isMOVQ2DQ(unsigned Opcode) {
	return Opcode == MMX_MOVQ2DQrr;
}

bool isVPBROADCASTQ(unsigned Opcode) {
	switch (Opcode) {
	case VPBROADCASTQYrm:
	case VPBROADCASTQYrr:
	case VPBROADCASTQZ128rm:
	case VPBROADCASTQZ128rmk:
	case VPBROADCASTQZ128rmkz:
	case VPBROADCASTQZ128rr:
	case VPBROADCASTQZ128rrk:
	case VPBROADCASTQZ128rrkz:
	case VPBROADCASTQZ256rm:
	case VPBROADCASTQZ256rmk:
	case VPBROADCASTQZ256rmkz:
	case VPBROADCASTQZ256rr:
	case VPBROADCASTQZ256rrk:
	case VPBROADCASTQZ256rrkz:
	case VPBROADCASTQZrm:
	case VPBROADCASTQZrmk:
	case VPBROADCASTQZrmkz:
	case VPBROADCASTQZrr:
	case VPBROADCASTQZrrk:
	case VPBROADCASTQZrrkz:
	case VPBROADCASTQrZ128rr:
	case VPBROADCASTQrZ128rrk:
	case VPBROADCASTQrZ128rrkz:
	case VPBROADCASTQrZ256rr:
	case VPBROADCASTQrZ256rrk:
	case VPBROADCASTQrZ256rrkz:
	case VPBROADCASTQrZrr:
	case VPBROADCASTQrZrrk:
	case VPBROADCASTQrZrrkz:
	case VPBROADCASTQrm:
	case VPBROADCASTQrr:
		return true;
	}
	return false;
}

bool isPALIGNR(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PALIGNRrmi:
	case MMX_PALIGNRrri:
	case PALIGNRrmi:
	case PALIGNRrri:
		return true;
	}
	return false;
}

bool isPUSHAL(unsigned Opcode) {
	return Opcode == PUSHA32;
}

bool isVPBROADCASTW(unsigned Opcode) {
	switch (Opcode) {
	case VPBROADCASTWYrm:
	case VPBROADCASTWYrr:
	case VPBROADCASTWZ128rm:
	case VPBROADCASTWZ128rmk:
	case VPBROADCASTWZ128rmkz:
	case VPBROADCASTWZ128rr:
	case VPBROADCASTWZ128rrk:
	case VPBROADCASTWZ128rrkz:
	case VPBROADCASTWZ256rm:
	case VPBROADCASTWZ256rmk:
	case VPBROADCASTWZ256rmkz:
	case VPBROADCASTWZ256rr:
	case VPBROADCASTWZ256rrk:
	case VPBROADCASTWZ256rrkz:
	case VPBROADCASTWZrm:
	case VPBROADCASTWZrmk:
	case VPBROADCASTWZrmkz:
	case VPBROADCASTWZrr:
	case VPBROADCASTWZrrk:
	case VPBROADCASTWZrrkz:
	case VPBROADCASTWrZ128rr:
	case VPBROADCASTWrZ128rrk:
	case VPBROADCASTWrZ128rrkz:
	case VPBROADCASTWrZ256rr:
	case VPBROADCASTWrZ256rrk:
	case VPBROADCASTWrZ256rrkz:
	case VPBROADCASTWrZrr:
	case VPBROADCASTWrZrrk:
	case VPBROADCASTWrZrrkz:
	case VPBROADCASTWrm:
	case VPBROADCASTWrr:
		return true;
	}
	return false;
}

bool isVPMULLQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMULLQZ128rm:
	case VPMULLQZ128rmb:
	case VPMULLQZ128rmbk:
	case VPMULLQZ128rmbkz:
	case VPMULLQZ128rmk:
	case VPMULLQZ128rmkz:
	case VPMULLQZ128rr:
	case VPMULLQZ128rrk:
	case VPMULLQZ128rrkz:
	case VPMULLQZ256rm:
	case VPMULLQZ256rmb:
	case VPMULLQZ256rmbk:
	case VPMULLQZ256rmbkz:
	case VPMULLQZ256rmk:
	case VPMULLQZ256rmkz:
	case VPMULLQZ256rr:
	case VPMULLQZ256rrk:
	case VPMULLQZ256rrkz:
	case VPMULLQZrm:
	case VPMULLQZrmb:
	case VPMULLQZrmbk:
	case VPMULLQZrmbkz:
	case VPMULLQZrmk:
	case VPMULLQZrmkz:
	case VPMULLQZrr:
	case VPMULLQZrrk:
	case VPMULLQZrrkz:
		return true;
	}
	return false;
}

bool isFINCSTP(unsigned Opcode) {
	return Opcode == FINCSTP;
}

bool isPUSHAW(unsigned Opcode) {
	return Opcode == PUSHA16;
}

bool isVPMULLW(unsigned Opcode) {
	switch (Opcode) {
	case VPMULLWYrm:
	case VPMULLWYrr:
	case VPMULLWZ128rm:
	case VPMULLWZ128rmk:
	case VPMULLWZ128rmkz:
	case VPMULLWZ128rr:
	case VPMULLWZ128rrk:
	case VPMULLWZ128rrkz:
	case VPMULLWZ256rm:
	case VPMULLWZ256rmk:
	case VPMULLWZ256rmkz:
	case VPMULLWZ256rr:
	case VPMULLWZ256rrk:
	case VPMULLWZ256rrkz:
	case VPMULLWZrm:
	case VPMULLWZrmk:
	case VPMULLWZrmkz:
	case VPMULLWZrr:
	case VPMULLWZrrk:
	case VPMULLWZrrkz:
	case VPMULLWrm:
	case VPMULLWrr:
		return true;
	}
	return false;
}

bool isPFPNACC(unsigned Opcode) {
	switch (Opcode) {
	case PFPNACCrm:
	case PFPNACCrr:
		return true;
	}
	return false;
}

bool isTESTUI(unsigned Opcode) {
	return Opcode == TESTUI;
}

bool isVPMOVZXWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVZXWDYrm:
	case VPMOVZXWDYrr:
	case VPMOVZXWDZ128rm:
	case VPMOVZXWDZ128rmk:
	case VPMOVZXWDZ128rmkz:
	case VPMOVZXWDZ128rr:
	case VPMOVZXWDZ128rrk:
	case VPMOVZXWDZ128rrkz:
	case VPMOVZXWDZ256rm:
	case VPMOVZXWDZ256rmk:
	case VPMOVZXWDZ256rmkz:
	case VPMOVZXWDZ256rr:
	case VPMOVZXWDZ256rrk:
	case VPMOVZXWDZ256rrkz:
	case VPMOVZXWDZrm:
	case VPMOVZXWDZrmk:
	case VPMOVZXWDZrmkz:
	case VPMOVZXWDZrr:
	case VPMOVZXWDZrrk:
	case VPMOVZXWDZrrkz:
	case VPMOVZXWDrm:
	case VPMOVZXWDrr:
		return true;
	}
	return false;
}

bool isVPDPWSSDS(unsigned Opcode) {
	switch (Opcode) {
	case VPDPWSSDSYrm:
	case VPDPWSSDSYrr:
	case VPDPWSSDSZ128m:
	case VPDPWSSDSZ128mb:
	case VPDPWSSDSZ128mbk:
	case VPDPWSSDSZ128mbkz:
	case VPDPWSSDSZ128mk:
	case VPDPWSSDSZ128mkz:
	case VPDPWSSDSZ128r:
	case VPDPWSSDSZ128rk:
	case VPDPWSSDSZ128rkz:
	case VPDPWSSDSZ256m:
	case VPDPWSSDSZ256mb:
	case VPDPWSSDSZ256mbk:
	case VPDPWSSDSZ256mbkz:
	case VPDPWSSDSZ256mk:
	case VPDPWSSDSZ256mkz:
	case VPDPWSSDSZ256r:
	case VPDPWSSDSZ256rk:
	case VPDPWSSDSZ256rkz:
	case VPDPWSSDSZm:
	case VPDPWSSDSZmb:
	case VPDPWSSDSZmbk:
	case VPDPWSSDSZmbkz:
	case VPDPWSSDSZmk:
	case VPDPWSSDSZmkz:
	case VPDPWSSDSZr:
	case VPDPWSSDSZrk:
	case VPDPWSSDSZrkz:
	case VPDPWSSDSrm:
	case VPDPWSSDSrr:
		return true;
	}
	return false;
}

bool isINVLPG(unsigned Opcode) {
	return Opcode == INVLPG;
}

bool isJCC(unsigned Opcode) {
	switch (Opcode) {
	case JCC_1:
	case JCC_2:
	case JCC_4:
		return true;
	}
	return false;
}

bool isVPMOVZXWQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVZXWQYrm:
	case VPMOVZXWQYrr:
	case VPMOVZXWQZ128rm:
	case VPMOVZXWQZ128rmk:
	case VPMOVZXWQZ128rmkz:
	case VPMOVZXWQZ128rr:
	case VPMOVZXWQZ128rrk:
	case VPMOVZXWQZ128rrkz:
	case VPMOVZXWQZ256rm:
	case VPMOVZXWQZ256rmk:
	case VPMOVZXWQZ256rmkz:
	case VPMOVZXWQZ256rr:
	case VPMOVZXWQZ256rrk:
	case VPMOVZXWQZ256rrkz:
	case VPMOVZXWQZrm:
	case VPMOVZXWQZrmk:
	case VPMOVZXWQZrmkz:
	case VPMOVZXWQZrr:
	case VPMOVZXWQZrrk:
	case VPMOVZXWQZrrkz:
	case VPMOVZXWQrm:
	case VPMOVZXWQrr:
		return true;
	}
	return false;
}

bool isBSF(unsigned Opcode) {
	switch (Opcode) {
	case BSF16rm:
	case BSF16rr:
	case BSF32rm:
	case BSF32rr:
	case BSF64rm:
	case BSF64rr:
		return true;
	}
	return false;
}

bool isROUNDPD(unsigned Opcode) {
	switch (Opcode) {
	case ROUNDPDm:
	case ROUNDPDr:
		return true;
	}
	return false;
}

bool isSAVEPREVSSP(unsigned Opcode) {
	return Opcode == SAVEPREVSSP;
}

bool isVSCATTERDPD(unsigned Opcode) {
	switch (Opcode) {
	case VSCATTERDPDZ128mr:
	case VSCATTERDPDZ256mr:
	case VSCATTERDPDZmr:
		return true;
	}
	return false;
}

bool isBSR(unsigned Opcode) {
	switch (Opcode) {
	case BSR16rm:
	case BSR16rr:
	case BSR32rm:
	case BSR32rr:
	case BSR64rm:
	case BSR64rr:
		return true;
	}
	return false;
}

bool isROUNDPS(unsigned Opcode) {
	switch (Opcode) {
	case ROUNDPSm:
	case ROUNDPSr:
		return true;
	}
	return false;
}

bool isCVTPI2PD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_CVTPI2PDrm:
	case MMX_CVTPI2PDrr:
		return true;
	}
	return false;
}

bool isKTESTB(unsigned Opcode) {
	return Opcode == KTESTBrr;
}

bool isKTESTD(unsigned Opcode) {
	return Opcode == KTESTDrr;
}

bool isVSCATTERDPS(unsigned Opcode) {
	switch (Opcode) {
	case VSCATTERDPSZ128mr:
	case VSCATTERDPSZ256mr:
	case VSCATTERDPSZmr:
		return true;
	}
	return false;
}

bool isFLD(unsigned Opcode) {
	switch (Opcode) {
	case LD_F32m:
	case LD_F64m:
	case LD_F80m:
	case LD_Frr:
		return true;
	}
	return false;
}

bool isBTC(unsigned Opcode) {
	switch (Opcode) {
	case BTC16mi8:
	case BTC16mr:
	case BTC16ri8:
	case BTC16rr:
	case BTC32mi8:
	case BTC32mr:
	case BTC32ri8:
	case BTC32rr:
	case BTC64mi8:
	case BTC64mr:
	case BTC64ri8:
	case BTC64rr:
		return true;
	}
	return false;
}

bool isVBCSTNEBF162PS(unsigned Opcode) {
	switch (Opcode) {
	case VBCSTNEBF162PSYrm:
	case VBCSTNEBF162PSrm:
		return true;
	}
	return false;
}

bool isCVTPI2PS(unsigned Opcode) {
	switch (Opcode) {
	case MMX_CVTPI2PSrm:
	case MMX_CVTPI2PSrr:
		return true;
	}
	return false;
}

bool isKTESTQ(unsigned Opcode) {
	return Opcode == KTESTQrr;
}

bool isVSCALEFSD(unsigned Opcode) {
	switch (Opcode) {
	case VSCALEFSDZrm:
	case VSCALEFSDZrmk:
	case VSCALEFSDZrmkz:
	case VSCALEFSDZrr:
	case VSCALEFSDZrrb_Int:
	case VSCALEFSDZrrb_Intk:
	case VSCALEFSDZrrb_Intkz:
	case VSCALEFSDZrrk:
	case VSCALEFSDZrrkz:
		return true;
	}
	return false;
}

bool isVSCALEFSH(unsigned Opcode) {
	switch (Opcode) {
	case VSCALEFSHZrm:
	case VSCALEFSHZrmk:
	case VSCALEFSHZrmkz:
	case VSCALEFSHZrr:
	case VSCALEFSHZrrb_Int:
	case VSCALEFSHZrrb_Intk:
	case VSCALEFSHZrrb_Intkz:
	case VSCALEFSHZrrk:
	case VSCALEFSHZrrkz:
		return true;
	}
	return false;
}

bool isKTESTW(unsigned Opcode) {
	return Opcode == KTESTWrr;
}

bool isXSTORE(unsigned Opcode) {
	return Opcode == XSTORE;
}

bool isVFMADDSUB132PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB132PDYm:
	case VFMADDSUB132PDYr:
	case VFMADDSUB132PDZ128m:
	case VFMADDSUB132PDZ128mb:
	case VFMADDSUB132PDZ128mbk:
	case VFMADDSUB132PDZ128mbkz:
	case VFMADDSUB132PDZ128mk:
	case VFMADDSUB132PDZ128mkz:
	case VFMADDSUB132PDZ128r:
	case VFMADDSUB132PDZ128rk:
	case VFMADDSUB132PDZ128rkz:
	case VFMADDSUB132PDZ256m:
	case VFMADDSUB132PDZ256mb:
	case VFMADDSUB132PDZ256mbk:
	case VFMADDSUB132PDZ256mbkz:
	case VFMADDSUB132PDZ256mk:
	case VFMADDSUB132PDZ256mkz:
	case VFMADDSUB132PDZ256r:
	case VFMADDSUB132PDZ256rk:
	case VFMADDSUB132PDZ256rkz:
	case VFMADDSUB132PDZm:
	case VFMADDSUB132PDZmb:
	case VFMADDSUB132PDZmbk:
	case VFMADDSUB132PDZmbkz:
	case VFMADDSUB132PDZmk:
	case VFMADDSUB132PDZmkz:
	case VFMADDSUB132PDZr:
	case VFMADDSUB132PDZrb:
	case VFMADDSUB132PDZrbk:
	case VFMADDSUB132PDZrbkz:
	case VFMADDSUB132PDZrk:
	case VFMADDSUB132PDZrkz:
	case VFMADDSUB132PDm:
	case VFMADDSUB132PDr:
		return true;
	}
	return false;
}

bool isVPSRAVD(unsigned Opcode) {
	switch (Opcode) {
	case VPSRAVDYrm:
	case VPSRAVDYrr:
	case VPSRAVDZ128rm:
	case VPSRAVDZ128rmb:
	case VPSRAVDZ128rmbk:
	case VPSRAVDZ128rmbkz:
	case VPSRAVDZ128rmk:
	case VPSRAVDZ128rmkz:
	case VPSRAVDZ128rr:
	case VPSRAVDZ128rrk:
	case VPSRAVDZ128rrkz:
	case VPSRAVDZ256rm:
	case VPSRAVDZ256rmb:
	case VPSRAVDZ256rmbk:
	case VPSRAVDZ256rmbkz:
	case VPSRAVDZ256rmk:
	case VPSRAVDZ256rmkz:
	case VPSRAVDZ256rr:
	case VPSRAVDZ256rrk:
	case VPSRAVDZ256rrkz:
	case VPSRAVDZrm:
	case VPSRAVDZrmb:
	case VPSRAVDZrmbk:
	case VPSRAVDZrmbkz:
	case VPSRAVDZrmk:
	case VPSRAVDZrmkz:
	case VPSRAVDZrr:
	case VPSRAVDZrrk:
	case VPSRAVDZrrkz:
	case VPSRAVDrm:
	case VPSRAVDrr:
		return true;
	}
	return false;
}

bool isBTR(unsigned Opcode) {
	switch (Opcode) {
	case BTR16mi8:
	case BTR16mr:
	case BTR16ri8:
	case BTR16rr:
	case BTR32mi8:
	case BTR32mr:
	case BTR32ri8:
	case BTR32rr:
	case BTR64mi8:
	case BTR64mr:
	case BTR64ri8:
	case BTR64rr:
		return true;
	}
	return false;
}

bool isBTS(unsigned Opcode) {
	switch (Opcode) {
	case BTS16mi8:
	case BTS16mr:
	case BTS16ri8:
	case BTS16rr:
	case BTS32mi8:
	case BTS32mr:
	case BTS32ri8:
	case BTS32rr:
	case BTS64mi8:
	case BTS64mr:
	case BTS64ri8:
	case BTS64rr:
		return true;
	}
	return false;
}

bool isVFMADDSUB132PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB132PHZ128m:
	case VFMADDSUB132PHZ128mb:
	case VFMADDSUB132PHZ128mbk:
	case VFMADDSUB132PHZ128mbkz:
	case VFMADDSUB132PHZ128mk:
	case VFMADDSUB132PHZ128mkz:
	case VFMADDSUB132PHZ128r:
	case VFMADDSUB132PHZ128rk:
	case VFMADDSUB132PHZ128rkz:
	case VFMADDSUB132PHZ256m:
	case VFMADDSUB132PHZ256mb:
	case VFMADDSUB132PHZ256mbk:
	case VFMADDSUB132PHZ256mbkz:
	case VFMADDSUB132PHZ256mk:
	case VFMADDSUB132PHZ256mkz:
	case VFMADDSUB132PHZ256r:
	case VFMADDSUB132PHZ256rk:
	case VFMADDSUB132PHZ256rkz:
	case VFMADDSUB132PHZm:
	case VFMADDSUB132PHZmb:
	case VFMADDSUB132PHZmbk:
	case VFMADDSUB132PHZmbkz:
	case VFMADDSUB132PHZmk:
	case VFMADDSUB132PHZmkz:
	case VFMADDSUB132PHZr:
	case VFMADDSUB132PHZrb:
	case VFMADDSUB132PHZrbk:
	case VFMADDSUB132PHZrbkz:
	case VFMADDSUB132PHZrk:
	case VFMADDSUB132PHZrkz:
		return true;
	}
	return false;
}

bool isLGDT(unsigned Opcode) {
	return Opcode == LGDT64m;
}

bool isVSCALEFSS(unsigned Opcode) {
	switch (Opcode) {
	case VSCALEFSSZrm:
	case VSCALEFSSZrmk:
	case VSCALEFSSZrmkz:
	case VSCALEFSSZrr:
	case VSCALEFSSZrrb_Int:
	case VSCALEFSSZrrb_Intk:
	case VSCALEFSSZrrb_Intkz:
	case VSCALEFSSZrrk:
	case VSCALEFSSZrrkz:
		return true;
	}
	return false;
}

bool isVEXTRACTF128(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTF128mr:
	case VEXTRACTF128rr:
		return true;
	}
	return false;
}

bool isPMOVSXBD(unsigned Opcode) {
	switch (Opcode) {
	case PMOVSXBDrm:
	case PMOVSXBDrr:
		return true;
	}
	return false;
}

bool isKXNORB(unsigned Opcode) {
	return Opcode == KXNORBrr;
}

bool isVFMADDSUB132PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB132PSYm:
	case VFMADDSUB132PSYr:
	case VFMADDSUB132PSZ128m:
	case VFMADDSUB132PSZ128mb:
	case VFMADDSUB132PSZ128mbk:
	case VFMADDSUB132PSZ128mbkz:
	case VFMADDSUB132PSZ128mk:
	case VFMADDSUB132PSZ128mkz:
	case VFMADDSUB132PSZ128r:
	case VFMADDSUB132PSZ128rk:
	case VFMADDSUB132PSZ128rkz:
	case VFMADDSUB132PSZ256m:
	case VFMADDSUB132PSZ256mb:
	case VFMADDSUB132PSZ256mbk:
	case VFMADDSUB132PSZ256mbkz:
	case VFMADDSUB132PSZ256mk:
	case VFMADDSUB132PSZ256mkz:
	case VFMADDSUB132PSZ256r:
	case VFMADDSUB132PSZ256rk:
	case VFMADDSUB132PSZ256rkz:
	case VFMADDSUB132PSZm:
	case VFMADDSUB132PSZmb:
	case VFMADDSUB132PSZmbk:
	case VFMADDSUB132PSZmbkz:
	case VFMADDSUB132PSZmk:
	case VFMADDSUB132PSZmkz:
	case VFMADDSUB132PSZr:
	case VFMADDSUB132PSZrb:
	case VFMADDSUB132PSZrbk:
	case VFMADDSUB132PSZrbkz:
	case VFMADDSUB132PSZrk:
	case VFMADDSUB132PSZrkz:
	case VFMADDSUB132PSm:
	case VFMADDSUB132PSr:
		return true;
	}
	return false;
}

bool isKXNORD(unsigned Opcode) {
	return Opcode == KXNORDrr;
}

bool isXRSTOR(unsigned Opcode) {
	return Opcode == XRSTOR;
}

bool isVPSRAVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSRAVQZ128rm:
	case VPSRAVQZ128rmb:
	case VPSRAVQZ128rmbk:
	case VPSRAVQZ128rmbkz:
	case VPSRAVQZ128rmk:
	case VPSRAVQZ128rmkz:
	case VPSRAVQZ128rr:
	case VPSRAVQZ128rrk:
	case VPSRAVQZ128rrkz:
	case VPSRAVQZ256rm:
	case VPSRAVQZ256rmb:
	case VPSRAVQZ256rmbk:
	case VPSRAVQZ256rmbkz:
	case VPSRAVQZ256rmk:
	case VPSRAVQZ256rmkz:
	case VPSRAVQZ256rr:
	case VPSRAVQZ256rrk:
	case VPSRAVQZ256rrkz:
	case VPSRAVQZrm:
	case VPSRAVQZrmb:
	case VPSRAVQZrmbk:
	case VPSRAVQZrmbkz:
	case VPSRAVQZrmk:
	case VPSRAVQZrmkz:
	case VPSRAVQZrr:
	case VPSRAVQZrrk:
	case VPSRAVQZrrkz:
		return true;
	}
	return false;
}

bool isVPSRAVW(unsigned Opcode) {
	switch (Opcode) {
	case VPSRAVWZ128rm:
	case VPSRAVWZ128rmk:
	case VPSRAVWZ128rmkz:
	case VPSRAVWZ128rr:
	case VPSRAVWZ128rrk:
	case VPSRAVWZ128rrkz:
	case VPSRAVWZ256rm:
	case VPSRAVWZ256rmk:
	case VPSRAVWZ256rmkz:
	case VPSRAVWZ256rr:
	case VPSRAVWZ256rrk:
	case VPSRAVWZ256rrkz:
	case VPSRAVWZrm:
	case VPSRAVWZrmk:
	case VPSRAVWZrmkz:
	case VPSRAVWZrr:
	case VPSRAVWZrrk:
	case VPSRAVWZrrkz:
		return true;
	}
	return false;
}

bool isVPDPWSSD(unsigned Opcode) {
	switch (Opcode) {
	case VPDPWSSDYrm:
	case VPDPWSSDYrr:
	case VPDPWSSDZ128m:
	case VPDPWSSDZ128mb:
	case VPDPWSSDZ128mbk:
	case VPDPWSSDZ128mbkz:
	case VPDPWSSDZ128mk:
	case VPDPWSSDZ128mkz:
	case VPDPWSSDZ128r:
	case VPDPWSSDZ128rk:
	case VPDPWSSDZ128rkz:
	case VPDPWSSDZ256m:
	case VPDPWSSDZ256mb:
	case VPDPWSSDZ256mbk:
	case VPDPWSSDZ256mbkz:
	case VPDPWSSDZ256mk:
	case VPDPWSSDZ256mkz:
	case VPDPWSSDZ256r:
	case VPDPWSSDZ256rk:
	case VPDPWSSDZ256rkz:
	case VPDPWSSDZm:
	case VPDPWSSDZmb:
	case VPDPWSSDZmbk:
	case VPDPWSSDZmbkz:
	case VPDPWSSDZmk:
	case VPDPWSSDZmkz:
	case VPDPWSSDZr:
	case VPDPWSSDZrk:
	case VPDPWSSDZrkz:
	case VPDPWSSDrm:
	case VPDPWSSDrr:
		return true;
	}
	return false;
}

bool isSQRTPD(unsigned Opcode) {
	switch (Opcode) {
	case SQRTPDm:
	case SQRTPDr:
		return true;
	}
	return false;
}

bool isVDPPD(unsigned Opcode) {
	switch (Opcode) {
	case VDPPDrmi:
	case VDPPDrri:
		return true;
	}
	return false;
}

bool isPMOVSXBQ(unsigned Opcode) {
	switch (Opcode) {
	case PMOVSXBQrm:
	case PMOVSXBQrr:
		return true;
	}
	return false;
}

bool isPMADDWD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMADDWDrm:
	case MMX_PMADDWDrr:
	case PMADDWDrm:
	case PMADDWDrr:
		return true;
	}
	return false;
}

bool isKXNORQ(unsigned Opcode) {
	return Opcode == KXNORQrr;
}

bool isSTUI(unsigned Opcode) {
	return Opcode == STUI;
}

bool isINSERTQ(unsigned Opcode) {
	switch (Opcode) {
	case INSERTQ:
	case INSERTQI:
		return true;
	}
	return false;
}

bool isPMOVSXBW(unsigned Opcode) {
	switch (Opcode) {
	case PMOVSXBWrm:
	case PMOVSXBWrr:
		return true;
	}
	return false;
}

bool isVCVTSI2SD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSI2SDZrm_Int:
	case VCVTSI2SDZrr_Int:
	case VCVTSI2SDrm_Int:
	case VCVTSI2SDrr_Int:
	case VCVTSI642SDZrm_Int:
	case VCVTSI642SDZrr_Int:
	case VCVTSI642SDZrrb_Int:
	case VCVTSI642SDrm_Int:
	case VCVTSI642SDrr_Int:
		return true;
	}
	return false;
}

bool isKXNORW(unsigned Opcode) {
	return Opcode == KXNORWrr;
}

bool isVSCATTERPF0DPD(unsigned Opcode) {
	return Opcode == VSCATTERPF0DPDm;
}

bool isVCVTSI2SH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSI2SHZrm_Int:
	case VCVTSI2SHZrr_Int:
	case VCVTSI2SHZrrb_Int:
	case VCVTSI642SHZrm_Int:
	case VCVTSI642SHZrr_Int:
	case VCVTSI642SHZrrb_Int:
		return true;
	}
	return false;
}

bool isSQRTPS(unsigned Opcode) {
	switch (Opcode) {
	case SQRTPSm:
	case SQRTPSr:
		return true;
	}
	return false;
}

bool isVDPPS(unsigned Opcode) {
	switch (Opcode) {
	case VDPPSYrmi:
	case VDPPSYrri:
	case VDPPSrmi:
	case VDPPSrri:
		return true;
	}
	return false;
}

bool isPSUBSB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBSBrm:
	case MMX_PSUBSBrr:
	case PSUBSBrm:
	case PSUBSBrr:
		return true;
	}
	return false;
}

bool isVPSHRDVD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHRDVDZ128m:
	case VPSHRDVDZ128mb:
	case VPSHRDVDZ128mbk:
	case VPSHRDVDZ128mbkz:
	case VPSHRDVDZ128mk:
	case VPSHRDVDZ128mkz:
	case VPSHRDVDZ128r:
	case VPSHRDVDZ128rk:
	case VPSHRDVDZ128rkz:
	case VPSHRDVDZ256m:
	case VPSHRDVDZ256mb:
	case VPSHRDVDZ256mbk:
	case VPSHRDVDZ256mbkz:
	case VPSHRDVDZ256mk:
	case VPSHRDVDZ256mkz:
	case VPSHRDVDZ256r:
	case VPSHRDVDZ256rk:
	case VPSHRDVDZ256rkz:
	case VPSHRDVDZm:
	case VPSHRDVDZmb:
	case VPSHRDVDZmbk:
	case VPSHRDVDZmbkz:
	case VPSHRDVDZmk:
	case VPSHRDVDZmkz:
	case VPSHRDVDZr:
	case VPSHRDVDZrk:
	case VPSHRDVDZrkz:
		return true;
	}
	return false;
}

bool isFIST(unsigned Opcode) {
	switch (Opcode) {
	case IST_F16m:
	case IST_F32m:
		return true;
	}
	return false;
}

bool isVCVTSI2SS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSI2SSZrm_Int:
	case VCVTSI2SSZrr_Int:
	case VCVTSI2SSZrrb_Int:
	case VCVTSI2SSrm_Int:
	case VCVTSI2SSrr_Int:
	case VCVTSI642SSZrm_Int:
	case VCVTSI642SSZrr_Int:
	case VCVTSI642SSZrrb_Int:
	case VCVTSI642SSrm_Int:
	case VCVTSI642SSrr_Int:
		return true;
	}
	return false;
}

bool isVSCATTERPF0DPS(unsigned Opcode) {
	return Opcode == VSCATTERPF0DPSm;
}

bool isVMOVNTPD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVNTPDYmr:
	case VMOVNTPDZ128mr:
	case VMOVNTPDZ256mr:
	case VMOVNTPDZmr:
	case VMOVNTPDmr:
		return true;
	}
	return false;
}

bool isROUNDSD(unsigned Opcode) {
	switch (Opcode) {
	case ROUNDSDm_Int:
	case ROUNDSDr_Int:
		return true;
	}
	return false;
}

bool isVPSHRDVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSHRDVQZ128m:
	case VPSHRDVQZ128mb:
	case VPSHRDVQZ128mbk:
	case VPSHRDVQZ128mbkz:
	case VPSHRDVQZ128mk:
	case VPSHRDVQZ128mkz:
	case VPSHRDVQZ128r:
	case VPSHRDVQZ128rk:
	case VPSHRDVQZ128rkz:
	case VPSHRDVQZ256m:
	case VPSHRDVQZ256mb:
	case VPSHRDVQZ256mbk:
	case VPSHRDVQZ256mbkz:
	case VPSHRDVQZ256mk:
	case VPSHRDVQZ256mkz:
	case VPSHRDVQZ256r:
	case VPSHRDVQZ256rk:
	case VPSHRDVQZ256rkz:
	case VPSHRDVQZm:
	case VPSHRDVQZmb:
	case VPSHRDVQZmbk:
	case VPSHRDVQZmbkz:
	case VPSHRDVQZmk:
	case VPSHRDVQZmkz:
	case VPSHRDVQZr:
	case VPSHRDVQZrk:
	case VPSHRDVQZrkz:
		return true;
	}
	return false;
}

bool isPSIGNB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSIGNBrm:
	case MMX_PSIGNBrr:
	case PSIGNBrm:
	case PSIGNBrr:
		return true;
	}
	return false;
}

bool isJECXZ(unsigned Opcode) {
	return Opcode == JECXZ;
}

bool isPSIGND(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSIGNDrm:
	case MMX_PSIGNDrr:
	case PSIGNDrm:
	case PSIGNDrr:
		return true;
	}
	return false;
}

bool isPSUBSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBSWrm:
	case MMX_PSUBSWrr:
	case PSUBSWrm:
	case PSUBSWrr:
		return true;
	}
	return false;
}

bool isMOVDQ2Q(unsigned Opcode) {
	return Opcode == MMX_MOVDQ2Qrr;
}

bool isPUSHFD(unsigned Opcode) {
	return Opcode == PUSHF32;
}

bool isVMOVNTPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVNTPSYmr:
	case VMOVNTPSZ128mr:
	case VMOVNTPSZ256mr:
	case VMOVNTPSZmr:
	case VMOVNTPSmr:
		return true;
	}
	return false;
}

bool isROUNDSS(unsigned Opcode) {
	switch (Opcode) {
	case ROUNDSSm_Int:
	case ROUNDSSr_Int:
		return true;
	}
	return false;
}

bool isVAESDECLAST(unsigned Opcode) {
	switch (Opcode) {
	case VAESDECLASTYrm:
	case VAESDECLASTYrr:
	case VAESDECLASTZ128rm:
	case VAESDECLASTZ128rr:
	case VAESDECLASTZ256rm:
	case VAESDECLASTZ256rr:
	case VAESDECLASTZrm:
	case VAESDECLASTZrr:
	case VAESDECLASTrm:
	case VAESDECLASTrr:
		return true;
	}
	return false;
}

bool isPAVGB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PAVGBrm:
	case MMX_PAVGBrr:
	case PAVGBrm:
	case PAVGBrr:
		return true;
	}
	return false;
}

bool isVPSUBB(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBBYrm:
	case VPSUBBYrr:
	case VPSUBBZ128rm:
	case VPSUBBZ128rmk:
	case VPSUBBZ128rmkz:
	case VPSUBBZ128rr:
	case VPSUBBZ128rrk:
	case VPSUBBZ128rrkz:
	case VPSUBBZ256rm:
	case VPSUBBZ256rmk:
	case VPSUBBZ256rmkz:
	case VPSUBBZ256rr:
	case VPSUBBZ256rrk:
	case VPSUBBZ256rrkz:
	case VPSUBBZrm:
	case VPSUBBZrmk:
	case VPSUBBZrmkz:
	case VPSUBBZrr:
	case VPSUBBZrrk:
	case VPSUBBZrrkz:
	case VPSUBBrm:
	case VPSUBBrr:
		return true;
	}
	return false;
}

bool isVPSHRDVW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHRDVWZ128m:
	case VPSHRDVWZ128mk:
	case VPSHRDVWZ128mkz:
	case VPSHRDVWZ128r:
	case VPSHRDVWZ128rk:
	case VPSHRDVWZ128rkz:
	case VPSHRDVWZ256m:
	case VPSHRDVWZ256mk:
	case VPSHRDVWZ256mkz:
	case VPSHRDVWZ256r:
	case VPSHRDVWZ256rk:
	case VPSHRDVWZ256rkz:
	case VPSHRDVWZm:
	case VPSHRDVWZmk:
	case VPSHRDVWZmkz:
	case VPSHRDVWZr:
	case VPSHRDVWZrk:
	case VPSHRDVWZrkz:
		return true;
	}
	return false;
}

bool isVPSUBD(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBDYrm:
	case VPSUBDYrr:
	case VPSUBDZ128rm:
	case VPSUBDZ128rmb:
	case VPSUBDZ128rmbk:
	case VPSUBDZ128rmbkz:
	case VPSUBDZ128rmk:
	case VPSUBDZ128rmkz:
	case VPSUBDZ128rr:
	case VPSUBDZ128rrk:
	case VPSUBDZ128rrkz:
	case VPSUBDZ256rm:
	case VPSUBDZ256rmb:
	case VPSUBDZ256rmbk:
	case VPSUBDZ256rmbkz:
	case VPSUBDZ256rmk:
	case VPSUBDZ256rmkz:
	case VPSUBDZ256rr:
	case VPSUBDZ256rrk:
	case VPSUBDZ256rrkz:
	case VPSUBDZrm:
	case VPSUBDZrmb:
	case VPSUBDZrmbk:
	case VPSUBDZrmbkz:
	case VPSUBDZrmk:
	case VPSUBDZrmkz:
	case VPSUBDZrr:
	case VPSUBDZrrk:
	case VPSUBDZrrkz:
	case VPSUBDrm:
	case VPSUBDrr:
		return true;
	}
	return false;
}

bool isPUSHFQ(unsigned Opcode) {
	return Opcode == PUSHF64;
}

bool isPSIGNW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSIGNWrm:
	case MMX_PSIGNWrr:
	case PSIGNWrm:
	case PSIGNWrr:
		return true;
	}
	return false;
}

bool isVBROADCASTSD(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTSDYrm:
	case VBROADCASTSDYrr:
	case VBROADCASTSDZ256rm:
	case VBROADCASTSDZ256rmk:
	case VBROADCASTSDZ256rmkz:
	case VBROADCASTSDZ256rr:
	case VBROADCASTSDZ256rrk:
	case VBROADCASTSDZ256rrkz:
	case VBROADCASTSDZrm:
	case VBROADCASTSDZrmk:
	case VBROADCASTSDZrmkz:
	case VBROADCASTSDZrr:
	case VBROADCASTSDZrrk:
	case VBROADCASTSDZrrkz:
		return true;
	}
	return false;
}

bool isVPSUBQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBQYrm:
	case VPSUBQYrr:
	case VPSUBQZ128rm:
	case VPSUBQZ128rmb:
	case VPSUBQZ128rmbk:
	case VPSUBQZ128rmbkz:
	case VPSUBQZ128rmk:
	case VPSUBQZ128rmkz:
	case VPSUBQZ128rr:
	case VPSUBQZ128rrk:
	case VPSUBQZ128rrkz:
	case VPSUBQZ256rm:
	case VPSUBQZ256rmb:
	case VPSUBQZ256rmbk:
	case VPSUBQZ256rmbkz:
	case VPSUBQZ256rmk:
	case VPSUBQZ256rmkz:
	case VPSUBQZ256rr:
	case VPSUBQZ256rrk:
	case VPSUBQZ256rrkz:
	case VPSUBQZrm:
	case VPSUBQZrmb:
	case VPSUBQZrmbk:
	case VPSUBQZrmbkz:
	case VPSUBQZrmk:
	case VPSUBQZrmkz:
	case VPSUBQZrr:
	case VPSUBQZrrk:
	case VPSUBQZrrkz:
	case VPSUBQrm:
	case VPSUBQrr:
		return true;
	}
	return false;
}

bool isPMOVSXDQ(unsigned Opcode) {
	switch (Opcode) {
	case PMOVSXDQrm:
	case PMOVSXDQrr:
		return true;
	}
	return false;
}

bool isPAVGW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PAVGWrm:
	case MMX_PAVGWrr:
	case PAVGWrm:
	case PAVGWrr:
		return true;
	}
	return false;
}

bool isVFMSUBADD132PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD132PDYm:
	case VFMSUBADD132PDYr:
	case VFMSUBADD132PDZ128m:
	case VFMSUBADD132PDZ128mb:
	case VFMSUBADD132PDZ128mbk:
	case VFMSUBADD132PDZ128mbkz:
	case VFMSUBADD132PDZ128mk:
	case VFMSUBADD132PDZ128mkz:
	case VFMSUBADD132PDZ128r:
	case VFMSUBADD132PDZ128rk:
	case VFMSUBADD132PDZ128rkz:
	case VFMSUBADD132PDZ256m:
	case VFMSUBADD132PDZ256mb:
	case VFMSUBADD132PDZ256mbk:
	case VFMSUBADD132PDZ256mbkz:
	case VFMSUBADD132PDZ256mk:
	case VFMSUBADD132PDZ256mkz:
	case VFMSUBADD132PDZ256r:
	case VFMSUBADD132PDZ256rk:
	case VFMSUBADD132PDZ256rkz:
	case VFMSUBADD132PDZm:
	case VFMSUBADD132PDZmb:
	case VFMSUBADD132PDZmbk:
	case VFMSUBADD132PDZmbkz:
	case VFMSUBADD132PDZmk:
	case VFMSUBADD132PDZmkz:
	case VFMSUBADD132PDZr:
	case VFMSUBADD132PDZrb:
	case VFMSUBADD132PDZrbk:
	case VFMSUBADD132PDZrbkz:
	case VFMSUBADD132PDZrk:
	case VFMSUBADD132PDZrkz:
	case VFMSUBADD132PDm:
	case VFMSUBADD132PDr:
		return true;
	}
	return false;
}

bool isVPSUBW(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBWYrm:
	case VPSUBWYrr:
	case VPSUBWZ128rm:
	case VPSUBWZ128rmk:
	case VPSUBWZ128rmkz:
	case VPSUBWZ128rr:
	case VPSUBWZ128rrk:
	case VPSUBWZ128rrkz:
	case VPSUBWZ256rm:
	case VPSUBWZ256rmk:
	case VPSUBWZ256rmkz:
	case VPSUBWZ256rr:
	case VPSUBWZ256rrk:
	case VPSUBWZ256rrkz:
	case VPSUBWZrm:
	case VPSUBWZrmk:
	case VPSUBWZrmkz:
	case VPSUBWZrr:
	case VPSUBWZrrk:
	case VPSUBWZrrkz:
	case VPSUBWrm:
	case VPSUBWrr:
		return true;
	}
	return false;
}

bool isVCVTPS2PHX(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2PHXZ128rm:
	case VCVTPS2PHXZ128rmb:
	case VCVTPS2PHXZ128rmbk:
	case VCVTPS2PHXZ128rmbkz:
	case VCVTPS2PHXZ128rmk:
	case VCVTPS2PHXZ128rmkz:
	case VCVTPS2PHXZ128rr:
	case VCVTPS2PHXZ128rrk:
	case VCVTPS2PHXZ128rrkz:
	case VCVTPS2PHXZ256rm:
	case VCVTPS2PHXZ256rmb:
	case VCVTPS2PHXZ256rmbk:
	case VCVTPS2PHXZ256rmbkz:
	case VCVTPS2PHXZ256rmk:
	case VCVTPS2PHXZ256rmkz:
	case VCVTPS2PHXZ256rr:
	case VCVTPS2PHXZ256rrk:
	case VCVTPS2PHXZ256rrkz:
	case VCVTPS2PHXZrm:
	case VCVTPS2PHXZrmb:
	case VCVTPS2PHXZrmbk:
	case VCVTPS2PHXZrmbkz:
	case VCVTPS2PHXZrmk:
	case VCVTPS2PHXZrmkz:
	case VCVTPS2PHXZrr:
	case VCVTPS2PHXZrrb:
	case VCVTPS2PHXZrrbk:
	case VCVTPS2PHXZrrbkz:
	case VCVTPS2PHXZrrk:
	case VCVTPS2PHXZrrkz:
		return true;
	}
	return false;
}

bool isMOVHLPS(unsigned Opcode) {
	return Opcode == MOVHLPSrr;
}

bool isVFMSUBADD132PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD132PHZ128m:
	case VFMSUBADD132PHZ128mb:
	case VFMSUBADD132PHZ128mbk:
	case VFMSUBADD132PHZ128mbkz:
	case VFMSUBADD132PHZ128mk:
	case VFMSUBADD132PHZ128mkz:
	case VFMSUBADD132PHZ128r:
	case VFMSUBADD132PHZ128rk:
	case VFMSUBADD132PHZ128rkz:
	case VFMSUBADD132PHZ256m:
	case VFMSUBADD132PHZ256mb:
	case VFMSUBADD132PHZ256mbk:
	case VFMSUBADD132PHZ256mbkz:
	case VFMSUBADD132PHZ256mk:
	case VFMSUBADD132PHZ256mkz:
	case VFMSUBADD132PHZ256r:
	case VFMSUBADD132PHZ256rk:
	case VFMSUBADD132PHZ256rkz:
	case VFMSUBADD132PHZm:
	case VFMSUBADD132PHZmb:
	case VFMSUBADD132PHZmbk:
	case VFMSUBADD132PHZmbkz:
	case VFMSUBADD132PHZmk:
	case VFMSUBADD132PHZmkz:
	case VFMSUBADD132PHZr:
	case VFMSUBADD132PHZrb:
	case VFMSUBADD132PHZrbk:
	case VFMSUBADD132PHZrbkz:
	case VFMSUBADD132PHZrk:
	case VFMSUBADD132PHZrkz:
		return true;
	}
	return false;
}

bool isVSHUFF64X2(unsigned Opcode) {
	switch (Opcode) {
	case VSHUFF64X2Z256rmbi:
	case VSHUFF64X2Z256rmbik:
	case VSHUFF64X2Z256rmbikz:
	case VSHUFF64X2Z256rmi:
	case VSHUFF64X2Z256rmik:
	case VSHUFF64X2Z256rmikz:
	case VSHUFF64X2Z256rri:
	case VSHUFF64X2Z256rrik:
	case VSHUFF64X2Z256rrikz:
	case VSHUFF64X2Zrmbi:
	case VSHUFF64X2Zrmbik:
	case VSHUFF64X2Zrmbikz:
	case VSHUFF64X2Zrmi:
	case VSHUFF64X2Zrmik:
	case VSHUFF64X2Zrmikz:
	case VSHUFF64X2Zrri:
	case VSHUFF64X2Zrrik:
	case VSHUFF64X2Zrrikz:
		return true;
	}
	return false;
}

bool isVBROADCASTSS(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTSSYrm:
	case VBROADCASTSSYrr:
	case VBROADCASTSSZ128rm:
	case VBROADCASTSSZ128rmk:
	case VBROADCASTSSZ128rmkz:
	case VBROADCASTSSZ128rr:
	case VBROADCASTSSZ128rrk:
	case VBROADCASTSSZ128rrkz:
	case VBROADCASTSSZ256rm:
	case VBROADCASTSSZ256rmk:
	case VBROADCASTSSZ256rmkz:
	case VBROADCASTSSZ256rr:
	case VBROADCASTSSZ256rrk:
	case VBROADCASTSSZ256rrkz:
	case VBROADCASTSSZrm:
	case VBROADCASTSSZrmk:
	case VBROADCASTSSZrmkz:
	case VBROADCASTSSZrr:
	case VBROADCASTSSZrrk:
	case VBROADCASTSSZrrkz:
	case VBROADCASTSSrm:
	case VBROADCASTSSrr:
		return true;
	}
	return false;
}

bool isVFMSUBADD132PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD132PSYm:
	case VFMSUBADD132PSYr:
	case VFMSUBADD132PSZ128m:
	case VFMSUBADD132PSZ128mb:
	case VFMSUBADD132PSZ128mbk:
	case VFMSUBADD132PSZ128mbkz:
	case VFMSUBADD132PSZ128mk:
	case VFMSUBADD132PSZ128mkz:
	case VFMSUBADD132PSZ128r:
	case VFMSUBADD132PSZ128rk:
	case VFMSUBADD132PSZ128rkz:
	case VFMSUBADD132PSZ256m:
	case VFMSUBADD132PSZ256mb:
	case VFMSUBADD132PSZ256mbk:
	case VFMSUBADD132PSZ256mbkz:
	case VFMSUBADD132PSZ256mk:
	case VFMSUBADD132PSZ256mkz:
	case VFMSUBADD132PSZ256r:
	case VFMSUBADD132PSZ256rk:
	case VFMSUBADD132PSZ256rkz:
	case VFMSUBADD132PSZm:
	case VFMSUBADD132PSZmb:
	case VFMSUBADD132PSZmbk:
	case VFMSUBADD132PSZmbkz:
	case VFMSUBADD132PSZmk:
	case VFMSUBADD132PSZmkz:
	case VFMSUBADD132PSZr:
	case VFMSUBADD132PSZrb:
	case VFMSUBADD132PSZrbk:
	case VFMSUBADD132PSZrbkz:
	case VFMSUBADD132PSZrk:
	case VFMSUBADD132PSZrkz:
	case VFMSUBADD132PSm:
	case VFMSUBADD132PSr:
		return true;
	}
	return false;
}

bool isMAXPD(unsigned Opcode) {
	switch (Opcode) {
	case MAXPDrm:
	case MAXPDrr:
		return true;
	}
	return false;
}

bool isSQRTSD(unsigned Opcode) {
	switch (Opcode) {
	case SQRTSDm_Int:
	case SQRTSDr_Int:
		return true;
	}
	return false;
}

bool isCVTPD2DQ(unsigned Opcode) {
	switch (Opcode) {
	case CVTPD2DQrm:
	case CVTPD2DQrr:
		return true;
	}
	return false;
}

bool isMAXPS(unsigned Opcode) {
	switch (Opcode) {
	case MAXPSrm:
	case MAXPSrr:
		return true;
	}
	return false;
}

bool isVPMOVQB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVQBZ128mr:
	case VPMOVQBZ128mrk:
	case VPMOVQBZ128rr:
	case VPMOVQBZ128rrk:
	case VPMOVQBZ128rrkz:
	case VPMOVQBZ256mr:
	case VPMOVQBZ256mrk:
	case VPMOVQBZ256rr:
	case VPMOVQBZ256rrk:
	case VPMOVQBZ256rrkz:
	case VPMOVQBZmr:
	case VPMOVQBZmrk:
	case VPMOVQBZrr:
	case VPMOVQBZrrk:
	case VPMOVQBZrrkz:
		return true;
	}
	return false;
}

bool isINVPCID(unsigned Opcode) {
	switch (Opcode) {
	case INVPCID32:
	case INVPCID64:
		return true;
	}
	return false;
}

bool isVCVTPS2DQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2DQYrm:
	case VCVTPS2DQYrr:
	case VCVTPS2DQZ128rm:
	case VCVTPS2DQZ128rmb:
	case VCVTPS2DQZ128rmbk:
	case VCVTPS2DQZ128rmbkz:
	case VCVTPS2DQZ128rmk:
	case VCVTPS2DQZ128rmkz:
	case VCVTPS2DQZ128rr:
	case VCVTPS2DQZ128rrk:
	case VCVTPS2DQZ128rrkz:
	case VCVTPS2DQZ256rm:
	case VCVTPS2DQZ256rmb:
	case VCVTPS2DQZ256rmbk:
	case VCVTPS2DQZ256rmbkz:
	case VCVTPS2DQZ256rmk:
	case VCVTPS2DQZ256rmkz:
	case VCVTPS2DQZ256rr:
	case VCVTPS2DQZ256rrk:
	case VCVTPS2DQZ256rrkz:
	case VCVTPS2DQZrm:
	case VCVTPS2DQZrmb:
	case VCVTPS2DQZrmbk:
	case VCVTPS2DQZrmbkz:
	case VCVTPS2DQZrmk:
	case VCVTPS2DQZrmkz:
	case VCVTPS2DQZrr:
	case VCVTPS2DQZrrb:
	case VCVTPS2DQZrrbk:
	case VCVTPS2DQZrrbkz:
	case VCVTPS2DQZrrk:
	case VCVTPS2DQZrrkz:
	case VCVTPS2DQrm:
	case VCVTPS2DQrr:
		return true;
	}
	return false;
}

bool isSQRTSS(unsigned Opcode) {
	switch (Opcode) {
	case SQRTSSm_Int:
	case SQRTSSr_Int:
		return true;
	}
	return false;
}

bool isVADDPD(unsigned Opcode) {
	switch (Opcode) {
	case VADDPDYrm:
	case VADDPDYrr:
	case VADDPDZ128rm:
	case VADDPDZ128rmb:
	case VADDPDZ128rmbk:
	case VADDPDZ128rmbkz:
	case VADDPDZ128rmk:
	case VADDPDZ128rmkz:
	case VADDPDZ128rr:
	case VADDPDZ128rrk:
	case VADDPDZ128rrkz:
	case VADDPDZ256rm:
	case VADDPDZ256rmb:
	case VADDPDZ256rmbk:
	case VADDPDZ256rmbkz:
	case VADDPDZ256rmk:
	case VADDPDZ256rmkz:
	case VADDPDZ256rr:
	case VADDPDZ256rrk:
	case VADDPDZ256rrkz:
	case VADDPDZrm:
	case VADDPDZrmb:
	case VADDPDZrmbk:
	case VADDPDZrmbkz:
	case VADDPDZrmk:
	case VADDPDZrmkz:
	case VADDPDZrr:
	case VADDPDZrrb:
	case VADDPDZrrbk:
	case VADDPDZrrbkz:
	case VADDPDZrrk:
	case VADDPDZrrkz:
	case VADDPDrm:
	case VADDPDrr:
		return true;
	}
	return false;
}

bool isLODSB(unsigned Opcode) {
	return Opcode == LODSB;
}

bool isLODSD(unsigned Opcode) {
	return Opcode == LODSL;
}

bool isV4FNMADDPS(unsigned Opcode) {
	switch (Opcode) {
	case V4FNMADDPSrm:
	case V4FNMADDPSrmk:
	case V4FNMADDPSrmkz:
		return true;
	}
	return false;
}

bool isIRETD(unsigned Opcode) {
	return Opcode == IRET32;
}

bool isVADDPH(unsigned Opcode) {
	switch (Opcode) {
	case VADDPHZ128rm:
	case VADDPHZ128rmb:
	case VADDPHZ128rmbk:
	case VADDPHZ128rmbkz:
	case VADDPHZ128rmk:
	case VADDPHZ128rmkz:
	case VADDPHZ128rr:
	case VADDPHZ128rrk:
	case VADDPHZ128rrkz:
	case VADDPHZ256rm:
	case VADDPHZ256rmb:
	case VADDPHZ256rmbk:
	case VADDPHZ256rmbkz:
	case VADDPHZ256rmk:
	case VADDPHZ256rmkz:
	case VADDPHZ256rr:
	case VADDPHZ256rrk:
	case VADDPHZ256rrkz:
	case VADDPHZrm:
	case VADDPHZrmb:
	case VADDPHZrmbk:
	case VADDPHZrmbkz:
	case VADDPHZrmk:
	case VADDPHZrmkz:
	case VADDPHZrr:
	case VADDPHZrrb:
	case VADDPHZrrbk:
	case VADDPHZrrbkz:
	case VADDPHZrrk:
	case VADDPHZrrkz:
		return true;
	}
	return false;
}

bool isVHADDPD(unsigned Opcode) {
	switch (Opcode) {
	case VHADDPDYrm:
	case VHADDPDYrr:
	case VHADDPDrm:
	case VHADDPDrr:
		return true;
	}
	return false;
}

bool isVPMOVQD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVQDZ128mr:
	case VPMOVQDZ128mrk:
	case VPMOVQDZ128rr:
	case VPMOVQDZ128rrk:
	case VPMOVQDZ128rrkz:
	case VPMOVQDZ256mr:
	case VPMOVQDZ256mrk:
	case VPMOVQDZ256rr:
	case VPMOVQDZ256rrk:
	case VPMOVQDZ256rrkz:
	case VPMOVQDZmr:
	case VPMOVQDZmrk:
	case VPMOVQDZrr:
	case VPMOVQDZrrk:
	case VPMOVQDZrrkz:
		return true;
	}
	return false;
}

bool isVPSUBUSB(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBUSBYrm:
	case VPSUBUSBYrr:
	case VPSUBUSBZ128rm:
	case VPSUBUSBZ128rmk:
	case VPSUBUSBZ128rmkz:
	case VPSUBUSBZ128rr:
	case VPSUBUSBZ128rrk:
	case VPSUBUSBZ128rrkz:
	case VPSUBUSBZ256rm:
	case VPSUBUSBZ256rmk:
	case VPSUBUSBZ256rmkz:
	case VPSUBUSBZ256rr:
	case VPSUBUSBZ256rrk:
	case VPSUBUSBZ256rrkz:
	case VPSUBUSBZrm:
	case VPSUBUSBZrmk:
	case VPSUBUSBZrmkz:
	case VPSUBUSBZrr:
	case VPSUBUSBZrrk:
	case VPSUBUSBZrrkz:
	case VPSUBUSBrm:
	case VPSUBUSBrr:
		return true;
	}
	return false;
}

bool isVPMOVQW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVQWZ128mr:
	case VPMOVQWZ128mrk:
	case VPMOVQWZ128rr:
	case VPMOVQWZ128rrk:
	case VPMOVQWZ128rrkz:
	case VPMOVQWZ256mr:
	case VPMOVQWZ256mrk:
	case VPMOVQWZ256rr:
	case VPMOVQWZ256rrk:
	case VPMOVQWZ256rrkz:
	case VPMOVQWZmr:
	case VPMOVQWZmrk:
	case VPMOVQWZrr:
	case VPMOVQWZrrk:
	case VPMOVQWZrrkz:
		return true;
	}
	return false;
}

bool isVADDPS(unsigned Opcode) {
	switch (Opcode) {
	case VADDPSYrm:
	case VADDPSYrr:
	case VADDPSZ128rm:
	case VADDPSZ128rmb:
	case VADDPSZ128rmbk:
	case VADDPSZ128rmbkz:
	case VADDPSZ128rmk:
	case VADDPSZ128rmkz:
	case VADDPSZ128rr:
	case VADDPSZ128rrk:
	case VADDPSZ128rrkz:
	case VADDPSZ256rm:
	case VADDPSZ256rmb:
	case VADDPSZ256rmbk:
	case VADDPSZ256rmbkz:
	case VADDPSZ256rmk:
	case VADDPSZ256rmkz:
	case VADDPSZ256rr:
	case VADDPSZ256rrk:
	case VADDPSZ256rrkz:
	case VADDPSZrm:
	case VADDPSZrmb:
	case VADDPSZrmbk:
	case VADDPSZrmbkz:
	case VADDPSZrmk:
	case VADDPSZrmkz:
	case VADDPSZrr:
	case VADDPSZrrb:
	case VADDPSZrrbk:
	case VADDPSZrrbkz:
	case VADDPSZrrk:
	case VADDPSZrrkz:
	case VADDPSrm:
	case VADDPSrr:
		return true;
	}
	return false;
}

bool isLODSQ(unsigned Opcode) {
	return Opcode == LODSQ;
}

bool isFIDIVR(unsigned Opcode) {
	switch (Opcode) {
	case DIVR_FI16m:
	case DIVR_FI32m:
		return true;
	}
	return false;
}

bool isIRETQ(unsigned Opcode) {
	return Opcode == IRET64;
}

bool isLODSW(unsigned Opcode) {
	return Opcode == LODSW;
}

bool isVHADDPS(unsigned Opcode) {
	switch (Opcode) {
	case VHADDPSYrm:
	case VHADDPSYrr:
	case VHADDPSrm:
	case VHADDPSrr:
		return true;
	}
	return false;
}

bool isCVTTPS2DQ(unsigned Opcode) {
	switch (Opcode) {
	case CVTTPS2DQrm:
	case CVTTPS2DQrr:
		return true;
	}
	return false;
}

bool isVPORD(unsigned Opcode) {
	switch (Opcode) {
	case VPORDZ128rm:
	case VPORDZ128rmb:
	case VPORDZ128rmbk:
	case VPORDZ128rmbkz:
	case VPORDZ128rmk:
	case VPORDZ128rmkz:
	case VPORDZ128rr:
	case VPORDZ128rrk:
	case VPORDZ128rrkz:
	case VPORDZ256rm:
	case VPORDZ256rmb:
	case VPORDZ256rmbk:
	case VPORDZ256rmbkz:
	case VPORDZ256rmk:
	case VPORDZ256rmkz:
	case VPORDZ256rr:
	case VPORDZ256rrk:
	case VPORDZ256rrkz:
	case VPORDZrm:
	case VPORDZrmb:
	case VPORDZrmbk:
	case VPORDZrmbkz:
	case VPORDZrmk:
	case VPORDZrmkz:
	case VPORDZrr:
	case VPORDZrrk:
	case VPORDZrrkz:
		return true;
	}
	return false;
}

bool isVP2INTERSECTD(unsigned Opcode) {
	switch (Opcode) {
	case VP2INTERSECTDZ128rm:
	case VP2INTERSECTDZ128rmb:
	case VP2INTERSECTDZ128rr:
	case VP2INTERSECTDZ256rm:
	case VP2INTERSECTDZ256rmb:
	case VP2INTERSECTDZ256rr:
	case VP2INTERSECTDZrm:
	case VP2INTERSECTDZrmb:
	case VP2INTERSECTDZrr:
		return true;
	}
	return false;
}

bool isVPSUBUSW(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBUSWYrm:
	case VPSUBUSWYrr:
	case VPSUBUSWZ128rm:
	case VPSUBUSWZ128rmk:
	case VPSUBUSWZ128rmkz:
	case VPSUBUSWZ128rr:
	case VPSUBUSWZ128rrk:
	case VPSUBUSWZ128rrkz:
	case VPSUBUSWZ256rm:
	case VPSUBUSWZ256rmk:
	case VPSUBUSWZ256rmkz:
	case VPSUBUSWZ256rr:
	case VPSUBUSWZ256rrk:
	case VPSUBUSWZ256rrkz:
	case VPSUBUSWZrm:
	case VPSUBUSWZrmk:
	case VPSUBUSWZrmkz:
	case VPSUBUSWZrr:
	case VPSUBUSWZrrk:
	case VPSUBUSWZrrkz:
	case VPSUBUSWrm:
	case VPSUBUSWrr:
		return true;
	}
	return false;
}

bool isVPERMILPD(unsigned Opcode) {
	switch (Opcode) {
	case VPERMILPDYmi:
	case VPERMILPDYri:
	case VPERMILPDYrm:
	case VPERMILPDYrr:
	case VPERMILPDZ128mbi:
	case VPERMILPDZ128mbik:
	case VPERMILPDZ128mbikz:
	case VPERMILPDZ128mi:
	case VPERMILPDZ128mik:
	case VPERMILPDZ128mikz:
	case VPERMILPDZ128ri:
	case VPERMILPDZ128rik:
	case VPERMILPDZ128rikz:
	case VPERMILPDZ128rm:
	case VPERMILPDZ128rmb:
	case VPERMILPDZ128rmbk:
	case VPERMILPDZ128rmbkz:
	case VPERMILPDZ128rmk:
	case VPERMILPDZ128rmkz:
	case VPERMILPDZ128rr:
	case VPERMILPDZ128rrk:
	case VPERMILPDZ128rrkz:
	case VPERMILPDZ256mbi:
	case VPERMILPDZ256mbik:
	case VPERMILPDZ256mbikz:
	case VPERMILPDZ256mi:
	case VPERMILPDZ256mik:
	case VPERMILPDZ256mikz:
	case VPERMILPDZ256ri:
	case VPERMILPDZ256rik:
	case VPERMILPDZ256rikz:
	case VPERMILPDZ256rm:
	case VPERMILPDZ256rmb:
	case VPERMILPDZ256rmbk:
	case VPERMILPDZ256rmbkz:
	case VPERMILPDZ256rmk:
	case VPERMILPDZ256rmkz:
	case VPERMILPDZ256rr:
	case VPERMILPDZ256rrk:
	case VPERMILPDZ256rrkz:
	case VPERMILPDZmbi:
	case VPERMILPDZmbik:
	case VPERMILPDZmbikz:
	case VPERMILPDZmi:
	case VPERMILPDZmik:
	case VPERMILPDZmikz:
	case VPERMILPDZri:
	case VPERMILPDZrik:
	case VPERMILPDZrikz:
	case VPERMILPDZrm:
	case VPERMILPDZrmb:
	case VPERMILPDZrmbk:
	case VPERMILPDZrmbkz:
	case VPERMILPDZrmk:
	case VPERMILPDZrmkz:
	case VPERMILPDZrr:
	case VPERMILPDZrrk:
	case VPERMILPDZrrkz:
	case VPERMILPDmi:
	case VPERMILPDri:
	case VPERMILPDrm:
	case VPERMILPDrr:
		return true;
	}
	return false;
}

bool isVPSHLDD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDDZ128rmbi:
	case VPSHLDDZ128rmbik:
	case VPSHLDDZ128rmbikz:
	case VPSHLDDZ128rmi:
	case VPSHLDDZ128rmik:
	case VPSHLDDZ128rmikz:
	case VPSHLDDZ128rri:
	case VPSHLDDZ128rrik:
	case VPSHLDDZ128rrikz:
	case VPSHLDDZ256rmbi:
	case VPSHLDDZ256rmbik:
	case VPSHLDDZ256rmbikz:
	case VPSHLDDZ256rmi:
	case VPSHLDDZ256rmik:
	case VPSHLDDZ256rmikz:
	case VPSHLDDZ256rri:
	case VPSHLDDZ256rrik:
	case VPSHLDDZ256rrikz:
	case VPSHLDDZrmbi:
	case VPSHLDDZrmbik:
	case VPSHLDDZrmbikz:
	case VPSHLDDZrmi:
	case VPSHLDDZrmik:
	case VPSHLDDZrmikz:
	case VPSHLDDZrri:
	case VPSHLDDZrrik:
	case VPSHLDDZrrikz:
		return true;
	}
	return false;
}

bool isMOVD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_MOVD64grr:
	case MMX_MOVD64mr:
	case MMX_MOVD64rm:
	case MMX_MOVD64rr:
	case MOVDI2PDIrm:
	case MOVDI2PDIrr:
	case MOVPDI2DImr:
	case MOVPDI2DIrr:
		return true;
	}
	return false;
}

bool isVPORQ(unsigned Opcode) {
	switch (Opcode) {
	case VPORQZ128rm:
	case VPORQZ128rmb:
	case VPORQZ128rmbk:
	case VPORQZ128rmbkz:
	case VPORQZ128rmk:
	case VPORQZ128rmkz:
	case VPORQZ128rr:
	case VPORQZ128rrk:
	case VPORQZ128rrkz:
	case VPORQZ256rm:
	case VPORQZ256rmb:
	case VPORQZ256rmbk:
	case VPORQZ256rmbkz:
	case VPORQZ256rmk:
	case VPORQZ256rmkz:
	case VPORQZ256rr:
	case VPORQZ256rrk:
	case VPORQZ256rrkz:
	case VPORQZrm:
	case VPORQZrmb:
	case VPORQZrmbk:
	case VPORQZrmbkz:
	case VPORQZrmk:
	case VPORQZrmkz:
	case VPORQZrr:
	case VPORQZrrk:
	case VPORQZrrkz:
		return true;
	}
	return false;
}

bool isVP2INTERSECTQ(unsigned Opcode) {
	switch (Opcode) {
	case VP2INTERSECTQZ128rm:
	case VP2INTERSECTQZ128rmb:
	case VP2INTERSECTQZ128rr:
	case VP2INTERSECTQZ256rm:
	case VP2INTERSECTQZ256rmb:
	case VP2INTERSECTQZ256rr:
	case VP2INTERSECTQZrm:
	case VP2INTERSECTQZrmb:
	case VP2INTERSECTQZrr:
		return true;
	}
	return false;
}

bool isCRC32(unsigned Opcode) {
	switch (Opcode) {
	case CRC32r32m16:
	case CRC32r32m32:
	case CRC32r32m8:
	case CRC32r32r16:
	case CRC32r32r32:
	case CRC32r32r8:
	case CRC32r64m64:
	case CRC32r64m8:
	case CRC32r64r64:
	case CRC32r64r8:
		return true;
	}
	return false;
}

bool isVPERMILPS(unsigned Opcode) {
	switch (Opcode) {
	case VPERMILPSYmi:
	case VPERMILPSYri:
	case VPERMILPSYrm:
	case VPERMILPSYrr:
	case VPERMILPSZ128mbi:
	case VPERMILPSZ128mbik:
	case VPERMILPSZ128mbikz:
	case VPERMILPSZ128mi:
	case VPERMILPSZ128mik:
	case VPERMILPSZ128mikz:
	case VPERMILPSZ128ri:
	case VPERMILPSZ128rik:
	case VPERMILPSZ128rikz:
	case VPERMILPSZ128rm:
	case VPERMILPSZ128rmb:
	case VPERMILPSZ128rmbk:
	case VPERMILPSZ128rmbkz:
	case VPERMILPSZ128rmk:
	case VPERMILPSZ128rmkz:
	case VPERMILPSZ128rr:
	case VPERMILPSZ128rrk:
	case VPERMILPSZ128rrkz:
	case VPERMILPSZ256mbi:
	case VPERMILPSZ256mbik:
	case VPERMILPSZ256mbikz:
	case VPERMILPSZ256mi:
	case VPERMILPSZ256mik:
	case VPERMILPSZ256mikz:
	case VPERMILPSZ256ri:
	case VPERMILPSZ256rik:
	case VPERMILPSZ256rikz:
	case VPERMILPSZ256rm:
	case VPERMILPSZ256rmb:
	case VPERMILPSZ256rmbk:
	case VPERMILPSZ256rmbkz:
	case VPERMILPSZ256rmk:
	case VPERMILPSZ256rmkz:
	case VPERMILPSZ256rr:
	case VPERMILPSZ256rrk:
	case VPERMILPSZ256rrkz:
	case VPERMILPSZmbi:
	case VPERMILPSZmbik:
	case VPERMILPSZmbikz:
	case VPERMILPSZmi:
	case VPERMILPSZmik:
	case VPERMILPSZmikz:
	case VPERMILPSZri:
	case VPERMILPSZrik:
	case VPERMILPSZrikz:
	case VPERMILPSZrm:
	case VPERMILPSZrmb:
	case VPERMILPSZrmbk:
	case VPERMILPSZrmbkz:
	case VPERMILPSZrmk:
	case VPERMILPSZrmkz:
	case VPERMILPSZrr:
	case VPERMILPSZrrk:
	case VPERMILPSZrrkz:
	case VPERMILPSmi:
	case VPERMILPSri:
	case VPERMILPSrm:
	case VPERMILPSrr:
		return true;
	}
	return false;
}

bool isMOVQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_MOVD64from64mr:
	case MMX_MOVD64from64rr:
	case MMX_MOVD64to64rm:
	case MMX_MOVD64to64rr:
	case MMX_MOVQ64mr:
	case MMX_MOVQ64rm:
	case MMX_MOVQ64rr:
	case MMX_MOVQ64rr_REV:
	case MOV64toPQIrm:
	case MOV64toPQIrr:
	case MOVPQI2QImr:
	case MOVPQI2QIrr:
	case MOVPQIto64mr:
	case MOVPQIto64rr:
	case MOVQI2PQIrm:
	case MOVZPQILo2PQIrr:
		return true;
	}
	return false;
}

bool isVPSHLDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDQZ128rmbi:
	case VPSHLDQZ128rmbik:
	case VPSHLDQZ128rmbikz:
	case VPSHLDQZ128rmi:
	case VPSHLDQZ128rmik:
	case VPSHLDQZ128rmikz:
	case VPSHLDQZ128rri:
	case VPSHLDQZ128rrik:
	case VPSHLDQZ128rrikz:
	case VPSHLDQZ256rmbi:
	case VPSHLDQZ256rmbik:
	case VPSHLDQZ256rmbikz:
	case VPSHLDQZ256rmi:
	case VPSHLDQZ256rmik:
	case VPSHLDQZ256rmikz:
	case VPSHLDQZ256rri:
	case VPSHLDQZ256rrik:
	case VPSHLDQZ256rrikz:
	case VPSHLDQZrmbi:
	case VPSHLDQZrmbik:
	case VPSHLDQZrmbikz:
	case VPSHLDQZrmi:
	case VPSHLDQZrmik:
	case VPSHLDQZrmikz:
	case VPSHLDQZrri:
	case VPSHLDQZrrik:
	case VPSHLDQZrrikz:
		return true;
	}
	return false;
}

bool isVPMASKMOVD(unsigned Opcode) {
	switch (Opcode) {
	case VPMASKMOVDYmr:
	case VPMASKMOVDYrm:
	case VPMASKMOVDmr:
	case VPMASKMOVDrm:
		return true;
	}
	return false;
}

bool isVPSHLDW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDWZ128rmi:
	case VPSHLDWZ128rmik:
	case VPSHLDWZ128rmikz:
	case VPSHLDWZ128rri:
	case VPSHLDWZ128rrik:
	case VPSHLDWZ128rrikz:
	case VPSHLDWZ256rmi:
	case VPSHLDWZ256rmik:
	case VPSHLDWZ256rmikz:
	case VPSHLDWZ256rri:
	case VPSHLDWZ256rrik:
	case VPSHLDWZ256rrikz:
	case VPSHLDWZrmi:
	case VPSHLDWZrmik:
	case VPSHLDWZrmikz:
	case VPSHLDWZrri:
	case VPSHLDWZrrik:
	case VPSHLDWZrrikz:
		return true;
	}
	return false;
}

bool isSLWPCB(unsigned Opcode) {
	switch (Opcode) {
	case SLWPCB:
	case SLWPCB64:
		return true;
	}
	return false;
}

bool isVORPD(unsigned Opcode) {
	switch (Opcode) {
	case VORPDYrm:
	case VORPDYrr:
	case VORPDZ128rm:
	case VORPDZ128rmb:
	case VORPDZ128rmbk:
	case VORPDZ128rmbkz:
	case VORPDZ128rmk:
	case VORPDZ128rmkz:
	case VORPDZ128rr:
	case VORPDZ128rrk:
	case VORPDZ128rrkz:
	case VORPDZ256rm:
	case VORPDZ256rmb:
	case VORPDZ256rmbk:
	case VORPDZ256rmbkz:
	case VORPDZ256rmk:
	case VORPDZ256rmkz:
	case VORPDZ256rr:
	case VORPDZ256rrk:
	case VORPDZ256rrkz:
	case VORPDZrm:
	case VORPDZrmb:
	case VORPDZrmbk:
	case VORPDZrmbkz:
	case VORPDZrmk:
	case VORPDZrmkz:
	case VORPDZrr:
	case VORPDZrrk:
	case VORPDZrrkz:
	case VORPDrm:
	case VORPDrr:
		return true;
	}
	return false;
}

bool isPCMPGTB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PCMPGTBrm:
	case MMX_PCMPGTBrr:
	case PCMPGTBrm:
	case PCMPGTBrr:
		return true;
	}
	return false;
}

bool isPCMPGTD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PCMPGTDrm:
	case MMX_PCMPGTDrr:
	case PCMPGTDrm:
	case PCMPGTDrr:
		return true;
	}
	return false;
}

bool isVPMASKMOVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMASKMOVQYmr:
	case VPMASKMOVQYrm:
	case VPMASKMOVQmr:
	case VPMASKMOVQrm:
		return true;
	}
	return false;
}

bool isPFRSQIT1(unsigned Opcode) {
	switch (Opcode) {
	case PFRSQIT1rm:
	case PFRSQIT1rr:
		return true;
	}
	return false;
}

bool isVORPS(unsigned Opcode) {
	switch (Opcode) {
	case VORPSYrm:
	case VORPSYrr:
	case VORPSZ128rm:
	case VORPSZ128rmb:
	case VORPSZ128rmbk:
	case VORPSZ128rmbkz:
	case VORPSZ128rmk:
	case VORPSZ128rmkz:
	case VORPSZ128rr:
	case VORPSZ128rrk:
	case VORPSZ128rrkz:
	case VORPSZ256rm:
	case VORPSZ256rmb:
	case VORPSZ256rmbk:
	case VORPSZ256rmbkz:
	case VORPSZ256rmk:
	case VORPSZ256rmkz:
	case VORPSZ256rr:
	case VORPSZ256rrk:
	case VORPSZ256rrkz:
	case VORPSZrm:
	case VORPSZrmb:
	case VORPSZrmbk:
	case VORPSZrmbkz:
	case VORPSZrmk:
	case VORPSZrmkz:
	case VORPSZrr:
	case VORPSZrrk:
	case VORPSZrrkz:
	case VORPSrm:
	case VORPSrr:
		return true;
	}
	return false;
}

bool isPCMPGTQ(unsigned Opcode) {
	switch (Opcode) {
	case PCMPGTQrm:
	case PCMPGTQrr:
		return true;
	}
	return false;
}

bool isBLCIC(unsigned Opcode) {
	switch (Opcode) {
	case BLCIC32rm:
	case BLCIC32rr:
	case BLCIC64rm:
	case BLCIC64rr:
		return true;
	}
	return false;
}

bool isMAXSD(unsigned Opcode) {
	switch (Opcode) {
	case MAXSDrm_Int:
	case MAXSDrr_Int:
		return true;
	}
	return false;
}

bool isPCMPGTW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PCMPGTWrm:
	case MMX_PCMPGTWrr:
	case PCMPGTWrm:
	case PCMPGTWrr:
		return true;
	}
	return false;
}

bool isVBCSTNESH2PS(unsigned Opcode) {
	switch (Opcode) {
	case VBCSTNESH2PSYrm:
	case VBCSTNESH2PSrm:
		return true;
	}
	return false;
}

bool isMOVLHPS(unsigned Opcode) {
	return Opcode == MOVLHPSrr;
}

bool isXSAVEC(unsigned Opcode) {
	return Opcode == XSAVEC;
}

bool isMAXSS(unsigned Opcode) {
	switch (Opcode) {
	case MAXSSrm_Int:
	case MAXSSrr_Int:
		return true;
	}
	return false;
}

bool isFST(unsigned Opcode) {
	switch (Opcode) {
	case ST_F32m:
	case ST_F64m:
	case ST_Frr:
		return true;
	}
	return false;
}

bool isVCVTSS2USI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSS2USI64Zrm_Int:
	case VCVTSS2USI64Zrr_Int:
	case VCVTSS2USI64Zrrb_Int:
	case VCVTSS2USIZrm_Int:
	case VCVTSS2USIZrr_Int:
	case VCVTSS2USIZrrb_Int:
		return true;
	}
	return false;
}

bool isVADDSD(unsigned Opcode) {
	switch (Opcode) {
	case VADDSDZrm_Int:
	case VADDSDZrm_Intk:
	case VADDSDZrm_Intkz:
	case VADDSDZrr_Int:
	case VADDSDZrr_Intk:
	case VADDSDZrr_Intkz:
	case VADDSDZrrb_Int:
	case VADDSDZrrb_Intk:
	case VADDSDZrrb_Intkz:
	case VADDSDrm_Int:
	case VADDSDrr_Int:
		return true;
	}
	return false;
}

bool isVRNDSCALEPD(unsigned Opcode) {
	switch (Opcode) {
	case VRNDSCALEPDZ128rmbi:
	case VRNDSCALEPDZ128rmbik:
	case VRNDSCALEPDZ128rmbikz:
	case VRNDSCALEPDZ128rmi:
	case VRNDSCALEPDZ128rmik:
	case VRNDSCALEPDZ128rmikz:
	case VRNDSCALEPDZ128rri:
	case VRNDSCALEPDZ128rrik:
	case VRNDSCALEPDZ128rrikz:
	case VRNDSCALEPDZ256rmbi:
	case VRNDSCALEPDZ256rmbik:
	case VRNDSCALEPDZ256rmbikz:
	case VRNDSCALEPDZ256rmi:
	case VRNDSCALEPDZ256rmik:
	case VRNDSCALEPDZ256rmikz:
	case VRNDSCALEPDZ256rri:
	case VRNDSCALEPDZ256rrik:
	case VRNDSCALEPDZ256rrikz:
	case VRNDSCALEPDZrmbi:
	case VRNDSCALEPDZrmbik:
	case VRNDSCALEPDZrmbikz:
	case VRNDSCALEPDZrmi:
	case VRNDSCALEPDZrmik:
	case VRNDSCALEPDZrmikz:
	case VRNDSCALEPDZrri:
	case VRNDSCALEPDZrrib:
	case VRNDSCALEPDZrribk:
	case VRNDSCALEPDZrribkz:
	case VRNDSCALEPDZrrik:
	case VRNDSCALEPDZrrikz:
		return true;
	}
	return false;
}

bool isVCVTTPH2UQQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPH2UQQZ128rm:
	case VCVTTPH2UQQZ128rmb:
	case VCVTTPH2UQQZ128rmbk:
	case VCVTTPH2UQQZ128rmbkz:
	case VCVTTPH2UQQZ128rmk:
	case VCVTTPH2UQQZ128rmkz:
	case VCVTTPH2UQQZ128rr:
	case VCVTTPH2UQQZ128rrk:
	case VCVTTPH2UQQZ128rrkz:
	case VCVTTPH2UQQZ256rm:
	case VCVTTPH2UQQZ256rmb:
	case VCVTTPH2UQQZ256rmbk:
	case VCVTTPH2UQQZ256rmbkz:
	case VCVTTPH2UQQZ256rmk:
	case VCVTTPH2UQQZ256rmkz:
	case VCVTTPH2UQQZ256rr:
	case VCVTTPH2UQQZ256rrk:
	case VCVTTPH2UQQZ256rrkz:
	case VCVTTPH2UQQZrm:
	case VCVTTPH2UQQZrmb:
	case VCVTTPH2UQQZrmbk:
	case VCVTTPH2UQQZrmbkz:
	case VCVTTPH2UQQZrmk:
	case VCVTTPH2UQQZrmkz:
	case VCVTTPH2UQQZrr:
	case VCVTTPH2UQQZrrb:
	case VCVTTPH2UQQZrrbk:
	case VCVTTPH2UQQZrrbkz:
	case VCVTTPH2UQQZrrk:
	case VCVTTPH2UQQZrrkz:
		return true;
	}
	return false;
}

bool isV4FNMADDSS(unsigned Opcode) {
	switch (Opcode) {
	case V4FNMADDSSrm:
	case V4FNMADDSSrmk:
	case V4FNMADDSSrmkz:
		return true;
	}
	return false;
}

bool isVADDSH(unsigned Opcode) {
	switch (Opcode) {
	case VADDSHZrm_Int:
	case VADDSHZrm_Intk:
	case VADDSHZrm_Intkz:
	case VADDSHZrr_Int:
	case VADDSHZrr_Intk:
	case VADDSHZrr_Intkz:
	case VADDSHZrrb_Int:
	case VADDSHZrrb_Intk:
	case VADDSHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isVRNDSCALEPH(unsigned Opcode) {
	switch (Opcode) {
	case VRNDSCALEPHZ128rmbi:
	case VRNDSCALEPHZ128rmbik:
	case VRNDSCALEPHZ128rmbikz:
	case VRNDSCALEPHZ128rmi:
	case VRNDSCALEPHZ128rmik:
	case VRNDSCALEPHZ128rmikz:
	case VRNDSCALEPHZ128rri:
	case VRNDSCALEPHZ128rrik:
	case VRNDSCALEPHZ128rrikz:
	case VRNDSCALEPHZ256rmbi:
	case VRNDSCALEPHZ256rmbik:
	case VRNDSCALEPHZ256rmbikz:
	case VRNDSCALEPHZ256rmi:
	case VRNDSCALEPHZ256rmik:
	case VRNDSCALEPHZ256rmikz:
	case VRNDSCALEPHZ256rri:
	case VRNDSCALEPHZ256rrik:
	case VRNDSCALEPHZ256rrikz:
	case VRNDSCALEPHZrmbi:
	case VRNDSCALEPHZrmbik:
	case VRNDSCALEPHZrmbikz:
	case VRNDSCALEPHZrmi:
	case VRNDSCALEPHZrmik:
	case VRNDSCALEPHZrmikz:
	case VRNDSCALEPHZrri:
	case VRNDSCALEPHZrrib:
	case VRNDSCALEPHZrribk:
	case VRNDSCALEPHZrribkz:
	case VRNDSCALEPHZrrik:
	case VRNDSCALEPHZrrikz:
		return true;
	}
	return false;
}

bool isXSAVES(unsigned Opcode) {
	return Opcode == XSAVES;
}

bool isXRESLDTRK(unsigned Opcode) {
	return Opcode == XRESLDTRK;
}

bool isVADDSS(unsigned Opcode) {
	switch (Opcode) {
	case VADDSSZrm_Int:
	case VADDSSZrm_Intk:
	case VADDSSZrm_Intkz:
	case VADDSSZrr_Int:
	case VADDSSZrr_Intk:
	case VADDSSZrr_Intkz:
	case VADDSSZrrb_Int:
	case VADDSSZrrb_Intk:
	case VADDSSZrrb_Intkz:
	case VADDSSrm_Int:
	case VADDSSrr_Int:
		return true;
	}
	return false;
}

bool isVRNDSCALEPS(unsigned Opcode) {
	switch (Opcode) {
	case VRNDSCALEPSZ128rmbi:
	case VRNDSCALEPSZ128rmbik:
	case VRNDSCALEPSZ128rmbikz:
	case VRNDSCALEPSZ128rmi:
	case VRNDSCALEPSZ128rmik:
	case VRNDSCALEPSZ128rmikz:
	case VRNDSCALEPSZ128rri:
	case VRNDSCALEPSZ128rrik:
	case VRNDSCALEPSZ128rrikz:
	case VRNDSCALEPSZ256rmbi:
	case VRNDSCALEPSZ256rmbik:
	case VRNDSCALEPSZ256rmbikz:
	case VRNDSCALEPSZ256rmi:
	case VRNDSCALEPSZ256rmik:
	case VRNDSCALEPSZ256rmikz:
	case VRNDSCALEPSZ256rri:
	case VRNDSCALEPSZ256rrik:
	case VRNDSCALEPSZ256rrikz:
	case VRNDSCALEPSZrmbi:
	case VRNDSCALEPSZrmbik:
	case VRNDSCALEPSZrmbikz:
	case VRNDSCALEPSZrmi:
	case VRNDSCALEPSZrmik:
	case VRNDSCALEPSZrmikz:
	case VRNDSCALEPSZrri:
	case VRNDSCALEPSZrrib:
	case VRNDSCALEPSZrribk:
	case VRNDSCALEPSZrribkz:
	case VRNDSCALEPSZrrik:
	case VRNDSCALEPSZrrikz:
		return true;
	}
	return false;
}

bool isVPHADDUWD(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDUWDrm:
	case VPHADDUWDrr:
		return true;
	}
	return false;
}

bool isRDSSPD(unsigned Opcode) {
	return Opcode == RDSSPD;
}

bool isVPHADDUWQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDUWQrm:
	case VPHADDUWQrr:
		return true;
	}
	return false;
}

bool isVMOVDDUP(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDDUPYrm:
	case VMOVDDUPYrr:
	case VMOVDDUPZ128rm:
	case VMOVDDUPZ128rmk:
	case VMOVDDUPZ128rmkz:
	case VMOVDDUPZ128rr:
	case VMOVDDUPZ128rrk:
	case VMOVDDUPZ128rrkz:
	case VMOVDDUPZ256rm:
	case VMOVDDUPZ256rmk:
	case VMOVDDUPZ256rmkz:
	case VMOVDDUPZ256rr:
	case VMOVDDUPZ256rrk:
	case VMOVDDUPZ256rrkz:
	case VMOVDDUPZrm:
	case VMOVDDUPZrmk:
	case VMOVDDUPZrmkz:
	case VMOVDDUPZrr:
	case VMOVDDUPZrrk:
	case VMOVDDUPZrrkz:
	case VMOVDDUPrm:
	case VMOVDDUPrr:
		return true;
	}
	return false;
}

bool isINSERTPS(unsigned Opcode) {
	switch (Opcode) {
	case INSERTPSrm:
	case INSERTPSrr:
		return true;
	}
	return false;
}

bool isNEG(unsigned Opcode) {
	switch (Opcode) {
	case NEG16m:
	case NEG16r:
	case NEG32m:
	case NEG32r:
	case NEG64m:
	case NEG64r:
	case NEG8m:
	case NEG8r:
		return true;
	}
	return false;
}

bool isMOVUPD(unsigned Opcode) {
	switch (Opcode) {
	case MOVUPDmr:
	case MOVUPDrm:
	case MOVUPDrr:
	case MOVUPDrr_REV:
		return true;
	}
	return false;
}

bool isRDSSPQ(unsigned Opcode) {
	return Opcode == RDSSPQ;
}

bool isVFRCZPD(unsigned Opcode) {
	switch (Opcode) {
	case VFRCZPDYrm:
	case VFRCZPDYrr:
	case VFRCZPDrm:
	case VFRCZPDrr:
		return true;
	}
	return false;
}

bool isPHMINPOSUW(unsigned Opcode) {
	switch (Opcode) {
	case PHMINPOSUWrm:
	case PHMINPOSUWrr:
		return true;
	}
	return false;
}

bool isVBROADCASTI32X2(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTI32X2Z128rm:
	case VBROADCASTI32X2Z128rmk:
	case VBROADCASTI32X2Z128rmkz:
	case VBROADCASTI32X2Z128rr:
	case VBROADCASTI32X2Z128rrk:
	case VBROADCASTI32X2Z128rrkz:
	case VBROADCASTI32X2Z256rm:
	case VBROADCASTI32X2Z256rmk:
	case VBROADCASTI32X2Z256rmkz:
	case VBROADCASTI32X2Z256rr:
	case VBROADCASTI32X2Z256rrk:
	case VBROADCASTI32X2Z256rrkz:
	case VBROADCASTI32X2Zrm:
	case VBROADCASTI32X2Zrmk:
	case VBROADCASTI32X2Zrmkz:
	case VBROADCASTI32X2Zrr:
	case VBROADCASTI32X2Zrrk:
	case VBROADCASTI32X2Zrrkz:
		return true;
	}
	return false;
}

bool isVBROADCASTI32X4(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTI32X4Z256rm:
	case VBROADCASTI32X4Z256rmk:
	case VBROADCASTI32X4Z256rmkz:
	case VBROADCASTI32X4rm:
	case VBROADCASTI32X4rmk:
	case VBROADCASTI32X4rmkz:
		return true;
	}
	return false;
}

bool isJMP(unsigned Opcode) {
	switch (Opcode) {
	case FARJMP32m:
	case JMP16m:
	case JMP16r:
	case JMP32m:
	case JMP32r:
	case JMP64m:
	case JMP64r:
	case JMP_1:
	case JMP_2:
	case JMP_4:
		return true;
	}
	return false;
}

bool isVBROADCASTI32X8(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTI32X8rm:
	case VBROADCASTI32X8rmk:
	case VBROADCASTI32X8rmkz:
		return true;
	}
	return false;
}

bool isMOVUPS(unsigned Opcode) {
	switch (Opcode) {
	case MOVUPSmr:
	case MOVUPSrm:
	case MOVUPSrr:
	case MOVUPSrr_REV:
		return true;
	}
	return false;
}

bool isVPANDND(unsigned Opcode) {
	switch (Opcode) {
	case VPANDNDZ128rm:
	case VPANDNDZ128rmb:
	case VPANDNDZ128rmbk:
	case VPANDNDZ128rmbkz:
	case VPANDNDZ128rmk:
	case VPANDNDZ128rmkz:
	case VPANDNDZ128rr:
	case VPANDNDZ128rrk:
	case VPANDNDZ128rrkz:
	case VPANDNDZ256rm:
	case VPANDNDZ256rmb:
	case VPANDNDZ256rmbk:
	case VPANDNDZ256rmbkz:
	case VPANDNDZ256rmk:
	case VPANDNDZ256rmkz:
	case VPANDNDZ256rr:
	case VPANDNDZ256rrk:
	case VPANDNDZ256rrkz:
	case VPANDNDZrm:
	case VPANDNDZrmb:
	case VPANDNDZrmbk:
	case VPANDNDZrmbkz:
	case VPANDNDZrmk:
	case VPANDNDZrmkz:
	case VPANDNDZrr:
	case VPANDNDZrrk:
	case VPANDNDZrrkz:
		return true;
	}
	return false;
}

bool isVCVTNE2PS2BF16(unsigned Opcode) {
	switch (Opcode) {
	case VCVTNE2PS2BF16Z128rm:
	case VCVTNE2PS2BF16Z128rmb:
	case VCVTNE2PS2BF16Z128rmbk:
	case VCVTNE2PS2BF16Z128rmbkz:
	case VCVTNE2PS2BF16Z128rmk:
	case VCVTNE2PS2BF16Z128rmkz:
	case VCVTNE2PS2BF16Z128rr:
	case VCVTNE2PS2BF16Z128rrk:
	case VCVTNE2PS2BF16Z128rrkz:
	case VCVTNE2PS2BF16Z256rm:
	case VCVTNE2PS2BF16Z256rmb:
	case VCVTNE2PS2BF16Z256rmbk:
	case VCVTNE2PS2BF16Z256rmbkz:
	case VCVTNE2PS2BF16Z256rmk:
	case VCVTNE2PS2BF16Z256rmkz:
	case VCVTNE2PS2BF16Z256rr:
	case VCVTNE2PS2BF16Z256rrk:
	case VCVTNE2PS2BF16Z256rrkz:
	case VCVTNE2PS2BF16Zrm:
	case VCVTNE2PS2BF16Zrmb:
	case VCVTNE2PS2BF16Zrmbk:
	case VCVTNE2PS2BF16Zrmbkz:
	case VCVTNE2PS2BF16Zrmk:
	case VCVTNE2PS2BF16Zrmkz:
	case VCVTNE2PS2BF16Zrr:
	case VCVTNE2PS2BF16Zrrk:
	case VCVTNE2PS2BF16Zrrkz:
		return true;
	}
	return false;
}

bool isVPROLVD(unsigned Opcode) {
	switch (Opcode) {
	case VPROLVDZ128rm:
	case VPROLVDZ128rmb:
	case VPROLVDZ128rmbk:
	case VPROLVDZ128rmbkz:
	case VPROLVDZ128rmk:
	case VPROLVDZ128rmkz:
	case VPROLVDZ128rr:
	case VPROLVDZ128rrk:
	case VPROLVDZ128rrkz:
	case VPROLVDZ256rm:
	case VPROLVDZ256rmb:
	case VPROLVDZ256rmbk:
	case VPROLVDZ256rmbkz:
	case VPROLVDZ256rmk:
	case VPROLVDZ256rmkz:
	case VPROLVDZ256rr:
	case VPROLVDZ256rrk:
	case VPROLVDZ256rrkz:
	case VPROLVDZrm:
	case VPROLVDZrmb:
	case VPROLVDZrmbk:
	case VPROLVDZrmbkz:
	case VPROLVDZrmk:
	case VPROLVDZrmkz:
	case VPROLVDZrr:
	case VPROLVDZrrk:
	case VPROLVDZrrkz:
		return true;
	}
	return false;
}

bool isVFRCZPS(unsigned Opcode) {
	switch (Opcode) {
	case VFRCZPSYrm:
	case VFRCZPSYrr:
	case VFRCZPSrm:
	case VFRCZPSrr:
		return true;
	}
	return false;
}

bool isJRCXZ(unsigned Opcode) {
	return Opcode == JRCXZ;
}

bool isFNSTCW(unsigned Opcode) {
	return Opcode == FNSTCW16m;
}

bool isFFREEP(unsigned Opcode) {
	return Opcode == FFREEP;
}

bool isVPANDNQ(unsigned Opcode) {
	switch (Opcode) {
	case VPANDNQZ128rm:
	case VPANDNQZ128rmb:
	case VPANDNQZ128rmbk:
	case VPANDNQZ128rmbkz:
	case VPANDNQZ128rmk:
	case VPANDNQZ128rmkz:
	case VPANDNQZ128rr:
	case VPANDNQZ128rrk:
	case VPANDNQZ128rrkz:
	case VPANDNQZ256rm:
	case VPANDNQZ256rmb:
	case VPANDNQZ256rmbk:
	case VPANDNQZ256rmbkz:
	case VPANDNQZ256rmk:
	case VPANDNQZ256rmkz:
	case VPANDNQZ256rr:
	case VPANDNQZ256rrk:
	case VPANDNQZ256rrkz:
	case VPANDNQZrm:
	case VPANDNQZrmb:
	case VPANDNQZrmbk:
	case VPANDNQZrmbkz:
	case VPANDNQZrmk:
	case VPANDNQZrmkz:
	case VPANDNQZrr:
	case VPANDNQZrrk:
	case VPANDNQZrrkz:
		return true;
	}
	return false;
}

bool isVPROLVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPROLVQZ128rm:
	case VPROLVQZ128rmb:
	case VPROLVQZ128rmbk:
	case VPROLVQZ128rmbkz:
	case VPROLVQZ128rmk:
	case VPROLVQZ128rmkz:
	case VPROLVQZ128rr:
	case VPROLVQZ128rrk:
	case VPROLVQZ128rrkz:
	case VPROLVQZ256rm:
	case VPROLVQZ256rmb:
	case VPROLVQZ256rmbk:
	case VPROLVQZ256rmbkz:
	case VPROLVQZ256rmk:
	case VPROLVQZ256rmkz:
	case VPROLVQZ256rr:
	case VPROLVQZ256rrk:
	case VPROLVQZ256rrkz:
	case VPROLVQZrm:
	case VPROLVQZrmb:
	case VPROLVQZrmbk:
	case VPROLVQZrmbkz:
	case VPROLVQZrmk:
	case VPROLVQZrmkz:
	case VPROLVQZrr:
	case VPROLVQZrrk:
	case VPROLVQZrrkz:
		return true;
	}
	return false;
}

bool isAESDECWIDE128KL(unsigned Opcode) {
	return Opcode == AESDECWIDE128KL;
}

bool isSTTILECFG(unsigned Opcode) {
	return Opcode == STTILECFG;
}

bool isVPMOVWB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVWBZ128mr:
	case VPMOVWBZ128mrk:
	case VPMOVWBZ128rr:
	case VPMOVWBZ128rrk:
	case VPMOVWBZ128rrkz:
	case VPMOVWBZ256mr:
	case VPMOVWBZ256mrk:
	case VPMOVWBZ256rr:
	case VPMOVWBZ256rrk:
	case VPMOVWBZ256rrkz:
	case VPMOVWBZmr:
	case VPMOVWBZmrk:
	case VPMOVWBZrr:
	case VPMOVWBZrrk:
	case VPMOVWBZrrkz:
		return true;
	}
	return false;
}

bool isVRNDSCALESD(unsigned Opcode) {
	switch (Opcode) {
	case VRNDSCALESDZm_Int:
	case VRNDSCALESDZm_Intk:
	case VRNDSCALESDZm_Intkz:
	case VRNDSCALESDZr_Int:
	case VRNDSCALESDZr_Intk:
	case VRNDSCALESDZr_Intkz:
	case VRNDSCALESDZrb_Int:
	case VRNDSCALESDZrb_Intk:
	case VRNDSCALESDZrb_Intkz:
		return true;
	}
	return false;
}

bool isVGATHERPF0QPD(unsigned Opcode) {
	return Opcode == VGATHERPF0QPDm;
}

bool isVPHSUBD(unsigned Opcode) {
	switch (Opcode) {
	case VPHSUBDYrm:
	case VPHSUBDYrr:
	case VPHSUBDrm:
	case VPHSUBDrr:
		return true;
	}
	return false;
}

bool isVCVTSD2USI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSD2USI64Zrm_Int:
	case VCVTSD2USI64Zrr_Int:
	case VCVTSD2USI64Zrrb_Int:
	case VCVTSD2USIZrm_Int:
	case VCVTSD2USIZrr_Int:
	case VCVTSD2USIZrrb_Int:
		return true;
	}
	return false;
}

bool isWRGSBASE(unsigned Opcode) {
	switch (Opcode) {
	case WRGSBASE:
	case WRGSBASE64:
		return true;
	}
	return false;
}

bool isVRNDSCALESH(unsigned Opcode) {
	switch (Opcode) {
	case VRNDSCALESHZm_Int:
	case VRNDSCALESHZm_Intk:
	case VRNDSCALESHZm_Intkz:
	case VRNDSCALESHZr_Int:
	case VRNDSCALESHZr_Intk:
	case VRNDSCALESHZr_Intkz:
	case VRNDSCALESHZrb_Int:
	case VRNDSCALESHZrb_Intk:
	case VRNDSCALESHZrb_Intkz:
		return true;
	}
	return false;
}

bool isAESENC256KL(unsigned Opcode) {
	return Opcode == AESENC256KL;
}

bool isVRNDSCALESS(unsigned Opcode) {
	switch (Opcode) {
	case VRNDSCALESSZm_Int:
	case VRNDSCALESSZm_Intk:
	case VRNDSCALESSZm_Intkz:
	case VRNDSCALESSZr_Int:
	case VRNDSCALESSZr_Intk:
	case VRNDSCALESSZr_Intkz:
	case VRNDSCALESSZrb_Int:
	case VRNDSCALESSZrb_Intk:
	case VRNDSCALESSZrb_Intkz:
		return true;
	}
	return false;
}

bool isVGATHERPF0QPS(unsigned Opcode) {
	return Opcode == VGATHERPF0QPSm;
}

bool isVPHSUBW(unsigned Opcode) {
	switch (Opcode) {
	case VPHSUBWYrm:
	case VPHSUBWYrr:
	case VPHSUBWrm:
	case VPHSUBWrr:
		return true;
	}
	return false;
}

bool isSHA1NEXTE(unsigned Opcode) {
	switch (Opcode) {
	case SHA1NEXTErm:
	case SHA1NEXTErr:
		return true;
	}
	return false;
}

bool isXCRYPTCTR(unsigned Opcode) {
	return Opcode == XCRYPTCTR;
}

bool isFMUL(unsigned Opcode) {
	switch (Opcode) {
	case MUL_F32m:
	case MUL_F64m:
	case MUL_FST0r:
	case MUL_FrST0:
		return true;
	}
	return false;
}

bool isFBLD(unsigned Opcode) {
	return Opcode == FBLDm;
}

bool isVMXON(unsigned Opcode) {
	return Opcode == VMXON;
}

bool isPUNPCKHQDQ(unsigned Opcode) {
	switch (Opcode) {
	case PUNPCKHQDQrm:
	case PUNPCKHQDQrr:
		return true;
	}
	return false;
}

bool isVFRCZSD(unsigned Opcode) {
	switch (Opcode) {
	case VFRCZSDrm:
	case VFRCZSDrr:
		return true;
	}
	return false;
}

bool isVFRCZSS(unsigned Opcode) {
	switch (Opcode) {
	case VFRCZSSrm:
	case VFRCZSSrr:
		return true;
	}
	return false;
}

bool isRMPUPDATE(unsigned Opcode) {
	return Opcode == RMPUPDATE;
}

bool isVFNMADDPD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADDPD4Ymr:
	case VFNMADDPD4Yrm:
	case VFNMADDPD4Yrr:
	case VFNMADDPD4Yrr_REV:
	case VFNMADDPD4mr:
	case VFNMADDPD4rm:
	case VFNMADDPD4rr:
	case VFNMADDPD4rr_REV:
		return true;
	}
	return false;
}

bool isVCOMISD(unsigned Opcode) {
	switch (Opcode) {
	case VCOMISDZrm:
	case VCOMISDZrr:
	case VCOMISDZrrb:
	case VCOMISDrm:
	case VCOMISDrr:
		return true;
	}
	return false;
}

bool isVCOMISH(unsigned Opcode) {
	switch (Opcode) {
	case VCOMISHZrm:
	case VCOMISHZrr:
	case VCOMISHZrrb:
		return true;
	}
	return false;
}

bool isAXOR(unsigned Opcode) {
	switch (Opcode) {
	case AXOR32mr:
	case AXOR64mr:
		return true;
	}
	return false;
}

bool isVFNMADDPS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADDPS4Ymr:
	case VFNMADDPS4Yrm:
	case VFNMADDPS4Yrr:
	case VFNMADDPS4Yrr_REV:
	case VFNMADDPS4mr:
	case VFNMADDPS4rm:
	case VFNMADDPS4rr:
	case VFNMADDPS4rr_REV:
		return true;
	}
	return false;
}

bool isVCOMISS(unsigned Opcode) {
	switch (Opcode) {
	case VCOMISSZrm:
	case VCOMISSZrr:
	case VCOMISSZrrb:
	case VCOMISSrm:
	case VCOMISSrr:
		return true;
	}
	return false;
}

bool isPHADDD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PHADDDrm:
	case MMX_PHADDDrr:
	case PHADDDrm:
	case PHADDDrr:
		return true;
	}
	return false;
}

bool isVPMULHUW(unsigned Opcode) {
	switch (Opcode) {
	case VPMULHUWYrm:
	case VPMULHUWYrr:
	case VPMULHUWZ128rm:
	case VPMULHUWZ128rmk:
	case VPMULHUWZ128rmkz:
	case VPMULHUWZ128rr:
	case VPMULHUWZ128rrk:
	case VPMULHUWZ128rrkz:
	case VPMULHUWZ256rm:
	case VPMULHUWZ256rmk:
	case VPMULHUWZ256rmkz:
	case VPMULHUWZ256rr:
	case VPMULHUWZ256rrk:
	case VPMULHUWZ256rrkz:
	case VPMULHUWZrm:
	case VPMULHUWZrmk:
	case VPMULHUWZrmkz:
	case VPMULHUWZrr:
	case VPMULHUWZrrk:
	case VPMULHUWZrrkz:
	case VPMULHUWrm:
	case VPMULHUWrr:
		return true;
	}
	return false;
}

bool isVFNMSUB132PD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB132PDYm:
	case VFNMSUB132PDYr:
	case VFNMSUB132PDZ128m:
	case VFNMSUB132PDZ128mb:
	case VFNMSUB132PDZ128mbk:
	case VFNMSUB132PDZ128mbkz:
	case VFNMSUB132PDZ128mk:
	case VFNMSUB132PDZ128mkz:
	case VFNMSUB132PDZ128r:
	case VFNMSUB132PDZ128rk:
	case VFNMSUB132PDZ128rkz:
	case VFNMSUB132PDZ256m:
	case VFNMSUB132PDZ256mb:
	case VFNMSUB132PDZ256mbk:
	case VFNMSUB132PDZ256mbkz:
	case VFNMSUB132PDZ256mk:
	case VFNMSUB132PDZ256mkz:
	case VFNMSUB132PDZ256r:
	case VFNMSUB132PDZ256rk:
	case VFNMSUB132PDZ256rkz:
	case VFNMSUB132PDZm:
	case VFNMSUB132PDZmb:
	case VFNMSUB132PDZmbk:
	case VFNMSUB132PDZmbkz:
	case VFNMSUB132PDZmk:
	case VFNMSUB132PDZmkz:
	case VFNMSUB132PDZr:
	case VFNMSUB132PDZrb:
	case VFNMSUB132PDZrbk:
	case VFNMSUB132PDZrbkz:
	case VFNMSUB132PDZrk:
	case VFNMSUB132PDZrkz:
	case VFNMSUB132PDm:
	case VFNMSUB132PDr:
		return true;
	}
	return false;
}

bool isPMOVMSKB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMOVMSKBrr:
	case PMOVMSKBrr:
		return true;
	}
	return false;
}

bool isVFNMSUB132PH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB132PHZ128m:
	case VFNMSUB132PHZ128mb:
	case VFNMSUB132PHZ128mbk:
	case VFNMSUB132PHZ128mbkz:
	case VFNMSUB132PHZ128mk:
	case VFNMSUB132PHZ128mkz:
	case VFNMSUB132PHZ128r:
	case VFNMSUB132PHZ128rk:
	case VFNMSUB132PHZ128rkz:
	case VFNMSUB132PHZ256m:
	case VFNMSUB132PHZ256mb:
	case VFNMSUB132PHZ256mbk:
	case VFNMSUB132PHZ256mbkz:
	case VFNMSUB132PHZ256mk:
	case VFNMSUB132PHZ256mkz:
	case VFNMSUB132PHZ256r:
	case VFNMSUB132PHZ256rk:
	case VFNMSUB132PHZ256rkz:
	case VFNMSUB132PHZm:
	case VFNMSUB132PHZmb:
	case VFNMSUB132PHZmbk:
	case VFNMSUB132PHZmbkz:
	case VFNMSUB132PHZmk:
	case VFNMSUB132PHZmkz:
	case VFNMSUB132PHZr:
	case VFNMSUB132PHZrb:
	case VFNMSUB132PHZrbk:
	case VFNMSUB132PHZrbkz:
	case VFNMSUB132PHZrk:
	case VFNMSUB132PHZrkz:
		return true;
	}
	return false;
}

bool isPHADDW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PHADDWrm:
	case MMX_PHADDWrr:
	case PHADDWrm:
	case PHADDWrr:
		return true;
	}
	return false;
}

bool isVFNMSUB132PS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB132PSYm:
	case VFNMSUB132PSYr:
	case VFNMSUB132PSZ128m:
	case VFNMSUB132PSZ128mb:
	case VFNMSUB132PSZ128mbk:
	case VFNMSUB132PSZ128mbkz:
	case VFNMSUB132PSZ128mk:
	case VFNMSUB132PSZ128mkz:
	case VFNMSUB132PSZ128r:
	case VFNMSUB132PSZ128rk:
	case VFNMSUB132PSZ128rkz:
	case VFNMSUB132PSZ256m:
	case VFNMSUB132PSZ256mb:
	case VFNMSUB132PSZ256mbk:
	case VFNMSUB132PSZ256mbkz:
	case VFNMSUB132PSZ256mk:
	case VFNMSUB132PSZ256mkz:
	case VFNMSUB132PSZ256r:
	case VFNMSUB132PSZ256rk:
	case VFNMSUB132PSZ256rkz:
	case VFNMSUB132PSZm:
	case VFNMSUB132PSZmb:
	case VFNMSUB132PSZmbk:
	case VFNMSUB132PSZmbkz:
	case VFNMSUB132PSZmk:
	case VFNMSUB132PSZmkz:
	case VFNMSUB132PSZr:
	case VFNMSUB132PSZrb:
	case VFNMSUB132PSZrbk:
	case VFNMSUB132PSZrbkz:
	case VFNMSUB132PSZrk:
	case VFNMSUB132PSZrkz:
	case VFNMSUB132PSm:
	case VFNMSUB132PSr:
		return true;
	}
	return false;
}

bool isVMOVDQA(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQAYmr:
	case VMOVDQAYrm:
	case VMOVDQAYrr:
	case VMOVDQAYrr_REV:
	case VMOVDQAmr:
	case VMOVDQArm:
	case VMOVDQArr:
	case VMOVDQArr_REV:
		return true;
	}
	return false;
}

bool isSENDUIPI(unsigned Opcode) {
	return Opcode == SENDUIPI;
}

bool isVCVTTPS2DQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPS2DQYrm:
	case VCVTTPS2DQYrr:
	case VCVTTPS2DQZ128rm:
	case VCVTTPS2DQZ128rmb:
	case VCVTTPS2DQZ128rmbk:
	case VCVTTPS2DQZ128rmbkz:
	case VCVTTPS2DQZ128rmk:
	case VCVTTPS2DQZ128rmkz:
	case VCVTTPS2DQZ128rr:
	case VCVTTPS2DQZ128rrk:
	case VCVTTPS2DQZ128rrkz:
	case VCVTTPS2DQZ256rm:
	case VCVTTPS2DQZ256rmb:
	case VCVTTPS2DQZ256rmbk:
	case VCVTTPS2DQZ256rmbkz:
	case VCVTTPS2DQZ256rmk:
	case VCVTTPS2DQZ256rmkz:
	case VCVTTPS2DQZ256rr:
	case VCVTTPS2DQZ256rrk:
	case VCVTTPS2DQZ256rrkz:
	case VCVTTPS2DQZrm:
	case VCVTTPS2DQZrmb:
	case VCVTTPS2DQZrmbk:
	case VCVTTPS2DQZrmbkz:
	case VCVTTPS2DQZrmk:
	case VCVTTPS2DQZrmkz:
	case VCVTTPS2DQZrr:
	case VCVTTPS2DQZrrb:
	case VCVTTPS2DQZrrbk:
	case VCVTTPS2DQZrrbkz:
	case VCVTTPS2DQZrrk:
	case VCVTTPS2DQZrrkz:
	case VCVTTPS2DQrm:
	case VCVTTPS2DQrr:
		return true;
	}
	return false;
}

bool isRCPPS(unsigned Opcode) {
	switch (Opcode) {
	case RCPPSm:
	case RCPPSr:
		return true;
	}
	return false;
}

bool isRDMSR(unsigned Opcode) {
	return Opcode == RDMSR;
}

bool isVCVTQQ2PD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTQQ2PDZ128rm:
	case VCVTQQ2PDZ128rmb:
	case VCVTQQ2PDZ128rmbk:
	case VCVTQQ2PDZ128rmbkz:
	case VCVTQQ2PDZ128rmk:
	case VCVTQQ2PDZ128rmkz:
	case VCVTQQ2PDZ128rr:
	case VCVTQQ2PDZ128rrk:
	case VCVTQQ2PDZ128rrkz:
	case VCVTQQ2PDZ256rm:
	case VCVTQQ2PDZ256rmb:
	case VCVTQQ2PDZ256rmbk:
	case VCVTQQ2PDZ256rmbkz:
	case VCVTQQ2PDZ256rmk:
	case VCVTQQ2PDZ256rmkz:
	case VCVTQQ2PDZ256rr:
	case VCVTQQ2PDZ256rrk:
	case VCVTQQ2PDZ256rrkz:
	case VCVTQQ2PDZrm:
	case VCVTQQ2PDZrmb:
	case VCVTQQ2PDZrmbk:
	case VCVTQQ2PDZrmbkz:
	case VCVTQQ2PDZrmk:
	case VCVTQQ2PDZrmkz:
	case VCVTQQ2PDZrr:
	case VCVTQQ2PDZrrb:
	case VCVTQQ2PDZrrbk:
	case VCVTQQ2PDZrrbkz:
	case VCVTQQ2PDZrrk:
	case VCVTQQ2PDZrrkz:
		return true;
	}
	return false;
}

bool isVRSQRT14PD(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT14PDZ128m:
	case VRSQRT14PDZ128mb:
	case VRSQRT14PDZ128mbk:
	case VRSQRT14PDZ128mbkz:
	case VRSQRT14PDZ128mk:
	case VRSQRT14PDZ128mkz:
	case VRSQRT14PDZ128r:
	case VRSQRT14PDZ128rk:
	case VRSQRT14PDZ128rkz:
	case VRSQRT14PDZ256m:
	case VRSQRT14PDZ256mb:
	case VRSQRT14PDZ256mbk:
	case VRSQRT14PDZ256mbkz:
	case VRSQRT14PDZ256mk:
	case VRSQRT14PDZ256mkz:
	case VRSQRT14PDZ256r:
	case VRSQRT14PDZ256rk:
	case VRSQRT14PDZ256rkz:
	case VRSQRT14PDZm:
	case VRSQRT14PDZmb:
	case VRSQRT14PDZmbk:
	case VRSQRT14PDZmbkz:
	case VRSQRT14PDZmk:
	case VRSQRT14PDZmkz:
	case VRSQRT14PDZr:
	case VRSQRT14PDZrk:
	case VRSQRT14PDZrkz:
		return true;
	}
	return false;
}

bool isXORPD(unsigned Opcode) {
	switch (Opcode) {
	case XORPDrm:
	case XORPDrr:
		return true;
	}
	return false;
}

bool isVCVTQQ2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTQQ2PHZ128rm:
	case VCVTQQ2PHZ128rmb:
	case VCVTQQ2PHZ128rmbk:
	case VCVTQQ2PHZ128rmbkz:
	case VCVTQQ2PHZ128rmk:
	case VCVTQQ2PHZ128rmkz:
	case VCVTQQ2PHZ128rr:
	case VCVTQQ2PHZ128rrk:
	case VCVTQQ2PHZ128rrkz:
	case VCVTQQ2PHZ256rm:
	case VCVTQQ2PHZ256rmb:
	case VCVTQQ2PHZ256rmbk:
	case VCVTQQ2PHZ256rmbkz:
	case VCVTQQ2PHZ256rmk:
	case VCVTQQ2PHZ256rmkz:
	case VCVTQQ2PHZ256rr:
	case VCVTQQ2PHZ256rrk:
	case VCVTQQ2PHZ256rrkz:
	case VCVTQQ2PHZrm:
	case VCVTQQ2PHZrmb:
	case VCVTQQ2PHZrmbk:
	case VCVTQQ2PHZrmbkz:
	case VCVTQQ2PHZrmk:
	case VCVTQQ2PHZrmkz:
	case VCVTQQ2PHZrr:
	case VCVTQQ2PHZrrb:
	case VCVTQQ2PHZrrbk:
	case VCVTQQ2PHZrrbkz:
	case VCVTQQ2PHZrrk:
	case VCVTQQ2PHZrrkz:
		return true;
	}
	return false;
}

bool isVMOVDQU(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQUYmr:
	case VMOVDQUYrm:
	case VMOVDQUYrr:
	case VMOVDQUYrr_REV:
	case VMOVDQUmr:
	case VMOVDQUrm:
	case VMOVDQUrr:
	case VMOVDQUrr_REV:
		return true;
	}
	return false;
}

bool isBLENDPD(unsigned Opcode) {
	switch (Opcode) {
	case BLENDPDrmi:
	case BLENDPDrri:
		return true;
	}
	return false;
}

bool isVCVTQQ2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTQQ2PSZ128rm:
	case VCVTQQ2PSZ128rmb:
	case VCVTQQ2PSZ128rmbk:
	case VCVTQQ2PSZ128rmbkz:
	case VCVTQQ2PSZ128rmk:
	case VCVTQQ2PSZ128rmkz:
	case VCVTQQ2PSZ128rr:
	case VCVTQQ2PSZ128rrk:
	case VCVTQQ2PSZ128rrkz:
	case VCVTQQ2PSZ256rm:
	case VCVTQQ2PSZ256rmb:
	case VCVTQQ2PSZ256rmbk:
	case VCVTQQ2PSZ256rmbkz:
	case VCVTQQ2PSZ256rmk:
	case VCVTQQ2PSZ256rmkz:
	case VCVTQQ2PSZ256rr:
	case VCVTQQ2PSZ256rrk:
	case VCVTQQ2PSZ256rrkz:
	case VCVTQQ2PSZrm:
	case VCVTQQ2PSZrmb:
	case VCVTQQ2PSZrmbk:
	case VCVTQQ2PSZrmbkz:
	case VCVTQQ2PSZrmk:
	case VCVTQQ2PSZrmkz:
	case VCVTQQ2PSZrr:
	case VCVTQQ2PSZrrb:
	case VCVTQQ2PSZrrbk:
	case VCVTQQ2PSZrrbkz:
	case VCVTQQ2PSZrrk:
	case VCVTQQ2PSZrrkz:
		return true;
	}
	return false;
}

bool isVRSQRT14PS(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT14PSZ128m:
	case VRSQRT14PSZ128mb:
	case VRSQRT14PSZ128mbk:
	case VRSQRT14PSZ128mbkz:
	case VRSQRT14PSZ128mk:
	case VRSQRT14PSZ128mkz:
	case VRSQRT14PSZ128r:
	case VRSQRT14PSZ128rk:
	case VRSQRT14PSZ128rkz:
	case VRSQRT14PSZ256m:
	case VRSQRT14PSZ256mb:
	case VRSQRT14PSZ256mbk:
	case VRSQRT14PSZ256mbkz:
	case VRSQRT14PSZ256mk:
	case VRSQRT14PSZ256mkz:
	case VRSQRT14PSZ256r:
	case VRSQRT14PSZ256rk:
	case VRSQRT14PSZ256rkz:
	case VRSQRT14PSZm:
	case VRSQRT14PSZmb:
	case VRSQRT14PSZmbk:
	case VRSQRT14PSZmbkz:
	case VRSQRT14PSZmk:
	case VRSQRT14PSZmkz:
	case VRSQRT14PSZr:
	case VRSQRT14PSZrk:
	case VRSQRT14PSZrkz:
		return true;
	}
	return false;
}

bool isXORPS(unsigned Opcode) {
	switch (Opcode) {
	case XORPSrm:
	case XORPSrr:
		return true;
	}
	return false;
}

bool isRCL(unsigned Opcode) {
	switch (Opcode) {
	case RCL16m1:
	case RCL16mCL:
	case RCL16mi:
	case RCL16r1:
	case RCL16rCL:
	case RCL16ri:
	case RCL32m1:
	case RCL32mCL:
	case RCL32mi:
	case RCL32r1:
	case RCL32rCL:
	case RCL32ri:
	case RCL64m1:
	case RCL64mCL:
	case RCL64mi:
	case RCL64r1:
	case RCL64rCL:
	case RCL64ri:
	case RCL8m1:
	case RCL8mCL:
	case RCL8mi:
	case RCL8r1:
	case RCL8rCL:
	case RCL8ri:
		return true;
	}
	return false;
}

bool isRCR(unsigned Opcode) {
	switch (Opcode) {
	case RCR16m1:
	case RCR16mCL:
	case RCR16mi:
	case RCR16r1:
	case RCR16rCL:
	case RCR16ri:
	case RCR32m1:
	case RCR32mCL:
	case RCR32mi:
	case RCR32r1:
	case RCR32rCL:
	case RCR32ri:
	case RCR64m1:
	case RCR64mCL:
	case RCR64mi:
	case RCR64r1:
	case RCR64rCL:
	case RCR64ri:
	case RCR8m1:
	case RCR8mCL:
	case RCR8mi:
	case RCR8r1:
	case RCR8rCL:
	case RCR8ri:
		return true;
	}
	return false;
}

bool isBLENDPS(unsigned Opcode) {
	switch (Opcode) {
	case BLENDPSrmi:
	case BLENDPSrri:
		return true;
	}
	return false;
}

bool isVPEXPANDB(unsigned Opcode) {
	switch (Opcode) {
	case VPEXPANDBZ128rm:
	case VPEXPANDBZ128rmk:
	case VPEXPANDBZ128rmkz:
	case VPEXPANDBZ128rr:
	case VPEXPANDBZ128rrk:
	case VPEXPANDBZ128rrkz:
	case VPEXPANDBZ256rm:
	case VPEXPANDBZ256rmk:
	case VPEXPANDBZ256rmkz:
	case VPEXPANDBZ256rr:
	case VPEXPANDBZ256rrk:
	case VPEXPANDBZ256rrkz:
	case VPEXPANDBZrm:
	case VPEXPANDBZrmk:
	case VPEXPANDBZrmkz:
	case VPEXPANDBZrr:
	case VPEXPANDBZrrk:
	case VPEXPANDBZrrkz:
		return true;
	}
	return false;
}

bool isVEXPANDPD(unsigned Opcode) {
	switch (Opcode) {
	case VEXPANDPDZ128rm:
	case VEXPANDPDZ128rmk:
	case VEXPANDPDZ128rmkz:
	case VEXPANDPDZ128rr:
	case VEXPANDPDZ128rrk:
	case VEXPANDPDZ128rrkz:
	case VEXPANDPDZ256rm:
	case VEXPANDPDZ256rmk:
	case VEXPANDPDZ256rmkz:
	case VEXPANDPDZ256rr:
	case VEXPANDPDZ256rrk:
	case VEXPANDPDZ256rrkz:
	case VEXPANDPDZrm:
	case VEXPANDPDZrmk:
	case VEXPANDPDZrmkz:
	case VEXPANDPDZrr:
	case VEXPANDPDZrrk:
	case VEXPANDPDZrrkz:
		return true;
	}
	return false;
}

bool isVPEXPANDD(unsigned Opcode) {
	switch (Opcode) {
	case VPEXPANDDZ128rm:
	case VPEXPANDDZ128rmk:
	case VPEXPANDDZ128rmkz:
	case VPEXPANDDZ128rr:
	case VPEXPANDDZ128rrk:
	case VPEXPANDDZ128rrkz:
	case VPEXPANDDZ256rm:
	case VPEXPANDDZ256rmk:
	case VPEXPANDDZ256rmkz:
	case VPEXPANDDZ256rr:
	case VPEXPANDDZ256rrk:
	case VPEXPANDDZ256rrkz:
	case VPEXPANDDZrm:
	case VPEXPANDDZrmk:
	case VPEXPANDDZrmkz:
	case VPEXPANDDZrr:
	case VPEXPANDDZrrk:
	case VPEXPANDDZrrkz:
		return true;
	}
	return false;
}

bool isBLSMSK(unsigned Opcode) {
	switch (Opcode) {
	case BLSMSK32rm:
	case BLSMSK32rr:
	case BLSMSK64rm:
	case BLSMSK64rr:
		return true;
	}
	return false;
}

bool isXSUSLDTRK(unsigned Opcode) {
	return Opcode == XSUSLDTRK;
}

bool isGF2P8MULB(unsigned Opcode) {
	switch (Opcode) {
	case GF2P8MULBrm:
	case GF2P8MULBrr:
		return true;
	}
	return false;
}

bool isSIDTD(unsigned Opcode) {
	return Opcode == SIDT32m;
}

bool isVEXPANDPS(unsigned Opcode) {
	switch (Opcode) {
	case VEXPANDPSZ128rm:
	case VEXPANDPSZ128rmk:
	case VEXPANDPSZ128rmkz:
	case VEXPANDPSZ128rr:
	case VEXPANDPSZ128rrk:
	case VEXPANDPSZ128rrkz:
	case VEXPANDPSZ256rm:
	case VEXPANDPSZ256rmk:
	case VEXPANDPSZ256rmkz:
	case VEXPANDPSZ256rr:
	case VEXPANDPSZ256rrk:
	case VEXPANDPSZ256rrkz:
	case VEXPANDPSZrm:
	case VEXPANDPSZrmk:
	case VEXPANDPSZrmkz:
	case VEXPANDPSZrr:
	case VEXPANDPSZrrk:
	case VEXPANDPSZrrkz:
		return true;
	}
	return false;
}

bool isVFNMADDSD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADDSD4mr:
	case VFNMADDSD4rm:
	case VFNMADDSD4rr:
	case VFNMADDSD4rr_REV:
		return true;
	}
	return false;
}

bool isCBW(unsigned Opcode) {
	return Opcode == CBW;
}

bool isVPEXPANDW(unsigned Opcode) {
	switch (Opcode) {
	case VPEXPANDWZ128rm:
	case VPEXPANDWZ128rmk:
	case VPEXPANDWZ128rmkz:
	case VPEXPANDWZ128rr:
	case VPEXPANDWZ128rrk:
	case VPEXPANDWZ128rrkz:
	case VPEXPANDWZ256rm:
	case VPEXPANDWZ256rmk:
	case VPEXPANDWZ256rmkz:
	case VPEXPANDWZ256rr:
	case VPEXPANDWZ256rrk:
	case VPEXPANDWZ256rrkz:
	case VPEXPANDWZrm:
	case VPEXPANDWZrmk:
	case VPEXPANDWZrmkz:
	case VPEXPANDWZrr:
	case VPEXPANDWZrrk:
	case VPEXPANDWZrrkz:
		return true;
	}
	return false;
}

bool isVPEXPANDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPEXPANDQZ128rm:
	case VPEXPANDQZ128rmk:
	case VPEXPANDQZ128rmkz:
	case VPEXPANDQZ128rr:
	case VPEXPANDQZ128rrk:
	case VPEXPANDQZ128rrkz:
	case VPEXPANDQZ256rm:
	case VPEXPANDQZ256rmk:
	case VPEXPANDQZ256rmkz:
	case VPEXPANDQZ256rr:
	case VPEXPANDQZ256rrk:
	case VPEXPANDQZ256rrkz:
	case VPEXPANDQZrm:
	case VPEXPANDQZrmk:
	case VPEXPANDQZrmkz:
	case VPEXPANDQZrr:
	case VPEXPANDQZrrk:
	case VPEXPANDQZrrkz:
		return true;
	}
	return false;
}

bool isFXRSTOR64(unsigned Opcode) {
	return Opcode == FXRSTOR64;
}

bool isVFNMADDSS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADDSS4mr:
	case VFNMADDSS4rm:
	case VFNMADDSS4rr:
	case VFNMADDSS4rr_REV:
		return true;
	}
	return false;
}

bool isSIDTW(unsigned Opcode) {
	return Opcode == SIDT16m;
}

bool isCVTPD2PI(unsigned Opcode) {
	switch (Opcode) {
	case MMX_CVTPD2PIrm:
	case MMX_CVTPD2PIrr:
		return true;
	}
	return false;
}

bool isVCVTPS2PD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2PDYrm:
	case VCVTPS2PDYrr:
	case VCVTPS2PDZ128rm:
	case VCVTPS2PDZ128rmb:
	case VCVTPS2PDZ128rmbk:
	case VCVTPS2PDZ128rmbkz:
	case VCVTPS2PDZ128rmk:
	case VCVTPS2PDZ128rmkz:
	case VCVTPS2PDZ128rr:
	case VCVTPS2PDZ128rrk:
	case VCVTPS2PDZ128rrkz:
	case VCVTPS2PDZ256rm:
	case VCVTPS2PDZ256rmb:
	case VCVTPS2PDZ256rmbk:
	case VCVTPS2PDZ256rmbkz:
	case VCVTPS2PDZ256rmk:
	case VCVTPS2PDZ256rmkz:
	case VCVTPS2PDZ256rr:
	case VCVTPS2PDZ256rrk:
	case VCVTPS2PDZ256rrkz:
	case VCVTPS2PDZrm:
	case VCVTPS2PDZrmb:
	case VCVTPS2PDZrmbk:
	case VCVTPS2PDZrmbkz:
	case VCVTPS2PDZrmk:
	case VCVTPS2PDZrmkz:
	case VCVTPS2PDZrr:
	case VCVTPS2PDZrrb:
	case VCVTPS2PDZrrbk:
	case VCVTPS2PDZrrbkz:
	case VCVTPS2PDZrrk:
	case VCVTPS2PDZrrkz:
	case VCVTPS2PDrm:
	case VCVTPS2PDrr:
		return true;
	}
	return false;
}

bool isVFNMSUB132SD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB132SDZm_Int:
	case VFNMSUB132SDZm_Intk:
	case VFNMSUB132SDZm_Intkz:
	case VFNMSUB132SDZr_Int:
	case VFNMSUB132SDZr_Intk:
	case VFNMSUB132SDZr_Intkz:
	case VFNMSUB132SDZrb_Int:
	case VFNMSUB132SDZrb_Intk:
	case VFNMSUB132SDZrb_Intkz:
	case VFNMSUB132SDm_Int:
	case VFNMSUB132SDr_Int:
		return true;
	}
	return false;
}

bool isVCVTPS2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2PHYmr:
	case VCVTPS2PHYrr:
	case VCVTPS2PHZ128mr:
	case VCVTPS2PHZ128mrk:
	case VCVTPS2PHZ128rr:
	case VCVTPS2PHZ128rrk:
	case VCVTPS2PHZ128rrkz:
	case VCVTPS2PHZ256mr:
	case VCVTPS2PHZ256mrk:
	case VCVTPS2PHZ256rr:
	case VCVTPS2PHZ256rrk:
	case VCVTPS2PHZ256rrkz:
	case VCVTPS2PHZmr:
	case VCVTPS2PHZmrk:
	case VCVTPS2PHZrr:
	case VCVTPS2PHZrrb:
	case VCVTPS2PHZrrbk:
	case VCVTPS2PHZrrbkz:
	case VCVTPS2PHZrrk:
	case VCVTPS2PHZrrkz:
	case VCVTPS2PHmr:
	case VCVTPS2PHrr:
		return true;
	}
	return false;
}

bool isFIDIV(unsigned Opcode) {
	switch (Opcode) {
	case DIV_FI16m:
	case DIV_FI32m:
		return true;
	}
	return false;
}

bool isVFNMSUB132SH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB132SHZm_Int:
	case VFNMSUB132SHZm_Intk:
	case VFNMSUB132SHZm_Intkz:
	case VFNMSUB132SHZr_Int:
	case VFNMSUB132SHZr_Intk:
	case VFNMSUB132SHZr_Intkz:
	case VFNMSUB132SHZrb_Int:
	case VFNMSUB132SHZrb_Intk:
	case VFNMSUB132SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isCVTPD2PS(unsigned Opcode) {
	switch (Opcode) {
	case CVTPD2PSrm:
	case CVTPD2PSrr:
		return true;
	}
	return false;
}

bool isINVEPT(unsigned Opcode) {
	switch (Opcode) {
	case INVEPT32:
	case INVEPT64:
		return true;
	}
	return false;
}

bool isVPERMI2PD(unsigned Opcode) {
	switch (Opcode) {
	case VPERMI2PD128rm:
	case VPERMI2PD128rmb:
	case VPERMI2PD128rmbk:
	case VPERMI2PD128rmbkz:
	case VPERMI2PD128rmk:
	case VPERMI2PD128rmkz:
	case VPERMI2PD128rr:
	case VPERMI2PD128rrk:
	case VPERMI2PD128rrkz:
	case VPERMI2PD256rm:
	case VPERMI2PD256rmb:
	case VPERMI2PD256rmbk:
	case VPERMI2PD256rmbkz:
	case VPERMI2PD256rmk:
	case VPERMI2PD256rmkz:
	case VPERMI2PD256rr:
	case VPERMI2PD256rrk:
	case VPERMI2PD256rrkz:
	case VPERMI2PDrm:
	case VPERMI2PDrmb:
	case VPERMI2PDrmbk:
	case VPERMI2PDrmbkz:
	case VPERMI2PDrmk:
	case VPERMI2PDrmkz:
	case VPERMI2PDrr:
	case VPERMI2PDrrk:
	case VPERMI2PDrrkz:
		return true;
	}
	return false;
}

bool isVFMSUB213PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB213PDYm:
	case VFMSUB213PDYr:
	case VFMSUB213PDZ128m:
	case VFMSUB213PDZ128mb:
	case VFMSUB213PDZ128mbk:
	case VFMSUB213PDZ128mbkz:
	case VFMSUB213PDZ128mk:
	case VFMSUB213PDZ128mkz:
	case VFMSUB213PDZ128r:
	case VFMSUB213PDZ128rk:
	case VFMSUB213PDZ128rkz:
	case VFMSUB213PDZ256m:
	case VFMSUB213PDZ256mb:
	case VFMSUB213PDZ256mbk:
	case VFMSUB213PDZ256mbkz:
	case VFMSUB213PDZ256mk:
	case VFMSUB213PDZ256mkz:
	case VFMSUB213PDZ256r:
	case VFMSUB213PDZ256rk:
	case VFMSUB213PDZ256rkz:
	case VFMSUB213PDZm:
	case VFMSUB213PDZmb:
	case VFMSUB213PDZmbk:
	case VFMSUB213PDZmbkz:
	case VFMSUB213PDZmk:
	case VFMSUB213PDZmkz:
	case VFMSUB213PDZr:
	case VFMSUB213PDZrb:
	case VFMSUB213PDZrbk:
	case VFMSUB213PDZrbkz:
	case VFMSUB213PDZrk:
	case VFMSUB213PDZrkz:
	case VFMSUB213PDm:
	case VFMSUB213PDr:
		return true;
	}
	return false;
}

bool isVFNMSUB132SS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB132SSZm_Int:
	case VFNMSUB132SSZm_Intk:
	case VFNMSUB132SSZm_Intkz:
	case VFNMSUB132SSZr_Int:
	case VFNMSUB132SSZr_Intk:
	case VFNMSUB132SSZr_Intkz:
	case VFNMSUB132SSZrb_Int:
	case VFNMSUB132SSZrb_Intk:
	case VFNMSUB132SSZrb_Intkz:
	case VFNMSUB132SSm_Int:
	case VFNMSUB132SSr_Int:
		return true;
	}
	return false;
}

bool isINVVPID(unsigned Opcode) {
	switch (Opcode) {
	case INVVPID32:
	case INVVPID64:
		return true;
	}
	return false;
}

bool isVFMSUB213PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB213PHZ128m:
	case VFMSUB213PHZ128mb:
	case VFMSUB213PHZ128mbk:
	case VFMSUB213PHZ128mbkz:
	case VFMSUB213PHZ128mk:
	case VFMSUB213PHZ128mkz:
	case VFMSUB213PHZ128r:
	case VFMSUB213PHZ128rk:
	case VFMSUB213PHZ128rkz:
	case VFMSUB213PHZ256m:
	case VFMSUB213PHZ256mb:
	case VFMSUB213PHZ256mbk:
	case VFMSUB213PHZ256mbkz:
	case VFMSUB213PHZ256mk:
	case VFMSUB213PHZ256mkz:
	case VFMSUB213PHZ256r:
	case VFMSUB213PHZ256rk:
	case VFMSUB213PHZ256rkz:
	case VFMSUB213PHZm:
	case VFMSUB213PHZmb:
	case VFMSUB213PHZmbk:
	case VFMSUB213PHZmbkz:
	case VFMSUB213PHZmk:
	case VFMSUB213PHZmkz:
	case VFMSUB213PHZr:
	case VFMSUB213PHZrb:
	case VFMSUB213PHZrbk:
	case VFMSUB213PHZrbkz:
	case VFMSUB213PHZrk:
	case VFMSUB213PHZrkz:
		return true;
	}
	return false;
}

bool isVPCOMB(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMBmi:
	case VPCOMBri:
		return true;
	}
	return false;
}

bool isVPERMI2PS(unsigned Opcode) {
	switch (Opcode) {
	case VPERMI2PS128rm:
	case VPERMI2PS128rmb:
	case VPERMI2PS128rmbk:
	case VPERMI2PS128rmbkz:
	case VPERMI2PS128rmk:
	case VPERMI2PS128rmkz:
	case VPERMI2PS128rr:
	case VPERMI2PS128rrk:
	case VPERMI2PS128rrkz:
	case VPERMI2PS256rm:
	case VPERMI2PS256rmb:
	case VPERMI2PS256rmbk:
	case VPERMI2PS256rmbkz:
	case VPERMI2PS256rmk:
	case VPERMI2PS256rmkz:
	case VPERMI2PS256rr:
	case VPERMI2PS256rrk:
	case VPERMI2PS256rrkz:
	case VPERMI2PSrm:
	case VPERMI2PSrmb:
	case VPERMI2PSrmbk:
	case VPERMI2PSrmbkz:
	case VPERMI2PSrmk:
	case VPERMI2PSrmkz:
	case VPERMI2PSrr:
	case VPERMI2PSrrk:
	case VPERMI2PSrrkz:
		return true;
	}
	return false;
}

bool isVPCOMD(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMDmi:
	case VPCOMDri:
		return true;
	}
	return false;
}

bool isSMSW(unsigned Opcode) {
	switch (Opcode) {
	case SMSW16m:
	case SMSW16r:
	case SMSW32r:
	case SMSW64r:
		return true;
	}
	return false;
}

bool isRCPSS(unsigned Opcode) {
	switch (Opcode) {
	case RCPSSm_Int:
	case RCPSSr_Int:
		return true;
	}
	return false;
}

bool isRET(unsigned Opcode) {
	switch (Opcode) {
	case RET16:
	case RET32:
	case RET64:
	case RETI16:
	case RETI32:
	case RETI64:
		return true;
	}
	return false;
}

bool isVFMSUB213PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB213PSYm:
	case VFMSUB213PSYr:
	case VFMSUB213PSZ128m:
	case VFMSUB213PSZ128mb:
	case VFMSUB213PSZ128mbk:
	case VFMSUB213PSZ128mbkz:
	case VFMSUB213PSZ128mk:
	case VFMSUB213PSZ128mkz:
	case VFMSUB213PSZ128r:
	case VFMSUB213PSZ128rk:
	case VFMSUB213PSZ128rkz:
	case VFMSUB213PSZ256m:
	case VFMSUB213PSZ256mb:
	case VFMSUB213PSZ256mbk:
	case VFMSUB213PSZ256mbkz:
	case VFMSUB213PSZ256mk:
	case VFMSUB213PSZ256mkz:
	case VFMSUB213PSZ256r:
	case VFMSUB213PSZ256rk:
	case VFMSUB213PSZ256rkz:
	case VFMSUB213PSZm:
	case VFMSUB213PSZmb:
	case VFMSUB213PSZmbk:
	case VFMSUB213PSZmbkz:
	case VFMSUB213PSZmk:
	case VFMSUB213PSZmkz:
	case VFMSUB213PSZr:
	case VFMSUB213PSZrb:
	case VFMSUB213PSZrbk:
	case VFMSUB213PSZrbkz:
	case VFMSUB213PSZrk:
	case VFMSUB213PSZrkz:
	case VFMSUB213PSm:
	case VFMSUB213PSr:
		return true;
	}
	return false;
}

bool isVRSQRT14SD(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT14SDZrm:
	case VRSQRT14SDZrmk:
	case VRSQRT14SDZrmkz:
	case VRSQRT14SDZrr:
	case VRSQRT14SDZrrk:
	case VRSQRT14SDZrrkz:
		return true;
	}
	return false;
}

bool isCVTTPS2PI(unsigned Opcode) {
	switch (Opcode) {
	case MMX_CVTTPS2PIrm:
	case MMX_CVTTPS2PIrr:
		return true;
	}
	return false;
}

bool isVMCLEAR(unsigned Opcode) {
	return Opcode == VMCLEARm;
}

bool isVPCOMQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMQmi:
	case VPCOMQri:
		return true;
	}
	return false;
}

bool isCDQ(unsigned Opcode) {
	return Opcode == CDQ;
}

bool isFLDL2E(unsigned Opcode) {
	return Opcode == FLDL2E;
}

bool isVCVTPS2QQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2QQZ128rm:
	case VCVTPS2QQZ128rmb:
	case VCVTPS2QQZ128rmbk:
	case VCVTPS2QQZ128rmbkz:
	case VCVTPS2QQZ128rmk:
	case VCVTPS2QQZ128rmkz:
	case VCVTPS2QQZ128rr:
	case VCVTPS2QQZ128rrk:
	case VCVTPS2QQZ128rrkz:
	case VCVTPS2QQZ256rm:
	case VCVTPS2QQZ256rmb:
	case VCVTPS2QQZ256rmbk:
	case VCVTPS2QQZ256rmbkz:
	case VCVTPS2QQZ256rmk:
	case VCVTPS2QQZ256rmkz:
	case VCVTPS2QQZ256rr:
	case VCVTPS2QQZ256rrk:
	case VCVTPS2QQZ256rrkz:
	case VCVTPS2QQZrm:
	case VCVTPS2QQZrmb:
	case VCVTPS2QQZrmbk:
	case VCVTPS2QQZrmbkz:
	case VCVTPS2QQZrmk:
	case VCVTPS2QQZrmkz:
	case VCVTPS2QQZrr:
	case VCVTPS2QQZrrb:
	case VCVTPS2QQZrrbk:
	case VCVTPS2QQZrrbkz:
	case VCVTPS2QQZrrk:
	case VCVTPS2QQZrrkz:
		return true;
	}
	return false;
}

bool isVPCOMW(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMWmi:
	case VPCOMWri:
		return true;
	}
	return false;
}

bool isVRSQRT14SS(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRT14SSZrm:
	case VRSQRT14SSZrmk:
	case VRSQRT14SSZrmkz:
	case VRSQRT14SSZrr:
	case VRSQRT14SSZrrk:
	case VRSQRT14SSZrrkz:
		return true;
	}
	return false;
}

bool isVP4DPWSSDS(unsigned Opcode) {
	switch (Opcode) {
	case VP4DPWSSDSrm:
	case VP4DPWSSDSrmk:
	case VP4DPWSSDSrmkz:
		return true;
	}
	return false;
}

bool isVPSLLDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLDQYri:
	case VPSLLDQZ128mi:
	case VPSLLDQZ128ri:
	case VPSLLDQZ256mi:
	case VPSLLDQZ256ri:
	case VPSLLDQZmi:
	case VPSLLDQZri:
	case VPSLLDQri:
		return true;
	}
	return false;
}

bool isVCVTSD2SH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSD2SHZrm_Int:
	case VCVTSD2SHZrm_Intk:
	case VCVTSD2SHZrm_Intkz:
	case VCVTSD2SHZrr_Int:
	case VCVTSD2SHZrr_Intk:
	case VCVTSD2SHZrr_Intkz:
	case VCVTSD2SHZrrb_Int:
	case VCVTSD2SHZrrb_Intk:
	case VCVTSD2SHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isFLDL2T(unsigned Opcode) {
	return Opcode == FLDL2T;
}

bool isVEXP2PD(unsigned Opcode) {
	switch (Opcode) {
	case VEXP2PDZm:
	case VEXP2PDZmb:
	case VEXP2PDZmbk:
	case VEXP2PDZmbkz:
	case VEXP2PDZmk:
	case VEXP2PDZmkz:
	case VEXP2PDZr:
	case VEXP2PDZrb:
	case VEXP2PDZrbk:
	case VEXP2PDZrbkz:
	case VEXP2PDZrk:
	case VEXP2PDZrkz:
		return true;
	}
	return false;
}

bool isVFNMADD132PD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD132PDYm:
	case VFNMADD132PDYr:
	case VFNMADD132PDZ128m:
	case VFNMADD132PDZ128mb:
	case VFNMADD132PDZ128mbk:
	case VFNMADD132PDZ128mbkz:
	case VFNMADD132PDZ128mk:
	case VFNMADD132PDZ128mkz:
	case VFNMADD132PDZ128r:
	case VFNMADD132PDZ128rk:
	case VFNMADD132PDZ128rkz:
	case VFNMADD132PDZ256m:
	case VFNMADD132PDZ256mb:
	case VFNMADD132PDZ256mbk:
	case VFNMADD132PDZ256mbkz:
	case VFNMADD132PDZ256mk:
	case VFNMADD132PDZ256mkz:
	case VFNMADD132PDZ256r:
	case VFNMADD132PDZ256rk:
	case VFNMADD132PDZ256rkz:
	case VFNMADD132PDZm:
	case VFNMADD132PDZmb:
	case VFNMADD132PDZmbk:
	case VFNMADD132PDZmbkz:
	case VFNMADD132PDZmk:
	case VFNMADD132PDZmkz:
	case VFNMADD132PDZr:
	case VFNMADD132PDZrb:
	case VFNMADD132PDZrbk:
	case VFNMADD132PDZrbkz:
	case VFNMADD132PDZrk:
	case VFNMADD132PDZrkz:
	case VFNMADD132PDm:
	case VFNMADD132PDr:
		return true;
	}
	return false;
}

bool isVCVTSD2SI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSD2SI64Zrm_Int:
	case VCVTSD2SI64Zrr_Int:
	case VCVTSD2SI64Zrrb_Int:
	case VCVTSD2SI64rm_Int:
	case VCVTSD2SI64rr_Int:
	case VCVTSD2SIZrm_Int:
	case VCVTSD2SIZrr_Int:
	case VCVTSD2SIZrrb_Int:
	case VCVTSD2SIrm_Int:
	case VCVTSD2SIrr_Int:
		return true;
	}
	return false;
}

bool isVPOPCNTB(unsigned Opcode) {
	switch (Opcode) {
	case VPOPCNTBZ128rm:
	case VPOPCNTBZ128rmk:
	case VPOPCNTBZ128rmkz:
	case VPOPCNTBZ128rr:
	case VPOPCNTBZ128rrk:
	case VPOPCNTBZ128rrkz:
	case VPOPCNTBZ256rm:
	case VPOPCNTBZ256rmk:
	case VPOPCNTBZ256rmkz:
	case VPOPCNTBZ256rr:
	case VPOPCNTBZ256rrk:
	case VPOPCNTBZ256rrkz:
	case VPOPCNTBZrm:
	case VPOPCNTBZrmk:
	case VPOPCNTBZrmkz:
	case VPOPCNTBZrr:
	case VPOPCNTBZrrk:
	case VPOPCNTBZrrkz:
		return true;
	}
	return false;
}

bool isVFNMADD132PH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD132PHZ128m:
	case VFNMADD132PHZ128mb:
	case VFNMADD132PHZ128mbk:
	case VFNMADD132PHZ128mbkz:
	case VFNMADD132PHZ128mk:
	case VFNMADD132PHZ128mkz:
	case VFNMADD132PHZ128r:
	case VFNMADD132PHZ128rk:
	case VFNMADD132PHZ128rkz:
	case VFNMADD132PHZ256m:
	case VFNMADD132PHZ256mb:
	case VFNMADD132PHZ256mbk:
	case VFNMADD132PHZ256mbkz:
	case VFNMADD132PHZ256mk:
	case VFNMADD132PHZ256mkz:
	case VFNMADD132PHZ256r:
	case VFNMADD132PHZ256rk:
	case VFNMADD132PHZ256rkz:
	case VFNMADD132PHZm:
	case VFNMADD132PHZmb:
	case VFNMADD132PHZmbk:
	case VFNMADD132PHZmbkz:
	case VFNMADD132PHZmk:
	case VFNMADD132PHZmkz:
	case VFNMADD132PHZr:
	case VFNMADD132PHZrb:
	case VFNMADD132PHZrbk:
	case VFNMADD132PHZrbkz:
	case VFNMADD132PHZrk:
	case VFNMADD132PHZrkz:
		return true;
	}
	return false;
}

bool isVINSERTI128(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTI128rm:
	case VINSERTI128rr:
		return true;
	}
	return false;
}

bool isVPOPCNTD(unsigned Opcode) {
	switch (Opcode) {
	case VPOPCNTDZ128rm:
	case VPOPCNTDZ128rmb:
	case VPOPCNTDZ128rmbk:
	case VPOPCNTDZ128rmbkz:
	case VPOPCNTDZ128rmk:
	case VPOPCNTDZ128rmkz:
	case VPOPCNTDZ128rr:
	case VPOPCNTDZ128rrk:
	case VPOPCNTDZ128rrkz:
	case VPOPCNTDZ256rm:
	case VPOPCNTDZ256rmb:
	case VPOPCNTDZ256rmbk:
	case VPOPCNTDZ256rmbkz:
	case VPOPCNTDZ256rmk:
	case VPOPCNTDZ256rmkz:
	case VPOPCNTDZ256rr:
	case VPOPCNTDZ256rrk:
	case VPOPCNTDZ256rrkz:
	case VPOPCNTDZrm:
	case VPOPCNTDZrmb:
	case VPOPCNTDZrmbk:
	case VPOPCNTDZrmbkz:
	case VPOPCNTDZrmk:
	case VPOPCNTDZrmkz:
	case VPOPCNTDZrr:
	case VPOPCNTDZrrk:
	case VPOPCNTDZrrkz:
		return true;
	}
	return false;
}

bool isSETCC(unsigned Opcode) {
	switch (Opcode) {
	case SETCCm:
	case SETCCr:
		return true;
	}
	return false;
}

bool isVCVTSD2SS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSD2SSZrm_Int:
	case VCVTSD2SSZrm_Intk:
	case VCVTSD2SSZrm_Intkz:
	case VCVTSD2SSZrr_Int:
	case VCVTSD2SSZrr_Intk:
	case VCVTSD2SSZrr_Intkz:
	case VCVTSD2SSZrrb_Int:
	case VCVTSD2SSZrrb_Intk:
	case VCVTSD2SSZrrb_Intkz:
	case VCVTSD2SSrm_Int:
	case VCVTSD2SSrr_Int:
		return true;
	}
	return false;
}

bool isVEXP2PS(unsigned Opcode) {
	switch (Opcode) {
	case VEXP2PSZm:
	case VEXP2PSZmb:
	case VEXP2PSZmbk:
	case VEXP2PSZmbkz:
	case VEXP2PSZmk:
	case VEXP2PSZmkz:
	case VEXP2PSZr:
	case VEXP2PSZrb:
	case VEXP2PSZrbk:
	case VEXP2PSZrbkz:
	case VEXP2PSZrk:
	case VEXP2PSZrkz:
		return true;
	}
	return false;
}

bool isVFNMADD132PS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD132PSYm:
	case VFNMADD132PSYr:
	case VFNMADD132PSZ128m:
	case VFNMADD132PSZ128mb:
	case VFNMADD132PSZ128mbk:
	case VFNMADD132PSZ128mbkz:
	case VFNMADD132PSZ128mk:
	case VFNMADD132PSZ128mkz:
	case VFNMADD132PSZ128r:
	case VFNMADD132PSZ128rk:
	case VFNMADD132PSZ128rkz:
	case VFNMADD132PSZ256m:
	case VFNMADD132PSZ256mb:
	case VFNMADD132PSZ256mbk:
	case VFNMADD132PSZ256mbkz:
	case VFNMADD132PSZ256mk:
	case VFNMADD132PSZ256mkz:
	case VFNMADD132PSZ256r:
	case VFNMADD132PSZ256rk:
	case VFNMADD132PSZ256rkz:
	case VFNMADD132PSZm:
	case VFNMADD132PSZmb:
	case VFNMADD132PSZmbk:
	case VFNMADD132PSZmbkz:
	case VFNMADD132PSZmk:
	case VFNMADD132PSZmkz:
	case VFNMADD132PSZr:
	case VFNMADD132PSZrb:
	case VFNMADD132PSZrbk:
	case VFNMADD132PSZrbkz:
	case VFNMADD132PSZrk:
	case VFNMADD132PSZrkz:
	case VFNMADD132PSm:
	case VFNMADD132PSr:
		return true;
	}
	return false;
}

bool isMOVDIRI(unsigned Opcode) {
	switch (Opcode) {
	case MOVDIRI32:
	case MOVDIRI64:
		return true;
	}
	return false;
}

bool isVPOPCNTQ(unsigned Opcode) {
	switch (Opcode) {
	case VPOPCNTQZ128rm:
	case VPOPCNTQZ128rmb:
	case VPOPCNTQZ128rmbk:
	case VPOPCNTQZ128rmbkz:
	case VPOPCNTQZ128rmk:
	case VPOPCNTQZ128rmkz:
	case VPOPCNTQZ128rr:
	case VPOPCNTQZ128rrk:
	case VPOPCNTQZ128rrkz:
	case VPOPCNTQZ256rm:
	case VPOPCNTQZ256rmb:
	case VPOPCNTQZ256rmbk:
	case VPOPCNTQZ256rmbkz:
	case VPOPCNTQZ256rmk:
	case VPOPCNTQZ256rmkz:
	case VPOPCNTQZ256rr:
	case VPOPCNTQZ256rrk:
	case VPOPCNTQZ256rrkz:
	case VPOPCNTQZrm:
	case VPOPCNTQZrmb:
	case VPOPCNTQZrmbk:
	case VPOPCNTQZrmbkz:
	case VPOPCNTQZrmk:
	case VPOPCNTQZrmkz:
	case VPOPCNTQZrr:
	case VPOPCNTQZrrk:
	case VPOPCNTQZrrkz:
		return true;
	}
	return false;
}

bool isVFMSUB231PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB231PDYm:
	case VFMSUB231PDYr:
	case VFMSUB231PDZ128m:
	case VFMSUB231PDZ128mb:
	case VFMSUB231PDZ128mbk:
	case VFMSUB231PDZ128mbkz:
	case VFMSUB231PDZ128mk:
	case VFMSUB231PDZ128mkz:
	case VFMSUB231PDZ128r:
	case VFMSUB231PDZ128rk:
	case VFMSUB231PDZ128rkz:
	case VFMSUB231PDZ256m:
	case VFMSUB231PDZ256mb:
	case VFMSUB231PDZ256mbk:
	case VFMSUB231PDZ256mbkz:
	case VFMSUB231PDZ256mk:
	case VFMSUB231PDZ256mkz:
	case VFMSUB231PDZ256r:
	case VFMSUB231PDZ256rk:
	case VFMSUB231PDZ256rkz:
	case VFMSUB231PDZm:
	case VFMSUB231PDZmb:
	case VFMSUB231PDZmbk:
	case VFMSUB231PDZmbkz:
	case VFMSUB231PDZmk:
	case VFMSUB231PDZmkz:
	case VFMSUB231PDZr:
	case VFMSUB231PDZrb:
	case VFMSUB231PDZrbk:
	case VFMSUB231PDZrbkz:
	case VFMSUB231PDZrk:
	case VFMSUB231PDZrkz:
	case VFMSUB231PDm:
	case VFMSUB231PDr:
		return true;
	}
	return false;
}

bool isVFMSUB231PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB231PHZ128m:
	case VFMSUB231PHZ128mb:
	case VFMSUB231PHZ128mbk:
	case VFMSUB231PHZ128mbkz:
	case VFMSUB231PHZ128mk:
	case VFMSUB231PHZ128mkz:
	case VFMSUB231PHZ128r:
	case VFMSUB231PHZ128rk:
	case VFMSUB231PHZ128rkz:
	case VFMSUB231PHZ256m:
	case VFMSUB231PHZ256mb:
	case VFMSUB231PHZ256mbk:
	case VFMSUB231PHZ256mbkz:
	case VFMSUB231PHZ256mk:
	case VFMSUB231PHZ256mkz:
	case VFMSUB231PHZ256r:
	case VFMSUB231PHZ256rk:
	case VFMSUB231PHZ256rkz:
	case VFMSUB231PHZm:
	case VFMSUB231PHZmb:
	case VFMSUB231PHZmbk:
	case VFMSUB231PHZmbkz:
	case VFMSUB231PHZmk:
	case VFMSUB231PHZmkz:
	case VFMSUB231PHZr:
	case VFMSUB231PHZrb:
	case VFMSUB231PHZrbk:
	case VFMSUB231PHZrbkz:
	case VFMSUB231PHZrk:
	case VFMSUB231PHZrkz:
		return true;
	}
	return false;
}

bool isFCMOVBE(unsigned Opcode) {
	return Opcode == CMOVBE_F;
}

bool isVPOPCNTW(unsigned Opcode) {
	switch (Opcode) {
	case VPOPCNTWZ128rm:
	case VPOPCNTWZ128rmk:
	case VPOPCNTWZ128rmkz:
	case VPOPCNTWZ128rr:
	case VPOPCNTWZ128rrk:
	case VPOPCNTWZ128rrkz:
	case VPOPCNTWZ256rm:
	case VPOPCNTWZ256rmk:
	case VPOPCNTWZ256rmkz:
	case VPOPCNTWZ256rr:
	case VPOPCNTWZ256rrk:
	case VPOPCNTWZ256rrkz:
	case VPOPCNTWZrm:
	case VPOPCNTWZrmk:
	case VPOPCNTWZrmkz:
	case VPOPCNTWZrr:
	case VPOPCNTWZrrk:
	case VPOPCNTWZrrkz:
		return true;
	}
	return false;
}

bool isCVTTSD2SI(unsigned Opcode) {
	switch (Opcode) {
	case CVTTSD2SI64rm_Int:
	case CVTTSD2SI64rr_Int:
	case CVTTSD2SIrm_Int:
	case CVTTSD2SIrr_Int:
		return true;
	}
	return false;
}

bool isVFMSUB231PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB231PSYm:
	case VFMSUB231PSYr:
	case VFMSUB231PSZ128m:
	case VFMSUB231PSZ128mb:
	case VFMSUB231PSZ128mbk:
	case VFMSUB231PSZ128mbkz:
	case VFMSUB231PSZ128mk:
	case VFMSUB231PSZ128mkz:
	case VFMSUB231PSZ128r:
	case VFMSUB231PSZ128rk:
	case VFMSUB231PSZ128rkz:
	case VFMSUB231PSZ256m:
	case VFMSUB231PSZ256mb:
	case VFMSUB231PSZ256mbk:
	case VFMSUB231PSZ256mbkz:
	case VFMSUB231PSZ256mk:
	case VFMSUB231PSZ256mkz:
	case VFMSUB231PSZ256r:
	case VFMSUB231PSZ256rk:
	case VFMSUB231PSZ256rkz:
	case VFMSUB231PSZm:
	case VFMSUB231PSZmb:
	case VFMSUB231PSZmbk:
	case VFMSUB231PSZmbkz:
	case VFMSUB231PSZmk:
	case VFMSUB231PSZmkz:
	case VFMSUB231PSZr:
	case VFMSUB231PSZrb:
	case VFMSUB231PSZrbk:
	case VFMSUB231PSZrbkz:
	case VFMSUB231PSZrk:
	case VFMSUB231PSZrkz:
	case VFMSUB231PSm:
	case VFMSUB231PSr:
		return true;
	}
	return false;
}

bool isNOP(unsigned Opcode) {
	switch (Opcode) {
	case NOOP:
	case NOOPL:
	case NOOPLr:
	case NOOPQ:
	case NOOPQr:
	case NOOPW:
	case NOOPWr:
		return true;
	}
	return false;
}

bool isNOT(unsigned Opcode) {
	switch (Opcode) {
	case NOT16m:
	case NOT16r:
	case NOT32m:
	case NOT32r:
	case NOT64m:
	case NOT64r:
	case NOT8m:
	case NOT8r:
		return true;
	}
	return false;
}

bool isTPAUSE(unsigned Opcode) {
	return Opcode == TPAUSE;
}

bool isVCVTNEPS2BF16(unsigned Opcode) {
	switch (Opcode) {
	case VCVTNEPS2BF16Yrm:
	case VCVTNEPS2BF16Yrr:
	case VCVTNEPS2BF16Z128rm:
	case VCVTNEPS2BF16Z128rmb:
	case VCVTNEPS2BF16Z128rmbk:
	case VCVTNEPS2BF16Z128rmbkz:
	case VCVTNEPS2BF16Z128rmk:
	case VCVTNEPS2BF16Z128rmkz:
	case VCVTNEPS2BF16Z128rr:
	case VCVTNEPS2BF16Z128rrk:
	case VCVTNEPS2BF16Z128rrkz:
	case VCVTNEPS2BF16Z256rm:
	case VCVTNEPS2BF16Z256rmb:
	case VCVTNEPS2BF16Z256rmbk:
	case VCVTNEPS2BF16Z256rmbkz:
	case VCVTNEPS2BF16Z256rmk:
	case VCVTNEPS2BF16Z256rmkz:
	case VCVTNEPS2BF16Z256rr:
	case VCVTNEPS2BF16Z256rrk:
	case VCVTNEPS2BF16Z256rrkz:
	case VCVTNEPS2BF16Zrm:
	case VCVTNEPS2BF16Zrmb:
	case VCVTNEPS2BF16Zrmbk:
	case VCVTNEPS2BF16Zrmbkz:
	case VCVTNEPS2BF16Zrmk:
	case VCVTNEPS2BF16Zrmkz:
	case VCVTNEPS2BF16Zrr:
	case VCVTNEPS2BF16Zrrk:
	case VCVTNEPS2BF16Zrrkz:
	case VCVTNEPS2BF16rm:
	case VCVTNEPS2BF16rr:
		return true;
	}
	return false;
}

bool isVFMSUB213SD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB213SDZm_Int:
	case VFMSUB213SDZm_Intk:
	case VFMSUB213SDZm_Intkz:
	case VFMSUB213SDZr_Int:
	case VFMSUB213SDZr_Intk:
	case VFMSUB213SDZr_Intkz:
	case VFMSUB213SDZrb_Int:
	case VFMSUB213SDZrb_Intk:
	case VFMSUB213SDZrb_Intkz:
	case VFMSUB213SDm_Int:
	case VFMSUB213SDr_Int:
		return true;
	}
	return false;
}

bool isVFMSUB213SH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB213SHZm_Int:
	case VFMSUB213SHZm_Intk:
	case VFMSUB213SHZm_Intkz:
	case VFMSUB213SHZr_Int:
	case VFMSUB213SHZr_Intk:
	case VFMSUB213SHZr_Intkz:
	case VFMSUB213SHZrb_Int:
	case VFMSUB213SHZrb_Intk:
	case VFMSUB213SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVFMADDCPH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDCPHZ128m:
	case VFMADDCPHZ128mb:
	case VFMADDCPHZ128mbk:
	case VFMADDCPHZ128mbkz:
	case VFMADDCPHZ128mk:
	case VFMADDCPHZ128mkz:
	case VFMADDCPHZ128r:
	case VFMADDCPHZ128rk:
	case VFMADDCPHZ128rkz:
	case VFMADDCPHZ256m:
	case VFMADDCPHZ256mb:
	case VFMADDCPHZ256mbk:
	case VFMADDCPHZ256mbkz:
	case VFMADDCPHZ256mk:
	case VFMADDCPHZ256mkz:
	case VFMADDCPHZ256r:
	case VFMADDCPHZ256rk:
	case VFMADDCPHZ256rkz:
	case VFMADDCPHZm:
	case VFMADDCPHZmb:
	case VFMADDCPHZmbk:
	case VFMADDCPHZmbkz:
	case VFMADDCPHZmk:
	case VFMADDCPHZmkz:
	case VFMADDCPHZr:
	case VFMADDCPHZrb:
	case VFMADDCPHZrbk:
	case VFMADDCPHZrbkz:
	case VFMADDCPHZrk:
	case VFMADDCPHZrkz:
		return true;
	}
	return false;
}

bool isVEXTRACTI64X2(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTI64x2Z256mr:
	case VEXTRACTI64x2Z256mrk:
	case VEXTRACTI64x2Z256rr:
	case VEXTRACTI64x2Z256rrk:
	case VEXTRACTI64x2Z256rrkz:
	case VEXTRACTI64x2Zmr:
	case VEXTRACTI64x2Zmrk:
	case VEXTRACTI64x2Zrr:
	case VEXTRACTI64x2Zrrk:
	case VEXTRACTI64x2Zrrkz:
		return true;
	}
	return false;
}

bool isVFMSUB213SS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB213SSZm_Int:
	case VFMSUB213SSZm_Intk:
	case VFMSUB213SSZm_Intkz:
	case VFMSUB213SSZr_Int:
	case VFMSUB213SSZr_Intk:
	case VFMSUB213SSZr_Intkz:
	case VFMSUB213SSZrb_Int:
	case VFMSUB213SSZrb_Intk:
	case VFMSUB213SSZrb_Intkz:
	case VFMSUB213SSm_Int:
	case VFMSUB213SSr_Int:
		return true;
	}
	return false;
}

bool isVEXTRACTI64X4(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTI64x4Zmr:
	case VEXTRACTI64x4Zmrk:
	case VEXTRACTI64x4Zrr:
	case VEXTRACTI64x4Zrrk:
	case VEXTRACTI64x4Zrrkz:
		return true;
	}
	return false;
}

bool isXSAVEC64(unsigned Opcode) {
	return Opcode == XSAVEC64;
}

bool isBLCFILL(unsigned Opcode) {
	switch (Opcode) {
	case BLCFILL32rm:
	case BLCFILL32rr:
	case BLCFILL64rm:
	case BLCFILL64rr:
		return true;
	}
	return false;
}

bool isXBEGIN(unsigned Opcode) {
	switch (Opcode) {
	case XBEGIN_2:
	case XBEGIN_4:
		return true;
	}
	return false;
}

bool isXCRYPTOFB(unsigned Opcode) {
	return Opcode == XCRYPTOFB;
}

bool isFUCOMI(unsigned Opcode) {
	return Opcode == UCOM_FIr;
}

bool isVMOVHPD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVHPDZ128mr:
	case VMOVHPDZ128rm:
	case VMOVHPDmr:
	case VMOVHPDrm:
		return true;
	}
	return false;
}

bool isMASKMOVDQU(unsigned Opcode) {
	switch (Opcode) {
	case MASKMOVDQU:
	case MASKMOVDQU64:
		return true;
	}
	return false;
}

bool isFUCOMP(unsigned Opcode) {
	return Opcode == UCOM_FPr;
}

bool isVFNMADD132SD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD132SDZm_Int:
	case VFNMADD132SDZm_Intk:
	case VFNMADD132SDZm_Intkz:
	case VFNMADD132SDZr_Int:
	case VFNMADD132SDZr_Intk:
	case VFNMADD132SDZr_Intkz:
	case VFNMADD132SDZrb_Int:
	case VFNMADD132SDZrb_Intk:
	case VFNMADD132SDZrb_Intkz:
	case VFNMADD132SDm_Int:
	case VFNMADD132SDr_Int:
		return true;
	}
	return false;
}

bool isMFENCE(unsigned Opcode) {
	return Opcode == MFENCE;
}

bool isVFNMADD132SH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD132SHZm_Int:
	case VFNMADD132SHZm_Intk:
	case VFNMADD132SHZm_Intkz:
	case VFNMADD132SHZr_Int:
	case VFNMADD132SHZr_Intk:
	case VFNMADD132SHZr_Intkz:
	case VFNMADD132SHZrb_Int:
	case VFNMADD132SHZrb_Intk:
	case VFNMADD132SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVMOVHPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVHPSZ128mr:
	case VMOVHPSZ128rm:
	case VMOVHPSmr:
	case VMOVHPSrm:
		return true;
	}
	return false;
}

bool isVPBROADCASTMW2D(unsigned Opcode) {
	switch (Opcode) {
	case VPBROADCASTMW2DZ128rr:
	case VPBROADCASTMW2DZ256rr:
	case VPBROADCASTMW2DZrr:
		return true;
	}
	return false;
}

bool isVFMADD213PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD213PDYm:
	case VFMADD213PDYr:
	case VFMADD213PDZ128m:
	case VFMADD213PDZ128mb:
	case VFMADD213PDZ128mbk:
	case VFMADD213PDZ128mbkz:
	case VFMADD213PDZ128mk:
	case VFMADD213PDZ128mkz:
	case VFMADD213PDZ128r:
	case VFMADD213PDZ128rk:
	case VFMADD213PDZ128rkz:
	case VFMADD213PDZ256m:
	case VFMADD213PDZ256mb:
	case VFMADD213PDZ256mbk:
	case VFMADD213PDZ256mbkz:
	case VFMADD213PDZ256mk:
	case VFMADD213PDZ256mkz:
	case VFMADD213PDZ256r:
	case VFMADD213PDZ256rk:
	case VFMADD213PDZ256rkz:
	case VFMADD213PDZm:
	case VFMADD213PDZmb:
	case VFMADD213PDZmbk:
	case VFMADD213PDZmbkz:
	case VFMADD213PDZmk:
	case VFMADD213PDZmkz:
	case VFMADD213PDZr:
	case VFMADD213PDZrb:
	case VFMADD213PDZrbk:
	case VFMADD213PDZrbkz:
	case VFMADD213PDZrk:
	case VFMADD213PDZrkz:
	case VFMADD213PDm:
	case VFMADD213PDr:
		return true;
	}
	return false;
}

bool isVFNMADD132SS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD132SSZm_Int:
	case VFNMADD132SSZm_Intk:
	case VFNMADD132SSZm_Intkz:
	case VFNMADD132SSZr_Int:
	case VFNMADD132SSZr_Intk:
	case VFNMADD132SSZr_Intkz:
	case VFNMADD132SSZrb_Int:
	case VFNMADD132SSZrb_Intk:
	case VFNMADD132SSZrb_Intkz:
	case VFNMADD132SSm_Int:
	case VFNMADD132SSr_Int:
		return true;
	}
	return false;
}

bool isVFMADD213PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD213PHZ128m:
	case VFMADD213PHZ128mb:
	case VFMADD213PHZ128mbk:
	case VFMADD213PHZ128mbkz:
	case VFMADD213PHZ128mk:
	case VFMADD213PHZ128mkz:
	case VFMADD213PHZ128r:
	case VFMADD213PHZ128rk:
	case VFMADD213PHZ128rkz:
	case VFMADD213PHZ256m:
	case VFMADD213PHZ256mb:
	case VFMADD213PHZ256mbk:
	case VFMADD213PHZ256mbkz:
	case VFMADD213PHZ256mk:
	case VFMADD213PHZ256mkz:
	case VFMADD213PHZ256r:
	case VFMADD213PHZ256rk:
	case VFMADD213PHZ256rkz:
	case VFMADD213PHZm:
	case VFMADD213PHZmb:
	case VFMADD213PHZmbk:
	case VFMADD213PHZmbkz:
	case VFMADD213PHZmk:
	case VFMADD213PHZmkz:
	case VFMADD213PHZr:
	case VFMADD213PHZrb:
	case VFMADD213PHZrbk:
	case VFMADD213PHZrbkz:
	case VFMADD213PHZrk:
	case VFMADD213PHZrkz:
		return true;
	}
	return false;
}

bool isVFMSUB231SD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB231SDZm_Int:
	case VFMSUB231SDZm_Intk:
	case VFMSUB231SDZm_Intkz:
	case VFMSUB231SDZr_Int:
	case VFMSUB231SDZr_Intk:
	case VFMSUB231SDZr_Intkz:
	case VFMSUB231SDZrb_Int:
	case VFMSUB231SDZrb_Intk:
	case VFMSUB231SDZrb_Intkz:
	case VFMSUB231SDm_Int:
	case VFMSUB231SDr_Int:
		return true;
	}
	return false;
}

bool isVPMULTISHIFTQB(unsigned Opcode) {
	switch (Opcode) {
	case VPMULTISHIFTQBZ128rm:
	case VPMULTISHIFTQBZ128rmb:
	case VPMULTISHIFTQBZ128rmbk:
	case VPMULTISHIFTQBZ128rmbkz:
	case VPMULTISHIFTQBZ128rmk:
	case VPMULTISHIFTQBZ128rmkz:
	case VPMULTISHIFTQBZ128rr:
	case VPMULTISHIFTQBZ128rrk:
	case VPMULTISHIFTQBZ128rrkz:
	case VPMULTISHIFTQBZ256rm:
	case VPMULTISHIFTQBZ256rmb:
	case VPMULTISHIFTQBZ256rmbk:
	case VPMULTISHIFTQBZ256rmbkz:
	case VPMULTISHIFTQBZ256rmk:
	case VPMULTISHIFTQBZ256rmkz:
	case VPMULTISHIFTQBZ256rr:
	case VPMULTISHIFTQBZ256rrk:
	case VPMULTISHIFTQBZ256rrkz:
	case VPMULTISHIFTQBZrm:
	case VPMULTISHIFTQBZrmb:
	case VPMULTISHIFTQBZrmbk:
	case VPMULTISHIFTQBZrmbkz:
	case VPMULTISHIFTQBZrmk:
	case VPMULTISHIFTQBZrmkz:
	case VPMULTISHIFTQBZrr:
	case VPMULTISHIFTQBZrrk:
	case VPMULTISHIFTQBZrrkz:
		return true;
	}
	return false;
}

bool isFNSAVE(unsigned Opcode) {
	return Opcode == FSAVEm;
}

bool isVFMSUB231SH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB231SHZm_Int:
	case VFMSUB231SHZm_Intk:
	case VFMSUB231SHZm_Intkz:
	case VFMSUB231SHZr_Int:
	case VFMSUB231SHZr_Intk:
	case VFMSUB231SHZr_Intkz:
	case VFMSUB231SHZrb_Int:
	case VFMSUB231SHZrb_Intk:
	case VFMSUB231SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVSHUFF32X4(unsigned Opcode) {
	switch (Opcode) {
	case VSHUFF32X4Z256rmbi:
	case VSHUFF32X4Z256rmbik:
	case VSHUFF32X4Z256rmbikz:
	case VSHUFF32X4Z256rmi:
	case VSHUFF32X4Z256rmik:
	case VSHUFF32X4Z256rmikz:
	case VSHUFF32X4Z256rri:
	case VSHUFF32X4Z256rrik:
	case VSHUFF32X4Z256rrikz:
	case VSHUFF32X4Zrmbi:
	case VSHUFF32X4Zrmbik:
	case VSHUFF32X4Zrmbikz:
	case VSHUFF32X4Zrmi:
	case VSHUFF32X4Zrmik:
	case VSHUFF32X4Zrmikz:
	case VSHUFF32X4Zrri:
	case VSHUFF32X4Zrrik:
	case VSHUFF32X4Zrrikz:
		return true;
	}
	return false;
}

bool isPMOVSXWD(unsigned Opcode) {
	switch (Opcode) {
	case PMOVSXWDrm:
	case PMOVSXWDrr:
		return true;
	}
	return false;
}

bool isVFMADD213PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD213PSYm:
	case VFMADD213PSYr:
	case VFMADD213PSZ128m:
	case VFMADD213PSZ128mb:
	case VFMADD213PSZ128mbk:
	case VFMADD213PSZ128mbkz:
	case VFMADD213PSZ128mk:
	case VFMADD213PSZ128mkz:
	case VFMADD213PSZ128r:
	case VFMADD213PSZ128rk:
	case VFMADD213PSZ128rkz:
	case VFMADD213PSZ256m:
	case VFMADD213PSZ256mb:
	case VFMADD213PSZ256mbk:
	case VFMADD213PSZ256mbkz:
	case VFMADD213PSZ256mk:
	case VFMADD213PSZ256mkz:
	case VFMADD213PSZ256r:
	case VFMADD213PSZ256rk:
	case VFMADD213PSZ256rkz:
	case VFMADD213PSZm:
	case VFMADD213PSZmb:
	case VFMADD213PSZmbk:
	case VFMADD213PSZmbkz:
	case VFMADD213PSZmk:
	case VFMADD213PSZmkz:
	case VFMADD213PSZr:
	case VFMADD213PSZrb:
	case VFMADD213PSZrbk:
	case VFMADD213PSZrbkz:
	case VFMADD213PSZrk:
	case VFMADD213PSZrkz:
	case VFMADD213PSm:
	case VFMADD213PSr:
		return true;
	}
	return false;
}

bool isVFMSUB231SS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUB231SSZm_Int:
	case VFMSUB231SSZm_Intk:
	case VFMSUB231SSZm_Intkz:
	case VFMSUB231SSZr_Int:
	case VFMSUB231SSZr_Intk:
	case VFMSUB231SSZr_Intkz:
	case VFMSUB231SSZrb_Int:
	case VFMSUB231SSZrb_Intk:
	case VFMSUB231SSZrb_Intkz:
	case VFMSUB231SSm_Int:
	case VFMSUB231SSr_Int:
		return true;
	}
	return false;
}

bool isVPCOMPRESSB(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMPRESSBZ128mr:
	case VPCOMPRESSBZ128mrk:
	case VPCOMPRESSBZ128rr:
	case VPCOMPRESSBZ128rrk:
	case VPCOMPRESSBZ128rrkz:
	case VPCOMPRESSBZ256mr:
	case VPCOMPRESSBZ256mrk:
	case VPCOMPRESSBZ256rr:
	case VPCOMPRESSBZ256rrk:
	case VPCOMPRESSBZ256rrkz:
	case VPCOMPRESSBZmr:
	case VPCOMPRESSBZmrk:
	case VPCOMPRESSBZrr:
	case VPCOMPRESSBZrrk:
	case VPCOMPRESSBZrrkz:
		return true;
	}
	return false;
}

bool isPMOVSXWQ(unsigned Opcode) {
	switch (Opcode) {
	case PMOVSXWQrm:
	case PMOVSXWQrr:
		return true;
	}
	return false;
}

bool isVPCOMPRESSD(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMPRESSDZ128mr:
	case VPCOMPRESSDZ128mrk:
	case VPCOMPRESSDZ128rr:
	case VPCOMPRESSDZ128rrk:
	case VPCOMPRESSDZ128rrkz:
	case VPCOMPRESSDZ256mr:
	case VPCOMPRESSDZ256mrk:
	case VPCOMPRESSDZ256rr:
	case VPCOMPRESSDZ256rrk:
	case VPCOMPRESSDZ256rrkz:
	case VPCOMPRESSDZmr:
	case VPCOMPRESSDZmrk:
	case VPCOMPRESSDZrr:
	case VPCOMPRESSDZrrk:
	case VPCOMPRESSDZrrkz:
		return true;
	}
	return false;
}

bool isVPABSB(unsigned Opcode) {
	switch (Opcode) {
	case VPABSBYrm:
	case VPABSBYrr:
	case VPABSBZ128rm:
	case VPABSBZ128rmk:
	case VPABSBZ128rmkz:
	case VPABSBZ128rr:
	case VPABSBZ128rrk:
	case VPABSBZ128rrkz:
	case VPABSBZ256rm:
	case VPABSBZ256rmk:
	case VPABSBZ256rmkz:
	case VPABSBZ256rr:
	case VPABSBZ256rrk:
	case VPABSBZ256rrkz:
	case VPABSBZrm:
	case VPABSBZrmk:
	case VPABSBZrmkz:
	case VPABSBZrr:
	case VPABSBZrrk:
	case VPABSBZrrkz:
	case VPABSBrm:
	case VPABSBrr:
		return true;
	}
	return false;
}

bool isVMOVNTDQA(unsigned Opcode) {
	switch (Opcode) {
	case VMOVNTDQAYrm:
	case VMOVNTDQAZ128rm:
	case VMOVNTDQAZ256rm:
	case VMOVNTDQAZrm:
	case VMOVNTDQArm:
		return true;
	}
	return false;
}

bool isVPABSD(unsigned Opcode) {
	switch (Opcode) {
	case VPABSDYrm:
	case VPABSDYrr:
	case VPABSDZ128rm:
	case VPABSDZ128rmb:
	case VPABSDZ128rmbk:
	case VPABSDZ128rmbkz:
	case VPABSDZ128rmk:
	case VPABSDZ128rmkz:
	case VPABSDZ128rr:
	case VPABSDZ128rrk:
	case VPABSDZ128rrkz:
	case VPABSDZ256rm:
	case VPABSDZ256rmb:
	case VPABSDZ256rmbk:
	case VPABSDZ256rmbkz:
	case VPABSDZ256rmk:
	case VPABSDZ256rmkz:
	case VPABSDZ256rr:
	case VPABSDZ256rrk:
	case VPABSDZ256rrkz:
	case VPABSDZrm:
	case VPABSDZrmb:
	case VPABSDZrmbk:
	case VPABSDZrmbkz:
	case VPABSDZrmk:
	case VPABSDZrmkz:
	case VPABSDZrr:
	case VPABSDZrrk:
	case VPABSDZrrkz:
	case VPABSDrm:
	case VPABSDrr:
		return true;
	}
	return false;
}

bool isVPCOMPRESSQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMPRESSQZ128mr:
	case VPCOMPRESSQZ128mrk:
	case VPCOMPRESSQZ128rr:
	case VPCOMPRESSQZ128rrk:
	case VPCOMPRESSQZ128rrkz:
	case VPCOMPRESSQZ256mr:
	case VPCOMPRESSQZ256mrk:
	case VPCOMPRESSQZ256rr:
	case VPCOMPRESSQZ256rrk:
	case VPCOMPRESSQZ256rrkz:
	case VPCOMPRESSQZmr:
	case VPCOMPRESSQZmrk:
	case VPCOMPRESSQZrr:
	case VPCOMPRESSQZrrk:
	case VPCOMPRESSQZrrkz:
		return true;
	}
	return false;
}

bool isVINSERTI64X2(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTI64x2Z256rm:
	case VINSERTI64x2Z256rmk:
	case VINSERTI64x2Z256rmkz:
	case VINSERTI64x2Z256rr:
	case VINSERTI64x2Z256rrk:
	case VINSERTI64x2Z256rrkz:
	case VINSERTI64x2Zrm:
	case VINSERTI64x2Zrmk:
	case VINSERTI64x2Zrmkz:
	case VINSERTI64x2Zrr:
	case VINSERTI64x2Zrrk:
	case VINSERTI64x2Zrrkz:
		return true;
	}
	return false;
}

bool isVPCOMPRESSW(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMPRESSWZ128mr:
	case VPCOMPRESSWZ128mrk:
	case VPCOMPRESSWZ128rr:
	case VPCOMPRESSWZ128rrk:
	case VPCOMPRESSWZ128rrkz:
	case VPCOMPRESSWZ256mr:
	case VPCOMPRESSWZ256mrk:
	case VPCOMPRESSWZ256rr:
	case VPCOMPRESSWZ256rrk:
	case VPCOMPRESSWZ256rrkz:
	case VPCOMPRESSWZmr:
	case VPCOMPRESSWZmrk:
	case VPCOMPRESSWZrr:
	case VPCOMPRESSWZrrk:
	case VPCOMPRESSWZrrkz:
		return true;
	}
	return false;
}

bool isVINSERTI64X4(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTI64x4Zrm:
	case VINSERTI64x4Zrmk:
	case VINSERTI64x4Zrmkz:
	case VINSERTI64x4Zrr:
	case VINSERTI64x4Zrrk:
	case VINSERTI64x4Zrrkz:
		return true;
	}
	return false;
}

bool isVPABSQ(unsigned Opcode) {
	switch (Opcode) {
	case VPABSQZ128rm:
	case VPABSQZ128rmb:
	case VPABSQZ128rmbk:
	case VPABSQZ128rmbkz:
	case VPABSQZ128rmk:
	case VPABSQZ128rmkz:
	case VPABSQZ128rr:
	case VPABSQZ128rrk:
	case VPABSQZ128rrkz:
	case VPABSQZ256rm:
	case VPABSQZ256rmb:
	case VPABSQZ256rmbk:
	case VPABSQZ256rmbkz:
	case VPABSQZ256rmk:
	case VPABSQZ256rmkz:
	case VPABSQZ256rr:
	case VPABSQZ256rrk:
	case VPABSQZ256rrkz:
	case VPABSQZrm:
	case VPABSQZrmb:
	case VPABSQZrmbk:
	case VPABSQZrmbkz:
	case VPABSQZrmk:
	case VPABSQZrmkz:
	case VPABSQZrr:
	case VPABSQZrrk:
	case VPABSQZrrkz:
		return true;
	}
	return false;
}

bool isVFMADDCSH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDCSHZm:
	case VFMADDCSHZmk:
	case VFMADDCSHZmkz:
	case VFMADDCSHZr:
	case VFMADDCSHZrb:
	case VFMADDCSHZrbk:
	case VFMADDCSHZrbkz:
	case VFMADDCSHZrk:
	case VFMADDCSHZrkz:
		return true;
	}
	return false;
}

bool isVPABSW(unsigned Opcode) {
	switch (Opcode) {
	case VPABSWYrm:
	case VPABSWYrr:
	case VPABSWZ128rm:
	case VPABSWZ128rmk:
	case VPABSWZ128rmkz:
	case VPABSWZ128rr:
	case VPABSWZ128rrk:
	case VPABSWZ128rrkz:
	case VPABSWZ256rm:
	case VPABSWZ256rmk:
	case VPABSWZ256rmkz:
	case VPABSWZ256rr:
	case VPABSWZ256rrk:
	case VPABSWZ256rrkz:
	case VPABSWZrm:
	case VPABSWZrmk:
	case VPABSWZrmkz:
	case VPABSWZrr:
	case VPABSWZrrk:
	case VPABSWZrrkz:
	case VPABSWrm:
	case VPABSWrr:
		return true;
	}
	return false;
}

bool isPVALIDATE(unsigned Opcode) {
	switch (Opcode) {
	case PVALIDATE32:
	case PVALIDATE64:
		return true;
	}
	return false;
}

bool isVFMADD231PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD231PDYm:
	case VFMADD231PDYr:
	case VFMADD231PDZ128m:
	case VFMADD231PDZ128mb:
	case VFMADD231PDZ128mbk:
	case VFMADD231PDZ128mbkz:
	case VFMADD231PDZ128mk:
	case VFMADD231PDZ128mkz:
	case VFMADD231PDZ128r:
	case VFMADD231PDZ128rk:
	case VFMADD231PDZ128rkz:
	case VFMADD231PDZ256m:
	case VFMADD231PDZ256mb:
	case VFMADD231PDZ256mbk:
	case VFMADD231PDZ256mbkz:
	case VFMADD231PDZ256mk:
	case VFMADD231PDZ256mkz:
	case VFMADD231PDZ256r:
	case VFMADD231PDZ256rk:
	case VFMADD231PDZ256rkz:
	case VFMADD231PDZm:
	case VFMADD231PDZmb:
	case VFMADD231PDZmbk:
	case VFMADD231PDZmbkz:
	case VFMADD231PDZmk:
	case VFMADD231PDZmkz:
	case VFMADD231PDZr:
	case VFMADD231PDZrb:
	case VFMADD231PDZrbk:
	case VFMADD231PDZrbkz:
	case VFMADD231PDZrk:
	case VFMADD231PDZrkz:
	case VFMADD231PDm:
	case VFMADD231PDr:
		return true;
	}
	return false;
}

bool isVFMADD231PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD231PHZ128m:
	case VFMADD231PHZ128mb:
	case VFMADD231PHZ128mbk:
	case VFMADD231PHZ128mbkz:
	case VFMADD231PHZ128mk:
	case VFMADD231PHZ128mkz:
	case VFMADD231PHZ128r:
	case VFMADD231PHZ128rk:
	case VFMADD231PHZ128rkz:
	case VFMADD231PHZ256m:
	case VFMADD231PHZ256mb:
	case VFMADD231PHZ256mbk:
	case VFMADD231PHZ256mbkz:
	case VFMADD231PHZ256mk:
	case VFMADD231PHZ256mkz:
	case VFMADD231PHZ256r:
	case VFMADD231PHZ256rk:
	case VFMADD231PHZ256rkz:
	case VFMADD231PHZm:
	case VFMADD231PHZmb:
	case VFMADD231PHZmbk:
	case VFMADD231PHZmbkz:
	case VFMADD231PHZmk:
	case VFMADD231PHZmkz:
	case VFMADD231PHZr:
	case VFMADD231PHZrb:
	case VFMADD231PHZrbk:
	case VFMADD231PHZrbkz:
	case VFMADD231PHZrk:
	case VFMADD231PHZrkz:
		return true;
	}
	return false;
}

bool isPUNPCKLQDQ(unsigned Opcode) {
	switch (Opcode) {
	case PUNPCKLQDQrm:
	case PUNPCKLQDQrr:
		return true;
	}
	return false;
}

bool isVCVTNEEPH2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTNEEPH2PSYrm:
	case VCVTNEEPH2PSrm:
		return true;
	}
	return false;
}

bool isVFMADD231PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD231PSYm:
	case VFMADD231PSYr:
	case VFMADD231PSZ128m:
	case VFMADD231PSZ128mb:
	case VFMADD231PSZ128mbk:
	case VFMADD231PSZ128mbkz:
	case VFMADD231PSZ128mk:
	case VFMADD231PSZ128mkz:
	case VFMADD231PSZ128r:
	case VFMADD231PSZ128rk:
	case VFMADD231PSZ128rkz:
	case VFMADD231PSZ256m:
	case VFMADD231PSZ256mb:
	case VFMADD231PSZ256mbk:
	case VFMADD231PSZ256mbkz:
	case VFMADD231PSZ256mk:
	case VFMADD231PSZ256mkz:
	case VFMADD231PSZ256r:
	case VFMADD231PSZ256rk:
	case VFMADD231PSZ256rkz:
	case VFMADD231PSZm:
	case VFMADD231PSZmb:
	case VFMADD231PSZmbk:
	case VFMADD231PSZmbkz:
	case VFMADD231PSZmk:
	case VFMADD231PSZmkz:
	case VFMADD231PSZr:
	case VFMADD231PSZrb:
	case VFMADD231PSZrbk:
	case VFMADD231PSZrbkz:
	case VFMADD231PSZrk:
	case VFMADD231PSZrkz:
	case VFMADD231PSm:
	case VFMADD231PSr:
		return true;
	}
	return false;
}

bool isAESENCWIDE256KL(unsigned Opcode) {
	return Opcode == AESENCWIDE256KL;
}

bool isVGF2P8MULB(unsigned Opcode) {
	switch (Opcode) {
	case VGF2P8MULBYrm:
	case VGF2P8MULBYrr:
	case VGF2P8MULBZ128rm:
	case VGF2P8MULBZ128rmk:
	case VGF2P8MULBZ128rmkz:
	case VGF2P8MULBZ128rr:
	case VGF2P8MULBZ128rrk:
	case VGF2P8MULBZ128rrkz:
	case VGF2P8MULBZ256rm:
	case VGF2P8MULBZ256rmk:
	case VGF2P8MULBZ256rmkz:
	case VGF2P8MULBZ256rr:
	case VGF2P8MULBZ256rrk:
	case VGF2P8MULBZ256rrkz:
	case VGF2P8MULBZrm:
	case VGF2P8MULBZrmk:
	case VGF2P8MULBZrmkz:
	case VGF2P8MULBZrr:
	case VGF2P8MULBZrrk:
	case VGF2P8MULBZrrkz:
	case VGF2P8MULBrm:
	case VGF2P8MULBrr:
		return true;
	}
	return false;
}

bool isWRSSD(unsigned Opcode) {
	return Opcode == WRSSD;
}

bool isMOVSHDUP(unsigned Opcode) {
	switch (Opcode) {
	case MOVSHDUPrm:
	case MOVSHDUPrr:
		return true;
	}
	return false;
}

bool isTZMSK(unsigned Opcode) {
	switch (Opcode) {
	case TZMSK32rm:
	case TZMSK32rr:
	case TZMSK64rm:
	case TZMSK64rr:
		return true;
	}
	return false;
}

bool isFPATAN(unsigned Opcode) {
	return Opcode == FPATAN;
}

bool isFUCOM(unsigned Opcode) {
	return Opcode == UCOM_Fr;
}

bool isVFMADD213SD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD213SDZm_Int:
	case VFMADD213SDZm_Intk:
	case VFMADD213SDZm_Intkz:
	case VFMADD213SDZr_Int:
	case VFMADD213SDZr_Intk:
	case VFMADD213SDZr_Intkz:
	case VFMADD213SDZrb_Int:
	case VFMADD213SDZrb_Intk:
	case VFMADD213SDZrb_Intkz:
	case VFMADD213SDm_Int:
	case VFMADD213SDr_Int:
		return true;
	}
	return false;
}

bool isWRSSQ(unsigned Opcode) {
	return Opcode == WRSSQ;
}

bool isVFMADD213SH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD213SHZm_Int:
	case VFMADD213SHZm_Intk:
	case VFMADD213SHZm_Intkz:
	case VFMADD213SHZr_Int:
	case VFMADD213SHZr_Intk:
	case VFMADD213SHZr_Intkz:
	case VFMADD213SHZrb_Int:
	case VFMADD213SHZrb_Intk:
	case VFMADD213SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVPTESTNMB(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTNMBZ128rm:
	case VPTESTNMBZ128rmk:
	case VPTESTNMBZ128rr:
	case VPTESTNMBZ128rrk:
	case VPTESTNMBZ256rm:
	case VPTESTNMBZ256rmk:
	case VPTESTNMBZ256rr:
	case VPTESTNMBZ256rrk:
	case VPTESTNMBZrm:
	case VPTESTNMBZrmk:
	case VPTESTNMBZrr:
	case VPTESTNMBZrrk:
		return true;
	}
	return false;
}

bool isVPTESTNMD(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTNMDZ128rm:
	case VPTESTNMDZ128rmb:
	case VPTESTNMDZ128rmbk:
	case VPTESTNMDZ128rmk:
	case VPTESTNMDZ128rr:
	case VPTESTNMDZ128rrk:
	case VPTESTNMDZ256rm:
	case VPTESTNMDZ256rmb:
	case VPTESTNMDZ256rmbk:
	case VPTESTNMDZ256rmk:
	case VPTESTNMDZ256rr:
	case VPTESTNMDZ256rrk:
	case VPTESTNMDZrm:
	case VPTESTNMDZrmb:
	case VPTESTNMDZrmbk:
	case VPTESTNMDZrmk:
	case VPTESTNMDZrr:
	case VPTESTNMDZrrk:
		return true;
	}
	return false;
}

bool isPMULUDQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMULUDQrm:
	case MMX_PMULUDQrr:
	case PMULUDQrm:
	case PMULUDQrr:
		return true;
	}
	return false;
}

bool isHRESET(unsigned Opcode) {
	return Opcode == HRESET;
}

bool isPMAXSB(unsigned Opcode) {
	switch (Opcode) {
	case PMAXSBrm:
	case PMAXSBrr:
		return true;
	}
	return false;
}

bool isPMAXSD(unsigned Opcode) {
	switch (Opcode) {
	case PMAXSDrm:
	case PMAXSDrr:
		return true;
	}
	return false;
}

bool isVFMADD213SS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD213SSZm_Int:
	case VFMADD213SSZm_Intk:
	case VFMADD213SSZm_Intkz:
	case VFMADD213SSZr_Int:
	case VFMADD213SSZr_Intk:
	case VFMADD213SSZr_Intkz:
	case VFMADD213SSZrb_Int:
	case VFMADD213SSZrb_Intk:
	case VFMADD213SSZrb_Intkz:
	case VFMADD213SSm_Int:
	case VFMADD213SSr_Int:
		return true;
	}
	return false;
}

bool isCLC(unsigned Opcode) {
	return Opcode == CLC;
}

bool isCLD(unsigned Opcode) {
	return Opcode == CLD;
}

bool isENCODEKEY256(unsigned Opcode) {
	return Opcode == ENCODEKEY256;
}

bool isVPTESTNMQ(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTNMQZ128rm:
	case VPTESTNMQZ128rmb:
	case VPTESTNMQZ128rmbk:
	case VPTESTNMQZ128rmk:
	case VPTESTNMQZ128rr:
	case VPTESTNMQZ128rrk:
	case VPTESTNMQZ256rm:
	case VPTESTNMQZ256rmb:
	case VPTESTNMQZ256rmbk:
	case VPTESTNMQZ256rmk:
	case VPTESTNMQZ256rr:
	case VPTESTNMQZ256rrk:
	case VPTESTNMQZrm:
	case VPTESTNMQZrmb:
	case VPTESTNMQZrmbk:
	case VPTESTNMQZrmk:
	case VPTESTNMQZrr:
	case VPTESTNMQZrrk:
		return true;
	}
	return false;
}

bool isVGATHERQPD(unsigned Opcode) {
	switch (Opcode) {
	case VGATHERQPDYrm:
	case VGATHERQPDZ128rm:
	case VGATHERQPDZ256rm:
	case VGATHERQPDZrm:
	case VGATHERQPDrm:
		return true;
	}
	return false;
}

bool isBOUND(unsigned Opcode) {
	switch (Opcode) {
	case BOUNDS16rm:
	case BOUNDS32rm:
		return true;
	}
	return false;
}

bool isCLI(unsigned Opcode) {
	return Opcode == CLI;
}

bool isVPTESTNMW(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTNMWZ128rm:
	case VPTESTNMWZ128rmk:
	case VPTESTNMWZ128rr:
	case VPTESTNMWZ128rrk:
	case VPTESTNMWZ256rm:
	case VPTESTNMWZ256rmk:
	case VPTESTNMWZ256rr:
	case VPTESTNMWZ256rrk:
	case VPTESTNMWZrm:
	case VPTESTNMWZrmk:
	case VPTESTNMWZrr:
	case VPTESTNMWZrrk:
		return true;
	}
	return false;
}

bool isPREFETCHIT0(unsigned Opcode) {
	return Opcode == PREFETCHIT0;
}

bool isPREFETCHIT1(unsigned Opcode) {
	return Opcode == PREFETCHIT1;
}

bool isPMAXSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMAXSWrm:
	case MMX_PMAXSWrr:
	case PMAXSWrm:
	case PMAXSWrr:
		return true;
	}
	return false;
}

bool isPFSUB(unsigned Opcode) {
	switch (Opcode) {
	case PFSUBrm:
	case PFSUBrr:
		return true;
	}
	return false;
}

bool isVCVTDQ2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTDQ2PHZ128rm:
	case VCVTDQ2PHZ128rmb:
	case VCVTDQ2PHZ128rmbk:
	case VCVTDQ2PHZ128rmbkz:
	case VCVTDQ2PHZ128rmk:
	case VCVTDQ2PHZ128rmkz:
	case VCVTDQ2PHZ128rr:
	case VCVTDQ2PHZ128rrk:
	case VCVTDQ2PHZ128rrkz:
	case VCVTDQ2PHZ256rm:
	case VCVTDQ2PHZ256rmb:
	case VCVTDQ2PHZ256rmbk:
	case VCVTDQ2PHZ256rmbkz:
	case VCVTDQ2PHZ256rmk:
	case VCVTDQ2PHZ256rmkz:
	case VCVTDQ2PHZ256rr:
	case VCVTDQ2PHZ256rrk:
	case VCVTDQ2PHZ256rrkz:
	case VCVTDQ2PHZrm:
	case VCVTDQ2PHZrmb:
	case VCVTDQ2PHZrmbk:
	case VCVTDQ2PHZrmbkz:
	case VCVTDQ2PHZrmk:
	case VCVTDQ2PHZrmkz:
	case VCVTDQ2PHZrr:
	case VCVTDQ2PHZrrb:
	case VCVTDQ2PHZrrbk:
	case VCVTDQ2PHZrrbkz:
	case VCVTDQ2PHZrrk:
	case VCVTDQ2PHZrrkz:
		return true;
	}
	return false;
}

bool isVPALIGNR(unsigned Opcode) {
	switch (Opcode) {
	case VPALIGNRYrmi:
	case VPALIGNRYrri:
	case VPALIGNRZ128rmi:
	case VPALIGNRZ128rmik:
	case VPALIGNRZ128rmikz:
	case VPALIGNRZ128rri:
	case VPALIGNRZ128rrik:
	case VPALIGNRZ128rrikz:
	case VPALIGNRZ256rmi:
	case VPALIGNRZ256rmik:
	case VPALIGNRZ256rmikz:
	case VPALIGNRZ256rri:
	case VPALIGNRZ256rrik:
	case VPALIGNRZ256rrikz:
	case VPALIGNRZrmi:
	case VPALIGNRZrmik:
	case VPALIGNRZrmikz:
	case VPALIGNRZrri:
	case VPALIGNRZrrik:
	case VPALIGNRZrrikz:
	case VPALIGNRrmi:
	case VPALIGNRrri:
		return true;
	}
	return false;
}

bool isVCVTDQ2PD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTDQ2PDYrm:
	case VCVTDQ2PDYrr:
	case VCVTDQ2PDZ128rm:
	case VCVTDQ2PDZ128rmb:
	case VCVTDQ2PDZ128rmbk:
	case VCVTDQ2PDZ128rmbkz:
	case VCVTDQ2PDZ128rmk:
	case VCVTDQ2PDZ128rmkz:
	case VCVTDQ2PDZ128rr:
	case VCVTDQ2PDZ128rrk:
	case VCVTDQ2PDZ128rrkz:
	case VCVTDQ2PDZ256rm:
	case VCVTDQ2PDZ256rmb:
	case VCVTDQ2PDZ256rmbk:
	case VCVTDQ2PDZ256rmbkz:
	case VCVTDQ2PDZ256rmk:
	case VCVTDQ2PDZ256rmkz:
	case VCVTDQ2PDZ256rr:
	case VCVTDQ2PDZ256rrk:
	case VCVTDQ2PDZ256rrkz:
	case VCVTDQ2PDZrm:
	case VCVTDQ2PDZrmb:
	case VCVTDQ2PDZrmbk:
	case VCVTDQ2PDZrmbkz:
	case VCVTDQ2PDZrmk:
	case VCVTDQ2PDZrmkz:
	case VCVTDQ2PDZrr:
	case VCVTDQ2PDZrrk:
	case VCVTDQ2PDZrrkz:
	case VCVTDQ2PDrm:
	case VCVTDQ2PDrr:
		return true;
	}
	return false;
}

bool isFNSTSW(unsigned Opcode) {
	switch (Opcode) {
	case FNSTSW16r:
	case FNSTSWm:
		return true;
	}
	return false;
}

bool isFISTP(unsigned Opcode) {
	switch (Opcode) {
	case IST_FP16m:
	case IST_FP32m:
	case IST_FP64m:
		return true;
	}
	return false;
}

bool isVFMADDPD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDPD4Ymr:
	case VFMADDPD4Yrm:
	case VFMADDPD4Yrr:
	case VFMADDPD4Yrr_REV:
	case VFMADDPD4mr:
	case VFMADDPD4rm:
	case VFMADDPD4rr:
	case VFMADDPD4rr_REV:
		return true;
	}
	return false;
}

bool isVGATHERQPS(unsigned Opcode) {
	switch (Opcode) {
	case VGATHERQPSYrm:
	case VGATHERQPSZ128rm:
	case VGATHERQPSZ256rm:
	case VGATHERQPSZrm:
	case VGATHERQPSrm:
		return true;
	}
	return false;
}

bool isPBLENDW(unsigned Opcode) {
	switch (Opcode) {
	case PBLENDWrmi:
	case PBLENDWrri:
		return true;
	}
	return false;
}

bool isVCVTDQ2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTDQ2PSYrm:
	case VCVTDQ2PSYrr:
	case VCVTDQ2PSZ128rm:
	case VCVTDQ2PSZ128rmb:
	case VCVTDQ2PSZ128rmbk:
	case VCVTDQ2PSZ128rmbkz:
	case VCVTDQ2PSZ128rmk:
	case VCVTDQ2PSZ128rmkz:
	case VCVTDQ2PSZ128rr:
	case VCVTDQ2PSZ128rrk:
	case VCVTDQ2PSZ128rrkz:
	case VCVTDQ2PSZ256rm:
	case VCVTDQ2PSZ256rmb:
	case VCVTDQ2PSZ256rmbk:
	case VCVTDQ2PSZ256rmbkz:
	case VCVTDQ2PSZ256rmk:
	case VCVTDQ2PSZ256rmkz:
	case VCVTDQ2PSZ256rr:
	case VCVTDQ2PSZ256rrk:
	case VCVTDQ2PSZ256rrkz:
	case VCVTDQ2PSZrm:
	case VCVTDQ2PSZrmb:
	case VCVTDQ2PSZrmbk:
	case VCVTDQ2PSZrmbkz:
	case VCVTDQ2PSZrmk:
	case VCVTDQ2PSZrmkz:
	case VCVTDQ2PSZrr:
	case VCVTDQ2PSZrrb:
	case VCVTDQ2PSZrrbk:
	case VCVTDQ2PSZrrbkz:
	case VCVTDQ2PSZrrk:
	case VCVTDQ2PSZrrkz:
	case VCVTDQ2PSrm:
	case VCVTDQ2PSrr:
		return true;
	}
	return false;
}

bool isPREFETCH(unsigned Opcode) {
	return Opcode == PREFETCH;
}

bool isSKINIT(unsigned Opcode) {
	return Opcode == SKINIT;
}

bool isCMC(unsigned Opcode) {
	return Opcode == CMC;
}

bool isVFMADDPS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDPS4Ymr:
	case VFMADDPS4Yrm:
	case VFMADDPS4Yrr:
	case VFMADDPS4Yrr_REV:
	case VFMADDPS4mr:
	case VFMADDPS4rm:
	case VFMADDPS4rr:
	case VFMADDPS4rr_REV:
		return true;
	}
	return false;
}

bool isVPACKSSDW(unsigned Opcode) {
	switch (Opcode) {
	case VPACKSSDWYrm:
	case VPACKSSDWYrr:
	case VPACKSSDWZ128rm:
	case VPACKSSDWZ128rmb:
	case VPACKSSDWZ128rmbk:
	case VPACKSSDWZ128rmbkz:
	case VPACKSSDWZ128rmk:
	case VPACKSSDWZ128rmkz:
	case VPACKSSDWZ128rr:
	case VPACKSSDWZ128rrk:
	case VPACKSSDWZ128rrkz:
	case VPACKSSDWZ256rm:
	case VPACKSSDWZ256rmb:
	case VPACKSSDWZ256rmbk:
	case VPACKSSDWZ256rmbkz:
	case VPACKSSDWZ256rmk:
	case VPACKSSDWZ256rmkz:
	case VPACKSSDWZ256rr:
	case VPACKSSDWZ256rrk:
	case VPACKSSDWZ256rrkz:
	case VPACKSSDWZrm:
	case VPACKSSDWZrmb:
	case VPACKSSDWZrmbk:
	case VPACKSSDWZrmbkz:
	case VPACKSSDWZrmk:
	case VPACKSSDWZrmkz:
	case VPACKSSDWZrr:
	case VPACKSSDWZrrk:
	case VPACKSSDWZrrkz:
	case VPACKSSDWrm:
	case VPACKSSDWrr:
		return true;
	}
	return false;
}

bool isFISUB(unsigned Opcode) {
	switch (Opcode) {
	case SUB_FI16m:
	case SUB_FI32m:
		return true;
	}
	return false;
}

bool isPADDB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDBrm:
	case MMX_PADDBrr:
	case PADDBrm:
	case PADDBrr:
		return true;
	}
	return false;
}

bool isPOPAL(unsigned Opcode) {
	return Opcode == POPA32;
}

bool isPADDD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDDrm:
	case MMX_PADDDrr:
	case PADDDrm:
	case PADDDrr:
		return true;
	}
	return false;
}

bool isVFMADD231SD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD231SDZm_Int:
	case VFMADD231SDZm_Intk:
	case VFMADD231SDZm_Intkz:
	case VFMADD231SDZr_Int:
	case VFMADD231SDZr_Intk:
	case VFMADD231SDZr_Intkz:
	case VFMADD231SDZrb_Int:
	case VFMADD231SDZrb_Intk:
	case VFMADD231SDZrb_Intkz:
	case VFMADD231SDm_Int:
	case VFMADD231SDr_Int:
		return true;
	}
	return false;
}

bool isCMP(unsigned Opcode) {
	switch (Opcode) {
	case CMP16i16:
	case CMP16mi:
	case CMP16mi8:
	case CMP16mr:
	case CMP16ri:
	case CMP16ri8:
	case CMP16rm:
	case CMP16rr:
	case CMP16rr_REV:
	case CMP32i32:
	case CMP32mi:
	case CMP32mi8:
	case CMP32mr:
	case CMP32ri:
	case CMP32ri8:
	case CMP32rm:
	case CMP32rr:
	case CMP32rr_REV:
	case CMP64i32:
	case CMP64mi32:
	case CMP64mi8:
	case CMP64mr:
	case CMP64ri32:
	case CMP64ri8:
	case CMP64rm:
	case CMP64rr:
	case CMP64rr_REV:
	case CMP8i8:
	case CMP8mi:
	case CMP8mi8:
	case CMP8mr:
	case CMP8ri:
	case CMP8ri8:
	case CMP8rm:
	case CMP8rr:
	case CMP8rr_REV:
		return true;
	}
	return false;
}

bool isKANDB(unsigned Opcode) {
	return Opcode == KANDBrr;
}

bool isKANDD(unsigned Opcode) {
	return Opcode == KANDDrr;
}

bool isVFMADD231SH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD231SHZm_Int:
	case VFMADD231SHZm_Intk:
	case VFMADD231SHZm_Intkz:
	case VFMADD231SHZr_Int:
	case VFMADD231SHZr_Intk:
	case VFMADD231SHZr_Intkz:
	case VFMADD231SHZrb_Int:
	case VFMADD231SHZrb_Intk:
	case VFMADD231SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVCVTSH2USI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSH2USI64Zrm_Int:
	case VCVTSH2USI64Zrr_Int:
	case VCVTSH2USI64Zrrb_Int:
	case VCVTSH2USIZrm_Int:
	case VCVTSH2USIZrr_Int:
	case VCVTSH2USIZrrb_Int:
		return true;
	}
	return false;
}

bool isPCONFIG(unsigned Opcode) {
	return Opcode == PCONFIG;
}

bool isPOPAW(unsigned Opcode) {
	return Opcode == POPA16;
}

bool isPADDQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDQrm:
	case MMX_PADDQrr:
	case PADDQrm:
	case PADDQrr:
		return true;
	}
	return false;
}

bool isPMAXUB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMAXUBrm:
	case MMX_PMAXUBrr:
	case PMAXUBrm:
	case PMAXUBrr:
		return true;
	}
	return false;
}

bool isROL(unsigned Opcode) {
	switch (Opcode) {
	case ROL16m1:
	case ROL16mCL:
	case ROL16mi:
	case ROL16r1:
	case ROL16rCL:
	case ROL16ri:
	case ROL32m1:
	case ROL32mCL:
	case ROL32mi:
	case ROL32r1:
	case ROL32rCL:
	case ROL32ri:
	case ROL64m1:
	case ROL64mCL:
	case ROL64mi:
	case ROL64r1:
	case ROL64rCL:
	case ROL64ri:
	case ROL8m1:
	case ROL8mCL:
	case ROL8mi:
	case ROL8r1:
	case ROL8rCL:
	case ROL8ri:
		return true;
	}
	return false;
}

bool isKANDQ(unsigned Opcode) {
	return Opcode == KANDQrr;
}

bool isPADDW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDWrm:
	case MMX_PADDWrr:
	case PADDWrm:
	case PADDWrr:
		return true;
	}
	return false;
}

bool isVCVTTPS2QQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPS2QQZ128rm:
	case VCVTTPS2QQZ128rmb:
	case VCVTTPS2QQZ128rmbk:
	case VCVTTPS2QQZ128rmbkz:
	case VCVTTPS2QQZ128rmk:
	case VCVTTPS2QQZ128rmkz:
	case VCVTTPS2QQZ128rr:
	case VCVTTPS2QQZ128rrk:
	case VCVTTPS2QQZ128rrkz:
	case VCVTTPS2QQZ256rm:
	case VCVTTPS2QQZ256rmb:
	case VCVTTPS2QQZ256rmbk:
	case VCVTTPS2QQZ256rmbkz:
	case VCVTTPS2QQZ256rmk:
	case VCVTTPS2QQZ256rmkz:
	case VCVTTPS2QQZ256rr:
	case VCVTTPS2QQZ256rrk:
	case VCVTTPS2QQZ256rrkz:
	case VCVTTPS2QQZrm:
	case VCVTTPS2QQZrmb:
	case VCVTTPS2QQZrmbk:
	case VCVTTPS2QQZrmbkz:
	case VCVTTPS2QQZrmk:
	case VCVTTPS2QQZrmkz:
	case VCVTTPS2QQZrr:
	case VCVTTPS2QQZrrb:
	case VCVTTPS2QQZrrbk:
	case VCVTTPS2QQZrrbkz:
	case VCVTTPS2QQZrrk:
	case VCVTTPS2QQZrrkz:
		return true;
	}
	return false;
}

bool isPMAXUD(unsigned Opcode) {
	switch (Opcode) {
	case PMAXUDrm:
	case PMAXUDrr:
		return true;
	}
	return false;
}

bool isROR(unsigned Opcode) {
	switch (Opcode) {
	case ROR16m1:
	case ROR16mCL:
	case ROR16mi:
	case ROR16r1:
	case ROR16rCL:
	case ROR16ri:
	case ROR32m1:
	case ROR32mCL:
	case ROR32mi:
	case ROR32r1:
	case ROR32rCL:
	case ROR32ri:
	case ROR64m1:
	case ROR64mCL:
	case ROR64mi:
	case ROR64r1:
	case ROR64rCL:
	case ROR64ri:
	case ROR8m1:
	case ROR8mCL:
	case ROR8mi:
	case ROR8r1:
	case ROR8rCL:
	case ROR8ri:
		return true;
	}
	return false;
}

bool isVFMADD231SS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADD231SSZm_Int:
	case VFMADD231SSZm_Intk:
	case VFMADD231SSZm_Intkz:
	case VFMADD231SSZr_Int:
	case VFMADD231SSZr_Intk:
	case VFMADD231SSZr_Intkz:
	case VFMADD231SSZrb_Int:
	case VFMADD231SSZrb_Intk:
	case VFMADD231SSZrb_Intkz:
	case VFMADD231SSm_Int:
	case VFMADD231SSr_Int:
		return true;
	}
	return false;
}

bool isKANDW(unsigned Opcode) {
	return Opcode == KANDWrr;
}

bool isVPSRAD(unsigned Opcode) {
	switch (Opcode) {
	case VPSRADYri:
	case VPSRADYrm:
	case VPSRADYrr:
	case VPSRADZ128mbi:
	case VPSRADZ128mbik:
	case VPSRADZ128mbikz:
	case VPSRADZ128mi:
	case VPSRADZ128mik:
	case VPSRADZ128mikz:
	case VPSRADZ128ri:
	case VPSRADZ128rik:
	case VPSRADZ128rikz:
	case VPSRADZ128rm:
	case VPSRADZ128rmk:
	case VPSRADZ128rmkz:
	case VPSRADZ128rr:
	case VPSRADZ128rrk:
	case VPSRADZ128rrkz:
	case VPSRADZ256mbi:
	case VPSRADZ256mbik:
	case VPSRADZ256mbikz:
	case VPSRADZ256mi:
	case VPSRADZ256mik:
	case VPSRADZ256mikz:
	case VPSRADZ256ri:
	case VPSRADZ256rik:
	case VPSRADZ256rikz:
	case VPSRADZ256rm:
	case VPSRADZ256rmk:
	case VPSRADZ256rmkz:
	case VPSRADZ256rr:
	case VPSRADZ256rrk:
	case VPSRADZ256rrkz:
	case VPSRADZmbi:
	case VPSRADZmbik:
	case VPSRADZmbikz:
	case VPSRADZmi:
	case VPSRADZmik:
	case VPSRADZmikz:
	case VPSRADZri:
	case VPSRADZrik:
	case VPSRADZrikz:
	case VPSRADZrm:
	case VPSRADZrmk:
	case VPSRADZrmkz:
	case VPSRADZrr:
	case VPSRADZrrk:
	case VPSRADZrrkz:
	case VPSRADri:
	case VPSRADrm:
	case VPSRADrr:
		return true;
	}
	return false;
}

bool isVROUNDPD(unsigned Opcode) {
	switch (Opcode) {
	case VROUNDPDYm:
	case VROUNDPDYr:
	case VROUNDPDm:
	case VROUNDPDr:
		return true;
	}
	return false;
}

bool isMOVBE(unsigned Opcode) {
	switch (Opcode) {
	case MOVBE16mr:
	case MOVBE16rm:
	case MOVBE32mr:
	case MOVBE32rm:
	case MOVBE64mr:
	case MOVBE64rm:
		return true;
	}
	return false;
}

bool isLOOP(unsigned Opcode) {
	return Opcode == LOOP;
}

bool isVCVTTSD2SI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTSD2SI64Zrm_Int:
	case VCVTTSD2SI64Zrr_Int:
	case VCVTTSD2SI64Zrrb_Int:
	case VCVTTSD2SI64rm_Int:
	case VCVTTSD2SI64rr_Int:
	case VCVTTSD2SIZrm_Int:
	case VCVTTSD2SIZrr_Int:
	case VCVTTSD2SIZrrb_Int:
	case VCVTTSD2SIrm_Int:
	case VCVTTSD2SIrr_Int:
		return true;
	}
	return false;
}

bool isVPSRAQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSRAQZ128mbi:
	case VPSRAQZ128mbik:
	case VPSRAQZ128mbikz:
	case VPSRAQZ128mi:
	case VPSRAQZ128mik:
	case VPSRAQZ128mikz:
	case VPSRAQZ128ri:
	case VPSRAQZ128rik:
	case VPSRAQZ128rikz:
	case VPSRAQZ128rm:
	case VPSRAQZ128rmk:
	case VPSRAQZ128rmkz:
	case VPSRAQZ128rr:
	case VPSRAQZ128rrk:
	case VPSRAQZ128rrkz:
	case VPSRAQZ256mbi:
	case VPSRAQZ256mbik:
	case VPSRAQZ256mbikz:
	case VPSRAQZ256mi:
	case VPSRAQZ256mik:
	case VPSRAQZ256mikz:
	case VPSRAQZ256ri:
	case VPSRAQZ256rik:
	case VPSRAQZ256rikz:
	case VPSRAQZ256rm:
	case VPSRAQZ256rmk:
	case VPSRAQZ256rmkz:
	case VPSRAQZ256rr:
	case VPSRAQZ256rrk:
	case VPSRAQZ256rrkz:
	case VPSRAQZmbi:
	case VPSRAQZmbik:
	case VPSRAQZmbikz:
	case VPSRAQZmi:
	case VPSRAQZmik:
	case VPSRAQZmikz:
	case VPSRAQZri:
	case VPSRAQZrik:
	case VPSRAQZrikz:
	case VPSRAQZrm:
	case VPSRAQZrmk:
	case VPSRAQZrmkz:
	case VPSRAQZrr:
	case VPSRAQZrrk:
	case VPSRAQZrrkz:
		return true;
	}
	return false;
}

bool isVROUNDPS(unsigned Opcode) {
	switch (Opcode) {
	case VROUNDPSYm:
	case VROUNDPSYr:
	case VROUNDPSm:
	case VROUNDPSr:
		return true;
	}
	return false;
}

bool isPMAXUW(unsigned Opcode) {
	switch (Opcode) {
	case PMAXUWrm:
	case PMAXUWrr:
		return true;
	}
	return false;
}

bool isVPSRAW(unsigned Opcode) {
	switch (Opcode) {
	case VPSRAWYri:
	case VPSRAWYrm:
	case VPSRAWYrr:
	case VPSRAWZ128mi:
	case VPSRAWZ128mik:
	case VPSRAWZ128mikz:
	case VPSRAWZ128ri:
	case VPSRAWZ128rik:
	case VPSRAWZ128rikz:
	case VPSRAWZ128rm:
	case VPSRAWZ128rmk:
	case VPSRAWZ128rmkz:
	case VPSRAWZ128rr:
	case VPSRAWZ128rrk:
	case VPSRAWZ128rrkz:
	case VPSRAWZ256mi:
	case VPSRAWZ256mik:
	case VPSRAWZ256mikz:
	case VPSRAWZ256ri:
	case VPSRAWZ256rik:
	case VPSRAWZ256rikz:
	case VPSRAWZ256rm:
	case VPSRAWZ256rmk:
	case VPSRAWZ256rmkz:
	case VPSRAWZ256rr:
	case VPSRAWZ256rrk:
	case VPSRAWZ256rrkz:
	case VPSRAWZmi:
	case VPSRAWZmik:
	case VPSRAWZmikz:
	case VPSRAWZri:
	case VPSRAWZrik:
	case VPSRAWZrikz:
	case VPSRAWZrm:
	case VPSRAWZrmk:
	case VPSRAWZrmkz:
	case VPSRAWZrr:
	case VPSRAWZrrk:
	case VPSRAWZrrkz:
	case VPSRAWri:
	case VPSRAWrm:
	case VPSRAWrr:
		return true;
	}
	return false;
}

bool isVPOR(unsigned Opcode) {
	switch (Opcode) {
	case VPORYrm:
	case VPORYrr:
	case VPORrm:
	case VPORrr:
		return true;
	}
	return false;
}

bool isVPEXTRB(unsigned Opcode) {
	switch (Opcode) {
	case VPEXTRBZmr:
	case VPEXTRBZrr:
	case VPEXTRBmr:
	case VPEXTRBrr:
		return true;
	}
	return false;
}

bool isVPEXTRD(unsigned Opcode) {
	switch (Opcode) {
	case VPEXTRDZmr:
	case VPEXTRDZrr:
	case VPEXTRDmr:
	case VPEXTRDrr:
		return true;
	}
	return false;
}

bool isXGETBV(unsigned Opcode) {
	return Opcode == XGETBV;
}

bool isVSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case VSUBPDYrm:
	case VSUBPDYrr:
	case VSUBPDZ128rm:
	case VSUBPDZ128rmb:
	case VSUBPDZ128rmbk:
	case VSUBPDZ128rmbkz:
	case VSUBPDZ128rmk:
	case VSUBPDZ128rmkz:
	case VSUBPDZ128rr:
	case VSUBPDZ128rrk:
	case VSUBPDZ128rrkz:
	case VSUBPDZ256rm:
	case VSUBPDZ256rmb:
	case VSUBPDZ256rmbk:
	case VSUBPDZ256rmbkz:
	case VSUBPDZ256rmk:
	case VSUBPDZ256rmkz:
	case VSUBPDZ256rr:
	case VSUBPDZ256rrk:
	case VSUBPDZ256rrkz:
	case VSUBPDZrm:
	case VSUBPDZrmb:
	case VSUBPDZrmbk:
	case VSUBPDZrmbkz:
	case VSUBPDZrmk:
	case VSUBPDZrmkz:
	case VSUBPDZrr:
	case VSUBPDZrrb:
	case VSUBPDZrrbk:
	case VSUBPDZrrbkz:
	case VSUBPDZrrk:
	case VSUBPDZrrkz:
	case VSUBPDrm:
	case VSUBPDrr:
		return true;
	}
	return false;
}

bool isENCLS(unsigned Opcode) {
	return Opcode == ENCLS;
}

bool isENCLU(unsigned Opcode) {
	return Opcode == ENCLU;
}

bool isENCLV(unsigned Opcode) {
	return Opcode == ENCLV;
}

bool isVSUBPH(unsigned Opcode) {
	switch (Opcode) {
	case VSUBPHZ128rm:
	case VSUBPHZ128rmb:
	case VSUBPHZ128rmbk:
	case VSUBPHZ128rmbkz:
	case VSUBPHZ128rmk:
	case VSUBPHZ128rmkz:
	case VSUBPHZ128rr:
	case VSUBPHZ128rrk:
	case VSUBPHZ128rrkz:
	case VSUBPHZ256rm:
	case VSUBPHZ256rmb:
	case VSUBPHZ256rmbk:
	case VSUBPHZ256rmbkz:
	case VSUBPHZ256rmk:
	case VSUBPHZ256rmkz:
	case VSUBPHZ256rr:
	case VSUBPHZ256rrk:
	case VSUBPHZ256rrkz:
	case VSUBPHZrm:
	case VSUBPHZrmb:
	case VSUBPHZrmbk:
	case VSUBPHZrmbkz:
	case VSUBPHZrmk:
	case VSUBPHZrmkz:
	case VSUBPHZrr:
	case VSUBPHZrrb:
	case VSUBPHZrrbk:
	case VSUBPHZrrbkz:
	case VSUBPHZrrk:
	case VSUBPHZrrkz:
		return true;
	}
	return false;
}

bool isVPEXTRQ(unsigned Opcode) {
	switch (Opcode) {
	case VPEXTRQZmr:
	case VPEXTRQZrr:
	case VPEXTRQmr:
	case VPEXTRQrr:
		return true;
	}
	return false;
}

bool isVHSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case VHSUBPDYrm:
	case VHSUBPDYrr:
	case VHSUBPDrm:
	case VHSUBPDrr:
		return true;
	}
	return false;
}

bool isVPEXTRW(unsigned Opcode) {
	switch (Opcode) {
	case VPEXTRWZmr:
	case VPEXTRWZrr:
	case VPEXTRWZrr_REV:
	case VPEXTRWmr:
	case VPEXTRWrr:
	case VPEXTRWrr_REV:
		return true;
	}
	return false;
}

bool isVSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case VSUBPSYrm:
	case VSUBPSYrr:
	case VSUBPSZ128rm:
	case VSUBPSZ128rmb:
	case VSUBPSZ128rmbk:
	case VSUBPSZ128rmbkz:
	case VSUBPSZ128rmk:
	case VSUBPSZ128rmkz:
	case VSUBPSZ128rr:
	case VSUBPSZ128rrk:
	case VSUBPSZ128rrkz:
	case VSUBPSZ256rm:
	case VSUBPSZ256rmb:
	case VSUBPSZ256rmbk:
	case VSUBPSZ256rmbkz:
	case VSUBPSZ256rmk:
	case VSUBPSZ256rmkz:
	case VSUBPSZ256rr:
	case VSUBPSZ256rrk:
	case VSUBPSZ256rrkz:
	case VSUBPSZrm:
	case VSUBPSZrmb:
	case VSUBPSZrmbk:
	case VSUBPSZrmbkz:
	case VSUBPSZrmk:
	case VSUBPSZrmkz:
	case VSUBPSZrr:
	case VSUBPSZrrb:
	case VSUBPSZrrbk:
	case VSUBPSZrrbkz:
	case VSUBPSZrrk:
	case VSUBPSZrrkz:
	case VSUBPSrm:
	case VSUBPSrr:
		return true;
	}
	return false;
}

bool isVGF2P8AFFINEINVQB(unsigned Opcode) {
	switch (Opcode) {
	case VGF2P8AFFINEINVQBYrmi:
	case VGF2P8AFFINEINVQBYrri:
	case VGF2P8AFFINEINVQBZ128rmbi:
	case VGF2P8AFFINEINVQBZ128rmbik:
	case VGF2P8AFFINEINVQBZ128rmbikz:
	case VGF2P8AFFINEINVQBZ128rmi:
	case VGF2P8AFFINEINVQBZ128rmik:
	case VGF2P8AFFINEINVQBZ128rmikz:
	case VGF2P8AFFINEINVQBZ128rri:
	case VGF2P8AFFINEINVQBZ128rrik:
	case VGF2P8AFFINEINVQBZ128rrikz:
	case VGF2P8AFFINEINVQBZ256rmbi:
	case VGF2P8AFFINEINVQBZ256rmbik:
	case VGF2P8AFFINEINVQBZ256rmbikz:
	case VGF2P8AFFINEINVQBZ256rmi:
	case VGF2P8AFFINEINVQBZ256rmik:
	case VGF2P8AFFINEINVQBZ256rmikz:
	case VGF2P8AFFINEINVQBZ256rri:
	case VGF2P8AFFINEINVQBZ256rrik:
	case VGF2P8AFFINEINVQBZ256rrikz:
	case VGF2P8AFFINEINVQBZrmbi:
	case VGF2P8AFFINEINVQBZrmbik:
	case VGF2P8AFFINEINVQBZrmbikz:
	case VGF2P8AFFINEINVQBZrmi:
	case VGF2P8AFFINEINVQBZrmik:
	case VGF2P8AFFINEINVQBZrmikz:
	case VGF2P8AFFINEINVQBZrri:
	case VGF2P8AFFINEINVQBZrrik:
	case VGF2P8AFFINEINVQBZrrikz:
	case VGF2P8AFFINEINVQBrmi:
	case VGF2P8AFFINEINVQBrri:
		return true;
	}
	return false;
}

bool isVMOVLPD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVLPDZ128mr:
	case VMOVLPDZ128rm:
	case VMOVLPDmr:
	case VMOVLPDrm:
		return true;
	}
	return false;
}

bool isVFMADDSD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSD4mr:
	case VFMADDSD4rm:
	case VFMADDSD4rr:
	case VFMADDSD4rr_REV:
		return true;
	}
	return false;
}

bool isVHSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case VHSUBPSYrm:
	case VHSUBPSYrr:
	case VHSUBPSrm:
	case VHSUBPSrr:
		return true;
	}
	return false;
}

bool isPSRLDQ(unsigned Opcode) {
	return Opcode == PSRLDQri;
}

bool isVMOVLPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVLPSZ128mr:
	case VMOVLPSZ128rm:
	case VMOVLPSmr:
	case VMOVLPSrm:
		return true;
	}
	return false;
}

bool isVFMADDSS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSS4mr:
	case VFMADDSS4rm:
	case VFMADDSS4rr:
	case VFMADDSS4rr_REV:
		return true;
	}
	return false;
}

bool isVRCP28PD(unsigned Opcode) {
	switch (Opcode) {
	case VRCP28PDZm:
	case VRCP28PDZmb:
	case VRCP28PDZmbk:
	case VRCP28PDZmbkz:
	case VRCP28PDZmk:
	case VRCP28PDZmkz:
	case VRCP28PDZr:
	case VRCP28PDZrb:
	case VRCP28PDZrbk:
	case VRCP28PDZrbkz:
	case VRCP28PDZrk:
	case VRCP28PDZrkz:
		return true;
	}
	return false;
}

bool isFPREM(unsigned Opcode) {
	return Opcode == FPREM;
}

bool isVPMADDWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMADDWDYrm:
	case VPMADDWDYrr:
	case VPMADDWDZ128rm:
	case VPMADDWDZ128rmk:
	case VPMADDWDZ128rmkz:
	case VPMADDWDZ128rr:
	case VPMADDWDZ128rrk:
	case VPMADDWDZ128rrkz:
	case VPMADDWDZ256rm:
	case VPMADDWDZ256rmk:
	case VPMADDWDZ256rmkz:
	case VPMADDWDZ256rr:
	case VPMADDWDZ256rrk:
	case VPMADDWDZ256rrkz:
	case VPMADDWDZrm:
	case VPMADDWDZrmk:
	case VPMADDWDZrmkz:
	case VPMADDWDZrr:
	case VPMADDWDZrrk:
	case VPMADDWDZrrkz:
	case VPMADDWDrm:
	case VPMADDWDrr:
		return true;
	}
	return false;
}

bool isVCVTSH2SD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSH2SDZrm_Int:
	case VCVTSH2SDZrm_Intk:
	case VCVTSH2SDZrm_Intkz:
	case VCVTSH2SDZrr_Int:
	case VCVTSH2SDZrr_Intk:
	case VCVTSH2SDZrr_Intkz:
	case VCVTSH2SDZrrb_Int:
	case VCVTSH2SDZrrb_Intk:
	case VCVTSH2SDZrrb_Intkz:
		return true;
	}
	return false;
}

bool isSERIALIZE(unsigned Opcode) {
	return Opcode == SERIALIZE;
}

bool isV4FMADDPS(unsigned Opcode) {
	switch (Opcode) {
	case V4FMADDPSrm:
	case V4FMADDPSrmk:
	case V4FMADDPSrmkz:
		return true;
	}
	return false;
}

bool isVRCP28PS(unsigned Opcode) {
	switch (Opcode) {
	case VRCP28PSZm:
	case VRCP28PSZmb:
	case VRCP28PSZmbk:
	case VRCP28PSZmbkz:
	case VRCP28PSZmk:
	case VRCP28PSZmkz:
	case VRCP28PSZr:
	case VRCP28PSZrb:
	case VRCP28PSZrbk:
	case VRCP28PSZrbkz:
	case VRCP28PSZrk:
	case VRCP28PSZrkz:
		return true;
	}
	return false;
}

bool isVCVTSH2SI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSH2SI64Zrm_Int:
	case VCVTSH2SI64Zrr_Int:
	case VCVTSH2SI64Zrrb_Int:
	case VCVTSH2SIZrm_Int:
	case VCVTSH2SIZrr_Int:
	case VCVTSH2SIZrrb_Int:
		return true;
	}
	return false;
}

bool isRETF(unsigned Opcode) {
	switch (Opcode) {
	case LRET16:
	case LRET32:
	case LRETI16:
	case LRETI32:
		return true;
	}
	return false;
}

bool isVROUNDSD(unsigned Opcode) {
	switch (Opcode) {
	case VROUNDSDm_Int:
	case VROUNDSDr_Int:
		return true;
	}
	return false;
}

bool isVCVTSH2SS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSH2SSZrm_Int:
	case VCVTSH2SSZrm_Intk:
	case VCVTSH2SSZrm_Intkz:
	case VCVTSH2SSZrr_Int:
	case VCVTSH2SSZrr_Intk:
	case VCVTSH2SSZrr_Intkz:
	case VCVTSH2SSZrrb_Int:
	case VCVTSH2SSZrrb_Intk:
	case VCVTSH2SSZrrb_Intkz:
		return true;
	}
	return false;
}

bool isVSCATTERPF1QPD(unsigned Opcode) {
	return Opcode == VSCATTERPF1QPDm;
}

bool isVPCONFLICTD(unsigned Opcode) {
	switch (Opcode) {
	case VPCONFLICTDZ128rm:
	case VPCONFLICTDZ128rmb:
	case VPCONFLICTDZ128rmbk:
	case VPCONFLICTDZ128rmbkz:
	case VPCONFLICTDZ128rmk:
	case VPCONFLICTDZ128rmkz:
	case VPCONFLICTDZ128rr:
	case VPCONFLICTDZ128rrk:
	case VPCONFLICTDZ128rrkz:
	case VPCONFLICTDZ256rm:
	case VPCONFLICTDZ256rmb:
	case VPCONFLICTDZ256rmbk:
	case VPCONFLICTDZ256rmbkz:
	case VPCONFLICTDZ256rmk:
	case VPCONFLICTDZ256rmkz:
	case VPCONFLICTDZ256rr:
	case VPCONFLICTDZ256rrk:
	case VPCONFLICTDZ256rrkz:
	case VPCONFLICTDZrm:
	case VPCONFLICTDZrmb:
	case VPCONFLICTDZrmbk:
	case VPCONFLICTDZrmbkz:
	case VPCONFLICTDZrmk:
	case VPCONFLICTDZrmkz:
	case VPCONFLICTDZrr:
	case VPCONFLICTDZrrk:
	case VPCONFLICTDZrrkz:
		return true;
	}
	return false;
}

bool isMOVNTI(unsigned Opcode) {
	switch (Opcode) {
	case MOVNTI_64mr:
	case MOVNTImr:
		return true;
	}
	return false;
}

bool isCQO(unsigned Opcode) {
	return Opcode == CQO;
}

bool isVROUNDSS(unsigned Opcode) {
	switch (Opcode) {
	case VROUNDSSm_Int:
	case VROUNDSSr_Int:
		return true;
	}
	return false;
}

bool isMOVNTQ(unsigned Opcode) {
	return Opcode == MMX_MOVNTQmr;
}

bool isVAESENC(unsigned Opcode) {
	switch (Opcode) {
	case VAESENCYrm:
	case VAESENCYrr:
	case VAESENCZ128rm:
	case VAESENCZ128rr:
	case VAESENCZ256rm:
	case VAESENCZ256rr:
	case VAESENCZrm:
	case VAESENCZrr:
	case VAESENCrm:
	case VAESENCrr:
		return true;
	}
	return false;
}

bool isVSCATTERPF1QPS(unsigned Opcode) {
	return Opcode == VSCATTERPF1QPSm;
}

bool isVPCONFLICTQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCONFLICTQZ128rm:
	case VPCONFLICTQZ128rmb:
	case VPCONFLICTQZ128rmbk:
	case VPCONFLICTQZ128rmbkz:
	case VPCONFLICTQZ128rmk:
	case VPCONFLICTQZ128rmkz:
	case VPCONFLICTQZ128rr:
	case VPCONFLICTQZ128rrk:
	case VPCONFLICTQZ128rrkz:
	case VPCONFLICTQZ256rm:
	case VPCONFLICTQZ256rmb:
	case VPCONFLICTQZ256rmbk:
	case VPCONFLICTQZ256rmbkz:
	case VPCONFLICTQZ256rmk:
	case VPCONFLICTQZ256rmkz:
	case VPCONFLICTQZ256rr:
	case VPCONFLICTQZ256rrk:
	case VPCONFLICTQZ256rrkz:
	case VPCONFLICTQZrm:
	case VPCONFLICTQZrmb:
	case VPCONFLICTQZrmbk:
	case VPCONFLICTQZrmbkz:
	case VPCONFLICTQZrmk:
	case VPCONFLICTQZrmkz:
	case VPCONFLICTQZrr:
	case VPCONFLICTQZrrk:
	case VPCONFLICTQZrrkz:
		return true;
	}
	return false;
}

bool isFCMOVNB(unsigned Opcode) {
	return Opcode == CMOVNB_F;
}

bool isLZCNT(unsigned Opcode) {
	switch (Opcode) {
	case LZCNT16rm:
	case LZCNT16rr:
	case LZCNT32rm:
	case LZCNT32rr:
	case LZCNT64rm:
	case LZCNT64rr:
		return true;
	}
	return false;
}

bool isFCMOVNE(unsigned Opcode) {
	return Opcode == CMOVNE_F;
}

bool isRSM(unsigned Opcode) {
	return Opcode == RSM;
}

bool isPOPCNT(unsigned Opcode) {
	switch (Opcode) {
	case POPCNT16rm:
	case POPCNT16rr:
	case POPCNT32rm:
	case POPCNT32rr:
	case POPCNT64rm:
	case POPCNT64rr:
		return true;
	}
	return false;
}

bool isVSUBSD(unsigned Opcode) {
	switch (Opcode) {
	case VSUBSDZrm_Int:
	case VSUBSDZrm_Intk:
	case VSUBSDZrm_Intkz:
	case VSUBSDZrr_Int:
	case VSUBSDZrr_Intk:
	case VSUBSDZrr_Intkz:
	case VSUBSDZrrb_Int:
	case VSUBSDZrrb_Intk:
	case VSUBSDZrrb_Intkz:
	case VSUBSDrm_Int:
	case VSUBSDrr_Int:
		return true;
	}
	return false;
}

bool isPOPFD(unsigned Opcode) {
	return Opcode == POPF32;
}

bool isVSUBSH(unsigned Opcode) {
	switch (Opcode) {
	case VSUBSHZrm_Int:
	case VSUBSHZrm_Intk:
	case VSUBSHZrm_Intkz:
	case VSUBSHZrr_Int:
	case VSUBSHZrr_Intk:
	case VSUBSHZrr_Intkz:
	case VSUBSHZrrb_Int:
	case VSUBSHZrrb_Intk:
	case VSUBSHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isVPANDD(unsigned Opcode) {
	switch (Opcode) {
	case VPANDDZ128rm:
	case VPANDDZ128rmb:
	case VPANDDZ128rmbk:
	case VPANDDZ128rmbkz:
	case VPANDDZ128rmk:
	case VPANDDZ128rmkz:
	case VPANDDZ128rr:
	case VPANDDZ128rrk:
	case VPANDDZ128rrkz:
	case VPANDDZ256rm:
	case VPANDDZ256rmb:
	case VPANDDZ256rmbk:
	case VPANDDZ256rmbkz:
	case VPANDDZ256rmk:
	case VPANDDZ256rmkz:
	case VPANDDZ256rr:
	case VPANDDZ256rrk:
	case VPANDDZ256rrkz:
	case VPANDDZrm:
	case VPANDDZrmb:
	case VPANDDZrmbk:
	case VPANDDZrmbkz:
	case VPANDDZrmk:
	case VPANDDZrmkz:
	case VPANDDZrr:
	case VPANDDZrrk:
	case VPANDDZrrkz:
		return true;
	}
	return false;
}

bool isFCMOVNU(unsigned Opcode) {
	return Opcode == CMOVNP_F;
}

bool isVMOVHLPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVHLPSZrr:
	case VMOVHLPSrr:
		return true;
	}
	return false;
}

bool isPOPFQ(unsigned Opcode) {
	return Opcode == POPF64;
}

bool isVPANDN(unsigned Opcode) {
	switch (Opcode) {
	case VPANDNYrm:
	case VPANDNYrr:
	case VPANDNrm:
	case VPANDNrr:
		return true;
	}
	return false;
}

bool isVFMADDSUB213PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB213PDYm:
	case VFMADDSUB213PDYr:
	case VFMADDSUB213PDZ128m:
	case VFMADDSUB213PDZ128mb:
	case VFMADDSUB213PDZ128mbk:
	case VFMADDSUB213PDZ128mbkz:
	case VFMADDSUB213PDZ128mk:
	case VFMADDSUB213PDZ128mkz:
	case VFMADDSUB213PDZ128r:
	case VFMADDSUB213PDZ128rk:
	case VFMADDSUB213PDZ128rkz:
	case VFMADDSUB213PDZ256m:
	case VFMADDSUB213PDZ256mb:
	case VFMADDSUB213PDZ256mbk:
	case VFMADDSUB213PDZ256mbkz:
	case VFMADDSUB213PDZ256mk:
	case VFMADDSUB213PDZ256mkz:
	case VFMADDSUB213PDZ256r:
	case VFMADDSUB213PDZ256rk:
	case VFMADDSUB213PDZ256rkz:
	case VFMADDSUB213PDZm:
	case VFMADDSUB213PDZmb:
	case VFMADDSUB213PDZmbk:
	case VFMADDSUB213PDZmbkz:
	case VFMADDSUB213PDZmk:
	case VFMADDSUB213PDZmkz:
	case VFMADDSUB213PDZr:
	case VFMADDSUB213PDZrb:
	case VFMADDSUB213PDZrbk:
	case VFMADDSUB213PDZrbkz:
	case VFMADDSUB213PDZrk:
	case VFMADDSUB213PDZrkz:
	case VFMADDSUB213PDm:
	case VFMADDSUB213PDr:
		return true;
	}
	return false;
}

bool isVCMPPD(unsigned Opcode) {
	switch (Opcode) {
	case VCMPPDYrmi:
	case VCMPPDYrri:
	case VCMPPDZ128rmbi:
	case VCMPPDZ128rmbik:
	case VCMPPDZ128rmi:
	case VCMPPDZ128rmik:
	case VCMPPDZ128rri:
	case VCMPPDZ128rrik:
	case VCMPPDZ256rmbi:
	case VCMPPDZ256rmbik:
	case VCMPPDZ256rmi:
	case VCMPPDZ256rmik:
	case VCMPPDZ256rri:
	case VCMPPDZ256rrik:
	case VCMPPDZrmbi:
	case VCMPPDZrmbik:
	case VCMPPDZrmi:
	case VCMPPDZrmik:
	case VCMPPDZrri:
	case VCMPPDZrrib:
	case VCMPPDZrribk:
	case VCMPPDZrrik:
	case VCMPPDrmi:
	case VCMPPDrri:
		return true;
	}
	return false;
}

bool isVFMADDSUB213PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB213PHZ128m:
	case VFMADDSUB213PHZ128mb:
	case VFMADDSUB213PHZ128mbk:
	case VFMADDSUB213PHZ128mbkz:
	case VFMADDSUB213PHZ128mk:
	case VFMADDSUB213PHZ128mkz:
	case VFMADDSUB213PHZ128r:
	case VFMADDSUB213PHZ128rk:
	case VFMADDSUB213PHZ128rkz:
	case VFMADDSUB213PHZ256m:
	case VFMADDSUB213PHZ256mb:
	case VFMADDSUB213PHZ256mbk:
	case VFMADDSUB213PHZ256mbkz:
	case VFMADDSUB213PHZ256mk:
	case VFMADDSUB213PHZ256mkz:
	case VFMADDSUB213PHZ256r:
	case VFMADDSUB213PHZ256rk:
	case VFMADDSUB213PHZ256rkz:
	case VFMADDSUB213PHZm:
	case VFMADDSUB213PHZmb:
	case VFMADDSUB213PHZmbk:
	case VFMADDSUB213PHZmbkz:
	case VFMADDSUB213PHZmk:
	case VFMADDSUB213PHZmkz:
	case VFMADDSUB213PHZr:
	case VFMADDSUB213PHZrb:
	case VFMADDSUB213PHZrbk:
	case VFMADDSUB213PHZrbkz:
	case VFMADDSUB213PHZrk:
	case VFMADDSUB213PHZrkz:
		return true;
	}
	return false;
}

bool isVSUBSS(unsigned Opcode) {
	switch (Opcode) {
	case VSUBSSZrm_Int:
	case VSUBSSZrm_Intk:
	case VSUBSSZrm_Intkz:
	case VSUBSSZrr_Int:
	case VSUBSSZrr_Intk:
	case VSUBSSZrr_Intkz:
	case VSUBSSZrrb_Int:
	case VSUBSSZrrb_Intk:
	case VSUBSSZrrb_Intkz:
	case VSUBSSrm_Int:
	case VSUBSSrr_Int:
		return true;
	}
	return false;
}

bool isVPANDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPANDQZ128rm:
	case VPANDQZ128rmb:
	case VPANDQZ128rmbk:
	case VPANDQZ128rmbkz:
	case VPANDQZ128rmk:
	case VPANDQZ128rmkz:
	case VPANDQZ128rr:
	case VPANDQZ128rrk:
	case VPANDQZ128rrkz:
	case VPANDQZ256rm:
	case VPANDQZ256rmb:
	case VPANDQZ256rmbk:
	case VPANDQZ256rmbkz:
	case VPANDQZ256rmk:
	case VPANDQZ256rmkz:
	case VPANDQZ256rr:
	case VPANDQZ256rrk:
	case VPANDQZ256rrkz:
	case VPANDQZrm:
	case VPANDQZrmb:
	case VPANDQZrmbk:
	case VPANDQZrmbkz:
	case VPANDQZrmk:
	case VPANDQZrmkz:
	case VPANDQZrr:
	case VPANDQZrrk:
	case VPANDQZrrkz:
		return true;
	}
	return false;
}

bool isVCMPPH(unsigned Opcode) {
	switch (Opcode) {
	case VCMPPHZ128rmbi:
	case VCMPPHZ128rmbik:
	case VCMPPHZ128rmi:
	case VCMPPHZ128rmik:
	case VCMPPHZ128rri:
	case VCMPPHZ128rrik:
	case VCMPPHZ256rmbi:
	case VCMPPHZ256rmbik:
	case VCMPPHZ256rmi:
	case VCMPPHZ256rmik:
	case VCMPPHZ256rri:
	case VCMPPHZ256rrik:
	case VCMPPHZrmbi:
	case VCMPPHZrmbik:
	case VCMPPHZrmi:
	case VCMPPHZrmik:
	case VCMPPHZrri:
	case VCMPPHZrrib:
	case VCMPPHZrribk:
	case VCMPPHZrrik:
		return true;
	}
	return false;
}

bool isVP4DPWSSD(unsigned Opcode) {
	switch (Opcode) {
	case VP4DPWSSDrm:
	case VP4DPWSSDrmk:
	case VP4DPWSSDrmkz:
		return true;
	}
	return false;
}

bool isENDBR32(unsigned Opcode) {
	return Opcode == ENDBR32;
}

bool isEMMS(unsigned Opcode) {
	return Opcode == MMX_EMMS;
}

bool isXCHG(unsigned Opcode) {
	switch (Opcode) {
	case XCHG16ar:
	case XCHG16rm:
	case XCHG16rr:
	case XCHG32ar:
	case XCHG32rm:
	case XCHG32rr:
	case XCHG64ar:
	case XCHG64rm:
	case XCHG64rr:
	case XCHG8rm:
	case XCHG8rr:
		return true;
	}
	return false;
}

bool isVFMADDSUB213PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB213PSYm:
	case VFMADDSUB213PSYr:
	case VFMADDSUB213PSZ128m:
	case VFMADDSUB213PSZ128mb:
	case VFMADDSUB213PSZ128mbk:
	case VFMADDSUB213PSZ128mbkz:
	case VFMADDSUB213PSZ128mk:
	case VFMADDSUB213PSZ128mkz:
	case VFMADDSUB213PSZ128r:
	case VFMADDSUB213PSZ128rk:
	case VFMADDSUB213PSZ128rkz:
	case VFMADDSUB213PSZ256m:
	case VFMADDSUB213PSZ256mb:
	case VFMADDSUB213PSZ256mbk:
	case VFMADDSUB213PSZ256mbkz:
	case VFMADDSUB213PSZ256mk:
	case VFMADDSUB213PSZ256mkz:
	case VFMADDSUB213PSZ256r:
	case VFMADDSUB213PSZ256rk:
	case VFMADDSUB213PSZ256rkz:
	case VFMADDSUB213PSZm:
	case VFMADDSUB213PSZmb:
	case VFMADDSUB213PSZmbk:
	case VFMADDSUB213PSZmbkz:
	case VFMADDSUB213PSZmk:
	case VFMADDSUB213PSZmkz:
	case VFMADDSUB213PSZr:
	case VFMADDSUB213PSZrb:
	case VFMADDSUB213PSZrbk:
	case VFMADDSUB213PSZrbkz:
	case VFMADDSUB213PSZrk:
	case VFMADDSUB213PSZrkz:
	case VFMADDSUB213PSm:
	case VFMADDSUB213PSr:
		return true;
	}
	return false;
}

bool isTDPBUSD(unsigned Opcode) {
	return Opcode == TDPBUSD;
}

bool isRDSEED(unsigned Opcode) {
	switch (Opcode) {
	case RDSEED16r:
	case RDSEED32r:
	case RDSEED64r:
		return true;
	}
	return false;
}

bool isVCMPPS(unsigned Opcode) {
	switch (Opcode) {
	case VCMPPSYrmi:
	case VCMPPSYrri:
	case VCMPPSZ128rmbi:
	case VCMPPSZ128rmbik:
	case VCMPPSZ128rmi:
	case VCMPPSZ128rmik:
	case VCMPPSZ128rri:
	case VCMPPSZ128rrik:
	case VCMPPSZ256rmbi:
	case VCMPPSZ256rmbik:
	case VCMPPSZ256rmi:
	case VCMPPSZ256rmik:
	case VCMPPSZ256rri:
	case VCMPPSZ256rrik:
	case VCMPPSZrmbi:
	case VCMPPSZrmbik:
	case VCMPPSZrmi:
	case VCMPPSZrmik:
	case VCMPPSZrri:
	case VCMPPSZrrib:
	case VCMPPSZrribk:
	case VCMPPSZrrik:
	case VCMPPSrmi:
	case VCMPPSrri:
		return true;
	}
	return false;
}

bool isVRCP28SD(unsigned Opcode) {
	switch (Opcode) {
	case VRCP28SDZm:
	case VRCP28SDZmk:
	case VRCP28SDZmkz:
	case VRCP28SDZr:
	case VRCP28SDZrb:
	case VRCP28SDZrbk:
	case VRCP28SDZrbkz:
	case VRCP28SDZrk:
	case VRCP28SDZrkz:
		return true;
	}
	return false;
}

bool isRDMSRLIST(unsigned Opcode) {
	return Opcode == RDMSRLIST;
}

bool isVRCP28SS(unsigned Opcode) {
	switch (Opcode) {
	case VRCP28SSZm:
	case VRCP28SSZmk:
	case VRCP28SSZmkz:
	case VRCP28SSZr:
	case VRCP28SSZrb:
	case VRCP28SSZrbk:
	case VRCP28SSZrbkz:
	case VRCP28SSZrk:
	case VRCP28SSZrkz:
		return true;
	}
	return false;
}

bool isV4FMADDSS(unsigned Opcode) {
	switch (Opcode) {
	case V4FMADDSSrm:
	case V4FMADDSSrmk:
	case V4FMADDSSrmkz:
		return true;
	}
	return false;
}

bool isAESKEYGENASSIST(unsigned Opcode) {
	switch (Opcode) {
	case AESKEYGENASSIST128rm:
	case AESKEYGENASSIST128rr:
		return true;
	}
	return false;
}

bool isFUCOMPI(unsigned Opcode) {
	return Opcode == UCOM_FIPr;
}

bool isTDPBF16PS(unsigned Opcode) {
	return Opcode == TDPBF16PS;
}

bool isFUCOMPP(unsigned Opcode) {
	return Opcode == UCOM_FPPr;
}

bool isVFMADDSUB231PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB231PDYm:
	case VFMADDSUB231PDYr:
	case VFMADDSUB231PDZ128m:
	case VFMADDSUB231PDZ128mb:
	case VFMADDSUB231PDZ128mbk:
	case VFMADDSUB231PDZ128mbkz:
	case VFMADDSUB231PDZ128mk:
	case VFMADDSUB231PDZ128mkz:
	case VFMADDSUB231PDZ128r:
	case VFMADDSUB231PDZ128rk:
	case VFMADDSUB231PDZ128rkz:
	case VFMADDSUB231PDZ256m:
	case VFMADDSUB231PDZ256mb:
	case VFMADDSUB231PDZ256mbk:
	case VFMADDSUB231PDZ256mbkz:
	case VFMADDSUB231PDZ256mk:
	case VFMADDSUB231PDZ256mkz:
	case VFMADDSUB231PDZ256r:
	case VFMADDSUB231PDZ256rk:
	case VFMADDSUB231PDZ256rkz:
	case VFMADDSUB231PDZm:
	case VFMADDSUB231PDZmb:
	case VFMADDSUB231PDZmbk:
	case VFMADDSUB231PDZmbkz:
	case VFMADDSUB231PDZmk:
	case VFMADDSUB231PDZmkz:
	case VFMADDSUB231PDZr:
	case VFMADDSUB231PDZrb:
	case VFMADDSUB231PDZrbk:
	case VFMADDSUB231PDZrbkz:
	case VFMADDSUB231PDZrk:
	case VFMADDSUB231PDZrkz:
	case VFMADDSUB231PDm:
	case VFMADDSUB231PDr:
		return true;
	}
	return false;
}

bool isVCVTPH2PSX(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2PSXZ128rm:
	case VCVTPH2PSXZ128rmb:
	case VCVTPH2PSXZ128rmbk:
	case VCVTPH2PSXZ128rmbkz:
	case VCVTPH2PSXZ128rmk:
	case VCVTPH2PSXZ128rmkz:
	case VCVTPH2PSXZ128rr:
	case VCVTPH2PSXZ128rrk:
	case VCVTPH2PSXZ128rrkz:
	case VCVTPH2PSXZ256rm:
	case VCVTPH2PSXZ256rmb:
	case VCVTPH2PSXZ256rmbk:
	case VCVTPH2PSXZ256rmbkz:
	case VCVTPH2PSXZ256rmk:
	case VCVTPH2PSXZ256rmkz:
	case VCVTPH2PSXZ256rr:
	case VCVTPH2PSXZ256rrk:
	case VCVTPH2PSXZ256rrkz:
	case VCVTPH2PSXZrm:
	case VCVTPH2PSXZrmb:
	case VCVTPH2PSXZrmbk:
	case VCVTPH2PSXZrmbkz:
	case VCVTPH2PSXZrmk:
	case VCVTPH2PSXZrmkz:
	case VCVTPH2PSXZrr:
	case VCVTPH2PSXZrrb:
	case VCVTPH2PSXZrrbk:
	case VCVTPH2PSXZrrbkz:
	case VCVTPH2PSXZrrk:
	case VCVTPH2PSXZrrkz:
		return true;
	}
	return false;
}

bool isVFMADDSUB231PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB231PHZ128m:
	case VFMADDSUB231PHZ128mb:
	case VFMADDSUB231PHZ128mbk:
	case VFMADDSUB231PHZ128mbkz:
	case VFMADDSUB231PHZ128mk:
	case VFMADDSUB231PHZ128mkz:
	case VFMADDSUB231PHZ128r:
	case VFMADDSUB231PHZ128rk:
	case VFMADDSUB231PHZ128rkz:
	case VFMADDSUB231PHZ256m:
	case VFMADDSUB231PHZ256mb:
	case VFMADDSUB231PHZ256mbk:
	case VFMADDSUB231PHZ256mbkz:
	case VFMADDSUB231PHZ256mk:
	case VFMADDSUB231PHZ256mkz:
	case VFMADDSUB231PHZ256r:
	case VFMADDSUB231PHZ256rk:
	case VFMADDSUB231PHZ256rkz:
	case VFMADDSUB231PHZm:
	case VFMADDSUB231PHZmb:
	case VFMADDSUB231PHZmbk:
	case VFMADDSUB231PHZmbkz:
	case VFMADDSUB231PHZmk:
	case VFMADDSUB231PHZmkz:
	case VFMADDSUB231PHZr:
	case VFMADDSUB231PHZrb:
	case VFMADDSUB231PHZrbk:
	case VFMADDSUB231PHZrbkz:
	case VFMADDSUB231PHZrk:
	case VFMADDSUB231PHZrkz:
		return true;
	}
	return false;
}

bool isBLENDVPD(unsigned Opcode) {
	switch (Opcode) {
	case BLENDVPDrm0:
	case BLENDVPDrr0:
		return true;
	}
	return false;
}

bool isPSWAPD(unsigned Opcode) {
	switch (Opcode) {
	case PSWAPDrm:
	case PSWAPDrr:
		return true;
	}
	return false;
}

bool isVMSAVE(unsigned Opcode) {
	switch (Opcode) {
	case VMSAVE32:
	case VMSAVE64:
		return true;
	}
	return false;
}

bool isVFMADDSUB231PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUB231PSYm:
	case VFMADDSUB231PSYr:
	case VFMADDSUB231PSZ128m:
	case VFMADDSUB231PSZ128mb:
	case VFMADDSUB231PSZ128mbk:
	case VFMADDSUB231PSZ128mbkz:
	case VFMADDSUB231PSZ128mk:
	case VFMADDSUB231PSZ128mkz:
	case VFMADDSUB231PSZ128r:
	case VFMADDSUB231PSZ128rk:
	case VFMADDSUB231PSZ128rkz:
	case VFMADDSUB231PSZ256m:
	case VFMADDSUB231PSZ256mb:
	case VFMADDSUB231PSZ256mbk:
	case VFMADDSUB231PSZ256mbkz:
	case VFMADDSUB231PSZ256mk:
	case VFMADDSUB231PSZ256mkz:
	case VFMADDSUB231PSZ256r:
	case VFMADDSUB231PSZ256rk:
	case VFMADDSUB231PSZ256rkz:
	case VFMADDSUB231PSZm:
	case VFMADDSUB231PSZmb:
	case VFMADDSUB231PSZmbk:
	case VFMADDSUB231PSZmbkz:
	case VFMADDSUB231PSZmk:
	case VFMADDSUB231PSZmkz:
	case VFMADDSUB231PSZr:
	case VFMADDSUB231PSZrb:
	case VFMADDSUB231PSZrbk:
	case VFMADDSUB231PSZrbkz:
	case VFMADDSUB231PSZrk:
	case VFMADDSUB231PSZrkz:
	case VFMADDSUB231PSm:
	case VFMADDSUB231PSr:
		return true;
	}
	return false;
}

bool isTDPBUUD(unsigned Opcode) {
	return Opcode == TDPBUUD;
}

bool isVBLENDMPD(unsigned Opcode) {
	switch (Opcode) {
	case VBLENDMPDZ128rm:
	case VBLENDMPDZ128rmb:
	case VBLENDMPDZ128rmbk:
	case VBLENDMPDZ128rmbkz:
	case VBLENDMPDZ128rmk:
	case VBLENDMPDZ128rmkz:
	case VBLENDMPDZ128rr:
	case VBLENDMPDZ128rrk:
	case VBLENDMPDZ128rrkz:
	case VBLENDMPDZ256rm:
	case VBLENDMPDZ256rmb:
	case VBLENDMPDZ256rmbk:
	case VBLENDMPDZ256rmbkz:
	case VBLENDMPDZ256rmk:
	case VBLENDMPDZ256rmkz:
	case VBLENDMPDZ256rr:
	case VBLENDMPDZ256rrk:
	case VBLENDMPDZ256rrkz:
	case VBLENDMPDZrm:
	case VBLENDMPDZrmb:
	case VBLENDMPDZrmbk:
	case VBLENDMPDZrmbkz:
	case VBLENDMPDZrmk:
	case VBLENDMPDZrmkz:
	case VBLENDMPDZrr:
	case VBLENDMPDZrrk:
	case VBLENDMPDZrrkz:
		return true;
	}
	return false;
}

bool isPFACC(unsigned Opcode) {
	switch (Opcode) {
	case PFACCrm:
	case PFACCrr:
		return true;
	}
	return false;
}

bool isBLENDVPS(unsigned Opcode) {
	switch (Opcode) {
	case BLENDVPSrm0:
	case BLENDVPSrr0:
		return true;
	}
	return false;
}

bool isVPERM2I128(unsigned Opcode) {
	switch (Opcode) {
	case VPERM2I128rm:
	case VPERM2I128rr:
		return true;
	}
	return false;
}

bool isVPCMPGTB(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPGTBYrm:
	case VPCMPGTBYrr:
	case VPCMPGTBZ128rm:
	case VPCMPGTBZ128rmk:
	case VPCMPGTBZ128rr:
	case VPCMPGTBZ128rrk:
	case VPCMPGTBZ256rm:
	case VPCMPGTBZ256rmk:
	case VPCMPGTBZ256rr:
	case VPCMPGTBZ256rrk:
	case VPCMPGTBZrm:
	case VPCMPGTBZrmk:
	case VPCMPGTBZrr:
	case VPCMPGTBZrrk:
	case VPCMPGTBrm:
	case VPCMPGTBrr:
		return true;
	}
	return false;
}

bool isLLWPCB(unsigned Opcode) {
	switch (Opcode) {
	case LLWPCB:
	case LLWPCB64:
		return true;
	}
	return false;
}

bool isVPCMPGTD(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPGTDYrm:
	case VPCMPGTDYrr:
	case VPCMPGTDZ128rm:
	case VPCMPGTDZ128rmb:
	case VPCMPGTDZ128rmbk:
	case VPCMPGTDZ128rmk:
	case VPCMPGTDZ128rr:
	case VPCMPGTDZ128rrk:
	case VPCMPGTDZ256rm:
	case VPCMPGTDZ256rmb:
	case VPCMPGTDZ256rmbk:
	case VPCMPGTDZ256rmk:
	case VPCMPGTDZ256rr:
	case VPCMPGTDZ256rrk:
	case VPCMPGTDZrm:
	case VPCMPGTDZrmb:
	case VPCMPGTDZrmbk:
	case VPCMPGTDZrmk:
	case VPCMPGTDZrr:
	case VPCMPGTDZrrk:
	case VPCMPGTDrm:
	case VPCMPGTDrr:
		return true;
	}
	return false;
}

bool isVFMSUBADD213PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD213PDYm:
	case VFMSUBADD213PDYr:
	case VFMSUBADD213PDZ128m:
	case VFMSUBADD213PDZ128mb:
	case VFMSUBADD213PDZ128mbk:
	case VFMSUBADD213PDZ128mbkz:
	case VFMSUBADD213PDZ128mk:
	case VFMSUBADD213PDZ128mkz:
	case VFMSUBADD213PDZ128r:
	case VFMSUBADD213PDZ128rk:
	case VFMSUBADD213PDZ128rkz:
	case VFMSUBADD213PDZ256m:
	case VFMSUBADD213PDZ256mb:
	case VFMSUBADD213PDZ256mbk:
	case VFMSUBADD213PDZ256mbkz:
	case VFMSUBADD213PDZ256mk:
	case VFMSUBADD213PDZ256mkz:
	case VFMSUBADD213PDZ256r:
	case VFMSUBADD213PDZ256rk:
	case VFMSUBADD213PDZ256rkz:
	case VFMSUBADD213PDZm:
	case VFMSUBADD213PDZmb:
	case VFMSUBADD213PDZmbk:
	case VFMSUBADD213PDZmbkz:
	case VFMSUBADD213PDZmk:
	case VFMSUBADD213PDZmkz:
	case VFMSUBADD213PDZr:
	case VFMSUBADD213PDZrb:
	case VFMSUBADD213PDZrbk:
	case VFMSUBADD213PDZrbkz:
	case VFMSUBADD213PDZrk:
	case VFMSUBADD213PDZrkz:
	case VFMSUBADD213PDm:
	case VFMSUBADD213PDr:
		return true;
	}
	return false;
}

bool isVFMSUBADD213PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD213PHZ128m:
	case VFMSUBADD213PHZ128mb:
	case VFMSUBADD213PHZ128mbk:
	case VFMSUBADD213PHZ128mbkz:
	case VFMSUBADD213PHZ128mk:
	case VFMSUBADD213PHZ128mkz:
	case VFMSUBADD213PHZ128r:
	case VFMSUBADD213PHZ128rk:
	case VFMSUBADD213PHZ128rkz:
	case VFMSUBADD213PHZ256m:
	case VFMSUBADD213PHZ256mb:
	case VFMSUBADD213PHZ256mbk:
	case VFMSUBADD213PHZ256mbkz:
	case VFMSUBADD213PHZ256mk:
	case VFMSUBADD213PHZ256mkz:
	case VFMSUBADD213PHZ256r:
	case VFMSUBADD213PHZ256rk:
	case VFMSUBADD213PHZ256rkz:
	case VFMSUBADD213PHZm:
	case VFMSUBADD213PHZmb:
	case VFMSUBADD213PHZmbk:
	case VFMSUBADD213PHZmbkz:
	case VFMSUBADD213PHZmk:
	case VFMSUBADD213PHZmkz:
	case VFMSUBADD213PHZr:
	case VFMSUBADD213PHZrb:
	case VFMSUBADD213PHZrbk:
	case VFMSUBADD213PHZrbkz:
	case VFMSUBADD213PHZrk:
	case VFMSUBADD213PHZrkz:
		return true;
	}
	return false;
}

bool isVBLENDMPS(unsigned Opcode) {
	switch (Opcode) {
	case VBLENDMPSZ128rm:
	case VBLENDMPSZ128rmb:
	case VBLENDMPSZ128rmbk:
	case VBLENDMPSZ128rmbkz:
	case VBLENDMPSZ128rmk:
	case VBLENDMPSZ128rmkz:
	case VBLENDMPSZ128rr:
	case VBLENDMPSZ128rrk:
	case VBLENDMPSZ128rrkz:
	case VBLENDMPSZ256rm:
	case VBLENDMPSZ256rmb:
	case VBLENDMPSZ256rmbk:
	case VBLENDMPSZ256rmbkz:
	case VBLENDMPSZ256rmk:
	case VBLENDMPSZ256rmkz:
	case VBLENDMPSZ256rr:
	case VBLENDMPSZ256rrk:
	case VBLENDMPSZ256rrkz:
	case VBLENDMPSZrm:
	case VBLENDMPSZrmb:
	case VBLENDMPSZrmbk:
	case VBLENDMPSZrmbkz:
	case VBLENDMPSZrmk:
	case VBLENDMPSZrmkz:
	case VBLENDMPSZrr:
	case VBLENDMPSZrrk:
	case VBLENDMPSZrrkz:
		return true;
	}
	return false;
}

bool isVCMPSD(unsigned Opcode) {
	switch (Opcode) {
	case VCMPSDZrm_Int:
	case VCMPSDZrm_Intk:
	case VCMPSDZrr_Int:
	case VCMPSDZrr_Intk:
	case VCMPSDZrrb_Int:
	case VCMPSDZrrb_Intk:
	case VCMPSDrm_Int:
	case VCMPSDrr_Int:
		return true;
	}
	return false;
}

bool isVCMPSH(unsigned Opcode) {
	switch (Opcode) {
	case VCMPSHZrm_Int:
	case VCMPSHZrm_Intk:
	case VCMPSHZrr_Int:
	case VCMPSHZrr_Intk:
	case VCMPSHZrrb_Int:
	case VCMPSHZrrb_Intk:
		return true;
	}
	return false;
}

bool isVPCMPGTQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPGTQYrm:
	case VPCMPGTQYrr:
	case VPCMPGTQZ128rm:
	case VPCMPGTQZ128rmb:
	case VPCMPGTQZ128rmbk:
	case VPCMPGTQZ128rmk:
	case VPCMPGTQZ128rr:
	case VPCMPGTQZ128rrk:
	case VPCMPGTQZ256rm:
	case VPCMPGTQZ256rmb:
	case VPCMPGTQZ256rmbk:
	case VPCMPGTQZ256rmk:
	case VPCMPGTQZ256rr:
	case VPCMPGTQZ256rrk:
	case VPCMPGTQZrm:
	case VPCMPGTQZrmb:
	case VPCMPGTQZrmbk:
	case VPCMPGTQZrmk:
	case VPCMPGTQZrr:
	case VPCMPGTQZrrk:
	case VPCMPGTQrm:
	case VPCMPGTQrr:
		return true;
	}
	return false;
}

bool isANDNPD(unsigned Opcode) {
	switch (Opcode) {
	case ANDNPDrm:
	case ANDNPDrr:
		return true;
	}
	return false;
}

bool isENDBR64(unsigned Opcode) {
	return Opcode == ENDBR64;
}

bool isVFMSUBADD213PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD213PSYm:
	case VFMSUBADD213PSYr:
	case VFMSUBADD213PSZ128m:
	case VFMSUBADD213PSZ128mb:
	case VFMSUBADD213PSZ128mbk:
	case VFMSUBADD213PSZ128mbkz:
	case VFMSUBADD213PSZ128mk:
	case VFMSUBADD213PSZ128mkz:
	case VFMSUBADD213PSZ128r:
	case VFMSUBADD213PSZ128rk:
	case VFMSUBADD213PSZ128rkz:
	case VFMSUBADD213PSZ256m:
	case VFMSUBADD213PSZ256mb:
	case VFMSUBADD213PSZ256mbk:
	case VFMSUBADD213PSZ256mbkz:
	case VFMSUBADD213PSZ256mk:
	case VFMSUBADD213PSZ256mkz:
	case VFMSUBADD213PSZ256r:
	case VFMSUBADD213PSZ256rk:
	case VFMSUBADD213PSZ256rkz:
	case VFMSUBADD213PSZm:
	case VFMSUBADD213PSZmb:
	case VFMSUBADD213PSZmbk:
	case VFMSUBADD213PSZmbkz:
	case VFMSUBADD213PSZmk:
	case VFMSUBADD213PSZmkz:
	case VFMSUBADD213PSZr:
	case VFMSUBADD213PSZrb:
	case VFMSUBADD213PSZrbk:
	case VFMSUBADD213PSZrbkz:
	case VFMSUBADD213PSZrk:
	case VFMSUBADD213PSZrkz:
	case VFMSUBADD213PSm:
	case VFMSUBADD213PSr:
		return true;
	}
	return false;
}

bool isVPCMPGTW(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPGTWYrm:
	case VPCMPGTWYrr:
	case VPCMPGTWZ128rm:
	case VPCMPGTWZ128rmk:
	case VPCMPGTWZ128rr:
	case VPCMPGTWZ128rrk:
	case VPCMPGTWZ256rm:
	case VPCMPGTWZ256rmk:
	case VPCMPGTWZ256rr:
	case VPCMPGTWZ256rrk:
	case VPCMPGTWZrm:
	case VPCMPGTWZrmk:
	case VPCMPGTWZrr:
	case VPCMPGTWZrrk:
	case VPCMPGTWrm:
	case VPCMPGTWrr:
		return true;
	}
	return false;
}

bool isVCMPSS(unsigned Opcode) {
	switch (Opcode) {
	case VCMPSSZrm_Int:
	case VCMPSSZrm_Intk:
	case VCMPSSZrr_Int:
	case VCMPSSZrr_Intk:
	case VCMPSSZrrb_Int:
	case VCMPSSZrrb_Intk:
	case VCMPSSrm_Int:
	case VCMPSSrr_Int:
		return true;
	}
	return false;
}

bool isPFADD(unsigned Opcode) {
	switch (Opcode) {
	case PFADDrm:
	case PFADDrr:
		return true;
	}
	return false;
}

bool isVMOVLHPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVLHPSZrr:
	case VMOVLHPSrr:
		return true;
	}
	return false;
}

bool isVPMINSD(unsigned Opcode) {
	switch (Opcode) {
	case VPMINSDYrm:
	case VPMINSDYrr:
	case VPMINSDZ128rm:
	case VPMINSDZ128rmb:
	case VPMINSDZ128rmbk:
	case VPMINSDZ128rmbkz:
	case VPMINSDZ128rmk:
	case VPMINSDZ128rmkz:
	case VPMINSDZ128rr:
	case VPMINSDZ128rrk:
	case VPMINSDZ128rrkz:
	case VPMINSDZ256rm:
	case VPMINSDZ256rmb:
	case VPMINSDZ256rmbk:
	case VPMINSDZ256rmbkz:
	case VPMINSDZ256rmk:
	case VPMINSDZ256rmkz:
	case VPMINSDZ256rr:
	case VPMINSDZ256rrk:
	case VPMINSDZ256rrkz:
	case VPMINSDZrm:
	case VPMINSDZrmb:
	case VPMINSDZrmbk:
	case VPMINSDZrmbkz:
	case VPMINSDZrmk:
	case VPMINSDZrmkz:
	case VPMINSDZrr:
	case VPMINSDZrrk:
	case VPMINSDZrrkz:
	case VPMINSDrm:
	case VPMINSDrr:
		return true;
	}
	return false;
}

bool isVPMINSB(unsigned Opcode) {
	switch (Opcode) {
	case VPMINSBYrm:
	case VPMINSBYrr:
	case VPMINSBZ128rm:
	case VPMINSBZ128rmk:
	case VPMINSBZ128rmkz:
	case VPMINSBZ128rr:
	case VPMINSBZ128rrk:
	case VPMINSBZ128rrkz:
	case VPMINSBZ256rm:
	case VPMINSBZ256rmk:
	case VPMINSBZ256rmkz:
	case VPMINSBZ256rr:
	case VPMINSBZ256rrk:
	case VPMINSBZ256rrkz:
	case VPMINSBZrm:
	case VPMINSBZrmk:
	case VPMINSBZrmkz:
	case VPMINSBZrr:
	case VPMINSBZrrk:
	case VPMINSBZrrkz:
	case VPMINSBrm:
	case VPMINSBrr:
		return true;
	}
	return false;
}

bool isANDNPS(unsigned Opcode) {
	switch (Opcode) {
	case ANDNPSrm:
	case ANDNPSrr:
		return true;
	}
	return false;
}

bool isPHADDSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PHADDSWrm:
	case MMX_PHADDSWrr:
	case PHADDSWrm:
	case PHADDSWrr:
		return true;
	}
	return false;
}

bool isVPSLLVD(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLVDYrm:
	case VPSLLVDYrr:
	case VPSLLVDZ128rm:
	case VPSLLVDZ128rmb:
	case VPSLLVDZ128rmbk:
	case VPSLLVDZ128rmbkz:
	case VPSLLVDZ128rmk:
	case VPSLLVDZ128rmkz:
	case VPSLLVDZ128rr:
	case VPSLLVDZ128rrk:
	case VPSLLVDZ128rrkz:
	case VPSLLVDZ256rm:
	case VPSLLVDZ256rmb:
	case VPSLLVDZ256rmbk:
	case VPSLLVDZ256rmbkz:
	case VPSLLVDZ256rmk:
	case VPSLLVDZ256rmkz:
	case VPSLLVDZ256rr:
	case VPSLLVDZ256rrk:
	case VPSLLVDZ256rrkz:
	case VPSLLVDZrm:
	case VPSLLVDZrmb:
	case VPSLLVDZrmbk:
	case VPSLLVDZrmbkz:
	case VPSLLVDZrmk:
	case VPSLLVDZrmkz:
	case VPSLLVDZrr:
	case VPSLLVDZrrk:
	case VPSLLVDZrrkz:
	case VPSLLVDrm:
	case VPSLLVDrr:
		return true;
	}
	return false;
}

bool isVDIVPD(unsigned Opcode) {
	switch (Opcode) {
	case VDIVPDYrm:
	case VDIVPDYrr:
	case VDIVPDZ128rm:
	case VDIVPDZ128rmb:
	case VDIVPDZ128rmbk:
	case VDIVPDZ128rmbkz:
	case VDIVPDZ128rmk:
	case VDIVPDZ128rmkz:
	case VDIVPDZ128rr:
	case VDIVPDZ128rrk:
	case VDIVPDZ128rrkz:
	case VDIVPDZ256rm:
	case VDIVPDZ256rmb:
	case VDIVPDZ256rmbk:
	case VDIVPDZ256rmbkz:
	case VDIVPDZ256rmk:
	case VDIVPDZ256rmkz:
	case VDIVPDZ256rr:
	case VDIVPDZ256rrk:
	case VDIVPDZ256rrkz:
	case VDIVPDZrm:
	case VDIVPDZrmb:
	case VDIVPDZrmbk:
	case VDIVPDZrmbkz:
	case VDIVPDZrmk:
	case VDIVPDZrmkz:
	case VDIVPDZrr:
	case VDIVPDZrrb:
	case VDIVPDZrrbk:
	case VDIVPDZrrbkz:
	case VDIVPDZrrk:
	case VDIVPDZrrkz:
	case VDIVPDrm:
	case VDIVPDrr:
		return true;
	}
	return false;
}

bool isVPMINSQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMINSQZ128rm:
	case VPMINSQZ128rmb:
	case VPMINSQZ128rmbk:
	case VPMINSQZ128rmbkz:
	case VPMINSQZ128rmk:
	case VPMINSQZ128rmkz:
	case VPMINSQZ128rr:
	case VPMINSQZ128rrk:
	case VPMINSQZ128rrkz:
	case VPMINSQZ256rm:
	case VPMINSQZ256rmb:
	case VPMINSQZ256rmbk:
	case VPMINSQZ256rmbkz:
	case VPMINSQZ256rmk:
	case VPMINSQZ256rmkz:
	case VPMINSQZ256rr:
	case VPMINSQZ256rrk:
	case VPMINSQZ256rrkz:
	case VPMINSQZrm:
	case VPMINSQZrmb:
	case VPMINSQZrmbk:
	case VPMINSQZrmbkz:
	case VPMINSQZrmk:
	case VPMINSQZrmkz:
	case VPMINSQZrr:
	case VPMINSQZrrk:
	case VPMINSQZrrkz:
		return true;
	}
	return false;
}

bool isVDIVPH(unsigned Opcode) {
	switch (Opcode) {
	case VDIVPHZ128rm:
	case VDIVPHZ128rmb:
	case VDIVPHZ128rmbk:
	case VDIVPHZ128rmbkz:
	case VDIVPHZ128rmk:
	case VDIVPHZ128rmkz:
	case VDIVPHZ128rr:
	case VDIVPHZ128rrk:
	case VDIVPHZ128rrkz:
	case VDIVPHZ256rm:
	case VDIVPHZ256rmb:
	case VDIVPHZ256rmbk:
	case VDIVPHZ256rmbkz:
	case VDIVPHZ256rmk:
	case VDIVPHZ256rmkz:
	case VDIVPHZ256rr:
	case VDIVPHZ256rrk:
	case VDIVPHZ256rrkz:
	case VDIVPHZrm:
	case VDIVPHZrmb:
	case VDIVPHZrmbk:
	case VDIVPHZrmbkz:
	case VDIVPHZrmk:
	case VDIVPHZrmkz:
	case VDIVPHZrr:
	case VDIVPHZrrb:
	case VDIVPHZrrbk:
	case VDIVPHZrrbkz:
	case VDIVPHZrrk:
	case VDIVPHZrrkz:
		return true;
	}
	return false;
}

bool isVPMINSW(unsigned Opcode) {
	switch (Opcode) {
	case VPMINSWYrm:
	case VPMINSWYrr:
	case VPMINSWZ128rm:
	case VPMINSWZ128rmk:
	case VPMINSWZ128rmkz:
	case VPMINSWZ128rr:
	case VPMINSWZ128rrk:
	case VPMINSWZ128rrkz:
	case VPMINSWZ256rm:
	case VPMINSWZ256rmk:
	case VPMINSWZ256rmkz:
	case VPMINSWZ256rr:
	case VPMINSWZ256rrk:
	case VPMINSWZ256rrkz:
	case VPMINSWZrm:
	case VPMINSWZrmk:
	case VPMINSWZrmkz:
	case VPMINSWZrr:
	case VPMINSWZrrk:
	case VPMINSWZrrkz:
	case VPMINSWrm:
	case VPMINSWrr:
		return true;
	}
	return false;
}

bool isVFNMSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUBPD4Ymr:
	case VFNMSUBPD4Yrm:
	case VFNMSUBPD4Yrr:
	case VFNMSUBPD4Yrr_REV:
	case VFNMSUBPD4mr:
	case VFNMSUBPD4rm:
	case VFNMSUBPD4rr:
	case VFNMSUBPD4rr_REV:
		return true;
	}
	return false;
}

bool isLWPVAL(unsigned Opcode) {
	switch (Opcode) {
	case LWPVAL32rmi:
	case LWPVAL32rri:
	case LWPVAL64rmi:
	case LWPVAL64rri:
		return true;
	}
	return false;
}

bool isAESDEC128KL(unsigned Opcode) {
	return Opcode == AESDEC128KL;
}

bool isFIADD(unsigned Opcode) {
	switch (Opcode) {
	case ADD_FI16m:
	case ADD_FI32m:
		return true;
	}
	return false;
}

bool isVPAND(unsigned Opcode) {
	switch (Opcode) {
	case VPANDYrm:
	case VPANDYrr:
	case VPANDrm:
	case VPANDrr:
		return true;
	}
	return false;
}

bool isMOVSLDUP(unsigned Opcode) {
	switch (Opcode) {
	case MOVSLDUPrm:
	case MOVSLDUPrr:
		return true;
	}
	return false;
}

bool isVPSLLVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLVQYrm:
	case VPSLLVQYrr:
	case VPSLLVQZ128rm:
	case VPSLLVQZ128rmb:
	case VPSLLVQZ128rmbk:
	case VPSLLVQZ128rmbkz:
	case VPSLLVQZ128rmk:
	case VPSLLVQZ128rmkz:
	case VPSLLVQZ128rr:
	case VPSLLVQZ128rrk:
	case VPSLLVQZ128rrkz:
	case VPSLLVQZ256rm:
	case VPSLLVQZ256rmb:
	case VPSLLVQZ256rmbk:
	case VPSLLVQZ256rmbkz:
	case VPSLLVQZ256rmk:
	case VPSLLVQZ256rmkz:
	case VPSLLVQZ256rr:
	case VPSLLVQZ256rrk:
	case VPSLLVQZ256rrkz:
	case VPSLLVQZrm:
	case VPSLLVQZrmb:
	case VPSLLVQZrmbk:
	case VPSLLVQZrmbkz:
	case VPSLLVQZrmk:
	case VPSLLVQZrmkz:
	case VPSLLVQZrr:
	case VPSLLVQZrrk:
	case VPSLLVQZrrkz:
	case VPSLLVQrm:
	case VPSLLVQrr:
		return true;
	}
	return false;
}

bool isVDIVPS(unsigned Opcode) {
	switch (Opcode) {
	case VDIVPSYrm:
	case VDIVPSYrr:
	case VDIVPSZ128rm:
	case VDIVPSZ128rmb:
	case VDIVPSZ128rmbk:
	case VDIVPSZ128rmbkz:
	case VDIVPSZ128rmk:
	case VDIVPSZ128rmkz:
	case VDIVPSZ128rr:
	case VDIVPSZ128rrk:
	case VDIVPSZ128rrkz:
	case VDIVPSZ256rm:
	case VDIVPSZ256rmb:
	case VDIVPSZ256rmbk:
	case VDIVPSZ256rmbkz:
	case VDIVPSZ256rmk:
	case VDIVPSZ256rmkz:
	case VDIVPSZ256rr:
	case VDIVPSZ256rrk:
	case VDIVPSZ256rrkz:
	case VDIVPSZrm:
	case VDIVPSZrmb:
	case VDIVPSZrmbk:
	case VDIVPSZrmbkz:
	case VDIVPSZrmk:
	case VDIVPSZrmkz:
	case VDIVPSZrr:
	case VDIVPSZrrb:
	case VDIVPSZrrbk:
	case VDIVPSZrrbkz:
	case VDIVPSZrrk:
	case VDIVPSZrrkz:
	case VDIVPSrm:
	case VDIVPSrr:
		return true;
	}
	return false;
}

bool isCWD(unsigned Opcode) {
	return Opcode == CWD;
}

bool isVPSLLVW(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLVWZ128rm:
	case VPSLLVWZ128rmk:
	case VPSLLVWZ128rmkz:
	case VPSLLVWZ128rr:
	case VPSLLVWZ128rrk:
	case VPSLLVWZ128rrkz:
	case VPSLLVWZ256rm:
	case VPSLLVWZ256rmk:
	case VPSLLVWZ256rmkz:
	case VPSLLVWZ256rr:
	case VPSLLVWZ256rrk:
	case VPSLLVWZ256rrkz:
	case VPSLLVWZrm:
	case VPSLLVWZrmk:
	case VPSLLVWZrmkz:
	case VPSLLVWZrr:
	case VPSLLVWZrrk:
	case VPSLLVWZrrkz:
		return true;
	}
	return false;
}

bool isCWDE(unsigned Opcode) {
	return Opcode == CWDE;
}

bool isVFNMSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUBPS4Ymr:
	case VFNMSUBPS4Yrm:
	case VFNMSUBPS4Yrr:
	case VFNMSUBPS4Yrr_REV:
	case VFNMSUBPS4mr:
	case VFNMSUBPS4rm:
	case VFNMSUBPS4rr:
	case VFNMSUBPS4rr_REV:
		return true;
	}
	return false;
}

bool isVFMSUBADD231PD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD231PDYm:
	case VFMSUBADD231PDYr:
	case VFMSUBADD231PDZ128m:
	case VFMSUBADD231PDZ128mb:
	case VFMSUBADD231PDZ128mbk:
	case VFMSUBADD231PDZ128mbkz:
	case VFMSUBADD231PDZ128mk:
	case VFMSUBADD231PDZ128mkz:
	case VFMSUBADD231PDZ128r:
	case VFMSUBADD231PDZ128rk:
	case VFMSUBADD231PDZ128rkz:
	case VFMSUBADD231PDZ256m:
	case VFMSUBADD231PDZ256mb:
	case VFMSUBADD231PDZ256mbk:
	case VFMSUBADD231PDZ256mbkz:
	case VFMSUBADD231PDZ256mk:
	case VFMSUBADD231PDZ256mkz:
	case VFMSUBADD231PDZ256r:
	case VFMSUBADD231PDZ256rk:
	case VFMSUBADD231PDZ256rkz:
	case VFMSUBADD231PDZm:
	case VFMSUBADD231PDZmb:
	case VFMSUBADD231PDZmbk:
	case VFMSUBADD231PDZmbkz:
	case VFMSUBADD231PDZmk:
	case VFMSUBADD231PDZmkz:
	case VFMSUBADD231PDZr:
	case VFMSUBADD231PDZrb:
	case VFMSUBADD231PDZrbk:
	case VFMSUBADD231PDZrbkz:
	case VFMSUBADD231PDZrk:
	case VFMSUBADD231PDZrkz:
	case VFMSUBADD231PDm:
	case VFMSUBADD231PDr:
		return true;
	}
	return false;
}

bool isVPDPBUSD(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBUSDYrm:
	case VPDPBUSDYrr:
	case VPDPBUSDZ128m:
	case VPDPBUSDZ128mb:
	case VPDPBUSDZ128mbk:
	case VPDPBUSDZ128mbkz:
	case VPDPBUSDZ128mk:
	case VPDPBUSDZ128mkz:
	case VPDPBUSDZ128r:
	case VPDPBUSDZ128rk:
	case VPDPBUSDZ128rkz:
	case VPDPBUSDZ256m:
	case VPDPBUSDZ256mb:
	case VPDPBUSDZ256mbk:
	case VPDPBUSDZ256mbkz:
	case VPDPBUSDZ256mk:
	case VPDPBUSDZ256mkz:
	case VPDPBUSDZ256r:
	case VPDPBUSDZ256rk:
	case VPDPBUSDZ256rkz:
	case VPDPBUSDZm:
	case VPDPBUSDZmb:
	case VPDPBUSDZmbk:
	case VPDPBUSDZmbkz:
	case VPDPBUSDZmk:
	case VPDPBUSDZmkz:
	case VPDPBUSDZr:
	case VPDPBUSDZrk:
	case VPDPBUSDZrkz:
	case VPDPBUSDrm:
	case VPDPBUSDrr:
		return true;
	}
	return false;
}

bool isVFMSUBADD231PH(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD231PHZ128m:
	case VFMSUBADD231PHZ128mb:
	case VFMSUBADD231PHZ128mbk:
	case VFMSUBADD231PHZ128mbkz:
	case VFMSUBADD231PHZ128mk:
	case VFMSUBADD231PHZ128mkz:
	case VFMSUBADD231PHZ128r:
	case VFMSUBADD231PHZ128rk:
	case VFMSUBADD231PHZ128rkz:
	case VFMSUBADD231PHZ256m:
	case VFMSUBADD231PHZ256mb:
	case VFMSUBADD231PHZ256mbk:
	case VFMSUBADD231PHZ256mbkz:
	case VFMSUBADD231PHZ256mk:
	case VFMSUBADD231PHZ256mkz:
	case VFMSUBADD231PHZ256r:
	case VFMSUBADD231PHZ256rk:
	case VFMSUBADD231PHZ256rkz:
	case VFMSUBADD231PHZm:
	case VFMSUBADD231PHZmb:
	case VFMSUBADD231PHZmbk:
	case VFMSUBADD231PHZmbkz:
	case VFMSUBADD231PHZmk:
	case VFMSUBADD231PHZmkz:
	case VFMSUBADD231PHZr:
	case VFMSUBADD231PHZrb:
	case VFMSUBADD231PHZrbk:
	case VFMSUBADD231PHZrbkz:
	case VFMSUBADD231PHZrk:
	case VFMSUBADD231PHZrkz:
		return true;
	}
	return false;
}

bool isPFNACC(unsigned Opcode) {
	switch (Opcode) {
	case PFNACCrm:
	case PFNACCrr:
		return true;
	}
	return false;
}

bool isPFRSQRT(unsigned Opcode) {
	switch (Opcode) {
	case PFRSQRTrm:
	case PFRSQRTrr:
		return true;
	}
	return false;
}

bool isVPMACSDD(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSDDrm:
	case VPMACSDDrr:
		return true;
	}
	return false;
}

bool isVFMSUBADD231PS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADD231PSYm:
	case VFMSUBADD231PSYr:
	case VFMSUBADD231PSZ128m:
	case VFMSUBADD231PSZ128mb:
	case VFMSUBADD231PSZ128mbk:
	case VFMSUBADD231PSZ128mbkz:
	case VFMSUBADD231PSZ128mk:
	case VFMSUBADD231PSZ128mkz:
	case VFMSUBADD231PSZ128r:
	case VFMSUBADD231PSZ128rk:
	case VFMSUBADD231PSZ128rkz:
	case VFMSUBADD231PSZ256m:
	case VFMSUBADD231PSZ256mb:
	case VFMSUBADD231PSZ256mbk:
	case VFMSUBADD231PSZ256mbkz:
	case VFMSUBADD231PSZ256mk:
	case VFMSUBADD231PSZ256mkz:
	case VFMSUBADD231PSZ256r:
	case VFMSUBADD231PSZ256rk:
	case VFMSUBADD231PSZ256rkz:
	case VFMSUBADD231PSZm:
	case VFMSUBADD231PSZmb:
	case VFMSUBADD231PSZmbk:
	case VFMSUBADD231PSZmbkz:
	case VFMSUBADD231PSZmk:
	case VFMSUBADD231PSZmkz:
	case VFMSUBADD231PSZr:
	case VFMSUBADD231PSZrb:
	case VFMSUBADD231PSZrbk:
	case VFMSUBADD231PSZrbkz:
	case VFMSUBADD231PSZrk:
	case VFMSUBADD231PSZrkz:
	case VFMSUBADD231PSm:
	case VFMSUBADD231PSr:
		return true;
	}
	return false;
}

bool isFRSTOR(unsigned Opcode) {
	return Opcode == FRSTORm;
}

bool isVPMINUB(unsigned Opcode) {
	switch (Opcode) {
	case VPMINUBYrm:
	case VPMINUBYrr:
	case VPMINUBZ128rm:
	case VPMINUBZ128rmk:
	case VPMINUBZ128rmkz:
	case VPMINUBZ128rr:
	case VPMINUBZ128rrk:
	case VPMINUBZ128rrkz:
	case VPMINUBZ256rm:
	case VPMINUBZ256rmk:
	case VPMINUBZ256rmkz:
	case VPMINUBZ256rr:
	case VPMINUBZ256rrk:
	case VPMINUBZ256rrkz:
	case VPMINUBZrm:
	case VPMINUBZrmk:
	case VPMINUBZrmkz:
	case VPMINUBZrr:
	case VPMINUBZrrk:
	case VPMINUBZrrkz:
	case VPMINUBrm:
	case VPMINUBrr:
		return true;
	}
	return false;
}

bool isVPMINUD(unsigned Opcode) {
	switch (Opcode) {
	case VPMINUDYrm:
	case VPMINUDYrr:
	case VPMINUDZ128rm:
	case VPMINUDZ128rmb:
	case VPMINUDZ128rmbk:
	case VPMINUDZ128rmbkz:
	case VPMINUDZ128rmk:
	case VPMINUDZ128rmkz:
	case VPMINUDZ128rr:
	case VPMINUDZ128rrk:
	case VPMINUDZ128rrkz:
	case VPMINUDZ256rm:
	case VPMINUDZ256rmb:
	case VPMINUDZ256rmbk:
	case VPMINUDZ256rmbkz:
	case VPMINUDZ256rmk:
	case VPMINUDZ256rmkz:
	case VPMINUDZ256rr:
	case VPMINUDZ256rrk:
	case VPMINUDZ256rrkz:
	case VPMINUDZrm:
	case VPMINUDZrmb:
	case VPMINUDZrmbk:
	case VPMINUDZrmbkz:
	case VPMINUDZrmk:
	case VPMINUDZrmkz:
	case VPMINUDZrr:
	case VPMINUDZrrk:
	case VPMINUDZrrkz:
	case VPMINUDrm:
	case VPMINUDrr:
		return true;
	}
	return false;
}

bool isKUNPCKBW(unsigned Opcode) {
	return Opcode == KUNPCKBWrr;
}

bool isDPPD(unsigned Opcode) {
	switch (Opcode) {
	case DPPDrmi:
	case DPPDrri:
		return true;
	}
	return false;
}

bool isVAESIMC(unsigned Opcode) {
	switch (Opcode) {
	case VAESIMCrm:
	case VAESIMCrr:
		return true;
	}
	return false;
}

bool isPTEST(unsigned Opcode) {
	switch (Opcode) {
	case PTESTrm:
	case PTESTrr:
		return true;
	}
	return false;
}

bool isVPMINUQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMINUQZ128rm:
	case VPMINUQZ128rmb:
	case VPMINUQZ128rmbk:
	case VPMINUQZ128rmbkz:
	case VPMINUQZ128rmk:
	case VPMINUQZ128rmkz:
	case VPMINUQZ128rr:
	case VPMINUQZ128rrk:
	case VPMINUQZ128rrkz:
	case VPMINUQZ256rm:
	case VPMINUQZ256rmb:
	case VPMINUQZ256rmbk:
	case VPMINUQZ256rmbkz:
	case VPMINUQZ256rmk:
	case VPMINUQZ256rmkz:
	case VPMINUQZ256rr:
	case VPMINUQZ256rrk:
	case VPMINUQZ256rrkz:
	case VPMINUQZrm:
	case VPMINUQZrmb:
	case VPMINUQZrmbk:
	case VPMINUQZrmbkz:
	case VPMINUQZrmk:
	case VPMINUQZrmkz:
	case VPMINUQZrr:
	case VPMINUQZrrk:
	case VPMINUQZrrkz:
		return true;
	}
	return false;
}

bool isUCOMISD(unsigned Opcode) {
	switch (Opcode) {
	case UCOMISDrm:
	case UCOMISDrr:
		return true;
	}
	return false;
}

bool isVPMINUW(unsigned Opcode) {
	switch (Opcode) {
	case VPMINUWYrm:
	case VPMINUWYrr:
	case VPMINUWZ128rm:
	case VPMINUWZ128rmk:
	case VPMINUWZ128rmkz:
	case VPMINUWZ128rr:
	case VPMINUWZ128rrk:
	case VPMINUWZ128rrkz:
	case VPMINUWZ256rm:
	case VPMINUWZ256rmk:
	case VPMINUWZ256rmkz:
	case VPMINUWZ256rr:
	case VPMINUWZ256rrk:
	case VPMINUWZ256rrkz:
	case VPMINUWZrm:
	case VPMINUWZrmk:
	case VPMINUWZrmkz:
	case VPMINUWZrr:
	case VPMINUWZrrk:
	case VPMINUWZrrkz:
	case VPMINUWrm:
	case VPMINUWrr:
		return true;
	}
	return false;
}

bool isDPPS(unsigned Opcode) {
	switch (Opcode) {
	case DPPSrmi:
	case DPPSrri:
		return true;
	}
	return false;
}

bool isFLDLG2(unsigned Opcode) {
	return Opcode == FLDLG2;
}

bool isVPMOVD2M(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVD2MZ128rr:
	case VPMOVD2MZ256rr:
	case VPMOVD2MZrr:
		return true;
	}
	return false;
}

bool isVMOVAPD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVAPDYmr:
	case VMOVAPDYrm:
	case VMOVAPDYrr:
	case VMOVAPDYrr_REV:
	case VMOVAPDZ128mr:
	case VMOVAPDZ128mrk:
	case VMOVAPDZ128rm:
	case VMOVAPDZ128rmk:
	case VMOVAPDZ128rmkz:
	case VMOVAPDZ128rr:
	case VMOVAPDZ128rr_REV:
	case VMOVAPDZ128rrk:
	case VMOVAPDZ128rrk_REV:
	case VMOVAPDZ128rrkz:
	case VMOVAPDZ128rrkz_REV:
	case VMOVAPDZ256mr:
	case VMOVAPDZ256mrk:
	case VMOVAPDZ256rm:
	case VMOVAPDZ256rmk:
	case VMOVAPDZ256rmkz:
	case VMOVAPDZ256rr:
	case VMOVAPDZ256rr_REV:
	case VMOVAPDZ256rrk:
	case VMOVAPDZ256rrk_REV:
	case VMOVAPDZ256rrkz:
	case VMOVAPDZ256rrkz_REV:
	case VMOVAPDZmr:
	case VMOVAPDZmrk:
	case VMOVAPDZrm:
	case VMOVAPDZrmk:
	case VMOVAPDZrmkz:
	case VMOVAPDZrr:
	case VMOVAPDZrr_REV:
	case VMOVAPDZrrk:
	case VMOVAPDZrrk_REV:
	case VMOVAPDZrrkz:
	case VMOVAPDZrrkz_REV:
	case VMOVAPDmr:
	case VMOVAPDrm:
	case VMOVAPDrr:
	case VMOVAPDrr_REV:
		return true;
	}
	return false;
}

bool isVPSRLD(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLDYri:
	case VPSRLDYrm:
	case VPSRLDYrr:
	case VPSRLDZ128mbi:
	case VPSRLDZ128mbik:
	case VPSRLDZ128mbikz:
	case VPSRLDZ128mi:
	case VPSRLDZ128mik:
	case VPSRLDZ128mikz:
	case VPSRLDZ128ri:
	case VPSRLDZ128rik:
	case VPSRLDZ128rikz:
	case VPSRLDZ128rm:
	case VPSRLDZ128rmk:
	case VPSRLDZ128rmkz:
	case VPSRLDZ128rr:
	case VPSRLDZ128rrk:
	case VPSRLDZ128rrkz:
	case VPSRLDZ256mbi:
	case VPSRLDZ256mbik:
	case VPSRLDZ256mbikz:
	case VPSRLDZ256mi:
	case VPSRLDZ256mik:
	case VPSRLDZ256mikz:
	case VPSRLDZ256ri:
	case VPSRLDZ256rik:
	case VPSRLDZ256rikz:
	case VPSRLDZ256rm:
	case VPSRLDZ256rmk:
	case VPSRLDZ256rmkz:
	case VPSRLDZ256rr:
	case VPSRLDZ256rrk:
	case VPSRLDZ256rrkz:
	case VPSRLDZmbi:
	case VPSRLDZmbik:
	case VPSRLDZmbikz:
	case VPSRLDZmi:
	case VPSRLDZmik:
	case VPSRLDZmikz:
	case VPSRLDZri:
	case VPSRLDZrik:
	case VPSRLDZrikz:
	case VPSRLDZrm:
	case VPSRLDZrmk:
	case VPSRLDZrmkz:
	case VPSRLDZrr:
	case VPSRLDZrrk:
	case VPSRLDZrrkz:
	case VPSRLDri:
	case VPSRLDrm:
	case VPSRLDrr:
		return true;
	}
	return false;
}

bool isTLBSYNC(unsigned Opcode) {
	return Opcode == TLBSYNC;
}

bool isXLATB(unsigned Opcode) {
	return Opcode == XLAT;
}

bool isUCOMISS(unsigned Opcode) {
	switch (Opcode) {
	case UCOMISSrm:
	case UCOMISSrr:
		return true;
	}
	return false;
}

bool isVPSRLQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLQYri:
	case VPSRLQYrm:
	case VPSRLQYrr:
	case VPSRLQZ128mbi:
	case VPSRLQZ128mbik:
	case VPSRLQZ128mbikz:
	case VPSRLQZ128mi:
	case VPSRLQZ128mik:
	case VPSRLQZ128mikz:
	case VPSRLQZ128ri:
	case VPSRLQZ128rik:
	case VPSRLQZ128rikz:
	case VPSRLQZ128rm:
	case VPSRLQZ128rmk:
	case VPSRLQZ128rmkz:
	case VPSRLQZ128rr:
	case VPSRLQZ128rrk:
	case VPSRLQZ128rrkz:
	case VPSRLQZ256mbi:
	case VPSRLQZ256mbik:
	case VPSRLQZ256mbikz:
	case VPSRLQZ256mi:
	case VPSRLQZ256mik:
	case VPSRLQZ256mikz:
	case VPSRLQZ256ri:
	case VPSRLQZ256rik:
	case VPSRLQZ256rikz:
	case VPSRLQZ256rm:
	case VPSRLQZ256rmk:
	case VPSRLQZ256rmkz:
	case VPSRLQZ256rr:
	case VPSRLQZ256rrk:
	case VPSRLQZ256rrkz:
	case VPSRLQZmbi:
	case VPSRLQZmbik:
	case VPSRLQZmbikz:
	case VPSRLQZmi:
	case VPSRLQZmik:
	case VPSRLQZmikz:
	case VPSRLQZri:
	case VPSRLQZrik:
	case VPSRLQZrikz:
	case VPSRLQZrm:
	case VPSRLQZrmk:
	case VPSRLQZrmkz:
	case VPSRLQZrr:
	case VPSRLQZrrk:
	case VPSRLQZrrkz:
	case VPSRLQri:
	case VPSRLQrm:
	case VPSRLQrr:
		return true;
	}
	return false;
}

bool isPDEP(unsigned Opcode) {
	switch (Opcode) {
	case PDEP32rm:
	case PDEP32rr:
	case PDEP64rm:
	case PDEP64rr:
		return true;
	}
	return false;
}

bool isVPDPBUUD(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBUUDYrm:
	case VPDPBUUDYrr:
	case VPDPBUUDrm:
	case VPDPBUUDrr:
		return true;
	}
	return false;
}

bool isVEXTRACTI32X4(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTI32x4Z256mr:
	case VEXTRACTI32x4Z256mrk:
	case VEXTRACTI32x4Z256rr:
	case VEXTRACTI32x4Z256rrk:
	case VEXTRACTI32x4Z256rrkz:
	case VEXTRACTI32x4Zmr:
	case VEXTRACTI32x4Zmrk:
	case VEXTRACTI32x4Zrr:
	case VEXTRACTI32x4Zrrk:
	case VEXTRACTI32x4Zrrkz:
		return true;
	}
	return false;
}

bool isPFCMPEQ(unsigned Opcode) {
	switch (Opcode) {
	case PFCMPEQrm:
	case PFCMPEQrr:
		return true;
	}
	return false;
}

bool isBLSIC(unsigned Opcode) {
	switch (Opcode) {
	case BLSIC32rm:
	case BLSIC32rr:
	case BLSIC64rm:
	case BLSIC64rr:
		return true;
	}
	return false;
}

bool isVPSRLW(unsigned Opcode) {
	switch (Opcode) {
	case VPSRLWYri:
	case VPSRLWYrm:
	case VPSRLWYrr:
	case VPSRLWZ128mi:
	case VPSRLWZ128mik:
	case VPSRLWZ128mikz:
	case VPSRLWZ128ri:
	case VPSRLWZ128rik:
	case VPSRLWZ128rikz:
	case VPSRLWZ128rm:
	case VPSRLWZ128rmk:
	case VPSRLWZ128rmkz:
	case VPSRLWZ128rr:
	case VPSRLWZ128rrk:
	case VPSRLWZ128rrkz:
	case VPSRLWZ256mi:
	case VPSRLWZ256mik:
	case VPSRLWZ256mikz:
	case VPSRLWZ256ri:
	case VPSRLWZ256rik:
	case VPSRLWZ256rikz:
	case VPSRLWZ256rm:
	case VPSRLWZ256rmk:
	case VPSRLWZ256rmkz:
	case VPSRLWZ256rr:
	case VPSRLWZ256rrk:
	case VPSRLWZ256rrkz:
	case VPSRLWZmi:
	case VPSRLWZmik:
	case VPSRLWZmikz:
	case VPSRLWZri:
	case VPSRLWZrik:
	case VPSRLWZrikz:
	case VPSRLWZrm:
	case VPSRLWZrmk:
	case VPSRLWZrmkz:
	case VPSRLWZrr:
	case VPSRLWZrrk:
	case VPSRLWZrrkz:
	case VPSRLWri:
	case VPSRLWrm:
	case VPSRLWrr:
		return true;
	}
	return false;
}

bool isVEXTRACTI32X8(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTI32x8Zmr:
	case VEXTRACTI32x8Zmrk:
	case VEXTRACTI32x8Zrr:
	case VEXTRACTI32x8Zrrk:
	case VEXTRACTI32x8Zrrkz:
		return true;
	}
	return false;
}

bool isVDIVSD(unsigned Opcode) {
	switch (Opcode) {
	case VDIVSDZrm_Int:
	case VDIVSDZrm_Intk:
	case VDIVSDZrm_Intkz:
	case VDIVSDZrr_Int:
	case VDIVSDZrr_Intk:
	case VDIVSDZrr_Intkz:
	case VDIVSDZrrb_Int:
	case VDIVSDZrrb_Intk:
	case VDIVSDZrrb_Intkz:
	case VDIVSDrm_Int:
	case VDIVSDrr_Int:
		return true;
	}
	return false;
}

bool isVFCMADDCPH(unsigned Opcode) {
	switch (Opcode) {
	case VFCMADDCPHZ128m:
	case VFCMADDCPHZ128mb:
	case VFCMADDCPHZ128mbk:
	case VFCMADDCPHZ128mbkz:
	case VFCMADDCPHZ128mk:
	case VFCMADDCPHZ128mkz:
	case VFCMADDCPHZ128r:
	case VFCMADDCPHZ128rk:
	case VFCMADDCPHZ128rkz:
	case VFCMADDCPHZ256m:
	case VFCMADDCPHZ256mb:
	case VFCMADDCPHZ256mbk:
	case VFCMADDCPHZ256mbkz:
	case VFCMADDCPHZ256mk:
	case VFCMADDCPHZ256mkz:
	case VFCMADDCPHZ256r:
	case VFCMADDCPHZ256rk:
	case VFCMADDCPHZ256rkz:
	case VFCMADDCPHZm:
	case VFCMADDCPHZmb:
	case VFCMADDCPHZmbk:
	case VFCMADDCPHZmbkz:
	case VFCMADDCPHZmk:
	case VFCMADDCPHZmkz:
	case VFCMADDCPHZr:
	case VFCMADDCPHZrb:
	case VFCMADDCPHZrbk:
	case VFCMADDCPHZrbkz:
	case VFCMADDCPHZrk:
	case VFCMADDCPHZrkz:
		return true;
	}
	return false;
}

bool isVMOVAPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVAPSYmr:
	case VMOVAPSYrm:
	case VMOVAPSYrr:
	case VMOVAPSYrr_REV:
	case VMOVAPSZ128mr:
	case VMOVAPSZ128mrk:
	case VMOVAPSZ128rm:
	case VMOVAPSZ128rmk:
	case VMOVAPSZ128rmkz:
	case VMOVAPSZ128rr:
	case VMOVAPSZ128rr_REV:
	case VMOVAPSZ128rrk:
	case VMOVAPSZ128rrk_REV:
	case VMOVAPSZ128rrkz:
	case VMOVAPSZ128rrkz_REV:
	case VMOVAPSZ256mr:
	case VMOVAPSZ256mrk:
	case VMOVAPSZ256rm:
	case VMOVAPSZ256rmk:
	case VMOVAPSZ256rmkz:
	case VMOVAPSZ256rr:
	case VMOVAPSZ256rr_REV:
	case VMOVAPSZ256rrk:
	case VMOVAPSZ256rrk_REV:
	case VMOVAPSZ256rrkz:
	case VMOVAPSZ256rrkz_REV:
	case VMOVAPSZmr:
	case VMOVAPSZmrk:
	case VMOVAPSZrm:
	case VMOVAPSZrmk:
	case VMOVAPSZrmkz:
	case VMOVAPSZrr:
	case VMOVAPSZrr_REV:
	case VMOVAPSZrrk:
	case VMOVAPSZrrk_REV:
	case VMOVAPSZrrkz:
	case VMOVAPSZrrkz_REV:
	case VMOVAPSmr:
	case VMOVAPSrm:
	case VMOVAPSrr:
	case VMOVAPSrr_REV:
		return true;
	}
	return false;
}

bool isFNOP(unsigned Opcode) {
	return Opcode == FNOP;
}

bool isVDIVSH(unsigned Opcode) {
	switch (Opcode) {
	case VDIVSHZrm_Int:
	case VDIVSHZrm_Intk:
	case VDIVSHZrm_Intkz:
	case VDIVSHZrr_Int:
	case VDIVSHZrr_Intk:
	case VDIVSHZrr_Intkz:
	case VDIVSHZrrb_Int:
	case VDIVSHZrrb_Intk:
	case VDIVSHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isBT(unsigned Opcode) {
	switch (Opcode) {
	case BT16mi8:
	case BT16mr:
	case BT16ri8:
	case BT16rr:
	case BT32mi8:
	case BT32mr:
	case BT32ri8:
	case BT32rr:
	case BT64mi8:
	case BT64mr:
	case BT64ri8:
	case BT64rr:
		return true;
	}
	return false;
}

bool isVFNMSUBSD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUBSD4mr:
	case VFNMSUBSD4rm:
	case VFNMSUBSD4rr:
	case VFNMSUBSD4rr_REV:
		return true;
	}
	return false;
}

bool isVPHMINPOSUW(unsigned Opcode) {
	switch (Opcode) {
	case VPHMINPOSUWrm:
	case VPHMINPOSUWrr:
		return true;
	}
	return false;
}

bool isVDIVSS(unsigned Opcode) {
	switch (Opcode) {
	case VDIVSSZrm_Int:
	case VDIVSSZrm_Intk:
	case VDIVSSZrm_Intkz:
	case VDIVSSZrr_Int:
	case VDIVSSZrr_Intk:
	case VDIVSSZrr_Intkz:
	case VDIVSSZrrb_Int:
	case VDIVSSZrrb_Intk:
	case VDIVSSZrrb_Intkz:
	case VDIVSSrm_Int:
	case VDIVSSrr_Int:
		return true;
	}
	return false;
}

bool isVCVTTSH2SI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTSH2SI64Zrm_Int:
	case VCVTTSH2SI64Zrr_Int:
	case VCVTTSH2SI64Zrrb_Int:
	case VCVTTSH2SIZrm_Int:
	case VCVTTSH2SIZrr_Int:
	case VCVTTSH2SIZrrb_Int:
		return true;
	}
	return false;
}

bool isKUNPCKDQ(unsigned Opcode) {
	return Opcode == KUNPCKDQrr;
}

bool isMULPD(unsigned Opcode) {
	switch (Opcode) {
	case MULPDrm:
	case MULPDrr:
		return true;
	}
	return false;
}

bool isBEXTR(unsigned Opcode) {
	switch (Opcode) {
	case BEXTR32rm:
	case BEXTR32rr:
	case BEXTR64rm:
	case BEXTR64rr:
	case BEXTRI32mi:
	case BEXTRI32ri:
	case BEXTRI64mi:
	case BEXTRI64ri:
		return true;
	}
	return false;
}

bool isVFNMSUBSS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUBSS4mr:
	case VFNMSUBSS4rm:
	case VFNMSUBSS4rr:
	case VFNMSUBSS4rr_REV:
		return true;
	}
	return false;
}

bool isMPSADBW(unsigned Opcode) {
	switch (Opcode) {
	case MPSADBWrmi:
	case MPSADBWrri:
		return true;
	}
	return false;
}

bool isMULPS(unsigned Opcode) {
	switch (Opcode) {
	case MULPSrm:
	case MULPSrr:
		return true;
	}
	return false;
}

bool isSHA256MSG1(unsigned Opcode) {
	switch (Opcode) {
	case SHA256MSG1rm:
	case SHA256MSG1rr:
		return true;
	}
	return false;
}

bool isSHA256MSG2(unsigned Opcode) {
	switch (Opcode) {
	case SHA256MSG2rm:
	case SHA256MSG2rr:
		return true;
	}
	return false;
}

bool isPOPF(unsigned Opcode) {
	return Opcode == POPF16;
}

bool isVERR(unsigned Opcode) {
	switch (Opcode) {
	case VERRm:
	case VERRr:
		return true;
	}
	return false;
}

bool isPFCMPGE(unsigned Opcode) {
	switch (Opcode) {
	case PFCMPGErm:
	case PFCMPGErr:
		return true;
	}
	return false;
}

bool isVCVTPS2UDQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2UDQZ128rm:
	case VCVTPS2UDQZ128rmb:
	case VCVTPS2UDQZ128rmbk:
	case VCVTPS2UDQZ128rmbkz:
	case VCVTPS2UDQZ128rmk:
	case VCVTPS2UDQZ128rmkz:
	case VCVTPS2UDQZ128rr:
	case VCVTPS2UDQZ128rrk:
	case VCVTPS2UDQZ128rrkz:
	case VCVTPS2UDQZ256rm:
	case VCVTPS2UDQZ256rmb:
	case VCVTPS2UDQZ256rmbk:
	case VCVTPS2UDQZ256rmbkz:
	case VCVTPS2UDQZ256rmk:
	case VCVTPS2UDQZ256rmkz:
	case VCVTPS2UDQZ256rr:
	case VCVTPS2UDQZ256rrk:
	case VCVTPS2UDQZ256rrkz:
	case VCVTPS2UDQZrm:
	case VCVTPS2UDQZrmb:
	case VCVTPS2UDQZrmbk:
	case VCVTPS2UDQZrmbkz:
	case VCVTPS2UDQZrmk:
	case VCVTPS2UDQZrmkz:
	case VCVTPS2UDQZrr:
	case VCVTPS2UDQZrrb:
	case VCVTPS2UDQZrrbk:
	case VCVTPS2UDQZrrbkz:
	case VCVTPS2UDQZrrk:
	case VCVTPS2UDQZrrkz:
		return true;
	}
	return false;
}

bool isVERW(unsigned Opcode) {
	switch (Opcode) {
	case VERWm:
	case VERWr:
		return true;
	}
	return false;
}

bool isVFMSUBADDPD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADDPD4Ymr:
	case VFMSUBADDPD4Yrm:
	case VFMSUBADDPD4Yrr:
	case VFMSUBADDPD4Yrr_REV:
	case VFMSUBADDPD4mr:
	case VFMSUBADDPD4rm:
	case VFMSUBADDPD4rr:
	case VFMSUBADDPD4rr_REV:
		return true;
	}
	return false;
}

bool isPFCMPGT(unsigned Opcode) {
	switch (Opcode) {
	case PFCMPGTrm:
	case PFCMPGTrr:
		return true;
	}
	return false;
}

bool isVEXTRACTI128(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTI128mr:
	case VEXTRACTI128rr:
		return true;
	}
	return false;
}

bool isVGF2P8AFFINEQB(unsigned Opcode) {
	switch (Opcode) {
	case VGF2P8AFFINEQBYrmi:
	case VGF2P8AFFINEQBYrri:
	case VGF2P8AFFINEQBZ128rmbi:
	case VGF2P8AFFINEQBZ128rmbik:
	case VGF2P8AFFINEQBZ128rmbikz:
	case VGF2P8AFFINEQBZ128rmi:
	case VGF2P8AFFINEQBZ128rmik:
	case VGF2P8AFFINEQBZ128rmikz:
	case VGF2P8AFFINEQBZ128rri:
	case VGF2P8AFFINEQBZ128rrik:
	case VGF2P8AFFINEQBZ128rrikz:
	case VGF2P8AFFINEQBZ256rmbi:
	case VGF2P8AFFINEQBZ256rmbik:
	case VGF2P8AFFINEQBZ256rmbikz:
	case VGF2P8AFFINEQBZ256rmi:
	case VGF2P8AFFINEQBZ256rmik:
	case VGF2P8AFFINEQBZ256rmikz:
	case VGF2P8AFFINEQBZ256rri:
	case VGF2P8AFFINEQBZ256rrik:
	case VGF2P8AFFINEQBZ256rrikz:
	case VGF2P8AFFINEQBZrmbi:
	case VGF2P8AFFINEQBZrmbik:
	case VGF2P8AFFINEQBZrmbikz:
	case VGF2P8AFFINEQBZrmi:
	case VGF2P8AFFINEQBZrmik:
	case VGF2P8AFFINEQBZrmikz:
	case VGF2P8AFFINEQBZrri:
	case VGF2P8AFFINEQBZrrik:
	case VGF2P8AFFINEQBZrrikz:
	case VGF2P8AFFINEQBrmi:
	case VGF2P8AFFINEQBrri:
		return true;
	}
	return false;
}

bool isPSLLD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSLLDri:
	case MMX_PSLLDrm:
	case MMX_PSLLDrr:
	case PSLLDri:
	case PSLLDrm:
	case PSLLDrr:
		return true;
	}
	return false;
}

bool isFSUBP(unsigned Opcode) {
	return Opcode == SUB_FPrST0;
}

bool isFSUBR(unsigned Opcode) {
	switch (Opcode) {
	case SUBR_F32m:
	case SUBR_F64m:
	case SUBR_FST0r:
	case SUBR_FrST0:
		return true;
	}
	return false;
}

bool isVBROADCASTF64X2(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTF64X2Z128rm:
	case VBROADCASTF64X2Z128rmk:
	case VBROADCASTF64X2Z128rmkz:
	case VBROADCASTF64X2rm:
	case VBROADCASTF64X2rmk:
	case VBROADCASTF64X2rmkz:
		return true;
	}
	return false;
}

bool isFCHS(unsigned Opcode) {
	return Opcode == CHS_F;
}

bool isCMPXCHG8B(unsigned Opcode) {
	return Opcode == CMPXCHG8B;
}

bool isVBROADCASTF64X4(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTF64X4rm:
	case VBROADCASTF64X4rmk:
	case VBROADCASTF64X4rmkz:
		return true;
	}
	return false;
}

bool isVINSERTI32X4(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTI32x4Z256rm:
	case VINSERTI32x4Z256rmk:
	case VINSERTI32x4Z256rmkz:
	case VINSERTI32x4Z256rr:
	case VINSERTI32x4Z256rrk:
	case VINSERTI32x4Z256rrkz:
	case VINSERTI32x4Zrm:
	case VINSERTI32x4Zrmk:
	case VINSERTI32x4Zrmkz:
	case VINSERTI32x4Zrr:
	case VINSERTI32x4Zrrk:
	case VINSERTI32x4Zrrkz:
		return true;
	}
	return false;
}

bool isVFMSUBADDPS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBADDPS4Ymr:
	case VFMSUBADDPS4Yrm:
	case VFMSUBADDPS4Yrr:
	case VFMSUBADDPS4Yrr_REV:
	case VFMSUBADDPS4mr:
	case VFMSUBADDPS4rm:
	case VFMSUBADDPS4rr:
	case VFMSUBADDPS4rr_REV:
		return true;
	}
	return false;
}

bool isVBROADCASTF128(unsigned Opcode) {
	return Opcode == VBROADCASTF128;
}

bool isVPERMIL2PD(unsigned Opcode) {
	switch (Opcode) {
	case VPERMIL2PDYmr:
	case VPERMIL2PDYrm:
	case VPERMIL2PDYrr:
	case VPERMIL2PDYrr_REV:
	case VPERMIL2PDmr:
	case VPERMIL2PDrm:
	case VPERMIL2PDrr:
	case VPERMIL2PDrr_REV:
		return true;
	}
	return false;
}

bool isPSLLQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSLLQri:
	case MMX_PSLLQrm:
	case MMX_PSLLQrr:
	case PSLLQri:
	case PSLLQrm:
	case PSLLQrr:
		return true;
	}
	return false;
}

bool isVINSERTI32X8(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTI32x8Zrm:
	case VINSERTI32x8Zrmk:
	case VINSERTI32x8Zrmkz:
	case VINSERTI32x8Zrr:
	case VINSERTI32x8Zrrk:
	case VINSERTI32x8Zrrkz:
		return true;
	}
	return false;
}

bool isLLDT(unsigned Opcode) {
	switch (Opcode) {
	case LLDT16m:
	case LLDT16r:
		return true;
	}
	return false;
}

bool isMOVMSKPD(unsigned Opcode) {
	return Opcode == MOVMSKPDrr;
}

bool isPSLLW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSLLWri:
	case MMX_PSLLWrm:
	case MMX_PSLLWrr:
	case PSLLWri:
	case PSLLWrm:
	case PSLLWrr:
		return true;
	}
	return false;
}

bool isVFCMADDCSH(unsigned Opcode) {
	switch (Opcode) {
	case VFCMADDCSHZm:
	case VFCMADDCSHZmk:
	case VFCMADDCSHZmkz:
	case VFCMADDCSHZr:
	case VFCMADDCSHZrb:
	case VFCMADDCSHZrbk:
	case VFCMADDCSHZrbkz:
	case VFCMADDCSHZrk:
	case VFCMADDCSHZrkz:
		return true;
	}
	return false;
}

bool isVPERMIL2PS(unsigned Opcode) {
	switch (Opcode) {
	case VPERMIL2PSYmr:
	case VPERMIL2PSYrm:
	case VPERMIL2PSYrr:
	case VPERMIL2PSYrr_REV:
	case VPERMIL2PSmr:
	case VPERMIL2PSrm:
	case VPERMIL2PSrr:
	case VPERMIL2PSrr_REV:
		return true;
	}
	return false;
}

bool isPF2ID(unsigned Opcode) {
	switch (Opcode) {
	case PF2IDrm:
	case PF2IDrr:
		return true;
	}
	return false;
}

bool isVPUNPCKHQDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKHQDQYrm:
	case VPUNPCKHQDQYrr:
	case VPUNPCKHQDQZ128rm:
	case VPUNPCKHQDQZ128rmb:
	case VPUNPCKHQDQZ128rmbk:
	case VPUNPCKHQDQZ128rmbkz:
	case VPUNPCKHQDQZ128rmk:
	case VPUNPCKHQDQZ128rmkz:
	case VPUNPCKHQDQZ128rr:
	case VPUNPCKHQDQZ128rrk:
	case VPUNPCKHQDQZ128rrkz:
	case VPUNPCKHQDQZ256rm:
	case VPUNPCKHQDQZ256rmb:
	case VPUNPCKHQDQZ256rmbk:
	case VPUNPCKHQDQZ256rmbkz:
	case VPUNPCKHQDQZ256rmk:
	case VPUNPCKHQDQZ256rmkz:
	case VPUNPCKHQDQZ256rr:
	case VPUNPCKHQDQZ256rrk:
	case VPUNPCKHQDQZ256rrkz:
	case VPUNPCKHQDQZrm:
	case VPUNPCKHQDQZrmb:
	case VPUNPCKHQDQZrmbk:
	case VPUNPCKHQDQZrmbkz:
	case VPUNPCKHQDQZrmk:
	case VPUNPCKHQDQZrmkz:
	case VPUNPCKHQDQZrr:
	case VPUNPCKHQDQZrrk:
	case VPUNPCKHQDQZrrkz:
	case VPUNPCKHQDQrm:
	case VPUNPCKHQDQrr:
		return true;
	}
	return false;
}

bool isMOVMSKPS(unsigned Opcode) {
	return Opcode == MOVMSKPSrr;
}

bool isMULSD(unsigned Opcode) {
	switch (Opcode) {
	case MULSDrm_Int:
	case MULSDrr_Int:
		return true;
	}
	return false;
}

bool isPF2IW(unsigned Opcode) {
	switch (Opcode) {
	case PF2IWrm:
	case PF2IWrr:
		return true;
	}
	return false;
}

bool isVBLENDPD(unsigned Opcode) {
	switch (Opcode) {
	case VBLENDPDYrmi:
	case VBLENDPDYrri:
	case VBLENDPDrmi:
	case VBLENDPDrri:
		return true;
	}
	return false;
}

bool isCLAC(unsigned Opcode) {
	return Opcode == CLAC;
}

bool isMULSS(unsigned Opcode) {
	switch (Opcode) {
	case MULSSrm_Int:
	case MULSSrr_Int:
		return true;
	}
	return false;
}

bool isORPD(unsigned Opcode) {
	switch (Opcode) {
	case ORPDrm:
	case ORPDrr:
		return true;
	}
	return false;
}

bool isCDQE(unsigned Opcode) {
	return Opcode == CDQE;
}

bool isVBLENDPS(unsigned Opcode) {
	switch (Opcode) {
	case VBLENDPSYrmi:
	case VBLENDPSYrri:
	case VBLENDPSrmi:
	case VBLENDPSrri:
		return true;
	}
	return false;
}

bool isTILESTORED(unsigned Opcode) {
	return Opcode == TILESTORED;
}

bool isORPS(unsigned Opcode) {
	switch (Opcode) {
	case ORPSrm:
	case ORPSrr:
		return true;
	}
	return false;
}

bool isVPINSRB(unsigned Opcode) {
	switch (Opcode) {
	case VPINSRBZrm:
	case VPINSRBZrr:
	case VPINSRBrm:
	case VPINSRBrr:
		return true;
	}
	return false;
}

bool isVPINSRD(unsigned Opcode) {
	switch (Opcode) {
	case VPINSRDZrm:
	case VPINSRDZrr:
	case VPINSRDrm:
	case VPINSRDrr:
		return true;
	}
	return false;
}

bool isBZHI(unsigned Opcode) {
	switch (Opcode) {
	case BZHI32rm:
	case BZHI32rr:
	case BZHI64rm:
	case BZHI64rr:
		return true;
	}
	return false;
}

bool isPUNPCKHBW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PUNPCKHBWrm:
	case MMX_PUNPCKHBWrr:
	case PUNPCKHBWrm:
	case PUNPCKHBWrr:
		return true;
	}
	return false;
}

bool isVCVTPD2UDQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPD2UDQZ128rm:
	case VCVTPD2UDQZ128rmb:
	case VCVTPD2UDQZ128rmbk:
	case VCVTPD2UDQZ128rmbkz:
	case VCVTPD2UDQZ128rmk:
	case VCVTPD2UDQZ128rmkz:
	case VCVTPD2UDQZ128rr:
	case VCVTPD2UDQZ128rrk:
	case VCVTPD2UDQZ128rrkz:
	case VCVTPD2UDQZ256rm:
	case VCVTPD2UDQZ256rmb:
	case VCVTPD2UDQZ256rmbk:
	case VCVTPD2UDQZ256rmbkz:
	case VCVTPD2UDQZ256rmk:
	case VCVTPD2UDQZ256rmkz:
	case VCVTPD2UDQZ256rr:
	case VCVTPD2UDQZ256rrk:
	case VCVTPD2UDQZ256rrkz:
	case VCVTPD2UDQZrm:
	case VCVTPD2UDQZrmb:
	case VCVTPD2UDQZrmbk:
	case VCVTPD2UDQZrmbkz:
	case VCVTPD2UDQZrmk:
	case VCVTPD2UDQZrmkz:
	case VCVTPD2UDQZrr:
	case VCVTPD2UDQZrrb:
	case VCVTPD2UDQZrrbk:
	case VCVTPD2UDQZrrbkz:
	case VCVTPD2UDQZrrk:
	case VCVTPD2UDQZrrkz:
		return true;
	}
	return false;
}

bool isVPINSRQ(unsigned Opcode) {
	switch (Opcode) {
	case VPINSRQZrm:
	case VPINSRQZrr:
	case VPINSRQrm:
	case VPINSRQrr:
		return true;
	}
	return false;
}

bool isVPINSRW(unsigned Opcode) {
	switch (Opcode) {
	case VPINSRWZrm:
	case VPINSRWZrr:
	case VPINSRWrm:
	case VPINSRWrr:
		return true;
	}
	return false;
}

bool isVPMACSSDQH(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSSDQHrm:
	case VPMACSSDQHrr:
		return true;
	}
	return false;
}

bool isGETSEC(unsigned Opcode) {
	return Opcode == GETSEC;
}

bool isCVTSS2SD(unsigned Opcode) {
	switch (Opcode) {
	case CVTSS2SDrm_Int:
	case CVTSS2SDrr_Int:
		return true;
	}
	return false;
}

bool isVPMACSSDQL(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSSDQLrm:
	case VPMACSSDQLrr:
		return true;
	}
	return false;
}

bool isANDN(unsigned Opcode) {
	switch (Opcode) {
	case ANDN32rm:
	case ANDN32rr:
	case ANDN64rm:
	case ANDN64rr:
		return true;
	}
	return false;
}

bool isCVTSS2SI(unsigned Opcode) {
	switch (Opcode) {
	case CVTSS2SI64rm_Int:
	case CVTSS2SI64rr_Int:
	case CVTSS2SIrm_Int:
	case CVTSS2SIrr_Int:
		return true;
	}
	return false;
}

bool isAESDEC(unsigned Opcode) {
	switch (Opcode) {
	case AESDECrm:
	case AESDECrr:
		return true;
	}
	return false;
}

bool isMOVSB(unsigned Opcode) {
	return Opcode == MOVSB;
}

bool isMOVSD(unsigned Opcode) {
	switch (Opcode) {
	case MOVSDmr:
	case MOVSDrm:
	case MOVSDrr:
	case MOVSDrr_REV:
	case MOVSL:
		return true;
	}
	return false;
}

bool isVFNMSUB213PD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB213PDYm:
	case VFNMSUB213PDYr:
	case VFNMSUB213PDZ128m:
	case VFNMSUB213PDZ128mb:
	case VFNMSUB213PDZ128mbk:
	case VFNMSUB213PDZ128mbkz:
	case VFNMSUB213PDZ128mk:
	case VFNMSUB213PDZ128mkz:
	case VFNMSUB213PDZ128r:
	case VFNMSUB213PDZ128rk:
	case VFNMSUB213PDZ128rkz:
	case VFNMSUB213PDZ256m:
	case VFNMSUB213PDZ256mb:
	case VFNMSUB213PDZ256mbk:
	case VFNMSUB213PDZ256mbkz:
	case VFNMSUB213PDZ256mk:
	case VFNMSUB213PDZ256mkz:
	case VFNMSUB213PDZ256r:
	case VFNMSUB213PDZ256rk:
	case VFNMSUB213PDZ256rkz:
	case VFNMSUB213PDZm:
	case VFNMSUB213PDZmb:
	case VFNMSUB213PDZmbk:
	case VFNMSUB213PDZmbkz:
	case VFNMSUB213PDZmk:
	case VFNMSUB213PDZmkz:
	case VFNMSUB213PDZr:
	case VFNMSUB213PDZrb:
	case VFNMSUB213PDZrbk:
	case VFNMSUB213PDZrbkz:
	case VFNMSUB213PDZrk:
	case VFNMSUB213PDZrkz:
	case VFNMSUB213PDm:
	case VFNMSUB213PDr:
		return true;
	}
	return false;
}

bool isVPMOVW2M(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVW2MZ128rr:
	case VPMOVW2MZ256rr:
	case VPMOVW2MZrr:
		return true;
	}
	return false;
}

bool isVFNMSUB213PH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB213PHZ128m:
	case VFNMSUB213PHZ128mb:
	case VFNMSUB213PHZ128mbk:
	case VFNMSUB213PHZ128mbkz:
	case VFNMSUB213PHZ128mk:
	case VFNMSUB213PHZ128mkz:
	case VFNMSUB213PHZ128r:
	case VFNMSUB213PHZ128rk:
	case VFNMSUB213PHZ128rkz:
	case VFNMSUB213PHZ256m:
	case VFNMSUB213PHZ256mb:
	case VFNMSUB213PHZ256mbk:
	case VFNMSUB213PHZ256mbkz:
	case VFNMSUB213PHZ256mk:
	case VFNMSUB213PHZ256mkz:
	case VFNMSUB213PHZ256r:
	case VFNMSUB213PHZ256rk:
	case VFNMSUB213PHZ256rkz:
	case VFNMSUB213PHZm:
	case VFNMSUB213PHZmb:
	case VFNMSUB213PHZmbk:
	case VFNMSUB213PHZmbkz:
	case VFNMSUB213PHZmk:
	case VFNMSUB213PHZmkz:
	case VFNMSUB213PHZr:
	case VFNMSUB213PHZrb:
	case VFNMSUB213PHZrbk:
	case VFNMSUB213PHZrbkz:
	case VFNMSUB213PHZrk:
	case VFNMSUB213PHZrkz:
		return true;
	}
	return false;
}

bool isVPACKSSWB(unsigned Opcode) {
	switch (Opcode) {
	case VPACKSSWBYrm:
	case VPACKSSWBYrr:
	case VPACKSSWBZ128rm:
	case VPACKSSWBZ128rmk:
	case VPACKSSWBZ128rmkz:
	case VPACKSSWBZ128rr:
	case VPACKSSWBZ128rrk:
	case VPACKSSWBZ128rrkz:
	case VPACKSSWBZ256rm:
	case VPACKSSWBZ256rmk:
	case VPACKSSWBZ256rmkz:
	case VPACKSSWBZ256rr:
	case VPACKSSWBZ256rrk:
	case VPACKSSWBZ256rrkz:
	case VPACKSSWBZrm:
	case VPACKSSWBZrmk:
	case VPACKSSWBZrmkz:
	case VPACKSSWBZrr:
	case VPACKSSWBZrrk:
	case VPACKSSWBZrrkz:
	case VPACKSSWBrm:
	case VPACKSSWBrr:
		return true;
	}
	return false;
}

bool isMOVSQ(unsigned Opcode) {
	return Opcode == MOVSQ;
}

bool isMOVSS(unsigned Opcode) {
	switch (Opcode) {
	case MOVSSmr:
	case MOVSSrm:
	case MOVSSrr:
	case MOVSSrr_REV:
		return true;
	}
	return false;
}

bool isVPMULHRSW(unsigned Opcode) {
	switch (Opcode) {
	case VPMULHRSWYrm:
	case VPMULHRSWYrr:
	case VPMULHRSWZ128rm:
	case VPMULHRSWZ128rmk:
	case VPMULHRSWZ128rmkz:
	case VPMULHRSWZ128rr:
	case VPMULHRSWZ128rrk:
	case VPMULHRSWZ128rrkz:
	case VPMULHRSWZ256rm:
	case VPMULHRSWZ256rmk:
	case VPMULHRSWZ256rmkz:
	case VPMULHRSWZ256rr:
	case VPMULHRSWZ256rrk:
	case VPMULHRSWZ256rrkz:
	case VPMULHRSWZrm:
	case VPMULHRSWZrmk:
	case VPMULHRSWZrmkz:
	case VPMULHRSWZrr:
	case VPMULHRSWZrrk:
	case VPMULHRSWZrrkz:
	case VPMULHRSWrm:
	case VPMULHRSWrr:
		return true;
	}
	return false;
}

bool isVFNMSUB213PS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB213PSYm:
	case VFNMSUB213PSYr:
	case VFNMSUB213PSZ128m:
	case VFNMSUB213PSZ128mb:
	case VFNMSUB213PSZ128mbk:
	case VFNMSUB213PSZ128mbkz:
	case VFNMSUB213PSZ128mk:
	case VFNMSUB213PSZ128mkz:
	case VFNMSUB213PSZ128r:
	case VFNMSUB213PSZ128rk:
	case VFNMSUB213PSZ128rkz:
	case VFNMSUB213PSZ256m:
	case VFNMSUB213PSZ256mb:
	case VFNMSUB213PSZ256mbk:
	case VFNMSUB213PSZ256mbkz:
	case VFNMSUB213PSZ256mk:
	case VFNMSUB213PSZ256mkz:
	case VFNMSUB213PSZ256r:
	case VFNMSUB213PSZ256rk:
	case VFNMSUB213PSZ256rkz:
	case VFNMSUB213PSZm:
	case VFNMSUB213PSZmb:
	case VFNMSUB213PSZmbk:
	case VFNMSUB213PSZmbkz:
	case VFNMSUB213PSZmk:
	case VFNMSUB213PSZmkz:
	case VFNMSUB213PSZr:
	case VFNMSUB213PSZrb:
	case VFNMSUB213PSZrbk:
	case VFNMSUB213PSZrbkz:
	case VFNMSUB213PSZrk:
	case VFNMSUB213PSZrkz:
	case VFNMSUB213PSm:
	case VFNMSUB213PSr:
		return true;
	}
	return false;
}

bool isCMOVCC(unsigned Opcode) {
	switch (Opcode) {
	case CMOV16rm:
	case CMOV16rr:
	case CMOV32rm:
	case CMOV32rr:
	case CMOV64rm:
	case CMOV64rr:
		return true;
	}
	return false;
}

bool isMOVSW(unsigned Opcode) {
	return Opcode == MOVSW;
}

bool isMOVSX(unsigned Opcode) {
	switch (Opcode) {
	case MOVSX16rm16:
	case MOVSX16rm8:
	case MOVSX16rr16:
	case MOVSX16rr8:
	case MOVSX32rm16:
	case MOVSX32rm8:
	case MOVSX32rr16:
	case MOVSX32rr8:
	case MOVSX64rm16:
	case MOVSX64rm8:
	case MOVSX64rr16:
	case MOVSX64rr8:
		return true;
	}
	return false;
}

bool isVPCOMUD(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMUDmi:
	case VPCOMUDri:
		return true;
	}
	return false;
}

bool isVPCOMUB(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMUBmi:
	case VPCOMUBri:
		return true;
	}
	return false;
}

bool isVPDPBSUDS(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBSUDSYrm:
	case VPDPBSUDSYrr:
	case VPDPBSUDSrm:
	case VPDPBSUDSrr:
		return true;
	}
	return false;
}

bool isFLDLN2(unsigned Opcode) {
	return Opcode == FLDLN2;
}

bool isPACKUSDW(unsigned Opcode) {
	switch (Opcode) {
	case PACKUSDWrm:
	case PACKUSDWrr:
		return true;
	}
	return false;
}

bool isVPCOMUQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMUQmi:
	case VPCOMUQri:
		return true;
	}
	return false;
}

bool isMONTMUL(unsigned Opcode) {
	return Opcode == MONTMUL;
}

bool isPUNPCKHDQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PUNPCKHDQrm:
	case MMX_PUNPCKHDQrr:
	case PUNPCKHDQrm:
	case PUNPCKHDQrr:
		return true;
	}
	return false;
}

bool isVPCOMUW(unsigned Opcode) {
	switch (Opcode) {
	case VPCOMUWmi:
	case VPCOMUWri:
		return true;
	}
	return false;
}

bool isPMULDQ(unsigned Opcode) {
	switch (Opcode) {
	case PMULDQrm:
	case PMULDQrr:
		return true;
	}
	return false;
}

bool isT1MSKC(unsigned Opcode) {
	switch (Opcode) {
	case T1MSKC32rm:
	case T1MSKC32rr:
	case T1MSKC64rm:
	case T1MSKC64rr:
		return true;
	}
	return false;
}

bool isIN(unsigned Opcode) {
	switch (Opcode) {
	case IN16ri:
	case IN16rr:
	case IN32ri:
	case IN32rr:
	case IN8ri:
	case IN8rr:
		return true;
	}
	return false;
}

bool isVPHADDBD(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDBDrm:
	case VPHADDBDrr:
		return true;
	}
	return false;
}

bool isSAR(unsigned Opcode) {
	switch (Opcode) {
	case SAR16m1:
	case SAR16mCL:
	case SAR16mi:
	case SAR16r1:
	case SAR16rCL:
	case SAR16ri:
	case SAR32m1:
	case SAR32mCL:
	case SAR32mi:
	case SAR32r1:
	case SAR32rCL:
	case SAR32ri:
	case SAR64m1:
	case SAR64mCL:
	case SAR64mi:
	case SAR64r1:
	case SAR64rCL:
	case SAR64ri:
	case SAR8m1:
	case SAR8mCL:
	case SAR8mi:
	case SAR8r1:
	case SAR8rCL:
	case SAR8ri:
		return true;
	}
	return false;
}

bool isVPHADDBQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDBQrm:
	case VPHADDBQrr:
		return true;
	}
	return false;
}

bool isVFNMSUB231PD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB231PDYm:
	case VFNMSUB231PDYr:
	case VFNMSUB231PDZ128m:
	case VFNMSUB231PDZ128mb:
	case VFNMSUB231PDZ128mbk:
	case VFNMSUB231PDZ128mbkz:
	case VFNMSUB231PDZ128mk:
	case VFNMSUB231PDZ128mkz:
	case VFNMSUB231PDZ128r:
	case VFNMSUB231PDZ128rk:
	case VFNMSUB231PDZ128rkz:
	case VFNMSUB231PDZ256m:
	case VFNMSUB231PDZ256mb:
	case VFNMSUB231PDZ256mbk:
	case VFNMSUB231PDZ256mbkz:
	case VFNMSUB231PDZ256mk:
	case VFNMSUB231PDZ256mkz:
	case VFNMSUB231PDZ256r:
	case VFNMSUB231PDZ256rk:
	case VFNMSUB231PDZ256rkz:
	case VFNMSUB231PDZm:
	case VFNMSUB231PDZmb:
	case VFNMSUB231PDZmbk:
	case VFNMSUB231PDZmbkz:
	case VFNMSUB231PDZmk:
	case VFNMSUB231PDZmkz:
	case VFNMSUB231PDZr:
	case VFNMSUB231PDZrb:
	case VFNMSUB231PDZrbk:
	case VFNMSUB231PDZrbkz:
	case VFNMSUB231PDZrk:
	case VFNMSUB231PDZrkz:
	case VFNMSUB231PDm:
	case VFNMSUB231PDr:
		return true;
	}
	return false;
}

bool isVPSHLDVD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDVDZ128m:
	case VPSHLDVDZ128mb:
	case VPSHLDVDZ128mbk:
	case VPSHLDVDZ128mbkz:
	case VPSHLDVDZ128mk:
	case VPSHLDVDZ128mkz:
	case VPSHLDVDZ128r:
	case VPSHLDVDZ128rk:
	case VPSHLDVDZ128rkz:
	case VPSHLDVDZ256m:
	case VPSHLDVDZ256mb:
	case VPSHLDVDZ256mbk:
	case VPSHLDVDZ256mbkz:
	case VPSHLDVDZ256mk:
	case VPSHLDVDZ256mkz:
	case VPSHLDVDZ256r:
	case VPSHLDVDZ256rk:
	case VPSHLDVDZ256rkz:
	case VPSHLDVDZm:
	case VPSHLDVDZmb:
	case VPSHLDVDZmbk:
	case VPSHLDVDZmbkz:
	case VPSHLDVDZmk:
	case VPSHLDVDZmkz:
	case VPSHLDVDZr:
	case VPSHLDVDZrk:
	case VPSHLDVDZrkz:
		return true;
	}
	return false;
}

bool isFSCALE(unsigned Opcode) {
	return Opcode == FSCALE;
}

bool isVFNMSUB231PH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB231PHZ128m:
	case VFNMSUB231PHZ128mb:
	case VFNMSUB231PHZ128mbk:
	case VFNMSUB231PHZ128mbkz:
	case VFNMSUB231PHZ128mk:
	case VFNMSUB231PHZ128mkz:
	case VFNMSUB231PHZ128r:
	case VFNMSUB231PHZ128rk:
	case VFNMSUB231PHZ128rkz:
	case VFNMSUB231PHZ256m:
	case VFNMSUB231PHZ256mb:
	case VFNMSUB231PHZ256mbk:
	case VFNMSUB231PHZ256mbkz:
	case VFNMSUB231PHZ256mk:
	case VFNMSUB231PHZ256mkz:
	case VFNMSUB231PHZ256r:
	case VFNMSUB231PHZ256rk:
	case VFNMSUB231PHZ256rkz:
	case VFNMSUB231PHZm:
	case VFNMSUB231PHZmb:
	case VFNMSUB231PHZmbk:
	case VFNMSUB231PHZmbkz:
	case VFNMSUB231PHZmk:
	case VFNMSUB231PHZmkz:
	case VFNMSUB231PHZr:
	case VFNMSUB231PHZrb:
	case VFNMSUB231PHZrbk:
	case VFNMSUB231PHZrbkz:
	case VFNMSUB231PHZrk:
	case VFNMSUB231PHZrkz:
		return true;
	}
	return false;
}

bool isVPHADDBW(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDBWrm:
	case VPHADDBWrr:
		return true;
	}
	return false;
}

bool isSBB(unsigned Opcode) {
	switch (Opcode) {
	case SBB16i16:
	case SBB16mi:
	case SBB16mi8:
	case SBB16mr:
	case SBB16ri:
	case SBB16ri8:
	case SBB16rm:
	case SBB16rr:
	case SBB16rr_REV:
	case SBB32i32:
	case SBB32mi:
	case SBB32mi8:
	case SBB32mr:
	case SBB32ri:
	case SBB32ri8:
	case SBB32rm:
	case SBB32rr:
	case SBB32rr_REV:
	case SBB64i32:
	case SBB64mi32:
	case SBB64mi8:
	case SBB64mr:
	case SBB64ri32:
	case SBB64ri8:
	case SBB64rm:
	case SBB64rr:
	case SBB64rr_REV:
	case SBB8i8:
	case SBB8mi:
	case SBB8mi8:
	case SBB8mr:
	case SBB8ri:
	case SBB8ri8:
	case SBB8rm:
	case SBB8rr:
	case SBB8rr_REV:
		return true;
	}
	return false;
}

bool isVPSHLDVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDVQZ128m:
	case VPSHLDVQZ128mb:
	case VPSHLDVQZ128mbk:
	case VPSHLDVQZ128mbkz:
	case VPSHLDVQZ128mk:
	case VPSHLDVQZ128mkz:
	case VPSHLDVQZ128r:
	case VPSHLDVQZ128rk:
	case VPSHLDVQZ128rkz:
	case VPSHLDVQZ256m:
	case VPSHLDVQZ256mb:
	case VPSHLDVQZ256mbk:
	case VPSHLDVQZ256mbkz:
	case VPSHLDVQZ256mk:
	case VPSHLDVQZ256mkz:
	case VPSHLDVQZ256r:
	case VPSHLDVQZ256rk:
	case VPSHLDVQZ256rkz:
	case VPSHLDVQZm:
	case VPSHLDVQZmb:
	case VPSHLDVQZmbk:
	case VPSHLDVQZmbkz:
	case VPSHLDVQZmk:
	case VPSHLDVQZmkz:
	case VPSHLDVQZr:
	case VPSHLDVQZrk:
	case VPSHLDVQZrkz:
		return true;
	}
	return false;
}

bool isVFNMSUB231PS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB231PSYm:
	case VFNMSUB231PSYr:
	case VFNMSUB231PSZ128m:
	case VFNMSUB231PSZ128mb:
	case VFNMSUB231PSZ128mbk:
	case VFNMSUB231PSZ128mbkz:
	case VFNMSUB231PSZ128mk:
	case VFNMSUB231PSZ128mkz:
	case VFNMSUB231PSZ128r:
	case VFNMSUB231PSZ128rk:
	case VFNMSUB231PSZ128rkz:
	case VFNMSUB231PSZ256m:
	case VFNMSUB231PSZ256mb:
	case VFNMSUB231PSZ256mbk:
	case VFNMSUB231PSZ256mbkz:
	case VFNMSUB231PSZ256mk:
	case VFNMSUB231PSZ256mkz:
	case VFNMSUB231PSZ256r:
	case VFNMSUB231PSZ256rk:
	case VFNMSUB231PSZ256rkz:
	case VFNMSUB231PSZm:
	case VFNMSUB231PSZmb:
	case VFNMSUB231PSZmbk:
	case VFNMSUB231PSZmbkz:
	case VFNMSUB231PSZmk:
	case VFNMSUB231PSZmkz:
	case VFNMSUB231PSZr:
	case VFNMSUB231PSZrb:
	case VFNMSUB231PSZrbk:
	case VFNMSUB231PSZrbkz:
	case VFNMSUB231PSZrk:
	case VFNMSUB231PSZrkz:
	case VFNMSUB231PSm:
	case VFNMSUB231PSr:
		return true;
	}
	return false;
}

bool isVPDPBUSDS(unsigned Opcode) {
	switch (Opcode) {
	case VPDPBUSDSYrm:
	case VPDPBUSDSYrr:
	case VPDPBUSDSZ128m:
	case VPDPBUSDSZ128mb:
	case VPDPBUSDSZ128mbk:
	case VPDPBUSDSZ128mbkz:
	case VPDPBUSDSZ128mk:
	case VPDPBUSDSZ128mkz:
	case VPDPBUSDSZ128r:
	case VPDPBUSDSZ128rk:
	case VPDPBUSDSZ128rkz:
	case VPDPBUSDSZ256m:
	case VPDPBUSDSZ256mb:
	case VPDPBUSDSZ256mbk:
	case VPDPBUSDSZ256mbkz:
	case VPDPBUSDSZ256mk:
	case VPDPBUSDSZ256mkz:
	case VPDPBUSDSZ256r:
	case VPDPBUSDSZ256rk:
	case VPDPBUSDSZ256rkz:
	case VPDPBUSDSZm:
	case VPDPBUSDSZmb:
	case VPDPBUSDSZmbk:
	case VPDPBUSDSZmbkz:
	case VPDPBUSDSZmk:
	case VPDPBUSDSZmkz:
	case VPDPBUSDSZr:
	case VPDPBUSDSZrk:
	case VPDPBUSDSZrkz:
	case VPDPBUSDSrm:
	case VPDPBUSDSrr:
		return true;
	}
	return false;
}

bool isFCOMPI(unsigned Opcode) {
	return Opcode == COM_FIPr;
}

bool isRSQRTPS(unsigned Opcode) {
	switch (Opcode) {
	case RSQRTPSm:
	case RSQRTPSr:
		return true;
	}
	return false;
}

bool isVSHUFPD(unsigned Opcode) {
	switch (Opcode) {
	case VSHUFPDYrmi:
	case VSHUFPDYrri:
	case VSHUFPDZ128rmbi:
	case VSHUFPDZ128rmbik:
	case VSHUFPDZ128rmbikz:
	case VSHUFPDZ128rmi:
	case VSHUFPDZ128rmik:
	case VSHUFPDZ128rmikz:
	case VSHUFPDZ128rri:
	case VSHUFPDZ128rrik:
	case VSHUFPDZ128rrikz:
	case VSHUFPDZ256rmbi:
	case VSHUFPDZ256rmbik:
	case VSHUFPDZ256rmbikz:
	case VSHUFPDZ256rmi:
	case VSHUFPDZ256rmik:
	case VSHUFPDZ256rmikz:
	case VSHUFPDZ256rri:
	case VSHUFPDZ256rrik:
	case VSHUFPDZ256rrikz:
	case VSHUFPDZrmbi:
	case VSHUFPDZrmbik:
	case VSHUFPDZrmbikz:
	case VSHUFPDZrmi:
	case VSHUFPDZrmik:
	case VSHUFPDZrmikz:
	case VSHUFPDZrri:
	case VSHUFPDZrrik:
	case VSHUFPDZrrikz:
	case VSHUFPDrmi:
	case VSHUFPDrri:
		return true;
	}
	return false;
}

bool isVPSHLDVW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDVWZ128m:
	case VPSHLDVWZ128mk:
	case VPSHLDVWZ128mkz:
	case VPSHLDVWZ128r:
	case VPSHLDVWZ128rk:
	case VPSHLDVWZ128rkz:
	case VPSHLDVWZ256m:
	case VPSHLDVWZ256mk:
	case VPSHLDVWZ256mkz:
	case VPSHLDVWZ256r:
	case VPSHLDVWZ256rk:
	case VPSHLDVWZ256rkz:
	case VPSHLDVWZm:
	case VPSHLDVWZmk:
	case VPSHLDVWZmkz:
	case VPSHLDVWZr:
	case VPSHLDVWZrk:
	case VPSHLDVWZrkz:
		return true;
	}
	return false;
}

bool isVPADDSB(unsigned Opcode) {
	switch (Opcode) {
	case VPADDSBYrm:
	case VPADDSBYrr:
	case VPADDSBZ128rm:
	case VPADDSBZ128rmk:
	case VPADDSBZ128rmkz:
	case VPADDSBZ128rr:
	case VPADDSBZ128rrk:
	case VPADDSBZ128rrkz:
	case VPADDSBZ256rm:
	case VPADDSBZ256rmk:
	case VPADDSBZ256rmkz:
	case VPADDSBZ256rr:
	case VPADDSBZ256rrk:
	case VPADDSBZ256rrkz:
	case VPADDSBZrm:
	case VPADDSBZrmk:
	case VPADDSBZrmkz:
	case VPADDSBZrr:
	case VPADDSBZrrk:
	case VPADDSBZrrkz:
	case VPADDSBrm:
	case VPADDSBrr:
		return true;
	}
	return false;
}

bool isFCOMPP(unsigned Opcode) {
	return Opcode == FCOMPP;
}

bool isDAA(unsigned Opcode) {
	return Opcode == DAA;
}

bool isVFNMSUB213SD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB213SDZm_Int:
	case VFNMSUB213SDZm_Intk:
	case VFNMSUB213SDZm_Intkz:
	case VFNMSUB213SDZr_Int:
	case VFNMSUB213SDZr_Intk:
	case VFNMSUB213SDZr_Intkz:
	case VFNMSUB213SDZrb_Int:
	case VFNMSUB213SDZrb_Intk:
	case VFNMSUB213SDZrb_Intkz:
	case VFNMSUB213SDm_Int:
	case VFNMSUB213SDr_Int:
		return true;
	}
	return false;
}

bool isVSHUFPS(unsigned Opcode) {
	switch (Opcode) {
	case VSHUFPSYrmi:
	case VSHUFPSYrri:
	case VSHUFPSZ128rmbi:
	case VSHUFPSZ128rmbik:
	case VSHUFPSZ128rmbikz:
	case VSHUFPSZ128rmi:
	case VSHUFPSZ128rmik:
	case VSHUFPSZ128rmikz:
	case VSHUFPSZ128rri:
	case VSHUFPSZ128rrik:
	case VSHUFPSZ128rrikz:
	case VSHUFPSZ256rmbi:
	case VSHUFPSZ256rmbik:
	case VSHUFPSZ256rmbikz:
	case VSHUFPSZ256rmi:
	case VSHUFPSZ256rmik:
	case VSHUFPSZ256rmikz:
	case VSHUFPSZ256rri:
	case VSHUFPSZ256rrik:
	case VSHUFPSZ256rrikz:
	case VSHUFPSZrmbi:
	case VSHUFPSZrmbik:
	case VSHUFPSZrmbikz:
	case VSHUFPSZrmi:
	case VSHUFPSZrmik:
	case VSHUFPSZrmikz:
	case VSHUFPSZrri:
	case VSHUFPSZrrik:
	case VSHUFPSZrrikz:
	case VSHUFPSrmi:
	case VSHUFPSrri:
		return true;
	}
	return false;
}

bool isINVLPGA(unsigned Opcode) {
	switch (Opcode) {
	case INVLPGA32:
	case INVLPGA64:
		return true;
	}
	return false;
}

bool isINVLPGB(unsigned Opcode) {
	switch (Opcode) {
	case INVLPGB32:
	case INVLPGB64:
		return true;
	}
	return false;
}

bool isVFNMSUB213SH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB213SHZm_Int:
	case VFNMSUB213SHZm_Intk:
	case VFNMSUB213SHZm_Intkz:
	case VFNMSUB213SHZr_Int:
	case VFNMSUB213SHZr_Intk:
	case VFNMSUB213SHZr_Intkz:
	case VFNMSUB213SHZrb_Int:
	case VFNMSUB213SHZrb_Intk:
	case VFNMSUB213SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isDAS(unsigned Opcode) {
	return Opcode == DAS;
}

bool isVPADDSW(unsigned Opcode) {
	switch (Opcode) {
	case VPADDSWYrm:
	case VPADDSWYrr:
	case VPADDSWZ128rm:
	case VPADDSWZ128rmk:
	case VPADDSWZ128rmkz:
	case VPADDSWZ128rr:
	case VPADDSWZ128rrk:
	case VPADDSWZ128rrkz:
	case VPADDSWZ256rm:
	case VPADDSWZ256rmk:
	case VPADDSWZ256rmkz:
	case VPADDSWZ256rr:
	case VPADDSWZ256rrk:
	case VPADDSWZ256rrkz:
	case VPADDSWZrm:
	case VPADDSWZrmk:
	case VPADDSWZrmkz:
	case VPADDSWZrr:
	case VPADDSWZrrk:
	case VPADDSWZrrkz:
	case VPADDSWrm:
	case VPADDSWrr:
		return true;
	}
	return false;
}

bool isVFNMSUB213SS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB213SSZm_Int:
	case VFNMSUB213SSZm_Intk:
	case VFNMSUB213SSZm_Intkz:
	case VFNMSUB213SSZr_Int:
	case VFNMSUB213SSZr_Intk:
	case VFNMSUB213SSZr_Intkz:
	case VFNMSUB213SSZrb_Int:
	case VFNMSUB213SSZrb_Intk:
	case VFNMSUB213SSZrb_Intkz:
	case VFNMSUB213SSm_Int:
	case VFNMSUB213SSr_Int:
		return true;
	}
	return false;
}

bool isFCOM(unsigned Opcode) {
	switch (Opcode) {
	case COM_FST0r:
	case FCOM32m:
	case FCOM64m:
		return true;
	}
	return false;
}

bool isKXORB(unsigned Opcode) {
	return Opcode == KXORBrr;
}

bool isKXORD(unsigned Opcode) {
	return Opcode == KXORDrr;
}

bool isFCOS(unsigned Opcode) {
	return Opcode == FCOS;
}

bool isVPHADDDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDDQrm:
	case VPHADDDQrr:
		return true;
	}
	return false;
}

bool isCLDEMOTE(unsigned Opcode) {
	return Opcode == CLDEMOTE;
}

bool isKXORQ(unsigned Opcode) {
	return Opcode == KXORQrr;
}

bool isKXORW(unsigned Opcode) {
	return Opcode == KXORWrr;
}

bool isVDPBF16PS(unsigned Opcode) {
	switch (Opcode) {
	case VDPBF16PSZ128m:
	case VDPBF16PSZ128mb:
	case VDPBF16PSZ128mbk:
	case VDPBF16PSZ128mbkz:
	case VDPBF16PSZ128mk:
	case VDPBF16PSZ128mkz:
	case VDPBF16PSZ128r:
	case VDPBF16PSZ128rk:
	case VDPBF16PSZ128rkz:
	case VDPBF16PSZ256m:
	case VDPBF16PSZ256mb:
	case VDPBF16PSZ256mbk:
	case VDPBF16PSZ256mbkz:
	case VDPBF16PSZ256mk:
	case VDPBF16PSZ256mkz:
	case VDPBF16PSZ256r:
	case VDPBF16PSZ256rk:
	case VDPBF16PSZ256rkz:
	case VDPBF16PSZm:
	case VDPBF16PSZmb:
	case VDPBF16PSZmbk:
	case VDPBF16PSZmbkz:
	case VDPBF16PSZmk:
	case VDPBF16PSZmkz:
	case VDPBF16PSZr:
	case VDPBF16PSZrk:
	case VDPBF16PSZrkz:
		return true;
	}
	return false;
}

bool isCLGI(unsigned Opcode) {
	return Opcode == CLGI;
}

bool isVMREAD(unsigned Opcode) {
	switch (Opcode) {
	case VMREAD32mr:
	case VMREAD32rr:
	case VMREAD64mr:
	case VMREAD64rr:
		return true;
	}
	return false;
}

bool isANDPD(unsigned Opcode) {
	switch (Opcode) {
	case ANDPDrm:
	case ANDPDrr:
		return true;
	}
	return false;
}

bool isVFMSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBPD4Ymr:
	case VFMSUBPD4Yrm:
	case VFMSUBPD4Yrr:
	case VFMSUBPD4Yrr_REV:
	case VFMSUBPD4mr:
	case VFMSUBPD4rm:
	case VFMSUBPD4rr:
	case VFMSUBPD4rr_REV:
		return true;
	}
	return false;
}

bool isVFNMADD213PD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD213PDYm:
	case VFNMADD213PDYr:
	case VFNMADD213PDZ128m:
	case VFNMADD213PDZ128mb:
	case VFNMADD213PDZ128mbk:
	case VFNMADD213PDZ128mbkz:
	case VFNMADD213PDZ128mk:
	case VFNMADD213PDZ128mkz:
	case VFNMADD213PDZ128r:
	case VFNMADD213PDZ128rk:
	case VFNMADD213PDZ128rkz:
	case VFNMADD213PDZ256m:
	case VFNMADD213PDZ256mb:
	case VFNMADD213PDZ256mbk:
	case VFNMADD213PDZ256mbkz:
	case VFNMADD213PDZ256mk:
	case VFNMADD213PDZ256mkz:
	case VFNMADD213PDZ256r:
	case VFNMADD213PDZ256rk:
	case VFNMADD213PDZ256rkz:
	case VFNMADD213PDZm:
	case VFNMADD213PDZmb:
	case VFNMADD213PDZmbk:
	case VFNMADD213PDZmbkz:
	case VFNMADD213PDZmk:
	case VFNMADD213PDZmkz:
	case VFNMADD213PDZr:
	case VFNMADD213PDZrb:
	case VFNMADD213PDZrbk:
	case VFNMADD213PDZrbkz:
	case VFNMADD213PDZrk:
	case VFNMADD213PDZrkz:
	case VFNMADD213PDm:
	case VFNMADD213PDr:
		return true;
	}
	return false;
}

bool isVFNMADD213PH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD213PHZ128m:
	case VFNMADD213PHZ128mb:
	case VFNMADD213PHZ128mbk:
	case VFNMADD213PHZ128mbkz:
	case VFNMADD213PHZ128mk:
	case VFNMADD213PHZ128mkz:
	case VFNMADD213PHZ128r:
	case VFNMADD213PHZ128rk:
	case VFNMADD213PHZ128rkz:
	case VFNMADD213PHZ256m:
	case VFNMADD213PHZ256mb:
	case VFNMADD213PHZ256mbk:
	case VFNMADD213PHZ256mbkz:
	case VFNMADD213PHZ256mk:
	case VFNMADD213PHZ256mkz:
	case VFNMADD213PHZ256r:
	case VFNMADD213PHZ256rk:
	case VFNMADD213PHZ256rkz:
	case VFNMADD213PHZm:
	case VFNMADD213PHZmb:
	case VFNMADD213PHZmbk:
	case VFNMADD213PHZmbkz:
	case VFNMADD213PHZmk:
	case VFNMADD213PHZmkz:
	case VFNMADD213PHZr:
	case VFNMADD213PHZrb:
	case VFNMADD213PHZrbk:
	case VFNMADD213PHZrbkz:
	case VFNMADD213PHZrk:
	case VFNMADD213PHZrkz:
		return true;
	}
	return false;
}

bool isVFNMSUB231SD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB231SDZm_Int:
	case VFNMSUB231SDZm_Intk:
	case VFNMSUB231SDZm_Intkz:
	case VFNMSUB231SDZr_Int:
	case VFNMSUB231SDZr_Intk:
	case VFNMSUB231SDZr_Intkz:
	case VFNMSUB231SDZrb_Int:
	case VFNMSUB231SDZrb_Intk:
	case VFNMSUB231SDZrb_Intkz:
	case VFNMSUB231SDm_Int:
	case VFNMSUB231SDr_Int:
		return true;
	}
	return false;
}

bool isFSQRT(unsigned Opcode) {
	return Opcode == SQRT_F;
}

bool isVFNMSUB231SH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB231SHZm_Int:
	case VFNMSUB231SHZm_Intk:
	case VFNMSUB231SHZm_Intkz:
	case VFNMSUB231SHZr_Int:
	case VFNMSUB231SHZr_Intk:
	case VFNMSUB231SHZr_Intkz:
	case VFNMSUB231SHZrb_Int:
	case VFNMSUB231SHZrb_Intk:
	case VFNMSUB231SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isPCLMULQDQ(unsigned Opcode) {
	switch (Opcode) {
	case PCLMULQDQrm:
	case PCLMULQDQrr:
		return true;
	}
	return false;
}

bool isVRCP14PD(unsigned Opcode) {
	switch (Opcode) {
	case VRCP14PDZ128m:
	case VRCP14PDZ128mb:
	case VRCP14PDZ128mbk:
	case VRCP14PDZ128mbkz:
	case VRCP14PDZ128mk:
	case VRCP14PDZ128mkz:
	case VRCP14PDZ128r:
	case VRCP14PDZ128rk:
	case VRCP14PDZ128rkz:
	case VRCP14PDZ256m:
	case VRCP14PDZ256mb:
	case VRCP14PDZ256mbk:
	case VRCP14PDZ256mbkz:
	case VRCP14PDZ256mk:
	case VRCP14PDZ256mkz:
	case VRCP14PDZ256r:
	case VRCP14PDZ256rk:
	case VRCP14PDZ256rkz:
	case VRCP14PDZm:
	case VRCP14PDZmb:
	case VRCP14PDZmbk:
	case VRCP14PDZmbkz:
	case VRCP14PDZmk:
	case VRCP14PDZmkz:
	case VRCP14PDZr:
	case VRCP14PDZrk:
	case VRCP14PDZrkz:
		return true;
	}
	return false;
}

bool isANDPS(unsigned Opcode) {
	switch (Opcode) {
	case ANDPSrm:
	case ANDPSrr:
		return true;
	}
	return false;
}

bool isVFMSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBPS4Ymr:
	case VFMSUBPS4Yrm:
	case VFMSUBPS4Yrr:
	case VFMSUBPS4Yrr_REV:
	case VFMSUBPS4mr:
	case VFMSUBPS4rm:
	case VFMSUBPS4rr:
	case VFMSUBPS4rr_REV:
		return true;
	}
	return false;
}

bool isVFNMADD213PS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD213PSYm:
	case VFNMADD213PSYr:
	case VFNMADD213PSZ128m:
	case VFNMADD213PSZ128mb:
	case VFNMADD213PSZ128mbk:
	case VFNMADD213PSZ128mbkz:
	case VFNMADD213PSZ128mk:
	case VFNMADD213PSZ128mkz:
	case VFNMADD213PSZ128r:
	case VFNMADD213PSZ128rk:
	case VFNMADD213PSZ128rkz:
	case VFNMADD213PSZ256m:
	case VFNMADD213PSZ256mb:
	case VFNMADD213PSZ256mbk:
	case VFNMADD213PSZ256mbkz:
	case VFNMADD213PSZ256mk:
	case VFNMADD213PSZ256mkz:
	case VFNMADD213PSZ256r:
	case VFNMADD213PSZ256rk:
	case VFNMADD213PSZ256rkz:
	case VFNMADD213PSZm:
	case VFNMADD213PSZmb:
	case VFNMADD213PSZmbk:
	case VFNMADD213PSZmbkz:
	case VFNMADD213PSZmk:
	case VFNMADD213PSZmkz:
	case VFNMADD213PSZr:
	case VFNMADD213PSZrb:
	case VFNMADD213PSZrbk:
	case VFNMADD213PSZrbkz:
	case VFNMADD213PSZrk:
	case VFNMADD213PSZrkz:
	case VFNMADD213PSm:
	case VFNMADD213PSr:
		return true;
	}
	return false;
}

bool isPMULHW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMULHWrm:
	case MMX_PMULHWrr:
	case PMULHWrm:
	case PMULHWrr:
		return true;
	}
	return false;
}

bool isVFNMSUB231SS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMSUB231SSZm_Int:
	case VFNMSUB231SSZm_Intk:
	case VFNMSUB231SSZm_Intkz:
	case VFNMSUB231SSZr_Int:
	case VFNMSUB231SSZr_Intk:
	case VFNMSUB231SSZr_Intkz:
	case VFNMSUB231SSZrb_Int:
	case VFNMSUB231SSZrb_Intk:
	case VFNMSUB231SSZrb_Intkz:
	case VFNMSUB231SSm_Int:
	case VFNMSUB231SSr_Int:
		return true;
	}
	return false;
}

bool isAESDECWIDE256KL(unsigned Opcode) {
	return Opcode == AESDECWIDE256KL;
}

bool isRSQRTSS(unsigned Opcode) {
	switch (Opcode) {
	case RSQRTSSm_Int:
	case RSQRTSSr_Int:
		return true;
	}
	return false;
}

bool isVRCP14PS(unsigned Opcode) {
	switch (Opcode) {
	case VRCP14PSZ128m:
	case VRCP14PSZ128mb:
	case VRCP14PSZ128mbk:
	case VRCP14PSZ128mbkz:
	case VRCP14PSZ128mk:
	case VRCP14PSZ128mkz:
	case VRCP14PSZ128r:
	case VRCP14PSZ128rk:
	case VRCP14PSZ128rkz:
	case VRCP14PSZ256m:
	case VRCP14PSZ256mb:
	case VRCP14PSZ256mbk:
	case VRCP14PSZ256mbkz:
	case VRCP14PSZ256mk:
	case VRCP14PSZ256mkz:
	case VRCP14PSZ256r:
	case VRCP14PSZ256rk:
	case VRCP14PSZ256rkz:
	case VRCP14PSZm:
	case VRCP14PSZmb:
	case VRCP14PSZmbk:
	case VRCP14PSZmbkz:
	case VRCP14PSZmk:
	case VRCP14PSZmkz:
	case VRCP14PSZr:
	case VRCP14PSZrk:
	case VRCP14PSZrkz:
		return true;
	}
	return false;
}

bool isVZEROUPPER(unsigned Opcode) {
	return Opcode == VZEROUPPER;
}

bool isVPAVGB(unsigned Opcode) {
	switch (Opcode) {
	case VPAVGBYrm:
	case VPAVGBYrr:
	case VPAVGBZ128rm:
	case VPAVGBZ128rmk:
	case VPAVGBZ128rmkz:
	case VPAVGBZ128rr:
	case VPAVGBZ128rrk:
	case VPAVGBZ128rrkz:
	case VPAVGBZ256rm:
	case VPAVGBZ256rmk:
	case VPAVGBZ256rmkz:
	case VPAVGBZ256rr:
	case VPAVGBZ256rrk:
	case VPAVGBZ256rrkz:
	case VPAVGBZrm:
	case VPAVGBZrmk:
	case VPAVGBZrmkz:
	case VPAVGBZrr:
	case VPAVGBZrrk:
	case VPAVGBZrrkz:
	case VPAVGBrm:
	case VPAVGBrr:
		return true;
	}
	return false;
}

bool isVPMOVSXBD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSXBDYrm:
	case VPMOVSXBDYrr:
	case VPMOVSXBDZ128rm:
	case VPMOVSXBDZ128rmk:
	case VPMOVSXBDZ128rmkz:
	case VPMOVSXBDZ128rr:
	case VPMOVSXBDZ128rrk:
	case VPMOVSXBDZ128rrkz:
	case VPMOVSXBDZ256rm:
	case VPMOVSXBDZ256rmk:
	case VPMOVSXBDZ256rmkz:
	case VPMOVSXBDZ256rr:
	case VPMOVSXBDZ256rrk:
	case VPMOVSXBDZ256rrkz:
	case VPMOVSXBDZrm:
	case VPMOVSXBDZrmk:
	case VPMOVSXBDZrmkz:
	case VPMOVSXBDZrr:
	case VPMOVSXBDZrrk:
	case VPMOVSXBDZrrkz:
	case VPMOVSXBDrm:
	case VPMOVSXBDrr:
		return true;
	}
	return false;
}

bool isRDGSBASE(unsigned Opcode) {
	switch (Opcode) {
	case RDGSBASE:
	case RDGSBASE64:
		return true;
	}
	return false;
}

bool isFLDCW(unsigned Opcode) {
	return Opcode == FLDCW16m;
}

bool isLIDTD(unsigned Opcode) {
	return Opcode == LIDT32m;
}

bool isVGATHERPF1DPD(unsigned Opcode) {
	return Opcode == VGATHERPF1DPDm;
}

bool isSFENCE(unsigned Opcode) {
	return Opcode == SFENCE;
}

bool isVPMOVSXBQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSXBQYrm:
	case VPMOVSXBQYrr:
	case VPMOVSXBQZ128rm:
	case VPMOVSXBQZ128rmk:
	case VPMOVSXBQZ128rmkz:
	case VPMOVSXBQZ128rr:
	case VPMOVSXBQZ128rrk:
	case VPMOVSXBQZ128rrkz:
	case VPMOVSXBQZ256rm:
	case VPMOVSXBQZ256rmk:
	case VPMOVSXBQZ256rmkz:
	case VPMOVSXBQZ256rr:
	case VPMOVSXBQZ256rrk:
	case VPMOVSXBQZ256rrkz:
	case VPMOVSXBQZrm:
	case VPMOVSXBQZrmk:
	case VPMOVSXBQZrmkz:
	case VPMOVSXBQZrr:
	case VPMOVSXBQZrrk:
	case VPMOVSXBQZrrkz:
	case VPMOVSXBQrm:
	case VPMOVSXBQrr:
		return true;
	}
	return false;
}

bool isVPAVGW(unsigned Opcode) {
	switch (Opcode) {
	case VPAVGWYrm:
	case VPAVGWYrr:
	case VPAVGWZ128rm:
	case VPAVGWZ128rmk:
	case VPAVGWZ128rmkz:
	case VPAVGWZ128rr:
	case VPAVGWZ128rrk:
	case VPAVGWZ128rrkz:
	case VPAVGWZ256rm:
	case VPAVGWZ256rmk:
	case VPAVGWZ256rmkz:
	case VPAVGWZ256rr:
	case VPAVGWZ256rrk:
	case VPAVGWZ256rrkz:
	case VPAVGWZrm:
	case VPAVGWZrmk:
	case VPAVGWZrmkz:
	case VPAVGWZrr:
	case VPAVGWZrrk:
	case VPAVGWZrrkz:
	case VPAVGWrm:
	case VPAVGWrr:
		return true;
	}
	return false;
}

bool isVPMOVSXBW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSXBWYrm:
	case VPMOVSXBWYrr:
	case VPMOVSXBWZ128rm:
	case VPMOVSXBWZ128rmk:
	case VPMOVSXBWZ128rmkz:
	case VPMOVSXBWZ128rr:
	case VPMOVSXBWZ128rrk:
	case VPMOVSXBWZ128rrkz:
	case VPMOVSXBWZ256rm:
	case VPMOVSXBWZ256rmk:
	case VPMOVSXBWZ256rmkz:
	case VPMOVSXBWZ256rr:
	case VPMOVSXBWZ256rrk:
	case VPMOVSXBWZ256rrkz:
	case VPMOVSXBWZrm:
	case VPMOVSXBWZrmk:
	case VPMOVSXBWZrmkz:
	case VPMOVSXBWZrr:
	case VPMOVSXBWZrrk:
	case VPMOVSXBWZrrkz:
	case VPMOVSXBWrm:
	case VPMOVSXBWrr:
		return true;
	}
	return false;
}

bool isFCMOVB(unsigned Opcode) {
	return Opcode == CMOVB_F;
}

bool isVMASKMOVDQU(unsigned Opcode) {
	return Opcode == VMASKMOVDQU64;
}

bool isVGATHERPF1DPS(unsigned Opcode) {
	return Opcode == VGATHERPF1DPSm;
}

bool isFCMOVE(unsigned Opcode) {
	return Opcode == CMOVE_F;
}

bool isVMLOAD(unsigned Opcode) {
	switch (Opcode) {
	case VMLOAD32:
	case VMLOAD64:
		return true;
	}
	return false;
}

bool isLIDTW(unsigned Opcode) {
	return Opcode == LIDT16m;
}

bool isVFNMADD231PD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD231PDYm:
	case VFNMADD231PDYr:
	case VFNMADD231PDZ128m:
	case VFNMADD231PDZ128mb:
	case VFNMADD231PDZ128mbk:
	case VFNMADD231PDZ128mbkz:
	case VFNMADD231PDZ128mk:
	case VFNMADD231PDZ128mkz:
	case VFNMADD231PDZ128r:
	case VFNMADD231PDZ128rk:
	case VFNMADD231PDZ128rkz:
	case VFNMADD231PDZ256m:
	case VFNMADD231PDZ256mb:
	case VFNMADD231PDZ256mbk:
	case VFNMADD231PDZ256mbkz:
	case VFNMADD231PDZ256mk:
	case VFNMADD231PDZ256mkz:
	case VFNMADD231PDZ256r:
	case VFNMADD231PDZ256rk:
	case VFNMADD231PDZ256rkz:
	case VFNMADD231PDZm:
	case VFNMADD231PDZmb:
	case VFNMADD231PDZmbk:
	case VFNMADD231PDZmbkz:
	case VFNMADD231PDZmk:
	case VFNMADD231PDZmkz:
	case VFNMADD231PDZr:
	case VFNMADD231PDZrb:
	case VFNMADD231PDZrbk:
	case VFNMADD231PDZrbkz:
	case VFNMADD231PDZrk:
	case VFNMADD231PDZrkz:
	case VFNMADD231PDm:
	case VFNMADD231PDr:
		return true;
	}
	return false;
}

bool isDEC(unsigned Opcode) {
	switch (Opcode) {
	case DEC16m:
	case DEC16r:
	case DEC16r_alt:
	case DEC32m:
	case DEC32r:
	case DEC32r_alt:
	case DEC64m:
	case DEC64r:
	case DEC8m:
	case DEC8r:
		return true;
	}
	return false;
}

bool isVFNMADD231PH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD231PHZ128m:
	case VFNMADD231PHZ128mb:
	case VFNMADD231PHZ128mbk:
	case VFNMADD231PHZ128mbkz:
	case VFNMADD231PHZ128mk:
	case VFNMADD231PHZ128mkz:
	case VFNMADD231PHZ128r:
	case VFNMADD231PHZ128rk:
	case VFNMADD231PHZ128rkz:
	case VFNMADD231PHZ256m:
	case VFNMADD231PHZ256mb:
	case VFNMADD231PHZ256mbk:
	case VFNMADD231PHZ256mbkz:
	case VFNMADD231PHZ256mk:
	case VFNMADD231PHZ256mkz:
	case VFNMADD231PHZ256r:
	case VFNMADD231PHZ256rk:
	case VFNMADD231PHZ256rkz:
	case VFNMADD231PHZm:
	case VFNMADD231PHZmb:
	case VFNMADD231PHZmbk:
	case VFNMADD231PHZmbkz:
	case VFNMADD231PHZmk:
	case VFNMADD231PHZmkz:
	case VFNMADD231PHZr:
	case VFNMADD231PHZrb:
	case VFNMADD231PHZrbk:
	case VFNMADD231PHZrbkz:
	case VFNMADD231PHZrk:
	case VFNMADD231PHZrkz:
		return true;
	}
	return false;
}

bool isFCMOVU(unsigned Opcode) {
	return Opcode == CMOVP_F;
}

bool isRSTORSSP(unsigned Opcode) {
	return Opcode == RSTORSSP;
}

bool isVMAXPD(unsigned Opcode) {
	switch (Opcode) {
	case VMAXPDYrm:
	case VMAXPDYrr:
	case VMAXPDZ128rm:
	case VMAXPDZ128rmb:
	case VMAXPDZ128rmbk:
	case VMAXPDZ128rmbkz:
	case VMAXPDZ128rmk:
	case VMAXPDZ128rmkz:
	case VMAXPDZ128rr:
	case VMAXPDZ128rrk:
	case VMAXPDZ128rrkz:
	case VMAXPDZ256rm:
	case VMAXPDZ256rmb:
	case VMAXPDZ256rmbk:
	case VMAXPDZ256rmbkz:
	case VMAXPDZ256rmk:
	case VMAXPDZ256rmkz:
	case VMAXPDZ256rr:
	case VMAXPDZ256rrk:
	case VMAXPDZ256rrkz:
	case VMAXPDZrm:
	case VMAXPDZrmb:
	case VMAXPDZrmbk:
	case VMAXPDZrmbkz:
	case VMAXPDZrmk:
	case VMAXPDZrmkz:
	case VMAXPDZrr:
	case VMAXPDZrrb:
	case VMAXPDZrrbk:
	case VMAXPDZrrbkz:
	case VMAXPDZrrk:
	case VMAXPDZrrkz:
	case VMAXPDrm:
	case VMAXPDrr:
		return true;
	}
	return false;
}

bool isVMAXPH(unsigned Opcode) {
	switch (Opcode) {
	case VMAXPHZ128rm:
	case VMAXPHZ128rmb:
	case VMAXPHZ128rmbk:
	case VMAXPHZ128rmbkz:
	case VMAXPHZ128rmk:
	case VMAXPHZ128rmkz:
	case VMAXPHZ128rr:
	case VMAXPHZ128rrk:
	case VMAXPHZ128rrkz:
	case VMAXPHZ256rm:
	case VMAXPHZ256rmb:
	case VMAXPHZ256rmbk:
	case VMAXPHZ256rmbkz:
	case VMAXPHZ256rmk:
	case VMAXPHZ256rmkz:
	case VMAXPHZ256rr:
	case VMAXPHZ256rrk:
	case VMAXPHZ256rrkz:
	case VMAXPHZrm:
	case VMAXPHZrmb:
	case VMAXPHZrmbk:
	case VMAXPHZrmbkz:
	case VMAXPHZrmk:
	case VMAXPHZrmkz:
	case VMAXPHZrr:
	case VMAXPHZrrb:
	case VMAXPHZrrbk:
	case VMAXPHZrrbkz:
	case VMAXPHZrrk:
	case VMAXPHZrrkz:
		return true;
	}
	return false;
}

bool isPUNPCKLBW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PUNPCKLBWrm:
	case MMX_PUNPCKLBWrr:
	case PUNPCKLBWrm:
	case PUNPCKLBWrr:
		return true;
	}
	return false;
}

bool isIMUL(unsigned Opcode) {
	switch (Opcode) {
	case IMUL16m:
	case IMUL16r:
	case IMUL16rm:
	case IMUL16rmi:
	case IMUL16rmi8:
	case IMUL16rr:
	case IMUL16rri:
	case IMUL16rri8:
	case IMUL32m:
	case IMUL32r:
	case IMUL32rm:
	case IMUL32rmi:
	case IMUL32rmi8:
	case IMUL32rr:
	case IMUL32rri:
	case IMUL32rri8:
	case IMUL64m:
	case IMUL64r:
	case IMUL64rm:
	case IMUL64rmi32:
	case IMUL64rmi8:
	case IMUL64rr:
	case IMUL64rri32:
	case IMUL64rri8:
	case IMUL8m:
	case IMUL8r:
		return true;
	}
	return false;
}

bool isTILELOADDT1(unsigned Opcode) {
	return Opcode == TILELOADDT1;
}

bool isVFNMADD231PS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD231PSYm:
	case VFNMADD231PSYr:
	case VFNMADD231PSZ128m:
	case VFNMADD231PSZ128mb:
	case VFNMADD231PSZ128mbk:
	case VFNMADD231PSZ128mbkz:
	case VFNMADD231PSZ128mk:
	case VFNMADD231PSZ128mkz:
	case VFNMADD231PSZ128r:
	case VFNMADD231PSZ128rk:
	case VFNMADD231PSZ128rkz:
	case VFNMADD231PSZ256m:
	case VFNMADD231PSZ256mb:
	case VFNMADD231PSZ256mbk:
	case VFNMADD231PSZ256mbkz:
	case VFNMADD231PSZ256mk:
	case VFNMADD231PSZ256mkz:
	case VFNMADD231PSZ256r:
	case VFNMADD231PSZ256rk:
	case VFNMADD231PSZ256rkz:
	case VFNMADD231PSZm:
	case VFNMADD231PSZmb:
	case VFNMADD231PSZmbk:
	case VFNMADD231PSZmbkz:
	case VFNMADD231PSZmk:
	case VFNMADD231PSZmkz:
	case VFNMADD231PSZr:
	case VFNMADD231PSZrb:
	case VFNMADD231PSZrbk:
	case VFNMADD231PSZrbkz:
	case VFNMADD231PSZrk:
	case VFNMADD231PSZrkz:
	case VFNMADD231PSm:
	case VFNMADD231PSr:
		return true;
	}
	return false;
}

bool isVMAXPS(unsigned Opcode) {
	switch (Opcode) {
	case VMAXPSYrm:
	case VMAXPSYrr:
	case VMAXPSZ128rm:
	case VMAXPSZ128rmb:
	case VMAXPSZ128rmbk:
	case VMAXPSZ128rmbkz:
	case VMAXPSZ128rmk:
	case VMAXPSZ128rmkz:
	case VMAXPSZ128rr:
	case VMAXPSZ128rrk:
	case VMAXPSZ128rrkz:
	case VMAXPSZ256rm:
	case VMAXPSZ256rmb:
	case VMAXPSZ256rmbk:
	case VMAXPSZ256rmbkz:
	case VMAXPSZ256rmk:
	case VMAXPSZ256rmkz:
	case VMAXPSZ256rr:
	case VMAXPSZ256rrk:
	case VMAXPSZ256rrkz:
	case VMAXPSZrm:
	case VMAXPSZrmb:
	case VMAXPSZrmbk:
	case VMAXPSZrmbkz:
	case VMAXPSZrmk:
	case VMAXPSZrmkz:
	case VMAXPSZrr:
	case VMAXPSZrrb:
	case VMAXPSZrrbk:
	case VMAXPSZrrbkz:
	case VMAXPSZrrk:
	case VMAXPSZrrkz:
	case VMAXPSrm:
	case VMAXPSrr:
		return true;
	}
	return false;
}

bool isVPXOR(unsigned Opcode) {
	switch (Opcode) {
	case VPXORYrm:
	case VPXORYrr:
	case VPXORrm:
	case VPXORrr:
		return true;
	}
	return false;
}

bool isOR(unsigned Opcode) {
	switch (Opcode) {
	case OR16i16:
	case OR16mi:
	case OR16mi8:
	case OR16mr:
	case OR16ri:
	case OR16ri8:
	case OR16rm:
	case OR16rr:
	case OR16rr_REV:
	case OR32i32:
	case OR32mi:
	case OR32mi8:
	case OR32mr:
	case OR32ri:
	case OR32ri8:
	case OR32rm:
	case OR32rr:
	case OR32rr_REV:
	case OR64i32:
	case OR64mi32:
	case OR64mi8:
	case OR64mr:
	case OR64ri32:
	case OR64ri8:
	case OR64rm:
	case OR64rr:
	case OR64rr_REV:
	case OR8i8:
	case OR8mi:
	case OR8mi8:
	case OR8mr:
	case OR8ri:
	case OR8ri8:
	case OR8rm:
	case OR8rr:
	case OR8rr_REV:
		return true;
	}
	return false;
}

bool isMOVZX(unsigned Opcode) {
	switch (Opcode) {
	case MOVZX16rm16:
	case MOVZX16rm8:
	case MOVZX16rr16:
	case MOVZX16rr8:
	case MOVZX32rm16:
	case MOVZX32rm8:
	case MOVZX32rr16:
	case MOVZX32rr8:
	case MOVZX64rm16:
	case MOVZX64rm8:
	case MOVZX64rr16:
	case MOVZX64rr8:
		return true;
	}
	return false;
}

bool isXSAVES64(unsigned Opcode) {
	return Opcode == XSAVES64;
}

bool isVPUNPCKHBW(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKHBWYrm:
	case VPUNPCKHBWYrr:
	case VPUNPCKHBWZ128rm:
	case VPUNPCKHBWZ128rmk:
	case VPUNPCKHBWZ128rmkz:
	case VPUNPCKHBWZ128rr:
	case VPUNPCKHBWZ128rrk:
	case VPUNPCKHBWZ128rrkz:
	case VPUNPCKHBWZ256rm:
	case VPUNPCKHBWZ256rmk:
	case VPUNPCKHBWZ256rmkz:
	case VPUNPCKHBWZ256rr:
	case VPUNPCKHBWZ256rrk:
	case VPUNPCKHBWZ256rrkz:
	case VPUNPCKHBWZrm:
	case VPUNPCKHBWZrmk:
	case VPUNPCKHBWZrmkz:
	case VPUNPCKHBWZrr:
	case VPUNPCKHBWZrrk:
	case VPUNPCKHBWZrrkz:
	case VPUNPCKHBWrm:
	case VPUNPCKHBWrr:
		return true;
	}
	return false;
}

bool isSYSRET(unsigned Opcode) {
	return Opcode == SYSRET;
}

bool isVFMSUBSD(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBSD4mr:
	case VFMSUBSD4rm:
	case VFMSUBSD4rr:
	case VFMSUBSD4rr_REV:
		return true;
	}
	return false;
}

bool isVFNMADD213SD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD213SDZm_Int:
	case VFNMADD213SDZm_Intk:
	case VFNMADD213SDZm_Intkz:
	case VFNMADD213SDZr_Int:
	case VFNMADD213SDZr_Intk:
	case VFNMADD213SDZr_Intkz:
	case VFNMADD213SDZrb_Int:
	case VFNMADD213SDZrb_Intk:
	case VFNMADD213SDZrb_Intkz:
	case VFNMADD213SDm_Int:
	case VFNMADD213SDr_Int:
		return true;
	}
	return false;
}

bool isVFNMADD213SH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD213SHZm_Int:
	case VFNMADD213SHZm_Intk:
	case VFNMADD213SHZm_Intkz:
	case VFNMADD213SHZr_Int:
	case VFNMADD213SHZr_Intk:
	case VFNMADD213SHZr_Intkz:
	case VFNMADD213SHZrb_Int:
	case VFNMADD213SHZrb_Intk:
	case VFNMADD213SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isSGDT(unsigned Opcode) {
	return Opcode == SGDT64m;
}

bool isVRCP14SD(unsigned Opcode) {
	switch (Opcode) {
	case VRCP14SDZrm:
	case VRCP14SDZrmk:
	case VRCP14SDZrmkz:
	case VRCP14SDZrr:
	case VRCP14SDZrrk:
	case VRCP14SDZrrkz:
		return true;
	}
	return false;
}

bool isVPMOVSXDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSXDQYrm:
	case VPMOVSXDQYrr:
	case VPMOVSXDQZ128rm:
	case VPMOVSXDQZ128rmk:
	case VPMOVSXDQZ128rmkz:
	case VPMOVSXDQZ128rr:
	case VPMOVSXDQZ128rrk:
	case VPMOVSXDQZ128rrkz:
	case VPMOVSXDQZ256rm:
	case VPMOVSXDQZ256rmk:
	case VPMOVSXDQZ256rmkz:
	case VPMOVSXDQZ256rr:
	case VPMOVSXDQZ256rrk:
	case VPMOVSXDQZ256rrkz:
	case VPMOVSXDQZrm:
	case VPMOVSXDQZrmk:
	case VPMOVSXDQZrmkz:
	case VPMOVSXDQZrr:
	case VPMOVSXDQZrrk:
	case VPMOVSXDQZrrkz:
	case VPMOVSXDQrm:
	case VPMOVSXDQrr:
		return true;
	}
	return false;
}

bool isSYSEXIT(unsigned Opcode) {
	return Opcode == SYSEXIT;
}

bool isPMOVZXBD(unsigned Opcode) {
	switch (Opcode) {
	case PMOVZXBDrm:
	case PMOVZXBDrr:
		return true;
	}
	return false;
}

bool isVFNMADD213SS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD213SSZm_Int:
	case VFNMADD213SSZm_Intk:
	case VFNMADD213SSZm_Intkz:
	case VFNMADD213SSZr_Int:
	case VFNMADD213SSZr_Intk:
	case VFNMADD213SSZr_Intkz:
	case VFNMADD213SSZrb_Int:
	case VFNMADD213SSZrb_Intk:
	case VFNMADD213SSZrb_Intkz:
	case VFNMADD213SSm_Int:
	case VFNMADD213SSr_Int:
		return true;
	}
	return false;
}

bool isVPMADCSSWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMADCSSWDrm:
	case VPMADCSSWDrr:
		return true;
	}
	return false;
}

bool isVPMULUDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMULUDQYrm:
	case VPMULUDQYrr:
	case VPMULUDQZ128rm:
	case VPMULUDQZ128rmb:
	case VPMULUDQZ128rmbk:
	case VPMULUDQZ128rmbkz:
	case VPMULUDQZ128rmk:
	case VPMULUDQZ128rmkz:
	case VPMULUDQZ128rr:
	case VPMULUDQZ128rrk:
	case VPMULUDQZ128rrkz:
	case VPMULUDQZ256rm:
	case VPMULUDQZ256rmb:
	case VPMULUDQZ256rmbk:
	case VPMULUDQZ256rmbkz:
	case VPMULUDQZ256rmk:
	case VPMULUDQZ256rmkz:
	case VPMULUDQZ256rr:
	case VPMULUDQZ256rrk:
	case VPMULUDQZ256rrkz:
	case VPMULUDQZrm:
	case VPMULUDQZrmb:
	case VPMULUDQZrmbk:
	case VPMULUDQZrmbkz:
	case VPMULUDQZrmk:
	case VPMULUDQZrmkz:
	case VPMULUDQZrr:
	case VPMULUDQZrrk:
	case VPMULUDQZrrkz:
	case VPMULUDQrm:
	case VPMULUDQrr:
		return true;
	}
	return false;
}

bool isVFMSUBSS(unsigned Opcode) {
	switch (Opcode) {
	case VFMSUBSS4mr:
	case VFMSUBSS4rm:
	case VFMSUBSS4rr:
	case VFMSUBSS4rr_REV:
		return true;
	}
	return false;
}

bool isVPXORD(unsigned Opcode) {
	switch (Opcode) {
	case VPXORDZ128rm:
	case VPXORDZ128rmb:
	case VPXORDZ128rmbk:
	case VPXORDZ128rmbkz:
	case VPXORDZ128rmk:
	case VPXORDZ128rmkz:
	case VPXORDZ128rr:
	case VPXORDZ128rrk:
	case VPXORDZ128rrkz:
	case VPXORDZ256rm:
	case VPXORDZ256rmb:
	case VPXORDZ256rmbk:
	case VPXORDZ256rmbkz:
	case VPXORDZ256rmk:
	case VPXORDZ256rmkz:
	case VPXORDZ256rr:
	case VPXORDZ256rrk:
	case VPXORDZ256rrkz:
	case VPXORDZrm:
	case VPXORDZrmb:
	case VPXORDZrmbk:
	case VPXORDZrmbkz:
	case VPXORDZrmk:
	case VPXORDZrmkz:
	case VPXORDZrr:
	case VPXORDZrrk:
	case VPXORDZrrkz:
		return true;
	}
	return false;
}

bool isXSAVEOPT64(unsigned Opcode) {
	return Opcode == XSAVEOPT64;
}

bool isXSHA1(unsigned Opcode) {
	return Opcode == XSHA1;
}

bool isVRCP14SS(unsigned Opcode) {
	switch (Opcode) {
	case VRCP14SSZrm:
	case VRCP14SSZrmk:
	case VRCP14SSZrmkz:
	case VRCP14SSZrr:
	case VRCP14SSZrrk:
	case VRCP14SSZrrkz:
		return true;
	}
	return false;
}

bool isPMOVZXBQ(unsigned Opcode) {
	switch (Opcode) {
	case PMOVZXBQrm:
	case PMOVZXBQrr:
		return true;
	}
	return false;
}

bool isSHL(unsigned Opcode) {
	switch (Opcode) {
	case SHL16m1:
	case SHL16mCL:
	case SHL16mi:
	case SHL16r1:
	case SHL16rCL:
	case SHL16ri:
	case SHL32m1:
	case SHL32mCL:
	case SHL32mi:
	case SHL32r1:
	case SHL32rCL:
	case SHL32ri:
	case SHL64m1:
	case SHL64mCL:
	case SHL64mi:
	case SHL64r1:
	case SHL64rCL:
	case SHL64ri:
	case SHL8m1:
	case SHL8mCL:
	case SHL8mi:
	case SHL8r1:
	case SHL8rCL:
	case SHL8ri:
		return true;
	}
	return false;
}

bool isPMOVZXBW(unsigned Opcode) {
	switch (Opcode) {
	case PMOVZXBWrm:
	case PMOVZXBWrr:
		return true;
	}
	return false;
}

bool isPMULLD(unsigned Opcode) {
	switch (Opcode) {
	case PMULLDrm:
	case PMULLDrr:
		return true;
	}
	return false;
}

bool isVCVTPS2UQQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPS2UQQZ128rm:
	case VCVTPS2UQQZ128rmb:
	case VCVTPS2UQQZ128rmbk:
	case VCVTPS2UQQZ128rmbkz:
	case VCVTPS2UQQZ128rmk:
	case VCVTPS2UQQZ128rmkz:
	case VCVTPS2UQQZ128rr:
	case VCVTPS2UQQZ128rrk:
	case VCVTPS2UQQZ128rrkz:
	case VCVTPS2UQQZ256rm:
	case VCVTPS2UQQZ256rmb:
	case VCVTPS2UQQZ256rmbk:
	case VCVTPS2UQQZ256rmbkz:
	case VCVTPS2UQQZ256rmk:
	case VCVTPS2UQQZ256rmkz:
	case VCVTPS2UQQZ256rr:
	case VCVTPS2UQQZ256rrk:
	case VCVTPS2UQQZ256rrkz:
	case VCVTPS2UQQZrm:
	case VCVTPS2UQQZrmb:
	case VCVTPS2UQQZrmbk:
	case VCVTPS2UQQZrmbkz:
	case VCVTPS2UQQZrmk:
	case VCVTPS2UQQZrmkz:
	case VCVTPS2UQQZrr:
	case VCVTPS2UQQZrrb:
	case VCVTPS2UQQZrrbk:
	case VCVTPS2UQQZrrbkz:
	case VCVTPS2UQQZrrk:
	case VCVTPS2UQQZrrkz:
		return true;
	}
	return false;
}

bool isVPBLENDD(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDDYrmi:
	case VPBLENDDYrri:
	case VPBLENDDrmi:
	case VPBLENDDrri:
		return true;
	}
	return false;
}

bool isVPXORQ(unsigned Opcode) {
	switch (Opcode) {
	case VPXORQZ128rm:
	case VPXORQZ128rmb:
	case VPXORQZ128rmbk:
	case VPXORQZ128rmbkz:
	case VPXORQZ128rmk:
	case VPXORQZ128rmkz:
	case VPXORQZ128rr:
	case VPXORQZ128rrk:
	case VPXORQZ128rrkz:
	case VPXORQZ256rm:
	case VPXORQZ256rmb:
	case VPXORQZ256rmbk:
	case VPXORQZ256rmbkz:
	case VPXORQZ256rmk:
	case VPXORQZ256rmkz:
	case VPXORQZ256rr:
	case VPXORQZ256rrk:
	case VPXORQZ256rrkz:
	case VPXORQZrm:
	case VPXORQZrmb:
	case VPXORQZrmbk:
	case VPXORQZrmbkz:
	case VPXORQZrmk:
	case VPXORQZrmkz:
	case VPXORQZrr:
	case VPXORQZrrk:
	case VPXORQZrrkz:
		return true;
	}
	return false;
}

bool isPUNPCKLDQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PUNPCKLDQrm:
	case MMX_PUNPCKLDQrr:
	case PUNPCKLDQrm:
	case PUNPCKLDQrr:
		return true;
	}
	return false;
}

bool isSHR(unsigned Opcode) {
	switch (Opcode) {
	case SHR16m1:
	case SHR16mCL:
	case SHR16mi:
	case SHR16r1:
	case SHR16rCL:
	case SHR16ri:
	case SHR32m1:
	case SHR32mCL:
	case SHR32mi:
	case SHR32r1:
	case SHR32rCL:
	case SHR32ri:
	case SHR64m1:
	case SHR64mCL:
	case SHR64mi:
	case SHR64r1:
	case SHR64rCL:
	case SHR64ri:
	case SHR8m1:
	case SHR8mCL:
	case SHR8mi:
	case SHR8r1:
	case SHR8rCL:
	case SHR8ri:
		return true;
	}
	return false;
}

bool isVPUNPCKLQDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKLQDQYrm:
	case VPUNPCKLQDQYrr:
	case VPUNPCKLQDQZ128rm:
	case VPUNPCKLQDQZ128rmb:
	case VPUNPCKLQDQZ128rmbk:
	case VPUNPCKLQDQZ128rmbkz:
	case VPUNPCKLQDQZ128rmk:
	case VPUNPCKLQDQZ128rmkz:
	case VPUNPCKLQDQZ128rr:
	case VPUNPCKLQDQZ128rrk:
	case VPUNPCKLQDQZ128rrkz:
	case VPUNPCKLQDQZ256rm:
	case VPUNPCKLQDQZ256rmb:
	case VPUNPCKLQDQZ256rmbk:
	case VPUNPCKLQDQZ256rmbkz:
	case VPUNPCKLQDQZ256rmk:
	case VPUNPCKLQDQZ256rmkz:
	case VPUNPCKLQDQZ256rr:
	case VPUNPCKLQDQZ256rrk:
	case VPUNPCKLQDQZ256rrkz:
	case VPUNPCKLQDQZrm:
	case VPUNPCKLQDQZrmb:
	case VPUNPCKLQDQZrmbk:
	case VPUNPCKLQDQZrmbkz:
	case VPUNPCKLQDQZrmk:
	case VPUNPCKLQDQZrmkz:
	case VPUNPCKLQDQZrr:
	case VPUNPCKLQDQZrrk:
	case VPUNPCKLQDQZrrkz:
	case VPUNPCKLQDQrm:
	case VPUNPCKLQDQrr:
		return true;
	}
	return false;
}

bool isVCVTPD2DQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPD2DQYrm:
	case VCVTPD2DQYrr:
	case VCVTPD2DQZ128rm:
	case VCVTPD2DQZ128rmb:
	case VCVTPD2DQZ128rmbk:
	case VCVTPD2DQZ128rmbkz:
	case VCVTPD2DQZ128rmk:
	case VCVTPD2DQZ128rmkz:
	case VCVTPD2DQZ128rr:
	case VCVTPD2DQZ128rrk:
	case VCVTPD2DQZ128rrkz:
	case VCVTPD2DQZ256rm:
	case VCVTPD2DQZ256rmb:
	case VCVTPD2DQZ256rmbk:
	case VCVTPD2DQZ256rmbkz:
	case VCVTPD2DQZ256rmk:
	case VCVTPD2DQZ256rmkz:
	case VCVTPD2DQZ256rr:
	case VCVTPD2DQZ256rrk:
	case VCVTPD2DQZ256rrkz:
	case VCVTPD2DQZrm:
	case VCVTPD2DQZrmb:
	case VCVTPD2DQZrmbk:
	case VCVTPD2DQZrmbkz:
	case VCVTPD2DQZrmk:
	case VCVTPD2DQZrmkz:
	case VCVTPD2DQZrr:
	case VCVTPD2DQZrrb:
	case VCVTPD2DQZrrbk:
	case VCVTPD2DQZrrbkz:
	case VCVTPD2DQZrrk:
	case VCVTPD2DQZrrkz:
	case VCVTPD2DQrm:
	case VCVTPD2DQrr:
		return true;
	}
	return false;
}

bool isPMULLW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMULLWrm:
	case MMX_PMULLWrr:
	case PMULLWrm:
	case PMULLWrr:
		return true;
	}
	return false;
}

bool isVPBLENDW(unsigned Opcode) {
	switch (Opcode) {
	case VPBLENDWYrmi:
	case VPBLENDWYrri:
	case VPBLENDWrmi:
	case VPBLENDWrri:
		return true;
	}
	return false;
}

bool isAESENCLAST(unsigned Opcode) {
	switch (Opcode) {
	case AESENCLASTrm:
	case AESENCLASTrr:
		return true;
	}
	return false;
}

bool isVPUNPCKHDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKHDQYrm:
	case VPUNPCKHDQYrr:
	case VPUNPCKHDQZ128rm:
	case VPUNPCKHDQZ128rmb:
	case VPUNPCKHDQZ128rmbk:
	case VPUNPCKHDQZ128rmbkz:
	case VPUNPCKHDQZ128rmk:
	case VPUNPCKHDQZ128rmkz:
	case VPUNPCKHDQZ128rr:
	case VPUNPCKHDQZ128rrk:
	case VPUNPCKHDQZ128rrkz:
	case VPUNPCKHDQZ256rm:
	case VPUNPCKHDQZ256rmb:
	case VPUNPCKHDQZ256rmbk:
	case VPUNPCKHDQZ256rmbkz:
	case VPUNPCKHDQZ256rmk:
	case VPUNPCKHDQZ256rmkz:
	case VPUNPCKHDQZ256rr:
	case VPUNPCKHDQZ256rrk:
	case VPUNPCKHDQZ256rrkz:
	case VPUNPCKHDQZrm:
	case VPUNPCKHDQZrmb:
	case VPUNPCKHDQZrmbk:
	case VPUNPCKHDQZrmbkz:
	case VPUNPCKHDQZrmk:
	case VPUNPCKHDQZrmkz:
	case VPUNPCKHDQZrr:
	case VPUNPCKHDQZrrk:
	case VPUNPCKHDQZrrkz:
	case VPUNPCKHDQrm:
	case VPUNPCKHDQrr:
		return true;
	}
	return false;
}

bool isVFNMADD231SD(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD231SDZm_Int:
	case VFNMADD231SDZm_Intk:
	case VFNMADD231SDZm_Intkz:
	case VFNMADD231SDZr_Int:
	case VFNMADD231SDZr_Intk:
	case VFNMADD231SDZr_Intkz:
	case VFNMADD231SDZrb_Int:
	case VFNMADD231SDZrb_Intk:
	case VFNMADD231SDZrb_Intkz:
	case VFNMADD231SDm_Int:
	case VFNMADD231SDr_Int:
		return true;
	}
	return false;
}

bool isVFNMADD231SH(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD231SHZm_Int:
	case VFNMADD231SHZm_Intk:
	case VFNMADD231SHZm_Intkz:
	case VFNMADD231SHZr_Int:
	case VFNMADD231SHZr_Intk:
	case VFNMADD231SHZr_Intkz:
	case VFNMADD231SHZrb_Int:
	case VFNMADD231SHZrb_Intk:
	case VFNMADD231SHZrb_Intkz:
		return true;
	}
	return false;
}

bool isVPMOVSDB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSDBZ128mr:
	case VPMOVSDBZ128mrk:
	case VPMOVSDBZ128rr:
	case VPMOVSDBZ128rrk:
	case VPMOVSDBZ128rrkz:
	case VPMOVSDBZ256mr:
	case VPMOVSDBZ256mrk:
	case VPMOVSDBZ256rr:
	case VPMOVSDBZ256rrk:
	case VPMOVSDBZ256rrkz:
	case VPMOVSDBZmr:
	case VPMOVSDBZmrk:
	case VPMOVSDBZrr:
	case VPMOVSDBZrrk:
	case VPMOVSDBZrrkz:
		return true;
	}
	return false;
}

bool isVMAXSD(unsigned Opcode) {
	switch (Opcode) {
	case VMAXSDZrm_Int:
	case VMAXSDZrm_Intk:
	case VMAXSDZrm_Intkz:
	case VMAXSDZrr_Int:
	case VMAXSDZrr_Intk:
	case VMAXSDZrr_Intkz:
	case VMAXSDZrrb_Int:
	case VMAXSDZrrb_Intk:
	case VMAXSDZrrb_Intkz:
	case VMAXSDrm_Int:
	case VMAXSDrr_Int:
		return true;
	}
	return false;
}

bool isVMAXSH(unsigned Opcode) {
	switch (Opcode) {
	case VMAXSHZrm_Int:
	case VMAXSHZrm_Intk:
	case VMAXSHZrm_Intkz:
	case VMAXSHZrr_Int:
	case VMAXSHZrr_Intk:
	case VMAXSHZrr_Intkz:
	case VMAXSHZrrb_Int:
	case VMAXSHZrrb_Intk:
	case VMAXSHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isLAHF(unsigned Opcode) {
	return Opcode == LAHF;
}

bool isCVTTPD2DQ(unsigned Opcode) {
	switch (Opcode) {
	case CVTTPD2DQrm:
	case CVTTPD2DQrr:
		return true;
	}
	return false;
}

bool isUNPCKHPD(unsigned Opcode) {
	switch (Opcode) {
	case UNPCKHPDrm:
	case UNPCKHPDrr:
		return true;
	}
	return false;
}

bool isVFNMADD231SS(unsigned Opcode) {
	switch (Opcode) {
	case VFNMADD231SSZm_Int:
	case VFNMADD231SSZm_Intk:
	case VFNMADD231SSZm_Intkz:
	case VFNMADD231SSZr_Int:
	case VFNMADD231SSZr_Intk:
	case VFNMADD231SSZr_Intkz:
	case VFNMADD231SSZrb_Int:
	case VFNMADD231SSZrb_Intk:
	case VFNMADD231SSZrb_Intkz:
	case VFNMADD231SSm_Int:
	case VFNMADD231SSr_Int:
		return true;
	}
	return false;
}

bool isVMAXSS(unsigned Opcode) {
	switch (Opcode) {
	case VMAXSSZrm_Int:
	case VMAXSSZrm_Intk:
	case VMAXSSZrm_Intkz:
	case VMAXSSZrr_Int:
	case VMAXSSZrr_Intk:
	case VMAXSSZrr_Intkz:
	case VMAXSSZrrb_Int:
	case VMAXSSZrrb_Intk:
	case VMAXSSZrrb_Intkz:
	case VMAXSSrm_Int:
	case VMAXSSrr_Int:
		return true;
	}
	return false;
}

bool isVPMACSSDD(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSSDDrm:
	case VPMACSSDDrr:
		return true;
	}
	return false;
}

bool isVPMOVSDW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSDWZ128mr:
	case VPMOVSDWZ128mrk:
	case VPMOVSDWZ128rr:
	case VPMOVSDWZ128rrk:
	case VPMOVSDWZ128rrkz:
	case VPMOVSDWZ256mr:
	case VPMOVSDWZ256mrk:
	case VPMOVSDWZ256rr:
	case VPMOVSDWZ256rrk:
	case VPMOVSDWZ256rrkz:
	case VPMOVSDWZmr:
	case VPMOVSDWZmrk:
	case VPMOVSDWZrr:
	case VPMOVSDWZrrk:
	case VPMOVSDWZrrkz:
		return true;
	}
	return false;
}

bool isPMOVZXDQ(unsigned Opcode) {
	switch (Opcode) {
	case PMOVZXDQrm:
	case PMOVZXDQrr:
		return true;
	}
	return false;
}

bool isUNPCKHPS(unsigned Opcode) {
	switch (Opcode) {
	case UNPCKHPSrm:
	case UNPCKHPSrr:
		return true;
	}
	return false;
}

bool isFCOMI(unsigned Opcode) {
	return Opcode == COM_FIr;
}

bool isCLFLUSH(unsigned Opcode) {
	return Opcode == CLFLUSH;
}

bool isTILELOADD(unsigned Opcode) {
	return Opcode == TILELOADD;
}

bool isFCOMP(unsigned Opcode) {
	switch (Opcode) {
	case COMP_FST0r:
	case FCOMP32m:
	case FCOMP64m:
		return true;
	}
	return false;
}

bool isDIV(unsigned Opcode) {
	switch (Opcode) {
	case DIV16m:
	case DIV16r:
	case DIV32m:
	case DIV32r:
	case DIV64m:
	case DIV64r:
	case DIV8m:
	case DIV8r:
		return true;
	}
	return false;
}

bool isVPMACSWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSWDrm:
	case VPMACSWDrr:
		return true;
	}
	return false;
}

bool isFPREM1(unsigned Opcode) {
	return Opcode == FPREM1;
}

bool isVSCATTERQPD(unsigned Opcode) {
	switch (Opcode) {
	case VSCATTERQPDZ128mr:
	case VSCATTERQPDZ256mr:
	case VSCATTERQPDZmr:
		return true;
	}
	return false;
}

bool isFYL2X(unsigned Opcode) {
	return Opcode == FYL2X;
}

bool isVPMACSWW(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSWWrm:
	case VPMACSWWrr:
		return true;
	}
	return false;
}

bool isPFMAX(unsigned Opcode) {
	switch (Opcode) {
	case PFMAXrm:
	case PFMAXrr:
		return true;
	}
	return false;
}

bool isVPSHUFB(unsigned Opcode) {
	switch (Opcode) {
	case VPSHUFBYrm:
	case VPSHUFBYrr:
	case VPSHUFBZ128rm:
	case VPSHUFBZ128rmk:
	case VPSHUFBZ128rmkz:
	case VPSHUFBZ128rr:
	case VPSHUFBZ128rrk:
	case VPSHUFBZ128rrkz:
	case VPSHUFBZ256rm:
	case VPSHUFBZ256rmk:
	case VPSHUFBZ256rmkz:
	case VPSHUFBZ256rr:
	case VPSHUFBZ256rrk:
	case VPSHUFBZ256rrkz:
	case VPSHUFBZrm:
	case VPSHUFBZrmk:
	case VPSHUFBZrmkz:
	case VPSHUFBZrr:
	case VPSHUFBZrrk:
	case VPSHUFBZrrkz:
	case VPSHUFBrm:
	case VPSHUFBrr:
		return true;
	}
	return false;
}

bool isVCVTPD2UQQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPD2UQQZ128rm:
	case VCVTPD2UQQZ128rmb:
	case VCVTPD2UQQZ128rmbk:
	case VCVTPD2UQQZ128rmbkz:
	case VCVTPD2UQQZ128rmk:
	case VCVTPD2UQQZ128rmkz:
	case VCVTPD2UQQZ128rr:
	case VCVTPD2UQQZ128rrk:
	case VCVTPD2UQQZ128rrkz:
	case VCVTPD2UQQZ256rm:
	case VCVTPD2UQQZ256rmb:
	case VCVTPD2UQQZ256rmbk:
	case VCVTPD2UQQZ256rmbkz:
	case VCVTPD2UQQZ256rmk:
	case VCVTPD2UQQZ256rmkz:
	case VCVTPD2UQQZ256rr:
	case VCVTPD2UQQZ256rrk:
	case VCVTPD2UQQZ256rrkz:
	case VCVTPD2UQQZrm:
	case VCVTPD2UQQZrmb:
	case VCVTPD2UQQZrmbk:
	case VCVTPD2UQQZrmbkz:
	case VCVTPD2UQQZrmk:
	case VCVTPD2UQQZrmkz:
	case VCVTPD2UQQZrr:
	case VCVTPD2UQQZrrb:
	case VCVTPD2UQQZrrbk:
	case VCVTPD2UQQZrrbkz:
	case VCVTPD2UQQZrrk:
	case VCVTPD2UQQZrrkz:
		return true;
	}
	return false;
}

bool isVPSHUFD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHUFDYmi:
	case VPSHUFDYri:
	case VPSHUFDZ128mbi:
	case VPSHUFDZ128mbik:
	case VPSHUFDZ128mbikz:
	case VPSHUFDZ128mi:
	case VPSHUFDZ128mik:
	case VPSHUFDZ128mikz:
	case VPSHUFDZ128ri:
	case VPSHUFDZ128rik:
	case VPSHUFDZ128rikz:
	case VPSHUFDZ256mbi:
	case VPSHUFDZ256mbik:
	case VPSHUFDZ256mbikz:
	case VPSHUFDZ256mi:
	case VPSHUFDZ256mik:
	case VPSHUFDZ256mikz:
	case VPSHUFDZ256ri:
	case VPSHUFDZ256rik:
	case VPSHUFDZ256rikz:
	case VPSHUFDZmbi:
	case VPSHUFDZmbik:
	case VPSHUFDZmbikz:
	case VPSHUFDZmi:
	case VPSHUFDZmik:
	case VPSHUFDZmikz:
	case VPSHUFDZri:
	case VPSHUFDZrik:
	case VPSHUFDZrikz:
	case VPSHUFDmi:
	case VPSHUFDri:
		return true;
	}
	return false;
}

bool isVSCATTERQPS(unsigned Opcode) {
	switch (Opcode) {
	case VSCATTERQPSZ128mr:
	case VSCATTERQPSZ256mr:
	case VSCATTERQPSZmr:
		return true;
	}
	return false;
}

bool isGF2P8AFFINEINVQB(unsigned Opcode) {
	switch (Opcode) {
	case GF2P8AFFINEINVQBrmi:
	case GF2P8AFFINEINVQBrri:
		return true;
	}
	return false;
}

bool isFEMMS(unsigned Opcode) {
	return Opcode == FEMMS;
}

bool isKUNPCKWD(unsigned Opcode) {
	return Opcode == KUNPCKWDrr;
}

bool isVPCLMULQDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCLMULQDQYrm:
	case VPCLMULQDQYrr:
	case VPCLMULQDQZ128rm:
	case VPCLMULQDQZ128rr:
	case VPCLMULQDQZ256rm:
	case VPCLMULQDQZ256rr:
	case VPCLMULQDQZrm:
	case VPCLMULQDQZrr:
	case VPCLMULQDQrm:
	case VPCLMULQDQrr:
		return true;
	}
	return false;
}

bool isKORB(unsigned Opcode) {
	return Opcode == KORBrr;
}

bool isVINSERTPS(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTPSZrm:
	case VINSERTPSZrr:
	case VINSERTPSrm:
	case VINSERTPSrr:
		return true;
	}
	return false;
}

bool isKORD(unsigned Opcode) {
	return Opcode == KORDrr;
}

bool isVMFUNC(unsigned Opcode) {
	return Opcode == VMFUNC;
}

bool isCPUID(unsigned Opcode) {
	return Opcode == CPUID;
}

bool isVSCATTERPF0QPD(unsigned Opcode) {
	return Opcode == VSCATTERPF0QPDm;
}

bool isOUT(unsigned Opcode) {
	switch (Opcode) {
	case OUT16ir:
	case OUT16rr:
	case OUT32ir:
	case OUT32rr:
	case OUT8ir:
	case OUT8rr:
		return true;
	}
	return false;
}

bool isKORQ(unsigned Opcode) {
	return Opcode == KORQrr;
}

bool isKORW(unsigned Opcode) {
	return Opcode == KORWrr;
}

bool isVSCATTERPF0QPS(unsigned Opcode) {
	return Opcode == VSCATTERPF0QPSm;
}

bool isPHSUBSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PHSUBSWrm:
	case MMX_PHSUBSWrr:
	case PHSUBSWrm:
	case PHSUBSWrr:
		return true;
	}
	return false;
}

bool isPFSUBR(unsigned Opcode) {
	switch (Opcode) {
	case PFSUBRrm:
	case PFSUBRrr:
		return true;
	}
	return false;
}

bool isVCVTPH2UDQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2UDQZ128rm:
	case VCVTPH2UDQZ128rmb:
	case VCVTPH2UDQZ128rmbk:
	case VCVTPH2UDQZ128rmbkz:
	case VCVTPH2UDQZ128rmk:
	case VCVTPH2UDQZ128rmkz:
	case VCVTPH2UDQZ128rr:
	case VCVTPH2UDQZ128rrk:
	case VCVTPH2UDQZ128rrkz:
	case VCVTPH2UDQZ256rm:
	case VCVTPH2UDQZ256rmb:
	case VCVTPH2UDQZ256rmbk:
	case VCVTPH2UDQZ256rmbkz:
	case VCVTPH2UDQZ256rmk:
	case VCVTPH2UDQZ256rmkz:
	case VCVTPH2UDQZ256rr:
	case VCVTPH2UDQZ256rrk:
	case VCVTPH2UDQZ256rrkz:
	case VCVTPH2UDQZrm:
	case VCVTPH2UDQZrmb:
	case VCVTPH2UDQZrmbk:
	case VCVTPH2UDQZrmbkz:
	case VCVTPH2UDQZrmk:
	case VCVTPH2UDQZrmkz:
	case VCVTPH2UDQZrr:
	case VCVTPH2UDQZrrb:
	case VCVTPH2UDQZrrbk:
	case VCVTPH2UDQZrrbkz:
	case VCVTPH2UDQZrrk:
	case VCVTPH2UDQZrrkz:
		return true;
	}
	return false;
}

bool isVBROADCASTF32X2(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTF32X2Z256rm:
	case VBROADCASTF32X2Z256rmk:
	case VBROADCASTF32X2Z256rmkz:
	case VBROADCASTF32X2Z256rr:
	case VBROADCASTF32X2Z256rrk:
	case VBROADCASTF32X2Z256rrkz:
	case VBROADCASTF32X2Zrm:
	case VBROADCASTF32X2Zrmk:
	case VBROADCASTF32X2Zrmkz:
	case VBROADCASTF32X2Zrr:
	case VBROADCASTF32X2Zrrk:
	case VBROADCASTF32X2Zrrkz:
		return true;
	}
	return false;
}

bool isVBROADCASTF32X4(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTF32X4Z256rm:
	case VBROADCASTF32X4Z256rmk:
	case VBROADCASTF32X4Z256rmkz:
	case VBROADCASTF32X4rm:
	case VBROADCASTF32X4rmk:
	case VBROADCASTF32X4rmkz:
		return true;
	}
	return false;
}

bool isSHA256RNDS2(unsigned Opcode) {
	switch (Opcode) {
	case SHA256RNDS2rm:
	case SHA256RNDS2rr:
		return true;
	}
	return false;
}

bool isVBROADCASTF32X8(unsigned Opcode) {
	switch (Opcode) {
	case VBROADCASTF32X8rm:
	case VBROADCASTF32X8rmk:
	case VBROADCASTF32X8rmkz:
		return true;
	}
	return false;
}

bool isVPERMB(unsigned Opcode) {
	switch (Opcode) {
	case VPERMBZ128rm:
	case VPERMBZ128rmk:
	case VPERMBZ128rmkz:
	case VPERMBZ128rr:
	case VPERMBZ128rrk:
	case VPERMBZ128rrkz:
	case VPERMBZ256rm:
	case VPERMBZ256rmk:
	case VPERMBZ256rmkz:
	case VPERMBZ256rr:
	case VPERMBZ256rrk:
	case VPERMBZ256rrkz:
	case VPERMBZrm:
	case VPERMBZrmk:
	case VPERMBZrmkz:
	case VPERMBZrr:
	case VPERMBZrrk:
	case VPERMBZrrkz:
		return true;
	}
	return false;
}

bool isVPUNPCKLBW(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKLBWYrm:
	case VPUNPCKLBWYrr:
	case VPUNPCKLBWZ128rm:
	case VPUNPCKLBWZ128rmk:
	case VPUNPCKLBWZ128rmkz:
	case VPUNPCKLBWZ128rr:
	case VPUNPCKLBWZ128rrk:
	case VPUNPCKLBWZ128rrkz:
	case VPUNPCKLBWZ256rm:
	case VPUNPCKLBWZ256rmk:
	case VPUNPCKLBWZ256rmkz:
	case VPUNPCKLBWZ256rr:
	case VPUNPCKLBWZ256rrk:
	case VPUNPCKLBWZ256rrkz:
	case VPUNPCKLBWZrm:
	case VPUNPCKLBWZrmk:
	case VPUNPCKLBWZrmkz:
	case VPUNPCKLBWZrr:
	case VPUNPCKLBWZrrk:
	case VPUNPCKLBWZrrkz:
	case VPUNPCKLBWrm:
	case VPUNPCKLBWrr:
		return true;
	}
	return false;
}

bool isVPERMD(unsigned Opcode) {
	switch (Opcode) {
	case VPERMDYrm:
	case VPERMDYrr:
	case VPERMDZ256rm:
	case VPERMDZ256rmb:
	case VPERMDZ256rmbk:
	case VPERMDZ256rmbkz:
	case VPERMDZ256rmk:
	case VPERMDZ256rmkz:
	case VPERMDZ256rr:
	case VPERMDZ256rrk:
	case VPERMDZ256rrkz:
	case VPERMDZrm:
	case VPERMDZrmb:
	case VPERMDZrmbk:
	case VPERMDZrmbkz:
	case VPERMDZrmk:
	case VPERMDZrmkz:
	case VPERMDZrr:
	case VPERMDZrrk:
	case VPERMDZrrkz:
		return true;
	}
	return false;
}

bool isXCRYPTECB(unsigned Opcode) {
	return Opcode == XCRYPTECB;
}

bool isVPERMQ(unsigned Opcode) {
	switch (Opcode) {
	case VPERMQYmi:
	case VPERMQYri:
	case VPERMQZ256mbi:
	case VPERMQZ256mbik:
	case VPERMQZ256mbikz:
	case VPERMQZ256mi:
	case VPERMQZ256mik:
	case VPERMQZ256mikz:
	case VPERMQZ256ri:
	case VPERMQZ256rik:
	case VPERMQZ256rikz:
	case VPERMQZ256rm:
	case VPERMQZ256rmb:
	case VPERMQZ256rmbk:
	case VPERMQZ256rmbkz:
	case VPERMQZ256rmk:
	case VPERMQZ256rmkz:
	case VPERMQZ256rr:
	case VPERMQZ256rrk:
	case VPERMQZ256rrkz:
	case VPERMQZmbi:
	case VPERMQZmbik:
	case VPERMQZmbikz:
	case VPERMQZmi:
	case VPERMQZmik:
	case VPERMQZmikz:
	case VPERMQZri:
	case VPERMQZrik:
	case VPERMQZrikz:
	case VPERMQZrm:
	case VPERMQZrmb:
	case VPERMQZrmbk:
	case VPERMQZrmbkz:
	case VPERMQZrmk:
	case VPERMQZrmkz:
	case VPERMQZrr:
	case VPERMQZrrk:
	case VPERMQZrrkz:
		return true;
	}
	return false;
}

bool isEXTRACTPS(unsigned Opcode) {
	switch (Opcode) {
	case EXTRACTPSmr:
	case EXTRACTPSrr:
		return true;
	}
	return false;
}

bool isVPERMW(unsigned Opcode) {
	switch (Opcode) {
	case VPERMWZ128rm:
	case VPERMWZ128rmk:
	case VPERMWZ128rmkz:
	case VPERMWZ128rr:
	case VPERMWZ128rrk:
	case VPERMWZ128rrkz:
	case VPERMWZ256rm:
	case VPERMWZ256rmk:
	case VPERMWZ256rmkz:
	case VPERMWZ256rr:
	case VPERMWZ256rrk:
	case VPERMWZ256rrkz:
	case VPERMWZrm:
	case VPERMWZrmk:
	case VPERMWZrmkz:
	case VPERMWZrr:
	case VPERMWZrrk:
	case VPERMWZrrkz:
		return true;
	}
	return false;
}

bool isHADDPD(unsigned Opcode) {
	switch (Opcode) {
	case HADDPDrm:
	case HADDPDrr:
		return true;
	}
	return false;
}

bool isFXSAVE(unsigned Opcode) {
	return Opcode == FXSAVE;
}

bool isVRCPPH(unsigned Opcode) {
	switch (Opcode) {
	case VRCPPHZ128m:
	case VRCPPHZ128mb:
	case VRCPPHZ128mbk:
	case VRCPPHZ128mbkz:
	case VRCPPHZ128mk:
	case VRCPPHZ128mkz:
	case VRCPPHZ128r:
	case VRCPPHZ128rk:
	case VRCPPHZ128rkz:
	case VRCPPHZ256m:
	case VRCPPHZ256mb:
	case VRCPPHZ256mbk:
	case VRCPPHZ256mbkz:
	case VRCPPHZ256mk:
	case VRCPPHZ256mkz:
	case VRCPPHZ256r:
	case VRCPPHZ256rk:
	case VRCPPHZ256rkz:
	case VRCPPHZm:
	case VRCPPHZmb:
	case VRCPPHZmbk:
	case VRCPPHZmbkz:
	case VRCPPHZmk:
	case VRCPPHZmkz:
	case VRCPPHZr:
	case VRCPPHZrk:
	case VRCPPHZrkz:
		return true;
	}
	return false;
}

bool isHADDPS(unsigned Opcode) {
	switch (Opcode) {
	case HADDPSrm:
	case HADDPSrr:
		return true;
	}
	return false;
}

bool isVPSADBW(unsigned Opcode) {
	switch (Opcode) {
	case VPSADBWYrm:
	case VPSADBWYrr:
	case VPSADBWZ128rm:
	case VPSADBWZ128rr:
	case VPSADBWZ256rm:
	case VPSADBWZ256rr:
	case VPSADBWZrm:
	case VPSADBWZrr:
	case VPSADBWrm:
	case VPSADBWrr:
		return true;
	}
	return false;
}

bool isRDRAND(unsigned Opcode) {
	switch (Opcode) {
	case RDRAND16r:
	case RDRAND32r:
	case RDRAND64r:
		return true;
	}
	return false;
}

bool isVRCPPS(unsigned Opcode) {
	switch (Opcode) {
	case VRCPPSYm:
	case VRCPPSYr:
	case VRCPPSm:
	case VRCPPSr:
		return true;
	}
	return false;
}

bool isVXORPD(unsigned Opcode) {
	switch (Opcode) {
	case VXORPDYrm:
	case VXORPDYrr:
	case VXORPDZ128rm:
	case VXORPDZ128rmb:
	case VXORPDZ128rmbk:
	case VXORPDZ128rmbkz:
	case VXORPDZ128rmk:
	case VXORPDZ128rmkz:
	case VXORPDZ128rr:
	case VXORPDZ128rrk:
	case VXORPDZ128rrkz:
	case VXORPDZ256rm:
	case VXORPDZ256rmb:
	case VXORPDZ256rmbk:
	case VXORPDZ256rmbkz:
	case VXORPDZ256rmk:
	case VXORPDZ256rmkz:
	case VXORPDZ256rr:
	case VXORPDZ256rrk:
	case VXORPDZ256rrkz:
	case VXORPDZrm:
	case VXORPDZrmb:
	case VXORPDZrmbk:
	case VXORPDZrmbkz:
	case VXORPDZrmk:
	case VXORPDZrmkz:
	case VXORPDZrr:
	case VXORPDZrrk:
	case VXORPDZrrkz:
	case VXORPDrm:
	case VXORPDrr:
		return true;
	}
	return false;
}

bool isFFREE(unsigned Opcode) {
	return Opcode == FFREE;
}

bool isPCMPEQB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PCMPEQBrm:
	case MMX_PCMPEQBrr:
	case PCMPEQBrm:
	case PCMPEQBrr:
		return true;
	}
	return false;
}

bool isPCMPEQD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PCMPEQDrm:
	case MMX_PCMPEQDrr:
	case PCMPEQDrm:
	case PCMPEQDrr:
		return true;
	}
	return false;
}

bool isVPUNPCKLDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKLDQYrm:
	case VPUNPCKLDQYrr:
	case VPUNPCKLDQZ128rm:
	case VPUNPCKLDQZ128rmb:
	case VPUNPCKLDQZ128rmbk:
	case VPUNPCKLDQZ128rmbkz:
	case VPUNPCKLDQZ128rmk:
	case VPUNPCKLDQZ128rmkz:
	case VPUNPCKLDQZ128rr:
	case VPUNPCKLDQZ128rrk:
	case VPUNPCKLDQZ128rrkz:
	case VPUNPCKLDQZ256rm:
	case VPUNPCKLDQZ256rmb:
	case VPUNPCKLDQZ256rmbk:
	case VPUNPCKLDQZ256rmbkz:
	case VPUNPCKLDQZ256rmk:
	case VPUNPCKLDQZ256rmkz:
	case VPUNPCKLDQZ256rr:
	case VPUNPCKLDQZ256rrk:
	case VPUNPCKLDQZ256rrkz:
	case VPUNPCKLDQZrm:
	case VPUNPCKLDQZrmb:
	case VPUNPCKLDQZrmbk:
	case VPUNPCKLDQZrmbkz:
	case VPUNPCKLDQZrmk:
	case VPUNPCKLDQZrmkz:
	case VPUNPCKLDQZrr:
	case VPUNPCKLDQZrrk:
	case VPUNPCKLDQZrrkz:
	case VPUNPCKLDQrm:
	case VPUNPCKLDQrr:
		return true;
	}
	return false;
}

bool isVTESTPD(unsigned Opcode) {
	switch (Opcode) {
	case VTESTPDYrm:
	case VTESTPDYrr:
	case VTESTPDrm:
	case VTESTPDrr:
		return true;
	}
	return false;
}

bool isVXORPS(unsigned Opcode) {
	switch (Opcode) {
	case VXORPSYrm:
	case VXORPSYrr:
	case VXORPSZ128rm:
	case VXORPSZ128rmb:
	case VXORPSZ128rmbk:
	case VXORPSZ128rmbkz:
	case VXORPSZ128rmk:
	case VXORPSZ128rmkz:
	case VXORPSZ128rr:
	case VXORPSZ128rrk:
	case VXORPSZ128rrkz:
	case VXORPSZ256rm:
	case VXORPSZ256rmb:
	case VXORPSZ256rmbk:
	case VXORPSZ256rmbkz:
	case VXORPSZ256rmk:
	case VXORPSZ256rmkz:
	case VXORPSZ256rr:
	case VXORPSZ256rrk:
	case VXORPSZ256rrkz:
	case VXORPSZrm:
	case VXORPSZrmb:
	case VXORPSZrmbk:
	case VXORPSZrmbkz:
	case VXORPSZrmk:
	case VXORPSZrmkz:
	case VXORPSZrr:
	case VXORPSZrrk:
	case VXORPSZrrkz:
	case VXORPSrm:
	case VXORPSrr:
		return true;
	}
	return false;
}

bool isTZCNT(unsigned Opcode) {
	switch (Opcode) {
	case TZCNT16rm:
	case TZCNT16rr:
	case TZCNT32rm:
	case TZCNT32rr:
	case TZCNT64rm:
	case TZCNT64rr:
		return true;
	}
	return false;
}

bool isCLTS(unsigned Opcode) {
	return Opcode == CLTS;
}

bool isPCMPEQQ(unsigned Opcode) {
	switch (Opcode) {
	case PCMPEQQrm:
	case PCMPEQQrr:
		return true;
	}
	return false;
}

bool isVFPCLASSPD(unsigned Opcode) {
	switch (Opcode) {
	case VFPCLASSPDZ128rm:
	case VFPCLASSPDZ128rmb:
	case VFPCLASSPDZ128rmbk:
	case VFPCLASSPDZ128rmk:
	case VFPCLASSPDZ128rr:
	case VFPCLASSPDZ128rrk:
	case VFPCLASSPDZ256rm:
	case VFPCLASSPDZ256rmb:
	case VFPCLASSPDZ256rmbk:
	case VFPCLASSPDZ256rmk:
	case VFPCLASSPDZ256rr:
	case VFPCLASSPDZ256rrk:
	case VFPCLASSPDZrm:
	case VFPCLASSPDZrmb:
	case VFPCLASSPDZrmbk:
	case VFPCLASSPDZrmk:
	case VFPCLASSPDZrr:
	case VFPCLASSPDZrrk:
		return true;
	}
	return false;
}

bool isVFPCLASSPH(unsigned Opcode) {
	switch (Opcode) {
	case VFPCLASSPHZ128rm:
	case VFPCLASSPHZ128rmb:
	case VFPCLASSPHZ128rmbk:
	case VFPCLASSPHZ128rmk:
	case VFPCLASSPHZ128rr:
	case VFPCLASSPHZ128rrk:
	case VFPCLASSPHZ256rm:
	case VFPCLASSPHZ256rmb:
	case VFPCLASSPHZ256rmbk:
	case VFPCLASSPHZ256rmk:
	case VFPCLASSPHZ256rr:
	case VFPCLASSPHZ256rrk:
	case VFPCLASSPHZrm:
	case VFPCLASSPHZrmb:
	case VFPCLASSPHZrmbk:
	case VFPCLASSPHZrmk:
	case VFPCLASSPHZrr:
	case VFPCLASSPHZrrk:
		return true;
	}
	return false;
}

bool isVTESTPS(unsigned Opcode) {
	switch (Opcode) {
	case VTESTPSYrm:
	case VTESTPSYrr:
	case VTESTPSrm:
	case VTESTPSrr:
		return true;
	}
	return false;
}

bool isPCMPEQW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PCMPEQWrm:
	case MMX_PCMPEQWrr:
	case PCMPEQWrm:
	case PCMPEQWrr:
		return true;
	}
	return false;
}

bool isUNPCKLPD(unsigned Opcode) {
	switch (Opcode) {
	case UNPCKLPDrm:
	case UNPCKLPDrr:
		return true;
	}
	return false;
}

bool isVMOVDQA32(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQA32Z128mr:
	case VMOVDQA32Z128mrk:
	case VMOVDQA32Z128rm:
	case VMOVDQA32Z128rmk:
	case VMOVDQA32Z128rmkz:
	case VMOVDQA32Z128rr:
	case VMOVDQA32Z128rr_REV:
	case VMOVDQA32Z128rrk:
	case VMOVDQA32Z128rrk_REV:
	case VMOVDQA32Z128rrkz:
	case VMOVDQA32Z128rrkz_REV:
	case VMOVDQA32Z256mr:
	case VMOVDQA32Z256mrk:
	case VMOVDQA32Z256rm:
	case VMOVDQA32Z256rmk:
	case VMOVDQA32Z256rmkz:
	case VMOVDQA32Z256rr:
	case VMOVDQA32Z256rr_REV:
	case VMOVDQA32Z256rrk:
	case VMOVDQA32Z256rrk_REV:
	case VMOVDQA32Z256rrkz:
	case VMOVDQA32Z256rrkz_REV:
	case VMOVDQA32Zmr:
	case VMOVDQA32Zmrk:
	case VMOVDQA32Zrm:
	case VMOVDQA32Zrmk:
	case VMOVDQA32Zrmkz:
	case VMOVDQA32Zrr:
	case VMOVDQA32Zrr_REV:
	case VMOVDQA32Zrrk:
	case VMOVDQA32Zrrk_REV:
	case VMOVDQA32Zrrkz:
	case VMOVDQA32Zrrkz_REV:
		return true;
	}
	return false;
}

bool isVFPCLASSPS(unsigned Opcode) {
	switch (Opcode) {
	case VFPCLASSPSZ128rm:
	case VFPCLASSPSZ128rmb:
	case VFPCLASSPSZ128rmbk:
	case VFPCLASSPSZ128rmk:
	case VFPCLASSPSZ128rr:
	case VFPCLASSPSZ128rrk:
	case VFPCLASSPSZ256rm:
	case VFPCLASSPSZ256rmb:
	case VFPCLASSPSZ256rmbk:
	case VFPCLASSPSZ256rmk:
	case VFPCLASSPSZ256rr:
	case VFPCLASSPSZ256rrk:
	case VFPCLASSPSZrm:
	case VFPCLASSPSZrmb:
	case VFPCLASSPSZrmbk:
	case VFPCLASSPSZrmk:
	case VFPCLASSPSZrr:
	case VFPCLASSPSZrrk:
		return true;
	}
	return false;
}

bool isVPMOVMSKB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVMSKBYrr:
	case VPMOVMSKBrr:
		return true;
	}
	return false;
}

bool isFDECSTP(unsigned Opcode) {
	return Opcode == FDECSTP;
}

bool isCLUI(unsigned Opcode) {
	return Opcode == CLUI;
}

bool isFLDPI(unsigned Opcode) {
	return Opcode == FLDPI;
}

bool isUNPCKLPS(unsigned Opcode) {
	switch (Opcode) {
	case UNPCKLPSrm:
	case UNPCKLPSrr:
		return true;
	}
	return false;
}

bool isVCVTTPD2DQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPD2DQYrm:
	case VCVTTPD2DQYrr:
	case VCVTTPD2DQZ128rm:
	case VCVTTPD2DQZ128rmb:
	case VCVTTPD2DQZ128rmbk:
	case VCVTTPD2DQZ128rmbkz:
	case VCVTTPD2DQZ128rmk:
	case VCVTTPD2DQZ128rmkz:
	case VCVTTPD2DQZ128rr:
	case VCVTTPD2DQZ128rrk:
	case VCVTTPD2DQZ128rrkz:
	case VCVTTPD2DQZ256rm:
	case VCVTTPD2DQZ256rmb:
	case VCVTTPD2DQZ256rmbk:
	case VCVTTPD2DQZ256rmbkz:
	case VCVTTPD2DQZ256rmk:
	case VCVTTPD2DQZ256rmkz:
	case VCVTTPD2DQZ256rr:
	case VCVTTPD2DQZ256rrk:
	case VCVTTPD2DQZ256rrkz:
	case VCVTTPD2DQZrm:
	case VCVTTPD2DQZrmb:
	case VCVTTPD2DQZrmbk:
	case VCVTTPD2DQZrmbkz:
	case VCVTTPD2DQZrmk:
	case VCVTTPD2DQZrmkz:
	case VCVTTPD2DQZrr:
	case VCVTTPD2DQZrrb:
	case VCVTTPD2DQZrrbk:
	case VCVTTPD2DQZrrbkz:
	case VCVTTPD2DQZrrk:
	case VCVTTPD2DQZrrkz:
	case VCVTTPD2DQrm:
	case VCVTTPD2DQrr:
		return true;
	}
	return false;
}

bool isPAVGUSB(unsigned Opcode) {
	switch (Opcode) {
	case PAVGUSBrm:
	case PAVGUSBrr:
		return true;
	}
	return false;
}

bool isCALL(unsigned Opcode) {
	switch (Opcode) {
	case CALL16m:
	case CALL16r:
	case CALL32m:
	case CALL32r:
	case CALL64m:
	case CALL64pcrel32:
	case CALL64r:
	case CALLpcrel16:
	case CALLpcrel32:
	case FARCALL32m:
		return true;
	}
	return false;
}

bool isFLDENV(unsigned Opcode) {
	return Opcode == FLDENVm;
}

bool isPACKUSWB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PACKUSWBrm:
	case MMX_PACKUSWBrr:
	case PACKUSWBrm:
	case PACKUSWBrr:
		return true;
	}
	return false;
}

bool isVPHADDSW(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDSWYrm:
	case VPHADDSWYrr:
	case VPHADDSWrm:
	case VPHADDSWrr:
		return true;
	}
	return false;
}

bool isLAR(unsigned Opcode) {
	switch (Opcode) {
	case LAR16rm:
	case LAR16rr:
	case LAR32rm:
	case LAR32rr:
	case LAR64rm:
	case LAR64rr:
		return true;
	}
	return false;
}

bool isCLFLUSHOPT(unsigned Opcode) {
	return Opcode == CLFLUSHOPT;
}

bool isVMMCALL(unsigned Opcode) {
	return Opcode == VMMCALL;
}

bool isARPL(unsigned Opcode) {
	switch (Opcode) {
	case ARPL16mr:
	case ARPL16rr:
		return true;
	}
	return false;
}

bool isXABORT(unsigned Opcode) {
	return Opcode == XABORT;
}

bool isPUNPCKHWD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PUNPCKHWDrm:
	case MMX_PUNPCKHWDrr:
	case PUNPCKHWDrm:
	case PUNPCKHWDrr:
		return true;
	}
	return false;
}

bool isVRCPSH(unsigned Opcode) {
	switch (Opcode) {
	case VRCPSHZrm:
	case VRCPSHZrmk:
	case VRCPSHZrmkz:
	case VRCPSHZrr:
	case VRCPSHZrrk:
	case VRCPSHZrrkz:
		return true;
	}
	return false;
}

bool isLDDQU(unsigned Opcode) {
	return Opcode == LDDQUrm;
}

bool isPFMIN(unsigned Opcode) {
	switch (Opcode) {
	case PFMINrm:
	case PFMINrr:
		return true;
	}
	return false;
}

bool isSYSRETQ(unsigned Opcode) {
	return Opcode == SYSRET64;
}

bool isVRCPSS(unsigned Opcode) {
	switch (Opcode) {
	case VRCPSSm_Int:
	case VRCPSSr_Int:
		return true;
	}
	return false;
}

bool isCLWB(unsigned Opcode) {
	return Opcode == CLWB;
}

bool isSTC(unsigned Opcode) {
	return Opcode == STC;
}

bool isSTD(unsigned Opcode) {
	return Opcode == STD;
}

bool isVMOVDQU8(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQU8Z128mr:
	case VMOVDQU8Z128mrk:
	case VMOVDQU8Z128rm:
	case VMOVDQU8Z128rmk:
	case VMOVDQU8Z128rmkz:
	case VMOVDQU8Z128rr:
	case VMOVDQU8Z128rr_REV:
	case VMOVDQU8Z128rrk:
	case VMOVDQU8Z128rrk_REV:
	case VMOVDQU8Z128rrkz:
	case VMOVDQU8Z128rrkz_REV:
	case VMOVDQU8Z256mr:
	case VMOVDQU8Z256mrk:
	case VMOVDQU8Z256rm:
	case VMOVDQU8Z256rmk:
	case VMOVDQU8Z256rmkz:
	case VMOVDQU8Z256rr:
	case VMOVDQU8Z256rr_REV:
	case VMOVDQU8Z256rrk:
	case VMOVDQU8Z256rrk_REV:
	case VMOVDQU8Z256rrkz:
	case VMOVDQU8Z256rrkz_REV:
	case VMOVDQU8Zmr:
	case VMOVDQU8Zmrk:
	case VMOVDQU8Zrm:
	case VMOVDQU8Zrmk:
	case VMOVDQU8Zrmkz:
	case VMOVDQU8Zrr:
	case VMOVDQU8Zrr_REV:
	case VMOVDQU8Zrrk:
	case VMOVDQU8Zrrk_REV:
	case VMOVDQU8Zrrkz:
	case VMOVDQU8Zrrkz_REV:
		return true;
	}
	return false;
}

bool isSTI(unsigned Opcode) {
	return Opcode == STI;
}

bool isSTR(unsigned Opcode) {
	switch (Opcode) {
	case STR16r:
	case STR32r:
	case STR64r:
	case STRm:
		return true;
	}
	return false;
}

bool isVFPCLASSSD(unsigned Opcode) {
	switch (Opcode) {
	case VFPCLASSSDZrm:
	case VFPCLASSSDZrmk:
	case VFPCLASSSDZrr:
	case VFPCLASSSDZrrk:
		return true;
	}
	return false;
}

bool isLDMXCSR(unsigned Opcode) {
	return Opcode == LDMXCSR;
}

bool isVFPCLASSSH(unsigned Opcode) {
	switch (Opcode) {
	case VFPCLASSSHZrm:
	case VFPCLASSSHZrmk:
	case VFPCLASSSHZrr:
	case VFPCLASSSHZrrk:
		return true;
	}
	return false;
}

bool isVCVTPD2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPD2PHZ128rm:
	case VCVTPD2PHZ128rmb:
	case VCVTPD2PHZ128rmbk:
	case VCVTPD2PHZ128rmbkz:
	case VCVTPD2PHZ128rmk:
	case VCVTPD2PHZ128rmkz:
	case VCVTPD2PHZ128rr:
	case VCVTPD2PHZ128rrk:
	case VCVTPD2PHZ128rrkz:
	case VCVTPD2PHZ256rm:
	case VCVTPD2PHZ256rmb:
	case VCVTPD2PHZ256rmbk:
	case VCVTPD2PHZ256rmbkz:
	case VCVTPD2PHZ256rmk:
	case VCVTPD2PHZ256rmkz:
	case VCVTPD2PHZ256rr:
	case VCVTPD2PHZ256rrk:
	case VCVTPD2PHZ256rrkz:
	case VCVTPD2PHZrm:
	case VCVTPD2PHZrmb:
	case VCVTPD2PHZrmbk:
	case VCVTPD2PHZrmbkz:
	case VCVTPD2PHZrmk:
	case VCVTPD2PHZrmkz:
	case VCVTPD2PHZrr:
	case VCVTPD2PHZrrb:
	case VCVTPD2PHZrrbk:
	case VCVTPD2PHZrrbkz:
	case VCVTPD2PHZrrk:
	case VCVTPD2PHZrrkz:
		return true;
	}
	return false;
}

bool isVCVTPH2DQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2DQZ128rm:
	case VCVTPH2DQZ128rmb:
	case VCVTPH2DQZ128rmbk:
	case VCVTPH2DQZ128rmbkz:
	case VCVTPH2DQZ128rmk:
	case VCVTPH2DQZ128rmkz:
	case VCVTPH2DQZ128rr:
	case VCVTPH2DQZ128rrk:
	case VCVTPH2DQZ128rrkz:
	case VCVTPH2DQZ256rm:
	case VCVTPH2DQZ256rmb:
	case VCVTPH2DQZ256rmbk:
	case VCVTPH2DQZ256rmbkz:
	case VCVTPH2DQZ256rmk:
	case VCVTPH2DQZ256rmkz:
	case VCVTPH2DQZ256rr:
	case VCVTPH2DQZ256rrk:
	case VCVTPH2DQZ256rrkz:
	case VCVTPH2DQZrm:
	case VCVTPH2DQZrmb:
	case VCVTPH2DQZrmbk:
	case VCVTPH2DQZrmbkz:
	case VCVTPH2DQZrmk:
	case VCVTPH2DQZrmkz:
	case VCVTPH2DQZrr:
	case VCVTPH2DQZrrb:
	case VCVTPH2DQZrrbk:
	case VCVTPH2DQZrrbkz:
	case VCVTPH2DQZrrk:
	case VCVTPH2DQZrrkz:
		return true;
	}
	return false;
}

bool isVFPCLASSSS(unsigned Opcode) {
	switch (Opcode) {
	case VFPCLASSSSZrm:
	case VFPCLASSSSZrmk:
	case VFPCLASSSSZrr:
	case VFPCLASSSSZrrk:
		return true;
	}
	return false;
}

bool isVMOVDQA64(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDQA64Z128mr:
	case VMOVDQA64Z128mrk:
	case VMOVDQA64Z128rm:
	case VMOVDQA64Z128rmk:
	case VMOVDQA64Z128rmkz:
	case VMOVDQA64Z128rr:
	case VMOVDQA64Z128rr_REV:
	case VMOVDQA64Z128rrk:
	case VMOVDQA64Z128rrk_REV:
	case VMOVDQA64Z128rrkz:
	case VMOVDQA64Z128rrkz_REV:
	case VMOVDQA64Z256mr:
	case VMOVDQA64Z256mrk:
	case VMOVDQA64Z256rm:
	case VMOVDQA64Z256rmk:
	case VMOVDQA64Z256rmkz:
	case VMOVDQA64Z256rr:
	case VMOVDQA64Z256rr_REV:
	case VMOVDQA64Z256rrk:
	case VMOVDQA64Z256rrk_REV:
	case VMOVDQA64Z256rrkz:
	case VMOVDQA64Z256rrkz_REV:
	case VMOVDQA64Zmr:
	case VMOVDQA64Zmrk:
	case VMOVDQA64Zrm:
	case VMOVDQA64Zrmk:
	case VMOVDQA64Zrmkz:
	case VMOVDQA64Zrr:
	case VMOVDQA64Zrr_REV:
	case VMOVDQA64Zrrk:
	case VMOVDQA64Zrrk_REV:
	case VMOVDQA64Zrrkz:
	case VMOVDQA64Zrrkz_REV:
		return true;
	}
	return false;
}

bool isSUB(unsigned Opcode) {
	switch (Opcode) {
	case SUB16i16:
	case SUB16mi:
	case SUB16mi8:
	case SUB16mr:
	case SUB16ri:
	case SUB16ri8:
	case SUB16rm:
	case SUB16rr:
	case SUB16rr_REV:
	case SUB32i32:
	case SUB32mi:
	case SUB32mi8:
	case SUB32mr:
	case SUB32ri:
	case SUB32ri8:
	case SUB32rm:
	case SUB32rr:
	case SUB32rr_REV:
	case SUB64i32:
	case SUB64mi32:
	case SUB64mi8:
	case SUB64mr:
	case SUB64ri32:
	case SUB64ri8:
	case SUB64rm:
	case SUB64rr:
	case SUB64rr_REV:
	case SUB8i8:
	case SUB8mi:
	case SUB8mi8:
	case SUB8mr:
	case SUB8ri:
	case SUB8ri8:
	case SUB8rm:
	case SUB8rr:
	case SUB8rr_REV:
		return true;
	}
	return false;
}

bool isVUCOMISD(unsigned Opcode) {
	switch (Opcode) {
	case VUCOMISDZrm:
	case VUCOMISDZrr:
	case VUCOMISDZrrb:
	case VUCOMISDrm:
	case VUCOMISDrr:
		return true;
	}
	return false;
}

bool isVCVTPD2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPD2PSYrm:
	case VCVTPD2PSYrr:
	case VCVTPD2PSZ128rm:
	case VCVTPD2PSZ128rmb:
	case VCVTPD2PSZ128rmbk:
	case VCVTPD2PSZ128rmbkz:
	case VCVTPD2PSZ128rmk:
	case VCVTPD2PSZ128rmkz:
	case VCVTPD2PSZ128rr:
	case VCVTPD2PSZ128rrk:
	case VCVTPD2PSZ128rrkz:
	case VCVTPD2PSZ256rm:
	case VCVTPD2PSZ256rmb:
	case VCVTPD2PSZ256rmbk:
	case VCVTPD2PSZ256rmbkz:
	case VCVTPD2PSZ256rmk:
	case VCVTPD2PSZ256rmkz:
	case VCVTPD2PSZ256rr:
	case VCVTPD2PSZ256rrk:
	case VCVTPD2PSZ256rrkz:
	case VCVTPD2PSZrm:
	case VCVTPD2PSZrmb:
	case VCVTPD2PSZrmbk:
	case VCVTPD2PSZrmbkz:
	case VCVTPD2PSZrmk:
	case VCVTPD2PSZrmkz:
	case VCVTPD2PSZrr:
	case VCVTPD2PSZrrb:
	case VCVTPD2PSZrrbk:
	case VCVTPD2PSZrrbkz:
	case VCVTPD2PSZrrk:
	case VCVTPD2PSZrrkz:
	case VCVTPD2PSrm:
	case VCVTPD2PSrr:
		return true;
	}
	return false;
}

bool isLEAVE(unsigned Opcode) {
	switch (Opcode) {
	case LEAVE:
	case LEAVE64:
		return true;
	}
	return false;
}

bool isOUTSB(unsigned Opcode) {
	return Opcode == OUTSB;
}

bool isOUTSD(unsigned Opcode) {
	return Opcode == OUTSL;
}

bool isVUCOMISH(unsigned Opcode) {
	switch (Opcode) {
	case VUCOMISHZrm:
	case VUCOMISHZrr:
	case VUCOMISHZrrb:
		return true;
	}
	return false;
}

bool isEXTRQ(unsigned Opcode) {
	switch (Opcode) {
	case EXTRQ:
	case EXTRQI:
		return true;
	}
	return false;
}

bool isVUCOMISS(unsigned Opcode) {
	switch (Opcode) {
	case VUCOMISSZrm:
	case VUCOMISSZrr:
	case VUCOMISSZrrb:
	case VUCOMISSrm:
	case VUCOMISSrr:
		return true;
	}
	return false;
}

bool isCVTTPD2PI(unsigned Opcode) {
	switch (Opcode) {
	case MMX_CVTTPD2PIrm:
	case MMX_CVTTPD2PIrr:
		return true;
	}
	return false;
}

bool isOUTSW(unsigned Opcode) {
	return Opcode == OUTSW;
}

bool isLDS(unsigned Opcode) {
	switch (Opcode) {
	case LDS16rm:
	case LDS32rm:
		return true;
	}
	return false;
}

bool isPSHUFHW(unsigned Opcode) {
	switch (Opcode) {
	case PSHUFHWmi:
	case PSHUFHWri:
		return true;
	}
	return false;
}

bool isPHSUBD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PHSUBDrm:
	case MMX_PHSUBDrr:
	case PHSUBDrm:
	case PHSUBDrr:
		return true;
	}
	return false;
}

bool isHLT(unsigned Opcode) {
	return Opcode == HLT;
}

bool isVCVTPD2QQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPD2QQZ128rm:
	case VCVTPD2QQZ128rmb:
	case VCVTPD2QQZ128rmbk:
	case VCVTPD2QQZ128rmbkz:
	case VCVTPD2QQZ128rmk:
	case VCVTPD2QQZ128rmkz:
	case VCVTPD2QQZ128rr:
	case VCVTPD2QQZ128rrk:
	case VCVTPD2QQZ128rrkz:
	case VCVTPD2QQZ256rm:
	case VCVTPD2QQZ256rmb:
	case VCVTPD2QQZ256rmbk:
	case VCVTPD2QQZ256rmbkz:
	case VCVTPD2QQZ256rmk:
	case VCVTPD2QQZ256rmkz:
	case VCVTPD2QQZ256rr:
	case VCVTPD2QQZ256rrk:
	case VCVTPD2QQZ256rrkz:
	case VCVTPD2QQZrm:
	case VCVTPD2QQZrmb:
	case VCVTPD2QQZrmbk:
	case VCVTPD2QQZrmbkz:
	case VCVTPD2QQZrmk:
	case VCVTPD2QQZrmkz:
	case VCVTPD2QQZrr:
	case VCVTPD2QQZrrb:
	case VCVTPD2QQZrrbk:
	case VCVTPD2QQZrrbkz:
	case VCVTPD2QQZrrk:
	case VCVTPD2QQZrrkz:
		return true;
	}
	return false;
}

bool isLIDT(unsigned Opcode) {
	return Opcode == LIDT64m;
}

bool isVPTERNLOGD(unsigned Opcode) {
	switch (Opcode) {
	case VPTERNLOGDZ128rmbi:
	case VPTERNLOGDZ128rmbik:
	case VPTERNLOGDZ128rmbikz:
	case VPTERNLOGDZ128rmi:
	case VPTERNLOGDZ128rmik:
	case VPTERNLOGDZ128rmikz:
	case VPTERNLOGDZ128rri:
	case VPTERNLOGDZ128rrik:
	case VPTERNLOGDZ128rrikz:
	case VPTERNLOGDZ256rmbi:
	case VPTERNLOGDZ256rmbik:
	case VPTERNLOGDZ256rmbikz:
	case VPTERNLOGDZ256rmi:
	case VPTERNLOGDZ256rmik:
	case VPTERNLOGDZ256rmikz:
	case VPTERNLOGDZ256rri:
	case VPTERNLOGDZ256rrik:
	case VPTERNLOGDZ256rrikz:
	case VPTERNLOGDZrmbi:
	case VPTERNLOGDZrmbik:
	case VPTERNLOGDZrmbikz:
	case VPTERNLOGDZrmi:
	case VPTERNLOGDZrmik:
	case VPTERNLOGDZrmikz:
	case VPTERNLOGDZrri:
	case VPTERNLOGDZrrik:
	case VPTERNLOGDZrrikz:
		return true;
	}
	return false;
}

bool isLEA(unsigned Opcode) {
	switch (Opcode) {
	case LEA16r:
	case LEA32r:
	case LEA64_32r:
	case LEA64r:
		return true;
	}
	return false;
}

bool isVPHADDWD(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDWDrm:
	case VPHADDWDrr:
		return true;
	}
	return false;
}

bool isPREFETCHNTA(unsigned Opcode) {
	return Opcode == PREFETCHNTA;
}

bool isVPTERNLOGQ(unsigned Opcode) {
	switch (Opcode) {
	case VPTERNLOGQZ128rmbi:
	case VPTERNLOGQZ128rmbik:
	case VPTERNLOGQZ128rmbikz:
	case VPTERNLOGQZ128rmi:
	case VPTERNLOGQZ128rmik:
	case VPTERNLOGQZ128rmikz:
	case VPTERNLOGQZ128rri:
	case VPTERNLOGQZ128rrik:
	case VPTERNLOGQZ128rrikz:
	case VPTERNLOGQZ256rmbi:
	case VPTERNLOGQZ256rmbik:
	case VPTERNLOGQZ256rmbikz:
	case VPTERNLOGQZ256rmi:
	case VPTERNLOGQZ256rmik:
	case VPTERNLOGQZ256rmikz:
	case VPTERNLOGQZ256rri:
	case VPTERNLOGQZ256rrik:
	case VPTERNLOGQZ256rrikz:
	case VPTERNLOGQZrmbi:
	case VPTERNLOGQZrmbik:
	case VPTERNLOGQZrmbikz:
	case VPTERNLOGQZrmi:
	case VPTERNLOGQZrmik:
	case VPTERNLOGQZrmikz:
	case VPTERNLOGQZrri:
	case VPTERNLOGQZrrik:
	case VPTERNLOGQZrrikz:
		return true;
	}
	return false;
}

bool isKSHIFTLB(unsigned Opcode) {
	return Opcode == KSHIFTLBri;
}

bool isKSHIFTLD(unsigned Opcode) {
	return Opcode == KSHIFTLDri;
}

bool isPHSUBW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PHSUBWrm:
	case MMX_PHSUBWrr:
	case PHSUBWrm:
	case PHSUBWrr:
		return true;
	}
	return false;
}

bool isVPMOVSQB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSQBZ128mr:
	case VPMOVSQBZ128mrk:
	case VPMOVSQBZ128rr:
	case VPMOVSQBZ128rrk:
	case VPMOVSQBZ128rrkz:
	case VPMOVSQBZ256mr:
	case VPMOVSQBZ256mrk:
	case VPMOVSQBZ256rr:
	case VPMOVSQBZ256rrk:
	case VPMOVSQBZ256rrkz:
	case VPMOVSQBZmr:
	case VPMOVSQBZmrk:
	case VPMOVSQBZrr:
	case VPMOVSQBZrrk:
	case VPMOVSQBZrrkz:
		return true;
	}
	return false;
}

bool isVPHADDWQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDWQrm:
	case VPHADDWQrr:
		return true;
	}
	return false;
}

bool isVCVTTSS2USI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTSS2USI64Zrm_Int:
	case VCVTTSS2USI64Zrr_Int:
	case VCVTTSS2USI64Zrrb_Int:
	case VCVTTSS2USIZrm_Int:
	case VCVTTSS2USIZrr_Int:
	case VCVTTSS2USIZrrb_Int:
		return true;
	}
	return false;
}

bool isVPMOVSQD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSQDZ128mr:
	case VPMOVSQDZ128mrk:
	case VPMOVSQDZ128rr:
	case VPMOVSQDZ128rrk:
	case VPMOVSQDZ128rrkz:
	case VPMOVSQDZ256mr:
	case VPMOVSQDZ256mrk:
	case VPMOVSQDZ256rr:
	case VPMOVSQDZ256rrk:
	case VPMOVSQDZ256rrkz:
	case VPMOVSQDZmr:
	case VPMOVSQDZmrk:
	case VPMOVSQDZrr:
	case VPMOVSQDZrrk:
	case VPMOVSQDZrrkz:
		return true;
	}
	return false;
}

bool isLES(unsigned Opcode) {
	switch (Opcode) {
	case LES16rm:
	case LES32rm:
		return true;
	}
	return false;
}

bool isVMPSADBW(unsigned Opcode) {
	switch (Opcode) {
	case VMPSADBWYrmi:
	case VMPSADBWYrri:
	case VMPSADBWrmi:
	case VMPSADBWrri:
		return true;
	}
	return false;
}

bool isKSHIFTLQ(unsigned Opcode) {
	return Opcode == KSHIFTLQri;
}

bool isADDSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case ADDSUBPDrm:
	case ADDSUBPDrr:
		return true;
	}
	return false;
}

bool isKSHIFTLW(unsigned Opcode) {
	return Opcode == KSHIFTLWri;
}

bool isXSAVE64(unsigned Opcode) {
	return Opcode == XSAVE64;
}

bool isVPMOVSQW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSQWZ128mr:
	case VPMOVSQWZ128mrk:
	case VPMOVSQWZ128rr:
	case VPMOVSQWZ128rrk:
	case VPMOVSQWZ128rrkz:
	case VPMOVSQWZ256mr:
	case VPMOVSQWZ256mrk:
	case VPMOVSQWZ256rr:
	case VPMOVSQWZ256rrk:
	case VPMOVSQWZ256rrkz:
	case VPMOVSQWZmr:
	case VPMOVSQWZmrk:
	case VPMOVSQWZrr:
	case VPMOVSQWZrrk:
	case VPMOVSQWZrrkz:
		return true;
	}
	return false;
}

bool isADDSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case ADDSUBPSrm:
	case ADDSUBPSrr:
		return true;
	}
	return false;
}

bool isENQCMD(unsigned Opcode) {
	switch (Opcode) {
	case ENQCMD16:
	case ENQCMD32:
	case ENQCMD64:
		return true;
	}
	return false;
}

bool isVCVTTPH2W(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPH2WZ128rm:
	case VCVTTPH2WZ128rmb:
	case VCVTTPH2WZ128rmbk:
	case VCVTTPH2WZ128rmbkz:
	case VCVTTPH2WZ128rmk:
	case VCVTTPH2WZ128rmkz:
	case VCVTTPH2WZ128rr:
	case VCVTTPH2WZ128rrk:
	case VCVTTPH2WZ128rrkz:
	case VCVTTPH2WZ256rm:
	case VCVTTPH2WZ256rmb:
	case VCVTTPH2WZ256rmbk:
	case VCVTTPH2WZ256rmbkz:
	case VCVTTPH2WZ256rmk:
	case VCVTTPH2WZ256rmkz:
	case VCVTTPH2WZ256rr:
	case VCVTTPH2WZ256rrk:
	case VCVTTPH2WZ256rrkz:
	case VCVTTPH2WZrm:
	case VCVTTPH2WZrmb:
	case VCVTTPH2WZrmbk:
	case VCVTTPH2WZrmbkz:
	case VCVTTPH2WZrmk:
	case VCVTTPH2WZrmkz:
	case VCVTTPH2WZrr:
	case VCVTTPH2WZrrb:
	case VCVTTPH2WZrrbk:
	case VCVTTPH2WZrrbkz:
	case VCVTTPH2WZrrk:
	case VCVTTPH2WZrrkz:
		return true;
	}
	return false;
}

bool isVMRUN(unsigned Opcode) {
	switch (Opcode) {
	case VMRUN32:
	case VMRUN64:
		return true;
	}
	return false;
}

bool isWRFSBASE(unsigned Opcode) {
	switch (Opcode) {
	case WRFSBASE:
	case WRFSBASE64:
		return true;
	}
	return false;
}

bool isCOMISD(unsigned Opcode) {
	switch (Opcode) {
	case COMISDrm:
	case COMISDrr:
		return true;
	}
	return false;
}

bool isLFS(unsigned Opcode) {
	switch (Opcode) {
	case LFS16rm:
	case LFS32rm:
	case LFS64rm:
		return true;
	}
	return false;
}

bool isSTOSB(unsigned Opcode) {
	return Opcode == STOSB;
}

bool isSTOSD(unsigned Opcode) {
	return Opcode == STOSL;
}

bool isCOMISS(unsigned Opcode) {
	switch (Opcode) {
	case COMISSrm:
	case COMISSrr:
		return true;
	}
	return false;
}

bool isVZEROALL(unsigned Opcode) {
	return Opcode == VZEROALL;
}

bool isVFMADDSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUBPD4Ymr:
	case VFMADDSUBPD4Yrm:
	case VFMADDSUBPD4Yrr:
	case VFMADDSUBPD4Yrr_REV:
	case VFMADDSUBPD4mr:
	case VFMADDSUBPD4rm:
	case VFMADDSUBPD4rr:
	case VFMADDSUBPD4rr_REV:
		return true;
	}
	return false;
}

bool isVEXTRACTPS(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTPSZmr:
	case VEXTRACTPSZrr:
	case VEXTRACTPSmr:
	case VEXTRACTPSrr:
		return true;
	}
	return false;
}

bool isKADDB(unsigned Opcode) {
	return Opcode == KADDBrr;
}

bool isKADDD(unsigned Opcode) {
	return Opcode == KADDDrr;
}

bool isXTEST(unsigned Opcode) {
	return Opcode == XTEST;
}

bool isFISTTP(unsigned Opcode) {
	switch (Opcode) {
	case ISTT_FP16m:
	case ISTT_FP32m:
	case ISTT_FP64m:
		return true;
	}
	return false;
}

bool isSTOSQ(unsigned Opcode) {
	return Opcode == STOSQ;
}

bool isJCXZ(unsigned Opcode) {
	return Opcode == JCXZ;
}

bool isSTOSW(unsigned Opcode) {
	return Opcode == STOSW;
}

bool isVFMADDSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case VFMADDSUBPS4Ymr:
	case VFMADDSUBPS4Yrm:
	case VFMADDSUBPS4Yrr:
	case VFMADDSUBPS4Yrr_REV:
	case VFMADDSUBPS4mr:
	case VFMADDSUBPS4rm:
	case VFMADDSUBPS4rr:
	case VFMADDSUBPS4rr_REV:
		return true;
	}
	return false;
}

bool isVPSHAB(unsigned Opcode) {
	switch (Opcode) {
	case VPSHABmr:
	case VPSHABrm:
	case VPSHABrr:
	case VPSHABrr_REV:
		return true;
	}
	return false;
}

bool isKADDQ(unsigned Opcode) {
	return Opcode == KADDQrr;
}

bool isLGS(unsigned Opcode) {
	switch (Opcode) {
	case LGS16rm:
	case LGS32rm:
	case LGS64rm:
		return true;
	}
	return false;
}

bool isVPMACSDQH(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSDQHrm:
	case VPMACSDQHrr:
		return true;
	}
	return false;
}

bool isMOVDQA(unsigned Opcode) {
	switch (Opcode) {
	case MOVDQAmr:
	case MOVDQArm:
	case MOVDQArr:
	case MOVDQArr_REV:
		return true;
	}
	return false;
}

bool isVPSHAD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHADmr:
	case VPSHADrm:
	case VPSHADrr:
	case VPSHADrr_REV:
		return true;
	}
	return false;
}

bool isKADDW(unsigned Opcode) {
	return Opcode == KADDWrr;
}

bool isPSMASH(unsigned Opcode) {
	return Opcode == PSMASH;
}

bool isPBLENDVB(unsigned Opcode) {
	switch (Opcode) {
	case PBLENDVBrm0:
	case PBLENDVBrr0:
		return true;
	}
	return false;
}

bool isVPMACSDQL(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSDQLrm:
	case VPMACSDQLrr:
		return true;
	}
	return false;
}

bool isVPHSUBBW(unsigned Opcode) {
	switch (Opcode) {
	case VPHSUBBWrm:
	case VPHSUBBWrr:
		return true;
	}
	return false;
}

bool isVPSHAQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSHAQmr:
	case VPSHAQrm:
	case VPSHAQrr:
	case VPSHAQrr_REV:
		return true;
	}
	return false;
}

bool isVSQRTPD(unsigned Opcode) {
	switch (Opcode) {
	case VSQRTPDYm:
	case VSQRTPDYr:
	case VSQRTPDZ128m:
	case VSQRTPDZ128mb:
	case VSQRTPDZ128mbk:
	case VSQRTPDZ128mbkz:
	case VSQRTPDZ128mk:
	case VSQRTPDZ128mkz:
	case VSQRTPDZ128r:
	case VSQRTPDZ128rk:
	case VSQRTPDZ128rkz:
	case VSQRTPDZ256m:
	case VSQRTPDZ256mb:
	case VSQRTPDZ256mbk:
	case VSQRTPDZ256mbkz:
	case VSQRTPDZ256mk:
	case VSQRTPDZ256mkz:
	case VSQRTPDZ256r:
	case VSQRTPDZ256rk:
	case VSQRTPDZ256rkz:
	case VSQRTPDZm:
	case VSQRTPDZmb:
	case VSQRTPDZmbk:
	case VSQRTPDZmbkz:
	case VSQRTPDZmk:
	case VSQRTPDZmkz:
	case VSQRTPDZr:
	case VSQRTPDZrb:
	case VSQRTPDZrbk:
	case VSQRTPDZrbkz:
	case VSQRTPDZrk:
	case VSQRTPDZrkz:
	case VSQRTPDm:
	case VSQRTPDr:
		return true;
	}
	return false;
}

bool isCLRSSBSY(unsigned Opcode) {
	return Opcode == CLRSSBSY;
}

bool isMINPD(unsigned Opcode) {
	switch (Opcode) {
	case MINPDrm:
	case MINPDrr:
		return true;
	}
	return false;
}

bool isVPSHAW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHAWmr:
	case VPSHAWrm:
	case VPSHAWrr:
	case VPSHAWrr_REV:
		return true;
	}
	return false;
}

bool isVSQRTPH(unsigned Opcode) {
	switch (Opcode) {
	case VSQRTPHZ128m:
	case VSQRTPHZ128mb:
	case VSQRTPHZ128mbk:
	case VSQRTPHZ128mbkz:
	case VSQRTPHZ128mk:
	case VSQRTPHZ128mkz:
	case VSQRTPHZ128r:
	case VSQRTPHZ128rk:
	case VSQRTPHZ128rkz:
	case VSQRTPHZ256m:
	case VSQRTPHZ256mb:
	case VSQRTPHZ256mbk:
	case VSQRTPHZ256mbkz:
	case VSQRTPHZ256mk:
	case VSQRTPHZ256mkz:
	case VSQRTPHZ256r:
	case VSQRTPHZ256rk:
	case VSQRTPHZ256rkz:
	case VSQRTPHZm:
	case VSQRTPHZmb:
	case VSQRTPHZmbk:
	case VSQRTPHZmbkz:
	case VSQRTPHZmk:
	case VSQRTPHZmkz:
	case VSQRTPHZr:
	case VSQRTPHZrb:
	case VSQRTPHZrbk:
	case VSQRTPHZrbkz:
	case VSQRTPHZrk:
	case VSQRTPHZrkz:
		return true;
	}
	return false;
}

bool isMOVDQU(unsigned Opcode) {
	switch (Opcode) {
	case MOVDQUmr:
	case MOVDQUrm:
	case MOVDQUrr:
	case MOVDQUrr_REV:
		return true;
	}
	return false;
}

bool isVSQRTPS(unsigned Opcode) {
	switch (Opcode) {
	case VSQRTPSYm:
	case VSQRTPSYr:
	case VSQRTPSZ128m:
	case VSQRTPSZ128mb:
	case VSQRTPSZ128mbk:
	case VSQRTPSZ128mbkz:
	case VSQRTPSZ128mk:
	case VSQRTPSZ128mkz:
	case VSQRTPSZ128r:
	case VSQRTPSZ128rk:
	case VSQRTPSZ128rkz:
	case VSQRTPSZ256m:
	case VSQRTPSZ256mb:
	case VSQRTPSZ256mbk:
	case VSQRTPSZ256mbkz:
	case VSQRTPSZ256mk:
	case VSQRTPSZ256mkz:
	case VSQRTPSZ256r:
	case VSQRTPSZ256rk:
	case VSQRTPSZ256rkz:
	case VSQRTPSZm:
	case VSQRTPSZmb:
	case VSQRTPSZmbk:
	case VSQRTPSZmbkz:
	case VSQRTPSZmk:
	case VSQRTPSZmkz:
	case VSQRTPSZr:
	case VSQRTPSZrb:
	case VSQRTPSZrbk:
	case VSQRTPSZrbkz:
	case VSQRTPSZrk:
	case VSQRTPSZrkz:
	case VSQRTPSm:
	case VSQRTPSr:
		return true;
	}
	return false;
}

bool isVPSUBSB(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBSBYrm:
	case VPSUBSBYrr:
	case VPSUBSBZ128rm:
	case VPSUBSBZ128rmk:
	case VPSUBSBZ128rmkz:
	case VPSUBSBZ128rr:
	case VPSUBSBZ128rrk:
	case VPSUBSBZ128rrkz:
	case VPSUBSBZ256rm:
	case VPSUBSBZ256rmk:
	case VPSUBSBZ256rmkz:
	case VPSUBSBZ256rr:
	case VPSUBSBZ256rrk:
	case VPSUBSBZ256rrkz:
	case VPSUBSBZrm:
	case VPSUBSBZrmk:
	case VPSUBSBZrmkz:
	case VPSUBSBZrr:
	case VPSUBSBZrrk:
	case VPSUBSBZrrkz:
	case VPSUBSBrm:
	case VPSUBSBrr:
		return true;
	}
	return false;
}

bool isMULX(unsigned Opcode) {
	switch (Opcode) {
	case MULX32rm:
	case MULX32rr:
	case MULX64rm:
	case MULX64rr:
		return true;
	}
	return false;
}

bool isMINPS(unsigned Opcode) {
	switch (Opcode) {
	case MINPSrm:
	case MINPSrr:
		return true;
	}
	return false;
}

bool isPSHUFLW(unsigned Opcode) {
	switch (Opcode) {
	case PSHUFLWmi:
	case PSHUFLWri:
		return true;
	}
	return false;
}

bool isVEXTRACTF64X2(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTF64x2Z256mr:
	case VEXTRACTF64x2Z256mrk:
	case VEXTRACTF64x2Z256rr:
	case VEXTRACTF64x2Z256rrk:
	case VEXTRACTF64x2Z256rrkz:
	case VEXTRACTF64x2Zmr:
	case VEXTRACTF64x2Zmrk:
	case VEXTRACTF64x2Zrr:
	case VEXTRACTF64x2Zrrk:
	case VEXTRACTF64x2Zrrkz:
		return true;
	}
	return false;
}

bool isVEXTRACTF64X4(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTF64x4Zmr:
	case VEXTRACTF64x4Zmrk:
	case VEXTRACTF64x4Zrr:
	case VEXTRACTF64x4Zrrk:
	case VEXTRACTF64x4Zrrkz:
		return true;
	}
	return false;
}

bool isVCVTTSD2USI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTSD2USI64Zrm_Int:
	case VCVTTSD2USI64Zrr_Int:
	case VCVTTSD2USI64Zrrb_Int:
	case VCVTTSD2USIZrm_Int:
	case VCVTTSD2USIZrr_Int:
	case VCVTTSD2USIZrrb_Int:
		return true;
	}
	return false;
}

bool isINCSSPD(unsigned Opcode) {
	return Opcode == INCSSPD;
}

bool isVPSIGNB(unsigned Opcode) {
	switch (Opcode) {
	case VPSIGNBYrm:
	case VPSIGNBYrr:
	case VPSIGNBrm:
	case VPSIGNBrr:
		return true;
	}
	return false;
}

bool isVPSUBSW(unsigned Opcode) {
	switch (Opcode) {
	case VPSUBSWYrm:
	case VPSUBSWYrr:
	case VPSUBSWZ128rm:
	case VPSUBSWZ128rmk:
	case VPSUBSWZ128rmkz:
	case VPSUBSWZ128rr:
	case VPSUBSWZ128rrk:
	case VPSUBSWZ128rrkz:
	case VPSUBSWZ256rm:
	case VPSUBSWZ256rmk:
	case VPSUBSWZ256rmkz:
	case VPSUBSWZ256rr:
	case VPSUBSWZ256rrk:
	case VPSUBSWZ256rrkz:
	case VPSUBSWZrm:
	case VPSUBSWZrmk:
	case VPSUBSWZrmkz:
	case VPSUBSWZrr:
	case VPSUBSWZrrk:
	case VPSUBSWZrrkz:
	case VPSUBSWrm:
	case VPSUBSWrr:
		return true;
	}
	return false;
}

bool isVPSIGND(unsigned Opcode) {
	switch (Opcode) {
	case VPSIGNDYrm:
	case VPSIGNDYrr:
	case VPSIGNDrm:
	case VPSIGNDrr:
		return true;
	}
	return false;
}

bool isXSAVEOPT(unsigned Opcode) {
	return Opcode == XSAVEOPT;
}

bool isVPMOVSXWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSXWDYrm:
	case VPMOVSXWDYrr:
	case VPMOVSXWDZ128rm:
	case VPMOVSXWDZ128rmk:
	case VPMOVSXWDZ128rmkz:
	case VPMOVSXWDZ128rr:
	case VPMOVSXWDZ128rrk:
	case VPMOVSXWDZ128rrkz:
	case VPMOVSXWDZ256rm:
	case VPMOVSXWDZ256rmk:
	case VPMOVSXWDZ256rmkz:
	case VPMOVSXWDZ256rr:
	case VPMOVSXWDZ256rrk:
	case VPMOVSXWDZ256rrkz:
	case VPMOVSXWDZrm:
	case VPMOVSXWDZrmk:
	case VPMOVSXWDZrmkz:
	case VPMOVSXWDZrr:
	case VPMOVSXWDZrrk:
	case VPMOVSXWDZrrkz:
	case VPMOVSXWDrm:
	case VPMOVSXWDrr:
		return true;
	}
	return false;
}

bool isINCSSPQ(unsigned Opcode) {
	return Opcode == INCSSPQ;
}

bool isGF2P8AFFINEQB(unsigned Opcode) {
	switch (Opcode) {
	case GF2P8AFFINEQBrmi:
	case GF2P8AFFINEQBrri:
		return true;
	}
	return false;
}

bool isVPMOVSXWQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSXWQYrm:
	case VPMOVSXWQYrr:
	case VPMOVSXWQZ128rm:
	case VPMOVSXWQZ128rmk:
	case VPMOVSXWQZ128rmkz:
	case VPMOVSXWQZ128rr:
	case VPMOVSXWQZ128rrk:
	case VPMOVSXWQZ128rrkz:
	case VPMOVSXWQZ256rm:
	case VPMOVSXWQZ256rmk:
	case VPMOVSXWQZ256rmkz:
	case VPMOVSXWQZ256rr:
	case VPMOVSXWQZ256rrk:
	case VPMOVSXWQZ256rrkz:
	case VPMOVSXWQZrm:
	case VPMOVSXWQZrmk:
	case VPMOVSXWQZrmkz:
	case VPMOVSXWQZrr:
	case VPMOVSXWQZrrk:
	case VPMOVSXWQZrrkz:
	case VPMOVSXWQrm:
	case VPMOVSXWQrr:
		return true;
	}
	return false;
}

bool isVPHSUBDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPHSUBDQrm:
	case VPHSUBDQrr:
		return true;
	}
	return false;
}

bool isVPSIGNW(unsigned Opcode) {
	switch (Opcode) {
	case VPSIGNWYrm:
	case VPSIGNWYrr:
	case VPSIGNWrm:
	case VPSIGNWrr:
		return true;
	}
	return false;
}

bool isSGDTD(unsigned Opcode) {
	return Opcode == SGDT32m;
}

bool isPUNPCKLWD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PUNPCKLWDrm:
	case MMX_PUNPCKLWDrr:
	case PUNPCKLWDrm:
	case PUNPCKLWDrr:
		return true;
	}
	return false;
}

bool isVPPERM(unsigned Opcode) {
	switch (Opcode) {
	case VPPERMrmr:
	case VPPERMrrm:
	case VPPERMrrr:
	case VPPERMrrr_REV:
		return true;
	}
	return false;
}

bool isAAA(unsigned Opcode) {
	return Opcode == AAA;
}

bool isPAND(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PANDrm:
	case MMX_PANDrr:
	case PANDrm:
	case PANDrr:
		return true;
	}
	return false;
}

bool isVCVTPH2UQQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2UQQZ128rm:
	case VCVTPH2UQQZ128rmb:
	case VCVTPH2UQQZ128rmbk:
	case VCVTPH2UQQZ128rmbkz:
	case VCVTPH2UQQZ128rmk:
	case VCVTPH2UQQZ128rmkz:
	case VCVTPH2UQQZ128rr:
	case VCVTPH2UQQZ128rrk:
	case VCVTPH2UQQZ128rrkz:
	case VCVTPH2UQQZ256rm:
	case VCVTPH2UQQZ256rmb:
	case VCVTPH2UQQZ256rmbk:
	case VCVTPH2UQQZ256rmbkz:
	case VCVTPH2UQQZ256rmk:
	case VCVTPH2UQQZ256rmkz:
	case VCVTPH2UQQZ256rr:
	case VCVTPH2UQQZ256rrk:
	case VCVTPH2UQQZ256rrkz:
	case VCVTPH2UQQZrm:
	case VCVTPH2UQQZrmb:
	case VCVTPH2UQQZrmbk:
	case VCVTPH2UQQZrmbkz:
	case VCVTPH2UQQZrmk:
	case VCVTPH2UQQZrmkz:
	case VCVTPH2UQQZrr:
	case VCVTPH2UQQZrrb:
	case VCVTPH2UQQZrrbk:
	case VCVTPH2UQQZrrbkz:
	case VCVTPH2UQQZrrk:
	case VCVTPH2UQQZrrkz:
		return true;
	}
	return false;
}

bool isAAD(unsigned Opcode) {
	return Opcode == AAD8i8;
}

bool isSGDTW(unsigned Opcode) {
	return Opcode == SGDT16m;
}

bool isVPUNPCKHWD(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKHWDYrm:
	case VPUNPCKHWDYrr:
	case VPUNPCKHWDZ128rm:
	case VPUNPCKHWDZ128rmk:
	case VPUNPCKHWDZ128rmkz:
	case VPUNPCKHWDZ128rr:
	case VPUNPCKHWDZ128rrk:
	case VPUNPCKHWDZ128rrkz:
	case VPUNPCKHWDZ256rm:
	case VPUNPCKHWDZ256rmk:
	case VPUNPCKHWDZ256rmkz:
	case VPUNPCKHWDZ256rr:
	case VPUNPCKHWDZ256rrk:
	case VPUNPCKHWDZ256rrkz:
	case VPUNPCKHWDZrm:
	case VPUNPCKHWDZrmk:
	case VPUNPCKHWDZrmkz:
	case VPUNPCKHWDZrr:
	case VPUNPCKHWDZrrk:
	case VPUNPCKHWDZrrkz:
	case VPUNPCKHWDrm:
	case VPUNPCKHWDrr:
		return true;
	}
	return false;
}

bool isAAM(unsigned Opcode) {
	return Opcode == AAM8i8;
}

bool isVCVTNEOPH2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTNEOPH2PSYrm:
	case VCVTNEOPH2PSrm:
		return true;
	}
	return false;
}

bool isAAS(unsigned Opcode) {
	return Opcode == AAS;
}

bool isVSQRTSD(unsigned Opcode) {
	switch (Opcode) {
	case VSQRTSDZm_Int:
	case VSQRTSDZm_Intk:
	case VSQRTSDZm_Intkz:
	case VSQRTSDZr_Int:
	case VSQRTSDZr_Intk:
	case VSQRTSDZr_Intkz:
	case VSQRTSDZrb_Int:
	case VSQRTSDZrb_Intk:
	case VSQRTSDZrb_Intkz:
	case VSQRTSDm_Int:
	case VSQRTSDr_Int:
		return true;
	}
	return false;
}

bool isBLCI(unsigned Opcode) {
	switch (Opcode) {
	case BLCI32rm:
	case BLCI32rr:
	case BLCI64rm:
	case BLCI64rr:
		return true;
	}
	return false;
}

bool isMINSD(unsigned Opcode) {
	switch (Opcode) {
	case MINSDrm_Int:
	case MINSDrr_Int:
		return true;
	}
	return false;
}

bool isVPSHUFBITQMB(unsigned Opcode) {
	switch (Opcode) {
	case VPSHUFBITQMBZ128rm:
	case VPSHUFBITQMBZ128rmk:
	case VPSHUFBITQMBZ128rr:
	case VPSHUFBITQMBZ128rrk:
	case VPSHUFBITQMBZ256rm:
	case VPSHUFBITQMBZ256rmk:
	case VPSHUFBITQMBZ256rr:
	case VPSHUFBITQMBZ256rrk:
	case VPSHUFBITQMBZrm:
	case VPSHUFBITQMBZrmk:
	case VPSHUFBITQMBZrr:
	case VPSHUFBITQMBZrrk:
		return true;
	}
	return false;
}

bool isKSHIFTRB(unsigned Opcode) {
	return Opcode == KSHIFTRBri;
}

bool isUMONITOR(unsigned Opcode) {
	switch (Opcode) {
	case UMONITOR16:
	case UMONITOR32:
	case UMONITOR64:
		return true;
	}
	return false;
}

bool isKSHIFTRD(unsigned Opcode) {
	return Opcode == KSHIFTRDri;
}

bool isFNCLEX(unsigned Opcode) {
	return Opcode == FNCLEX;
}

bool isVSQRTSH(unsigned Opcode) {
	switch (Opcode) {
	case VSQRTSHZm_Int:
	case VSQRTSHZm_Intk:
	case VSQRTSHZm_Intkz:
	case VSQRTSHZr_Int:
	case VSQRTSHZr_Intk:
	case VSQRTSHZr_Intkz:
	case VSQRTSHZrb_Int:
	case VSQRTSHZrb_Intk:
	case VSQRTSHZrb_Intkz:
		return true;
	}
	return false;
}

bool isBLCS(unsigned Opcode) {
	switch (Opcode) {
	case BLCS32rm:
	case BLCS32rr:
	case BLCS64rm:
	case BLCS64rr:
		return true;
	}
	return false;
}

bool isVINSERTF64X2(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTF64x2Z256rm:
	case VINSERTF64x2Z256rmk:
	case VINSERTF64x2Z256rmkz:
	case VINSERTF64x2Z256rr:
	case VINSERTF64x2Z256rrk:
	case VINSERTF64x2Z256rrkz:
	case VINSERTF64x2Zrm:
	case VINSERTF64x2Zrmk:
	case VINSERTF64x2Zrmkz:
	case VINSERTF64x2Zrr:
	case VINSERTF64x2Zrrk:
	case VINSERTF64x2Zrrkz:
		return true;
	}
	return false;
}

bool isVSQRTSS(unsigned Opcode) {
	switch (Opcode) {
	case VSQRTSSZm_Int:
	case VSQRTSSZm_Intk:
	case VSQRTSSZm_Intkz:
	case VSQRTSSZr_Int:
	case VSQRTSSZr_Intk:
	case VSQRTSSZr_Intkz:
	case VSQRTSSZrb_Int:
	case VSQRTSSZrb_Intk:
	case VSQRTSSZrb_Intkz:
	case VSQRTSSm_Int:
	case VSQRTSSr_Int:
		return true;
	}
	return false;
}

bool isVINSERTF64X4(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTF64x4Zrm:
	case VINSERTF64x4Zrmk:
	case VINSERTF64x4Zrmkz:
	case VINSERTF64x4Zrr:
	case VINSERTF64x4Zrrk:
	case VINSERTF64x4Zrrkz:
		return true;
	}
	return false;
}

bool isMINSS(unsigned Opcode) {
	switch (Opcode) {
	case MINSSrm_Int:
	case MINSSrr_Int:
		return true;
	}
	return false;
}

bool isVPBROADCASTMB2Q(unsigned Opcode) {
	switch (Opcode) {
	case VPBROADCASTMB2QZ128rr:
	case VPBROADCASTMB2QZ256rr:
	case VPBROADCASTMB2QZrr:
		return true;
	}
	return false;
}

bool isKSHIFTRQ(unsigned Opcode) {
	return Opcode == KSHIFTRQri;
}

bool isVMOVSHDUP(unsigned Opcode) {
	switch (Opcode) {
	case VMOVSHDUPYrm:
	case VMOVSHDUPYrr:
	case VMOVSHDUPZ128rm:
	case VMOVSHDUPZ128rmk:
	case VMOVSHDUPZ128rmkz:
	case VMOVSHDUPZ128rr:
	case VMOVSHDUPZ128rrk:
	case VMOVSHDUPZ128rrkz:
	case VMOVSHDUPZ256rm:
	case VMOVSHDUPZ256rmk:
	case VMOVSHDUPZ256rmkz:
	case VMOVSHDUPZ256rr:
	case VMOVSHDUPZ256rrk:
	case VMOVSHDUPZ256rrkz:
	case VMOVSHDUPZrm:
	case VMOVSHDUPZrmk:
	case VMOVSHDUPZrmkz:
	case VMOVSHDUPZrr:
	case VMOVSHDUPZrrk:
	case VMOVSHDUPZrrkz:
	case VMOVSHDUPrm:
	case VMOVSHDUPrr:
		return true;
	}
	return false;
}

bool isVPMOVSWB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVSWBZ128mr:
	case VPMOVSWBZ128mrk:
	case VPMOVSWBZ128rr:
	case VPMOVSWBZ128rrk:
	case VPMOVSWBZ128rrkz:
	case VPMOVSWBZ256mr:
	case VPMOVSWBZ256mrk:
	case VPMOVSWBZ256rr:
	case VPMOVSWBZ256rrk:
	case VPMOVSWBZ256rrkz:
	case VPMOVSWBZmr:
	case VPMOVSWBZmrk:
	case VPMOVSWBZrr:
	case VPMOVSWBZrrk:
	case VPMOVSWBZrrkz:
		return true;
	}
	return false;
}

bool isPMOVZXWD(unsigned Opcode) {
	switch (Opcode) {
	case PMOVZXWDrm:
	case PMOVZXWDrr:
		return true;
	}
	return false;
}

bool isFSIN(unsigned Opcode) {
	return Opcode == FSIN;
}

bool isPSLLDQ(unsigned Opcode) {
	return Opcode == PSLLDQri;
}

bool isKSHIFTRW(unsigned Opcode) {
	return Opcode == KSHIFTRWri;
}

bool isVPADDD(unsigned Opcode) {
	switch (Opcode) {
	case VPADDDYrm:
	case VPADDDYrr:
	case VPADDDZ128rm:
	case VPADDDZ128rmb:
	case VPADDDZ128rmbk:
	case VPADDDZ128rmbkz:
	case VPADDDZ128rmk:
	case VPADDDZ128rmkz:
	case VPADDDZ128rr:
	case VPADDDZ128rrk:
	case VPADDDZ128rrkz:
	case VPADDDZ256rm:
	case VPADDDZ256rmb:
	case VPADDDZ256rmbk:
	case VPADDDZ256rmbkz:
	case VPADDDZ256rmk:
	case VPADDDZ256rmkz:
	case VPADDDZ256rr:
	case VPADDDZ256rrk:
	case VPADDDZ256rrkz:
	case VPADDDZrm:
	case VPADDDZrmb:
	case VPADDDZrmbk:
	case VPADDDZrmbkz:
	case VPADDDZrmk:
	case VPADDDZrmkz:
	case VPADDDZrr:
	case VPADDDZrrk:
	case VPADDDZrrkz:
	case VPADDDrm:
	case VPADDDrr:
		return true;
	}
	return false;
}

bool isVPADDB(unsigned Opcode) {
	switch (Opcode) {
	case VPADDBYrm:
	case VPADDBYrr:
	case VPADDBZ128rm:
	case VPADDBZ128rmk:
	case VPADDBZ128rmkz:
	case VPADDBZ128rr:
	case VPADDBZ128rrk:
	case VPADDBZ128rrkz:
	case VPADDBZ256rm:
	case VPADDBZ256rmk:
	case VPADDBZ256rmkz:
	case VPADDBZ256rr:
	case VPADDBZ256rrk:
	case VPADDBZ256rrkz:
	case VPADDBZrm:
	case VPADDBZrmk:
	case VPADDBZrmkz:
	case VPADDBZrr:
	case VPADDBZrrk:
	case VPADDBZrrkz:
	case VPADDBrm:
	case VPADDBrr:
		return true;
	}
	return false;
}

bool isVPMACSSWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSSWDrm:
	case VPMACSSWDrr:
		return true;
	}
	return false;
}

bool isPMOVZXWQ(unsigned Opcode) {
	switch (Opcode) {
	case PMOVZXWQrm:
	case PMOVZXWQrr:
		return true;
	}
	return false;
}

bool isVPADDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPADDQYrm:
	case VPADDQYrr:
	case VPADDQZ128rm:
	case VPADDQZ128rmb:
	case VPADDQZ128rmbk:
	case VPADDQZ128rmbkz:
	case VPADDQZ128rmk:
	case VPADDQZ128rmkz:
	case VPADDQZ128rr:
	case VPADDQZ128rrk:
	case VPADDQZ128rrkz:
	case VPADDQZ256rm:
	case VPADDQZ256rmb:
	case VPADDQZ256rmbk:
	case VPADDQZ256rmbkz:
	case VPADDQZ256rmk:
	case VPADDQZ256rmkz:
	case VPADDQZ256rr:
	case VPADDQZ256rrk:
	case VPADDQZ256rrkz:
	case VPADDQZrm:
	case VPADDQZrmb:
	case VPADDQZrmbk:
	case VPADDQZrmbkz:
	case VPADDQZrmk:
	case VPADDQZrmkz:
	case VPADDQZrr:
	case VPADDQZrrk:
	case VPADDQZrrkz:
	case VPADDQrm:
	case VPADDQrr:
		return true;
	}
	return false;
}

bool isVPADDW(unsigned Opcode) {
	switch (Opcode) {
	case VPADDWYrm:
	case VPADDWYrr:
	case VPADDWZ128rm:
	case VPADDWZ128rmk:
	case VPADDWZ128rmkz:
	case VPADDWZ128rr:
	case VPADDWZ128rrk:
	case VPADDWZ128rrkz:
	case VPADDWZ256rm:
	case VPADDWZ256rmk:
	case VPADDWZ256rmkz:
	case VPADDWZ256rr:
	case VPADDWZ256rrk:
	case VPADDWZ256rrkz:
	case VPADDWZrm:
	case VPADDWZrmk:
	case VPADDWZrmkz:
	case VPADDWZrr:
	case VPADDWZrrk:
	case VPADDWZrrkz:
	case VPADDWrm:
	case VPADDWrr:
		return true;
	}
	return false;
}

bool isVRSQRTPH(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRTPHZ128m:
	case VRSQRTPHZ128mb:
	case VRSQRTPHZ128mbk:
	case VRSQRTPHZ128mbkz:
	case VRSQRTPHZ128mk:
	case VRSQRTPHZ128mkz:
	case VRSQRTPHZ128r:
	case VRSQRTPHZ128rk:
	case VRSQRTPHZ128rkz:
	case VRSQRTPHZ256m:
	case VRSQRTPHZ256mb:
	case VRSQRTPHZ256mbk:
	case VRSQRTPHZ256mbkz:
	case VRSQRTPHZ256mk:
	case VRSQRTPHZ256mkz:
	case VRSQRTPHZ256r:
	case VRSQRTPHZ256rk:
	case VRSQRTPHZ256rkz:
	case VRSQRTPHZm:
	case VRSQRTPHZmb:
	case VRSQRTPHZmbk:
	case VRSQRTPHZmbkz:
	case VRSQRTPHZmk:
	case VRSQRTPHZmkz:
	case VRSQRTPHZr:
	case VRSQRTPHZrk:
	case VRSQRTPHZrkz:
		return true;
	}
	return false;
}

bool isVPMACSSWW(unsigned Opcode) {
	switch (Opcode) {
	case VPMACSSWWrm:
	case VPMACSSWWrr:
		return true;
	}
	return false;
}

bool isVRSQRTPS(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRTPSYm:
	case VRSQRTPSYr:
	case VRSQRTPSm:
	case VRSQRTPSr:
		return true;
	}
	return false;
}

bool isVCVTTPH2DQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPH2DQZ128rm:
	case VCVTTPH2DQZ128rmb:
	case VCVTTPH2DQZ128rmbk:
	case VCVTTPH2DQZ128rmbkz:
	case VCVTTPH2DQZ128rmk:
	case VCVTTPH2DQZ128rmkz:
	case VCVTTPH2DQZ128rr:
	case VCVTTPH2DQZ128rrk:
	case VCVTTPH2DQZ128rrkz:
	case VCVTTPH2DQZ256rm:
	case VCVTTPH2DQZ256rmb:
	case VCVTTPH2DQZ256rmbk:
	case VCVTTPH2DQZ256rmbkz:
	case VCVTTPH2DQZ256rmk:
	case VCVTTPH2DQZ256rmkz:
	case VCVTTPH2DQZ256rr:
	case VCVTTPH2DQZ256rrk:
	case VCVTTPH2DQZ256rrkz:
	case VCVTTPH2DQZrm:
	case VCVTTPH2DQZrmb:
	case VCVTTPH2DQZrmbk:
	case VCVTTPH2DQZrmbkz:
	case VCVTTPH2DQZrmk:
	case VCVTTPH2DQZrmkz:
	case VCVTTPH2DQZrr:
	case VCVTTPH2DQZrrb:
	case VCVTTPH2DQZrrbk:
	case VCVTTPH2DQZrrbkz:
	case VCVTTPH2DQZrrk:
	case VCVTTPH2DQZrrkz:
		return true;
	}
	return false;
}

bool isWRMSR(unsigned Opcode) {
	return Opcode == WRMSR;
}

bool isXSETBV(unsigned Opcode) {
	return Opcode == XSETBV;
}

bool isMOVSXD(unsigned Opcode) {
	switch (Opcode) {
	case MOVSX16rm32:
	case MOVSX16rr32:
	case MOVSX32rm32:
	case MOVSX32rr32:
	case MOVSX64rm32:
	case MOVSX64rr32:
		return true;
	}
	return false;
}

bool isADC(unsigned Opcode) {
	switch (Opcode) {
	case ADC16i16:
	case ADC16mi:
	case ADC16mi8:
	case ADC16mr:
	case ADC16ri:
	case ADC16ri8:
	case ADC16rm:
	case ADC16rr:
	case ADC16rr_REV:
	case ADC32i32:
	case ADC32mi:
	case ADC32mi8:
	case ADC32mr:
	case ADC32ri:
	case ADC32ri8:
	case ADC32rm:
	case ADC32rr:
	case ADC32rr_REV:
	case ADC64i32:
	case ADC64mi32:
	case ADC64mi8:
	case ADC64mr:
	case ADC64ri32:
	case ADC64ri8:
	case ADC64rm:
	case ADC64rr:
	case ADC64rr_REV:
	case ADC8i8:
	case ADC8mi:
	case ADC8mi8:
	case ADC8mr:
	case ADC8ri:
	case ADC8ri8:
	case ADC8rm:
	case ADC8rr:
	case ADC8rr_REV:
		return true;
	}
	return false;
}

bool isADD(unsigned Opcode) {
	switch (Opcode) {
	case ADD16i16:
	case ADD16mi:
	case ADD16mi8:
	case ADD16mr:
	case ADD16ri:
	case ADD16ri8:
	case ADD16rm:
	case ADD16rr:
	case ADD16rr_REV:
	case ADD32i32:
	case ADD32mi:
	case ADD32mi8:
	case ADD32mr:
	case ADD32ri:
	case ADD32ri8:
	case ADD32rm:
	case ADD32rr:
	case ADD32rr_REV:
	case ADD64i32:
	case ADD64mi32:
	case ADD64mi8:
	case ADD64mr:
	case ADD64ri32:
	case ADD64ri8:
	case ADD64rm:
	case ADD64rr:
	case ADD64rr_REV:
	case ADD8i8:
	case ADD8mi:
	case ADD8mi8:
	case ADD8mr:
	case ADD8ri:
	case ADD8ri8:
	case ADD8rm:
	case ADD8rr:
	case ADD8rr_REV:
		return true;
	}
	return false;
}

bool isFDIV(unsigned Opcode) {
	switch (Opcode) {
	case DIV_F32m:
	case DIV_F64m:
	case DIV_FST0r:
	case DIV_FrST0:
		return true;
	}
	return false;
}

bool isAESDEC256KL(unsigned Opcode) {
	return Opcode == AESDEC256KL;
}

bool isVPCMPUB(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPUBZ128rmi:
	case VPCMPUBZ128rmik:
	case VPCMPUBZ128rri:
	case VPCMPUBZ128rrik:
	case VPCMPUBZ256rmi:
	case VPCMPUBZ256rmik:
	case VPCMPUBZ256rri:
	case VPCMPUBZ256rrik:
	case VPCMPUBZrmi:
	case VPCMPUBZrmik:
	case VPCMPUBZrri:
	case VPCMPUBZrrik:
		return true;
	}
	return false;
}

bool isVCVTTPD2QQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPD2QQZ128rm:
	case VCVTTPD2QQZ128rmb:
	case VCVTTPD2QQZ128rmbk:
	case VCVTTPD2QQZ128rmbkz:
	case VCVTTPD2QQZ128rmk:
	case VCVTTPD2QQZ128rmkz:
	case VCVTTPD2QQZ128rr:
	case VCVTTPD2QQZ128rrk:
	case VCVTTPD2QQZ128rrkz:
	case VCVTTPD2QQZ256rm:
	case VCVTTPD2QQZ256rmb:
	case VCVTTPD2QQZ256rmbk:
	case VCVTTPD2QQZ256rmbkz:
	case VCVTTPD2QQZ256rmk:
	case VCVTTPD2QQZ256rmkz:
	case VCVTTPD2QQZ256rr:
	case VCVTTPD2QQZ256rrk:
	case VCVTTPD2QQZ256rrkz:
	case VCVTTPD2QQZrm:
	case VCVTTPD2QQZrmb:
	case VCVTTPD2QQZrmbk:
	case VCVTTPD2QQZrmbkz:
	case VCVTTPD2QQZrmk:
	case VCVTTPD2QQZrmkz:
	case VCVTTPD2QQZrr:
	case VCVTTPD2QQZrrb:
	case VCVTTPD2QQZrrbk:
	case VCVTTPD2QQZrrbkz:
	case VCVTTPD2QQZrrk:
	case VCVTTPD2QQZrrkz:
		return true;
	}
	return false;
}

bool isVPCMPUD(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPUDZ128rmi:
	case VPCMPUDZ128rmib:
	case VPCMPUDZ128rmibk:
	case VPCMPUDZ128rmik:
	case VPCMPUDZ128rri:
	case VPCMPUDZ128rrik:
	case VPCMPUDZ256rmi:
	case VPCMPUDZ256rmib:
	case VPCMPUDZ256rmibk:
	case VPCMPUDZ256rmik:
	case VPCMPUDZ256rri:
	case VPCMPUDZ256rrik:
	case VPCMPUDZrmi:
	case VPCMPUDZrmib:
	case VPCMPUDZrmibk:
	case VPCMPUDZrmik:
	case VPCMPUDZrri:
	case VPCMPUDZrrik:
		return true;
	}
	return false;
}

bool isPFMUL(unsigned Opcode) {
	switch (Opcode) {
	case PFMULrm:
	case PFMULrr:
		return true;
	}
	return false;
}

bool isPREFETCHW(unsigned Opcode) {
	return Opcode == PREFETCHW;
}

bool isVPCMPUQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPUQZ128rmi:
	case VPCMPUQZ128rmib:
	case VPCMPUQZ128rmibk:
	case VPCMPUQZ128rmik:
	case VPCMPUQZ128rri:
	case VPCMPUQZ128rrik:
	case VPCMPUQZ256rmi:
	case VPCMPUQZ256rmib:
	case VPCMPUQZ256rmibk:
	case VPCMPUQZ256rmik:
	case VPCMPUQZ256rri:
	case VPCMPUQZ256rrik:
	case VPCMPUQZrmi:
	case VPCMPUQZrmib:
	case VPCMPUQZrmibk:
	case VPCMPUQZrmik:
	case VPCMPUQZrri:
	case VPCMPUQZrrik:
		return true;
	}
	return false;
}

bool isKORTESTB(unsigned Opcode) {
	return Opcode == KORTESTBrr;
}

bool isMOVHPD(unsigned Opcode) {
	switch (Opcode) {
	case MOVHPDmr:
	case MOVHPDrm:
		return true;
	}
	return false;
}

bool isKORTESTD(unsigned Opcode) {
	return Opcode == KORTESTDrr;
}

bool isCVTSI2SD(unsigned Opcode) {
	switch (Opcode) {
	case CVTSI2SDrm_Int:
	case CVTSI2SDrr_Int:
	case CVTSI642SDrm_Int:
	case CVTSI642SDrr_Int:
		return true;
	}
	return false;
}

bool isFSUBRP(unsigned Opcode) {
	return Opcode == SUBR_FPrST0;
}

bool isIRET(unsigned Opcode) {
	return Opcode == IRET16;
}

bool isPTWRITE(unsigned Opcode) {
	switch (Opcode) {
	case PTWRITE64m:
	case PTWRITE64r:
	case PTWRITEm:
	case PTWRITEr:
		return true;
	}
	return false;
}

bool isVPCMPUW(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPUWZ128rmi:
	case VPCMPUWZ128rmik:
	case VPCMPUWZ128rri:
	case VPCMPUWZ128rrik:
	case VPCMPUWZ256rmi:
	case VPCMPUWZ256rmik:
	case VPCMPUWZ256rri:
	case VPCMPUWZ256rrik:
	case VPCMPUWZrmi:
	case VPCMPUWZrmik:
	case VPCMPUWZrri:
	case VPCMPUWZrrik:
		return true;
	}
	return false;
}

bool isKORTESTQ(unsigned Opcode) {
	return Opcode == KORTESTQrr;
}

bool isMOVHPS(unsigned Opcode) {
	switch (Opcode) {
	case MOVHPSmr:
	case MOVHPSrm:
		return true;
	}
	return false;
}

bool isFIMUL(unsigned Opcode) {
	switch (Opcode) {
	case MUL_FI16m:
	case MUL_FI32m:
		return true;
	}
	return false;
}

bool isCVTSI2SS(unsigned Opcode) {
	switch (Opcode) {
	case CVTSI2SSrm_Int:
	case CVTSI2SSrr_Int:
	case CVTSI642SSrm_Int:
	case CVTSI642SSrr_Int:
		return true;
	}
	return false;
}

bool isVCVTPH2PD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2PDZ128rm:
	case VCVTPH2PDZ128rmb:
	case VCVTPH2PDZ128rmbk:
	case VCVTPH2PDZ128rmbkz:
	case VCVTPH2PDZ128rmk:
	case VCVTPH2PDZ128rmkz:
	case VCVTPH2PDZ128rr:
	case VCVTPH2PDZ128rrk:
	case VCVTPH2PDZ128rrkz:
	case VCVTPH2PDZ256rm:
	case VCVTPH2PDZ256rmb:
	case VCVTPH2PDZ256rmbk:
	case VCVTPH2PDZ256rmbkz:
	case VCVTPH2PDZ256rmk:
	case VCVTPH2PDZ256rmkz:
	case VCVTPH2PDZ256rr:
	case VCVTPH2PDZ256rrk:
	case VCVTPH2PDZ256rrkz:
	case VCVTPH2PDZrm:
	case VCVTPH2PDZrmb:
	case VCVTPH2PDZrmbk:
	case VCVTPH2PDZrmbkz:
	case VCVTPH2PDZrmk:
	case VCVTPH2PDZrmkz:
	case VCVTPH2PDZrr:
	case VCVTPH2PDZrrb:
	case VCVTPH2PDZrrbk:
	case VCVTPH2PDZrrbkz:
	case VCVTPH2PDZrrk:
	case VCVTPH2PDZrrkz:
		return true;
	}
	return false;
}

bool isKORTESTW(unsigned Opcode) {
	return Opcode == KORTESTWrr;
}

bool isPADDUSB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDUSBrm:
	case MMX_PADDUSBrr:
	case PADDUSBrm:
	case PADDUSBrr:
		return true;
	}
	return false;
}

bool isVSHUFI64X2(unsigned Opcode) {
	switch (Opcode) {
	case VSHUFI64X2Z256rmbi:
	case VSHUFI64X2Z256rmbik:
	case VSHUFI64X2Z256rmbikz:
	case VSHUFI64X2Z256rmi:
	case VSHUFI64X2Z256rmik:
	case VSHUFI64X2Z256rmikz:
	case VSHUFI64X2Z256rri:
	case VSHUFI64X2Z256rrik:
	case VSHUFI64X2Z256rrikz:
	case VSHUFI64X2Zrmbi:
	case VSHUFI64X2Zrmbik:
	case VSHUFI64X2Zrmbikz:
	case VSHUFI64X2Zrmi:
	case VSHUFI64X2Zrmik:
	case VSHUFI64X2Zrmikz:
	case VSHUFI64X2Zrri:
	case VSHUFI64X2Zrrik:
	case VSHUFI64X2Zrrikz:
		return true;
	}
	return false;
}

bool isVRSQRTSH(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRTSHZrm:
	case VRSQRTSHZrmk:
	case VRSQRTSHZrmkz:
	case VRSQRTSHZrr:
	case VRSQRTSHZrrk:
	case VRSQRTSHZrrkz:
		return true;
	}
	return false;
}

bool isVMWRITE(unsigned Opcode) {
	switch (Opcode) {
	case VMWRITE32rm:
	case VMWRITE32rr:
	case VMWRITE64rm:
	case VMWRITE64rr:
		return true;
	}
	return false;
}

bool isTILERELEASE(unsigned Opcode) {
	return Opcode == TILERELEASE;
}

bool isVCVTPH2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2PSYrm:
	case VCVTPH2PSYrr:
	case VCVTPH2PSZ128rm:
	case VCVTPH2PSZ128rmk:
	case VCVTPH2PSZ128rmkz:
	case VCVTPH2PSZ128rr:
	case VCVTPH2PSZ128rrk:
	case VCVTPH2PSZ128rrkz:
	case VCVTPH2PSZ256rm:
	case VCVTPH2PSZ256rmk:
	case VCVTPH2PSZ256rmkz:
	case VCVTPH2PSZ256rr:
	case VCVTPH2PSZ256rrk:
	case VCVTPH2PSZ256rrkz:
	case VCVTPH2PSZrm:
	case VCVTPH2PSZrmk:
	case VCVTPH2PSZrmkz:
	case VCVTPH2PSZrr:
	case VCVTPH2PSZrrb:
	case VCVTPH2PSZrrbk:
	case VCVTPH2PSZrrbkz:
	case VCVTPH2PSZrrk:
	case VCVTPH2PSZrrkz:
	case VCVTPH2PSrm:
	case VCVTPH2PSrr:
		return true;
	}
	return false;
}

bool isVMOVUPD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVUPDYmr:
	case VMOVUPDYrm:
	case VMOVUPDYrr:
	case VMOVUPDYrr_REV:
	case VMOVUPDZ128mr:
	case VMOVUPDZ128mrk:
	case VMOVUPDZ128rm:
	case VMOVUPDZ128rmk:
	case VMOVUPDZ128rmkz:
	case VMOVUPDZ128rr:
	case VMOVUPDZ128rr_REV:
	case VMOVUPDZ128rrk:
	case VMOVUPDZ128rrk_REV:
	case VMOVUPDZ128rrkz:
	case VMOVUPDZ128rrkz_REV:
	case VMOVUPDZ256mr:
	case VMOVUPDZ256mrk:
	case VMOVUPDZ256rm:
	case VMOVUPDZ256rmk:
	case VMOVUPDZ256rmkz:
	case VMOVUPDZ256rr:
	case VMOVUPDZ256rr_REV:
	case VMOVUPDZ256rrk:
	case VMOVUPDZ256rrk_REV:
	case VMOVUPDZ256rrkz:
	case VMOVUPDZ256rrkz_REV:
	case VMOVUPDZmr:
	case VMOVUPDZmrk:
	case VMOVUPDZrm:
	case VMOVUPDZrmk:
	case VMOVUPDZrmkz:
	case VMOVUPDZrr:
	case VMOVUPDZrr_REV:
	case VMOVUPDZrrk:
	case VMOVUPDZrrk_REV:
	case VMOVUPDZrrkz:
	case VMOVUPDZrrkz_REV:
	case VMOVUPDmr:
	case VMOVUPDrm:
	case VMOVUPDrr:
	case VMOVUPDrr_REV:
		return true;
	}
	return false;
}

bool isPADDUSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PADDUSWrm:
	case MMX_PADDUSWrr:
	case PADDUSWrm:
	case PADDUSWrr:
		return true;
	}
	return false;
}

bool isVRSQRTSS(unsigned Opcode) {
	switch (Opcode) {
	case VRSQRTSSm_Int:
	case VRSQRTSSr_Int:
		return true;
	}
	return false;
}

bool isVPCMOV(unsigned Opcode) {
	switch (Opcode) {
	case VPCMOVYrmr:
	case VPCMOVYrrm:
	case VPCMOVYrrr:
	case VPCMOVYrrr_REV:
	case VPCMOVrmr:
	case VPCMOVrrm:
	case VPCMOVrrr:
	case VPCMOVrrr_REV:
		return true;
	}
	return false;
}

bool isVCVTUSI2SD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUSI2SDZrm_Int:
	case VCVTUSI2SDZrr_Int:
	case VCVTUSI642SDZrm_Int:
	case VCVTUSI642SDZrr_Int:
	case VCVTUSI642SDZrrb_Int:
		return true;
	}
	return false;
}

bool isVMOVUPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVUPSYmr:
	case VMOVUPSYrm:
	case VMOVUPSYrr:
	case VMOVUPSYrr_REV:
	case VMOVUPSZ128mr:
	case VMOVUPSZ128mrk:
	case VMOVUPSZ128rm:
	case VMOVUPSZ128rmk:
	case VMOVUPSZ128rmkz:
	case VMOVUPSZ128rr:
	case VMOVUPSZ128rr_REV:
	case VMOVUPSZ128rrk:
	case VMOVUPSZ128rrk_REV:
	case VMOVUPSZ128rrkz:
	case VMOVUPSZ128rrkz_REV:
	case VMOVUPSZ256mr:
	case VMOVUPSZ256mrk:
	case VMOVUPSZ256rm:
	case VMOVUPSZ256rmk:
	case VMOVUPSZ256rmkz:
	case VMOVUPSZ256rr:
	case VMOVUPSZ256rr_REV:
	case VMOVUPSZ256rrk:
	case VMOVUPSZ256rrk_REV:
	case VMOVUPSZ256rrkz:
	case VMOVUPSZ256rrkz_REV:
	case VMOVUPSZmr:
	case VMOVUPSZmrk:
	case VMOVUPSZrm:
	case VMOVUPSZrmk:
	case VMOVUPSZrmkz:
	case VMOVUPSZrr:
	case VMOVUPSZrr_REV:
	case VMOVUPSZrrk:
	case VMOVUPSZrrk_REV:
	case VMOVUPSZrrkz:
	case VMOVUPSZrrkz_REV:
	case VMOVUPSmr:
	case VMOVUPSrm:
	case VMOVUPSrr:
	case VMOVUPSrr_REV:
		return true;
	}
	return false;
}

bool isVCVTUSI2SH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUSI2SHZrm_Int:
	case VCVTUSI2SHZrr_Int:
	case VCVTUSI2SHZrrb_Int:
	case VCVTUSI642SHZrm_Int:
	case VCVTUSI642SHZrr_Int:
	case VCVTUSI642SHZrrb_Int:
		return true;
	}
	return false;
}

bool isVPCMPB(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPBZ128rmi:
	case VPCMPBZ128rmik:
	case VPCMPBZ128rri:
	case VPCMPBZ128rrik:
	case VPCMPBZ256rmi:
	case VPCMPBZ256rmik:
	case VPCMPBZ256rri:
	case VPCMPBZ256rrik:
	case VPCMPBZrmi:
	case VPCMPBZrmik:
	case VPCMPBZrri:
	case VPCMPBZrrik:
		return true;
	}
	return false;
}

bool isVPGATHERDD(unsigned Opcode) {
	switch (Opcode) {
	case VPGATHERDDYrm:
	case VPGATHERDDZ128rm:
	case VPGATHERDDZ256rm:
	case VPGATHERDDZrm:
	case VPGATHERDDrm:
		return true;
	}
	return false;
}

bool isVPCMPD(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPDZ128rmi:
	case VPCMPDZ128rmib:
	case VPCMPDZ128rmibk:
	case VPCMPDZ128rmik:
	case VPCMPDZ128rri:
	case VPCMPDZ128rrik:
	case VPCMPDZ256rmi:
	case VPCMPDZ256rmib:
	case VPCMPDZ256rmibk:
	case VPCMPDZ256rmik:
	case VPCMPDZ256rri:
	case VPCMPDZ256rrik:
	case VPCMPDZrmi:
	case VPCMPDZrmib:
	case VPCMPDZrmibk:
	case VPCMPDZrmik:
	case VPCMPDZrri:
	case VPCMPDZrrik:
		return true;
	}
	return false;
}

bool isVCVTUSI2SS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUSI2SSZrm_Int:
	case VCVTUSI2SSZrr_Int:
	case VCVTUSI2SSZrrb_Int:
	case VCVTUSI642SSZrm_Int:
	case VCVTUSI642SSZrr_Int:
	case VCVTUSI642SSZrrb_Int:
		return true;
	}
	return false;
}

bool isLFENCE(unsigned Opcode) {
	return Opcode == LFENCE;
}

bool isVCVTPH2QQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2QQZ128rm:
	case VCVTPH2QQZ128rmb:
	case VCVTPH2QQZ128rmbk:
	case VCVTPH2QQZ128rmbkz:
	case VCVTPH2QQZ128rmk:
	case VCVTPH2QQZ128rmkz:
	case VCVTPH2QQZ128rr:
	case VCVTPH2QQZ128rrk:
	case VCVTPH2QQZ128rrkz:
	case VCVTPH2QQZ256rm:
	case VCVTPH2QQZ256rmb:
	case VCVTPH2QQZ256rmbk:
	case VCVTPH2QQZ256rmbkz:
	case VCVTPH2QQZ256rmk:
	case VCVTPH2QQZ256rmkz:
	case VCVTPH2QQZ256rr:
	case VCVTPH2QQZ256rrk:
	case VCVTPH2QQZ256rrkz:
	case VCVTPH2QQZrm:
	case VCVTPH2QQZrmb:
	case VCVTPH2QQZrmbk:
	case VCVTPH2QQZrmbkz:
	case VCVTPH2QQZrmk:
	case VCVTPH2QQZrmkz:
	case VCVTPH2QQZrr:
	case VCVTPH2QQZrrb:
	case VCVTPH2QQZrrbk:
	case VCVTPH2QQZrrbkz:
	case VCVTPH2QQZrrk:
	case VCVTPH2QQZrrkz:
		return true;
	}
	return false;
}

bool isVGATHERPF0DPD(unsigned Opcode) {
	return Opcode == VGATHERPF0DPDm;
}

bool isSEAMRET(unsigned Opcode) {
	return Opcode == SEAMRET;
}

bool isPI2FD(unsigned Opcode) {
	switch (Opcode) {
	case PI2FDrm:
	case PI2FDrr:
		return true;
	}
	return false;
}

bool isVPGATHERDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPGATHERDQYrm:
	case VPGATHERDQZ128rm:
	case VPGATHERDQZ256rm:
	case VPGATHERDQZrm:
	case VPGATHERDQrm:
		return true;
	}
	return false;
}

bool isPCMPESTRI(unsigned Opcode) {
	switch (Opcode) {
	case PCMPESTRIrm:
	case PCMPESTRIrr:
		return true;
	}
	return false;
}

bool isVPCMPQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPQZ128rmi:
	case VPCMPQZ128rmib:
	case VPCMPQZ128rmibk:
	case VPCMPQZ128rmik:
	case VPCMPQZ128rri:
	case VPCMPQZ128rrik:
	case VPCMPQZ256rmi:
	case VPCMPQZ256rmib:
	case VPCMPQZ256rmibk:
	case VPCMPQZ256rmik:
	case VPCMPQZ256rri:
	case VPCMPQZ256rrik:
	case VPCMPQZrmi:
	case VPCMPQZrmib:
	case VPCMPQZrmibk:
	case VPCMPQZrmik:
	case VPCMPQZrri:
	case VPCMPQZrrik:
		return true;
	}
	return false;
}

bool isPCMPESTRM(unsigned Opcode) {
	switch (Opcode) {
	case PCMPESTRMrm:
	case PCMPESTRMrr:
		return true;
	}
	return false;
}

bool isVPCMPW(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPWZ128rmi:
	case VPCMPWZ128rmik:
	case VPCMPWZ128rri:
	case VPCMPWZ128rrik:
	case VPCMPWZ256rmi:
	case VPCMPWZ256rmik:
	case VPCMPWZ256rri:
	case VPCMPWZ256rrik:
	case VPCMPWZrmi:
	case VPCMPWZrmik:
	case VPCMPWZrri:
	case VPCMPWZrrik:
		return true;
	}
	return false;
}

bool isVGATHERPF0DPS(unsigned Opcode) {
	return Opcode == VGATHERPF0DPSm;
}

bool isVPMOVUSDB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVUSDBZ128mr:
	case VPMOVUSDBZ128mrk:
	case VPMOVUSDBZ128rr:
	case VPMOVUSDBZ128rrk:
	case VPMOVUSDBZ128rrkz:
	case VPMOVUSDBZ256mr:
	case VPMOVUSDBZ256mrk:
	case VPMOVUSDBZ256rr:
	case VPMOVUSDBZ256rrk:
	case VPMOVUSDBZ256rrkz:
	case VPMOVUSDBZmr:
	case VPMOVUSDBZmrk:
	case VPMOVUSDBZrr:
	case VPMOVUSDBZrrk:
	case VPMOVUSDBZrrkz:
		return true;
	}
	return false;
}

bool isPI2FW(unsigned Opcode) {
	switch (Opcode) {
	case PI2FWrm:
	case PI2FWrr:
		return true;
	}
	return false;
}

bool isSYSEXITQ(unsigned Opcode) {
	return Opcode == SYSEXIT64;
}

bool isCVTPS2DQ(unsigned Opcode) {
	switch (Opcode) {
	case CVTPS2DQrm:
	case CVTPS2DQrr:
		return true;
	}
	return false;
}

bool isRDPKRU(unsigned Opcode) {
	return Opcode == RDPKRUr;
}

bool isVPMOVUSDW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVUSDWZ128mr:
	case VPMOVUSDWZ128mrk:
	case VPMOVUSDWZ128rr:
	case VPMOVUSDWZ128rrk:
	case VPMOVUSDWZ128rrkz:
	case VPMOVUSDWZ256mr:
	case VPMOVUSDWZ256mrk:
	case VPMOVUSDWZ256rr:
	case VPMOVUSDWZ256rrk:
	case VPMOVUSDWZ256rrkz:
	case VPMOVUSDWZmr:
	case VPMOVUSDWZmrk:
	case VPMOVUSDWZrr:
	case VPMOVUSDWZrrk:
	case VPMOVUSDWZrrkz:
		return true;
	}
	return false;
}

bool isPSUBB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBBrm:
	case MMX_PSUBBrr:
	case PSUBBrm:
	case PSUBBrr:
		return true;
	}
	return false;
}

bool isPSUBD(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBDrm:
	case MMX_PSUBDrr:
	case PSUBDrm:
	case PSUBDrr:
		return true;
	}
	return false;
}

bool isVPSHRDD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHRDDZ128rmbi:
	case VPSHRDDZ128rmbik:
	case VPSHRDDZ128rmbikz:
	case VPSHRDDZ128rmi:
	case VPSHRDDZ128rmik:
	case VPSHRDDZ128rmikz:
	case VPSHRDDZ128rri:
	case VPSHRDDZ128rrik:
	case VPSHRDDZ128rrikz:
	case VPSHRDDZ256rmbi:
	case VPSHRDDZ256rmbik:
	case VPSHRDDZ256rmbikz:
	case VPSHRDDZ256rmi:
	case VPSHRDDZ256rmik:
	case VPSHRDDZ256rmikz:
	case VPSHRDDZ256rri:
	case VPSHRDDZ256rrik:
	case VPSHRDDZ256rrikz:
	case VPSHRDDZrmbi:
	case VPSHRDDZrmbik:
	case VPSHRDDZrmbikz:
	case VPSHRDDZrmi:
	case VPSHRDDZrmik:
	case VPSHRDDZrmikz:
	case VPSHRDDZrri:
	case VPSHRDDZrrik:
	case VPSHRDDZrrikz:
		return true;
	}
	return false;
}

bool isRETFQ(unsigned Opcode) {
	switch (Opcode) {
	case LRET64:
	case LRETI64:
		return true;
	}
	return false;
}

bool isVPERMT2PD(unsigned Opcode) {
	switch (Opcode) {
	case VPERMT2PD128rm:
	case VPERMT2PD128rmb:
	case VPERMT2PD128rmbk:
	case VPERMT2PD128rmbkz:
	case VPERMT2PD128rmk:
	case VPERMT2PD128rmkz:
	case VPERMT2PD128rr:
	case VPERMT2PD128rrk:
	case VPERMT2PD128rrkz:
	case VPERMT2PD256rm:
	case VPERMT2PD256rmb:
	case VPERMT2PD256rmbk:
	case VPERMT2PD256rmbkz:
	case VPERMT2PD256rmk:
	case VPERMT2PD256rmkz:
	case VPERMT2PD256rr:
	case VPERMT2PD256rrk:
	case VPERMT2PD256rrkz:
	case VPERMT2PDrm:
	case VPERMT2PDrmb:
	case VPERMT2PDrmbk:
	case VPERMT2PDrmbkz:
	case VPERMT2PDrmk:
	case VPERMT2PDrmkz:
	case VPERMT2PDrr:
	case VPERMT2PDrrk:
	case VPERMT2PDrrkz:
		return true;
	}
	return false;
}

bool isMOVABS(unsigned Opcode) {
	switch (Opcode) {
	case MOV16ao64:
	case MOV16o64a:
	case MOV32ao64:
	case MOV32o64a:
	case MOV64ao64:
	case MOV64o64a:
	case MOV64ri:
	case MOV8ao64:
	case MOV8o64a:
		return true;
	}
	return false;
}

bool isVPSHRDQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSHRDQZ128rmbi:
	case VPSHRDQZ128rmbik:
	case VPSHRDQZ128rmbikz:
	case VPSHRDQZ128rmi:
	case VPSHRDQZ128rmik:
	case VPSHRDQZ128rmikz:
	case VPSHRDQZ128rri:
	case VPSHRDQZ128rrik:
	case VPSHRDQZ128rrikz:
	case VPSHRDQZ256rmbi:
	case VPSHRDQZ256rmbik:
	case VPSHRDQZ256rmbikz:
	case VPSHRDQZ256rmi:
	case VPSHRDQZ256rmik:
	case VPSHRDQZ256rmikz:
	case VPSHRDQZ256rri:
	case VPSHRDQZ256rrik:
	case VPSHRDQZ256rrikz:
	case VPSHRDQZrmbi:
	case VPSHRDQZrmbik:
	case VPSHRDQZrmbikz:
	case VPSHRDQZrmi:
	case VPSHRDQZrmik:
	case VPSHRDQZrmikz:
	case VPSHRDQZrri:
	case VPSHRDQZrrik:
	case VPSHRDQZrrikz:
		return true;
	}
	return false;
}

bool isPSUBQ(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBQrm:
	case MMX_PSUBQrr:
	case PSUBQrm:
	case PSUBQrr:
		return true;
	}
	return false;
}

bool isVPSHRDW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHRDWZ128rmi:
	case VPSHRDWZ128rmik:
	case VPSHRDWZ128rmikz:
	case VPSHRDWZ128rri:
	case VPSHRDWZ128rrik:
	case VPSHRDWZ128rrikz:
	case VPSHRDWZ256rmi:
	case VPSHRDWZ256rmik:
	case VPSHRDWZ256rmikz:
	case VPSHRDWZ256rri:
	case VPSHRDWZ256rrik:
	case VPSHRDWZ256rrikz:
	case VPSHRDWZrmi:
	case VPSHRDWZrmik:
	case VPSHRDWZrmikz:
	case VPSHRDWZrri:
	case VPSHRDWZrrik:
	case VPSHRDWZrrikz:
		return true;
	}
	return false;
}

bool isVPSHLB(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLBmr:
	case VPSHLBrm:
	case VPSHLBrr:
	case VPSHLBrr_REV:
		return true;
	}
	return false;
}

bool isPSUBW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PSUBWrm:
	case MMX_PSUBWrr:
	case PSUBWrm:
	case PSUBWrr:
		return true;
	}
	return false;
}

bool isVPSHLD(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLDmr:
	case VPSHLDrm:
	case VPSHLDrr:
	case VPSHLDrr_REV:
		return true;
	}
	return false;
}

bool isVPERMT2PS(unsigned Opcode) {
	switch (Opcode) {
	case VPERMT2PS128rm:
	case VPERMT2PS128rmb:
	case VPERMT2PS128rmbk:
	case VPERMT2PS128rmbkz:
	case VPERMT2PS128rmk:
	case VPERMT2PS128rmkz:
	case VPERMT2PS128rr:
	case VPERMT2PS128rrk:
	case VPERMT2PS128rrkz:
	case VPERMT2PS256rm:
	case VPERMT2PS256rmb:
	case VPERMT2PS256rmbk:
	case VPERMT2PS256rmbkz:
	case VPERMT2PS256rmk:
	case VPERMT2PS256rmkz:
	case VPERMT2PS256rr:
	case VPERMT2PS256rrk:
	case VPERMT2PS256rrkz:
	case VPERMT2PSrm:
	case VPERMT2PSrmb:
	case VPERMT2PSrmbk:
	case VPERMT2PSrmbkz:
	case VPERMT2PSrmk:
	case VPERMT2PSrmkz:
	case VPERMT2PSrr:
	case VPERMT2PSrrk:
	case VPERMT2PSrrkz:
		return true;
	}
	return false;
}

bool isVPUNPCKLWD(unsigned Opcode) {
	switch (Opcode) {
	case VPUNPCKLWDYrm:
	case VPUNPCKLWDYrr:
	case VPUNPCKLWDZ128rm:
	case VPUNPCKLWDZ128rmk:
	case VPUNPCKLWDZ128rmkz:
	case VPUNPCKLWDZ128rr:
	case VPUNPCKLWDZ128rrk:
	case VPUNPCKLWDZ128rrkz:
	case VPUNPCKLWDZ256rm:
	case VPUNPCKLWDZ256rmk:
	case VPUNPCKLWDZ256rmkz:
	case VPUNPCKLWDZ256rr:
	case VPUNPCKLWDZ256rrk:
	case VPUNPCKLWDZ256rrkz:
	case VPUNPCKLWDZrm:
	case VPUNPCKLWDZrmk:
	case VPUNPCKLWDZrmkz:
	case VPUNPCKLWDZrr:
	case VPUNPCKLWDZrrk:
	case VPUNPCKLWDZrrkz:
	case VPUNPCKLWDrm:
	case VPUNPCKLWDrr:
		return true;
	}
	return false;
}

bool isVPSHLQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLQmr:
	case VPSHLQrm:
	case VPSHLQrr:
	case VPSHLQrr_REV:
		return true;
	}
	return false;
}

bool isVPSHLW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHLWmr:
	case VPSHLWrm:
	case VPSHLWrr:
	case VPSHLWrr_REV:
		return true;
	}
	return false;
}

bool isLSL(unsigned Opcode) {
	switch (Opcode) {
	case LSL16rm:
	case LSL16rr:
	case LSL32rm:
	case LSL32rr:
	case LSL64rm:
	case LSL64rr:
		return true;
	}
	return false;
}

bool isVBROADCASTI128(unsigned Opcode) {
	return Opcode == VBROADCASTI128;
}

bool isLSS(unsigned Opcode) {
	switch (Opcode) {
	case LSS16rm:
	case LSS32rm:
	case LSS64rm:
		return true;
	}
	return false;
}

bool isVPHADDD(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDDYrm:
	case VPHADDDYrr:
	case VPHADDDrm:
	case VPHADDDrr:
		return true;
	}
	return false;
}

bool isADDPD(unsigned Opcode) {
	switch (Opcode) {
	case ADDPDrm:
	case ADDPDrr:
		return true;
	}
	return false;
}

bool isVMASKMOVPD(unsigned Opcode) {
	switch (Opcode) {
	case VMASKMOVPDYmr:
	case VMASKMOVPDYrm:
	case VMASKMOVPDmr:
	case VMASKMOVPDrm:
		return true;
	}
	return false;
}

bool isADDPS(unsigned Opcode) {
	switch (Opcode) {
	case ADDPSrm:
	case ADDPSrr:
		return true;
	}
	return false;
}

bool isINSB(unsigned Opcode) {
	return Opcode == INSB;
}

bool isINSD(unsigned Opcode) {
	return Opcode == INSL;
}

bool isVPHADDW(unsigned Opcode) {
	switch (Opcode) {
	case VPHADDWYrm:
	case VPHADDWYrr:
	case VPHADDWrm:
	case VPHADDWrr:
		return true;
	}
	return false;
}

bool isLTR(unsigned Opcode) {
	switch (Opcode) {
	case LTRm:
	case LTRr:
		return true;
	}
	return false;
}

bool isVMASKMOVPS(unsigned Opcode) {
	switch (Opcode) {
	case VMASKMOVPSYmr:
	case VMASKMOVPSYrm:
	case VMASKMOVPSmr:
	case VMASKMOVPSrm:
		return true;
	}
	return false;
}

bool isVCVTPH2UW(unsigned Opcode) {
	switch (Opcode) {
	case VCVTPH2UWZ128rm:
	case VCVTPH2UWZ128rmb:
	case VCVTPH2UWZ128rmbk:
	case VCVTPH2UWZ128rmbkz:
	case VCVTPH2UWZ128rmk:
	case VCVTPH2UWZ128rmkz:
	case VCVTPH2UWZ128rr:
	case VCVTPH2UWZ128rrk:
	case VCVTPH2UWZ128rrkz:
	case VCVTPH2UWZ256rm:
	case VCVTPH2UWZ256rmb:
	case VCVTPH2UWZ256rmbk:
	case VCVTPH2UWZ256rmbkz:
	case VCVTPH2UWZ256rmk:
	case VCVTPH2UWZ256rmkz:
	case VCVTPH2UWZ256rr:
	case VCVTPH2UWZ256rrk:
	case VCVTPH2UWZ256rrkz:
	case VCVTPH2UWZrm:
	case VCVTPH2UWZrmb:
	case VCVTPH2UWZrmbk:
	case VCVTPH2UWZrmbkz:
	case VCVTPH2UWZrmk:
	case VCVTPH2UWZrmkz:
	case VCVTPH2UWZrr:
	case VCVTPH2UWZrrb:
	case VCVTPH2UWZrrbk:
	case VCVTPH2UWZrrbkz:
	case VCVTPH2UWZrrk:
	case VCVTPH2UWZrrkz:
		return true;
	}
	return false;
}

bool isINT3(unsigned Opcode) {
	return Opcode == INT3;
}

bool isKNOTB(unsigned Opcode) {
	return Opcode == KNOTBrr;
}

bool isKNOTD(unsigned Opcode) {
	return Opcode == KNOTDrr;
}

bool isINSW(unsigned Opcode) {
	return Opcode == INSW;
}

bool isVBLENDVPD(unsigned Opcode) {
	switch (Opcode) {
	case VBLENDVPDYrm:
	case VBLENDVPDYrr:
	case VBLENDVPDrm:
	case VBLENDVPDrr:
		return true;
	}
	return false;
}

bool isBLSFILL(unsigned Opcode) {
	switch (Opcode) {
	case BLSFILL32rm:
	case BLSFILL32rr:
	case BLSFILL64rm:
	case BLSFILL64rr:
		return true;
	}
	return false;
}

bool isMONITOR(unsigned Opcode) {
	switch (Opcode) {
	case MONITOR32rrr:
	case MONITOR64rrr:
		return true;
	}
	return false;
}

bool isKNOTQ(unsigned Opcode) {
	return Opcode == KNOTQrr;
}

bool isCMPXCHG16B(unsigned Opcode) {
	return Opcode == CMPXCHG16B;
}

bool isKNOTW(unsigned Opcode) {
	return Opcode == KNOTWrr;
}

bool isPEXTRB(unsigned Opcode) {
	switch (Opcode) {
	case PEXTRBmr:
	case PEXTRBrr:
		return true;
	}
	return false;
}

bool isVPRORVD(unsigned Opcode) {
	switch (Opcode) {
	case VPRORVDZ128rm:
	case VPRORVDZ128rmb:
	case VPRORVDZ128rmbk:
	case VPRORVDZ128rmbkz:
	case VPRORVDZ128rmk:
	case VPRORVDZ128rmkz:
	case VPRORVDZ128rr:
	case VPRORVDZ128rrk:
	case VPRORVDZ128rrkz:
	case VPRORVDZ256rm:
	case VPRORVDZ256rmb:
	case VPRORVDZ256rmbk:
	case VPRORVDZ256rmbkz:
	case VPRORVDZ256rmk:
	case VPRORVDZ256rmkz:
	case VPRORVDZ256rr:
	case VPRORVDZ256rrk:
	case VPRORVDZ256rrkz:
	case VPRORVDZrm:
	case VPRORVDZrmb:
	case VPRORVDZrmbk:
	case VPRORVDZrmbkz:
	case VPRORVDZrmk:
	case VPRORVDZrmkz:
	case VPRORVDZrr:
	case VPRORVDZrrk:
	case VPRORVDZrrkz:
		return true;
	}
	return false;
}

bool isPEXTRD(unsigned Opcode) {
	switch (Opcode) {
	case PEXTRDmr:
	case PEXTRDrr:
		return true;
	}
	return false;
}

bool isVAESENCLAST(unsigned Opcode) {
	switch (Opcode) {
	case VAESENCLASTYrm:
	case VAESENCLASTYrr:
	case VAESENCLASTZ128rm:
	case VAESENCLASTZ128rr:
	case VAESENCLASTZ256rm:
	case VAESENCLASTZ256rr:
	case VAESENCLASTZrm:
	case VAESENCLASTZrr:
	case VAESENCLASTrm:
	case VAESENCLASTrr:
		return true;
	}
	return false;
}

bool isINTO(unsigned Opcode) {
	return Opcode == INTO;
}

bool isVBLENDVPS(unsigned Opcode) {
	switch (Opcode) {
	case VBLENDVPSYrm:
	case VBLENDVPSYrr:
	case VBLENDVPSrm:
	case VBLENDVPSrr:
		return true;
	}
	return false;
}

bool isVPRORVQ(unsigned Opcode) {
	switch (Opcode) {
	case VPRORVQZ128rm:
	case VPRORVQZ128rmb:
	case VPRORVQZ128rmbk:
	case VPRORVQZ128rmbkz:
	case VPRORVQZ128rmk:
	case VPRORVQZ128rmkz:
	case VPRORVQZ128rr:
	case VPRORVQZ128rrk:
	case VPRORVQZ128rrkz:
	case VPRORVQZ256rm:
	case VPRORVQZ256rmb:
	case VPRORVQZ256rmbk:
	case VPRORVQZ256rmbkz:
	case VPRORVQZ256rmk:
	case VPRORVQZ256rmkz:
	case VPRORVQZ256rr:
	case VPRORVQZ256rrk:
	case VPRORVQZ256rrkz:
	case VPRORVQZrm:
	case VPRORVQZrmb:
	case VPRORVQZrmbk:
	case VPRORVQZrmbkz:
	case VPRORVQZrmk:
	case VPRORVQZrmkz:
	case VPRORVQZrr:
	case VPRORVQZrrk:
	case VPRORVQZrrkz:
		return true;
	}
	return false;
}

bool isPEXTRQ(unsigned Opcode) {
	switch (Opcode) {
	case PEXTRQmr:
	case PEXTRQrr:
		return true;
	}
	return false;
}

bool isHSUBPD(unsigned Opcode) {
	switch (Opcode) {
	case HSUBPDrm:
	case HSUBPDrr:
		return true;
	}
	return false;
}

bool isPEXTRW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PEXTRWrr:
	case PEXTRWmr:
	case PEXTRWrr:
	case PEXTRWrr_REV:
		return true;
	}
	return false;
}

bool isFDIVRP(unsigned Opcode) {
	return Opcode == DIVR_FPrST0;
}

bool isSCASB(unsigned Opcode) {
	return Opcode == SCASB;
}

bool isF2XM1(unsigned Opcode) {
	return Opcode == F2XM1;
}

bool isSCASD(unsigned Opcode) {
	return Opcode == SCASL;
}

bool isFISUBR(unsigned Opcode) {
	switch (Opcode) {
	case SUBR_FI16m:
	case SUBR_FI32m:
		return true;
	}
	return false;
}

bool isMOVLPD(unsigned Opcode) {
	switch (Opcode) {
	case MOVLPDmr:
	case MOVLPDrm:
		return true;
	}
	return false;
}

bool isHSUBPS(unsigned Opcode) {
	switch (Opcode) {
	case HSUBPSrm:
	case HSUBPSrr:
		return true;
	}
	return false;
}

bool isSCASQ(unsigned Opcode) {
	return Opcode == SCASQ;
}

bool isFSTP(unsigned Opcode) {
	switch (Opcode) {
	case ST_FP32m:
	case ST_FP64m:
	case ST_FP80m:
	case ST_FPrr:
		return true;
	}
	return false;
}

bool isVDBPSADBW(unsigned Opcode) {
	switch (Opcode) {
	case VDBPSADBWZ128rmi:
	case VDBPSADBWZ128rmik:
	case VDBPSADBWZ128rmikz:
	case VDBPSADBWZ128rri:
	case VDBPSADBWZ128rrik:
	case VDBPSADBWZ128rrikz:
	case VDBPSADBWZ256rmi:
	case VDBPSADBWZ256rmik:
	case VDBPSADBWZ256rmikz:
	case VDBPSADBWZ256rri:
	case VDBPSADBWZ256rrik:
	case VDBPSADBWZ256rrikz:
	case VDBPSADBWZrmi:
	case VDBPSADBWZrmik:
	case VDBPSADBWZrmikz:
	case VDBPSADBWZrri:
	case VDBPSADBWZrrik:
	case VDBPSADBWZrrikz:
		return true;
	}
	return false;
}

bool isADDSD(unsigned Opcode) {
	switch (Opcode) {
	case ADDSDrm_Int:
	case ADDSDrr_Int:
		return true;
	}
	return false;
}

bool isMOVLPS(unsigned Opcode) {
	switch (Opcode) {
	case MOVLPSmr:
	case MOVLPSrm:
		return true;
	}
	return false;
}

bool isSCASW(unsigned Opcode) {
	return Opcode == SCASW;
}

bool isVCVTW2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTW2PHZ128rm:
	case VCVTW2PHZ128rmb:
	case VCVTW2PHZ128rmbk:
	case VCVTW2PHZ128rmbkz:
	case VCVTW2PHZ128rmk:
	case VCVTW2PHZ128rmkz:
	case VCVTW2PHZ128rr:
	case VCVTW2PHZ128rrk:
	case VCVTW2PHZ128rrkz:
	case VCVTW2PHZ256rm:
	case VCVTW2PHZ256rmb:
	case VCVTW2PHZ256rmbk:
	case VCVTW2PHZ256rmbkz:
	case VCVTW2PHZ256rmk:
	case VCVTW2PHZ256rmkz:
	case VCVTW2PHZ256rr:
	case VCVTW2PHZ256rrk:
	case VCVTW2PHZ256rrkz:
	case VCVTW2PHZrm:
	case VCVTW2PHZrmb:
	case VCVTW2PHZrmbk:
	case VCVTW2PHZrmbkz:
	case VCVTW2PHZrmk:
	case VCVTW2PHZrmkz:
	case VCVTW2PHZrr:
	case VCVTW2PHZrrb:
	case VCVTW2PHZrrbk:
	case VCVTW2PHZrrbkz:
	case VCVTW2PHZrrk:
	case VCVTW2PHZrrkz:
		return true;
	}
	return false;
}

bool isVPTEST(unsigned Opcode) {
	switch (Opcode) {
	case VPTESTYrm:
	case VPTESTYrr:
	case VPTESTrm:
	case VPTESTrr:
		return true;
	}
	return false;
}

bool isFLD1(unsigned Opcode) {
	return Opcode == LD_F1;
}

bool isWBINVD(unsigned Opcode) {
	return Opcode == WBINVD;
}

bool isADDSS(unsigned Opcode) {
	switch (Opcode) {
	case ADDSSrm_Int:
	case ADDSSrr_Int:
		return true;
	}
	return false;
}

bool isPOP(unsigned Opcode) {
	switch (Opcode) {
	case POP16r:
	case POP16rmm:
	case POP16rmr:
	case POP32r:
	case POP32rmm:
	case POP32rmr:
	case POP64r:
	case POP64rmm:
	case POP64rmr:
	case POPDS16:
	case POPDS32:
	case POPES16:
	case POPES32:
	case POPFS16:
	case POPFS32:
	case POPFS64:
	case POPGS16:
	case POPGS32:
	case POPGS64:
	case POPSS16:
	case POPSS32:
		return true;
	}
	return false;
}

bool isINVD(unsigned Opcode) {
	return Opcode == INVD;
}

bool isPOR(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PORrm:
	case MMX_PORrr:
	case PORrm:
	case PORrr:
		return true;
	}
	return false;
}

bool isAND(unsigned Opcode) {
	switch (Opcode) {
	case AND16i16:
	case AND16mi:
	case AND16mi8:
	case AND16mr:
	case AND16ri:
	case AND16ri8:
	case AND16rm:
	case AND16rr:
	case AND16rr_REV:
	case AND32i32:
	case AND32mi:
	case AND32mi8:
	case AND32mr:
	case AND32ri:
	case AND32ri8:
	case AND32rm:
	case AND32rr:
	case AND32rr_REV:
	case AND64i32:
	case AND64mi32:
	case AND64mi8:
	case AND64mr:
	case AND64ri32:
	case AND64ri8:
	case AND64rm:
	case AND64rr:
	case AND64rr_REV:
	case AND8i8:
	case AND8mi:
	case AND8mi8:
	case AND8mr:
	case AND8ri:
	case AND8ri8:
	case AND8rm:
	case AND8rr:
	case AND8rr_REV:
		return true;
	}
	return false;
}

bool isFYL2XP1(unsigned Opcode) {
	return Opcode == FYL2XP1;
}

bool isFSUB(unsigned Opcode) {
	switch (Opcode) {
	case SUB_F32m:
	case SUB_F64m:
	case SUB_FST0r:
	case SUB_FrST0:
		return true;
	}
	return false;
}

bool isENTER(unsigned Opcode) {
	return Opcode == ENTER;
}

bool isVMOVSLDUP(unsigned Opcode) {
	switch (Opcode) {
	case VMOVSLDUPYrm:
	case VMOVSLDUPYrr:
	case VMOVSLDUPZ128rm:
	case VMOVSLDUPZ128rmk:
	case VMOVSLDUPZ128rmkz:
	case VMOVSLDUPZ128rr:
	case VMOVSLDUPZ128rrk:
	case VMOVSLDUPZ128rrkz:
	case VMOVSLDUPZ256rm:
	case VMOVSLDUPZ256rmk:
	case VMOVSLDUPZ256rmkz:
	case VMOVSLDUPZ256rr:
	case VMOVSLDUPZ256rrk:
	case VMOVSLDUPZ256rrkz:
	case VMOVSLDUPZrm:
	case VMOVSLDUPZrmk:
	case VMOVSLDUPZrmkz:
	case VMOVSLDUPZrr:
	case VMOVSLDUPZrrk:
	case VMOVSLDUPZrrkz:
	case VMOVSLDUPrm:
	case VMOVSLDUPrr:
		return true;
	}
	return false;
}

bool isADCX(unsigned Opcode) {
	switch (Opcode) {
	case ADCX32rm:
	case ADCX32rr:
	case ADCX64rm:
	case ADCX64rr:
		return true;
	}
	return false;
}

bool isXADD(unsigned Opcode) {
	switch (Opcode) {
	case XADD16rm:
	case XADD16rr:
	case XADD32rm:
	case XADD32rr:
	case XADD64rm:
	case XADD64rr:
	case XADD8rm:
	case XADD8rr:
		return true;
	}
	return false;
}

bool isAESENC(unsigned Opcode) {
	switch (Opcode) {
	case AESENCrm:
	case AESENCrr:
		return true;
	}
	return false;
}

bool isFLDZ(unsigned Opcode) {
	return Opcode == LD_F0;
}

bool isXRSTORS64(unsigned Opcode) {
	return Opcode == XRSTORS64;
}

bool isVCVTTSH2USI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTSH2USI64Zrm_Int:
	case VCVTTSH2USI64Zrr_Int:
	case VCVTTSH2USI64Zrrb_Int:
	case VCVTTSH2USIZrm_Int:
	case VCVTTSH2USIZrr_Int:
	case VCVTTSH2USIZrrb_Int:
		return true;
	}
	return false;
}

bool isVMULPD(unsigned Opcode) {
	switch (Opcode) {
	case VMULPDYrm:
	case VMULPDYrr:
	case VMULPDZ128rm:
	case VMULPDZ128rmb:
	case VMULPDZ128rmbk:
	case VMULPDZ128rmbkz:
	case VMULPDZ128rmk:
	case VMULPDZ128rmkz:
	case VMULPDZ128rr:
	case VMULPDZ128rrk:
	case VMULPDZ128rrkz:
	case VMULPDZ256rm:
	case VMULPDZ256rmb:
	case VMULPDZ256rmbk:
	case VMULPDZ256rmbkz:
	case VMULPDZ256rmk:
	case VMULPDZ256rmkz:
	case VMULPDZ256rr:
	case VMULPDZ256rrk:
	case VMULPDZ256rrkz:
	case VMULPDZrm:
	case VMULPDZrmb:
	case VMULPDZrmbk:
	case VMULPDZrmbkz:
	case VMULPDZrmk:
	case VMULPDZrmkz:
	case VMULPDZrr:
	case VMULPDZrrb:
	case VMULPDZrrbk:
	case VMULPDZrrbkz:
	case VMULPDZrrk:
	case VMULPDZrrkz:
	case VMULPDrm:
	case VMULPDrr:
		return true;
	}
	return false;
}

bool isFDIVP(unsigned Opcode) {
	return Opcode == DIV_FPrST0;
}

bool isVGETMANTPD(unsigned Opcode) {
	switch (Opcode) {
	case VGETMANTPDZ128rmbi:
	case VGETMANTPDZ128rmbik:
	case VGETMANTPDZ128rmbikz:
	case VGETMANTPDZ128rmi:
	case VGETMANTPDZ128rmik:
	case VGETMANTPDZ128rmikz:
	case VGETMANTPDZ128rri:
	case VGETMANTPDZ128rrik:
	case VGETMANTPDZ128rrikz:
	case VGETMANTPDZ256rmbi:
	case VGETMANTPDZ256rmbik:
	case VGETMANTPDZ256rmbikz:
	case VGETMANTPDZ256rmi:
	case VGETMANTPDZ256rmik:
	case VGETMANTPDZ256rmikz:
	case VGETMANTPDZ256rri:
	case VGETMANTPDZ256rrik:
	case VGETMANTPDZ256rrikz:
	case VGETMANTPDZrmbi:
	case VGETMANTPDZrmbik:
	case VGETMANTPDZrmbikz:
	case VGETMANTPDZrmi:
	case VGETMANTPDZrmik:
	case VGETMANTPDZrmikz:
	case VGETMANTPDZrri:
	case VGETMANTPDZrrib:
	case VGETMANTPDZrribk:
	case VGETMANTPDZrribkz:
	case VGETMANTPDZrrik:
	case VGETMANTPDZrrikz:
		return true;
	}
	return false;
}

bool isFDIVR(unsigned Opcode) {
	switch (Opcode) {
	case DIVR_F32m:
	case DIVR_F64m:
	case DIVR_FST0r:
	case DIVR_FrST0:
		return true;
	}
	return false;
}

bool isVPMOVM2D(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVM2DZ128rr:
	case VPMOVM2DZ256rr:
	case VPMOVM2DZrr:
		return true;
	}
	return false;
}

bool isVMULPH(unsigned Opcode) {
	switch (Opcode) {
	case VMULPHZ128rm:
	case VMULPHZ128rmb:
	case VMULPHZ128rmbk:
	case VMULPHZ128rmbkz:
	case VMULPHZ128rmk:
	case VMULPHZ128rmkz:
	case VMULPHZ128rr:
	case VMULPHZ128rrk:
	case VMULPHZ128rrkz:
	case VMULPHZ256rm:
	case VMULPHZ256rmb:
	case VMULPHZ256rmbk:
	case VMULPHZ256rmbkz:
	case VMULPHZ256rmk:
	case VMULPHZ256rmkz:
	case VMULPHZ256rr:
	case VMULPHZ256rrk:
	case VMULPHZ256rrkz:
	case VMULPHZrm:
	case VMULPHZrmb:
	case VMULPHZrmbk:
	case VMULPHZrmbkz:
	case VMULPHZrmk:
	case VMULPHZrmkz:
	case VMULPHZrr:
	case VMULPHZrrb:
	case VMULPHZrrbk:
	case VMULPHZrrbkz:
	case VMULPHZrrk:
	case VMULPHZrrkz:
		return true;
	}
	return false;
}

bool isVGETMANTPH(unsigned Opcode) {
	switch (Opcode) {
	case VGETMANTPHZ128rmbi:
	case VGETMANTPHZ128rmbik:
	case VGETMANTPHZ128rmbikz:
	case VGETMANTPHZ128rmi:
	case VGETMANTPHZ128rmik:
	case VGETMANTPHZ128rmikz:
	case VGETMANTPHZ128rri:
	case VGETMANTPHZ128rrik:
	case VGETMANTPHZ128rrikz:
	case VGETMANTPHZ256rmbi:
	case VGETMANTPHZ256rmbik:
	case VGETMANTPHZ256rmbikz:
	case VGETMANTPHZ256rmi:
	case VGETMANTPHZ256rmik:
	case VGETMANTPHZ256rmikz:
	case VGETMANTPHZ256rri:
	case VGETMANTPHZ256rrik:
	case VGETMANTPHZ256rrikz:
	case VGETMANTPHZrmbi:
	case VGETMANTPHZrmbik:
	case VGETMANTPHZrmbikz:
	case VGETMANTPHZrmi:
	case VGETMANTPHZrmik:
	case VGETMANTPHZrmikz:
	case VGETMANTPHZrri:
	case VGETMANTPHZrrib:
	case VGETMANTPHZrribk:
	case VGETMANTPHZrribkz:
	case VGETMANTPHZrrik:
	case VGETMANTPHZrrikz:
		return true;
	}
	return false;
}

bool isVPMOVM2B(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVM2BZ128rr:
	case VPMOVM2BZ256rr:
	case VPMOVM2BZrr:
		return true;
	}
	return false;
}

bool isAOR(unsigned Opcode) {
	switch (Opcode) {
	case AOR32mr:
	case AOR64mr:
		return true;
	}
	return false;
}

bool isVPHSUBSW(unsigned Opcode) {
	switch (Opcode) {
	case VPHSUBSWYrm:
	case VPHSUBSWYrr:
	case VPHSUBSWrm:
	case VPHSUBSWrr:
		return true;
	}
	return false;
}

bool isVMULPS(unsigned Opcode) {
	switch (Opcode) {
	case VMULPSYrm:
	case VMULPSYrr:
	case VMULPSZ128rm:
	case VMULPSZ128rmb:
	case VMULPSZ128rmbk:
	case VMULPSZ128rmbkz:
	case VMULPSZ128rmk:
	case VMULPSZ128rmkz:
	case VMULPSZ128rr:
	case VMULPSZ128rrk:
	case VMULPSZ128rrkz:
	case VMULPSZ256rm:
	case VMULPSZ256rmb:
	case VMULPSZ256rmbk:
	case VMULPSZ256rmbkz:
	case VMULPSZ256rmk:
	case VMULPSZ256rmkz:
	case VMULPSZ256rr:
	case VMULPSZ256rrk:
	case VMULPSZ256rrkz:
	case VMULPSZrm:
	case VMULPSZrmb:
	case VMULPSZrmbk:
	case VMULPSZrmbkz:
	case VMULPSZrmk:
	case VMULPSZrmkz:
	case VMULPSZrr:
	case VMULPSZrrb:
	case VMULPSZrrbk:
	case VMULPSZrrbkz:
	case VMULPSZrrk:
	case VMULPSZrrkz:
	case VMULPSrm:
	case VMULPSrr:
		return true;
	}
	return false;
}

bool isVPMOVM2Q(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVM2QZ128rr:
	case VPMOVM2QZ256rr:
	case VPMOVM2QZrr:
		return true;
	}
	return false;
}

bool isMOVNTDQ(unsigned Opcode) {
	return Opcode == MOVNTDQmr;
}

bool isVGETMANTPS(unsigned Opcode) {
	switch (Opcode) {
	case VGETMANTPSZ128rmbi:
	case VGETMANTPSZ128rmbik:
	case VGETMANTPSZ128rmbikz:
	case VGETMANTPSZ128rmi:
	case VGETMANTPSZ128rmik:
	case VGETMANTPSZ128rmikz:
	case VGETMANTPSZ128rri:
	case VGETMANTPSZ128rrik:
	case VGETMANTPSZ128rrikz:
	case VGETMANTPSZ256rmbi:
	case VGETMANTPSZ256rmbik:
	case VGETMANTPSZ256rmbikz:
	case VGETMANTPSZ256rmi:
	case VGETMANTPSZ256rmik:
	case VGETMANTPSZ256rmikz:
	case VGETMANTPSZ256rri:
	case VGETMANTPSZ256rrik:
	case VGETMANTPSZ256rrikz:
	case VGETMANTPSZrmbi:
	case VGETMANTPSZrmbik:
	case VGETMANTPSZrmbikz:
	case VGETMANTPSZrmi:
	case VGETMANTPSZrmik:
	case VGETMANTPSZrmikz:
	case VGETMANTPSZrri:
	case VGETMANTPSZrrib:
	case VGETMANTPSZrribk:
	case VGETMANTPSZrribkz:
	case VGETMANTPSZrrik:
	case VGETMANTPSZrrikz:
		return true;
	}
	return false;
}

bool isVPCMPESTRI(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPESTRIrm:
	case VPCMPESTRIrr:
		return true;
	}
	return false;
}

bool isVPMOVM2W(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVM2WZ128rr:
	case VPMOVM2WZ256rr:
	case VPMOVM2WZrr:
		return true;
	}
	return false;
}

bool isVPCMPESTRM(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPESTRMrm:
	case VPCMPESTRMrr:
		return true;
	}
	return false;
}

bool isSYSENTER(unsigned Opcode) {
	return Opcode == SYSENTER;
}

bool isVPERMPD(unsigned Opcode) {
	switch (Opcode) {
	case VPERMPDYmi:
	case VPERMPDYri:
	case VPERMPDZ256mbi:
	case VPERMPDZ256mbik:
	case VPERMPDZ256mbikz:
	case VPERMPDZ256mi:
	case VPERMPDZ256mik:
	case VPERMPDZ256mikz:
	case VPERMPDZ256ri:
	case VPERMPDZ256rik:
	case VPERMPDZ256rikz:
	case VPERMPDZ256rm:
	case VPERMPDZ256rmb:
	case VPERMPDZ256rmbk:
	case VPERMPDZ256rmbkz:
	case VPERMPDZ256rmk:
	case VPERMPDZ256rmkz:
	case VPERMPDZ256rr:
	case VPERMPDZ256rrk:
	case VPERMPDZ256rrkz:
	case VPERMPDZmbi:
	case VPERMPDZmbik:
	case VPERMPDZmbikz:
	case VPERMPDZmi:
	case VPERMPDZmik:
	case VPERMPDZmikz:
	case VPERMPDZri:
	case VPERMPDZrik:
	case VPERMPDZrikz:
	case VPERMPDZrm:
	case VPERMPDZrmb:
	case VPERMPDZrmbk:
	case VPERMPDZrmbkz:
	case VPERMPDZrmk:
	case VPERMPDZrmkz:
	case VPERMPDZrr:
	case VPERMPDZrrk:
	case VPERMPDZrrkz:
		return true;
	}
	return false;
}

bool isVCVTTPH2QQ(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPH2QQZ128rm:
	case VCVTTPH2QQZ128rmb:
	case VCVTTPH2QQZ128rmbk:
	case VCVTTPH2QQZ128rmbkz:
	case VCVTTPH2QQZ128rmk:
	case VCVTTPH2QQZ128rmkz:
	case VCVTTPH2QQZ128rr:
	case VCVTTPH2QQZ128rrk:
	case VCVTTPH2QQZ128rrkz:
	case VCVTTPH2QQZ256rm:
	case VCVTTPH2QQZ256rmb:
	case VCVTTPH2QQZ256rmbk:
	case VCVTTPH2QQZ256rmbkz:
	case VCVTTPH2QQZ256rmk:
	case VCVTTPH2QQZ256rmkz:
	case VCVTTPH2QQZ256rr:
	case VCVTTPH2QQZ256rrk:
	case VCVTTPH2QQZ256rrkz:
	case VCVTTPH2QQZrm:
	case VCVTTPH2QQZrmb:
	case VCVTTPH2QQZrmbk:
	case VCVTTPH2QQZrmbkz:
	case VCVTTPH2QQZrmk:
	case VCVTTPH2QQZrmkz:
	case VCVTTPH2QQZrr:
	case VCVTTPH2QQZrrb:
	case VCVTTPH2QQZrrbk:
	case VCVTTPH2QQZrrbkz:
	case VCVTTPH2QQZrrk:
	case VCVTTPH2QQZrrkz:
		return true;
	}
	return false;
}

bool isPUSHF(unsigned Opcode) {
	return Opcode == PUSHF16;
}

bool isPXOR(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PXORrm:
	case MMX_PXORrr:
	case PXORrm:
	case PXORrr:
		return true;
	}
	return false;
}

bool isCMPXCHG(unsigned Opcode) {
	switch (Opcode) {
	case CMPXCHG16rm:
	case CMPXCHG16rr:
	case CMPXCHG32rm:
	case CMPXCHG32rr:
	case CMPXCHG64rm:
	case CMPXCHG64rr:
	case CMPXCHG8rm:
	case CMPXCHG8rr:
		return true;
	}
	return false;
}

bool isVPERMPS(unsigned Opcode) {
	switch (Opcode) {
	case VPERMPSYrm:
	case VPERMPSYrr:
	case VPERMPSZ256rm:
	case VPERMPSZ256rmb:
	case VPERMPSZ256rmbk:
	case VPERMPSZ256rmbkz:
	case VPERMPSZ256rmk:
	case VPERMPSZ256rmkz:
	case VPERMPSZ256rr:
	case VPERMPSZ256rrk:
	case VPERMPSZ256rrkz:
	case VPERMPSZrm:
	case VPERMPSZrmb:
	case VPERMPSZrmbk:
	case VPERMPSZrmbkz:
	case VPERMPSZrmk:
	case VPERMPSZrmkz:
	case VPERMPSZrr:
	case VPERMPSZrrk:
	case VPERMPSZrrkz:
		return true;
	}
	return false;
}

bool isVMRESUME(unsigned Opcode) {
	return Opcode == VMRESUME;
}

bool isVPSLLD(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLDYri:
	case VPSLLDYrm:
	case VPSLLDYrr:
	case VPSLLDZ128mbi:
	case VPSLLDZ128mbik:
	case VPSLLDZ128mbikz:
	case VPSLLDZ128mi:
	case VPSLLDZ128mik:
	case VPSLLDZ128mikz:
	case VPSLLDZ128ri:
	case VPSLLDZ128rik:
	case VPSLLDZ128rikz:
	case VPSLLDZ128rm:
	case VPSLLDZ128rmk:
	case VPSLLDZ128rmkz:
	case VPSLLDZ128rr:
	case VPSLLDZ128rrk:
	case VPSLLDZ128rrkz:
	case VPSLLDZ256mbi:
	case VPSLLDZ256mbik:
	case VPSLLDZ256mbikz:
	case VPSLLDZ256mi:
	case VPSLLDZ256mik:
	case VPSLLDZ256mikz:
	case VPSLLDZ256ri:
	case VPSLLDZ256rik:
	case VPSLLDZ256rikz:
	case VPSLLDZ256rm:
	case VPSLLDZ256rmk:
	case VPSLLDZ256rmkz:
	case VPSLLDZ256rr:
	case VPSLLDZ256rrk:
	case VPSLLDZ256rrkz:
	case VPSLLDZmbi:
	case VPSLLDZmbik:
	case VPSLLDZmbikz:
	case VPSLLDZmi:
	case VPSLLDZmik:
	case VPSLLDZmikz:
	case VPSLLDZri:
	case VPSLLDZrik:
	case VPSLLDZrikz:
	case VPSLLDZrm:
	case VPSLLDZrmk:
	case VPSLLDZrmkz:
	case VPSLLDZrr:
	case VPSLLDZrrk:
	case VPSLLDZrrkz:
	case VPSLLDri:
	case VPSLLDrm:
	case VPSLLDrr:
		return true;
	}
	return false;
}

bool isVPSLLQ(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLQYri:
	case VPSLLQYrm:
	case VPSLLQYrr:
	case VPSLLQZ128mbi:
	case VPSLLQZ128mbik:
	case VPSLLQZ128mbikz:
	case VPSLLQZ128mi:
	case VPSLLQZ128mik:
	case VPSLLQZ128mikz:
	case VPSLLQZ128ri:
	case VPSLLQZ128rik:
	case VPSLLQZ128rikz:
	case VPSLLQZ128rm:
	case VPSLLQZ128rmk:
	case VPSLLQZ128rmkz:
	case VPSLLQZ128rr:
	case VPSLLQZ128rrk:
	case VPSLLQZ128rrkz:
	case VPSLLQZ256mbi:
	case VPSLLQZ256mbik:
	case VPSLLQZ256mbikz:
	case VPSLLQZ256mi:
	case VPSLLQZ256mik:
	case VPSLLQZ256mikz:
	case VPSLLQZ256ri:
	case VPSLLQZ256rik:
	case VPSLLQZ256rikz:
	case VPSLLQZ256rm:
	case VPSLLQZ256rmk:
	case VPSLLQZ256rmkz:
	case VPSLLQZ256rr:
	case VPSLLQZ256rrk:
	case VPSLLQZ256rrkz:
	case VPSLLQZmbi:
	case VPSLLQZmbik:
	case VPSLLQZmbikz:
	case VPSLLQZmi:
	case VPSLLQZmik:
	case VPSLLQZmikz:
	case VPSLLQZri:
	case VPSLLQZrik:
	case VPSLLQZrikz:
	case VPSLLQZrm:
	case VPSLLQZrmk:
	case VPSLLQZrmkz:
	case VPSLLQZrr:
	case VPSLLQZrrk:
	case VPSLLQZrrkz:
	case VPSLLQri:
	case VPSLLQrm:
	case VPSLLQrr:
		return true;
	}
	return false;
}

bool isVEXTRACTF32X4(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTF32x4Z256mr:
	case VEXTRACTF32x4Z256mrk:
	case VEXTRACTF32x4Z256rr:
	case VEXTRACTF32x4Z256rrk:
	case VEXTRACTF32x4Z256rrkz:
	case VEXTRACTF32x4Zmr:
	case VEXTRACTF32x4Zmrk:
	case VEXTRACTF32x4Zrr:
	case VEXTRACTF32x4Zrrk:
	case VEXTRACTF32x4Zrrkz:
		return true;
	}
	return false;
}

bool isVPSLLW(unsigned Opcode) {
	switch (Opcode) {
	case VPSLLWYri:
	case VPSLLWYrm:
	case VPSLLWYrr:
	case VPSLLWZ128mi:
	case VPSLLWZ128mik:
	case VPSLLWZ128mikz:
	case VPSLLWZ128ri:
	case VPSLLWZ128rik:
	case VPSLLWZ128rikz:
	case VPSLLWZ128rm:
	case VPSLLWZ128rmk:
	case VPSLLWZ128rmkz:
	case VPSLLWZ128rr:
	case VPSLLWZ128rrk:
	case VPSLLWZ128rrkz:
	case VPSLLWZ256mi:
	case VPSLLWZ256mik:
	case VPSLLWZ256mikz:
	case VPSLLWZ256ri:
	case VPSLLWZ256rik:
	case VPSLLWZ256rikz:
	case VPSLLWZ256rm:
	case VPSLLWZ256rmk:
	case VPSLLWZ256rmkz:
	case VPSLLWZ256rr:
	case VPSLLWZ256rrk:
	case VPSLLWZ256rrkz:
	case VPSLLWZmi:
	case VPSLLWZmik:
	case VPSLLWZmikz:
	case VPSLLWZri:
	case VPSLLWZrik:
	case VPSLLWZrikz:
	case VPSLLWZrm:
	case VPSLLWZrmk:
	case VPSLLWZrmkz:
	case VPSLLWZrr:
	case VPSLLWZrrk:
	case VPSLLWZrrkz:
	case VPSLLWri:
	case VPSLLWrm:
	case VPSLLWrr:
		return true;
	}
	return false;
}

bool isBLSI(unsigned Opcode) {
	switch (Opcode) {
	case BLSI32rm:
	case BLSI32rr:
	case BLSI64rm:
	case BLSI64rr:
		return true;
	}
	return false;
}

bool isVEXTRACTF32X8(unsigned Opcode) {
	switch (Opcode) {
	case VEXTRACTF32x8Zmr:
	case VEXTRACTF32x8Zmrk:
	case VEXTRACTF32x8Zrr:
	case VEXTRACTF32x8Zrrk:
	case VEXTRACTF32x8Zrrkz:
		return true;
	}
	return false;
}

bool isBLSR(unsigned Opcode) {
	switch (Opcode) {
	case BLSR32rm:
	case BLSR32rr:
	case BLSR64rm:
	case BLSR64rr:
		return true;
	}
	return false;
}

bool isVMULSD(unsigned Opcode) {
	switch (Opcode) {
	case VMULSDZrm_Int:
	case VMULSDZrm_Intk:
	case VMULSDZrm_Intkz:
	case VMULSDZrr_Int:
	case VMULSDZrr_Intk:
	case VMULSDZrr_Intkz:
	case VMULSDZrrb_Int:
	case VMULSDZrrb_Intk:
	case VMULSDZrrb_Intkz:
	case VMULSDrm_Int:
	case VMULSDrr_Int:
		return true;
	}
	return false;
}

bool isVGETMANTSD(unsigned Opcode) {
	switch (Opcode) {
	case VGETMANTSDZrmi:
	case VGETMANTSDZrmik:
	case VGETMANTSDZrmikz:
	case VGETMANTSDZrri:
	case VGETMANTSDZrrib:
	case VGETMANTSDZrribk:
	case VGETMANTSDZrribkz:
	case VGETMANTSDZrrik:
	case VGETMANTSDZrrikz:
		return true;
	}
	return false;
}

bool isVPCMPEQB(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPEQBYrm:
	case VPCMPEQBYrr:
	case VPCMPEQBZ128rm:
	case VPCMPEQBZ128rmk:
	case VPCMPEQBZ128rr:
	case VPCMPEQBZ128rrk:
	case VPCMPEQBZ256rm:
	case VPCMPEQBZ256rmk:
	case VPCMPEQBZ256rr:
	case VPCMPEQBZ256rrk:
	case VPCMPEQBZrm:
	case VPCMPEQBZrmk:
	case VPCMPEQBZrr:
	case VPCMPEQBZrrk:
	case VPCMPEQBrm:
	case VPCMPEQBrr:
		return true;
	}
	return false;
}

bool isVMULSH(unsigned Opcode) {
	switch (Opcode) {
	case VMULSHZrm_Int:
	case VMULSHZrm_Intk:
	case VMULSHZrm_Intkz:
	case VMULSHZrr_Int:
	case VMULSHZrr_Intk:
	case VMULSHZrr_Intkz:
	case VMULSHZrrb_Int:
	case VMULSHZrrb_Intk:
	case VMULSHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isVPCMPEQD(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPEQDYrm:
	case VPCMPEQDYrr:
	case VPCMPEQDZ128rm:
	case VPCMPEQDZ128rmb:
	case VPCMPEQDZ128rmbk:
	case VPCMPEQDZ128rmk:
	case VPCMPEQDZ128rr:
	case VPCMPEQDZ128rrk:
	case VPCMPEQDZ256rm:
	case VPCMPEQDZ256rmb:
	case VPCMPEQDZ256rmbk:
	case VPCMPEQDZ256rmk:
	case VPCMPEQDZ256rr:
	case VPCMPEQDZ256rrk:
	case VPCMPEQDZrm:
	case VPCMPEQDZrmb:
	case VPCMPEQDZrmbk:
	case VPCMPEQDZrmk:
	case VPCMPEQDZrr:
	case VPCMPEQDZrrk:
	case VPCMPEQDrm:
	case VPCMPEQDrr:
		return true;
	}
	return false;
}

bool isVGETMANTSH(unsigned Opcode) {
	switch (Opcode) {
	case VGETMANTSHZrmi:
	case VGETMANTSHZrmik:
	case VGETMANTSHZrmikz:
	case VGETMANTSHZrri:
	case VGETMANTSHZrrib:
	case VGETMANTSHZrribk:
	case VGETMANTSHZrribkz:
	case VGETMANTSHZrrik:
	case VGETMANTSHZrrikz:
		return true;
	}
	return false;
}

bool isLMSW(unsigned Opcode) {
	switch (Opcode) {
	case LMSW16m:
	case LMSW16r:
		return true;
	}
	return false;
}

bool isFNSTENV(unsigned Opcode) {
	return Opcode == FSTENVm;
}

bool isVMULSS(unsigned Opcode) {
	switch (Opcode) {
	case VMULSSZrm_Int:
	case VMULSSZrm_Intk:
	case VMULSSZrm_Intkz:
	case VMULSSZrr_Int:
	case VMULSSZrr_Intk:
	case VMULSSZrr_Intkz:
	case VMULSSZrrb_Int:
	case VMULSSZrrb_Intk:
	case VMULSSZrrb_Intkz:
	case VMULSSrm_Int:
	case VMULSSrr_Int:
		return true;
	}
	return false;
}

bool isVGETMANTSS(unsigned Opcode) {
	switch (Opcode) {
	case VGETMANTSSZrmi:
	case VGETMANTSSZrmik:
	case VGETMANTSSZrmikz:
	case VGETMANTSSZrri:
	case VGETMANTSSZrrib:
	case VGETMANTSSZrribk:
	case VGETMANTSSZrribkz:
	case VGETMANTSSZrrik:
	case VGETMANTSSZrrikz:
		return true;
	}
	return false;
}

bool isVPCMPEQQ(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPEQQYrm:
	case VPCMPEQQYrr:
	case VPCMPEQQZ128rm:
	case VPCMPEQQZ128rmb:
	case VPCMPEQQZ128rmbk:
	case VPCMPEQQZ128rmk:
	case VPCMPEQQZ128rr:
	case VPCMPEQQZ128rrk:
	case VPCMPEQQZ256rm:
	case VPCMPEQQZ256rmb:
	case VPCMPEQQZ256rmbk:
	case VPCMPEQQZ256rmk:
	case VPCMPEQQZ256rr:
	case VPCMPEQQZ256rrk:
	case VPCMPEQQZrm:
	case VPCMPEQQZrmb:
	case VPCMPEQQZrmbk:
	case VPCMPEQQZrmk:
	case VPCMPEQQZrr:
	case VPCMPEQQZrrk:
	case VPCMPEQQrm:
	case VPCMPEQQrr:
		return true;
	}
	return false;
}

bool isVPCMPEQW(unsigned Opcode) {
	switch (Opcode) {
	case VPCMPEQWYrm:
	case VPCMPEQWYrr:
	case VPCMPEQWZ128rm:
	case VPCMPEQWZ128rmk:
	case VPCMPEQWZ128rr:
	case VPCMPEQWZ128rrk:
	case VPCMPEQWZ256rm:
	case VPCMPEQWZ256rmk:
	case VPCMPEQWZ256rr:
	case VPCMPEQWZ256rrk:
	case VPCMPEQWZrm:
	case VPCMPEQWZrmk:
	case VPCMPEQWZrr:
	case VPCMPEQWZrrk:
	case VPCMPEQWrm:
	case VPCMPEQWrr:
		return true;
	}
	return false;
}

bool isTDPBSSD(unsigned Opcode) {
	return Opcode == TDPBSSD;
}

bool isVPHSUBWD(unsigned Opcode) {
	switch (Opcode) {
	case VPHSUBWDrm:
	case VPHSUBWDrr:
		return true;
	}
	return false;
}

bool isUMWAIT(unsigned Opcode) {
	return Opcode == UMWAIT;
}

bool isBSWAP(unsigned Opcode) {
	switch (Opcode) {
	case BSWAP16r_BAD:
	case BSWAP32r:
	case BSWAP64r:
		return true;
	}
	return false;
}

bool isVCVTUQQ2PD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUQQ2PDZ128rm:
	case VCVTUQQ2PDZ128rmb:
	case VCVTUQQ2PDZ128rmbk:
	case VCVTUQQ2PDZ128rmbkz:
	case VCVTUQQ2PDZ128rmk:
	case VCVTUQQ2PDZ128rmkz:
	case VCVTUQQ2PDZ128rr:
	case VCVTUQQ2PDZ128rrk:
	case VCVTUQQ2PDZ128rrkz:
	case VCVTUQQ2PDZ256rm:
	case VCVTUQQ2PDZ256rmb:
	case VCVTUQQ2PDZ256rmbk:
	case VCVTUQQ2PDZ256rmbkz:
	case VCVTUQQ2PDZ256rmk:
	case VCVTUQQ2PDZ256rmkz:
	case VCVTUQQ2PDZ256rr:
	case VCVTUQQ2PDZ256rrk:
	case VCVTUQQ2PDZ256rrkz:
	case VCVTUQQ2PDZrm:
	case VCVTUQQ2PDZrmb:
	case VCVTUQQ2PDZrmbk:
	case VCVTUQQ2PDZrmbkz:
	case VCVTUQQ2PDZrmk:
	case VCVTUQQ2PDZrmkz:
	case VCVTUQQ2PDZrr:
	case VCVTUQQ2PDZrrb:
	case VCVTUQQ2PDZrrbk:
	case VCVTUQQ2PDZrrbkz:
	case VCVTUQQ2PDZrrk:
	case VCVTUQQ2PDZrrkz:
		return true;
	}
	return false;
}

bool isPEXT(unsigned Opcode) {
	switch (Opcode) {
	case PEXT32rm:
	case PEXT32rr:
	case PEXT64rm:
	case PEXT64rr:
		return true;
	}
	return false;
}

bool isVCVTUQQ2PH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUQQ2PHZ128rm:
	case VCVTUQQ2PHZ128rmb:
	case VCVTUQQ2PHZ128rmbk:
	case VCVTUQQ2PHZ128rmbkz:
	case VCVTUQQ2PHZ128rmk:
	case VCVTUQQ2PHZ128rmkz:
	case VCVTUQQ2PHZ128rr:
	case VCVTUQQ2PHZ128rrk:
	case VCVTUQQ2PHZ128rrkz:
	case VCVTUQQ2PHZ256rm:
	case VCVTUQQ2PHZ256rmb:
	case VCVTUQQ2PHZ256rmbk:
	case VCVTUQQ2PHZ256rmbkz:
	case VCVTUQQ2PHZ256rmk:
	case VCVTUQQ2PHZ256rmkz:
	case VCVTUQQ2PHZ256rr:
	case VCVTUQQ2PHZ256rrk:
	case VCVTUQQ2PHZ256rrkz:
	case VCVTUQQ2PHZrm:
	case VCVTUQQ2PHZrmb:
	case VCVTUQQ2PHZrmbk:
	case VCVTUQQ2PHZrmbkz:
	case VCVTUQQ2PHZrmk:
	case VCVTUQQ2PHZrmkz:
	case VCVTUQQ2PHZrr:
	case VCVTUQQ2PHZrrb:
	case VCVTUQQ2PHZrrbk:
	case VCVTUQQ2PHZrrbkz:
	case VCVTUQQ2PHZrrk:
	case VCVTUQQ2PHZrrkz:
		return true;
	}
	return false;
}

bool isVMOVMSKPD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVMSKPDYrr:
	case VMOVMSKPDrr:
		return true;
	}
	return false;
}

bool isLOADIWKEY(unsigned Opcode) {
	return Opcode == LOADIWKEY;
}

bool isPMINSB(unsigned Opcode) {
	switch (Opcode) {
	case PMINSBrm:
	case PMINSBrr:
		return true;
	}
	return false;
}

bool isPMINSD(unsigned Opcode) {
	switch (Opcode) {
	case PMINSDrm:
	case PMINSDrr:
		return true;
	}
	return false;
}

bool isVCVTUQQ2PS(unsigned Opcode) {
	switch (Opcode) {
	case VCVTUQQ2PSZ128rm:
	case VCVTUQQ2PSZ128rmb:
	case VCVTUQQ2PSZ128rmbk:
	case VCVTUQQ2PSZ128rmbkz:
	case VCVTUQQ2PSZ128rmk:
	case VCVTUQQ2PSZ128rmkz:
	case VCVTUQQ2PSZ128rr:
	case VCVTUQQ2PSZ128rrk:
	case VCVTUQQ2PSZ128rrkz:
	case VCVTUQQ2PSZ256rm:
	case VCVTUQQ2PSZ256rmb:
	case VCVTUQQ2PSZ256rmbk:
	case VCVTUQQ2PSZ256rmbkz:
	case VCVTUQQ2PSZ256rmk:
	case VCVTUQQ2PSZ256rmkz:
	case VCVTUQQ2PSZ256rr:
	case VCVTUQQ2PSZ256rrk:
	case VCVTUQQ2PSZ256rrkz:
	case VCVTUQQ2PSZrm:
	case VCVTUQQ2PSZrmb:
	case VCVTUQQ2PSZrmbk:
	case VCVTUQQ2PSZrmbkz:
	case VCVTUQQ2PSZrmk:
	case VCVTUQQ2PSZrmkz:
	case VCVTUQQ2PSZrr:
	case VCVTUQQ2PSZrrb:
	case VCVTUQQ2PSZrrbk:
	case VCVTUQQ2PSZrrbkz:
	case VCVTUQQ2PSZrrk:
	case VCVTUQQ2PSZrrkz:
		return true;
	}
	return false;
}

bool isSTMXCSR(unsigned Opcode) {
	return Opcode == STMXCSR;
}

bool isCVTPS2PD(unsigned Opcode) {
	switch (Opcode) {
	case CVTPS2PDrm:
	case CVTPS2PDrr:
		return true;
	}
	return false;
}

bool isVMOVMSKPS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVMSKPSYrr:
	case VMOVMSKPSrr:
		return true;
	}
	return false;
}

bool isVPROLD(unsigned Opcode) {
	switch (Opcode) {
	case VPROLDZ128mbi:
	case VPROLDZ128mbik:
	case VPROLDZ128mbikz:
	case VPROLDZ128mi:
	case VPROLDZ128mik:
	case VPROLDZ128mikz:
	case VPROLDZ128ri:
	case VPROLDZ128rik:
	case VPROLDZ128rikz:
	case VPROLDZ256mbi:
	case VPROLDZ256mbik:
	case VPROLDZ256mbikz:
	case VPROLDZ256mi:
	case VPROLDZ256mik:
	case VPROLDZ256mikz:
	case VPROLDZ256ri:
	case VPROLDZ256rik:
	case VPROLDZ256rikz:
	case VPROLDZmbi:
	case VPROLDZmbik:
	case VPROLDZmbikz:
	case VPROLDZmi:
	case VPROLDZmik:
	case VPROLDZmikz:
	case VPROLDZri:
	case VPROLDZrik:
	case VPROLDZrikz:
		return true;
	}
	return false;
}

bool isFPTAN(unsigned Opcode) {
	return Opcode == FPTAN;
}

bool isCVTPS2PI(unsigned Opcode) {
	switch (Opcode) {
	case MMX_CVTPS2PIrm:
	case MMX_CVTPS2PIrr:
		return true;
	}
	return false;
}

bool isVMXOFF(unsigned Opcode) {
	return Opcode == VMXOFF;
}

bool isXRSTOR64(unsigned Opcode) {
	return Opcode == XRSTOR64;
}

bool isPMINSW(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMINSWrm:
	case MMX_PMINSWrr:
	case PMINSWrm:
	case PMINSWrr:
		return true;
	}
	return false;
}

bool isVMOVSD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVSDZmr:
	case VMOVSDZmrk:
	case VMOVSDZrm:
	case VMOVSDZrmk:
	case VMOVSDZrmkz:
	case VMOVSDZrr:
	case VMOVSDZrr_REV:
	case VMOVSDZrrk:
	case VMOVSDZrrk_REV:
	case VMOVSDZrrkz:
	case VMOVSDZrrkz_REV:
	case VMOVSDmr:
	case VMOVSDrm:
	case VMOVSDrr:
	case VMOVSDrr_REV:
		return true;
	}
	return false;
}

bool isVPGATHERQD(unsigned Opcode) {
	switch (Opcode) {
	case VPGATHERQDYrm:
	case VPGATHERQDZ128rm:
	case VPGATHERQDZ256rm:
	case VPGATHERQDZrm:
	case VPGATHERQDrm:
		return true;
	}
	return false;
}

bool isVINSERTF32X4(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTF32x4Z256rm:
	case VINSERTF32x4Z256rmk:
	case VINSERTF32x4Z256rmkz:
	case VINSERTF32x4Z256rr:
	case VINSERTF32x4Z256rrk:
	case VINSERTF32x4Z256rrkz:
	case VINSERTF32x4Zrm:
	case VINSERTF32x4Zrmk:
	case VINSERTF32x4Zrmkz:
	case VINSERTF32x4Zrr:
	case VINSERTF32x4Zrrk:
	case VINSERTF32x4Zrrkz:
		return true;
	}
	return false;
}

bool isVMOVSH(unsigned Opcode) {
	switch (Opcode) {
	case VMOVSHZmr:
	case VMOVSHZmrk:
	case VMOVSHZrm:
	case VMOVSHZrmk:
	case VMOVSHZrmkz:
	case VMOVSHZrr:
	case VMOVSHZrr_REV:
	case VMOVSHZrrk:
	case VMOVSHZrrk_REV:
	case VMOVSHZrrkz:
	case VMOVSHZrrkz_REV:
		return true;
	}
	return false;
}

bool isVPGATHERQQ(unsigned Opcode) {
	switch (Opcode) {
	case VPGATHERQQYrm:
	case VPGATHERQQZ128rm:
	case VPGATHERQQZ256rm:
	case VPGATHERQQZrm:
	case VPGATHERQQrm:
		return true;
	}
	return false;
}

bool isVPROLQ(unsigned Opcode) {
	switch (Opcode) {
	case VPROLQZ128mbi:
	case VPROLQZ128mbik:
	case VPROLQZ128mbikz:
	case VPROLQZ128mi:
	case VPROLQZ128mik:
	case VPROLQZ128mikz:
	case VPROLQZ128ri:
	case VPROLQZ128rik:
	case VPROLQZ128rikz:
	case VPROLQZ256mbi:
	case VPROLQZ256mbik:
	case VPROLQZ256mbikz:
	case VPROLQZ256mi:
	case VPROLQZ256mik:
	case VPROLQZ256mikz:
	case VPROLQZ256ri:
	case VPROLQZ256rik:
	case VPROLQZ256rikz:
	case VPROLQZmbi:
	case VPROLQZmbik:
	case VPROLQZmbikz:
	case VPROLQZmi:
	case VPROLQZmik:
	case VPROLQZmikz:
	case VPROLQZri:
	case VPROLQZrik:
	case VPROLQZrikz:
		return true;
	}
	return false;
}

bool isVINSERTF32X8(unsigned Opcode) {
	switch (Opcode) {
	case VINSERTF32x8Zrm:
	case VINSERTF32x8Zrmk:
	case VINSERTF32x8Zrmkz:
	case VINSERTF32x8Zrr:
	case VINSERTF32x8Zrrk:
	case VINSERTF32x8Zrrkz:
		return true;
	}
	return false;
}

bool isXSAVE(unsigned Opcode) {
	return Opcode == XSAVE;
}

bool isTDPFP16PS(unsigned Opcode) {
	return Opcode == TDPFP16PS;
}

bool isVCVTTPH2UW(unsigned Opcode) {
	switch (Opcode) {
	case VCVTTPH2UWZ128rm:
	case VCVTTPH2UWZ128rmb:
	case VCVTTPH2UWZ128rmbk:
	case VCVTTPH2UWZ128rmbkz:
	case VCVTTPH2UWZ128rmk:
	case VCVTTPH2UWZ128rmkz:
	case VCVTTPH2UWZ128rr:
	case VCVTTPH2UWZ128rrk:
	case VCVTTPH2UWZ128rrkz:
	case VCVTTPH2UWZ256rm:
	case VCVTTPH2UWZ256rmb:
	case VCVTTPH2UWZ256rmbk:
	case VCVTTPH2UWZ256rmbkz:
	case VCVTTPH2UWZ256rmk:
	case VCVTTPH2UWZ256rmkz:
	case VCVTTPH2UWZ256rr:
	case VCVTTPH2UWZ256rrk:
	case VCVTTPH2UWZ256rrkz:
	case VCVTTPH2UWZrm:
	case VCVTTPH2UWZrmb:
	case VCVTTPH2UWZrmbk:
	case VCVTTPH2UWZrmbkz:
	case VCVTTPH2UWZrmk:
	case VCVTTPH2UWZrmkz:
	case VCVTTPH2UWZrr:
	case VCVTTPH2UWZrrb:
	case VCVTTPH2UWZrrbk:
	case VCVTTPH2UWZrrbkz:
	case VCVTTPH2UWZrrk:
	case VCVTTPH2UWZrrkz:
		return true;
	}
	return false;
}

bool isVMOVSS(unsigned Opcode) {
	switch (Opcode) {
	case VMOVSSZmr:
	case VMOVSSZmrk:
	case VMOVSSZrm:
	case VMOVSSZrmk:
	case VMOVSSZrmkz:
	case VMOVSSZrr:
	case VMOVSSZrr_REV:
	case VMOVSSZrrk:
	case VMOVSSZrrk_REV:
	case VMOVSSZrrkz:
	case VMOVSSZrrkz_REV:
	case VMOVSSmr:
	case VMOVSSrm:
	case VMOVSSrr:
	case VMOVSSrr_REV:
		return true;
	}
	return false;
}

bool isRDTSCP(unsigned Opcode) {
	return Opcode == RDTSCP;
}

bool isVPMOVUSQB(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVUSQBZ128mr:
	case VPMOVUSQBZ128mrk:
	case VPMOVUSQBZ128rr:
	case VPMOVUSQBZ128rrk:
	case VPMOVUSQBZ128rrkz:
	case VPMOVUSQBZ256mr:
	case VPMOVUSQBZ256mrk:
	case VPMOVUSQBZ256rr:
	case VPMOVUSQBZ256rrk:
	case VPMOVUSQBZ256rrkz:
	case VPMOVUSQBZmr:
	case VPMOVUSQBZmrk:
	case VPMOVUSQBZrr:
	case VPMOVUSQBZrrk:
	case VPMOVUSQBZrrkz:
		return true;
	}
	return false;
}

bool isVPMOVUSQD(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVUSQDZ128mr:
	case VPMOVUSQDZ128mrk:
	case VPMOVUSQDZ128rr:
	case VPMOVUSQDZ128rrk:
	case VPMOVUSQDZ128rrkz:
	case VPMOVUSQDZ256mr:
	case VPMOVUSQDZ256mrk:
	case VPMOVUSQDZ256rr:
	case VPMOVUSQDZ256rrk:
	case VPMOVUSQDZ256rrkz:
	case VPMOVUSQDZmr:
	case VPMOVUSQDZmrk:
	case VPMOVUSQDZrr:
	case VPMOVUSQDZrrk:
	case VPMOVUSQDZrrkz:
		return true;
	}
	return false;
}

bool isTDPBSUD(unsigned Opcode) {
	return Opcode == TDPBSUD;
}

bool isBLCMSK(unsigned Opcode) {
	switch (Opcode) {
	case BLCMSK32rm:
	case BLCMSK32rr:
	case BLCMSK64rm:
	case BLCMSK64rr:
		return true;
	}
	return false;
}

bool isVPMOVUSQW(unsigned Opcode) {
	switch (Opcode) {
	case VPMOVUSQWZ128mr:
	case VPMOVUSQWZ128mrk:
	case VPMOVUSQWZ128rr:
	case VPMOVUSQWZ128rrk:
	case VPMOVUSQWZ128rrkz:
	case VPMOVUSQWZ256mr:
	case VPMOVUSQWZ256mrk:
	case VPMOVUSQWZ256rr:
	case VPMOVUSQWZ256rrk:
	case VPMOVUSQWZ256rrkz:
	case VPMOVUSQWZmr:
	case VPMOVUSQWZmrk:
	case VPMOVUSQWZrr:
	case VPMOVUSQWZrrk:
	case VPMOVUSQWZrrkz:
		return true;
	}
	return false;
}

bool isVPMADCSWD(unsigned Opcode) {
	switch (Opcode) {
	case VPMADCSWDrm:
	case VPMADCSWDrr:
		return true;
	}
	return false;
}

bool isVGATHERDPD(unsigned Opcode) {
	switch (Opcode) {
	case VGATHERDPDYrm:
	case VGATHERDPDZ128rm:
	case VGATHERDPDZ256rm:
	case VGATHERDPDZrm:
	case VGATHERDPDrm:
		return true;
	}
	return false;
}

bool isSHLD(unsigned Opcode) {
	switch (Opcode) {
	case SHLD16mrCL:
	case SHLD16mri8:
	case SHLD16rrCL:
	case SHLD16rri8:
	case SHLD32mrCL:
	case SHLD32mri8:
	case SHLD32rrCL:
	case SHLD32rri8:
	case SHLD64mrCL:
	case SHLD64mri8:
	case SHLD64rrCL:
	case SHLD64rri8:
		return true;
	}
	return false;
}

bool isPMINUB(unsigned Opcode) {
	switch (Opcode) {
	case MMX_PMINUBrm:
	case MMX_PMINUBrr:
	case PMINUBrm:
	case PMINUBrr:
		return true;
	}
	return false;
}

bool isPMINUD(unsigned Opcode) {
	switch (Opcode) {
	case PMINUDrm:
	case PMINUDrr:
		return true;
	}
	return false;
}

bool isAESIMC(unsigned Opcode) {
	switch (Opcode) {
	case AESIMCrm:
	case AESIMCrr:
		return true;
	}
	return false;
}

bool isCVTSD2SI(unsigned Opcode) {
	switch (Opcode) {
	case CVTSD2SI64rm_Int:
	case CVTSD2SI64rr_Int:
	case CVTSD2SIrm_Int:
	case CVTSD2SIrr_Int:
		return true;
	}
	return false;
}

bool isVLDMXCSR(unsigned Opcode) {
	return Opcode == VLDMXCSR;
}

bool isVCVTSS2SD(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSS2SDZrm_Int:
	case VCVTSS2SDZrm_Intk:
	case VCVTSS2SDZrm_Intkz:
	case VCVTSS2SDZrr_Int:
	case VCVTSS2SDZrr_Intk:
	case VCVTSS2SDZrr_Intkz:
	case VCVTSS2SDZrrb_Int:
	case VCVTSS2SDZrrb_Intk:
	case VCVTSS2SDZrrb_Intkz:
	case VCVTSS2SDrm_Int:
	case VCVTSS2SDrr_Int:
		return true;
	}
	return false;
}

bool isVCVTSS2SH(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSS2SHZrm_Int:
	case VCVTSS2SHZrm_Intk:
	case VCVTSS2SHZrm_Intkz:
	case VCVTSS2SHZrr_Int:
	case VCVTSS2SHZrr_Intk:
	case VCVTSS2SHZrr_Intkz:
	case VCVTSS2SHZrrb_Int:
	case VCVTSS2SHZrrb_Intk:
	case VCVTSS2SHZrrb_Intkz:
		return true;
	}
	return false;
}

bool isSLDT(unsigned Opcode) {
	switch (Opcode) {
	case SLDT16m:
	case SLDT16r:
	case SLDT32r:
	case SLDT64r:
		return true;
	}
	return false;
}

bool isVCVTSS2SI(unsigned Opcode) {
	switch (Opcode) {
	case VCVTSS2SI64Zrm_Int:
	case VCVTSS2SI64Zrr_Int:
	case VCVTSS2SI64Zrrb_Int:
	case VCVTSS2SI64rm_Int:
	case VCVTSS2SI64rr_Int:
	case VCVTSS2SIZrm_Int:
	case VCVTSS2SIZrr_Int:
	case VCVTSS2SIZrrb_Int:
	case VCVTSS2SIrm_Int:
	case VCVTSS2SIrr_Int:
		return true;
	}
	return false;
}

bool isVGATHERDPS(unsigned Opcode) {
	switch (Opcode) {
	case VGATHERDPSYrm:
	case VGATHERDPSZ128rm:
	case VGATHERDPSZ256rm:
	case VGATHERDPSZrm:
	case VGATHERDPSrm:
		return true;
	}
	return false;
}

bool isFABS(unsigned Opcode) {
	return Opcode == ABS_F;
}

bool isCVTSD2SS(unsigned Opcode) {
	switch (Opcode) {
	case CVTSD2SSrm_Int:
	case CVTSD2SSrr_Int:
		return true;
	}
	return false;
}

bool isSHLX(unsigned Opcode) {
	switch (Opcode) {
	case SHLX32rm:
	case SHLX32rr:
	case SHLX64rm:
	case SHLX64rr:
		return true;
	}
	return false;
}

bool isMONITORX(unsigned Opcode) {
	switch (Opcode) {
	case MONITORX32rrr:
	case MONITORX64rrr:
		return true;
	}
	return false;
}

bool isPMINUW(unsigned Opcode) {
	switch (Opcode) {
	case PMINUWrm:
	case PMINUWrr:
		return true;
	}
	return false;
}

bool isVPMAXSB(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXSBYrm:
	case VPMAXSBYrr:
	case VPMAXSBZ128rm:
	case VPMAXSBZ128rmk:
	case VPMAXSBZ128rmkz:
	case VPMAXSBZ128rr:
	case VPMAXSBZ128rrk:
	case VPMAXSBZ128rrkz:
	case VPMAXSBZ256rm:
	case VPMAXSBZ256rmk:
	case VPMAXSBZ256rmkz:
	case VPMAXSBZ256rr:
	case VPMAXSBZ256rrk:
	case VPMAXSBZ256rrkz:
	case VPMAXSBZrm:
	case VPMAXSBZrmk:
	case VPMAXSBZrmkz:
	case VPMAXSBZrr:
	case VPMAXSBZrrk:
	case VPMAXSBZrrkz:
	case VPMAXSBrm:
	case VPMAXSBrr:
		return true;
	}
	return false;
}

bool isVPMAXSD(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXSDYrm:
	case VPMAXSDYrr:
	case VPMAXSDZ128rm:
	case VPMAXSDZ128rmb:
	case VPMAXSDZ128rmbk:
	case VPMAXSDZ128rmbkz:
	case VPMAXSDZ128rmk:
	case VPMAXSDZ128rmkz:
	case VPMAXSDZ128rr:
	case VPMAXSDZ128rrk:
	case VPMAXSDZ128rrkz:
	case VPMAXSDZ256rm:
	case VPMAXSDZ256rmb:
	case VPMAXSDZ256rmbk:
	case VPMAXSDZ256rmbkz:
	case VPMAXSDZ256rmk:
	case VPMAXSDZ256rmkz:
	case VPMAXSDZ256rr:
	case VPMAXSDZ256rrk:
	case VPMAXSDZ256rrkz:
	case VPMAXSDZrm:
	case VPMAXSDZrmb:
	case VPMAXSDZrmbk:
	case VPMAXSDZrmbkz:
	case VPMAXSDZrmk:
	case VPMAXSDZrmkz:
	case VPMAXSDZrr:
	case VPMAXSDZrrk:
	case VPMAXSDZrrkz:
	case VPMAXSDrm:
	case VPMAXSDrr:
		return true;
	}
	return false;
}

bool isMOVAPD(unsigned Opcode) {
	switch (Opcode) {
	case MOVAPDmr:
	case MOVAPDrm:
	case MOVAPDrr:
	case MOVAPDrr_REV:
		return true;
	}
	return false;
}

bool isENQCMDS(unsigned Opcode) {
	switch (Opcode) {
	case ENQCMDS16:
	case ENQCMDS32:
	case ENQCMDS64:
		return true;
	}
	return false;
}

bool isVMOVD(unsigned Opcode) {
	switch (Opcode) {
	case VMOVDI2PDIZrm:
	case VMOVDI2PDIZrr:
	case VMOVDI2PDIrm:
	case VMOVDI2PDIrr:
	case VMOVPDI2DIZmr:
	case VMOVPDI2DIZrr:
	case VMOVPDI2DImr:
	case VMOVPDI2DIrr:
		return true;
	}
	return false;
}

bool isVPMAXSQ(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXSQZ128rm:
	case VPMAXSQZ128rmb:
	case VPMAXSQZ128rmbk:
	case VPMAXSQZ128rmbkz:
	case VPMAXSQZ128rmk:
	case VPMAXSQZ128rmkz:
	case VPMAXSQZ128rr:
	case VPMAXSQZ128rrk:
	case VPMAXSQZ128rrkz:
	case VPMAXSQZ256rm:
	case VPMAXSQZ256rmb:
	case VPMAXSQZ256rmbk:
	case VPMAXSQZ256rmbkz:
	case VPMAXSQZ256rmk:
	case VPMAXSQZ256rmkz:
	case VPMAXSQZ256rr:
	case VPMAXSQZ256rrk:
	case VPMAXSQZ256rrkz:
	case VPMAXSQZrm:
	case VPMAXSQZrmb:
	case VPMAXSQZrmbk:
	case VPMAXSQZrmbkz:
	case VPMAXSQZrmk:
	case VPMAXSQZrmkz:
	case VPMAXSQZrr:
	case VPMAXSQZrrk:
	case VPMAXSQZrrkz:
		return true;
	}
	return false;
}

bool isCVTTSS2SI(unsigned Opcode) {
	switch (Opcode) {
	case CVTTSS2SI64rm_Int:
	case CVTTSS2SI64rr_Int:
	case CVTTSS2SIrm_Int:
	case CVTTSS2SIrr_Int:
		return true;
	}
	return false;
}

bool isVPMAXSW(unsigned Opcode) {
	switch (Opcode) {
	case VPMAXSWYrm:
	case VPMAXSWYrr:
	case VPMAXSWZ128rm:
	case VPMAXSWZ128rmk:
	case VPMAXSWZ128rmkz:
	case VPMAXSWZ128rr:
	case VPMAXSWZ128rrk:
	case VPMAXSWZ128rrkz:
	case VPMAXSWZ256rm:
	case VPMAXSWZ256rmk:
	case VPMAXSWZ256rmkz:
	case VPMAXSWZ256rr:
	case VPMAXSWZ256rrk:
	case VPMAXSWZ256rrkz:
	case VPMAXSWZrm:
	case VPMAXSWZrmk:
	case VPMAXSWZrmkz:
	case VPMAXSWZrr:
	case VPMAXSWZrrk:
	case VPMAXSWZrrkz:
	case VPMAXSWrm:
	case VPMAXSWrr:
		return true;
	}
	return false;
}

bool isMOVAPS(unsigned Opcode) {
	switch (Opcode) {
	case MOVAPSmr:
	case MOVAPSrm:
	case MOVAPSrr:
	case MOVAPSrr_REV:
		return true;
	}
	return false;
}

bool isVPACKUSDW(unsigned Opcode) {
	switch (Opcode) {
	case VPACKUSDWYrm:
	case VPACKUSDWYrr:
	case VPACKUSDWZ128rm:
	case VPACKUSDWZ128rmb:
	case VPACKUSDWZ128rmbk:
	case VPACKUSDWZ128rmbkz:
	case VPACKUSDWZ128rmk:
	case VPACKUSDWZ128rmkz:
	case VPACKUSDWZ128rr:
	case VPACKUSDWZ128rrk:
	case VPACKUSDWZ128rrkz:
	case VPACKUSDWZ256rm:
	case VPACKUSDWZ256rmb:
	case VPACKUSDWZ256rmbk:
	case VPACKUSDWZ256rmbkz:
	case VPACKUSDWZ256rmk:
	case VPACKUSDWZ256rmkz:
	case VPACKUSDWZ256rr:
	case VPACKUSDWZ256rrk:
	case VPACKUSDWZ256rrkz:
	case VPACKUSDWZrm:
	case VPACKUSDWZrmb:
	case VPACKUSDWZrmbk:
	case VPACKUSDWZrmbkz:
	case VPACKUSDWZrmk:
	case VPACKUSDWZrmkz:
	case VPACKUSDWZrr:
	case VPACKUSDWZrrk:
	case VPACKUSDWZrrkz:
	case VPACKUSDWrm:
	case VPACKUSDWrr:
		return true;
	}
	return false;
}

bool isVFIXUPIMMPD(unsigned Opcode) {
	switch (Opcode) {
	case VFIXUPIMMPDZ128rmbi:
	case VFIXUPIMMPDZ128rmbik:
	case VFIXUPIMMPDZ128rmbikz:
	case VFIXUPIMMPDZ128rmi:
	case VFIXUPIMMPDZ128rmik:
	case VFIXUPIMMPDZ128rmikz:
	case VFIXUPIMMPDZ128rri:
	case VFIXUPIMMPDZ128rrik:
	case VFIXUPIMMPDZ128rrikz:
	case VFIXUPIMMPDZ256rmbi:
	case VFIXUPIMMPDZ256rmbik:
	case VFIXUPIMMPDZ256rmbikz:
	case VFIXUPIMMPDZ256rmi:
	case VFIXUPIMMPDZ256rmik:
	case VFIXUPIMMPDZ256rmikz:
	case VFIXUPIMMPDZ256rri:
	case VFIXUPIMMPDZ256rrik:
	case VFIXUPIMMPDZ256rrikz:
	case VFIXUPIMMPDZrmbi:
	case VFIXUPIMMPDZrmbik:
	case VFIXUPIMMPDZrmbikz:
	case VFIXUPIMMPDZrmi:
	case VFIXUPIMMPDZrmik:
	case VFIXUPIMMPDZrmikz:
	case VFIXUPIMMPDZrri:
	case VFIXUPIMMPDZrrib:
	case VFIXUPIMMPDZrribk:
	case VFIXUPIMMPDZrribkz:
	case VFIXUPIMMPDZrrik:
	case VFIXUPIMMPDZrrikz:
		return true;
	}
	return false;
}

bool isVMOVQ(unsigned Opcode) {
	switch (Opcode) {
	case VMOV64toPQIZrm:
	case VMOV64toPQIZrr:
	case VMOV64toPQIrm:
	case VMOV64toPQIrr:
	case VMOVPQI2QIZmr:
	case VMOVPQI2QIZrr:
	case VMOVPQI2QImr:
	case VMOVPQI2QIrr:
	case VMOVPQIto64Zmr:
	case VMOVPQIto64Zrr:
	case VMOVPQIto64mr:
	case VMOVPQIto64rr:
	case VMOVQI2PQIZrm:
	case VMOVQI2PQIrm:
	case VMOVZPQILo2PQIZrr:
	case VMOVZPQILo2PQIrr:
		return true;
	}
	return false;
}

bool isVPSHUFHW(unsigned Opcode) {
	switch (Opcode) {
	case VPSHUFHWYmi:
	case VPSHUFHWYri:
	case VPSHUFHWZ128mi:
	case VPSHUFHWZ128mik:
	case VPSHUFHWZ128mikz:
	case VPSHUFHWZ128ri:
	case VPSHUFHWZ128rik:
	case VPSHUFHWZ128rikz:
	case VPSHUFHWZ256mi:
	case VPSHUFHWZ256mik:
	case VPSHUFHWZ256mikz:
	case VPSHUFHWZ256ri:
	case VPSHUFHWZ256rik:
	case VPSHUFHWZ256rikz:
	case VPSHUFHWZmi:
	case VPSHUFHWZmik:
	case VPSHUFHWZmikz:
	case VPSHUFHWZri:
	case VPSHUFHWZrik:
	case VPSHUFHWZrikz:
	case VPSHUFHWmi:
	case VPSHUFHWri:
		return true;
	}
	return false;
}

bool isPCMPISTRI(unsigned Opcode) {
	switch (Opcode) {
	case PCMPISTRIrm:
	case PCMPISTRIrr:
		return true;
	}
	return false;
}

bool isVMOVW(unsigned Opcode) {
	switch (Opcode) {
	case VMOVSH2Wrr:
	case VMOVSHtoW64rr:
	case VMOVW2SHrr:
	case VMOVW64toSHrr:
	case VMOVWmr:
	case VMOVWrm:
		return true;
	}
	return false;
}

#endif // GET_X86_MNEMONIC_TABLES_CPP

} // end namespace X86
} // end namespace llvm