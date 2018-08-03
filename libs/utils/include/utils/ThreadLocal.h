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

#define UTILS_DECLARE_TLS(clazz) utils::ThreadLocal<clazz>
#define UTILS_DEFINE_TLS(clazz) utils::ThreadLocal<clazz>

#include <algorithm>
#include <pthread.h>

namespace utils {

/* ------------------------------------------------------------------------------------------------
 * ThreadLocal is a workaround for compilers/platforms that don't support C++0x11 thread_local
 * e.g.:
 *      static ThreadLocal<Foo> s_current_thread(args);
 *
 * is equivalent to:
 *
 *      static thread_local Foo s_current_thread(args);
 *
 */

// TODO: Specialization for basic types (to avoid heap-allocation)

template <typename T>
class ThreadLocal {
public:
    ThreadLocal() noexcept {
        pthread_key_create(&mKey, destructor);
    }

    // construct from the arguments of T ctor
    template<typename ... ARGS>
    explicit ThreadLocal(ARGS&&... args) noexcept : ThreadLocal() {
        T* ptr = new T(std::forward<ARGS>(args)...);
        pthread_setspecific(mKey, ptr);
    }

    ~ThreadLocal() {
        pthread_key_delete(mKey);
    }

    ThreadLocal(const ThreadLocal& rhs) = delete;
    ThreadLocal(ThreadLocal&&  rhs) = delete;
    ThreadLocal& operator=(const ThreadLocal& rhs) = delete;
    ThreadLocal& operator=(ThreadLocal&&  rhs) = delete;

    // assign a T
    T& operator=(T value) noexcept {
        T& r = getRef();
        std::swap(r, value);
        return r;
    }

    // convert to a T
    operator T&() noexcept {
        T& r = getRef();
        return r;
    }

    operator T const&() const noexcept {
        T const& r = const_cast<ThreadLocal*>(this)->getRef();
        return r;
    }

    // pointer-like access, in case T is a smart pointer for instance
    T* operator->() noexcept {
        T& r = getRef();
        return &r;
    }

    T const* operator->() const noexcept {
        T const& r = const_cast<ThreadLocal*>(this)->getRef();
        return &r;
    }

private:
    // destructor called at thread exit
    static void destructor(void *specific) {
        T* ptr = static_cast<T*>(specific);
        delete ptr;
    }

    UTILS_NOINLINE
    T& getRef() noexcept {
        T* ptr = static_cast<T*>(pthread_getspecific(mKey));
        if (UTILS_LIKELY(ptr)) {
            return *ptr;
        }
        return allocRef();
    }

    UTILS_NOINLINE
    T& allocRef() noexcept {
        T* ptr = new T();
        pthread_setspecific(mKey, ptr);
        return *ptr;
    }

    pthread_key_t mKey;
};


/*
 * Specialization for pointers
 */

template <typename T>
class ThreadLocal<T *> {
public:
    ThreadLocal() noexcept {
        pthread_key_create(&mKey, nullptr);
    }

    // construct from the arguments of T ctor
    explicit ThreadLocal(T* rhs) noexcept : ThreadLocal() {
        pthread_setspecific(mKey, rhs);
    }

    ~ThreadLocal() {
        pthread_key_delete(mKey);
    }

    ThreadLocal(const ThreadLocal& rhs) = delete;
    ThreadLocal(ThreadLocal&&  rhs) = delete;
    ThreadLocal& operator=(const ThreadLocal& rhs) = delete;
    ThreadLocal& operator=(ThreadLocal&&  rhs) = delete;


    ThreadLocal& operator = (T const * rhs) noexcept {
        pthread_setspecific(mKey, rhs);
        return *this;
    }


    operator bool() noexcept {
        return pthread_getspecific(mKey) != nullptr;
    }

    // convert to a T*
    operator T*() noexcept {
        return static_cast<T*>(pthread_getspecific(mKey));
    }

    operator T const*() const noexcept {
        return static_cast<T const*>(pthread_getspecific(mKey));
    }

    // access like a pointer
    T* operator->() noexcept {
        return static_cast<T*>(pthread_getspecific(mKey));
    }

    T const* operator->() const noexcept {
        return static_cast<T const*>(pthread_getspecific(mKey));
    }

    T& operator*() noexcept {
        return *static_cast<T*>(pthread_getspecific(mKey));
    }

    T const& operator*() const noexcept {
        return *static_cast<T const*>(pthread_getspecific(mKey));
    }

private:
    pthread_key_t mKey;
};

}  // namespace utils

#endif // UTILS_HAS_FEATURE_CXX_THREAD_LOCAL

#endif // TNT_UTILS_THREADLOCAL_H
