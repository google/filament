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

#include <limits>

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

extern "C" JNIEXPORT jboolean JNICALL
        Java_com_google_android_filament_utils_RemoteServer_nAcquireReceivedMessage(JNIEnv* env,
                jclass, jlong native, jobject result) {
    RemoteServer* server = (RemoteServer*) native;
    ReceivedMessage const* msg = server->acquireReceivedMessage();
    if (msg == nullptr) {
        return false;
    }

    if (msg->bufferByteCount > std::numeric_limits<jint>::max()) {
        jclass exception = env->FindClass("java/lang/IllegalStateException");
        if (exception) {
            env->ThrowNew(exception, "Received message is too large for a Java ByteBuffer");
        }
        server->releaseReceivedMessage(msg);
        return false;
    }

    jclass resultClass = env->GetObjectClass(result);
    jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");
    if (env->ExceptionCheck() || resultClass == nullptr || byteBufferClass == nullptr) {
        server->releaseReceivedMessage(msg);
        return false;
    }
    jfieldID labelField = env->GetFieldID(resultClass, "label", "Ljava/lang/String;");
    jfieldID bufferField = env->GetFieldID(resultClass, "buffer", "Ljava/nio/ByteBuffer;");
    jmethodID allocateDirect =
            env->GetStaticMethodID(byteBufferClass, "allocateDirect", "(I)Ljava/nio/ByteBuffer;");
    if (env->ExceptionCheck() || labelField == nullptr || bufferField == nullptr ||
            allocateDirect == nullptr) {
        server->releaseReceivedMessage(msg);
        return false;
    }
    jobject buffer = env->CallStaticObjectMethod(byteBufferClass, allocateDirect,
            static_cast<jint>(msg->bufferByteCount));
    jstring label = env->NewStringUTF(msg->label);
    if (env->ExceptionCheck() || buffer == nullptr || label == nullptr) {
        server->releaseReceivedMessage(msg);
        return false;
    }
    void* address = env->GetDirectBufferAddress(buffer);
    if (address == nullptr && msg->bufferByteCount != 0) {
        jclass exception = env->FindClass("java/lang/IllegalStateException");
        if (exception) {
            env->ThrowNew(exception, "Could not access the allocated direct ByteBuffer");
        }
        server->releaseReceivedMessage(msg);
        return false;
    }

    if (msg->bufferByteCount) {
        memcpy(address, msg->buffer, msg->bufferByteCount);
    }

    env->SetObjectField(result, labelField, label);
    if (env->ExceptionCheck()) {
        server->releaseReceivedMessage(msg);
        return false;
    }
    env->SetObjectField(result, bufferField, buffer);
    server->releaseReceivedMessage(msg);
    return !env->ExceptionCheck();
}
