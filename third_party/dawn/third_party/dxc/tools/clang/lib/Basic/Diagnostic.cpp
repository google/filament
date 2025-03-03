//===--- Diagnostic.cpp - C Language Family Diagnostic Handling -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Diagnostic-related interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/CharInfo.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/PartialDiagnostic.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/Locale.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

const DiagnosticBuilder &clang::operator<<(const DiagnosticBuilder &DB,
                                           DiagNullabilityKind nullability) {
  StringRef string;
  switch (nullability.first) {
  case NullabilityKind::NonNull:
    string = nullability.second ? "'nonnull'" : "'_Nonnull'";
    break;

  case NullabilityKind::Nullable:
    string = nullability.second ? "'nullable'" : "'_Nullable'";
    break;

  case NullabilityKind::Unspecified:
    string = nullability.second ? "'null_unspecified'" : "'_Null_unspecified'";
    break;
  }

  DB.AddString(string);
  return DB;
}

static void DummyArgToStringFn(DiagnosticsEngine::ArgumentKind AK, intptr_t QT,
                            StringRef Modifier, StringRef Argument,
                            ArrayRef<DiagnosticsEngine::ArgumentValue> PrevArgs,
                            SmallVectorImpl<char> &Output,
                            void *Cookie,
                            ArrayRef<intptr_t> QualTypeVals) {
  StringRef Str = "<can't format argument>";
  Output.append(Str.begin(), Str.end());
}

DiagnosticsEngine::DiagnosticsEngine(
    const IntrusiveRefCntPtr<DiagnosticIDs> &diags, DiagnosticOptions *DiagOpts,
    DiagnosticConsumer *client, bool ShouldOwnClient)
    : Diags(diags), DiagOpts(DiagOpts), Client(nullptr), SourceMgr(nullptr) {
  setClient(client, ShouldOwnClient);
  ArgToStringFn = DummyArgToStringFn;
  ArgToStringCookie = nullptr;

  AllExtensionsSilenced = 0;
  IgnoreAllWarnings = false;
  WarningsAsErrors = false;
  EnableAllWarnings = false;
  ErrorsAsFatal = false;
  SuppressSystemWarnings = false;
  SuppressAllDiagnostics = false;
  ElideType = true;
  PrintTemplateTree = false;
  ShowColors = false;
  ShowOverloads = Ovl_All;
  ExtBehavior = diag::Severity::Ignored;

  ErrorLimit = 0;
  TemplateBacktraceLimit = 0;
  ConstexprBacktraceLimit = 0;

  Reset();
}

DiagnosticsEngine::~DiagnosticsEngine() {
  // If we own the diagnostic client, destroy it first so that it can access the
  // engine from its destructor.
  setClient(nullptr);
}

void DiagnosticsEngine::setClient(DiagnosticConsumer *client,
                                  bool ShouldOwnClient) {
  Owner.reset(ShouldOwnClient ? client : nullptr);
  Client = client;
}

void DiagnosticsEngine::pushMappings(SourceLocation Loc) {
  DiagStateOnPushStack.push_back(GetCurDiagState());
}

bool DiagnosticsEngine::popMappings(SourceLocation Loc) {
  if (DiagStateOnPushStack.empty())
    return false;

  if (DiagStateOnPushStack.back() != GetCurDiagState()) {
    // State changed at some point between push/pop.
    PushDiagStatePoint(DiagStateOnPushStack.back(), Loc);
  }
  DiagStateOnPushStack.pop_back();
  return true;
}

void DiagnosticsEngine::Reset() {
  ErrorOccurred = false;
  UncompilableErrorOccurred = false;
  FatalErrorOccurred = false;
  UnrecoverableErrorOccurred = false;
  
  NumWarnings = 0;
  NumErrors = 0;
  TrapNumErrorsOccurred = 0;
  TrapNumUnrecoverableErrorsOccurred = 0;
  
  CurDiagID = ~0U;
  LastDiagLevel = DiagnosticIDs::Ignored;
  DelayedDiagID = 0;

  // Clear state related to #pragma diagnostic.
  DiagStates.clear();
  DiagStatePoints.clear();
  DiagStateOnPushStack.clear();

  // Create a DiagState and DiagStatePoint representing diagnostic changes
  // through command-line.
  DiagStates.emplace_back();
  DiagStatePoints.push_back(DiagStatePoint(&DiagStates.back(), FullSourceLoc()));
}

void DiagnosticsEngine::SetDelayedDiagnostic(unsigned DiagID, StringRef Arg1,
                                             StringRef Arg2) {
  if (DelayedDiagID)
    return;

  DelayedDiagID = DiagID;
  DelayedDiagArg1 = Arg1.str();
  DelayedDiagArg2 = Arg2.str();
}

void DiagnosticsEngine::ReportDelayed() {
  Report(DelayedDiagID) << DelayedDiagArg1 << DelayedDiagArg2;
  DelayedDiagID = 0;
  DelayedDiagArg1.clear();
  DelayedDiagArg2.clear();
}

