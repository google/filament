//===--- CGDebugInfo.cpp - Emit Debug Information for a Module ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This coordinates the debug information generation while generating code.
//
//===----------------------------------------------------------------------===//

#include "CGDebugInfo.h"
#include "CGBlocks.h"
#include "CGCXXABI.h"
#include "CGObjCRuntime.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "dxc/Support/Path.h" // HLSL Change
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Version.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
using namespace clang;
using namespace clang::CodeGen;

CGDebugInfo::CGDebugInfo(CodeGenModule &CGM)
    : CGM(CGM), DebugKind(CGM.getCodeGenOpts().getDebugInfo()),
      DBuilder(CGM.getModule()) {
  CreateCompileUnit();
}

CGDebugInfo::~CGDebugInfo() {
  assert(LexicalBlockStack.empty() &&
         "Region stack mismatch, stack not empty!");
}

ApplyDebugLocation::ApplyDebugLocation(CodeGenFunction &CGF,
                                       SourceLocation TemporaryLocation)
    : CGF(CGF) {
  init(TemporaryLocation);
}

ApplyDebugLocation::ApplyDebugLocation(CodeGenFunction &CGF,
                                       bool DefaultToEmpty,
                                       SourceLocation TemporaryLocation)
    : CGF(CGF) {
  init(TemporaryLocation, DefaultToEmpty);
}

void ApplyDebugLocation::init(SourceLocation TemporaryLocation,
                              bool DefaultToEmpty) {
  if (auto *DI = CGF.getDebugInfo()) {
    OriginalLocation = CGF.Builder.getCurrentDebugLocation();
    if (TemporaryLocation.isInvalid()) {
      if (DefaultToEmpty)
        CGF.Builder.SetCurrentDebugLocation(llvm::DebugLoc());
      else {
        // Construct a location that has a valid scope, but no line info.
        assert(!DI->LexicalBlockStack.empty());
        CGF.Builder.SetCurrentDebugLocation(
            llvm::DebugLoc::get(0, 0, DI->LexicalBlockStack.back()));
      }
    } else
      DI->EmitLocation(CGF.Builder, TemporaryLocation);
  }
}

ApplyDebugLocation::ApplyDebugLocation(CodeGenFunction &CGF, const Expr *E)
    : CGF(CGF) {
  init(E->getExprLoc());
}

ApplyDebugLocation::ApplyDebugLocation(CodeGenFunction &CGF, llvm::DebugLoc Loc)
    : CGF(CGF) {
  if (CGF.getDebugInfo()) {
    OriginalLocation = CGF.Builder.getCurrentDebugLocation();
    if (Loc)
      CGF.Builder.SetCurrentDebugLocation(std::move(Loc));
  }
}

ApplyDebugLocation::~ApplyDebugLocation() {
  // Query CGF so the location isn't overwritten when location updates are
  // temporarily disabled (for C++ default function arguments)
  if (CGF.getDebugInfo())
    CGF.Builder.SetCurrentDebugLocation(std::move(OriginalLocation));
}

void CGDebugInfo::setLocation(SourceLocation Loc) {
  // If the new location isn't valid return.
  if (Loc.isInvalid())
    return;

  CurLoc = CGM.getContext().getSourceManager().getExpansionLoc(Loc);

  // If we've changed files in the middle of a lexical scope go ahead
  // and create a new lexical scope with file node if it's different
  // from the one in the scope.
  if (LexicalBlockStack.empty())
    return;

  SourceManager &SM = CGM.getContext().getSourceManager();
  auto *Scope = cast<llvm::DIScope>(LexicalBlockStack.back());
  PresumedLoc PCLoc = SM.getPresumedLoc(CurLoc, /*UseLineDirectives*/ false); // HLSL Change

  if (PCLoc.isInvalid() || Scope->getFilename() == PCLoc.getFilename())
    return;

  if (auto *LBF = dyn_cast<llvm::DILexicalBlockFile>(Scope)) {
    LexicalBlockStack.pop_back();
    LexicalBlockStack.emplace_back(DBuilder.createLexicalBlockFile(
        LBF->getScope(), getOrCreateFile(CurLoc)));
  } else if (isa<llvm::DILexicalBlock>(Scope) ||
             isa<llvm::DISubprogram>(Scope)) {
    LexicalBlockStack.pop_back();
    LexicalBlockStack.emplace_back(
        DBuilder.createLexicalBlockFile(Scope, getOrCreateFile(CurLoc)));
  }
}

llvm::DIScope *CGDebugInfo::getContextDescriptor(const Decl *Context) {
  if (!Context)
    return TheCU;

  auto I = RegionMap.find(Context);
  if (I != RegionMap.end()) {
    llvm::Metadata *V = I->second;
    return dyn_cast_or_null<llvm::DIScope>(V);
  }

  // Check namespace.
  if (const NamespaceDecl *NSDecl = dyn_cast<NamespaceDecl>(Context))
    return getOrCreateNameSpace(NSDecl);

  if (const RecordDecl *RDecl = dyn_cast<RecordDecl>(Context))
    if (!RDecl->isDependentType())
      return getOrCreateType(CGM.getContext().getTypeDeclType(RDecl),
                             getOrCreateMainFile());
  return TheCU;
}

StringRef CGDebugInfo::getFunctionName(const FunctionDecl *FD) {
  assert(FD && "Invalid FunctionDecl!");
  IdentifierInfo *FII = FD->getIdentifier();
  FunctionTemplateSpecializationInfo *Info =
      FD->getTemplateSpecializationInfo();
  if (!Info && FII)
    return FII->getName();

  // Otherwise construct human readable name for debug info.
  SmallString<128> NS;
  llvm::raw_svector_ostream OS(NS);
  FD->printName(OS);

  // Add any template specialization args.
  if (Info) {
    const TemplateArgumentList *TArgs = Info->TemplateArguments;
    const TemplateArgument *Args = TArgs->data();
    unsigned NumArgs = TArgs->size();
    PrintingPolicy Policy(CGM.getLangOpts());
    TemplateSpecializationType::PrintTemplateArgumentList(OS, Args, NumArgs,
                                                          Policy);
  }

  // Copy this name on the side and use its reference.
  return internString(OS.str());
}

#if 0 // HLSL Change - no ObjC support
StringRef CGDebugInfo::getObjCMethodName(const ObjCMethodDecl *OMD) {
  SmallString<256> MethodName;
  llvm::raw_svector_ostream OS(MethodName);
  OS << (OMD->isInstanceMethod() ? '-' : '+') << '[';
  const DeclContext *DC = OMD->getDeclContext();
  if (const ObjCImplementationDecl *OID =
          dyn_cast<const ObjCImplementationDecl>(DC)) {
    OS << OID->getName();
  } else if (const ObjCInterfaceDecl *OID =
                 dyn_cast<const ObjCInterfaceDecl>(DC)) {
    OS << OID->getName();
  } else if (const ObjCCategoryImplDecl *OCD =
                 dyn_cast<const ObjCCategoryImplDecl>(DC)) {
    OS << ((const NamedDecl *)OCD)->getIdentifier()->getNameStart() << '('
       << OCD->getIdentifier()->getNameStart() << ')';
  } else if (isa<ObjCProtocolDecl>(DC)) {
    // We can extract the type of the class from the self pointer.
    if (ImplicitParamDecl *SelfDecl = OMD->getSelfDecl()) {
      QualType ClassTy =
          cast<ObjCObjectPointerType>(SelfDecl->getType())->getPointeeType();
      ClassTy.print(OS, PrintingPolicy(LangOptions()));
    }
  }
  OS << ' ' << OMD->getSelector().getAsString() << ']';

  return internString(OS.str());
}
#endif // HLSL Change - no ObjC support

StringRef CGDebugInfo::getSelectorName(Selector S) {
  return internString(S.getAsString());
}

StringRef CGDebugInfo::getClassName(const RecordDecl *RD) {
  // quick optimization to avoid having to intern strings that are already
  // stored reliably elsewhere
  if (!isa<ClassTemplateSpecializationDecl>(RD))
    return RD->getName();

  SmallString<128> Name;
  {
    llvm::raw_svector_ostream OS(Name);
    RD->getNameForDiagnostic(OS, CGM.getContext().getPrintingPolicy(),
                             /*Qualified*/ false);
  }

  // Copy this name on the side and use its reference.
  return internString(Name);
}

llvm::DIFile *CGDebugInfo::getOrCreateFile(SourceLocation Loc) {
  if (!Loc.isValid())
    // If Location is not valid then use main input file.
    return DBuilder.createFile(TheCU->getFilename(), TheCU->getDirectory());

  SourceManager &SM = CGM.getContext().getSourceManager();
  PresumedLoc PLoc = SM.getPresumedLoc(Loc, /*UseLineDirectives*/ false); // HLSL Change

  if (PLoc.isInvalid() || StringRef(PLoc.getFilename()).empty())
    // If the location is not valid then use main input file.
    return DBuilder.createFile(TheCU->getFilename(), TheCU->getDirectory());

  // Cache the results.
  const char *fname = PLoc.getFilename();
  auto it = DIFileCache.find(fname);

  if (it != DIFileCache.end()) {
    // Verify that the information still exists.
    if (llvm::Metadata *V = it->second)
      return cast<llvm::DIFile>(V);
  }

  llvm::DIFile *F =
      DBuilder.createFile(PLoc.getFilename(), getCurrentDirname());

  DIFileCache[fname].reset(F);
  return F;
}

llvm::DIFile *CGDebugInfo::getOrCreateMainFile() {
  return DBuilder.createFile(TheCU->getFilename(), TheCU->getDirectory());
}

unsigned CGDebugInfo::getLineNumber(SourceLocation Loc) {
  if (Loc.isInvalid() && CurLoc.isInvalid())
    return 0;
  SourceManager &SM = CGM.getContext().getSourceManager();
  PresumedLoc PLoc = SM.getPresumedLoc(Loc.isValid() ? Loc : CurLoc, /*UseLineDirectives*/ false); // HLSL Change
  return PLoc.isValid() ? PLoc.getLine() : 0;
}

unsigned CGDebugInfo::getColumnNumber(SourceLocation Loc, bool Force) {
  // We may not want column information at all.
  if (!Force && !CGM.getCodeGenOpts().DebugColumnInfo)
    return 0;

  // If the location is invalid then use the current column.
  if (Loc.isInvalid() && CurLoc.isInvalid())
    return 0;
  SourceManager &SM = CGM.getContext().getSourceManager();
  PresumedLoc PLoc = SM.getPresumedLoc(Loc.isValid() ? Loc : CurLoc, /*UseLineDirectives*/ false); // HLSL Change
  return PLoc.isValid() ? PLoc.getColumn() : 0;
}

StringRef CGDebugInfo::getCurrentDirname() {
  if (!CGM.getCodeGenOpts().DebugCompilationDir.empty()
      || CGM.getLangOpts().HLSL) // HLSL Change
    return CGM.getCodeGenOpts().DebugCompilationDir;

  if (!CWDName.empty())
    return CWDName;
  SmallString<256> CWD;
  llvm::sys::fs::current_path(CWD);
  return CWDName = internString(CWD);
}

void CGDebugInfo::CreateCompileUnit() {

  // Should we be asking the SourceManager for the main file name, instead of
  // accepting it as an argument? This just causes the main file name to
  // mismatch with source locations and create extra lexical scopes or
  // mismatched debug info (a CU with a DW_AT_file of "-", because that's what
  // the driver passed, but functions/other things have DW_AT_file of "<stdin>"
  // because that's what the SourceManager says)

  // Get absolute path name.
  // SourceManager &SM = CGM.getContext().getSourceManager(); // HLSL Change - unused
  std::string MainFileName = CGM.getCodeGenOpts().MainFileName;
  if (MainFileName.empty())
    MainFileName = "<stdin>";

  // The main file name provided via the "-main-file-name" option contains just
  // the file name itself with no path information. This file name may have had
  // a relative path, so we look into the actual file entry for the main
  // file to determine the real absolute path for the file.

#if 0  // HLSL change. The directory is already part of the name, this would duplicate it
  std::string MainFileDir;
  if (const FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID())) {
    MainFileDir = MainFile->getDir()->getName();
    if (MainFileDir != ".") {
      llvm::SmallString<1024> MainFileDirSS(MainFileDir);
      llvm::sys::path::append(MainFileDirSS, MainFileName);
      MainFileName = MainFileDirSS.str();
    }
  }
#endif  // HLSL change. The full path is already here

  // Save filename string.
  StringRef Filename = internString(MainFileName);

  // Save split dwarf file string.
  std::string SplitDwarfFile = CGM.getCodeGenOpts().SplitDwarfFile;
  StringRef SplitDwarfFilename = internString(SplitDwarfFile);

  llvm::dwarf::SourceLanguage LangTag;
  const LangOptions &LO = CGM.getLangOpts();
  if (LO.CPlusPlus) {
    if (LO.ObjC1)
      LangTag = llvm::dwarf::DW_LANG_ObjC_plus_plus;
    else
      LangTag = llvm::dwarf::DW_LANG_C_plus_plus;
  } else if (LO.ObjC1) {
    LangTag = llvm::dwarf::DW_LANG_ObjC;
  } else if (LO.C99) {
    LangTag = llvm::dwarf::DW_LANG_C99;
  } else {
    LangTag = llvm::dwarf::DW_LANG_C89;
  }

  std::string Producer = getClangFullVersion();

  // Figure out which version of the ObjC runtime we have.
  unsigned RuntimeVers = 0;
  if (LO.ObjC1)
    RuntimeVers = LO.ObjCRuntime.isNonFragile() ? 2 : 1;

  // Create new compile unit.
  // FIXME - Eliminate TheCU.
  TheCU = DBuilder.createCompileUnit(
      LangTag, Filename, getCurrentDirname(), Producer, LO.Optimize,
      CGM.getCodeGenOpts().DwarfDebugFlags, RuntimeVers, SplitDwarfFilename,
      DebugKind <= CodeGenOptions::DebugLineTablesOnly
          ? llvm::DIBuilder::LineTablesOnly
          : llvm::DIBuilder::FullDebug,
      0 /* DWOid */,
      DebugKind != CodeGenOptions::LocTrackingOnly);
}

llvm::DIType *CGDebugInfo::CreateType(const BuiltinType *BT) {
  llvm::dwarf::TypeKind Encoding;
  StringRef BTName;
  switch (BT->getKind()) {
#define BUILTIN_TYPE(Id, SingletonId)
#define PLACEHOLDER_TYPE(Id, SingletonId) case BuiltinType::Id:
#include "clang/AST/BuiltinTypes.def"
  case BuiltinType::Dependent:
    llvm_unreachable("Unexpected builtin type");
  case BuiltinType::NullPtr:
    return DBuilder.createNullPtrType();
  case BuiltinType::Void:
    return nullptr;
#if 0 // HLSL Change - no ObjC or OpenCL support
  case BuiltinType::ObjCClass:
    if (!ClassTy)
      ClassTy = DBuilder.createForwardDecl(llvm::dwarf::DW_TAG_structure_type,
                                           "objc_class", TheCU,
                                           getOrCreateMainFile(), 0);
    return ClassTy;
  case BuiltinType::ObjCId: {
    // typedef struct objc_class *Class;
    // typedef struct objc_object {
    //  Class isa;
    // } *id;

    if (ObjTy)
      return ObjTy;

    if (!ClassTy)
      ClassTy = DBuilder.createForwardDecl(llvm::dwarf::DW_TAG_structure_type,
                                           "objc_class", TheCU,
                                           getOrCreateMainFile(), 0);

    unsigned Size = CGM.getContext().getTypeSize(CGM.getContext().VoidPtrTy);

    auto *ISATy = DBuilder.createPointerType(ClassTy, Size);

    ObjTy =
        DBuilder.createStructType(TheCU, "objc_object", getOrCreateMainFile(),
                                  0, 0, 0, 0, nullptr, llvm::DINodeArray());

    DBuilder.replaceArrays(
        ObjTy,
        DBuilder.getOrCreateArray(&*DBuilder.createMemberType(
            ObjTy, "isa", getOrCreateMainFile(), 0, Size, 0, 0, 0, ISATy)));
    return ObjTy;
  }
  case BuiltinType::ObjCSel: {
    if (!SelTy)
      SelTy = DBuilder.createForwardDecl(llvm::dwarf::DW_TAG_structure_type,
                                         "objc_selector", TheCU,
                                         getOrCreateMainFile(), 0);
    return SelTy;
  }

  case BuiltinType::OCLImage1d:
    return getOrCreateStructPtrType("opencl_image1d_t", OCLImage1dDITy);
  case BuiltinType::OCLImage1dArray:
    return getOrCreateStructPtrType("opencl_image1d_array_t",
                                    OCLImage1dArrayDITy);
  case BuiltinType::OCLImage1dBuffer:
    return getOrCreateStructPtrType("opencl_image1d_buffer_t",
                                    OCLImage1dBufferDITy);
  case BuiltinType::OCLImage2d:
    return getOrCreateStructPtrType("opencl_image2d_t", OCLImage2dDITy);
  case BuiltinType::OCLImage2dArray:
    return getOrCreateStructPtrType("opencl_image2d_array_t",
                                    OCLImage2dArrayDITy);
  case BuiltinType::OCLImage3d:
    return getOrCreateStructPtrType("opencl_image3d_t", OCLImage3dDITy);
  case BuiltinType::OCLSampler:
    return DBuilder.createBasicType(
        "opencl_sampler_t", CGM.getContext().getTypeSize(BT),
        CGM.getContext().getTypeAlign(BT), llvm::dwarf::DW_ATE_unsigned);
  case BuiltinType::OCLEvent:
    return getOrCreateStructPtrType("opencl_event_t", OCLEventDITy);
#else
  case BuiltinType::ObjCClass:
  case BuiltinType::ObjCId:
  case BuiltinType::ObjCSel:
  case BuiltinType::OCLImage1d:
  case BuiltinType::OCLImage1dArray:
  case BuiltinType::OCLImage1dBuffer:
  case BuiltinType::OCLImage2d:
  case BuiltinType::OCLImage2dArray:
  case BuiltinType::OCLImage3d:
  case BuiltinType::OCLSampler:
  case BuiltinType::OCLEvent:
    llvm_unreachable("No ObjC or OpenCL support");
#endif // HLSL Change - no ObjC or OpenCL support

  case BuiltinType::UChar:
  case BuiltinType::Char_U:
    Encoding = llvm::dwarf::DW_ATE_unsigned_char;
    break;
  case BuiltinType::Char_S:
  case BuiltinType::SChar:
    Encoding = llvm::dwarf::DW_ATE_signed_char;
    break;
  case BuiltinType::Char16:
  case BuiltinType::Char32:
    Encoding = llvm::dwarf::DW_ATE_UTF;
    break;
  case BuiltinType::UShort:
  case BuiltinType::Min16UInt:      // HLSL Change
  case BuiltinType::Int8_4Packed:   // HLSL Change
  case BuiltinType::UInt8_4Packed:  // HLSL Change
  case BuiltinType::UInt:
  case BuiltinType::UInt128:
  case BuiltinType::ULong:
  case BuiltinType::WChar_U:
  case BuiltinType::ULongLong:
    Encoding = llvm::dwarf::DW_ATE_unsigned;
    break;
  // HLSL Changes begin
  case BuiltinType::Min12Int:
  case BuiltinType::Min16Int:
  // HLSL Changed end
  case BuiltinType::Short:
  case BuiltinType::Int:
  case BuiltinType::Int128:
  case BuiltinType::Long:
  case BuiltinType::WChar_S:
  case BuiltinType::LongLong:
  case BuiltinType::LitInt:
    Encoding = llvm::dwarf::DW_ATE_signed;
    break;
  case BuiltinType::Bool:
    Encoding = llvm::dwarf::DW_ATE_boolean;
    break;
  // HLSL Changes begin
  case BuiltinType::Min10Float:
  case BuiltinType::Min16Float:
  case BuiltinType::HalfFloat:
  // HLSL Changes end
  case BuiltinType::Half:
  case BuiltinType::Float:
  case BuiltinType::LongDouble:
  case BuiltinType::Double:
  case BuiltinType::LitFloat:
    Encoding = llvm::dwarf::DW_ATE_float;
    break;
  }

  switch (BT->getKind()) {
  case BuiltinType::Long:
    BTName = "long int";
    break;
  case BuiltinType::LongLong:
    // BTName = "long long int"; // HLSL Change
    BTName = "int64_t"; // HLSL Change
    break;
  case BuiltinType::ULong:
    BTName = "long unsigned int";
    break;
  case BuiltinType::ULongLong:
    // BTName = "long long unsigned int"; // HLSL Change
    BTName = "uint64_t"; // HLSL Change
    break;
  default:
    BTName = BT->getName(CGM.getLangOpts());
    break;
  }
  // Bit size, align and offset of the type.
  uint64_t Size = CGM.getContext().getTypeSize(BT);
  uint64_t Align = CGM.getContext().getTypeAlign(BT);
  return DBuilder.createBasicType(BTName, Size, Align, Encoding);
}

