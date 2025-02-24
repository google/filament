// Copyright 2019 Google LLC
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

#ifndef dap_future_h
#define dap_future_h

#include <condition_variable>
#include <memory>
#include <mutex>

namespace dap {

// internal functionality
namespace detail {
template <typename T>
struct promise_state {
  T val;
  std::mutex mutex;
  std::condition_variable cv;
  bool hasVal = false;
};
}  // namespace detail

// forward declaration
template <typename T>
class promise;

// future_status is the enumeration returned by future::wait_for and
// future::wait_until.
enum class future_status {
  ready,
  timeout,
};

// future is a minimal reimplementation of std::future, that does not suffer
// from TSAN false positives. See:
// https://gcc.gnu.org/bugzilla//show_bug.cgi?id=69204
template <typename T>
class future {
 public:
  using State = detail::promise_state<T>;

  // constructors
  inline future() = default;
  inline future(future&&) = default;

  // valid() returns true if the future has an internal state.
  bool valid() const;

  // get() blocks until the future has a valid result, and returns it.
  // The future must have a valid internal state to call this method.
  inline T get();

  // wait() blocks until the future has a valid result.
  // The future must have a valid internal state to call this method.
  void wait() const;

  // wait_for() blocks until the future has a valid result, or the timeout is
  // reached.
  // The future must have a valid internal state to call this method.
  template <class Rep, class Period>
  future_status wait_for(
      const std::chrono::duration<Rep, Period>& timeout) const;

  // wait_until() blocks until the future has a valid result, or the timeout is
  // reached.
  // The future must have a valid internal state to call this method.
  template <class Clock, class Duration>
  future_status wait_until(
      const std::chrono::time_point<Clock, Duration>& timeout) const;

 private:
  friend promise<T>;
  future(const future&) = delete;
  inline future(const std::shared_ptr<State>& state);

  std::shared_ptr<State> state = std::make_shared<State>();
};

template <typename T>
future<T>::future(const std::shared_ptr<State>& s) : state(s) {}

template <typename T>
bool future<T>::valid() const {
  return state;
}

template <typename T>
T future<T>::get() {
  std::unique_lock<std::mutex> lock(state->mutex);
  state->cv.wait(lock, [&] { return state->hasVal; });
  return state->val;
}

template <typename T>
void future<T>::wait() const {
  std::unique_lock<std::mutex> lock(state->mutex);
  state->cv.wait(lock, [&] { return state->hasVal; });
}

template <typename T>
template <class Rep, class Period>
future_status future<T>::wait_for(
    const std::chrono::duration<Rep, Period>& timeout) const {
  std::unique_lock<std::mutex> lock(state->mutex);
  return state->cv.wait_for(lock, timeout, [&] { return state->hasVal; })
             ? future_status::ready
             : future_status::timeout;
}

template <typename T>
template <class Clock, class Duration>
future_status future<T>::wait_until(
    const std::chrono::time_point<Clock, Duration>& timeout) const {
  std::unique_lock<std::mutex> lock(state->mutex);
  return state->cv.wait_until(lock, timeout, [&] { return state->hasVal; })
             ? future_status::ready
             : future_status::timeout;
}

// promise is a minimal reimplementation of std::promise, that does not suffer
// from TSAN false positives. See:
// https://gcc.gnu.org/bugzilla//show_bug.cgi?id=69204
template <typename T>
class promise {
 public:
  // constructors
  inline promise() = default;
  inline promise(promise&& other) = default;
  inline promise(const promise& other) = default;

  // set_value() stores value to the shared state.
  // set_value() must only be called once.
  inline void set_value(const T& value) const;
  inline void set_value(T&& value) const;

  // get_future() returns a future sharing this promise's state.
  future<T> get_future();

 private:
  using State = detail::promise_state<T>;
  std::shared_ptr<State> state = std::make_shared<State>();
};

template <typename T>
future<T> promise<T>::get_future() {
  return future<T>(state);
}

template <typename T>
void promise<T>::set_value(const T& value) const {
  std::unique_lock<std::mutex> lock(state->mutex);
  state->val = value;
  state->hasVal = true;
  state->cv.notify_all();
}

template <typename T>
void promise<T>::set_value(T&& value) const {
  std::unique_lock<std::mutex> lock(state->mutex);
  state->val = std::move(value);
  state->hasVal = true;
  state->cv.notify_all();
}

}  // namespace dap

#endif  // dap_future_h