DiagnosticsEngine::DiagStatePointsTy::iterator
DiagnosticsEngine::GetDiagStatePointForLoc(SourceLocation L) const {
  assert(!DiagStatePoints.empty());
  assert(DiagStatePoints.front().Loc.isInvalid() &&
         "Should have created a DiagStatePoint for command-line");

  if (!SourceMgr)
    return DiagStatePoints.end() - 1;

  FullSourceLoc Loc(L, *SourceMgr);
  if (Loc.isInvalid())
    return DiagStatePoints.end() - 1;

  DiagStatePointsTy::iterator Pos = DiagStatePoints.end();
  FullSourceLoc LastStateChangePos = DiagStatePoints.back().Loc;
  if (LastStateChangePos.isValid() &&
      Loc.isBeforeInTranslationUnitThan(LastStateChangePos))
    Pos = std::upper_bound(DiagStatePoints.begin(), DiagStatePoints.end(),
                           DiagStatePoint(nullptr, Loc));
  --Pos;
  return Pos;
}

void DiagnosticsEngine::setSeverity(diag::kind Diag, diag::Severity Map,
                                    SourceLocation L) {
  assert(Diag < diag::DIAG_UPPER_LIMIT &&
         "Can only map builtin diagnostics");
  assert((Diags->isBuiltinWarningOrExtension(Diag) ||
          (Map == diag::Severity::Fatal || Map == diag::Severity::Error)) &&
         "Cannot map errors into warnings!");
  assert(!DiagStatePoints.empty());
  assert((L.isInvalid() || SourceMgr) && "No SourceMgr for valid location");

  FullSourceLoc Loc = SourceMgr? FullSourceLoc(L, *SourceMgr) : FullSourceLoc();
  FullSourceLoc LastStateChangePos = DiagStatePoints.back().Loc;
  // Don't allow a mapping to a warning override an error/fatal mapping.
  if (Map == diag::Severity::Warning) {
    DiagnosticMapping &Info = GetCurDiagState()->getOrAddMapping(Diag);
    if (Info.getSeverity() == diag::Severity::Error ||
        Info.getSeverity() == diag::Severity::Fatal)
      Map = Info.getSeverity();
  }
  DiagnosticMapping Mapping = makeUserMapping(Map, L);

  // Common case; setting all the diagnostics of a group in one place.
  if (Loc.isInvalid() || Loc == LastStateChangePos) {
    GetCurDiagState()->setMapping(Diag, Mapping);
    return;
  }

  assert(SourceMgr != nullptr); // SourceMgr==nullptr is only valid for invalid locations, which cause an earlier exit
  // Another common case; modifying diagnostic state in a source location
  // after the previous one.
  if ((Loc.isValid() && LastStateChangePos.isInvalid()) ||
      LastStateChangePos.isBeforeInTranslationUnitThan(Loc)) {
    // A diagnostic pragma occurred, create a new DiagState initialized with
    // the current one and a new DiagStatePoint to record at which location
    // the new state became active.
    DiagStates.push_back(*GetCurDiagState());
    PushDiagStatePoint(&DiagStates.back(), Loc);
    GetCurDiagState()->setMapping(Diag, Mapping);
    return;
  }

  // We allow setting the diagnostic state in random source order for
  // completeness but it should not be actually happening in normal practice.

  DiagStatePointsTy::iterator Pos = GetDiagStatePointForLoc(Loc);
  assert(Pos != DiagStatePoints.end());

  // Update all diagnostic states that are active after the given location.
  for (DiagStatePointsTy::iterator
         I = Pos+1, E = DiagStatePoints.end(); I != E; ++I) {
    GetCurDiagState()->setMapping(Diag, Mapping);
  }

  // If the location corresponds to an existing point, just update its state.
  if (Pos->Loc == Loc) {
    GetCurDiagState()->setMapping(Diag, Mapping);
    return;
  }

  // Create a new state/point and fit it into the vector of DiagStatePoints
  // so that the vector is always ordered according to location.
  assert(Pos->Loc.isBeforeInTranslationUnitThan(Loc));
  DiagStates.push_back(*Pos->State);
  DiagState *NewState = &DiagStates.back();
  GetCurDiagState()->setMapping(Diag, Mapping);
  DiagStatePoints.insert(Pos+1, DiagStatePoint(NewState,
                                               FullSourceLoc(Loc, *SourceMgr)));
}

bool DiagnosticsEngine::setSeverityForGroup(diag::Flavor Flavor,
                                            StringRef Group, diag::Severity Map,
                                            SourceLocation Loc) {
  // Get the diagnostics in this group.
  SmallVector<diag::kind, 256> GroupDiags;
  if (Diags->getDiagnosticsInGroup(Flavor, Group, GroupDiags))
    return true;

  // Set the mapping.
  for (diag::kind Diag : GroupDiags)
    setSeverity(Diag, Map, Loc);

  return false;
}