llvm::DIType *CGDebugInfo::CreateType(const ComplexType *Ty) {
  // Bit size, align and offset of the type.
  llvm::dwarf::TypeKind Encoding = llvm::dwarf::DW_ATE_complex_float;
  if (Ty->isComplexIntegerType())
    Encoding = llvm::dwarf::DW_ATE_lo_user;

  uint64_t Size = CGM.getContext().getTypeSize(Ty);
  uint64_t Align = CGM.getContext().getTypeAlign(Ty);
  return DBuilder.createBasicType("complex", Size, Align, Encoding);
}

llvm::DIType *CGDebugInfo::CreateQualifiedType(QualType Ty,
                                               llvm::DIFile *Unit) {
  QualifierCollector Qc;
  const Type *T = Qc.strip(Ty);

  // Ignore these qualifiers for now.
  Qc.removeObjCGCAttr();
  Qc.removeAddressSpace();
  Qc.removeObjCLifetime();

  // We will create one Derived type for one qualifier and recurse to handle any
  // additional ones.
  llvm::dwarf::Tag Tag;
  if (Qc.hasConst()) {
    Tag = llvm::dwarf::DW_TAG_const_type;
    Qc.removeConst();
  } else if (Qc.hasVolatile()) {
    Tag = llvm::dwarf::DW_TAG_volatile_type;
    Qc.removeVolatile();
  } else if (Qc.hasRestrict()) {
    Tag = llvm::dwarf::DW_TAG_restrict_type;
    Qc.removeRestrict();
  } else {
    assert(Qc.empty() && "Unknown type qualifier for debug info");
    return getOrCreateType(QualType(T, 0), Unit);
  }

  auto *FromTy = getOrCreateType(Qc.apply(CGM.getContext(), T), Unit);

  // No need to fill in the Name, Line, Size, Alignment, Offset in case of
  // CVR derived types.
  return DBuilder.createQualifiedType(Tag, FromTy);
}

#if 0 // HLSL Change - no ObjC support
llvm::DIType *CGDebugInfo::CreateType(const ObjCObjectPointerType *Ty,
                                      llvm::DIFile *Unit) {

  // The frontend treats 'id' as a typedef to an ObjCObjectType,
  // whereas 'id<protocol>' is treated as an ObjCPointerType. For the
  // debug info, we want to emit 'id' in both cases.
  if (Ty->isObjCQualifiedIdType())
    return getOrCreateType(CGM.getContext().getObjCIdType(), Unit);

  return CreatePointerLikeType(llvm::dwarf::DW_TAG_pointer_type, Ty,
                               Ty->getPointeeType(), Unit);
}
#endif // HLSL Change - no ObjC support

llvm::DIType *CGDebugInfo::CreateType(const PointerType *Ty,
                                      llvm::DIFile *Unit) {
  return CreatePointerLikeType(llvm::dwarf::DW_TAG_pointer_type, Ty,
                               Ty->getPointeeType(), Unit);
}

/// \return whether a C++ mangling exists for the type defined by TD.
static bool hasCXXMangling(const TagDecl *TD, llvm::DICompileUnit *TheCU) {
  switch (TheCU->getSourceLanguage()) {
  case llvm::dwarf::DW_LANG_C_plus_plus:
    return true;
  case llvm::dwarf::DW_LANG_ObjC_plus_plus:
    return isa<CXXRecordDecl>(TD) || isa<EnumDecl>(TD);
  default:
    return false;
  }
}

/// In C++ mode, types have linkage, so we can rely on the ODR and
/// on their mangled names, if they're external.
static SmallString<256> getUniqueTagTypeName(const TagType *Ty,
                                             CodeGenModule &CGM,
                                             llvm::DICompileUnit *TheCU) {
  SmallString<256> FullName;
  const TagDecl *TD = Ty->getDecl();

  if (!hasCXXMangling(TD, TheCU) || !TD->isExternallyVisible())
    return FullName;

  // Microsoft Mangler does not have support for mangleCXXRTTIName yet.
  if (CGM.getTarget().getCXXABI().isMicrosoft())
    return FullName;

  // TODO: This is using the RTTI name. Is there a better way to get
  // a unique string for a type?
  llvm::raw_svector_ostream Out(FullName);
  CGM.getCXXABI().getMangleContext().mangleCXXRTTIName(QualType(Ty, 0), Out);
  Out.flush();
  return FullName;
}

/// \return the approproate DWARF tag for a composite type.
static llvm::dwarf::Tag getTagForRecord(const RecordDecl *RD) {
   llvm::dwarf::Tag Tag;
  if (RD->isStruct() || RD->isInterface())
    Tag = llvm::dwarf::DW_TAG_structure_type;
  else if (RD->isUnion())
    Tag = llvm::dwarf::DW_TAG_union_type;
  else {
    // FIXME: This could be a struct type giving a default visibility different
    // than C++ class type, but needs llvm metadata changes first.
    assert(RD->isClass());
    Tag = llvm::dwarf::DW_TAG_class_type;
  }
  return Tag;
}

llvm::DICompositeType *
CGDebugInfo::getOrCreateRecordFwdDecl(const RecordType *Ty,
                                      llvm::DIScope *Ctx) {
  const RecordDecl *RD = Ty->getDecl();
  if (llvm::DIType *T = getTypeOrNull(CGM.getContext().getRecordType(RD)))
    return cast<llvm::DICompositeType>(T);
  llvm::DIFile *DefUnit = getOrCreateFile(RD->getLocation());
  unsigned Line = getLineNumber(RD->getLocation());
  StringRef RDName = getClassName(RD);

  uint64_t Size = 0;
  uint64_t Align = 0;

  const RecordDecl *D = RD->getDefinition();
  if (D && D->isCompleteDefinition()) {
    Size = CGM.getContext().getTypeSize(Ty);
    Align = CGM.getContext().getTypeAlign(Ty);
  }

  // Create the type.
  SmallString<256> FullName = getUniqueTagTypeName(Ty, CGM, TheCU);
  llvm::DICompositeType *RetTy = DBuilder.createReplaceableCompositeType(
      getTagForRecord(RD), RDName, Ctx, DefUnit, Line, 0, Size, Align,
      llvm::DINode::FlagFwdDecl, FullName);
  ReplaceMap.emplace_back(
      std::piecewise_construct, std::make_tuple(Ty),
      std::make_tuple(static_cast<llvm::Metadata *>(RetTy)));
  return RetTy;
}

llvm::DIType *CGDebugInfo::CreatePointerLikeType(llvm::dwarf::Tag Tag,
                                                 const Type *Ty,
                                                 QualType PointeeTy,
                                                 llvm::DIFile *Unit) {
  if (Tag == llvm::dwarf::DW_TAG_reference_type ||
      Tag == llvm::dwarf::DW_TAG_rvalue_reference_type)
    return DBuilder.createReferenceType(Tag, getOrCreateType(PointeeTy, Unit));

  // Bit size, align and offset of the type.
  // Size is always the size of a pointer. We can't use getTypeSize here
  // because that does not return the correct value for references.
  unsigned AS = CGM.getContext().getTargetAddressSpace(PointeeTy);
  uint64_t Size = CGM.getTarget().getPointerWidth(AS);
  uint64_t Align = CGM.getContext().getTypeAlign(Ty);

  return DBuilder.createPointerType(getOrCreateType(PointeeTy, Unit), Size,
                                    Align);
}

llvm::DIType *CGDebugInfo::getOrCreateStructPtrType(StringRef Name,
                                                    llvm::DIType *&Cache) {
  if (Cache)
    return Cache;
  Cache = DBuilder.createForwardDecl(llvm::dwarf::DW_TAG_structure_type, Name,
                                     TheCU, getOrCreateMainFile(), 0);
  unsigned Size = CGM.getContext().getTypeSize(CGM.getContext().VoidPtrTy);
  Cache = DBuilder.createPointerType(Cache, Size);
  return Cache;
}

#if 0 // HLSL Change - no block support
llvm::DIType *CGDebugInfo::CreateType(const BlockPointerType *Ty,
                                      llvm::DIFile *Unit) {
  SmallVector<llvm::Metadata *, 8> EltTys;
  QualType FType;
  uint64_t FieldSize, FieldOffset;
  unsigned FieldAlign;
  llvm::DINodeArray Elements;

  FieldOffset = 0;
  FType = CGM.getContext().UnsignedLongTy;
  EltTys.push_back(CreateMemberType(Unit, FType, "reserved", &FieldOffset));
  EltTys.push_back(CreateMemberType(Unit, FType, "Size", &FieldOffset));

  Elements = DBuilder.getOrCreateArray(EltTys);
  EltTys.clear();

  unsigned Flags = llvm::DINode::FlagAppleBlock;
  unsigned LineNo = 0;

  auto *EltTy =
      DBuilder.createStructType(Unit, "__block_descriptor", nullptr, LineNo,
                                FieldOffset, 0, Flags, nullptr, Elements);

  // Bit size, align and offset of the type.
  uint64_t Size = CGM.getContext().getTypeSize(Ty);

  auto *DescTy = DBuilder.createPointerType(EltTy, Size);

  FieldOffset = 0;
  FType = CGM.getContext().getPointerType(CGM.getContext().VoidTy);
  EltTys.push_back(CreateMemberType(Unit, FType, "__isa", &FieldOffset));
  FType = CGM.getContext().IntTy;
  EltTys.push_back(CreateMemberType(Unit, FType, "__flags", &FieldOffset));
  EltTys.push_back(CreateMemberType(Unit, FType, "__reserved", &FieldOffset));
  FType = CGM.getContext().getPointerType(Ty->getPointeeType());
  EltTys.push_back(CreateMemberType(Unit, FType, "__FuncPtr", &FieldOffset));

  FType = CGM.getContext().getPointerType(CGM.getContext().VoidTy);
  FieldSize = CGM.getContext().getTypeSize(Ty);
  FieldAlign = CGM.getContext().getTypeAlign(Ty);
  EltTys.push_back(DBuilder.createMemberType(Unit, "__descriptor", nullptr, LineNo,
                                             FieldSize, FieldAlign, FieldOffset,
                                             0, DescTy));

  FieldOffset += FieldSize;
  Elements = DBuilder.getOrCreateArray(EltTys);

  // The __block_literal_generic structs are marked with a special
  // DW_AT_APPLE_BLOCK attribute and are an implementation detail only
  // the debugger needs to know about. To allow type uniquing, emit
  // them without a name or a location.
  EltTy =
      DBuilder.createStructType(Unit, "", nullptr, LineNo,
                                FieldOffset, 0, Flags, nullptr, Elements);

  return DBuilder.createPointerType(EltTy, Size);
}
#endif // HLSL Change - no block support

llvm::DIType *CGDebugInfo::CreateType(const TemplateSpecializationType *Ty,
                                      llvm::DIFile *Unit) {
  assert(Ty->isTypeAlias());
  llvm::DIType *Src = getOrCreateType(Ty->getAliasedType(), Unit);

  SmallString<128> NS;
  llvm::raw_svector_ostream OS(NS);
  Ty->getTemplateName().print(OS, CGM.getContext().getPrintingPolicy(),
                              /*qualified*/ false);

  TemplateSpecializationType::PrintTemplateArgumentList(
      OS, Ty->getArgs(), Ty->getNumArgs(),
      CGM.getContext().getPrintingPolicy());

  TypeAliasDecl *AliasDecl = cast<TypeAliasTemplateDecl>(
      Ty->getTemplateName().getAsTemplateDecl())->getTemplatedDecl();

  SourceLocation Loc = AliasDecl->getLocation();
  return DBuilder.createTypedef(
      Src, internString(OS.str()), getOrCreateFile(Loc), getLineNumber(Loc),
      getContextDescriptor(cast<Decl>(AliasDecl->getDeclContext())));
}

llvm::DIType *CGDebugInfo::CreateType(const TypedefType *Ty,
                                      llvm::DIFile *Unit) {
  // We don't set size information, but do specify where the typedef was
  // declared.
  SourceLocation Loc = Ty->getDecl()->getLocation();

  // Typedefs are derived from some other type.
  return DBuilder.createTypedef(
      getOrCreateType(Ty->getDecl()->getUnderlyingType(), Unit),
      Ty->getDecl()->getName(), getOrCreateFile(Loc), getLineNumber(Loc),
      getContextDescriptor(cast<Decl>(Ty->getDecl()->getDeclContext())));
}

llvm::DIType *CGDebugInfo::CreateType(const FunctionType *Ty,
                                      llvm::DIFile *Unit) {
  SmallVector<llvm::Metadata *, 16> EltTys;

  // Add the result type at least.
  EltTys.push_back(getOrCreateType(Ty->getReturnType(), Unit));

  // Set up remainder of arguments if there is a prototype.
  // otherwise emit it as a variadic function.
  if (isa<FunctionNoProtoType>(Ty))
    EltTys.push_back(DBuilder.createUnspecifiedParameter());
  else if (const FunctionProtoType *FPT = dyn_cast<FunctionProtoType>(Ty)) {
    for (unsigned i = 0, e = FPT->getNumParams(); i != e; ++i)
      EltTys.push_back(getOrCreateType(FPT->getParamType(i), Unit));
    if (FPT->isVariadic())
      EltTys.push_back(DBuilder.createUnspecifiedParameter());
  }

  llvm::DITypeRefArray EltTypeArray = DBuilder.getOrCreateTypeArray(EltTys);
  return DBuilder.createSubroutineType(Unit, EltTypeArray);
}

/// Convert an AccessSpecifier into the corresponding DINode flag.
/// As an optimization, return 0 if the access specifier equals the
/// default for the containing type.
static unsigned getAccessFlag(AccessSpecifier Access, const RecordDecl *RD) {
  AccessSpecifier Default = clang::AS_none;
  if (RD && RD->isClass())
    Default = clang::AS_private;
  else if (RD && (RD->isStruct() || RD->isUnion()))
    Default = clang::AS_public;

  if (Access == Default)
    return 0;

  switch (Access) {
  case clang::AS_private:
    return llvm::DINode::FlagPrivate;
  case clang::AS_protected:
    return llvm::DINode::FlagProtected;
  case clang::AS_public:
    return llvm::DINode::FlagPublic;
  case clang::AS_none:
    return 0;
  }
  llvm_unreachable("unexpected access enumerator");
}

llvm::DIType *CGDebugInfo::createFieldType(
    StringRef name, QualType type, uint64_t sizeInBitsOverride,
    SourceLocation loc, AccessSpecifier AS, uint64_t offsetInBits,
    llvm::DIFile *tunit, llvm::DIScope *scope, const RecordDecl *RD) {
  llvm::DIType *debugType = getOrCreateType(type, tunit);

  // Get the location for the field.
  llvm::DIFile *file = getOrCreateFile(loc);
  unsigned line = getLineNumber(loc);

  uint64_t SizeInBits = 0;
  unsigned AlignInBits = 0;
  if (!type->isIncompleteArrayType()) {
    TypeInfo TI = CGM.getContext().getTypeInfo(type);
    SizeInBits = TI.Width;
    AlignInBits = TI.Align;

    if (sizeInBitsOverride)
      SizeInBits = sizeInBitsOverride;
  }

  unsigned flags = getAccessFlag(AS, RD);
  return DBuilder.createMemberType(scope, name, file, line, SizeInBits,
                                   AlignInBits, offsetInBits, flags, debugType);
}

void CGDebugInfo::CollectRecordLambdaFields(
    const CXXRecordDecl *CXXDecl, SmallVectorImpl<llvm::Metadata *> &elements,
    llvm::DIType *RecordTy) {
  // For C++11 Lambdas a Field will be the same as a Capture, but the Capture
  // has the name and the location of the variable so we should iterate over
  // both concurrently.
  const ASTRecordLayout &layout = CGM.getContext().getASTRecordLayout(CXXDecl);
  RecordDecl::field_iterator Field = CXXDecl->field_begin();
  unsigned fieldno = 0;
  for (CXXRecordDecl::capture_const_iterator I = CXXDecl->captures_begin(),
                                             E = CXXDecl->captures_end();
       I != E; ++I, ++Field, ++fieldno) {
    const LambdaCapture &C = *I;
    if (C.capturesVariable()) {
      VarDecl *V = C.getCapturedVar();
      llvm::DIFile *VUnit = getOrCreateFile(C.getLocation());
      StringRef VName = V->getName();
      uint64_t SizeInBitsOverride = 0;
      if (Field->isBitField()) {
        SizeInBitsOverride = Field->getBitWidthValue(CGM.getContext());
        assert(SizeInBitsOverride && "found named 0-width bitfield");
      }
      llvm::DIType *fieldType = createFieldType(
          VName, Field->getType(), SizeInBitsOverride, C.getLocation(),
          Field->getAccess(), layout.getFieldOffset(fieldno), VUnit, RecordTy,
          CXXDecl);
      elements.push_back(fieldType);
    } else if (C.capturesThis()) {
      // TODO: Need to handle 'this' in some way by probably renaming the
      // this of the lambda class and having a field member of 'this' or
      // by using AT_object_pointer for the function and having that be
      // used as 'this' for semantic references.
      FieldDecl *f = *Field;
      llvm::DIFile *VUnit = getOrCreateFile(f->getLocation());
      QualType type = f->getType();
      llvm::DIType *fieldType = createFieldType(
          "this", type, 0, f->getLocation(), f->getAccess(),
          layout.getFieldOffset(fieldno), VUnit, RecordTy, CXXDecl);

      elements.push_back(fieldType);
    }
  }
}

