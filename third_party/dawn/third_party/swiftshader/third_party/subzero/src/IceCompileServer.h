//===- subzero/src/IceCompileServer.h - Compile server ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the compile server.
///
/// Given a compiler implementation, it dispatches compile requests to the
/// implementation.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECOMPILESERVER_H
#define SUBZERO_SRC_ICECOMPILESERVER_H

#include "IceCompiler.h"
#include "IceDefs.h"
#include "IceGlobalContext.h"

namespace llvm {
class DataStreamer;
class raw_fd_ostream;
} // namespace llvm

namespace Ice {

/// A CompileServer awaits compile requests, and dispatches the requests to a
/// given Compiler. Each request is paired with an input stream, a context
/// (which has the output stream), and a set of arguments. The CompileServer
/// takes over the current thread to listen to requests, and compile requests
/// are handled on separate threads.
///
/// Currently, this only handles a single request.
///
/// When run on the commandline, it receives and therefore dispatches the
/// request immediately. When run in the browser, it blocks waiting for a
/// request.
class CompileServer {
  CompileServer(const CompileServer &) = delete;
  CompileServer &operator=(const CompileServer &) = delete;

public:
  CompileServer() = default;

  virtual ~CompileServer() = default;

  virtual void run() = 0;

  virtual ErrorCode &getErrorCode() { return LastError; }
  void transferErrorCode(ErrorCodes Code) { LastError.assign(Code); }

  int runAndReturnErrorCode() {
    run();
    return getErrorCode().value();
  }

protected:
  Compiler &getCompiler() { return Comp; }

  Compiler Comp;
  ErrorCode LastError;
};

/// Commandline variant of the compile server.
class CLCompileServer : public CompileServer {
  CLCompileServer() = delete;
  CLCompileServer(const CLCompileServer &) = delete;
  CLCompileServer &operator=(const CLCompileServer &) = delete;

public:
  CLCompileServer(int argc, char **argv) : argc(argc), argv(argv) {}

  ~CLCompileServer() final = default;

  void run() final;

private:
  int argc;
  char **argv;
  std::unique_ptr<GlobalContext> Ctx;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECOMPILESERVER_H
