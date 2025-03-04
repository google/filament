//===--- Preprocessor.h - C Language Family Preprocessor --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the clang::Preprocessor interface.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_PREPROCESSOR_H
#define LLVM_CLANG_LEX_PREPROCESSOR_H

#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/ModuleMap.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/PTHLexer.h"
#include "clang/Lex/PTHManager.h"
#include "clang/Lex/TokenLexer.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "llvm/Support/Allocator.h"
#include <memory>
#include <vector>

#include "llvm/Support/OacrIgnoreCond.h" // HLSL Change - options change some lexical rules (tokens, identifier chars, others)

namespace llvm {
  template<unsigned InternalLen> class SmallString;
}

namespace clang {

class SourceManager;
class ExternalPreprocessorSource;
class FileManager;
class FileEntry;
class HeaderSearch;
class PragmaNamespace;
class PragmaHandler;
class CommentHandler;
class ScratchBuffer;
class TargetInfo;
class PPCallbacks;
class CodeCompletionHandler;
class DirectoryLookup;
class PreprocessingRecord;
class ModuleLoader;
class PreprocessorOptions;

/// \brief Stores token information for comparing actual tokens with
/// predefined values.  Only handles simple tokens and identifiers.
class TokenValue {
  tok::TokenKind Kind;
  IdentifierInfo *II;

public:
  TokenValue(tok::TokenKind Kind) : Kind(Kind), II(nullptr) {
    assert(Kind != tok::raw_identifier && "Raw identifiers are not supported.");
    assert(Kind != tok::identifier &&
           "Identifiers should be created by TokenValue(IdentifierInfo *)");
    assert(!tok::isLiteral(Kind) && "Literals are not supported.");
    assert(!tok::isAnnotation(Kind) && "Annotations are not supported.");
  }
  TokenValue(IdentifierInfo *II) : Kind(tok::identifier), II(II) {}
  bool operator==(const Token &Tok) const {
    return Tok.getKind() == Kind &&
        (!II || II == Tok.getIdentifierInfo());
  }
};

/// \brief Context in which macro name is used.
enum MacroUse {
  MU_Other  = 0,  // other than #define or #undef
  MU_Define = 1,  // macro name specified in #define
  MU_Undef  = 2   // macro name specified in #undef
};

/// \brief Engages in a tight little dance with the lexer to efficiently
/// preprocess tokens.
///
/// Lexers know only about tokens within a single source file, and don't
/// know anything about preprocessor-level issues like the \#include stack,
/// token expansion, etc.
class Preprocessor : public RefCountedBase<Preprocessor> {
  IntrusiveRefCntPtr<PreprocessorOptions> PPOpts;
  DiagnosticsEngine        *Diags;
  LangOptions       &LangOpts;
  const TargetInfo  *Target;
  FileManager       &FileMgr;
  SourceManager     &SourceMgr;
  std::unique_ptr<ScratchBuffer> ScratchBuf;
  HeaderSearch      &HeaderInfo;
  ModuleLoader      &TheModuleLoader;

  /// \brief External source of macros.
  ExternalPreprocessorSource *ExternalSource;


  /// An optional PTHManager object used for getting tokens from
  /// a token cache rather than lexing the original source file.
  std::unique_ptr<PTHManager> PTH;

  /// A BumpPtrAllocator object used to quickly allocate and release
  /// objects internal to the Preprocessor.
  llvm::BumpPtrAllocator BP;

  /// Identifiers for builtin macros and other builtins.
  IdentifierInfo *Ident__LINE__, *Ident__FILE__;   // __LINE__, __FILE__
  IdentifierInfo *Ident__DATE__, *Ident__TIME__;   // __DATE__, __TIME__
  IdentifierInfo *Ident__INCLUDE_LEVEL__;          // __INCLUDE_LEVEL__
  IdentifierInfo *Ident__BASE_FILE__;              // __BASE_FILE__
  IdentifierInfo *Ident__TIMESTAMP__;              // __TIMESTAMP__
  IdentifierInfo *Ident__COUNTER__;                // __COUNTER__
  IdentifierInfo *Ident_Pragma, *Ident__pragma;    // _Pragma, __pragma
  IdentifierInfo *Ident__identifier;               // __identifier
  IdentifierInfo *Ident__VA_ARGS__;                // __VA_ARGS__
  IdentifierInfo *Ident__has_feature;              // __has_feature
  IdentifierInfo *Ident__has_extension;            // __has_extension
  IdentifierInfo *Ident__has_builtin;              // __has_builtin
  IdentifierInfo *Ident__has_attribute;            // __has_attribute
  IdentifierInfo *Ident__has_include;              // __has_include
  IdentifierInfo *Ident__has_include_next;         // __has_include_next
  IdentifierInfo *Ident__has_warning;              // __has_warning
  IdentifierInfo *Ident__is_identifier;            // __is_identifier
  IdentifierInfo *Ident__building_module;          // __building_module
  IdentifierInfo *Ident__MODULE__;                 // __MODULE__
  IdentifierInfo *Ident__has_cpp_attribute;        // __has_cpp_attribute
  IdentifierInfo *Ident__has_declspec;             // __has_declspec_attribute

  SourceLocation DATELoc, TIMELoc;
  unsigned CounterValue;  // Next __COUNTER__ value.

  enum {
    /// \brief Maximum depth of \#includes.
    MaxAllowedIncludeStackDepth = 200
  };

  // State that is set before the preprocessor begins.
  bool KeepComments : 1;
  bool KeepMacroComments : 1;
  bool SuppressIncludeNotFoundError : 1;

  // State that changes while the preprocessor runs:
  bool InMacroArgs : 1;            // True if parsing fn macro invocation args.

  /// Whether the preprocessor owns the header search object.
  bool OwnsHeaderSearch : 1;

  /// True if macro expansion is disabled.
  bool DisableMacroExpansion : 1;

  /// Temporarily disables DisableMacroExpansion (i.e. enables expansion)
  /// when parsing preprocessor directives.
  bool MacroExpansionInDirectivesOverride : 1;

  class ResetMacroExpansionHelper;

  /// \brief Whether we have already loaded macros from the external source.
  mutable bool ReadMacrosFromExternalSource : 1;

  /// \brief True if pragmas are enabled.
  bool PragmasEnabled : 1;

  /// \brief True if the current build action is a preprocessing action.
  bool PreprocessedOutput : 1;

  /// \brief True if we are currently preprocessing a #if or #elif directive
  bool ParsingIfOrElifDirective;

  /// \brief True if we are pre-expanding macro arguments.
  bool InMacroArgPreExpansion;

  /// \brief Mapping/lookup information for all identifiers in
  /// the program, including program keywords.
  mutable IdentifierTable Identifiers;

  /// \brief This table contains all the selectors in the program.
  ///
  /// Unlike IdentifierTable above, this table *isn't* populated by the
  /// preprocessor. It is declared/expanded here because its role/lifetime is
  /// conceptually similar to the IdentifierTable. In addition, the current
  /// control flow (in clang::ParseAST()), make it convenient to put here.
  ///
  /// FIXME: Make sure the lifetime of Identifiers/Selectors *isn't* tied to
  /// the lifetime of the preprocessor.
  SelectorTable Selectors;

  /// \brief Information about builtins.
  Builtin::Context BuiltinInfo;

  /// \brief Tracks all of the pragmas that the client registered
  /// with this preprocessor.
  std::unique_ptr<PragmaNamespace> PragmaHandlers;

  /// \brief Pragma handlers of the original source is stored here during the
  /// parsing of a model file.
  std::unique_ptr<PragmaNamespace> PragmaHandlersBackup;

  /// \brief Tracks all of the comment handlers that the client registered
  /// with this preprocessor.
  std::vector<CommentHandler *> CommentHandlers;

  /// \brief True if we want to ignore EOF token and continue later on (thus 
  /// avoid tearing the Lexer and etc. down).
  bool IncrementalProcessing;

  /// The kind of translation unit we are processing.
  TranslationUnitKind TUKind;

  /// \brief The code-completion handler.
  CodeCompletionHandler *CodeComplete;

  /// \brief The file that we're performing code-completion for, if any.
  const FileEntry *CodeCompletionFile;

  /// \brief The offset in file for the code-completion point.
  unsigned CodeCompletionOffset;

  /// \brief The location for the code-completion point. This gets instantiated
  /// when the CodeCompletionFile gets \#include'ed for preprocessing.
  SourceLocation CodeCompletionLoc;

  /// \brief The start location for the file of the code-completion point.
  ///
  /// This gets instantiated when the CodeCompletionFile gets \#include'ed
  /// for preprocessing.
  SourceLocation CodeCompletionFileLoc;

  /// \brief The source location of the \c import contextual keyword we just 
  /// lexed, if any.
  SourceLocation ModuleImportLoc;

  /// \brief The module import path that we're currently processing.
  SmallVector<std::pair<IdentifierInfo *, SourceLocation>, 2> ModuleImportPath;

  /// \brief Whether the last token we lexed was an '@'.
  bool LastTokenWasAt;

  /// \brief Whether the module import expects an identifier next. Otherwise,
  /// it expects a '.' or ';'.
  bool ModuleImportExpectsIdentifier;
  
  /// \brief The source location of the currently-active
  /// \#pragma clang arc_cf_code_audited begin.
  SourceLocation PragmaARCCFCodeAuditedLoc;

  /// \brief The source location of the currently-active
  /// \#pragma clang assume_nonnull begin.
  SourceLocation PragmaAssumeNonNullLoc;

  /// \brief True if we hit the code-completion point.
  bool CodeCompletionReached;

