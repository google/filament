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

// The Trace API produces a trace event file that can be consumed with Chrome's
// chrome://tracing viewer.
// Documentation can be found at:
//   https://www.chromium.org/developers/how-tos/trace-event-profiling-tool
//   https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit

#ifndef marl_trace_h
#define marl_trace_h

#define MARL_TRACE_ENABLED 0

#if MARL_TRACE_ENABLED

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <cstring>
#include <mutex>
#include <ostream>
#include <queue>
#include <thread>

namespace marl {

// Trace writes a trace event file into the current working directory that can
// be consumed with Chrome's chrome://tracing viewer.
// Use the MARL_* macros below instead of using this class directly.
class Trace {
 public:
  static constexpr size_t MaxEventNameLength = 64;

  static Trace* get();

  void nameThread(const char* fmt, ...);
  void beginEvent(const char* fmt, ...);
  void endEvent();
  void beginAsyncEvent(uint32_t id, const char* fmt, ...);
  void endAsyncEvent(uint32_t id, const char* fmt, ...);

  class ScopedEvent {
   public:
    inline ScopedEvent(const char* fmt, ...);
    inline ~ScopedEvent();

   private:
    Trace* const trace;
  };

  class ScopedAsyncEvent {
   public:
    inline ScopedAsyncEvent(uint32_t id, const char* fmt, ...);
    inline ~ScopedAsyncEvent();

   private:
    Trace* const trace;
    const uint32_t id;
    std::string name;
  };

 private:
  Trace();
  ~Trace();
  Trace(const Trace&) = delete;
  Trace& operator=(const Trace&) = delete;

  struct Event {
    enum class Type : uint8_t {
      Begin = 'B',
      End = 'E',
      Complete = 'X',
      Instant = 'i',
      Counter = 'C',
      AsyncStart = 'b',
      AsyncInstant = 'n',
      AsyncEnd = 'e',
      FlowStart = 's',
      FlowStep = 't',
      FlowEnd = 'f',
      Sample = 'P',
      ObjectCreated = 'N',
      ObjectSnapshot = 'O',
      ObjectDestroyed = 'D',
      Metadata = 'M',
      GlobalMemoryDump = 'V',
      ProcessMemoryDump = 'v',
      Mark = 'R',
      ClockSync = 'c',
      ContextEnter = '(',
      ContextLeave = ')',

      // Internal types
      Shutdown = 'S',
    };

    Event();
    virtual ~Event() = default;
    virtual Type type() const = 0;
    virtual void write(std::ostream& out) const;

    char name[MaxEventNameLength] = {};
    const char** categories = nullptr;
    uint64_t timestamp = 0;  // in microseconds
    uint32_t processID = 0;
    uint32_t threadID;
    uint32_t fiberID;
  };

  struct BeginEvent : public Event {
    Type type() const override { return Type::Begin; }
  };
  struct EndEvent : public Event {
    Type type() const override { return Type::End; }
  };
  struct MetadataEvent : public Event {
    Type type() const override { return Type::Metadata; }
  };
  struct Shutdown : public Event {
    Type type() const override { return Type::Shutdown; }
  };

  struct AsyncEvent : public Event {
    void write(std::ostream& out) const override;
    uint32_t id;
  };

  struct AsyncStartEvent : public AsyncEvent {
    Type type() const override { return Type::AsyncStart; }
  };
  struct AsyncEndEvent : public AsyncEvent {
    Type type() const override { return Type::AsyncEnd; }
  };

  struct NameThreadEvent : public MetadataEvent {
    void write(std::ostream& out) const override;
  };

  uint64_t timestamp();  // in microseconds

  void put(Event*);
  std::unique_ptr<Event> take();

  struct EventQueue {
    std::queue<std::unique_ptr<Event> > data;  // guarded by mutes
    std::condition_variable condition;
    std::mutex mutex;
  };
  // TODO: Increasing this from 1 can cause events to go out of order.
  // Investigate, fix.
  std::array<EventQueue, 1> eventQueues;
  std::atomic<unsigned int> eventQueueWriteIdx = {0};
  unsigned int eventQueueReadIdx = 0;
  std::chrono::time_point<std::chrono::high_resolution_clock> createdAt =
      std::chrono::high_resolution_clock::now();
  std::thread thread;
  std::atomic<bool> stopped = {false};
};

Trace::ScopedEvent::ScopedEvent(const char* fmt, ...) : trace(Trace::get()) {
  if (trace != nullptr) {
    char name[Trace::MaxEventNameLength];
    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(name, Trace::MaxEventNameLength, fmt, vararg);
    va_end(vararg);

    trace->beginEvent(name);
  }
}

Trace::ScopedEvent::~ScopedEvent() {
  if (trace != nullptr) {
    trace->endEvent();
  }
}

Trace::ScopedAsyncEvent::ScopedAsyncEvent(uint32_t id, const char* fmt, ...)
    : trace(Trace::get()), id(id) {
  if (trace != nullptr) {
    char buf[Trace::MaxEventNameLength];
    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(buf, Trace::MaxEventNameLength, fmt, vararg);
    va_end(vararg);
    name = buf;

    trace->beginAsyncEvent(id, "%s", buf);
  }
}

Trace::ScopedAsyncEvent::~ScopedAsyncEvent() {
  if (trace != nullptr) {
    trace->endAsyncEvent(id, "%s", name.c_str());
  }
}

}  // namespace marl

#define MARL_CONCAT_(a, b) a##b
#define MARL_CONCAT(a, b) MARL_CONCAT_(a, b)
#define MARL_SCOPED_EVENT(...) \
  marl::Trace::ScopedEvent MARL_CONCAT(scoped_event, __LINE__)(__VA_ARGS__);
#define MARL_BEGIN_ASYNC_EVENT(id, ...)    \
  do {                                     \
    if (auto t = marl::Trace::get()) {     \
      t->beginAsyncEvent(id, __VA_ARGS__); \
    }                                      \
  } while (false);
#define MARL_END_ASYNC_EVENT(id, ...)    \
  do {                                   \
    if (auto t = marl::Trace::get()) {   \
      t->endAsyncEvent(id, __VA_ARGS__); \
    }                                    \
  } while (false);
#define MARL_SCOPED_ASYNC_EVENT(id, ...) \
  marl::Trace::ScopedAsyncEvent MARL_CONCAT(defer_, __LINE__)(id, __VA_ARGS__);
#define MARL_NAME_THREAD(...)          \
  do {                                 \
    if (auto t = marl::Trace::get()) { \
      t->nameThread(__VA_ARGS__);      \
    }                                  \
  } while (false);

#else  // MARL_TRACE_ENABLED

#define MARL_SCOPED_EVENT(...)
#define MARL_BEGIN_ASYNC_EVENT(id, ...)
#define MARL_END_ASYNC_EVENT(id, ...)
#define MARL_SCOPED_ASYNC_EVENT(id, ...)
#define MARL_NAME_THREAD(...)

#endif  // MARL_TRACE_ENABLED

#endif  // marl_trace_h