llvm::DIDerivedType *
CGDebugInfo::CreateRecordStaticField(const VarDecl *Var, llvm::DIType *RecordTy,
                                     const RecordDecl *RD) {
  // Create the descriptor for the static variable, with or without
  // constant initializers.
  Var = Var->getCanonicalDecl();
  llvm::DIFile *VUnit = getOrCreateFile(Var->getLocation());
  llvm::DIType *VTy = getOrCreateType(Var->getType(), VUnit);

  unsigned LineNumber = getLineNumber(Var->getLocation());
  StringRef VName = Var->getName();
  llvm::Constant *C = nullptr;
  if (Var->getInit()) {
    const APValue *Value = Var->evaluateValue();
    if (Value) {
      if (Value->isInt())
        C = llvm::ConstantInt::get(CGM.getLLVMContext(), Value->getInt());
      if (Value->isFloat())
        C = llvm::ConstantFP::get(CGM.getLLVMContext(), Value->getFloat());
    }
  }

  unsigned Flags = getAccessFlag(Var->getAccess(), RD);
  llvm::DIDerivedType *GV = DBuilder.createStaticMemberType(
      RecordTy, VName, VUnit, LineNumber, VTy, Flags, C);
  StaticDataMemberCache[Var->getCanonicalDecl()].reset(GV);
  return GV;
}

void CGDebugInfo::CollectRecordNormalField(
    const FieldDecl *field, uint64_t OffsetInBits, llvm::DIFile *tunit,
    SmallVectorImpl<llvm::Metadata *> &elements, llvm::DIType *RecordTy,
    const RecordDecl *RD) {
  StringRef name = field->getName();
  QualType type = field->getType();

  // Ignore unnamed fields unless they're anonymous structs/unions.
  if (name.empty() && !type->isRecordType())
    return;

  uint64_t SizeInBitsOverride = 0;
  if (field->isBitField()) {
    SizeInBitsOverride = field->getBitWidthValue(CGM.getContext());
    assert(SizeInBitsOverride && "found named 0-width bitfield");
  }

  llvm::DIType *fieldType =
      createFieldType(name, type, SizeInBitsOverride, field->getLocation(),
                      field->getAccess(), OffsetInBits, tunit, RecordTy, RD);

  elements.push_back(fieldType);
}

void CGDebugInfo::CollectRecordFields(
    const RecordDecl *record, llvm::DIFile *tunit,
    SmallVectorImpl<llvm::Metadata *> &elements,
    llvm::DICompositeType *RecordTy) {
  const CXXRecordDecl *CXXDecl = dyn_cast<CXXRecordDecl>(record);

  if (CXXDecl && CXXDecl->isLambda())
    CollectRecordLambdaFields(CXXDecl, elements, RecordTy);
  else {
    const ASTRecordLayout &layout = CGM.getContext().getASTRecordLayout(record);

    // Field number for non-static fields.
    unsigned fieldNo = 0;

    // Static and non-static members should appear in the same order as
    // the corresponding declarations in the source program.
    for (const auto *I : record->decls())
      if (const auto *V = dyn_cast<VarDecl>(I)) {
        // Reuse the existing static member declaration if one exists
        auto MI = StaticDataMemberCache.find(V->getCanonicalDecl());
        if (MI != StaticDataMemberCache.end()) {
          assert(MI->second &&
                 "Static data member declaration should still exist");
          elements.push_back(cast<llvm::DIDerivedTypeBase>(MI->second));
        } else {
          auto Field = CreateRecordStaticField(V, RecordTy, record);
          elements.push_back(Field);
        }
      } else if (const auto *field = dyn_cast<FieldDecl>(I)) {
        CollectRecordNormalField(field, layout.getFieldOffset(fieldNo), tunit,
                                 elements, RecordTy, record);

        // Bump field number for next field.
        ++fieldNo;
      }
  }
}

// HLSL Change Begins
// Hook to allow us to lie about the contents of some HLSL types in the debug info,
// by exposing clean members rather than our implementation detail internals.
// Note that the debug size of types is not based on the fields reported here,
// but rather on ASTContext::getTypeSize, so they should be consistent.
bool CGDebugInfo::TryCollectHLSLRecordElements(const RecordType *Ty,
    llvm::DICompositeType *DITy,
    SmallVectorImpl<llvm::Metadata *> &Elements) {
  QualType QualTy(Ty, 0);
  if (hlsl::IsHLSLVecType(QualTy)) {
    // The HLSL vector type is defined as containing a field 'h' of
    // extended vector type, which is represented as an array in DWARF.
    // However, we logically represent it as one field per component.
    QualType ElemQualTy = hlsl::GetHLSLVecElementType(QualTy);
    unsigned AlignBits = CGM.getContext().getTypeAlign(ElemQualTy);
    unsigned VecSize = hlsl::GetHLSLVecSize(QualTy);
    unsigned ElemSizeInBits = CGM.getContext().getTypeSize(ElemQualTy);
    unsigned CurrentAlignedOffset = 0;
    for (unsigned ElemIdx = 0; ElemIdx < VecSize; ++ElemIdx) {
      StringRef FieldName = StringRef(&"xyzw"[ElemIdx], 1);
      CurrentAlignedOffset =
          llvm::RoundUpToAlignment(CurrentAlignedOffset, AlignBits);
      llvm::DIType *FieldType =
          createFieldType(FieldName, ElemQualTy, 0, SourceLocation(),
                          AccessSpecifier::AS_public, CurrentAlignedOffset,
                          /* tunit */ nullptr, DITy, Ty->getDecl());
      CurrentAlignedOffset += ElemSizeInBits;
      Elements.emplace_back(FieldType);
    }

    return true;
  }
  else if (hlsl::IsHLSLMatType(QualTy)) {
    // The HLSL matrix type is defined as containing a field 'h' of
    // array of extended vector type, but logically we want to represent
    // it as per-element fields.
    QualType ElemQualTy = hlsl::GetHLSLMatElementType(QualTy);
    uint32_t NumRows, NumCols;
    hlsl::GetHLSLMatRowColCount(QualTy, NumRows, NumCols);
    unsigned ElemSizeInBits = CGM.getContext().getTypeSize(ElemQualTy);
    for (unsigned RowIdx = 0; RowIdx < NumRows; ++RowIdx) {
      for (unsigned ColIdx = 0; ColIdx < NumCols; ++ColIdx) {
        char FieldName[] = "_11";
        FieldName[1] += RowIdx;
        FieldName[2] += ColIdx;
        unsigned RowMajorIdx = RowIdx * NumCols + ColIdx;
        unsigned OffsetInBits = ElemSizeInBits * RowMajorIdx;
        llvm::DIType *FieldType = createFieldType(FieldName, ElemQualTy, 0,
          SourceLocation(), AccessSpecifier::AS_public, OffsetInBits,
          /* tunit */ nullptr, DITy, Ty->getDecl());
        Elements.emplace_back(FieldType);
      }
    }

    return true;
  } else if (hlsl::IsHLSLResourceType(QualTy) || hlsl::IsHLSLNodeType(QualTy) ||
             hlsl::IsHLSLStreamOutputType(QualTy)) {
    // Should appear as having no members rather than exposing our internal handles.
    return true;
  }

  return false;
}
// HLSL Chage Ends

llvm::DISubroutineType *
CGDebugInfo::getOrCreateMethodType(const CXXMethodDecl *Method,
                                   llvm::DIFile *Unit) {
  const FunctionProtoType *Func = Method->getType()->getAs<FunctionProtoType>();
  if (Method->isStatic())
    return cast_or_null<llvm::DISubroutineType>(
        getOrCreateType(QualType(Func, 0), Unit));
  return getOrCreateInstanceMethodType(Method->getThisType(CGM.getContext()),
                                       Func, Unit);
}

llvm::DISubroutineType *CGDebugInfo::getOrCreateInstanceMethodType(
    QualType ThisPtr, const FunctionProtoType *Func, llvm::DIFile *Unit) {
  // Add "this" pointer.
  llvm::DITypeRefArray Args(
      cast<llvm::DISubroutineType>(getOrCreateType(QualType(Func, 0), Unit))
          ->getTypeArray());
  assert(Args.size() && "Invalid number of arguments!");

  SmallVector<llvm::Metadata *, 16> Elts;

  // First element is always return type. For 'void' functions it is NULL.
  Elts.push_back(Args[0]);

  // "this" pointer is always first argument.
  const CXXRecordDecl *RD = ThisPtr->getPointeeCXXRecordDecl();
  if (isa<ClassTemplateSpecializationDecl>(RD)) {
    // HLSL Change Begin - This is a reference.
    llvm::DIType *ThisPtrType = getOrCreateType(ThisPtr, Unit);
    // HLSL Change End - This is a reference.
    TypeCache[ThisPtr.getAsOpaquePtr()].reset(ThisPtrType);
    // TODO: This and the artificial type below are misleading, the
    // types aren't artificial the argument is, but the current
    // metadata doesn't represent that.
    ThisPtrType = DBuilder.createObjectPointerType(ThisPtrType);
    Elts.push_back(ThisPtrType);
  } else {
    llvm::DIType *ThisPtrType = getOrCreateType(ThisPtr, Unit);
    TypeCache[ThisPtr.getAsOpaquePtr()].reset(ThisPtrType);
    ThisPtrType = DBuilder.createObjectPointerType(ThisPtrType);
    Elts.push_back(ThisPtrType);
  }

  // Copy rest of the arguments.
  for (unsigned i = 1, e = Args.size(); i != e; ++i)
    Elts.push_back(Args[i]);

  llvm::DITypeRefArray EltTypeArray = DBuilder.getOrCreateTypeArray(Elts);

  unsigned Flags = 0;
  if (Func->getExtProtoInfo().RefQualifier == RQ_LValue)
    Flags |= llvm::DINode::FlagLValueReference;
  if (Func->getExtProtoInfo().RefQualifier == RQ_RValue)
    Flags |= llvm::DINode::FlagRValueReference;

  return DBuilder.createSubroutineType(Unit, EltTypeArray, Flags);
}

/// isFunctionLocalClass - Return true if CXXRecordDecl is defined
/// inside a function.
static bool isFunctionLocalClass(const CXXRecordDecl *RD) {
  if (const CXXRecordDecl *NRD = dyn_cast<CXXRecordDecl>(RD->getDeclContext()))
    return isFunctionLocalClass(NRD);
  if (isa<FunctionDecl>(RD->getDeclContext()))
    return true;
  return false;
}

llvm::DISubprogram *CGDebugInfo::CreateCXXMemberFunction(
    const CXXMethodDecl *Method, llvm::DIFile *Unit, llvm::DIType *RecordTy) {
  bool IsCtorOrDtor =
      isa<CXXConstructorDecl>(Method) || isa<CXXDestructorDecl>(Method);

  StringRef MethodName = getFunctionName(Method);
  llvm::DISubroutineType *MethodTy = getOrCreateMethodType(Method, Unit);

  // Since a single ctor/dtor corresponds to multiple functions, it doesn't
  // make sense to give a single ctor/dtor a linkage name.
  StringRef MethodLinkageName;
  if (!IsCtorOrDtor && !isFunctionLocalClass(Method->getParent()))
    MethodLinkageName = CGM.getMangledName(Method);

  // Get the location for the method.
  llvm::DIFile *MethodDefUnit = nullptr;
  unsigned MethodLine = 0;
  if (!Method->isImplicit()) {
    MethodDefUnit = getOrCreateFile(Method->getLocation());
    MethodLine = getLineNumber(Method->getLocation());
  }

  // Collect virtual method info.
  llvm::DIType *ContainingType = nullptr;
  unsigned Virtuality = 0;
  unsigned VIndex = 0;

  if (Method->isVirtual()) {
    if (Method->isPure())
      Virtuality = llvm::dwarf::DW_VIRTUALITY_pure_virtual;
    else
      Virtuality = llvm::dwarf::DW_VIRTUALITY_virtual;

    // It doesn't make sense to give a virtual destructor a vtable index,
    // since a single destructor has two entries in the vtable.
    // FIXME: Add proper support for debug info for virtual calls in
    // the Microsoft ABI, where we may use multiple vptrs to make a vftable
    // lookup if we have multiple or virtual inheritance.
    if (!isa<CXXDestructorDecl>(Method) &&
        !CGM.getTarget().getCXXABI().isMicrosoft())
      VIndex = CGM.getItaniumVTableContext().getMethodVTableIndex(Method);
    ContainingType = RecordTy;
  }

  unsigned Flags = 0;
  if (Method->isImplicit())
    Flags |= llvm::DINode::FlagArtificial;
  Flags |= getAccessFlag(Method->getAccess(), Method->getParent());
  if (const CXXConstructorDecl *CXXC = dyn_cast<CXXConstructorDecl>(Method)) {
    if (CXXC->isExplicit())
      Flags |= llvm::DINode::FlagExplicit;
  } else if (const CXXConversionDecl *CXXC =
                 dyn_cast<CXXConversionDecl>(Method)) {
    if (CXXC->isExplicit())
      Flags |= llvm::DINode::FlagExplicit;
  }
  if (Method->hasPrototype())
    Flags |= llvm::DINode::FlagPrototyped;
  if (Method->getRefQualifier() == RQ_LValue)
    Flags |= llvm::DINode::FlagLValueReference;
  if (Method->getRefQualifier() == RQ_RValue)
    Flags |= llvm::DINode::FlagRValueReference;

  llvm::DINodeArray TParamsArray = CollectFunctionTemplateParams(Method, Unit);
  llvm::DISubprogram *SP = DBuilder.createMethod(
      RecordTy, MethodName, MethodLinkageName, MethodDefUnit, MethodLine,
      MethodTy, /*isLocalToUnit=*/false,
      /* isDefinition=*/false, Virtuality, VIndex, ContainingType, Flags,
      CGM.getLangOpts().Optimize, nullptr, TParamsArray.get());

  SPCache[Method->getCanonicalDecl()].reset(SP);

  return SP;
}

void CGDebugInfo::CollectCXXMemberFunctions(
    const CXXRecordDecl *RD, llvm::DIFile *Unit,
    SmallVectorImpl<llvm::Metadata *> &EltTys, llvm::DIType *RecordTy) {

  // Since we want more than just the individual member decls if we
  // have templated functions iterate over every declaration to gather
  // the functions.
  for (const auto *I : RD->decls()) {
    const auto *Method = dyn_cast<CXXMethodDecl>(I);
    // If the member is implicit, don't add it to the member list. This avoids
    // the member being added to type units by LLVM, while still allowing it
    // to be emitted into the type declaration/reference inside the compile
    // unit.
    // Ditto 'nodebug' methods, for consistency with CodeGenFunction.cpp.
    // FIXME: Handle Using(Shadow?)Decls here to create
    // DW_TAG_imported_declarations inside the class for base decls brought into
    // derived classes. GDB doesn't seem to notice/leverage these when I tried
    // it, so I'm not rushing to fix this. (GCC seems to produce them, if
    // referenced)
    if (!Method || Method->isImplicit() || Method->hasAttr<NoDebugAttr>())
      continue;

    if (Method->getType()->getAs<FunctionProtoType>()->getContainedAutoType())
      continue;

    // Reuse the existing member function declaration if it exists.
    // It may be associated with the declaration of the type & should be
    // reused as we're building the definition.
    //
    // This situation can arise in the vtable-based debug info reduction where
    // implicit members are emitted in a non-vtable TU.
    auto MI = SPCache.find(Method->getCanonicalDecl());
    EltTys.push_back(MI == SPCache.end()
                         ? CreateCXXMemberFunction(Method, Unit, RecordTy)
                         : static_cast<llvm::Metadata *>(MI->second));
  }
}

void CGDebugInfo::CollectCXXBases(const CXXRecordDecl *RD, llvm::DIFile *Unit,
                                  SmallVectorImpl<llvm::Metadata *> &EltTys,
                                  llvm::DIType *RecordTy) {
  const ASTRecordLayout &RL = CGM.getContext().getASTRecordLayout(RD);
  for (const auto &BI : RD->bases()) {
    unsigned BFlags = 0;
    uint64_t BaseOffset;

    const CXXRecordDecl *Base =
        cast<CXXRecordDecl>(BI.getType()->getAs<RecordType>()->getDecl());

    if (BI.isVirtual()) {
      if (CGM.getTarget().getCXXABI().isItaniumFamily()) {
        // virtual base offset offset is -ve. The code generator emits dwarf
        // expression where it expects +ve number.
        BaseOffset = 0 - CGM.getItaniumVTableContext()
                             .getVirtualBaseOffsetOffset(RD, Base)
                             .getQuantity();
      } else {
        // In the MS ABI, store the vbtable offset, which is analogous to the
        // vbase offset offset in Itanium.
        BaseOffset =
            4 * CGM.getMicrosoftVTableContext().getVBTableIndex(RD, Base);
      }
      BFlags = llvm::DINode::FlagVirtual;
    } else
      BaseOffset = CGM.getContext().toBits(RL.getBaseClassOffset(Base));
    // FIXME: Inconsistent units for BaseOffset. It is in bytes when
    // BI->isVirtual() and bits when not.

    BFlags |= getAccessFlag(BI.getAccessSpecifier(), RD);
    llvm::DIType *DTy = DBuilder.createInheritance(
        RecordTy, getOrCreateType(BI.getType(), Unit), BaseOffset, BFlags);
    EltTys.push_back(DTy);
  }
}

