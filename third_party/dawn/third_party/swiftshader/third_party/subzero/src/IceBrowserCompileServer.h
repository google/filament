//===- subzero/src/IceBrowserCompileServer.h - Browser server ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the browser-based compile server.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEBROWSERCOMPILESERVER_H
#define SUBZERO_SRC_ICEBROWSERCOMPILESERVER_H

#include "IceClFlags.h"
#include "IceCompileServer.h"
#include "IceDefs.h"
#include "IceELFStreamer.h"

#include <atomic>
#include <thread>

namespace llvm {
class QueueStreamer;
class raw_fd_ostream;
} // end of namespace llvm

namespace Ice {

/// The browser variant of the compile server. Compared to the commandline
/// version, this version gets compile requests over IPC. Each compile request
/// will have a slimmed down version of argc, argv while other flags are set to
/// defaults that make sense in the browser case. The output file is specified
/// via a posix FD, and input bytes are pushed to the server.
class BrowserCompileServer : public CompileServer {
  BrowserCompileServer(const BrowserCompileServer &) = delete;
  BrowserCompileServer &operator=(const BrowserCompileServer &) = delete;
  class StringStream;

public:
  BrowserCompileServer() : HadError(false) {}

  ~BrowserCompileServer() final;

  void run() final;

  ErrorCode &getErrorCode() final;

  /// Parse and set up the flags for compile jobs.
  void getParsedFlags(bool UseNumThreadsFromBrowser, uint32_t NumThreads,
                      int argc, const char *const *argv);

  /// Creates the streams + context and starts the compile thread, handing off
  /// the streams + context.
  void startCompileThread(int OutFD);

  /// Call to push more bytes to the current input stream. Returns false on
  /// success and true on error.
  bool pushInputBytes(const void *Data, size_t NumBytes);

  /// Notify the input stream of EOF.
  void endInputStream();

  /// Wait for the compile thread to complete then reset the state.
  void waitForCompileThread() {
    CompileThread.join();
    if (Ctx->getErrorStatus()->value())
      LastError.assign(Ctx->getErrorStatus()->value());
    // Reset some state. The InputStream is deleted by the compiler so only
    // reset this to nullptr. Free and flush the rest of the streams.
    InputStream = nullptr;
    EmitStream.reset(nullptr);
    ELFStream.reset(nullptr);
  }

  StringStream &getErrorStream() { return *ErrorStream; }

  void setFatalError(const std::string &Reason);

private:
  class StringStream {
  public:
    StringStream() : StrBuf(Buffer) {}
    const std::string &getContents() { return StrBuf.str(); }
    Ostream &getStream() { return StrBuf; }

  private:
    std::string Buffer;
    llvm::raw_string_ostream StrBuf;
  };
  /// This currently only handles a single compile request, hence one copy of
  /// the state.
  std::unique_ptr<GlobalContext> Ctx;
  /// A borrowed reference to the current InputStream. The compiler owns the
  /// actual reference so the server must be careful not to access after the
  /// compiler is done.
  llvm::QueueStreamer *InputStream = nullptr;
  std::unique_ptr<Ostream> LogStream;
  std::unique_ptr<llvm::raw_fd_ostream> EmitStream;
  std::unique_ptr<StringStream> ErrorStream;
  std::unique_ptr<ELFStreamer> ELFStream;
  std::thread CompileThread;
  std::atomic<bool> HadError;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEBROWSERCOMPILESERVER_H
