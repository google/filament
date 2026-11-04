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
 * Client apps can call acquireReceivedMessage to check for new data and pop it off the small
 * internal queue.
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

    /**
     * Checks if a download is currently in progress and returns its label.
     * Returns null if nothing is being downloaded.
     */
    public @Nullable String peekIncomingLabel() {
        return nPeekIncomingLabel(mNativeObject);
    }

    /**
     * Checks if a peeked message has JSON content.
     */
    public static boolean isJson(@Nullable String label) {
        return label != null && label.endsWith(".json");
    }

    /**
     * Checks if a peeked message has binary content.
     */
    public static boolean isBinary(@Nullable String label) {
        return label != null && !label.endsWith(".json");
    }

    /**
     * Pops a message off the incoming queue or returns null if there are no unread messages.
     */
    public @Nullable ReceivedMessage acquireReceivedMessage() {
        // Pop the message off the queue first. Once acquired, the native message is owned by us
        // and can no longer be modified or freed by the network thread.
        long nativeMessage = nAcquireReceivedMessage(mNativeObject);
        if (nativeMessage == 0) {
            return null;
        }
        try {
            int length = nGetReceivedBufferLength(nativeMessage);
            if (length <= 0) {
                return null;
            }
            ReceivedMessage message = new ReceivedMessage();
            message.label = nGetReceivedLabel(nativeMessage);
            message.buffer = ByteBuffer.allocateDirect(length);
            message.buffer.order(ByteOrder.LITTLE_ENDIAN);
            nCopyReceivedBuffer(nativeMessage, message.buffer);
            return message;
        } finally {
            nReleaseReceivedMessage(mNativeObject, nativeMessage);
        }
    }

    /*
     * Destroys the native WebSocket server and frees up the port.
     *
     * This might need to be done explicitly (as opposed to waiting for gc) to free up the port.
     */
    public void close() {
        nDestroy(mNativeObject);
        mNativeObject = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        nDestroy(mNativeObject);
        super.finalize();
    }

    private static native long nCreate(int port);
    private static native String nPeekIncomingLabel(long nativeObject);
    private static native long nAcquireReceivedMessage(long nativeObject);
    private static native String nGetReceivedLabel(long nativeMessage);
    private static native int nGetReceivedBufferLength(long nativeMessage);
    private static native void nCopyReceivedBuffer(long nativeMessage, ByteBuffer buffer);
    private static native void nReleaseReceivedMessage(long nativeObject, long nativeMessage);
    private static native void nDestroy(long nativeObject);
}
