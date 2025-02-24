//===- subzero/src/IceClFlags.h - Cl Flags for translation ------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares Ice::ClFlags which implements command line processing.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECLFLAGS_H
#define SUBZERO_SRC_ICECLFLAGS_H

#include "IceBuildDefs.h"
#include "IceClFlags.def"
#include "IceDefs.h"
#include "IceRangeSpec.h"
#include "IceTypes.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/IRReader/IRReader.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <string>
#include <utility>
#include <vector>

#ifndef PNACL_LLVM
namespace llvm {
// \brief Define the expected format of the file.
enum NaClFileFormat {
  // LLVM IR source or bitcode file (as appropriate).
  LLVMFormat,
  // PNaCl bitcode file.
  PNaClFormat,
  // Autodetect if PNaCl or LLVM format.
  AutodetectFileFormat
};
} // end of namespace llvm
#endif // !PNACL_LLVM

namespace Ice {
// detail defines the type cl_type_traits, which is used to define the
// getters/setters for the ClFlags class. It converts the cl_detail::*_flag
// types to appropriate types for the several getters and setters created.
namespace detail {
// Base cl_type_traits.
template <typename B, typename CL> struct cl_type_traits {};

// cl_type_traits specialized cl::list<std::string>, non-MINIMAL build.
template <> struct cl_type_traits<std::string, cl_detail::dev_list_flag> {
  using storage_type = std::vector<std::string>;
};

// cl_type_traits specialized cl::list<Ice::VerboseItem>, non-MINIMAL build.
template <> struct cl_type_traits<Ice::VerboseItem, cl_detail::dev_list_flag> {
  using storage_type = Ice::VerboseMask;
};

// cl_type_traits specialized cl::opt<T>, non-MINIMAL build.
template <typename T> struct cl_type_traits<T, cl_detail::dev_opt_flag> {
  using storage_type = T;
};

// cl_type_traits specialized cl::opt<T>, MINIMAL build.
template <typename T> struct cl_type_traits<T, cl_detail::release_opt_flag> {
  using storage_type = T;
};

} // end of namespace detail

/// Define variables which configure translation and related support functions.
class ClFlags {
  ClFlags(const ClFlags &) = delete;
  ClFlags &operator=(const ClFlags &) = delete;

public:
  /// User defined constructor.
  ClFlags() { resetClFlags(); }

  /// The command line flags.
  static ClFlags Flags;

  /// \brief Parse commmand line options for Subzero.
  ///
  /// This is done use cl::ParseCommandLineOptions() and the static variables of
  /// type cl::opt defined in IceClFlags.cpp
  static void parseFlags(int argc, const char *const *argv);

  /// Reset all configuration options to their nominal values.
  void resetClFlags();

  /// \brief Retrieve the configuration option state
  ///
  /// This is defined by static variables
  /// anonymous_namespace{IceClFlags.cpp}::AllowErrorRecoveryObj,
  /// anonymous_namespace{IceClFlags.cpp}::AllowIacaMarksObj,
  /// ...
  static void getParsedClFlags(ClFlags &OutFlags);

#define X(Name, Type, ClType, ...)                                             \
private:                                                                       \
  using Name##StorageType =                                                    \
      detail::cl_type_traits<Type, cl_detail::ClType>::storage_type;           \
                                                                               \
  Name##StorageType Name;                                                      \
                                                                               \
  template <bool E>                                                            \
  typename std::enable_if<E, void>::type set##Name##Impl(                      \
      Name##StorageType Value) {                                               \
    Name = std::move(Value);                                                   \
  }                                                                            \
                                                                               \
  template <bool E>                                                            \
  typename std::enable_if<!E, void>::type set##Name##Impl(Name##StorageType) { \
  }                                                                            \
                                                                               \
public:                                                                        \
  Name##StorageType get##Name() const { return Name; }                         \
  void set##Name(Name##StorageType Value) {                                    \
    /* TODO(jpp): figure out which optional flags are used in minimal, and     \
       what are the defaults for them. */                                      \
    static constexpr bool Enable =                                             \
        std::is_same<cl_detail::ClType, cl_detail::release_opt_flag>::value || \
        !BuildDefs::minimal() || true;                                         \
    set##Name##Impl<Enable>(std::move(Value));                                 \
  }                                                                            \
                                                                               \
private:
  COMMAND_LINE_FLAGS
#undef X

public:
  bool isSequential() const { return NumTranslationThreads == 0; }
  bool isParseParallel() const {
    return getParseParallel() && !isSequential() && getBuildOnRead();
  }
  std::string getAppName() const { return AppName; }
  void setAppName(const std::string &Value) { AppName = Value; }

  /// \brief Get the value of ClFlags::GenerateUnitTestMessages
  ///
  /// Note: If dump routines have been turned off, the error messages
  /// will not be readable. Hence, turn off.
  bool getGenerateUnitTestMessages() const {
    return !BuildDefs::dump() || GenerateUnitTestMessages;
  }
  /// Set ClFlags::GenerateUnitTestMessages to a new value
  void setGenerateUnitTestMessages(bool NewValue) {
    GenerateUnitTestMessages = NewValue;
  }
  bool matchForceO2(GlobalString Name, uint32_t Number) const {
    return ForceO2.match(Name, Number);
  }
  bool matchSplitInsts(const std::string &Name, uint32_t Number) const {
    return SplitInsts.match(Name, Number);
  }
  bool matchTestStatus(GlobalString Name, uint32_t Number) const {
    return TestStatus.match(Name, Number);
  }
  bool matchTimingFocus(GlobalString Name, uint32_t Number) const {
    return TimingFocus.match(Name, Number);
  }
  bool matchTranslateOnly(GlobalString Name, uint32_t Number) const {
    return TranslateOnly.match(Name, Number);
  }
  bool matchVerboseFocusOn(GlobalString Name, uint32_t Number) const {
    return VerboseFocus.match(Name, Number);
  }
  bool matchVerboseFocusOn(const std::string &Name, uint32_t Number) const {
    return VerboseFocus.match(Name, Number);
  }

private:
  std::string AppName;

  /// Initialized to false; not set by the command line.
  bool GenerateUnitTestMessages;

  RangeSpec ForceO2;
  RangeSpec SplitInsts;
  RangeSpec TestStatus;
  RangeSpec TimingFocus;
  RangeSpec TranslateOnly;
  RangeSpec VerboseFocus;
};

inline const ClFlags &getFlags() { return ClFlags::Flags; }

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECLFLAGS_H
