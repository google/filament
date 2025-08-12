//==--- Attr.td - attribute definitions -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class DocumentationCategory<string name> {
  string Name = name;
  code Content = [{}];
}
def DocCatFunction : DocumentationCategory<"Function Attributes">;
def DocCatVariable : DocumentationCategory<"Variable Attributes">;
def DocCatType : DocumentationCategory<"Type Attributes">;
def DocCatStmt : DocumentationCategory<"Statement Attributes">;
// Attributes listed under the Undocumented category do not generate any public
// documentation. Ideally, this category should be used for internal-only
// attributes which contain no spellings.
def DocCatUndocumented : DocumentationCategory<"Undocumented">;

class DocDeprecated<string replacement = ""> {
  // If the Replacement field is empty, no replacement will be listed with the
  // documentation. Otherwise, the documentation will specify the attribute has
  // been superseded by this replacement.
  string Replacement = replacement;
}

// Specifies the documentation to be associated with the given category.
class Documentation {
  DocumentationCategory Category;
  code Content;

  // If the heading is empty, one may be picked automatically. If the attribute
  // only has one spelling, no heading is required as the attribute's sole
  // spelling is sufficient. If all spellings are semantically common, the
  // heading will be the semantic spelling. If the spellings are not
  // semantically common and no heading is provided, an error will be emitted.
  string Heading = "";

  // When set, specifies that the attribute is deprecated and can optionally
  // specify a replacement attribute.
  DocDeprecated Deprecated;
}

// Specifies that the attribute is explicitly undocumented. This can be a
// helpful placeholder for the attribute while working on the implementation,
// but should not be used once feature work has been completed.
def Undocumented : Documentation {
  let Category = DocCatUndocumented;
}

include "clang/Basic/AttrDocs.td"

// An attribute's subject is whatever it appertains to. In this file, it is
// more accurately a list of things that an attribute can appertain to. All
// Decls and Stmts are possibly AttrSubjects (even though the syntax may not
// allow attributes on a given Decl or Stmt).
class AttrSubject;

include "clang/Basic/DeclNodes.td"
include "clang/Basic/StmtNodes.td"

// A subset-subject is an AttrSubject constrained to operate only on some subset
// of that subject.
//
// The code fragment is a boolean expression that will confirm that the subject
// meets the requirements; the subject will have the name S, and will have the
// type specified by the base. It should be a simple boolean expression.
class SubsetSubject<AttrSubject base, code check> : AttrSubject {
  AttrSubject Base = base;
  code CheckCode = check;
}

// This is the type of a variable which C++11 allows alignas(...) to appertain
// to.
def NormalVar : SubsetSubject<Var,
                              [{S->getStorageClass() != VarDecl::Register &&
                                S->getKind() != Decl::ImplicitParam &&
                                S->getKind() != Decl::ParmVar &&
                                S->getKind() != Decl::NonTypeTemplateParm}]>;
def NonBitField : SubsetSubject<Field,
                                [{!S->isBitField()}]>;

def ObjCInstanceMethod : SubsetSubject<ObjCMethod,
                                       [{S->isInstanceMethod()}]>;

def ObjCInterfaceDeclInitMethod : SubsetSubject<ObjCMethod,
                               [{S->getMethodFamily() == OMF_init &&
                                 (isa<ObjCInterfaceDecl>(S->getDeclContext()) ||
                                  (isa<ObjCCategoryDecl>(S->getDeclContext()) &&
            cast<ObjCCategoryDecl>(S->getDeclContext())->IsClassExtension()))}]>;

def Struct : SubsetSubject<Record,
                           [{!S->isUnion()}]>;

def TLSVar : SubsetSubject<Var,
                           [{S->getTLSKind() != 0}]>;

def SharedVar : SubsetSubject<Var,
                              [{S->hasGlobalStorage() && !S->getTLSKind()}]>;

def GlobalVar : SubsetSubject<Var,
                             [{S->hasGlobalStorage()}]>;

// FIXME: this hack is needed because DeclNodes.td defines the base Decl node
// type to be a class, not a definition. This makes it impossible to create an
// attribute subject which accepts a Decl. Normally, this is not a problem,
// because the attribute can have no Subjects clause to accomplish this. But in
// the case of a SubsetSubject, there's no way to express it without this hack.
def DeclBase : AttrSubject;
def FunctionLike : SubsetSubject<DeclBase,
                                  [{S->getFunctionType(false) != NULL}]>;

def OpenCLKernelFunction : SubsetSubject<Function, [{
  S->hasAttr<OpenCLKernelAttr>()
}]>;

// HasFunctionProto is a more strict version of FunctionLike, so it should
// never be specified in a Subjects list along with FunctionLike (due to the
// inclusive nature of subject testing).
def HasFunctionProto : SubsetSubject<DeclBase,
                                     [{(S->getFunctionType(true) != NULL &&
                              isa<FunctionProtoType>(S->getFunctionType())) ||
                                       isa<ObjCMethodDecl>(S) ||
                                       isa<BlockDecl>(S)}]>;

// A single argument to an attribute
class Argument<string name, bit optional> {
  string Name = name;
  bit Optional = optional;
}

class BoolArgument<string name, bit opt = 0> : Argument<name, opt>;
class IdentifierArgument<string name, bit opt = 0> : Argument<name, opt>;
class IntArgument<string name, bit opt = 0> : Argument<name, opt>;
class StringArgument<string name, bit opt = 0> : Argument<name, opt>;
class ExprArgument<string name, bit opt = 0> : Argument<name, opt>;
class FunctionArgument<string name, bit opt = 0> : Argument<name, opt>;
class TypeArgument<string name, bit opt = 0> : Argument<name, opt>;
class UnsignedArgument<string name, bit opt = 0> : Argument<name, opt>;
class VariadicUnsignedArgument<string name> : Argument<name, 1>;
class VariadicExprArgument<string name> : Argument<name, 1>;
class VariadicStringArgument<string name> : Argument<name, 1>;

// A version of the form major.minor[.subminor].
class VersionArgument<string name, bit opt = 0> : Argument<name, opt>;

// This one's a doozy, so it gets its own special type
// It can be an unsigned integer, or a type. Either can
// be dependent.
class AlignedArgument<string name, bit opt = 0> : Argument<name, opt>;

// A bool argument with a default value
class DefaultBoolArgument<string name, bit default> : BoolArgument<name, 1> {
  bit Default = default;
}

// An integer argument with a default value
class DefaultIntArgument<string name, int default> : IntArgument<name, 1> {
  int Default = default;
}

// This argument is more complex, it includes the enumerator type name,
// a list of strings to accept, and a list of enumerators to map them to.
class EnumArgument<string name, string type, list<string> values,
                   list<string> enums, bit opt = 0> : Argument<name, opt> {
  string Type = type;
  list<string> Values = values;
  list<string> Enums = enums;
}

// FIXME: There should be a VariadicArgument type that takes any other type
//        of argument and generates the appropriate type.
class VariadicEnumArgument<string name, string type, list<string> values,
                           list<string> enums> : Argument<name, 1>  {
  string Type = type;
  list<string> Values = values;
  list<string> Enums = enums;
}

// This handles one spelling of an attribute.
class Spelling<string name, string variety> {
  string Name = name;
  string Variety = variety;
  bit KnownToGCC;
}

class GNU<string name> : Spelling<name, "GNU">;
class Declspec<string name> : Spelling<name, "Declspec">;
class CXX11<string namespace, string name, int version = 1>
    : Spelling<name, "CXX11"> {
  string Namespace = namespace;
  int Version = version;
}
class Keyword<string name> : Spelling<name, "Keyword">;
class Pragma<string namespace, string name> : Spelling<name, "Pragma"> {
  string Namespace = namespace;
}

// The GCC spelling implies GNU<name, "GNU"> and CXX11<"gnu", name> and also
// sets KnownToGCC to 1. This spelling should be used for any GCC-compatible
// attributes.
class GCC<string name> : Spelling<name, "GCC"> {
  let KnownToGCC = 1;
}

class Accessor<string name, list<Spelling> spellings> {
  string Name = name;
  list<Spelling> Spellings = spellings;
}

class SubjectDiag<bit warn> {
  bit Warn = warn;
}
def WarnDiag : SubjectDiag<1>;
def ErrorDiag : SubjectDiag<0>;

class SubjectList<list<AttrSubject> subjects, SubjectDiag diag = WarnDiag,
                  string customDiag = ""> {
  list<AttrSubject> Subjects = subjects;
  SubjectDiag Diag = diag;
  string CustomDiag = customDiag;
}

class LangOpt<string name, bit negated = 0> {
  string Name = name;
  bit Negated = negated;
}
def MicrosoftExt : LangOpt<"MicrosoftExt">;
def Borland : LangOpt<"Borland">;
def CUDA : LangOpt<"CUDA">;
def COnly : LangOpt<"CPlusPlus", 1>;
def SPIRV : LangOpt<"SPIRV">; // SPIRV Change

// Defines targets for target-specific attributes. The list of strings should
// specify architectures for which the target applies, based off the ArchType
// enumeration in Triple.h.
class TargetArch<list<string> arches> {
  list<string> Arches = arches;
  list<string> OSes;
}
def TargetARM : TargetArch<["arm", "thumb"]>;
def TargetMSP430 : TargetArch<["msp430"]>;
def TargetX86 : TargetArch<["x86"]>;
def TargetWindows : TargetArch<["x86", "x86_64", "arm", "thumb"]> {
  let OSes = ["Win32"];
}
def TargetMips : TargetArch<["mips", "mipsel"]>;

class Attr {
  // The various ways in which an attribute can be spelled in source
  list<Spelling> Spellings;
  // The things to which an attribute can appertain
  SubjectList Subjects;
  // The arguments allowed on an attribute
  list<Argument> Args = [];
  // Accessors which should be generated for the attribute.
  list<Accessor> Accessors = [];
  // Set to true for attributes with arguments which require delayed parsing.
  bit LateParsed = 0;
  // Set to false to prevent an attribute from being propagated from a template
  // to the instantiation.
  bit Clone = 1;
  // Set to true for attributes which must be instantiated within templates
  bit TemplateDependent = 0;
  // Set to true for attributes that have a corresponding AST node.
  bit ASTNode = 1;
  // Set to true for attributes which have handler in Sema.
  bit SemaHandler = 1;
  // Set to true for attributes that are completely ignored.
  bit Ignored = 0;
  // Set to true if the attribute's parsing does not match its semantic
  // content. Eg) It parses 3 args, but semantically takes 4 args.  Opts out of
  // common attribute error checking.
  bit HasCustomParsing = 0;
  // Set to true if all of the attribute's arguments should be parsed in an
  // unevaluated context.
  bit ParseArgumentsAsUnevaluated = 0;
  // Set to true if this attribute can be duplicated on a subject when merging
  // attributes. By default, attributes are not merged.
  bit DuplicatesAllowedWhileMerging = 0;
  // Lists language options, one of which is required to be true for the
  // attribute to be applicable. If empty, no language options are required.
  list<LangOpt> LangOpts = [];
  // Any additional text that should be included verbatim in the class.
  code AdditionalMembers = [{}];
  // Any documentation that should be associated with the attribute. Since an
  // attribute may be documented under multiple categories, more than one
  // Documentation entry may be listed.
  list<Documentation> Documentation;
}

/// A type attribute is not processed on a declaration or a statement.
class TypeAttr : Attr {
  // By default, type attributes do not get an AST node.
  let ASTNode = 0;
}

/// An inheritable attribute is inherited by later redeclarations.
class InheritableAttr : Attr;

/// A target-specific attribute.  This class is meant to be used as a mixin
/// with InheritableAttr or Attr depending on the attribute's needs.
class TargetSpecificAttr<TargetArch target> {
  TargetArch Target = target;
  // Attributes are generally required to have unique spellings for their names
  // so that the parser can determine what kind of attribute it has parsed.
  // However, target-specific attributes are special in that the attribute only
  // "exists" for a given target. So two target-specific attributes can share
  // the same name when they exist in different targets. To support this, a
  // Kind can be explicitly specified for a target-specific attribute. This
  // corresponds to the AttributeList::AT_* enum that is generated and it
  // should contain a shared value between the attributes.
  //
  // Target-specific attributes which use this feature should ensure that the
  // spellings match exactly betweeen the attributes, and if the arguments or
  // subjects differ, should specify HasCustomParsing = 1 and implement their
  // own parsing and semantic handling requirements as-needed.
  string ParseKind;
}

/// An inheritable parameter attribute is inherited by later
/// redeclarations, even when it's written on a parameter.
class InheritableParamAttr : InheritableAttr;

