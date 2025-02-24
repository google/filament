//===- subzero/src/IceRevision.h - Revision string embedding ----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the function for returning the Subzero revision string.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREVISION_H
#define SUBZERO_SRC_ICEREVISION_H

namespace Ice {

// Returns the Subzero revision string, which is meant to be essentially the git
// hash of the repo when Subzero was built.
//
// Note: It would be possible to declare this a constexpr char[] and put its
// definition right here in the include file.  But since the git hash is passed
// to the compiler on the command line, and compilation is directed through a
// Makefile, lack of recompilation could lead to different files seeing
// inconsistent revision strings.
const char *getSubzeroRevision();

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREVISION_H
