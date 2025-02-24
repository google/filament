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

// This is an example application that uses Marl to find and print all the prime
// numbers in the range 1 to 10000000.

#include "marl/defer.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/ticket.h"

#include <vector>

#include <math.h>

// searchMax defines the upper limit on primes to find.
constexpr int searchMax = 10000000;

// searchChunkSize is the number of numbers searched, per task, for primes.
constexpr int searchChunkSize = 10000;

// isPrime returns true if i is prime.
bool isPrime(int i) {
  auto c = static_cast<int>(sqrt(i));
  for (int j = 2; j <= c; j++) {
    if (i % j == 0) {
      return false;
    }
  }
  return true;
}

int main() {
  // Create a marl scheduler using the full number of logical cpus.
  // Bind this scheduler to the main thread so we can call marl::schedule()
  marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
  scheduler.bind();
  defer(scheduler.unbind());  // unbind before destructing the scheduler.

  // Create a ticket queue. This will be used to ensure that the primes are
  // printed in ascending order.
  marl::Ticket::Queue queue;

  // Iterate over the range [1, searchMax] in steps of searchChunkSize.
  for (int searchBase = 1; searchBase <= searchMax;
       searchBase += searchChunkSize) {
    // Take a ticket from the ticket queue for this task.
    auto ticket = queue.take();

    // Schedule the task.
    marl::schedule([=] {
      // Find all the primes in [searchBase, searchBase+searchChunkSize-1].
      // Note this is run in parallel with the other scheduled tasks.
      std::vector<int> primes;
      for (int i = searchBase; i < searchBase + searchChunkSize; i++) {
        if (isPrime(i)) {
          primes.push_back(i);
        }
      }

      // Wait until the ticket is called. This ensures that the primes are
      // printed in ascending order. This may cause this task to yield and allow
      // other tasks to be executed while waiting for this ticket to be called.
      ticket.wait();

      // Print the primes.
      for (auto prime : primes) {
        printf("%d is prime\n", prime);
      }

      // Call the next ticket so that those primes can be printed.
      ticket.done();
    });
  }

  // take a ticket and wait on it to ensure that all the primes have been
  // calculated and printed.
  queue.take().wait();

  return 0;
}