  /// \brief The directory that the main file should be considered to occupy,
  /// if it does not correspond to a real file (as happens when building a
  /// module).
  const DirectoryEntry *MainFileDir;

  /// \brief The number of bytes that we will initially skip when entering the
  /// main file, along with a flag that indicates whether skipping this number
  /// of bytes will place the lexer at the start of a line.
  ///
  /// This is used when loading a precompiled preamble.
  std::pair<int, bool> SkipMainFilePreamble;

  /// \brief The current top of the stack that we're lexing from if
  /// not expanding a macro and we are lexing directly from source code.
  ///
  /// Only one of CurLexer, CurPTHLexer, or CurTokenLexer will be non-null.
  std::unique_ptr<Lexer> CurLexer;

  /// \brief The current top of stack that we're lexing from if
  /// not expanding from a macro and we are lexing from a PTH cache.
  ///
  /// Only one of CurLexer, CurPTHLexer, or CurTokenLexer will be non-null.
  std::unique_ptr<PTHLexer> CurPTHLexer;

  /// \brief The current top of the stack what we're lexing from
  /// if not expanding a macro.
  ///
  /// This is an alias for either CurLexer or  CurPTHLexer.
  PreprocessorLexer *CurPPLexer;

  /// \brief Used to find the current FileEntry, if CurLexer is non-null
  /// and if applicable.
  ///
  /// This allows us to implement \#include_next and find directory-specific
  /// properties.
  const DirectoryLookup *CurDirLookup;

  /// \brief The current macro we are expanding, if we are expanding a macro.
  ///
  /// One of CurLexer and CurTokenLexer must be null.
  std::unique_ptr<TokenLexer> CurTokenLexer;

  /// \brief The kind of lexer we're currently working with.
  enum CurLexerKind {
    CLK_Lexer,
    CLK_PTHLexer,
    CLK_TokenLexer,
    CLK_CachingLexer,
    CLK_LexAfterModuleImport
  } CurLexerKind;

  /// \brief If the current lexer is for a submodule that is being built, this
  /// is that submodule.
  Module *CurSubmodule;

  /// \brief Keeps track of the stack of files currently
  /// \#included, and macros currently being expanded from, not counting
  /// CurLexer/CurTokenLexer.
  struct IncludeStackInfo {
    enum CurLexerKind           CurLexerKind;
    Module                     *TheSubmodule;
    std::unique_ptr<Lexer>      TheLexer;
    std::unique_ptr<PTHLexer>   ThePTHLexer;
    PreprocessorLexer          *ThePPLexer;
    std::unique_ptr<TokenLexer> TheTokenLexer;
    const DirectoryLookup      *TheDirLookup;

    // The following constructors are completely useless copies of the default
    // versions, only needed to pacify MSVC.
    IncludeStackInfo(enum CurLexerKind CurLexerKind, Module *TheSubmodule,
                     std::unique_ptr<Lexer> &&TheLexer,
                     std::unique_ptr<PTHLexer> &&ThePTHLexer,
                     PreprocessorLexer *ThePPLexer,
                     std::unique_ptr<TokenLexer> &&TheTokenLexer,
                     const DirectoryLookup *TheDirLookup)
        : CurLexerKind(std::move(CurLexerKind)),
          TheSubmodule(std::move(TheSubmodule)), TheLexer(std::move(TheLexer)),
          ThePTHLexer(std::move(ThePTHLexer)),
          ThePPLexer(std::move(ThePPLexer)),
          TheTokenLexer(std::move(TheTokenLexer)),
          TheDirLookup(std::move(TheDirLookup)) {}
    IncludeStackInfo(IncludeStackInfo &&RHS)
        : CurLexerKind(std::move(RHS.CurLexerKind)),
          TheSubmodule(std::move(RHS.TheSubmodule)),
          TheLexer(std::move(RHS.TheLexer)),
          ThePTHLexer(std::move(RHS.ThePTHLexer)),
          ThePPLexer(std::move(RHS.ThePPLexer)),
          TheTokenLexer(std::move(RHS.TheTokenLexer)),
          TheDirLookup(std::move(RHS.TheDirLookup)) {}
  };
  std::vector<IncludeStackInfo> IncludeMacroStack;

  /// \brief Actions invoked when some preprocessor activity is
  /// encountered (e.g. a file is \#included, etc).
  std::unique_ptr<PPCallbacks> Callbacks;

  struct MacroExpandsInfo {
    Token Tok;
    MacroDefinition MD;
    SourceRange Range;
    MacroExpandsInfo(Token Tok, MacroDefinition MD, SourceRange Range)
      : Tok(Tok), MD(MD), Range(Range) { }
  };
  SmallVector<MacroExpandsInfo, 2> DelayedMacroExpandsCallbacks;

  /// Information about a name that has been used to define a module macro.
  struct ModuleMacroInfo {
    ModuleMacroInfo(MacroDirective *MD)
        : MD(MD), ActiveModuleMacrosGeneration(0), IsAmbiguous(false) {}

    /// The most recent macro directive for this identifier.
    MacroDirective *MD;
    /// The active module macros for this identifier.
    llvm::TinyPtrVector<ModuleMacro*> ActiveModuleMacros;
    /// The generation number at which we last updated ActiveModuleMacros.
    /// \see Preprocessor::VisibleModules.
    unsigned ActiveModuleMacrosGeneration;
    /// Whether this macro name is ambiguous.
    bool IsAmbiguous;
    /// The module macros that are overridden by this macro.
    llvm::TinyPtrVector<ModuleMacro*> OverriddenMacros;
  };

  /// The state of a macro for an identifier.
  class MacroState {
    mutable llvm::PointerUnion<MacroDirective *, ModuleMacroInfo *> State;

    ModuleMacroInfo *getModuleInfo(Preprocessor &PP,
                                   const IdentifierInfo *II) const {
      // FIXME: Find a spare bit on IdentifierInfo and store a
      //        HasModuleMacros flag.
      if (!II->hasMacroDefinition() ||
          (!PP.getLangOpts().Modules &&
           !PP.getLangOpts().ModulesLocalVisibility) ||
          !PP.CurSubmoduleState->VisibleModules.getGeneration())
        return nullptr;

      auto *Info = State.dyn_cast<ModuleMacroInfo*>();
      if (!Info) {
        Info = new (PP.getPreprocessorAllocator())
            ModuleMacroInfo(State.get<MacroDirective *>());
        State = Info;
      }

      if (PP.CurSubmoduleState->VisibleModules.getGeneration() !=
          Info->ActiveModuleMacrosGeneration)
        PP.updateModuleMacroInfo(II, *Info);
      return Info;
    }

  public:
    MacroState() : MacroState(nullptr) {}
    MacroState(MacroDirective *MD) : State(MD) {}
    MacroState(MacroState &&O) LLVM_NOEXCEPT : State(O.State) {
      O.State = (MacroDirective *)nullptr;
    }
    MacroState &operator=(MacroState &&O) LLVM_NOEXCEPT {
      auto S = O.State;
      O.State = (MacroDirective *)nullptr;
      State = S;
      return *this;
    }
    ~MacroState() {
      if (auto *Info = State.dyn_cast<ModuleMacroInfo*>())
        Info->~ModuleMacroInfo();
    }

    MacroDirective *getLatest() const {
      if (auto *Info = State.dyn_cast<ModuleMacroInfo*>())
        return Info->MD;
      return State.get<MacroDirective*>();
    }
    void setLatest(MacroDirective *MD) {
      if (auto *Info = State.dyn_cast<ModuleMacroInfo*>())
        Info->MD = MD;
      else
        State = MD;
    }

    bool isAmbiguous(Preprocessor &PP, const IdentifierInfo *II) const {
      auto *Info = getModuleInfo(PP, II);
      return Info ? Info->IsAmbiguous : false;
    }
    ArrayRef<ModuleMacro *>
    getActiveModuleMacros(Preprocessor &PP, const IdentifierInfo *II) const {
      if (auto *Info = getModuleInfo(PP, II))
        return Info->ActiveModuleMacros;
      return None;
    }

    MacroDirective::DefInfo findDirectiveAtLoc(SourceLocation Loc,
                                               SourceManager &SourceMgr) const {
      // FIXME: Incorporate module macros into the result of this.
      if (auto *Latest = getLatest())
        return Latest->findDirectiveAtLoc(Loc, SourceMgr);
      return MacroDirective::DefInfo();
    }

    void overrideActiveModuleMacros(Preprocessor &PP, IdentifierInfo *II) {
      if (auto *Info = getModuleInfo(PP, II)) {
        Info->OverriddenMacros.insert(Info->OverriddenMacros.end(),
                                      Info->ActiveModuleMacros.begin(),
                                      Info->ActiveModuleMacros.end());
        Info->ActiveModuleMacros.clear();
        Info->IsAmbiguous = false;
      }
    }
    ArrayRef<ModuleMacro*> getOverriddenMacros() const {
      if (auto *Info = State.dyn_cast<ModuleMacroInfo*>())
        return Info->OverriddenMacros;
      return None;
    }
    void setOverriddenMacros(Preprocessor &PP,
                             ArrayRef<ModuleMacro *> Overrides) {
      auto *Info = State.dyn_cast<ModuleMacroInfo*>();
      if (!Info) {
        if (Overrides.empty())
          return;
        Info = new (PP.getPreprocessorAllocator())
            ModuleMacroInfo(State.get<MacroDirective *>());
        State = Info;
      }
      Info->OverriddenMacros.clear();
      Info->OverriddenMacros.insert(Info->OverriddenMacros.end(),
                                    Overrides.begin(), Overrides.end());
      Info->ActiveModuleMacrosGeneration = 0;
    }
  };

