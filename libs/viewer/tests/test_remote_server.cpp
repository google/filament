/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <viewer/RemoteServer.h>

#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace filament::viewer;

namespace {

class WebSocketClient {
public:
    explicit WebSocketClient(int port) {
        mSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        for (int attempt = 0; attempt < 100; ++attempt) {
            if (connect(mSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        const std::string request =
                "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\n"
                "Connection: Upgrade\r\nSec-WebSocket-Key: ZmlsYW1lbnQtdGVzdA==\r\n"
                "Sec-WebSocket-Version: 13\r\n\r\n";
        EXPECT_EQ(send(mSocket, request.data(), request.size(), 0), ssize_t(request.size()));
        std::array<char, 1024> response = {};
        const ssize_t size = recv(mSocket, response.data(), response.size(), 0);
        EXPECT_GT(size, 0);
        if (size > 0) {
            EXPECT_NE(std::string(response.data(), size).find(" 101 "), std::string::npos);
        }
    }

    ~WebSocketClient() { close(); }

    void close() {
        if (mSocket >= 0) {
            ::shutdown(mSocket, SHUT_RDWR);
            ::close(mSocket);
            mSocket = -1;
        }
    }

    void frame(const std::vector<char>& payload, bool final, uint8_t opcode) {
        std::vector<uint8_t> frame;
        frame.push_back(uint8_t((final ? 0x80 : 0) | opcode));
        if (payload.size() < 126) {
            frame.push_back(uint8_t(0x80 | payload.size()));
        } else {
            frame.push_back(0x80 | 126);
            frame.push_back(uint8_t(payload.size() >> 8));
            frame.push_back(uint8_t(payload.size()));
        }
        const std::array<uint8_t, 4> mask = { 0x12, 0x34, 0x56, 0x78 };
        frame.insert(frame.end(), mask.begin(), mask.end());
        for (size_t i = 0; i < payload.size(); ++i) {
            frame.push_back(uint8_t(payload[i]) ^ mask[i % mask.size()]);
        }
        ASSERT_EQ(send(mSocket, frame.data(), frame.size(), 0), ssize_t(frame.size()));
    }

    void part(const std::string& value, size_t split = 0) {
        if (split && split < value.size()) {
            frame({ value.begin(), value.begin() + split }, false, 0x2);
            frame({ value.begin() + split, value.end() }, true, 0x0);
        } else {
            frame({ value.begin(), value.end() }, true, 0x2);
        }
    }

private:
    int mSocket = -1;
};

int reservePort() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    EXPECT_EQ(bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
    socklen_t size = sizeof(address);
    EXPECT_EQ(getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size), 0);
    ::close(fd);
    return ntohs(address.sin_port);
}

struct Snapshot {
    std::string label;
    std::string body;
};

std::vector<Snapshot> acquire(RemoteServer& server, size_t expected) {
    std::vector<Snapshot> messages;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (messages.size() < expected && std::chrono::steady_clock::now() < deadline) {
        if (auto message = server.acquireReceivedMessage()) {
            messages.push_back({ message->label,
                message->bufferByteCount ? std::string(message->buffer, message->bufferByteCount)
                                         : std::string() });
            server.releaseReceivedMessage(message);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    return messages;
}

TEST(RemoteServerTest, InterleavedConnectionsAssembleIndependently) {
    const int port = reservePort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());
    WebSocketClient first(port);
    WebSocketClient second(port);

    first.frame({ 'a', 'l', 'p' }, false, 0x2);
    second.frame({ 'b', 'e', 't' }, false, 0x2);
    first.frame({ 'h', 'a' }, true, 0x0);
    second.frame({ 'a' }, true, 0x0);
    first.frame({ 'A', 'A' }, false, 0x2);
    second.frame({ 'B', 'B' }, false, 0x2);
    first.frame({ 'A' }, true, 0x0);
    second.frame({ 'B' }, true, 0x0);

    auto messages = acquire(server, 2);
    ASSERT_EQ(messages.size(), 2u);
    std::sort(messages.begin(), messages.end(),
            [](const Snapshot& lhs, const Snapshot& rhs) { return lhs.label < rhs.label; });
    EXPECT_EQ(messages[0].label, "alpha");
    EXPECT_EQ(messages[0].body, "AAA");
    EXPECT_EQ(messages[1].label, "beta");
    EXPECT_EQ(messages[1].body, "BBB");
}

TEST(RemoteServerTest, DisconnectDiscardsOnlyThatConnectionsPartialMessage) {
    const int port = reservePort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());
    WebSocketClient abandoned(port);
    abandoned.part("abandoned");
    abandoned.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    WebSocketClient complete(port);
    complete.part("complete");
    complete.part("body");
    auto messages = acquire(server, 1);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].label, "complete");
    EXPECT_EQ(messages[0].body, "body");
}

TEST(RemoteServerTest, SameLabelReplacementKeepsNewestCompleteMessage) {
    const int port = reservePort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());
    WebSocketClient first(port);
    WebSocketClient second(port);
    first.part("same");
    first.part("old");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    second.part("same");
    second.part("new");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto messages = acquire(server, 1);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].label, "same");
    EXPECT_EQ(messages[0].body, "new");
    EXPECT_EQ(acquire(server, 1).size(), 0u);
}

TEST(RemoteServerTest, EmptyBodyIsACompleteMessage) {
    const int port = reservePort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());
    WebSocketClient client(port);
    client.part("empty");
    client.frame({}, true, 0x2);

    auto messages = acquire(server, 1);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].label, "empty");
    EXPECT_TRUE(messages[0].body.empty());
}

TEST(RemoteServerTest, EmptyQueueReturnsNullAndDoesNotAffectNextMessage) {
    const int port = reservePort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());

    EXPECT_EQ(server.acquireReceivedMessage(), nullptr);
    EXPECT_EQ(server.peekIncomingLabel(), nullptr);

    WebSocketClient client(port);
    client.part("after-empty");
    client.part("body");

    auto messages = acquire(server, 1);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages[0].label, "after-empty");
    EXPECT_EQ(messages[0].body, "body");
    EXPECT_EQ(server.acquireReceivedMessage(), nullptr);
}

TEST(RemoteServerTest, DestructionDiscardsConnectedPartialMessages) {
    const int port = reservePort();
    auto server = std::make_unique<RemoteServer>(port);
    ASSERT_TRUE(server->isValid());
    WebSocketClient first(port);
    WebSocketClient second(port);
    first.part("first-partial");
    second.part("second-partial");

    server.reset();
}

} // anonymous namespace

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
