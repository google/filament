//===- subzero/src/PNaClTranslator.cpp - ICE from bitcode -----------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the interface for translation from PNaCl bitcode files to
/// ICE to machine code.
///
//===----------------------------------------------------------------------===//

#include "PNaClTranslator.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceGlobalInits.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRangeSpec.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Bitcode/NaCl/NaClBitcodeDecoders.h"
#include "llvm/Bitcode/NaCl/NaClBitcodeDefs.h"
#include "llvm/Bitcode/NaCl/NaClBitcodeHeader.h"
#include "llvm/Bitcode/NaCl/NaClBitcodeParser.h"
#include "llvm/Bitcode/NaCl/NaClReaderWriter.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <unordered_set>

// Define a hash function for SmallString's, so that it can be used in hash
// tables.
namespace std {
template <unsigned InternalLen> struct hash<llvm::SmallString<InternalLen>> {
  size_t operator()(const llvm::SmallString<InternalLen> &Key) const {
    return llvm::hash_combine_range(Key.begin(), Key.end());
  }
};
} // end of namespace std

namespace {
using namespace llvm;

// Models elements in the list of types defined in the types block. These
// elements can be undefined, a (simple) type, or a function type signature.
// Note that an extended type is undefined on construction. Use methods
// setAsSimpleType and setAsFuncSigType to define the extended type.
class ExtendedType {
  ExtendedType &operator=(const ExtendedType &Ty) = delete;

public:
  /// Discriminator for LLVM-style RTTI.
  enum TypeKind { Undefined, Simple, FuncSig };

  ExtendedType() = default;
  ExtendedType(const ExtendedType &Ty) = default;

  virtual ~ExtendedType() = default;

  ExtendedType::TypeKind getKind() const { return Kind; }
  void dump(Ice::Ostream &Stream) const;

  /// Changes the extended type to a simple type with the given / value.
  void setAsSimpleType(Ice::Type Ty) {
    assert(Kind == Undefined);
    Kind = Simple;
    Signature.setReturnType(Ty);
  }

  /// Changes the extended type to an (empty) function signature type.
  void setAsFunctionType() {
    assert(Kind == Undefined);
    Kind = FuncSig;
  }

protected:
  // Note: For simple types, the return type of the signature will be used to
  // hold the simple type.
  Ice::FuncSigType Signature;

private:
  ExtendedType::TypeKind Kind = Undefined;
};

Ice::Ostream &operator<<(Ice::Ostream &Stream, const ExtendedType &Ty) {
  if (!Ice::BuildDefs::dump())
    return Stream;
  Ty.dump(Stream);
  return Stream;
}

Ice::Ostream &operator<<(Ice::Ostream &Stream, ExtendedType::TypeKind Kind) {
  if (!Ice::BuildDefs::dump())
    return Stream;
  Stream << "ExtendedType::";
  switch (Kind) {
  case ExtendedType::Undefined:
    Stream << "Undefined";
    break;
  case ExtendedType::Simple:
    Stream << "Simple";
    break;
  case ExtendedType::FuncSig:
    Stream << "FuncSig";
    break;
  }
  return Stream;
}

// Models an ICE type as an extended type.
class SimpleExtendedType : public ExtendedType {
  SimpleExtendedType() = delete;
  SimpleExtendedType(const SimpleExtendedType &) = delete;
  SimpleExtendedType &operator=(const SimpleExtendedType &) = delete;

public:
  Ice::Type getType() const { return Signature.getReturnType(); }

  static bool classof(const ExtendedType *Ty) {
    return Ty->getKind() == Simple;
  }
};

// Models a function signature as an extended type.
class FuncSigExtendedType : public ExtendedType {
  FuncSigExtendedType() = delete;
  FuncSigExtendedType(const FuncSigExtendedType &) = delete;
  FuncSigExtendedType &operator=(const FuncSigExtendedType &) = delete;

public:
  const Ice::FuncSigType &getSignature() const { return Signature; }
  void setReturnType(Ice::Type ReturnType) {
    Signature.setReturnType(ReturnType);
  }
  void appendArgType(Ice::Type ArgType) { Signature.appendArgType(ArgType); }
  static bool classof(const ExtendedType *Ty) {
    return Ty->getKind() == FuncSig;
  }
};

void ExtendedType::dump(Ice::Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  Stream << Kind;
  switch (Kind) {
  case Simple: {
    Stream << " " << Signature.getReturnType();
    break;
  }
  case FuncSig: {
    Stream << " " << Signature;
  }
  default:
    break;
  }
}

// Models integer literals as a sequence of bits. Used to read integer values
// from bitcode files. Based on llvm::APInt.
class BitcodeInt {
  BitcodeInt() = delete;
  BitcodeInt(const BitcodeInt &) = delete;
  BitcodeInt &operator=(const BitcodeInt &) = delete;

public:
  BitcodeInt(Ice::SizeT Bits, uint64_t Val) : BitWidth(Bits), Val(Val) {
    assert(Bits && "bitwidth too small");
    assert(Bits <= BITS_PER_WORD && "bitwidth too big");
    clearUnusedBits();
  }

  int64_t getSExtValue() const {
    return static_cast<int64_t>(Val << (BITS_PER_WORD - BitWidth)) >>
           (BITS_PER_WORD - BitWidth);
  }

  template <typename IntType, typename FpType>
  inline FpType convertToFp() const {
    static_assert(sizeof(IntType) == sizeof(FpType),
                  "IntType and FpType should be the same width");
    assert(BitWidth == sizeof(IntType) * CHAR_BIT);
    auto V = static_cast<IntType>(Val);
    return Ice::Utils::bitCopy<FpType>(V);
  }

private:
  /// Bits in the (internal) value.
  static const Ice::SizeT BITS_PER_WORD = sizeof(uint64_t) * CHAR_BIT;

  uint32_t BitWidth; /// The number of bits in the floating point number.
  uint64_t Val;      /// The (64-bit) equivalent integer value.

  /// Clear unused high order bits.
  void clearUnusedBits() {
    // If all bits are used, we want to leave the value alone.
    if (BitWidth == BITS_PER_WORD)
      return;

    // Mask out the high bits.
    Val &= ~static_cast<uint64_t>(0) >> (BITS_PER_WORD - BitWidth);
  }
};

class BlockParserBaseClass;

// Top-level class to read PNaCl bitcode files, and translate to ICE.
class TopLevelParser final : public NaClBitcodeParser {
  TopLevelParser() = delete;
  TopLevelParser(const TopLevelParser &) = delete;
  TopLevelParser &operator=(const TopLevelParser &) = delete;

public:
  TopLevelParser(Ice::Translator &Translator, NaClBitstreamCursor &Cursor,
                 Ice::ErrorCode &ErrorStatus)
      : NaClBitcodeParser(Cursor), Translator(Translator),
        ErrorStatus(ErrorStatus),
        VariableDeclarations(new Ice::VariableDeclarationList()) {}

  ~TopLevelParser() override = default;

  Ice::Translator &getTranslator() const { return Translator; }

  /// Generates error with given Message, occurring at BitPosition within the
  /// bitcode file. Always returns true.
  bool ErrorAt(naclbitc::ErrorLevel Level, uint64_t BitPosition,
               const std::string &Message) override;

  /// Generates error message with respect to the current block parser.
  bool blockError(const std::string &Message);

  /// Changes the size of the type list to the given size.
  void resizeTypeIDValues(size_t NewSize) { TypeIDValues.resize(NewSize); }

  size_t getNumTypeIDValues() const { return TypeIDValues.size(); }

  /// Returns a pointer to the pool where globals are allocated.
  Ice::VariableDeclarationList *getGlobalVariablesPool() {
    return VariableDeclarations.get();
  }

  /// Returns the undefined type associated with type ID. Note: Returns extended
  /// type ready to be defined.
  ExtendedType *getTypeByIDForDefining(NaClBcIndexSize_t ID) {
    // Get corresponding element, verifying the value is still undefined (and
    // hence allowed to be defined).
    ExtendedType *Ty = getTypeByIDAsKind(ID, ExtendedType::Undefined);
    if (Ty)
      return Ty;
    if (ID >= TypeIDValues.size()) {
      if (ID >= NaClBcIndexSize_t_Max) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Can't define more than " << NaClBcIndexSize_t_Max
               << " types\n";
        blockError(StrBuf.str());
        // Recover by using existing type slot.
        return &TypeIDValues[0];
      }
      Ice::Utils::reserveAndResize(TypeIDValues, ID + 1);
    }
    return &TypeIDValues[ID];
  }

  /// Returns the type associated with the given index.
  Ice::Type getSimpleTypeByID(NaClBcIndexSize_t ID) {
    const ExtendedType *Ty = getTypeByIDAsKind(ID, ExtendedType::Simple);
    if (Ty == nullptr)
      // Return error recovery value.
      return Ice::IceType_void;
    return cast<SimpleExtendedType>(Ty)->getType();
  }

  /// Returns the type signature associated with the given index.
  const Ice::FuncSigType &getFuncSigTypeByID(NaClBcIndexSize_t ID) {
    const ExtendedType *Ty = getTypeByIDAsKind(ID, ExtendedType::FuncSig);
    if (Ty == nullptr)
      // Return error recovery value.
      return UndefinedFuncSigType;
    return cast<FuncSigExtendedType>(Ty)->getSignature();
  }

  /// Sets the next function ID to the given LLVM function.
  void setNextFunctionID(Ice::FunctionDeclaration *Fcn) {
    FunctionDeclarations.push_back(Fcn);
  }

  /// Returns the value id that should be associated with the the current
  /// function block. Increments internal counters during call so that it will
  /// be in correct position for next function block.
  NaClBcIndexSize_t getNextFunctionBlockValueID() {
    size_t NumDeclaredFunctions = FunctionDeclarations.size();
    while (NextDefiningFunctionID < NumDeclaredFunctions &&
           FunctionDeclarations[NextDefiningFunctionID]->isProto())
      ++NextDefiningFunctionID;
    if (NextDefiningFunctionID >= NumDeclaredFunctions)
      Fatal("More function blocks than defined function addresses");
    return NextDefiningFunctionID++;
  }

  /// Returns the function associated with ID.
  Ice::FunctionDeclaration *getFunctionByID(NaClBcIndexSize_t ID) {
    if (ID < FunctionDeclarations.size())
      return FunctionDeclarations[ID];
    return reportGetFunctionByIDError(ID);
  }

  /// Returns the constant associated with the given global value ID.
  Ice::Constant *getGlobalConstantByID(NaClBcIndexSize_t ID) {
    assert(ID < ValueIDConstants.size());
    return ValueIDConstants[ID];
  }

  /// Install names for all global values without names. Called after the global
  /// value symbol table is processed, but before any function blocks are
  /// processed.
  void installGlobalNames() {
    assert(VariableDeclarations);
    installGlobalVarNames();
    installFunctionNames();
  }

  void verifyFunctionTypeSignatures();

  void createValueIDs() {
    assert(VariableDeclarations);
    ValueIDConstants.reserve(VariableDeclarations->size() +
                             FunctionDeclarations.size());
    createValueIDsForFunctions();
    createValueIDsForGlobalVars();
  }

  /// Returns the number of function declarations in the bitcode file.
  size_t getNumFunctionIDs() const { return FunctionDeclarations.size(); }

  /// Returns the number of global declarations (i.e. IDs) defined in the
  /// bitcode file.
  size_t getNumGlobalIDs() const {
    if (VariableDeclarations) {
      return FunctionDeclarations.size() + VariableDeclarations->size();
    } else {
      return ValueIDConstants.size();
    }
  }

  /// Adds the given global declaration to the end of the list of global
  /// declarations.
  void addGlobalDeclaration(Ice::VariableDeclaration *Decl) {
    assert(VariableDeclarations);
    VariableDeclarations->push_back(Decl);
  }

  /// Returns the global variable declaration with the given index.
  Ice::VariableDeclaration *getGlobalVariableByID(NaClBcIndexSize_t Index) {
    assert(VariableDeclarations);
    if (Index < VariableDeclarations->size())
      return VariableDeclarations->at(Index);
    return reportGetGlobalVariableByIDError(Index);
  }

  /// Returns the global declaration (variable or function) with the given
  /// Index.
  Ice::GlobalDeclaration *getGlobalDeclarationByID(NaClBcIndexSize_t Index) {
    size_t NumFunctionIds = FunctionDeclarations.size();
    if (Index < NumFunctionIds)
      return getFunctionByID(Index);
    else
      return getGlobalVariableByID(Index - NumFunctionIds);
  }

  /// Returns true if a module block has been parsed.
  bool parsedModuleBlock() const { return ParsedModuleBlock; }

  /// Returns the list of parsed global variable declarations. Releases
  /// ownership of the current list of global variables. Note: only returns
  /// non-null pointer on first call. All successive calls return a null
  /// pointer.
  std::unique_ptr<Ice::VariableDeclarationList> getGlobalVariables() {
    // Before returning, check that ValidIDConstants has already been built.
    assert(!VariableDeclarations ||
           VariableDeclarations->size() <= ValueIDConstants.size());
    return std::move(VariableDeclarations);
  }

  // Upper limit of alignment power allowed by LLVM
  static constexpr uint32_t AlignPowerLimit = 29;

  // Extracts the corresponding Alignment to use, given the AlignPower (i.e.
  // 2**(AlignPower-1), or 0 if AlignPower == 0). Parser defines the block
  // context the alignment check appears in, and Prefix defines the context the
  // alignment appears in.
  uint32_t extractAlignment(NaClBitcodeParser *Parser, const char *Prefix,
                            uint32_t AlignPower) {
    if (AlignPower <= AlignPowerLimit + 1)
      return (1 << AlignPower) >> 1;
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << Prefix << " alignment greater than 2**" << AlignPowerLimit
           << ". Found: 2**" << (AlignPower - 1);
    Parser->Error(StrBuf.str());
    // Error recover with value that is always acceptable.
    return 1;
  }

private:
  // The translator associated with the parser.
  Ice::Translator &Translator;

  // ErrorStatus should only be updated while this lock is locked.
  Ice::GlobalLockType ErrorReportingLock;
  // The exit status that should be set to true if an error occurs.
  Ice::ErrorCode &ErrorStatus;

  // The types associated with each type ID.
  std::vector<ExtendedType> TypeIDValues;
  // The set of functions (prototype and defined).
  Ice::FunctionDeclarationList FunctionDeclarations;
  // The ID of the next possible defined function ID in FunctionDeclarations.
  // FunctionDeclarations is filled first. It's the set of functions (either
  // defined or isproto). Then function definitions are encountered/parsed and
  // NextDefiningFunctionID is incremented to track the next actually-defined
  // function.
  size_t NextDefiningFunctionID = 0;
  // The set of global variables.
  std::unique_ptr<Ice::VariableDeclarationList> VariableDeclarations;
  // Relocatable constants associated with global declarations.
  Ice::ConstantList ValueIDConstants;
  // Error recovery value to use when getFuncSigTypeByID fails.
  Ice::FuncSigType UndefinedFuncSigType;
  // Defines if a module block has already been parsed.
  bool ParsedModuleBlock = false;

  bool ParseBlock(unsigned BlockID) override;