  /// For each IdentifierInfo that was associated with a macro, we
  /// keep a mapping to the history of all macro definitions and #undefs in
  /// the reverse order (the latest one is in the head of the list).
  ///
  /// This mapping lives within the \p CurSubmoduleState.
  typedef llvm::DenseMap<const IdentifierInfo *, MacroState> MacroMap;

  friend class ASTReader;

  struct SubmoduleState;

  /// \brief Information about a submodule that we're currently building.
  struct BuildingSubmoduleInfo {
    BuildingSubmoduleInfo(Module *M, SourceLocation ImportLoc,
                          SubmoduleState *OuterSubmoduleState)
        : M(M), ImportLoc(ImportLoc), OuterSubmoduleState(OuterSubmoduleState) {
    }

    /// The module that we are building.
    Module *M;
    /// The location at which the module was included.
    SourceLocation ImportLoc;
    /// The previous SubmoduleState.
    SubmoduleState *OuterSubmoduleState;
  };
  SmallVector<BuildingSubmoduleInfo, 8> BuildingSubmoduleStack;

  /// \brief Information about a submodule's preprocessor state.
  struct SubmoduleState {
    /// The macros for the submodule.
    MacroMap Macros;
    /// The set of modules that are visible within the submodule.
    VisibleModuleSet VisibleModules;
    // FIXME: CounterValue?
    // FIXME: PragmaPushMacroInfo?
  };
  std::map<Module*, SubmoduleState> Submodules;

  /// The preprocessor state for preprocessing outside of any submodule.
  SubmoduleState NullSubmoduleState;

  /// The current submodule state. Will be \p NullSubmoduleState if we're not
  /// in a submodule.
  SubmoduleState *CurSubmoduleState;

  /// The set of known macros exported from modules.
  llvm::FoldingSet<ModuleMacro> ModuleMacros;

  /// The list of module macros, for each identifier, that are not overridden by
  /// any other module macro.
  llvm::DenseMap<const IdentifierInfo *, llvm::TinyPtrVector<ModuleMacro*>>
      LeafModuleMacros;

  /// \brief Macros that we want to warn because they are not used at the end
  /// of the translation unit.
  ///
  /// We store just their SourceLocations instead of
  /// something like MacroInfo*. The benefit of this is that when we are
  /// deserializing from PCH, we don't need to deserialize identifier & macros
  /// just so that we can report that they are unused, we just warn using
  /// the SourceLocations of this set (that will be filled by the ASTReader).
  /// We are using SmallPtrSet instead of a vector for faster removal.
  typedef llvm::SmallPtrSet<SourceLocation, 32> WarnUnusedMacroLocsTy;
  WarnUnusedMacroLocsTy WarnUnusedMacroLocs;

  /// \brief A "freelist" of MacroArg objects that can be
  /// reused for quick allocation.
  MacroArgs *MacroArgCache;
  friend class MacroArgs;

  /// For each IdentifierInfo used in a \#pragma push_macro directive,
  /// we keep a MacroInfo stack used to restore the previous macro value.
  llvm::DenseMap<IdentifierInfo*, std::vector<MacroInfo*> > PragmaPushMacroInfo;

  // Various statistics we track for performance analysis.
  unsigned NumDirectives, NumDefined, NumUndefined, NumPragma;
  unsigned NumIf, NumElse, NumEndif;
  unsigned NumEnteredSourceFiles, MaxIncludeStackDepth;
  unsigned NumMacroExpanded, NumFnMacroExpanded, NumBuiltinMacroExpanded;
  unsigned NumFastMacroExpanded, NumTokenPaste, NumFastTokenPaste;
  unsigned NumSkipped;

  /// \brief The predefined macros that preprocessor should use from the
  /// command line etc.
  std::string Predefines;

  /// \brief The file ID for the preprocessor predefines.
  FileID PredefinesFileID;

  /// \{
  /// \brief Cache of macro expanders to reduce malloc traffic.
  enum { TokenLexerCacheSize = 8 };
  unsigned NumCachedTokenLexers;
  std::unique_ptr<TokenLexer> TokenLexerCache[TokenLexerCacheSize];
  /// \}

  /// \brief Keeps macro expanded tokens for TokenLexers.
  //
  /// Works like a stack; a TokenLexer adds the macro expanded tokens that is
  /// going to lex in the cache and when it finishes the tokens are removed
  /// from the end of the cache.
  SmallVector<Token, 16> MacroExpandedTokens;
  std::vector<std::pair<TokenLexer *, size_t> > MacroExpandingLexersStack;

  /// \brief A record of the macro definitions and expansions that
  /// occurred during preprocessing.
  ///
  /// This is an optional side structure that can be enabled with
  /// \c createPreprocessingRecord() prior to preprocessing.
  PreprocessingRecord *Record;

  /// Cached tokens state.
  typedef SmallVector<Token, 1> CachedTokensTy;

  /// \brief Cached tokens are stored here when we do backtracking or
  /// lookahead. They are "lexed" by the CachingLex() method.
  CachedTokensTy CachedTokens;

  /// \brief The position of the cached token that CachingLex() should
  /// "lex" next.
  ///
  /// If it points beyond the CachedTokens vector, it means that a normal
  /// Lex() should be invoked.
  CachedTokensTy::size_type CachedLexPos;

  /// \brief Stack of backtrack positions, allowing nested backtracks.
  ///
  /// The EnableBacktrackAtThisPos() method pushes a position to
  /// indicate where CachedLexPos should be set when the BackTrack() method is
  /// invoked (at which point the last position is popped).
  std::vector<CachedTokensTy::size_type> BacktrackPositions;

  struct MacroInfoChain {
    MacroInfo MI;
    MacroInfoChain *Next;
  };

  /// MacroInfos are managed as a chain for easy disposal.  This is the head
  /// of that list.
  MacroInfoChain *MIChainHead;

  struct DeserializedMacroInfoChain {
    MacroInfo MI;
    unsigned OwningModuleID; // MUST be immediately after the MacroInfo object
                     // so it can be accessed by MacroInfo::getOwningModuleID().
    DeserializedMacroInfoChain *Next;
  };
  DeserializedMacroInfoChain *DeserialMIChainHead;

public:
  Preprocessor(IntrusiveRefCntPtr<PreprocessorOptions> PPOpts,
               DiagnosticsEngine &diags, LangOptions &opts,
               SourceManager &SM, HeaderSearch &Headers,
               ModuleLoader &TheModuleLoader,
               IdentifierInfoLookup *IILookup = nullptr,
               bool OwnsHeaderSearch = false,
               TranslationUnitKind TUKind = TU_Complete);

  ~Preprocessor();

  /// \brief Initialize the preprocessor using information about the target.
  ///
  /// \param Target is owned by the caller and must remain valid for the
  /// lifetime of the preprocessor.
  void Initialize(const TargetInfo &Target);

  /// \brief Initialize the preprocessor to parse a model file
  ///
  /// To parse model files the preprocessor of the original source is reused to
  /// preserver the identifier table. However to avoid some duplicate
  /// information in the preprocessor some cleanup is needed before it is used
  /// to parse model files. This method does that cleanup.
  void InitializeForModelFile();

  /// \brief Cleanup after model file parsing
  void FinalizeForModelFile();

  /// \brief Retrieve the preprocessor options used to initialize this
  /// preprocessor.
  PreprocessorOptions &getPreprocessorOpts() const { return *PPOpts; }
  
  DiagnosticsEngine &getDiagnostics() const { return *Diags; }
  void setDiagnostics(DiagnosticsEngine &D) { Diags = &D; }

  const LangOptions &getLangOpts() const { return LangOpts; }
  const TargetInfo &getTargetInfo() const { return *Target; }
  FileManager &getFileManager() const { return FileMgr; }
  SourceManager &getSourceManager() const { return SourceMgr; }
  HeaderSearch &getHeaderSearchInfo() const { return HeaderInfo; }

  IdentifierTable &getIdentifierTable() { return Identifiers; }
  const IdentifierTable &getIdentifierTable() const { return Identifiers; }
  SelectorTable &getSelectorTable() { return Selectors; }
  Builtin::Context &getBuiltinInfo() { return BuiltinInfo; }
  llvm::BumpPtrAllocator &getPreprocessorAllocator() { return BP; }

  void setPTHManager(PTHManager* pm);

  PTHManager *getPTHManager() { return PTH.get(); }

  void setExternalSource(ExternalPreprocessorSource *Source) {
    ExternalSource = Source;
  }

  ExternalPreprocessorSource *getExternalSource() const {
    return ExternalSource;
  }

  /// \brief Retrieve the module loader associated with this preprocessor.
  ModuleLoader &getModuleLoader() const { return TheModuleLoader; }

  bool hadModuleLoaderFatalFailure() const {
    return TheModuleLoader.HadFatalFailure;
  }

  /// \brief True if we are currently preprocessing a #if or #elif directive
  bool isParsingIfOrElifDirective() const { 
    return ParsingIfOrElifDirective;
  }

  /// \brief Control whether the preprocessor retains comments in output.
  void SetCommentRetentionState(bool KeepComments, bool KeepMacroComments) {
    this->KeepComments = KeepComments | KeepMacroComments;
    this->KeepMacroComments = KeepMacroComments;
  }

  bool getCommentRetentionState() const { return KeepComments; }

  void setPragmasEnabled(bool Enabled) { PragmasEnabled = Enabled; }
  bool getPragmasEnabled() const { return PragmasEnabled; }

  void SetSuppressIncludeNotFoundError(bool Suppress) {
    SuppressIncludeNotFoundError = Suppress;
  }

  bool GetSuppressIncludeNotFoundError() {
    return SuppressIncludeNotFoundError;
  }

