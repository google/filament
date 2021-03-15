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

#include <jni.h>

#include <viewer/RemoteServer.h>

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
    char const* label = server->peekIncomingLabel();
    return label ? env->NewStringUTF(label) : nullptr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_utils_RemoteServer_nPeekReceivedLabel(JNIEnv* env, jclass, jlong native) {
    RemoteServer* server = (RemoteServer*) native;
    ReceivedMessage const* msg = server->peekReceivedMessage();
    return msg ? env->NewStringUTF(msg->label) : nullptr;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_utils_RemoteServer_nPeekReceivedBufferLength(JNIEnv* env, jclass, jlong native) {
    RemoteServer* server = (RemoteServer*) native;
    ReceivedMessage const* msg = server->peekReceivedMessage();
    return msg ? msg->bufferByteCount : 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_RemoteServer_nAcquireReceivedMessage(JNIEnv* env, jclass, jlong native, jobject buffer, jint length) {
    RemoteServer* server = (RemoteServer*) native;
    ReceivedMessage const* msg = server->acquireReceivedMessage();
    if (msg == nullptr) {
        return;
    }

    void* address = env->GetDirectBufferAddress(buffer);
    if (address == nullptr) {
        // This should never happen because the Java layer does allocateDirect.
        return;
    }

    memcpy(address, msg->buffer, length);
    server->releaseReceivedMessage(msg);
}

