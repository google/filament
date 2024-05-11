// Copyright (c) 2018 Google LLC
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

#include <cassert>
#include <cerrno>
#include <cstring>
#include <functional>
#include <sstream>

#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "source/opt/log.h"
#include "source/reduce/reducer.h"
#include "source/spirv_reducer_options.h"
#include "source/util/string_utils.h"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"

namespace {

// Check that the std::system function can actually be used.
bool CheckExecuteCommand() {
  int res = std::system(nullptr);
  return res != 0;
}

// Execute a command using the shell.
// Returns true if and only if the command's exit status was 0.
bool ExecuteCommand(const std::string& command) {
  errno = 0;
  int status = std::system(command.c_str());
  assert(errno == 0 && "failed to execute command");
  // The result returned by 'system' is implementation-defined, but is
  // usually the case that the returned value is 0 when the command's exit
  // code was 0.  We are assuming that here, and that's all we depend on.
  return status == 0;
}

// Status and actions to perform after parsing command-line arguments.
enum ReduceActions { REDUCE_CONTINUE, REDUCE_STOP };

struct ReduceStatus {
  ReduceActions action;
  int code;
};

void PrintUsage(const char* program) {
  // NOTE: Please maintain flags in lexicographical order.
  printf(
      R"(%s - Reduce a SPIR-V binary file with respect to a user-provided
interestingness test.

USAGE: %s [options] <input.spv> -o <output.spv> -- <interestingness_test> [args...]

The SPIR-V binary is read from <input.spv>. The reduced SPIR-V binary is
written to <output.spv>.

Whether a binary is interesting is determined by <interestingness_test>, which
should be the path to a script. The "--" characters are optional but denote
that all arguments that follow are positional arguments and thus will be
forwarded to the interestingness test, and not parsed by %s.

 * The script must be executable.

 * The script should take the path to a SPIR-V binary file (.spv) as an
   argument, and exit with code 0 if and only if the binary file is
   interesting.  The binary will be passed to the script as an argument after
   any other provided arguments [args...].

 * Example: an interestingness test for reducing a SPIR-V binary file that
   causes tool "foo" to exit with error code 1 and print "Fatal error: bar" to
   standard error should:
     - invoke "foo" on the binary passed as the script argument;
     - capture the return code and standard error from "bar";
     - exit with code 0 if and only if the return code of "foo" was 1 and the
       standard error from "bar" contained "Fatal error: bar".

 * The reducer does not place a time limit on how long the interestingness test
   takes to run, so it is advisable to use per-command timeouts inside the
   script when invoking SPIR-V-processing tools (such as "foo" in the above
   example).

NOTE: The reducer is a work in progress.

Options (in lexicographical order):

  --fail-on-validation-error
               Stop reduction with an error if any reduction step produces a
               SPIR-V module that fails to validate.
  -h, --help
               Print this help.
  --step-limit=
               32-bit unsigned integer specifying maximum number of steps the
               reducer will take before giving up.
  --target-function=
               32-bit unsigned integer specifying the id of a function in the
               input module.  The reducer will restrict attention to this
               function, and will not make changes to other functions or to
               instructions outside of functions, except that some global
               instructions may be added in support of reducing the target
               function.  If 0 is specified (the default) then all functions are
               reduced.
  --temp-file-prefix=
               Specifies a temporary file prefix that will be used to output
               temporary shader files during reduction.  A number and .spv
               extension will be added.  The default is "temp_", which will
               cause files like "temp_0001.spv" to be output to the current
               directory.
  --version
               Display reducer version information.

Supported validator options are as follows. See `spirv-val --help` for details.
  --before-hlsl-legalization
  --relax-block-layout
  --relax-logical-pointer
  --relax-struct-store
  --scalar-block-layout
  --skip-block-layout
)",
      program, program, program);
}

// Message consumer for this tool.  Used to emit diagnostics during
// initialization and setup. Note that |source| and |position| are irrelevant
// here because we are still not processing a SPIR-V input file.
void ReduceDiagnostic(spv_message_level_t level, const char* /*source*/,
                      const spv_position_t& /*position*/, const char* message) {
  if (level == SPV_MSG_ERROR) {
    fprintf(stderr, "error: ");
  }
  fprintf(stderr, "%s\n", message);
}

