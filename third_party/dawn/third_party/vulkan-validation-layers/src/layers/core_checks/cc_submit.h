/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

// About submit time validation module:
// In some cases validation of the submitted work cannot be done directly during QueueSubmit call.
// Timeline semaphore's wait-before-signal can reorder batches comparing to the submission order.
// That's why validation that depends on the final batch order cannot be done immediately in
// QueueSubmit (it will validate in submission order). This file provides support for submit validation
// where ordering is important.

namespace vvl {
struct QueueSubmission;
}  // namespace vvl

class Logger;

// Performs validationn when QueueSubmision is ready to retire.
struct QueueSubmissionValidator {
    const Logger &error_logger;

    QueueSubmissionValidator(const Logger &error_logger) : error_logger(error_logger) {}
    void Validate(const vvl::QueueSubmission& submission) const;
};
