//===- subzero/src/IceGlobalInits.h - Global declarations -------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the representation of function declarations, global variable
/// declarations, and the corresponding variable initializers in Subzero.
///
/// Global variable initializers are represented as a sequence of simple
/// initializers.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEGLOBALINITS_H
#define SUBZERO_SRC_ICEGLOBALINITS_H

#include "IceDefs.h"
#include "IceFixups.h"
#include "IceGlobalContext.h"
#include "IceIntrinsics.h"
#include "IceMangling.h"
#include "IceOperand.h"
#include "IceTypes.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/Bitcode/NaCl/NaClBitcodeParser.h" // for NaClBitcodeRecord.
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/GlobalValue.h" // for GlobalValue::LinkageTypes.

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <memory>
#include <utility>

// TODO(kschimpf): Remove ourselves from using LLVM representation for calling
// conventions and linkage types.

namespace Ice {

/// Base class for global variable and function declarations.
class GlobalDeclaration {
  GlobalDeclaration() = delete;
  GlobalDeclaration(const GlobalDeclaration &) = delete;
  GlobalDeclaration &operator=(const GlobalDeclaration &) = delete;

public:
  /// Discriminator for LLVM-style RTTI.
  enum GlobalDeclarationKind {
    FunctionDeclarationKind,
    VariableDeclarationKind
  };
  GlobalDeclarationKind getKind() const { return Kind; }
  GlobalString getName() const { return Name; }
  void setName(GlobalContext *Ctx, const std::string &NewName) {
    Name = Ctx->getGlobalString(getSuppressMangling() ? NewName
                                                      : mangleName(NewName));
  }
  void setName(GlobalString NewName) { Name = NewName; }
  void setName(GlobalContext *Ctx) {
    Name = GlobalString::createWithoutString(Ctx);
  }
  bool hasName() const { return Name.hasStdString(); }
  bool isInternal() const {
    return Linkage == llvm::GlobalValue::InternalLinkage;
  }
  llvm::GlobalValue::LinkageTypes getLinkage() const { return Linkage; }
  void setLinkage(llvm::GlobalValue::LinkageTypes L) {
    assert(!hasName());
    Linkage = L;
  }
  bool isExternal() const {
    return Linkage == llvm::GlobalValue::ExternalLinkage;
  }
  virtual ~GlobalDeclaration() = default;

  /// Prints out type of the global declaration.
  virtual void dumpType(Ostream &Stream) const = 0;

  /// Prints out the global declaration.
  virtual void dump(Ostream &Stream) const = 0;

  /// Returns true if when emitting names, we should suppress mangling.
  virtual bool getSuppressMangling() const = 0;

  /// Returns textual name of linkage.
  const char *getLinkageName() const {
    return isInternal() ? "internal" : "external";
  }

protected:
  GlobalDeclaration(GlobalDeclarationKind Kind,
                    llvm::GlobalValue::LinkageTypes Linkage)
      : Kind(Kind), Linkage(Linkage) {}

  /// Returns true if linkage is defined correctly for the global declaration,
  /// based on default rules.
  bool verifyLinkageDefault() const {
    switch (Linkage) {
    default:
      return false;
    case llvm::GlobalValue::InternalLinkage:
      return true;
    case llvm::GlobalValue::ExternalLinkage:
      return getFlags().getAllowExternDefinedSymbols();
    }
  }

  const GlobalDeclarationKind Kind;
  llvm::GlobalValue::LinkageTypes Linkage;
  GlobalString Name;
};

/// Models a function declaration. This includes the type signature of the
/// function, its calling conventions, and its linkage.
class FunctionDeclaration : public GlobalDeclaration {
  FunctionDeclaration() = delete;
  FunctionDeclaration(const FunctionDeclaration &) = delete;
  FunctionDeclaration &operator=(const FunctionDeclaration &) = delete;

public:
  static FunctionDeclaration *create(GlobalContext *Context,
                                     const FuncSigType &Signature,
                                     llvm::CallingConv::ID CallingConv,
                                     llvm::GlobalValue::LinkageTypes Linkage,
                                     bool IsProto) {
    return new (Context->allocate<FunctionDeclaration>())
        FunctionDeclaration(Signature, CallingConv, Linkage, IsProto);
  }
  const FuncSigType &getSignature() const { return Signature; }
  llvm::CallingConv::ID getCallingConv() const { return CallingConv; }
  /// isProto implies that there isn't a (local) definition for the function.
  bool isProto() const { return IsProto; }
  static bool classof(const GlobalDeclaration *Addr) {
    return Addr->getKind() == FunctionDeclarationKind;
  }
  void dumpType(Ostream &Stream) const final;
  void dump(Ostream &Stream) const final;
  bool getSuppressMangling() const final { return isExternal() && IsProto; }

