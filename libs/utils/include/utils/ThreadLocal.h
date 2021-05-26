/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_UTILS_THREADLOCAL_H
#define TNT_UTILS_THREADLOCAL_H

#include <utils/compiler.h>

#if UTILS_HAS_FEATURE_CXX_THREAD_LOCAL

#define UTILS_DECLARE_TLS(clazz) thread_local clazz
#define UTILS_DEFINE_TLS(clazz) thread_local clazz

#else // UTILS_HAS_FEATURE_CXX_THREAD_LOCAL

#warning "thread_local not supported by this platform"
#define UTILS_DECLARE_TLS(clazz) clazz
#define UTILS_DEFINE_TLS(clazz) clazz

#endif // UTILS_HAS_FEATURE_CXX_THREAD_LOCAL

#endif // TNT_UTILS_THREADLOCAL_H
