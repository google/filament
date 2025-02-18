// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_RWMUTEX_H_
#define SRC_TINT_UTILS_RWMUTEX_H_

#include <condition_variable>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////
// RWMutex
////////////////////////////////////////////////////////////////////////////////

namespace tint::socket {

/// A RWMutex is a reader/writer mutual exclusion lock.
/// The lock can be held by an arbitrary number of readers or a single writer.
/// Also known as a shared mutex.
class RWMutex {
  public:
    inline RWMutex() = default;

    /// LockReader() locks the mutex for reading.
    /// Multiple read locks can be held while there are no writer locks.
    inline void LockReader();

    /// UnlockReader() unlocks the mutex for reading.
    inline void UnlockReader();

    /// LockWriter() locks the mutex for writing.
    /// If the lock is already locked for reading or writing, LockWriter blocks
    /// until the lock is available.
    inline void LockWriter();

    /// UnlockWriter() unlocks the mutex for writing.
    inline void UnlockWriter();

  private:
    RWMutex(const RWMutex&) = delete;
    RWMutex& operator=(const RWMutex&) = delete;

    int read_locks = 0;
    int pending_write_locks = 0;
    std::mutex mutex;
    std::condition_variable cv;
};

void RWMutex::LockReader() {
    std::unique_lock<std::mutex> lock(mutex);
    read_locks++;
}

void RWMutex::UnlockReader() {
    std::unique_lock<std::mutex> lock(mutex);
    read_locks--;
    if (read_locks == 0 && pending_write_locks > 0) {
        cv.notify_one();
    }
}

void RWMutex::LockWriter() {
    std::unique_lock<std::mutex> lock(mutex);
    if (read_locks > 0) {
        pending_write_locks++;
        cv.wait(lock, [&] { return read_locks == 0; });
        pending_write_locks--;
    }
    lock.release();  // Keep lock held
}

void RWMutex::UnlockWriter() {
    if (pending_write_locks > 0) {
        cv.notify_one();
    }
    mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////
// RLock
////////////////////////////////////////////////////////////////////////////////

/// RLock is a RAII read lock helper for a RWMutex.
class RLock {
  public:
    /// Constructor.
    /// Locks `mutex` with a read-lock for the lifetime of the WLock.
    /// @param mutex the mutex
    explicit inline RLock(RWMutex& mutex);
    /// Destructor.
    /// Unlocks the RWMutex.
    inline ~RLock();

    /// Move constructor
    /// @param other the other RLock to move into this RLock.
    inline RLock(RLock&& other);
    /// Move assignment operator
    /// @param other the other RLock to move into this RLock.
    /// @returns this RLock so calls can be chained
    inline RLock& operator=(RLock&& other);

  private:
    RLock(const RLock&) = delete;
    RLock& operator=(const RLock&) = delete;

    RWMutex* m;
};

RLock::RLock(RWMutex& mutex) : m(&mutex) {
    m->LockReader();
}

RLock::~RLock() {
    if (m != nullptr) {
        m->UnlockReader();
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

/// WLock is a RAII write lock helper for a RWMutex.
class WLock {
  public:
    /// Constructor.
    /// Locks `mutex` with a write-lock for the lifetime of the WLock.
    /// @param mutex the mutex
    explicit inline WLock(RWMutex& mutex);

    /// Destructor.
    /// Unlocks the RWMutex.
    inline ~WLock();

    /// Move constructor
    /// @param other the other WLock to move into this WLock.
    inline WLock(WLock&& other);
    /// Move assignment operator
    /// @param other the other WLock to move into this WLock.
    /// @returns this WLock so calls can be chained
    inline WLock& operator=(WLock&& other);

  private:
    WLock(const WLock&) = delete;
    WLock& operator=(const WLock&) = delete;

    RWMutex* m;
};

WLock::WLock(RWMutex& mutex) : m(&mutex) {
    m->LockWriter();
}

WLock::~WLock() {
    if (m != nullptr) {
        m->UnlockWriter();
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

}  // namespace tint::socket

#endif  // SRC_TINT_UTILS_RWMUTEX_H_
