// Copyright (c) 2019 Google LLC
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
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>

#include "source/fuzz/force_render_red.h"
#include "source/fuzz/fuzzer.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "source/fuzz/replayer.h"
#include "source/fuzz/shrinker.h"
#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "source/opt/log.h"
#include "source/spirv_fuzzer_options.h"
#include "source/util/make_unique.h"
#include "source/util/string_utils.h"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"

namespace {

enum class FuzzingTarget { kSpirv, kWgsl };

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
enum class FuzzActions {
  FORCE_RENDER_RED,  // Turn the shader into a form such that it is guaranteed
                     // to render a red image.
  FUZZ,    // Run the fuzzer to apply transformations in a randomized fashion.
  REPLAY,  // Replay an existing sequence of transformations.
  SHRINK,  // Shrink an existing sequence of transformations with respect to an
           // interestingness function.
  STOP     // Do nothing.
};

struct FuzzStatus {
  FuzzActions action;
  int code;
};

void PrintUsage(const char* program) {
  // NOTE: Please maintain flags in lexicographical order.
  printf(
      R"(%s - Fuzzes an equivalent SPIR-V binary based on a given binary.

USAGE: %s [options] <input.spv> -o <output.spv> \
  --donors=<donors.txt>
USAGE: %s [options] <input.spv> -o <output.spv> \
  --shrink=<input.transformations> -- <interestingness_test> [args...]

The SPIR-V binary is read from <input.spv>.  If <input.facts> is also present,
facts about the SPIR-V binary are read from this file.

The transformed SPIR-V binary is written to <output.spv>.  Human-readable and
binary representations of the transformations that were applied are written to
<output.transformations_json> and <output.transformations>, respectively.

When passing --shrink=<input.transformations> an <interestingness_test>
must also be provided; this is the path to a script that returns 0 if and only
if a given SPIR-V binary is interesting.  The SPIR-V binary will be passed to
the script as an argument after any other provided arguments [args...].  The
"--" characters are optional but denote that all arguments that follow are
positional arguments and thus will be forwarded to the interestingness script,
and not parsed by %s.

NOTE: The fuzzer is a work in progress.

Options (in lexicographical order):

  -h, --help
               Print this help.
  --donors=
               File specifying a series of donor files, one per line.  Must be
               provided if the tool is invoked in fuzzing mode; incompatible
               with replay and shrink modes.  The file should be empty if no
               donors are to be used.
  --enable-all-passes
               By default, spirv-fuzz follows the philosophy of "swarm testing"
               (Groce et al., 2012): only a subset of fuzzer passes are enabled
               on any given fuzzer run, with the subset being chosen randomly.
               This flag instead forces *all* fuzzer passes to be enabled.  When
               running spirv-fuzz many times this is likely to produce *less*
               diverse fuzzed modules than when swarm testing is used.  The
               purpose of the flag is to allow that hypothesis to be tested.
  --force-render-red
               Transforms the input shader into a shader that writes red to the
               output buffer, and then captures the original shader as the body
               of a conditional with a dynamically false guard.  Exploits input
               facts to make the guard non-obviously false.  This option is a
               helper for massaging crash-inducing tests into a runnable
               format; it does not perform any fuzzing.
  --fuzzer-pass-validation
               Run the validator after applying each fuzzer pass during
               fuzzing.  Aborts fuzzing early if an invalid binary is created.
               Useful for debugging spirv-fuzz.
  --repeated-pass-strategy=
               Available strategies are:
               - looped (the default): a sequence of fuzzer passes is chosen at
                 the start of fuzzing, via randomly choosing enabled passes, and
                 augmenting these choices with fuzzer passes that it is
                 recommended to run subsequently.  Fuzzing then involves
                 repeatedly applying this fixed sequence of passes.
               - random: each time a fuzzer pass is requested, this strategy
                 either provides one at random from the set of enabled passes,
                 or provides a pass that has been recommended based on a pass
                 that was used previously.
               - simple: each time a fuzzer pass is requested, one is provided
                 at random from the set of enabled passes.
  --fuzzing-target=
              This option will adjust probabilities of applying certain
              transformations s.t. the module always remains valid according
              to the semantics of some fuzzing target. Available targets:
              - spir-v - module is valid according to the SPIR-V spec.
              - wgsl - module is valid according to the WGSL spec.
  --replay
               File from which to read a sequence of transformations to replay
               (instead of fuzzing)
  --replay-range=
               Signed 32-bit integer.  If set to a positive value N, only the
               first N transformations will be applied during replay.  If set to
               a negative value -N, all but the final N transformations will be
               applied during replay.  If set to 0 (the default), all
               transformations will be applied during replay.  Ignored unless
               --replay is used.
  --replay-validation
               Run the validator after applying each transformation during
               replay (including the replay that occurs during shrinking).
               Aborts if an invalid binary is created.  Useful for debugging
               spirv-fuzz.
  --seed=
               Unsigned 32-bit integer seed to control random number
               generation.
  --shrink=
               File from which to read a sequence of transformations to shrink
               (instead of fuzzing)
  --shrinker-step-limit=
               Unsigned 32-bit integer specifying maximum number of steps the
               shrinker will take before giving up.  Ignored unless --shrink
               is used.
  --shrinker-temp-file-prefix=
               Specifies a temporary file prefix that will be used to output
               temporary shader files during shrinking.  A number and .spv
               extension will be added.  The default is "temp_", which will
               cause files like "temp_0001.spv" to be output to the current
               directory.  Ignored unless --shrink is used.
  --version
               Display fuzzer version information.

Supported validator options are as follows. See `spirv-val --help` for details.
  --before-hlsl-legalization
  --relax-block-layout
  --relax-logical-pointer
  --relax-struct-store
  --scalar-block-layout
  --skip-block-layout
)",
      program, program, program, program);
}

