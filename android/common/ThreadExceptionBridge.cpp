//
// Created by Rohit T P on 23/12/25.
//

#include "ThreadExceptionBridge.h"
#include <sstream>

namespace filament::android {

// ============================================================================
// MainThreadDetector Implementation
// ============================================================================

std::atomic<std::thread::id> MainThreadDetector::sMainThreadId{std::thread::id{}};
thread_local bool MainThreadDetector::sIsMainThread = false;

void MainThreadDetector::initialize() noexcept {
    sMainThreadId.store(std::this_thread::get_id(), std::memory_order_release);
}

bool MainThreadDetector::isMainThread() noexcept {
    // Check thread-local flag first (fast path for threads marked by jniGuard)
    if (sIsMainThread) {
        return true;
    }

    // Check if this thread matches the cached main thread ID
    auto mainId = sMainThreadId.load(std::memory_order_acquire);
    if (mainId != std::thread::id{} && std::this_thread::get_id() == mainId) {
        return true;
    }

    return false;
}

void MainThreadDetector::markAsMainThread() noexcept {
    sIsMainThread = true;

    // Also update the global main thread ID if not set
    std::thread::id expected{};
    sMainThreadId.compare_exchange_strong(expected, std::this_thread::get_id(),
                                          std::memory_order_release);
}

// ============================================================================
// ThreadExceptionQueue Implementation
// ============================================================================

std::mutex ThreadExceptionQueue::sMutex;
std::deque<ThreadException> ThreadExceptionQueue::sExceptions;
std::atomic<bool> ThreadExceptionQueue::sHasException{false};

void ThreadExceptionQueue::store(const ThreadException& ex) noexcept {
    try {
        std::lock_guard<std::mutex> lock(sMutex);
        sExceptions.push_back(ex);
        sHasException.store(true, std::memory_order_release);
    } catch (...) {
        // If we can't even store the exception, there's nothing we can do
        // PanicState has already been notified
    }
}

std::optional<ThreadException> ThreadExceptionQueue::take() noexcept {
    if (!sHasException.load(std::memory_order_acquire)) {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(sMutex);
    if (sExceptions.empty()) {
        sHasException.store(false, std::memory_order_release);
        return std::nullopt;
    }

    ThreadException ex = std::move(sExceptions.front());
    sExceptions.pop_front();

    if (sExceptions.empty()) {
        sHasException.store(false, std::memory_order_release);
    }

    return ex;
}

bool ThreadExceptionQueue::hasException() noexcept {
    return sHasException.load(std::memory_order_acquire);
}

// ============================================================================
// JNI Helper Functions
// ============================================================================

static void throwFilamentPanicException(JNIEnv* env, const std::string& msg) {
    jclass cls = env->FindClass("com/google/android/filament/FilamentPanicException");
    if (!cls) {
        env->ExceptionClear(); // avoid recursive exception states
        return;
    }
    env->ThrowNew(cls, msg.c_str());
}

static void throwFilamentNativeException(JNIEnv* env, const std::string& msg) {
    jclass cls = env->FindClass("com/google/android/filament/FilamentNativeException");
    if (!cls) {
        env->ExceptionClear(); // avoid recursive exception states
        return;
    }
    env->ThrowNew(cls, msg.c_str());
}

void throwStoredExceptionIfAny(JNIEnv* env) noexcept {
    auto maybeEx = ThreadExceptionQueue::take();
    if (!maybeEx.has_value()) {
        return;
    }

    const ThreadException& ex = maybeEx.value();

    // Build detailed error message
    std::ostringstream oss;
    oss << "Filament worker thread exception\n"
        << "Location: " << ex.location << "\n"
        << "Thread ID: " << ex.threadId << "\n"
        << "Type: ";

    switch (ex.type) {
        case ThreadException::Type::PANIC:
            oss << "Panic";
            break;
        case ThreadException::Type::STD_EXCEPTION:
            oss << "std::exception";
            break;
        case ThreadException::Type::UNKNOWN:
            oss << "Unknown";
            break;
    }

    oss << "\nMessage: " << ex.message;

    // Throw appropriate custom exception type
    if (ex.type == ThreadException::Type::PANIC) {
        throwFilamentPanicException(env, oss.str());
    } else {
        throwFilamentNativeException(env, oss.str());
    }
}

} // namespace filament::android
