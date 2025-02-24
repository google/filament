/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <atomic>
#include <mutex>
#include <cassert>

// TODO https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9384
// Fix GCC 13 issues with regex
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include <regex>
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic pop
#endif

#include "error_message/log_message_type.h"
#include <vulkan/vulkan_core.h>

// ErrorMonitor Usage:
//
// Call SetDesiredFailureMsg with a string to be compared against all
// encountered log messages, or a validation error enum identifying
// desired error message. Passing NULL or VALIDATION_ERROR_MAX_ENUM
// will match all log messages. logMsg will return true for skipCall
// only if msg is matched or NULL.
//
// Call VerifyFound to determine if all desired failure messages
// were encountered. Call VerifyNotFound to determine if any unexpected
// failure was encountered.
class ErrorMonitor {
  public:
    ErrorMonitor(bool print_all_errors);
    ~ErrorMonitor() noexcept = default;

    void CreateCallback(VkInstance instance) noexcept;
    void DestroyCallback(VkInstance instance) noexcept;

    ErrorMonitor(const ErrorMonitor &) = delete;
    ErrorMonitor &operator=(const ErrorMonitor &) = delete;
    ErrorMonitor(ErrorMonitor &&) noexcept = delete;
    ErrorMonitor &operator=(ErrorMonitor &&) noexcept = delete;

    // Set monitor to pristine state
    void Reset();

    // ErrorMonitor will look for an error message containing the specified string(s)
    void SetDesiredFailureMsg(const VkFlags msg_flags, const char *const msg_string);
    void SetDesiredFailureMsg(const VkFlags msg_flags, const std::string &msg);
    void SetDesiredFailureMsgRegex(const VkFlags msg_flags, const char *vuid, std::string regex_str);
    // Any error message matching undesired_regex_str will provoke a test failure
    void SetDesiredFailureMsgRegex(const VkFlags msg_flags, const char *vuid, std::string regex_str,
                                   std::string undesired_regex_str);
    // Most tests check for kErrorBit so default to just using it
    void SetDesiredError(const char *msg, uint32_t count = 1);
    // Regex uses modified ECMAScript regular expression grammar https://eel.is/c++draft/re.grammar
    void SetDesiredErrorRegex(const char *vuid, std::string regex_str, uint32_t count = 1);
    // And use this for
    void SetDesiredWarningRegex(const char *vuid, std::string regex_str, uint32_t count = 1);
    void SetDesiredWarning(const char *msg, uint32_t count = 1);
    void SetDesiredInfo(const char *msg, uint32_t count = 1);

    // Set an error that the error monitor will ignore. Do not use this function if you are creating a new test.
    // TODO: This is stopgap to block new unexpected errors from being introduced. The long-term goal is to remove the use of this
    // function and its definition.
    void SetUnexpectedError(const char *const msg);

    // Set an error that should not cause a test failure
    void SetAllowedFailureMsg(const char *const msg);

    VkBool32 CheckForDesiredMsg(const char *vuid, const char *const msg_string);
    VkDebugReportFlagsEXT GetMessageFlags();
    void SetError(const char *const errorString);
    void SetBailout(std::atomic<bool> *bailout);

    // Helpers
    void VerifyFound();
    void Finish();

    const auto *GetDebugCreateInfo() const { return &debug_create_info_; }

    // ExpectSuccess now takes an optional argument allowing a custom combination of debug flags
    void ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask = kErrorBit);

  private:
    void SetDesiredFailureMsgRegex(const VkFlags msg_flags, const char *vuid, std::string msg_regex, std::regex regex);
    bool ExpectingSuccess() const;
    bool NeedCheckSuccess() const;
    void VerifyNotFound();
    // TODO: This is stopgap to block new unexpected errors from being introduced. The long-term goal is to remove the use of this
    // function and its definition.
    bool IgnoreMessage(std::string const &msg) const;
    bool AnyDesiredMsgFound() const;
    void MonitorReset();
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(mutex_); }

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkDebugUtilsMessengerEXT debug_obj_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info_{};
#else
    VkDebugReportCallbackEXT debug_obj_ = VK_NULL_HANDLE;
    VkDebugReportCallbackCreateInfoEXT debug_create_info_{};
#endif

    VkFlags message_flags_{};
    std::vector<std::string> failure_message_strings_;
    struct VuidAndMessage {
        std::string_view vuid;
        enum MsgType { Undefined, String, Regex } msg_type = Undefined;
        std::string msg_string;  // also used to store regex string
        std::regex msg_regex;
        std::regex undesired_msg_regex;
        std::string undesired_msg_regex_string;
        void SetMsgString(std::string msg) {
            msg_type = MsgType::String;
            msg_string = std::move(msg);
        }
        void SetMsgRegex(std::string regex_str, std::regex regex) {
            msg_type = MsgType::Regex;
            msg_string = std::move(regex_str);
            msg_regex = std::move(regex);
        }
        void SetUndesiredMsgRegex(std::string undesired_regex_str, std::regex undesired_regex) {
            undesired_msg_regex_string = std::move(undesired_regex_str);
            undesired_msg_regex = std::move(undesired_regex);
        }
        bool Search(const char *vuid, std::string_view msg) const;
        bool SearchUndesiredRegex(std::string_view msg) const;
        std::string Print() const;
    };
    std::vector<VuidAndMessage> desired_messages_;
    std::vector<VuidAndMessage> undesired_messages_;
    std::vector<std::string> ignore_message_strings_;
    std::vector<std::string> allowed_message_strings_;
    mutable std::mutex mutex_;
    std::atomic<bool> *bailout_{};
    bool message_found_{};
    bool print_all_errors_{};
};
