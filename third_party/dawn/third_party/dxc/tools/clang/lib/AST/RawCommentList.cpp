//===--- RawCommentList.cpp - Processing raw comments -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/RawCommentList.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "clang/AST/CommentBriefParser.h"
#include "clang/AST/CommentCommandTraits.h"
#include "clang/AST/CommentLexer.h"
#include "clang/AST/CommentParser.h"
#include "clang/AST/CommentSema.h"
#include "llvm/ADT/STLExtras.h"

using namespace clang;

namespace {
/// Get comment kind and bool describing if it is a trailing comment.
std::pair<RawComment::CommentKind, bool> getCommentKind(StringRef Comment,
                                                        bool ParseAllComments) {
  const size_t MinCommentLength = ParseAllComments ? 2 : 3;
  if ((Comment.size() < MinCommentLength) || Comment[0] != '/')
    return std::make_pair(RawComment::RCK_Invalid, false);

  RawComment::CommentKind K;
  if (Comment[1] == '/') {
    if (Comment.size() < 3)
      return std::make_pair(RawComment::RCK_OrdinaryBCPL, false);

    if (Comment[2] == '/')
      K = RawComment::RCK_BCPLSlash;
    else if (Comment[2] == '!')
      K = RawComment::RCK_BCPLExcl;
    else
      return std::make_pair(RawComment::RCK_OrdinaryBCPL, false);
  } else {
    assert(Comment.size() >= 4);

    // Comment lexer does not understand escapes in comment markers, so pretend
    // that this is not a comment.
    if (Comment[1] != '*' ||
        Comment[Comment.size() - 2] != '*' ||
        Comment[Comment.size() - 1] != '/')
      return std::make_pair(RawComment::RCK_Invalid, false);

    if (Comment[2] == '*')
      K = RawComment::RCK_JavaDoc;
    else if (Comment[2] == '!')
      K = RawComment::RCK_Qt;
    else
      return std::make_pair(RawComment::RCK_OrdinaryC, false);
  }
  const bool TrailingComment = (Comment.size() > 3) && (Comment[3] == '<');
  return std::make_pair(K, TrailingComment);
}

bool mergedCommentIsTrailingComment(StringRef Comment) {
  return (Comment.size() > 3) && (Comment[3] == '<');
}
} // unnamed namespace

RawComment::RawComment(const SourceManager &SourceMgr, SourceRange SR,
                       bool Merged, bool ParseAllComments) :
    Range(SR), RawTextValid(false), BriefTextValid(false),
    IsAttached(false), IsAlmostTrailingComment(false),
    ParseAllComments(ParseAllComments) {
  // Extract raw comment text, if possible.
  if (SR.getBegin() == SR.getEnd() || getRawText(SourceMgr).empty()) {
    Kind = RCK_Invalid;
    return;
  }

  if (!Merged) {
    // Guess comment kind.
    std::pair<CommentKind, bool> K = getCommentKind(RawText, ParseAllComments);
    Kind = K.first;
    IsTrailingComment = K.second;

    IsAlmostTrailingComment = RawText.startswith("//<") ||
                                 RawText.startswith("/*<");
  } else {
    Kind = RCK_Merged;
    IsTrailingComment = mergedCommentIsTrailingComment(RawText);
  }
}

StringRef RawComment::getRawTextSlow(const SourceManager &SourceMgr) const {
  FileID BeginFileID;
  FileID EndFileID;
  unsigned BeginOffset;
  unsigned EndOffset;

  std::tie(BeginFileID, BeginOffset) =
      SourceMgr.getDecomposedLoc(Range.getBegin());
  std::tie(EndFileID, EndOffset) = SourceMgr.getDecomposedLoc(Range.getEnd());

  const unsigned Length = EndOffset - BeginOffset;
  if (Length < 2)
    return StringRef();

  // The comment can't begin in one file and end in another.
  assert(BeginFileID == EndFileID);

  bool Invalid = false;
  const char *BufferStart = SourceMgr.getBufferData(BeginFileID,
                                                    &Invalid).data();
  if (Invalid)
    return StringRef();

  return StringRef(BufferStart + BeginOffset, Length);
}

const char *RawComment::extractBriefText(const ASTContext &Context) const {
  // Make sure that RawText is valid.
  getRawText(Context.getSourceManager());

  // Since we will be copying the resulting text, all allocations made during
  // parsing are garbage after resulting string is formed.  Thus we can use
  // a separate allocator for all temporary stuff.
  llvm::BumpPtrAllocator Allocator;

  comments::Lexer L(Allocator, Context.getDiagnostics(),
                    Context.getCommentCommandTraits(),
                    Range.getBegin(),
                    RawText.begin(), RawText.end());
  comments::BriefParser P(L, Context.getCommentCommandTraits());

  const std::string Result = P.Parse();
  const unsigned BriefTextLength = Result.size();
  char *BriefTextPtr = new (Context) char[BriefTextLength + 1];
  memcpy(BriefTextPtr, Result.c_str(), BriefTextLength + 1);
  BriefText = BriefTextPtr;
  BriefTextValid = true;

  return BriefTextPtr;
}

