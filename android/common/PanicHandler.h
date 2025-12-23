//
// Created by Claude Code on 24/12/25.
//

#pragma once
#include <utils/Panic.h>
#include <pthread.h>
#include "ThreadExceptionBridge.h"
#include "PanicState.h"

namespace filament::android {

/**
 * PanicHandler intercepts all Filament panics and attempts to prevent app crashes
 * by storing panic information and selectively killing only worker threads.
 *
 * Behavior:
 * - Non-noexcept functions: Panic stored → thrown → caught by jniGuard → Java exception
 * - Worker threads (noexcept): Panic stored → thread killed via pthread_exit() → app survives
 * - Main thread (noexcept): Panic stored → std::terminate() → app crashes (safer)
 *
 * WARNING: Worker thread killing is aggressive recovery with potential side effects:
 * - Resource leaks (destructors don't run)
 * - Deadlocks (if thread held locks)
 * - Zombies (if other threads wait on killed thread)
 *
 * However, PanicState disables all future Filament operations after panic, limiting damage.
 */
class PanicHandler {
public:
    /**
     * Initialize and register the global panic handler.
     * Should be called once during JNI_OnLoad.
     */
    static void initialize() noexcept;

    /**
     * Unregister the panic handler.
     * Should be called during JNI_OnUnload (optional).
     */
    static void shutdown() noexcept;

private:
    /**
     * The actual panic handler callback invoked by Filament's panic system.
     * This is called synchronously before the panic is thrown or abort() is called.
     *
     * @param user User-provided context pointer (unused, always nullptr)
     * @param panic The panic exception object containing error details
     */
    static void handlePanic(void* user, utils::Panic const& panic) noexcept;
};

} // namespace filament::android
