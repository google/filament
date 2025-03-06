//===--- TargetInfo.cpp - Information about Target machine ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the TargetInfo and TargetInfoImpl interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/LangOptions.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstdlib>
using namespace clang;

static const LangAS::Map DefaultAddrSpaceMap = { 0 };

// TargetInfo Constructor.
TargetInfo::TargetInfo(const llvm::Triple &T) : TargetOpts(), Triple(T) {
  // Set defaults.  Defaults are set for a 32-bit RISC platform, like PPC or
  // SPARC.  These should be overridden by concrete targets as needed.
  BigEndian = true;
  TLSSupported = true;
  NoAsmVariants = false;
  PointerWidth = PointerAlign = 32;
  BoolWidth = BoolAlign = 8;
  IntWidth = IntAlign = 32;
  LongWidth = LongAlign = 32;
  LongLongWidth = LongLongAlign = 64;
  SuitableAlign = 64;
  DefaultAlignForAttributeAligned = 128;
  MinGlobalAlign = 0;
  HalfWidth = 16;
  HalfAlign = 16;
  FloatWidth = 32;
  FloatAlign = 32;
  DoubleWidth = 64;
  DoubleAlign = 64;
  LongDoubleWidth = 64;
  LongDoubleAlign = 64;
  LargeArrayMinWidth = 0;
  LargeArrayAlign = 0;
  MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 0;
  MaxVectorAlign = 0;
  MaxTLSAlign = 0;
  SimdDefaultAlign = 0;
  SizeType = UnsignedLong;
  PtrDiffType = SignedLong;
  IntMaxType = SignedLongLong;
  IntPtrType = SignedLong;
  WCharType = SignedInt;
  WIntType = SignedInt;
  Char16Type = UnsignedShort;
  Char32Type = UnsignedInt;
  Int64Type = SignedLongLong;
  SigAtomicType = SignedInt;
  ProcessIDType = SignedInt;
  UseSignedCharForObjCBool = true;
  UseBitFieldTypeAlignment = true;
  UseZeroLengthBitfieldAlignment = false;
  ZeroLengthBitfieldBoundary = 0;
  HalfFormat = &llvm::APFloat::IEEEhalf;
  FloatFormat = &llvm::APFloat::IEEEsingle;
  DoubleFormat = &llvm::APFloat::IEEEdouble;
  LongDoubleFormat = &llvm::APFloat::IEEEdouble;
  DescriptionString = nullptr;
  UserLabelPrefix = "_";
  MCountName = "mcount";
  RegParmMax = 0;
  SSERegParmMax = 0;
  HasAlignMac68kSupport = false;

  // Default to no types using fpret.
  RealTypeUsesObjCFPRet = 0;

  // Default to not using fp2ret for __Complex long double
  ComplexLongDoubleUsesFP2Ret = false;

  // Set the C++ ABI based on the triple.
  TheCXXABI.set(Triple.isKnownWindowsMSVCEnvironment()
                    ? TargetCXXABI::Microsoft
                    : TargetCXXABI::GenericItanium);

  // Default to an empty address space map.
  AddrSpaceMap = &DefaultAddrSpaceMap;
  UseAddrSpaceMapMangling = false;

  // Default to an unknown platform name.
  PlatformName = "unknown";
  PlatformMinVersion = VersionTuple();
}

// Out of line virtual dtor for TargetInfo.
TargetInfo::~TargetInfo() {}

/// getTypeName - Return the user string for the specified integer type enum.
/// For example, SignedShort -> "short".
const char *TargetInfo::getTypeName(IntType T) {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:       return "signed char";
  case UnsignedChar:     return "unsigned char";
  case SignedShort:      return "short";
  case UnsignedShort:    return "unsigned short";
  case SignedInt:        return "int";
  case UnsignedInt:      return "unsigned int";
  case SignedLong:       return "long int";
  case UnsignedLong:     return "long unsigned int";
  case SignedLongLong:   return "long long int";
  case UnsignedLongLong: return "long long unsigned int";
  }
}

/// getTypeConstantSuffix - Return the constant suffix for the specified
/// integer type enum. For example, SignedLong -> "L".
const char *TargetInfo::getTypeConstantSuffix(IntType T) const {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case SignedShort:
  case SignedInt:        return "";
  case SignedLong:       return "L";
  case SignedLongLong:   return "LL";
  case UnsignedChar:
    if (getCharWidth() < getIntWidth())
      return "";
    LLVM_FALLTHROUGH; // HLSL Change
  case UnsignedShort:
    if (getShortWidth() < getIntWidth())
      return "";
    LLVM_FALLTHROUGH; // HLSL Change
  case UnsignedInt:      return "U";
  case UnsignedLong:     return "UL";
  case UnsignedLongLong: return "ULL";
  }
}

