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

#ifndef TNT_UTILS_CONDITION_H
#define TNT_UTILS_CONDITION_H

/**
 * @file Condition.h
 * @brief Custom low-overhead condition variable primitives for Filament.
 *
 * Filament provides two types of condition variables: C++ standard std::condition_variable
 * and custom utils::Condition.
 *
 * ARCHITECTURAL TRADEOFFS & CHOICE CRITERIA:
 *
 * 1. Use C++ standard std::condition_variable when:
 *    - CRITICAL OS SCHEDULING / PI SUPPORT: The condition variable blocks a thread for extended
 *      durations, or handles high-priority wakeups (e.g. CommandBufferQueue frame synchronizations).
 *      Standard CVs are fully integrated with standard std::mutex and leverage native OS wakeup queues,
 *      which respect thread scheduling policies and priority inheritance seamlessly.
 *
 * 2. Use custom utils::Condition when:
 *    - LOW-OVERHEAD / SAME-PRIORITY WAKEUPS: The condition variable is used between same-priority threads
 *      (where priority inversion is not a risk) and footprint size is a primary concern.
 *      On Linux/Android, utils::Condition is a futex-based 4-byte atomic variable, completely bypassing
 *      standard library heap allocation and dynamic library dispatch wrappers, enabling extremely fast
 *      signaling for local sub-systems.
 */

#if defined(__ANDROID__)
#include <utils/linux/Condition.h>
#else
#include <utils/generic/Condition.h>
#endif

#endif // TNT_UTILS_CONDITION_H
