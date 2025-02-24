/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
#include "error_monitor.h"
#include "test_common.h"
#include "error_message/log_message_type.h"
#include "generated/vk_function_pointers.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Note. VK_EXT_debug_report is deprecated by the VK_EXT_debug_utils extension.
// However, we still support this old extension due to CI running old Android devices.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT, uint64_t,
                                                    size_t, int32_t, const char *, const char *message, void *user_data) {
    auto *error_monitor = reinterpret_cast<ErrorMonitor *>(user_data);

    if (message_flags & error_monitor->GetMessageFlags()) {
        return error_monitor->CheckForDesiredMsg("", message);
    }
    return VK_FALSE;
}
#else

static inline LogMessageTypeFlags DebugAnnotFlagsToMsgTypeFlags(VkDebugUtilsMessageSeverityFlagBitsEXT da_severity,
                                                                VkDebugUtilsMessageTypeFlagsEXT da_type) {
    LogMessageTypeFlags msg_type_flags = 0;
    if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        msg_type_flags |= kErrorBit;
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        if ((da_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
            msg_type_flags |= kPerformanceWarningBit;
        } else {
            msg_type_flags |= kWarningBit;
        }
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        msg_type_flags |= kInformationBit;
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
        msg_type_flags |= kVerboseBit;
    }
    return msg_type_flags;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
    const auto message_flags = DebugAnnotFlagsToMsgTypeFlags(message_severity, message_types);
    const char *vuid = callback_data->pMessageIdName;
    const char *message = callback_data->pMessage;
    auto *error_monitor = reinterpret_cast<ErrorMonitor *>(user_data);

    if (message_flags & error_monitor->GetMessageFlags()) {
        return error_monitor->CheckForDesiredMsg(vuid, message);
    }
    return VK_FALSE;
}
#endif

ErrorMonitor::ErrorMonitor(bool print_all_errors) : print_all_errors_(print_all_errors) {
    MonitorReset();
    ExpectSuccess(kErrorBit);
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    debug_create_info_ = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                          nullptr,
                          0,
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                          &DebugCallback,
                          this};
#else
    debug_create_info_ = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, nullptr,
                          VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                              VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                              VK_DEBUG_REPORT_DEBUG_BIT_EXT,
                          &DebugCallback, this};
#endif
}

void ErrorMonitor::CreateCallback(VkInstance instance) noexcept {
    assert(instance);
    assert(!debug_obj_);

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    assert(vk::CreateDebugUtilsMessengerEXT != nullptr);
    const VkResult result = vk::CreateDebugUtilsMessengerEXT(instance, &debug_create_info_, nullptr, &debug_obj_);
#else
    assert(vk::CreateDebugReportCallbackEXT != nullptr);
    const VkResult result = vk::CreateDebugReportCallbackEXT(instance, &debug_create_info_, nullptr, &debug_obj_);
#endif
    if (result != VK_SUCCESS) {
        assert(false);
        debug_obj_ = VK_NULL_HANDLE;
    }
}

void ErrorMonitor::DestroyCallback(VkInstance instance) noexcept {
    assert(instance);
    assert(debug_obj_);  // valid to call with null object, but probably bug

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    vk::DestroyDebugUtilsMessengerEXT(instance, debug_obj_, nullptr);
#else
    vk::DestroyDebugReportCallbackEXT(instance, debug_obj_, nullptr);
#endif
    debug_obj_ = VK_NULL_HANDLE;
}

void ErrorMonitor::MonitorReset() {
    message_flags_ = kErrorBit;
    bailout_ = nullptr;
    message_found_ = false;
    failure_message_strings_.clear();
    desired_messages_.clear();
    ignore_message_strings_.clear();
    allowed_message_strings_.clear();
}

void ErrorMonitor::Reset() {
    auto guard = Lock();
    MonitorReset();
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msg_flags, const std::string &msg) {
    SetDesiredFailureMsg(msg_flags, msg.c_str());
}

void ErrorMonitor::SetDesiredFailureMsg(const VkFlags msg_flags, const char *const msg_string) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }

    auto guard = Lock();
    VuidAndMessage vuid_and_regex;
    vuid_and_regex.SetMsgString(msg_string);
    desired_messages_.emplace_back(std::move(vuid_and_regex));
    message_flags_ |= msg_flags;
}

