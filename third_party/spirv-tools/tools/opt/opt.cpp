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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "source/opt/log.h"
#include "source/opt/loop_peeling.h"
#include "source/opt/set_spec_constant_default_value_pass.h"
#include "source/spirv_validator_options.h"
#include "spirv-tools/optimizer.hpp"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"

namespace {

// Status and actions to perform after parsing command-line arguments.
enum OptActions { OPT_CONTINUE, OPT_STOP };

struct OptStatus {
  OptActions action;
  int code;
};

// Message consumer for this tool.  Used to emit diagnostics during
// initialization and setup. Note that |source| and |position| are irrelevant
// here because we are still not processing a SPIR-V input file.
void opt_diagnostic(spv_message_level_t level, const char* /*source*/,
                    const spv_position_t& /*positon*/, const char* message) {
  if (level == SPV_MSG_ERROR) {
    fprintf(stderr, "error: ");
  }
  fprintf(stderr, "%s\n", message);
}

std::string GetListOfPassesAsString(const spvtools::Optimizer& optimizer) {
  std::stringstream ss;
  for (const auto& name : optimizer.GetPassNames()) {
    ss << "\n\t\t" << name;
  }
  return ss.str();
}

const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_3;

std::string GetLegalizationPasses() {
  spvtools::Optimizer optimizer(kDefaultEnvironment);
  optimizer.RegisterLegalizationPasses();
  return GetListOfPassesAsString(optimizer);
}

std::string GetOptimizationPasses() {
  spvtools::Optimizer optimizer(kDefaultEnvironment);
  optimizer.RegisterPerformancePasses();
  return GetListOfPassesAsString(optimizer);
}

std::string GetSizePasses() {
  spvtools::Optimizer optimizer(kDefaultEnvironment);
  optimizer.RegisterSizePasses();
  return GetListOfPassesAsString(optimizer);
}

void PrintUsage(const char* program) {
  // NOTE: Please maintain flags in lexicographical order.
  printf(
      R"(%s - Optimize a SPIR-V binary file.

USAGE: %s [options] [<input>] -o <output>

The SPIR-V binary is read from <input>. If no file is specified,
or if <input> is "-", then the binary is read from standard input.
if <output> is "-", then the optimized output is written to
standard output.

NOTE: The optimizer is a work in progress.

Options (in lexicographical order):
  --ccp
               Apply the conditional constant propagation transform.  This will
               propagate constant values throughout the program, and simplify
               expressions and conditional jumps with known predicate
               values.  Performed on entry point call tree functions and
               exported functions.
  --cfg-cleanup
               Cleanup the control flow graph. This will remove any unnecessary
               code from the CFG like unreachable code. Performed on entry
               point call tree functions and exported functions.
  --combine-access-chains
               Combines chained access chains to produce a single instruction
               where possible.
  --compact-ids
               Remap result ids to a compact range starting from %%1 and without
               any gaps.
  --convert-local-access-chains
               Convert constant index access chain loads/stores into
               equivalent load/stores with inserts and extracts. Performed
               on function scope variables referenced only with load, store,
               and constant index access chains in entry point call tree
               functions.
  --copy-propagate-arrays
               Does propagation of memory references when an array is a copy of
               another.  It will only propagate an array if the source is never
               written to, and the only store to the target is the copy.
  --eliminate-common-uniform
               Perform load/load elimination for duplicate uniform values.
               Converts any constant index access chain uniform loads into
               its equivalent load and extract. Some loads will be moved
               to facilitate sharing. Performed only on entry point
               call tree functions.
  --eliminate-dead-branches
               Convert conditional branches with constant condition to the
               indicated unconditional brranch. Delete all resulting dead
               code. Performed only on entry point call tree functions.
  --eliminate-dead-code-aggressive
               Delete instructions which do not contribute to a function's
               output. Performed only on entry point call tree functions.
  --eliminate-dead-const
               Eliminate dead constants.
  --eliminate-dead-functions
               Deletes functions that cannot be reached from entry points or
               exported functions.
  --eliminate-dead-inserts
               Deletes unreferenced inserts into composites, most notably
               unused stores to vector components, that are not removed by
               aggressive dead code elimination.
  --eliminate-dead-variables
               Deletes module scope variables that are not referenced.
  --eliminate-insert-extract
               DEPRECATED.  This pass has been replaced by the simplification
               pass, and that pass will be run instead.
               See --simplify-instructions.
  --eliminate-local-multi-store
               Replace stores and loads of function scope variables that are
               stored multiple times. Performed on variables referenceed only
               with loads and stores. Performed only on entry point call tree
               functions.
  --eliminate-local-single-block
               Perform single-block store/load and load/load elimination.
               Performed only on function scope variables in entry point
               call tree functions.
  --eliminate-local-single-store
               Replace stores and loads of function scope variables that are
               only stored once. Performed on variables referenceed only with
               loads and stores. Performed only on entry point call tree
               functions.
  --flatten-decorations
               Replace decoration groups with repeated OpDecorate and
               OpMemberDecorate instructions.
  --fold-spec-const-op-composite
               Fold the spec constants defined by OpSpecConstantOp or
               OpSpecConstantComposite instructions to front-end constants
               when possible.
  --freeze-spec-const
               Freeze the values of specialization constants to their default
               values.
  --if-conversion
               Convert if-then-else like assignments into OpSelect.
  --inline-entry-points-exhaustive
               Exhaustively inline all function calls in entry point call tree
               functions. Currently does not inline calls to functions with
               early return in a loop.
  --legalize-hlsl
               Runs a series of optimizations that attempts to take SPIR-V
               generated by an HLSL front-end and generates legal Vulkan SPIR-V.
               The optimizations are:
               %s

               Note this does not guarantee legal code. This option passes the
               option --relax-logical-pointer to the validator.
  --local-redundancy-elimination
               Looks for instructions in the same basic block that compute the
               same value, and deletes the redundant ones.
  --loop-fission
               Splits any top level loops in which the register pressure has exceeded
               a given threshold. The threshold must follow the use of this flag and
               must be a positive integer value.
  --loop-fusion
               Identifies adjacent loops with the same lower and upper bound.
               If this is legal, then merge the loops into a single loop.
               Includes heuristics to ensure it does not increase number of
               registers too much, while reducing the number of loads from
               memory. Takes an additional positive integer argument to set
               the maximum number of registers.
  --loop-unroll
               Fully unrolls loops marked with the Unroll flag
  --loop-unroll-partial
               Partially unrolls loops marked with the Unroll flag. Takes an
               additional non-0 integer argument to set the unroll factor, or
               how many times a loop body should be duplicated
  --loop-peeling
               Execute few first (respectively last) iterations before
               (respectively after) the loop if it can elide some branches.
  --loop-peeling-threshold
               Takes a non-0 integer argument to set the loop peeling code size
               growth threshold. The threshold prevents the loop peeling
               from happening if the code size increase created by
               the optimization is above the threshold.
  --merge-blocks
               Join two blocks into a single block if the second has the
               first as its only predecessor. Performed only on entry point
               call tree functions.
  --merge-return
               Changes functions that have multiple return statements so they
               have a single return statement.

               For structured control flow it is assumed that the only
               unreachable blocks in the function are trivial merge and continue
               blocks.

               A trivial merge block contains the label and an OpUnreachable
               instructions, nothing else.  A trivial continue block contain a
               label and an OpBranch to the header, nothing else.

               These conditions are guaranteed to be met after running
               dead-branch elimination.
  --loop-unswitch
               Hoists loop-invariant conditionals out of loops by duplicating
               the loop on each branch of the conditional and adjusting each
               copy of the loop.
  -O
               Optimize for performance. Apply a sequence of transformations
               in an attempt to improve the performance of the generated
               code. For this version of the optimizer, this flag is equivalent
               to specifying the following optimization code names:
               %s
  -Os
               Optimize for size. Apply a sequence of transformations in an
               attempt to minimize the size of the generated code. For this
               version of the optimizer, this flag is equivalent to specifying
               the following optimization code names:
               %s

               NOTE: The specific transformations done by -O and -Os change
                     from release to release.
  -Oconfig=<file>
               Apply the sequence of transformations indicated in <file>.
               This file contains a sequence of strings separated by whitespace
               (tabs, newlines or blanks). Each string is one of the flags
               accepted by spirv-opt. Optimizations will be applied in the
               sequence they appear in the file. This is equivalent to
               specifying all the flags on the command line. For example,
               given the file opts.cfg with the content:

                --inline-entry-points-exhaustive
                --eliminate-dead-code-aggressive

               The following two invocations to spirv-opt are equivalent:

               $ spirv-opt -Oconfig=opts.cfg program.spv

               $ spirv-opt --inline-entry-points-exhaustive \
                    --eliminate-dead-code-aggressive program.spv

               Lines starting with the character '#' in the configuration
               file indicate a comment and will be ignored.

               The -O, -Os, and -Oconfig flags act as macros. Using one of them
               is equivalent to explicitly inserting the underlying flags at
               that position in the command line. For example, the invocation
               'spirv-opt --merge-blocks -O ...' applies the transformation
               --merge-blocks followed by all the transformations implied by
               -O.
  --print-all
               Print SPIR-V assembly to standard error output before each pass
               and after the last pass.
  --private-to-local
               Change the scope of private variables that are used in a single
               function to that function.
  --reduce-load-size
               Replaces loads of composite objects where not every component is
               used by loads of just the elements that are used.
  --redundancy-elimination
               Looks for instructions in the same function that compute the
               same value, and deletes the redundant ones.
  --relax-struct-store
               Allow store from one struct type to a different type with
               compatible layout and members. This option is forwarded to the
               validator.
  --remove-duplicates
               Removes duplicate types, decorations, capabilities and extension
               instructions.
  --replace-invalid-opcode
               Replaces instructions whose opcode is valid for shader modules,
               but not for the current shader stage.  To have an effect, all
               entry points must have the same execution model.
  --ssa-rewrite
               Replace loads and stores to function local variables with
               operations on SSA IDs.
  --scalar-replacement[=<n>]
               Replace aggregate function scope variables that are only accessed
               via their elements with new function variables representing each
               element.  <n> is a limit on the size of the aggragates that will
               be replaced.  0 means there is no limit.  The default value is
               100.
  --set-spec-const-default-value "<spec id>:<default value> ..."
               Set the default values of the specialization constants with
               <spec id>:<default value> pairs specified in a double-quoted
               string. <spec id>:<default value> pairs must be separated by
               blank spaces, and in each pair, spec id and default value must
               be separated with colon ':' without any blank spaces in between.
               e.g.: --set-spec-const-default-value "1:100 2:400"
  --simplify-instructions
               Will simplify all instructions in the function as much as
               possible.
  --skip-validation
               Will not validate the SPIR-V before optimizing.  If the SPIR-V
               is invalid, the optimizer may fail or generate incorrect code.
               This options should be used rarely, and with caution.
  --strength-reduction
               Replaces instructions with equivalent and less expensive ones.
  --strip-debug
               Remove all debug instructions.
  --strip-reflect
               Remove all reflection information.  For now, this covers
               reflection information defined by SPV_GOOGLE_hlsl_functionality1.
  --time-report
               Print the resource utilization of each pass (e.g., CPU time,
               RSS) to standard error output. Currently it supports only Unix
               systems. This option is the same as -ftime-report in GCC. It
               prints CPU/WALL/USR/SYS time (and RSS if possible), but note that
               USR/SYS time are returned by getrusage() and can have a small
               error.
  --vector-dce
               This pass looks for components of vectors that are unused, and
               removes them from the vector.  Note this would still leave around
               lots of dead code that a pass of ADCE will be able to remove.
  --workaround-1209
               Rewrites instructions for which there are known driver bugs to
               avoid triggering those bugs.
               Current workarounds: Avoid OpUnreachable in loops.
  --unify-const
               Remove the duplicated constants.
  -h, --help
               Print this help.
  --version
               Display optimizer version information.
)",
      program, program, GetLegalizationPasses().c_str(),
      GetOptimizationPasses().c_str(), GetSizePasses().c_str());
}