  /// Sets whether the preprocessor is responsible for producing output or if
  /// it is producing tokens to be consumed by Parse and Sema.
  void setPreprocessedOutput(bool IsPreprocessedOutput) {
    PreprocessedOutput = IsPreprocessedOutput;
  }

  /// Returns true if the preprocessor is responsible for generating output,
  /// false if it is producing tokens to be consumed by Parse and Sema.
  bool isPreprocessedOutput() const { return PreprocessedOutput; }

  /// \brief Return true if we are lexing directly from the specified lexer.
  bool isCurrentLexer(const PreprocessorLexer *L) const {
    return CurPPLexer == L;
  }

  /// \brief Return the current lexer being lexed from.
  ///
  /// Note that this ignores any potentially active macro expansions and _Pragma
  /// expansions going on at the time.
  PreprocessorLexer *getCurrentLexer() const { return CurPPLexer; }

  /// \brief Return the current file lexer being lexed from.
  ///
  /// Note that this ignores any potentially active macro expansions and _Pragma
  /// expansions going on at the time.
  PreprocessorLexer *getCurrentFileLexer() const;

  /// \brief Return the submodule owning the file being lexed.
  Module *getCurrentSubmodule() const { return CurSubmodule; }

  /// \brief Returns the FileID for the preprocessor predefines.
  FileID getPredefinesFileID() const { return PredefinesFileID; }

  /// \{
  /// \brief Accessors for preprocessor callbacks.
  ///
  /// Note that this class takes ownership of any PPCallbacks object given to
  /// it.
  PPCallbacks *getPPCallbacks() const { return Callbacks.get(); }
  void addPPCallbacks(std::unique_ptr<PPCallbacks> C) {
    if (Callbacks)
      C = llvm::make_unique<PPChainedCallbacks>(std::move(C),
                                                std::move(Callbacks));
    Callbacks = std::move(C);
  }
  /// \}

  bool isMacroDefined(StringRef Id) {
    return isMacroDefined(&Identifiers.get(Id));
  }
  bool isMacroDefined(const IdentifierInfo *II) {
    return II->hasMacroDefinition() &&
           (!getLangOpts().Modules || (bool)getMacroDefinition(II));
  }

  /// \brief Determine whether II is defined as a macro within the module M,
  /// if that is a module that we've already preprocessed. Does not check for
  /// macros imported into M.
  bool isMacroDefinedInLocalModule(const IdentifierInfo *II, Module *M) {
    if (!II->hasMacroDefinition())
      return false;
    auto I = Submodules.find(M);
    if (I == Submodules.end())
      return false;
    auto J = I->second.Macros.find(II);
    if (J == I->second.Macros.end())
      return false;
    auto *MD = J->second.getLatest();
    return MD && MD->isDefined();
  }

  MacroDefinition getMacroDefinition(const IdentifierInfo *II) {
    if (!II->hasMacroDefinition())
      return MacroDefinition();

    MacroState &S = CurSubmoduleState->Macros[II];
    auto *MD = S.getLatest();
    while (MD && isa<VisibilityMacroDirective>(MD))
      MD = MD->getPrevious();
    return MacroDefinition(dyn_cast_or_null<DefMacroDirective>(MD),
                           S.getActiveModuleMacros(*this, II),
                           S.isAmbiguous(*this, II));
  }

  MacroDefinition getMacroDefinitionAtLoc(const IdentifierInfo *II,
                                          SourceLocation Loc) {
    if (!II->hadMacroDefinition())
      return MacroDefinition();

    MacroState &S = CurSubmoduleState->Macros[II];
    MacroDirective::DefInfo DI;
    if (auto *MD = S.getLatest())
      DI = MD->findDirectiveAtLoc(Loc, getSourceManager());
    // FIXME: Compute the set of active module macros at the specified location.
    return MacroDefinition(DI.getDirective(),
                           S.getActiveModuleMacros(*this, II),
                           S.isAmbiguous(*this, II));
  }

  /// \brief Given an identifier, return its latest non-imported MacroDirective
  /// if it is \#define'd and not \#undef'd, or null if it isn't \#define'd.
  MacroDirective *getLocalMacroDirective(const IdentifierInfo *II) const {
    if (!II->hasMacroDefinition())
      return nullptr;

    auto *MD = getLocalMacroDirectiveHistory(II);
    if (!MD || MD->getDefinition().isUndefined())
      return nullptr;

    return MD;
  }

  const MacroInfo *getMacroInfo(const IdentifierInfo *II) const {
    return const_cast<Preprocessor*>(this)->getMacroInfo(II);
  }

  MacroInfo *getMacroInfo(const IdentifierInfo *II) {
    if (!II->hasMacroDefinition())
      return nullptr;
    if (auto MD = getMacroDefinition(II))
      return MD.getMacroInfo();
    return nullptr;
  }

  /// \brief Given an identifier, return the latest non-imported macro
  /// directive for that identifier.
  ///
  /// One can iterate over all previous macro directives from the most recent
  /// one.
  MacroDirective *getLocalMacroDirectiveHistory(const IdentifierInfo *II) const;

  /// \brief Add a directive to the macro directive history for this identifier.
  void appendMacroDirective(IdentifierInfo *II, MacroDirective *MD);
  DefMacroDirective *appendDefMacroDirective(IdentifierInfo *II, MacroInfo *MI,
                                             SourceLocation Loc) {
    DefMacroDirective *MD = AllocateDefMacroDirective(MI, Loc);
    appendMacroDirective(II, MD);
    return MD;
  }
  DefMacroDirective *appendDefMacroDirective(IdentifierInfo *II,
                                             MacroInfo *MI) {
    return appendDefMacroDirective(II, MI, MI->getDefinitionLoc());
  }
  /// \brief Set a MacroDirective that was loaded from a PCH file.
  void setLoadedMacroDirective(IdentifierInfo *II, MacroDirective *MD);

  /// \brief Register an exported macro for a module and identifier.
  ModuleMacro *addModuleMacro(Module *Mod, IdentifierInfo *II, MacroInfo *Macro,
                              ArrayRef<ModuleMacro *> Overrides, bool &IsNew);
  ModuleMacro *getModuleMacro(Module *Mod, IdentifierInfo *II);

  /// \brief Get the list of leaf (non-overridden) module macros for a name.
  ArrayRef<ModuleMacro*> getLeafModuleMacros(const IdentifierInfo *II) const {
    auto I = LeafModuleMacros.find(II);
    if (I != LeafModuleMacros.end())
      return I->second;
    return None;
  }

  /// \{
  /// Iterators for the macro history table. Currently defined macros have
  /// IdentifierInfo::hasMacroDefinition() set and an empty
  /// MacroInfo::getUndefLoc() at the head of the list.
  typedef MacroMap::const_iterator macro_iterator;
  macro_iterator macro_begin(bool IncludeExternalMacros = true) const;
  macro_iterator macro_end(bool IncludeExternalMacros = true) const;
  llvm::iterator_range<macro_iterator>
  macros(bool IncludeExternalMacros = true) const {
    return llvm::make_range(macro_begin(IncludeExternalMacros),
                            macro_end(IncludeExternalMacros));
  }
  /// \}

  /// \brief Return the name of the macro defined before \p Loc that has
  /// spelling \p Tokens.  If there are multiple macros with same spelling,
  /// return the last one defined.
  StringRef getLastMacroWithSpelling(SourceLocation Loc,
                                     ArrayRef<TokenValue> Tokens) const;

  const std::string &getPredefines() const { return Predefines; }
  /// \brief Set the predefines for this Preprocessor.
  ///
  /// These predefines are automatically injected when parsing the main file.
  void setPredefines(const char *P) { Predefines = P; }
  void setPredefines(StringRef P) { Predefines = P; }

  /// Return information about the specified preprocessor
  /// identifier token.
  IdentifierInfo *getIdentifierInfo(StringRef Name) const {
    return &Identifiers.get(Name);
  }

  /// \brief Add the specified pragma handler to this preprocessor.
  ///
  /// If \p Namespace is non-null, then it is a token required to exist on the
  /// pragma line before the pragma string starts, e.g. "STDC" or "GCC".
  void AddPragmaHandler(StringRef Namespace, PragmaHandler *Handler);
  void AddPragmaHandler(PragmaHandler *Handler) {
    AddPragmaHandler(StringRef(), Handler);
  }

  /// \brief Remove the specific pragma handler from this preprocessor.
  ///
  /// If \p Namespace is non-null, then it should be the namespace that
  /// \p Handler was added to. It is an error to remove a handler that
  /// has not been registered.
  void RemovePragmaHandler(StringRef Namespace, PragmaHandler *Handler);
  void RemovePragmaHandler(PragmaHandler *Handler) {
    RemovePragmaHandler(StringRef(), Handler);
  }

  /// Install empty handlers for all pragmas (making them ignored).
  void IgnorePragmas();

  /// \brief Add the specified comment handler to the preprocessor.
  void addCommentHandler(CommentHandler *Handler);

  /// \brief Remove the specified comment handler.
  ///
  /// It is an error to remove a handler that has not been registered.
  void removeCommentHandler(CommentHandler *Handler);

  /// \brief Set the code completion handler to the given object.
  void setCodeCompletionHandler(CodeCompletionHandler &Handler) {
    CodeComplete = &Handler;
  }

  /// \brief Retrieve the current code-completion handler.
  CodeCompletionHandler *getCodeCompletionHandler() const {
    return CodeComplete;
  }

  /// \brief Clear out the code completion handler.
  void clearCodeCompletionHandler() {
    CodeComplete = nullptr;
  }

  /// \brief Hook used by the lexer to invoke the "natural language" code
  /// completion point.
  void CodeCompleteNaturalLanguage();

