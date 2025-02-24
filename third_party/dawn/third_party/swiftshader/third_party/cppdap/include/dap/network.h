// Copyright 2019 Google LLC
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

#ifndef dap_network_h
#define dap_network_h

#include <functional>
#include <memory>

namespace dap {
class ReaderWriter;

namespace net {

// connect() connects to the given TCP address and port.
// If timeoutMillis is non-zero and no connection was made before timeoutMillis
// milliseconds, then nullptr is returned.
std::shared_ptr<ReaderWriter> connect(const char* addr,
                                      int port,
                                      uint32_t timeoutMillis = 0);

// Server implements a basic TCP server.
class Server {
  // ignoreErrors() matches the OnError signature, and does nothing.
  static inline void ignoreErrors(const char*) {}

 public:
  using OnError = std::function<void(const char*)>;
  using OnConnect = std::function<void(const std::shared_ptr<ReaderWriter>&)>;

  virtual ~Server() = default;

  // create() constructs and returns a new Server.
  static std::unique_ptr<Server> create();

  // start() begins listening for connections on the given port.
  // callback will be called for each connection.
  // onError will be called for any connection errors.
  virtual bool start(int port,
                     const OnConnect& callback,
                     const OnError& onError = ignoreErrors) = 0;

  // stop() stops listening for connections.
  // stop() is implicitly called on destruction.
  virtual void stop() = 0;
};

}  // namespace net
}  // namespace dap

#endif  // dap_network_h
