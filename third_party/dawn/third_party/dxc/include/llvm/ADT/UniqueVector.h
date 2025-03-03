//===-- llvm/ADT/UniqueVector.h ---------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_ADT_UNIQUEVECTOR_H
#define LLVM_ADT_UNIQUEVECTOR_H

#include <cassert>
#include <map>
#include <vector>

namespace llvm {
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
/// UniqueVector - This class produces a sequential ID number (base 1) for each
/// unique entry that is added.  T is the type of entries in the vector. This
/// class should have an implementation of operator== and of operator<.
/// Entries can be fetched using operator[] with the entry ID.
template<class T> class UniqueVector {
public:
  typedef typename std::vector<T> VectorType;
  typedef typename VectorType::iterator iterator;
  typedef typename VectorType::const_iterator const_iterator;

private:
  // Map - Used to handle the correspondence of entry to ID.
  std::map<T, unsigned> Map;

  // Vector - ID ordered vector of entries. Entries can be indexed by ID - 1.
  //
  VectorType Vector;

public:
  /// insert - Append entry to the vector if it doesn't already exist.  Returns
  /// the entry's index + 1 to be used as a unique ID.
  unsigned insert(const T &Entry) {
    // Check if the entry is already in the map.
    unsigned &Val = Map[Entry];

    // See if entry exists, if so return prior ID.
    if (Val) return Val;

    // Compute ID for entry.
    Val = static_cast<unsigned>(Vector.size()) + 1;

    // Insert in vector.
    Vector.push_back(Entry);
    return Val;
  }

  /// idFor - return the ID for an existing entry.  Returns 0 if the entry is
  /// not found.
  unsigned idFor(const T &Entry) const {
    // Search for entry in the map.
    typename std::map<T, unsigned>::const_iterator MI = Map.find(Entry);

    // See if entry exists, if so return ID.
    if (MI != Map.end()) return MI->second;

    // No luck.
    return 0;
  }

  /// operator[] - Returns a reference to the entry with the specified ID.
  ///
  const T &operator[](unsigned ID) const {
    assert(ID-1 < size() && "ID is 0 or out of range!");
    return Vector[ID - 1];
  }

  /// \brief Return an iterator to the start of the vector.
  iterator begin() { return Vector.begin(); }

  /// \brief Return an iterator to the start of the vector.
  const_iterator begin() const { return Vector.begin(); }

  /// \brief Return an iterator to the end of the vector.
  iterator end() { return Vector.end(); }

  /// \brief Return an iterator to the end of the vector.
  const_iterator end() const { return Vector.end(); }

  /// size - Returns the number of entries in the vector.
  ///
  size_t size() const { return Vector.size(); }

  /// empty - Returns true if the vector is empty.
  ///
  bool empty() const { return Vector.empty(); }

  /// reset - Clears all the entries.
  ///
  void reset() {
    Map.clear();
    Vector.resize(0, 0);
  }
};

} // End of namespace llvm

#endif // LLVM_ADT_UNIQUEVECTOR_H
