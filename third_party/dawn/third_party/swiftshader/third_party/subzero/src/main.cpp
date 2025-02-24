//===- subzero/src/main.cpp - Entry point for bitcode translation ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the entry point for translating PNaCl bitcode into native
/// code.
///
//===----------------------------------------------------------------------===//

#include "IceBrowserCompileServer.h"
#include "IceBuildDefs.h"
#include "IceCompileServer.h"

#ifdef __pnacl__
#include <malloc.h>
#endif // __pnacl__

/// Depending on whether we are building the compiler for the browser or
/// standalone, we will end up creating a Ice::BrowserCompileServer or
/// Ice::CLCompileServer object. Method
/// Ice::CompileServer::runAndReturnErrorCode is used for the invocation.
/// There are no real commandline arguments in the browser case. They are
/// supplied via IPC so argc, and argv are not used in that case.
/// We can only compile the Ice::BrowserCompileServer object with the PNaCl
/// compiler toolchain, when building Subzero as a sandboxed translator.
int main(int argc, char **argv) {
#ifdef __pnacl__
#define M_GRANULARITY (-2)
  // PNaCl's default malloc implementation grabs small chunks of memory with
  // mmap at a time, hence causing significant slowdowns. This call ensures that
  // mmap is used to allocate 16MB at a time, to amortize the system call cost.
  mallopt(M_GRANULARITY, 16 * 1024 * 1024);
#undef M_GRANULARITY
#endif // __pnacl__

  if (Ice::BuildDefs::browser()) {
    assert(argc == 1);
    return Ice::BrowserCompileServer().runAndReturnErrorCode();
  }
  return Ice::CLCompileServer(argc, argv).runAndReturnErrorCode();
}