ReduceStatus ParseFlags(int argc, const char** argv,
                        std::string* in_binary_file,
                        std::string* out_binary_file,
                        std::vector<std::string>* interestingness_test,
                        std::string* temp_file_prefix,
                        spvtools::ReducerOptions* reducer_options,
                        spvtools::ValidatorOptions* validator_options) {
  uint32_t positional_arg_index = 0;
  bool only_positional_arguments_remain = false;

  for (int argi = 1; argi < argc; ++argi) {
    const char* cur_arg = argv[argi];
    if ('-' == cur_arg[0] && !only_positional_arguments_remain) {
      if (0 == strcmp(cur_arg, "--version")) {
        spvtools::Logf(ReduceDiagnostic, SPV_MSG_INFO, nullptr, {}, "%s\n",
                       spvSoftwareVersionDetailsString());
        return {REDUCE_STOP, 0};
      } else if (0 == strcmp(cur_arg, "--help") || 0 == strcmp(cur_arg, "-h")) {
        PrintUsage(argv[0]);
        return {REDUCE_STOP, 0};
      } else if (0 == strcmp(cur_arg, "-o")) {
        if (out_binary_file->empty() && argi + 1 < argc) {
          *out_binary_file = std::string(argv[++argi]);
        } else {
          PrintUsage(argv[0]);
          return {REDUCE_STOP, 1};
        }
      } else if (0 == strncmp(cur_arg,
                              "--step-limit=", sizeof("--step-limit=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        char* end = nullptr;
        errno = 0;
        const auto step_limit =
            static_cast<uint32_t>(strtol(split_flag.second.c_str(), &end, 10));
        assert(end != split_flag.second.c_str() && errno == 0);
        reducer_options->set_step_limit(step_limit);
      } else if (0 == strncmp(cur_arg, "--target-function=",
                              sizeof("--target-function=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        char* end = nullptr;
        errno = 0;
        const auto target_function =
            static_cast<uint32_t>(strtol(split_flag.second.c_str(), &end, 10));
        assert(end != split_flag.second.c_str() && errno == 0);
        reducer_options->set_target_function(target_function);
      } else if (0 == strcmp(cur_arg, "--fail-on-validation-error")) {
        reducer_options->set_fail_on_validation_error(true);
      } else if (0 == strcmp(cur_arg, "--before-hlsl-legalization")) {
        validator_options->SetBeforeHlslLegalization(true);
      } else if (0 == strcmp(cur_arg, "--relax-logical-pointer")) {
        validator_options->SetRelaxLogicalPointer(true);
      } else if (0 == strcmp(cur_arg, "--relax-block-layout")) {
        validator_options->SetRelaxBlockLayout(true);
      } else if (0 == strcmp(cur_arg, "--scalar-block-layout")) {
        validator_options->SetScalarBlockLayout(true);
      } else if (0 == strcmp(cur_arg, "--skip-block-layout")) {
        validator_options->SetSkipBlockLayout(true);
      } else if (0 == strcmp(cur_arg, "--relax-struct-store")) {
        validator_options->SetRelaxStructStore(true);
      } else if (0 == strncmp(cur_arg, "--temp-file-prefix=",
                              sizeof("--temp-file-prefix=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        *temp_file_prefix = std::string(split_flag.second);
      } else if (0 == strcmp(cur_arg, "--")) {
        only_positional_arguments_remain = true;
      } else {
        std::stringstream ss;
        ss << "Unrecognized argument: " << cur_arg << std::endl;
        spvtools::Error(ReduceDiagnostic, nullptr, {}, ss.str().c_str());
        PrintUsage(argv[0]);
        return {REDUCE_STOP, 1};
      }
    } else if (positional_arg_index == 0) {
      // Binary input file name
      assert(in_binary_file->empty());
      *in_binary_file = std::string(cur_arg);
      positional_arg_index++;
    } else {
      interestingness_test->push_back(std::string(cur_arg));
    }
  }

  if (in_binary_file->empty()) {
    spvtools::Error(ReduceDiagnostic, nullptr, {}, "No input file specified");
    return {REDUCE_STOP, 1};
  }

  if (out_binary_file->empty()) {
    spvtools::Error(ReduceDiagnostic, nullptr, {}, "-o required");
    return {REDUCE_STOP, 1};
  }

  if (interestingness_test->empty()) {
    spvtools::Error(ReduceDiagnostic, nullptr, {},
                    "No interestingness test specified");
    return {REDUCE_STOP, 1};
  }

  return {REDUCE_CONTINUE, 0};
}

}  // namespace

// Dumps |binary| to file |filename|. Useful for interactive debugging.
void DumpShader(const std::vector<uint32_t>& binary, const char* filename) {
  auto write_file_succeeded =
      WriteFile(filename, "wb", &binary[0], binary.size());
  if (!write_file_succeeded) {
    std::cerr << "Failed to dump shader" << std::endl;
  }
}

// Dumps the SPIRV-V module in |context| to file |filename|. Useful for
// interactive debugging.
void DumpShader(spvtools::opt::IRContext* context, const char* filename) {
  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, false);
  DumpShader(binary, filename);
}

const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;

int main(int argc, const char** argv) {
  std::string in_binary_file;
  std::string out_binary_file;
  std::vector<std::string> interestingness_test;
  std::string temp_file_prefix = "temp_";

  spv_target_env target_env = kDefaultEnvironment;
  spvtools::ReducerOptions reducer_options;
  spvtools::ValidatorOptions validator_options;

  ReduceStatus status = ParseFlags(
      argc, argv, &in_binary_file, &out_binary_file, &interestingness_test,
      &temp_file_prefix, &reducer_options, &validator_options);

  if (status.action == REDUCE_STOP) {
    return status.code;
  }

  if (!CheckExecuteCommand()) {
    std::cerr << "could not find shell interpreter for executing a command"
              << std::endl;
    return 2;
  }

  spvtools::reduce::Reducer reducer(target_env);

  std::stringstream joined;
  joined << interestingness_test[0];
  for (size_t i = 1, size = interestingness_test.size(); i < size; ++i) {
    joined << " " << interestingness_test[i];
  }
  std::string interestingness_command_joined = joined.str();

  reducer.SetInterestingnessFunction(
      [interestingness_command_joined, temp_file_prefix](
          std::vector<uint32_t> binary, uint32_t reductions_applied) -> bool {
        std::stringstream ss;
        ss << temp_file_prefix << std::setw(4) << std::setfill('0')
           << reductions_applied << ".spv";
        const auto spv_file = ss.str();
        const std::string command =
            interestingness_command_joined + " " + spv_file;
        auto write_file_succeeded =
            WriteFile(spv_file.c_str(), "wb", &binary[0], binary.size());
        (void)(write_file_succeeded);
        assert(write_file_succeeded);
        return ExecuteCommand(command);
      });

  reducer.AddDefaultReductionPasses();

  reducer.SetMessageConsumer(spvtools::utils::CLIMessageConsumer);

  std::vector<uint32_t> binary_in;
  if (!ReadBinaryFile<uint32_t>(in_binary_file.c_str(), &binary_in)) {
    return 1;
  }

  const uint32_t target_function = (*reducer_options).target_function;
  if (target_function) {
    // A target function was specified; check that it exists.
    std::unique_ptr<spvtools::opt::IRContext> context = spvtools::BuildModule(
        kDefaultEnvironment, spvtools::utils::CLIMessageConsumer,
        binary_in.data(), binary_in.size());
    bool found_target_function = false;
    for (auto& function : *context->module()) {
      if (function.result_id() == target_function) {
        found_target_function = true;
        break;
      }
    }
    if (!found_target_function) {
      std::stringstream strstr;
      strstr << "Target function with id " << target_function
             << " was requested, but not found in the module; stopping.";
      spvtools::utils::CLIMessageConsumer(SPV_MSG_ERROR, nullptr, {},
                                          strstr.str().c_str());
      return 1;
    }
  }

  std::vector<uint32_t> binary_out;
  const auto reduction_status = reducer.Run(std::move(binary_in), &binary_out,
                                            reducer_options, validator_options);

  // Always try to write the output file, even if the reduction failed.
  if (!WriteFile<uint32_t>(out_binary_file.c_str(), "wb", binary_out.data(),
                           binary_out.size())) {
    return 1;
  }

  // These are the only successful statuses.
  switch (reduction_status) {
    case spvtools::reduce::Reducer::ReductionResultStatus::kComplete:
    case spvtools::reduce::Reducer::ReductionResultStatus::kReachedStepLimit:
      return 0;
    default:
      break;
  }

  return 1;
}
