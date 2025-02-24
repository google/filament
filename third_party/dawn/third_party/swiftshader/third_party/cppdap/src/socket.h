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

#ifndef dap_socket_h
#define dap_socket_h

#include "dap/io.h"

#include <atomic>
#include <memory>

namespace dap {

class Socket {
 public:
  class Shared;

  // connect() connects to the given TCP address and port.
  // If timeoutMillis is non-zero and no connection was made before
  // timeoutMillis milliseconds, then nullptr is returned.
  static std::shared_ptr<ReaderWriter> connect(const char* address,
                                               const char* port,
                                               uint32_t timeoutMillis);

  Socket(const char* address, const char* port);
  bool isOpen() const;
  std::shared_ptr<ReaderWriter> accept() const;
  void close() const;

 private:
  std::shared_ptr<Shared> shared;
};

}  // namespace dap

#endif  // dap_socket_h