  // Gets extended type associated with the given index, assuming the extended
  // type is of the WantedKind. Generates error message if corresponding
  // extended type of WantedKind can't be found, and returns nullptr.
  ExtendedType *getTypeByIDAsKind(NaClBcIndexSize_t ID,
                                  ExtendedType::TypeKind WantedKind) {
    ExtendedType *Ty = nullptr;
    if (ID < TypeIDValues.size()) {
      Ty = &TypeIDValues[ID];
      if (Ty->getKind() == WantedKind)
        return Ty;
    }
    // Generate an error message and set ErrorStatus.
    this->reportBadTypeIDAs(ID, Ty, WantedKind);
    return nullptr;
  }

  // Gives Decl a name if it doesn't already have one. Prefix and NameIndex are
  // used to generate the name. NameIndex is automatically incremented if a new
  // name is created. DeclType is literal text describing the type of name being
  // created. Also generates a warning if created names may conflict with named
  // declarations.
  void installDeclarationName(Ice::GlobalDeclaration *Decl,
                              const std::string &Prefix, const char *DeclType,
                              NaClBcIndexSize_t &NameIndex) {
    if (Decl->hasName()) {
      Translator.checkIfUnnamedNameSafe(Decl->getName().toString(), DeclType,
                                        Prefix);
    } else {
      Ice::GlobalContext *Ctx = Translator.getContext();
      // Synthesize a dummy name if any of the following is true:
      // - DUMP is enabled
      // - The symbol is external
      // - The -timing-funcs flag is enabled
      // - Some RangeSpec is initialized with actual names
      if (Ice::BuildDefs::dump() || !Decl->isInternal() ||
          Ice::RangeSpec::hasNames() || Ice::getFlags().getTimeEachFunction()) {
        Decl->setName(Ctx, Translator.createUnnamedName(Prefix, NameIndex));
      } else {
        Decl->setName(Ctx);
      }
      ++NameIndex;
    }
  }

  // Installs names for global variables without names.
  void installGlobalVarNames() {
    assert(VariableDeclarations);
    const std::string &GlobalPrefix = Ice::getFlags().getDefaultGlobalPrefix();
    if (!GlobalPrefix.empty()) {
      NaClBcIndexSize_t NameIndex = 0;
      for (Ice::VariableDeclaration *Var : *VariableDeclarations) {
        installDeclarationName(Var, GlobalPrefix, "global", NameIndex);
      }
    }
  }

  // Installs names for functions without names.
  void installFunctionNames() {
    const std::string &FunctionPrefix =
        Ice::getFlags().getDefaultFunctionPrefix();
    if (!FunctionPrefix.empty()) {
      NaClBcIndexSize_t NameIndex = 0;
      for (Ice::FunctionDeclaration *Func : FunctionDeclarations) {
        installDeclarationName(Func, FunctionPrefix, "function", NameIndex);
      }
    }
  }

  // Builds a constant symbol named Name.  IsExternal is true iff the symbol is
  // external.
  Ice::Constant *getConstantSym(Ice::GlobalString Name, bool IsExternal) const {
    Ice::GlobalContext *Ctx = getTranslator().getContext();
    if (IsExternal) {
      return Ctx->getConstantExternSym(Name);
    } else {
      const Ice::RelocOffsetT Offset = 0;
      return Ctx->getConstantSym(Offset, Name);
    }
  }

  void reportLinkageError(const char *Kind,
                          const Ice::GlobalDeclaration &Decl) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << Kind << " " << Decl.getName()
           << " has incorrect linkage: " << Decl.getLinkageName();
    if (Decl.isExternal())
      StrBuf << "\n  Use flag -allow-externally-defined-symbols to override";
    Error(StrBuf.str());
  }

  // Converts function declarations into constant value IDs.
  void createValueIDsForFunctions() {
    Ice::GlobalContext *Ctx = getTranslator().getContext();
    for (const Ice::FunctionDeclaration *Func : FunctionDeclarations) {
      if (!Func->verifyLinkageCorrect(Ctx))
        reportLinkageError("Function", *Func);
      Ice::Constant *C = getConstantSym(Func->getName(), Func->isProto());
      ValueIDConstants.push_back(C);
    }
  }

  // Converts global variable declarations into constant value IDs.
  void createValueIDsForGlobalVars() {
    for (const Ice::VariableDeclaration *Decl : *VariableDeclarations) {
      if (!Decl->verifyLinkageCorrect())
        reportLinkageError("Global", *Decl);
      Ice::Constant *C =
          getConstantSym(Decl->getName(), !Decl->hasInitializer());
      ValueIDConstants.push_back(C);
    }
  }

  // Reports that type ID is undefined, or not of the WantedType.
  void reportBadTypeIDAs(NaClBcIndexSize_t ID, const ExtendedType *Ty,
                         ExtendedType::TypeKind WantedType);

  // Reports that there is no function declaration for ID. Returns an error
  // recovery value to use.
  Ice::FunctionDeclaration *reportGetFunctionByIDError(NaClBcIndexSize_t ID);

  // Reports that there is not global variable declaration for ID. Returns an
  // error recovery value to use.
  Ice::VariableDeclaration *
  reportGetGlobalVariableByIDError(NaClBcIndexSize_t Index);

  // Reports that there is no corresponding ICE type for LLVMTy, and returns
  // Ice::IceType_void.
  Ice::Type convertToIceTypeError(Type *LLVMTy);
};

bool TopLevelParser::ErrorAt(naclbitc::ErrorLevel Level, uint64_t Bit,
                             const std::string &Message) {
  Ice::GlobalContext *Context = Translator.getContext();
  {
    std::unique_lock<Ice::GlobalLockType> _(ErrorReportingLock);
    ErrorStatus.assign(Ice::EC_Bitcode);
  }
  { // Lock while printing out error message.
    Ice::OstreamLocker L(Context);
    raw_ostream &OldErrStream = setErrStream(Context->getStrError());
    NaClBitcodeParser::ErrorAt(Level, Bit, Message);
    setErrStream(OldErrStream);
  }
  if (Level >= naclbitc::Error && !Ice::getFlags().getAllowErrorRecovery())
    Fatal();
  return true;
}

void TopLevelParser::reportBadTypeIDAs(NaClBcIndexSize_t ID,
                                       const ExtendedType *Ty,
                                       ExtendedType::TypeKind WantedType) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  if (Ty == nullptr) {
    StrBuf << "Can't find extended type for type id: " << ID;
  } else {
    StrBuf << "Type id " << ID << " not " << WantedType << ". Found: " << *Ty;
  }
  blockError(StrBuf.str());
}

Ice::FunctionDeclaration *
TopLevelParser::reportGetFunctionByIDError(NaClBcIndexSize_t ID) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Function index " << ID
         << " not allowed. Out of range. Must be less than "
         << FunctionDeclarations.size();
  blockError(StrBuf.str());
  if (!FunctionDeclarations.empty())
    return FunctionDeclarations[0];
  Fatal();
}

Ice::VariableDeclaration *
TopLevelParser::reportGetGlobalVariableByIDError(NaClBcIndexSize_t Index) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Global index " << Index
         << " not allowed. Out of range. Must be less than "
         << VariableDeclarations->size();
  blockError(StrBuf.str());
  if (!VariableDeclarations->empty())
    return VariableDeclarations->at(0);
  Fatal();
}

Ice::Type TopLevelParser::convertToIceTypeError(Type *LLVMTy) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Invalid LLVM type: " << *LLVMTy;
  Error(StrBuf.str());
  return Ice::IceType_void;
}

void TopLevelParser::verifyFunctionTypeSignatures() {
  const Ice::GlobalContext *Ctx = getTranslator().getContext();
  for (Ice::FunctionDeclaration *FuncDecl : FunctionDeclarations) {
    if (!FuncDecl->validateTypeSignature(Ctx))
      Error(FuncDecl->getTypeSignatureError(Ctx));
  }
}

// Base class for parsing blocks within the bitcode file. Note: Because this is
// the base class of block parsers, we generate error messages if ParseBlock or
// ParseRecord is not overridden in derived classes.
class BlockParserBaseClass : public NaClBitcodeParser {
  BlockParserBaseClass() = delete;
  BlockParserBaseClass(const BlockParserBaseClass &) = delete;
  BlockParserBaseClass &operator=(const BlockParserBaseClass &) = delete;

public:
  // Constructor for the top-level module block parser.
  BlockParserBaseClass(unsigned BlockID, TopLevelParser *Context)
      : NaClBitcodeParser(BlockID, Context), Context(Context) {}

  BlockParserBaseClass(unsigned BlockID, BlockParserBaseClass *EnclosingParser,
                       NaClBitstreamCursor &Cursor)
      : NaClBitcodeParser(BlockID, EnclosingParser, Cursor),
        Context(EnclosingParser->Context) {}

  ~BlockParserBaseClass() override {}

  // Returns the printable name of the type of block being parsed.
  virtual const char *getBlockName() const {
    // If this class is used, it is parsing an unknown block.
    return "unknown";
  }

  // Generates an error Message with the Bit address prefixed to it.
  bool ErrorAt(naclbitc::ErrorLevel Level, uint64_t Bit,
               const std::string &Message) override;

protected:
  // The context parser that contains the decoded state.
  TopLevelParser *Context;
  // True if ErrorAt has been called in this block.
  bool BlockHasError = false;

  // Constructor for nested block parsers.
  BlockParserBaseClass(unsigned BlockID, BlockParserBaseClass *EnclosingParser)
      : NaClBitcodeParser(BlockID, EnclosingParser),
        Context(EnclosingParser->Context) {}

  // Gets the translator associated with the bitcode parser.
  Ice::Translator &getTranslator() const { return Context->getTranslator(); }

  // Default implementation. Reports that block is unknown and skips its
  // contents.
  bool ParseBlock(unsigned BlockID) override;

  // Default implementation. Reports that the record is not understood.
  void ProcessRecord() override;

  // Checks if the size of the record is Size. Return true if valid. Otherwise
  // generates an error and returns false.
  bool isValidRecordSize(size_t Size, const char *RecordName) {
    const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
    if (Values.size() == Size)
      return true;
    reportRecordSizeError(Size, RecordName, nullptr);
    return false;
  }

  // Checks if the size of the record is at least as large as the LowerLimit.
  // Returns true if valid. Otherwise generates an error and returns false.
  bool isValidRecordSizeAtLeast(size_t LowerLimit, const char *RecordName) {
    const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
    if (Values.size() >= LowerLimit)
      return true;
    reportRecordSizeError(LowerLimit, RecordName, "at least");
    return false;
  }

  // Checks if the size of the record is no larger than the
  // UpperLimit.  Returns true if valid. Otherwise generates an error and
  // returns false.
  bool isValidRecordSizeAtMost(size_t UpperLimit, const char *RecordName) {
    const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
    if (Values.size() <= UpperLimit)
      return true;
    reportRecordSizeError(UpperLimit, RecordName, "no more than");
    return false;
  }

  // Checks if the size of the record is at least as large as the LowerLimit,
  // and no larger than the UpperLimit. Returns true if valid. Otherwise
  // generates an error and returns false.
  bool isValidRecordSizeInRange(size_t LowerLimit, size_t UpperLimit,
                                const char *RecordName) {
    return isValidRecordSizeAtLeast(LowerLimit, RecordName) ||
           isValidRecordSizeAtMost(UpperLimit, RecordName);
  }

private:
  /// Generates a record size error. ExpectedSize is the number of elements
  /// expected. RecordName is the name of the kind of record that has incorrect
  /// size. ContextMessage (if not nullptr) is appended to "record expects" to
  /// describe how ExpectedSize should be interpreted.
  void reportRecordSizeError(size_t ExpectedSize, const char *RecordName,
                             const char *ContextMessage);
};

bool TopLevelParser::blockError(const std::string &Message) {
  // TODO(kschimpf): Remove this method. This method used to redirect
  // block-level errors to the block we are in, rather than the top-level
  // block. This gave better bit location for error messages. However, with
  // parallel parsing, we can't keep a field to redirect (there could be many
  // and we don't know which block parser applies). Hence, This redirect can't
  // be applied anymore.
  return Error(Message);
}

// Generates an error Message with the bit address prefixed to it.
bool BlockParserBaseClass::ErrorAt(naclbitc::ErrorLevel Level, uint64_t Bit,
                                   const std::string &Message) {
  BlockHasError = true;
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  // Note: If dump routines have been turned off, the error messages will not
  // be readable. Hence, replace with simple error. We also use the simple form
  // for unit tests.
  if (Ice::getFlags().getGenerateUnitTestMessages()) {
    StrBuf << "Invalid " << getBlockName() << " record: <" << Record.GetCode();
    for (const uint64_t Val : Record.GetValues()) {
      StrBuf << " " << Val;
    }
    StrBuf << ">";
  } else {
    StrBuf << Message;
  }
  return Context->ErrorAt(Level, Record.GetCursor().getErrorBitNo(Bit),
                          StrBuf.str());
}

void BlockParserBaseClass::reportRecordSizeError(size_t ExpectedSize,
                                                 const char *RecordName,
                                                 const char *ContextMessage) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  const char *BlockName = getBlockName();
  const char FirstChar = toupper(*BlockName);
  StrBuf << FirstChar << (BlockName + 1) << " " << RecordName
         << " record expects";
  if (ContextMessage)
    StrBuf << " " << ContextMessage;
  StrBuf << " " << ExpectedSize << " argument";
  if (ExpectedSize > 1)
    StrBuf << "s";
  StrBuf << ". Found: " << Record.GetValues().size();
  Error(StrBuf.str());
}

bool BlockParserBaseClass::ParseBlock(unsigned BlockID) {
  // If called, derived class doesn't know how to handle block. Report error
  // and skip.
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Don't know how to parse block id: " << BlockID;
  Error(StrBuf.str());
  SkipBlock();
  return false;
}

void BlockParserBaseClass::ProcessRecord() {
  // If called, derived class doesn't know how to handle.
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Don't know how to process " << getBlockName()
         << " record:" << Record;
  Error(StrBuf.str());
}

// Class to parse a types block.
class TypesParser final : public BlockParserBaseClass {
  TypesParser() = delete;
  TypesParser(const TypesParser &) = delete;
  TypesParser &operator=(const TypesParser &) = delete;

public:
  TypesParser(unsigned BlockID, BlockParserBaseClass *EnclosingParser)
      : BlockParserBaseClass(BlockID, EnclosingParser),
        Timer(Ice::TimerStack::TT_parseTypes, getTranslator().getContext()) {}

  ~TypesParser() override {
    if (ExpectedNumTypes != Context->getNumTypeIDValues()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Types block expected " << ExpectedNumTypes
             << " types but found: " << NextTypeId;
      Error(StrBuf.str());
    }
  }

private:
  Ice::TimerMarker Timer;
  // The type ID that will be associated with the next type defining record in
  // the types block.
  NaClBcIndexSize_t NextTypeId = 0;

  // The expected number of types, based on record TYPE_CODE_NUMENTRY.
  NaClBcIndexSize_t ExpectedNumTypes = 0;

  void ProcessRecord() override;

  const char *getBlockName() const override { return "type"; }

