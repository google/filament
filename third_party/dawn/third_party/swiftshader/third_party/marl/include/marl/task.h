// Copyright 2020 The Marl Authors.
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

#ifndef marl_task_h
#define marl_task_h

#include "export.h"

#include <functional>

namespace marl {

// Task is a unit of work for the scheduler.
class Task {
 public:
  using Function = std::function<void()>;

  enum class Flags {
    None = 0,

    // SameThread ensures the task will be run on the same thread that scheduled
    // the task. This can offer performance improvements if the current thread
    // is immediately going to block on the newly scheduled task, by reducing
    // overheads of waking another thread.
    SameThread = 1,
  };

  MARL_NO_EXPORT inline Task();
  MARL_NO_EXPORT inline Task(const Task&);
  MARL_NO_EXPORT inline Task(Task&&);
  MARL_NO_EXPORT inline Task(const Function& function,
                             Flags flags = Flags::None);
  MARL_NO_EXPORT inline Task(Function&& function, Flags flags = Flags::None);
  MARL_NO_EXPORT inline Task& operator=(const Task&);
  MARL_NO_EXPORT inline Task& operator=(Task&&);
  MARL_NO_EXPORT inline Task& operator=(const Function&);
  MARL_NO_EXPORT inline Task& operator=(Function&&);

  // operator bool() returns true if the Task has a valid function.
  MARL_NO_EXPORT inline operator bool() const;

  // operator()() runs the task.
  MARL_NO_EXPORT inline void operator()() const;

  // is() returns true if the Task was created with the given flag.
  MARL_NO_EXPORT inline bool is(Flags flag) const;

 private:
  Function function;
  Flags flags = Flags::None;
};

Task::Task() = default;
Task::Task(const Task& o) : function(o.function), flags(o.flags) {}
Task::Task(Task&& o) : function(std::move(o.function)), flags(o.flags) {}
Task::Task(const Function& function_, Flags flags_ /* = Flags::None */)
    : function(function_), flags(flags_) {}
Task::Task(Function&& function_, Flags flags_ /* = Flags::None */)
    : function(std::move(function_)), flags(flags_) {}
Task& Task::operator=(const Task& o) {
  function = o.function;
  flags = o.flags;
  return *this;
}
Task& Task::operator=(Task&& o) {
  function = std::move(o.function);
  flags = o.flags;
  return *this;
}

Task& Task::operator=(const Function& f) {
  function = f;
  flags = Flags::None;
  return *this;
}
Task& Task::operator=(Function&& f) {
  function = std::move(f);
  flags = Flags::None;
  return *this;
}
Task::operator bool() const {
  return function.operator bool();
}

void Task::operator()() const {
  function();
}

bool Task::is(Flags flag) const {
  return (static_cast<int>(flags) & static_cast<int>(flag)) ==
         static_cast<int>(flag);
}

}  // namespace marl

#endif  // marl_task_h