llvm::DINodeArray
CGDebugInfo::CollectTemplateParams(const TemplateParameterList *TPList,
                                   ArrayRef<TemplateArgument> TAList,
                                   llvm::DIFile *Unit) {
  SmallVector<llvm::Metadata *, 16> TemplateParams;
  for (unsigned i = 0, e = TAList.size(); i != e; ++i) {
    const TemplateArgument &TA = TAList[i];
    StringRef Name;
    if (TPList)
      Name = TPList->getParam(i)->getName();
    switch (TA.getKind()) {
    case TemplateArgument::Type: {
      llvm::DIType *TTy = getOrCreateType(TA.getAsType(), Unit);
      TemplateParams.push_back(
          DBuilder.createTemplateTypeParameter(TheCU, Name, TTy));
    } break;
    case TemplateArgument::Integral: {
      llvm::DIType *TTy = getOrCreateType(TA.getIntegralType(), Unit);
      TemplateParams.push_back(DBuilder.createTemplateValueParameter(
          TheCU, Name, TTy,
          llvm::ConstantInt::get(CGM.getLLVMContext(), TA.getAsIntegral())));
    } break;
    case TemplateArgument::Declaration: {
      const ValueDecl *D = TA.getAsDecl();
      QualType T = TA.getParamTypeForDecl().getDesugaredType(CGM.getContext());
      llvm::DIType *TTy = getOrCreateType(T, Unit);
      llvm::Constant *V = nullptr;
      const CXXMethodDecl *MD;
      // Variable pointer template parameters have a value that is the address
      // of the variable.
      if (const auto *VD = dyn_cast<VarDecl>(D))
        V = CGM.GetAddrOfGlobalVar(VD);
      // Member function pointers have special support for building them, though
      // this is currently unsupported in LLVM CodeGen.
      else if ((MD = dyn_cast<CXXMethodDecl>(D)) && MD->isInstance())
        V = CGM.getCXXABI().EmitMemberFunctionPointer(MD);
      else if (const auto *FD = dyn_cast<FunctionDecl>(D))
        V = CGM.GetAddrOfFunction(FD);
      // Member data pointers have special handling too to compute the fixed
      // offset within the object.
      else if (const auto *MPT = dyn_cast<MemberPointerType>(T.getTypePtr())) {
        // These five lines (& possibly the above member function pointer
        // handling) might be able to be refactored to use similar code in
        // CodeGenModule::getMemberPointerConstant
        uint64_t fieldOffset = CGM.getContext().getFieldOffset(D);
        CharUnits chars =
            CGM.getContext().toCharUnitsFromBits((int64_t)fieldOffset);
        V = CGM.getCXXABI().EmitMemberDataPointer(MPT, chars);
      }
      TemplateParams.push_back(DBuilder.createTemplateValueParameter(
          TheCU, Name, TTy,
          cast_or_null<llvm::Constant>(V->stripPointerCasts())));
    } break;
    case TemplateArgument::NullPtr: {
      QualType T = TA.getNullPtrType();
      llvm::DIType *TTy = getOrCreateType(T, Unit);
      llvm::Constant *V = nullptr;
      // Special case member data pointer null values since they're actually -1
      // instead of zero.
      if (const MemberPointerType *MPT =
              dyn_cast<MemberPointerType>(T.getTypePtr()))
        // But treat member function pointers as simple zero integers because
        // it's easier than having a special case in LLVM's CodeGen. If LLVM
        // CodeGen grows handling for values of non-null member function
        // pointers then perhaps we could remove this special case and rely on
        // EmitNullMemberPointer for member function pointers.
        if (MPT->isMemberDataPointer())
          V = CGM.getCXXABI().EmitNullMemberPointer(MPT);
      if (!V)
        V = llvm::ConstantInt::get(CGM.Int8Ty, 0);
      TemplateParams.push_back(DBuilder.createTemplateValueParameter(
          TheCU, Name, TTy, cast<llvm::Constant>(V)));
    } break;
    case TemplateArgument::Template:
      TemplateParams.push_back(DBuilder.createTemplateTemplateParameter(
          TheCU, Name, nullptr,
          TA.getAsTemplate().getAsTemplateDecl()->getQualifiedNameAsString()));
      break;
    case TemplateArgument::Pack:
      TemplateParams.push_back(DBuilder.createTemplateParameterPack(
          TheCU, Name, nullptr,
          CollectTemplateParams(nullptr, TA.getPackAsArray(), Unit)));
      break;
    case TemplateArgument::Expression: {
      const Expr *E = TA.getAsExpr();
      QualType T = E->getType();
      if (E->isGLValue())
        T = CGM.getContext().getLValueReferenceType(T);
      llvm::Constant *V = CGM.EmitConstantExpr(E, T);
      assert(V && "Expression in template argument isn't constant");
      llvm::DIType *TTy = getOrCreateType(T, Unit);
      TemplateParams.push_back(DBuilder.createTemplateValueParameter(
          TheCU, Name, TTy, cast<llvm::Constant>(V->stripPointerCasts())));
    } break;
    // And the following should never occur:
    case TemplateArgument::TemplateExpansion:
    case TemplateArgument::Null:
      llvm_unreachable(
          "These argument types shouldn't exist in concrete types");
    }
  }
  return DBuilder.getOrCreateArray(TemplateParams);
}

llvm::DINodeArray
CGDebugInfo::CollectFunctionTemplateParams(const FunctionDecl *FD,
                                           llvm::DIFile *Unit) {
  if (FD->getTemplatedKind() ==
      FunctionDecl::TK_FunctionTemplateSpecialization) {
    const TemplateParameterList *TList = FD->getTemplateSpecializationInfo()
                                             ->getTemplate()
                                             ->getTemplateParameters();
    return CollectTemplateParams(
        TList, FD->getTemplateSpecializationArgs()->asArray(), Unit);
  }
  return llvm::DINodeArray();
}

llvm::DINodeArray CGDebugInfo::CollectCXXTemplateParams(
    const ClassTemplateSpecializationDecl *TSpecial, llvm::DIFile *Unit) {
  // Always get the full list of parameters, not just the ones from
  // the specialization.
  TemplateParameterList *TPList =
      TSpecial->getSpecializedTemplate()->getTemplateParameters();
  const TemplateArgumentList &TAList = TSpecial->getTemplateArgs();
  return CollectTemplateParams(TPList, TAList.asArray(), Unit);
}

llvm::DIType *CGDebugInfo::getOrCreateVTablePtrType(llvm::DIFile *Unit) {
  if (VTablePtrType)
    return VTablePtrType;

  ASTContext &Context = CGM.getContext();

  /* Function type */
  llvm::Metadata *STy = getOrCreateType(Context.IntTy, Unit);
  llvm::DITypeRefArray SElements = DBuilder.getOrCreateTypeArray(STy);
  llvm::DIType *SubTy = DBuilder.createSubroutineType(Unit, SElements);
  unsigned Size = Context.getTypeSize(Context.VoidPtrTy);
  llvm::DIType *vtbl_ptr_type =
      DBuilder.createPointerType(SubTy, Size, 0, "__vtbl_ptr_type");
  VTablePtrType = DBuilder.createPointerType(vtbl_ptr_type, Size);
  return VTablePtrType;
}

StringRef CGDebugInfo::getVTableName(const CXXRecordDecl *RD) {
  // Copy the gdb compatible name on the side and use its reference.
  return internString("_vptr$", RD->getNameAsString());
}

void CGDebugInfo::CollectVTableInfo(const CXXRecordDecl *RD, llvm::DIFile *Unit,
                                    SmallVectorImpl<llvm::Metadata *> &EltTys) {
  const ASTRecordLayout &RL = CGM.getContext().getASTRecordLayout(RD);

  // If there is a primary base then it will hold vtable info.
  if (RL.getPrimaryBase())
    return;

  // If this class is not dynamic then there is not any vtable info to collect.
  if (!RD->isDynamicClass())
    return;

  unsigned Size = CGM.getContext().getTypeSize(CGM.getContext().VoidPtrTy);
  llvm::DIType *VPTR = DBuilder.createMemberType(
      Unit, getVTableName(RD), Unit, 0, Size, 0, 0,
      llvm::DINode::FlagArtificial, getOrCreateVTablePtrType(Unit));
  EltTys.push_back(VPTR);
}

llvm::DIType *CGDebugInfo::getOrCreateRecordType(QualType RTy,
                                                 SourceLocation Loc) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  llvm::DIType *T = getOrCreateType(RTy, getOrCreateFile(Loc));
  return T;
}

llvm::DIType *CGDebugInfo::getOrCreateInterfaceType(QualType D,
                                                    SourceLocation Loc) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  llvm::DIType *T = getOrCreateType(D, getOrCreateFile(Loc));
  RetainedTypes.push_back(D.getAsOpaquePtr());
  return T;
}

