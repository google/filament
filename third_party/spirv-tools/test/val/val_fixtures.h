// Copyright (c) 2015-2016 The Khronos Group Inc.
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

// Common validation fixtures for unit tests

#ifndef TEST_VAL_VAL_FIXTURES_H_
#define TEST_VAL_VAL_FIXTURES_H_

#include <memory>
#include <string>

#include "source/val/validation_state.h"
#include "spirv-tools/libspirv.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtest {

template <typename T>
class ValidateBase : public ::testing::Test,
                     public ::testing::WithParamInterface<T> {
 public:
  ValidateBase();

  virtual void TearDown();

  // Returns the a spv_const_binary struct
  spv_const_binary get_const_binary();

  // Assembles the given SPIR-V text, checks that it fails to assemble,
  // and returns resulting diagnostic.  No internal state is updated.
  // Setting the desired_result to SPV_SUCCESS is used to allow all results
  std::string CompileFailure(std::string code,
                             spv_target_env env = SPV_ENV_UNIVERSAL_1_0,
                             spv_result_t desired_result = SPV_SUCCESS);

  // Checks that 'code' is valid SPIR-V text representation and stores the
  // binary version for further method calls.
  void CompileSuccessfully(std::string code,
                           spv_target_env env = SPV_ENV_UNIVERSAL_1_0);

  // Overwrites the word at index 'index' with the given word.
  // For testing purposes, it is often useful to be able to manipulate the
  // assembled binary before running the validator on it.
  // This function overwrites the word at the given index with a new word.
  void OverwriteAssembledBinary(uint32_t index, uint32_t word);

  // Performs validation on the SPIR-V code.
  spv_result_t ValidateInstructions(spv_target_env env = SPV_ENV_UNIVERSAL_1_0);

  // Performs validation. Returns the status and stores validation state into
  // the vstate_ member.
  spv_result_t ValidateAndRetrieveValidationState(
      spv_target_env env = SPV_ENV_UNIVERSAL_1_0);

  // Destroys the stored binary.
  void DestroyBinary() {
    spvBinaryDestroy(binary_);
    binary_ = nullptr;
  }

  // Destroys the stored diagnostic.
  void DestroyDiagnostic() {
    spvDiagnosticDestroy(diagnostic_);
    diagnostic_ = nullptr;
  }

  std::string getDiagnosticString();
  spv_position_t getErrorPosition();
  spv_validator_options getValidatorOptions();

  spv_binary binary_;
  spv_diagnostic diagnostic_;
  spv_validator_options options_;
  std::unique_ptr<spvtools::val::ValidationState_t> vstate_;
};

template <typename T>
ValidateBase<T>::ValidateBase() : binary_(nullptr), diagnostic_(nullptr) {
  // Initialize to default command line options. Different tests can then
  // specialize specific options as necessary.
  options_ = spvValidatorOptionsCreate();
}

template <typename T>
spv_const_binary ValidateBase<T>::get_const_binary() {
  return spv_const_binary(binary_);
}

template <typename T>
void ValidateBase<T>::TearDown() {
  if (diagnostic_) {
    spvDiagnosticPrint(diagnostic_);
  }
  DestroyBinary();
  DestroyDiagnostic();
  spvValidatorOptionsDestroy(options_);
}

template <typename T>
std::string ValidateBase<T>::CompileFailure(std::string code,
                                            spv_target_env env,
                                            spv_result_t desired_result) {
  spv_diagnostic diagnostic = nullptr;
  spv_result_t actual_result =
      spvTextToBinary(ScopedContext(env).context, code.c_str(), code.size(),
                      &binary_, &diagnostic);
  EXPECT_NE(SPV_SUCCESS, actual_result);
  // optional check for exact result
  if (desired_result != SPV_SUCCESS) {
    EXPECT_EQ(actual_result, desired_result);
  }
  std::string result(diagnostic->error);
  spvDiagnosticDestroy(diagnostic);
  return result;
}

template <typename T>
void ValidateBase<T>::CompileSuccessfully(std::string code,
                                          spv_target_env env) {
  DestroyBinary();
  spv_diagnostic diagnostic = nullptr;
  ScopedContext context(env);
  auto status = spvTextToBinary(context.context, code.c_str(), code.size(),
                                &binary_, &diagnostic);
  EXPECT_EQ(SPV_SUCCESS, status)
      << "ERROR: " << diagnostic->error
      << "\nSPIR-V could not be compiled into binary:\n"
      << code;
  ASSERT_EQ(SPV_SUCCESS, status);
  spvDiagnosticDestroy(diagnostic);
}

template <typename T>
void ValidateBase<T>::OverwriteAssembledBinary(uint32_t index, uint32_t word) {
  ASSERT_TRUE(index < binary_->wordCount)
      << "OverwriteAssembledBinary: The given index is larger than the binary "
         "word count.";
  binary_->code[index] = word;
}

template <typename T>
spv_result_t ValidateBase<T>::ValidateInstructions(spv_target_env env) {
  DestroyDiagnostic();
  if (binary_ == nullptr) {
    fprintf(stderr,
            "ERROR: Attempting to validate a null binary, did you forget to "
            "call CompileSuccessfully?");
    fflush(stderr);
  }
  assert(binary_ != nullptr);
  return spvValidateWithOptions(ScopedContext(env).context, options_,
                                get_const_binary(), &diagnostic_);
}

template <typename T>
spv_result_t ValidateBase<T>::ValidateAndRetrieveValidationState(
    spv_target_env env) {
  DestroyDiagnostic();
  return spvtools::val::ValidateBinaryAndKeepValidationState(
      ScopedContext(env).context, options_, get_const_binary()->code,
      get_const_binary()->wordCount, &diagnostic_, &vstate_);
}

template <typename T>
std::string ValidateBase<T>::getDiagnosticString() {
  return diagnostic_ == nullptr ? std::string()
                                : std::string(diagnostic_->error);
}

template <typename T>
spv_validator_options ValidateBase<T>::getValidatorOptions() {
  return options_;
}

template <typename T>
spv_position_t ValidateBase<T>::getErrorPosition() {
  return diagnostic_ == nullptr ? spv_position_t() : diagnostic_->position;
}

}  // namespace spvtest

// For Vulkan testing.
// Allows test parameter test to list all possible VUIDs with a delimiter that
// is then split here to check if one VUID was in the error message
MATCHER_P(AnyVUID, vuid_set, "VUID from the set is in error message") {
  // use space as delimiter because clang-format will properly line break VUID
  // strings which is important the entire VUID is in a single line for script
  // to scan
  std::string delimiter = " ";
  std::string token;
  std::string vuids = std::string(vuid_set);
  size_t position;

  // Catch case were someone accidentally left spaces by trimming string
  // clang-format off
  vuids.erase(std::find_if(vuids.rbegin(), vuids.rend(), [](unsigned char c) {
    return (c != ' ');
  }).base(), vuids.end());
  vuids.erase(vuids.begin(), std::find_if(vuids.begin(), vuids.end(), [](unsigned char c) {
    return (c != ' ');
  }));
  // clang-format on

  do {
    position = vuids.find(delimiter);
    if (position != std::string::npos) {
      token = vuids.substr(0, position);
      vuids.erase(0, position + delimiter.length());
    } else {
      token = vuids.substr(0);  // last item
    }

    // arg contains diagnostic message
    if (arg.find(token) != std::string::npos) {
      return true;
    }
  } while (position != std::string::npos);
  return false;
}

#endif  // TEST_VAL_VAL_FIXTURES_H_