void ErrorMonitor::SetDesiredFailureMsgRegex(const VkFlags msg_flags, const char *vuid, std::string regex_str) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }

    auto guard = Lock();
    VuidAndMessage vuid_and_regex;
    vuid_and_regex.vuid = vuid;
    std::regex regex(regex_str);
    vuid_and_regex.SetMsgRegex(std::move(regex_str), std::move(regex));
    desired_messages_.emplace_back(std::move(vuid_and_regex));
    message_flags_ |= msg_flags;
}

void ErrorMonitor::SetDesiredFailureMsgRegex(const VkFlags msg_flags, const char *vuid, std::string regex_str,
                                             std::string undesired_regex_str) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }

    auto guard = Lock();
    VuidAndMessage vuid_and_regex;
    vuid_and_regex.vuid = vuid;
    std::regex regex(regex_str);
    std::regex undesired_regex(undesired_regex_str);
    vuid_and_regex.SetMsgRegex(std::move(regex_str), std::move(regex));
    desired_messages_.emplace_back(std::move(vuid_and_regex));
    message_flags_ |= msg_flags;
}

void ErrorMonitor::SetDesiredFailureMsgRegex(const VkFlags msg_flags, const char *vuid, std::string regex_str, std::regex regex) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }

    auto guard = Lock();
    VuidAndMessage vuid_and_regex;
    vuid_and_regex.vuid = vuid;
    vuid_and_regex.SetMsgRegex(std::move(regex_str), std::move(regex));
    desired_messages_.emplace_back(std::move(vuid_and_regex));
    message_flags_ |= msg_flags;
}

void ErrorMonitor::SetDesiredError(const char *msg, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        SetDesiredFailureMsg(kErrorBit, msg);
    }
}

void ErrorMonitor::SetDesiredErrorRegex(const char *vuid, std::string regex_str, uint32_t count /*= 1*/) {
    const std::regex regex(regex_str);
    for (uint32_t i = 0; i < count; i++) {
        SetDesiredFailureMsgRegex(kErrorBit, vuid, regex_str, regex);
    }
}

void ErrorMonitor::SetDesiredWarningRegex(const char *vuid, std::string regex_str, uint32_t count /*= 1*/) {
    const std::regex regex(regex_str);
    for (uint32_t i = 0; i < count; i++) {
        SetDesiredFailureMsgRegex(kWarningBit, vuid, regex_str, regex);
    }
}

void ErrorMonitor::SetDesiredWarning(const char *msg, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        SetDesiredFailureMsg(kWarningBit, msg);
    }
}

void ErrorMonitor::SetDesiredInfo(const char *msg, uint32_t count /*= 1*/) {
    for (uint32_t i = 0; i < count; ++i) {
        SetDesiredFailureMsg(kInformationBit, msg);
    }
}

void ErrorMonitor::SetAllowedFailureMsg(const char *const msg) {
    auto guard = Lock();
    allowed_message_strings_.emplace_back(msg);
}

void ErrorMonitor::SetUnexpectedError(const char *const msg) {
    if (NeedCheckSuccess()) {
        VerifyNotFound();
    }
    auto guard = Lock();
    ignore_message_strings_.emplace_back(msg);
}

VkBool32 ErrorMonitor::CheckForDesiredMsg(const char *vuid, const char *const msg_string) {
    VkBool32 result = VK_FALSE;
    auto guard = Lock();
    if (bailout_ != nullptr) {
        *bailout_ = true;
    }
    std::string error_string(msg_string);
    bool found_expected = false;

    if (print_all_errors_) {
        std::cout << error_string << "\n\n";
    }

    if (IgnoreMessage(error_string)) {
        return result;
    }

    for (const VuidAndMessage &undesired_message : undesired_messages_) {
        if (undesired_message.Search(vuid, msg_string)) {
        }
    }

    for (size_t desired_message_i = 0; desired_message_i < desired_messages_.size(); ++desired_message_i) {
        VuidAndMessage &desired_message = desired_messages_[desired_message_i];
        if (desired_message.SearchUndesiredRegex(msg_string)) {
            ADD_FAILURE() << "Received undesired error message " << '"' << desired_message.undesired_msg_regex_string << '"'
                          << " in:\n"
                          << error_string;
            return VK_TRUE;
        }
        if (desired_message.Search(vuid, msg_string)) {
            found_expected = true;
            failure_message_strings_.emplace_back(error_string);
            message_found_ = true;
            result = VK_TRUE;
            // Remove desired message
            std::swap(desired_message, desired_messages_[desired_messages_.size() - 1]);
            desired_messages_.resize(desired_messages_.size() - 1);
            break;
        }
    }

    if (!found_expected) {
        for (const auto &allowed_msg : allowed_message_strings_) {
            if (error_string.find(allowed_msg) != std::string::npos) {
                found_expected = true;
                break;
            }
        }
    }

    if (!found_expected) {
        result = VK_TRUE;
        ADD_FAILURE() << error_string;
    }

    return result;
}

