// Copyright 2020 Google LLC
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

#ifndef dap_rwmutex_h
#define dap_rwmutex_h

#include <condition_variable>
#include <mutex>

namespace dap {

////////////////////////////////////////////////////////////////////////////////
// RWMutex
////////////////////////////////////////////////////////////////////////////////

// A RWMutex is a reader/writer mutual exclusion lock.
// The lock can be held by an arbitrary number of readers or a single writer.
// Also known as a shared mutex.
class RWMutex {
 public:
  inline RWMutex() = default;

  // lockReader() locks the mutex for reading.
  // Multiple read locks can be held while there are no writer locks.
  inline void lockReader();

  // unlockReader() unlocks the mutex for reading.
  inline void unlockReader();

  // lockWriter() locks the mutex for writing.
  // If the lock is already locked for reading or writing, lockWriter blocks
  // until the lock is available.
  inline void lockWriter();

  // unlockWriter() unlocks the mutex for writing.
  inline void unlockWriter();

 private:
  RWMutex(const RWMutex&) = delete;
  RWMutex& operator=(const RWMutex&) = delete;

  int readLocks = 0;
  int pendingWriteLocks = 0;
  std::mutex mutex;
  std::condition_variable cv;
};

void RWMutex::lockReader() {
  std::unique_lock<std::mutex> lock(mutex);
  readLocks++;
}

void RWMutex::unlockReader() {
  std::unique_lock<std::mutex> lock(mutex);
  readLocks--;
  if (readLocks == 0 && pendingWriteLocks > 0) {
    cv.notify_one();
  }
}

void RWMutex::lockWriter() {
  std::unique_lock<std::mutex> lock(mutex);
  if (readLocks > 0) {
    pendingWriteLocks++;
    cv.wait(lock, [&] { return readLocks == 0; });
    pendingWriteLocks--;
  }
  lock.release();  // Keep lock held
}

void RWMutex::unlockWriter() {
  if (pendingWriteLocks > 0) {
    cv.notify_one();
  }
  mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////
// RLock
////////////////////////////////////////////////////////////////////////////////

// RLock is a RAII read lock helper for a RWMutex.
class RLock {
 public:
  inline RLock(RWMutex& mutex);
  inline ~RLock();

  inline RLock(RLock&&);
  inline RLock& operator=(RLock&&);

 private:
  RLock(const RLock&) = delete;
  RLock& operator=(const RLock&) = delete;

  RWMutex* m;
};

RLock::RLock(RWMutex& mutex) : m(&mutex) {
  m->lockReader();
}

RLock::~RLock() {
  if (m != nullptr) {
    m->unlockReader();
  }
}

RLock::RLock(RLock&& other) {
  m = other.m;
  other.m = nullptr;
}

RLock& RLock::operator=(RLock&& other) {
  m = other.m;
  other.m = nullptr;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// WLock
////////////////////////////////////////////////////////////////////////////////

// WLock is a RAII write lock helper for a RWMutex.
class WLock {
 public:
  inline WLock(RWMutex& mutex);
  inline ~WLock();

  inline WLock(WLock&&);
  inline WLock& operator=(WLock&&);

 private:
  WLock(const WLock&) = delete;
  WLock& operator=(const WLock&) = delete;

  RWMutex* m;
};

WLock::WLock(RWMutex& mutex) : m(&mutex) {
  m->lockWriter();
}

WLock::~WLock() {
  if (m != nullptr) {
    m->unlockWriter();
  }
}

WLock::WLock(WLock&& other) {
  m = other.m;
  other.m = nullptr;
}

WLock& WLock::operator=(WLock&& other) {
  m = other.m;
  other.m = nullptr;
  return *this;
}
}  // namespace dap

#endif
