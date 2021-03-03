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

#ifndef VIEWER_REMOTE_SERVER_H
#define VIEWER_REMOTE_SERVER_H

#include <stddef.h>
#include <mutex>

class CivetServer;

namespace filament {
namespace viewer {

class WsHandler;

/**
 * Encapsulates a message sent from the web client.
 *
 * All instances of ReceivedMessage and their data / strings are owned by RemoteServer.
 * These can be freed via RemoteServer::releaseReceivedMessage().
 */
struct ReceivedMessage {
    char* label;
    char* buffer;
    size_t bufferByteCount;
    size_t messageUid;
};

/**
 * Manages a tiny WebSocket server that can receive model data and viewer settings.
 *
 * Client apps can call peekReceivedMessage to check for a new model, or acquireReceivedMessage
 * to pop it off the small internal queue. When they are done examining the message contents
 * they should call releaseReceivedMessage.
 *
 * TODO: Currently this can only receive model data. We would like to extend it to receive
 * viewer settings and commands (e.g. "Start Automation Test").
 */
class RemoteServer {
public:
    RemoteServer(int port = 8082);
    ~RemoteServer();
    bool isValid() const { return mCivetServer; }
    char const* peekIncomingLabel() const;
    ReceivedMessage const* peekReceivedMessage() const;
    ReceivedMessage const* acquireReceivedMessage();
    void releaseReceivedMessage(ReceivedMessage const* message);

private:
    void enqueueReceivedMessage(ReceivedMessage* message);
    void setIncomingMessage(ReceivedMessage* message);
    CivetServer* mCivetServer = nullptr;
    WsHandler* mWsHandler = nullptr;
    size_t mNextMessageUid = 0;
    size_t mOldestMessageUid = 0;
    static const size_t kMessageCapacity = 4;
    ReceivedMessage* mReceivedMessages[kMessageCapacity] = {};
    ReceivedMessage* mIncomingMessage = nullptr;
    mutable std::mutex mReceivedMessagesMutex;
    friend class WsHandler;
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_REMOTE_SERVER_H
