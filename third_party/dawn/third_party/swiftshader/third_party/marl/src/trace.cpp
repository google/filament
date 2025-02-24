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
// about:tracing viewer.
// Documentation can be found at:
//   https://www.chromium.org/developers/how-tos/trace-event-profiling-tool
//   https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit

#include "marl/trace.h"

#include "marl/defer.h"
#include "marl/scheduler.h"
#include "marl/thread.h"

#if MARL_TRACE_ENABLED

#include <atomic>
#include <fstream>

namespace {

// Chrome traces can choke or error on very large trace files.
// Limit the number of events created to this number.
static constexpr int MaxEvents = 100000;

uint64_t threadFiberID(uint32_t threadID, uint32_t fiberID) {
  return static_cast<uint64_t>(threadID) * 31 + static_cast<uint64_t>(fiberID);
}

}  // anonymous namespace

namespace marl {

Trace* Trace::get() {
  static Trace trace;
  return &trace;
}

Trace::Trace() {
  nameThread("main");
  thread = std::thread([&] {
    Thread::setName("Trace worker");

    auto out = std::fstream("chrome.trace", std::ios_base::out);

    out << "[" << std::endl;
    defer(out << std::endl << "]" << std::endl);

    auto first = true;
    for (int i = 0; i < MaxEvents; i++) {
      auto event = take();
      if (event->type() == Event::Type::Shutdown) {
        return;
      }
      if (!first) {
        out << "," << std::endl;
      };
      first = false;
      out << "{" << std::endl;
      event->write(out);
      out << "}";
    }

    stopped = true;

    while (take()->type() != Event::Type::Shutdown) {
    }
  });
}

Trace::~Trace() {
  put(new Shutdown());
  thread.join();
}

void Trace::nameThread(const char* fmt, ...) {
  if (stopped) {
    return;
  }
  auto event = new NameThreadEvent();

  va_list vararg;
  va_start(vararg, fmt);
  vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
  va_end(vararg);

  put(event);
}

void Trace::beginEvent(const char* fmt, ...) {
  if (stopped) {
    return;
  }
  auto event = new BeginEvent();

  va_list vararg;
  va_start(vararg, fmt);
  vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
  va_end(vararg);

  event->timestamp = timestamp();
  put(event);
}

void Trace::endEvent() {
  if (stopped) {
    return;
  }
  auto event = new EndEvent();
  event->timestamp = timestamp();
  put(event);
}

void Trace::beginAsyncEvent(uint32_t id, const char* fmt, ...) {
  if (stopped) {
    return;
  }
  auto event = new AsyncStartEvent();

  va_list vararg;
  va_start(vararg, fmt);
  vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
  va_end(vararg);

  event->timestamp = timestamp();
  event->id = id;
  put(event);
}

void Trace::endAsyncEvent(uint32_t id, const char* fmt, ...) {
  if (stopped) {
    return;
  }
  auto event = new AsyncEndEvent();

  va_list vararg;
  va_start(vararg, fmt);
  vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
  va_end(vararg);

  event->timestamp = timestamp();
  event->id = id;
  put(event);
}

uint64_t Trace::timestamp() {
  auto now = std::chrono::high_resolution_clock::now();
  auto diff =
      std::chrono::duration_cast<std::chrono::microseconds>(now - createdAt);
  return static_cast<uint64_t>(diff.count());
}

void Trace::put(Event* event) {
  auto idx = eventQueueWriteIdx++ % eventQueues.size();
  auto& queue = eventQueues[idx];
  std::unique_lock<std::mutex> lock(queue.mutex);
  auto notify = queue.data.size() == 0;
  queue.data.push(std::unique_ptr<Event>(event));
  lock.unlock();
  if (notify) {
    queue.condition.notify_one();
  }
}

std::unique_ptr<Trace::Event> Trace::take() {
  auto idx = eventQueueReadIdx++ % eventQueues.size();
  auto& queue = eventQueues[idx];
  std::unique_lock<std::mutex> lock(queue.mutex);
  queue.condition.wait(lock, [&queue] { return queue.data.size() > 0; });
  auto event = std::move(queue.data.front());
  queue.data.pop();
  return event;
}

#define QUOTE(x) "\"" << x << "\""
#define INDENT "  "

Trace::Event::Event()
    : threadID(std::hash<std::thread::id>()(std::this_thread::get_id())) {
  if (auto fiber = Scheduler::Fiber::current()) {
    fiberID = fiber->id;
  }
}

void Trace::Event::write(std::ostream& out) const {
  out << INDENT << QUOTE("name") << ": " << QUOTE(name) << "," << std::endl;
  if (categories != nullptr) {
    out << INDENT << QUOTE("cat") << ": "
        << "\"";
    auto first = true;
    for (auto category = *categories; category != nullptr; category++) {
      if (!first) {
        out << ",";
      }
      out << category;
    }
    out << "\"," << std::endl;
  }
  if (fiberID != 0) {
    out << INDENT << QUOTE("args") << ": "
        << "{" << std::endl
        << INDENT << INDENT << QUOTE("fiber") << ": " << fiberID << std::endl
        << INDENT << "}," << std::endl;
  }
  if (threadID != 0) {
    out << INDENT << QUOTE("tid") << ": " << threadFiberID(threadID, fiberID)
        << "," << std::endl;
  }
  out << INDENT << QUOTE("ph") << ": " << QUOTE(static_cast<char>(type()))
      << "," << std::endl
      << INDENT << QUOTE("pid") << ": " << processID << "," << std::endl
      << INDENT << QUOTE("ts") << ": " << timestamp << std::endl;
}

void Trace::NameThreadEvent::write(std::ostream& out) const {
  out << INDENT << QUOTE("name") << ": " << QUOTE("thread_name") << ","
      << std::endl
      << INDENT << QUOTE("ph") << ": " << QUOTE("M") << "," << std::endl
      << INDENT << QUOTE("pid") << ": " << processID << "," << std::endl
      << INDENT << QUOTE("tid") << ": " << threadFiberID(threadID, fiberID)
      << "," << std::endl
      << INDENT << QUOTE("args") << ": {" << QUOTE("name") << ": "
      << QUOTE(name) << "}" << std::endl;
}

void Trace::AsyncEvent::write(std::ostream& out) const {
  out << INDENT << QUOTE("id") << ": " << QUOTE(id) << "," << std::endl
      << INDENT << QUOTE("cat") << ": " << QUOTE("async") << "," << std::endl;
  Event::write(out);
}

}  // namespace marl

#endif  // MARL_TRACE_ENABLED