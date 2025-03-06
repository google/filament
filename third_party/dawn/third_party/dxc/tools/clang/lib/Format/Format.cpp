//===--- Format.cpp - Format C++ code -------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements functions declared in Format.h. This will be
/// split into separate files as we go.
///
//===----------------------------------------------------------------------===//

#include "ContinuationIndenter.h"
#include "TokenAnnotator.h"
#include "UnwrappedLineFormatter.h"
#include "UnwrappedLineParser.h"
#include "WhitespaceManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Format/Format.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/YAMLTraits.h"
#include <queue>
#include <string>

#define DEBUG_TYPE "format-formatter"

using clang::format::FormatStyle;

LLVM_YAML_IS_FLOW_SEQUENCE_VECTOR(std::string)

namespace llvm {
namespace yaml {
template <> struct ScalarEnumerationTraits<FormatStyle::LanguageKind> {
  static void enumeration(IO &IO, FormatStyle::LanguageKind &Value) {
    IO.enumCase(Value, "Cpp", FormatStyle::LK_Cpp);
    IO.enumCase(Value, "Java", FormatStyle::LK_Java);
    IO.enumCase(Value, "JavaScript", FormatStyle::LK_JavaScript);
    IO.enumCase(Value, "Proto", FormatStyle::LK_Proto);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::LanguageStandard> {
  static void enumeration(IO &IO, FormatStyle::LanguageStandard &Value) {
    IO.enumCase(Value, "Cpp03", FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "C++03", FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "Cpp11", FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "C++11", FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "Auto", FormatStyle::LS_Auto);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::UseTabStyle> {
  static void enumeration(IO &IO, FormatStyle::UseTabStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::UT_Never);
    IO.enumCase(Value, "false", FormatStyle::UT_Never);
    IO.enumCase(Value, "Always", FormatStyle::UT_Always);
    IO.enumCase(Value, "true", FormatStyle::UT_Always);
    IO.enumCase(Value, "ForIndentation", FormatStyle::UT_ForIndentation);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::ShortFunctionStyle> {
  static void enumeration(IO &IO, FormatStyle::ShortFunctionStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::SFS_None);
    IO.enumCase(Value, "false", FormatStyle::SFS_None);
    IO.enumCase(Value, "All", FormatStyle::SFS_All);
    IO.enumCase(Value, "true", FormatStyle::SFS_All);
    IO.enumCase(Value, "Inline", FormatStyle::SFS_Inline);
    IO.enumCase(Value, "Empty", FormatStyle::SFS_Empty);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BinaryOperatorStyle> {
  static void enumeration(IO &IO, FormatStyle::BinaryOperatorStyle &Value) {
    IO.enumCase(Value, "All", FormatStyle::BOS_All);
    IO.enumCase(Value, "true", FormatStyle::BOS_All);
    IO.enumCase(Value, "None", FormatStyle::BOS_None);
    IO.enumCase(Value, "false", FormatStyle::BOS_None);
    IO.enumCase(Value, "NonAssignment", FormatStyle::BOS_NonAssignment);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BraceBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::BraceBreakingStyle &Value) {
    IO.enumCase(Value, "Attach", FormatStyle::BS_Attach);
    IO.enumCase(Value, "Linux", FormatStyle::BS_Linux);
    IO.enumCase(Value, "Mozilla", FormatStyle::BS_Mozilla);
    IO.enumCase(Value, "Stroustrup", FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", FormatStyle::BS_Allman);
    IO.enumCase(Value, "GNU", FormatStyle::BS_GNU);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::DefinitionReturnTypeBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::DefinitionReturnTypeBreakingStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::DRTBS_None);
    IO.enumCase(Value, "All", FormatStyle::DRTBS_All);
    IO.enumCase(Value, "TopLevel", FormatStyle::DRTBS_TopLevel);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::DRTBS_None);
    IO.enumCase(Value, "true", FormatStyle::DRTBS_All);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::NamespaceIndentationKind> {
  static void enumeration(IO &IO,
                          FormatStyle::NamespaceIndentationKind &Value) {
    IO.enumCase(Value, "None", FormatStyle::NI_None);
    IO.enumCase(Value, "Inner", FormatStyle::NI_Inner);
    IO.enumCase(Value, "All", FormatStyle::NI_All);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::PointerAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::PointerAlignmentStyle &Value) {
    IO.enumCase(Value, "Middle", FormatStyle::PAS_Middle);
    IO.enumCase(Value, "Left", FormatStyle::PAS_Left);
    IO.enumCase(Value, "Right", FormatStyle::PAS_Right);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::PAS_Left);
    IO.enumCase(Value, "false", FormatStyle::PAS_Right);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SpaceBeforeParensOptions> {
  static void enumeration(IO &IO,
                          FormatStyle::SpaceBeforeParensOptions &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SBPO_Never);
    IO.enumCase(Value, "ControlStatements",
                FormatStyle::SBPO_ControlStatements);
    IO.enumCase(Value, "Always", FormatStyle::SBPO_Always);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::SBPO_Never);
    IO.enumCase(Value, "true", FormatStyle::SBPO_ControlStatements);
  }
};

template <> struct MappingTraits<FormatStyle> {
  static void mapping(IO &IO, FormatStyle &Style) {
    // When reading, read the language first, we need it for getPredefinedStyle.
    IO.mapOptional("Language", Style.Language);

    if (IO.outputting()) {
      StringRef StylesArray[] = {"LLVM",    "Google", "Chromium",
                                 "Mozilla", "WebKit", "GNU"};
      ArrayRef<StringRef> Styles(StylesArray);
      for (size_t i = 0, e = Styles.size(); i < e; ++i) {
        StringRef StyleName(Styles[i]);
        FormatStyle PredefinedStyle;
        if (getPredefinedStyle(StyleName, Style.Language, &PredefinedStyle) &&
            Style == PredefinedStyle) {
          IO.mapOptional("# BasedOnStyle", StyleName);
          break;
        }
      }
    } else {
      StringRef BasedOnStyle;
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      if (!BasedOnStyle.empty()) {
        FormatStyle::LanguageKind OldLanguage = Style.Language;
        FormatStyle::LanguageKind Language =
            ((FormatStyle *)IO.getContext())->Language;
        if (!getPredefinedStyle(BasedOnStyle, Language, &Style)) {
          IO.setError(Twine("Unknown value for BasedOnStyle: ", BasedOnStyle));
          return;
        }
        Style.Language = OldLanguage;
      }
    }

    // For backward compatibility.
    if (!IO.outputting()) {
      IO.mapOptional("DerivePointerBinding", Style.DerivePointerAlignment);
      IO.mapOptional("IndentFunctionDeclarationAfterType",
                     Style.IndentWrappedFunctionNames);
      IO.mapOptional("PointerBindsToType", Style.PointerAlignment);
      IO.mapOptional("SpaceAfterControlStatementKeyword",
                     Style.SpaceBeforeParens);
    }

    IO.mapOptional("AccessModifierOffset", Style.AccessModifierOffset);
    IO.mapOptional("AlignAfterOpenBracket", Style.AlignAfterOpenBracket);
    IO.mapOptional("AlignConsecutiveAssignments",
                   Style.AlignConsecutiveAssignments);
    IO.mapOptional("AlignEscapedNewlinesLeft", Style.AlignEscapedNewlinesLeft);
    IO.mapOptional("AlignOperands", Style.AlignOperands);
    IO.mapOptional("AlignTrailingComments", Style.AlignTrailingComments);
    IO.mapOptional("AllowAllParametersOfDeclarationOnNextLine",
                   Style.AllowAllParametersOfDeclarationOnNextLine);
    IO.mapOptional("AllowShortBlocksOnASingleLine",
                   Style.AllowShortBlocksOnASingleLine);
    IO.mapOptional("AllowShortCaseLabelsOnASingleLine",
                   Style.AllowShortCaseLabelsOnASingleLine);
    IO.mapOptional("AllowShortFunctionsOnASingleLine",
                   Style.AllowShortFunctionsOnASingleLine);
    IO.mapOptional("AllowShortIfStatementsOnASingleLine",
                   Style.AllowShortIfStatementsOnASingleLine);
    IO.mapOptional("AllowShortLoopsOnASingleLine",
                   Style.AllowShortLoopsOnASingleLine);
    IO.mapOptional("AlwaysBreakAfterDefinitionReturnType",
                   Style.AlwaysBreakAfterDefinitionReturnType);
    IO.mapOptional("AlwaysBreakBeforeMultilineStrings",
                   Style.AlwaysBreakBeforeMultilineStrings);
    IO.mapOptional("AlwaysBreakTemplateDeclarations",
                   Style.AlwaysBreakTemplateDeclarations);
    IO.mapOptional("BinPackArguments", Style.BinPackArguments);
    IO.mapOptional("BinPackParameters", Style.BinPackParameters);
    IO.mapOptional("BreakBeforeBinaryOperators",
                   Style.BreakBeforeBinaryOperators);
    IO.mapOptional("BreakBeforeBraces", Style.BreakBeforeBraces);
    IO.mapOptional("BreakBeforeTernaryOperators",
                   Style.BreakBeforeTernaryOperators);
    IO.mapOptional("BreakConstructorInitializersBeforeComma",
                   Style.BreakConstructorInitializersBeforeComma);
    IO.mapOptional("ColumnLimit", Style.ColumnLimit);
    IO.mapOptional("CommentPragmas", Style.CommentPragmas);
    IO.mapOptional("ConstructorInitializerAllOnOneLineOrOnePerLine",
                   Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    IO.mapOptional("ConstructorInitializerIndentWidth",
                   Style.ConstructorInitializerIndentWidth);
    IO.mapOptional("ContinuationIndentWidth", Style.ContinuationIndentWidth);
    IO.mapOptional("Cpp11BracedListStyle", Style.Cpp11BracedListStyle);
    IO.mapOptional("DerivePointerAlignment", Style.DerivePointerAlignment);
    IO.mapOptional("DisableFormat", Style.DisableFormat);
    IO.mapOptional("ExperimentalAutoDetectBinPacking",
                   Style.ExperimentalAutoDetectBinPacking);
    IO.mapOptional("ForEachMacros", Style.ForEachMacros);
    IO.mapOptional("IndentCaseLabels", Style.IndentCaseLabels);
    IO.mapOptional("IndentWidth", Style.IndentWidth);
    IO.mapOptional("IndentWrappedFunctionNames",
                   Style.IndentWrappedFunctionNames);
    IO.mapOptional("KeepEmptyLinesAtTheStartOfBlocks",
                   Style.KeepEmptyLinesAtTheStartOfBlocks);
    IO.mapOptional("MacroBlockBegin", Style.MacroBlockBegin);
    IO.mapOptional("MacroBlockEnd", Style.MacroBlockEnd);
    IO.mapOptional("MaxEmptyLinesToKeep", Style.MaxEmptyLinesToKeep);
    IO.mapOptional("NamespaceIndentation", Style.NamespaceIndentation);
    IO.mapOptional("ObjCBlockIndentWidth", Style.ObjCBlockIndentWidth);
    IO.mapOptional("ObjCSpaceAfterProperty", Style.ObjCSpaceAfterProperty);
    IO.mapOptional("ObjCSpaceBeforeProtocolList",
                   Style.ObjCSpaceBeforeProtocolList);
    IO.mapOptional("PenaltyBreakBeforeFirstCallParameter",
                   Style.PenaltyBreakBeforeFirstCallParameter);
    IO.mapOptional("PenaltyBreakComment", Style.PenaltyBreakComment);
    IO.mapOptional("PenaltyBreakFirstLessLess",
                   Style.PenaltyBreakFirstLessLess);
    IO.mapOptional("PenaltyBreakString", Style.PenaltyBreakString);
    IO.mapOptional("PenaltyExcessCharacter", Style.PenaltyExcessCharacter);
    IO.mapOptional("PenaltyReturnTypeOnItsOwnLine",
                   Style.PenaltyReturnTypeOnItsOwnLine);
    IO.mapOptional("PointerAlignment", Style.PointerAlignment);
    IO.mapOptional("SpaceAfterCStyleCast", Style.SpaceAfterCStyleCast);
    IO.mapOptional("SpaceBeforeAssignmentOperators",
                   Style.SpaceBeforeAssignmentOperators);
    IO.mapOptional("SpaceBeforeParens", Style.SpaceBeforeParens);
    IO.mapOptional("SpaceInEmptyParentheses", Style.SpaceInEmptyParentheses);
    IO.mapOptional("SpacesBeforeTrailingComments",
                   Style.SpacesBeforeTrailingComments);
    IO.mapOptional("SpacesInAngles", Style.SpacesInAngles);
    IO.mapOptional("SpacesInContainerLiterals",
                   Style.SpacesInContainerLiterals);
    IO.mapOptional("SpacesInCStyleCastParentheses",
                   Style.SpacesInCStyleCastParentheses);
    IO.mapOptional("SpacesInParentheses", Style.SpacesInParentheses);
    IO.mapOptional("SpacesInSquareBrackets", Style.SpacesInSquareBrackets);
    IO.mapOptional("Standard", Style.Standard);
    IO.mapOptional("TabWidth", Style.TabWidth);
    IO.mapOptional("UseTab", Style.UseTab);
  }
};

// Allows to read vector<FormatStyle> while keeping default values.
// IO.getContext() should contain a pointer to the FormatStyle structure, that
// will be used to get default values for missing keys.
// If the first element has no Language specified, it will be treated as the
// default one for the following elements.
template <> struct DocumentListTraits<std::vector<FormatStyle>> {
  static size_t size(IO &IO, std::vector<FormatStyle> &Seq) {
    return Seq.size();
  }
  static FormatStyle &element(IO &IO, std::vector<FormatStyle> &Seq,
                              size_t Index) {
    if (Index >= Seq.size()) {
      assert(Index == Seq.size());
      FormatStyle Template;
      if (Seq.size() > 0 && Seq[0].Language == FormatStyle::LK_None) {
        Template = Seq[0];
      } else {
        Template = *((const FormatStyle *)IO.getContext());
        Template.Language = FormatStyle::LK_None;
      }
      Seq.resize(Index + 1, Template);
    }
    return Seq[Index];
  }
};
}
}

namespace clang {
namespace format {

const std::error_category &getParseCategory() {
  static ParseErrorCategory C;
  return C;
}
std::error_code make_error_code(ParseError e) {
  return std::error_code(static_cast<int>(e), getParseCategory());
}

const char *ParseErrorCategory::name() const LLVM_NOEXCEPT {
  return "clang-format.parse_error";
}

std::string ParseErrorCategory::message(int EV) const {
  switch (static_cast<ParseError>(EV)) {
  case ParseError::Success:
    return "Success";
  case ParseError::Error:
    return "Invalid argument";
  case ParseError::Unsuitable:
    return "Unsuitable";
  }
  llvm_unreachable("unexpected parse error");
}

FormatStyle getLLVMStyle() {
  FormatStyle LLVMStyle;
  LLVMStyle.Language = FormatStyle::LK_Cpp;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignEscapedNewlinesLeft = false;
  LLVMStyle.AlignAfterOpenBracket = true;
  LLVMStyle.AlignOperands = true;
  LLVMStyle.AlignTrailingComments = true;
  LLVMStyle.AlignConsecutiveAssignments = false;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_All;
  LLVMStyle.AllowShortBlocksOnASingleLine = false;
  LLVMStyle.AllowShortCaseLabelsOnASingleLine = false;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = false;
  LLVMStyle.AllowShortLoopsOnASingleLine = false;
  LLVMStyle.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_None;
  LLVMStyle.AlwaysBreakBeforeMultilineStrings = false;
  LLVMStyle.AlwaysBreakTemplateDeclarations = false;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BinPackArguments = true;
  LLVMStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_None;
  LLVMStyle.BreakBeforeTernaryOperators = true;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BreakConstructorInitializersBeforeComma = false;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.CommentPragmas = "^ IWYU pragma:";
  LLVMStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
  LLVMStyle.ConstructorInitializerIndentWidth = 4;
  LLVMStyle.ContinuationIndentWidth = 4;
  LLVMStyle.Cpp11BracedListStyle = true;
  LLVMStyle.DerivePointerAlignment = false;
  LLVMStyle.ExperimentalAutoDetectBinPacking = false;
  LLVMStyle.ForEachMacros.push_back("foreach");
  LLVMStyle.ForEachMacros.push_back("Q_FOREACH");
  LLVMStyle.ForEachMacros.push_back("BOOST_FOREACH");
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentWrappedFunctionNames = false;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.TabWidth = 8;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.KeepEmptyLinesAtTheStartOfBlocks = true;
  LLVMStyle.NamespaceIndentation = FormatStyle::NI_None;
  LLVMStyle.ObjCBlockIndentWidth = 2;
  LLVMStyle.ObjCSpaceAfterProperty = false;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PointerAlignment = FormatStyle::PAS_Right;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.Standard = FormatStyle::LS_Cpp11;
  LLVMStyle.UseTab = FormatStyle::UT_Never;
  LLVMStyle.SpacesInParentheses = false;
  LLVMStyle.SpacesInSquareBrackets = false;
  LLVMStyle.SpaceInEmptyParentheses = false;
  LLVMStyle.SpacesInContainerLiterals = true;
  LLVMStyle.SpacesInCStyleCastParentheses = false;
  LLVMStyle.SpaceAfterCStyleCast = false;
  LLVMStyle.SpaceBeforeParens = FormatStyle::SBPO_ControlStatements;
  LLVMStyle.SpaceBeforeAssignmentOperators = true;
  LLVMStyle.SpacesInAngles = false;

  LLVMStyle.PenaltyBreakComment = 300;
  LLVMStyle.PenaltyBreakFirstLessLess = 120;
  LLVMStyle.PenaltyBreakString = 1000;
  LLVMStyle.PenaltyExcessCharacter = 1000000;
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 60;
  LLVMStyle.PenaltyBreakBeforeFirstCallParameter = 19;

  LLVMStyle.DisableFormat = false;

  return LLVMStyle;
}

FormatStyle getGoogleStyle(FormatStyle::LanguageKind Language) {
  FormatStyle GoogleStyle = getLLVMStyle();
  GoogleStyle.Language = Language;

  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlinesLeft = true;
  GoogleStyle.AllowShortIfStatementsOnASingleLine = true;
  GoogleStyle.AllowShortLoopsOnASingleLine = true;
  GoogleStyle.AlwaysBreakBeforeMultilineStrings = true;
  GoogleStyle.AlwaysBreakTemplateDeclarations = true;
  GoogleStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  GoogleStyle.DerivePointerAlignment = true;
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.KeepEmptyLinesAtTheStartOfBlocks = false;
  GoogleStyle.ObjCSpaceAfterProperty = false;
  GoogleStyle.ObjCSpaceBeforeProtocolList = false;
  GoogleStyle.PointerAlignment = FormatStyle::PAS_Left;
  GoogleStyle.SpacesBeforeTrailingComments = 2;
  GoogleStyle.Standard = FormatStyle::LS_Auto;

  GoogleStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  GoogleStyle.PenaltyBreakBeforeFirstCallParameter = 1;

  if (Language == FormatStyle::LK_Java) {
    GoogleStyle.AlignAfterOpenBracket = false;
    GoogleStyle.AlignOperands = false;
    GoogleStyle.AlignTrailingComments = false;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    GoogleStyle.AllowShortIfStatementsOnASingleLine = false;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_NonAssignment;
    GoogleStyle.ColumnLimit = 100;
    GoogleStyle.SpaceAfterCStyleCast = true;
    GoogleStyle.SpacesBeforeTrailingComments = 1;
  } else if (Language == FormatStyle::LK_JavaScript) {
    GoogleStyle.BreakBeforeTernaryOperators = false;
    GoogleStyle.MaxEmptyLinesToKeep = 3;
    GoogleStyle.SpacesInContainerLiterals = false;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
  } else if (Language == FormatStyle::LK_Proto) {
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_None;
    GoogleStyle.SpacesInContainerLiterals = false;
  }

  return GoogleStyle;
}

FormatStyle getChromiumStyle(FormatStyle::LanguageKind Language) {
  FormatStyle ChromiumStyle = getGoogleStyle(Language);
  if (Language == FormatStyle::LK_Java) {
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = true;
    ChromiumStyle.IndentWidth = 4;
    ChromiumStyle.ContinuationIndentWidth = 8;
  } else {
    ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
    ChromiumStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
    ChromiumStyle.AllowShortLoopsOnASingleLine = false;
    ChromiumStyle.BinPackParameters = false;
    ChromiumStyle.DerivePointerAlignment = false;
  }
  ChromiumStyle.MacroBlockBegin = "^IPC_BEGIN_MESSAGE_MAP$";
  ChromiumStyle.MacroBlockBegin = "^IPC_END_MESSAGE_MAP$";
  return ChromiumStyle;
}

FormatStyle getMozillaStyle() {
  FormatStyle MozillaStyle = getLLVMStyle();
  MozillaStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  MozillaStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
  MozillaStyle.AlwaysBreakAfterDefinitionReturnType =
      FormatStyle::DRTBS_TopLevel;
  MozillaStyle.AlwaysBreakTemplateDeclarations = true;
  MozillaStyle.BreakBeforeBraces = FormatStyle::BS_Mozilla;
  MozillaStyle.BreakConstructorInitializersBeforeComma = true;
  MozillaStyle.ConstructorInitializerIndentWidth = 2;
  MozillaStyle.ContinuationIndentWidth = 2;
  MozillaStyle.Cpp11BracedListStyle = false;
  MozillaStyle.IndentCaseLabels = true;
  MozillaStyle.ObjCSpaceAfterProperty = true;
  MozillaStyle.ObjCSpaceBeforeProtocolList = false;
  MozillaStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  MozillaStyle.PointerAlignment = FormatStyle::PAS_Left;
  return MozillaStyle;
}

FormatStyle getWebKitStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AccessModifierOffset = -4;
  Style.AlignAfterOpenBracket = false;
  Style.AlignOperands = false;
  Style.AlignTrailingComments = false;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_Stroustrup;
  Style.BreakConstructorInitializersBeforeComma = true;
  Style.Cpp11BracedListStyle = false;
  Style.ColumnLimit = 0;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.ObjCBlockIndentWidth = 4;
  Style.ObjCSpaceAfterProperty = true;
  Style.PointerAlignment = FormatStyle::PAS_Left;
  Style.Standard = FormatStyle::LS_Cpp03;
  return Style;
}

FormatStyle getGNUStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_All;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_GNU;
  Style.BreakBeforeTernaryOperators = true;
  Style.Cpp11BracedListStyle = false;
  Style.ColumnLimit = 79;
  Style.SpaceBeforeParens = FormatStyle::SBPO_Always;
  Style.Standard = FormatStyle::LS_Cpp03;
  return Style;
}

FormatStyle getNoStyle() {
  FormatStyle NoStyle = getLLVMStyle();
  NoStyle.DisableFormat = true;
  return NoStyle;
}

bool getPredefinedStyle(StringRef Name, FormatStyle::LanguageKind Language,
                        FormatStyle *Style) {
  if (Name.equals_lower("llvm")) {
    *Style = getLLVMStyle();
  } else if (Name.equals_lower("chromium")) {
    *Style = getChromiumStyle(Language);
  } else if (Name.equals_lower("mozilla")) {
    *Style = getMozillaStyle();
  } else if (Name.equals_lower("google")) {
    *Style = getGoogleStyle(Language);
  } else if (Name.equals_lower("webkit")) {
    *Style = getWebKitStyle();
  } else if (Name.equals_lower("gnu")) {
    *Style = getGNUStyle();
  } else if (Name.equals_lower("none")) {
    *Style = getNoStyle();
  } else {
    return false;
  }

  Style->Language = Language;
  return true;
}

std::error_code parseConfiguration(StringRef Text, FormatStyle *Style) {
  assert(Style);
  FormatStyle::LanguageKind Language = Style->Language;
  assert(Language != FormatStyle::LK_None);
  if (Text.trim().empty())
    return make_error_code(ParseError::Error);

  std::vector<FormatStyle> Styles;
  llvm::yaml::Input Input(Text);
  // DocumentListTraits<vector<FormatStyle>> uses the context to get default
  // values for the fields, keys for which are missing from the configuration.
  // Mapping also uses the context to get the language to find the correct
  // base style.
  Input.setContext(Style);
  Input >> Styles;
  if (Input.error())
    return Input.error();

  for (unsigned i = 0; i < Styles.size(); ++i) {
    // Ensures that only the first configuration can skip the Language option.
    if (Styles[i].Language == FormatStyle::LK_None && i != 0)
      return make_error_code(ParseError::Error);
    // Ensure that each language is configured at most once.
    for (unsigned j = 0; j < i; ++j) {
      if (Styles[i].Language == Styles[j].Language) {
        DEBUG(llvm::dbgs()
              << "Duplicate languages in the config file on positions " << j
              << " and " << i << "\n");
        return make_error_code(ParseError::Error);
      }
    }
  }
  // Look for a suitable configuration starting from the end, so we can
  // find the configuration for the specific language first, and the default
  // configuration (which can only be at slot 0) after it.
  for (int i = Styles.size() - 1; i >= 0; --i) {
    if (Styles[i].Language == Language ||
        Styles[i].Language == FormatStyle::LK_None) {
      *Style = Styles[i];
      Style->Language = Language;
      return make_error_code(ParseError::Success);
    }
  }
  return make_error_code(ParseError::Unsuitable);
}

std::string configurationAsText(const FormatStyle &Style) {
  std::string Text;
  llvm::raw_string_ostream Stream(Text);
  llvm::yaml::Output Output(Stream);
  // We use the same mapping method for input and output, so we need a non-const
  // reference here.
  FormatStyle NonConstStyle = Style;
  Output << NonConstStyle;
  return Stream.str();
}

namespace {

class FormatTokenLexer {
public:
  FormatTokenLexer(SourceManager &SourceMgr, FileID ID, FormatStyle &Style,
                   encoding::Encoding Encoding)
      : FormatTok(nullptr), IsFirstToken(true), GreaterStashed(false),
        LessStashed(false), Column(0), TrailingWhitespace(0),
        SourceMgr(SourceMgr), ID(ID), Style(Style),
        IdentTable(getFormattingLangOpts(Style)), Keywords(IdentTable),
        Encoding(Encoding), FirstInLineIndex(0), FormattingDisabled(false),
        MacroBlockBeginRegex(Style.MacroBlockBegin),
        MacroBlockEndRegex(Style.MacroBlockEnd) {
    Lex.reset(new Lexer(ID, SourceMgr.getBuffer(ID), SourceMgr,
                        getFormattingLangOpts(Style)));
    Lex->SetKeepWhitespaceMode(true);

    for (const std::string &ForEachMacro : Style.ForEachMacros)
      ForEachMacros.push_back(&IdentTable.get(ForEachMacro));
    std::sort(ForEachMacros.begin(), ForEachMacros.end());
  }

  ArrayRef<FormatToken *> lex() {
    assert(Tokens.empty());
    assert(FirstInLineIndex == 0);
    do {
      Tokens.push_back(getNextToken());
      tryMergePreviousTokens();
      if (Tokens.back()->NewlinesBefore > 0 || Tokens.back()->IsMultiline)
        FirstInLineIndex = Tokens.size() - 1;
    } while (Tokens.back()->Tok.isNot(tok::eof));
    return Tokens;
  }

  const AdditionalKeywords &getKeywords() { return Keywords; }

private:
  void tryMergePreviousTokens() {
    if (tryMerge_TMacro())
      return;
    if (tryMergeConflictMarkers())
      return;
    if (tryMergeLessLess())
      return;

    if (Style.Language == FormatStyle::LK_JavaScript) {
      if (tryMergeJSRegexLiteral())
        return;
      if (tryMergeEscapeSequence())
        return;
      if (tryMergeTemplateString())
        return;

      static const tok::TokenKind JSIdentity[] = {tok::equalequal, tok::equal};
      static const tok::TokenKind JSNotIdentity[] = {tok::exclaimequal,
                                                     tok::equal};
      static const tok::TokenKind JSShiftEqual[] = {tok::greater, tok::greater,
                                                    tok::greaterequal};
      static const tok::TokenKind JSRightArrow[] = {tok::equal, tok::greater};
      // FIXME: Investigate what token type gives the correct operator priority.
      if (tryMergeTokens(JSIdentity, TT_BinaryOperator))
        return;
      if (tryMergeTokens(JSNotIdentity, TT_BinaryOperator))
        return;
      if (tryMergeTokens(JSShiftEqual, TT_BinaryOperator))
        return;
      if (tryMergeTokens(JSRightArrow, TT_JsFatArrow))
        return;
    }
  }

  bool tryMergeLessLess() {
    // Merge X,less,less,Y into X,lessless,Y unless X or Y is less.
    if (Tokens.size() < 3)
      return false;

    bool FourthTokenIsLess = false;
    if (Tokens.size() > 3)
      FourthTokenIsLess = (Tokens.end() - 4)[0]->is(tok::less);

    auto First = Tokens.end() - 3;
    if (First[2]->is(tok::less) || First[1]->isNot(tok::less) ||
        First[0]->isNot(tok::less) || FourthTokenIsLess)
      return false;

    // Only merge if there currently is no whitespace between the two "<".
    if (First[1]->WhitespaceRange.getBegin() !=
        First[1]->WhitespaceRange.getEnd())
      return false;

    First[0]->Tok.setKind(tok::lessless);
    First[0]->TokenText = "<<";
    First[0]->ColumnWidth += 1;
    Tokens.erase(Tokens.end() - 2);
    return true;
  }

  bool tryMergeTokens(ArrayRef<tok::TokenKind> Kinds, TokenType NewType) {
    if (Tokens.size() < Kinds.size())
      return false;

    SmallVectorImpl<FormatToken *>::const_iterator First =
        Tokens.end() - Kinds.size();
    if (!First[0]->is(Kinds[0]))
      return false;
    unsigned AddLength = 0;
    for (unsigned i = 1; i < Kinds.size(); ++i) {
      if (!First[i]->is(Kinds[i]) ||
          First[i]->WhitespaceRange.getBegin() !=
              First[i]->WhitespaceRange.getEnd())
        return false;
      AddLength += First[i]->TokenText.size();
    }
    Tokens.resize(Tokens.size() - Kinds.size() + 1);
    First[0]->TokenText = StringRef(First[0]->TokenText.data(),
                                    First[0]->TokenText.size() + AddLength);
    First[0]->ColumnWidth += AddLength;
    First[0]->Type = NewType;
    return true;
  }

  // Tries to merge an escape sequence, i.e. a "\\" and the following
  // character. Use e.g. inside JavaScript regex literals.
  bool tryMergeEscapeSequence() {
    if (Tokens.size() < 2)
      return false;
    FormatToken *Previous = Tokens[Tokens.size() - 2];
    if (Previous->isNot(tok::unknown) || Previous->TokenText != "\\")
      return false;
    ++Previous->ColumnWidth;
    StringRef Text = Previous->TokenText;
    Previous->TokenText = StringRef(Text.data(), Text.size() + 1);
    resetLexer(SourceMgr.getFileOffset(Tokens.back()->Tok.getLocation()) + 1);
    Tokens.resize(Tokens.size() - 1);
    Column = Previous->OriginalColumn + Previous->ColumnWidth;
    return true;
  }

  // Try to determine whether the current token ends a JavaScript regex literal.
  // We heuristically assume that this is a regex literal if we find two
  // unescaped slashes on a line and the token before the first slash is one of
  // "(;,{}![:?", a binary operator or 'return', as those cannot be followed by
  // a division.
  bool tryMergeJSRegexLiteral() {
    if (Tokens.size() < 2)
      return false;

    // If this is a string literal with a slash inside, compute the slash's
    // offset and try to find the beginning of the regex literal.
    // Also look at tok::unknown, as it can be an unterminated char literal.
    size_t SlashInStringPos = StringRef::npos;
    if (Tokens.back()->isOneOf(tok::string_literal, tok::char_constant,
                               tok::unknown)) {
      // Start search from position 1 as otherwise, this is an unknown token
      // for an unterminated /*-comment which is handled elsewhere.
      SlashInStringPos = Tokens.back()->TokenText.find('/', 1);
      if (SlashInStringPos == StringRef::npos)
        return false;
    }

    // If a regex literal ends in "\//", this gets represented by an unknown
    // token "\" and a comment.
    bool MightEndWithEscapedSlash =
        Tokens.back()->is(tok::comment) &&
        Tokens.back()->TokenText.startswith("//") &&
        Tokens[Tokens.size() - 2]->TokenText == "\\";
    if (!MightEndWithEscapedSlash && SlashInStringPos == StringRef::npos &&
        (Tokens.back()->isNot(tok::slash) ||
         (Tokens[Tokens.size() - 2]->is(tok::unknown) &&
          Tokens[Tokens.size() - 2]->TokenText == "\\")))
      return false;

    unsigned TokenCount = 0;
    for (auto I = Tokens.rbegin() + 1, E = Tokens.rend(); I != E; ++I) {
      ++TokenCount;
      auto Prev = I + 1;
      while (Prev != E && Prev[0]->is(tok::comment))
        ++Prev;
      if (I[0]->isOneOf(tok::slash, tok::slashequal) &&
          (Prev == E ||
           ((Prev[0]->isOneOf(tok::l_paren, tok::semi, tok::l_brace,
                              tok::r_brace, tok::exclaim, tok::l_square,
                              tok::colon, tok::comma, tok::question,
                              tok::kw_return) ||
             Prev[0]->isBinaryOperator())))) {
        unsigned LastColumn = Tokens.back()->OriginalColumn;
        SourceLocation Loc = Tokens.back()->Tok.getLocation();
        if (MightEndWithEscapedSlash) {
          // This regex literal ends in '\//'. Skip past the '//' of the last
          // token and re-start lexing from there.
          resetLexer(SourceMgr.getFileOffset(Loc) + 2);
        } else if (SlashInStringPos != StringRef::npos) {
          // This regex literal ends in a string_literal with a slash inside.
          // Calculate end column and reset lexer appropriately.
          resetLexer(SourceMgr.getFileOffset(Loc) + SlashInStringPos + 1);
          LastColumn += SlashInStringPos;
        }
        Tokens.resize(Tokens.size() - TokenCount);
        Tokens.back()->Tok.setKind(tok::unknown);
        Tokens.back()->Type = TT_RegexLiteral;
        // Treat regex literals like other string_literals.
        Tokens.back()->Tok.setKind(tok::string_literal);
        Tokens.back()->ColumnWidth += LastColumn - I[0]->OriginalColumn;
        return true;
      }

      // There can't be a newline inside a regex literal.
      if (I[0]->NewlinesBefore > 0)
        return false;
    }
    return false;
  }

  bool tryMergeTemplateString() {
    if (Tokens.size() < 2)
      return false;

    FormatToken *EndBacktick = Tokens.back();
    // Backticks get lexed as tok::unknown tokens. If a template string contains
    // a comment start, it gets lexed as a tok::comment, or tok::unknown if
    // unterminated.
    if (!EndBacktick->isOneOf(tok::comment, tok::string_literal,
                              tok::char_constant, tok::unknown))
      return false;
    size_t CommentBacktickPos = EndBacktick->TokenText.find('`');
    // Unknown token that's not actually a backtick, or a comment that doesn't
    // contain a backtick.
    if (CommentBacktickPos == StringRef::npos)
      return false;

    unsigned TokenCount = 0;
    bool IsMultiline = false;
    unsigned EndColumnInFirstLine =
        EndBacktick->OriginalColumn + EndBacktick->ColumnWidth;
    for (auto I = Tokens.rbegin() + 1, E = Tokens.rend(); I != E; I++) {
      ++TokenCount;
      if (I[0]->IsMultiline)
        IsMultiline = true;

      // If there was a preceding template string, this must be the start of a
      // template string, not the end.
      if (I[0]->is(TT_TemplateString))
        return false;

      if (I[0]->isNot(tok::unknown) || I[0]->TokenText != "`") {
        // Keep track of the rhs offset of the last token to wrap across lines -
        // its the rhs offset of the first line of the template string, used to
        // determine its width.
        if (I[0]->IsMultiline)
          EndColumnInFirstLine = I[0]->OriginalColumn + I[0]->ColumnWidth;
        // If the token has newlines, the token before it (if it exists) is the
        // rhs end of the previous line.
        if (I[0]->NewlinesBefore > 0 && (I + 1 != E)) {
          EndColumnInFirstLine = I[1]->OriginalColumn + I[1]->ColumnWidth;
          IsMultiline = true;
        }
        continue;
      }

      Tokens.resize(Tokens.size() - TokenCount);
      Tokens.back()->Type = TT_TemplateString;
      const char *EndOffset =
          EndBacktick->TokenText.data() + 1 + CommentBacktickPos;
      if (CommentBacktickPos != 0) {
        // If the backtick was not the first character (e.g. in a comment),
        // re-lex after the backtick position.
        SourceLocation Loc = EndBacktick->Tok.getLocation();
        resetLexer(SourceMgr.getFileOffset(Loc) + CommentBacktickPos + 1);
      }
      Tokens.back()->TokenText =
          StringRef(Tokens.back()->TokenText.data(),
                    EndOffset - Tokens.back()->TokenText.data());

      unsigned EndOriginalColumn = EndBacktick->OriginalColumn;
      if (EndOriginalColumn == 0) {
        SourceLocation Loc = EndBacktick->Tok.getLocation();
        EndOriginalColumn = SourceMgr.getSpellingColumnNumber(Loc);
      }
      // If the ` is further down within the token (e.g. in a comment).
      EndOriginalColumn += CommentBacktickPos;

      if (IsMultiline) {
        // ColumnWidth is from backtick to last token in line.
        // LastLineColumnWidth is 0 to backtick.
        // x = `some content
        //     until here`;
        Tokens.back()->ColumnWidth =
            EndColumnInFirstLine - Tokens.back()->OriginalColumn;
        // +1 for the ` itself.
        Tokens.back()->LastLineColumnWidth = EndOriginalColumn + 1;
        Tokens.back()->IsMultiline = true;
      } else {
        // Token simply spans from start to end, +1 for the ` itself.
        Tokens.back()->ColumnWidth =
            EndOriginalColumn - Tokens.back()->OriginalColumn + 1;
      }
      return true;
    }
    return false;
  }

  bool tryMerge_TMacro() {
    if (Tokens.size() < 4)
      return false;
    FormatToken *Last = Tokens.back();
    if (!Last->is(tok::r_paren))
      return false;

    FormatToken *String = Tokens[Tokens.size() - 2];
    if (!String->is(tok::string_literal) || String->IsMultiline)
      return false;

    if (!Tokens[Tokens.size() - 3]->is(tok::l_paren))
      return false;

    FormatToken *Macro = Tokens[Tokens.size() - 4];
    if (Macro->TokenText != "_T")
      return false;

    const char *Start = Macro->TokenText.data();
    const char *End = Last->TokenText.data() + Last->TokenText.size();
    String->TokenText = StringRef(Start, End - Start);
    String->IsFirst = Macro->IsFirst;
    String->LastNewlineOffset = Macro->LastNewlineOffset;
    String->WhitespaceRange = Macro->WhitespaceRange;
    String->OriginalColumn = Macro->OriginalColumn;
    String->ColumnWidth = encoding::columnWidthWithTabs(
        String->TokenText, String->OriginalColumn, Style.TabWidth, Encoding);
    String->NewlinesBefore = Macro->NewlinesBefore;
    String->HasUnescapedNewline = Macro->HasUnescapedNewline;

    Tokens.pop_back();
    Tokens.pop_back();
    Tokens.pop_back();
    Tokens.back() = String;
    return true;
  }

  bool tryMergeConflictMarkers() {
    if (Tokens.back()->NewlinesBefore == 0 && Tokens.back()->isNot(tok::eof))
      return false;

    // Conflict lines look like:
    // <marker> <text from the vcs>
    // For example:
    // >>>>>>> /file/in/file/system at revision 1234
    //
    // We merge all tokens in a line that starts with a conflict marker
    // into a single token with a special token type that the unwrapped line
    // parser will use to correctly rebuild the underlying code.

    FileID ID;
    // Get the position of the first token in the line.
    unsigned FirstInLineOffset;
    std::tie(ID, FirstInLineOffset) = SourceMgr.getDecomposedLoc(
        Tokens[FirstInLineIndex]->getStartOfNonWhitespace());
    StringRef Buffer = SourceMgr.getBuffer(ID)->getBuffer();
    // Calculate the offset of the start of the current line.
    auto LineOffset = Buffer.rfind('\n', FirstInLineOffset);
    if (LineOffset == StringRef::npos) {
      LineOffset = 0;
    } else {
      ++LineOffset;
    }

    auto FirstSpace = Buffer.find_first_of(" \n", LineOffset);
    StringRef LineStart;
    if (FirstSpace == StringRef::npos) {
      LineStart = Buffer.substr(LineOffset);
    } else {
      LineStart = Buffer.substr(LineOffset, FirstSpace - LineOffset);
    }

    TokenType Type = TT_Unknown;
    if (LineStart == "<<<<<<<" || LineStart == ">>>>") {
      Type = TT_ConflictStart;
    } else if (LineStart == "|||||||" || LineStart == "=======" ||
               LineStart == "====") {
      Type = TT_ConflictAlternative;
    } else if (LineStart == ">>>>>>>" || LineStart == "<<<<") {
      Type = TT_ConflictEnd;
    }

    if (Type != TT_Unknown) {
      FormatToken *Next = Tokens.back();

      Tokens.resize(FirstInLineIndex + 1);
      // We do not need to build a complete token here, as we will skip it
      // during parsing anyway (as we must not touch whitespace around conflict
      // markers).
      Tokens.back()->Type = Type;
      Tokens.back()->Tok.setKind(tok::kw___unknown_anytype);

      Tokens.push_back(Next);
      return true;
    }

    return false;
  }

  FormatToken *getStashedToken() {
    // Create a synthesized second '>' or '<' token.
    Token Tok = FormatTok->Tok;
    StringRef TokenText = FormatTok->TokenText;

    unsigned OriginalColumn = FormatTok->OriginalColumn;
    FormatTok = new (Allocator.Allocate()) FormatToken;
    FormatTok->Tok = Tok;
    SourceLocation TokLocation =
        FormatTok->Tok.getLocation().getLocWithOffset(Tok.getLength() - 1);
    FormatTok->Tok.setLocation(TokLocation);
    FormatTok->WhitespaceRange = SourceRange(TokLocation, TokLocation);
    FormatTok->TokenText = TokenText;
    FormatTok->ColumnWidth = 1;
    FormatTok->OriginalColumn = OriginalColumn + 1;

    return FormatTok;
  }

  FormatToken *getNextToken() {
    if (GreaterStashed) {
      GreaterStashed = false;
      return getStashedToken();
    }
    if (LessStashed) {
      LessStashed = false;
      return getStashedToken();
    }

    FormatTok = new (Allocator.Allocate()) FormatToken;
    readRawToken(*FormatTok);
    SourceLocation WhitespaceStart =
        FormatTok->Tok.getLocation().getLocWithOffset(-TrailingWhitespace);
    FormatTok->IsFirst = IsFirstToken;
    IsFirstToken = false;

    // Consume and record whitespace until we find a significant token.
    unsigned WhitespaceLength = TrailingWhitespace;
    while (FormatTok->Tok.is(tok::unknown)) {
      StringRef Text = FormatTok->TokenText;
      auto EscapesNewline = [&](int pos) {
        // A '\r' here is just part of '\r\n'. Skip it.
        if (pos >= 0 && Text[pos] == '\r')
          --pos;
        // See whether there is an odd number of '\' before this.
        unsigned count = 0;
        for (; pos >= 0; --pos, ++count)
          if (Text[pos] != '\\')
            break;
        return count & 1;
      };
      // FIXME: This miscounts tok:unknown tokens that are not just
      // whitespace, e.g. a '`' character.
      for (int i = 0, e = Text.size(); i != e; ++i) {
        switch (Text[i]) {
        case '\n':
          ++FormatTok->NewlinesBefore;
          FormatTok->HasUnescapedNewline = !EscapesNewline(i - 1);
          FormatTok->LastNewlineOffset = WhitespaceLength + i + 1;
          Column = 0;
          break;
        case '\r':
          FormatTok->LastNewlineOffset = WhitespaceLength + i + 1;
          Column = 0;
          break;
        case '\f':
        case '\v':
          Column = 0;
          break;
        case ' ':
          ++Column;
          break;
        case '\t':
          Column += Style.TabWidth - Column % Style.TabWidth;
          break;
        case '\\':
          if (i + 1 == e || (Text[i + 1] != '\r' && Text[i + 1] != '\n'))
            FormatTok->Type = TT_ImplicitStringLiteral;
          break;
        default:
          FormatTok->Type = TT_ImplicitStringLiteral;
          break;
        }
      }

      if (FormatTok->is(TT_ImplicitStringLiteral))
        break;
      WhitespaceLength += FormatTok->Tok.getLength();

      readRawToken(*FormatTok);
    }

    // In case the token starts with escaped newlines, we want to
    // take them into account as whitespace - this pattern is quite frequent
    // in macro definitions.
    // FIXME: Add a more explicit test.
    while (FormatTok->TokenText.size() > 1 && FormatTok->TokenText[0] == '\\' &&
           FormatTok->TokenText[1] == '\n') {
      ++FormatTok->NewlinesBefore;
      WhitespaceLength += 2;
      FormatTok->LastNewlineOffset = 2;
      Column = 0;
      FormatTok->TokenText = FormatTok->TokenText.substr(2);
    }

    FormatTok->WhitespaceRange = SourceRange(
        WhitespaceStart, WhitespaceStart.getLocWithOffset(WhitespaceLength));

    FormatTok->OriginalColumn = Column;

    TrailingWhitespace = 0;
    if (FormatTok->Tok.is(tok::comment)) {
      // FIXME: Add the trimmed whitespace to Column.
      StringRef UntrimmedText = FormatTok->TokenText;
      FormatTok->TokenText = FormatTok->TokenText.rtrim(" \t\v\f");
      TrailingWhitespace = UntrimmedText.size() - FormatTok->TokenText.size();
    } else if (FormatTok->Tok.is(tok::raw_identifier)) {
      IdentifierInfo &Info = IdentTable.get(FormatTok->TokenText);
      FormatTok->Tok.setIdentifierInfo(&Info);
      FormatTok->Tok.setKind(Info.getTokenID());
      if (Style.Language == FormatStyle::LK_Java &&
          FormatTok->isOneOf(tok::kw_struct, tok::kw_union, tok::kw_delete)) {
        FormatTok->Tok.setKind(tok::identifier);
        FormatTok->Tok.setIdentifierInfo(nullptr);
      }
    } else if (FormatTok->Tok.is(tok::greatergreater)) {
      FormatTok->Tok.setKind(tok::greater);
      FormatTok->TokenText = FormatTok->TokenText.substr(0, 1);
      GreaterStashed = true;
    } else if (FormatTok->Tok.is(tok::lessless)) {
      FormatTok->Tok.setKind(tok::less);
      FormatTok->TokenText = FormatTok->TokenText.substr(0, 1);
      LessStashed = true;
    }

    // Now FormatTok is the next non-whitespace token.

    StringRef Text = FormatTok->TokenText;
    size_t FirstNewlinePos = Text.find('\n');
    if (FirstNewlinePos == StringRef::npos) {
      // FIXME: ColumnWidth actually depends on the start column, we need to
      // take this into account when the token is moved.
      FormatTok->ColumnWidth =
          encoding::columnWidthWithTabs(Text, Column, Style.TabWidth, Encoding);
      Column += FormatTok->ColumnWidth;
    } else {
      FormatTok->IsMultiline = true;
      // FIXME: ColumnWidth actually depends on the start column, we need to
      // take this into account when the token is moved.
      FormatTok->ColumnWidth = encoding::columnWidthWithTabs(
          Text.substr(0, FirstNewlinePos), Column, Style.TabWidth, Encoding);

      // The last line of the token always starts in column 0.
      // Thus, the length can be precomputed even in the presence of tabs.
      FormatTok->LastLineColumnWidth = encoding::columnWidthWithTabs(
          Text.substr(Text.find_last_of('\n') + 1), 0, Style.TabWidth,
          Encoding);
      Column = FormatTok->LastLineColumnWidth;
    }

    if (Style.Language == FormatStyle::LK_Cpp) {
      if (!(Tokens.size() > 0 && Tokens.back()->Tok.getIdentifierInfo() &&
            Tokens.back()->Tok.getIdentifierInfo()->getPPKeywordID() ==
                tok::pp_define) &&
          std::find(ForEachMacros.begin(), ForEachMacros.end(),
                    FormatTok->Tok.getIdentifierInfo()) != ForEachMacros.end()) {
        FormatTok->Type = TT_ForEachMacro;
      } else if (FormatTok->is(tok::identifier)) {
        if (MacroBlockBeginRegex.match(Text)) {
          FormatTok->Type = TT_MacroBlockBegin;
        } else if (MacroBlockEndRegex.match(Text)) {
          FormatTok->Type = TT_MacroBlockEnd;
        }
      }
    }

    return FormatTok;
  }

  FormatToken *FormatTok;
  bool IsFirstToken;
  bool GreaterStashed, LessStashed;
  unsigned Column;
  unsigned TrailingWhitespace;
  std::unique_ptr<Lexer> Lex;
  SourceManager &SourceMgr;
  FileID ID;
  FormatStyle &Style;
  IdentifierTable IdentTable;
  AdditionalKeywords Keywords;
  encoding::Encoding Encoding;
  llvm::SpecificBumpPtrAllocator<FormatToken> Allocator;
  // Index (in 'Tokens') of the last token that starts a new line.
  unsigned FirstInLineIndex;
  SmallVector<FormatToken *, 16> Tokens;
  SmallVector<IdentifierInfo *, 8> ForEachMacros;

  bool FormattingDisabled;

  llvm::Regex MacroBlockBeginRegex;
  llvm::Regex MacroBlockEndRegex;

  void readRawToken(FormatToken &Tok) {
    Lex->LexFromRawLexer(Tok.Tok);
    Tok.TokenText = StringRef(SourceMgr.getCharacterData(Tok.Tok.getLocation()),
                              Tok.Tok.getLength());
    // For formatting, treat unterminated string literals like normal string
    // literals.
    if (Tok.is(tok::unknown)) {
      if (!Tok.TokenText.empty() && Tok.TokenText[0] == '"') {
        Tok.Tok.setKind(tok::string_literal);
        Tok.IsUnterminatedLiteral = true;
      } else if (Style.Language == FormatStyle::LK_JavaScript &&
                 Tok.TokenText == "''") {
        Tok.Tok.setKind(tok::char_constant);
      }
    }

    if (Tok.is(tok::comment) && (Tok.TokenText == "// clang-format on" ||
                                 Tok.TokenText == "/* clang-format on */")) {
      FormattingDisabled = false;
    }

    Tok.Finalized = FormattingDisabled;

    if (Tok.is(tok::comment) && (Tok.TokenText == "// clang-format off" ||
                                 Tok.TokenText == "/* clang-format off */")) {
      FormattingDisabled = true;
    }
  }

  void resetLexer(unsigned Offset) {
    StringRef Buffer = SourceMgr.getBufferData(ID);
    Lex.reset(new Lexer(SourceMgr.getLocForStartOfFile(ID),
                        getFormattingLangOpts(Style), Buffer.begin(),
                        Buffer.begin() + Offset, Buffer.end()));
    Lex->SetKeepWhitespaceMode(true);
    TrailingWhitespace = 0;
  }
};

static StringRef getLanguageName(FormatStyle::LanguageKind Language) {
  switch (Language) {
  case FormatStyle::LK_Cpp:
    return "C++";
  case FormatStyle::LK_Java:
    return "Java";
  case FormatStyle::LK_JavaScript:
    return "JavaScript";
  case FormatStyle::LK_Proto:
    return "Proto";
  default:
    return "Unknown";
  }
}

class Formatter : public UnwrappedLineConsumer {
public:
  Formatter(const FormatStyle &Style, SourceManager &SourceMgr, FileID ID,
            ArrayRef<CharSourceRange> Ranges)
      : Style(Style), ID(ID), SourceMgr(SourceMgr),
        Whitespaces(SourceMgr, Style,
                    inputUsesCRLF(SourceMgr.getBufferData(ID))),
        Ranges(Ranges.begin(), Ranges.end()), UnwrappedLines(1),
        Encoding(encoding::detectEncoding(SourceMgr.getBufferData(ID))) {
    DEBUG(llvm::dbgs() << "File encoding: "
                       << (Encoding == encoding::Encoding_UTF8 ? "UTF8"
                                                               : "unknown")
                       << "\n");
    DEBUG(llvm::dbgs() << "Language: " << getLanguageName(Style.Language)
                       << "\n");
  }

  tooling::Replacements format(bool *IncompleteFormat) {
    tooling::Replacements Result;
    FormatTokenLexer Tokens(SourceMgr, ID, Style, Encoding);

    UnwrappedLineParser Parser(Style, Tokens.getKeywords(), Tokens.lex(),
                               *this);
    Parser.parse();
    assert(UnwrappedLines.rbegin()->empty());
    for (unsigned Run = 0, RunE = UnwrappedLines.size(); Run + 1 != RunE;
         ++Run) {
      DEBUG(llvm::dbgs() << "Run " << Run << "...\n");
      SmallVector<AnnotatedLine *, 16> AnnotatedLines;
      for (unsigned i = 0, e = UnwrappedLines[Run].size(); i != e; ++i) {
        AnnotatedLines.push_back(new AnnotatedLine(UnwrappedLines[Run][i]));
      }
      tooling::Replacements RunResult =
          format(AnnotatedLines, Tokens, IncompleteFormat);
      DEBUG({
        llvm::dbgs() << "Replacements for run " << Run << ":\n";
        for (tooling::Replacements::iterator I = RunResult.begin(),
                                             E = RunResult.end();
             I != E; ++I) {
          llvm::dbgs() << I->toString() << "\n";
        }
      });
      for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
        delete AnnotatedLines[i];
      }
      Result.insert(RunResult.begin(), RunResult.end());
      Whitespaces.reset();
    }
    return Result;
  }

  tooling::Replacements format(SmallVectorImpl<AnnotatedLine *> &AnnotatedLines,
                               FormatTokenLexer &Tokens,
                               bool *IncompleteFormat) {
    TokenAnnotator Annotator(Style, Tokens.getKeywords());
    for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
      Annotator.annotate(*AnnotatedLines[i]);
    }
    deriveLocalStyle(AnnotatedLines);
    for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
      Annotator.calculateFormattingInformation(*AnnotatedLines[i]);
    }
    computeAffectedLines(AnnotatedLines.begin(), AnnotatedLines.end());

    Annotator.setCommentLineLevels(AnnotatedLines);
    ContinuationIndenter Indenter(Style, Tokens.getKeywords(), SourceMgr,
                                  Whitespaces, Encoding,
                                  BinPackInconclusiveFunctions);
    UnwrappedLineFormatter(&Indenter, &Whitespaces, Style, Tokens.getKeywords(),
                           IncompleteFormat)
        .format(AnnotatedLines);
    return Whitespaces.generateReplacements();
  }

private:
  // Determines which lines are affected by the SourceRanges given as input.
  // Returns \c true if at least one line between I and E or one of their
  // children is affected.
  bool computeAffectedLines(SmallVectorImpl<AnnotatedLine *>::iterator I,
                            SmallVectorImpl<AnnotatedLine *>::iterator E) {
    bool SomeLineAffected = false;
    const AnnotatedLine *PreviousLine = nullptr;
    while (I != E) {
      AnnotatedLine *Line = *I;
      Line->LeadingEmptyLinesAffected = affectsLeadingEmptyLines(*Line->First);

      // If a line is part of a preprocessor directive, it needs to be formatted
      // if any token within the directive is affected.
      if (Line->InPPDirective) {
        FormatToken *Last = Line->Last;
        SmallVectorImpl<AnnotatedLine *>::iterator PPEnd = I + 1;
        while (PPEnd != E && !(*PPEnd)->First->HasUnescapedNewline) {
          Last = (*PPEnd)->Last;
          ++PPEnd;
        }

        if (affectsTokenRange(*Line->First, *Last,
                              /*IncludeLeadingNewlines=*/false)) {
          SomeLineAffected = true;
          markAllAsAffected(I, PPEnd);
        }
        I = PPEnd;
        continue;
      }

      if (nonPPLineAffected(Line, PreviousLine))
        SomeLineAffected = true;

      PreviousLine = Line;
      ++I;
    }
    return SomeLineAffected;
  }

  // Determines whether 'Line' is affected by the SourceRanges given as input.
  // Returns \c true if line or one if its children is affected.
  bool nonPPLineAffected(AnnotatedLine *Line,
                         const AnnotatedLine *PreviousLine) {
    bool SomeLineAffected = false;
    Line->ChildrenAffected =
        computeAffectedLines(Line->Children.begin(), Line->Children.end());
    if (Line->ChildrenAffected)
      SomeLineAffected = true;

    // Stores whether one of the line's tokens is directly affected.
    bool SomeTokenAffected = false;
    // Stores whether we need to look at the leading newlines of the next token
    // in order to determine whether it was affected.
    bool IncludeLeadingNewlines = false;

    // Stores whether the first child line of any of this line's tokens is
    // affected.
    bool SomeFirstChildAffected = false;

    for (FormatToken *Tok = Line->First; Tok; Tok = Tok->Next) {
      // Determine whether 'Tok' was affected.
      if (affectsTokenRange(*Tok, *Tok, IncludeLeadingNewlines))
        SomeTokenAffected = true;

      // Determine whether the first child of 'Tok' was affected.
      if (!Tok->Children.empty() && Tok->Children.front()->Affected)
        SomeFirstChildAffected = true;

      IncludeLeadingNewlines = Tok->Children.empty();
    }

    // Was this line moved, i.e. has it previously been on the same line as an
    // affected line?
    bool LineMoved = PreviousLine && PreviousLine->Affected &&
                     Line->First->NewlinesBefore == 0;

    bool IsContinuedComment =
        Line->First->is(tok::comment) && Line->First->Next == nullptr &&
        Line->First->NewlinesBefore < 2 && PreviousLine &&
        PreviousLine->Affected && PreviousLine->Last->is(tok::comment);

    if (SomeTokenAffected || SomeFirstChildAffected || LineMoved ||
        IsContinuedComment) {
      Line->Affected = true;
      SomeLineAffected = true;
    }
    return SomeLineAffected;
  }

  // Marks all lines between I and E as well as all their children as affected.
  void markAllAsAffected(SmallVectorImpl<AnnotatedLine *>::iterator I,
                         SmallVectorImpl<AnnotatedLine *>::iterator E) {
    while (I != E) {
      (*I)->Affected = true;
      markAllAsAffected((*I)->Children.begin(), (*I)->Children.end());
      ++I;
    }
  }

  // Returns true if the range from 'First' to 'Last' intersects with one of the
  // input ranges.
  bool affectsTokenRange(const FormatToken &First, const FormatToken &Last,
                         bool IncludeLeadingNewlines) {
    SourceLocation Start = First.WhitespaceRange.getBegin();
    if (!IncludeLeadingNewlines)
      Start = Start.getLocWithOffset(First.LastNewlineOffset);
    SourceLocation End = Last.getStartOfNonWhitespace();
    End = End.getLocWithOffset(Last.TokenText.size());
    CharSourceRange Range = CharSourceRange::getCharRange(Start, End);
    return affectsCharSourceRange(Range);
  }

  // Returns true if one of the input ranges intersect the leading empty lines
  // before 'Tok'.
  bool affectsLeadingEmptyLines(const FormatToken &Tok) {
    CharSourceRange EmptyLineRange = CharSourceRange::getCharRange(
        Tok.WhitespaceRange.getBegin(),
        Tok.WhitespaceRange.getBegin().getLocWithOffset(Tok.LastNewlineOffset));
    return affectsCharSourceRange(EmptyLineRange);
  }

  // Returns true if 'Range' intersects with one of the input ranges.
  bool affectsCharSourceRange(const CharSourceRange &Range) {
    for (SmallVectorImpl<CharSourceRange>::const_iterator I = Ranges.begin(),
                                                          E = Ranges.end();
         I != E; ++I) {
      if (!SourceMgr.isBeforeInTranslationUnit(Range.getEnd(), I->getBegin()) &&
          !SourceMgr.isBeforeInTranslationUnit(I->getEnd(), Range.getBegin()))
        return true;
    }
    return false;
  }

  static bool inputUsesCRLF(StringRef Text) {
    return Text.count('\r') * 2 > Text.count('\n');
  }

  void
  deriveLocalStyle(const SmallVectorImpl<AnnotatedLine *> &AnnotatedLines) {
    unsigned CountBoundToVariable = 0;
    unsigned CountBoundToType = 0;
    bool HasCpp03IncompatibleFormat = false;
    bool HasBinPackedFunction = false;
    bool HasOnePerLineFunction = false;
    for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
      if (!AnnotatedLines[i]->First->Next)
        continue;
      FormatToken *Tok = AnnotatedLines[i]->First->Next;
      while (Tok->Next) {
        if (Tok->is(TT_PointerOrReference)) {
          bool SpacesBefore =
              Tok->WhitespaceRange.getBegin() != Tok->WhitespaceRange.getEnd();
          bool SpacesAfter = Tok->Next->WhitespaceRange.getBegin() !=
                             Tok->Next->WhitespaceRange.getEnd();
          if (SpacesBefore && !SpacesAfter)
            ++CountBoundToVariable;
          else if (!SpacesBefore && SpacesAfter)
            ++CountBoundToType;
        }

        if (Tok->WhitespaceRange.getBegin() == Tok->WhitespaceRange.getEnd()) {
          if (Tok->is(tok::coloncolon) && Tok->Previous->is(TT_TemplateOpener))
            HasCpp03IncompatibleFormat = true;
          if (Tok->is(TT_TemplateCloser) &&
              Tok->Previous->is(TT_TemplateCloser))
            HasCpp03IncompatibleFormat = true;
        }

        if (Tok->PackingKind == PPK_BinPacked)
          HasBinPackedFunction = true;
        if (Tok->PackingKind == PPK_OnePerLine)
          HasOnePerLineFunction = true;

        Tok = Tok->Next;
      }
    }
    if (Style.DerivePointerAlignment) {
      if (CountBoundToType > CountBoundToVariable)
        Style.PointerAlignment = FormatStyle::PAS_Left;
      else if (CountBoundToType < CountBoundToVariable)
        Style.PointerAlignment = FormatStyle::PAS_Right;
    }
    if (Style.Standard == FormatStyle::LS_Auto) {
      Style.Standard = HasCpp03IncompatibleFormat ? FormatStyle::LS_Cpp11
                                                  : FormatStyle::LS_Cpp03;
    }
    BinPackInconclusiveFunctions =
        HasBinPackedFunction || !HasOnePerLineFunction;
  }

  void consumeUnwrappedLine(const UnwrappedLine &TheLine) override {
    assert(!UnwrappedLines.empty());
    UnwrappedLines.back().push_back(TheLine);
  }

  void finishRun() override {
    UnwrappedLines.push_back(SmallVector<UnwrappedLine, 16>());
  }

  FormatStyle Style;
  FileID ID;
  SourceManager &SourceMgr;
  WhitespaceManager Whitespaces;
  SmallVector<CharSourceRange, 8> Ranges;
  SmallVector<SmallVector<UnwrappedLine, 16>, 2> UnwrappedLines;

  encoding::Encoding Encoding;
  bool BinPackInconclusiveFunctions;
};

} // end anonymous namespace

tooling::Replacements reformat(const FormatStyle &Style,
                               SourceManager &SourceMgr, FileID ID,
                               ArrayRef<CharSourceRange> Ranges,
                               bool *IncompleteFormat) {
  if (Style.DisableFormat)
    return tooling::Replacements();
  Formatter formatter(Style, SourceMgr, ID, Ranges);
  return formatter.format(IncompleteFormat);
}

tooling::Replacements reformat(const FormatStyle &Style, StringRef Code,
                               ArrayRef<tooling::Range> Ranges,
                               StringRef FileName, bool *IncompleteFormat) {
  if (Style.DisableFormat)
    return tooling::Replacements();

  FileManager Files((FileSystemOptions()));
  DiagnosticsEngine Diagnostics(
      IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs),
      new DiagnosticOptions);
  SourceManager SourceMgr(Diagnostics, Files);
  std::unique_ptr<llvm::MemoryBuffer> Buf =
      llvm::MemoryBuffer::getMemBuffer(Code, FileName);
  const clang::FileEntry *Entry =
      Files.getVirtualFile(FileName, Buf->getBufferSize(), 0);
  SourceMgr.overrideFileContents(Entry, std::move(Buf));
  FileID ID =
      SourceMgr.createFileID(Entry, SourceLocation(), clang::SrcMgr::C_User);
  SourceLocation StartOfFile = SourceMgr.getLocForStartOfFile(ID);
  std::vector<CharSourceRange> CharRanges;
  for (const tooling::Range &Range : Ranges) {
    SourceLocation Start = StartOfFile.getLocWithOffset(Range.getOffset());
    SourceLocation End = Start.getLocWithOffset(Range.getLength());
    CharRanges.push_back(CharSourceRange::getCharRange(Start, End));
  }
  return reformat(Style, SourceMgr, ID, CharRanges, IncompleteFormat);
}

LangOptions getFormattingLangOpts(const FormatStyle &Style) {
  LangOptions LangOpts;
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS
  LangOpts.CPlusPlus = 1;
  LangOpts.CPlusPlus11 = Style.Standard == FormatStyle::LS_Cpp03 ? 0 : 1;
  LangOpts.CPlusPlus14 = Style.Standard == FormatStyle::LS_Cpp03 ? 0 : 1;
  LangOpts.LineComment = 1;
  bool AlternativeOperators = Style.Language == FormatStyle::LK_Cpp;
  LangOpts.CXXOperatorNames = AlternativeOperators ? 1 : 0;
  LangOpts.Bool = 1;
  LangOpts.ObjC1 = 1;
  LangOpts.ObjC2 = 1;
  LangOpts.MicrosoftExt = 1; // To get kw___try, kw___finally.
#endif
  return LangOpts;
}

const char *StyleOptionHelpDescription =
    "Coding style, currently supports:\n"
    "  LLVM, Google, Chromium, Mozilla, WebKit.\n"
    "Use -style=file to load style configuration from\n"
    ".clang-format file located in one of the parent\n"
    "directories of the source file (or current\n"
    "directory for stdin).\n"
    "Use -style=\"{key: value, ...}\" to set specific\n"
    "parameters, e.g.:\n"
    "  -style=\"{BasedOnStyle: llvm, IndentWidth: 8}\"";

static FormatStyle::LanguageKind getLanguageByFileName(StringRef FileName) {
  if (FileName.endswith(".java")) {
    return FormatStyle::LK_Java;
  } else if (FileName.endswith_lower(".js") || FileName.endswith_lower(".ts")) {
    // JavaScript or TypeScript.
    return FormatStyle::LK_JavaScript;
  } else if (FileName.endswith_lower(".proto") ||
             FileName.endswith_lower(".protodevel")) {
    return FormatStyle::LK_Proto;
  }
  return FormatStyle::LK_Cpp;
}

FormatStyle getStyle(StringRef StyleName, StringRef FileName,
                     StringRef FallbackStyle) {
  FormatStyle Style = getLLVMStyle();
  Style.Language = getLanguageByFileName(FileName);
  if (!getPredefinedStyle(FallbackStyle, Style.Language, &Style)) {
    llvm::errs() << "Invalid fallback style \"" << FallbackStyle
                 << "\" using LLVM style\n";
    return Style;
  }

  if (StyleName.startswith("{")) {
    // Parse YAML/JSON style from the command line.
    if (std::error_code ec = parseConfiguration(StyleName, &Style)) {
      llvm::errs() << "Error parsing -style: " << ec.message() << ", using "
                   << FallbackStyle << " style\n";
    }
    return Style;
  }

  if (!StyleName.equals_lower("file")) {
    if (!getPredefinedStyle(StyleName, Style.Language, &Style))
      llvm::errs() << "Invalid value for -style, using " << FallbackStyle
                   << " style\n";
    return Style;
  }

  // Look for .clang-format/_clang-format file in the file's parent directories.
  SmallString<128> UnsuitableConfigFiles;
  SmallString<128> Path(FileName);
  llvm::sys::fs::make_absolute(Path);
  for (StringRef Directory = Path; !Directory.empty();
       Directory = llvm::sys::path::parent_path(Directory)) {
    if (!llvm::sys::fs::is_directory(Directory))
      continue;
    SmallString<128> ConfigFile(Directory);

    llvm::sys::path::append(ConfigFile, ".clang-format");
    DEBUG(llvm::dbgs() << "Trying " << ConfigFile << "...\n");
    bool IsFile = false;
    // Ignore errors from is_regular_file: we only need to know if we can read
    // the file or not.
    llvm::sys::fs::is_regular_file(Twine(ConfigFile), IsFile);

    if (!IsFile) {
      // Try _clang-format too, since dotfiles are not commonly used on Windows.
      ConfigFile = Directory;
      llvm::sys::path::append(ConfigFile, "_clang-format");
      DEBUG(llvm::dbgs() << "Trying " << ConfigFile << "...\n");
      llvm::sys::fs::is_regular_file(Twine(ConfigFile), IsFile);
    }

    if (IsFile) {
      llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> Text =
          llvm::MemoryBuffer::getFile(ConfigFile.c_str());
      if (std::error_code EC = Text.getError()) {
        llvm::errs() << EC.message() << "\n";
        break;
      }
      if (std::error_code ec =
              parseConfiguration(Text.get()->getBuffer(), &Style)) {
        if (ec == ParseError::Unsuitable) {
          if (!UnsuitableConfigFiles.empty())
            UnsuitableConfigFiles.append(", ");
          UnsuitableConfigFiles.append(ConfigFile);
          continue;
        }
        llvm::errs() << "Error reading " << ConfigFile << ": " << ec.message()
                     << "\n";
        break;
      }
      DEBUG(llvm::dbgs() << "Using configuration file " << ConfigFile << "\n");
      return Style;
    }
  }
  if (!UnsuitableConfigFiles.empty()) {
    llvm::errs() << "Configuration file(s) do(es) not support "
                 << getLanguageName(Style.Language) << ": "
                 << UnsuitableConfigFiles << "\n";
  }
  return Style;
}

} // namespace format
} // namespace clang