// Reads command-line flags  the file specified in |oconfig_flag|. This string
// is assumed to have the form "-Oconfig=FILENAME". This function parses the
// string and extracts the file name after the '=' sign.
//
// Flags found in |FILENAME| are pushed at the end of the vector |file_flags|.
//
// This function returns true on success, false on failure.
bool ReadFlagsFromFile(const char* oconfig_flag,
                       std::vector<std::string>* file_flags) {
  const char* fname = strchr(oconfig_flag, '=');
  if (fname == nullptr || fname[0] != '=') {
    spvtools::Errorf(opt_diagnostic, nullptr, {}, "Invalid -Oconfig flag %s",
                     oconfig_flag);
    return false;
  }
  fname++;

  std::ifstream input_file;
  input_file.open(fname);
  if (input_file.fail()) {
    spvtools::Errorf(opt_diagnostic, nullptr, {}, "Could not open file '%s'",
                     fname);
    return false;
  }

  std::string line;
  while (std::getline(input_file, line)) {
    // Ignore empty lines and lines starting with the comment marker '#'.
    if (line.length() == 0 || line[0] == '#') {
      continue;
    }

    // Tokenize the line.  Add all found tokens to the list of found flags. This
    // mimics the way the shell will parse whitespace on the command line. NOTE:
    // This does not support quoting and it is not intended to.
    std::istringstream iss(line);
    while (!iss.eof()) {
      std::string flag;
      iss >> flag;
      file_flags->push_back(flag);
    }
  }

  return true;
}

