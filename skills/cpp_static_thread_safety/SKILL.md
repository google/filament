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

*   **Standard Primitives (`std::mutex` & `std::condition_variable`)**:
    *   *Use Case*: Heavy producer-consumer queues or structures performing blocking wait loops (`wait()`).
    *   *Rationale*: Required for native kernel thread scheduling, priority queues, and preventing priority inversion stutters via Priority Inheritance (PI).
*   **Lightweight Primitives (`utils::Mutex` & `utils::Condition`)**:
    *   *Use Case*: High-frequency, low-contention locks guarding simple structures (like hashmap lookups or state changes) with **zero condition waits**.
    *   *Rationale*: Custom 4-byte futex wrappers (compared to standard mutex's 40 bytes) that minimize cache-line footprint in hot render paths.

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
