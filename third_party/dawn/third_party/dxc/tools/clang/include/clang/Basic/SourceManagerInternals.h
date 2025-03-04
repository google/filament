//===--- SourceManagerInternals.h - SourceManager Internals -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines implementation details of the clang::SourceManager class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_SOURCEMANAGERINTERNALS_H
#define LLVM_CLANG_BASIC_SOURCEMANAGERINTERNALS_H

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringMap.h"
#include <map>

namespace clang {

//===----------------------------------------------------------------------===//
// Line Table Implementation
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

struct LineEntry {
  /// \brief The offset in this file that the line entry occurs at.
  unsigned FileOffset;

  /// \brief The presumed line number of this line entry: \#line 4.
  unsigned LineNo;

  /// \brief The ID of the filename identified by this line entry:
  /// \#line 4 "foo.c".  This is -1 if not specified.
  int FilenameID;

  /// \brief Set the 0 if no flags, 1 if a system header,
  SrcMgr::CharacteristicKind FileKind;

  /// \brief The offset of the virtual include stack location,
  /// which is manipulated by GNU linemarker directives.
  ///
  /// If this is 0 then there is no virtual \#includer.
  unsigned IncludeOffset;

  static LineEntry get(unsigned Offs, unsigned Line, int Filename,
                       SrcMgr::CharacteristicKind FileKind,
                       unsigned IncludeOffset) {
    LineEntry E;
    E.FileOffset = Offs;
    E.LineNo = Line;
    E.FilenameID = Filename;
    E.FileKind = FileKind;
    E.IncludeOffset = IncludeOffset;
    return E;
  }
};

// needed for FindNearestLineEntry (upper_bound of LineEntry)
inline bool operator<(const LineEntry &lhs, const LineEntry &rhs) {
  // FIXME: should check the other field?
  return lhs.FileOffset < rhs.FileOffset;
}

inline bool operator<(const LineEntry &E, unsigned Offset) {
  return E.FileOffset < Offset;
}

inline bool operator<(unsigned Offset, const LineEntry &E) {
  return Offset < E.FileOffset;
}

/// \brief Used to hold and unique data used to represent \#line information.
class LineTableInfo {
  /// \brief Map used to assign unique IDs to filenames in \#line directives. 
  ///
  /// This allows us to unique the filenames that
  /// frequently reoccur and reference them with indices.  FilenameIDs holds
  /// the mapping from string -> ID, and FilenamesByID holds the mapping of ID
  /// to string.
  llvm::StringMap<unsigned, llvm::BumpPtrAllocator> FilenameIDs;
  std::vector<llvm::StringMapEntry<unsigned>*> FilenamesByID;

  /// \brief Map from FileIDs to a list of line entries (sorted by the offset
  /// at which they occur in the file).
  std::map<FileID, std::vector<LineEntry> > LineEntries;
public:
  void clear() {
    FilenameIDs.clear();
    FilenamesByID.clear();
    LineEntries.clear();
  }

  unsigned getLineTableFilenameID(StringRef Str);
  const char *getFilename(unsigned ID) const {
    assert(ID < FilenamesByID.size() && "Invalid FilenameID");
    return FilenamesByID[ID]->getKeyData();
  }
  unsigned getNumFilenames() const { return FilenamesByID.size(); }

  void AddLineNote(FileID FID, unsigned Offset,
                   unsigned LineNo, int FilenameID);
  void AddLineNote(FileID FID, unsigned Offset,
                   unsigned LineNo, int FilenameID,
                   unsigned EntryExit, SrcMgr::CharacteristicKind FileKind);


  /// \brief Find the line entry nearest to FID that is before it.
  ///
  /// If there is no line entry before \p Offset in \p FID, returns null.
  const LineEntry *FindNearestLineEntry(FileID FID, unsigned Offset);

  // Low-level access
  typedef std::map<FileID, std::vector<LineEntry> >::iterator iterator;
  iterator begin() { return LineEntries.begin(); }
  iterator end() { return LineEntries.end(); }

  /// \brief Add a new line entry that has already been encoded into
  /// the internal representation of the line table.
  void AddEntry(FileID FID, const std::vector<LineEntry> &Entries);
};

} // end namespace clang

#endif
