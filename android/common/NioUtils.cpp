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

#include "common/NioUtils.h"

#include <algorithm>

#include <utils/Log.h>

AutoBuffer::AutoBuffer(JNIEnv *env, jobject buffer, jint size, bool commit) noexcept :
        mEnv(env),
        mDoCommit(commit) {

    mNioUtils.jniClass = env->FindClass("com/google/android/filament/NioUtils");
    mNioUtils.jniClass = (jclass) env->NewGlobalRef(mNioUtils.jniClass);

    mNioUtils.getBasePointer = env->GetStaticMethodID(mNioUtils.jniClass,
            "getBasePointer", "(Ljava/nio/Buffer;JI)J");
    mNioUtils.getBaseArray = env->GetStaticMethodID(mNioUtils.jniClass,
            "getBaseArray", "(Ljava/nio/Buffer;)Ljava/lang/Object;");
    mNioUtils.getBaseArrayOffset = env->GetStaticMethodID(mNioUtils.jniClass,
            "getBaseArrayOffset", "(Ljava/nio/Buffer;I)I");
    mNioUtils.getBufferType = env->GetStaticMethodID(mNioUtils.jniClass,
            "getBufferType", "(Ljava/nio/Buffer;)I");

    mBuffer = env->NewGlobalRef(buffer);

    mType = (BufferType) env->CallStaticIntMethod(
                mNioUtils.jniClass, mNioUtils.getBufferType, mBuffer);

    switch (mType) {
        case BufferType::BYTE:
            mShift = 0;
            break;
        case BufferType::CHAR:
        case BufferType::SHORT:
            mShift = 1;
            break;
        case BufferType::INT:
        case BufferType::FLOAT:
            mShift = 2;
            break;
        case BufferType::LONG:
        case BufferType::DOUBLE:
            mShift = 3;
            break;
    }

    mSize = (size_t) size << mShift;

    jlong address = (jlong) env->GetDirectBufferAddress(mBuffer);
    if (address) {
        // Direct buffer case
        mData = reinterpret_cast<void *>(env->CallStaticLongMethod(mNioUtils.jniClass,
                mNioUtils.getBasePointer, mBuffer, address, mShift));
        mUserData = mData;
    } else {
        // wrapped array case
        jarray array = (jarray) env->CallStaticObjectMethod(mNioUtils.jniClass,
                mNioUtils.getBaseArray, mBuffer);

        jint offset = env->CallStaticIntMethod(mNioUtils.jniClass,
                mNioUtils.getBaseArrayOffset, mBuffer, mShift);

        mBaseArray = (jarray) env->NewGlobalRef(array);
        switch (mType) {
            case BufferType::BYTE:
                mData = env->GetByteArrayElements((jbyteArray)mBaseArray, nullptr);
                break;
            case BufferType::CHAR:
                mData = env->GetCharArrayElements((jcharArray)mBaseArray, nullptr);
                break;
            case BufferType::SHORT:
                mData = env->GetShortArrayElements((jshortArray)mBaseArray, nullptr);
                break;
            case BufferType::INT:
                mData = env->GetIntArrayElements((jintArray)mBaseArray, nullptr);
                break;
            case BufferType::LONG:
                mData = env->GetLongArrayElements((jlongArray)mBaseArray, nullptr);
                break;
            case BufferType::FLOAT:
                mData = env->GetFloatArrayElements((jfloatArray)mBaseArray, nullptr);
                break;
            case BufferType::DOUBLE:
                mData = env->GetDoubleArrayElements((jdoubleArray)mBaseArray, nullptr);
                break;
        }
        mUserData = (void *) ((char *) mData + offset);
    }
}

AutoBuffer::AutoBuffer(AutoBuffer &&rhs) noexcept {
    mEnv = rhs.mEnv;
    std::swap(mData, rhs.mData);
    std::swap(mUserData, rhs.mUserData);
    std::swap(mSize, rhs.mSize);
    std::swap(mType, rhs.mType);
    std::swap(mShift, rhs.mShift);
    std::swap(mBuffer, rhs.mBuffer);
    std::swap(mBaseArray, rhs.mBaseArray);
    std::swap(mNioUtils, rhs.mNioUtils);
}

AutoBuffer::~AutoBuffer() noexcept {
    JNIEnv *env = mEnv;
    if (mBaseArray) {
        jint mode = mDoCommit ? 0 : JNI_ABORT;
        switch (mType) {
            case BufferType::BYTE:
                env->ReleaseByteArrayElements((jbyteArray)mBaseArray, (jbyte *) mData, mode);
                break;
            case BufferType::CHAR:
                env->ReleaseCharArrayElements((jcharArray)mBaseArray, (jchar *) mData, mode);
                break;
            case BufferType::SHORT:
                env->ReleaseShortArrayElements((jshortArray)mBaseArray, (jshort *) mData, mode);
                break;
            case BufferType::INT:
                env->ReleaseIntArrayElements((jintArray)mBaseArray, (jint *) mData, mode);
                break;
            case BufferType::LONG:
                env->ReleaseLongArrayElements((jlongArray)mBaseArray, (jlong *) mData, mode);
                break;
            case BufferType::FLOAT:
                env->ReleaseFloatArrayElements((jfloatArray)mBaseArray, (jfloat *) mData, mode);
                break;
            case BufferType::DOUBLE:
                env->ReleaseDoubleArrayElements((jdoubleArray)mBaseArray, (jdouble *) mData, mode);
                break;
        }
        env->DeleteGlobalRef(mBaseArray);
    }
    if (mBuffer) {
        env->DeleteGlobalRef(mBuffer);
    }
    mEnv->DeleteGlobalRef(mNioUtils.jniClass);
}