  /// \brief Retrieve the preprocessing record, or NULL if there is no
  /// preprocessing record.
  PreprocessingRecord *getPreprocessingRecord() const { return Record; }

  /// \brief Create a new preprocessing record, which will keep track of
  /// all macro expansions, macro definitions, etc.
  void createPreprocessingRecord();

  /// \brief Enter the specified FileID as the main source file,
  /// which implicitly adds the builtin defines etc.
  void EnterMainSourceFile();

  /// \brief Inform the preprocessor callbacks that processing is complete.
  void EndSourceFile();

  /// \brief Add a source file to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  ///
  /// Emits a diagnostic, doesn't enter the file, and returns true on error.
  bool EnterSourceFile(FileID CurFileID, const DirectoryLookup *Dir,
                       SourceLocation Loc);

  /// \brief Add a Macro to the top of the include stack and start lexing
  /// tokens from it instead of the current buffer.
  ///
  /// \param Args specifies the tokens input to a function-like macro.
  /// \param ILEnd specifies the location of the ')' for a function-like macro
  /// or the identifier for an object-like macro.
  void EnterMacro(Token &Identifier, SourceLocation ILEnd, MacroInfo *Macro,
                  MacroArgs *Args);

  /// \brief Add a "macro" context to the top of the include stack,
  /// which will cause the lexer to start returning the specified tokens.
  ///
  /// If \p DisableMacroExpansion is true, tokens lexed from the token stream
  /// will not be subject to further macro expansion. Otherwise, these tokens
  /// will be re-macro-expanded when/if expansion is enabled.
  ///
  /// If \p OwnsTokens is false, this method assumes that the specified stream
  /// of tokens has a permanent owner somewhere, so they do not need to be
  /// copied. If it is true, it assumes the array of tokens is allocated with
  /// \c new[] and must be freed.
  void EnterTokenStream(const Token *Toks, unsigned NumToks,
                        bool DisableMacroExpansion, bool OwnsTokens);

  /// \brief Pop the current lexer/macro exp off the top of the lexer stack.
  ///
  /// This should only be used in situations where the current state of the
  /// top-of-stack lexer is known.
  void RemoveTopOfLexerStack();

  /// From the point that this method is called, and until
  /// CommitBacktrackedTokens() or Backtrack() is called, the Preprocessor
  /// keeps track of the lexed tokens so that a subsequent Backtrack() call will
  /// make the Preprocessor re-lex the same tokens.
  ///
  /// Nested backtracks are allowed, meaning that EnableBacktrackAtThisPos can
  /// be called multiple times and CommitBacktrackedTokens/Backtrack calls will
  /// be combined with the EnableBacktrackAtThisPos calls in reverse order.
  ///
  /// NOTE: *DO NOT* forget to call either CommitBacktrackedTokens or Backtrack
  /// at some point after EnableBacktrackAtThisPos. If you don't, caching of
  /// tokens will continue indefinitely.
  ///
  void EnableBacktrackAtThisPos();

  /// \brief Disable the last EnableBacktrackAtThisPos call.
  void CommitBacktrackedTokens();

  /// \brief Make Preprocessor re-lex the tokens that were lexed since
  /// EnableBacktrackAtThisPos() was previously called.
  void Backtrack();

  /// \brief True if EnableBacktrackAtThisPos() was called and
  /// caching of tokens is on.
  bool isBacktrackEnabled() const { return !BacktrackPositions.empty(); }

  /// \brief Lex the next token for this preprocessor.
  void Lex(Token &Result);

  void LexAfterModuleImport(Token &Result);

  void makeModuleVisible(Module *M, SourceLocation Loc);

  SourceLocation getModuleImportLoc(Module *M) const {
    return CurSubmoduleState->VisibleModules.getImportLoc(M);
  }

  /// \brief Lex a string literal, which may be the concatenation of multiple
  /// string literals and may even come from macro expansion.
  /// \returns true on success, false if a error diagnostic has been generated.
  bool LexStringLiteral(Token &Result, std::string &String,
                        const char *DiagnosticTag, bool AllowMacroExpansion) {
    if (AllowMacroExpansion)
      Lex(Result);
    else
      LexUnexpandedToken(Result);
    return FinishLexStringLiteral(Result, String, DiagnosticTag,
                                  AllowMacroExpansion);
  }

  /// \brief Complete the lexing of a string literal where the first token has
  /// already been lexed (see LexStringLiteral).
  bool FinishLexStringLiteral(Token &Result, std::string &String,
                              const char *DiagnosticTag,
                              bool AllowMacroExpansion);

  /// \brief Lex a token.  If it's a comment, keep lexing until we get
  /// something not a comment.
  ///
  /// This is useful in -E -C mode where comments would foul up preprocessor
  /// directive handling.
  void LexNonComment(Token &Result) {
    do
      Lex(Result);
    while (Result.getKind() == tok::comment);
  }

  /// \brief Just like Lex, but disables macro expansion of identifier tokens.
  void LexUnexpandedToken(Token &Result) {
    // Disable macro expansion.
    bool OldVal = DisableMacroExpansion;
    DisableMacroExpansion = true;
    // Lex the token.
    Lex(Result);

    // Reenable it.
    DisableMacroExpansion = OldVal;
  }

  /// \brief Like LexNonComment, but this disables macro expansion of
  /// identifier tokens.
  void LexUnexpandedNonComment(Token &Result) {
    do
      LexUnexpandedToken(Result);
    while (Result.getKind() == tok::comment);
  }

  /// \brief Parses a simple integer literal to get its numeric value.  Floating
  /// point literals and user defined literals are rejected.  Used primarily to
  /// handle pragmas that accept integer arguments.
  bool parseSimpleIntegerLiteral(Token &Tok, uint64_t &Value);

  /// Disables macro expansion everywhere except for preprocessor directives.
  void SetMacroExpansionOnlyInDirectives() {
    DisableMacroExpansion = true;
    MacroExpansionInDirectivesOverride = true;
  }

  /// \brief Peeks ahead N tokens and returns that token without consuming any
  /// tokens.
  ///
  /// LookAhead(0) returns the next token that would be returned by Lex(),
  /// LookAhead(1) returns the token after it, etc.  This returns normal
  /// tokens after phase 5.  As such, it is equivalent to using
  /// 'Lex', not 'LexUnexpandedToken'.
  const Token &LookAhead(unsigned N) {
    if (CachedLexPos + N < CachedTokens.size())
      return CachedTokens[CachedLexPos+N];
    else
      return PeekAhead(N+1);
  }

  /// \brief When backtracking is enabled and tokens are cached,
  /// this allows to revert a specific number of tokens.
  ///
  /// Note that the number of tokens being reverted should be up to the last
  /// backtrack position, not more.
  void RevertCachedTokens(unsigned N) {
    assert(isBacktrackEnabled() &&
           "Should only be called when tokens are cached for backtracking");
    assert(signed(CachedLexPos) - signed(N) >= signed(BacktrackPositions.back())
         && "Should revert tokens up to the last backtrack position, not more");
    assert(signed(CachedLexPos) - signed(N) >= 0 &&
           "Corrupted backtrack positions ?");
    CachedLexPos -= N;
  }

  /// \brief Enters a token in the token stream to be lexed next.
  ///
  /// If BackTrack() is called afterwards, the token will remain at the
  /// insertion point.
  void EnterToken(const Token &Tok) {
    EnterCachingLexMode();
    CachedTokens.insert(CachedTokens.begin()+CachedLexPos, Tok);
  }

  /// We notify the Preprocessor that if it is caching tokens (because
  /// backtrack is enabled) it should replace the most recent cached tokens
  /// with the given annotation token. This function has no effect if
  /// backtracking is not enabled.
  ///
  /// Note that the use of this function is just for optimization, so that the
  /// cached tokens doesn't get re-parsed and re-resolved after a backtrack is
  /// invoked.
  void AnnotateCachedTokens(const Token &Tok) {
    assert(Tok.isAnnotation() && "Expected annotation token");
    if (CachedLexPos != 0 && isBacktrackEnabled())
      AnnotatePreviousCachedTokens(Tok);
  }

  /// Get the location of the last cached token, suitable for setting the end
  /// location of an annotation token.
  SourceLocation getLastCachedTokenLocation() const {
    assert(CachedLexPos != 0);
    return CachedTokens[CachedLexPos-1].getLastLoc();
  }

  /// \brief Replace the last token with an annotation token.
  ///
  /// Like AnnotateCachedTokens(), this routine replaces an
  /// already-parsed (and resolved) token with an annotation
  /// token. However, this routine only replaces the last token with
  /// the annotation token; it does not affect any other cached
  /// tokens. This function has no effect if backtracking is not
  /// enabled.
  void ReplaceLastTokenWithAnnotation(const Token &Tok) {
    assert(Tok.isAnnotation() && "Expected annotation token");
    if (CachedLexPos != 0 && isBacktrackEnabled())
      CachedTokens[CachedLexPos-1] = Tok;
  }

  /// Update the current token to represent the provided
  /// identifier, in order to cache an action performed by typo correction.
  void TypoCorrectToken(const Token &Tok) {
    assert(Tok.getIdentifierInfo() && "Expected identifier token");
    if (CachedLexPos != 0 && isBacktrackEnabled())
      CachedTokens[CachedLexPos-1] = Tok;
  }

  /// \brief Recompute the current lexer kind based on the CurLexer/CurPTHLexer/
  /// CurTokenLexer pointers.
  void recomputeCurLexerKind();

  /// \brief Returns true if incremental processing is enabled
  bool isIncrementalProcessingEnabled() const { return IncrementalProcessing; }

