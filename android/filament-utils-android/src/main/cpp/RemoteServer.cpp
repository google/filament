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

#include <jni.h>

#include <algorithm>

#include <string.h>

using namespace filament::viewer;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_RemoteServer_nCreate(JNIEnv* env, jclass, jint port) {
    RemoteServer* server = new RemoteServer(port);
    if (!server->isValid()) {
        delete server;
        return 0;
    }
    return (jlong) server;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_RemoteServer_nDestroy(JNIEnv*, jclass, jlong native) {
    RemoteServer* server = (RemoteServer*) native;
    delete server;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_utils_RemoteServer_nPeekIncomingLabel(JNIEnv* env, jclass, jlong native) {
    RemoteServer* server = (RemoteServer*) native;
    // Copy the label under the server lock; the network thread may free the message at any time.
    utils::CString const label = server->peekIncomingLabel();
    return label.empty() ? nullptr : env->NewStringUTF(label.c_str());
}

// The functions below operate on a message that has been popped off the queue with
// nAcquireReceivedMessage. Once acquired, the message is owned by the caller and cannot be
// modified or freed by the network thread, until nReleaseReceivedMessage is called.

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_RemoteServer_nAcquireReceivedMessage(JNIEnv*, jclass, jlong native) {
    RemoteServer* server = (RemoteServer*) native;
    return (jlong) server->acquireReceivedMessage();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_utils_RemoteServer_nGetReceivedLabel(JNIEnv* env, jclass, jlong nativeMessage) {
    ReceivedMessage const* msg = (ReceivedMessage const*) nativeMessage;
    return env->NewStringUTF(msg->label);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_utils_RemoteServer_nGetReceivedBufferLength(JNIEnv*, jclass, jlong nativeMessage) {
    ReceivedMessage const* msg = (ReceivedMessage const*) nativeMessage;
    return (jint) msg->bufferByteCount;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_RemoteServer_nCopyReceivedBuffer(JNIEnv* env, jclass, jlong nativeMessage, jobject buffer) {
    ReceivedMessage const* msg = (ReceivedMessage const*) nativeMessage;
    void* address = env->GetDirectBufferAddress(buffer);
    if (address == nullptr) {
        // This should never happen because the Java layer does allocateDirect.
        return;
    }
    // Never copy more than either the destination capacity or the message size.
    jlong const capacity = env->GetDirectBufferCapacity(buffer);
    if (capacity < 0) {
        return;
    }
    size_t const length = std::min(size_t(capacity), msg->bufferByteCount);
    memcpy(address, msg->buffer, length);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_RemoteServer_nReleaseReceivedMessage(JNIEnv*, jclass, jlong native, jlong nativeMessage) {
    RemoteServer* server = (RemoteServer*) native;
    server->releaseReceivedMessage((ReceivedMessage const*) nativeMessage);
}