bool DiagnosticsEngine::setDiagnosticGroupWarningAsError(StringRef Group,
                                                         bool Enabled) {
  // If we are enabling this feature, just set the diagnostic mappings to map to
  // errors.
  if (Enabled)
    return setSeverityForGroup(diag::Flavor::WarningOrError, Group,
                               diag::Severity::Error);

  // Otherwise, we want to set the diagnostic mapping's "no Werror" bit, and
  // potentially downgrade anything already mapped to be a warning.

  // Get the diagnostics in this group.
  SmallVector<diag::kind, 8> GroupDiags;
  if (Diags->getDiagnosticsInGroup(diag::Flavor::WarningOrError, Group,
                                   GroupDiags))
    return true;

  // Perform the mapping change.
  for (unsigned i = 0, e = GroupDiags.size(); i != e; ++i) {
    DiagnosticMapping &Info = GetCurDiagState()->getOrAddMapping(GroupDiags[i]);

    if (Info.getSeverity() == diag::Severity::Error ||
        Info.getSeverity() == diag::Severity::Fatal)
      Info.setSeverity(diag::Severity::Warning);

    Info.setNoWarningAsError(true);
  }

  return false;
}

bool DiagnosticsEngine::setDiagnosticGroupErrorAsFatal(StringRef Group,
                                                       bool Enabled) {
  // If we are enabling this feature, just set the diagnostic mappings to map to
  // fatal errors.
  if (Enabled)
    return setSeverityForGroup(diag::Flavor::WarningOrError, Group,
                               diag::Severity::Fatal);

  // Otherwise, we want to set the diagnostic mapping's "no Werror" bit, and
  // potentially downgrade anything already mapped to be an error.

  // Get the diagnostics in this group.
  SmallVector<diag::kind, 8> GroupDiags;
  if (Diags->getDiagnosticsInGroup(diag::Flavor::WarningOrError, Group,
                                   GroupDiags))
    return true;

  // Perform the mapping change.
  for (unsigned i = 0, e = GroupDiags.size(); i != e; ++i) {
    DiagnosticMapping &Info = GetCurDiagState()->getOrAddMapping(GroupDiags[i]);

    if (Info.getSeverity() == diag::Severity::Fatal)
      Info.setSeverity(diag::Severity::Error);

    Info.setNoErrorAsFatal(true);
  }

  return false;
}

void DiagnosticsEngine::setSeverityForAll(diag::Flavor Flavor,
                                          diag::Severity Map,
                                          SourceLocation Loc) {
  // Get all the diagnostics.
  SmallVector<diag::kind, 64> AllDiags;
  Diags->getAllDiagnostics(Flavor, AllDiags);

  // Set the mapping.
  for (unsigned i = 0, e = AllDiags.size(); i != e; ++i)
    if (Diags->isBuiltinWarningOrExtension(AllDiags[i]))
      setSeverity(AllDiags[i], Map, Loc);
}

void DiagnosticsEngine::Report(const StoredDiagnostic &storedDiag) {
  assert(CurDiagID == ~0U && "Multiple diagnostics in flight at once!");

  CurDiagLoc = storedDiag.getLocation();
  CurDiagID = storedDiag.getID();
  NumDiagArgs = 0;

  DiagRanges.clear();
  DiagRanges.append(storedDiag.range_begin(), storedDiag.range_end());

  DiagFixItHints.clear();
  DiagFixItHints.append(storedDiag.fixit_begin(), storedDiag.fixit_end());

  assert(Client && "DiagnosticConsumer not set!");
  Level DiagLevel = storedDiag.getLevel();
  Diagnostic Info(this, storedDiag.getMessage());
  Client->HandleDiagnostic(DiagLevel, Info);
  if (Client->IncludeInDiagnosticCounts()) {
    if (DiagLevel == DiagnosticsEngine::Warning)
      ++NumWarnings;
  }

  CurDiagID = ~0U;
}

bool DiagnosticsEngine::EmitCurrentDiagnostic(bool Force) {
  assert(getClient() && "DiagnosticClient not set!");

  bool Emitted;
  if (Force) {
    Diagnostic Info(this);

    // Figure out the diagnostic level of this message.
    DiagnosticIDs::Level DiagLevel
      = Diags->getDiagnosticLevel(Info.getID(), Info.getLocation(), *this);

    Emitted = (DiagLevel != DiagnosticIDs::Ignored);
    if (Emitted) {
      // Emit the diagnostic regardless of suppression level.
      Diags->EmitDiag(*this, DiagLevel);
    }
  } else {
    // Process the diagnostic, sending the accumulated information to the
    // DiagnosticConsumer.
    Emitted = ProcessDiag();
  }

  // Clear out the current diagnostic object.
  unsigned DiagID = CurDiagID;
  Clear();

  // If there was a delayed diagnostic, emit it now.
  if (!Force && DelayedDiagID && DelayedDiagID != DiagID)
    ReportDelayed();

  return Emitted;
}


DiagnosticConsumer::~DiagnosticConsumer() {}

void DiagnosticConsumer::HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                                        const Diagnostic &Info) {
  if (!IncludeInDiagnosticCounts())
    return;

  if (DiagLevel == DiagnosticsEngine::Warning)
    ++NumWarnings;
  else if (DiagLevel >= DiagnosticsEngine::Error)
    ++NumErrors;
}

