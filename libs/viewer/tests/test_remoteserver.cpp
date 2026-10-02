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

#include <utils/CString.h>

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <thread>
#include <vector>

#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

using namespace filament::viewer;
using namespace std::chrono_literals;

namespace {

// Minimal blocking WebSocket client, just enough to talk to RemoteServer.
class WsClient {
public:
    WsClient() = default;
    WsClient(WsClient const&) = delete;
    WsClient& operator=(WsClient const&) = delete;
    ~WsClient() { close(); }

    // Connects and performs the WebSocket handshake. This blocks until the server has accepted the
    // upgrade, i.e. until a server worker thread is available for this connection.
    bool connect(int port) {
        mFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (mFd < 0) {
            return false;
        }
        timeval tv{ .tv_sec = 10, .tv_usec = 0 };
        setsockopt(mFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#if defined(SO_NOSIGPIPE)
        int one = 1;
        setsockopt(mFd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(uint16_t(port));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::connect(mFd, (sockaddr const*) &addr, sizeof(addr)) != 0) {
            return false;
        }
        static constexpr char REQUEST[] =
                "GET / HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                "Sec-WebSocket-Version: 13\r\n"
                "\r\n";
        if (!sendAll(REQUEST, sizeof(REQUEST) - 1)) {
            return false;
        }
        char response[1024];
        size_t length = 0;
        while (std::string_view(response, length).find("\r\n\r\n") == std::string_view::npos) {
            if (length == sizeof(response) || ::recv(mFd, response + length, 1, 0) != 1) {
                return false;
            }
            length++;
        }
        return std::string_view(response, length).find(" 101 ") != std::string_view::npos;
    }

    // Sends one (masked) binary frame.
    bool sendFrame(void const* data, size_t size, bool fin = true, uint8_t opcode = 0x2) {
        std::vector<uint8_t> frame;
        frame.push_back(uint8_t((fin ? 0x80 : 0x00) | opcode));
        if (size < 126) {
            frame.push_back(uint8_t(0x80 | size));
        } else if (size < 65536) {
            frame.push_back(0x80 | 126);
            frame.push_back(uint8_t(size >> 8));
            frame.push_back(uint8_t(size));
        } else {
            frame.push_back(0x80 | 127);
            for (int i = 7; i >= 0; --i) {
                frame.push_back(uint8_t(uint64_t(size) >> (i * 8)));
            }
        }
        uint8_t const mask[4] = { 0x12, 0x34, 0x56, 0x78 };
        frame.insert(frame.end(), mask, mask + 4);
        uint8_t const* bytes = (uint8_t const*) data;
        for (size_t i = 0; i < size; i++) {
            frame.push_back(bytes[i] ^ mask[i % 4]);
        }
        return sendAll(frame.data(), frame.size());
    }

    // Sends a complete RemoteServer message: a label frame followed by a buffer frame.
    bool sendMessage(std::string_view label, std::string_view buffer) {
        return sendFrame(label.data(), label.size()) && sendFrame(buffer.data(), buffer.size());
    }

    void close() {
        if (mFd >= 0) {
            ::close(mFd);
            mFd = -1;
        }
    }

private:
    bool sendAll(void const* data, size_t size) {
        char const* p = (char const*) data;
        while (size) {
            ssize_t const n = ::send(mFd, p, size, 0);
            if (n <= 0) {
                return false;
            }
            p += n;
            size -= size_t(n);
        }
        return true;
    }

    int mFd = -1;
};

struct Message {
    utils::CString label;
    utils::CString buffer;
};

// Polls the server until a message is available or the timeout expires.
bool waitForMessage(RemoteServer& server, Message* out, std::chrono::milliseconds timeout = 5s) {
    auto const deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (ReceivedMessage const* msg = server.acquireReceivedMessage()) {
            out->label = utils::CString(msg->label);
            out->buffer = utils::CString(msg->buffer, msg->bufferByteCount);
            server.releaseReceivedMessage(msg);
            return true;
        }
        std::this_thread::sleep_for(5ms);
    }
    return false;
}

// Polls the server until the in-progress label matches the expected value.
bool waitForIncomingLabel(RemoteServer& server, std::string_view expected,
        std::chrono::milliseconds timeout = 5s) {
    auto const deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (std::string_view(server.getIncomingLabel()) == expected) {
            return true;
        }
        std::this_thread::sleep_for(5ms);
    }
    return false;
}

// Each test uses its own port so that lingering sockets from a previous test don't interfere.
int nextPort() {
    static std::atomic<int> sPort{ 28082 };
    return sPort++;
}

class RemoteServerTest : public testing::Test {};

} // anonymous namespace