OptStatus ParseFlags(int argc, const char** argv,
                     spvtools::Optimizer* optimizer, const char** in_file,
                     const char** out_file, spvtools::ValidatorOptions* options,
                     bool* skip_validator);

// Parses and handles the -Oconfig flag. |prog_name| contains the name of
// the spirv-opt binary (used to build a new argv vector for the recursive
// invocation to ParseFlags). |opt_flag| contains the -Oconfig=FILENAME flag.
// |optimizer|, |in_file| and |out_file| are as in ParseFlags.
//
// This returns the same OptStatus instance returned by ParseFlags.
OptStatus ParseOconfigFlag(const char* prog_name, const char* opt_flag,
                           spvtools::Optimizer* optimizer, const char** in_file,
                           const char** out_file) {
  std::vector<std::string> flags;
  flags.push_back(prog_name);

  std::vector<std::string> file_flags;
  if (!ReadFlagsFromFile(opt_flag, &file_flags)) {
    spvtools::Error(opt_diagnostic, nullptr, {},
                    "Could not read optimizer flags from configuration file");
    return {OPT_STOP, 1};
  }
  flags.insert(flags.end(), file_flags.begin(), file_flags.end());

  const char** new_argv = new const char*[flags.size()];
  for (size_t i = 0; i < flags.size(); i++) {
    if (flags[i].find("-Oconfig=") != std::string::npos) {
      spvtools::Error(
          opt_diagnostic, nullptr, {},
          "Flag -Oconfig= may not be used inside the configuration file");
      return {OPT_STOP, 1};
    }
    new_argv[i] = flags[i].c_str();
  }

  bool skip_validator = false;
  return ParseFlags(static_cast<int>(flags.size()), new_argv, optimizer,
                    in_file, out_file, nullptr, &skip_validator);
}