  void setNextTypeIDAsSimpleType(Ice::Type Ty) {
    Context->getTypeByIDForDefining(NextTypeId++)->setAsSimpleType(Ty);
  }
};

void TypesParser::ProcessRecord() {
  const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
  switch (Record.GetCode()) {
  case naclbitc::TYPE_CODE_NUMENTRY: {
    // NUMENTRY: [numentries]
    if (!isValidRecordSize(1, "count"))
      return;
    uint64_t Size = Values[0];
    if (Size > NaClBcIndexSize_t_Max) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Size to big for count record: " << Size;
      Error(StrBuf.str());
      ExpectedNumTypes = NaClBcIndexSize_t_Max;
    }
    // The code double checks that Expected size and the actual size at the end
    // of the block. To reduce allocations we preallocate the space.
    //
    // However, if the number is large, we suspect that the number is
    // (possibly) incorrect. In that case, we preallocate a smaller space.
    constexpr uint64_t DefaultLargeResizeValue = 1000000;
    Context->resizeTypeIDValues(std::min(Size, DefaultLargeResizeValue));
    ExpectedNumTypes = Size;
    return;
  }
  case naclbitc::TYPE_CODE_VOID:
    // VOID
    if (!isValidRecordSize(0, "void"))
      return;
    setNextTypeIDAsSimpleType(Ice::IceType_void);
    return;
  case naclbitc::TYPE_CODE_FLOAT:
    // FLOAT
    if (!isValidRecordSize(0, "float"))
      return;
    setNextTypeIDAsSimpleType(Ice::IceType_f32);
    return;
  case naclbitc::TYPE_CODE_DOUBLE:
    // DOUBLE
    if (!isValidRecordSize(0, "double"))
      return;
    setNextTypeIDAsSimpleType(Ice::IceType_f64);
    return;
  case naclbitc::TYPE_CODE_INTEGER:
    // INTEGER: [width]
    if (!isValidRecordSize(1, "integer"))
      return;
    switch (Values[0]) {
    case 1:
      setNextTypeIDAsSimpleType(Ice::IceType_i1);
      return;
    case 8:
      setNextTypeIDAsSimpleType(Ice::IceType_i8);
      return;
    case 16:
      setNextTypeIDAsSimpleType(Ice::IceType_i16);
      return;
    case 32:
      setNextTypeIDAsSimpleType(Ice::IceType_i32);
      return;
    case 64:
      setNextTypeIDAsSimpleType(Ice::IceType_i64);
      return;
    default:
      break;
    }
    {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Type integer record with invalid bitsize: " << Values[0];
      Error(StrBuf.str());
    }
    return;
  case naclbitc::TYPE_CODE_VECTOR: {
    // VECTOR: [numelts, eltty]
    if (!isValidRecordSize(2, "vector"))
      return;
    Ice::Type BaseTy = Context->getSimpleTypeByID(Values[1]);
    Ice::SizeT Size = Values[0];
    switch (BaseTy) {
    case Ice::IceType_i1:
      switch (Size) {
      case 4:
        setNextTypeIDAsSimpleType(Ice::IceType_v4i1);
        return;
      case 8:
        setNextTypeIDAsSimpleType(Ice::IceType_v8i1);
        return;
      case 16:
        setNextTypeIDAsSimpleType(Ice::IceType_v16i1);
        return;
      default:
        break;
      }
      break;
    case Ice::IceType_i8:
      if (Size == 16) {
        setNextTypeIDAsSimpleType(Ice::IceType_v16i8);
        return;
      }
      break;
    case Ice::IceType_i16:
      if (Size == 8) {
        setNextTypeIDAsSimpleType(Ice::IceType_v8i16);
        return;
      }
      break;
    case Ice::IceType_i32:
      if (Size == 4) {
        setNextTypeIDAsSimpleType(Ice::IceType_v4i32);
        return;
      }
      break;
    case Ice::IceType_f32:
      if (Size == 4) {
        setNextTypeIDAsSimpleType(Ice::IceType_v4f32);
        return;
      }
      break;
    default:
      break;
    }
    {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Invalid type vector record: <" << Values[0] << " x " << BaseTy
             << ">";
      Error(StrBuf.str());
    }
    return;
  }
  case naclbitc::TYPE_CODE_FUNCTION: {
    // FUNCTION: [vararg, retty, paramty x N]
    if (!isValidRecordSizeAtLeast(2, "signature"))
      return;
    if (Values[0])
      Error("Function type can't define varargs");
    ExtendedType *Ty = Context->getTypeByIDForDefining(NextTypeId++);
    Ty->setAsFunctionType();
    auto *FuncTy = cast<FuncSigExtendedType>(Ty);
    FuncTy->setReturnType(Context->getSimpleTypeByID(Values[1]));
    for (size_t i = 2, e = Values.size(); i != e; ++i) {
      // Check that type void not used as argument type. Note: PNaCl
      // restrictions can't be checked until we know the name, because we have
      // to check for intrinsic signatures.
      Ice::Type ArgTy = Context->getSimpleTypeByID(Values[i]);
      if (ArgTy == Ice::IceType_void) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Type for parameter " << (i - 1)
               << " not valid. Found: " << ArgTy;
        ArgTy = Ice::IceType_i32;
      }
      FuncTy->appendArgType(ArgTy);
    }
    return;
  }
  default:
    BlockParserBaseClass::ProcessRecord();
    return;
  }
  llvm_unreachable("Unknown type block record not processed!");
}

/// Parses the globals block (i.e. global variable declarations and
/// corresponding initializers).
class GlobalsParser final : public BlockParserBaseClass {
  GlobalsParser() = delete;
  GlobalsParser(const GlobalsParser &) = delete;
  GlobalsParser &operator=(const GlobalsParser &) = delete;

public:
  GlobalsParser(unsigned BlockID, BlockParserBaseClass *EnclosingParser)
      : BlockParserBaseClass(BlockID, EnclosingParser),
        Timer(Ice::TimerStack::TT_parseGlobals, getTranslator().getContext()),
        NumFunctionIDs(Context->getNumFunctionIDs()),
        DummyGlobalVar(Ice::VariableDeclaration::create(
            Context->getGlobalVariablesPool())),
        CurGlobalVar(DummyGlobalVar) {
    Context->getGlobalVariablesPool()->willNotBeEmitted(DummyGlobalVar);
  }

  ~GlobalsParser() override = default;

  const char *getBlockName() const override { return "globals"; }

private:
  using GlobalVarsMapType =
      std::unordered_map<NaClBcIndexSize_t, Ice::VariableDeclaration *>;

  Ice::TimerMarker Timer;

  // Holds global variables generated/referenced in the global variables block.
  GlobalVarsMapType GlobalVarsMap;

  // Holds the number of defined function IDs.
  NaClBcIndexSize_t NumFunctionIDs;

  // Holds the specified number of global variables by the count record in the
  // global variables block.
  NaClBcIndexSize_t SpecifiedNumberVars = 0;

  // Keeps track of how many initializers are expected for the global variable
  // declaration being built.
  NaClBcIndexSize_t InitializersNeeded = 0;

  // The index of the next global variable declaration.
  NaClBcIndexSize_t NextGlobalID = 0;

  // Dummy global variable declaration to guarantee CurGlobalVar is always
  // defined (allowing code to not need to check if CurGlobalVar is nullptr).
  Ice::VariableDeclaration *DummyGlobalVar;

  // Holds the current global variable declaration being built.
  Ice::VariableDeclaration *CurGlobalVar;

  // Returns the global variable associated with the given Index.
  Ice::VariableDeclaration *getGlobalVarByID(NaClBcIndexSize_t Index) {
    Ice::VariableDeclaration *&Decl = GlobalVarsMap[Index];
    if (Decl == nullptr)
      Decl =
          Ice::VariableDeclaration::create(Context->getGlobalVariablesPool());
    return Decl;
  }

  // Returns the global declaration associated with the given index.
  Ice::GlobalDeclaration *getGlobalDeclByID(NaClBcIndexSize_t Index) {
    if (Index < NumFunctionIDs)
      return Context->getFunctionByID(Index);
    return getGlobalVarByID(Index - NumFunctionIDs);
  }

  // If global variables parsed correctly, install them into the top-level
  // context.
  void installGlobalVariables() {
    // Verify specified number of globals matches number found.
    size_t NumGlobals = GlobalVarsMap.size();
    if (SpecifiedNumberVars != NumGlobals ||
        SpecifiedNumberVars != NextGlobalID) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << getBlockName() << " block expects " << SpecifiedNumberVars
             << " global variables. Found: " << GlobalVarsMap.size();
      Error(StrBuf.str());
      return;
    }
    // Install global variables into top-level context.
    for (size_t I = 0; I < NumGlobals; ++I)
      Context->addGlobalDeclaration(GlobalVarsMap[I]);
  }

  void ExitBlock() override {
    verifyNoMissingInitializers();
    installGlobalVariables();
    BlockParserBaseClass::ExitBlock();
  }

  void ProcessRecord() override;

  // Checks if the number of initializers for the CurGlobalVar is the same as
  // the number found in the bitcode file. If different, and error message is
  // generated, and the internal state of the parser is fixed so this condition
  // is no longer violated.
  void verifyNoMissingInitializers() {
    size_t NumInits = CurGlobalVar->getInitializers().size();
    if (InitializersNeeded != NumInits) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Global variable @g" << NextGlobalID << " expected "
             << InitializersNeeded << " initializer";
      if (InitializersNeeded > 1)
        StrBuf << "s";
      StrBuf << ". Found: " << NumInits;
      Error(StrBuf.str());
      InitializersNeeded = NumInits;
    }
  }
};

void GlobalsParser::ProcessRecord() {
  const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
  switch (Record.GetCode()) {
  case naclbitc::GLOBALVAR_COUNT:
    // COUNT: [n]
    if (!isValidRecordSize(1, "count"))
      return;
    if (SpecifiedNumberVars || NextGlobalID) {
      Error("Globals count record not first in block.");
      return;
    }
    SpecifiedNumberVars = Values[0];
    return;
  case naclbitc::GLOBALVAR_VAR: {
    // VAR: [align, isconst]
    if (!isValidRecordSize(2, "variable"))
      return;
    verifyNoMissingInitializers();
    // Always build the global variable, even if IR generation is turned off.
    // This is needed because we need a placeholder in the top-level context
    // when no IR is generated.
    uint32_t Alignment =
        Context->extractAlignment(this, "Global variable", Values[0]);
    CurGlobalVar = getGlobalVarByID(NextGlobalID);
    InitializersNeeded = 1;
    CurGlobalVar->setAlignment(Alignment);
    CurGlobalVar->setIsConstant(Values[1] != 0);
    ++NextGlobalID;
    return;
  }
  case naclbitc::GLOBALVAR_COMPOUND:
    // COMPOUND: [size]
    if (!isValidRecordSize(1, "compound"))
      return;
    if (!CurGlobalVar->getInitializers().empty()) {
      Error("Globals compound record not first initializer");
      return;
    }
    if (Values[0] < 2) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << getBlockName()
             << " compound record size invalid. Found: " << Values[0];
      Error(StrBuf.str());
      return;
    }
    InitializersNeeded = Values[0];
    return;
  case naclbitc::GLOBALVAR_ZEROFILL: {
    // ZEROFILL: [size]
    if (!isValidRecordSize(1, "zerofill"))
      return;
    auto *Pool = Context->getGlobalVariablesPool();
    CurGlobalVar->addInitializer(
        Ice::VariableDeclaration::ZeroInitializer::create(Pool, Values[0]));
    return;
  }
  case naclbitc::GLOBALVAR_DATA: {
    // DATA: [b0, b1, ...]
    if (!isValidRecordSizeAtLeast(1, "data"))
      return;
    auto *Pool = Context->getGlobalVariablesPool();
    CurGlobalVar->addInitializer(
        Ice::VariableDeclaration::DataInitializer::create(Pool, Values));
    return;
  }
  case naclbitc::GLOBALVAR_RELOC: {
    // RELOC: [val, [addend]]
    if (!isValidRecordSizeInRange(1, 2, "reloc"))
      return;
    NaClBcIndexSize_t Index = Values[0];
    NaClBcIndexSize_t IndexLimit = SpecifiedNumberVars + NumFunctionIDs;
    if (Index >= IndexLimit) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Relocation index " << Index << " to big. Expect index < "
             << IndexLimit;
      Error(StrBuf.str());
    }
    uint64_t Offset = 0;
    if (Values.size() == 2) {
      Offset = Values[1];
      if (Offset > std::numeric_limits<uint32_t>::max()) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Addend of global reloc record too big: " << Offset;
        Error(StrBuf.str());
      }
    }
    auto *Pool = Context->getGlobalVariablesPool();
    Ice::GlobalContext *Ctx = getTranslator().getContext();
    CurGlobalVar->addInitializer(
        Ice::VariableDeclaration::RelocInitializer::create(
            Pool, getGlobalDeclByID(Index),
            {Ice::RelocOffset::create(Ctx, Offset)}));
    return;
  }
  default:
    BlockParserBaseClass::ProcessRecord();
    return;
  }
}

/// Base class for parsing a valuesymtab block in the bitcode file.
class ValuesymtabParser : public BlockParserBaseClass {
  ValuesymtabParser() = delete;
  ValuesymtabParser(const ValuesymtabParser &) = delete;
  void operator=(const ValuesymtabParser &) = delete;

public:
  ValuesymtabParser(unsigned BlockID, BlockParserBaseClass *EnclosingParser)
      : BlockParserBaseClass(BlockID, EnclosingParser) {}

  ~ValuesymtabParser() override = default;

  const char *getBlockName() const override { return "valuesymtab"; }

protected:
  using StringType = SmallString<128>;

  // Returns the name to identify the kind of symbol table this is
  // in error messages.
  virtual const char *getTableKind() const = 0;

  // Associates Name with the value defined by the given Index.
  virtual void setValueName(NaClBcIndexSize_t Index, StringType &Name) = 0;

  // Associates Name with the value defined by the given Index;
  virtual void setBbName(NaClBcIndexSize_t Index, StringType &Name) = 0;

  // Reports that the assignment of Name to the value associated with
  // index is not possible, for the given Context.
  void reportUnableToAssign(const char *Context, NaClBcIndexSize_t Index,
                            StringType &Name);

private:
  using NamesSetType = std::unordered_set<StringType>;
  NamesSetType ValueNames;
  NamesSetType BlockNames;

  void ProcessRecord() override;

  // Extracts out ConvertedName. Returns true if unique wrt to Names.
  bool convertToString(NamesSetType &Names, StringType &ConvertedName) {
    const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
    for (size_t i = 1, e = Values.size(); i != e; ++i) {
      ConvertedName += static_cast<char>(Values[i]);
    }
    auto Pair = Names.insert(ConvertedName);
    return Pair.second;
  }

  void ReportDuplicateName(const char *NameCat, StringType &Name);
};

void ValuesymtabParser::reportUnableToAssign(const char *Context,
                                             NaClBcIndexSize_t Index,
                                             StringType &Name) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << getTableKind() << " " << getBlockName() << ": " << Context
         << " name '" << Name << "' can't be associated with index " << Index;
  Error(StrBuf.str());
}

