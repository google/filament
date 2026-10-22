/*
 * Copyright (C) 2021 The Android Open Source Project
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
#include <utils/Log.h>

#include <CivetServer.h>

#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

using namespace utils;

namespace filament {
namespace viewer {

class MessageSender : public CivetServer {
public:
    MessageSender(const char** options) : CivetServer(options) {}
    void sendMessage(const char* label, const char* buffer, size_t bufsize);
};

// Note that MessageReceiver holds per-connection state (mChunk, mReceivedMessage) but a single
// instance is shared by all connections. The server is configured with a single worker thread
// (see kServerOptions), which serializes all connections and keeps the label/buffer state machine
// consistent. mStateMutex is a safety net: if num_threads is ever raised, concurrent connections
// will interleave their frames (a protocol error) but will never corrupt memory.
class MessageReceiver : public CivetWebSocketHandler {
   public:
    MessageReceiver(RemoteServer* server) : mServer(server) {}
    ~MessageReceiver() { mServer->releaseReceivedMessage(mReceivedMessage); }
    bool handleData(CivetServer* server, struct mg_connection*, int, char* , size_t) override;
    void handleClose(CivetServer* server, const struct mg_connection*) override;
   private:
    RemoteServer* mServer;
    std::mutex mStateMutex;
    std::vector<char> mChunk;
    ReceivedMessage* mReceivedMessage = nullptr;
};

RemoteServer::RemoteServer(int port) {
    // civetweb dedicates a worker thread to each WebSocket connection for its entire lifetime, and
    // all connections share the same MessageReceiver. A single worker thread guarantees that
    // MessageReceiver is never entered concurrently; additional clients wait until the current one
    // disconnects. Because a single stale connection would otherwise block the server until TCP
    // keep-alive kicks in (hours), ping/pong is enabled so that civetweb drops peers that stop
    // answering (MG_MAX_UNANSWERED_PING * websocket_timeout_ms, about one minute).
    const char* kServerOptions[] = {
        "listening_ports",            "8082",
        "num_threads",                "1",
        "enable_websocket_ping_pong", "yes",
        "websocket_timeout_ms",       "10000",
        "error_log_file",             "civetweb.txt",
        nullptr,
    };
    char portString[16];
    snprintf(portString, sizeof(portString), "%d", port);
    kServerOptions[1] = portString;
    mMessageSender = new MessageSender(kServerOptions);
    if (!mMessageSender->getContext()) {
        slog.e << "Unable to start RemoteServer, see civetweb.txt for details." << io::endl;
        delete mMessageSender;
        mMessageSender = nullptr;
        mMessageReceiver = nullptr;
        return;
    }
    mMessageReceiver = new MessageReceiver(this);
    mMessageSender->addWebSocketHandler("", mMessageReceiver);
    slog.i << "RemoteServer listening at ws://localhost:" << port << io::endl;
}

RemoteServer::~RemoteServer() {
    delete mMessageSender;
    delete mMessageReceiver;
    for (auto msg : mReceivedMessages) {
        releaseReceivedMessage(msg);
    }
}

CString RemoteServer::peekIncomingLabel() const {
    std::lock_guard lock(mReceivedMessagesMutex);
    return mIncomingMessage ? CString(mIncomingMessage->label) : CString();
}

ReceivedMessage const * RemoteServer::acquireReceivedMessage() {
    std::lock_guard lock(mReceivedMessagesMutex);

    // Find the oldest message in the queue by looking for the lowest id.
    ReceivedMessage** oldest = nullptr;
    for (auto& msg : mReceivedMessages) {
        if (msg && (!oldest || msg->messageUid < (*oldest)->messageUid)) oldest = &msg;
    }
    if (!oldest) return nullptr;

    // If this message is the most recent download, then mark the download as completed.
    ReceivedMessage const * result = *oldest;
    if (result == mIncomingMessage) {
        mIncomingMessage = nullptr;
    }

    // Replace the message slot with null and return the message to the caller.
    *oldest = nullptr;
    return result;
}

void RemoteServer::setIncomingMessage(ReceivedMessage* message) {
    std::lock_guard lock(mReceivedMessagesMutex);
    mIncomingMessage = message;
}

void RemoteServer::discardIncomingMessage(ReceivedMessage* message) {
    std::lock_guard lock(mReceivedMessagesMutex);
    if (mIncomingMessage == message) {
        mIncomingMessage = nullptr;
    }
    releaseReceivedMessage(message);
}

void RemoteServer::enqueueReceivedMessage(ReceivedMessage* message) {
    std::lock_guard lock(mReceivedMessagesMutex);

    // Check if any unread messages have the same label as the incoming message. If so, it is safe
    // to discard the old message and snarf its slot.
    ReceivedMessage** empty_slot = nullptr;
    for (auto& old_message : mReceivedMessages) {
        if (old_message == nullptr) {
            empty_slot = &old_message;
            continue;
        }
        if (!strcmp(old_message->label, message->label)) {
            if (old_message != message) {
                if (old_message == mIncomingMessage) {
                    mIncomingMessage = nullptr;
                }
                releaseReceivedMessage(old_message);
            }
            message->messageUid = mNextMessageUid++;
            old_message = message;
            return;
        }
    }

    // Otherwise use any empty slot in the queue.
    if (empty_slot) {
        message->messageUid = mNextMessageUid++;
        *empty_slot = message;
        return;
    }

    // If there are no empty slots, then discard the message. This basically never happens.
    slog.e << "Discarding message, message queue overflow." << io::endl;
    if (message == mIncomingMessage) {
        mIncomingMessage = nullptr;
    }
    releaseReceivedMessage(message);
}

void RemoteServer::releaseReceivedMessage(ReceivedMessage const* message) {
    if (message) {
        delete[] message->label;
        delete[] message->buffer;
        delete message;
    }
}

void RemoteServer::sendMessage(const Settings& settings) {
    const auto& json = mSerializer.writeJson(settings);
    mMessageSender->sendMessage("settings.json", json.c_str(), json.size() + 1);
}

void RemoteServer::sendMessage(const char* label, const char* buffer, size_t bufsize) {
    mMessageSender->sendMessage(label, buffer, bufsize);
}

// NOTE: This is invoked off the main thread.
bool MessageReceiver::handleData(CivetServer* server, struct mg_connection* conn, int bits,
                                  char* data, size_t size) {
    const bool final = bits & 0x80;
    const int opcode = bits & 0xf;
    // Ignore all control frames (close, ping, pong, reserved). civetweb consumes ping/pong itself
    // when enable_websocket_ping_pong is on, but a client may still send them unsolicited.
    if (opcode >= MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE) {
        return true;
    }

    std::lock_guard lock(mStateMutex);

    // Append this frame to the aggregated chunk.
    mChunk.insert(mChunk.end(), data, data + size);

    // If this message part still has outstanding frames, return early.
    if (!final) {
        return true;
    }

    // Part 1 of the message is the label.
    if (mReceivedMessage == nullptr) {
        mReceivedMessage = new ReceivedMessage({});
        mReceivedMessage->label = new char[mChunk.size() + 1]{};
        memcpy(mReceivedMessage->label, mChunk.data(), mChunk.size());
        mServer->setIncomingMessage(mReceivedMessage);
        mChunk.clear();
        return true;
    }

    // Part 2 of the message is the buffer.
    auto message = mReceivedMessage;
    message->bufferByteCount = mChunk.size();
    message->buffer = new char[message->bufferByteCount];
    memcpy(message->buffer, mChunk.data(), message->bufferByteCount);
    mChunk.clear();

    // We have all parts, so go ahead and enqueue the incoming message.
    mServer->enqueueReceivedMessage(mReceivedMessage);
    mReceivedMessage = nullptr;
    return true;
}

// NOTE: This is invoked off the main thread.
void MessageReceiver::handleClose(CivetServer* server, const struct mg_connection* conn) {
    std::lock_guard lock(mStateMutex);
    // Discard any partially received message so that it doesn't leak and so that the next client
    // starts with a clean state machine.
    if (mReceivedMessage) {
        mServer->discardIncomingMessage(mReceivedMessage);
        mReceivedMessage = nullptr;
    }
    mChunk.clear();
}

void MessageSender::sendMessage(const char* label, const char* buffer, size_t bufsize) {
    // The connections map is mutated by civetweb worker threads, so take a snapshot under the
    // context lock. The writes must happen outside of the context lock, because civetweb acquires
    // the connection lock before the context lock when closing a connection. Connection objects
    // are preallocated for the lifetime of the context, so a stale pointer is still valid memory;
    // at worst the write fails or reaches the next connection that reuses the same slot.
    std::vector<mg_connection*> targets;
    mg_lock_context(context);
    targets.reserve(connections.size());
    for (auto const& iter : connections) {
        targets.push_back(iter.first);
    }
    mg_unlock_context(context);

    for (mg_connection* conn : targets) {
        mg_websocket_write(conn, 0x80, label, strlen(label) + 1);
        mg_websocket_write(conn, 0x80, buffer, bufsize);
    }
}

} // namespace viewer
} // namespace filament
