#ifdef GET_ATTR_ENUM
#undef GET_ATTR_ENUM
Alignment,
AllocSize,
AlwaysInline,
ArgMemOnly,
Builtin,
ByVal,
Cold,
Convergent,
Dereferenceable,
DereferenceableOrNull,
InAlloca,
InReg,
InaccessibleMemOnly,
InaccessibleMemOrArgMemOnly,
InlineHint,
JumpTable,
MinSize,
Naked,
Nest,
NoAlias,
NoBuiltin,
NoCapture,
NoDuplicate,
NoImplicitFloat,
NoInline,
NoRecurse,
NoRedZone,
NoReturn,
NoUnwind,
NonLazyBind,
NonNull,
OptimizeForSize,
OptimizeNone,
ReadNone,
ReadOnly,
Returned,
ReturnsTwice,
SExt,
SafeStack,
SanitizeAddress,
SanitizeMemory,
SanitizeThread,
StackAlignment,
StackProtect,
StackProtectReq,
StackProtectStrong,
StructRet,
SwiftError,
SwiftSelf,
UWTable,
WriteOnly,
ZExt,
#endif
#ifdef GET_ATTR_KIND_FROM_NAME
#undef GET_ATTR_KIND_FROM_NAME
static Attribute::AttrKind getAttrKindFromName(StringRef AttrName) {
  return StringSwitch<Attribute::AttrKind>(AttrName)
    .Case("align", Attribute::Alignment)
    .Case("allocsize", Attribute::AllocSize)
    .Case("alwaysinline", Attribute::AlwaysInline)
    .Case("argmemonly", Attribute::ArgMemOnly)
    .Case("builtin", Attribute::Builtin)
    .Case("byval", Attribute::ByVal)
    .Case("cold", Attribute::Cold)
    .Case("convergent", Attribute::Convergent)
    .Case("dereferenceable", Attribute::Dereferenceable)
    .Case("dereferenceable_or_null", Attribute::DereferenceableOrNull)
    .Case("inalloca", Attribute::InAlloca)
    .Case("inreg", Attribute::InReg)
    .Case("inaccessiblememonly", Attribute::InaccessibleMemOnly)
    .Case("inaccessiblemem_or_argmemonly", Attribute::InaccessibleMemOrArgMemOnly)
    .Case("inlinehint", Attribute::InlineHint)
    .Case("jumptable", Attribute::JumpTable)
    .Case("minsize", Attribute::MinSize)
    .Case("naked", Attribute::Naked)
    .Case("nest", Attribute::Nest)
    .Case("noalias", Attribute::NoAlias)
    .Case("nobuiltin", Attribute::NoBuiltin)
    .Case("nocapture", Attribute::NoCapture)
    .Case("noduplicate", Attribute::NoDuplicate)
    .Case("noimplicitfloat", Attribute::NoImplicitFloat)
    .Case("noinline", Attribute::NoInline)
    .Case("norecurse", Attribute::NoRecurse)
    .Case("noredzone", Attribute::NoRedZone)
    .Case("noreturn", Attribute::NoReturn)
    .Case("nounwind", Attribute::NoUnwind)
    .Case("nonlazybind", Attribute::NonLazyBind)
    .Case("nonnull", Attribute::NonNull)
    .Case("optsize", Attribute::OptimizeForSize)
    .Case("optnone", Attribute::OptimizeNone)
    .Case("readnone", Attribute::ReadNone)
    .Case("readonly", Attribute::ReadOnly)
    .Case("returned", Attribute::Returned)
    .Case("returns_twice", Attribute::ReturnsTwice)
    .Case("signext", Attribute::SExt)
    .Case("safestack", Attribute::SafeStack)
    .Case("sanitize_address", Attribute::SanitizeAddress)
    .Case("sanitize_memory", Attribute::SanitizeMemory)
    .Case("sanitize_thread", Attribute::SanitizeThread)
    .Case("alignstack", Attribute::StackAlignment)
    .Case("ssp", Attribute::StackProtect)
    .Case("sspreq", Attribute::StackProtectReq)
    .Case("sspstrong", Attribute::StackProtectStrong)
    .Case("sret", Attribute::StructRet)
    .Case("swifterror", Attribute::SwiftError)
    .Case("swiftself", Attribute::SwiftSelf)
    .Case("uwtable", Attribute::UWTable)
    .Case("writeonly", Attribute::WriteOnly)
    .Case("zeroext", Attribute::ZExt)
    .Default(Attribute::None);
}

#endif
#ifdef GET_ATTR_COMPAT_FUNC
#undef GET_ATTR_COMPAT_FUNC
struct EnumAttr {
  static bool isSet(const Function &Fn,
                    Attribute::AttrKind Kind) {
    return Fn.hasFnAttribute(Kind);
  }

  static void set(Function &Fn,
                  Attribute::AttrKind Kind, bool Val) {
    if (Val)
      Fn.addFnAttr(Kind);
    else
      Fn.removeFnAttr(Kind);
  }
};

struct StrBoolAttr {
  static bool isSet(const Function &Fn,
                    StringRef Kind) {
    auto A = Fn.getFnAttribute(Kind);
    return A.getValueAsString().equals("true");
  }