void ValuesymtabParser::ReportDuplicateName(const char *NameCat,
                                            StringType &Name) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << getTableKind() << " " << getBlockName() << " defines duplicate "
         << NameCat << " name: '" << Name << "'";
  Error(StrBuf.str());
}

void ValuesymtabParser::ProcessRecord() {
  const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
  StringType ConvertedName;
  switch (Record.GetCode()) {
  case naclbitc::VST_CODE_ENTRY: {
    // VST_ENTRY: [ValueId, namechar x N]
    if (!isValidRecordSizeAtLeast(2, "value entry"))
      return;
    if (convertToString(ValueNames, ConvertedName))
      setValueName(Values[0], ConvertedName);
    else
      ReportDuplicateName("value", ConvertedName);
    return;
  }
  case naclbitc::VST_CODE_BBENTRY: {
    // VST_BBENTRY: [BbId, namechar x N]
    if (!isValidRecordSizeAtLeast(2, "basic block entry"))
      return;
    if (convertToString(BlockNames, ConvertedName))
      setBbName(Values[0], ConvertedName);
    else
      ReportDuplicateName("block", ConvertedName);
    return;
  }
  default:
    break;
  }
  // If reached, don't know how to handle record.
  BlockParserBaseClass::ProcessRecord();
  return;
}

/// Parses function blocks in the bitcode file.
class FunctionParser final : public BlockParserBaseClass {
  FunctionParser() = delete;
  FunctionParser(const FunctionParser &) = delete;
  FunctionParser &operator=(const FunctionParser &) = delete;

public:
  FunctionParser(unsigned BlockID, BlockParserBaseClass *EnclosingParser,
                 NaClBcIndexSize_t FcnId)
      : BlockParserBaseClass(BlockID, EnclosingParser),
        Timer(Ice::TimerStack::TT_parseFunctions, getTranslator().getContext()),
        Func(nullptr), FuncDecl(Context->getFunctionByID(FcnId)),
        CachedNumGlobalValueIDs(Context->getNumGlobalIDs()),
        NextLocalInstIndex(Context->getNumGlobalIDs()) {}

  FunctionParser(unsigned BlockID, BlockParserBaseClass *EnclosingParser,
                 NaClBcIndexSize_t FcnId, NaClBitstreamCursor &Cursor)
      : BlockParserBaseClass(BlockID, EnclosingParser, Cursor),
        Timer(Ice::TimerStack::TT_parseFunctions, getTranslator().getContext()),
        Func(nullptr), FuncDecl(Context->getFunctionByID(FcnId)),
        CachedNumGlobalValueIDs(Context->getNumGlobalIDs()),
        NextLocalInstIndex(Context->getNumGlobalIDs()) {}

  std::unique_ptr<Ice::Cfg> parseFunction(uint32_t SeqNumber) {
    bool ParserResult;
    Ice::GlobalContext *Ctx = getTranslator().getContext();
    {
      Ice::TimerMarker T(Ctx, FuncDecl->getName().toStringOrEmpty());
      // Note: The Cfg is created, even when IR generation is disabled. This is
      // done to install a CfgLocalAllocator for various internal containers.
      Ice::GlobalContext *Ctx = getTranslator().getContext();
      Func = Ice::Cfg::create(Ctx, SeqNumber);

      Ice::CfgLocalAllocatorScope _(Func.get());

      // TODO(kschimpf) Clean up API to add a function signature to a CFG.
      const Ice::FuncSigType &Signature = FuncDecl->getSignature();

      Func->setFunctionName(FuncDecl->getName());
      Func->setReturnType(Signature.getReturnType());
      Func->setInternal(FuncDecl->getLinkage() == GlobalValue::InternalLinkage);
      CurrentNode = installNextBasicBlock();
      Func->setEntryNode(CurrentNode);
      for (Ice::Type ArgType : Signature.getArgList()) {
        Func->addArg(getNextInstVar(ArgType));
      }

      ParserResult = ParseThisBlock();
    }

    if (ParserResult || BlockHasError)
      Func->setError("Unable to parse function");

    return std::move(Func);
  }

  ~FunctionParser() override = default;

  const char *getBlockName() const override { return "function"; }

  Ice::Cfg *getFunc() const { return Func.get(); }

  size_t getNumGlobalIDs() const { return CachedNumGlobalValueIDs; }

  void setNextLocalInstIndex(Ice::Operand *Op) {
    setOperand(NextLocalInstIndex++, Op);
  }

  // Set the next constant ID to the given constant C.
  void setNextConstantID(Ice::Constant *C) { setNextLocalInstIndex(C); }

  // Returns the value referenced by the given value Index.
  Ice::Operand *getOperand(NaClBcIndexSize_t Index) {
    if (Index < CachedNumGlobalValueIDs) {
      return Context->getGlobalConstantByID(Index);
    }
    NaClBcIndexSize_t LocalIndex = Index - CachedNumGlobalValueIDs;
    if (LocalIndex >= LocalOperands.size())
      reportGetOperandUndefined(Index);
    Ice::Operand *Op = LocalOperands[LocalIndex];
    if (Op == nullptr)
      reportGetOperandUndefined(Index);
    return Op;
  }

private:
  Ice::TimerMarker Timer;
  // The number of words in the bitstream defining the function block.
  uint64_t NumBytesDefiningFunction = 0;
  // Maximum number of records that can appear in the function block, based on
  // the number of bytes defining the function block.
  uint64_t MaxRecordsInBlock = 0;
  // The corresponding ICE function defined by the function block.
  std::unique_ptr<Ice::Cfg> Func;
  // The index to the current basic block being built.
  NaClBcIndexSize_t CurrentBbIndex = 0;
  // The number of basic blocks declared for the function block.
  NaClBcIndexSize_t DeclaredNumberBbs = 0;
  // The basic block being built.
  Ice::CfgNode *CurrentNode = nullptr;
  // The corresponding function declaration.
  Ice::FunctionDeclaration *FuncDecl;
  // Holds the dividing point between local and global absolute value indices.
  size_t CachedNumGlobalValueIDs;
  // Holds operands local to the function block, based on indices defined in
  // the bitcode file.
  Ice::OperandList LocalOperands;
  // Holds the index within LocalOperands corresponding to the next instruction
  // that generates a value.
  NaClBcIndexSize_t NextLocalInstIndex;
  // True if the last processed instruction was a terminating instruction.
  bool InstIsTerminating = false;

  bool ParseBlock(unsigned BlockID) override;

  void ProcessRecord() override;

  void EnterBlock(unsigned NumWords) override {
    // Note: Bitstream defines words as 32-bit values.
    NumBytesDefiningFunction = NumWords * sizeof(uint32_t);
    // We know that all records are minimally defined by a two-bit abreviation.
    MaxRecordsInBlock = NumBytesDefiningFunction * (CHAR_BIT >> 1);
  }

  void ExitBlock() override;

  // Creates and appends a new basic block to the list of basic blocks.
  Ice::CfgNode *installNextBasicBlock() {
    Ice::CfgNode *Node = Func->makeNode();
    return Node;
  }

  // Returns the Index-th basic block in the list of basic blocks.
  Ice::CfgNode *getBasicBlock(NaClBcIndexSize_t Index) {
    if (Index >= Func->getNumNodes()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Reference to basic block " << Index
             << " not found. Must be less than " << Func->getNumNodes();
      Error(StrBuf.str());
      Index = 0;
    }
    return Func->getNodes()[Index];
  }

  // Returns the Index-th basic block in the list of basic blocks. Assumes
  // Index corresponds to a branch instruction. Hence, if the branch references
  // the entry block, it also generates a corresponding error.
  Ice::CfgNode *getBranchBasicBlock(NaClBcIndexSize_t Index) {
    if (Index == 0) {
      Error("Branch to entry block not allowed");
    }
    return getBasicBlock(Index);
  }

  // Generate an instruction variable with type Ty.
  Ice::Variable *createInstVar(Ice::Type Ty) {
    if (Ty == Ice::IceType_void) {
      Error("Can't define instruction value using type void");
      // Recover since we can't throw an exception.
      Ty = Ice::IceType_i32;
    }
    return Func->makeVariable(Ty);
  }

  // Generates the next available local variable using the given type.
  Ice::Variable *getNextInstVar(Ice::Type Ty) {
    assert(NextLocalInstIndex >= CachedNumGlobalValueIDs);
    // Before creating one, see if a forwardtyperef has already defined it.
    NaClBcIndexSize_t LocalIndex = NextLocalInstIndex - CachedNumGlobalValueIDs;
    if (LocalIndex < LocalOperands.size()) {
      Ice::Operand *Op = LocalOperands[LocalIndex];
      if (Op != nullptr) {
        if (auto *Var = dyn_cast<Ice::Variable>(Op)) {
          if (Var->getType() == Ty) {
            ++NextLocalInstIndex;
            return Var;
          }
        }
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Illegal forward referenced instruction ("
               << NextLocalInstIndex << "): " << *Op;
        Error(StrBuf.str());
        ++NextLocalInstIndex;
        return createInstVar(Ty);
      }
    }
    Ice::Variable *Var = createInstVar(Ty);
    setOperand(NextLocalInstIndex++, Var);
    return Var;
  }

  // Converts a relative index (wrt to BaseIndex) to an absolute value index.
  NaClBcIndexSize_t convertRelativeToAbsIndex(NaClRelBcIndexSize_t Id,
                                              NaClRelBcIndexSize_t BaseIndex) {
    if (BaseIndex < Id) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Invalid relative value id: " << Id
             << " (must be <= " << BaseIndex << ")";
      Error(StrBuf.str());
      return 0;
    }
    return BaseIndex - Id;
  }

  // Sets element Index (in the local operands list) to Op.
  void setOperand(NaClBcIndexSize_t Index, Ice::Operand *Op) {
    assert(Op);
    // Check if simple push works.
    NaClBcIndexSize_t LocalIndex = Index - CachedNumGlobalValueIDs;
    if (LocalIndex == LocalOperands.size()) {
      LocalOperands.push_back(Op);
      return;
    }

    // Must be forward reference, expand vector to accommodate.
    if (LocalIndex >= LocalOperands.size()) {
      if (LocalIndex > MaxRecordsInBlock) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Forward reference @" << Index << " too big. Have "
               << CachedNumGlobalValueIDs << " globals and function contains "
               << NumBytesDefiningFunction << " bytes";
        Fatal(StrBuf.str());
        // Recover by using index one beyond the maximal allowed.
        LocalIndex = MaxRecordsInBlock;
      }
      Ice::Utils::reserveAndResize(LocalOperands, LocalIndex + 1);
    }

    // If element not defined, set it.
    Ice::Operand *OldOp = LocalOperands[LocalIndex];
    if (OldOp == nullptr) {
      LocalOperands[LocalIndex] = Op;
      return;
    }

    // See if forward reference matches.
    if (OldOp == Op)
      return;