/// ModifierIs - Return true if the specified modifier matches specified string.
template <std::size_t StrLen>
static bool ModifierIs(const char *Modifier, unsigned ModifierLen,
                       const char (&Str)[StrLen]) {
  return StrLen-1 == ModifierLen && !memcmp(Modifier, Str, StrLen-1);
}

/// ScanForward - Scans forward, looking for the given character, skipping
/// nested clauses and escaped characters.
static const char *ScanFormat(const char *I, const char *E, char Target) {
  unsigned Depth = 0;

  for ( ; I != E; ++I) {
    if (Depth == 0 && *I == Target) return I;
    if (Depth != 0 && *I == '}') Depth--;

    if (*I == '%') {
      I++;
      if (I == E) break;

      // Escaped characters get implicitly skipped here.

      // Format specifier.
      if (!isDigit(*I) && !isPunctuation(*I)) {
        for (I++; I != E && !isDigit(*I) && *I != '{'; I++) ;
        if (I == E) break;
        if (*I == '{')
          Depth++;
      }
    }
  }
  return E;
}

/// HandleSelectModifier - Handle the integer 'select' modifier.  This is used
/// like this:  %select{foo|bar|baz}2.  This means that the integer argument
/// "%2" has a value from 0-2.  If the value is 0, the diagnostic prints 'foo'.
/// If the value is 1, it prints 'bar'.  If it has the value 2, it prints 'baz'.
/// This is very useful for certain classes of variant diagnostics.
static void HandleSelectModifier(const Diagnostic &DInfo, unsigned ValNo,
                                 const char *Argument, unsigned ArgumentLen,
                                 SmallVectorImpl<char> &OutStr) {
  const char *ArgumentEnd = Argument+ArgumentLen;

  // Skip over 'ValNo' |'s.
  while (ValNo) {
    const char *NextVal = ScanFormat(Argument, ArgumentEnd, '|');
    assert(NextVal != ArgumentEnd && "Value for integer select modifier was"
           " larger than the number of options in the diagnostic string!");
    Argument = NextVal+1;  // Skip this string.
    --ValNo;
  }

  // Get the end of the value.  This is either the } or the |.
  const char *EndPtr = ScanFormat(Argument, ArgumentEnd, '|');

  // Recursively format the result of the select clause into the output string.
  DInfo.FormatDiagnostic(Argument, EndPtr, OutStr);
}

/// HandleIntegerSModifier - Handle the integer 's' modifier.  This adds the
/// letter 's' to the string if the value is not 1.  This is used in cases like
/// this:  "you have %4 parameter%s4!". // HLSL Change - remove minor profanity
static void HandleIntegerSModifier(unsigned ValNo,
                                   SmallVectorImpl<char> &OutStr) {
  if (ValNo != 1)
    OutStr.push_back('s');
}

/// HandleOrdinalModifier - Handle the integer 'ord' modifier.  This
/// prints the ordinal form of the given integer, with 1 corresponding
/// to the first ordinal.  Currently this is hard-coded to use the
/// English form.
static void HandleOrdinalModifier(unsigned ValNo,
                                  SmallVectorImpl<char> &OutStr) {
  assert(ValNo != 0 && "ValNo must be strictly positive!");

  llvm::raw_svector_ostream Out(OutStr);

  // We could use text forms for the first N ordinals, but the numeric
  // forms are actually nicer in diagnostics because they stand out.
  Out << ValNo << llvm::getOrdinalSuffix(ValNo);
}


/// PluralNumber - Parse an unsigned integer and advance Start.
static unsigned PluralNumber(const char *&Start, const char *End) {
  // Programming 101: Parse a decimal number :-)
  unsigned Val = 0;
  while (Start != End && *Start >= '0' && *Start <= '9') {
    Val *= 10;
    Val += *Start - '0';
    ++Start;
  }
  return Val;
}

/// TestPluralRange - Test if Val is in the parsed range. Modifies Start.
static bool TestPluralRange(unsigned Val, const char *&Start, const char *End) {
  if (*Start != '[') {
    unsigned Ref = PluralNumber(Start, End);
    return Ref == Val;
  }

  ++Start;
  unsigned Low = PluralNumber(Start, End);
  assert(*Start == ',' && "Bad plural expression syntax: expected ,");
  ++Start;
  unsigned High = PluralNumber(Start, End);
  assert(*Start == ']' && "Bad plural expression syntax: expected )");
  ++Start;
  return Low <= Val && Val <= High;
}

/// EvalPluralExpr - Actual expression evaluator for HandlePluralModifier.
static bool EvalPluralExpr(unsigned ValNo, const char *Start, const char *End) {
  // Empty condition?
  if (*Start == ':')
    return true;

  while (1) {
    char C = *Start;
    if (C == '%') {
      // Modulo expression
      ++Start;
      unsigned Arg = PluralNumber(Start, End);
      assert(*Start == '=' && "Bad plural expression syntax: expected =");
      ++Start;
      unsigned ValMod = ValNo % Arg;
      if (TestPluralRange(ValMod, Start, End))
        return true;
    } else {
      assert((C == '[' || (C >= '0' && C <= '9')) &&
             "Bad plural expression syntax: unexpected character");
      // Range expression
      if (TestPluralRange(ValNo, Start, End))
        return true;
    }

    // Scan for next or-expr part.
    Start = std::find(Start, End, ',');
    if (Start == End)
      break;
    ++Start;
  }
  return false;
}