  static void set(Function &Fn,
                  StringRef Kind, bool Val) {
    Fn.addFnAttr(Kind, Val ? "true" : "false");
  }
};

// EnumAttr classes
struct AlignmentAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Alignment;
  }
};
struct AllocSizeAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::AllocSize;
  }
};
struct AlwaysInlineAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::AlwaysInline;
  }
};
struct ArgMemOnlyAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::ArgMemOnly;
  }
};
struct BuiltinAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Builtin;
  }
};
struct ByValAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::ByVal;
  }
};
struct ColdAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Cold;
  }
};
struct ConvergentAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Convergent;
  }
};
struct DereferenceableAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Dereferenceable;
  }
};
struct DereferenceableOrNullAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::DereferenceableOrNull;
  }
};
struct InAllocaAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::InAlloca;
  }
};
struct InRegAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::InReg;
  }
};
struct InaccessibleMemOnlyAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::InaccessibleMemOnly;
  }
};
struct InaccessibleMemOrArgMemOnlyAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::InaccessibleMemOrArgMemOnly;
  }
};
struct InlineHintAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::InlineHint;
  }
};
struct JumpTableAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::JumpTable;
  }
};
struct MinSizeAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::MinSize;
  }
};
struct NakedAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Naked;
  }
};
struct NestAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Nest;
  }
};
struct NoAliasAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoAlias;
  }
};
struct NoBuiltinAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoBuiltin;
  }
};
struct NoCaptureAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoCapture;
  }
};
struct NoDuplicateAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoDuplicate;
  }
};
struct NoImplicitFloatAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoImplicitFloat;
  }
};
struct NoInlineAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoInline;
  }
};
struct NoRecurseAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoRecurse;
  }
};
struct NoRedZoneAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoRedZone;
  }
};
struct NoReturnAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoReturn;
  }
};
struct NoUnwindAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NoUnwind;
  }
};
struct NonLazyBindAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NonLazyBind;
  }
};
struct NonNullAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::NonNull;
  }
};
struct OptimizeForSizeAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::OptimizeForSize;
  }
};
struct OptimizeNoneAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::OptimizeNone;
  }
};
struct ReadNoneAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::ReadNone;
  }
};
struct ReadOnlyAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::ReadOnly;
  }
};
struct ReturnedAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::Returned;
  }
};
struct ReturnsTwiceAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::ReturnsTwice;
  }
};
struct SExtAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SExt;
  }
};
struct SafeStackAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SafeStack;
  }
};
struct SanitizeAddressAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SanitizeAddress;
  }
};
struct SanitizeMemoryAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SanitizeMemory;
  }
};
struct SanitizeThreadAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SanitizeThread;
  }
};
struct StackAlignmentAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::StackAlignment;
  }
};
struct StackProtectAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::StackProtect;
  }
};
struct StackProtectReqAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::StackProtectReq;
  }
};
struct StackProtectStrongAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::StackProtectStrong;
  }
};
struct StructRetAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::StructRet;
  }
};
struct SwiftErrorAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SwiftError;
  }
};
struct SwiftSelfAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::SwiftSelf;
  }
};
struct UWTableAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::UWTable;
  }
};
struct WriteOnlyAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::WriteOnly;
  }
};
struct ZExtAttr : EnumAttr {
  static enum Attribute::AttrKind getKind() {
    return llvm::Attribute::ZExt;
  }
};

// StrBoolAttr classes
struct LessPreciseFPMADAttr : StrBoolAttr {
  static const char *getKind() {
    return "less-precise-fpmad";
  }
};
struct NoInfsFPMathAttr : StrBoolAttr {
  static const char *getKind() {
    return "no-infs-fp-math";
  }
};
struct NoJumpTablesAttr : StrBoolAttr {
  static const char *getKind() {
    return "no-jump-tables";
  }
};
struct NoNansFPMathAttr : StrBoolAttr {
  static const char *getKind() {
    return "no-nans-fp-math";
  }
};
struct UnsafeFPMathAttr : StrBoolAttr {
  static const char *getKind() {
    return "unsafe-fp-math";
  }
};

static inline bool hasCompatibleFnAttrs(const Function &Caller,
                                        const Function &Callee) {
  bool Ret = true;

  Ret &= isEqual<SanitizeAddressAttr>(Caller, Callee);
  Ret &= isEqual<SanitizeThreadAttr>(Caller, Callee);
  Ret &= isEqual<SanitizeMemoryAttr>(Caller, Callee);
  Ret &= isEqual<SafeStackAttr>(Caller, Callee);

  return Ret;
}

static inline void mergeFnAttrs(Function &Caller,
                                const Function &Callee) {
  adjustCallerSSPLevel(Caller, Callee);
  setAND<LessPreciseFPMADAttr>(Caller, Callee);
  setAND<NoInfsFPMathAttr>(Caller, Callee);
  setAND<NoNansFPMathAttr>(Caller, Callee);
  setAND<UnsafeFPMathAttr>(Caller, Callee);
  setOR<NoImplicitFloatAttr>(Caller, Callee);
  setOR<NoJumpTablesAttr>(Caller, Callee);
}

#endif