  /// Returns true if linkage is correct for the function declaration.
  bool verifyLinkageCorrect(const GlobalContext *Ctx) const {
    return verifyLinkageDefault();
  }

  /// Validates that the type signature of the function is correct. Returns true
  /// if valid.
  bool validateTypeSignature() const;

  /// Generates an error message describing why validateTypeSignature returns
  /// false.
  std::string getTypeSignatureError(const GlobalContext *Ctx);

private:
  const Ice::FuncSigType Signature;
  llvm::CallingConv::ID CallingConv;
  const bool IsProto;

  FunctionDeclaration(const FuncSigType &Signature,
                      llvm::CallingConv::ID CallingConv,
                      llvm::GlobalValue::LinkageTypes Linkage, bool IsProto)
      : GlobalDeclaration(FunctionDeclarationKind, Linkage),
        Signature(Signature), CallingConv(CallingConv), IsProto(IsProto) {}
};

/// Models a global variable declaration, and its initializers.
class VariableDeclaration : public GlobalDeclaration {
  VariableDeclaration(const VariableDeclaration &) = delete;
  VariableDeclaration &operator=(const VariableDeclaration &) = delete;

public:
  /// Base class for a global variable initializer.
  class Initializer {
    Initializer(const Initializer &) = delete;
    Initializer &operator=(const Initializer &) = delete;

  public:
    /// Discriminator for LLVM-style RTTI.
    enum InitializerKind {
      DataInitializerKind,
      ZeroInitializerKind,
      RelocInitializerKind
    };
    InitializerKind getKind() const { return Kind; }
    virtual SizeT getNumBytes() const = 0;
    virtual void dump(Ostream &Stream) const = 0;
    virtual void dumpType(Ostream &Stream) const;

  protected:
    explicit Initializer(InitializerKind Kind) : Kind(Kind) {}

  private:
    const InitializerKind Kind;
  };
  static_assert(std::is_trivially_destructible<Initializer>::value,
                "Initializer must be trivially destructible.");

  /// Models the data in a data initializer.
  using DataVecType = char *;

  /// Defines a sequence of byte values as a data initializer.
  class DataInitializer : public Initializer {
    DataInitializer(const DataInitializer &) = delete;
    DataInitializer &operator=(const DataInitializer &) = delete;

  public:
    template <class... Args>
    static DataInitializer *create(VariableDeclarationList *VDL,
                                   Args &&... TheArgs) {
      return new (VDL->allocate_initializer<DataInitializer>())
          DataInitializer(VDL, std::forward<Args>(TheArgs)...);
    }

    const llvm::StringRef getContents() const {
      return llvm::StringRef(Contents, ContentsSize);
    }
    SizeT getNumBytes() const final { return ContentsSize; }
    void dump(Ostream &Stream) const final;
    static bool classof(const Initializer *D) {
      return D->getKind() == DataInitializerKind;
    }

  private:
    DataInitializer(VariableDeclarationList *VDL,
                    const llvm::NaClBitcodeRecord::RecordVector &Values)
        : Initializer(DataInitializerKind), ContentsSize(Values.size()),
          // ugh, we should actually do new char[], but this may involve
          // implementation-specific details. Given that Contents is arena
          // allocated, and never delete[]d, just use char --
          // AllocOwner->allocate_array will allocate a buffer with the right
          // size.
          Contents(new (VDL->allocate_initializer<char>(ContentsSize)) char) {
      for (SizeT I = 0; I < Values.size(); ++I)
        Contents[I] = static_cast<int8_t>(Values[I]);
    }