void CGDebugInfo::completeType(const EnumDecl *ED) {
  if (DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return;
  QualType Ty = CGM.getContext().getEnumType(ED);
  void *TyPtr = Ty.getAsOpaquePtr();
  auto I = TypeCache.find(TyPtr);
  if (I == TypeCache.end() || !cast<llvm::DIType>(I->second)->isForwardDecl())
    return;
  llvm::DIType *Res = CreateTypeDefinition(Ty->castAs<EnumType>());
  assert(!Res->isForwardDecl());
  TypeCache[TyPtr].reset(Res);
}

void CGDebugInfo::completeType(const RecordDecl *RD) {
  if (DebugKind > CodeGenOptions::LimitedDebugInfo ||
      !CGM.getLangOpts().CPlusPlus)
    completeRequiredType(RD);
}

void CGDebugInfo::completeRequiredType(const RecordDecl *RD) {
  if (DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return;

  if (const CXXRecordDecl *CXXDecl = dyn_cast<CXXRecordDecl>(RD))
    if (CXXDecl->isDynamicClass())
      return;

  QualType Ty = CGM.getContext().getRecordType(RD);
  llvm::DIType *T = getTypeOrNull(Ty);
  if (T && T->isForwardDecl())
    completeClassData(RD);
}

void CGDebugInfo::completeClassData(const RecordDecl *RD) {
  if (DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return;
  QualType Ty = CGM.getContext().getRecordType(RD);
  void *TyPtr = Ty.getAsOpaquePtr();
  auto I = TypeCache.find(TyPtr);
  if (I != TypeCache.end() && !cast<llvm::DIType>(I->second)->isForwardDecl())
    return;
  llvm::DIType *Res = CreateTypeDefinition(Ty->castAs<RecordType>());
  assert(!Res->isForwardDecl());
  TypeCache[TyPtr].reset(Res);
}

static bool hasExplicitMemberDefinition(CXXRecordDecl::method_iterator I,
                                        CXXRecordDecl::method_iterator End) {
  for (; I != End; ++I)
    if (FunctionDecl *Tmpl = I->getInstantiatedFromMemberFunction())
      if (!Tmpl->isImplicit() && Tmpl->isThisDeclarationADefinition() &&
          !I->getMemberSpecializationInfo()->isExplicitSpecialization())
        return true;
  return false;
}

static bool shouldOmitDefinition(CodeGenOptions::DebugInfoKind DebugKind,
                                 const RecordDecl *RD,
                                 const LangOptions &LangOpts) {
  if (DebugKind > CodeGenOptions::LimitedDebugInfo)
    return false;

  if (!LangOpts.CPlusPlus)
    return false;

  if (!RD->isCompleteDefinitionRequired())
    return true;

  const CXXRecordDecl *CXXDecl = dyn_cast<CXXRecordDecl>(RD);

  if (!CXXDecl)
    return false;

  if (CXXDecl->hasDefinition() && CXXDecl->isDynamicClass())
    return true;

  TemplateSpecializationKind Spec = TSK_Undeclared;
  if (const ClassTemplateSpecializationDecl *SD =
          dyn_cast<ClassTemplateSpecializationDecl>(RD))
    Spec = SD->getSpecializationKind();

  if (Spec == TSK_ExplicitInstantiationDeclaration &&
      hasExplicitMemberDefinition(CXXDecl->method_begin(),
                                  CXXDecl->method_end()))
    return true;

  return false;
}

llvm::DIType *CGDebugInfo::CreateType(const RecordType *Ty) {
  RecordDecl *RD = Ty->getDecl();
  llvm::DIType *T = cast_or_null<llvm::DIType>(getTypeOrNull(QualType(Ty, 0)));
  if (T || shouldOmitDefinition(DebugKind, RD, CGM.getLangOpts())) {
    if (!T)
      T = getOrCreateRecordFwdDecl(
          Ty, getContextDescriptor(cast<Decl>(RD->getDeclContext())));
    return T;
  }

  return CreateTypeDefinition(Ty);
}

llvm::DIType *CGDebugInfo::CreateTypeDefinition(const RecordType *Ty) {
  RecordDecl *RD = Ty->getDecl();

  // Get overall information about the record type for the debug info.
  llvm::DIFile *DefUnit = getOrCreateFile(RD->getLocation());

  // Records and classes and unions can all be recursive.  To handle them, we
  // first generate a debug descriptor for the struct as a forward declaration.
  // Then (if it is a definition) we go through and get debug info for all of
  // its members.  Finally, we create a descriptor for the complete type (which
  // may refer to the forward decl if the struct is recursive) and replace all
  // uses of the forward declaration with the final definition.

  auto *FwdDecl =
      cast<llvm::DICompositeType>(getOrCreateLimitedType(Ty, DefUnit));

  const RecordDecl *D = RD->getDefinition();
  if (!D || !D->isCompleteDefinition())
    return FwdDecl;

  if (const CXXRecordDecl *CXXDecl = dyn_cast<CXXRecordDecl>(RD))
    CollectContainingType(CXXDecl, FwdDecl);

  // Push the struct on region stack.
  LexicalBlockStack.emplace_back(&*FwdDecl);
  RegionMap[Ty->getDecl()].reset(FwdDecl);

  // Convert all the elements.
  SmallVector<llvm::Metadata *, 16> EltTys;
  // what about nested types?

  if (!TryCollectHLSLRecordElements(Ty, FwdDecl, EltTys)) { // HLSL Change

    // Note: The split of CXXDecl information here is intentional, the
    // gdb tests will depend on a certain ordering at printout. The debug
    // information offsets are still correct if we merge them all together
    // though.
    const CXXRecordDecl *CXXDecl = dyn_cast<CXXRecordDecl>(RD);
    if (CXXDecl) {
      CollectCXXBases(CXXDecl, DefUnit, EltTys, FwdDecl);
      CollectVTableInfo(CXXDecl, DefUnit, EltTys);
    }

    // Collect data fields (including static variables and any initializers).
    CollectRecordFields(RD, DefUnit, EltTys, FwdDecl);
    if (CXXDecl)
      CollectCXXMemberFunctions(CXXDecl, DefUnit, EltTys, FwdDecl);

  } // HLSL Change

  LexicalBlockStack.pop_back();
  RegionMap.erase(Ty->getDecl());

  llvm::DINodeArray Elements = DBuilder.getOrCreateArray(EltTys);
  DBuilder.replaceArrays(FwdDecl, Elements);

  if (FwdDecl->isTemporary())
    FwdDecl =
        llvm::MDNode::replaceWithPermanent(llvm::TempDICompositeType(FwdDecl));

  RegionMap[Ty->getDecl()].reset(FwdDecl);
  return FwdDecl;
}

#if 0 // HLSL Change - no ObjC support
llvm::DIType *CGDebugInfo::CreateType(const ObjCObjectType *Ty,
                                      llvm::DIFile *Unit) {
  // Ignore protocols.
  return getOrCreateType(Ty->getBaseType(), Unit);
}

/// \return true if Getter has the default name for the property PD.
static bool hasDefaultGetterName(const ObjCPropertyDecl *PD,
                                 const ObjCMethodDecl *Getter) {
  assert(PD);
  if (!Getter)
    return true;

  assert(Getter->getDeclName().isObjCZeroArgSelector());
  return PD->getName() ==
         Getter->getDeclName().getObjCSelector().getNameForSlot(0);
}

/// \return true if Setter has the default name for the property PD.
static bool hasDefaultSetterName(const ObjCPropertyDecl *PD,
                                 const ObjCMethodDecl *Setter) {
  assert(PD);
  if (!Setter)
    return true;

  assert(Setter->getDeclName().isObjCOneArgSelector());
  return SelectorTable::constructSetterName(PD->getName()) ==
         Setter->getDeclName().getObjCSelector().getNameForSlot(0);
}

llvm::DIType *CGDebugInfo::CreateType(const ObjCInterfaceType *Ty,
                                      llvm::DIFile *Unit) {
  ObjCInterfaceDecl *ID = Ty->getDecl();
  if (!ID)
    return nullptr;

  // Get overall information about the record type for the debug info.
  llvm::DIFile *DefUnit = getOrCreateFile(ID->getLocation());
  unsigned Line = getLineNumber(ID->getLocation());
  auto RuntimeLang =
      static_cast<llvm::dwarf::SourceLanguage>(TheCU->getSourceLanguage());

  // If this is just a forward declaration return a special forward-declaration
  // debug type since we won't be able to lay out the entire type.
  ObjCInterfaceDecl *Def = ID->getDefinition();
  if (!Def || !Def->getImplementation()) {
    llvm::DIType *FwdDecl = DBuilder.createReplaceableCompositeType(
        llvm::dwarf::DW_TAG_structure_type, ID->getName(), TheCU, DefUnit, Line,
        RuntimeLang);
    ObjCInterfaceCache.push_back(ObjCInterfaceCacheEntry(Ty, FwdDecl, Unit));
    return FwdDecl;
  }

  return CreateTypeDefinition(Ty, Unit);
}
#endif // HLSL Change - no ObjC support

llvm::DIModule *
CGDebugInfo::getOrCreateModuleRef(ExternalASTSource::ASTSourceDescriptor Mod) {
  auto it = ModuleRefCache.find(Mod.Signature);
  if (it != ModuleRefCache.end())
    return it->second;

  // Macro definitions that were defined with "-D" on the command line.
  SmallString<128> ConfigMacros;
  {
    llvm::raw_svector_ostream OS(ConfigMacros);
    const auto &PPOpts = CGM.getPreprocessorOpts();
    unsigned I = 0;
    // Translate the macro definitions back into a commmand line.
    for (auto &M : PPOpts.Macros) {
      if (++I > 1)
        OS << " ";
      const std::string &Macro = M.first;
      bool Undef = M.second;
      OS << "\"-" << (Undef ? 'U' : 'D');
      for (char c : Macro)
        switch (c) {
        case '\\' : OS << "\\\\"; break;
        case '"'  : OS << "\\\""; break;
        default: OS << c;
        }
      OS << '\"';
    }
  }
  llvm::DIBuilder DIB(CGM.getModule());
  auto *CU = DIB.createCompileUnit(
      TheCU->getSourceLanguage(), internString(Mod.ModuleName),
      internString(Mod.Path), TheCU->getProducer(), true, StringRef(), 0,
      internString(Mod.ASTFile), llvm::DIBuilder::FullDebug, Mod.Signature);
  llvm::DIModule *ModuleRef =
      DIB.createModule(CU, Mod.ModuleName, ConfigMacros, internString(Mod.Path),
                       internString(CGM.getHeaderSearchOpts().Sysroot));
  DIB.finalize();
  ModuleRefCache.insert(std::make_pair(Mod.Signature, ModuleRef));
  return ModuleRef;
}

#if 0 // HLSL Change - no ObjC support
llvm::DIType *CGDebugInfo::CreateTypeDefinition(const ObjCInterfaceType *Ty,
                                                llvm::DIFile *Unit) {
  ObjCInterfaceDecl *ID = Ty->getDecl();
  llvm::DIFile *DefUnit = getOrCreateFile(ID->getLocation());
  unsigned Line = getLineNumber(ID->getLocation());
  unsigned RuntimeLang = TheCU->getSourceLanguage();

  // Bit size, align and offset of the type.
  uint64_t Size = CGM.getContext().getTypeSize(Ty);
  uint64_t Align = CGM.getContext().getTypeAlign(Ty);

  unsigned Flags = 0;
  if (ID->getImplementation())
    Flags |= llvm::DINode::FlagObjcClassComplete;

  llvm::DICompositeType *RealDecl = DBuilder.createStructType(
      Unit, ID->getName(), DefUnit, Line, Size, Align, Flags, nullptr,
      llvm::DINodeArray(), RuntimeLang);

  QualType QTy(Ty, 0);
  TypeCache[QTy.getAsOpaquePtr()].reset(RealDecl);

  // Push the struct on region stack.
  LexicalBlockStack.emplace_back(RealDecl);
  RegionMap[Ty->getDecl()].reset(RealDecl);

  // Convert all the elements.
  SmallVector<llvm::Metadata *, 16> EltTys;

  ObjCInterfaceDecl *SClass = ID->getSuperClass();
  if (SClass) {
    llvm::DIType *SClassTy =
        getOrCreateType(CGM.getContext().getObjCInterfaceType(SClass), Unit);
    if (!SClassTy)
      return nullptr;

    llvm::DIType *InhTag = DBuilder.createInheritance(RealDecl, SClassTy, 0, 0);
    EltTys.push_back(InhTag);
  }

  // Create entries for all of the properties.
  for (const auto *PD : ID->properties()) {
    SourceLocation Loc = PD->getLocation();
    llvm::DIFile *PUnit = getOrCreateFile(Loc);
    unsigned PLine = getLineNumber(Loc);
    ObjCMethodDecl *Getter = PD->getGetterMethodDecl();
    ObjCMethodDecl *Setter = PD->getSetterMethodDecl();
    llvm::MDNode *PropertyNode = DBuilder.createObjCProperty(
        PD->getName(), PUnit, PLine,
        hasDefaultGetterName(PD, Getter) ? ""
                                         : getSelectorName(PD->getGetterName()),
        hasDefaultSetterName(PD, Setter) ? ""
                                         : getSelectorName(PD->getSetterName()),
        PD->getPropertyAttributes(), getOrCreateType(PD->getType(), PUnit));
    EltTys.push_back(PropertyNode);
  }

  const ASTRecordLayout &RL = CGM.getContext().getASTObjCInterfaceLayout(ID);
  unsigned FieldNo = 0;
  for (ObjCIvarDecl *Field = ID->all_declared_ivar_begin(); Field;
       Field = Field->getNextIvar(), ++FieldNo) {
    llvm::DIType *FieldTy = getOrCreateType(Field->getType(), Unit);
    if (!FieldTy)
      return nullptr;

    StringRef FieldName = Field->getName();

    // Ignore unnamed fields.
    if (FieldName.empty())
      continue;

    // Get the location for the field.
    llvm::DIFile *FieldDefUnit = getOrCreateFile(Field->getLocation());
    unsigned FieldLine = getLineNumber(Field->getLocation());
    QualType FType = Field->getType();
    uint64_t FieldSize = 0;
    unsigned FieldAlign = 0;

    if (!FType->isIncompleteArrayType()) {

      // Bit size, align and offset of the type.
      FieldSize = Field->isBitField()
                      ? Field->getBitWidthValue(CGM.getContext())
                      : CGM.getContext().getTypeSize(FType);
      FieldAlign = CGM.getContext().getTypeAlign(FType);
    }

    uint64_t FieldOffset;
    if (CGM.getLangOpts().ObjCRuntime.isNonFragile()) {
      // We don't know the runtime offset of an ivar if we're using the
      // non-fragile ABI.  For bitfields, use the bit offset into the first
      // byte of storage of the bitfield.  For other fields, use zero.
      if (Field->isBitField()) {
        FieldOffset =
            CGM.getObjCRuntime().ComputeBitfieldBitOffset(CGM, ID, Field);
        FieldOffset %= CGM.getContext().getCharWidth();
      } else {
        FieldOffset = 0;
      }
    } else {
      FieldOffset = RL.getFieldOffset(FieldNo);
    }

    unsigned Flags = 0;
    if (Field->getAccessControl() == ObjCIvarDecl::Protected)
      Flags = llvm::DINode::FlagProtected;
    else if (Field->getAccessControl() == ObjCIvarDecl::Private)
      Flags = llvm::DINode::FlagPrivate;
    else if (Field->getAccessControl() == ObjCIvarDecl::Public)
      Flags = llvm::DINode::FlagPublic;

    llvm::MDNode *PropertyNode = nullptr;
    if (ObjCImplementationDecl *ImpD = ID->getImplementation()) {
      if (ObjCPropertyImplDecl *PImpD =
              ImpD->FindPropertyImplIvarDecl(Field->getIdentifier())) {
        if (ObjCPropertyDecl *PD = PImpD->getPropertyDecl()) {
          SourceLocation Loc = PD->getLocation();
          llvm::DIFile *PUnit = getOrCreateFile(Loc);
          unsigned PLine = getLineNumber(Loc);
          ObjCMethodDecl *Getter = PD->getGetterMethodDecl();
          ObjCMethodDecl *Setter = PD->getSetterMethodDecl();
          PropertyNode = DBuilder.createObjCProperty(
              PD->getName(), PUnit, PLine,
              hasDefaultGetterName(PD, Getter) ? "" : getSelectorName(
                                                          PD->getGetterName()),
              hasDefaultSetterName(PD, Setter) ? "" : getSelectorName(
                                                          PD->getSetterName()),
              PD->getPropertyAttributes(),
              getOrCreateType(PD->getType(), PUnit));
        }
      }
    }
    FieldTy = DBuilder.createObjCIVar(FieldName, FieldDefUnit, FieldLine,
                                      FieldSize, FieldAlign, FieldOffset, Flags,
                                      FieldTy, PropertyNode);
    EltTys.push_back(FieldTy);
  }

  llvm::DINodeArray Elements = DBuilder.getOrCreateArray(EltTys);
  DBuilder.replaceArrays(RealDecl, Elements);

  LexicalBlockStack.pop_back();
  return RealDecl;
}
#endif // HLSL Change - no ObjC support

llvm::DIType *CGDebugInfo::CreateType(const VectorType *Ty,
                                      llvm::DIFile *Unit) {
  llvm::DIType *ElementTy = getOrCreateType(Ty->getElementType(), Unit);
  int64_t Count = Ty->getNumElements();
  if (Count == 0)
    // If number of elements are not known then this is an unbounded array.
    // Use Count == -1 to express such arrays.
    Count = -1;

  llvm::Metadata *Subscript = DBuilder.getOrCreateSubrange(0, Count);
  llvm::DINodeArray SubscriptArray = DBuilder.getOrCreateArray(Subscript);

  uint64_t Size = CGM.getContext().getTypeSize(Ty);
  uint64_t Align = CGM.getContext().getTypeAlign(Ty);

  return DBuilder.createVectorType(Size, Align, ElementTy, SubscriptArray);
}

llvm::DIType *CGDebugInfo::CreateType(const ArrayType *Ty, llvm::DIFile *Unit) {
  uint64_t Size;
  uint64_t Align;

  // FIXME: make getTypeAlign() aware of VLAs and incomplete array types
  if (const VariableArrayType *VAT = dyn_cast<VariableArrayType>(Ty)) {
    Size = 0;
    Align =
        CGM.getContext().getTypeAlign(CGM.getContext().getBaseElementType(VAT));
  } else if (Ty->isIncompleteArrayType()) {
    Size = 0;
    if (Ty->getElementType()->isIncompleteType())
      Align = 0;
    else
      Align = CGM.getContext().getTypeAlign(Ty->getElementType());
  } else if (Ty->isIncompleteType()) {
    Size = 0;
    Align = 0;
  } else {
    // Size and align of the whole array, not the element type.
    Size = CGM.getContext().getTypeSize(Ty);
    Align = CGM.getContext().getTypeAlign(Ty);
  }

  // Add the dimensions of the array.  FIXME: This loses CV qualifiers from
  // interior arrays, do we care?  Why aren't nested arrays represented the
  // obvious/recursive way?
  SmallVector<llvm::Metadata *, 8> Subscripts;
  QualType EltTy(Ty, 0);
  while ((Ty = dyn_cast<ArrayType>(EltTy))) {
    // If the number of elements is known, then count is that number. Otherwise,
    // it's -1. This allows us to represent a subrange with an array of 0
    // elements, like this:
    //
    //   struct foo {
    //     int x[0];
    //   };
    int64_t Count = -1; // Count == -1 is an unbounded array.
    if (const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(Ty))
      Count = CAT->getSize().getZExtValue();

    // FIXME: Verify this is right for VLAs.
    Subscripts.push_back(DBuilder.getOrCreateSubrange(0, Count));
    EltTy = Ty->getElementType();
  }

  llvm::DINodeArray SubscriptArray = DBuilder.getOrCreateArray(Subscripts);

  return DBuilder.createArrayType(Size, Align, getOrCreateType(EltTy, Unit),
                                  SubscriptArray);
}

llvm::DIType *CGDebugInfo::CreateType(const LValueReferenceType *Ty,
                                      llvm::DIFile *Unit) {
  return CreatePointerLikeType(llvm::dwarf::DW_TAG_reference_type, Ty,
                               Ty->getPointeeType(), Unit);
}

llvm::DIType *CGDebugInfo::CreateType(const RValueReferenceType *Ty,
                                      llvm::DIFile *Unit) {
  return CreatePointerLikeType(llvm::dwarf::DW_TAG_rvalue_reference_type, Ty,
                               Ty->getPointeeType(), Unit);
}

llvm::DIType *CGDebugInfo::CreateType(const MemberPointerType *Ty,
                                      llvm::DIFile *U) {
  uint64_t Size = CGM.getCXXABI().isTypeInfoCalculable(QualType(Ty, 0))
                      ? CGM.getContext().getTypeSize(Ty)
                      : 0;
  llvm::DIType *ClassType = getOrCreateType(QualType(Ty->getClass(), 0), U);
  if (Ty->isMemberDataPointerType())
    return DBuilder.createMemberPointerType(
        getOrCreateType(Ty->getPointeeType(), U), ClassType, Size);

  const FunctionProtoType *FPT =
      Ty->getPointeeType()->getAs<FunctionProtoType>();
  return DBuilder.createMemberPointerType(
      getOrCreateInstanceMethodType(CGM.getContext().getPointerType(QualType(
                                        Ty->getClass(), FPT->getTypeQuals())),
                                    FPT, U),
      ClassType, Size);
}

llvm::DIType *CGDebugInfo::CreateType(const AtomicType *Ty, llvm::DIFile *U) {
  // Ignore the atomic wrapping
  // FIXME: What is the correct representation?
  return getOrCreateType(Ty->getValueType(), U);
}

llvm::DIType *CGDebugInfo::CreateEnumType(const EnumType *Ty) {
  const EnumDecl *ED = Ty->getDecl();
  uint64_t Size = 0;
  uint64_t Align = 0;
  if (!ED->getTypeForDecl()->isIncompleteType()) {
    Size = CGM.getContext().getTypeSize(ED->getTypeForDecl());
    Align = CGM.getContext().getTypeAlign(ED->getTypeForDecl());
  }

  SmallString<256> FullName = getUniqueTagTypeName(Ty, CGM, TheCU);

  // If this is just a forward declaration, construct an appropriately
  // marked node and just return it.
  if (!ED->getDefinition()) {
    llvm::DIScope *EDContext =
        getContextDescriptor(cast<Decl>(ED->getDeclContext()));
    llvm::DIFile *DefUnit = getOrCreateFile(ED->getLocation());
    unsigned Line = getLineNumber(ED->getLocation());
    StringRef EDName = ED->getName();
    llvm::DIType *RetTy = DBuilder.createReplaceableCompositeType(
        llvm::dwarf::DW_TAG_enumeration_type, EDName, EDContext, DefUnit, Line,
        0, Size, Align, llvm::DINode::FlagFwdDecl, FullName);
    ReplaceMap.emplace_back(
        std::piecewise_construct, std::make_tuple(Ty),
        std::make_tuple(static_cast<llvm::Metadata *>(RetTy)));
    return RetTy;
  }

  return CreateTypeDefinition(Ty);
}

llvm::DIType *CGDebugInfo::CreateTypeDefinition(const EnumType *Ty) {
  const EnumDecl *ED = Ty->getDecl();
  uint64_t Size = 0;
  uint64_t Align = 0;
  if (!ED->getTypeForDecl()->isIncompleteType()) {
    Size = CGM.getContext().getTypeSize(ED->getTypeForDecl());
    Align = CGM.getContext().getTypeAlign(ED->getTypeForDecl());
  }

  SmallString<256> FullName = getUniqueTagTypeName(Ty, CGM, TheCU);

  // Create elements for each enumerator.
  SmallVector<llvm::Metadata *, 16> Enumerators;
  ED = ED->getDefinition();
  for (const auto *Enum : ED->enumerators()) {
    Enumerators.push_back(DBuilder.createEnumerator(
        Enum->getName(), Enum->getInitVal().getSExtValue()));
  }

  // Return a CompositeType for the enum itself.
  llvm::DINodeArray EltArray = DBuilder.getOrCreateArray(Enumerators);

  llvm::DIFile *DefUnit = getOrCreateFile(ED->getLocation());
  unsigned Line = getLineNumber(ED->getLocation());
  llvm::DIScope *EnumContext =
      getContextDescriptor(cast<Decl>(ED->getDeclContext()));
  llvm::DIType *ClassTy =
      ED->isFixed() ? getOrCreateType(ED->getIntegerType(), DefUnit) : nullptr;
  return DBuilder.createEnumerationType(EnumContext, ED->getName(), DefUnit,
                                        Line, Size, Align, EltArray, ClassTy,
                                        FullName);
}

static QualType UnwrapTypeForDebugInfo(QualType T, const ASTContext &C) {
  Qualifiers Quals;
  do {
    Qualifiers InnerQuals = T.getLocalQualifiers();
    // Qualifiers::operator+() doesn't like it if you add a Qualifier
    // that is already there.
    Quals += Qualifiers::removeCommonQualifiers(Quals, InnerQuals);
    Quals += InnerQuals;
    QualType LastT = T;
    switch (T->getTypeClass()) {
    default:
      return C.getQualifiedType(T.getTypePtr(), Quals);
    case Type::TemplateSpecialization: {
      const auto *Spec = cast<TemplateSpecializationType>(T);
      if (Spec->isTypeAlias())
        return C.getQualifiedType(T.getTypePtr(), Quals);
      T = Spec->desugar();
      break;
    }
    case Type::TypeOfExpr:
      T = cast<TypeOfExprType>(T)->getUnderlyingExpr()->getType();
      break;
    case Type::TypeOf:
      T = cast<TypeOfType>(T)->getUnderlyingType();
      break;
    case Type::Decltype:
      T = cast<DecltypeType>(T)->getUnderlyingType();
      break;
    case Type::UnaryTransform:
      T = cast<UnaryTransformType>(T)->getUnderlyingType();
      break;
    case Type::Attributed:
      T = cast<AttributedType>(T)->getEquivalentType();
      break;
    case Type::Elaborated:
      T = cast<ElaboratedType>(T)->getNamedType();
      break;
    case Type::Paren:
      T = cast<ParenType>(T)->getInnerType();
      break;
    case Type::SubstTemplateTypeParm:
      T = cast<SubstTemplateTypeParmType>(T)->getReplacementType();
      break;
    case Type::Auto:
      QualType DT = cast<AutoType>(T)->getDeducedType();
      assert(!DT.isNull() && "Undeduced types shouldn't reach here.");
      T = DT;
      break;
    }

    assert(T != LastT && "Type unwrapping failed to unwrap!");
    (void)LastT;
  } while (true);
}

llvm::DIType *CGDebugInfo::getTypeOrNull(QualType Ty) {

  // Unwrap the type as needed for debug information.
  Ty = UnwrapTypeForDebugInfo(Ty, CGM.getContext());

  auto it = TypeCache.find(Ty.getAsOpaquePtr());
  if (it != TypeCache.end()) {
    // Verify that the debug info still exists.
    if (llvm::Metadata *V = it->second)
      return cast<llvm::DIType>(V);
  }

  return nullptr;
}

void CGDebugInfo::completeTemplateDefinition(
    const ClassTemplateSpecializationDecl &SD) {
  if (DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return;

  completeClassData(&SD);
  // In case this type has no member function definitions being emitted, ensure
  // it is retained
  RetainedTypes.push_back(CGM.getContext().getRecordType(&SD).getAsOpaquePtr());
}

llvm::DIType *CGDebugInfo::getOrCreateType(QualType Ty, llvm::DIFile *Unit) {
  if (Ty.isNull())
    return nullptr;

  // Unwrap the type as needed for debug information.
  Ty = UnwrapTypeForDebugInfo(Ty, CGM.getContext());

  if (auto *T = getTypeOrNull(Ty))
    return T;

  // Otherwise create the type.
  llvm::DIType *Res = CreateTypeNode(Ty, Unit);
  void *TyPtr = Ty.getAsOpaquePtr();

  // And update the type cache.
  TypeCache[TyPtr].reset(Res);

  return Res;
}

#if 0 // HLSL Change - no ObjC support
unsigned CGDebugInfo::Checksum(const ObjCInterfaceDecl *ID) {
  // The assumption is that the number of ivars can only increase
  // monotonically, so it is safe to just use their current number as
  // a checksum.
  unsigned Sum = 0;
  for (const ObjCIvarDecl *Ivar = ID->all_declared_ivar_begin();
       Ivar != nullptr; Ivar = Ivar->getNextIvar())
    ++Sum;

  return Sum;
}

ObjCInterfaceDecl *CGDebugInfo::getObjCInterfaceDecl(QualType Ty) {
  switch (Ty->getTypeClass()) {
  case Type::ObjCObjectPointer:
    return getObjCInterfaceDecl(
        cast<ObjCObjectPointerType>(Ty)->getPointeeType());
  case Type::ObjCInterface:
    return cast<ObjCInterfaceType>(Ty)->getDecl();
  default:
    return nullptr;
  }
}
#endif // HLSL Change - no ObjC support

llvm::DIType *CGDebugInfo::CreateTypeNode(QualType Ty, llvm::DIFile *Unit) {
  // Handle qualifiers, which recursively handles what they refer to.
  if (Ty.hasLocalQualifiers())
    return CreateQualifiedType(Ty, Unit);

  // Work out details of type.
  switch (Ty->getTypeClass()) {
#define TYPE(Class, Base)
#define ABSTRACT_TYPE(Class, Base)
#define NON_CANONICAL_TYPE(Class, Base)
#define DEPENDENT_TYPE(Class, Base) case Type::Class:
#include "clang/AST/TypeNodes.def"
    llvm_unreachable("Dependent types cannot show up in debug information");

  case Type::ExtVector:
  case Type::Vector:
    return CreateType(cast<VectorType>(Ty), Unit);
#if 0 // HLSL Change - no ObjC support
  case Type::ObjCObjectPointer:
    return CreateType(cast<ObjCObjectPointerType>(Ty), Unit);
  case Type::ObjCObject:
    return CreateType(cast<ObjCObjectType>(Ty), Unit);
  case Type::ObjCInterface:
    return CreateType(cast<ObjCInterfaceType>(Ty), Unit);
#else
  case Type::ObjCObjectPointer:
  case Type::ObjCObject:
  case Type::ObjCInterface:
    llvm_unreachable("No ObjC Support");
#endif // HLSL Change - no ObjC support
  case Type::Builtin:
    return CreateType(cast<BuiltinType>(Ty));
  case Type::Complex:
    return CreateType(cast<ComplexType>(Ty));
  case Type::Pointer:
    return CreateType(cast<PointerType>(Ty), Unit);
  case Type::Adjusted:
  case Type::Decayed:
    // Decayed and adjusted types use the adjusted type in LLVM and DWARF.
    return CreateType(
        cast<PointerType>(cast<AdjustedType>(Ty)->getAdjustedType()), Unit);
#if 0 // HLSL Change - no block support
  case Type::BlockPointer:
    return CreateType(cast<BlockPointerType>(Ty), Unit);
#else
  case Type::BlockPointer:
    llvm_unreachable("No Block Support");
#endif // HLSL Change - no block support
  case Type::Typedef:
    return CreateType(cast<TypedefType>(Ty), Unit);
  case Type::Record:
    return CreateType(cast<RecordType>(Ty));
  case Type::Enum:
    return CreateEnumType(cast<EnumType>(Ty));
  case Type::FunctionProto:
  case Type::FunctionNoProto:
    return CreateType(cast<FunctionType>(Ty), Unit);
  case Type::ConstantArray:
  case Type::VariableArray:
  case Type::IncompleteArray:
    return CreateType(cast<ArrayType>(Ty), Unit);

  case Type::LValueReference:
    return CreateType(cast<LValueReferenceType>(Ty), Unit);
  case Type::RValueReference:
    return CreateType(cast<RValueReferenceType>(Ty), Unit);

  case Type::MemberPointer:
    return CreateType(cast<MemberPointerType>(Ty), Unit);

  case Type::Atomic:
    return CreateType(cast<AtomicType>(Ty), Unit);

  case Type::TemplateSpecialization:
    return CreateType(cast<TemplateSpecializationType>(Ty), Unit);

  case Type::Auto:
  case Type::Attributed:
  case Type::Elaborated:
  case Type::Paren:
  case Type::SubstTemplateTypeParm:
  case Type::TypeOfExpr:
  case Type::TypeOf:
  case Type::Decltype:
  case Type::UnaryTransform:
  case Type::PackExpansion:
    break;
  }

  llvm_unreachable("type should have been unwrapped!");
}

llvm::DIType *CGDebugInfo::getOrCreateLimitedType(const RecordType *Ty,
                                                  llvm::DIFile *Unit) {
  QualType QTy(Ty, 0);

  auto *T = cast_or_null<llvm::DICompositeTypeBase>(getTypeOrNull(QTy));

  // We may have cached a forward decl when we could have created
  // a non-forward decl. Go ahead and create a non-forward decl
  // now.
  if (T && !T->isForwardDecl())
    return T;

  // Otherwise create the type.
  llvm::DICompositeType *Res = CreateLimitedType(Ty);

  // Propagate members from the declaration to the definition
  // CreateType(const RecordType*) will overwrite this with the members in the
  // correct order if the full type is needed.
  DBuilder.replaceArrays(Res, T ? T->getElements() : llvm::DINodeArray());

  // And update the type cache.
  TypeCache[QTy.getAsOpaquePtr()].reset(Res);
  return Res;
}

// TODO: Currently used for context chains when limiting debug info.
llvm::DICompositeType *CGDebugInfo::CreateLimitedType(const RecordType *Ty) {
  RecordDecl *RD = Ty->getDecl();

  // Get overall information about the record type for the debug info.
  llvm::DIFile *DefUnit = getOrCreateFile(RD->getLocation());
  unsigned Line = getLineNumber(RD->getLocation());
  StringRef RDName = getClassName(RD);

  llvm::DIScope *RDContext =
      getContextDescriptor(cast<Decl>(RD->getDeclContext()));

  // If we ended up creating the type during the context chain construction,
  // just return that.
  auto *T = cast_or_null<llvm::DICompositeType>(
      getTypeOrNull(CGM.getContext().getRecordType(RD)));
  if (T && (!T->isForwardDecl() || !RD->getDefinition()))
    return T;

  // If this is just a forward or incomplete declaration, construct an
  // appropriately marked node and just return it.
  const RecordDecl *D = RD->getDefinition();
  if (!D || !D->isCompleteDefinition())
    return getOrCreateRecordFwdDecl(Ty, RDContext);

  uint64_t Size = CGM.getContext().getTypeSize(Ty);
  uint64_t Align = CGM.getContext().getTypeAlign(Ty);

  SmallString<256> FullName = getUniqueTagTypeName(Ty, CGM, TheCU);

  llvm::DICompositeType *RealDecl = DBuilder.createReplaceableCompositeType(
      getTagForRecord(RD), RDName, RDContext, DefUnit, Line, 0, Size, Align, 0,
      FullName);

  RegionMap[Ty->getDecl()].reset(RealDecl);
  TypeCache[QualType(Ty, 0).getAsOpaquePtr()].reset(RealDecl);

  if (const ClassTemplateSpecializationDecl *TSpecial =
          dyn_cast<ClassTemplateSpecializationDecl>(RD))
    DBuilder.replaceArrays(RealDecl, llvm::DINodeArray(),
                           CollectCXXTemplateParams(TSpecial, DefUnit));
  return RealDecl;
}

void CGDebugInfo::CollectContainingType(const CXXRecordDecl *RD,
                                        llvm::DICompositeType *RealDecl) {
  // A class's primary base or the class itself contains the vtable.
  llvm::DICompositeType *ContainingType = nullptr;
  const ASTRecordLayout &RL = CGM.getContext().getASTRecordLayout(RD);
  if (const CXXRecordDecl *PBase = RL.getPrimaryBase()) {
    // Seek non-virtual primary base root.
    while (1) {
      const ASTRecordLayout &BRL = CGM.getContext().getASTRecordLayout(PBase);
      const CXXRecordDecl *PBT = BRL.getPrimaryBase();
      if (PBT && !BRL.isPrimaryBaseVirtual())
        PBase = PBT;
      else
        break;
    }
    ContainingType = cast<llvm::DICompositeType>(
        getOrCreateType(QualType(PBase->getTypeForDecl(), 0),
                        getOrCreateFile(RD->getLocation())));
  } else if (RD->isDynamicClass())
    ContainingType = RealDecl;

  DBuilder.replaceVTableHolder(RealDecl, ContainingType);
}

llvm::DIType *CGDebugInfo::CreateMemberType(llvm::DIFile *Unit, QualType FType,
                                            StringRef Name, uint64_t *Offset) {
  llvm::DIType *FieldTy = CGDebugInfo::getOrCreateType(FType, Unit);
  uint64_t FieldSize = CGM.getContext().getTypeSize(FType);
  unsigned FieldAlign = CGM.getContext().getTypeAlign(FType);
  llvm::DIType *Ty = DBuilder.createMemberType(Unit, Name, Unit, 0, FieldSize,
                                               FieldAlign, *Offset, 0, FieldTy);
  *Offset += FieldSize;
  return Ty;
}

void CGDebugInfo::collectFunctionDeclProps(GlobalDecl GD, llvm::DIFile *Unit,
                                           StringRef &Name,
                                           StringRef &LinkageName,
                                           llvm::DIScope *&FDContext,
                                           llvm::DINodeArray &TParamsArray,
                                           unsigned &Flags) {
  const FunctionDecl *FD = cast<FunctionDecl>(GD.getDecl());
  Name = getFunctionName(FD);
  // Use mangled name as linkage name for C/C++ functions.
  if (FD->hasPrototype()) {
    LinkageName = CGM.getMangledName(GD);
    Flags |= llvm::DINode::FlagPrototyped;
  }
  // No need to replicate the linkage name if it isn't different from the
  // subprogram name, no need to have it at all unless coverage is enabled or
  // debug is set to more than just line tables.
  if (LinkageName == Name ||
      (!CGM.getCodeGenOpts().EmitGcovArcs &&
       !CGM.getCodeGenOpts().EmitGcovNotes &&
       DebugKind <= CodeGenOptions::DebugLineTablesOnly))
    LinkageName = StringRef();

  if (DebugKind >= CodeGenOptions::LimitedDebugInfo) {
    if (const NamespaceDecl *NSDecl =
        dyn_cast_or_null<NamespaceDecl>(FD->getDeclContext()))
      FDContext = getOrCreateNameSpace(NSDecl);
    else if (const RecordDecl *RDecl =
             dyn_cast_or_null<RecordDecl>(FD->getDeclContext()))
      FDContext = getContextDescriptor(cast<Decl>(RDecl));
    // Collect template parameters.
    TParamsArray = CollectFunctionTemplateParams(FD, Unit);
  }
}

void CGDebugInfo::collectVarDeclProps(const VarDecl *VD, llvm::DIFile *&Unit,
                                      unsigned &LineNo, QualType &T,
                                      StringRef &Name, StringRef &LinkageName,
                                      llvm::DIScope *&VDContext) {
  Unit = getOrCreateFile(VD->getLocation());
  LineNo = getLineNumber(VD->getLocation());

  setLocation(VD->getLocation());

  T = VD->getType();
  if (T->isIncompleteArrayType()) {
    // CodeGen turns int[] into int[1] so we'll do the same here.
    llvm::APInt ConstVal(32, 1);
    QualType ET = CGM.getContext().getAsArrayType(T)->getElementType();

    T = CGM.getContext().getConstantArrayType(ET, ConstVal,
                                              ArrayType::Normal, 0);
  }

  Name = VD->getName();
  if (VD->getDeclContext() && !isa<FunctionDecl>(VD->getDeclContext()) &&
      !isa<ObjCMethodDecl>(VD->getDeclContext()))
    LinkageName = CGM.getMangledName(VD);
  if (LinkageName == Name)
    LinkageName = StringRef();

  // Since we emit declarations (DW_AT_members) for static members, place the
  // definition of those static members in the namespace they were declared in
  // in the source code (the lexical decl context).
  // FIXME: Generalize this for even non-member global variables where the
  // declaration and definition may have different lexical decl contexts, once
  // we have support for emitting declarations of (non-member) global variables.
  const DeclContext *DC = VD->isStaticDataMember() ? VD->getLexicalDeclContext()
                                                   : VD->getDeclContext();
  // When a record type contains an in-line initialization of a static data
  // member, and the record type is marked as __declspec(dllexport), an implicit
  // definition of the member will be created in the record context.  DWARF
  // doesn't seem to have a nice way to describe this in a form that consumers
  // are likely to understand, so fake the "normal" situation of a definition
  // outside the class by putting it in the global scope.
  if (DC->isRecord())
    DC = CGM.getContext().getTranslationUnitDecl();
  VDContext = getContextDescriptor(dyn_cast<Decl>(DC));
}

llvm::DISubprogram *
CGDebugInfo::getFunctionForwardDeclaration(const FunctionDecl *FD) {
  llvm::DINodeArray TParamsArray;
  StringRef Name, LinkageName;
  unsigned Flags = 0;
  SourceLocation Loc = FD->getLocation();
  llvm::DIFile *Unit = getOrCreateFile(Loc);
  llvm::DIScope *DContext = Unit;
  unsigned Line = getLineNumber(Loc);

  collectFunctionDeclProps(FD, Unit, Name, LinkageName, DContext,
                           TParamsArray, Flags);
  // Build function type.
  SmallVector<QualType, 16> ArgTypes;
  for (const ParmVarDecl *Parm: FD->parameters())
    ArgTypes.push_back(Parm->getType());
  QualType FnType = CGM.getContext().getFunctionType(
      FD->getReturnType(), ArgTypes, FunctionProtoType::ExtProtoInfo(),
      ArrayRef<hlsl::ParameterModifier>()); // HLSL Change - parameter modifiers
  llvm::DISubprogram *SP = DBuilder.createTempFunctionFwdDecl(
      DContext, Name, LinkageName, Unit, Line,
      getOrCreateFunctionType(FD, FnType, Unit), !FD->isExternallyVisible(),
      false /*declaration*/, 0, Flags, CGM.getLangOpts().Optimize, nullptr,
      TParamsArray.get(), getFunctionDeclaration(FD));
  const FunctionDecl *CanonDecl = cast<FunctionDecl>(FD->getCanonicalDecl());
  FwdDeclReplaceMap.emplace_back(std::piecewise_construct,
                                 std::make_tuple(CanonDecl),
                                 std::make_tuple(SP));
  return SP;
}

llvm::DIGlobalVariable *
CGDebugInfo::getGlobalVariableForwardDeclaration(const VarDecl *VD) {
  QualType T;
  StringRef Name, LinkageName;
  SourceLocation Loc = VD->getLocation();
  llvm::DIFile *Unit = getOrCreateFile(Loc);
  llvm::DIScope *DContext = Unit;
  unsigned Line = getLineNumber(Loc);

  collectVarDeclProps(VD, Unit, Line, T, Name, LinkageName, DContext);
  auto *GV = DBuilder.createTempGlobalVariableFwdDecl(
      DContext, Name, LinkageName, Unit, Line, getOrCreateType(T, Unit),
      !VD->isExternallyVisible(), nullptr, nullptr);
  FwdDeclReplaceMap.emplace_back(
      std::piecewise_construct,
      std::make_tuple(cast<VarDecl>(VD->getCanonicalDecl())),
      std::make_tuple(static_cast<llvm::Metadata *>(GV)));
  return GV;
}

llvm::DINode *CGDebugInfo::getDeclarationOrDefinition(const Decl *D) {
  // We only need a declaration (not a definition) of the type - so use whatever
  // we would otherwise do to get a type for a pointee. (forward declarations in
  // limited debug info, full definitions (if the type definition is available)
  // in unlimited debug info)
  if (const TypeDecl *TD = dyn_cast<TypeDecl>(D))
    return getOrCreateType(CGM.getContext().getTypeDeclType(TD),
                           getOrCreateFile(TD->getLocation()));
  auto I = DeclCache.find(D->getCanonicalDecl());

  if (I != DeclCache.end())
    return dyn_cast_or_null<llvm::DINode>(I->second);

  // No definition for now. Emit a forward definition that might be
  // merged with a potential upcoming definition.
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D))
    return getFunctionForwardDeclaration(FD);
  else if (const auto *VD = dyn_cast<VarDecl>(D))
    return getGlobalVariableForwardDeclaration(VD);

  return nullptr;
}

llvm::DISubprogram *CGDebugInfo::getFunctionDeclaration(const Decl *D) {
  if (!D || DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return nullptr;

  const FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
  if (!FD)
    return nullptr;

  // Setup context.
  auto *S = getContextDescriptor(cast<Decl>(D->getDeclContext()));

  auto MI = SPCache.find(FD->getCanonicalDecl());
  if (MI == SPCache.end()) {
    if (const CXXMethodDecl *MD =
            dyn_cast<CXXMethodDecl>(FD->getCanonicalDecl())) {
      return CreateCXXMemberFunction(MD, getOrCreateFile(MD->getLocation()),
                                     cast<llvm::DICompositeType>(S));
    }
  }
  if (MI != SPCache.end()) {
    auto *SP = dyn_cast_or_null<llvm::DISubprogram>(MI->second);
    if (SP && !SP->isDefinition())
      return SP;
  }

  for (auto NextFD : FD->redecls()) {
    auto MI = SPCache.find(NextFD->getCanonicalDecl());
    if (MI != SPCache.end()) {
      auto *SP = dyn_cast_or_null<llvm::DISubprogram>(MI->second);
      if (SP && !SP->isDefinition())
        return SP;
    }
  }
  return nullptr;
}

// getOrCreateFunctionType - Construct type. If it is a c++ method, include
// implicit parameter "this".
llvm::DISubroutineType *CGDebugInfo::getOrCreateFunctionType(const Decl *D,
                                                             QualType FnType,
                                                             llvm::DIFile *F) {
  if (!D || DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    // Create fake but valid subroutine type. Otherwise -verify would fail, and
    // subprogram DIE will miss DW_AT_decl_file and DW_AT_decl_line fields.
    return DBuilder.createSubroutineType(F,
                                         DBuilder.getOrCreateTypeArray(None));

  if (const CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(D))
    return getOrCreateMethodType(Method, F);
#if 0 // HLSL Change - no ObjC support
  if (const ObjCMethodDecl *OMethod = dyn_cast<ObjCMethodDecl>(D)) {
    // Add "self" and "_cmd"
    SmallVector<llvm::Metadata *, 16> Elts;

    // First element is always return type. For 'void' functions it is NULL.
    QualType ResultTy = OMethod->getReturnType();

    // Replace the instancetype keyword with the actual type.
    if (ResultTy == CGM.getContext().getObjCInstanceType())
      ResultTy = CGM.getContext().getPointerType(
          QualType(OMethod->getClassInterface()->getTypeForDecl(), 0));

    Elts.push_back(getOrCreateType(ResultTy, F));
    // "self" pointer is always first argument.
    QualType SelfDeclTy = OMethod->getSelfDecl()->getType();
    Elts.push_back(CreateSelfType(SelfDeclTy, getOrCreateType(SelfDeclTy, F)));
    // "_cmd" pointer is always second argument.
    Elts.push_back(DBuilder.createArtificialType(
        getOrCreateType(OMethod->getCmdDecl()->getType(), F)));
    // Get rest of the arguments.
    for (const auto *PI : OMethod->params())
      Elts.push_back(getOrCreateType(PI->getType(), F));
    // Variadic methods need a special marker at the end of the type list.
    if (OMethod->isVariadic())
      Elts.push_back(DBuilder.createUnspecifiedParameter());

    llvm::DITypeRefArray EltTypeArray = DBuilder.getOrCreateTypeArray(Elts);
    return DBuilder.createSubroutineType(F, EltTypeArray);
  }
#endif // HLSL Change - no ObjC support

  // Handle variadic function types; they need an additional
  // unspecified parameter.
  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
    if (FD->isVariadic()) {
      SmallVector<llvm::Metadata *, 16> EltTys;
      EltTys.push_back(getOrCreateType(FD->getReturnType(), F));
      if (const FunctionProtoType *FPT = dyn_cast<FunctionProtoType>(FnType))
        for (unsigned i = 0, e = FPT->getNumParams(); i != e; ++i)
          EltTys.push_back(getOrCreateType(FPT->getParamType(i), F));
      EltTys.push_back(DBuilder.createUnspecifiedParameter());
      llvm::DITypeRefArray EltTypeArray = DBuilder.getOrCreateTypeArray(EltTys);
      return DBuilder.createSubroutineType(F, EltTypeArray);
    }

  return cast<llvm::DISubroutineType>(getOrCreateType(FnType, F));
}

void CGDebugInfo::EmitFunctionStart(GlobalDecl GD, SourceLocation Loc,
                                    SourceLocation ScopeLoc, QualType FnType,
                                    llvm::Function *Fn, CGBuilderTy &Builder) {

  StringRef Name;
  StringRef LinkageName;

  FnBeginRegionCount.push_back(LexicalBlockStack.size());

  const Decl *D = GD.getDecl();
  bool HasDecl = (D != nullptr);

  unsigned Flags = 0;
  llvm::DIFile *Unit = getOrCreateFile(Loc);
  llvm::DIScope *FDContext = Unit;
  llvm::DINodeArray TParamsArray;
  if (!HasDecl) {
    // Use llvm function name.
    LinkageName = Fn->getName();
  } else if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    // If there is a subprogram for this function available then use it.
    auto FI = SPCache.find(FD->getCanonicalDecl());
    if (FI != SPCache.end()) {
      auto *SP = dyn_cast_or_null<llvm::DISubprogram>(FI->second);
      if (SP && SP->isDefinition()) {
        LexicalBlockStack.emplace_back(SP);
        RegionMap[D].reset(SP);
        return;
      }
    }
    collectFunctionDeclProps(GD, Unit, Name, LinkageName, FDContext,
                             TParamsArray, Flags);
#if 0 // HLSL Change - no ObjC support
  } else if (const ObjCMethodDecl *OMD = dyn_cast<ObjCMethodDecl>(D)) {
    Name = getObjCMethodName(OMD);
    Flags |= llvm::DINode::FlagPrototyped;
#endif // HLSL Change - no ObjC support
  } else {
    // Use llvm function name.
    Name = Fn->getName();
    Flags |= llvm::DINode::FlagPrototyped;
  }
  if (!Name.empty() && Name[0] == '\01')
    Name = Name.substr(1);

  if (!HasDecl || D->isImplicit()) {
    Flags |= llvm::DINode::FlagArtificial;
    // Artificial functions without a location should not silently reuse CurLoc.
    if (Loc.isInvalid())
      CurLoc = SourceLocation();
  }
  unsigned LineNo = getLineNumber(Loc);
  unsigned ScopeLine = getLineNumber(ScopeLoc);

  // FIXME: The function declaration we're constructing here is mostly reusing
  // declarations from CXXMethodDecl and not constructing new ones for arbitrary
  // FunctionDecls. When/if we fix this we can have FDContext be TheCU/null for
  // all subprograms instead of the actual context since subprogram definitions
  // are emitted as CU level entities by the backend.
  llvm::DISubprogram *SP = DBuilder.createFunction(
      FDContext, Name, LinkageName, Unit, LineNo,
      getOrCreateFunctionType(D, FnType, Unit), Fn->hasInternalLinkage(),
      true /*definition*/, ScopeLine, Flags, CGM.getLangOpts().Optimize, Fn,
      TParamsArray.get(), getFunctionDeclaration(D));
  // We might get here with a VarDecl in the case we're generating
  // code for the initialization of globals. Do not record these decls
  // as they will overwrite the actual VarDecl Decl in the cache.
  if (HasDecl && isa<FunctionDecl>(D))
    DeclCache[D->getCanonicalDecl()].reset(static_cast<llvm::Metadata *>(SP));

  // Push the function onto the lexical block stack.
  LexicalBlockStack.emplace_back(SP);

  if (HasDecl)
    RegionMap[D].reset(SP);
}

void CGDebugInfo::EmitLocation(CGBuilderTy &Builder, SourceLocation Loc) {
  // Update our current location
  setLocation(Loc);

  if (CurLoc.isInvalid() || CurLoc.isMacroID())
    return;

  llvm::MDNode *Scope = LexicalBlockStack.back();
  Builder.SetCurrentDebugLocation(llvm::DebugLoc::get(
      getLineNumber(CurLoc), getColumnNumber(CurLoc), Scope));
}

void CGDebugInfo::CreateLexicalBlock(SourceLocation Loc) {
  llvm::MDNode *Back = nullptr;
  if (!LexicalBlockStack.empty())
    Back = LexicalBlockStack.back().get();
  LexicalBlockStack.emplace_back(DBuilder.createLexicalBlock(
      cast<llvm::DIScope>(Back), getOrCreateFile(CurLoc), getLineNumber(CurLoc),
      getColumnNumber(CurLoc)));
}

void CGDebugInfo::EmitLexicalBlockStart(CGBuilderTy &Builder,
                                        SourceLocation Loc) {
  // Set our current location.
  setLocation(Loc);

  // Emit a line table change for the current location inside the new scope.
  Builder.SetCurrentDebugLocation(llvm::DebugLoc::get(
      getLineNumber(Loc), getColumnNumber(Loc), LexicalBlockStack.back()));

  if (DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return;

  // Create a new lexical block and push it on the stack.
  CreateLexicalBlock(Loc);
}

void CGDebugInfo::EmitLexicalBlockEnd(CGBuilderTy &Builder,
                                      SourceLocation Loc) {
  assert(!LexicalBlockStack.empty() && "Region stack mismatch, stack empty!");

  // Provide an entry in the line table for the end of the block.
  EmitLocation(Builder, Loc);

  if (DebugKind <= CodeGenOptions::DebugLineTablesOnly)
    return;

  LexicalBlockStack.pop_back();
}

void CGDebugInfo::EmitFunctionEnd(CGBuilderTy &Builder) {
  assert(!LexicalBlockStack.empty() && "Region stack mismatch, stack empty!");
  unsigned RCount = FnBeginRegionCount.back();
  assert(RCount <= LexicalBlockStack.size() && "Region stack mismatch");

  // Pop all regions for this function.
  while (LexicalBlockStack.size() != RCount) {
    // Provide an entry in the line table for the end of the block.
    EmitLocation(Builder, CurLoc);
    LexicalBlockStack.pop_back();
  }
  FnBeginRegionCount.pop_back();
}

llvm::DIType *CGDebugInfo::EmitTypeForVarWithBlocksAttr(const VarDecl *VD,
                                                        uint64_t *XOffset) {

  SmallVector<llvm::Metadata *, 5> EltTys;
  QualType FType;
  uint64_t FieldSize, FieldOffset;
  unsigned FieldAlign;

  llvm::DIFile *Unit = getOrCreateFile(VD->getLocation());
  QualType Type = VD->getType();

  FieldOffset = 0;
  FType = CGM.getContext().getPointerType(CGM.getContext().VoidTy);
  EltTys.push_back(CreateMemberType(Unit, FType, "__isa", &FieldOffset));
  EltTys.push_back(CreateMemberType(Unit, FType, "__forwarding", &FieldOffset));
  FType = CGM.getContext().IntTy;
  EltTys.push_back(CreateMemberType(Unit, FType, "__flags", &FieldOffset));
  EltTys.push_back(CreateMemberType(Unit, FType, "__size", &FieldOffset));

#if 0 // HLSL Change - no ObjC or block support
  bool HasCopyAndDispose = CGM.getContext().BlockRequiresCopying(Type, VD);
  if (HasCopyAndDispose) {
    FType = CGM.getContext().getPointerType(CGM.getContext().VoidTy);
    EltTys.push_back(
        CreateMemberType(Unit, FType, "__copy_helper", &FieldOffset));
    EltTys.push_back(
        CreateMemberType(Unit, FType, "__destroy_helper", &FieldOffset));
  }
  bool HasByrefExtendedLayout;
  Qualifiers::ObjCLifetime Lifetime;
  if (CGM.getContext().getByrefLifetime(Type, Lifetime,
                                        HasByrefExtendedLayout) &&
      HasByrefExtendedLayout) {
    FType = CGM.getContext().getPointerType(CGM.getContext().VoidTy);
    EltTys.push_back(
        CreateMemberType(Unit, FType, "__byref_variable_layout", &FieldOffset));
  }
#endif // HLSL Change - no ObjC or block support

  CharUnits Align = CGM.getContext().getDeclAlign(VD);
  if (Align > CGM.getContext().toCharUnitsFromBits(
                  CGM.getTarget().getPointerAlign(0))) {
    CharUnits FieldOffsetInBytes =
        CGM.getContext().toCharUnitsFromBits(FieldOffset);
    CharUnits AlignedOffsetInBytes =
        FieldOffsetInBytes.RoundUpToAlignment(Align);
    CharUnits NumPaddingBytes = AlignedOffsetInBytes - FieldOffsetInBytes;

    if (NumPaddingBytes.isPositive()) {
      llvm::APInt pad(32, NumPaddingBytes.getQuantity());
      FType = CGM.getContext().getConstantArrayType(CGM.getContext().CharTy,
                                                    pad, ArrayType::Normal, 0);
      EltTys.push_back(CreateMemberType(Unit, FType, "", &FieldOffset));
    }
  }

  FType = Type;
  llvm::DIType *FieldTy = getOrCreateType(FType, Unit);
  FieldSize = CGM.getContext().getTypeSize(FType);
  FieldAlign = CGM.getContext().toBits(Align);

  *XOffset = FieldOffset;
  FieldTy = DBuilder.createMemberType(Unit, VD->getName(), Unit, 0, FieldSize,
                                      FieldAlign, FieldOffset, 0, FieldTy);
  EltTys.push_back(FieldTy);
  FieldOffset += FieldSize;

  llvm::DINodeArray Elements = DBuilder.getOrCreateArray(EltTys);

  unsigned Flags = llvm::DINode::FlagBlockByrefStruct;

  return DBuilder.createStructType(Unit, "", Unit, 0, FieldOffset, 0, Flags,
                                   nullptr, Elements);
}

void CGDebugInfo::EmitDeclare(const VarDecl *VD, llvm::dwarf::Tag Tag,
                              llvm::Value *Storage, unsigned ArgNo,
                              CGBuilderTy &Builder) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  assert(!LexicalBlockStack.empty() && "Region stack mismatch, stack empty!");

  bool Unwritten =
      VD->isImplicit() || (isa<Decl>(VD->getDeclContext()) &&
                           cast<Decl>(VD->getDeclContext())->isImplicit());
  llvm::DIFile *Unit = nullptr;
  if (!Unwritten)
    Unit = getOrCreateFile(VD->getLocation());
  llvm::DIType *Ty;
  uint64_t XOffset = 0;
  if (VD->hasAttr<BlocksAttr>())
    Ty = EmitTypeForVarWithBlocksAttr(VD, &XOffset);
  else
    Ty = getOrCreateType(VD->getType(), Unit);

  // If there is no debug info for this type then do not emit debug info
  // for this variable.
  if (!Ty)
    return;

  // Get location information.
  unsigned Line = 0;
  unsigned Column = 0;
  if (!Unwritten) {
    Line = getLineNumber(VD->getLocation());
    Column = getColumnNumber(VD->getLocation());
  }
  SmallVector<int64_t, 9> Expr;
  unsigned Flags = 0;
  if (VD->isImplicit())
    Flags |= llvm::DINode::FlagArtificial;
  // If this is the first argument and it is implicit then
  // give it an object pointer flag.
  // FIXME: There has to be a better way to do this, but for static
  // functions there won't be an implicit param at arg1 and
  // otherwise it is 'self' or 'this'.
  if (isa<ImplicitParamDecl>(VD) && ArgNo == 1)
    Flags |= llvm::DINode::FlagObjectPointer;
  if (llvm::Argument *Arg = dyn_cast<llvm::Argument>(Storage))
    if (Arg->getType()->isPointerTy() && !Arg->hasByValAttr() &&
        !VD->getType()->isPointerType())
      Expr.push_back(llvm::dwarf::DW_OP_deref);

  auto *Scope = cast<llvm::DIScope>(LexicalBlockStack.back());

  StringRef Name = VD->getName();
  if (!Name.empty()) {
    if (VD->hasAttr<BlocksAttr>()) {
      CharUnits offset = CharUnits::fromQuantity(32);
      Expr.push_back(llvm::dwarf::DW_OP_plus);
      // offset of __forwarding field
      offset = CGM.getContext().toCharUnitsFromBits(
          CGM.getTarget().getPointerWidth(0));
      Expr.push_back(offset.getQuantity());
      Expr.push_back(llvm::dwarf::DW_OP_deref);
      Expr.push_back(llvm::dwarf::DW_OP_plus);
      // offset of x field
      offset = CGM.getContext().toCharUnitsFromBits(XOffset);
      Expr.push_back(offset.getQuantity());

      // Create the descriptor for the variable.
      auto *D = DBuilder.createLocalVariable(Tag, Scope, VD->getName(), Unit,
                                             Line, Ty, ArgNo);

      // Insert an llvm.dbg.declare into the current block.
      DBuilder.insertDeclare(Storage, D, DBuilder.createExpression(Expr),
                             llvm::DebugLoc::get(Line, Column, Scope),
                             Builder.GetInsertBlock());
      return;
    } else if (isa<VariableArrayType>(VD->getType()))
      Expr.push_back(llvm::dwarf::DW_OP_deref);
  } else if (const RecordType *RT = dyn_cast<RecordType>(VD->getType())) {
    // If VD is an anonymous union then Storage represents value for
    // all union fields.
    const RecordDecl *RD = cast<RecordDecl>(RT->getDecl());
    if (RD->isUnion() && RD->isAnonymousStructOrUnion()) {
      // GDB has trouble finding local variables in anonymous unions, so we emit
      // artifical local variables for each of the members.
      //
      // FIXME: Remove this code as soon as GDB supports this.
      // The debug info verifier in LLVM operates based on the assumption that a
      // variable has the same size as its storage and we had to disable the check
      // for artificial variables.
      for (const auto *Field : RD->fields()) {
        llvm::DIType *FieldTy = getOrCreateType(Field->getType(), Unit);
        StringRef FieldName = Field->getName();

        // Ignore unnamed fields. Do not ignore unnamed records.
        if (FieldName.empty() && !isa<RecordType>(Field->getType()))
          continue;

        // Use VarDecl's Tag, Scope and Line number.
        auto *D = DBuilder.createLocalVariable(
            Tag, Scope, FieldName, Unit, Line, FieldTy,
            CGM.getLangOpts().Optimize, Flags | llvm::DINode::FlagArtificial,
            ArgNo);

        // Insert an llvm.dbg.declare into the current block.
        DBuilder.insertDeclare(Storage, D, DBuilder.createExpression(Expr),
                               llvm::DebugLoc::get(Line, Column, Scope),
                               Builder.GetInsertBlock());
      }
    }
  }

  // Create the descriptor for the variable.
  auto *D =
      DBuilder.createLocalVariable(Tag, Scope, Name, Unit, Line, Ty,
                                   CGM.getLangOpts().Optimize, Flags, ArgNo);

  // Insert an llvm.dbg.declare into the current block.
  DBuilder.insertDeclare(Storage, D, DBuilder.createExpression(Expr),
                         llvm::DebugLoc::get(Line, Column, Scope),
                         Builder.GetInsertBlock());
}

void CGDebugInfo::EmitDeclareOfAutoVariable(const VarDecl *VD,
                                            llvm::Value *Storage,
                                            CGBuilderTy &Builder) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  EmitDeclare(VD, llvm::dwarf::DW_TAG_auto_variable, Storage, 0, Builder);
}

llvm::DIType *CGDebugInfo::CreateSelfType(const QualType &QualTy,
                                          llvm::DIType *Ty) {
  llvm::DIType *CachedTy = getTypeOrNull(QualTy);
  if (CachedTy)
    Ty = CachedTy;
  return DBuilder.createObjectPointerType(Ty);
}

void CGDebugInfo::EmitDeclareOfBlockDeclRefVariable(
    const VarDecl *VD, llvm::Value *Storage, CGBuilderTy &Builder,
    const CGBlockInfo &blockInfo, llvm::Instruction *InsertPoint) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  assert(!LexicalBlockStack.empty() && "Region stack mismatch, stack empty!");

  if (Builder.GetInsertBlock() == nullptr)
    return;

  bool isByRef = VD->hasAttr<BlocksAttr>();

  uint64_t XOffset = 0;
  llvm::DIFile *Unit = getOrCreateFile(VD->getLocation());
  llvm::DIType *Ty;
  if (isByRef)
    Ty = EmitTypeForVarWithBlocksAttr(VD, &XOffset);
  else
    Ty = getOrCreateType(VD->getType(), Unit);

  // Self is passed along as an implicit non-arg variable in a
  // block. Mark it as the object pointer.
  if (isa<ImplicitParamDecl>(VD) && VD->getName() == "self")
    Ty = CreateSelfType(VD->getType(), Ty);

  // Get location information.
  unsigned Line = getLineNumber(VD->getLocation());
  unsigned Column = getColumnNumber(VD->getLocation());

  const llvm::DataLayout &target = CGM.getDataLayout();

  CharUnits offset = CharUnits::fromQuantity(
      target.getStructLayout(blockInfo.StructureType)
          ->getElementOffset(blockInfo.getCapture(VD).getIndex()));

  SmallVector<int64_t, 9> addr;
  if (isa<llvm::AllocaInst>(Storage))
    addr.push_back(llvm::dwarf::DW_OP_deref);
  addr.push_back(llvm::dwarf::DW_OP_plus);
  addr.push_back(offset.getQuantity());
  if (isByRef) {
    addr.push_back(llvm::dwarf::DW_OP_deref);
    addr.push_back(llvm::dwarf::DW_OP_plus);
    // offset of __forwarding field
    offset =
        CGM.getContext().toCharUnitsFromBits(target.getPointerSizeInBits(0));
    addr.push_back(offset.getQuantity());
    addr.push_back(llvm::dwarf::DW_OP_deref);
    addr.push_back(llvm::dwarf::DW_OP_plus);
    // offset of x field
    offset = CGM.getContext().toCharUnitsFromBits(XOffset);
    addr.push_back(offset.getQuantity());
  }

  // Create the descriptor for the variable.
  auto *D = DBuilder.createLocalVariable(
      llvm::dwarf::DW_TAG_auto_variable,
      cast<llvm::DILocalScope>(LexicalBlockStack.back()), VD->getName(), Unit,
      Line, Ty);

  // Insert an llvm.dbg.declare into the current block.
  auto DL = llvm::DebugLoc::get(Line, Column, LexicalBlockStack.back());
  if (InsertPoint)
    DBuilder.insertDeclare(Storage, D, DBuilder.createExpression(addr), DL,
                           InsertPoint);
  else
    DBuilder.insertDeclare(Storage, D, DBuilder.createExpression(addr), DL,
                           Builder.GetInsertBlock());
}

void CGDebugInfo::EmitDeclareOfArgVariable(const VarDecl *VD, llvm::Value *AI,
                                           unsigned ArgNo,
                                           CGBuilderTy &Builder) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  EmitDeclare(VD, llvm::dwarf::DW_TAG_arg_variable, AI, ArgNo, Builder);
}