VkDebugReportFlagsEXT ErrorMonitor::GetMessageFlags() { return message_flags_; }

bool ErrorMonitor::AnyDesiredMsgFound() const { return message_found_; }

void ErrorMonitor::SetError(const char *const errorString) {
    auto guard = Lock();
    message_found_ = true;
    failure_message_strings_.emplace_back(errorString);
}

void ErrorMonitor::SetBailout(std::atomic<bool> *bailout) {
    auto guard = Lock();
    bailout_ = bailout;
}

void ErrorMonitor::ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask) {
    // Match ANY message matching specified type
    auto guard = Lock();
    desired_messages_.clear();
    message_flags_ = message_flag_mask;
}

bool ErrorMonitor::ExpectingSuccess() const {
    return (desired_messages_.size() == 1) && (desired_messages_.begin()->msg_string == "") && ignore_message_strings_.empty();
}

bool ErrorMonitor::NeedCheckSuccess() const { return ExpectingSuccess(); }

void ErrorMonitor::VerifyFound() {
    {
        // The lock must be released before the ExpectSuccess call at the end
        auto guard = Lock();
        // Not receiving expected message(s) is a failure.
        for (const auto &desired_msg : desired_messages_) {
            ADD_FAILURE() << "Did not receive expected error '" << desired_msg.Print() << "'";
        }

        MonitorReset();
    }

    ExpectSuccess();
}

void ErrorMonitor::Finish() {
    VerifyNotFound();
    Reset();
}

void ErrorMonitor::VerifyNotFound() {
    auto guard = Lock();
    // ExpectSuccess() configured us to match anything. Any error is a failure.
    if (AnyDesiredMsgFound()) {
        for (const auto &msg : failure_message_strings_) {
            ADD_FAILURE() << "Expected to succeed but got error: " << msg;
        }
    }
    MonitorReset();
}

bool ErrorMonitor::IgnoreMessage(std::string const &msg) const {
    if (ignore_message_strings_.empty()) {
        return false;
    }

    return std::find_if(ignore_message_strings_.begin(), ignore_message_strings_.end(), [&msg](std::string const &str) {
               return msg.find(str) != std::string::npos;
           }) != ignore_message_strings_.end();
}

bool ErrorMonitor::VuidAndMessage::Search(const char *vuid_, std::string_view msg) const {
#ifndef VK_USE_PLATFORM_ANDROID_KHR
    assert(msg_type != VuidAndMessage::Undefined);
    const bool vuid_compare = !vuid.empty() ? (vuid == vuid_) : true;
    switch (msg_type) {
        case ErrorMonitor::VuidAndMessage::String:
            return vuid_compare && msg.find(msg_string) != std::string::npos;
        case ErrorMonitor::VuidAndMessage::Regex:
            return vuid_compare && std::regex_search(msg.data(), msg_regex);
        default:
            return false;
    }
#else
    // With VK_EXT_debug_report, VUID is in msg
    (void)vuid_;
    assert(msg_type != VuidAndMessage::Undefined);
    const bool vuid_compare = !vuid.empty() ? msg.find(vuid) != std::string::npos : true;
    switch (msg_type) {
        case ErrorMonitor::VuidAndMessage::String:
            return vuid_compare && msg.find(msg_string) != std::string::npos;
        case ErrorMonitor::VuidAndMessage::Regex:
            return vuid_compare && std::regex_search(msg.data(), msg_regex);
        default:
            return false;
    }
#endif
}

bool ErrorMonitor::VuidAndMessage::SearchUndesiredRegex(std::string_view msg) const {
    return !undesired_msg_regex_string.empty() && std::regex_search(msg.data(), undesired_msg_regex);
}

std::string ErrorMonitor::VuidAndMessage::Print() const {
    std::string str(vuid);
    if (!str.empty()) {
        str += ' ';
    }
    str += msg_string;

    return str;
}