/// getTypeFormatModifier - Return the printf format modifier for the
/// specified integer type enum. For example, SignedLong -> "l".

const char *TargetInfo::getTypeFormatModifier(IntType T) {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case UnsignedChar:     return "hh";
  case SignedShort:
  case UnsignedShort:    return "h";
  case SignedInt:
  case UnsignedInt:      return "";
  case SignedLong:
  case UnsignedLong:     return "l";
  case SignedLongLong:
  case UnsignedLongLong: return "ll";
  }
}

/// getTypeWidth - Return the width (in bits) of the specified integer type
/// enum. For example, SignedInt -> getIntWidth().
unsigned TargetInfo::getTypeWidth(IntType T) const {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case UnsignedChar:     return getCharWidth();
  case SignedShort:
  case UnsignedShort:    return getShortWidth();
  case SignedInt:
  case UnsignedInt:      return getIntWidth();
  case SignedLong:
  case UnsignedLong:     return getLongWidth();
  case SignedLongLong:
  case UnsignedLongLong: return getLongLongWidth();
  };
}

TargetInfo::IntType TargetInfo::getIntTypeByWidth(
    unsigned BitWidth, bool IsSigned) const {
  if (getCharWidth() == BitWidth)
    return IsSigned ? SignedChar : UnsignedChar;
  if (getShortWidth() == BitWidth)
    return IsSigned ? SignedShort : UnsignedShort;
  if (getIntWidth() == BitWidth)
    return IsSigned ? SignedInt : UnsignedInt;
  if (getLongWidth() == BitWidth)
    return IsSigned ? SignedLong : UnsignedLong;
  if (getLongLongWidth() == BitWidth)
    return IsSigned ? SignedLongLong : UnsignedLongLong;
  return NoInt;
}

TargetInfo::IntType TargetInfo::getLeastIntTypeByWidth(unsigned BitWidth,
                                                       bool IsSigned) const {
  if (getCharWidth() >= BitWidth)
    return IsSigned ? SignedChar : UnsignedChar;
  if (getShortWidth() >= BitWidth)
    return IsSigned ? SignedShort : UnsignedShort;
  if (getIntWidth() >= BitWidth)
    return IsSigned ? SignedInt : UnsignedInt;
  if (getLongWidth() >= BitWidth)
    return IsSigned ? SignedLong : UnsignedLong;
  if (getLongLongWidth() >= BitWidth)
    return IsSigned ? SignedLongLong : UnsignedLongLong;
  return NoInt;
}

TargetInfo::RealType TargetInfo::getRealTypeByWidth(unsigned BitWidth) const {
  if (getFloatWidth() == BitWidth)
    return Float;
  if (getDoubleWidth() == BitWidth)
    return Double;

  switch (BitWidth) {
  case 96:
    if (&getLongDoubleFormat() == &llvm::APFloat::x87DoubleExtended)
      return LongDouble;
    break;
  case 128:
    if (&getLongDoubleFormat() == &llvm::APFloat::PPCDoubleDouble ||
        &getLongDoubleFormat() == &llvm::APFloat::IEEEquad)
      return LongDouble;
    break;
  }

  return NoFloat;
}

/// getTypeAlign - Return the alignment (in bits) of the specified integer type
/// enum. For example, SignedInt -> getIntAlign().
unsigned TargetInfo::getTypeAlign(IntType T) const {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case UnsignedChar:     return getCharAlign();
  case SignedShort:
  case UnsignedShort:    return getShortAlign();
  case SignedInt:
  case UnsignedInt:      return getIntAlign();
  case SignedLong:
  case UnsignedLong:     return getLongAlign();
  case SignedLongLong:
  case UnsignedLongLong: return getLongLongAlign();
  };
}

/// isTypeSigned - Return whether an integer types is signed. Returns true if
/// the type is signed; false otherwise.
bool TargetInfo::isTypeSigned(IntType T) {
  switch (T) {
  default: llvm_unreachable("not an integer!");
  case SignedChar:
  case SignedShort:
  case SignedInt:
  case SignedLong:
  case SignedLongLong:
    return true;
  case UnsignedChar:
  case UnsignedShort:
  case UnsignedInt:
  case UnsignedLong:
  case UnsignedLongLong:
    return false;
  };
}

