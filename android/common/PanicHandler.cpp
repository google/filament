//
// Created by Claude Code on 24/12/25.
//

#include "PanicHandler.h"
#include <android/log.h>
#include <sstream>

namespace filament::android {

void PanicHandler::initialize() noexcept {
    utils::Panic::setPanicHandler(&PanicHandler::handlePanic, nullptr);
    __android_log_print(ANDROID_LOG_INFO, "FilamentPanic",
        "Panic handler registered - worker thread recovery enabled");
}

void PanicHandler::shutdown() noexcept {
    utils::Panic::setPanicHandler(nullptr, nullptr);
    __android_log_print(ANDROID_LOG_INFO, "FilamentPanic",
        "Panic handler unregistered");
}

void PanicHandler::handlePanic(void* user, utils::Panic const& panic) noexcept {
    // Log the panic with full details
    __android_log_print(ANDROID_LOG_ERROR, "FilamentPanic",
        "PANIC INTERCEPTED:");
    __android_log_print(ANDROID_LOG_ERROR, "FilamentPanic",
        "  Type: %s", panic.getType());
    __android_log_print(ANDROID_LOG_ERROR, "FilamentPanic",
        "  Location: %s", panic.getFunction());
    __android_log_print(ANDROID_LOG_ERROR, "FilamentPanic",
        "  File: %s:%d", panic.getFile(), panic.getLine());
    __android_log_print(ANDROID_LOG_ERROR, "FilamentPanic",
        "  Reason: %s", panic.getReason());

    // Build detailed error message for exception queue
    std::ostringstream oss;
    oss << panic.getType() << " in " << panic.getFunction()
        << " at " << panic.getFile() << ":" << panic.getLine()
        << " - " << panic.getReason();

    // Store panic information in the exception queue
    // This ensures it's available via healthCheck() even if thread is killed
    ThreadException ex{
        .threadId = std::this_thread::get_id(),
        .location = panic.getFunction(),
        .message = oss.str(),
        .type = ThreadException::Type::PANIC
    };
    ThreadExceptionQueue::store(ex);

    // Mark Filament as disabled to prevent future operations
    PanicState::report(panic.getFunction(), panic.getReason());

    // AGGRESSIVE RECOVERY: Kill worker threads, preserve main thread
    const bool isMainThread = MainThreadDetector::isMainThread();

    if (!isMainThread) {
        // This is a worker thread - attempt to kill ONLY this thread
        __android_log_print(ANDROID_LOG_WARN, "FilamentPanic",
            "Worker thread panic detected - terminating thread to prevent app crash");
        __android_log_print(ANDROID_LOG_WARN, "FilamentPanic",
            "WARNING: Thread killed without cleanup - potential resource leaks");

        // Use pthread_exit to terminate just this thread
        // CRITICAL NOTES:
        // - Destructors will NOT run (resource leaks possible)
        // - If thread holds locks, other threads may deadlock
        // - If other threads wait on this thread, they'll hang
        // - However, PanicState has disabled all Filament operations
        // - Better than crashing the entire app
        pthread_exit(nullptr);
        // Never returns
    }

    // Main thread path: Return normally and let panic propagate
    // - If in non-noexcept context: panic will throw, jniGuard will catch it
    // - If in noexcept context: std::terminate() will be called (unavoidable)
    //
    // We don't kill the main thread because:
    // 1. Killing main thread would crash the app anyway
    // 2. std::terminate() provides cleaner crash with stack trace
    // 3. Android system can handle process termination properly
    __android_log_print(ANDROID_LOG_ERROR, "FilamentPanic",
        "Main thread panic - returning to default handler");

    // Return normally - execution continues to the throw/abort in Filament's panic system
}

} // namespace filament::android
