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

#ifndef marl_ticket_h
#define marl_ticket_h

#include "conditionvariable.h"
#include "pool.h"
#include "scheduler.h"

namespace marl {

// Ticket is a synchronization primitive used to serially order execution.
//
// Tickets exist in 3 mutually exclusive states: Waiting, Called and Finished.
//
// Tickets are obtained from a Ticket::Queue, using the Ticket::Queue::take()
// methods. The order in which tickets are taken from the queue dictates the
// order in which they are called.
//
// The first ticket to be taken from a queue will be in the 'called' state,
// subsequent tickets will be in the 'waiting' state.
//
// Ticket::wait() will block until the ticket is called.
//
// Ticket::done() moves the ticket into the 'finished' state. If all preceeding
// tickets are finished, done() will call the next unfinished ticket.
//
// If the last remaining reference to an unfinished ticket is dropped then
// done() will be automatically called on that ticket.
//
// Example:
//
//  void runTasksConcurrentThenSerially(int numConcurrentTasks)
//  {
//      marl::Ticket::Queue queue;
//      for (int i = 0; i < numConcurrentTasks; i++)
//      {
//          auto ticket = queue.take();
//          marl::schedule([=] {
//              doConcurrentWork(); // <- function may be called concurrently
//              ticket.wait(); // <- serialize tasks
//              doSerialWork(); // <- function will not be called concurrently
//              ticket.done(); // <- optional, as done() is called implicitly on
//                             // dropping of last reference
//          });
//      }
//  }
class Ticket {
  struct Shared;
  struct Record;

 public:
  using OnCall = std::function<void()>;

  // Queue hands out Tickets.
  class Queue {
   public:
    // take() returns a single ticket from the queue.
    MARL_NO_EXPORT inline Ticket take();

    // take() retrieves count tickets from the queue, calling f() with each
    // retrieved ticket.
    // F must be a function of the signature: void(Ticket&&)
    template <typename F>
    MARL_NO_EXPORT inline void take(size_t count, const F& f);

   private:
    std::shared_ptr<Shared> shared = std::make_shared<Shared>();
    UnboundedPool<Record> pool;
  };

  MARL_NO_EXPORT inline Ticket() = default;
  MARL_NO_EXPORT inline Ticket(const Ticket& other) = default;
  MARL_NO_EXPORT inline Ticket(Ticket&& other) = default;
  MARL_NO_EXPORT inline Ticket& operator=(const Ticket& other) = default;

  // wait() blocks until the ticket is called.
  MARL_NO_EXPORT inline void wait() const;

  // done() marks the ticket as finished and calls the next ticket.
  MARL_NO_EXPORT inline void done() const;

  // onCall() registers the function f to be invoked when this ticket is
  // called. If the ticket is already called prior to calling onCall(), then
  // f() will be executed immediately.
  // F must be a function of the OnCall signature.
  template <typename F>
  MARL_NO_EXPORT inline void onCall(F&& f) const;

 private:
  // Internal doubly-linked-list data structure. One per ticket instance.
  struct Record {
    MARL_NO_EXPORT inline ~Record();

    MARL_NO_EXPORT inline void done();
    MARL_NO_EXPORT inline void callAndUnlock(marl::lock& lock);
    MARL_NO_EXPORT inline void unlink();  // guarded by shared->mutex

    ConditionVariable isCalledCondVar;

    std::shared_ptr<Shared> shared;
    Record* next = nullptr;  // guarded by shared->mutex
    Record* prev = nullptr;  // guarded by shared->mutex
    OnCall onCall;           // guarded by shared->mutex
    bool isCalled = false;   // guarded by shared->mutex
    std::atomic<bool> isDone = {false};
  };

  // Data shared between all tickets and the queue.
  struct Shared {
    marl::mutex mutex;
    Record tail;
  };

  MARL_NO_EXPORT inline Ticket(Loan<Record>&& record);

  Loan<Record> record;
};

////////////////////////////////////////////////////////////////////////////////
// Ticket
////////////////////////////////////////////////////////////////////////////////

Ticket::Ticket(Loan<Record>&& record) : record(std::move(record)) {}

void Ticket::wait() const {
  marl::lock lock(record->shared->mutex);
  record->isCalledCondVar.wait(lock, [this] { return record->isCalled; });
}

void Ticket::done() const {
  record->done();
}

template <typename Function>
void Ticket::onCall(Function&& f) const {
  marl::lock lock(record->shared->mutex);
  if (record->isCalled) {
    marl::schedule(std::forward<Function>(f));
    return;
  }
  if (record->onCall) {
    struct Joined {
      void operator()() const {
        a();
        b();
      }
      OnCall a, b;
    };
    record->onCall =
        std::move(Joined{std::move(record->onCall), std::forward<Function>(f)});
  } else {
    record->onCall = std::forward<Function>(f);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Ticket::Queue
////////////////////////////////////////////////////////////////////////////////

Ticket Ticket::Queue::take() {
  Ticket out;
  take(1, [&](Ticket&& ticket) { out = std::move(ticket); });
  return out;
}

template <typename F>
void Ticket::Queue::take(size_t n, const F& f) {
  Loan<Record> first, last;
  pool.borrow(n, [&](Loan<Record>&& record) {
    Loan<Record> rec = std::move(record);
    rec->shared = shared;
    if (first.get() == nullptr) {
      first = rec;
    }
    if (last.get() != nullptr) {
      last->next = rec.get();
      rec->prev = last.get();
    }
    last = rec;
    f(std::move(Ticket(std::move(rec))));
  });
  last->next = &shared->tail;
  marl::lock lock(shared->mutex);
  first->prev = shared->tail.prev;
  shared->tail.prev = last.get();
  if (first->prev == nullptr) {
    first->callAndUnlock(lock);
  } else {
    first->prev->next = first.get();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Ticket::Record
////////////////////////////////////////////////////////////////////////////////

Ticket::Record::~Record() {
  if (shared != nullptr) {
    done();
  }
}

void Ticket::Record::done() {
  if (isDone.exchange(true)) {
    return;
  }
  marl::lock lock(shared->mutex);
  auto callNext = (prev == nullptr && next != nullptr) ? next : nullptr;
  unlink();
  if (callNext != nullptr) {
    // lock needs to be held otherwise callNext might be destructed.
    callNext->callAndUnlock(lock);
  }
}

void Ticket::Record::callAndUnlock(marl::lock& lock) {
  if (isCalled) {
    return;
  }
  isCalled = true;
  OnCall callback;
  std::swap(callback, onCall);
  isCalledCondVar.notify_all();
  lock.unlock_no_tsa();

  if (callback) {
    marl::schedule(std::move(callback));
  }
}

void Ticket::Record::unlink() {
  if (prev != nullptr) {
    prev->next = next;
  }
  if (next != nullptr) {
    next->prev = prev;
  }
  prev = nullptr;
  next = nullptr;
}

}  // namespace marl

#endif  // marl_ticket_h
