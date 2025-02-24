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

#ifndef marl_thread_h
#define marl_thread_h

#include "containers.h"
#include "export.h"

#include <functional>

namespace marl {

// Thread provides an OS abstraction for threads of execution.
class Thread {
 public:
  using Func = std::function<void()>;

  // Core identifies a logical processor unit.
  // How a core is identified varies by platform.
  struct Core {
    struct Windows {
      uint8_t group;  // Group number
      uint8_t index;  // Core within the processor group
    };
    struct Pthread {
      uint16_t index;  // Core number
    };
    union {
      Windows windows;
      Pthread pthread;
    };

    // Comparison functions
    MARL_NO_EXPORT inline bool operator==(Core) const;
    MARL_NO_EXPORT inline bool operator<(Core) const;
  };

  // Affinity holds the affinity mask for a thread - a description of what cores
  // the thread is allowed to run on.
  struct Affinity {
    // supported is true if marl supports controlling thread affinity for this
    // platform.
#if defined(_WIN32) ||                                                       \
    (defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)) || \
    defined(__FreeBSD__)
    static constexpr bool supported = true;
#else
    static constexpr bool supported = false;
#endif

    // Policy is an interface that provides a get() method for returning an
    // Affinity for the given thread by id.
    class Policy {
     public:
      virtual ~Policy() {}

      // anyOf() returns a Policy that returns an Affinity for a number of
      // available cores in affinity.
      //
      // Windows requires that each thread is only associated with a
      // single affinity group, so the Policy's returned affinity will contain
      // cores all from the same group.
      MARL_EXPORT static std::shared_ptr<Policy> anyOf(
          Affinity&& affinity,
          Allocator* allocator = Allocator::Default);

      // oneOf() returns a Policy that returns an affinity with a single enabled
      // core from affinity. The single enabled core in the Policy's returned
      // affinity is:
      //      affinity[threadId % affinity.count()]
      MARL_EXPORT static std::shared_ptr<Policy> oneOf(
          Affinity&& affinity,
          Allocator* allocator = Allocator::Default);

      // get() returns the thread Affinity for the given thread by id.
      MARL_EXPORT virtual Affinity get(uint32_t threadId,
                                       Allocator* allocator) const = 0;
    };

    MARL_EXPORT Affinity(Allocator*);

    MARL_EXPORT Affinity(Affinity&&);

    MARL_EXPORT Affinity& operator=(Affinity&&);

    MARL_EXPORT Affinity(const Affinity&, Allocator* allocator);

    // all() returns an Affinity with all the cores available to the process.
    MARL_EXPORT static Affinity all(Allocator* allocator = Allocator::Default);

    MARL_EXPORT Affinity(std::initializer_list<Core>, Allocator* allocator);

    MARL_EXPORT Affinity(const containers::vector<Core, 32>&,
                         Allocator* allocator);

    // count() returns the number of enabled cores in the affinity.
    MARL_EXPORT size_t count() const;

    // operator[] returns the i'th enabled core from this affinity.
    MARL_EXPORT Core operator[](size_t index) const;

    // add() adds the cores from the given affinity to this affinity.
    // This affinity is returned to allow for fluent calls.
    MARL_EXPORT Affinity& add(const Affinity&);

    // remove() removes the cores from the given affinity from this affinity.
    // This affinity is returned to allow for fluent calls.
    MARL_EXPORT Affinity& remove(const Affinity&);

   private:
    Affinity(const Affinity&) = delete;

    containers::vector<Core, 32> cores;
  };

  MARL_EXPORT Thread() = default;

  MARL_EXPORT Thread(Thread&&);

  MARL_EXPORT Thread& operator=(Thread&&);

  // Start a new thread using the given affinity that calls func.
  MARL_EXPORT Thread(Affinity&& affinity, Func&& func);

  MARL_EXPORT ~Thread();

  // join() blocks until the thread completes.
  MARL_EXPORT void join();

  // setName() sets the name of the currently executing thread for displaying
  // in a debugger.
  MARL_EXPORT static void setName(const char* fmt, ...);

  // numLogicalCPUs() returns the number of available logical CPU cores for
  // the system.
  MARL_EXPORT static unsigned int numLogicalCPUs();

 private:
  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;

  class Impl;
  Impl* impl = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// Thread::Core
////////////////////////////////////////////////////////////////////////////////
// Comparison functions
bool Thread::Core::operator==(Core other) const {
  return pthread.index == other.pthread.index;
}

bool Thread::Core::operator<(Core other) const {
  return pthread.index < other.pthread.index;
}

}  // namespace marl

#endif  // marl_thread_h