/// adjust - Set forced language options.
/// Apply changes to the target information with respect to certain
/// language options which change the target configuration.
void TargetInfo::adjust(const LangOptions &Opts) {
  if (Opts.NoBitFieldTypeAlign)
    UseBitFieldTypeAlignment = false;
  if (Opts.ShortWChar)
    WCharType = UnsignedShort;

  // HLSL Change Begin
  // We should have always set these values for HLSL, but instead we hacked
  // ASTContext::getTypeInfoImpl to override some of the uses (but not all).
  // With 202x relying more on Clang's C/C++ support we need to correctly set
  // the TargetInfo at least for this subset of types. This change probably
  // shouldn't be dependent on the language version, but I made it this way to
  // reduce any unforseen risk.
  if (Opts.HLSL) {
    // DXILTargetInfo in Tergets.cpp sets base values that aren't dependent on
    // language options. The values here adjust the base values once the
    // language options are initialized.
    if (Opts.HLSLVersion >= hlsl::LangStd::v202x) {
      IntWidth = IntAlign = 32;
      LongLongWidth = LongLongAlign = 64;
      if (!Opts.UseMinPrecision)
        HalfWidth = HalfAlign = 16;
      else
        HalfWidth = HalfAlign = 32;
      FloatWidth = FloatAlign = 32;
    }
  }
  // HLSL Change End

  if (Opts.OpenCL) {
    // OpenCL C requires specific widths for types, irrespective of
    // what these normally are for the target.
    // We also define long long and long double here, although the
    // OpenCL standard only mentions these as "reserved".
    IntWidth = IntAlign = 32;
    LongWidth = LongAlign = 64;
    LongLongWidth = LongLongAlign = 128;
    HalfWidth = HalfAlign = 16;
    FloatWidth = FloatAlign = 32;
    
    // Embedded 32-bit targets (OpenCL EP) might have double C type 
    // defined as float. Let's not override this as it might lead 
    // to generating illegal code that uses 64bit doubles.
    if (DoubleWidth != FloatWidth) {
      DoubleWidth = DoubleAlign = 64;
      DoubleFormat = &llvm::APFloat::IEEEdouble;
    }
    LongDoubleWidth = LongDoubleAlign = 128;

    assert(PointerWidth == 32 || PointerWidth == 64);
    bool Is32BitArch = PointerWidth == 32;
    SizeType = Is32BitArch ? UnsignedInt : UnsignedLong;
    PtrDiffType = Is32BitArch ? SignedInt : SignedLong;
    IntPtrType = Is32BitArch ? SignedInt : SignedLong;

    IntMaxType = SignedLongLong;
    Int64Type = SignedLong;

    HalfFormat = &llvm::APFloat::IEEEhalf;
    FloatFormat = &llvm::APFloat::IEEEsingle;
    LongDoubleFormat = &llvm::APFloat::IEEEquad;
  }
}

//===----------------------------------------------------------------------===//


static StringRef removeGCCRegisterPrefix(StringRef Name) {
  if (Name[0] == '%' || Name[0] == '#')
    Name = Name.substr(1);

  return Name;
}

/// isValidClobber - Returns whether the passed in string is
/// a valid clobber in an inline asm statement. This is used by
/// Sema.
bool TargetInfo::isValidClobber(StringRef Name) const {
  return (isValidGCCRegisterName(Name) ||
	  Name == "memory" || Name == "cc");
}

/// isValidGCCRegisterName - Returns whether the passed in string
/// is a valid register name according to GCC. This is used by Sema for
/// inline asm statements.
bool TargetInfo::isValidGCCRegisterName(StringRef Name) const {
  if (Name.empty())
    return false;

  const char * const *Names;
  unsigned NumNames;

  // Get rid of any register prefix.
  Name = removeGCCRegisterPrefix(Name);
  if (Name.empty())
      return false;

  getGCCRegNames(Names, NumNames);

  // If we have a number it maps to an entry in the register name array.
  if (isDigit(Name[0])) {
    int n;
    if (!Name.getAsInteger(0, n))
      return n >= 0 && (unsigned)n < NumNames;
  }

  // Check register names.
  for (unsigned i = 0; i < NumNames; i++) {
    if (Name == Names[i])
      return true;
  }

  // Check any additional names that we have.
  const AddlRegName *AddlNames;
  unsigned NumAddlNames;
  getGCCAddlRegNames(AddlNames, NumAddlNames);
  for (unsigned i = 0; i < NumAddlNames; i++)
    for (unsigned j = 0; j < llvm::array_lengthof(AddlNames[i].Names); j++) {
      if (!AddlNames[i].Names[j])
	break;
      // Make sure the register that the additional name is for is within
      // the bounds of the register names from above.
      if (AddlNames[i].Names[j] == Name && AddlNames[i].RegNum < NumNames)
	return true;
  }

  // Now check aliases.
  const GCCRegAlias *Aliases;
  unsigned NumAliases;

  getGCCRegAliases(Aliases, NumAliases);
  for (unsigned i = 0; i < NumAliases; i++) {
    for (unsigned j = 0 ; j < llvm::array_lengthof(Aliases[i].Aliases); j++) {
      if (!Aliases[i].Aliases[j])
        break;
      if (Aliases[i].Aliases[j] == Name)
        return true;
    }
  }

  return false;
}