    // Error has occurred.
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Multiple definitions for index " << Index << ": " << *Op
           << " and " << *OldOp;
    Error(StrBuf.str());
    LocalOperands[LocalIndex] = Op;
  }

  // Returns the relative operand (wrt to BaseIndex) referenced by the given
  // value Index.
  Ice::Operand *getRelativeOperand(NaClBcIndexSize_t Index,
                                   NaClBcIndexSize_t BaseIndex) {
    return getOperand(convertRelativeToAbsIndex(Index, BaseIndex));
  }

  // Returns the absolute index of the next value generating instruction.
  NaClBcIndexSize_t getNextInstIndex() const { return NextLocalInstIndex; }

  // Generates type error message for binary operator Op operating on Type
  // OpTy.
  void reportInvalidBinaryOp(Ice::InstArithmetic::OpKind Op, Ice::Type OpTy);

  // Validates if integer logical Op, for type OpTy, is valid. Returns true if
  // valid. Otherwise generates error message and returns false.
  bool isValidIntegerLogicalOp(Ice::InstArithmetic::OpKind Op, Ice::Type OpTy) {
    if (Ice::isIntegerType(OpTy))
      return true;
    reportInvalidBinaryOp(Op, OpTy);
    return false;
  }

  // Validates if integer (or vector of integers) arithmetic Op, for type OpTy,
  // is valid. Returns true if valid. Otherwise generates error message and
  // returns false.
  bool isValidIntegerArithOp(Ice::InstArithmetic::OpKind Op, Ice::Type OpTy) {
    if (Ice::isIntegerArithmeticType(OpTy))
      return true;
    reportInvalidBinaryOp(Op, OpTy);
    return false;
  }

  // Checks if floating arithmetic Op, for type OpTy, is valid. Returns true if
  // valid. Otherwise generates an error message and returns false;
  bool isValidFloatingArithOp(Ice::InstArithmetic::OpKind Op, Ice::Type OpTy) {
    if (Ice::isFloatingType(OpTy))
      return true;
    reportInvalidBinaryOp(Op, OpTy);
    return false;
  }

  // Checks if the type of operand Op is the valid pointer type, for the given
  // InstructionName. Returns true if valid. Otherwise generates an error
  // message and returns false.
  bool isValidPointerType(Ice::Operand *Op, const char *InstructionName) {
    Ice::Type PtrType = Ice::getPointerType();
    if (Op->getType() == PtrType)
      return true;
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << InstructionName << " address not " << PtrType
           << ". Found: " << Op->getType();
    Error(StrBuf.str());
    return false;
  }

  // Checks if loading/storing a value of type Ty is allowed. Returns true if
  // Valid. Otherwise generates an error message and returns false.
  bool isValidLoadStoreType(Ice::Type Ty, const char *InstructionName) {
    if (isLoadStoreType(Ty))
      return true;
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << InstructionName << " type not allowed: " << Ty << "*";
    Error(StrBuf.str());
    return false;
  }

  // Checks if loading/storing a value of type Ty is allowed for the given
  // Alignment. Otherwise generates an error message and returns false.
  bool isValidLoadStoreAlignment(size_t Alignment, Ice::Type Ty,
                                 const char *InstructionName) {
    if (!isValidLoadStoreType(Ty, InstructionName))
      return false;
    if (isAllowedAlignment(Alignment, Ty))
      return true;
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << InstructionName << " " << Ty << "*: not allowed for alignment "
           << Alignment;
    Error(StrBuf.str());
    return false;
  }

  // Defines if the given alignment is valid for the given type. Simplified
  // version of PNaClABIProps::isAllowedAlignment, based on API's offered for
  // Ice::Type.
  bool isAllowedAlignment(size_t Alignment, Ice::Type Ty) const {
    return Alignment == typeAlignInBytes(Ty) ||
           (Alignment == 1 && !isVectorType(Ty));
  }

  // Types of errors that can occur for insertelement and extractelement
  // instructions.
  enum VectorIndexCheckValue {
    VectorIndexNotVector,
    VectorIndexNotConstant,
    VectorIndexNotInRange,
    VectorIndexNotI32,
    VectorIndexValid
  };

  void dumpVectorIndexCheckValue(raw_ostream &Stream,
                                 VectorIndexCheckValue Value) const {
    if (!Ice::BuildDefs::dump())
      return;
    switch (Value) {
    case VectorIndexNotVector:
      Stream << "Vector index on non vector";
      break;
    case VectorIndexNotConstant:
      Stream << "Vector index not integer constant";
      break;
    case VectorIndexNotInRange:
      Stream << "Vector index not in range of vector";
      break;
    case VectorIndexNotI32:
      Stream << "Vector index not of type " << Ice::IceType_i32;
      break;
    case VectorIndexValid:
      Stream << "Valid vector index";
      break;
    }
  }

  // Returns whether the given vector index (for insertelement and
  // extractelement instructions) is valid.
  VectorIndexCheckValue validateVectorIndex(const Ice::Operand *Vec,
                                            const Ice::Operand *Index) const {
    Ice::Type VecType = Vec->getType();
    if (!Ice::isVectorType(VecType))
      return VectorIndexNotVector;
    const auto *C = dyn_cast<Ice::ConstantInteger32>(Index);
    if (C == nullptr)
      return VectorIndexNotConstant;
    if (static_cast<size_t>(C->getValue()) >= typeNumElements(VecType))
      return VectorIndexNotInRange;
    if (Index->getType() != Ice::IceType_i32)
      return VectorIndexNotI32;
    return VectorIndexValid;
  }

  // Takes the PNaCl bitcode binary operator Opcode, and the opcode type Ty,
  // and sets Op to the corresponding ICE binary opcode. Returns true if able
  // to convert, false otherwise.
  bool convertBinopOpcode(unsigned Opcode, Ice::Type Ty,
                          Ice::InstArithmetic::OpKind &Op) {
    switch (Opcode) {
    default: {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Binary opcode " << Opcode << "not understood for type " << Ty;
      Error(StrBuf.str());
      Op = Ice::InstArithmetic::Add;
      return false;
    }
    case naclbitc::BINOP_ADD:
      if (Ice::isIntegerType(Ty)) {
        Op = Ice::InstArithmetic::Add;
        return isValidIntegerArithOp(Op, Ty);
      } else {
        Op = Ice::InstArithmetic::Fadd;
        return isValidFloatingArithOp(Op, Ty);
      }
    case naclbitc::BINOP_SUB:
      if (Ice::isIntegerType(Ty)) {
        Op = Ice::InstArithmetic::Sub;
        return isValidIntegerArithOp(Op, Ty);
      } else {
        Op = Ice::InstArithmetic::Fsub;
        return isValidFloatingArithOp(Op, Ty);
      }
    case naclbitc::BINOP_MUL:
      if (Ice::isIntegerType(Ty)) {
        Op = Ice::InstArithmetic::Mul;
        return isValidIntegerArithOp(Op, Ty);
      } else {
        Op = Ice::InstArithmetic::Fmul;
        return isValidFloatingArithOp(Op, Ty);
      }
    case naclbitc::BINOP_UDIV:
      Op = Ice::InstArithmetic::Udiv;
      return isValidIntegerArithOp(Op, Ty);
    case naclbitc::BINOP_SDIV:
      if (Ice::isIntegerType(Ty)) {
        Op = Ice::InstArithmetic::Sdiv;
        return isValidIntegerArithOp(Op, Ty);
      } else {
        Op = Ice::InstArithmetic::Fdiv;
        return isValidFloatingArithOp(Op, Ty);
      }
    case naclbitc::BINOP_UREM:
      Op = Ice::InstArithmetic::Urem;
      return isValidIntegerArithOp(Op, Ty);
    case naclbitc::BINOP_SREM:
      if (Ice::isIntegerType(Ty)) {
        Op = Ice::InstArithmetic::Srem;
        return isValidIntegerArithOp(Op, Ty);
      } else {
        Op = Ice::InstArithmetic::Frem;
        return isValidFloatingArithOp(Op, Ty);
      }
    case naclbitc::BINOP_SHL:
      Op = Ice::InstArithmetic::Shl;
      return isValidIntegerArithOp(Op, Ty);
    case naclbitc::BINOP_LSHR:
      Op = Ice::InstArithmetic::Lshr;
      return isValidIntegerArithOp(Op, Ty);
    case naclbitc::BINOP_ASHR:
      Op = Ice::InstArithmetic::Ashr;
      return isValidIntegerArithOp(Op, Ty);
    case naclbitc::BINOP_AND:
      Op = Ice::InstArithmetic::And;
      return isValidIntegerLogicalOp(Op, Ty);
    case naclbitc::BINOP_OR:
      Op = Ice::InstArithmetic::Or;
      return isValidIntegerLogicalOp(Op, Ty);
    case naclbitc::BINOP_XOR:
      Op = Ice::InstArithmetic::Xor;
      return isValidIntegerLogicalOp(Op, Ty);
    }
  }

  /// Simplifies out vector types from Type1 and Type2, if both are vectors of
  /// the same size. Returns true iff both are vectors of the same size, or are
  /// both scalar types.
  static bool simplifyOutCommonVectorType(Ice::Type &Type1, Ice::Type &Type2) {
    bool IsType1Vector = isVectorType(Type1);
    bool IsType2Vector = isVectorType(Type2);
    if (IsType1Vector != IsType2Vector)
      return false;
    if (!IsType1Vector)
      return true;
    if (typeNumElements(Type1) != typeNumElements(Type2))
      return false;
    Type1 = typeElementType(Type1);
    Type2 = typeElementType(Type2);
    return true;
  }

  /// Returns true iff an integer truncation from SourceType to TargetType is
  /// valid.
  static bool isIntTruncCastValid(Ice::Type SourceType, Ice::Type TargetType) {
    return Ice::isIntegerType(SourceType) && Ice::isIntegerType(TargetType) &&
           simplifyOutCommonVectorType(SourceType, TargetType) &&
           getScalarIntBitWidth(SourceType) > getScalarIntBitWidth(TargetType);
  }

  /// Returns true iff a floating type truncation from SourceType to TargetType
  /// is valid.
  static bool isFloatTruncCastValid(Ice::Type SourceType,
                                    Ice::Type TargetType) {
    return simplifyOutCommonVectorType(SourceType, TargetType) &&
           SourceType == Ice::IceType_f64 && TargetType == Ice::IceType_f32;
  }

  /// Returns true iff an integer extension from SourceType to TargetType is
  /// valid.
  static bool isIntExtCastValid(Ice::Type SourceType, Ice::Type TargetType) {
    return isIntTruncCastValid(TargetType, SourceType);
  }

  /// Returns true iff a floating type extension from SourceType to TargetType
  /// is valid.
  static bool isFloatExtCastValid(Ice::Type SourceType, Ice::Type TargetType) {
    return isFloatTruncCastValid(TargetType, SourceType);
  }

  /// Returns true iff a cast from floating type SourceType to integer type
  /// TargetType is valid.
  static bool isFloatToIntCastValid(Ice::Type SourceType,
                                    Ice::Type TargetType) {
    if (!(Ice::isFloatingType(SourceType) && Ice::isIntegerType(TargetType)))
      return false;
    bool IsSourceVector = isVectorType(SourceType);
    bool IsTargetVector = isVectorType(TargetType);
    if (IsSourceVector != IsTargetVector)
      return false;
    if (IsSourceVector) {
      return typeNumElements(SourceType) == typeNumElements(TargetType);
    }
    return true;
  }

  /// Returns true iff a cast from integer type SourceType to floating type
  /// TargetType is valid.
  static bool isIntToFloatCastValid(Ice::Type SourceType,
                                    Ice::Type TargetType) {
    return isFloatToIntCastValid(TargetType, SourceType);
  }

  /// Returns the number of bits used to model type Ty when defining the bitcast
  /// instruction.
  static Ice::SizeT bitcastSizeInBits(Ice::Type Ty) {
    if (Ice::isVectorType(Ty))
      return Ice::typeNumElements(Ty) *
             bitcastSizeInBits(Ice::typeElementType(Ty));
    if (Ty == Ice::IceType_i1)
      return 1;
    return Ice::typeWidthInBytes(Ty) * CHAR_BIT;
  }

  /// Returns true iff a bitcast from SourceType to TargetType is allowed.
  static bool isBitcastValid(Ice::Type SourceType, Ice::Type TargetType) {
    return bitcastSizeInBits(SourceType) == bitcastSizeInBits(TargetType);
  }

  /// Returns true iff the NaCl bitcode Opcode is a valid cast opcode for
  /// converting SourceType to TargetType. Updates CastKind to the corresponding
  /// instruction cast opcode. Also generates an error message when this
  /// function returns false.
  bool convertCastOpToIceOp(uint64_t Opcode, Ice::Type SourceType,
                            Ice::Type TargetType,
                            Ice::InstCast::OpKind &CastKind) {
    bool Result;
    switch (Opcode) {
    default: {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Cast opcode " << Opcode << " not understood.\n";
      Error(StrBuf.str());
      CastKind = Ice::InstCast::Bitcast;
      return false;
    }
    case naclbitc::CAST_TRUNC:
      CastKind = Ice::InstCast::Trunc;
      Result = isIntTruncCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_ZEXT:
      CastKind = Ice::InstCast::Zext;
      Result = isIntExtCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_SEXT:
      CastKind = Ice::InstCast::Sext;
      Result = isIntExtCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_FPTOUI:
      CastKind = Ice::InstCast::Fptoui;
      Result = isFloatToIntCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_FPTOSI:
      CastKind = Ice::InstCast::Fptosi;
      Result = isFloatToIntCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_UITOFP:
      CastKind = Ice::InstCast::Uitofp;
      Result = isIntToFloatCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_SITOFP:
      CastKind = Ice::InstCast::Sitofp;
      Result = isIntToFloatCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_FPTRUNC:
      CastKind = Ice::InstCast::Fptrunc;
      Result = isFloatTruncCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_FPEXT:
      CastKind = Ice::InstCast::Fpext;
      Result = isFloatExtCastValid(SourceType, TargetType);
      break;
    case naclbitc::CAST_BITCAST:
      CastKind = Ice::InstCast::Bitcast;
      Result = isBitcastValid(SourceType, TargetType);
      break;
    }
    if (!Result) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Illegal cast: " << Ice::InstCast::getCastName(CastKind) << " "
             << SourceType << " to " << TargetType;
      Error(StrBuf.str());
    }
    return Result;
  }

  // Converts PNaCl bitcode Icmp operator to corresponding ICE op. Returns true
  // if able to convert, false otherwise.
  bool convertNaClBitcICmpOpToIce(uint64_t Op,
                                  Ice::InstIcmp::ICond &Cond) const {
    switch (Op) {
    case naclbitc::ICMP_EQ:
      Cond = Ice::InstIcmp::Eq;
      return true;
    case naclbitc::ICMP_NE:
      Cond = Ice::InstIcmp::Ne;
      return true;
    case naclbitc::ICMP_UGT:
      Cond = Ice::InstIcmp::Ugt;
      return true;
    case naclbitc::ICMP_UGE:
      Cond = Ice::InstIcmp::Uge;
      return true;
    case naclbitc::ICMP_ULT:
      Cond = Ice::InstIcmp::Ult;
      return true;
    case naclbitc::ICMP_ULE:
      Cond = Ice::InstIcmp::Ule;
      return true;
    case naclbitc::ICMP_SGT:
      Cond = Ice::InstIcmp::Sgt;
      return true;
    case naclbitc::ICMP_SGE:
      Cond = Ice::InstIcmp::Sge;
      return true;
    case naclbitc::ICMP_SLT:
      Cond = Ice::InstIcmp::Slt;
      return true;
    case naclbitc::ICMP_SLE:
      Cond = Ice::InstIcmp::Sle;
      return true;
    default:
      // Make sure Cond is always initialized.
      Cond = static_cast<Ice::InstIcmp::ICond>(0);
      return false;
    }
  }

  // Converts PNaCl bitcode Fcmp operator to corresponding ICE op. Returns true
  // if able to convert, false otherwise.
  bool convertNaClBitcFCompOpToIce(uint64_t Op,
                                   Ice::InstFcmp::FCond &Cond) const {
    switch (Op) {
    case naclbitc::FCMP_FALSE:
      Cond = Ice::InstFcmp::False;
      return true;
    case naclbitc::FCMP_OEQ:
      Cond = Ice::InstFcmp::Oeq;
      return true;
    case naclbitc::FCMP_OGT:
      Cond = Ice::InstFcmp::Ogt;
      return true;
    case naclbitc::FCMP_OGE:
      Cond = Ice::InstFcmp::Oge;
      return true;
    case naclbitc::FCMP_OLT:
      Cond = Ice::InstFcmp::Olt;
      return true;
    case naclbitc::FCMP_OLE:
      Cond = Ice::InstFcmp::Ole;
      return true;
    case naclbitc::FCMP_ONE:
      Cond = Ice::InstFcmp::One;
      return true;
    case naclbitc::FCMP_ORD:
      Cond = Ice::InstFcmp::Ord;
      return true;
    case naclbitc::FCMP_UNO:
      Cond = Ice::InstFcmp::Uno;
      return true;
    case naclbitc::FCMP_UEQ:
      Cond = Ice::InstFcmp::Ueq;
      return true;
    case naclbitc::FCMP_UGT:
      Cond = Ice::InstFcmp::Ugt;
      return true;
    case naclbitc::FCMP_UGE:
      Cond = Ice::InstFcmp::Uge;
      return true;
    case naclbitc::FCMP_ULT:
      Cond = Ice::InstFcmp::Ult;
      return true;
    case naclbitc::FCMP_ULE:
      Cond = Ice::InstFcmp::Ule;
      return true;
    case naclbitc::FCMP_UNE:
      Cond = Ice::InstFcmp::Une;
      return true;
    case naclbitc::FCMP_TRUE:
      Cond = Ice::InstFcmp::True;
      return true;
    default:
      // Make sure Cond is always initialized.
      Cond = static_cast<Ice::InstFcmp::FCond>(0);
      return false;
    }
  }

  // Creates an error instruction, generating a value of type Ty, and adds a
  // placeholder so that instruction indices line up. Some instructions, such
  // as a call, will not generate a value if the return type is void. In such
  // cases, a placeholder value for the badly formed instruction is not needed.
  // Hence, if Ty is void, an error instruction is not appended.
  void appendErrorInstruction(Ice::Type Ty) {
    // Note: we don't worry about downstream translation errors because the
    // function will not be translated if any errors occur.
    if (Ty == Ice::IceType_void)
      return;
    Ice::Variable *Var = getNextInstVar(Ty);
    CurrentNode->appendInst(Ice::InstAssign::create(Func.get(), Var, Var));
  }

  Ice::Operand *reportGetOperandUndefined(NaClBcIndexSize_t Index) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Value index " << Index << " not defined!";
    Error(StrBuf.str());
    // Recover and return some value.
    if (!LocalOperands.empty())
      return LocalOperands.front();
    return Context->getGlobalConstantByID(0);
  }

  void verifyCallArgTypeMatches(Ice::FunctionDeclaration *Fcn, Ice::SizeT Index,
                                Ice::Type ArgType, Ice::Type ParamType) {
    if (ArgType != ParamType) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Argument " << (Index + 1) << " of " << printName(Fcn)
             << " expects " << ParamType << ". Found: " << ArgType;
      Error(StrBuf.str());
    }
  }

  const std::string printName(Ice::FunctionDeclaration *Fcn) {
    if (Fcn)
      return Fcn->getName().toString();
    return "function";
  }
};

