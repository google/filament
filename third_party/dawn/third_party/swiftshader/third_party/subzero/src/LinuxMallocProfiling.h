//===--- subzero/src/LinuxMallocProfiling.h - malloc/new tracing  ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief malloc/new/...caller tracing.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_LINUXMALLOCPROFILING_H
#define SUBZERO_SRC_LINUXMALLOCPROFILING_H

#include "IceDefs.h"

namespace Ice {

class LinuxMallocProfiling {
private:
  LinuxMallocProfiling(const LinuxMallocProfiling &) = delete;
  LinuxMallocProfiling &operator=(const LinuxMallocProfiling &) = delete;

#ifdef ALLOW_LINUX_MALLOC_PROFILE
  Ostream *Ls;
#endif // ALLOW_LINUX_MALLOC_PROFILE

public:
  LinuxMallocProfiling(size_t NumThreads, Ostream *Ls);
  ~LinuxMallocProfiling();
};

} // end of namespace Ice

#endif // SUBZERO_SRC_LINUXMALLOCPROFILING_H
