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

// Example of a task that creates and waits on sub tasks.

#include "marl/defer.h"
#include "marl/scheduler.h"
#include "marl/waitgroup.h"

#include <cstdio>

int main() {
  // Create a marl scheduler using the 4 hardware threads.
  // Bind this scheduler to the main thread so we can call marl::schedule()
  marl::Scheduler::Config cfg;
  cfg.setWorkerThreadCount(4);

  marl::Scheduler scheduler(cfg);
  scheduler.bind();
  defer(scheduler.unbind());  // Automatically unbind before returning.

  // marl::schedule() requires the scheduler to be bound to the current thread
  // (see above). The scheduler ensures that tasks are run on a thread with the
  // same scheduler automatically bound, so we don't need to call
  // marl::Scheduler::bind() again below.

  // Sequence of task events:
  //   __________________________________________________________
  //  |                                                          |
  //  |               ---> [task B] ----                         |
  //  |             /                    \                       |
  //  |  [task A] -----> [task A: wait] -----> [task A: resume]  |
  //  |             \                    /                       |
  //  |               ---> [task C] ----                         |
  //  |__________________________________________________________|

  // Create a WaitGroup for waiting for task A to finish.
  // This has an initial count of 1 (A)
  marl::WaitGroup a_wg(1);

  // Schedule task A
  marl::schedule([=] {
    defer(a_wg.done());  // Decrement a_wg when task A is done

    printf("Hello from task A\n");
    printf("Starting tasks B and C...\n");

    // Create a WaitGroup for waiting on task B and C to finish.
    // This has an initial count of 2 (B + C)
    marl::WaitGroup bc_wg(2);

    // Schedule task B
    marl::schedule([=] {
      defer(bc_wg.done());  // Decrement bc_wg when task B is done
      printf("Hello from task B\n");
    });

    // Schedule task C
    marl::schedule([=] {
      defer(bc_wg.done());  // Decrement bc_wg when task C is done
      printf("Hello from task C\n");
    });

    // Wait for tasks B and C to finish.
    bc_wg.wait();
  });

  // Wait for task A (and so B and C) to finish.
  a_wg.wait();

  printf("Task A has finished\n");
}
