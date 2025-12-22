//
// Created by Rohit T P on 23/12/25.
//

#include "PanicState.h"

namespace filament::android {

std::atomic<bool> PanicState::sDisabled{false};
std::atomic<bool> PanicState::sHasMessage{false};
std::mutex PanicState::sMutex;
std::string PanicState::sMessage;

void PanicState::report(const char* where, const char* msg) noexcept {
    sDisabled.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(sMutex);
        sMessage = (where ? std::string(where) : std::string("filament")) +
                   ": " +
                   (msg ? msg : "unknown error");
    }
    sHasMessage.store(true, std::memory_order_release);
}

bool PanicState::isDisabled() noexcept {
    return sDisabled.load(std::memory_order_acquire);
}

std::string PanicState::takeMessage() noexcept {
    if (!sHasMessage.exchange(false, std::memory_order_acq_rel)) {
        return {};
    }
    std::lock_guard<std::mutex> lock(sMutex);
    std::string out = sMessage;
    sMessage.clear();
    return out;
}

} // namespace filament::android