#if 0 // HLSL Change - no block support
namespace {
struct BlockLayoutChunk {
  uint64_t OffsetInBits;
  const BlockDecl::Capture *Capture;
};
bool operator<(const BlockLayoutChunk &l, const BlockLayoutChunk &r) {
  return l.OffsetInBits < r.OffsetInBits;
}
}

void CGDebugInfo::EmitDeclareOfBlockLiteralArgVariable(const CGBlockInfo &block,
                                                       llvm::Value *Arg,
                                                       unsigned ArgNo,
                                                       llvm::Value *LocalAddr,
                                                       CGBuilderTy &Builder) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  ASTContext &C = CGM.getContext();
  const BlockDecl *blockDecl = block.getBlockDecl();

  // Collect some general information about the block's location.
  SourceLocation loc = blockDecl->getCaretLocation();
  llvm::DIFile *tunit = getOrCreateFile(loc);
  unsigned line = getLineNumber(loc);
  unsigned column = getColumnNumber(loc);

  // Build the debug-info type for the block literal.
  getContextDescriptor(cast<Decl>(blockDecl->getDeclContext()));

  const llvm::StructLayout *blockLayout =
      CGM.getDataLayout().getStructLayout(block.StructureType);

  SmallVector<llvm::Metadata *, 16> fields;
  fields.push_back(createFieldType("__isa", C.VoidPtrTy, 0, loc, AS_public,
                                   blockLayout->getElementOffsetInBits(0),
                                   tunit, tunit));
  fields.push_back(createFieldType("__flags", C.IntTy, 0, loc, AS_public,
                                   blockLayout->getElementOffsetInBits(1),
                                   tunit, tunit));
  fields.push_back(createFieldType("__reserved", C.IntTy, 0, loc, AS_public,
                                   blockLayout->getElementOffsetInBits(2),
                                   tunit, tunit));
  auto *FnTy = block.getBlockExpr()->getFunctionType();
  auto FnPtrType = CGM.getContext().getPointerType(FnTy->desugar());
  fields.push_back(createFieldType("__FuncPtr", FnPtrType, 0, loc, AS_public,
                                   blockLayout->getElementOffsetInBits(3),
                                   tunit, tunit));
  fields.push_back(createFieldType(
      "__descriptor", C.getPointerType(block.NeedsCopyDispose
                                           ? C.getBlockDescriptorExtendedType()
                                           : C.getBlockDescriptorType()),
      0, loc, AS_public, blockLayout->getElementOffsetInBits(4), tunit, tunit));

  // We want to sort the captures by offset, not because DWARF
  // requires this, but because we're paranoid about debuggers.
  SmallVector<BlockLayoutChunk, 8> chunks;

  // 'this' capture.
  if (blockDecl->capturesCXXThis()) {
    BlockLayoutChunk chunk;
    chunk.OffsetInBits =
        blockLayout->getElementOffsetInBits(block.CXXThisIndex);
    chunk.Capture = nullptr;
    chunks.push_back(chunk);
  }

  // Variable captures.
  for (const auto &capture : blockDecl->captures()) {
    const VarDecl *variable = capture.getVariable();
    const CGBlockInfo::Capture &captureInfo = block.getCapture(variable);

    // Ignore constant captures.
    if (captureInfo.isConstant())
      continue;

    BlockLayoutChunk chunk;
    chunk.OffsetInBits =
        blockLayout->getElementOffsetInBits(captureInfo.getIndex());
    chunk.Capture = &capture;
    chunks.push_back(chunk);
  }

  // Sort by offset.
  llvm::array_pod_sort(chunks.begin(), chunks.end());

  for (SmallVectorImpl<BlockLayoutChunk>::iterator i = chunks.begin(),
                                                   e = chunks.end();
       i != e; ++i) {
    uint64_t offsetInBits = i->OffsetInBits;
    const BlockDecl::Capture *capture = i->Capture;

    // If we have a null capture, this must be the C++ 'this' capture.
    if (!capture) {
      const CXXMethodDecl *method =
          cast<CXXMethodDecl>(blockDecl->getNonClosureContext());
      QualType type = method->getThisType(C);

      fields.push_back(createFieldType("this", type, 0, loc, AS_public,
                                       offsetInBits, tunit, tunit));
      continue;
    }

    const VarDecl *variable = capture->getVariable();
    StringRef name = variable->getName();

    llvm::DIType *fieldType;
    if (capture->isByRef()) {
      TypeInfo PtrInfo = C.getTypeInfo(C.VoidPtrTy);

      // FIXME: this creates a second copy of this type!
      uint64_t xoffset;
      fieldType = EmitTypeForVarWithBlocksAttr(variable, &xoffset);
      fieldType = DBuilder.createPointerType(fieldType, PtrInfo.Width);
      fieldType =
          DBuilder.createMemberType(tunit, name, tunit, line, PtrInfo.Width,
                                    PtrInfo.Align, offsetInBits, 0, fieldType);
    } else {
      fieldType = createFieldType(name, variable->getType(), 0, loc, AS_public,
                                  offsetInBits, tunit, tunit);
    }
    fields.push_back(fieldType);
  }

  SmallString<36> typeName;
  llvm::raw_svector_ostream(typeName) << "__block_literal_"
                                      << CGM.getUniqueBlockCount();

  llvm::DINodeArray fieldsArray = DBuilder.getOrCreateArray(fields);

  llvm::DIType *type = DBuilder.createStructType(
      tunit, typeName.str(), tunit, line,
      CGM.getContext().toBits(block.BlockSize),
      CGM.getContext().toBits(block.BlockAlign), 0, nullptr, fieldsArray);
  type = DBuilder.createPointerType(type, CGM.PointerWidthInBits);

  // Get overall information about the block.
  unsigned flags = llvm::DINode::FlagArtificial;
  auto *scope = cast<llvm::DILocalScope>(LexicalBlockStack.back());

  // Create the descriptor for the parameter.
  auto *debugVar = DBuilder.createLocalVariable(
      llvm::dwarf::DW_TAG_arg_variable, scope, Arg->getName(), tunit, line,
      type, CGM.getLangOpts().Optimize, flags, ArgNo);

  if (LocalAddr) {
    // Insert an llvm.dbg.value into the current block.
    DBuilder.insertDbgValueIntrinsic(
        LocalAddr, 0, debugVar, DBuilder.createExpression(),
        llvm::DebugLoc::get(line, column, scope), Builder.GetInsertBlock());
  }

  // Insert an llvm.dbg.declare into the current block.
  DBuilder.insertDeclare(Arg, debugVar, DBuilder.createExpression(),
                         llvm::DebugLoc::get(line, column, scope),
                         Builder.GetInsertBlock());
}
#endif // HLSL Change - no block support