TEST_F(RemoteServerTest, SingleClientRoundTrip) {
    int const port = nextPort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());

    WsClient client;
    ASSERT_TRUE(client.connect(port));
    ASSERT_TRUE(client.sendMessage("settings.json", "{}"));

    Message msg;
    ASSERT_TRUE(waitForMessage(server, &msg));
    EXPECT_EQ(std::string_view(msg.label), "settings.json");
    EXPECT_EQ(std::string_view(msg.buffer), "{}");
}

// Two concurrent clients must not share the label/buffer state machine. Without the fix, client B's
// label frame is interpreted as the buffer of client A's pending message, which is exactly the
// shared-state aliasing that leads to the heap corruption described in b/557282169.
TEST_F(RemoteServerTest, ConcurrentClientsDoNotShareState) {
    int const port = nextPort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());

    // Client A sends only the label part of a message, leaving it pending on the server.
    WsClient a;
    ASSERT_TRUE(a.connect(port));
    ASSERT_TRUE(a.sendFrame("a.json", 6));
    ASSERT_TRUE(waitForIncomingLabel(server, "a.json"));

    // Client B sends a complete message while A is still connected.
    std::atomic<bool> bSent{ false };
    std::thread bThread([&] {
        WsClient b;
        if (b.connect(port) && b.sendMessage("b.glb", "B")) {
            bSent = true;
        }
        // Keep B's connection open until its message has been processed.
        std::this_thread::sleep_for(1s);
    });

    // Give B's frames every chance to be processed while A is still connected.
    std::this_thread::sleep_for(300ms);
    a.close();

    Message msg;
    bool const received = waitForMessage(server, &msg);
    bThread.join();

    ASSERT_TRUE(bSent);
    ASSERT_TRUE(received);
    EXPECT_EQ(std::string_view(msg.label), "b.glb");
    EXPECT_EQ(std::string_view(msg.buffer), "B");
}

// A client that disconnects in the middle of a message must not leave stale state behind, neither
// in the incoming label nor in the label/buffer state machine seen by the next client.
TEST_F(RemoteServerTest, DisconnectDiscardsPartialMessage) {
    int const port = nextPort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());

    {
        WsClient a;
        ASSERT_TRUE(a.connect(port));
        ASSERT_TRUE(a.sendFrame("a.json", 6));
        ASSERT_TRUE(waitForIncomingLabel(server, "a.json"));
    }

    EXPECT_TRUE(waitForIncomingLabel(server, ""));

    WsClient b;
    ASSERT_TRUE(b.connect(port));
    ASSERT_TRUE(b.sendMessage("b.glb", "B"));

    Message msg;
    ASSERT_TRUE(waitForMessage(server, &msg));
    EXPECT_EQ(std::string_view(msg.label), "b.glb");
    EXPECT_EQ(std::string_view(msg.buffer), "B");
}

// Stress test for the heap corruption itself (Race A: concurrent std::vector insert, Race B:
// double-enqueue of the same message). Two clients send large multi-frame messages with the same
// label as fast as possible while the main thread drains the queue. This is timing dependent, so
// it is most effective under ASan or TSan, but it can also crash a regular build.
TEST_F(RemoteServerTest, ConcurrentClientsStress) {
    int const port = nextPort();
    RemoteServer server(port);
    ASSERT_TRUE(server.isValid());

    constexpr int MESSAGES_PER_CLIENT = 200;
    std::vector<char> const payload(8192, 'x');
    std::atomic<int> clientsDone{ 0 };

    auto const clientMain = [&](char id) {
        constexpr std::string_view label = "stress.glb";
        std::vector<char> const chunk(4096, id);
        WsClient client;
        if (client.connect(port)) {
            for (int i = 0; i < MESSAGES_PER_CLIENT; i++) {
                if (!client.sendFrame(label.data(), label.size()) ||
                        !client.sendFrame(chunk.data(), chunk.size(), false) ||
                        !client.sendFrame(payload.data(), payload.size(), true, 0x0)) {
                    break;
                }
            }
        }
        clientsDone++;
    };

    std::thread t1(clientMain, 'A');
    std::thread t2(clientMain, 'B');

    // Drain the queue while the clients are sending. Every message must be well formed.
    size_t received = 0;
    size_t malformed = 0;
    auto const deadline = std::chrono::steady_clock::now() + 30s;
    while (clientsDone < 2 && std::chrono::steady_clock::now() < deadline) {
        if (ReceivedMessage const* msg = server.acquireReceivedMessage()) {
            if (strcmp(msg->label, "stress.glb") != 0 ||
                    msg->bufferByteCount != 4096u + payload.size()) {
                malformed++;
            }
            server.releaseReceivedMessage(msg);
            received++;
        } else {
            std::this_thread::yield();
        }
    }

    t1.join();
    t2.join();
    EXPECT_GT(received, 0u);
    EXPECT_EQ(malformed, 0u);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