    DataInitializer(VariableDeclarationList *VDL, const char *Str,
                    size_t StrLen)
        : Initializer(DataInitializerKind), ContentsSize(StrLen),
          Contents(new (VDL->allocate_initializer<char>(ContentsSize)) char) {
      for (size_t i = 0; i < StrLen; ++i)
        Contents[i] = Str[i];
    }

    /// The byte contents of the data initializer.
    const SizeT ContentsSize;
    DataVecType Contents;
  };
  static_assert(std::is_trivially_destructible<DataInitializer>::value,
                "DataInitializer must be trivially destructible.");

  /// Defines a sequence of bytes initialized to zero.
  class ZeroInitializer : public Initializer {
    ZeroInitializer(const ZeroInitializer &) = delete;
    ZeroInitializer &operator=(const ZeroInitializer &) = delete;

  public:
    static ZeroInitializer *create(VariableDeclarationList *VDL, SizeT Size) {
      return new (VDL->allocate_initializer<ZeroInitializer>())
          ZeroInitializer(Size);
    }
    SizeT getNumBytes() const final { return Size; }
    void dump(Ostream &Stream) const final;
    static bool classof(const Initializer *Z) {
      return Z->getKind() == ZeroInitializerKind;
    }

  private:
    explicit ZeroInitializer(SizeT Size)
        : Initializer(ZeroInitializerKind), Size(Size) {}

    /// The number of bytes to be zero initialized.
    SizeT Size;
  };
  static_assert(std::is_trivially_destructible<ZeroInitializer>::value,
                "ZeroInitializer must be trivially destructible.");

  /// Defines the relocation value of another global declaration.
  class RelocInitializer : public Initializer {
    RelocInitializer(const RelocInitializer &) = delete;
    RelocInitializer &operator=(const RelocInitializer &) = delete;

  public:
    static RelocInitializer *create(VariableDeclarationList *VDL,
                                    const GlobalDeclaration *Declaration,
                                    const RelocOffsetArray &OffsetExpr) {
      constexpr bool NoFixup = false;
      return new (VDL->allocate_initializer<RelocInitializer>())
          RelocInitializer(VDL, Declaration, OffsetExpr, NoFixup);
    }

    static RelocInitializer *create(VariableDeclarationList *VDL,
                                    const GlobalDeclaration *Declaration,
                                    const RelocOffsetArray &OffsetExpr,
                                    FixupKind Fixup) {
      constexpr bool HasFixup = true;
      return new (VDL->allocate_initializer<RelocInitializer>())
          RelocInitializer(VDL, Declaration, OffsetExpr, HasFixup, Fixup);
    }

    RelocOffsetT getOffset() const {
      RelocOffsetT Offset = 0;
      for (SizeT i = 0; i < OffsetExprSize; ++i) {
        Offset += OffsetExpr[i]->getOffset();
      }
      return Offset;
    }

    bool hasFixup() const { return HasFixup; }
    FixupKind getFixup() const {
      assert(HasFixup);
      return Fixup;
    }

    const GlobalDeclaration *getDeclaration() const { return Declaration; }
    SizeT getNumBytes() const final { return RelocAddrSize; }
    void dump(Ostream &Stream) const final;
    void dumpType(Ostream &Stream) const final;
    static bool classof(const Initializer *R) {
      return R->getKind() == RelocInitializerKind;
    }

  private:
    RelocInitializer(VariableDeclarationList *VDL,
                     const GlobalDeclaration *Declaration,
                     const RelocOffsetArray &OffsetExpr, bool HasFixup,
                     FixupKind Fixup = 0)
        : Initializer(RelocInitializerKind),
          Declaration(Declaration), // The global declaration used in the reloc.
          OffsetExprSize(OffsetExpr.size()),
          OffsetExpr(new (VDL->allocate_initializer<RelocOffset *>(
              OffsetExprSize)) RelocOffset *),
          HasFixup(HasFixup), Fixup(Fixup) {
      for (SizeT i = 0; i < OffsetExprSize; ++i) {
        this->OffsetExpr[i] = OffsetExpr[i];
      }
    }

