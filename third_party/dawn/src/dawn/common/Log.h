// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_LOG_H_
#define SRC_DAWN_COMMON_LOG_H_

// Dawn targets shouldn't use iostream or printf directly for several reasons:
//  - iostream adds static initializers which we want to avoid.
//  - printf and iostream don't show up in logcat on Android so printf debugging doesn't work but
//  log-message debugging does.
//  - log severity helps provide intent compared to a printf.
//
// Logging should in general be avoided: errors should go through the regular WebGPU error reporting
// mechanism and others form of logging should (TODO: eventually) go through the logging dependency
// injection, so for example they show up in Chromium's about:gpu page. Nonetheless there are some
// cases where logging is necessary and when this file was first introduced we needed to replace all
// uses of iostream so we could see them in Android's logcat.
//
// Regular logging is done using the [Debug|Info|Warning|Error]Log() function this way:
//
//   InfoLog() << things << that << ostringstream << supports; // No need for a std::endl or "\n"
//
// It creates a LogMessage object that isn't stored anywhere and gets its destructor called
// immediately which outputs the stored ostringstream in the right place.
//
// This file also contains DAWN_DEBUG for "printf debugging" which works on Android and
// additionally outputs the file, line and function name. Use it this way:
//
//   // Pepper this throughout code to get a log of the execution
//   DAWN_DEBUG();
//
//   // Get more information
//   DAWN_DEBUG() << texture.GetFormat();

#include <sstream>

namespace dawn {

// Log levels mostly used to signal intent where the log message is produced and used to route
// the message to the correct output.
enum class LogSeverity {
    Debug,
    Info,
    Warning,
    Error,
};

// Essentially an ostringstream that will print itself in its destructor.
class LogMessage {
  public:
    explicit LogMessage(LogSeverity severity);
    ~LogMessage();

    LogMessage(LogMessage&& other);
    LogMessage& operator=(LogMessage&& other);

    template <typename T>
    LogMessage& operator<<(T&& value) {
        mStream << value;
        return *this;
    }

  private:
    LogMessage(const LogMessage& other) = delete;
    LogMessage& operator=(const LogMessage& other) = delete;

    LogSeverity mSeverity;
    std::ostringstream mStream;
};

// Short-hands to create a LogMessage with the respective severity.
LogMessage DebugLog();
LogMessage InfoLog();
LogMessage WarningLog();
LogMessage ErrorLog();

// DAWN_DEBUG is a helper macro that creates a DebugLog and outputs file/line/function
// information
LogMessage DebugLog(const char* file, const char* function, int line);
#define DAWN_DEBUG() ::dawn::DebugLog(__FILE__, __func__, __LINE__)

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_LOG_H_
