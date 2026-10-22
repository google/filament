/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <filament/Engine.h>

#include <jni.h>

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_android_FilamentHelper_nGetJobSystem(JNIEnv*, jclass, jlong nativeEngine) {
    Engine* engine = reinterpret_cast<Engine*>(nativeEngine);
    return reinterpret_cast<jlong>(&engine->getJobSystem());
}
