//
// Created by Rohit T P on 23/12/25.
//

#pragma once
#include <atomic>
#include <mutex>
#include <string>

namespace filament::android {

struct PanicState {
    static void report(const char* where, const char* msg) noexcept;
    static bool isDisabled() noexcept;

    // Returns empty string if none. Clears stored error.
    static std::string takeMessage() noexcept;

private:
    static std::atomic<bool> sDisabled;
    static std::atomic<bool> sHasMessage;
    static std::mutex sMutex;
    static std::string sMessage;
};

} // namespace filament::android