/// HandlePluralModifier - Handle the integer 'plural' modifier. This is used
/// for complex plural forms, or in languages where all plurals are complex.
/// The syntax is: %plural{cond1:form1|cond2:form2|:form3}, where condn are
/// conditions that are tested in order, the form corresponding to the first
/// that applies being emitted. The empty condition is always true, making the
/// last form a default case.
/// Conditions are simple boolean expressions, where n is the number argument.
/// Here are the rules.
/// condition  := expression | empty
/// empty      :=                             -> always true
/// expression := numeric [',' expression]    -> logical or
/// numeric    := range                       -> true if n in range
///             | '%' number '=' range        -> true if n % number in range
/// range      := number
///             | '[' number ',' number ']'   -> ranges are inclusive both ends
///
/// Here are some examples from the GNU gettext manual written in this form:
/// English:
/// {1:form0|:form1}
/// Latvian:
/// {0:form2|%100=11,%10=0,%10=[2,9]:form1|:form0}
/// Gaeilge:
/// {1:form0|2:form1|:form2}
/// Romanian:
/// {1:form0|0,%100=[1,19]:form1|:form2}
/// Lithuanian:
/// {%10=0,%100=[10,19]:form2|%10=1:form0|:form1}
/// Russian (requires repeated form):
/// {%100=[11,14]:form2|%10=1:form0|%10=[2,4]:form1|:form2}
/// Slovak
/// {1:form0|[2,4]:form1|:form2}
/// Polish (requires repeated form):
/// {1:form0|%100=[10,20]:form2|%10=[2,4]:form1|:form2}
static void HandlePluralModifier(const Diagnostic &DInfo, unsigned ValNo,
                                 const char *Argument, unsigned ArgumentLen,
                                 SmallVectorImpl<char> &OutStr) {
  const char *ArgumentEnd = Argument + ArgumentLen;
  while (1) {
    assert(Argument < ArgumentEnd && "Plural expression didn't match.");
    const char *ExprEnd = Argument;
    while (*ExprEnd != ':') {
      assert(ExprEnd != ArgumentEnd && "Plural missing expression end");
      ++ExprEnd;
    }
    if (EvalPluralExpr(ValNo, Argument, ExprEnd)) {
      Argument = ExprEnd + 1;
      ExprEnd = ScanFormat(Argument, ArgumentEnd, '|');

      // Recursively format the result of the plural clause into the
      // output string.
      DInfo.FormatDiagnostic(Argument, ExprEnd, OutStr);
      return;
    }
    Argument = ScanFormat(Argument, ArgumentEnd - 1, '|') + 1;
  }
}

/// \brief Returns the friendly description for a token kind that will appear
/// without quotes in diagnostic messages. These strings may be translatable in
/// future.
static const char *getTokenDescForDiagnostic(tok::TokenKind Kind) {
  switch (Kind) {
  case tok::identifier:
    return "identifier";
  default:
    return nullptr;
  }
}

/// FormatDiagnostic - Format this diagnostic into a string, substituting the
/// formal arguments into the %0 slots.  The result is appended onto the Str
/// array.
void Diagnostic::
FormatDiagnostic(SmallVectorImpl<char> &OutStr) const {
  if (!StoredDiagMessage.empty()) {
    OutStr.append(StoredDiagMessage.begin(), StoredDiagMessage.end());
    return;
  }

  StringRef Diag = 
    getDiags()->getDiagnosticIDs()->getDescription(getID());

  FormatDiagnostic(Diag.begin(), Diag.end(), OutStr);
}

