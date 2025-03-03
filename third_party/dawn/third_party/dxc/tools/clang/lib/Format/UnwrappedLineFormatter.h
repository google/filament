//===--- UnwrappedLineFormatter.h - Format C++ code -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements a combinartorial exploration of all the different
/// linebreaks unwrapped lines can be formatted in.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_FORMAT_UNWRAPPEDLINEFORMATTER_H
#define LLVM_CLANG_LIB_FORMAT_UNWRAPPEDLINEFORMATTER_H

#include "ContinuationIndenter.h"
#include "clang/Format/Format.h"
#include <map>
#include <queue>
#include <string>

namespace clang {
namespace format {

class ContinuationIndenter;
class WhitespaceManager;

class UnwrappedLineFormatter {
public:
  UnwrappedLineFormatter(ContinuationIndenter *Indenter,
                         WhitespaceManager *Whitespaces,
                         const FormatStyle &Style,
                         const AdditionalKeywords &Keywords,
                         bool *IncompleteFormat)
      : Indenter(Indenter), Whitespaces(Whitespaces), Style(Style),
        Keywords(Keywords), IncompleteFormat(IncompleteFormat) {}

  /// \brief Format the current block and return the penalty.
  unsigned format(const SmallVectorImpl<AnnotatedLine *> &Lines,
                  bool DryRun = false, int AdditionalIndent = 0,
                  bool FixBadIndentation = false);

private:
  /// \brief Add a new line and the required indent before the first Token
  /// of the \c UnwrappedLine if there was no structural parsing error.
  void formatFirstToken(FormatToken &RootToken,
                        const AnnotatedLine *PreviousLine, unsigned IndentLevel,
                        unsigned Indent, bool InPPDirective);

  /// \brief Returns the column limit for a line, taking into account whether we
  /// need an escaped newline due to a continued preprocessor directive.
  unsigned getColumnLimit(bool InPPDirective,
                          const AnnotatedLine *NextLine) const;

  // Cache to store the penalty of formatting a vector of AnnotatedLines
  // starting from a specific additional offset. Improves performance if there
  // are many nested blocks.
  std::map<std::pair<const SmallVectorImpl<AnnotatedLine *> *, unsigned>,
           unsigned> PenaltyCache;

  ContinuationIndenter *Indenter;
  WhitespaceManager *Whitespaces;
  const FormatStyle &Style;
  const AdditionalKeywords &Keywords;
  bool *IncompleteFormat;
};
} // end namespace format
} // end namespace clang

#endif // LLVM_CLANG_LIB_FORMAT_UNWRAPPEDLINEFORMATTER_H