void FunctionParser::ExitBlock() {
  // Check if the last instruction in the function was terminating.
  if (!InstIsTerminating) {
    Error("Last instruction in function not terminator");
    // Recover by inserting an unreachable instruction.
    CurrentNode->appendInst(Ice::InstUnreachable::create(Func.get()));
  }
  ++CurrentBbIndex;
  if (CurrentBbIndex != DeclaredNumberBbs) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Function declared " << DeclaredNumberBbs
           << " basic blocks, but defined " << CurrentBbIndex << ".";
    Error(StrBuf.str());
  }
  // Before translating, check for blocks without instructions, and insert
  // unreachable. This shouldn't happen, but be safe.
  size_t Index = 0;
  for (Ice::CfgNode *Node : Func->getNodes()) {
    if (Node->getInsts().empty()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Basic block " << Index << " contains no instructions";
      Error(StrBuf.str());
      Node->appendInst(Ice::InstUnreachable::create(Func.get()));
    }
    ++Index;
  }
  Func->computeInOutEdges();
}

void FunctionParser::reportInvalidBinaryOp(Ice::InstArithmetic::OpKind Op,
                                           Ice::Type OpTy) {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Invalid operator type for " << Ice::InstArithmetic::getOpName(Op)
         << ". Found " << OpTy;
  Error(StrBuf.str());
}

void FunctionParser::ProcessRecord() {
  // Note: To better separate parse/IR generation times, when IR generation is
  // disabled we do the following:
  // 1) Delay exiting until after we extract operands.
  // 2) return before we access operands, since all operands will be a nullptr.
  const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
  if (InstIsTerminating) {
    InstIsTerminating = false;
    ++CurrentBbIndex;
    CurrentNode = getBasicBlock(CurrentBbIndex);
  }
  // The base index for relative indexing.
  NaClBcIndexSize_t BaseIndex = getNextInstIndex();
  switch (Record.GetCode()) {
  case naclbitc::FUNC_CODE_DECLAREBLOCKS: {
    // DECLAREBLOCKS: [n]
    if (!isValidRecordSize(1, "count"))
      return;
    if (DeclaredNumberBbs > 0) {
      Error("Duplicate function block count record");
      return;
    }

    // Check for bad large sizes, since they can make ridiculous memory
    // requests and hang the user for large amounts of time.
    uint64_t NumBbs = Values[0];
    if (NumBbs > MaxRecordsInBlock) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Function defines " << NumBbs
             << " basic blocks, which is too big for a function containing "
             << NumBytesDefiningFunction << " bytes";
      Error(StrBuf.str());
      NumBbs = MaxRecordsInBlock;
    }

    if (NumBbs == 0) {
      Error("Functions must contain at least one basic block.");
      NumBbs = 1;
    }

    DeclaredNumberBbs = NumBbs;
    // Install the basic blocks, skipping bb0 which was created in the
    // constructor.
    for (size_t i = 1; i < NumBbs; ++i)
      installNextBasicBlock();
    return;
  }
  case naclbitc::FUNC_CODE_INST_BINOP: {
    // Note: Old bitcode files may have an additional 'flags' operand, which is
    // ignored.

    // BINOP: [opval, opval, opcode, [flags]]

    if (!isValidRecordSizeInRange(3, 4, "binop"))
      return;
    Ice::Operand *Op1 = getRelativeOperand(Values[0], BaseIndex);
    Ice::Operand *Op2 = getRelativeOperand(Values[1], BaseIndex);
    Ice::Type Type1 = Op1->getType();
    Ice::Type Type2 = Op2->getType();
    if (Type1 != Type2) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Binop argument types differ: " << Type1 << " and " << Type2;
      Error(StrBuf.str());
      appendErrorInstruction(Type1);
      return;
    }

    Ice::InstArithmetic::OpKind Opcode;
    if (!convertBinopOpcode(Values[2], Type1, Opcode)) {
      appendErrorInstruction(Type1);
      return;
    }
    CurrentNode->appendInst(Ice::InstArithmetic::create(
        Func.get(), Opcode, getNextInstVar(Type1), Op1, Op2));
    return;
  }
  case naclbitc::FUNC_CODE_INST_CAST: {
    // CAST: [opval, destty, castopc]
    if (!isValidRecordSize(3, "cast"))
      return;
    Ice::Operand *Src = getRelativeOperand(Values[0], BaseIndex);
    Ice::Type CastType = Context->getSimpleTypeByID(Values[1]);
    Ice::InstCast::OpKind CastKind;
    if (!convertCastOpToIceOp(Values[2], Src->getType(), CastType, CastKind)) {
      appendErrorInstruction(CastType);
      return;
    }
    CurrentNode->appendInst(Ice::InstCast::create(
        Func.get(), CastKind, getNextInstVar(CastType), Src));
    return;
  }
  case naclbitc::FUNC_CODE_INST_VSELECT: {
    // VSELECT: [opval, opval, pred]
    if (!isValidRecordSize(3, "select"))
      return;
    Ice::Operand *ThenVal = getRelativeOperand(Values[0], BaseIndex);
    Ice::Operand *ElseVal = getRelativeOperand(Values[1], BaseIndex);
    Ice::Operand *CondVal = getRelativeOperand(Values[2], BaseIndex);
    Ice::Type ThenType = ThenVal->getType();
    Ice::Type ElseType = ElseVal->getType();
    if (ThenType != ElseType) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Select operands not same type. Found " << ThenType << " and "
             << ElseType;
      Error(StrBuf.str());
      appendErrorInstruction(ThenType);
      return;
    }
    Ice::Type CondType = CondVal->getType();
    if (isVectorType(CondType)) {
      if (!isVectorType(ThenType) ||
          typeElementType(CondType) != Ice::IceType_i1 ||
          typeNumElements(ThenType) != typeNumElements(CondType)) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Select condition type " << CondType
               << " not allowed for values of type " << ThenType;
        Error(StrBuf.str());
        appendErrorInstruction(ThenType);
        return;
      }
    } else if (CondVal->getType() != Ice::IceType_i1) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Select condition " << CondVal
             << " not type i1. Found: " << CondVal->getType();
      Error(StrBuf.str());
      appendErrorInstruction(ThenType);
      return;
    }
    CurrentNode->appendInst(Ice::InstSelect::create(
        Func.get(), getNextInstVar(ThenType), CondVal, ThenVal, ElseVal));
    return;
  }
  case naclbitc::FUNC_CODE_INST_EXTRACTELT: {
    // EXTRACTELT: [opval, opval]
    if (!isValidRecordSize(2, "extract element"))
      return;
    Ice::Operand *Vec = getRelativeOperand(Values[0], BaseIndex);
    Ice::Operand *Index = getRelativeOperand(Values[1], BaseIndex);
    Ice::Type VecType = Vec->getType();
    VectorIndexCheckValue IndexCheckValue = validateVectorIndex(Vec, Index);
    if (IndexCheckValue != VectorIndexValid) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      dumpVectorIndexCheckValue(StrBuf, IndexCheckValue);
      StrBuf << ": extractelement " << VecType << " " << *Vec << ", "
             << Index->getType() << " " << *Index;
      Error(StrBuf.str());
      appendErrorInstruction(VecType);
      return;
    }
    CurrentNode->appendInst(Ice::InstExtractElement::create(
        Func.get(), getNextInstVar(typeElementType(VecType)), Vec, Index));
    return;
  }
  case naclbitc::FUNC_CODE_INST_INSERTELT: {
    // INSERTELT: [opval, opval, opval]
    if (!isValidRecordSize(3, "insert element"))
      return;
    Ice::Operand *Vec = getRelativeOperand(Values[0], BaseIndex);
    Ice::Operand *Elt = getRelativeOperand(Values[1], BaseIndex);
    Ice::Operand *Index = getRelativeOperand(Values[2], BaseIndex);
    Ice::Type VecType = Vec->getType();
    VectorIndexCheckValue IndexCheckValue = validateVectorIndex(Vec, Index);
    if (IndexCheckValue != VectorIndexValid) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      dumpVectorIndexCheckValue(StrBuf, IndexCheckValue);
      StrBuf << ": insertelement " << VecType << " " << *Vec << ", "
             << Elt->getType() << " " << *Elt << ", " << Index->getType() << " "
             << *Index;
      Error(StrBuf.str());
      appendErrorInstruction(Elt->getType());
      return;
    }
    if (Ice::typeElementType(VecType) != Elt->getType()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Insertelement: Element type "
             << Ice::typeString(Elt->getType()) << " doesn't match vector type "
             << Ice::typeString(VecType);
      Error(StrBuf.str());
      appendErrorInstruction(Elt->getType());
      return;
    }
    CurrentNode->appendInst(Ice::InstInsertElement::create(
        Func.get(), getNextInstVar(VecType), Vec, Elt, Index));
    return;
  }
  case naclbitc::FUNC_CODE_INST_CMP2: {
    // CMP2: [opval, opval, pred]
    if (!isValidRecordSize(3, "compare"))
      return;
    Ice::Operand *Op1 = getRelativeOperand(Values[0], BaseIndex);
    Ice::Operand *Op2 = getRelativeOperand(Values[1], BaseIndex);
    Ice::Type Op1Type = Op1->getType();
    Ice::Type Op2Type = Op2->getType();
    Ice::Type DestType = getCompareResultType(Op1Type);
    if (Op1Type != Op2Type) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Compare argument types differ: " << Op1Type << " and "
             << Op2Type;
      Error(StrBuf.str());
      appendErrorInstruction(DestType);
      Op2 = Op1;
    }
    if (DestType == Ice::IceType_void) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Compare not defined for type " << Op1Type;
      Error(StrBuf.str());
      return;
    }
    Ice::Variable *Dest = getNextInstVar(DestType);
    if (isIntegerType(Op1Type)) {
      Ice::InstIcmp::ICond Cond;
      if (!convertNaClBitcICmpOpToIce(Values[2], Cond)) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Compare record contains unknown integer predicate index: "
               << Values[2];
        Error(StrBuf.str());
        appendErrorInstruction(DestType);
      }
      CurrentNode->appendInst(
          Ice::InstIcmp::create(Func.get(), Cond, Dest, Op1, Op2));
    } else if (isFloatingType(Op1Type)) {
      Ice::InstFcmp::FCond Cond;
      if (!convertNaClBitcFCompOpToIce(Values[2], Cond)) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Compare record contains unknown float predicate index: "
               << Values[2];
        Error(StrBuf.str());
        appendErrorInstruction(DestType);
      }
      CurrentNode->appendInst(
          Ice::InstFcmp::create(Func.get(), Cond, Dest, Op1, Op2));
    } else {
      // Not sure this can happen, but be safe.
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Compare on type not understood: " << Op1Type;
      Error(StrBuf.str());
      appendErrorInstruction(DestType);
      return;
    }
    return;
  }
  case naclbitc::FUNC_CODE_INST_RET: {
    // RET: [opval?]
    InstIsTerminating = true;
    if (!isValidRecordSizeInRange(0, 1, "return"))
      return;
    if (Values.empty()) {
      CurrentNode->appendInst(Ice::InstRet::create(Func.get()));
    } else {
      Ice::Operand *RetVal = getRelativeOperand(Values[0], BaseIndex);
      CurrentNode->appendInst(Ice::InstRet::create(Func.get(), RetVal));
    }
    return;
  }
  case naclbitc::FUNC_CODE_INST_BR: {
    InstIsTerminating = true;
    if (Values.size() == 1) {
      // BR: [bb#]
      Ice::CfgNode *Block = getBranchBasicBlock(Values[0]);
      if (Block == nullptr)
        return;
      CurrentNode->appendInst(Ice::InstBr::create(Func.get(), Block));
    } else {
      // BR: [bb#, bb#, opval]
      if (!isValidRecordSize(3, "branch"))
        return;
      Ice::Operand *Cond = getRelativeOperand(Values[2], BaseIndex);
      if (Cond->getType() != Ice::IceType_i1) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Branch condition " << *Cond
               << " not i1. Found: " << Cond->getType();
        Error(StrBuf.str());
        return;
      }
      Ice::CfgNode *ThenBlock = getBranchBasicBlock(Values[0]);
      Ice::CfgNode *ElseBlock = getBranchBasicBlock(Values[1]);
      if (ThenBlock == nullptr || ElseBlock == nullptr)
        return;
      CurrentNode->appendInst(
          Ice::InstBr::create(Func.get(), Cond, ThenBlock, ElseBlock));
    }
    return;
  }
  case naclbitc::FUNC_CODE_INST_SWITCH: {
    // SWITCH: [Condty, Cond, BbIndex, NumCases Case ...]
    // where Case = [1, 1, Value, BbIndex].
    //
    // Note: Unlike most instructions, we don't infer the type of Cond, but
    // provide it as a separate field. There are also unnecessary data fields
    // (i.e. constants 1). These were not cleaned up in PNaCl bitcode because
    // the bitcode format was already frozen when the problem was noticed.
    InstIsTerminating = true;
    if (!isValidRecordSizeAtLeast(4, "switch"))
      return;

    Ice::Type CondTy = Context->getSimpleTypeByID(Values[0]);
    if (!Ice::isScalarIntegerType(CondTy)) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Case condition must be non-wide integer. Found: " << CondTy;
      Error(StrBuf.str());
      return;
    }
    Ice::SizeT BitWidth = Ice::getScalarIntBitWidth(CondTy);
    Ice::Operand *Cond = getRelativeOperand(Values[1], BaseIndex);

    if (CondTy != Cond->getType()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Case condition expects type " << CondTy
             << ". Found: " << Cond->getType();
      Error(StrBuf.str());
      return;
    }
    Ice::CfgNode *DefaultLabel = getBranchBasicBlock(Values[2]);
    if (DefaultLabel == nullptr)
      return;
    uint64_t NumCasesRaw = Values[3];
    if (NumCasesRaw > std::numeric_limits<uint32_t>::max()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Too many cases specified in switch: " << NumCasesRaw;
      Error(StrBuf.str());
      NumCasesRaw = std::numeric_limits<uint32_t>::max();
    }
    uint32_t NumCases = NumCasesRaw;

    // Now recognize each of the cases.
    if (!isValidRecordSize(4 + NumCases * 4, "switch"))
      return;
    std::unique_ptr<Ice::InstSwitch> Switch(
        Ice::InstSwitch::create(Func.get(), NumCases, Cond, DefaultLabel));
    unsigned ValCaseIndex = 4; // index to beginning of case entry.
    for (uint32_t CaseIndex = 0; CaseIndex < NumCases;
         ++CaseIndex, ValCaseIndex += 4) {
      if (Values[ValCaseIndex] != 1 || Values[ValCaseIndex + 1] != 1) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Sequence [1, 1, value, label] expected for case entry "
               << "in switch record. (at index" << ValCaseIndex << ")";
        Error(StrBuf.str());
        return;
      }
      BitcodeInt Value(BitWidth,
                       NaClDecodeSignRotatedValue(Values[ValCaseIndex + 2]));
      Ice::CfgNode *Label = getBranchBasicBlock(Values[ValCaseIndex + 3]);
      if (Label == nullptr)
        return;
      Switch->addBranch(CaseIndex, Value.getSExtValue(), Label);
    }
    CurrentNode->appendInst(Switch.release());
    return;
  }
  case naclbitc::FUNC_CODE_INST_UNREACHABLE: {
    // UNREACHABLE: []
    InstIsTerminating = true;
    if (!isValidRecordSize(0, "unreachable"))
      return;
    CurrentNode->appendInst(Ice::InstUnreachable::create(Func.get()));
    return;
  }
  case naclbitc::FUNC_CODE_INST_PHI: {
    // PHI: [ty, val1, bb1, ..., valN, bbN] for n >= 2.
    if (!isValidRecordSizeAtLeast(3, "phi"))
      return;
    Ice::Type Ty = Context->getSimpleTypeByID(Values[0]);
    if ((Values.size() & 0x1) == 0) {
      // Not an odd number of values.
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "function block phi record size not valid: " << Values.size();
      Error(StrBuf.str());
      appendErrorInstruction(Ty);
      return;
    }
    if (Ty == Ice::IceType_void) {
      Error("Phi record using type void not allowed");
      return;
    }
    Ice::Variable *Dest = getNextInstVar(Ty);
    Ice::InstPhi *Phi =
        Ice::InstPhi::create(Func.get(), Values.size() >> 1, Dest);
    for (size_t i = 1; i < Values.size(); i += 2) {
      Ice::Operand *Op =
          getRelativeOperand(NaClDecodeSignRotatedValue(Values[i]), BaseIndex);
      if (Op->getType() != Ty) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Value " << *Op << " not type " << Ty
               << " in phi instruction. Found: " << Op->getType();
        Error(StrBuf.str());
        appendErrorInstruction(Ty);
        return;
      }
      Phi->addArgument(Op, getBasicBlock(Values[i + 1]));
    }
    CurrentNode->appendInst(Phi);
    return;
  }
  case naclbitc::FUNC_CODE_INST_ALLOCA: {
    // ALLOCA: [Size, align]
    if (!isValidRecordSize(2, "alloca"))
      return;
    Ice::Operand *ByteCount = getRelativeOperand(Values[0], BaseIndex);
    uint32_t Alignment = Context->extractAlignment(this, "Alloca", Values[1]);
    Ice::Type PtrTy = Ice::getPointerType();
    if (ByteCount->getType() != Ice::IceType_i32) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Alloca on non-i32 value. Found: " << *ByteCount;
      Error(StrBuf.str());
      appendErrorInstruction(PtrTy);
      return;
    }
    CurrentNode->appendInst(Ice::InstAlloca::create(
        Func.get(), getNextInstVar(PtrTy), ByteCount, Alignment));
    return;
  }
  case naclbitc::FUNC_CODE_INST_LOAD: {
    // LOAD: [address, align, ty]
    if (!isValidRecordSize(3, "load"))
      return;
    Ice::Operand *Address = getRelativeOperand(Values[0], BaseIndex);
    Ice::Type Ty = Context->getSimpleTypeByID(Values[2]);
    uint32_t Alignment = Context->extractAlignment(this, "Load", Values[1]);
    if (!isValidPointerType(Address, "Load")) {
      appendErrorInstruction(Ty);
      return;
    }
    if (!isValidLoadStoreAlignment(Alignment, Ty, "Load")) {
      appendErrorInstruction(Ty);
      return;
    }
    CurrentNode->appendInst(Ice::InstLoad::create(
        Func.get(), getNextInstVar(Ty), Address, Alignment));
    return;
  }
  case naclbitc::FUNC_CODE_INST_STORE: {
    // STORE: [address, value, align]
    if (!isValidRecordSize(3, "store"))
      return;
    Ice::Operand *Address = getRelativeOperand(Values[0], BaseIndex);
    Ice::Operand *Value = getRelativeOperand(Values[1], BaseIndex);
    uint32_t Alignment = Context->extractAlignment(this, "Store", Values[2]);
    if (!isValidPointerType(Address, "Store"))
      return;
    if (!isValidLoadStoreAlignment(Alignment, Value->getType(), "Store"))
      return;
    CurrentNode->appendInst(
        Ice::InstStore::create(Func.get(), Value, Address, Alignment));
    return;
  }
  case naclbitc::FUNC_CODE_INST_CALL:
  case naclbitc::FUNC_CODE_INST_CALL_INDIRECT: {
    // CALL: [cc, fnid, arg0, arg1...]
    // CALL_INDIRECT: [cc, fn, returnty, args...]
    //
    // Note: The difference between CALL and CALL_INDIRECT is that CALL has a
    // reference to an explicit function declaration, while the CALL_INDIRECT
    // is just an address. For CALL, we can infer the return type by looking up
    // the type signature associated with the function declaration. For
    // CALL_INDIRECT we can only infer the type signature via argument types,
    // and the corresponding return type stored in CALL_INDIRECT record.
    Ice::SizeT ParamsStartIndex = 2;
    if (Record.GetCode() == naclbitc::FUNC_CODE_INST_CALL) {
      if (!isValidRecordSizeAtLeast(2, "call"))
        return;
    } else {
      if (!isValidRecordSizeAtLeast(3, "call indirect"))
        return;
      ParamsStartIndex = 3;
    }

    uint32_t CalleeIndex = convertRelativeToAbsIndex(Values[1], BaseIndex);
    Ice::Operand *Callee = getOperand(CalleeIndex);

    // Pull out signature/return type of call (if possible).
    Ice::FunctionDeclaration *Fcn = nullptr;
    const Ice::FuncSigType *Signature = nullptr;
    Ice::Type ReturnType = Ice::IceType_void;
    const Ice::Intrinsics::FullIntrinsicInfo *IntrinsicInfo = nullptr;
    if (Record.GetCode() == naclbitc::FUNC_CODE_INST_CALL) {
      Fcn = Context->getFunctionByID(CalleeIndex);
      Signature = &Fcn->getSignature();
      ReturnType = Signature->getReturnType();
      Ice::SizeT NumParams = Values.size() - ParamsStartIndex;
      if (NumParams != Signature->getNumArgs()) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Call to " << printName(Fcn) << " has " << NumParams
               << " parameters. Signature expects: " << Signature->getNumArgs();
        Error(StrBuf.str());
        if (ReturnType != Ice::IceType_void)
          setNextLocalInstIndex(nullptr);
        return;
      }

      // Check if this direct call is to an Intrinsic (starts with "llvm.")
      IntrinsicInfo = Fcn->getIntrinsicInfo(getTranslator().getContext());
      if (IntrinsicInfo && IntrinsicInfo->getNumArgs() != NumParams) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Call to " << printName(Fcn) << " has " << NumParams
               << " parameters. Intrinsic expects: " << Signature->getNumArgs();
        Error(StrBuf.str());
        if (ReturnType != Ice::IceType_void)
          setNextLocalInstIndex(nullptr);
        return;
      }
    } else { // Record.GetCode() == naclbitc::FUNC_CODE_INST_CALL_INDIRECT
      // There is no signature. Assume defined by parameter types.
      ReturnType = Context->getSimpleTypeByID(Values[2]);
      if (Callee != nullptr)
        isValidPointerType(Callee, "Call indirect");
    }

    if (Callee == nullptr)
      return;

    // Extract out the the call parameters.
    SmallVector<Ice::Operand *, 8> Params;
    for (Ice::SizeT Index = ParamsStartIndex; Index < Values.size(); ++Index) {
      Ice::Operand *Op = getRelativeOperand(Values[Index], BaseIndex);
      if (Op == nullptr) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Parameter " << (Index - ParamsStartIndex + 1) << " of "
               << printName(Fcn) << " is not defined";
        Error(StrBuf.str());
        if (ReturnType != Ice::IceType_void)
          setNextLocalInstIndex(nullptr);
        return;
      }
      Params.push_back(Op);
    }

    // Check return type.
    if (IntrinsicInfo == nullptr && !isCallReturnType(ReturnType)) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Return type of " << printName(Fcn)
             << " is invalid: " << ReturnType;
      Error(StrBuf.str());
      ReturnType = Ice::IceType_i32;
    }

    // Type check call parameters.
    for (Ice::SizeT Index = 0; Index < Params.size(); ++Index) {
      Ice::Operand *Op = Params[Index];
      Ice::Type OpType = Op->getType();
      if (Signature)
        verifyCallArgTypeMatches(Fcn, Index, OpType,
                                 Signature->getArgType(Index));
      else if (!isCallParameterType(OpType)) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Argument " << *Op << " of " << printName(Fcn)
               << " has invalid type: " << Op->getType();
        Error(StrBuf.str());
        appendErrorInstruction(ReturnType);
        return;
      }
    }

    // Extract call information.
    uint64_t CCInfo = Values[0];
    CallingConv::ID CallingConv;
    if (!naclbitc::DecodeCallingConv(CCInfo >> 1, CallingConv)) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Function call calling convention value " << (CCInfo >> 1)
             << " not understood.";
      Error(StrBuf.str());
      appendErrorInstruction(ReturnType);
      return;
    }
    const bool IsTailCall = (CCInfo & 1);

    // Create the call instruction.
    Ice::Variable *Dest = (ReturnType == Ice::IceType_void)
                              ? nullptr
                              : getNextInstVar(ReturnType);
    std::unique_ptr<Ice::InstCall> Instr;
    if (IntrinsicInfo) {
      Instr.reset(Ice::InstIntrinsic::create(Func.get(), Params.size(), Dest,
                                             Callee, IntrinsicInfo->Info));
    } else {
      Instr.reset(Ice::InstCall::create(Func.get(), Params.size(), Dest, Callee,
                                        IsTailCall));
    }
    for (Ice::Operand *Param : Params)
      Instr->addArg(Param);
    CurrentNode->appendInst(Instr.release());
    return;
  }
  case naclbitc::FUNC_CODE_INST_FORWARDTYPEREF: {
    // FORWARDTYPEREF: [opval, ty]
    if (!isValidRecordSize(2, "forward type ref"))
      return;
    Ice::Type OpType = Context->getSimpleTypeByID(Values[1]);
    setOperand(Values[0], createInstVar(OpType));
    return;
  }
  default:
    // Generate error message!
    BlockParserBaseClass::ProcessRecord();
    return;
  }
}