void Diagnostic::
FormatDiagnostic(const char *DiagStr, const char *DiagEnd,
                 SmallVectorImpl<char> &OutStr) const {

  // When the diagnostic string is only "%0", the entire string is being given
  // by an outside source.  Remove unprintable characters from this string
  // and skip all the other string processing.
  if (DiagEnd - DiagStr == 2 &&
      StringRef(DiagStr, DiagEnd - DiagStr).equals("%0") &&
      getArgKind(0) == DiagnosticsEngine::ak_std_string) {
    const std::string &S = getArgStdStr(0);
    for (char c : S) {
      if (llvm::sys::locale::isPrint(c) || c == '\t') {
        OutStr.push_back(c);
      }
    }
    return;
  }

  /// FormattedArgs - Keep track of all of the arguments formatted by
  /// ConvertArgToString and pass them into subsequent calls to
  /// ConvertArgToString, allowing the implementation to avoid redundancies in
  /// obvious cases.
  SmallVector<DiagnosticsEngine::ArgumentValue, 8> FormattedArgs;

  /// QualTypeVals - Pass a vector of arrays so that QualType names can be
  /// compared to see if more information is needed to be printed.
  SmallVector<intptr_t, 2> QualTypeVals;
  SmallVector<char, 64> Tree;

  for (unsigned i = 0, e = getNumArgs(); i < e; ++i)
    if (getArgKind(i) == DiagnosticsEngine::ak_qualtype)
      QualTypeVals.push_back(getRawArg(i));

  while (DiagStr != DiagEnd) {
    if (DiagStr[0] != '%') {
      // Append non-%0 substrings to Str if we have one.
      const char *StrEnd = std::find(DiagStr, DiagEnd, '%');
      OutStr.append(DiagStr, StrEnd);
      DiagStr = StrEnd;
      continue;
    } else if (isPunctuation(DiagStr[1])) {
      OutStr.push_back(DiagStr[1]);  // %% -> %.
      DiagStr += 2;
      continue;
    }

    // Skip the %.
    ++DiagStr;

    // This must be a placeholder for a diagnostic argument.  The format for a
    // placeholder is one of "%0", "%modifier0", or "%modifier{arguments}0".
    // The digit is a number from 0-9 indicating which argument this comes from.
    // The modifier is a string of digits from the set [-a-z]+, arguments is a
    // brace enclosed string.
    const char *Modifier = nullptr, *Argument = nullptr;
    unsigned ModifierLen = 0, ArgumentLen = 0;

    // Check to see if we have a modifier.  If so eat it.
    if (!isDigit(DiagStr[0])) {
      Modifier = DiagStr;
      while (DiagStr[0] == '-' ||
             (DiagStr[0] >= 'a' && DiagStr[0] <= 'z'))
        ++DiagStr;
      ModifierLen = DiagStr-Modifier;

      // If we have an argument, get it next.
      if (DiagStr[0] == '{') {
        ++DiagStr; // Skip {.
        Argument = DiagStr;

        DiagStr = ScanFormat(DiagStr, DiagEnd, '}');
        assert(DiagStr != DiagEnd && "Mismatched {}'s in diagnostic string!");
        ArgumentLen = DiagStr-Argument;
        ++DiagStr;  // Skip }.
      }
    }

    assert(isDigit(*DiagStr) && "Invalid format for argument in diagnostic");
    unsigned ArgNo = *DiagStr++ - '0';

    // Only used for type diffing.
    unsigned ArgNo2 = ArgNo;

    DiagnosticsEngine::ArgumentKind Kind = getArgKind(ArgNo);
    if (ModifierIs(Modifier, ModifierLen, "diff")) {
      assert(*DiagStr == ',' && isDigit(*(DiagStr + 1)) &&
             "Invalid format for diff modifier");
      ++DiagStr;  // Comma.
      ArgNo2 = *DiagStr++ - '0';
      DiagnosticsEngine::ArgumentKind Kind2 = getArgKind(ArgNo2);
      if (Kind == DiagnosticsEngine::ak_qualtype &&
          Kind2 == DiagnosticsEngine::ak_qualtype)
        Kind = DiagnosticsEngine::ak_qualtype_pair;
      else {
        // %diff only supports QualTypes.  For other kinds of arguments,
        // use the default printing.  For example, if the modifier is:
        //   "%diff{compare $ to $|other text}1,2"
        // treat it as:
        //   "compare %1 to %2"
        const char *Pipe = ScanFormat(Argument, Argument + ArgumentLen, '|');
        const char *FirstDollar = ScanFormat(Argument, Pipe, '$');
        const char *SecondDollar = ScanFormat(FirstDollar + 1, Pipe, '$');
        const char ArgStr1[] = { '%', static_cast<char>('0' + ArgNo) };
        const char ArgStr2[] = { '%', static_cast<char>('0' + ArgNo2) };
        FormatDiagnostic(Argument, FirstDollar, OutStr);
        FormatDiagnostic(ArgStr1, ArgStr1 + 2, OutStr);
        FormatDiagnostic(FirstDollar + 1, SecondDollar, OutStr);
        FormatDiagnostic(ArgStr2, ArgStr2 + 2, OutStr);
        FormatDiagnostic(SecondDollar + 1, Pipe, OutStr);
        continue;
      }
    }
    
    switch (Kind) {
    // ---- STRINGS ----
    case DiagnosticsEngine::ak_std_string: {
      const std::string &S = getArgStdStr(ArgNo);
      assert(ModifierLen == 0 && "No modifiers for strings yet");
      OutStr.append(S.begin(), S.end());
      break;
    }
    case DiagnosticsEngine::ak_c_string: {
      const char *S = getArgCStr(ArgNo);
      assert(ModifierLen == 0 && "No modifiers for strings yet");

      // Don't crash if get passed a null pointer by accident.
      if (!S)
        S = "(null)";

      OutStr.append(S, S + strlen(S));
      break;
    }
    // ---- INTEGERS ----
    case DiagnosticsEngine::ak_sint: {
      int Val = getArgSInt(ArgNo);

      if (ModifierIs(Modifier, ModifierLen, "select")) {
        HandleSelectModifier(*this, (unsigned)Val, Argument, ArgumentLen,
                             OutStr);
      } else if (ModifierIs(Modifier, ModifierLen, "s")) {
        HandleIntegerSModifier(Val, OutStr);
      } else if (ModifierIs(Modifier, ModifierLen, "plural")) {
        HandlePluralModifier(*this, (unsigned)Val, Argument, ArgumentLen,
                             OutStr);
      } else if (ModifierIs(Modifier, ModifierLen, "ordinal")) {
        HandleOrdinalModifier((unsigned)Val, OutStr);
      } else {
        assert(ModifierLen == 0 && "Unknown integer modifier");
        llvm::raw_svector_ostream(OutStr) << Val;
      }
      break;
    }
    case DiagnosticsEngine::ak_uint: {
      unsigned Val = getArgUInt(ArgNo);

      if (ModifierIs(Modifier, ModifierLen, "select")) {
        HandleSelectModifier(*this, Val, Argument, ArgumentLen, OutStr);
      } else if (ModifierIs(Modifier, ModifierLen, "s")) {
        HandleIntegerSModifier(Val, OutStr);
      } else if (ModifierIs(Modifier, ModifierLen, "plural")) {
        HandlePluralModifier(*this, (unsigned)Val, Argument, ArgumentLen,
                             OutStr);
      } else if (ModifierIs(Modifier, ModifierLen, "ordinal")) {
        HandleOrdinalModifier(Val, OutStr);
      } else {
        assert(ModifierLen == 0 && "Unknown integer modifier");
        llvm::raw_svector_ostream(OutStr) << Val;
      }
      break;
    }
    // ---- TOKEN SPELLINGS ----
    case DiagnosticsEngine::ak_tokenkind: {
      tok::TokenKind Kind = static_cast<tok::TokenKind>(getRawArg(ArgNo));
      assert(ModifierLen == 0 && "No modifiers for token kinds yet");

      llvm::raw_svector_ostream Out(OutStr);
      if (const char *S = tok::getPunctuatorSpelling(Kind))
        // Quoted token spelling for punctuators.
        Out << '\'' << S << '\'';
      else if (const char *S = tok::getKeywordSpelling(Kind))
        // Unquoted token spelling for keywords.
        Out << S;
      else if (const char *S = getTokenDescForDiagnostic(Kind))
        // Unquoted translatable token name.
        Out << S;
      else if (const char *S = tok::getTokenName(Kind))
        // Debug name, shouldn't appear in user-facing diagnostics.
        Out << '<' << S << '>';
      else
        Out << "(null)";
      break;
    }
    // ---- NAMES and TYPES ----
    case DiagnosticsEngine::ak_identifierinfo: {
      const IdentifierInfo *II = getArgIdentifier(ArgNo);
      assert(ModifierLen == 0 && "No modifiers for strings yet");

      // Don't crash if get passed a null pointer by accident.
      if (!II) {
        const char *S = "(null)";
        OutStr.append(S, S + strlen(S));
        continue;
      }

      llvm::raw_svector_ostream(OutStr) << '\'' << II->getName() << '\'';
      break;
    }
    case DiagnosticsEngine::ak_qualtype:
    case DiagnosticsEngine::ak_declarationname:
    case DiagnosticsEngine::ak_nameddecl:
    case DiagnosticsEngine::ak_nestednamespec:
    case DiagnosticsEngine::ak_declcontext:
    case DiagnosticsEngine::ak_attr:
      getDiags()->ConvertArgToString(Kind, getRawArg(ArgNo),
                                     StringRef(Modifier, ModifierLen),
                                     StringRef(Argument, ArgumentLen),
                                     FormattedArgs,
                                     OutStr, QualTypeVals);
      break;
    case DiagnosticsEngine::ak_qualtype_pair:
      // Create a struct with all the info needed for printing.
      TemplateDiffTypes TDT;
      TDT.FromType = getRawArg(ArgNo);
      TDT.ToType = getRawArg(ArgNo2);
      TDT.ElideType = getDiags()->ElideType;
      TDT.ShowColors = getDiags()->ShowColors;
      TDT.TemplateDiffUsed = false;
      intptr_t val = reinterpret_cast<intptr_t>(&TDT);

      const char *ArgumentEnd = Argument + ArgumentLen;
      const char *Pipe = ScanFormat(Argument, ArgumentEnd, '|');

      // Print the tree.  If this diagnostic already has a tree, skip the
      // second tree.
      if (getDiags()->PrintTemplateTree && Tree.empty()) {
        TDT.PrintFromType = true;
        TDT.PrintTree = true;
        getDiags()->ConvertArgToString(Kind, val,
                                       StringRef(Modifier, ModifierLen),
                                       StringRef(Argument, ArgumentLen),
                                       FormattedArgs,
                                       Tree, QualTypeVals);
        // If there is no tree information, fall back to regular printing.
        if (!Tree.empty()) {
          FormatDiagnostic(Pipe + 1, ArgumentEnd, OutStr);
          break;
        }
      }

      // Non-tree printing, also the fall-back when tree printing fails.
      // The fall-back is triggered when the types compared are not templates.
      const char *FirstDollar = ScanFormat(Argument, ArgumentEnd, '$');
      const char *SecondDollar = ScanFormat(FirstDollar + 1, ArgumentEnd, '$');

      // Append before text
      FormatDiagnostic(Argument, FirstDollar, OutStr);

      // Append first type
      TDT.PrintTree = false;
      TDT.PrintFromType = true;
      getDiags()->ConvertArgToString(Kind, val,
                                     StringRef(Modifier, ModifierLen),
                                     StringRef(Argument, ArgumentLen),
                                     FormattedArgs,
                                     OutStr, QualTypeVals);
      if (!TDT.TemplateDiffUsed)
        FormattedArgs.push_back(std::make_pair(DiagnosticsEngine::ak_qualtype,
                                               TDT.FromType));

      // Append middle text
      FormatDiagnostic(FirstDollar + 1, SecondDollar, OutStr);

      // Append second type
      TDT.PrintFromType = false;
      getDiags()->ConvertArgToString(Kind, val,
                                     StringRef(Modifier, ModifierLen),
                                     StringRef(Argument, ArgumentLen),
                                     FormattedArgs,
                                     OutStr, QualTypeVals);
      if (!TDT.TemplateDiffUsed)
        FormattedArgs.push_back(std::make_pair(DiagnosticsEngine::ak_qualtype,
                                               TDT.ToType));

      // Append end text
      FormatDiagnostic(SecondDollar + 1, Pipe, OutStr);
      break;
    }
    
    // Remember this argument info for subsequent formatting operations.  Turn
    // std::strings into a null terminated string to make it be the same case as
    // all the other ones.
    if (Kind == DiagnosticsEngine::ak_qualtype_pair)
      continue;
    else if (Kind != DiagnosticsEngine::ak_std_string)
      FormattedArgs.push_back(std::make_pair(Kind, getRawArg(ArgNo)));
    else
      FormattedArgs.push_back(std::make_pair(DiagnosticsEngine::ak_c_string,
                                        (intptr_t)getArgStdStr(ArgNo).c_str()));
    
  }

  // Append the type tree to the end of the diagnostics.
  OutStr.append(Tree.begin(), Tree.end());
}

