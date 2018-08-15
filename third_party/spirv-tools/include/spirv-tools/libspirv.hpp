// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_
#define INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "spirv-tools/libspirv.h"

namespace spvtools {

// Message consumer. The C strings for source and message are only alive for the
// specific invocation.
using MessageConsumer = std::function<void(
    spv_message_level_t /* level */, const char* /* source */,
    const spv_position_t& /* position */, const char* /* message */
    )>;

// C++ RAII wrapper around the C context object spv_context.
class Context {
 public:
  // Constructs a context targeting the given environment |env|.
  //
  // The constructed instance will have an empty message consumer, which just
  // ignores all messages from the library. Use SetMessageConsumer() to supply
  // one if messages are of concern.
  explicit Context(spv_target_env env);

  // Enables move constructor/assignment operations.
  Context(Context&& other);
  Context& operator=(Context&& other);

  // Disables copy constructor/assignment operations.
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  // Destructs this instance.
  ~Context();

  // Sets the message consumer to the given |consumer|. The |consumer| will be
  // invoked once for each message communicated from the library.
  void SetMessageConsumer(MessageConsumer consumer);

  // Returns the underlying spv_context.
  spv_context& CContext();
  const spv_context& CContext() const;

 private:
  spv_context context_;
};

// A RAII wrapper around a validator options object.
class ValidatorOptions {
 public:
  ValidatorOptions() : options_(spvValidatorOptionsCreate()) {}
  ~ValidatorOptions() { spvValidatorOptionsDestroy(options_); }
  // Allow implicit conversion to the underlying object.
  operator spv_validator_options() const { return options_; }

  // Sets a limit.
  void SetUniversalLimit(spv_validator_limit limit_type, uint32_t limit) {
    spvValidatorOptionsSetUniversalLimit(options_, limit_type, limit);
  }

  void SetRelaxStructStore(bool val) {
    spvValidatorOptionsSetRelaxStoreStruct(options_, val);
  }

  // Enables VK_KHR_relaxed_block_layout when validating standard
  // uniform/storage buffer layout.
  void SetRelaxBlockLayout(bool val) {
    spvValidatorOptionsSetRelaxBlockLayout(options_, val);
  }

  // Skips validating standard uniform/storage buffer layout.
  void SetSkipBlockLayout(bool val) {
    spvValidatorOptionsSetSkipBlockLayout(options_, val);
  }

  // Records whether or not the validator should relax the rules on pointer
  // usage in logical addressing mode.
  //
  // When relaxed, it will allow the following usage cases of pointers:
  // 1) OpVariable allocating an object whose type is a pointer type
  // 2) OpReturnValue returning a pointer value
  void SetRelaxLogicalPointer(bool val) {
    spvValidatorOptionsSetRelaxLogicalPointer(options_, val);
  }

 private:
  spv_validator_options options_;
};

// C++ interface for SPIRV-Tools functionalities. It wraps the context
// (including target environment and the corresponding SPIR-V grammar) and
// provides methods for assembling, disassembling, and validating.
//
// Instances of this class provide basic thread-safety guarantee.
class SpirvTools {
 public:
  enum {
    // Default assembling option used by assemble():
    kDefaultAssembleOption = SPV_TEXT_TO_BINARY_OPTION_NONE,

    // Default disassembling option used by Disassemble():
    // * Avoid prefix comments from decoding the SPIR-V module header, and
    // * Use friendly names for variables.
    kDefaultDisassembleOption = SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                                SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES
  };

  // Constructs an instance targeting the given environment |env|.
  //
  // The constructed instance will have an empty message consumer, which just
  // ignores all messages from the library. Use SetMessageConsumer() to supply
  // one if messages are of concern.
  explicit SpirvTools(spv_target_env env);

  // Disables copy/move constructor/assignment operations.
  SpirvTools(const SpirvTools&) = delete;
  SpirvTools(SpirvTools&&) = delete;
  SpirvTools& operator=(const SpirvTools&) = delete;
  SpirvTools& operator=(SpirvTools&&) = delete;

  // Destructs this instance.
  ~SpirvTools();

  // Sets the message consumer to the given |consumer|. The |consumer| will be
  // invoked once for each message communicated from the library.
  void SetMessageConsumer(MessageConsumer consumer);

  // Assembles the given assembly |text| and writes the result to |binary|.
  // Returns true on successful assembling. |binary| will be kept untouched if
  // assembling is unsuccessful.
  bool Assemble(const std::string& text, std::vector<uint32_t>* binary,
                uint32_t options = kDefaultAssembleOption) const;
  // |text_size| specifies the number of bytes in |text|. A terminating null
  // character is not required to present in |text| as long as |text| is valid.
  bool Assemble(const char* text, size_t text_size,
                std::vector<uint32_t>* binary,
                uint32_t options = kDefaultAssembleOption) const;

  // Disassembles the given SPIR-V |binary| with the given |options| and writes
  // the assembly to |text|. Returns ture on successful disassembling. |text|
  // will be kept untouched if diassembling is unsuccessful.
  bool Disassemble(const std::vector<uint32_t>& binary, std::string* text,
                   uint32_t options = kDefaultDisassembleOption) const;
  // |binary_size| specifies the number of words in |binary|.
  bool Disassemble(const uint32_t* binary, size_t binary_size,
                   std::string* text,
                   uint32_t options = kDefaultDisassembleOption) const;

  // Validates the given SPIR-V |binary|. Returns true if no issues are found.
  // Otherwise, returns false and communicates issues via the message consumer
  // registered.
  bool Validate(const std::vector<uint32_t>& binary) const;
  // |binary_size| specifies the number of words in |binary|.
  bool Validate(const uint32_t* binary, size_t binary_size) const;
  // Like the previous overload, but takes an options object.
  bool Validate(const uint32_t* binary, size_t binary_size,
                const ValidatorOptions& options) const;

 private:
  struct Impl;  // Opaque struct for holding the data fields used by this class.
  std::unique_ptr<Impl> impl_;  // Unique pointer to implementation data.
};

}  // namespace spvtools

#endif  // INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_
