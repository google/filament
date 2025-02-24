// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_COMMAND_COMMAND_H_
#define SRC_TINT_UTILS_COMMAND_COMMAND_H_

#include <string>
#include <utility>

namespace tint {

/// Command is a helper used by tests for executing a process with a number of
/// arguments and an optional stdin string, and then collecting and returning
/// the process's stdout and stderr output as strings.
class Command {
  public:
    /// Output holds the output of the process
    struct Output {
        /// stdout from the process
        std::string out;
        /// stderr from the process
        std::string err;
        /// process error code
        int error_code = 0;
    };

    /// Constructor
    /// @param path path to the executable
    explicit Command(const std::string& path);

    /// Looks for an executable with the given name in the current working directory,
    /// then in the executable directory if not found there, then in each of the
    /// directories in the `PATH` environment variable.
    /// @param executable the executable name
    /// @returns a Command which will return true for Found() if the executable
    /// was found.
    static Command LookPath(const std::string& executable);

    /// @return true if the executable exists at the path provided to the
    /// constructor
    bool Found() const;

    /// @returns the path of the command
    const std::string& Path() const { return path_; }

    /// Invokes the command with the given argument strings, blocking until the
    /// process has returned.
    /// @param args the string arguments to pass to the process
    /// @returns the process output
    template <typename... ARGS>
    Output operator()(ARGS... args) const {
        return Exec({std::forward<ARGS>(args)...});
    }

    /// Exec invokes the command with the given argument strings, blocking until
    /// the process has returned.
    /// @param args the string arguments to pass to the process
    /// @returns the process output
    Output Exec(std::initializer_list<std::string> args) const;

    /// @param input the input data to pipe to the process's stdin
    void SetInput(const std::string& input) { input_ = input; }

  private:
    std::string const path_;
    std::string input_;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_COMMAND_COMMAND_H_