StoredDiagnostic::StoredDiagnostic() { }

StoredDiagnostic::StoredDiagnostic(DiagnosticsEngine::Level Level, unsigned ID,
                                   StringRef Message)
  : ID(ID), Level(Level), Loc(), Message(Message) { }

StoredDiagnostic::StoredDiagnostic(DiagnosticsEngine::Level Level, 
                                   const Diagnostic &Info)
  : ID(Info.getID()), Level(Level) 
{
  assert((Info.getLocation().isInvalid() || Info.hasSourceManager()) &&
       "Valid source location without setting a source manager for diagnostic");
  if (Info.getLocation().isValid())
    Loc = FullSourceLoc(Info.getLocation(), Info.getSourceManager());
  SmallString<64> Message;
  Info.FormatDiagnostic(Message);
  this->Message.assign(Message.begin(), Message.end());
  this->Ranges.assign(Info.getRanges().begin(), Info.getRanges().end());
  this->FixIts.assign(Info.getFixItHints().begin(), Info.getFixItHints().end());
}

StoredDiagnostic::StoredDiagnostic(DiagnosticsEngine::Level Level, unsigned ID,
                                   StringRef Message, FullSourceLoc Loc,
                                   ArrayRef<CharSourceRange> Ranges,
                                   ArrayRef<FixItHint> FixIts)
  : ID(ID), Level(Level), Loc(Loc), Message(Message), 
    Ranges(Ranges.begin(), Ranges.end()), FixIts(FixIts.begin(), FixIts.end())
{
}

