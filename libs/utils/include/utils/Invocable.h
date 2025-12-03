/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_UTILS_INVOCABLE_H
#define TNT_UTILS_INVOCABLE_H

#include <type_traits>
#include <utility>

#include <assert.h>

namespace utils {
namespace io {
class ostream;
}

/*
 * Invocable is a move-only general purpose function wrapper. Instances can
 * store and invoke lambda expressions and other function objects.
 *
 * It is similar to std::function, with the following differences:
 *  - Invocable is move only.
 *  - Invocable can capture move only types.
 *  - No conversion between 'compatible' functions.
 *  - No small buffer optimization
 */

// Helper for enabling methods if Fn matches the function signature
// requirements of the Invocable type.
#if __cplusplus >= 201703L
// only available on C++17 and up
template<typename Fn, typename R, typename... Args>
using EnableIfFnMatchesInvocable =
        std::enable_if_t<std::is_invocable_v<Fn, Args...> &&
                         std::is_same_v<R, std::invoke_result_t<Fn, Args...>>, int>;
#else
template<typename Fn, typename R, typename... Args>
using EnableIfFnMatchesInvocable = std::enable_if_t<true, int>;
#endif

class InvocableBase {
protected:
    static io::ostream& printInvocable(io::ostream& out, const char* name);
};

template<typename Signature>
class Invocable;

template<typename R, typename... Args>
class Invocable<R(Args...)> : protected InvocableBase {
public:
    // Creates an Invocable that does not contain a functor.
    // Will evaluate to false.
    Invocable() = default;
    Invocable(std::nullptr_t) noexcept {}

    ~Invocable() noexcept;

    // Creates an Invocable from the functor passed in.
    template<typename Fn, EnableIfFnMatchesInvocable<Fn, R, Args...> = 0>
    Invocable(Fn&& fn) noexcept; // NOLINT(google-explicit-constructor)

    Invocable(const Invocable&) = delete;
    Invocable(Invocable&& rhs) noexcept;

    Invocable& operator=(const Invocable&) = delete;
    Invocable& operator=(Invocable&& rhs) noexcept;
    Invocable& operator=(std::nullptr_t) noexcept;

    // Invokes the invocable with the args passed in.
    // If the Invocable is empty, this will assert.
    template<typename... OperatorArgs>
    R operator()(OperatorArgs&& ... args);
    template<typename... OperatorArgs>
    R operator()(OperatorArgs&& ... args) const;

    // Evaluates to true if Invocable contains a functor, false otherwise.
    explicit operator bool() const noexcept;

private:
#if !defined(NDEBUG)
    friend io::ostream& operator<<(io::ostream& out, const Invocable&) {
        return printInvocable(out, "Invocable<>"); // TODO: is there a way to do better here?
    }
#endif
    void* mInvocable = nullptr;
    void (*mDeleter)(void*) = nullptr;
    R (* mInvoker)(void*, Args...) = nullptr;
};

template<typename R, typename... Args>
template<typename Fn, EnableIfFnMatchesInvocable<Fn, R, Args...>>
Invocable<R(Args...)>::Invocable(Fn&& fn) noexcept
        : mInvocable(new Fn(std::forward<std::decay_t<Fn>>(fn))),
          mDeleter(+[](void* erased_invocable) {
              auto typed_invocable = static_cast<Fn*>(erased_invocable);
              delete typed_invocable;
          }),
          mInvoker(+[](void* erased_invocable, Args... args) -> R {
              auto typed_invocable = static_cast<Fn*>(erased_invocable);
              return (*typed_invocable)(std::forward<Args>(args)...);
          })
{
}

template<typename R, typename... Args>
Invocable<R(Args...)>::~Invocable() noexcept {
    if (mDeleter) {
        mDeleter(mInvocable);
    }
}

template<typename R, typename... Args>
Invocable<R(Args...)>::Invocable(Invocable&& rhs) noexcept
        : mInvocable(rhs.mInvocable),
          mDeleter(rhs.mDeleter),
          mInvoker(rhs.mInvoker) {
    rhs.mInvocable = nullptr;
    rhs.mDeleter = nullptr;
    rhs.mInvoker = nullptr;
}

template<typename R, typename... Args>
Invocable<R(Args...)>& Invocable<R(Args...)>::operator=(Invocable&& rhs) noexcept {
    if (this != &rhs) {
        std::swap(mInvocable, rhs.mInvocable);
        std::swap(mDeleter, rhs.mDeleter);
        std::swap(mInvoker, rhs.mInvoker);
    }
    return *this;
}

template<typename R, typename... Args>
Invocable<R(Args...)>& Invocable<R(Args...)>::operator=(std::nullptr_t) noexcept {
    if (mDeleter) {
        mDeleter(mInvocable);
    }
    mInvocable = nullptr;
    mDeleter = nullptr;
    mInvoker = nullptr;
    return *this;
}

template<typename R, typename... Args>
template<typename... OperatorArgs>
R Invocable<R(Args...)>::operator()(OperatorArgs&& ... args) {
    assert(mInvoker && mInvocable);
    return mInvoker(mInvocable, std::forward<OperatorArgs>(args)...);
}

template<typename R, typename... Args>
template<typename... OperatorArgs>
R Invocable<R(Args...)>::operator()(OperatorArgs&& ... args) const {
    assert(mInvoker && mInvocable);
    return mInvoker(mInvocable, std::forward<OperatorArgs>(args)...);
}

template<typename R, typename... Args>
Invocable<R(Args...)>::operator bool() const noexcept {
    return mInvoker != nullptr && mInvocable != nullptr;
}

} // namespace utils

#endif // TNT_UTILS_INVOCABLE_H