  /// \brief Enables the incremental processing
  void enableIncrementalProcessing(bool value = true) {
    IncrementalProcessing = value;
  }
  
  /// \brief Specify the point at which code-completion will be performed.
  ///
  /// \param File the file in which code completion should occur. If
  /// this file is included multiple times, code-completion will
  /// perform completion the first time it is included. If NULL, this
  /// function clears out the code-completion point.
  ///
  /// \param Line the line at which code completion should occur
  /// (1-based).
  ///
  /// \param Column the column at which code completion should occur
  /// (1-based).
  ///
  /// \returns true if an error occurred, false otherwise.
  bool SetCodeCompletionPoint(const FileEntry *File,
                              unsigned Line, unsigned Column);

  /// \brief Determine if we are performing code completion.
  bool isCodeCompletionEnabled() const { return CodeCompletionFile != nullptr; }

  /// \brief Returns the location of the code-completion point.
  ///
  /// Returns an invalid location if code-completion is not enabled or the file
  /// containing the code-completion point has not been lexed yet.
  SourceLocation getCodeCompletionLoc() const { return CodeCompletionLoc; }

  /// \brief Returns the start location of the file of code-completion point.
  ///
  /// Returns an invalid location if code-completion is not enabled or the file
  /// containing the code-completion point has not been lexed yet.
  SourceLocation getCodeCompletionFileLoc() const {
    return CodeCompletionFileLoc;
  }

  /// \brief Returns true if code-completion is enabled and we have hit the
  /// code-completion point.
  bool isCodeCompletionReached() const { return CodeCompletionReached; }

  /// \brief Note that we hit the code-completion point.
  void setCodeCompletionReached() {
    assert(isCodeCompletionEnabled() && "Code-completion not enabled!");
    CodeCompletionReached = true;
    // Silence any diagnostics that occur after we hit the code-completion.
    getDiagnostics().setSuppressAllDiagnostics(true);
  }

  /// \brief The location of the currently-active \#pragma clang
  /// arc_cf_code_audited begin.
  ///
  /// Returns an invalid location if there is no such pragma active.
  SourceLocation getPragmaARCCFCodeAuditedLoc() const {
    return PragmaARCCFCodeAuditedLoc;
  }

  /// \brief Set the location of the currently-active \#pragma clang
  /// arc_cf_code_audited begin.  An invalid location ends the pragma.
  void setPragmaARCCFCodeAuditedLoc(SourceLocation Loc) {
    PragmaARCCFCodeAuditedLoc = Loc;
  }

  /// \brief The location of the currently-active \#pragma clang
  /// assume_nonnull begin.
  ///
  /// Returns an invalid location if there is no such pragma active.
  SourceLocation getPragmaAssumeNonNullLoc() const {
    return PragmaAssumeNonNullLoc;
  }

  /// \brief Set the location of the currently-active \#pragma clang
  /// assume_nonnull begin.  An invalid location ends the pragma.
  void setPragmaAssumeNonNullLoc(SourceLocation Loc) {
    PragmaAssumeNonNullLoc = Loc;
  }

  /// \brief Set the directory in which the main file should be considered
  /// to have been found, if it is not a real file.
  void setMainFileDir(const DirectoryEntry *Dir) {
    MainFileDir = Dir;
  }

  /// \brief Instruct the preprocessor to skip part of the main source file.
  ///
  /// \param Bytes The number of bytes in the preamble to skip.
  ///
  /// \param StartOfLine Whether skipping these bytes puts the lexer at the
  /// start of a line.
  void setSkipMainFilePreamble(unsigned Bytes, bool StartOfLine) {
    SkipMainFilePreamble.first = Bytes;
    SkipMainFilePreamble.second = StartOfLine;
  }

  /// Forwarding function for diagnostics.  This emits a diagnostic at
  /// the specified Token's location, translating the token's start
  /// position in the current buffer into a SourcePosition object for rendering.
  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags->Report(Loc, DiagID);
  }

  DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID) const {
    return Diags->Report(Tok.getLocation(), DiagID);
  }

  /// Return the 'spelling' of the token at the given
  /// location; does not go up to the spelling location or down to the
  /// expansion location.
  ///
  /// \param buffer A buffer which will be used only if the token requires
  ///   "cleaning", e.g. if it contains trigraphs or escaped newlines
  /// \param invalid If non-null, will be set \c true if an error occurs.
  StringRef getSpelling(SourceLocation loc,
                        SmallVectorImpl<char> &buffer,
                        bool *invalid = nullptr) const {
    return Lexer::getSpelling(loc, buffer, SourceMgr, LangOpts, invalid);
  }

  /// \brief Return the 'spelling' of the Tok token.
  ///
  /// The spelling of a token is the characters used to represent the token in
  /// the source file after trigraph expansion and escaped-newline folding.  In
  /// particular, this wants to get the true, uncanonicalized, spelling of
  /// things like digraphs, UCNs, etc.
  ///
  /// \param Invalid If non-null, will be set \c true if an error occurs.
  std::string getSpelling(const Token &Tok, bool *Invalid = nullptr) const {
    return Lexer::getSpelling(Tok, SourceMgr, LangOpts, Invalid);
  }

  /// \brief Get the spelling of a token into a preallocated buffer, instead
  /// of as an std::string.
  ///
  /// The caller is required to allocate enough space for the token, which is
  /// guaranteed to be at least Tok.getLength() bytes long. The length of the
  /// actual result is returned.
  ///
  /// Note that this method may do two possible things: it may either fill in
  /// the buffer specified with characters, or it may *change the input pointer*
  /// to point to a constant buffer with the data already in it (avoiding a
  /// copy).  The caller is not allowed to modify the returned buffer pointer
  /// if an internal buffer is returned.
  unsigned getSpelling(const Token &Tok, const char *&Buffer,
                       bool *Invalid = nullptr) const {
    return Lexer::getSpelling(Tok, Buffer, SourceMgr, LangOpts, Invalid);
  }

  /// \brief Get the spelling of a token into a SmallVector.
  ///
  /// Note that the returned StringRef may not point to the
  /// supplied buffer if a copy can be avoided.
  StringRef getSpelling(const Token &Tok,
                        SmallVectorImpl<char> &Buffer,
                        bool *Invalid = nullptr) const;

  /// \brief Relex the token at the specified location.
  /// \returns true if there was a failure, false on success.
  bool getRawToken(SourceLocation Loc, Token &Result,
                   bool IgnoreWhiteSpace = false) {
    return Lexer::getRawToken(Loc, Result, SourceMgr, LangOpts, IgnoreWhiteSpace);
  }

  /// \brief Given a Token \p Tok that is a numeric constant with length 1,
  /// return the character.
  char
  getSpellingOfSingleCharacterNumericConstant(const Token &Tok,
                                              bool *Invalid = nullptr) const {
    assert(Tok.is(tok::numeric_constant) &&
           Tok.getLength() == 1 && "Called on unsupported token");
    assert(!Tok.needsCleaning() && "Token can't need cleaning with length 1");

    // If the token is carrying a literal data pointer, just use it.
    if (const char *D = Tok.getLiteralData())
      return *D;

    // Otherwise, fall back on getCharacterData, which is slower, but always
    // works.
    return *SourceMgr.getCharacterData(Tok.getLocation(), Invalid);
  }

  /// \brief Retrieve the name of the immediate macro expansion.
  ///
  /// This routine starts from a source location, and finds the name of the
  /// macro responsible for its immediate expansion. It looks through any
  /// intervening macro argument expansions to compute this. It returns a
  /// StringRef that refers to the SourceManager-owned buffer of the source
  /// where that macro name is spelled. Thus, the result shouldn't out-live
  /// the SourceManager.
  StringRef getImmediateMacroName(SourceLocation Loc) {
    return Lexer::getImmediateMacroName(Loc, SourceMgr, getLangOpts());
  }

  /// \brief Plop the specified string into a scratch buffer and set the
  /// specified token's location and length to it. 
  ///
  /// If specified, the source location provides a location of the expansion
  /// point of the token.
  void CreateString(StringRef Str, Token &Tok,
                    SourceLocation ExpansionLocStart = SourceLocation(),
                    SourceLocation ExpansionLocEnd = SourceLocation());

  /// \brief Computes the source location just past the end of the
  /// token at this source location.
  ///
  /// This routine can be used to produce a source location that
  /// points just past the end of the token referenced by \p Loc, and
  /// is generally used when a diagnostic needs to point just after a
  /// token where it expected something different that it received. If
  /// the returned source location would not be meaningful (e.g., if
  /// it points into a macro), this routine returns an invalid
  /// source location.
  ///
  /// \param Offset an offset from the end of the token, where the source
  /// location should refer to. The default offset (0) produces a source
  /// location pointing just past the end of the token; an offset of 1 produces
  /// a source location pointing to the last character in the token, etc.
  SourceLocation getLocForEndOfToken(SourceLocation Loc, unsigned Offset = 0) {
    return Lexer::getLocForEndOfToken(Loc, Offset, SourceMgr, LangOpts);
  }

  /// \brief Returns true if the given MacroID location points at the first
  /// token of the macro expansion.
  ///
  /// \param MacroBegin If non-null and function returns true, it is set to
  /// begin location of the macro.
  bool isAtStartOfMacroExpansion(SourceLocation loc,
                                 SourceLocation *MacroBegin = nullptr) const {
    return Lexer::isAtStartOfMacroExpansion(loc, SourceMgr, LangOpts,
                                            MacroBegin);
  }

  /// \brief Returns true if the given MacroID location points at the last
  /// token of the macro expansion.
  ///
  /// \param MacroEnd If non-null and function returns true, it is set to
  /// end location of the macro.
  bool isAtEndOfMacroExpansion(SourceLocation loc,
                               SourceLocation *MacroEnd = nullptr) const {
    return Lexer::isAtEndOfMacroExpansion(loc, SourceMgr, LangOpts, MacroEnd);
  }

  /// \brief Print the token to stderr, used for debugging.
  void DumpToken(const Token &Tok, bool DumpFlags = false) const;
  void DumpLocation(SourceLocation Loc) const;
  void DumpMacro(const MacroInfo &MI) const;
  void dumpMacroInfo(const IdentifierInfo *II);

  /// \brief Given a location that specifies the start of a
  /// token, return a new location that specifies a character within the token.
  SourceLocation AdvanceToTokenCharacter(SourceLocation TokStart,
                                         unsigned Char) const {
    return Lexer::AdvanceToTokenCharacter(TokStart, Char, SourceMgr, LangOpts);
  }

  /// \brief Increment the counters for the number of token paste operations
  /// performed.
  ///
  /// If fast was specified, this is a 'fast paste' case we handled.
  void IncrementPasteCounter(bool isFast) {
    if (isFast)
      ++NumFastTokenPaste;
    else
      ++NumTokenPaste;
  }

  void PrintStats();

  size_t getTotalMemory() const;

  /// When the macro expander pastes together a comment (/##/) in Microsoft
  /// mode, this method handles updating the current state, returning the
  /// token on the next source line.
  void HandleMicrosoftCommentPaste(Token &Tok);

  //===--------------------------------------------------------------------===//
  // Preprocessor callback methods.  These are invoked by a lexer as various
  // directives and events are found.

  /// Given a tok::raw_identifier token, look up the
  /// identifier information for the token and install it into the token,
  /// updating the token kind accordingly.
  IdentifierInfo *LookUpIdentifierInfo(Token &Identifier) const;

