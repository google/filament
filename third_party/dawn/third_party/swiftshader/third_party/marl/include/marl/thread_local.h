// Copyright 2023 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// A wrapper around a thread_local variable, or a pthread key

#ifndef marl_thread_local_h
#define marl_thread_local_h

#ifdef MARL_USE_PTHREAD_THREAD_LOCAL
#include "debug.h"

#include <pthread.h>
#include <type_traits>

template <typename T>
class ThreadLocal {
  static_assert(std::is_pointer<T>::value,
                "The current implementation of ThreadLocal requires that T "
                "must be a pointer");

 public:
  inline ThreadLocal(T v) {
    pthread_key_create(&key, NULL);
    pthread_setspecific(key, v);
  }
  inline ~ThreadLocal() { pthread_key_delete(key); }
  inline operator T() const { return static_cast<T>(pthread_getspecific(key)); }
  inline ThreadLocal& operator=(T v) {
    pthread_setspecific(key, v);
    return *this;
  }

 private:
  ThreadLocal(const ThreadLocal&) = delete;
  ThreadLocal& operator=(const ThreadLocal&) = delete;

  pthread_key_t key;
};

#define MARL_DECLARE_THREAD_LOCAL(TYPE, NAME) static ThreadLocal<TYPE> NAME
#define MARL_INSTANTIATE_THREAD_LOCAL(TYPE, NAME, VALUE) \
  ThreadLocal<TYPE> NAME { VALUE }

#else

#define MARL_DECLARE_THREAD_LOCAL(TYPE, NAME) static thread_local TYPE NAME
#define MARL_INSTANTIATE_THREAD_LOCAL(TYPE, NAME, VALUE) \
  thread_local TYPE NAME { VALUE }

#endif

#endif  // marl_thread_local_h