StringRef
TargetInfo::getNormalizedGCCRegisterName(StringRef Name) const {
  assert(isValidGCCRegisterName(Name) && "Invalid register passed in");

  // Get rid of any register prefix.
  Name = removeGCCRegisterPrefix(Name);

  const char * const *Names;
  unsigned NumNames;

  getGCCRegNames(Names, NumNames);

  // First, check if we have a number.
  if (isDigit(Name[0])) {
    int n;
    if (!Name.getAsInteger(0, n)) {
      assert(n >= 0 && (unsigned)n < NumNames &&
             "Out of bounds register number!");
      return Names[n];
    }
  }

  // Check any additional names that we have.
  const AddlRegName *AddlNames;
  unsigned NumAddlNames;
  getGCCAddlRegNames(AddlNames, NumAddlNames);
  for (unsigned i = 0; i < NumAddlNames; i++)
    for (unsigned j = 0; j < llvm::array_lengthof(AddlNames[i].Names); j++) {
      if (!AddlNames[i].Names[j])
	break;
      // Make sure the register that the additional name is for is within
      // the bounds of the register names from above.
      if (AddlNames[i].Names[j] == Name && AddlNames[i].RegNum < NumNames)
	return Name;
    }

  // Now check aliases.
  const GCCRegAlias *Aliases;
  unsigned NumAliases;

  getGCCRegAliases(Aliases, NumAliases);
  for (unsigned i = 0; i < NumAliases; i++) {
    for (unsigned j = 0 ; j < llvm::array_lengthof(Aliases[i].Aliases); j++) {
      if (!Aliases[i].Aliases[j])
        break;
      if (Aliases[i].Aliases[j] == Name)
        return Aliases[i].Register;
    }
  }

  return Name;
}

bool TargetInfo::validateOutputConstraint(ConstraintInfo &Info) const {
  const char *Name = Info.getConstraintStr().c_str();
  // An output constraint must start with '=' or '+'
  if (*Name != '=' && *Name != '+')
    return false;

  if (*Name == '+')
    Info.setIsReadWrite();

  Name++;
  while (*Name) {
    switch (*Name) {
    default:
      if (!validateAsmConstraint(Name, Info)) {
        // FIXME: We temporarily return false
        // so we can add more constraints as we hit it.
        // Eventually, an unknown constraint should just be treated as 'g'.
        return false;
      }
      break;
    case '&': // early clobber.
      Info.setEarlyClobber();
      break;
    case '%': // commutative.
      // FIXME: Check that there is a another register after this one.
      break;
    case 'r': // general register.
      Info.setAllowsRegister();
      break;
    case 'm': // memory operand.
    case 'o': // offsetable memory operand.
    case 'V': // non-offsetable memory operand.
    case '<': // autodecrement memory operand.
    case '>': // autoincrement memory operand.
      Info.setAllowsMemory();
      break;
    case 'g': // general register, memory operand or immediate integer.
    case 'X': // any operand.
      Info.setAllowsRegister();
      Info.setAllowsMemory();
      break;
    case ',': // multiple alternative constraint.  Pass it.
      // Handle additional optional '=' or '+' modifiers.
      if (Name[1] == '=' || Name[1] == '+')
        Name++;
      break;
    case '#': // Ignore as constraint.
      while (Name[1] && Name[1] != ',')
        Name++;
      break;
    case '?': // Disparage slightly code.
    case '!': // Disparage severely.
    case '*': // Ignore for choosing register preferences.
      break;  // Pass them.
    }

    Name++;
  }

  // Early clobber with a read-write constraint which doesn't permit registers
  // is invalid.
  if (Info.earlyClobber() && Info.isReadWrite() && !Info.allowsRegister())
    return false;

  // If a constraint allows neither memory nor register operands it contains
  // only modifiers. Reject it.
  return Info.allowsMemory() || Info.allowsRegister();
}