private:
  llvm::DenseMap<IdentifierInfo*,unsigned> PoisonReasons;

public:

  /// \brief Specifies the reason for poisoning an identifier.
  ///
  /// If that identifier is accessed while poisoned, then this reason will be
  /// used instead of the default "poisoned" diagnostic.
  void SetPoisonReason(IdentifierInfo *II, unsigned DiagID);

  /// \brief Display reason for poisoned identifier.
  void HandlePoisonedIdentifier(Token & Tok);

  void MaybeHandlePoisonedIdentifier(Token & Identifier) {
    if(IdentifierInfo * II = Identifier.getIdentifierInfo()) {
      if(II->isPoisoned()) {
        HandlePoisonedIdentifier(Identifier);
      }
    }
  }

private:
  /// Identifiers used for SEH handling in Borland. These are only
  /// allowed in particular circumstances
  // __except block
  IdentifierInfo *Ident__exception_code,
                 *Ident___exception_code,
                 *Ident_GetExceptionCode;
  // __except filter expression
  IdentifierInfo *Ident__exception_info,
                 *Ident___exception_info,
                 *Ident_GetExceptionInfo;
  // __finally
  IdentifierInfo *Ident__abnormal_termination,
                 *Ident___abnormal_termination,
                 *Ident_AbnormalTermination;

  const char *getCurLexerEndPos();

public:
  void PoisonSEHIdentifiers(bool Poison = true); // Borland

  /// \brief Callback invoked when the lexer reads an identifier and has
  /// filled in the tokens IdentifierInfo member. 
  ///
  /// This callback potentially macro expands it or turns it into a named
  /// token (like 'for').
  ///
  /// \returns true if we actually computed a token, false if we need to
  /// lex again.
  bool HandleIdentifier(Token &Identifier);


  /// \brief Callback invoked when the lexer hits the end of the current file.
  ///
  /// This either returns the EOF token and returns true, or
  /// pops a level off the include stack and returns false, at which point the
  /// client should call lex again.
  bool HandleEndOfFile(Token &Result, bool isEndOfMacro = false);

  /// \brief Callback invoked when the current TokenLexer hits the end of its
  /// token stream.
  bool HandleEndOfTokenLexer(Token &Result);

  /// \brief Callback invoked when the lexer sees a # token at the start of a
  /// line.
  ///
  /// This consumes the directive, modifies the lexer/preprocessor state, and
  /// advances the lexer(s) so that the next token read is the correct one.
  void HandleDirective(Token &Result);

  /// \brief Ensure that the next token is a tok::eod token.
  ///
  /// If not, emit a diagnostic and consume up until the eod.
  /// If \p EnableMacros is true, then we consider macros that expand to zero
  /// tokens as being ok.
  void CheckEndOfDirective(const char *Directive, bool EnableMacros = false);

  /// \brief Read and discard all tokens remaining on the current line until
  /// the tok::eod token is found.
  void DiscardUntilEndOfDirective();

  /// \brief Returns true if the preprocessor has seen a use of
  /// __DATE__ or __TIME__ in the file so far.
  bool SawDateOrTime() const {
    return DATELoc != SourceLocation() || TIMELoc != SourceLocation();
  }
  unsigned getCounterValue() const { return CounterValue; }
  void setCounterValue(unsigned V) { CounterValue = V; }

  /// \brief Retrieves the module that we're currently building, if any.
  Module *getCurrentModule();
  
  /// \brief Allocate a new MacroInfo object with the provided SourceLocation.
  MacroInfo *AllocateMacroInfo(SourceLocation L);

  /// \brief Allocate a new MacroInfo object loaded from an AST file.
  MacroInfo *AllocateDeserializedMacroInfo(SourceLocation L,
                                           unsigned SubModuleID);

  /// \brief Turn the specified lexer token into a fully checked and spelled
  /// filename, e.g. as an operand of \#include. 
  ///
  /// The caller is expected to provide a buffer that is large enough to hold
  /// the spelling of the filename, but is also expected to handle the case
  /// when this method decides to use a different buffer.
  ///
  /// \returns true if the input filename was in <>'s or false if it was
  /// in ""'s.
  bool GetIncludeFilenameSpelling(SourceLocation Loc,StringRef &Filename);

  /// \brief Given a "foo" or \<foo> reference, look up the indicated file.
  ///
  /// Returns null on failure.  \p isAngled indicates whether the file
  /// reference is for system \#include's or not (i.e. using <> instead of "").
  const FileEntry *LookupFile(SourceLocation FilenameLoc, StringRef Filename,
                              bool isAngled, const DirectoryLookup *FromDir,
                              const FileEntry *FromFile,
                              const DirectoryLookup *&CurDir,
                              SmallVectorImpl<char> *SearchPath,
                              SmallVectorImpl<char> *RelativePath,
                              ModuleMap::KnownHeader *SuggestedModule,
                              bool SkipCache = false);

  /// \brief Get the DirectoryLookup structure used to find the current
  /// FileEntry, if CurLexer is non-null and if applicable. 
  ///
  /// This allows us to implement \#include_next and find directory-specific
  /// properties.
  const DirectoryLookup *GetCurDirLookup() { return CurDirLookup; }

  /// \brief Return true if we're in the top-level file, not in a \#include.
  bool isInPrimaryFile() const;

  /// \brief Handle cases where the \#include name is expanded
  /// from a macro as multiple tokens, which need to be glued together. 
  ///
  /// This occurs for code like:
  /// \code
  ///    \#define FOO <x/y.h>
  ///    \#include FOO
  /// \endcode
  /// because in this case, "<x/y.h>" is returned as 7 tokens, not one.
  ///
  /// This code concatenates and consumes tokens up to the '>' token.  It
  /// returns false if the > was found, otherwise it returns true if it finds
  /// and consumes the EOD marker.
  bool ConcatenateIncludeName(SmallString<128> &FilenameBuffer,
                              SourceLocation &End);

  /// \brief Lex an on-off-switch (C99 6.10.6p2) and verify that it is
  /// followed by EOD.  Return true if the token is not a valid on-off-switch.
  bool LexOnOffSwitch(tok::OnOffSwitch &OOS);

  bool CheckMacroName(Token &MacroNameTok, MacroUse isDefineUndef,
                      bool *ShadowFlag = nullptr);