llvm::DIDerivedType *
CGDebugInfo::getOrCreateStaticDataMemberDeclarationOrNull(const VarDecl *D) {
  if (!D->isStaticDataMember())
    return nullptr;

  auto MI = StaticDataMemberCache.find(D->getCanonicalDecl());
  if (MI != StaticDataMemberCache.end()) {
    assert(MI->second && "Static data member declaration should still exist");
    return cast<llvm::DIDerivedType>(MI->second);
  }

  // If the member wasn't found in the cache, lazily construct and add it to the
  // type (used when a limited form of the type is emitted).
  auto DC = D->getDeclContext();
  auto *Ctxt =
      cast<llvm::DICompositeType>(getContextDescriptor(cast<Decl>(DC)));
  return CreateRecordStaticField(D, Ctxt, cast<RecordDecl>(DC));
}

llvm::DIGlobalVariable *CGDebugInfo::CollectAnonRecordDecls(
    const RecordDecl *RD, llvm::DIFile *Unit, unsigned LineNo,
    StringRef LinkageName, llvm::GlobalVariable *Var, llvm::DIScope *DContext) {
  llvm::DIGlobalVariable *GV = nullptr;

  for (const auto *Field : RD->fields()) {
    llvm::DIType *FieldTy = getOrCreateType(Field->getType(), Unit);
    StringRef FieldName = Field->getName();

    // Ignore unnamed fields, but recurse into anonymous records.
    if (FieldName.empty()) {
      const RecordType *RT = dyn_cast<RecordType>(Field->getType());
      if (RT)
        GV = CollectAnonRecordDecls(RT->getDecl(), Unit, LineNo, LinkageName,
                                    Var, DContext);
      continue;
    }
    // Use VarDecl's Tag, Scope and Line number.
    GV = DBuilder.createGlobalVariable(DContext, FieldName, LinkageName, Unit,
                                       LineNo, FieldTy,
                                       Var->hasInternalLinkage(), Var, nullptr);
  }
  return GV;
}