    const GlobalDeclaration *Declaration;
    /// The offset to add to the relocation.
    const SizeT OffsetExprSize;
    RelocOffset **OffsetExpr;
    const bool HasFixup = false;
    const FixupKind Fixup = 0;
  };
  static_assert(std::is_trivially_destructible<RelocInitializer>::value,
                "RelocInitializer must be trivially destructible.");

  /// Models the list of initializers.
  // TODO(jpp): missing allocator.
  using InitializerListType = std::vector<Initializer *>;

  static VariableDeclaration *create(VariableDeclarationList *VDL,
                                     bool SuppressMangling = false,
                                     llvm::GlobalValue::LinkageTypes Linkage =
                                         llvm::GlobalValue::InternalLinkage) {
    return new (VDL->allocate_variable_declaration<VariableDeclaration>())
        VariableDeclaration(Linkage, SuppressMangling);
  }

  static VariableDeclaration *createExternal(VariableDeclarationList *VDL) {
    constexpr bool SuppressMangling = true;
    constexpr llvm::GlobalValue::LinkageTypes Linkage =
        llvm::GlobalValue::ExternalLinkage;
    return create(VDL, SuppressMangling, Linkage);
  }

  const InitializerListType &getInitializers() const { return Initializers; }
  bool getIsConstant() const { return IsConstant; }
  void setIsConstant(bool NewValue) { IsConstant = NewValue; }
  uint32_t getAlignment() const { return Alignment; }
  void setAlignment(uint32_t NewAlignment) { Alignment = NewAlignment; }
  bool hasInitializer() const { return HasInitializer; }
  bool hasNonzeroInitializer() const {
    return !(Initializers.size() == 1 &&
             llvm::isa<ZeroInitializer>(Initializers[0]));
  }

  /// Returns the number of bytes for the initializer of the global address.
  SizeT getNumBytes() const {
    SizeT Count = 0;
    for (const auto *Init : Initializers) {
      Count += Init->getNumBytes();
    }
    return Count;
  }

  /// Adds Initializer to the list of initializers. Takes ownership of the
  /// initializer.
  void addInitializer(Initializer *Initializer) {
    const bool OldSuppressMangling = getSuppressMangling();
    Initializers.emplace_back(Initializer);
    HasInitializer = true;
    // The getSuppressMangling() logic depends on whether the global variable
    // has initializers.  If its value changed as a result of adding an
    // initializer, then make sure we haven't previously set the name based on
    // faulty SuppressMangling logic.
    const bool SameMangling = (OldSuppressMangling == getSuppressMangling());
    (void)SameMangling;
    assert(Name.hasStdString() || SameMangling);
  }

  /// Prints out type for initializer associated with the declaration to Stream.
  void dumpType(Ostream &Stream) const final;

  /// Prints out the definition of the global variable declaration (including
  /// initialization).
  virtual void dump(Ostream &Stream) const override;

  /// Returns true if linkage is correct for the variable declaration.
  bool verifyLinkageCorrect() const { return verifyLinkageDefault(); }

  static bool classof(const GlobalDeclaration *Addr) {
    return Addr->getKind() == VariableDeclarationKind;
  }

  bool getSuppressMangling() const final {
    if (ForceSuppressMangling)
      return true;
    return isExternal() && !hasInitializer();
  }

  void discardInitializers() { Initializers.clear(); }

private:
  /// List of initializers for the declared variable.
  InitializerListType Initializers;
  bool HasInitializer = false;
  /// The alignment of the declared variable.
  uint32_t Alignment = 0;
  /// True if a declared (global) constant.
  bool IsConstant = false;
  /// If set to true, force getSuppressMangling() to return true.
  const bool ForceSuppressMangling;

  VariableDeclaration(llvm::GlobalValue::LinkageTypes Linkage,
                      bool SuppressMangling)
      : GlobalDeclaration(VariableDeclarationKind, Linkage),
        ForceSuppressMangling(SuppressMangling) {}
};

template <class StreamType>
inline StreamType &operator<<(StreamType &Stream,
                              const VariableDeclaration::Initializer &Init) {
  Init.dump(Stream);
  return Stream;
}

template <class StreamType>
inline StreamType &operator<<(StreamType &Stream,
                              const GlobalDeclaration &Addr) {
  Addr.dump(Stream);
  return Stream;
}

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEGLOBALINITS_H
