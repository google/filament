---
name: cpp-static-thread-safety
description: >
  Enforce C++ static thread safety annotations and correct synchronization primitives.
  Use this skill when designing multi-threaded classes or editing guarded member fields.
---

# C++ Static Thread Safety & Synchronization Guidelines

Filament leverages Clang's static thread safety analysis to verify lock holding requirements at compile-time. All multi-threaded classes must use explicit capability annotations to guarantee race-free state access.

---

## 1. Selecting the Correct Lock Primitive

*   **Filament Primitives (`utils::Mutex` & `utils::Condition`) — MANDATORY**:
    *   *Rule*: All engine, backend, and utility code under `filament/` must use `utils::Mutex` and `utils::Condition` exclusively. Do **not** use `std::mutex` or `std::condition_variable`.
    *   *Deadlock & Order Debugging*: `utils::Mutex` transparently integrates with Filament's compile-time lock debugging facility (`FILAMENT_DEBUG_MUTEX` / `-u`). When enabled, it maintains a global cycle dependency graph via BFS during `lock()` and `try_lock()`, immediately trapping lock-order inversions and self-deadlocks with exact `CallStack` traces. Any locks defined via `std::mutex` bypass this tracker entirely and remain invisible to deadlock diagnostics.
    *   *Memory & Cache Hygiene*: On Android and Linux (`linuxutil::Mutex`), `utils::Mutex` is only 4 bytes (a single atomic futex word) versus 40 bytes for `std::mutex`. For structures allocated in large volumes or embedded in handles/fences, this 10x size reduction prevents struct bloat and maintains cache-line density.
    *   *Condition Variable Support*: `utils::Condition` (`Condition::wait` / `wait_until`) is explicitly templated (`template <typename M>`) to work seamlessly with `UniqueLock<utils::Mutex>` for producer-consumer queues and blocking wait loops.
    *   *Priority Inversion Reality*: C++ standard `std::mutex` (`pthread_mutex_t`) does **not** provide priority inheritance out of the box (`PTHREAD_PRIO_NONE`). Therefore, `std::mutex` offers zero priority inversion protection over `utils::Mutex`.

---

## 2. Lock Guard & RAII Lifetime Conventions

*   **Immutability Invariant**: Use `LockGuard const` as the default for all standard synchronized blocks to guarantee scope-bound read-only lock scopes.
    ```cpp
    // Correct
    utils::LockGuard const lock(mLock);
    ```
*   **Condition Variables Overloads**: Use `UniqueLock` (non-const) strictly when passing locks to condition variables (`wait(lock)`) or when explicit `.unlock()` / `.lock()` boundaries are required for performance or deadlock prevention.
*   **Lock Dependency**: Any source file (`.cpp`) that instantiates `LockGuard` or `UniqueLock` **must** explicitly include the matching utility header:
    ```cpp
    #include <utils/Mutex.h>
    ```

---

## 3. Resolving Clang's Lambda Closure Limitations

Clang evaluates C++ anonymous closures (lambdas) as separate context boundaries. Because lambdas lack capability attributes, standard condition variable waits passing local predicates (e.g., using `std::ranges::all_of`) will trigger false-positive thread safety errors.

To resolve this, use one of the following approved patterns:

### Pattern A: Inner Lambda Bypass (Recommended)
Decorate *only* the CV wait predicate lambda operator with `UTILS_NO_THREAD_SAFETY_ANALYSIS` to ignore nested boundaries while preserving outer compile-time checks:
```cpp
UniqueLock lock(mQueueLock);
mQueueCondition.wait(lock, [this]() UTILS_NO_THREAD_SAFETY_ANALYSIS {
    return mExitRequested || 
           (!std::ranges::all_of(mQueues, [](auto&& q) { return q.empty(); }));
});
```

### Pattern B: Manual Loop Inlining
If the check is flat, completely inline the CV predicate as a standard `while` loop to bring the member variables directly into the parent function's locked scope:
```cpp
UniqueLock lock(mLock);
while (mFreeSpace < requiredSize) {
    mCondition.wait(lock);
}
```

---

## 4. Dynamic Threading & Preprocessor Safety

*   **Single-Threaded Parity**: All thread safety annotations (`UTILS_GUARDED_BY`) are conditionally compiled out on single-threaded configurations. To prevent compile crashes when `FILAMENT_SINGLE_THREADED` is defined, standard annotations are gated by `UTILS_HAS_THREADING` in `compiler.h`.