// Message consumer for this tool.  Used to emit diagnostics during
// initialization and setup. Note that |source| and |position| are irrelevant
// here because we are still not processing a SPIR-V input file.
void FuzzDiagnostic(spv_message_level_t level, const char* /*source*/,
                    const spv_position_t& /*position*/, const char* message) {
  if (level == SPV_MSG_ERROR) {
    fprintf(stderr, "error: ");
  }
  fprintf(stderr, "%s\n", message);
}

FuzzStatus ParseFlags(
    int argc, const char** argv, std::string* in_binary_file,
    std::string* out_binary_file, std::string* donors_file,
    std::string* replay_transformations_file,
    std::vector<std::string>* interestingness_test,
    std::string* shrink_transformations_file,
    std::string* shrink_temp_file_prefix,
    spvtools::fuzz::RepeatedPassStrategy* repeated_pass_strategy,
    FuzzingTarget* fuzzing_target, spvtools::FuzzerOptions* fuzzer_options,
    spvtools::ValidatorOptions* validator_options) {
  uint32_t positional_arg_index = 0;
  bool only_positional_arguments_remain = false;
  bool force_render_red = false;

  *repeated_pass_strategy =
      spvtools::fuzz::RepeatedPassStrategy::kLoopedWithRecommendations;

  for (int argi = 1; argi < argc; ++argi) {
    const char* cur_arg = argv[argi];
    if ('-' == cur_arg[0] && !only_positional_arguments_remain) {
      if (0 == strcmp(cur_arg, "--version")) {
        spvtools::Logf(FuzzDiagnostic, SPV_MSG_INFO, nullptr, {}, "%s\n",
                       spvSoftwareVersionDetailsString());
        return {FuzzActions::STOP, 0};
      } else if (0 == strcmp(cur_arg, "--help") || 0 == strcmp(cur_arg, "-h")) {
        PrintUsage(argv[0]);
        return {FuzzActions::STOP, 0};
      } else if (0 == strcmp(cur_arg, "-o")) {
        if (out_binary_file->empty() && argi + 1 < argc) {
          *out_binary_file = std::string(argv[++argi]);
        } else {
          PrintUsage(argv[0]);
          return {FuzzActions::STOP, 1};
        }
      } else if (0 == strncmp(cur_arg, "--donors=", sizeof("--donors=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        *donors_file = std::string(split_flag.second);
      } else if (0 == strncmp(cur_arg, "--enable-all-passes",
                              sizeof("--enable-all-passes") - 1)) {
        fuzzer_options->enable_all_passes();
      } else if (0 == strncmp(cur_arg, "--force-render-red",
                              sizeof("--force-render-red") - 1)) {
        force_render_red = true;
      } else if (0 == strncmp(cur_arg, "--fuzzer-pass-validation",
                              sizeof("--fuzzer-pass-validation") - 1)) {
        fuzzer_options->enable_fuzzer_pass_validation();
      } else if (0 == strncmp(cur_arg, "--replay=", sizeof("--replay=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        *replay_transformations_file = std::string(split_flag.second);
      } else if (0 == strncmp(cur_arg, "--repeated-pass-strategy=",
                              sizeof("--repeated-pass-strategy=") - 1)) {
        std::string strategy = spvtools::utils::SplitFlagArgs(cur_arg).second;
        if (strategy == "looped") {
          *repeated_pass_strategy =
              spvtools::fuzz::RepeatedPassStrategy::kLoopedWithRecommendations;
        } else if (strategy == "random") {
          *repeated_pass_strategy =
              spvtools::fuzz::RepeatedPassStrategy::kRandomWithRecommendations;
        } else if (strategy == "simple") {
          *repeated_pass_strategy =
              spvtools::fuzz::RepeatedPassStrategy::kSimple;
        } else {
          std::stringstream ss;
          ss << "Unknown repeated pass strategy '" << strategy << "'"
             << std::endl;
          ss << "Valid options are 'looped', 'random' and 'simple'.";
          spvtools::Error(FuzzDiagnostic, nullptr, {}, ss.str().c_str());
          return {FuzzActions::STOP, 1};
        }
      } else if (0 == strncmp(cur_arg, "--fuzzing-target=",
                              sizeof("--fuzzing-target=") - 1)) {
        std::string target = spvtools::utils::SplitFlagArgs(cur_arg).second;
        if (target == "spir-v") {
          *fuzzing_target = FuzzingTarget::kSpirv;
        } else if (target == "wgsl") {
          *fuzzing_target = FuzzingTarget::kWgsl;
        } else {
          std::stringstream ss;
          ss << "Unknown fuzzing target '" << target << "'" << std::endl;
          ss << "Valid options are 'spir-v' and 'wgsl'.";
          spvtools::Error(FuzzDiagnostic, nullptr, {}, ss.str().c_str());
          return {FuzzActions::STOP, 1};
        }
      } else if (0 == strncmp(cur_arg, "--replay-range=",
                              sizeof("--replay-range=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        char* end = nullptr;
        errno = 0;
        const auto replay_range =
            static_cast<int32_t>(strtol(split_flag.second.c_str(), &end, 10));
        assert(end != split_flag.second.c_str() && errno == 0);
        fuzzer_options->set_replay_range(replay_range);
      } else if (0 == strncmp(cur_arg, "--replay-validation",
                              sizeof("--replay-validation") - 1)) {
        fuzzer_options->enable_replay_validation();
      } else if (0 == strncmp(cur_arg, "--shrink=", sizeof("--shrink=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        *shrink_transformations_file = std::string(split_flag.second);
      } else if (0 == strncmp(cur_arg, "--seed=", sizeof("--seed=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        char* end = nullptr;
        errno = 0;
        const auto seed =
            static_cast<uint32_t>(strtol(split_flag.second.c_str(), &end, 10));
        assert(end != split_flag.second.c_str() && errno == 0);
        fuzzer_options->set_random_seed(seed);
      } else if (0 == strncmp(cur_arg, "--shrinker-step-limit=",
                              sizeof("--shrinker-step-limit=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        char* end = nullptr;
        errno = 0;
        const auto step_limit =
            static_cast<uint32_t>(strtol(split_flag.second.c_str(), &end, 10));
        assert(end != split_flag.second.c_str() && errno == 0);
        fuzzer_options->set_shrinker_step_limit(step_limit);
      } else if (0 == strncmp(cur_arg, "--shrinker-temp-file-prefix=",
                              sizeof("--shrinker-temp-file-prefix=") - 1)) {
        const auto split_flag = spvtools::utils::SplitFlagArgs(cur_arg);
        *shrink_temp_file_prefix = std::string(split_flag.second);
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
      } else if (0 == strcmp(cur_arg, "--")) {
        only_positional_arguments_remain = true;
      } else {
        std::stringstream ss;
        ss << "Unrecognized argument: " << cur_arg << std::endl;
        spvtools::Error(FuzzDiagnostic, nullptr, {}, ss.str().c_str());
        PrintUsage(argv[0]);
        return {FuzzActions::STOP, 1};
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
    spvtools::Error(FuzzDiagnostic, nullptr, {}, "No input file specified");
    return {FuzzActions::STOP, 1};
  }

  if (out_binary_file->empty()) {
    spvtools::Error(FuzzDiagnostic, nullptr, {}, "-o required");
    return {FuzzActions::STOP, 1};
  }

  auto const_fuzzer_options =
      static_cast<spv_const_fuzzer_options>(*fuzzer_options);
  if (force_render_red) {
    if (!replay_transformations_file->empty() ||
        !shrink_transformations_file->empty() ||
        const_fuzzer_options->replay_validation_enabled) {
      spvtools::Error(FuzzDiagnostic, nullptr, {},
                      "The --force-render-red argument cannot be used with any "
                      "other arguments except -o.");
      return {FuzzActions::STOP, 1};
    }
    return {FuzzActions::FORCE_RENDER_RED, 0};
  }

  if (replay_transformations_file->empty() &&
      shrink_transformations_file->empty() &&
      static_cast<spv_const_fuzzer_options>(*fuzzer_options)
          ->replay_validation_enabled) {
    spvtools::Error(FuzzDiagnostic, nullptr, {},
                    "The --replay-validation argument can only be used with "
                    "one of the --replay or --shrink arguments.");
    return {FuzzActions::STOP, 1};
  }

  if (shrink_transformations_file->empty() && !interestingness_test->empty()) {
    spvtools::Error(FuzzDiagnostic, nullptr, {},
                    "Too many positional arguments specified; extra positional "
                    "arguments are used as the interestingness function, which "
                    "are only valid with the --shrink option.");
    return {FuzzActions::STOP, 1};
  }

  if (!shrink_transformations_file->empty() && interestingness_test->empty()) {
    spvtools::Error(
        FuzzDiagnostic, nullptr, {},
        "The --shrink option requires an interestingness function.");
    return {FuzzActions::STOP, 1};
  }

  if (!replay_transformations_file->empty() ||
      !shrink_transformations_file->empty()) {
    // Donors should not be provided when replaying or shrinking: they only make
    // sense during fuzzing.
    if (!donors_file->empty()) {
      spvtools::Error(FuzzDiagnostic, nullptr, {},
                      "The --donors argument is not compatible with --replay "
                      "nor --shrink.");
      return {FuzzActions::STOP, 1};
    }
  }

  if (!replay_transformations_file->empty()) {
    // A replay transformations file was given, thus the tool is being invoked
    // in replay mode.
    if (!shrink_transformations_file->empty()) {
      spvtools::Error(
          FuzzDiagnostic, nullptr, {},
          "The --replay and --shrink arguments are mutually exclusive.");
      return {FuzzActions::STOP, 1};
    }
    return {FuzzActions::REPLAY, 0};
  }

  if (!shrink_transformations_file->empty()) {
    // The tool is being invoked in shrink mode.
    assert(!interestingness_test->empty() &&
           "An error should have been raised if --shrink was provided without "
           "an interestingness test.");
    return {FuzzActions::SHRINK, 0};
  }

  // The tool is being invoked in fuzz mode.
  if (donors_file->empty()) {
    spvtools::Error(FuzzDiagnostic, nullptr, {},
                    "Fuzzing requires that the --donors option is used.");
    return {FuzzActions::STOP, 1};
  }
  return {FuzzActions::FUZZ, 0};
}

bool ParseTransformations(
    const std::string& transformations_file,
    spvtools::fuzz::protobufs::TransformationSequence* transformations) {
  std::ifstream transformations_stream;
  transformations_stream.open(transformations_file,
                              std::ios::in | std::ios::binary);
  auto parse_success =
      transformations->ParseFromIstream(&transformations_stream);
  transformations_stream.close();
  if (!parse_success) {
    spvtools::Error(FuzzDiagnostic, nullptr, {},
                    ("Error reading transformations from file '" +
                     transformations_file + "'")
                        .c_str());
    return false;
  }
  return true;
}

bool Replay(const spv_target_env& target_env,
            spv_const_fuzzer_options fuzzer_options,
            spv_validator_options validator_options,
            const std::vector<uint32_t>& binary_in,
            const spvtools::fuzz::protobufs::FactSequence& initial_facts,
            const std::string& replay_transformations_file,
            std::vector<uint32_t>* binary_out,
            spvtools::fuzz::protobufs::TransformationSequence*
                transformations_applied) {
  spvtools::fuzz::protobufs::TransformationSequence transformation_sequence;
  if (!ParseTransformations(replay_transformations_file,
                            &transformation_sequence)) {
    return false;
  }

  uint32_t num_transformations_to_apply;
  if (fuzzer_options->replay_range > 0) {
    // We have a positive replay range, N.  We would like transformations
    // [0, N), truncated to the number of available transformations if N is too
    // large.
    num_transformations_to_apply = static_cast<uint32_t>(
        std::min(fuzzer_options->replay_range,
                 transformation_sequence.transformation_size()));
  } else {
    // We have non-positive replay range, -N (where N may be 0).  We would like
    // transformations [0, num_transformations - N), or no transformations if N
    // is too large.
    num_transformations_to_apply = static_cast<uint32_t>(
        std::max(0, transformation_sequence.transformation_size() +
                        fuzzer_options->replay_range));
  }

  auto replay_result =
      spvtools::fuzz::Replayer(
          target_env, spvtools::utils::CLIMessageConsumer, binary_in,
          initial_facts, transformation_sequence, num_transformations_to_apply,
          fuzzer_options->replay_validation_enabled, validator_options)
          .Run();
  replay_result.transformed_module->module()->ToBinary(binary_out, false);
  *transformations_applied = std::move(replay_result.applied_transformations);
  return replay_result.status ==
         spvtools::fuzz::Replayer::ReplayerResultStatus::kComplete;
}

bool Shrink(const spv_target_env& target_env,
            spv_const_fuzzer_options fuzzer_options,
            spv_validator_options validator_options,
            const std::vector<uint32_t>& binary_in,
            const spvtools::fuzz::protobufs::FactSequence& initial_facts,
            const std::string& shrink_transformations_file,
            const std::string& shrink_temp_file_prefix,
            const std::vector<std::string>& interestingness_command,
            std::vector<uint32_t>* binary_out,
            spvtools::fuzz::protobufs::TransformationSequence*
                transformations_applied) {
  spvtools::fuzz::protobufs::TransformationSequence transformation_sequence;
  if (!ParseTransformations(shrink_transformations_file,
                            &transformation_sequence)) {
    return false;
  }
  assert(!interestingness_command.empty() &&
         "An error should have been raised because the interestingness_command "
         "is empty.");
  std::stringstream joined;
  joined << interestingness_command[0];
  for (size_t i = 1, size = interestingness_command.size(); i < size; ++i) {
    joined << " " << interestingness_command[i];
  }
  std::string interestingness_command_joined = joined.str();

  spvtools::fuzz::Shrinker::InterestingnessFunction interestingness_function =
      [interestingness_command_joined, shrink_temp_file_prefix](
          std::vector<uint32_t> binary, uint32_t reductions_applied) -> bool {
    std::stringstream ss;
    ss << shrink_temp_file_prefix << std::setw(4) << std::setfill('0')
       << reductions_applied << ".spv";
    const auto spv_file = ss.str();
    const std::string command = interestingness_command_joined + " " + spv_file;
    auto write_file_succeeded =
        WriteFile(spv_file.c_str(), "wb", &binary[0], binary.size());
    (void)(write_file_succeeded);
    assert(write_file_succeeded);
    return ExecuteCommand(command);
  };

  auto shrink_result =
      spvtools::fuzz::Shrinker(
          target_env, spvtools::utils::CLIMessageConsumer, binary_in,
          initial_facts, transformation_sequence, interestingness_function,
          fuzzer_options->shrinker_step_limit,
          fuzzer_options->replay_validation_enabled, validator_options)
          .Run();

  *binary_out = std::move(shrink_result.transformed_binary);
  *transformations_applied = std::move(shrink_result.applied_transformations);
  return spvtools::fuzz::Shrinker::ShrinkerResultStatus::kComplete ==
             shrink_result.status ||
         spvtools::fuzz::Shrinker::ShrinkerResultStatus::kStepLimitReached ==
             shrink_result.status;
}

bool Fuzz(const spv_target_env& target_env,
          spv_const_fuzzer_options fuzzer_options,
          spv_validator_options validator_options,
          const std::vector<uint32_t>& binary_in,
          const spvtools::fuzz::protobufs::FactSequence& initial_facts,
          const std::string& donors,
          spvtools::fuzz::RepeatedPassStrategy repeated_pass_strategy,
          FuzzingTarget fuzzing_target, std::vector<uint32_t>* binary_out,
          spvtools::fuzz::protobufs::TransformationSequence*
              transformations_applied) {
  auto message_consumer = spvtools::utils::CLIMessageConsumer;

  std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier> donor_suppliers;

  std::ifstream donors_file(donors);
  if (!donors_file) {
    spvtools::Error(FuzzDiagnostic, nullptr, {}, "Error opening donors file");
    return false;
  }
  std::string donor_filename;
  while (std::getline(donors_file, donor_filename)) {
    donor_suppliers.emplace_back(
        [donor_filename, message_consumer,
         target_env]() -> std::unique_ptr<spvtools::opt::IRContext> {
          std::vector<uint32_t> donor_binary;
          if (!ReadBinaryFile<uint32_t>(donor_filename.c_str(),
                                        &donor_binary)) {
            return nullptr;
          }
          return spvtools::BuildModule(target_env, message_consumer,
                                       donor_binary.data(),
                                       donor_binary.size());
        });
  }

  std::unique_ptr<spvtools::opt::IRContext> ir_context;
  if (!spvtools::fuzz::fuzzerutil::BuildIRContext(target_env, message_consumer,
                                                  binary_in, validator_options,
                                                  &ir_context)) {
    spvtools::Error(FuzzDiagnostic, nullptr, {}, "Initial binary is invalid");
    return false;
  }

  assert((fuzzing_target == FuzzingTarget::kWgsl ||
          fuzzing_target == FuzzingTarget::kSpirv) &&
         "Not all fuzzing targets are handled");
  auto fuzzer_context = spvtools::MakeUnique<spvtools::fuzz::FuzzerContext>(
      spvtools::MakeUnique<spvtools::fuzz::PseudoRandomGenerator>(
          fuzzer_options->has_random_seed
              ? fuzzer_options->random_seed
              : static_cast<uint32_t>(std::random_device()())),
      spvtools::fuzz::FuzzerContext::GetMinFreshId(ir_context.get()),
      fuzzing_target == FuzzingTarget::kWgsl);

  auto transformation_context =
      spvtools::MakeUnique<spvtools::fuzz::TransformationContext>(
          spvtools::MakeUnique<spvtools::fuzz::FactManager>(ir_context.get()),
          validator_options);
  transformation_context->GetFactManager()->AddInitialFacts(message_consumer,
                                                            initial_facts);

  spvtools::fuzz::Fuzzer fuzzer(
      std::move(ir_context), std::move(transformation_context),
      std::move(fuzzer_context), message_consumer, donor_suppliers,
      fuzzer_options->all_passes_enabled, repeated_pass_strategy,
      fuzzer_options->fuzzer_pass_validation_enabled, validator_options, false);
  auto fuzz_result = fuzzer.Run(0);
  if (fuzz_result.status ==
      spvtools::fuzz::Fuzzer::Status::kFuzzerPassLedToInvalidModule) {
    spvtools::Error(FuzzDiagnostic, nullptr, {}, "Error running fuzzer");
    return false;
  }

  fuzzer.GetIRContext()->module()->ToBinary(binary_out, true);
  *transformations_applied = fuzzer.GetTransformationSequence();
  return true;
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

// Dumps |transformations| to file |filename| in binary format. Useful for
// interactive debugging.
void DumpTransformationsBinary(
    const spvtools::fuzz::protobufs::TransformationSequence& transformations,
    const char* filename) {
  std::ofstream transformations_file;
  transformations_file.open(filename, std::ios::out | std::ios::binary);
  transformations.SerializeToOstream(&transformations_file);
  transformations_file.close();
}

// The Chromium project applies the following patch to the protobuf library:
//
// source.chromium.org/chromium/chromium/src/+/main:third_party/protobuf/patches/0003-remove-static-initializers.patch
//
// This affects how Status objects must be constructed. This method provides a
// convenient way to get the OK status that works both with and without the
// patch. With the patch OK is a StatusPod, from which a Status can be
// constructed. Without the patch, OK is already a Status, and we harmlessly
// copy-construct the result from it.
google::protobuf::util::Status GetProtobufOkStatus() {
  return google::protobuf::util::Status(google::protobuf::util::Status::OK);
}

// Dumps |transformations| to file |filename| in JSON format. Useful for
// interactive debugging.
void DumpTransformationsJson(
    const spvtools::fuzz::protobufs::TransformationSequence& transformations,
    const char* filename) {
  std::string json_string;
  auto json_options = google::protobuf::util::JsonOptions();
  json_options.add_whitespace = true;
  auto json_generation_status = google::protobuf::util::MessageToJsonString(
      transformations, &json_string, json_options);
  if (json_generation_status == GetProtobufOkStatus()) {
    std::ofstream transformations_json_file(filename);
    transformations_json_file << json_string;
    transformations_json_file.close();
  }
}

const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_3;

int main(int argc, const char** argv) {
  std::string in_binary_file;
  std::string out_binary_file;
  std::string donors_file;
  std::string replay_transformations_file;
  std::vector<std::string> interestingness_test;
  std::string shrink_transformations_file;
  std::string shrink_temp_file_prefix = "temp_";
  spvtools::fuzz::RepeatedPassStrategy repeated_pass_strategy;
  auto fuzzing_target = FuzzingTarget::kSpirv;

  spvtools::FuzzerOptions fuzzer_options;
  spvtools::ValidatorOptions validator_options;

  FuzzStatus status =
      ParseFlags(argc, argv, &in_binary_file, &out_binary_file, &donors_file,
                 &replay_transformations_file, &interestingness_test,
                 &shrink_transformations_file, &shrink_temp_file_prefix,
                 &repeated_pass_strategy, &fuzzing_target, &fuzzer_options,
                 &validator_options);

  if (status.action == FuzzActions::STOP) {
    return status.code;
  }

  std::vector<uint32_t> binary_in;
  if (!ReadBinaryFile<uint32_t>(in_binary_file.c_str(), &binary_in)) {
    return 1;
  }

  spvtools::fuzz::protobufs::FactSequence initial_facts;

  // If not found, dot_pos will be std::string::npos, which can be used in
  // substr to mean "the end of the string"; there is no need to check the
  // result.
  size_t dot_pos = in_binary_file.rfind('.');
  std::string in_facts_file = in_binary_file.substr(0, dot_pos) + ".facts";
  std::ifstream facts_input(in_facts_file);
  if (facts_input) {
    std::string facts_json_string((std::istreambuf_iterator<char>(facts_input)),
                                  std::istreambuf_iterator<char>());
    facts_input.close();
    if (GetProtobufOkStatus() != google::protobuf::util::JsonStringToMessage(
                                     facts_json_string, &initial_facts)) {
      spvtools::Error(FuzzDiagnostic, nullptr, {}, "Error reading facts data");
      return 1;
    }
  }

  std::vector<uint32_t> binary_out;
  spvtools::fuzz::protobufs::TransformationSequence transformations_applied;

  spv_target_env target_env = kDefaultEnvironment;

  switch (status.action) {
    case FuzzActions::FORCE_RENDER_RED:
      if (!spvtools::fuzz::ForceRenderRed(
              target_env, validator_options, binary_in, initial_facts,
              spvtools::utils::CLIMessageConsumer, &binary_out)) {
        return 1;
      }
      break;
    case FuzzActions::FUZZ:
      if (!Fuzz(target_env, fuzzer_options, validator_options, binary_in,
                initial_facts, donors_file, repeated_pass_strategy,
                fuzzing_target, &binary_out, &transformations_applied)) {
        return 1;
      }
      break;
    case FuzzActions::REPLAY:
      if (!Replay(target_env, fuzzer_options, validator_options, binary_in,
                  initial_facts, replay_transformations_file, &binary_out,
                  &transformations_applied)) {
        return 1;
      }
      break;
    case FuzzActions::SHRINK: {
      if (!CheckExecuteCommand()) {
        std::cerr << "could not find shell interpreter for executing a command"
                  << std::endl;
        return 1;
      }
      if (!Shrink(target_env, fuzzer_options, validator_options, binary_in,
                  initial_facts, shrink_transformations_file,
                  shrink_temp_file_prefix, interestingness_test, &binary_out,
                  &transformations_applied)) {
        return 1;
      }
    } break;
    default:
      assert(false && "Unknown fuzzer action.");
      break;
  }

  if (!WriteFile<uint32_t>(out_binary_file.c_str(), "wb", binary_out.data(),
                           binary_out.size())) {
    spvtools::Error(FuzzDiagnostic, nullptr, {}, "Error writing out binary");
    return 1;
  }

  if (status.action != FuzzActions::FORCE_RENDER_RED) {
    // If not found, dot_pos will be std::string::npos, which can be used in
    // substr to mean "the end of the string"; there is no need to check the
    // result.
    dot_pos = out_binary_file.rfind('.');
    std::string output_file_prefix = out_binary_file.substr(0, dot_pos);
    std::ofstream transformations_file;
    transformations_file.open(output_file_prefix + ".transformations",
                              std::ios::out | std::ios::binary);
    bool success =
        transformations_applied.SerializeToOstream(&transformations_file);
    transformations_file.close();
    if (!success) {
      spvtools::Error(FuzzDiagnostic, nullptr, {},
                      "Error writing out transformations binary");
      return 1;
    }

    std::string json_string;
    auto json_options = google::protobuf::util::JsonOptions();
    json_options.add_whitespace = true;
    auto json_generation_status = google::protobuf::util::MessageToJsonString(
        transformations_applied, &json_string, json_options);
    if (json_generation_status != GetProtobufOkStatus()) {
      spvtools::Error(FuzzDiagnostic, nullptr, {},
                      "Error writing out transformations in JSON format");
      return 1;
    }

    std::ofstream transformations_json_file(output_file_prefix +
                                            ".transformations_json");
    transformations_json_file << json_string;
    transformations_json_file.close();
  }

  return 0;
}