comments::FullComment *RawComment::parse(const ASTContext &Context,
                                         const Preprocessor *PP,
                                         const Decl *D) const {
  // Make sure that RawText is valid.
  getRawText(Context.getSourceManager());

  comments::Lexer L(Context.getAllocator(), Context.getDiagnostics(),
                    Context.getCommentCommandTraits(),
                    getSourceRange().getBegin(),
                    RawText.begin(), RawText.end());
  comments::Sema S(Context.getAllocator(), Context.getSourceManager(),
                   Context.getDiagnostics(),
                   Context.getCommentCommandTraits(),
                   PP);
  S.setDecl(D);
  comments::Parser P(L, S, Context.getAllocator(), Context.getSourceManager(),
                     Context.getDiagnostics(),
                     Context.getCommentCommandTraits());

  return P.parseFullComment();
}

static bool onlyWhitespaceBetween(SourceManager &SM,
                                  SourceLocation Loc1, SourceLocation Loc2,
                                  unsigned MaxNewlinesAllowed) {
  std::pair<FileID, unsigned> Loc1Info = SM.getDecomposedLoc(Loc1);
  std::pair<FileID, unsigned> Loc2Info = SM.getDecomposedLoc(Loc2);

  // Question does not make sense if locations are in different files.
  if (Loc1Info.first != Loc2Info.first)
    return false;

  bool Invalid = false;
  const char *Buffer = SM.getBufferData(Loc1Info.first, &Invalid).data();
  if (Invalid)
    return false;

  unsigned NumNewlines = 0;
  assert(Loc1Info.second <= Loc2Info.second && "Loc1 after Loc2!");
  // Look for non-whitespace characters and remember any newlines seen.
  for (unsigned I = Loc1Info.second; I != Loc2Info.second; ++I) {
    switch (Buffer[I]) {
    default:
      return false;
    case ' ':
    case '\t':
    case '\f':
    case '\v':
      break;
    case '\r':
    case '\n':
      ++NumNewlines;

      // Check if we have found more than the maximum allowed number of
      // newlines.
      if (NumNewlines > MaxNewlinesAllowed)
        return false;

      // Collapse \r\n and \n\r into a single newline.
      if (I + 1 != Loc2Info.second &&
          (Buffer[I + 1] == '\n' || Buffer[I + 1] == '\r') &&
          Buffer[I] != Buffer[I + 1])
        ++I;
      break;
    }
  }

  return true;
}

void RawCommentList::addComment(const RawComment &RC,
                                llvm::BumpPtrAllocator &Allocator) {
  if (RC.isInvalid())
    return;

  // Check if the comments are not in source order.
  while (!Comments.empty() &&
         !SourceMgr.isBeforeInTranslationUnit(Comments.back()->getLocStart(),
                                              RC.getLocStart())) {
    // If they are, just pop a few last comments that don't fit.
    // This happens if an \#include directive contains comments.
    Comments.pop_back();
  }

  // Ordinary comments are not interesting for us.
  if (RC.isOrdinary())
    return;

  // If this is the first Doxygen comment, save it (because there isn't
  // anything to merge it with).
  if (Comments.empty()) {
    Comments.push_back(new (Allocator) RawComment(RC));
    return;
  }

  const RawComment &C1 = *Comments.back();
  const RawComment &C2 = RC;

  // Merge comments only if there is only whitespace between them.
  // Can't merge trailing and non-trailing comments.
  // Merge comments if they are on same or consecutive lines.
  if (C1.isTrailingComment() == C2.isTrailingComment() &&
      onlyWhitespaceBetween(SourceMgr, C1.getLocEnd(), C2.getLocStart(),
                            /*MaxNewlinesAllowed=*/1)) {
    SourceRange MergedRange(C1.getLocStart(), C2.getLocEnd());
    *Comments.back() = RawComment(SourceMgr, MergedRange, true,
                                  RC.isParseAllComments());
  } else {
    Comments.push_back(new (Allocator) RawComment(RC));
  }
}

void RawCommentList::addDeserializedComments(ArrayRef<RawComment *> DeserializedComments) {
  std::vector<RawComment *> MergedComments;
  MergedComments.reserve(Comments.size() + DeserializedComments.size());

  std::merge(Comments.begin(), Comments.end(),
             DeserializedComments.begin(), DeserializedComments.end(),
             std::back_inserter(MergedComments),
             BeforeThanCompare<RawComment>(SourceMgr));
  std::swap(Comments, MergedComments);
}