bool TargetInfo::resolveSymbolicName(const char *&Name,
                                     ConstraintInfo *OutputConstraints,
                                     unsigned NumOutputs,
                                     unsigned &Index) const {
  assert(*Name == '[' && "Symbolic name did not start with '['");
  Name++;
  const char *Start = Name;
  while (*Name && *Name != ']')
    Name++;

  if (!*Name) {
    // Missing ']'
    return false;
  }

  std::string SymbolicName(Start, Name - Start);

  for (Index = 0; Index != NumOutputs; ++Index)
    if (SymbolicName == OutputConstraints[Index].getName())
      return true;

  return false;
}

bool TargetInfo::validateInputConstraint(ConstraintInfo *OutputConstraints,
                                         unsigned NumOutputs,
                                         ConstraintInfo &Info) const {
  const char *Name = Info.ConstraintStr.c_str();

  if (!*Name)
    return false;

  while (*Name) {
    switch (*Name) {
    default:
      // Check if we have a matching constraint
      if (*Name >= '0' && *Name <= '9') {
        const char *DigitStart = Name;
        while (Name[1] >= '0' && Name[1] <= '9')
          Name++;
        const char *DigitEnd = Name;
        unsigned i;
        if (StringRef(DigitStart, DigitEnd - DigitStart + 1)
                .getAsInteger(10, i))
          return false;

        // Check if matching constraint is out of bounds.
        if (i >= NumOutputs) return false;

        // A number must refer to an output only operand.
        if (OutputConstraints[i].isReadWrite())
          return false;

        // If the constraint is already tied, it must be tied to the 
        // same operand referenced to by the number.
        if (Info.hasTiedOperand() && Info.getTiedOperand() != i)
          return false;

        // The constraint should have the same info as the respective
        // output constraint.
        Info.setTiedOperand(i, OutputConstraints[i]);
      } else if (!validateAsmConstraint(Name, Info)) {
        // FIXME: This error return is in place temporarily so we can
        // add more constraints as we hit it.  Eventually, an unknown
        // constraint should just be treated as 'g'.
        return false;
      }
      break;
    case '[': {
      unsigned Index = 0;
      if (!resolveSymbolicName(Name, OutputConstraints, NumOutputs, Index))
        return false;

      // If the constraint is already tied, it must be tied to the 
      // same operand referenced to by the number.
      if (Info.hasTiedOperand() && Info.getTiedOperand() != Index)
        return false;

      // A number must refer to an output only operand.
      if (OutputConstraints[Index].isReadWrite())
        return false;

      Info.setTiedOperand(Index, OutputConstraints[Index]);
      break;
    }
    case '%': // commutative
      // FIXME: Fail if % is used with the last operand.
      break;
    case 'i': // immediate integer.
    case 'n': // immediate integer with a known value.
      break;
    case 'I':  // Various constant constraints with target-specific meanings.
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
      if (!validateAsmConstraint(Name, Info))
        return false;
      break;
    case 'r': // general register.
      Info.setAllowsRegister();
      break;
    case 'm': // memory operand.
    case 'o': // offsettable memory operand.
    case 'V': // non-offsettable memory operand.
    case '<': // autodecrement memory operand.
    case '>': // autoincrement memory operand.
      Info.setAllowsMemory();
      break;
    case 'g': // general register, memory operand or immediate integer.
    case 'X': // any operand.
      Info.setAllowsRegister();
      Info.setAllowsMemory();
      break;
    case 'E': // immediate floating point.
    case 'F': // immediate floating point.
    case 'p': // address operand.
      break;
    case ',': // multiple alternative constraint.  Ignore comma.
      break;
    case '#': // Ignore as constraint.
      while (Name[1] && Name[1] != ',')
        Name++;
      break;
    case '?': // Disparage slightly code.
    case '!': // Disparage severely.
    case '*': // Ignore for choosing register preferences.
      break;  // Pass them.
    }

    Name++;
  }

  return true;
}

bool TargetCXXABI::tryParse(llvm::StringRef name) {
  const Kind unknown = static_cast<Kind>(-1);
  Kind kind = llvm::StringSwitch<Kind>(name)
    .Case("arm", GenericARM)
    .Case("ios", iOS)
    .Case("itanium", GenericItanium)
    .Case("microsoft", Microsoft)
    .Case("mips", GenericMIPS)
    .Default(unknown);
  if (kind == unknown) return false;

  set(kind);
  return true;
}