/// Parses constants within a function block.
class ConstantsParser final : public BlockParserBaseClass {
  ConstantsParser() = delete;
  ConstantsParser(const ConstantsParser &) = delete;
  ConstantsParser &operator=(const ConstantsParser &) = delete;

public:
  ConstantsParser(unsigned BlockID, FunctionParser *FuncParser)
      : BlockParserBaseClass(BlockID, FuncParser),
        Timer(Ice::TimerStack::TT_parseConstants, getTranslator().getContext()),
        FuncParser(FuncParser) {}

  ~ConstantsParser() override = default;

  const char *getBlockName() const override { return "constants"; }

private:
  Ice::TimerMarker Timer;
  // The parser of the function block this constants block appears in.
  FunctionParser *FuncParser;
  // The type to use for succeeding constants.
  Ice::Type NextConstantType = Ice::IceType_void;

  void ProcessRecord() override;

  Ice::GlobalContext *getContext() { return getTranslator().getContext(); }

  // Returns true if the type to use for succeeding constants is defined. If
  // false, also generates an error message.
  bool isValidNextConstantType() {
    if (NextConstantType != Ice::IceType_void)
      return true;
    Error("Constant record not preceded by set type record");
    return false;
  }
};

void ConstantsParser::ProcessRecord() {
  const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
  switch (Record.GetCode()) {
  case naclbitc::CST_CODE_SETTYPE: {
    // SETTYPE: [typeid]
    if (!isValidRecordSize(1, "set type"))
      return;
    NextConstantType = Context->getSimpleTypeByID(Values[0]);
    if (NextConstantType == Ice::IceType_void)
      Error("constants block set type not allowed for void type");
    return;
  }
  case naclbitc::CST_CODE_UNDEF: {
    // UNDEF
    if (!isValidRecordSize(0, "undef"))
      return;
    if (!isValidNextConstantType())
      return;
    FuncParser->setNextConstantID(
        getContext()->getConstantUndef(NextConstantType));
    return;
  }
  case naclbitc::CST_CODE_INTEGER: {
    // INTEGER: [intval]
    if (!isValidRecordSize(1, "integer"))
      return;
    if (!isValidNextConstantType())
      return;
    if (Ice::isScalarIntegerType(NextConstantType)) {
      BitcodeInt Value(Ice::getScalarIntBitWidth(NextConstantType),
                       NaClDecodeSignRotatedValue(Values[0]));
      if (Ice::Constant *C = getContext()->getConstantInt(
              NextConstantType, Value.getSExtValue())) {
        FuncParser->setNextConstantID(C);
        return;
      }
    }
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "constant block integer record for non-integer type "
           << NextConstantType;
    Error(StrBuf.str());
    return;
  }
  case naclbitc::CST_CODE_FLOAT: {
    // FLOAT: [fpval]
    if (!isValidRecordSize(1, "float"))
      return;
    if (!isValidNextConstantType())
      return;
    switch (NextConstantType) {
    case Ice::IceType_f32: {
      const BitcodeInt Value(32, static_cast<uint32_t>(Values[0]));
      float FpValue = Value.convertToFp<int32_t, float>();
      FuncParser->setNextConstantID(getContext()->getConstantFloat(FpValue));
      return;
    }
    case Ice::IceType_f64: {
      const BitcodeInt Value(64, Values[0]);
      double FpValue = Value.convertToFp<uint64_t, double>();
      FuncParser->setNextConstantID(getContext()->getConstantDouble(FpValue));
      return;
    }
    default: {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "constant block float record for non-floating type "
             << NextConstantType;
      Error(StrBuf.str());
      return;
    }
    }
  }
  default:
    // Generate error message!
    BlockParserBaseClass::ProcessRecord();
    return;
  }
}

// Parses valuesymtab blocks appearing in a function block.
class FunctionValuesymtabParser final : public ValuesymtabParser {
  FunctionValuesymtabParser() = delete;
  FunctionValuesymtabParser(const FunctionValuesymtabParser &) = delete;
  void operator=(const FunctionValuesymtabParser &) = delete;

public:
  FunctionValuesymtabParser(unsigned BlockID, FunctionParser *EnclosingParser)
      : ValuesymtabParser(BlockID, EnclosingParser),
        Timer(Ice::TimerStack::TT_parseFunctionValuesymtabs,
              getTranslator().getContext()) {}

private:
  Ice::TimerMarker Timer;
  // Returns the enclosing function parser.
  FunctionParser *getFunctionParser() const {
    return reinterpret_cast<FunctionParser *>(GetEnclosingParser());
  }

  const char *getTableKind() const override { return "Function"; }

  void setValueName(NaClBcIndexSize_t Index, StringType &Name) override;
  void setBbName(NaClBcIndexSize_t Index, StringType &Name) override;

  // Reports that the assignment of Name to the value associated with index is
  // not possible, for the given Context.
  void reportUnableToAssign(const char *Context, NaClBcIndexSize_t Index,
                            StringType &Name) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Function-local " << Context << " name '" << Name
           << "' can't be associated with index " << Index;
    Error(StrBuf.str());
  }
};

