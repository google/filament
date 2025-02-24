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

#include "socket.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <chrono>
#include <thread>
#include <vector>

// Basic socket send & receive test
TEST(Socket, SendRecv) {
  const char* port = "19021";

  auto server = dap::Socket("localhost", port);

  auto client = dap::Socket::connect("localhost", port, 0);
  ASSERT_TRUE(client != nullptr);

  const std::string expect = "Hello World!";
  std::string read;

  auto thread = std::thread([&] {
    auto conn = server.accept();
    ASSERT_TRUE(conn != nullptr);
    char c;
    while (conn->read(&c, 1) != 0) {
      read += c;
    }
  });

  ASSERT_TRUE(client->write(expect.data(), expect.size()));

  client->close();
  thread.join();

  ASSERT_EQ(read, expect);
}

// See https://github.com/google/cppdap/issues/37
TEST(Socket, CloseOnDifferentThread) {
  const char* port = "19021";

  auto server = dap::Socket("localhost", port);

  auto client = dap::Socket::connect("localhost", port, 0);
  ASSERT_TRUE(client != nullptr);

  auto conn = server.accept();

  auto thread = std::thread([&] {
    // Closing client on different thread should unblock client->read().
    client->close();
  });

  char c;
  while (client->read(&c, 1) != 0) {
  }

  thread.join();
}

TEST(Socket, ConnectTimeout) {
  const char* port = "19021";
  const int timeoutMillis = 200;
  const int maxAttempts = 1024;

  using namespace std::chrono;

  auto server = dap::Socket("localhost", port);

  std::vector<std::shared_ptr<dap::ReaderWriter>> connections;

  for (int i = 0; i < maxAttempts; i++) {
    auto start = system_clock::now();
    auto connection = dap::Socket::connect("localhost", port, timeoutMillis);
    auto end = system_clock::now();

    if (!connection) {
      auto timeTakenMillis = duration_cast<milliseconds>(end - start).count();
      ASSERT_GE(timeTakenMillis + 20,  // +20ms for a bit of timing wiggle room
                timeoutMillis);
      return;
    }

    // Keep hold of the connections to saturate any incoming socket buffers.
    connections.emplace_back(std::move(connection));
  }

  FAIL() << "Failed to test timeout after " << maxAttempts << " attempts";
}
