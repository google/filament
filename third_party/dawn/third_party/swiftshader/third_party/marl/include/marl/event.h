// Copyright 2019 The Marl Authors.
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

#ifndef marl_event_h
#define marl_event_h

#include "conditionvariable.h"
#include "containers.h"
#include "export.h"
#include "memory.h"

#include <chrono>

namespace marl {

// Event is a synchronization primitive used to block until a signal is raised.
class Event {
 public:
  enum class Mode : uint8_t {
    // The event signal will be automatically reset when a call to wait()
    // returns.
    // A single call to signal() will only unblock a single (possibly
    // future) call to wait().
    Auto,

    // While the event is in the signaled state, any calls to wait() will
    // unblock without automatically reseting the signaled state.
    // The signaled state can be reset with a call to clear().
    Manual
  };

  MARL_NO_EXPORT inline Event(Mode mode = Mode::Auto,
                              bool initialState = false,
                              Allocator* allocator = Allocator::Default);

  // signal() signals the event, possibly unblocking a call to wait().
  MARL_NO_EXPORT inline void signal() const;

  // clear() clears the signaled state.
  MARL_NO_EXPORT inline void clear() const;

  // wait() blocks until the event is signaled.
  // If the event was constructed with the Auto Mode, then only one
  // call to wait() will unblock before returning, upon which the signalled
  // state will be automatically cleared.
  MARL_NO_EXPORT inline void wait() const;

  // wait_for() blocks until the event is signaled, or the timeout has been
  // reached.
  // If the timeout was reached, then wait_for() return false.
  // If the event is signalled and event was constructed with the Auto Mode,
  // then only one call to wait() will unblock before returning, upon which the
  // signalled state will be automatically cleared.
  template <typename Rep, typename Period>
  MARL_NO_EXPORT inline bool wait_for(
      const std::chrono::duration<Rep, Period>& duration) const;

  // wait_until() blocks until the event is signaled, or the timeout has been
  // reached.
  // If the timeout was reached, then wait_for() return false.
  // If the event is signalled and event was constructed with the Auto Mode,
  // then only one call to wait() will unblock before returning, upon which the
  // signalled state will be automatically cleared.
  template <typename Clock, typename Duration>
  MARL_NO_EXPORT inline bool wait_until(
      const std::chrono::time_point<Clock, Duration>& timeout) const;

  // test() returns true if the event is signaled, otherwise false.
  // If the event is signalled and was constructed with the Auto Mode
  // then the signalled state will be automatically cleared upon returning.
  MARL_NO_EXPORT inline bool test() const;

  // isSignalled() returns true if the event is signaled, otherwise false.
  // Unlike test() the signal is not automatically cleared when the event was
  // constructed with the Auto Mode.
  // Note: No lock is held after bool() returns, so the event state may
  // immediately change after returning. Use with caution.
  MARL_NO_EXPORT inline bool isSignalled() const;

  // any returns an event that is automatically signalled whenever any of the
  // events in the list are signalled.
  template <typename Iterator>
  MARL_NO_EXPORT inline static Event any(Mode mode,
                                         const Iterator& begin,
                                         const Iterator& end);

  // any returns an event that is automatically signalled whenever any of the
  // events in the list are signalled.
  // This overload defaults to using the Auto mode.
  template <typename Iterator>
  MARL_NO_EXPORT inline static Event any(const Iterator& begin,
                                         const Iterator& end);

 private:
  struct Shared {
    MARL_NO_EXPORT inline Shared(Allocator* allocator,
                                 Mode mode,
                                 bool initialState);
    MARL_NO_EXPORT inline void signal();
    MARL_NO_EXPORT inline void wait();

    template <typename Rep, typename Period>
    MARL_NO_EXPORT inline bool wait_for(
        const std::chrono::duration<Rep, Period>& duration);

    template <typename Clock, typename Duration>
    MARL_NO_EXPORT inline bool wait_until(
        const std::chrono::time_point<Clock, Duration>& timeout);

    marl::mutex mutex;
    ConditionVariable cv;
    containers::vector<std::shared_ptr<Shared>, 1> deps;
    const Mode mode;
    bool signalled;
  };

  const std::shared_ptr<Shared> shared;
};

Event::Shared::Shared(Allocator* allocator, Mode mode_, bool initialState)
    : cv(allocator), mode(mode_), signalled(initialState) {}

void Event::Shared::signal() {
  marl::lock lock(mutex);
  if (signalled) {
    return;
  }
  signalled = true;
  if (mode == Mode::Auto) {
    cv.notify_one();
  } else {
    cv.notify_all();
  }
  for (auto dep : deps) {
    dep->signal();
  }
}

void Event::Shared::wait() {
  marl::lock lock(mutex);
  cv.wait(lock, [&] { return signalled; });
  if (mode == Mode::Auto) {
    signalled = false;
  }
}

template <typename Rep, typename Period>
bool Event::Shared::wait_for(
    const std::chrono::duration<Rep, Period>& duration) {
  marl::lock lock(mutex);
  if (!cv.wait_for(lock, duration, [&] { return signalled; })) {
    return false;
  }
  if (mode == Mode::Auto) {
    signalled = false;
  }
  return true;
}

template <typename Clock, typename Duration>
bool Event::Shared::wait_until(
    const std::chrono::time_point<Clock, Duration>& timeout) {
  marl::lock lock(mutex);
  if (!cv.wait_until(lock, timeout, [&] { return signalled; })) {
    return false;
  }
  if (mode == Mode::Auto) {
    signalled = false;
  }
  return true;
}

Event::Event(Mode mode /* = Mode::Auto */,
             bool initialState /* = false */,
             Allocator* allocator /* = Allocator::Default */)
    : shared(allocator->make_shared<Shared>(allocator, mode, initialState)) {}

void Event::signal() const {
  shared->signal();
}

void Event::clear() const {
  marl::lock lock(shared->mutex);
  shared->signalled = false;
}

void Event::wait() const {
  shared->wait();
}

template <typename Rep, typename Period>
bool Event::wait_for(const std::chrono::duration<Rep, Period>& duration) const {
  return shared->wait_for(duration);
}

template <typename Clock, typename Duration>
bool Event::wait_until(
    const std::chrono::time_point<Clock, Duration>& timeout) const {
  return shared->wait_until(timeout);
}

bool Event::test() const {
  marl::lock lock(shared->mutex);
  if (!shared->signalled) {
    return false;
  }
  if (shared->mode == Mode::Auto) {
    shared->signalled = false;
  }
  return true;
}

bool Event::isSignalled() const {
  marl::lock lock(shared->mutex);
  return shared->signalled;
}

template <typename Iterator>
Event Event::any(Mode mode, const Iterator& begin, const Iterator& end) {
  Event any(mode, false);
  for (auto it = begin; it != end; it++) {
    auto s = it->shared;
    marl::lock lock(s->mutex);
    if (s->signalled) {
      any.signal();
    }
    s->deps.push_back(any.shared);
  }
  return any;
}

template <typename Iterator>
Event Event::any(const Iterator& begin, const Iterator& end) {
  return any(Mode::Auto, begin, end);
}

}  // namespace marl

#endif  // marl_event_h
