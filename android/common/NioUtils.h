/*
 * Copyright (C) 2017 The Android Open Source Project
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

#pragma once

#include <cstdint>

#include <jni.h>

class AutoBuffer {
public:
    enum class BufferType : uint8_t {
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE
    };

    // Clients should pass "true" for the commit argument if they intend to mutate the buffer
    // contents from native code.
    AutoBuffer(JNIEnv* env, jobject buffer, jint size, bool commit = false) noexcept;
    AutoBuffer(AutoBuffer&& rhs) noexcept;
    ~AutoBuffer() noexcept;

    void* getData() const noexcept {
        return mUserData;
    }

    size_t getSize() const noexcept {
        return mSize;
    }

    size_t getShift() const noexcept {
        return mShift;
    }

    size_t countToByte(size_t count) const noexcept {
        return count << mShift;
    }

private:
    void* mUserData = nullptr;
    size_t mSize = 0;
    BufferType mType = BufferType::BYTE;
    uint8_t mShift = 0;

    JNIEnv* mEnv;
    void* mData = nullptr;
    jobject mBuffer = nullptr;
    jarray mBaseArray = nullptr;
    bool mDoCommit = false;

    struct {
        jclass jniClass;
        jmethodID getBasePointer;
        jmethodID getBaseArray;
        jmethodID getBaseArrayOffset;
        jmethodID getBufferType;
    } mNioUtils{};
};
