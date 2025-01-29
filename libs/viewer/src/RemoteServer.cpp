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

#include <CivetServer.h>

#include <utils/Log.h>

#include <cstring>
#include <vector>

using namespace utils;

namespace filament {
namespace viewer {

class MessageSender : public CivetServer {
public:
    MessageSender(const char** options) : CivetServer(options) {}
    void sendMessage(const char* label, const char* buffer, size_t bufsize);
};

class MessageReceiver : public CivetWebSocketHandler {
   public:
    MessageReceiver(RemoteServer* server) : mServer(server) {}
    ~MessageReceiver() { delete mReceivedMessage; }
    bool handleData(CivetServer* server, struct mg_connection*, int, char* , size_t) override;
   private:
    RemoteServer* mServer;
    std::vector<char> mChunk;
    ReceivedMessage* mReceivedMessage = nullptr;
};

RemoteServer::RemoteServer(int port) {
    const char* kServerOptions[] = {
        "listening_ports", "8082",
        "num_threads",     "2",
        "error_log_file",  "civetweb.txt",
        nullptr,
    };
    std::string portString = std::to_string(port);
    kServerOptions[1] = portString.c_str();
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

char const * RemoteServer::peekIncomingLabel() const {
    std::lock_guard lock(mReceivedMessagesMutex);
    return mIncomingMessage ? mIncomingMessage->label : nullptr;
}

ReceivedMessage const * RemoteServer::peekReceivedMessage() const {
    std::lock_guard lock(mReceivedMessagesMutex);

    // Find the oldest message in the queue by looking for the lowest id.
    // Note that this queue is not a ring buffer, it's just a tiny sparse array.
    ReceivedMessage const* oldest = nullptr;
    for (auto msg : mReceivedMessages) {
        if (msg && (!oldest || msg->messageUid < oldest->messageUid)) oldest = msg;
    }

    return oldest;
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
            releaseReceivedMessage(old_message);
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
    if (opcode == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE) {
        return true;
    }

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

void MessageSender::sendMessage(const char* label, const char* buffer, size_t bufsize) {
    for (auto iter : connections) {
        mg_websocket_write(const_cast<mg_connection *>(iter.first), 0x80, label, strlen(label) + 1);
        mg_websocket_write(const_cast<mg_connection *>(iter.first), 0x80, buffer, bufsize);
    }
}

} // namespace viewer
} // namespace filament