// Canonicalize the flag in |argv[argi]| of the form '--pass arg' into
// '--pass=arg'. The optimizer only accepts arguments to pass names that use the
// form '--pass_name=arg'.  Since spirv-opt also accepts the other form, this
// function makes the necessary conversion.
//
// Pass flags that require additional arguments should be handled here.  Note
// that additional arguments should be given as a single string.  If the flag
// requires more than one argument, the pass creator in
// Optimizer::GetPassFromFlag() should parse it accordingly (e.g., see the
// handler for --set-spec-const-default-value).
//
// If the argument requests one of the passes that need an additional argument,
// |argi| is modified to point past the current argument, and the string
// "argv[argi]=argv[argi + 1]" is returned. Otherwise, |argi| is unmodified and
// the string "|argv[argi]|" is returned.
std::string CanonicalizeFlag(const char** argv, int argc, int* argi) {
  const char* cur_arg = argv[*argi];
  const char* next_arg = (*argi + 1 < argc) ? argv[*argi + 1] : nullptr;
  std::ostringstream canonical_arg;
  canonical_arg << cur_arg;

  // NOTE: DO NOT ADD NEW FLAGS HERE.
  //
  // These flags are supported for backwards compatibility.  When adding new
  // passes that need extra arguments in its command-line flag, please make them
  // use the syntax "--pass_name[=pass_arg].
  if (0 == strcmp(cur_arg, "--set-spec-const-default-value") ||
      0 == strcmp(cur_arg, "--loop-fission") ||
      0 == strcmp(cur_arg, "--loop-fusion") ||
      0 == strcmp(cur_arg, "--loop-unroll-partial") ||
      0 == strcmp(cur_arg, "--loop-peeling-threshold")) {
    if (next_arg) {
      canonical_arg << "=" << next_arg;
      ++(*argi);
    }
  }

  return canonical_arg.str();
}