StoredDiagnostic::~StoredDiagnostic() { }

/// IncludeInDiagnosticCounts - This method (whose default implementation
///  returns true) indicates whether the diagnostics handled by this
///  DiagnosticConsumer should be included in the number of diagnostics
///  reported by DiagnosticsEngine.
bool DiagnosticConsumer::IncludeInDiagnosticCounts() const { return true; }

void IgnoringDiagConsumer::anchor() { }

ForwardingDiagnosticConsumer::~ForwardingDiagnosticConsumer() {}

void ForwardingDiagnosticConsumer::HandleDiagnostic(
       DiagnosticsEngine::Level DiagLevel,
       const Diagnostic &Info) {
  Target.HandleDiagnostic(DiagLevel, Info);
}

void ForwardingDiagnosticConsumer::clear() {
  DiagnosticConsumer::clear();
  Target.clear();
}

bool ForwardingDiagnosticConsumer::IncludeInDiagnosticCounts() const {
  return Target.IncludeInDiagnosticCounts();
}

PartialDiagnostic::StorageAllocator::StorageAllocator() {
  for (unsigned I = 0; I != NumCached; ++I)
    FreeList[I] = Cached + I;
  NumFreeListEntries = NumCached;
}

PartialDiagnostic::StorageAllocator::~StorageAllocator() {
  // Don't assert if we are in a CrashRecovery context, as this invariant may
  // be invalidated during a crash.
  assert((NumFreeListEntries == NumCached || 
          llvm::CrashRecoveryContext::isRecoveringFromCrash()) && 
         "A partial is on the lamb");
}
