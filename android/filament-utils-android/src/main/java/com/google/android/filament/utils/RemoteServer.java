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

package com.google.android.filament.utils;

import androidx.annotation.Nullable;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Manages a tiny WebSocket server that can receive model data and viewer settings.
 *
 * Client apps can call acquireReceivedMessage to check for a new model and pop it off the small
 * internal queue.
 *
 * TODO: Currently this can only receive model data. We would like to extend it to receive
 * viewer settings and commands (e.g. "Start Automation Test").
 */
public class RemoteServer {
    private long mNativeObject;

    /**
    * Encapsulates a message sent from the web client.
    */
    public static class ReceivedMessage {
        public String label;
        public ByteBuffer buffer;
    }

    public RemoteServer(int port) {
        mNativeObject = nCreate(port);
        if (mNativeObject == 0) throw new IllegalStateException("Couldn't create RemoteServer");
    }

    public @Nullable String peekIncomingLabel() {
        return nPeekIncomingLabel(mNativeObject);
    }

    public @Nullable ReceivedMessage acquireReceivedMessage() {
        int length = nPeekReceivedBufferLength(mNativeObject);
        if (length == 0) {
            return null;
        }
        ReceivedMessage message = new ReceivedMessage();
        message.label = nPeekReceivedLabel(mNativeObject);
        message.buffer = ByteBuffer.allocateDirect(length);
        message.buffer.order(ByteOrder.LITTLE_ENDIAN);
        nAcquireReceivedMessage(mNativeObject, message.buffer, length);
        return message;
    }

    @Override
    protected void finalize() throws Throwable {
        nDestroy(mNativeObject);
        super.finalize();
    }

    private static native long nCreate(int port);
    private static native String nPeekIncomingLabel(long nativeObject);
    private static native String nPeekReceivedLabel(long nativeObject);
    private static native int nPeekReceivedBufferLength(long nativeObject);
    private static native void nAcquireReceivedMessage(long nativeObject, ByteBuffer buffer, int length);
    private static native void nDestroy(long nativeObject);
}
