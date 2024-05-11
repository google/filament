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

#include <viewer/Settings.h>

#include <utils/compiler.h>

#include <stddef.h>
#include <mutex>

class CivetServer;

namespace filament {
namespace viewer {

class MessageSender;
class MessageReceiver;

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
 * Client apps can call peekReceivedMessage to check for new data, or acquireReceivedMessage
 * to pop it off the small internal queue. When they are done examining the message contents
 * they should call releaseReceivedMessage.
 */
class UTILS_PUBLIC RemoteServer {
public:
    RemoteServer(int port = 8082);
    ~RemoteServer();
    bool isValid() const { return mMessageSender; }

    /**
     * Checks if a download is currently in progress and returns its label.
     * Returns null if nothing is being downloaded.
     */
    char const* peekIncomingLabel() const;

    /**
     * Pops a message off the incoming queue or returns null if there are no unread messages.
     *
     * After examining its contents, users should free the message with releaseReceivedMessage.
     */
    ReceivedMessage const* acquireReceivedMessage();

    /**
     * Frees the memory that holds the contents of a received message.
     */
    void releaseReceivedMessage(ReceivedMessage const* message);

    void sendMessage(const Settings& settings);
    void sendMessage(const char* label, const char* buffer, size_t bufsize);

    // For internal use (makes JNI simpler)
    ReceivedMessage const* peekReceivedMessage() const;

private:
    void enqueueReceivedMessage(ReceivedMessage* message);
    void setIncomingMessage(ReceivedMessage* message);
    MessageSender* mMessageSender = nullptr;
    MessageReceiver* mMessageReceiver = nullptr;
    size_t mNextMessageUid = 0;
    static const size_t kMessageCapacity = 4;
    ReceivedMessage* mReceivedMessages[kMessageCapacity] = {};
    ReceivedMessage* mIncomingMessage = nullptr;
    JsonSerializer mSerializer;
    mutable std::mutex mReceivedMessagesMutex;
    friend class MessageReceiver;
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_REMOTE_SERVER_H