private:

  void PushIncludeMacroStack() {
    assert(CurLexerKind != CLK_CachingLexer && "cannot push a caching lexer");
    IncludeMacroStack.emplace_back(
        CurLexerKind, CurSubmodule, std::move(CurLexer), std::move(CurPTHLexer),
        CurPPLexer, std::move(CurTokenLexer), CurDirLookup);
    CurPPLexer = nullptr;
  }

  void PopIncludeMacroStack() {
    CurLexer = std::move(IncludeMacroStack.back().TheLexer);
    CurPTHLexer = std::move(IncludeMacroStack.back().ThePTHLexer);
    CurPPLexer = IncludeMacroStack.back().ThePPLexer;
    CurTokenLexer = std::move(IncludeMacroStack.back().TheTokenLexer);
    CurDirLookup  = IncludeMacroStack.back().TheDirLookup;
    CurSubmodule = IncludeMacroStack.back().TheSubmodule;
    CurLexerKind = IncludeMacroStack.back().CurLexerKind;
    IncludeMacroStack.pop_back();
  }

  void PropagateLineStartLeadingSpaceInfo(Token &Result);

  void EnterSubmodule(Module *M, SourceLocation ImportLoc);
  void LeaveSubmodule();

  /// Update the set of active module macros and ambiguity flag for a module
  /// macro name.
  void updateModuleMacroInfo(const IdentifierInfo *II, ModuleMacroInfo &Info);

  /// \brief Allocate a new MacroInfo object.
  MacroInfo *AllocateMacroInfo();

  DefMacroDirective *AllocateDefMacroDirective(MacroInfo *MI,
                                               SourceLocation Loc);
  UndefMacroDirective *AllocateUndefMacroDirective(SourceLocation UndefLoc);
  VisibilityMacroDirective *AllocateVisibilityMacroDirective(SourceLocation Loc,
                                                             bool isPublic);

  /// \brief Lex and validate a macro name, which occurs after a
  /// \#define or \#undef.
  ///
  /// \param MacroNameTok Token that represents the name defined or undefined.
  /// \param IsDefineUndef Kind if preprocessor directive.
  /// \param ShadowFlag Points to flag that is set if macro name shadows
  ///                   a keyword.
  ///
  /// This emits a diagnostic, sets the token kind to eod,
  /// and discards the rest of the macro line if the macro name is invalid.
  void ReadMacroName(Token &MacroNameTok, MacroUse IsDefineUndef = MU_Other,
                     bool *ShadowFlag = nullptr);

  /// The ( starting an argument list of a macro definition has just been read.
  /// Lex the rest of the arguments and the closing ), updating \p MI with
  /// what we learn and saving in \p LastTok the last token read.
  /// Return true if an error occurs parsing the arg list.
  bool ReadMacroDefinitionArgList(MacroInfo *MI, Token& LastTok);

  /// We just read a \#if or related directive and decided that the
  /// subsequent tokens are in the \#if'd out portion of the
  /// file.  Lex the rest of the file, until we see an \#endif.  If \p
  /// FoundNonSkipPortion is true, then we have already emitted code for part of
  /// this \#if directive, so \#else/\#elif blocks should never be entered. If
  /// \p FoundElse is false, then \#else directives are ok, if not, then we have
  /// already seen one so a \#else directive is a duplicate.  When this returns,
  /// the caller can lex the first valid token.
  void SkipExcludedConditionalBlock(SourceLocation IfTokenLoc,
                                    bool FoundNonSkipPortion, bool FoundElse,
                                    SourceLocation ElseLoc = SourceLocation());

  /// \brief A fast PTH version of SkipExcludedConditionalBlock.
  void PTHSkipExcludedConditionalBlock();

  /// \brief Evaluate an integer constant expression that may occur after a
  /// \#if or \#elif directive and return it as a bool.
  ///
  /// If the expression is equivalent to "!defined(X)" return X in IfNDefMacro.
  bool EvaluateDirectiveExpression(IdentifierInfo *&IfNDefMacro);

  /// \brief Install the standard preprocessor pragmas:
  /// \#pragma GCC poison/system_header/dependency and \#pragma once.
  void RegisterBuiltinPragmas();

  /// \brief Register builtin macros such as __LINE__ with the identifier table.
  void RegisterBuiltinMacros();

  /// If an identifier token is read that is to be expanded as a macro, handle
  /// it and return the next token as 'Tok'.  If we lexed a token, return true;
  /// otherwise the caller should lex again.
  bool HandleMacroExpandedIdentifier(Token &Tok, const MacroDefinition &MD);

  /// \brief Cache macro expanded tokens for TokenLexers.
  //
  /// Works like a stack; a TokenLexer adds the macro expanded tokens that is
  /// going to lex in the cache and when it finishes the tokens are removed
  /// from the end of the cache.
  Token *cacheMacroExpandedTokens(TokenLexer *tokLexer,
                                  ArrayRef<Token> tokens);
  void removeCachedMacroExpandedTokensOfLastLexer();
  friend void TokenLexer::ExpandFunctionArguments();

  /// Determine whether the next preprocessor token to be
  /// lexed is a '('.  If so, consume the token and return true, if not, this
  /// method should have no observable side-effect on the lexed tokens.
  bool isNextPPTokenLParen();

  /// After reading "MACRO(", this method is invoked to read all of the formal
  /// arguments specified for the macro invocation.  Returns null on error.
  MacroArgs *ReadFunctionLikeMacroArgs(Token &MacroName, MacroInfo *MI,
                                       SourceLocation &ExpansionEnd);

  /// \brief If an identifier token is read that is to be expanded
  /// as a builtin macro, handle it and return the next token as 'Tok'.
  void ExpandBuiltinMacro(Token &Tok);

  /// \brief Read a \c _Pragma directive, slice it up, process it, then
  /// return the first token after the directive.
  /// This assumes that the \c _Pragma token has just been read into \p Tok.
  void Handle_Pragma(Token &Tok);

  /// \brief Like Handle_Pragma except the pragma text is not enclosed within
  /// a string literal.
  void HandleMicrosoft__pragma(Token &Tok);

  /// \brief Add a lexer to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  void EnterSourceFileWithLexer(Lexer *TheLexer, const DirectoryLookup *Dir);

  /// \brief Add a lexer to the top of the include stack and
  /// start getting tokens from it using the PTH cache.
  void EnterSourceFileWithPTH(PTHLexer *PL, const DirectoryLookup *Dir);

  /// \brief Set the FileID for the preprocessor predefines.
  void setPredefinesFileID(FileID FID) {
    assert(PredefinesFileID.isInvalid() && "PredefinesFileID already set!");
    PredefinesFileID = FID;
  }

  /// \brief Returns true if we are lexing from a file and not a
  /// pragma or a macro.
  static bool IsFileLexer(const Lexer* L, const PreprocessorLexer* P) {
    return L ? !L->isPragmaLexer() : P != nullptr;
  }

  static bool IsFileLexer(const IncludeStackInfo& I) {
    return IsFileLexer(I.TheLexer.get(), I.ThePPLexer);
  }

  bool IsFileLexer() const {
    return IsFileLexer(CurLexer.get(), CurPPLexer);
  }

  //===--------------------------------------------------------------------===//
  // Caching stuff.
  void CachingLex(Token &Result);
  bool InCachingLexMode() const {
    // If the Lexer pointers are 0 and IncludeMacroStack is empty, it means
    // that we are past EOF, not that we are in CachingLex mode.
    return !CurPPLexer && !CurTokenLexer && !CurPTHLexer &&
           !IncludeMacroStack.empty();
  }
  void EnterCachingLexMode();
  void ExitCachingLexMode() {
    if (InCachingLexMode())
      RemoveTopOfLexerStack();
  }
  const Token &PeekAhead(unsigned N);
  void AnnotatePreviousCachedTokens(const Token &Tok);

  //===--------------------------------------------------------------------===//
  /// Handle*Directive - implement the various preprocessor directives.  These
  /// should side-effect the current preprocessor object so that the next call
  /// to Lex() will return the appropriate token next.
  void HandleLineDirective(Token &Tok);
  void HandleDigitDirective(Token &Tok);
  void HandleUserDiagnosticDirective(Token &Tok, bool isWarning);
  void HandleIdentSCCSDirective(Token &Tok);
  void HandleMacroPublicDirective(Token &Tok);
  void HandleMacroPrivateDirective(Token &Tok);

  // File inclusion.
  void HandleIncludeDirective(SourceLocation HashLoc,
                              Token &Tok,
                              const DirectoryLookup *LookupFrom = nullptr,
                              const FileEntry *LookupFromFile = nullptr,
                              bool isImport = false);
  void HandleIncludeNextDirective(SourceLocation HashLoc, Token &Tok);
  void HandleIncludeMacrosDirective(SourceLocation HashLoc, Token &Tok);
  void HandleImportDirective(SourceLocation HashLoc, Token &Tok);
  void HandleMicrosoftImportDirective(Token &Tok);

public:
  // Module inclusion testing.
  /// \brief Find the module that owns the source or header file that
  /// \p Loc points to. If the location is in a file that was included
  /// into a module, or is outside any module, returns nullptr.
  Module *getModuleForLocation(SourceLocation Loc);

  /// \brief Find the module that contains the specified location, either
  /// directly or indirectly.
  Module *getModuleContainingLocation(SourceLocation Loc);

private:
  // Macro handling.
  void HandleDefineDirective(Token &Tok, bool ImmediatelyAfterTopLevelIfndef);
  void HandleUndefDirective(Token &Tok);

  // Conditional Inclusion.
  void HandleIfdefDirective(Token &Tok, bool isIfndef,
                            bool ReadAnyTokensBeforeDirective);
  void HandleIfDirective(Token &Tok, bool ReadAnyTokensBeforeDirective);
  void HandleEndifDirective(Token &Tok);
  void HandleElseDirective(Token &Tok);
  void HandleElifDirective(Token &Tok);

  // Pragmas.
  void HandlePragmaDirective(SourceLocation IntroducerLoc,
                             PragmaIntroducerKind Introducer);
public:
  void HandlePragmaOnce(Token &OnceTok);
  void HandlePragmaMark();
  void HandlePragmaPoison(Token &PoisonTok);
  void HandlePragmaSystemHeader(Token &SysHeaderTok);
  void HandlePragmaDependency(Token &DependencyTok);
  void HandlePragmaPushMacro(Token &Tok);
  void HandlePragmaPopMacro(Token &Tok);
  void HandlePragmaIncludeAlias(Token &Tok);
  IdentifierInfo *ParsePragmaPushOrPopMacro(Token &Tok);

  // Return true and store the first token only if any CommentHandler
  // has inserted some tokens and getCommentRetentionState() is false.
  bool HandleComment(Token &Token, SourceRange Comment);

  /// \brief A macro is used, update information about macros that need unused
  /// warnings.
  void markMacroAsUsed(MacroInfo *MI);
};

/// \brief Abstract base class that describes a handler that will receive
/// source ranges for each of the comments encountered in the source file.
class CommentHandler {
public:
  virtual ~CommentHandler();

  // The handler shall return true if it has pushed any tokens
  // to be read using e.g. EnterToken or EnterTokenStream.
  virtual bool HandleComment(Preprocessor &PP, SourceRange Comment) = 0;
};

}  // end namespace clang

#endif