/// An ignored attribute, which we parse but discard with no checking.
class IgnoredAttr : Attr {
  let Ignored = 1;
  let ASTNode = 0;
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

//
// Attributes begin here
//

def AddressSpace : TypeAttr {
  let Spellings = [GNU<"address_space">];
  let Args = [IntArgument<"AddressSpace">];
  let Documentation = [Undocumented];
}

def Alias : Attr {
  let Spellings = [GCC<"alias">];
  let Args = [StringArgument<"Aliasee">];
  let Subjects = SubjectList<[Function, GlobalVar], ErrorDiag,
                             "ExpectedFunctionGlobalVarMethodOrProperty">;
  let Documentation = [Undocumented];
}

def Aligned : InheritableAttr {
  let Spellings = [GCC<"aligned">, Declspec<"align">, Keyword<"alignas">,
                   Keyword<"_Alignas">];
//  let Subjects = SubjectList<[NonBitField, NormalVar, Tag]>;
  let Args = [AlignedArgument<"Alignment", 1>];
  let Accessors = [Accessor<"isGNU", [GCC<"aligned">]>,
                   Accessor<"isC11", [Keyword<"_Alignas">]>,
                   Accessor<"isAlignas", [Keyword<"alignas">,
                                          Keyword<"_Alignas">]>,
                   Accessor<"isDeclspec",[Declspec<"align">]>];
  let Documentation = [Undocumented];
}

def AlignValue : Attr {
  let Spellings = [
    // Unfortunately, this is semantically an assertion, not a directive
    // (something else must ensure the alignment), so aligned_value is a
    // probably a better name. We might want to add an aligned_value spelling in
    // the future (and a corresponding C++ attribute), but this can be done
    // later once we decide if we also want them to have slightly-different
    // semantics than Intel's align_value.
    GNU<"align_value">
    // Intel's compiler on Windows also supports:
    // , Declspec<"align_value">
  ];
  let Args = [ExprArgument<"Alignment">];
  let Subjects = SubjectList<[Var, TypedefName], WarnDiag,
                             "ExpectedVariableOrTypedef">;
  let Documentation = [AlignValueDocs];
}

def AlignMac68k : InheritableAttr {
  // This attribute has no spellings as it is only ever created implicitly.
  let Spellings = [];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

def AlwaysInline : InheritableAttr {
  let Spellings = [GCC<"always_inline">, Keyword<"__forceinline">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def TLSModel : InheritableAttr {
  let Spellings = [GCC<"tls_model">];
  let Subjects = SubjectList<[TLSVar], ErrorDiag, "ExpectedTLSVar">;
  let Args = [StringArgument<"Model">];
  let Documentation = [TLSModelDocs];
}

def AnalyzerNoReturn : InheritableAttr {
  let Spellings = [GNU<"analyzer_noreturn">];
  let Documentation = [Undocumented];
}

def Annotate : InheritableParamAttr {
  let Spellings = [GNU<"annotate">];
  let Args = [StringArgument<"Annotation">];
  let Documentation = [Undocumented];
}

def ARMInterrupt : InheritableAttr, TargetSpecificAttr<TargetARM> {
  // NOTE: If you add any additional spellings, MSP430Interrupt's spellings
  // must match.
  let Spellings = [GNU<"interrupt">];
  let Args = [EnumArgument<"Interrupt", "InterruptType",
                           ["IRQ", "FIQ", "SWI", "ABORT", "UNDEF", ""],
                           ["IRQ", "FIQ", "SWI", "ABORT", "UNDEF", "Generic"],
                           1>];
  let ParseKind = "Interrupt";
  let HasCustomParsing = 1;
  let Documentation = [ARMInterruptDocs];
}

def AsmLabel : InheritableAttr {
  let Spellings = [Keyword<"asm">, Keyword<"__asm__">];
  let Args = [StringArgument<"Label">];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

def Availability : InheritableAttr {
  let Spellings = [GNU<"availability">];
  let Args = [IdentifierArgument<"platform">, VersionArgument<"introduced">,
              VersionArgument<"deprecated">, VersionArgument<"obsoleted">,
              BoolArgument<"unavailable">, StringArgument<"message">];
  let AdditionalMembers =
[{static llvm::StringRef getPrettyPlatformName(llvm::StringRef Platform) {
    return llvm::StringSwitch<llvm::StringRef>(Platform)
             .Case("android", "Android")
             .Case("ios", "iOS")
             .Case("macosx", "OS X")
             .Case("ios_app_extension", "iOS (App Extension)")
             .Case("macosx_app_extension", "OS X (App Extension)")
             .Default(llvm::StringRef());
} }];
  let HasCustomParsing = 1;
  let DuplicatesAllowedWhileMerging = 1;
//  let Subjects = SubjectList<[Named]>;
  let Documentation = [AvailabilityDocs];
}

def Blocks : InheritableAttr {
  let Spellings = [GNU<"blocks">];
  let Args = [EnumArgument<"Type", "BlockType", ["byref"], ["ByRef"]>];
  let Documentation = [Undocumented];
}

def Bounded : IgnoredAttr {
  let Spellings = [GNU<"bounded">];
}

def CarriesDependency : InheritableParamAttr {
  let Spellings = [GNU<"carries_dependency">,
                   CXX11<"","carries_dependency", 200809>];
  let Subjects = SubjectList<[ParmVar, ObjCMethod, Function], ErrorDiag>;
  let Documentation = [CarriesDependencyDocs];
}

def CDecl : InheritableAttr {
  let Spellings = [GCC<"cdecl">, Keyword<"__cdecl">, Keyword<"_cdecl">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [Undocumented];
}

// cf_audited_transfer indicates that the given function has been
// audited and has been marked with the appropriate cf_consumed and
// cf_returns_retained attributes.  It is generally applied by
// '#pragma clang arc_cf_code_audited' rather than explicitly.
def CFAuditedTransfer : InheritableAttr {
  let Spellings = [GNU<"cf_audited_transfer">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

// cf_unknown_transfer is an explicit opt-out of cf_audited_transfer.
// It indicates that the function has unknown or unautomatable
// transfer semantics.
def CFUnknownTransfer : InheritableAttr {
  let Spellings = [GNU<"cf_unknown_transfer">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

def CFReturnsRetained : InheritableAttr {
  let Spellings = [GNU<"cf_returns_retained">];
//  let Subjects = SubjectList<[ObjCMethod, ObjCProperty, Function]>;
  let Documentation = [Undocumented];
}

def CFReturnsNotRetained : InheritableAttr {
  let Spellings = [GNU<"cf_returns_not_retained">];
//  let Subjects = SubjectList<[ObjCMethod, ObjCProperty, Function]>;
  let Documentation = [Undocumented];
}

def CFConsumed : InheritableParamAttr {
  let Spellings = [GNU<"cf_consumed">];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def Cleanup : InheritableAttr {
  let Spellings = [GCC<"cleanup">];
  let Args = [FunctionArgument<"FunctionDecl">];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [Undocumented];
}

def Cold : InheritableAttr {
  let Spellings = [GCC<"cold">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def Common : InheritableAttr {
  let Spellings = [GCC<"common">];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [Undocumented];
}

def Const : InheritableAttr {
  let Spellings = [GCC<"const">, GCC<"__const">];
  let Documentation = [Undocumented];
}

def Constructor : InheritableAttr {
  let Spellings = [GCC<"constructor">];
  let Args = [DefaultIntArgument<"Priority", 65535>];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def CUDAConstant : InheritableAttr {
  let Spellings = [GNU<"constant">];
  let Subjects = SubjectList<[Var]>;
  let LangOpts = [CUDA];
  let Documentation = [Undocumented];
}

def CUDADevice : InheritableAttr {
  let Spellings = [GNU<"device">];
  let Subjects = SubjectList<[Function, Var]>;
  let LangOpts = [CUDA];
  let Documentation = [Undocumented];
}

def CUDAGlobal : InheritableAttr {
  let Spellings = [GNU<"global">];
  let Subjects = SubjectList<[Function]>;
  let LangOpts = [CUDA];
  let Documentation = [Undocumented];
}

def CUDAHost : InheritableAttr {
  let Spellings = [GNU<"host">];
  let Subjects = SubjectList<[Function]>;
  let LangOpts = [CUDA];
  let Documentation = [Undocumented];
}

def CUDAInvalidTarget : InheritableAttr {
  let Spellings = [];
  let Subjects = SubjectList<[Function]>;
  let LangOpts = [CUDA];
  let Documentation = [Undocumented];
}

def CUDALaunchBounds : InheritableAttr {
  let Spellings = [GNU<"launch_bounds">];
  let Args = [ExprArgument<"MaxThreads">, ExprArgument<"MinBlocks", 1>];
  let LangOpts = [CUDA];
  let Subjects = SubjectList<[ObjCMethod, FunctionLike], WarnDiag,
                             "ExpectedFunctionOrMethod">;
  // An AST node is created for this attribute, but is not used by other parts
  // of the compiler. However, this node needs to exist in the AST because
  // non-LLVM backends may be relying on the attribute's presence.
  let Documentation = [Undocumented];
}

def CUDAShared : InheritableAttr {
  let Spellings = [GNU<"shared">];
  let Subjects = SubjectList<[Var]>;
  let LangOpts = [CUDA];
  let Documentation = [Undocumented];
}

// HLSL Change Starts

// HLSL Control Flow Attributes
def HLSLAllowUAVCondition : Attr {
  let Spellings = [CXX11<"", "allow_uav_condition", 2015>];
  let Documentation = [Undocumented];
}
def HLSLBranch : Attr {
  let Spellings = [CXX11<"", "branch", 2015>];
  let Documentation = [Undocumented];
}
def HLSLCall : Attr {
  let Spellings = [CXX11<"", "call", 2015>];
  let Documentation = [Undocumented];
}
def HLSLFastOpt : Attr {
  let Spellings = [CXX11<"", "fastopt", 2015>];
  let Documentation = [Undocumented];
}
def HLSLFlatten : Attr {
  let Spellings = [CXX11<"", "flatten", 2015>];
  let Documentation = [Undocumented];
}
def HLSLForceCase : Attr {
  let Spellings = [CXX11<"", "forcecase", 2015>];
  let Documentation = [Undocumented];
}
def HLSLLoop : Attr {
  let Spellings = [CXX11<"", "loop", 2015>];
  let Documentation = [Undocumented];
}
def HLSLUnroll : Attr {
  let Spellings = [CXX11<"", "unroll", 2015>];
  let Args = [IntArgument<"Count", 1>];
  let Documentation = [Undocumented];
}

// HLSL Function Attributes
def HLSLClipPlanes : InheritableAttr {
  let Spellings = [CXX11<"", "clipplanes", 2015>];
  let Args = [ExprArgument<"ClipPlane1">,
              ExprArgument<"ClipPlane2", 1>,
              ExprArgument<"ClipPlane3", 1>,
              ExprArgument<"ClipPlane4", 1>,
              ExprArgument<"ClipPlane5", 1>,
              ExprArgument<"ClipPlane6", 1>];
  let Documentation = [Undocumented];
}
def HLSLDomain : InheritableAttr {
  let Spellings = [CXX11<"", "domain", 2015>];
  let Args = [StringArgument<"DomainType">]; // one of tri, quad, isoline
  let Documentation = [Undocumented];
}
def HLSLEarlyDepthStencil : InheritableAttr {
  let Spellings = [CXX11<"", "earlydepthstencil", 2015>];
  let Documentation = [Undocumented];
}
def HLSLInstance: InheritableAttr {
  let Spellings = [CXX11<"", "instance", 2015>];
  let Args = [IntArgument<"Count">];
  let Documentation = [Undocumented];
}
def HLSLMaxTessFactor: InheritableAttr {
  let Spellings = [CXX11<"", "maxtessfactor", 2015>];
  let Args = [IntArgument<"Factor">];
  let Documentation = [Undocumented];
}
def HLSLNumThreads: InheritableAttr {
  let Spellings = [CXX11<"", "numthreads", 2015>];
  let Args = [IntArgument<"X">, IntArgument<"Y">, IntArgument<"Z">];
  let Documentation = [Undocumented];
}
def HLSLRootSignature: InheritableAttr {
  let Spellings = [CXX11<"", "RootSignature", 2015>];
  let Args = [StringArgument<"SignatureName">];
  let Documentation = [Undocumented];
}
def HLSLOutputControlPoints: InheritableAttr {
  let Spellings = [CXX11<"", "outputcontrolpoints", 2015>];
  let Args = [IntArgument<"Count">];
  let Documentation = [Undocumented];
}
def HLSLOutputTopology: InheritableAttr {
  let Spellings = [CXX11<"", "outputtopology", 2015>];
  let Args = [StringArgument<"Topology">]; // one of point, line, triangle_cw or triangle_ccw
  let Documentation = [Undocumented];
}
def HLSLPartitioning: InheritableAttr {
  let Spellings = [CXX11<"", "partitioning", 2015>];
  let Args = [StringArgument<"Scheme">]; // one of integer, fractional_even, fractional_odd or pow2
  let Documentation = [Undocumented];
}
def HLSLPatchConstantFunc: InheritableAttr {
  let Spellings = [CXX11<"", "patchconstantfunc", 2015>];
  let Args = [StringArgument<"FunctionName">];
  let Documentation = [Undocumented];
}
def HLSLMaxVertexCount: InheritableAttr {
  let Spellings = [CXX11<"", "maxvertexcount", 2015>];
  let Args = [IntArgument<"Count">];
  let Documentation = [Undocumented];
}
def HLSLIntrinsic: InheritableAttr {
  let Spellings = [CXX11<"", "intrinsic", 2015>];
  let Args = [StringArgument<"group">, StringArgument<"lowering">, IntArgument<"opcode">];
  let Documentation = [Undocumented];
}
def HLSLPrecise : InheritableAttr {
  let Spellings = [CXX11<"", "precise", 2015>];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName]>;
  let Documentation = [Undocumented];
}

def HLSLShared : InheritableAttr {
  let Spellings = [CXX11<"", "shared", 2015>];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [Undocumented];
}

def HLSLGroupShared : InheritableAttr {
  let Spellings = [CXX11<"", "groupshared", 2015>];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [Undocumented];
}

def HLSLUniform : InheritableAttr {
  let Spellings = [CXX11<"", "uniform", 2015>];
  let Subjects = SubjectList<[Var, ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLInOut : InheritableAttr {
  let Spellings = [CXX11<"", "inout", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLIn : InheritableAttr {
  let Spellings = [CXX11<"", "in", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLOut : InheritableAttr {
  let Spellings = [CXX11<"", "out", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLLinear : InheritableAttr {
  let Spellings = [CXX11<"", "linear", 2015>];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Documentation = [Undocumented];
}

def HLSLCenter : InheritableAttr {
  let Spellings = [CXX11<"", "center", 2015>];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Documentation = [Undocumented];
}

def HLSLNoPerspective : InheritableAttr {
  let Spellings = [CXX11<"", "noperspective", 2015>];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Documentation = [Undocumented];
}

def HLSLSample : InheritableAttr {
  let Spellings = [CXX11<"", "sample", 2015>];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Documentation = [Undocumented];
}

def HLSLCentroid : InheritableAttr {
  let Spellings = [CXX11<"", "centroid", 2015>];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Documentation = [Undocumented];
}

def HLSLNoInterpolation : InheritableAttr {
  let Spellings = [CXX11<"", "nointerpolation", 2015>];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Documentation = [Undocumented];
}

def HLSLSemantic : InheritableAttr {
  let Spellings = [];
  let Subjects = SubjectList<[Function, ParmVar, Field]>;
  let Args = [StringArgument<"name">];
  let Documentation = [Undocumented];
}

def HLSLColumnMajor : TypeAttr {
  let Spellings = [CXX11<"", "column_major", 2015>];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName]>;
  let Documentation = [Undocumented];
}

def HLSLRowMajor : TypeAttr {
  let Spellings = [CXX11<"", "row_major", 2015>];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName]>;
  let Documentation = [Undocumented];
}

def HLSLUnorm : InheritableAttr {
  let Spellings = [CXX11<"", "unorm", 2015>];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName]>;
  let Documentation = [Undocumented];
}

def HLSLSnorm : InheritableAttr {
  let Spellings = [CXX11<"", "snorm", 2015>];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName]>;
  let Documentation = [Undocumented];
}

def HLSLPoint : InheritableAttr {
  let Spellings = [CXX11<"", "point", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLLine : InheritableAttr {
  let Spellings = [CXX11<"", "line", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLLineAdj : InheritableAttr {
  let Spellings = [CXX11<"", "lineadj", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLTriangle : InheritableAttr {
  let Spellings = [CXX11<"", "triangle", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLTriangleAdj : InheritableAttr {
  let Spellings = [CXX11<"", "triangleadj", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLGloballyCoherent : InheritableAttr {
  let Spellings = [CXX11<"", "globallycoherent", 2015>];
  let Subjects = SubjectList<[Var, Function]>;
  let Documentation = [Undocumented];
}

def HLSLReorderCoherent : InheritableAttr {
  let Spellings = [CXX11<"", "reordercoherent", 2015>];
  let Subjects = SubjectList<[Var, Function]>;
  let Documentation = [Undocumented];
}

def HLSLShader : InheritableAttr {
  let Spellings = [CXX11<"", "shader", 2017>];
  let Args = [StringArgument<"stage">]; // one of compute, pixel, vertex, hull, domain, geometry, node
  let Documentation = [Undocumented];
}

// Add experimental target-dependent attribute to function for backend ("exp-name"="value")
def HLSLExperimental : InheritableAttr {
  let Spellings = [CXX11<"", "experimental", 2017>];
  let Args = [StringArgument<"name">, StringArgument<"value">];
  let Subjects = SubjectList<[Function], ErrorDiag, "ExpectedFunction">;
  let Documentation = [Undocumented];
}

def HLSLExport : InheritableAttr {
  let Spellings = [CXX11<"", "export", 2015>];
  let Subjects = SubjectList<[Function], ErrorDiag, "ExpectedFunction">;
  let Documentation = [Undocumented];
}

def HLSLIndices : InheritableAttr {
  let Spellings = [CXX11<"", "indices", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLVertices : InheritableAttr {
  let Spellings = [CXX11<"", "vertices", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLPrimitives : InheritableAttr {
  let Spellings = [CXX11<"", "primitives", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLPayload : InheritableAttr {
  let Spellings = [CXX11<"", "payload", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLRayPayload : InheritableAttr {
  let Spellings = [CXX11<"", "raypayload", 2015>];
  let Documentation = [Undocumented];
}

def HLSLWaveSensitive : InheritableAttr {
  let Spellings = [CXX11<"", "wavesensitive", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLWaveSize : InheritableAttr {
  let Spellings = [CXX11<"", "wavesize", 2017>];
  let Args = [IntArgument<"Min">, DefaultIntArgument<"Max", 0>, DefaultIntArgument<"Preferred", 0>];
  let Documentation = [Undocumented];

  let AdditionalMembers = [{
    private:
    int SpelledArgsCount = 0;
    public:
      int getSpelledArgsCount() const { return SpelledArgsCount; }
      void setSpelledArgsCount(int C) { SpelledArgsCount = C; }
  }];
}

def HLSLWaveOpsIncludeHelperLanes : InheritableAttr {
  let Spellings = [CXX11<"", "waveopsincludehelperlanes", 2015>];
  let Documentation = [Undocumented];
}

def HLSLMaybeUnused : InheritableAttr {
  let Spellings = [CXX11<"", "maybe_unused", 2017>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

// This attribute has no spelling, it is attached to methods by the compiler for
// methods that require C++ lookup rules.
def HLSLCXXOverload : InheritableAttr {
  let Spellings = [];
  let Subjects = SubjectList<[CXXMethod]>;
  let Documentation = [Undocumented];
}

// This attribute means something subtly different from the HLSLIntrinsic
// attribute. Prior to this being introduced a subset of the HLSL intrinsics
// were placed in the `hlsl` namespace, but not in a way that actually qualified
// their name lookup. The presence of the namespace as the declcontext was then
// sused in CGExpr to determine how to emit the call. This attribute replaces
// the namespace in that context.
def HLSLBuiltinCall : InheritableAttr {
  let Spellings = [];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def HLSLVector : InheritableAttr {
  let Spellings = []; // No spellings!
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def HLSLMatrix : InheritableAttr {
  let Spellings = []; // No spellings!
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def HLSLTessPatch : InheritableAttr {
  let Spellings = []; // No spellings!
  let Args = [BoolArgument<"IsInput">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def HLSLStreamOutput : InheritableAttr {
  let Spellings = []; // No spellings!
  // PrimVertices are the number of vertices that make up the streamed
  // primitive. Points have 1. Lines have 2. Triangles have 3.
  let Args = [UnsignedArgument<"PrimVertices">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def HLSLResource : InheritableAttr {
  let Spellings = []; // No spellings!
  let Args = [UnsignedArgument<"ResKindUint">,
              UnsignedArgument<"ResClassUint">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];

  // Add enum typed getters for safety and brevity.
  let AdditionalMembers = [{
  hlsl::DXIL::ResourceKind getResKind() const {
    return (hlsl::DXIL::ResourceKind)getResKindUint();
  }
  hlsl::DXIL::ResourceClass getResClass() const {
    return (hlsl::DXIL::ResourceClass)getResClassUint();
  }
  }];
}

def HLSLNodeLaunch : InheritableAttr {
  let Spellings = [CXX11<"", "nodelaunch", 2017>];
  let Args = [StringArgument<"LaunchType">]; // one of broadcasting, coalescing, thread
  let Documentation = [Undocumented];
}

def HLSLNodeIsProgramEntry : InheritableAttr {
  let Spellings = [CXX11<"", "nodeisprogramentry", 2017>];
  let Documentation = [Undocumented];
}

def HLSLNodeId : InheritableAttr {
  let Spellings = [CXX11<"", "nodeid", 2017>];
  let Args = [StringArgument<"Name">,DefaultIntArgument<"ArrayIndex", 0>];
  let Documentation = [Undocumented];
}

def HLSLNodeLocalRootArgumentsTableIndex : InheritableAttr {
  let Spellings = [CXX11<"", "nodelocalrootargumentstableindex", 2017>];
  let Args = [UnsignedArgument<"Index">];
  let Documentation = [Undocumented];
}

def HLSLNodeShareInputOf : InheritableAttr {
  let Spellings = [CXX11<"", "nodeshareinputof", 2017>];
  let Args = [StringArgument<"Name">,UnsignedArgument<"ArrayIndex", 1>];
  let Documentation = [Undocumented];
}

def HLSLNodeDispatchGrid: InheritableAttr {
  let Spellings = [CXX11<"", "nodedispatchgrid", 2015>];
  let Args = [UnsignedArgument<"X">, UnsignedArgument<"Y">, UnsignedArgument<"Z">];
  let Documentation = [Undocumented];
}

def HLSLNodeMaxDispatchGrid: InheritableAttr {
  let Spellings = [CXX11<"", "nodemaxdispatchgrid", 2015>];
  let Args = [UnsignedArgument<"X">, UnsignedArgument<"Y">, UnsignedArgument<"Z">];
  let Documentation = [Undocumented];
}

def HLSLNodeMaxRecursionDepth : InheritableAttr {
  let Spellings = [CXX11<"", "nodemaxrecursiondepth", 2017>];
  let Args = [UnsignedArgument<"Count">];
  let Documentation = [Undocumented];
}

def HLSLNodeTrackRWInputSharing : InheritableAttr {
  let Spellings = [CXX11<"", "nodetrackrwinputsharing", 2017>];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def HLSLNodeObject : InheritableAttr {
  let Spellings = []; // No spellings!
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];

  let Args = [EnumArgument<"Type", "RecordType",
                           ["DispatchNodeInputRecord",
                           "RWDispatchNodeInputRecord",
                           "GroupNodeInputRecords",
                           "RWGroupNodeInputRecords",
                           "ThreadNodeInputRecord",
                           "RWThreadNodeInputRecord",
                           "EmptyNodeInput",
                           "NodeOutput",
                           "EmptyNodeOutput",
                           "NodeOutputArray",
                           "EmptyNodeOutputArray",
                           "GroupNodeOutputRecords",
                           "ThreadNodeOutputRecords"],
                           ["DispatchNodeInputRecord",
                           "RWDispatchNodeInputRecord",
                           "GroupNodeInputRecords",
                           "RWGroupNodeInputRecords",
                           "ThreadNodeInputRecord",
                           "RWThreadNodeInputRecord",
                           "EmptyNodeInput",
                           "NodeOutput",
                           "EmptyNodeOutput",
                           "NodeOutputArray",
                           "EmptyNodeOutputArray",
                           "GroupNodeOutputRecords",
                           "ThreadNodeOutputRecords"]>];

  let AdditionalMembers =
  [{hlsl::DXIL::NodeIOKind getNodeIOType() const {
      switch (type) {
      case DispatchNodeInputRecord:
        return hlsl::DXIL::NodeIOKind::DispatchNodeInputRecord;
      case RWDispatchNodeInputRecord:
        return hlsl::DXIL::NodeIOKind::RWDispatchNodeInputRecord;
      case GroupNodeInputRecords:
        return hlsl::DXIL::NodeIOKind::GroupNodeInputRecords;
      case RWGroupNodeInputRecords:
        return hlsl::DXIL::NodeIOKind::RWGroupNodeInputRecords;
      case ThreadNodeInputRecord:
        return hlsl::DXIL::NodeIOKind::ThreadNodeInputRecord;
      case RWThreadNodeInputRecord:
        return hlsl::DXIL::NodeIOKind::RWThreadNodeInputRecord;
      case EmptyNodeInput:
        return hlsl::DXIL::NodeIOKind::EmptyInput;
      case NodeOutput:
        return hlsl::DXIL::NodeIOKind::NodeOutput;
      case EmptyNodeOutput:
        return hlsl::DXIL::NodeIOKind::EmptyOutput;
      case NodeOutputArray:
        return hlsl::DXIL::NodeIOKind::NodeOutputArray;
      case EmptyNodeOutputArray:
        return hlsl::DXIL::NodeIOKind::EmptyOutputArray;
      case GroupNodeOutputRecords:
        return hlsl::DXIL::NodeIOKind::GroupNodeOutputRecords;
      case ThreadNodeOutputRecords:
        return hlsl::DXIL::NodeIOKind::ThreadNodeOutputRecords;
      }
      llvm_unreachable("all cases exhausted");
      return hlsl::DXIL::NodeIOKind::Invalid;
    }

    static RecordType toAttrType(hlsl::DXIL::NodeIOKind Type) {
      switch (Type) {
      case hlsl::DXIL::NodeIOKind::DispatchNodeInputRecord:
        return DispatchNodeInputRecord;
      case hlsl::DXIL::NodeIOKind::RWDispatchNodeInputRecord:
        return RWDispatchNodeInputRecord;
      case hlsl::DXIL::NodeIOKind::GroupNodeInputRecords:
        return GroupNodeInputRecords;
      case hlsl::DXIL::NodeIOKind::RWGroupNodeInputRecords:
        return RWGroupNodeInputRecords;
      case hlsl::DXIL::NodeIOKind::ThreadNodeInputRecord:
        return ThreadNodeInputRecord;
      case hlsl::DXIL::NodeIOKind::RWThreadNodeInputRecord:
        return RWThreadNodeInputRecord;
      case hlsl::DXIL::NodeIOKind::EmptyInput:
        return EmptyNodeInput;
      case hlsl::DXIL::NodeIOKind::NodeOutput:
        return NodeOutput;
      case hlsl::DXIL::NodeIOKind::EmptyOutput:
        return EmptyNodeOutput;
      case hlsl::DXIL::NodeIOKind::NodeOutputArray:
        return NodeOutputArray;
      case hlsl::DXIL::NodeIOKind::EmptyOutputArray:
        return EmptyNodeOutputArray;
      case hlsl::DXIL::NodeIOKind::GroupNodeOutputRecords:
        return GroupNodeOutputRecords;
      case hlsl::DXIL::NodeIOKind::ThreadNodeOutputRecords:
        return ThreadNodeOutputRecords;
      }
      llvm_unreachable("all cases exhausted");
    }

    static HLSLNodeObjectAttr *CreateImplicit(ASTContext &Ctx,
                                              hlsl::DXIL::NodeIOKind Type,
                                              SourceRange Loc = SourceRange()) {
      return CreateImplicit(Ctx, toAttrType(Type), Loc);
    }

    static const char *ConvertRecordTypeToStr(hlsl::DXIL::NodeIOKind Type) {
      return ConvertRecordTypeToStr(toAttrType(Type));
    }
    }];
}

// HLSL Ray Query Attribute

def HLSLRayQueryObject : InheritableAttr {
  let Spellings = []; // No spellings!
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def HLSLSubObject : InheritableAttr {
  let Spellings = []; // No spellings!
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
  let Args = [UnsignedArgument<"SubObjKindUint">, UnsignedArgument<"HitGroupType">];
}

// HLSL HitObject Attribute

def HLSLHitObject : InheritableAttr {
  let Spellings = []; // No spellings!
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

// HLSL Parameter Attributes

def HLSLMaxRecords : InheritableAttr {
  let Spellings = [CXX11<"", "MaxRecords", 2015>];
  let Args = [IntArgument<"maxCount">];
  let Documentation = [Undocumented];
}

def HLSLMaxRecordsSharedWith : InheritableParamAttr {
  let Spellings = [CXX11<"", "maxrecordssharedwith", 2015>];
  let Args = [IdentifierArgument<"Name">];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLAllowSparseNodes : InheritableParamAttr {
  let Spellings = [CXX11<"", "allowsparsenodes", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def HLSLNodeArraySize : InheritableAttr {
  let Spellings = [CXX11<"", "NodeArraySize", 2015>];
  let Args = [IntArgument<"count">];
  let Documentation = [Undocumented];
}

def HLSLUnboundedSparseNodes : InheritableParamAttr {
  let Spellings = [CXX11<"", "unboundedsparsenodes", 2015>];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

// HLSL Change Ends

// SPIRV Change Starts

// StructuredBuffer types that can have associated counters
def CounterStructuredBuffer : SubsetSubject<
    Var,
    [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
      (S->getType()->getAs<RecordType>()->getDecl()->getName() == "RWStructuredBuffer" ||
       S->getType()->getAs<RecordType>()->getDecl()->getName() == "AppendStructuredBuffer" ||
       S->getType()->getAs<RecordType>()->getDecl()->getName() == "ConsumeStructuredBuffer")}]>;

// Array of StructuredBuffer types that can have associated counters
def ArrayOfCounterStructuredBuffer
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAsArrayTypeUnsafe() &&
                 S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>() &&
                 S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl() &&
                  (S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWStructuredBuffer" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "AppendStructuredBuffer" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "ConsumeStructuredBuffer")}]>;

// Global variable with "ConstantBuffer" type
def ConstantBuffer
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
                 S->getType()->getAs<RecordType>()->getDecl() &&
                 S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                     "ConstantBuffer"}]>;

// Global variable with "ConstantBuffer" or "TextureBuffer" type
def ConstantTextureBuffer
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
                 S->getType()->getAs<RecordType>()->getDecl() &&
                 (S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "ConstantBuffer" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "TextureBuffer")}]>;

// Global variable with "(RW|RasterizerOrdered)Texture" type
def RWTexture
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
                 S->getType()->getAs<RecordType>()->getDecl() &&
                  (S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture1D" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture1DArray" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture2D" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture2DArray" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture3D" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture1D" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture1DArray" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture2D" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture2DArray" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture3D")}]>;

// Global variable of array of "(RW|RasterizerOrdered)Texture" type
def ArrayOfRWTexture
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAsArrayTypeUnsafe() &&
                 S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>() &&
                 S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl() &&
                  (S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture1D" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture1DArray" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture2D" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture2DArray" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWTexture3D" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture1D" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture1DArray" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture2D" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture2DArray" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedTexture3D")}]>;

// Global variable with "[RW|RasterizerOrdered]Buffer" type
def Buffer
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
                 S->getType()->getAs<RecordType>()->getDecl() &&
                 (S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "Buffer" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWBuffer" ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedBuffer")}]>;

// Global variable or array of "[RW|RasterizerOrdered]Buffer" type
def ArrayOfBuffer
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAsArrayTypeUnsafe() &&
                 S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>() &&
                 S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl() &&
                 (S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "Buffer" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RWBuffer" ||
                  S->getType()->getAsArrayTypeUnsafe()->getElementType()->getAs<RecordType>()->getDecl()->getName() ==
                      "RasterizerOrderedBuffer")}]>;

// Global variable with "Texture*" or "Sampler*" type
def TextureOrSampler
    : SubsetSubject<
          Var, [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
                 S->getType()->getAs<RecordType>()->getDecl() &&
                 (S->getType()->getAs<RecordType>()->getDecl()->getName().startswith("Texture") ||
                  S->getType()->getAs<RecordType>()->getDecl()->getName().startswith("Sampler"))}]>;

def VKBuiltIn : InheritableAttr {
  let Spellings = [CXX11<"vk", "builtin">];
  let Subjects = SubjectList<[Function, ParmVar, Field], ErrorDiag>;
  let Args = [StringArgument<"BuiltIn">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKExtBuiltinInput : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_builtin_input">];
  let Subjects = SubjectList<[Var], ErrorDiag>;
  let Args = [IntArgument<"BuiltInID">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKExtBuiltinOutput : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_builtin_output">];
  let Subjects = SubjectList<[Var], ErrorDiag>;
  let Args = [IntArgument<"BuiltInID">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKLocation : InheritableAttr {
  let Spellings = [CXX11<"vk", "location">];
  let Subjects = SubjectList<[Function, ParmVar, Field], ErrorDiag>;
  let Args = [IntArgument<"Number">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKIndex : InheritableAttr {
  let Spellings = [CXX11<"vk", "index">];
  let Subjects = SubjectList<[Function, ParmVar, Field], ErrorDiag>;
  let Args = [IntArgument<"Number">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKBinding : InheritableAttr {
  let Spellings = [CXX11<"vk", "binding">];
  let Subjects = SubjectList<[GlobalVar, HLSLBuffer, ConstantTextureBuffer],
                             ErrorDiag, "ExpectedGlobalVarOrCTBuffer">;
  let Args = [IntArgument<"Binding">, DefaultIntArgument<"Set", 0>];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKCapabilityExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_capability">];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName], ErrorDiag>;
  let Args = [IntArgument<"capability">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKCounterBinding : InheritableAttr {
  let Spellings = [CXX11<"vk", "counter_binding">];
  let Subjects = SubjectList<[ArrayOfCounterStructuredBuffer, CounterStructuredBuffer], ErrorDiag, "ExpectedCounterStructuredBuffer">;
  let Args = [IntArgument<"Binding">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKDecorateExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_decorate">];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName], ErrorDiag>;
  let Args = [UnsignedArgument<"decorate">, VariadicUnsignedArgument<"literals">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKDecorateIdExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_decorate_id">];
  let Subjects =
      SubjectList<[Function, Var, ParmVar, Field, TypedefName], ErrorDiag>;
  let Args = [UnsignedArgument<"decorate">, VariadicExprArgument<"arguments">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKDecorateStringExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_decorate_string">];
  let Subjects =
      SubjectList<[Function, Var, ParmVar, Field, TypedefName], ErrorDiag>;
  let Args = [UnsignedArgument<"decorate">, VariadicStringArgument<"arguments">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKExtensionExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_extension">];
  let Subjects = SubjectList<[Function, Var, ParmVar, Field, TypedefName], ErrorDiag>;
  let Args = [StringArgument<"name">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKSpvExecutionMode : InheritableAttr {
  let Spellings = [CXX11<"vk", "spvexecutionmode">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Args = [UnsignedArgument<"ExecutionMode">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStorageClassExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_storage_class">];
  let Subjects = SubjectList<[Var, ParmVar], ErrorDiag>;
  let Args = [UnsignedArgument<"stclass">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKBufferPointer : InheritableAttr {
  let Spellings = [CXX11<"", "hlsl_vk_buffer_pointer", 2021>];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKAliasedPointer : InheritableAttr {
  let Spellings = [CXX11<"vk", "aliased_pointer">];
  let Subjects = SubjectList<[Var, ParmVar], ErrorDiag>;
  let Args = [];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

// Global variables that are of struct type
def StructGlobalVar : SubsetSubject<Var, [{S->hasGlobalStorage() && S->getType()->isStructureType()}]>;

def VKPushConstant : InheritableAttr {
  let Spellings = [CXX11<"vk", "push_constant">];
  let Subjects = SubjectList<[StructGlobalVar], ErrorDiag, "ExpectedStructGlobalVar">;
  let Args = [];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKOffset : InheritableAttr {
  let Spellings = [CXX11<"vk", "offset">];
  let Subjects = SubjectList<[Field], ErrorDiag, "ExpectedField">;
  let Args = [IntArgument<"Offset">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKCombinedImageSampler : InheritableAttr {
  let Spellings = [CXX11<"vk", "combinedImageSampler">];
  let Subjects = SubjectList<[TextureOrSampler],
                             ErrorDiag, "ExpectedTextureOrSamplerState">;
  let Args = [];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKImageFormat : InheritableAttr {
  let Spellings = [CXX11<"vk", "image_format">];
  let Subjects = SubjectList<[RWTexture, ArrayOfRWTexture, Buffer, ArrayOfBuffer],
                             ErrorDiag, "ExpectedRWTextureOrBuffer">;
  let Args = [EnumArgument<"ImageFormat", "ImageFormatType",
                           ["unknown", "rgba32f", "rgba16f", "r32f", "rgba8", "rgba8snorm",
                           "rg32f", "rg16f", "r11g11b10f", "r16f", "rgba16", "rgb10a2",
                           "rg16", "rg8", "r16", "r8", "rgba16snorm", "rg16snorm", "rg8snorm",
                           "r16snorm", "r8snorm", "rgba32i", "rgba16i", "rgba8i", "r32i",
                           "rg32i", "rg16i", "rg8i", "r16i", "r8i", "rgba32ui", "rgba16ui", "rgba8ui",
                           "r32ui", "rgb10a2ui", "rg32ui", "rg16ui", "rg8ui", "r16ui",
                           "r8ui", "r64ui", "r64i"],
                           ["unknown", "rgba32f", "rgba16f", "r32f", "rgba8", "rgba8snorm",
                           "rg32f", "rg16f", "r11g11b10f", "r16f", "rgba16", "rgb10a2",
                           "rg16", "rg8", "r16", "r8", "rgba16snorm", "rg16snorm", "rg8snorm",
                           "r16snorm", "r8snorm", "rgba32i", "rgba16i", "rgba8i", "r32i",
                           "rg32i", "rg16i", "rg8i", "r16i", "r8i", "rgba32ui", "rgba16ui", "rgba8ui",
                           "r32ui", "rgb10a2ui", "rg32ui", "rg16ui", "rg8ui", "r16ui",
                           "r8ui", "r64ui", "r64i"]>];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def SubpassInput : SubsetSubject<
    Var,
    [{S->hasGlobalStorage() && S->getType()->getAs<RecordType>() &&
      (S->getType()->getAs<RecordType>()->getDecl()->getName() == "SubpassInput" ||
       S->getType()->getAs<RecordType>()->getDecl()->getName() == "SubpassInputMS")}]>;

def VKInputAttachmentIndex : InheritableAttr {
  let Spellings = [CXX11<"vk", "input_attachment_index">];
  let Subjects = SubjectList<[SubpassInput], ErrorDiag, "ExpectedSubpassInput">;
  let Args = [IntArgument<"index">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKInstructionExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_instruction">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Args = [IntArgument<"opcode">, StringArgument<"instruction_set", 1>];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKLiteralExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_literal">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKReferenceExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_reference">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKTypeDefExt : InheritableAttr {
  let Spellings = [CXX11<"vk", "ext_type_def">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Args = [UnsignedArgument<"id">, UnsignedArgument<"opcode">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

// Global variables that are of scalar type
def ScalarGlobalVar : SubsetSubject<Var, [{S->hasGlobalStorage() && S->getType()->isScalarType()}]>;

def VKConstantId : InheritableAttr {
  let Spellings = [CXX11<"vk", "constant_id">];
  let Subjects = SubjectList<[ScalarGlobalVar], ErrorDiag, "ExpectedScalarGlobalVar">;
  let Args = [IntArgument<"SpecConstId">];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKPostDepthCoverage : InheritableAttr {
  let Spellings = [CXX11<"vk", "post_depth_coverage">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKShaderRecordNV : InheritableAttr {
  let Spellings = [CXX11<"vk", "shader_record_nv">];
  let Subjects = SubjectList<[StructGlobalVar, HLSLBuffer, ConstantBuffer],
                             ErrorDiag, "ExpectedCTBuffer">;
  let Args = [];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKShaderRecordEXT : InheritableAttr {
  let Spellings = [CXX11<"vk", "shader_record_ext">];
  let Subjects = SubjectList<[StructGlobalVar, HLSLBuffer, ConstantBuffer],
                             ErrorDiag, "ExpectedCTBuffer">;
  let Args = [];
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKEarlyAndLateTests : InheritableAttr {
  let Spellings = [CXX11<"vk", "early_and_late_tests">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKDepthUnchanged : InheritableAttr {
  let Spellings = [CXX11<"vk", "depth_unchanged">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStencilRefUnchangedFront : InheritableAttr {
  let Spellings = [CXX11<"vk", "stencil_ref_unchanged_front">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStencilRefGreaterEqualFront : InheritableAttr {
  let Spellings = [CXX11<"vk", "stencil_ref_greater_equal_front">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStencilRefLessEqualFront : InheritableAttr {
  let Spellings = [CXX11<"vk", "stencil_ref_less_equal_front">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStencilRefUnchangedBack : InheritableAttr {
  let Spellings = [CXX11<"vk", "stencil_ref_unchanged_back">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStencilRefGreaterEqualBack : InheritableAttr {
  let Spellings = [CXX11<"vk", "stencil_ref_greater_equal_back">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}

def VKStencilRefLessEqualBack : InheritableAttr {
  let Spellings = [CXX11<"vk", "stencil_ref_less_equal_back">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let LangOpts = [SPIRV];
  let Documentation = [Undocumented];
}
// SPIRV Change Ends

def C11NoReturn : InheritableAttr {
  let Spellings = [Keyword<"_Noreturn">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let SemaHandler = 0;
  let Documentation = [C11NoReturnDocs];
}

def CXX11NoReturn : InheritableAttr {
  let Spellings = [CXX11<"","noreturn", 200809>];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [CXX11NoReturnDocs];
}

def OpenCLKernel : InheritableAttr {
  let Spellings = [Keyword<"__kernel">, Keyword<"kernel">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

// This attribute is both a type attribute, and a declaration attribute (for
// parameter variables).
def OpenCLImageAccess : Attr {
  let Spellings = [Keyword<"__read_only">, Keyword<"read_only">,
                   Keyword<"__write_only">, Keyword<"write_only">,
                   Keyword<"__read_write">, Keyword<"read_write">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let Accessors = [Accessor<"isReadOnly", [Keyword<"__read_only">,
                                           Keyword<"read_only">]>,
                   Accessor<"isReadWrite", [Keyword<"__read_write">,
                                            Keyword<"read_write">]>,
                   Accessor<"isWriteOnly", [Keyword<"__write_only">,
                                            Keyword<"write_only">]>];
  let Documentation = [Undocumented];
}

def OpenCLPrivateAddressSpace : TypeAttr {
  let Spellings = [Keyword<"__private">, Keyword<"private">];
  let Documentation = [OpenCLAddressSpacePrivateDocs];
}

def OpenCLGlobalAddressSpace : TypeAttr {
  let Spellings = [Keyword<"__global">, Keyword<"global">];
  let Documentation = [OpenCLAddressSpaceGlobalDocs];
}

def OpenCLLocalAddressSpace : TypeAttr {
  let Spellings = [Keyword<"__local">, Keyword<"local">];
  let Documentation = [OpenCLAddressSpaceLocalDocs];
}

def OpenCLConstantAddressSpace : TypeAttr {
  let Spellings = [Keyword<"__constant">, Keyword<"constant">];
  let Documentation = [OpenCLAddressSpaceConstantDocs];
}

def OpenCLGenericAddressSpace : TypeAttr {
  let Spellings = [Keyword<"__generic">, Keyword<"generic">];
  let Documentation = [OpenCLAddressSpaceGenericDocs];
}

def Deprecated : InheritableAttr {
  let Spellings = [GCC<"deprecated">, Declspec<"deprecated">,
                   CXX11<"","deprecated", 201309>];
  let Args = [StringArgument<"Message", 1>];
  let Documentation = [Undocumented];
}

def Destructor : InheritableAttr {
  let Spellings = [GCC<"destructor">];
  let Args = [DefaultIntArgument<"Priority", 65535>];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def EnableIf : InheritableAttr {
  let Spellings = [GNU<"enable_if">];
  let Subjects = SubjectList<[Function]>;
  let Args = [ExprArgument<"Cond">, StringArgument<"Message">];
  let TemplateDependent = 1;
  let Documentation = [EnableIfDocs];
}

def ExtVectorType : Attr {
  let Spellings = [GNU<"ext_vector_type">];
  let Subjects = SubjectList<[TypedefName], ErrorDiag>;
  let Args = [ExprArgument<"NumElements">];
  let ASTNode = 0;
  let Documentation = [Undocumented];
}

def FallThrough : Attr {
  let Spellings = [CXX11<"clang", "fallthrough">];
//  let Subjects = [NullStmt];
  let Documentation = [FallthroughDocs];
}

def FastCall : InheritableAttr {
  let Spellings = [GCC<"fastcall">, Keyword<"__fastcall">,
                   Keyword<"_fastcall">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [FastCallDocs];
}

def Final : InheritableAttr {
  let Spellings = [Keyword<"final">, Keyword<"sealed">];
  let Accessors = [Accessor<"isSpelledAsSealed", [Keyword<"sealed">]>];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

def MinSize : InheritableAttr {
  let Spellings = [GNU<"minsize">];
  let Subjects = SubjectList<[Function, ObjCMethod], ErrorDiag>;
  let Documentation = [Undocumented];
}

def FlagEnum : InheritableAttr {
  let Spellings = [GNU<"flag_enum">];
  let Subjects = SubjectList<[Enum]>;
  let Documentation = [FlagEnumDocs];
  let LangOpts = [COnly];
  let AdditionalMembers = [{
private:
    llvm::APInt FlagBits;
public:
    llvm::APInt &getFlagBits() {
      return FlagBits;
    }

    const llvm::APInt &getFlagBits() const {
      return FlagBits;
    }
}];
}

def Flatten : InheritableAttr {
  let Spellings = [GCC<"flatten">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [FlattenDocs];
}

def Format : InheritableAttr {
  let Spellings = [GCC<"format">];
  let Args = [IdentifierArgument<"Type">, IntArgument<"FormatIdx">,
              IntArgument<"FirstArg">];
  let Subjects = SubjectList<[ObjCMethod, Block, HasFunctionProto], WarnDiag,
                             "ExpectedFunction">;
  let Documentation = [FormatDocs];
}

def FormatArg : InheritableAttr {
  let Spellings = [GCC<"format_arg">];
  let Args = [IntArgument<"FormatIdx">];
  let Subjects = SubjectList<[ObjCMethod, HasFunctionProto], WarnDiag,
                             "ExpectedFunction">;
  let Documentation = [Undocumented];
}

def GNUInline : InheritableAttr {
  let Spellings = [GCC<"gnu_inline">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def Hot : InheritableAttr {
  let Spellings = [GCC<"hot">];
  let Subjects = SubjectList<[Function]>;
  // An AST node is created for this attribute, but not actually used beyond
  // semantic checking for mutual exclusion with the Cold attribute.
  let Documentation = [Undocumented];
}

def IBAction : InheritableAttr {
  let Spellings = [GNU<"ibaction">];
  let Subjects = SubjectList<[ObjCInstanceMethod], WarnDiag,
                             "ExpectedObjCInstanceMethod">;
  // An AST node is created for this attribute, but is not used by other parts
  // of the compiler. However, this node needs to exist in the AST because
  // external tools rely on it.
  let Documentation = [Undocumented];
}

def IBOutlet : InheritableAttr {
  let Spellings = [GNU<"iboutlet">];
//  let Subjects = [ObjCIvar, ObjCProperty];
  let Documentation = [Undocumented];
}

def IBOutletCollection : InheritableAttr {
  let Spellings = [GNU<"iboutletcollection">];
  let Args = [TypeArgument<"Interface", 1>];
//  let Subjects = [ObjCIvar, ObjCProperty];
  let Documentation = [Undocumented];
}

def Restrict : InheritableAttr {
  let Spellings = [Declspec<"restrict">, GCC<"malloc">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def MaxFieldAlignment : InheritableAttr {
  // This attribute has no spellings as it is only ever created implicitly.
  let Spellings = [];
  let Args = [UnsignedArgument<"Alignment">];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

def MayAlias : InheritableAttr {
  // FIXME: this is a type attribute in GCC, but a declaration attribute here.
  let Spellings = [GCC<"may_alias">];
  let Documentation = [Undocumented];
}

def MSABI : InheritableAttr {
  let Spellings = [GCC<"ms_abi">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [MSABIDocs];
}

def MSP430Interrupt : InheritableAttr, TargetSpecificAttr<TargetMSP430> {
  // NOTE: If you add any additional spellings, ARMInterrupt's spellings must
  // match.
  let Spellings = [GNU<"interrupt">];
  let Args = [UnsignedArgument<"Number">];
  let ParseKind = "Interrupt";
  let HasCustomParsing = 1;
  let Documentation = [Undocumented];
}

def Mips16 : InheritableAttr, TargetSpecificAttr<TargetMips> {
  let Spellings = [GCC<"mips16">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

def Mode : Attr {
  let Spellings = [GCC<"mode">];
  let Args = [IdentifierArgument<"Mode">];
  let Documentation = [Undocumented];
}

def Naked : InheritableAttr {
  let Spellings = [GCC<"naked">, Declspec<"naked">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def NeonPolyVectorType : TypeAttr {
  let Spellings = [GNU<"neon_polyvector_type">];
  let Args = [IntArgument<"NumElements">];
  let Documentation = [Undocumented];
}

def NeonVectorType : TypeAttr {
  let Spellings = [GNU<"neon_vector_type">];
  let Args = [IntArgument<"NumElements">];
  let Documentation = [Undocumented];
}

def ReturnsTwice : InheritableAttr {
  let Spellings = [GCC<"returns_twice">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def NoCommon : InheritableAttr {
  let Spellings = [GCC<"nocommon">];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [Undocumented];
}

def NoDebug : InheritableAttr {
  let Spellings = [GCC<"nodebug">];
  let Documentation = [Undocumented];
}

def NoDuplicate : InheritableAttr {
  let Spellings = [GNU<"noduplicate">, CXX11<"clang", "noduplicate">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [NoDuplicateDocs];
}

def NoInline : InheritableAttr {
  let Spellings = [GCC<"noinline">, Declspec<"noinline">,
    CXX11<"", "noinline", 2017>]; // HLSL Change
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def NoMips16 : InheritableAttr, TargetSpecificAttr<TargetMips> {
  let Spellings = [GCC<"nomips16">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

// This is not a TargetSpecificAttr so that is silently accepted and
// ignored on other targets as encouraged by the OpenCL spec.
//
// See OpenCL 1.2 6.11.5: "It is our intention that a particular
// implementation of OpenCL be free to ignore all attributes and the
// resulting executable binary will produce the same result."
//
// However, only AMD GPU targets will emit the corresponding IR
// attribute.
//
// FIXME: This provides a sub-optimal error message if you attempt to
// use this in CUDA, since CUDA does not use the same terminology.
def AMDGPUNumVGPR : InheritableAttr {
  let Spellings = [GNU<"amdgpu_num_vgpr">];
  let Args = [UnsignedArgument<"NumVGPR">];
  let Documentation = [AMDGPUNumVGPRDocs];

// FIXME: This should be for OpenCLKernelFunction, but is not to
// workaround needing to see kernel attribute before others to know if
// this should be rejected on non-kernels.
  let Subjects = SubjectList<[Function], ErrorDiag,
                             "ExpectedKernelFunction">;
}

def AMDGPUNumSGPR : InheritableAttr {
  let Spellings = [GNU<"amdgpu_num_sgpr">];
  let Args = [UnsignedArgument<"NumSGPR">];
  let Documentation = [AMDGPUNumSGPRDocs];
  let Subjects = SubjectList<[Function], ErrorDiag,
                              "ExpectedKernelFunction">;
}

def NoSplitStack : InheritableAttr {
  let Spellings = [GCC<"no_split_stack">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [NoSplitStackDocs];
}

def NonNull : InheritableAttr {
  let Spellings = [GCC<"nonnull">];
  let Subjects = SubjectList<[ObjCMethod, HasFunctionProto, ParmVar], WarnDiag,
                             "ExpectedFunctionMethodOrParameter">;
  let Args = [VariadicUnsignedArgument<"Args">];
  let AdditionalMembers =
[{bool isNonNull(unsigned idx) const {
    if (!args_size())
      return true;
    for (const auto &V : args())
      if (V == idx)
        return true;
    return false;
  } }];
  // FIXME: We should merge duplicates into a single nonnull attribute.
  let DuplicatesAllowedWhileMerging = 1;
  let Documentation = [NonNullDocs];
}

def ReturnsNonNull : InheritableAttr {
  let Spellings = [GCC<"returns_nonnull">];
  let Subjects = SubjectList<[ObjCMethod, Function], WarnDiag,
                             "ExpectedFunctionOrMethod">;
  let Documentation = [ReturnsNonNullDocs];
}

// Nullability type attributes.
def TypeNonNull : TypeAttr {
  let Spellings = [Keyword<"_Nonnull">];
  let Documentation = [TypeNonNullDocs];
}

def TypeNullable : TypeAttr {
  let Spellings = [Keyword<"_Nullable">];
  let Documentation = [TypeNullableDocs];
}

def TypeNullUnspecified : TypeAttr {
  let Spellings = [Keyword<"_Null_unspecified">];
  let Documentation = [TypeNullUnspecifiedDocs];
}

def ObjCKindOf : TypeAttr {
  let Spellings = [Keyword<"__kindof">];
  let Documentation = [Undocumented];
}

def AssumeAligned : InheritableAttr {
  let Spellings = [GCC<"assume_aligned">];
  let Subjects = SubjectList<[ObjCMethod, Function]>;
  let Args = [ExprArgument<"Alignment">, ExprArgument<"Offset", 1>];
  let Documentation = [AssumeAlignedDocs];
}

def NoReturn : InheritableAttr {
  let Spellings = [GCC<"noreturn">, Declspec<"noreturn">];
  // FIXME: Does GCC allow this on the function instead?
  let Documentation = [Undocumented];
}

def NoInstrumentFunction : InheritableAttr {
  let Spellings = [GCC<"no_instrument_function">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def NoThrow : InheritableAttr {
  let Spellings = [GCC<"nothrow">, Declspec<"nothrow">];
  let Documentation = [Undocumented];
}

def ObjCBridge : InheritableAttr {
  let Spellings = [GNU<"objc_bridge">];
  let Subjects = SubjectList<[Record, TypedefName], ErrorDiag,
                             "ExpectedStructOrUnionOrTypedef">;
  let Args = [IdentifierArgument<"BridgedType">];
  let Documentation = [Undocumented];
}

def ObjCBridgeMutable : InheritableAttr {
  let Spellings = [GNU<"objc_bridge_mutable">];
  let Subjects = SubjectList<[Record], ErrorDiag>;
  let Args = [IdentifierArgument<"BridgedType">];
  let Documentation = [Undocumented];
}

def ObjCBridgeRelated : InheritableAttr {
  let Spellings = [GNU<"objc_bridge_related">];
  let Subjects = SubjectList<[Record], ErrorDiag>;
  let Args = [IdentifierArgument<"RelatedClass">,
          IdentifierArgument<"ClassMethod">,
          IdentifierArgument<"InstanceMethod">];
  let HasCustomParsing = 1;
  let Documentation = [Undocumented];
}

def NSReturnsRetained : InheritableAttr {
  let Spellings = [GNU<"ns_returns_retained">];
//  let Subjects = SubjectList<[ObjCMethod, ObjCProperty, Function]>;
  let Documentation = [Undocumented];
}

def NSReturnsNotRetained : InheritableAttr {
  let Spellings = [GNU<"ns_returns_not_retained">];
//  let Subjects = SubjectList<[ObjCMethod, ObjCProperty, Function]>;
  let Documentation = [Undocumented];
}

def NSReturnsAutoreleased : InheritableAttr {
  let Spellings = [GNU<"ns_returns_autoreleased">];
//  let Subjects = SubjectList<[ObjCMethod, ObjCProperty, Function]>;
  let Documentation = [Undocumented];
}

def NSConsumesSelf : InheritableAttr {
  let Spellings = [GNU<"ns_consumes_self">];
  let Subjects = SubjectList<[ObjCMethod]>;
  let Documentation = [Undocumented];
}

def NSConsumed : InheritableParamAttr {
  let Spellings = [GNU<"ns_consumed">];
  let Subjects = SubjectList<[ParmVar]>;
  let Documentation = [Undocumented];
}

def ObjCException : InheritableAttr {
  let Spellings = [GNU<"objc_exception">];
  let Subjects = SubjectList<[ObjCInterface], ErrorDiag>;
  let Documentation = [Undocumented];
}

def ObjCMethodFamily : InheritableAttr {
  let Spellings = [GNU<"objc_method_family">];
  let Subjects = SubjectList<[ObjCMethod], ErrorDiag>;
  let Args = [EnumArgument<"Family", "FamilyKind",
               ["none", "alloc", "copy", "init", "mutableCopy", "new"],
               ["OMF_None", "OMF_alloc", "OMF_copy", "OMF_init",
                "OMF_mutableCopy", "OMF_new"]>];
  let Documentation = [ObjCMethodFamilyDocs];
}

def ObjCNSObject : InheritableAttr {
  let Spellings = [GNU<"NSObject">];
  let Documentation = [Undocumented];
}

def ObjCIndependentClass : InheritableAttr {
  let Spellings = [GNU<"objc_independent_class">];
  let Documentation = [Undocumented];
}

def ObjCPreciseLifetime : InheritableAttr {
  let Spellings = [GNU<"objc_precise_lifetime">];
  let Subjects = SubjectList<[Var], ErrorDiag>;
  let Documentation = [Undocumented];
}

def ObjCReturnsInnerPointer : InheritableAttr {
  let Spellings = [GNU<"objc_returns_inner_pointer">];
  let Subjects = SubjectList<[ObjCMethod, ObjCProperty], ErrorDiag>;
  let Documentation = [Undocumented];
}

def ObjCRequiresSuper : InheritableAttr {
  let Spellings = [GNU<"objc_requires_super">];
  let Subjects = SubjectList<[ObjCMethod], ErrorDiag>;
  let Documentation = [ObjCRequiresSuperDocs];
}

def ObjCRootClass : InheritableAttr {
  let Spellings = [GNU<"objc_root_class">];
  let Subjects = SubjectList<[ObjCInterface], ErrorDiag>;
  let Documentation = [Undocumented];
}

def ObjCExplicitProtocolImpl : InheritableAttr {
  let Spellings = [GNU<"objc_protocol_requires_explicit_implementation">];
  let Subjects = SubjectList<[ObjCProtocol], ErrorDiag>;
  let Documentation = [Undocumented];
}

def ObjCDesignatedInitializer : Attr {
  let Spellings = [GNU<"objc_designated_initializer">];
  let Subjects = SubjectList<[ObjCInterfaceDeclInitMethod], ErrorDiag,
                             "ExpectedObjCInterfaceDeclInitMethod">;
  let Documentation = [Undocumented];
}

def ObjCRuntimeName : Attr {
  let Spellings = [GNU<"objc_runtime_name">];
  let Subjects = SubjectList<[ObjCInterface, ObjCProtocol], ErrorDiag>;
  let Args = [StringArgument<"MetadataName">];
  let Documentation = [ObjCRuntimeNameDocs];
}

def ObjCBoxable : Attr {
  let Spellings = [GNU<"objc_boxable">];
  let Subjects = SubjectList<[Record], ErrorDiag, "ExpectedStructOrUnion">;
  let Documentation = [ObjCBoxableDocs];
}

def OptimizeNone : InheritableAttr {
  let Spellings = [GNU<"optnone">, CXX11<"clang", "optnone">];
  let Subjects = SubjectList<[Function, ObjCMethod]>;
  let Documentation = [OptnoneDocs];
}

def Overloadable : Attr {
  let Spellings = [GNU<"overloadable">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [OverloadableDocs];
}

def Override : InheritableAttr {
  let Spellings = [Keyword<"override">];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

def Ownership : InheritableAttr {
  let Spellings = [GNU<"ownership_holds">, GNU<"ownership_returns">,
                   GNU<"ownership_takes">];
  let Accessors = [Accessor<"isHolds", [GNU<"ownership_holds">]>,
                   Accessor<"isReturns", [GNU<"ownership_returns">]>,
                   Accessor<"isTakes", [GNU<"ownership_takes">]>];
  let AdditionalMembers = [{
    enum OwnershipKind { Holds, Returns, Takes };
    OwnershipKind getOwnKind() const {
      return isHolds() ? Holds :
             isTakes() ? Takes :
             Returns;
    }
  }];
  let Args = [IdentifierArgument<"Module">, VariadicUnsignedArgument<"Args">];
  let Subjects = SubjectList<[HasFunctionProto], WarnDiag, "ExpectedFunction">;
  let Documentation = [Undocumented];
}

def Packed : InheritableAttr {
  let Spellings = [GCC<"packed">];
//  let Subjects = [Tag, Field];
  let Documentation = [Undocumented];
}

def IntelOclBicc : InheritableAttr {
  let Spellings = [GNU<"intel_ocl_bicc">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [Undocumented];
}

def Pcs : InheritableAttr {
  let Spellings = [GCC<"pcs">];
  let Args = [EnumArgument<"PCS", "PCSType",
                           ["aapcs", "aapcs-vfp"],
                           ["AAPCS", "AAPCS_VFP"]>];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [PcsDocs];
}

def Pure : InheritableAttr {
  let Spellings = [GCC<"pure">];
  let Documentation = [Undocumented];
}

def Regparm : TypeAttr {
  let Spellings = [GCC<"regparm">];
  let Args = [UnsignedArgument<"NumParams">];
  let Documentation = [RegparmDocs];
}

def ReqdWorkGroupSize : InheritableAttr {
  let Spellings = [GNU<"reqd_work_group_size">];
  let Args = [UnsignedArgument<"XDim">, UnsignedArgument<"YDim">,
              UnsignedArgument<"ZDim">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

def WorkGroupSizeHint :  InheritableAttr {
  let Spellings = [GNU<"work_group_size_hint">];
  let Args = [UnsignedArgument<"XDim">,
              UnsignedArgument<"YDim">,
              UnsignedArgument<"ZDim">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

def InitPriority : InheritableAttr {
  let Spellings = [GNU<"init_priority">];
  let Args = [UnsignedArgument<"Priority">];
  let Subjects = SubjectList<[Var], ErrorDiag>;
  let Documentation = [Undocumented];
}

def Section : InheritableAttr {
  let Spellings = [GCC<"section">, Declspec<"allocate">];
  let Args = [StringArgument<"Name">];
  let Subjects = SubjectList<[Function, GlobalVar,
                              ObjCMethod, ObjCProperty], ErrorDiag,
                             "ExpectedFunctionGlobalVarMethodOrProperty">;
  let Documentation = [SectionDocs];
}

def Sentinel : InheritableAttr {
  let Spellings = [GCC<"sentinel">];
  let Args = [DefaultIntArgument<"Sentinel", 0>,
              DefaultIntArgument<"NullPos", 0>];
//  let Subjects = SubjectList<[Function, ObjCMethod, Block, Var]>;
  let Documentation = [Undocumented];
}

def StdCall : InheritableAttr {
  let Spellings = [GCC<"stdcall">, Keyword<"__stdcall">, Keyword<"_stdcall">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [StdCallDocs];
}

def SysVABI : InheritableAttr {
  let Spellings = [GCC<"sysv_abi">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [Undocumented];
}

def ThisCall : InheritableAttr {
  let Spellings = [GCC<"thiscall">, Keyword<"__thiscall">,
                   Keyword<"_thiscall">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [ThisCallDocs];
}

def VectorCall : InheritableAttr {
  let Spellings = [GNU<"vectorcall">, Keyword<"__vectorcall">,
                   Keyword<"_vectorcall">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [VectorCallDocs];
}

def Pascal : InheritableAttr {
  let Spellings = [GNU<"pascal">, Keyword<"__pascal">, Keyword<"_pascal">];
//  let Subjects = [Function, ObjCMethod];
  let Documentation = [Undocumented];
}

def Target : InheritableAttr {
  let Spellings = [GCC<"target">];
  let Args = [StringArgument<"features">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [TargetDocs];
}

def TransparentUnion : InheritableAttr {
  let Spellings = [GCC<"transparent_union">];
//  let Subjects = SubjectList<[Record, TypedefName]>;
  let Documentation = [Undocumented];
}

def Unavailable : InheritableAttr {
  let Spellings = [GNU<"unavailable">];
  let Args = [StringArgument<"Message", 1>];
  let Documentation = [Undocumented];
}

def ArcWeakrefUnavailable : InheritableAttr {
  let Spellings = [GNU<"objc_arc_weak_reference_unavailable">];
  let Subjects = SubjectList<[ObjCInterface], ErrorDiag>;
  let Documentation = [Undocumented];
}

def ObjCGC : TypeAttr {
  let Spellings = [GNU<"objc_gc">];
  let Args = [IdentifierArgument<"Kind">];
  let Documentation = [Undocumented];
}

def ObjCOwnership : InheritableAttr {
  let Spellings = [GNU<"objc_ownership">];
  let Args = [IdentifierArgument<"Kind">];
  let ASTNode = 0;
  let Documentation = [Undocumented];
}

def ObjCRequiresPropertyDefs : InheritableAttr {
  let Spellings = [GNU<"objc_requires_property_definitions">];
  let Subjects = SubjectList<[ObjCInterface], ErrorDiag>;
  let Documentation = [Undocumented];
}

def Unused : InheritableAttr {
  let Spellings = [GCC<"unused">];
  let Subjects = SubjectList<[Var, ObjCIvar, Type, Label, Field, ObjCMethod,
                              FunctionLike], WarnDiag,
                             "ExpectedVariableFunctionOrLabel">;
  let Documentation = [Undocumented];
}

def Used : InheritableAttr {
  let Spellings = [GCC<"used">];
  let Documentation = [Undocumented];
}

def Uuid : InheritableAttr {
  let Spellings = [Declspec<"uuid">];
  let Args = [StringArgument<"Guid">];
//  let Subjects = SubjectList<[CXXRecord]>;
  let LangOpts = [MicrosoftExt, Borland];
  let Documentation = [Undocumented];
}

def VectorSize : TypeAttr {
  let Spellings = [GCC<"vector_size">];
  let Args = [ExprArgument<"NumBytes">];
  let Documentation = [Undocumented];
}

def VecTypeHint : InheritableAttr {
  let Spellings = [GNU<"vec_type_hint">];
  let Args = [TypeArgument<"TypeHint">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

def Visibility : InheritableAttr {
  let Clone = 0;
  let Spellings = [GCC<"visibility">];
  let Args = [EnumArgument<"Visibility", "VisibilityType",
                           ["default", "hidden", "internal", "protected"],
                           ["Default", "Hidden", "Hidden", "Protected"]>];
  let Documentation = [Undocumented];
}

def TypeVisibility : InheritableAttr {
  let Clone = 0;
  let Spellings = [GNU<"type_visibility">, CXX11<"clang", "type_visibility">];
  let Args = [EnumArgument<"Visibility", "VisibilityType",
                           ["default", "hidden", "internal", "protected"],
                           ["Default", "Hidden", "Hidden", "Protected"]>];
//  let Subjects = [Tag, ObjCInterface, Namespace];
  let Documentation = [Undocumented];
}

def VecReturn : InheritableAttr {
  let Spellings = [GNU<"vecreturn">];
  let Subjects = SubjectList<[CXXRecord], ErrorDiag>;
  let Documentation = [Undocumented];
}

def WarnUnused : InheritableAttr {
  let Spellings = [GNU<"warn_unused">];
  let Subjects = SubjectList<[Record]>;
  let Documentation = [Undocumented];
}

def WarnUnusedResult : InheritableAttr {
  let Spellings = [GCC<"warn_unused_result">,
                   CXX11<"clang", "warn_unused_result">];
  let Subjects = SubjectList<[ObjCMethod, CXXRecord, FunctionLike], WarnDiag,
                             "ExpectedFunctionMethodOrClass">;
  let Documentation = [Undocumented];
}

def Weak : InheritableAttr {
  let Spellings = [GCC<"weak">];
  let Subjects = SubjectList<[Var, Function, CXXRecord]>;
  let Documentation = [Undocumented];
}

def WeakImport : InheritableAttr {
  let Spellings = [GNU<"weak_import">];
  let Documentation = [Undocumented];
}

def WeakRef : InheritableAttr {
  let Spellings = [GCC<"weakref">];
  // A WeakRef that has an argument is treated as being an AliasAttr
  let Args = [StringArgument<"Aliasee", 1>];
  let Subjects = SubjectList<[Var, Function], ErrorDiag>;
  let Documentation = [Undocumented];
}

def X86ForceAlignArgPointer : InheritableAttr, TargetSpecificAttr<TargetX86> {
  let Spellings = [GNU<"force_align_arg_pointer">];
  // Technically, this appertains to a FunctionDecl, but the target-specific
  // code silently allows anything function-like (such as typedefs or function
  // pointers), but does not apply the attribute to them.
  let Documentation = [Undocumented];
}

def NoSanitize : InheritableAttr {
  let Spellings = [GNU<"no_sanitize">, CXX11<"clang", "no_sanitize">];
  let Args = [VariadicStringArgument<"Sanitizers">];
  let Subjects = SubjectList<[Function, ObjCMethod], ErrorDiag>;
  let Documentation = [NoSanitizeDocs];
  let AdditionalMembers = [{
    SanitizerMask getMask() const {
      SanitizerMask Mask = 0;
      for (auto SanitizerName : sanitizers()) {
        SanitizerMask ParsedMask =
            parseSanitizerValue(SanitizerName, /*AllowGroups=*/true);
        Mask |= expandSanitizerGroups(ParsedMask);
      }
      return Mask;
    }
  }];
}

// Attributes to disable a specific sanitizer. No new sanitizers should be added
// to this list; the no_sanitize attribute should be extended instead.
def NoSanitizeSpecific : InheritableAttr {
  let Spellings = [GCC<"no_address_safety_analysis">,
                   GCC<"no_sanitize_address">,
                   GCC<"no_sanitize_thread">,
                   GNU<"no_sanitize_memory">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [NoSanitizeAddressDocs, NoSanitizeThreadDocs,
                       NoSanitizeMemoryDocs];
  let ASTNode = 0;
}

// C/C++ Thread safety attributes (e.g. for deadlock, data race checking)

def GuardedVar : InheritableAttr {
  let Spellings = [GNU<"guarded_var">];
  let Subjects = SubjectList<[Field, SharedVar], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
}

def PtGuardedVar : InheritableAttr {
  let Spellings = [GNU<"pt_guarded_var">];
  let Subjects = SubjectList<[Field, SharedVar], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
}

def Lockable : InheritableAttr {
  let Spellings = [GNU<"lockable">];
  let Subjects = SubjectList<[Record]>;
  let Documentation = [Undocumented];
  let ASTNode = 0;  // Replaced by Capability
}

def ScopedLockable : InheritableAttr {
  let Spellings = [GNU<"scoped_lockable">];
  let Subjects = SubjectList<[Record]>;
  let Documentation = [Undocumented];
}

def Capability : InheritableAttr {
  let Spellings = [GNU<"capability">, CXX11<"clang", "capability">,
                   GNU<"shared_capability">,
                   CXX11<"clang", "shared_capability">];
  let Subjects = SubjectList<[Struct, TypedefName], ErrorDiag,
                             "ExpectedStructOrTypedef">;
  let Args = [StringArgument<"Name">];
  let Accessors = [Accessor<"isShared",
                    [GNU<"shared_capability">,
                     CXX11<"clang","shared_capability">]>];
  let Documentation = [Undocumented];
  let AdditionalMembers = [{
    bool isMutex() const { return getName().equals_lower("mutex"); }
    bool isRole() const { return getName().equals_lower("role"); }
  }];
}

def AssertCapability : InheritableAttr {
  let Spellings = [GNU<"assert_capability">,
                   CXX11<"clang", "assert_capability">,
                   GNU<"assert_shared_capability">,
                   CXX11<"clang", "assert_shared_capability">];
  let Subjects = SubjectList<[Function]>;
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Args = [ExprArgument<"Expr">];
  let Accessors = [Accessor<"isShared",
                    [GNU<"assert_shared_capability">,
                     CXX11<"clang", "assert_shared_capability">]>];
  let Documentation = [AssertCapabilityDocs];
}

def AcquireCapability : InheritableAttr {
  let Spellings = [GNU<"acquire_capability">,
                   CXX11<"clang", "acquire_capability">,
                   GNU<"acquire_shared_capability">,
                   CXX11<"clang", "acquire_shared_capability">,
                   GNU<"exclusive_lock_function">,
                   GNU<"shared_lock_function">];
  let Subjects = SubjectList<[Function]>;
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Args = [VariadicExprArgument<"Args">];
  let Accessors = [Accessor<"isShared",
                    [GNU<"acquire_shared_capability">,
                     CXX11<"clang", "acquire_shared_capability">,
                     GNU<"shared_lock_function">]>];
  let Documentation = [AcquireCapabilityDocs];
}

def TryAcquireCapability : InheritableAttr {
  let Spellings = [GNU<"try_acquire_capability">,
                   CXX11<"clang", "try_acquire_capability">,
                   GNU<"try_acquire_shared_capability">,
                   CXX11<"clang", "try_acquire_shared_capability">];
  let Subjects = SubjectList<[Function],
                             ErrorDiag>;
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Args = [ExprArgument<"SuccessValue">, VariadicExprArgument<"Args">];
  let Accessors = [Accessor<"isShared",
                    [GNU<"try_acquire_shared_capability">,
                     CXX11<"clang", "try_acquire_shared_capability">]>];
  let Documentation = [TryAcquireCapabilityDocs];
}

def ReleaseCapability : InheritableAttr {
  let Spellings = [GNU<"release_capability">,
                   CXX11<"clang", "release_capability">,
                   GNU<"release_shared_capability">,
                   CXX11<"clang", "release_shared_capability">,
                   GNU<"release_generic_capability">,
                   CXX11<"clang", "release_generic_capability">,
                   GNU<"unlock_function">];
  let Subjects = SubjectList<[Function]>;
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Args = [VariadicExprArgument<"Args">];
  let Accessors = [Accessor<"isShared",
                    [GNU<"release_shared_capability">,
                     CXX11<"clang", "release_shared_capability">]>,
                   Accessor<"isGeneric",
                     [GNU<"release_generic_capability">,
                      CXX11<"clang", "release_generic_capability">,
                      GNU<"unlock_function">]>];
  let Documentation = [ReleaseCapabilityDocs];
}

def RequiresCapability : InheritableAttr {
  let Spellings = [GNU<"requires_capability">,
                   CXX11<"clang", "requires_capability">,
                   GNU<"exclusive_locks_required">,
                   GNU<"requires_shared_capability">,
                   CXX11<"clang", "requires_shared_capability">,
                   GNU<"shared_locks_required">];
  let Args = [VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Function]>;
  let Accessors = [Accessor<"isShared", [GNU<"requires_shared_capability">,
                                         GNU<"shared_locks_required">,
                                CXX11<"clang","requires_shared_capability">]>];
  let Documentation = [Undocumented];
}

def NoThreadSafetyAnalysis : InheritableAttr {
  let Spellings = [GNU<"no_thread_safety_analysis">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def GuardedBy : InheritableAttr {
  let Spellings = [GNU<"guarded_by">];
  let Args = [ExprArgument<"Arg">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Field, SharedVar], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
}

def PtGuardedBy : InheritableAttr {
  let Spellings = [GNU<"pt_guarded_by">];
  let Args = [ExprArgument<"Arg">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Field, SharedVar], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
}

def AcquiredAfter : InheritableAttr {
  let Spellings = [GNU<"acquired_after">];
  let Args = [VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Field, SharedVar], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
}

def AcquiredBefore : InheritableAttr {
  let Spellings = [GNU<"acquired_before">];
  let Args = [VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Field, SharedVar], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
}

def AssertExclusiveLock : InheritableAttr {
  let Spellings = [GNU<"assert_exclusive_lock">];
  let Args = [VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def AssertSharedLock : InheritableAttr {
  let Spellings = [GNU<"assert_shared_lock">];
  let Args = [VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

// The first argument is an integer or boolean value specifying the return value
// of a successful lock acquisition.
def ExclusiveTrylockFunction : InheritableAttr {
  let Spellings = [GNU<"exclusive_trylock_function">];
  let Args = [ExprArgument<"SuccessValue">, VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

// The first argument is an integer or boolean value specifying the return value
// of a successful lock acquisition.
def SharedTrylockFunction : InheritableAttr {
  let Spellings = [GNU<"shared_trylock_function">];
  let Args = [ExprArgument<"SuccessValue">, VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def LockReturned : InheritableAttr {
  let Spellings = [GNU<"lock_returned">];
  let Args = [ExprArgument<"Arg">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

def LocksExcluded : InheritableAttr {
  let Spellings = [GNU<"locks_excluded">];
  let Args = [VariadicExprArgument<"Args">];
  let LateParsed = 1;
  let TemplateDependent = 1;
  let ParseArgumentsAsUnevaluated = 1;
  let DuplicatesAllowedWhileMerging = 1;
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
}

// C/C++ consumed attributes.

def Consumable : InheritableAttr {
  let Spellings = [GNU<"consumable">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Args = [EnumArgument<"DefaultState", "ConsumedState",
                           ["unknown", "consumed", "unconsumed"],
                           ["Unknown", "Consumed", "Unconsumed"]>];
  let Documentation = [ConsumableDocs];
}

def ConsumableAutoCast : InheritableAttr {
  let Spellings = [GNU<"consumable_auto_cast_state">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def ConsumableSetOnRead : InheritableAttr {
  let Spellings = [GNU<"consumable_set_state_on_read">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [Undocumented];
}

def CallableWhen : InheritableAttr {
  let Spellings = [GNU<"callable_when">];
  let Subjects = SubjectList<[CXXMethod]>;
  let Args = [VariadicEnumArgument<"CallableStates", "ConsumedState",
                                   ["unknown", "consumed", "unconsumed"],
                                   ["Unknown", "Consumed", "Unconsumed"]>];
  let Documentation = [CallableWhenDocs];
}

def ParamTypestate : InheritableAttr {
  let Spellings = [GNU<"param_typestate">];
  let Subjects = SubjectList<[ParmVar]>;
  let Args = [EnumArgument<"ParamState", "ConsumedState",
                           ["unknown", "consumed", "unconsumed"],
                           ["Unknown", "Consumed", "Unconsumed"]>];
  let Documentation = [ParamTypestateDocs];
}

def ReturnTypestate : InheritableAttr {
  let Spellings = [GNU<"return_typestate">];
  let Subjects = SubjectList<[Function, ParmVar]>;
  let Args = [EnumArgument<"State", "ConsumedState",
                           ["unknown", "consumed", "unconsumed"],
                           ["Unknown", "Consumed", "Unconsumed"]>];
  let Documentation = [ReturnTypestateDocs];
}

def SetTypestate : InheritableAttr {
  let Spellings = [GNU<"set_typestate">];
  let Subjects = SubjectList<[CXXMethod]>;
  let Args = [EnumArgument<"NewState", "ConsumedState",
                           ["unknown", "consumed", "unconsumed"],
                           ["Unknown", "Consumed", "Unconsumed"]>];
  let Documentation = [SetTypestateDocs];
}

def TestTypestate : InheritableAttr {
  let Spellings = [GNU<"test_typestate">];
  let Subjects = SubjectList<[CXXMethod]>;
  let Args = [EnumArgument<"TestState", "ConsumedState",
                           ["consumed", "unconsumed"],
                           ["Consumed", "Unconsumed"]>];
  let Documentation = [TestTypestateDocs];
}

// Type safety attributes for `void *' pointers and type tags.

def ArgumentWithTypeTag : InheritableAttr {
  let Spellings = [GNU<"argument_with_type_tag">,
                   GNU<"pointer_with_type_tag">];
  let Args = [IdentifierArgument<"ArgumentKind">,
              UnsignedArgument<"ArgumentIdx">,
              UnsignedArgument<"TypeTagIdx">,
              BoolArgument<"IsPointer">];
  let HasCustomParsing = 1;
  let Documentation = [ArgumentWithTypeTagDocs, PointerWithTypeTagDocs];
}

def TypeTagForDatatype : InheritableAttr {
  let Spellings = [GNU<"type_tag_for_datatype">];
  let Args = [IdentifierArgument<"ArgumentKind">,
              TypeArgument<"MatchingCType">,
              BoolArgument<"LayoutCompatible">,
              BoolArgument<"MustBeNull">];
//  let Subjects = SubjectList<[Var], ErrorDiag>;
  let HasCustomParsing = 1;
  let Documentation = [TypeTagForDatatypeDocs];
}

// Microsoft-related attributes

def MSNoVTable : InheritableAttr {
  let Spellings = [Declspec<"novtable">];
  let Subjects = SubjectList<[CXXRecord]>;
  let Documentation = [MSNoVTableDocs];
}

def : IgnoredAttr {
  let Spellings = [Declspec<"property">];
}

def MSStruct : InheritableAttr {
  let Spellings = [GCC<"ms_struct">];
  let Subjects = SubjectList<[Record]>;
  let Documentation = [Undocumented];
}

def DLLExport : InheritableAttr, TargetSpecificAttr<TargetWindows> {
  let Spellings = [Declspec<"dllexport">, GCC<"dllexport">];
  let Subjects = SubjectList<[Function, Var, CXXRecord]>;
  let Documentation = [Undocumented];
}

def DLLImport : InheritableAttr, TargetSpecificAttr<TargetWindows> {
  let Spellings = [Declspec<"dllimport">, GCC<"dllimport">];
  let Subjects = SubjectList<[Function, Var, CXXRecord]>;
  let Documentation = [Undocumented];
}

def SelectAny : InheritableAttr {
  let Spellings = [Declspec<"selectany">];
  let LangOpts = [MicrosoftExt];
  let Documentation = [Undocumented];
}

def Thread : Attr {
  let Spellings = [Declspec<"thread">];
  let LangOpts = [MicrosoftExt];
  let Documentation = [ThreadDocs];
  let Subjects = SubjectList<[Var]>;
}

def Win64 : IgnoredAttr {
  let Spellings = [Keyword<"__w64">];
  let LangOpts = [MicrosoftExt];
}

def Ptr32 : TypeAttr {
  let Spellings = [Keyword<"__ptr32">];
  let Documentation = [Undocumented];
}

def Ptr64 : TypeAttr {
  let Spellings = [Keyword<"__ptr64">];
  let Documentation = [Undocumented];
}

def SPtr : TypeAttr {
  let Spellings = [Keyword<"__sptr">];
  let Documentation = [Undocumented];
}

def UPtr : TypeAttr {
  let Spellings = [Keyword<"__uptr">];
  let Documentation = [Undocumented];
}

def MSInheritance : InheritableAttr {
  let LangOpts = [MicrosoftExt];
  let Args = [DefaultBoolArgument<"BestCase", 1>];
  let Spellings = [Keyword<"__single_inheritance">,
                   Keyword<"__multiple_inheritance">,
                   Keyword<"__virtual_inheritance">,
                   Keyword<"__unspecified_inheritance">];
  let AdditionalMembers = [{
  static bool hasVBPtrOffsetField(Spelling Inheritance) {
    return Inheritance == Keyword_unspecified_inheritance;
  }

  // Only member pointers to functions need a this adjustment, since it can be
  // combined with the field offset for data pointers.
  static bool hasNVOffsetField(bool IsMemberFunction, Spelling Inheritance) {
    return IsMemberFunction && Inheritance >= Keyword_multiple_inheritance;
  }

  static bool hasVBTableOffsetField(Spelling Inheritance) {
    return Inheritance >= Keyword_virtual_inheritance;
  }

  static bool hasOnlyOneField(bool IsMemberFunction,
                              Spelling Inheritance) {
    if (IsMemberFunction)
      return Inheritance <= Keyword_single_inheritance;
    return Inheritance <= Keyword_multiple_inheritance;
  }
  }];
  let Documentation = [MSInheritanceDocs];
}

def MSVtorDisp : InheritableAttr {
  // This attribute has no spellings as it is only ever created implicitly.
  let Spellings = [];
  let Args = [UnsignedArgument<"vdm">];
  let SemaHandler = 0;

  let AdditionalMembers = [{
  enum Mode {
    Never,
    ForVBaseOverride,
    ForVFTable
  };

  Mode getVtorDispMode() const { return Mode(vdm); }
  }];
  let Documentation = [Undocumented];
}

def InitSeg : Attr {
  let Spellings = [Pragma<"", "init_seg">];
  let Args = [StringArgument<"Section">];
  let SemaHandler = 0;
  let Documentation = [InitSegDocs];
  let AdditionalMembers = [{
  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    OS << '(' << getSection() << ')';
  }
  }];
}

def Unaligned : IgnoredAttr {
  let Spellings = [Keyword<"__unaligned">];
}

def LoopHint : Attr {
  /// #pragma clang loop <option> directive
  /// vectorize: vectorizes loop operations if State == Enable.
  /// vectorize_width: vectorize loop operations with width 'Value'.
  /// interleave: interleave multiple loop iterations if State == Enable.
  /// interleave_count: interleaves 'Value' loop interations.
  /// unroll: fully unroll loop if State == Enable.
  /// unroll_count: unrolls loop 'Value' times.

  /// #pragma unroll <argument> directive
  /// <no arg>: fully unrolls loop.
  /// boolean: fully unrolls loop if State == Enable.
  /// expression: unrolls loop 'Value' times.

  let Spellings = [Pragma<"clang", "loop">, Pragma<"", "unroll">,
                   Pragma<"", "nounroll">];

  /// State of the loop optimization specified by the spelling.
  let Args = [EnumArgument<"Option", "OptionType",
                          ["vectorize", "vectorize_width", "interleave", "interleave_count",
                           "unroll", "unroll_count"],
                          ["Vectorize", "VectorizeWidth", "Interleave", "InterleaveCount",
                           "Unroll", "UnrollCount"]>,
              EnumArgument<"State", "LoopHintState",
                           ["default", "enable", "disable", "assume_safety"],
                           ["Default", "Enable", "Disable", "AssumeSafety"]>,
              ExprArgument<"Value">];

  let AdditionalMembers = [{
  static const char *getOptionName(int Option) {
    switch(Option) {
    case Vectorize: return "vectorize";
    case VectorizeWidth: return "vectorize_width";
    case Interleave: return "interleave";
    case InterleaveCount: return "interleave_count";
    case Unroll: return "unroll";
    case UnrollCount: return "unroll_count";
    }
    llvm_unreachable("Unhandled LoopHint option.");
  }

  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    // For "#pragma unroll" and "#pragma nounroll" the string "unroll" or
    // "nounroll" is already emitted as the pragma name.
    if (SpellingIndex == Pragma_nounroll) {
      OS << "\n";
      return;
    }
    else if (SpellingIndex == Pragma_unroll) {
      OS << getValueString(Policy) << "\n";
      return;
    }

    assert(SpellingIndex == Pragma_clang_loop && "Unexpected spelling");
    OS << getOptionName(option) << getValueString(Policy) << "\n";
  }

  // Return a string containing the loop hint argument including the
  // enclosing parentheses.
  std::string getValueString(const PrintingPolicy &Policy) const {
    std::string ValueName;
    llvm::raw_string_ostream OS(ValueName);
    OS << "(";
    if (option == VectorizeWidth || option == InterleaveCount ||
        option == UnrollCount)
      value->printPretty(OS, nullptr, Policy);
    else if (state == Default)
      return "";
    else if (state == Enable)
      OS << (option == Unroll ? "full" : "enable");
    else if (state == AssumeSafety)
      OS << "assume_safety";
    else
      OS << "disable";
    OS << ")";
    return OS.str();
  }

  // Return a string suitable for identifying this attribute in diagnostics.
  std::string getDiagnosticName(const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    if (SpellingIndex == Pragma_nounroll)
      return "#pragma nounroll";
    else if (SpellingIndex == Pragma_unroll)
      return "#pragma unroll" + getValueString(Policy);

    assert(SpellingIndex == Pragma_clang_loop && "Unexpected spelling");
    return getOptionName(option) + getValueString(Policy);
  }
  }];

  let Documentation = [LoopHintDocs, UnrollHintDocs];
}

def CapturedRecord : InheritableAttr {
  // This attribute has no spellings as it is only ever created implicitly.
  let Spellings = [];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}

def OMPThreadPrivateDecl : InheritableAttr {
  // This attribute has no spellings as it is only ever created implicitly.
  let Spellings = [];
  let SemaHandler = 0;
  let Documentation = [Undocumented];
}
