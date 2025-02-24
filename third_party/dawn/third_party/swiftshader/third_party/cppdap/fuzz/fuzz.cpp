// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// cppdap fuzzer program.
// Run with: ${CPPDAP_PATH}/fuzz/run.sh
// Requires modern clang toolchain.

#include "content_stream.h"
#include "string_buffer.h"

#include "dap/protocol.h"
#include "dap/session.h"

#include <condition_variable>
#include <mutex>

namespace {

// Event provides a basic wait and signal synchronization primitive.
class Event {
 public:
  // wait() blocks until the event is fired or the given timeout is reached.
  template <typename DURATION>
  inline void wait(const DURATION& duration) {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, duration, [&] { return fired; });
  }

  // fire() sets signals the event, and unblocks any calls to wait().
  inline void fire() {
    std::unique_lock<std::mutex> lock(mutex);
    fired = true;
    cv.notify_all();
  }

 private:
  std::mutex mutex;
  std::condition_variable cv;
  bool fired = false;
};

}  // namespace

// List of requests that we handle for fuzzing.
#define DAP_REQUEST_LIST()                                                     \
  DAP_REQUEST(dap::AttachRequest, dap::AttachResponse)                         \
  DAP_REQUEST(dap::BreakpointLocationsRequest,                                 \
              dap::BreakpointLocationsResponse)                                \
  DAP_REQUEST(dap::CancelRequest, dap::CancelResponse)                         \
  DAP_REQUEST(dap::CompletionsRequest, dap::CompletionsResponse)               \
  DAP_REQUEST(dap::ConfigurationDoneRequest, dap::ConfigurationDoneResponse)   \
  DAP_REQUEST(dap::ContinueRequest, dap::ContinueResponse)                     \
  DAP_REQUEST(dap::DataBreakpointInfoRequest, dap::DataBreakpointInfoResponse) \
  DAP_REQUEST(dap::DisassembleRequest, dap::DisassembleResponse)               \
  DAP_REQUEST(dap::DisconnectRequest, dap::DisconnectResponse)                 \
  DAP_REQUEST(dap::EvaluateRequest, dap::EvaluateResponse)                     \
  DAP_REQUEST(dap::ExceptionInfoRequest, dap::ExceptionInfoResponse)           \
  DAP_REQUEST(dap::GotoRequest, dap::GotoResponse)                             \
  DAP_REQUEST(dap::GotoTargetsRequest, dap::GotoTargetsResponse)               \
  DAP_REQUEST(dap::InitializeRequest, dap::InitializeResponse)                 \
  DAP_REQUEST(dap::LaunchRequest, dap::LaunchResponse)                         \
  DAP_REQUEST(dap::LoadedSourcesRequest, dap::LoadedSourcesResponse)           \
  DAP_REQUEST(dap::ModulesRequest, dap::ModulesResponse)                       \
  DAP_REQUEST(dap::NextRequest, dap::NextResponse)                             \
  DAP_REQUEST(dap::PauseRequest, dap::PauseResponse)                           \
  DAP_REQUEST(dap::ReadMemoryRequest, dap::ReadMemoryResponse)                 \
  DAP_REQUEST(dap::RestartFrameRequest, dap::RestartFrameResponse)             \
  DAP_REQUEST(dap::RestartRequest, dap::RestartResponse)                       \
  DAP_REQUEST(dap::ReverseContinueRequest, dap::ReverseContinueResponse)       \
  DAP_REQUEST(dap::RunInTerminalRequest, dap::RunInTerminalResponse)           \
  DAP_REQUEST(dap::ScopesRequest, dap::ScopesResponse)                         \
  DAP_REQUEST(dap::SetBreakpointsRequest, dap::SetBreakpointsResponse)         \
  DAP_REQUEST(dap::SetDataBreakpointsRequest, dap::SetDataBreakpointsResponse) \
  DAP_REQUEST(dap::SetExceptionBreakpointsRequest,                             \
              dap::SetExceptionBreakpointsResponse)                            \
  DAP_REQUEST(dap::SetExpressionRequest, dap::SetExpressionResponse)           \
  DAP_REQUEST(dap::SetFunctionBreakpointsRequest,                              \
              dap::SetFunctionBreakpointsResponse)                             \
  DAP_REQUEST(dap::SetVariableRequest, dap::SetVariableResponse)               \
  DAP_REQUEST(dap::SourceRequest, dap::SourceResponse)                         \
  DAP_REQUEST(dap::StackTraceRequest, dap::StackTraceResponse)                 \
  DAP_REQUEST(dap::StepBackRequest, dap::StepBackResponse)                     \
  DAP_REQUEST(dap::StepInRequest, dap::StepInResponse)                         \
  DAP_REQUEST(dap::StepInTargetsRequest, dap::StepInTargetsResponse)           \
  DAP_REQUEST(dap::StepOutRequest, dap::StepOutResponse)                       \
  DAP_REQUEST(dap::TerminateRequest, dap::TerminateResponse)                   \
  DAP_REQUEST(dap::TerminateThreadsRequest, dap::TerminateThreadsResponse)     \
  DAP_REQUEST(dap::ThreadsRequest, dap::ThreadsResponse)                       \
  DAP_REQUEST(dap::VariablesRequest, dap::VariablesResponse)

// Fuzzing main function.
// See http://llvm.org/docs/LibFuzzer.html for details.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // The first byte can optionally control fuzzing mode.
  enum class ControlMode {
    // Don't wrap the input data with a stream writer. Allows testing for stream
    // writing.
    TestStreamWriter,

    // Don't append a 'done' request. This may cause the test to take longer to
    // complete (it may have to block on a timeout), but exercises the
    // unrecognised-message cases.
    DontAppendDoneRequest,

    // Number of control modes in this enum.
    Count,
  };

  // Scan first byte for control mode.
  bool useContentStreamWriter = true;
  bool appendDoneRequest = true;
  if (size > 0 && data[0] < static_cast<uint8_t>(ControlMode::Count)) {
    useContentStreamWriter =
        data[0] != static_cast<uint8_t>(ControlMode::TestStreamWriter);
    appendDoneRequest =
        data[0] != static_cast<uint8_t>(ControlMode::DontAppendDoneRequest);
    data++;
    size--;
  }

  // in contains the input data
  auto in = std::make_shared<dap::StringBuffer>();

  dap::ContentWriter writer(in);
  if (useContentStreamWriter) {
    writer.write(std::string(reinterpret_cast<const char*>(data), size));
  } else {
    in->write(data, size);
  }

  if (appendDoneRequest) {
    writer.write(R"(
  {
      "seq": 10,
      "type": "request",
      "command": "done",
  }
    )");
  }

  // Each test is done if we receive a request, or report an error.
  Event requestOrError;

#define DAP_REQUEST(REQUEST, RESPONSE)           \
  session->registerHandler([&](const REQUEST&) { \
    requestOrError.fire();                       \
    return RESPONSE{};                           \
  });

  auto session = dap::Session::create();
  DAP_REQUEST_LIST();

  session->onError([&](const char*) { requestOrError.fire(); });

  auto out = std::make_shared<dap::StringBuffer>();
  session->bind(dap::ReaderWriter::create(in, out));

  // Give up after a second if we don't get a request or error reported.
  requestOrError.wait(std::chrono::seconds(1));

  return 0;
}