void CGDebugInfo::EmitGlobalVariable(llvm::GlobalVariable *Var,
                                     const VarDecl *D) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  // Create global variable debug descriptor.
  llvm::DIFile *Unit = nullptr;
  llvm::DIScope *DContext = nullptr;
  unsigned LineNo;
  StringRef DeclName, LinkageName;
  QualType T;
  collectVarDeclProps(D, Unit, LineNo, T, DeclName, LinkageName, DContext);

  // Attempt to store one global variable for the declaration - even if we
  // emit a lot of fields.
  llvm::DIGlobalVariable *GV = nullptr;

  // If this is an anonymous union then we'll want to emit a global
  // variable for each member of the anonymous union so that it's possible
  // to find the name of any field in the union.
  if (T->isUnionType() && DeclName.empty()) {
    const RecordDecl *RD = cast<RecordType>(T)->getDecl();
    assert(RD->isAnonymousStructOrUnion() &&
           "unnamed non-anonymous struct or union?");
    GV = CollectAnonRecordDecls(RD, Unit, LineNo, LinkageName, Var, DContext);
  } else {
    GV = DBuilder.createGlobalVariable(
        DContext, DeclName, LinkageName, Unit, LineNo, getOrCreateType(T, Unit),
        Var->hasInternalLinkage(), Var,
        getOrCreateStaticDataMemberDeclarationOrNull(D));
  }
  DeclCache[D->getCanonicalDecl()].reset(static_cast<llvm::Metadata *>(GV));
}

void CGDebugInfo::EmitGlobalVariable(const ValueDecl *VD,
                                     llvm::Constant *Init) {
  assert(DebugKind >= CodeGenOptions::LimitedDebugInfo);
  // Create the descriptor for the variable.
  llvm::DIFile *Unit = getOrCreateFile(VD->getLocation());
  StringRef Name = VD->getName();
  llvm::DIType *Ty = getOrCreateType(VD->getType(), Unit);
  if (const EnumConstantDecl *ECD = dyn_cast<EnumConstantDecl>(VD)) {
    const EnumDecl *ED = cast<EnumDecl>(ECD->getDeclContext());
    assert(isa<EnumType>(ED->getTypeForDecl()) && "Enum without EnumType?");
    Ty = getOrCreateType(QualType(ED->getTypeForDecl(), 0), Unit);
  }
  // Do not use global variables for enums.
  //
  // FIXME: why not?
  if (Ty->getTag() == llvm::dwarf::DW_TAG_enumeration_type)
    return;
  // Do not emit separate definitions for function local const/statics.
  if (isa<FunctionDecl>(VD->getDeclContext()))
    return;
  VD = cast<ValueDecl>(VD->getCanonicalDecl());
  auto *VarD = cast<VarDecl>(VD);
  if (VarD->isStaticDataMember()) {
    auto *RD = cast<RecordDecl>(VarD->getDeclContext());
    getContextDescriptor(RD);
    // Ensure that the type is retained even though it's otherwise unreferenced.
    RetainedTypes.push_back(
        CGM.getContext().getRecordType(RD).getAsOpaquePtr());
    return;
  }

  llvm::DIScope *DContext =
      getContextDescriptor(dyn_cast<Decl>(VD->getDeclContext()));

  auto &GV = DeclCache[VD];
  if (GV)
    return;
  GV.reset(DBuilder.createGlobalVariable(
      DContext, Name, StringRef(), Unit, getLineNumber(VD->getLocation()), Ty,
      true, Init, getOrCreateStaticDataMemberDeclarationOrNull(VarD)));
}

llvm::DIScope *CGDebugInfo::getCurrentContextDescriptor(const Decl *D) {
  if (!LexicalBlockStack.empty())
    return LexicalBlockStack.back();
  return getContextDescriptor(D);
}

void CGDebugInfo::EmitUsingDirective(const UsingDirectiveDecl &UD) {
  if (CGM.getCodeGenOpts().getDebugInfo() < CodeGenOptions::LimitedDebugInfo)
    return;
  DBuilder.createImportedModule(
      getCurrentContextDescriptor(cast<Decl>(UD.getDeclContext())),
      getOrCreateNameSpace(UD.getNominatedNamespace()),
      getLineNumber(UD.getLocation()));
}

void CGDebugInfo::EmitUsingDecl(const UsingDecl &UD) {
  if (CGM.getCodeGenOpts().getDebugInfo() < CodeGenOptions::LimitedDebugInfo)
    return;
  assert(UD.shadow_size() &&
         "We shouldn't be codegening an invalid UsingDecl containing no decls");
  // Emitting one decl is sufficient - debuggers can detect that this is an
  // overloaded name & provide lookup for all the overloads.
  const UsingShadowDecl &USD = **UD.shadow_begin();
  if (llvm::DINode *Target =
          getDeclarationOrDefinition(USD.getUnderlyingDecl()))
    DBuilder.createImportedDeclaration(
        getCurrentContextDescriptor(cast<Decl>(USD.getDeclContext())), Target,
        getLineNumber(USD.getLocation()));
}

void CGDebugInfo::EmitImportDecl(const ImportDecl &ID) {
  auto *Reader = CGM.getContext().getExternalSource();
  auto Info = Reader->getSourceDescriptor(*ID.getImportedModule());
  DBuilder.createImportedDeclaration(
    getCurrentContextDescriptor(cast<Decl>(ID.getDeclContext())),
                                getOrCreateModuleRef(Info),
                                getLineNumber(ID.getLocation()));
}

llvm::DIImportedEntity *
CGDebugInfo::EmitNamespaceAlias(const NamespaceAliasDecl &NA) {
  if (CGM.getCodeGenOpts().getDebugInfo() < CodeGenOptions::LimitedDebugInfo)
    return nullptr;
  auto &VH = NamespaceAliasCache[&NA];
  if (VH)
    return cast<llvm::DIImportedEntity>(VH);
  llvm::DIImportedEntity *R;
  if (const NamespaceAliasDecl *Underlying =
          dyn_cast<NamespaceAliasDecl>(NA.getAliasedNamespace()))
    // This could cache & dedup here rather than relying on metadata deduping.
    R = DBuilder.createImportedDeclaration(
        getCurrentContextDescriptor(cast<Decl>(NA.getDeclContext())),
        EmitNamespaceAlias(*Underlying), getLineNumber(NA.getLocation()),
        NA.getName());
  else
    R = DBuilder.createImportedDeclaration(
        getCurrentContextDescriptor(cast<Decl>(NA.getDeclContext())),
        getOrCreateNameSpace(cast<NamespaceDecl>(NA.getAliasedNamespace())),
        getLineNumber(NA.getLocation()), NA.getName());
  VH.reset(R);
  return R;
}

llvm::DINamespace *
CGDebugInfo::getOrCreateNameSpace(const NamespaceDecl *NSDecl) {
  NSDecl = NSDecl->getCanonicalDecl();
  auto I = NameSpaceCache.find(NSDecl);
  if (I != NameSpaceCache.end())
    return cast<llvm::DINamespace>(I->second);

  unsigned LineNo = getLineNumber(NSDecl->getLocation());
  llvm::DIFile *FileD = getOrCreateFile(NSDecl->getLocation());
  llvm::DIScope *Context =
      getContextDescriptor(dyn_cast<Decl>(NSDecl->getDeclContext()));
  llvm::DINamespace *NS =
      DBuilder.createNameSpace(Context, NSDecl->getName(), FileD, LineNo);
  NameSpaceCache[NSDecl].reset(NS);
  return NS;
}

void CGDebugInfo::finalize() {
  // Creating types might create further types - invalidating the current
  // element and the size(), so don't cache/reference them.
#if 1 // HLSL Change - no ObjC support
  assert(ObjCInterfaceCache.size() == 0);
#else
  for (size_t i = 0; i != ObjCInterfaceCache.size(); ++i) {
    ObjCInterfaceCacheEntry E = ObjCInterfaceCache[i];
    llvm::DIType *Ty = E.Type->getDecl()->getDefinition()
                           ? CreateTypeDefinition(E.Type, E.Unit)
                           : E.Decl;
    DBuilder.replaceTemporary(llvm::TempDIType(E.Decl), Ty);
  }
#endif // HLSL Change - no ObjC support

  for (auto p : ReplaceMap) {
    assert(p.second);
    auto *Ty = cast<llvm::DIType>(p.second);
    assert(Ty->isForwardDecl());

    auto it = TypeCache.find(p.first);
    assert(it != TypeCache.end());
    assert(it->second);

    DBuilder.replaceTemporary(llvm::TempDIType(Ty),
                              cast<llvm::DIType>(it->second));
  }

  for (const auto &p : FwdDeclReplaceMap) {
    assert(p.second);
    llvm::TempMDNode FwdDecl(cast<llvm::MDNode>(p.second));
    llvm::Metadata *Repl;

    auto it = DeclCache.find(p.first);
    // If there has been no definition for the declaration, call RAUW
    // with ourselves, that will destroy the temporary MDNode and
    // replace it with a standard one, avoiding leaking memory.
    if (it == DeclCache.end())
      Repl = p.second;
    else
      Repl = it->second;

    DBuilder.replaceTemporary(std::move(FwdDecl), cast<llvm::MDNode>(Repl));
  }

  // We keep our own list of retained types, because we need to look
  // up the final type in the type cache.
  for (std::vector<void *>::const_iterator RI = RetainedTypes.begin(),
         RE = RetainedTypes.end(); RI != RE; ++RI)
    DBuilder.retainType(cast<llvm::DIType>(TypeCache[*RI]));

  DBuilder.finalize();
}

void CGDebugInfo::EmitExplicitCastType(QualType Ty) {
  if (CGM.getCodeGenOpts().getDebugInfo() < CodeGenOptions::LimitedDebugInfo)
    return;

  if (auto *DieTy = getOrCreateType(Ty, getOrCreateMainFile()))
    // Don't ignore in case of explicit cast where it is referenced indirectly.
    DBuilder.retainType(DieTy);
}