// the number of command-line flags. |argv| points to an array of strings
// holding the flags. |optimizer| is the Optimizer instance used to optimize the
// program.
//
// On return, this function stores the name of the input program in |in_file|.
// The name of the output file in |out_file|. The return value indicates whether
// optimization should continue and a status code indicating an error or
// success.
OptStatus ParseFlags(int argc, const char** argv,
                     spvtools::Optimizer* optimizer, const char** in_file,
                     const char** out_file, spvtools::ValidatorOptions* options,
                     bool* skip_validator) {
  std::vector<std::string> pass_flags;
  for (int argi = 1; argi < argc; ++argi) {
    const char* cur_arg = argv[argi];
    if ('-' == cur_arg[0]) {
      if (0 == strcmp(cur_arg, "--version")) {
        spvtools::Logf(opt_diagnostic, SPV_MSG_INFO, nullptr, {}, "%s\n",
                       spvSoftwareVersionDetailsString());
        return {OPT_STOP, 0};
      } else if (0 == strcmp(cur_arg, "--help") || 0 == strcmp(cur_arg, "-h")) {
        PrintUsage(argv[0]);
        return {OPT_STOP, 0};
      } else if (0 == strcmp(cur_arg, "-o")) {
        if (!*out_file && argi + 1 < argc) {
          *out_file = argv[++argi];
        } else {
          PrintUsage(argv[0]);
          return {OPT_STOP, 1};
        }
      } else if ('\0' == cur_arg[1]) {
        // Setting a filename of "-" to indicate stdin.
        if (!*in_file) {
          *in_file = cur_arg;
        } else {
          spvtools::Error(opt_diagnostic, nullptr, {},
                          "More than one input file specified");
          return {OPT_STOP, 1};
        }
      } else if (0 == strncmp(cur_arg, "-Oconfig=", sizeof("-Oconfig=") - 1)) {
        OptStatus status =
            ParseOconfigFlag(argv[0], cur_arg, optimizer, in_file, out_file);
        if (status.action != OPT_CONTINUE) {
          return status;
        }
      } else if (0 == strcmp(cur_arg, "--skip-validation")) {
        *skip_validator = true;
      } else if (0 == strcmp(cur_arg, "--print-all")) {
        optimizer->SetPrintAll(&std::cerr);
      } else if (0 == strcmp(cur_arg, "--time-report")) {
        optimizer->SetTimeReport(&std::cerr);
      } else if (0 == strcmp(cur_arg, "--relax-struct-store")) {
        options->SetRelaxStructStore(true);
      } else {
        // Some passes used to accept the form '--pass arg', canonicalize them
        // to '--pass=arg'.
        pass_flags.push_back(CanonicalizeFlag(argv, argc, &argi));

        // If we were requested to legalize SPIR-V generated from the HLSL
        // front-end, skip validation.
        if (0 == strcmp(cur_arg, "--legalize-hlsl")) {
          options->SetRelaxLogicalPointer(true);
        }
      }
    } else {
      if (!*in_file) {
        *in_file = cur_arg;
      } else {
        spvtools::Error(opt_diagnostic, nullptr, {},
                        "More than one input file specified");
        return {OPT_STOP, 1};
      }
    }
  }

  if (!optimizer->RegisterPassesFromFlags(pass_flags)) {
    return {OPT_STOP, 1};
  }

  return {OPT_CONTINUE, 0};
}

}  // namespace

int main(int argc, const char** argv) {
  const char* in_file = nullptr;
  const char* out_file = nullptr;
  bool skip_validator = false;

  spv_target_env target_env = kDefaultEnvironment;
  spvtools::ValidatorOptions options;

  spvtools::Optimizer optimizer(target_env);
  optimizer.SetMessageConsumer(spvtools::utils::CLIMessageConsumer);

  OptStatus status = ParseFlags(argc, argv, &optimizer, &in_file, &out_file,
                                &options, &skip_validator);

  if (status.action == OPT_STOP) {
    return status.code;
  }

  if (out_file == nullptr) {
    spvtools::Error(opt_diagnostic, nullptr, {}, "-o required");
    return 1;
  }

  std::vector<uint32_t> binary;
  if (!ReadFile<uint32_t>(in_file, "rb", &binary)) {
    return 1;
  }

  // By using the same vector as input and output, we save time in the case
  // that there was no change.
  bool ok = optimizer.Run(binary.data(), binary.size(), &binary, options,
                          skip_validator);

  if (!WriteFile<uint32_t>(out_file, "wb", binary.data(), binary.size())) {
    return 1;
  }

  return ok ? 0 : 1;
}