void FunctionValuesymtabParser::setValueName(NaClBcIndexSize_t Index,
                                             StringType &Name) {
  // Note: We check when Index is too small, so that we can error recover
  // (FP->getOperand will create fatal error).
  if (Index < getFunctionParser()->getNumGlobalIDs()) {
    reportUnableToAssign("Global value", Index, Name);
    return;
  }
  Ice::Operand *Op = getFunctionParser()->getOperand(Index);
  if (auto *V = dyn_cast<Ice::Variable>(Op)) {
    if (Ice::BuildDefs::dump()) {
      std::string Nm(Name.data(), Name.size());
      V->setName(getFunctionParser()->getFunc(), Nm);
    }
  } else {
    reportUnableToAssign("Local value", Index, Name);
  }
}

void FunctionValuesymtabParser::setBbName(NaClBcIndexSize_t Index,
                                          StringType &Name) {
  if (!Ice::BuildDefs::dump())
    return;
  if (Index >= getFunctionParser()->getFunc()->getNumNodes()) {
    reportUnableToAssign("Basic block", Index, Name);
    return;
  }
  std::string Nm(Name.data(), Name.size());
  if (Ice::BuildDefs::dump())
    getFunctionParser()->getFunc()->getNodes()[Index]->setName(Nm);
}

bool FunctionParser::ParseBlock(unsigned BlockID) {
#ifndef PNACL_LLVM
  constexpr bool PNaClAllowLocalSymbolTables = true;
#endif // !PNACL_LLVM
  switch (BlockID) {
  case naclbitc::CONSTANTS_BLOCK_ID: {
    ConstantsParser Parser(BlockID, this);
    return Parser.ParseThisBlock();
  }
  case naclbitc::VALUE_SYMTAB_BLOCK_ID: {
    if (PNaClAllowLocalSymbolTables) {
      FunctionValuesymtabParser Parser(BlockID, this);
      return Parser.ParseThisBlock();
    }
    break;
  }
  default:
    break;
  }
  return BlockParserBaseClass::ParseBlock(BlockID);
}

/// Parses the module block in the bitcode file.
class ModuleParser final : public BlockParserBaseClass {
  ModuleParser() = delete;
  ModuleParser(const ModuleParser &) = delete;
  ModuleParser &operator=(const ModuleParser &) = delete;

public:
  ModuleParser(unsigned BlockID, TopLevelParser *Context)
      : BlockParserBaseClass(BlockID, Context),
        Timer(Ice::TimerStack::TT_parseModule,
              Context->getTranslator().getContext()),
        IsParseParallel(Ice::getFlags().isParseParallel()) {}
  ~ModuleParser() override = default;
  const char *getBlockName() const override { return "module"; }
  NaClBitstreamCursor &getCursor() const { return Record.GetCursor(); }

private:
  Ice::TimerMarker Timer;
  // True if we have already installed names for unnamed global declarations,
  // and have generated global constant initializers.
  bool GlobalDeclarationNamesAndInitializersInstalled = false;
  // True if we have already processed the symbol table for the module.
  bool FoundValuesymtab = false;
  const bool IsParseParallel;

  // Generates names for unnamed global addresses (i.e. functions and global
  // variables). Then lowers global variable declaration initializers to the
  // target. May be called multiple times. Only the first call will do the
  // installation.
  void installGlobalNamesAndGlobalVarInitializers() {
    if (!GlobalDeclarationNamesAndInitializersInstalled) {
      Context->installGlobalNames();
      Context->createValueIDs();
      Context->verifyFunctionTypeSignatures();
      std::unique_ptr<Ice::VariableDeclarationList> Globals =
          Context->getGlobalVariables();
      if (Globals)
        getTranslator().lowerGlobals(std::move(Globals));
      GlobalDeclarationNamesAndInitializersInstalled = true;
    }
  }
  bool ParseBlock(unsigned BlockID) override;

  void ExitBlock() override {
    installGlobalNamesAndGlobalVarInitializers();
    Context->getTranslator().getContext()->waitForWorkerThreads();
  }

  void ProcessRecord() override;
};

class ModuleValuesymtabParser : public ValuesymtabParser {
  ModuleValuesymtabParser() = delete;
  ModuleValuesymtabParser(const ModuleValuesymtabParser &) = delete;
  void operator=(const ModuleValuesymtabParser &) = delete;

public:
  ModuleValuesymtabParser(unsigned BlockID, ModuleParser *MP)
      : ValuesymtabParser(BlockID, MP),
        Timer(Ice::TimerStack::TT_parseModuleValuesymtabs,
              getTranslator().getContext()) {}

  ~ModuleValuesymtabParser() override = default;

private:
  Ice::TimerMarker Timer;
  const char *getTableKind() const override { return "Module"; }
  void setValueName(NaClBcIndexSize_t Index, StringType &Name) override;
  void setBbName(NaClBcIndexSize_t Index, StringType &Name) override;
};

void ModuleValuesymtabParser::setValueName(NaClBcIndexSize_t Index,
                                           StringType &Name) {
  Ice::GlobalDeclaration *Decl = Context->getGlobalDeclarationByID(Index);
  if (llvm::isa<Ice::VariableDeclaration>(Decl) &&
      Decl->isPNaClABIExternalName(Name.str())) {
    // Force linkage of (specific) Global Variables be external for the PNaCl
    // ABI. PNaCl bitcode has a linkage field for Functions, but not for
    // GlobalVariables (because the latter is not needed for pexes, so it has
    // been removed).
    Decl->setLinkage(llvm::GlobalValue::ExternalLinkage);
  }

  // Unconditionally capture the name if it is provided in the input file,
  // regardless of whether dump is enabled or whether the symbol is internal vs
  // external.  This fits in well with the lit tests, and most symbols in a
  // conforming pexe are nameless and don't take this path.
  Decl->setName(getTranslator().getContext(),
                StringRef(Name.data(), Name.size()));
}

void ModuleValuesymtabParser::setBbName(NaClBcIndexSize_t Index,
                                        StringType &Name) {
  reportUnableToAssign("Basic block", Index, Name);
}

class CfgParserWorkItem final : public Ice::OptWorkItem {
  CfgParserWorkItem() = delete;
  CfgParserWorkItem(const CfgParserWorkItem &) = delete;
  CfgParserWorkItem &operator=(const CfgParserWorkItem &) = delete;

public:
  CfgParserWorkItem(unsigned BlockID, NaClBcIndexSize_t FcnId,
                    ModuleParser *ModParser, std::unique_ptr<uint8_t[]> Buffer,
                    uintptr_t BufferSize, uint64_t StartBit, uint32_t SeqNumber)
      : BlockID(BlockID), FcnId(FcnId), ModParser(ModParser),
        Buffer(std::move(Buffer)), BufferSize(BufferSize), StartBit(StartBit),
        SeqNumber(SeqNumber) {}
  std::unique_ptr<Ice::Cfg> getParsedCfg() override;
  ~CfgParserWorkItem() override = default;

private:
  const unsigned BlockID;
  const NaClBcIndexSize_t FcnId;
  // Note: ModParser can't be const because the function parser needs to
  // access non-const member functions (of ModuleParser and TopLevelParser).
  // TODO(kschimpf): Fix this issue.
  ModuleParser *ModParser;
  const std::unique_ptr<uint8_t[]> Buffer;
  const uintptr_t BufferSize;
  const uint64_t StartBit;
  const uint32_t SeqNumber;
};

std::unique_ptr<Ice::Cfg> CfgParserWorkItem::getParsedCfg() {
  NaClBitstreamCursor &OldCursor(ModParser->getCursor());
  llvm::NaClBitstreamReader Reader(OldCursor.getStartWordByteForBit(StartBit),
                                   Buffer.get(), Buffer.get() + BufferSize,
                                   OldCursor.getBitStreamReader());
  NaClBitstreamCursor NewCursor(Reader);
  NewCursor.JumpToBit(NewCursor.getWordBitNo(StartBit));
  FunctionParser Parser(BlockID, ModParser, FcnId, NewCursor);
  return Parser.parseFunction(SeqNumber);
}

bool ModuleParser::ParseBlock(unsigned BlockID) {
  switch (BlockID) {
  case naclbitc::BLOCKINFO_BLOCK_ID:
    return NaClBitcodeParser::ParseBlock(BlockID);
  case naclbitc::TYPE_BLOCK_ID_NEW: {
    TypesParser Parser(BlockID, this);
    return Parser.ParseThisBlock();
  }
  case naclbitc::GLOBALVAR_BLOCK_ID: {
    GlobalsParser Parser(BlockID, this);
    return Parser.ParseThisBlock();
  }
  case naclbitc::VALUE_SYMTAB_BLOCK_ID: {
    if (FoundValuesymtab)
      Fatal("Duplicate valuesymtab in module");

    // If we have already processed a function block (i.e. we have already
    // installed global names and variable initializers) we can no longer accept
    // the value symbol table. Names have already been generated.
    if (GlobalDeclarationNamesAndInitializersInstalled)
      Fatal("Module valuesymtab not allowed after function blocks");

    FoundValuesymtab = true;
    ModuleValuesymtabParser Parser(BlockID, this);
    return Parser.ParseThisBlock();
  }
  case naclbitc::FUNCTION_BLOCK_ID: {
    installGlobalNamesAndGlobalVarInitializers();
    Ice::GlobalContext *Ctx = Context->getTranslator().getContext();
    uint32_t SeqNumber = Context->getTranslator().getNextSequenceNumber();
    NaClBcIndexSize_t FcnId = Context->getNextFunctionBlockValueID();
    if (IsParseParallel) {
      // Skip the block and copy into a buffer. Note: We copy into a buffer
      // using the top-level parser to make sure that the underlying
      // buffer reading from the data streamer is not thread safe.
      NaClBitstreamCursor &Cursor = Record.GetCursor();
      uint64_t StartBit = Cursor.GetCurrentBitNo();
      if (SkipBlock())
        return true;
      const uint64_t EndBit = Cursor.GetCurrentBitNo();
      const uintptr_t StartByte = Cursor.getStartWordByteForBit(StartBit);
      const uintptr_t EndByte = Cursor.getEndWordByteForBit(EndBit);
      const uintptr_t BufferSize = EndByte - StartByte;
      std::unique_ptr<uint8_t[]> Buffer((uint8_t *)(new uint8_t[BufferSize]));
      for (size_t i = Cursor.fillBuffer(Buffer.get(), BufferSize, StartByte);
           i < BufferSize; ++i) {
        Buffer[i] = 0;
      }
      Ctx->optQueueBlockingPush(Ice::makeUnique<CfgParserWorkItem>(
          BlockID, FcnId, this, std::move(Buffer), BufferSize, StartBit,
          SeqNumber));
      return false;
    } else {
      FunctionParser Parser(BlockID, this, FcnId);
      std::unique_ptr<Ice::Cfg> Func = Parser.parseFunction(SeqNumber);
      bool Failed = Func->hasError();
      getTranslator().translateFcn(std::move(Func));
      return Failed && !Ice::getFlags().getAllowErrorRecovery();
    }
  }
  default:
    return BlockParserBaseClass::ParseBlock(BlockID);
  }
}

void ModuleParser::ProcessRecord() {
  const NaClBitcodeRecord::RecordVector &Values = Record.GetValues();
  switch (Record.GetCode()) {
  case naclbitc::MODULE_CODE_VERSION: {
    // VERSION: [version#]
    if (!isValidRecordSize(1, "version"))
      return;
    uint64_t Version = Values[0];
    if (Version != 1) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Unknown bitstream version: " << Version;
      Error(StrBuf.str());
    }
    return;
  }
  case naclbitc::MODULE_CODE_FUNCTION: {
    // FUNCTION:  [type, callingconv, isproto, linkage]
    if (!isValidRecordSize(4, "address"))
      return;
    const Ice::FuncSigType &Signature = Context->getFuncSigTypeByID(Values[0]);
    CallingConv::ID CallingConv;
    if (!naclbitc::DecodeCallingConv(Values[1], CallingConv)) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Function address has unknown calling convention: "
             << Values[1];
      Error(StrBuf.str());
      return;
    }
    GlobalValue::LinkageTypes Linkage;
    if (!naclbitc::DecodeLinkage(Values[3], Linkage)) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Function address has unknown linkage. Found " << Values[3];
      Error(StrBuf.str());
      return;
    }
    bool IsProto = Values[2] == 1;
    auto *Func = Ice::FunctionDeclaration::create(
        Context->getTranslator().getContext(), Signature, CallingConv, Linkage,
        IsProto);
    Context->setNextFunctionID(Func);
    return;
  }
  default:
    BlockParserBaseClass::ProcessRecord();
    return;
  }
}

bool TopLevelParser::ParseBlock(unsigned BlockID) {
  if (BlockID == naclbitc::MODULE_BLOCK_ID) {
    if (ParsedModuleBlock)
      Fatal("Input can't contain more than one module");
    ModuleParser Parser(BlockID, this);
    bool ParseFailed = Parser.ParseThisBlock();
    ParsedModuleBlock = true;
    return ParseFailed;
  }
  // Generate error message by using default block implementation.
  BlockParserBaseClass Parser(BlockID, this);
  return Parser.ParseThisBlock();
}

} // end of anonymous namespace

namespace Ice {

void PNaClTranslator::translateBuffer(const std::string &IRFilename,
                                      MemoryBuffer *MemBuf) {
  std::unique_ptr<MemoryObject> MemObj(getNonStreamedMemoryObject(
      reinterpret_cast<const unsigned char *>(MemBuf->getBufferStart()),
      reinterpret_cast<const unsigned char *>(MemBuf->getBufferEnd())));
  translate(IRFilename, std::move(MemObj));
}

void PNaClTranslator::translate(const std::string &IRFilename,
                                std::unique_ptr<MemoryObject> &&MemObj) {
  // On error, we report_fatal_error to avoid destroying the MemObj. That may
  // still be in use by IceBrowserCompileServer. Otherwise, we need to change
  // the MemObj to be ref-counted, or have a wrapper, or simply leak. We also
  // need a hook to tell the IceBrowserCompileServer to unblock its
  // QueueStreamer.
  // https://code.google.com/p/nativeclient/issues/detail?id=4163
  // Read header and verify it is good.
  NaClBitcodeHeader Header;
  if (Header.Read(MemObj.get())) {
    llvm::report_fatal_error("Invalid PNaCl bitcode header");
  }
  if (!Header.IsSupported()) {
    getContext()->getStrError() << Header.Unsupported();
    if (!Header.IsReadable()) {
      llvm::report_fatal_error("Invalid PNaCl bitcode header");
    }
  }

  // Create a bitstream reader to read the bitcode file.
  NaClBitstreamReader InputStreamFile(MemObj.release(), Header);
  NaClBitstreamCursor InputStream(InputStreamFile);

  TopLevelParser Parser(*this, InputStream, ErrorStatus);
  while (!InputStream.AtEndOfStream()) {
    if (Parser.Parse()) {
      ErrorStatus.assign(EC_Bitcode);
      return;
    }
  }

  if (!Parser.parsedModuleBlock()) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << IRFilename << ": Does not contain a module!";
    llvm::report_fatal_error(StrBuf.str());
  }
  if (InputStreamFile.getBitcodeBytes().getExtent() % 4 != 0) {
    llvm::report_fatal_error("Bitcode stream should be a multiple of 4 bytes");
  }
}

} // end of namespace Ice
