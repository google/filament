//
// Created by Rohit T P on 23/12/25.
//

#pragma once
#include <jni.h>
#include <string>
#include <type_traits>
#include <functional>
#include <thread>
#include <mutex>
#include <deque>
#include <atomic>
#include <optional>
#include <utils/Panic.h>
#include "PanicState.h"

namespace filament::android {

// ============================================================================
// Main Thread Detection
// ============================================================================

class MainThreadDetector {
public:
    // Called once during JNI_OnLoad to cache main thread ID
    static void initialize() noexcept;

    // Check if current thread is the main thread
    static bool isMainThread() noexcept;

    // Mark current thread as main thread (used by jniGuard)
    static void markAsMainThread() noexcept;

private:
    // Store main thread ID
    static std::atomic<std::thread::id> sMainThreadId;
    static thread_local bool sIsMainThread;
};

// ============================================================================
// Thread Exception Storage
// ============================================================================

struct ThreadException {
    std::thread::id threadId;
    std::string location;
    std::string message;
    enum class Type { PANIC, STD_EXCEPTION, UNKNOWN } type;
};

class ThreadExceptionQueue {
public:
    // Store an exception from a worker thread (thread-safe)
    static void store(const ThreadException& ex) noexcept;

    // Take the first stored exception (clears it from queue)
    // Returns empty optional if no exception exists
    static std::optional<ThreadException> take() noexcept;

    // Check if any exception is stored
    static bool hasException() noexcept;

private:
    static std::mutex sMutex;
    static std::deque<ThreadException> sExceptions;
    static std::atomic<bool> sHasException;
};

// ============================================================================
// Thread Guard Wrapper - Main API
// ============================================================================

// For functions that return a value
template <typename Ret, typename Fn>
inline Ret runThreadGuarded(const char* where, Ret defaultValue, Fn&& fn) {
    // Check if Filament is already disabled (fast path)
    if (PanicState::isDisabled()) {
        // On worker thread, just return default; main thread will get exception via healthCheck
        return defaultValue;
    }

    const bool isMain = MainThreadDetector::isMainThread();

    try {
        return fn();

    } catch (const utils::Panic& p) {
        if (isMain) {
            // Main thread: exception will propagate to Java immediately (caller must be jniGuard)
            throw;
        } else {
            // Worker thread: store and disable Filament
            ThreadException ex{
                .threadId = std::this_thread::get_id(),
                .location = where ? where : "unknown",
                .message = p.what(),
                .type = ThreadException::Type::PANIC
            };
            ThreadExceptionQueue::store(ex);
            PanicState::report(where, p.what());
        }

    } catch (const std::exception& e) {
        if (isMain) {
            throw;
        } else {
            ThreadException ex{
                .threadId = std::this_thread::get_id(),
                .location = where ? where : "unknown",
                .message = e.what(),
                .type = ThreadException::Type::STD_EXCEPTION
            };
            ThreadExceptionQueue::store(ex);
            PanicState::report(where, e.what());
        }

    } catch (...) {
        if (isMain) {
            throw;
        } else {
            ThreadException ex{
                .threadId = std::this_thread::get_id(),
                .location = where ? where : "unknown",
                .message = "unknown exception",
                .type = ThreadException::Type::UNKNOWN
            };
            ThreadExceptionQueue::store(ex);
            PanicState::report(where, "unknown exception");
        }
    }

    return defaultValue;
}

// For void functions
template <typename Fn>
inline void runThreadGuardedVoid(const char* where, Fn&& fn) {
    if (PanicState::isDisabled()) {
        return;
    }

    const bool isMain = MainThreadDetector::isMainThread();

    try {
        fn();

    } catch (const utils::Panic& p) {
        if (isMain) {
            throw;
        } else {
            ThreadException ex{
                .threadId = std::this_thread::get_id(),
                .location = where ? where : "unknown",
                .message = p.what(),
                .type = ThreadException::Type::PANIC
            };
            ThreadExceptionQueue::store(ex);
            PanicState::report(where, p.what());
        }

    } catch (const std::exception& e) {
        if (isMain) {
            throw;
        } else {
            ThreadException ex{
                .threadId = std::this_thread::get_id(),
                .location = where ? where : "unknown",
                .message = e.what(),
                .type = ThreadException::Type::STD_EXCEPTION
            };
            ThreadExceptionQueue::store(ex);
            PanicState::report(where, e.what());
        }

    } catch (...) {
        if (isMain) {
            throw;
        } else {
            ThreadException ex{
                .threadId = std::this_thread::get_id(),
                .location = where ? where : "unknown",
                .message = "unknown exception",
                .type = ThreadException::Type::UNKNOWN
            };
            ThreadExceptionQueue::store(ex);
            PanicState::report(where, "unknown exception");
        }
    }
}

// ============================================================================
// JNI Health Check
// ============================================================================

// JNI Health Check - callable from Java as static method
// Throws Java exception if worker thread exception is stored
void throwStoredExceptionIfAny(JNIEnv* env) noexcept;

} // namespace filament::android
