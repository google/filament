//===--- TokenAnnotator.h - Format C++ code ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements a token annotator, i.e. creates
/// \c AnnotatedTokens out of \c FormatTokens with required extra information.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_FORMAT_TOKENANNOTATOR_H
#define LLVM_CLANG_LIB_FORMAT_TOKENANNOTATOR_H

#include "UnwrappedLineParser.h"
#include "clang/Format/Format.h"
#include <string>

namespace clang {
class SourceManager;

namespace format {

enum LineType {
  LT_Invalid,
  LT_ImportStatement,
  LT_ObjCDecl, // An @interface, @implementation, or @protocol line.
  LT_ObjCMethodDecl,
  LT_ObjCProperty, // An @property line.
  LT_Other,
  LT_PreprocessorDirective,
  LT_VirtualFunctionDecl
};

class AnnotatedLine {
public:
  AnnotatedLine(const UnwrappedLine &Line)
      : First(Line.Tokens.front().Tok), Level(Line.Level),
        InPPDirective(Line.InPPDirective),
        MustBeDeclaration(Line.MustBeDeclaration), MightBeFunctionDecl(false),
        IsMultiVariableDeclStmt(false), Affected(false),
        LeadingEmptyLinesAffected(false), ChildrenAffected(false) {
    assert(!Line.Tokens.empty());

    // Calculate Next and Previous for all tokens. Note that we must overwrite
    // Next and Previous for every token, as previous formatting runs might have
    // left them in a different state.
    First->Previous = nullptr;
    FormatToken *Current = First;
    for (std::list<UnwrappedLineNode>::const_iterator I = ++Line.Tokens.begin(),
                                                      E = Line.Tokens.end();
         I != E; ++I) {
      const UnwrappedLineNode &Node = *I;
      Current->Next = I->Tok;
      I->Tok->Previous = Current;
      Current = Current->Next;
      Current->Children.clear();
      for (const auto &Child : Node.Children) {
        Children.push_back(new AnnotatedLine(Child));
        Current->Children.push_back(Children.back());
      }
    }
    Last = Current;
    Last->Next = nullptr;
  }

  ~AnnotatedLine() {
    for (unsigned i = 0, e = Children.size(); i != e; ++i) {
      delete Children[i];
    }
    FormatToken *Current = First;
    while (Current) {
      Current->Children.clear();
      Current->Role.reset();
      Current = Current->Next;
    }
  }

  /// \c true if this line starts with the given tokens in order, ignoring
  /// comments.
  template <typename... Ts> bool startsWith(Ts... Tokens) const {
    return startsWith(First, Tokens...);
  }

  FormatToken *First;
  FormatToken *Last;

  SmallVector<AnnotatedLine *, 0> Children;

  LineType Type;
  unsigned Level;
  bool InPPDirective;
  bool MustBeDeclaration;
  bool MightBeFunctionDecl;
  bool IsMultiVariableDeclStmt;

  /// \c True if this line should be formatted, i.e. intersects directly or
  /// indirectly with one of the input ranges.
  bool Affected;

  /// \c True if the leading empty lines of this line intersect with one of the
  /// input ranges.
  bool LeadingEmptyLinesAffected;

  /// \c True if a one of this line's children intersects with an input range.
  bool ChildrenAffected;

private:
  // Disallow copying.
  AnnotatedLine(const AnnotatedLine &) = delete;
  void operator=(const AnnotatedLine &) = delete;

  template <typename A, typename... Ts>
  bool startsWith(FormatToken *Tok, A K1) const {
    while (Tok && Tok->is(tok::comment))
      Tok = Tok->Next;
    return Tok && Tok->is(K1);
  }

  template <typename A, typename... Ts>
  bool startsWith(FormatToken *Tok, A K1, Ts... Tokens) const {
    return startsWith(Tok, K1) && startsWith(Tok->Next, Tokens...);
  }
};

/// \brief Determines extra information about the tokens comprising an
/// \c UnwrappedLine.
class TokenAnnotator {
public:
  TokenAnnotator(const FormatStyle &Style, const AdditionalKeywords &Keywords)
      : Style(Style), Keywords(Keywords) {}

  /// \brief Adapts the indent levels of comment lines to the indent of the
  /// subsequent line.
  // FIXME: Can/should this be done in the UnwrappedLineParser?
  void setCommentLineLevels(SmallVectorImpl<AnnotatedLine *> &Lines);

  void annotate(AnnotatedLine &Line);
  void calculateFormattingInformation(AnnotatedLine &Line);

private:
  /// \brief Calculate the penalty for splitting before \c Tok.
  unsigned splitPenalty(const AnnotatedLine &Line, const FormatToken &Tok,
                        bool InFunctionDecl);

  bool spaceRequiredBetween(const AnnotatedLine &Line, const FormatToken &Left,
                            const FormatToken &Right);

  bool spaceRequiredBefore(const AnnotatedLine &Line, const FormatToken &Tok);

  bool mustBreakBefore(const AnnotatedLine &Line, const FormatToken &Right);

  bool canBreakBefore(const AnnotatedLine &Line, const FormatToken &Right);

  void printDebugInfo(const AnnotatedLine &Line);

  void calculateUnbreakableTailLengths(AnnotatedLine &Line);

  const FormatStyle &Style;

  const AdditionalKeywords &Keywords;
};

} // end namespace format
} // end namespace clang

#endif
