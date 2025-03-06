//===----- Commit.h - A unit of edits ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_EDIT_COMMIT_H
#define LLVM_CLANG_EDIT_COMMIT_H

#include "clang/Edit/FileOffset.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"

namespace clang {
  class LangOptions;
  class PPConditionalDirectiveRecord;

namespace edit {
  class EditedSource;

class Commit {
public:
  enum EditKind {
    Act_Insert,
    Act_InsertFromRange,
    Act_Remove
  };

  struct Edit {
    EditKind Kind;
    StringRef Text;
    SourceLocation OrigLoc;
    FileOffset Offset;
    FileOffset InsertFromRangeOffs;
    unsigned Length;
    bool BeforePrev;

    SourceLocation getFileLocation(SourceManager &SM) const;
    CharSourceRange getFileRange(SourceManager &SM) const;
    CharSourceRange getInsertFromRange(SourceManager &SM) const;
  };

private:
  const SourceManager &SourceMgr;
  const LangOptions &LangOpts;
  const PPConditionalDirectiveRecord *PPRec;
  EditedSource *Editor;

  bool IsCommitable;
  SmallVector<Edit, 8> CachedEdits;
  
  llvm::BumpPtrAllocator StrAlloc;

public:
  explicit Commit(EditedSource &Editor);
  Commit(const SourceManager &SM, const LangOptions &LangOpts,
         const PPConditionalDirectiveRecord *PPRec = nullptr)
    : SourceMgr(SM), LangOpts(LangOpts), PPRec(PPRec), Editor(nullptr),
      IsCommitable(true) { }

  bool isCommitable() const { return IsCommitable; }

  bool insert(SourceLocation loc, StringRef text, bool afterToken = false,
              bool beforePreviousInsertions = false);
  bool insertAfterToken(SourceLocation loc, StringRef text,
                        bool beforePreviousInsertions = false) {
    return insert(loc, text, /*afterToken=*/true, beforePreviousInsertions);
  }
  bool insertBefore(SourceLocation loc, StringRef text) {
    return insert(loc, text, /*afterToken=*/false,
                  /*beforePreviousInsertions=*/true);
  }
  bool insertFromRange(SourceLocation loc, CharSourceRange range,
                       bool afterToken = false,
                       bool beforePreviousInsertions = false);
  bool insertWrap(StringRef before, CharSourceRange range, StringRef after);

  bool remove(CharSourceRange range);

  bool replace(CharSourceRange range, StringRef text);
  bool replaceWithInner(CharSourceRange range, CharSourceRange innerRange);
  bool replaceText(SourceLocation loc, StringRef text,
                   StringRef replacementText);

  bool insertFromRange(SourceLocation loc, SourceRange TokenRange,
                       bool afterToken = false,
                       bool beforePreviousInsertions = false) {
    return insertFromRange(loc, CharSourceRange::getTokenRange(TokenRange),
                           afterToken, beforePreviousInsertions);
  }
  bool insertWrap(StringRef before, SourceRange TokenRange, StringRef after) {
    return insertWrap(before, CharSourceRange::getTokenRange(TokenRange), after);
  }
  bool remove(SourceRange TokenRange) {
    return remove(CharSourceRange::getTokenRange(TokenRange));
  }
  bool replace(SourceRange TokenRange, StringRef text) {
    return replace(CharSourceRange::getTokenRange(TokenRange), text);
  }
  bool replaceWithInner(SourceRange TokenRange, SourceRange TokenInnerRange) {
    return replaceWithInner(CharSourceRange::getTokenRange(TokenRange),
                            CharSourceRange::getTokenRange(TokenInnerRange));
  }

  typedef SmallVectorImpl<Edit>::const_iterator edit_iterator;
  edit_iterator edit_begin() const { return CachedEdits.begin(); }
  edit_iterator edit_end() const { return CachedEdits.end(); }

private:
  void addInsert(SourceLocation OrigLoc,
                FileOffset Offs, StringRef text, bool beforePreviousInsertions);
  void addInsertFromRange(SourceLocation OrigLoc, FileOffset Offs,
                          FileOffset RangeOffs, unsigned RangeLen,
                          bool beforePreviousInsertions);
  void addRemove(SourceLocation OrigLoc, FileOffset Offs, unsigned Len);

  bool canInsert(SourceLocation loc, FileOffset &Offset);
  bool canInsertAfterToken(SourceLocation loc, FileOffset &Offset,
                           SourceLocation &AfterLoc);
  bool canInsertInOffset(SourceLocation OrigLoc, FileOffset Offs);
  bool canRemoveRange(CharSourceRange range, FileOffset &Offs, unsigned &Len);
  bool canReplaceText(SourceLocation loc, StringRef text,
                      FileOffset &Offs, unsigned &Len);

  void commitInsert(FileOffset offset, StringRef text,
                    bool beforePreviousInsertions);
  void commitRemove(FileOffset offset, unsigned length);

  bool isAtStartOfMacroExpansion(SourceLocation loc,
                                 SourceLocation *MacroBegin = nullptr) const;
  bool isAtEndOfMacroExpansion(SourceLocation loc,
                               SourceLocation *MacroEnd = nullptr) const;

  StringRef copyString(StringRef str) {
    char *buf = StrAlloc.Allocate<char>(str.size());
    std::memcpy(buf, str.data(), str.size());
    return StringRef(buf, str.size());
  }
};

}

} // end namespace clang

#endif
