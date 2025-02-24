//===- subzero/src/IceClFlags.cpp - Command line flags and parsing --------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines commandline flags parsing of class Ice::ClFlags.
///
/// This currently relies on llvm::cl to parse. In the future, the minimal build
/// can have a simpler parser.
///
//===----------------------------------------------------------------------===//

#include "IceClFlags.h"

#include "IceClFlags.def"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/Support/CommandLine.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <utility>

namespace {
// cl is used to alias the llvm::cl types and functions that we need.
namespace cl {

using alias = llvm::cl::alias;

using aliasopt = llvm::cl::aliasopt;

using llvm::cl::CommaSeparated;

using desc = llvm::cl::desc;

template <typename T> using initializer = llvm::cl::initializer<T>;

template <typename T> initializer<T> init(const T &Val) {
  return initializer<T>(Val);
}

template <typename T> using list = llvm::cl::list<T>;

using llvm::cl::NotHidden;

template <typename T> using opt = llvm::cl::opt<T>;

using llvm::cl::ParseCommandLineOptions;

using llvm::cl::Positional;

// LLVM commit 3ffe113e11168abcd809ec5ac539538ade5db0cb changed the internals of
// llvm::cl that need to be mirrored here.  That commit removed the clEnumValEnd
// macro, so we can use that to determine which version of LLVM we're compiling
// against.
#if defined(clEnumValEnd)

#define CLENUMVALEND , clEnumValEnd

template <typename T> using ValuesClass = llvm::cl::ValuesClass<T>;

template <typename T, typename... A>
ValuesClass<T> values(const char *Arg, T Val, const char *Desc, A &&... Args) {
  return llvm::cl::values(Arg, Val, Desc, std::forward<A>(Args)..., nullptr);
}

#else // !defined(clEnumValEnd)

#define CLENUMVALEND

using llvm::cl::OptionEnumValue;

template <typename... A> llvm::cl::ValuesClass values(A &&... Args) {
  return llvm::cl::values(std::forward<A>(Args)...);
}

#endif // !defined(clEnumValEnd)

using llvm::cl::value_desc;
} // end of namespace cl

// cl_type_traits is used to convert between a tuple of <T, cl_detail::*flag> to
// the appropriate (llvm::)cl object.
template <typename B, typename CL> struct cl_type_traits {};

template <typename T>
struct cl_type_traits<T, ::Ice::cl_detail::dev_list_flag> {
  using cl_type = cl::list<T>;
};

template <typename T> struct cl_type_traits<T, ::Ice::cl_detail::dev_opt_flag> {
  using cl_type = cl::opt<T>;
};

template <typename T>
struct cl_type_traits<T, ::Ice::cl_detail::release_opt_flag> {
  using cl_type = cl::opt<T>;
};

#define X(Name, Type, ClType, ...)                                             \
  cl_type_traits<Type, Ice::cl_detail::ClType>::cl_type Name##Obj(__VA_ARGS__);
COMMAND_LINE_FLAGS
#undef X

// Add declarations that do not need to add members to ClFlags below.
cl::alias AllowExternDefinedSymbolsA(
    "allow-extern", cl::desc("Alias for --allow-externally-defined-symbols"),
    cl::NotHidden, cl::aliasopt(AllowExternDefinedSymbolsObj));

std::string AppNameObj;

} // end of anonymous namespace

namespace Ice {

ClFlags ClFlags::Flags;

void ClFlags::parseFlags(int argc, const char *const *argv) {
  cl::ParseCommandLineOptions(argc, argv);
  AppNameObj = argv[0];
}

namespace {
// flagInitOrStorageTypeDefault is some template voodoo for peeling off the
// llvm::cl modifiers from a flag's declaration, until its initial value is
// found. If none is found, then the default value for the storage type is
// returned.
template <typename Ty> Ty flagInitOrStorageTypeDefault() { return Ty(); }

template <typename Ty, typename T, typename... A>
Ty flagInitOrStorageTypeDefault(cl::initializer<T> &&Value, A &&...) {
  return Value.Init;
}

// is_cl_initializer is used to prevent an ambiguous call between the previous
// version of flagInitOrStorageTypeDefault, and the next, which is flagged by
// g++.
template <typename T> struct is_cl_initializer {
  static constexpr bool value = false;
};

template <typename T> struct is_cl_initializer<cl::initializer<T>> {
  static constexpr bool value = true;
};

template <typename Ty, typename T, typename... A>
typename std::enable_if<!is_cl_initializer<T>::value, Ty>::type
flagInitOrStorageTypeDefault(T &&, A &&... Other) {
  return flagInitOrStorageTypeDefault<Ty>(std::forward<A>(Other)...);
}

} // end of anonymous namespace

void ClFlags::resetClFlags() {
#define X(Name, Type, ClType, ...)                                             \
  Name = flagInitOrStorageTypeDefault<                                         \
      detail::cl_type_traits<Type, cl_detail::ClType>::storage_type>(          \
      __VA_ARGS__);
  COMMAND_LINE_FLAGS
#undef X
}

namespace {

// toSetterParam is template magic that is needed to convert between (llvm::)cl
// objects and the arguments to ClFlags' setters. ToSetterParam is a traits
// object that we need in order for the multiple specializations to
// toSetterParam to agree on their return type.
template <typename T> struct ToSetterParam { using ReturnType = const T &; };

template <> struct ToSetterParam<cl::list<Ice::VerboseItem>> {
  using ReturnType = Ice::VerboseMask;
};

template <> struct ToSetterParam<cl::list<std::string>> {
  using ReturnType = std::vector<std::string>;
};

template <typename T>
typename ToSetterParam<T>::ReturnType toSetterParam(const T &Param) {
  return Param;
}

template <>
ToSetterParam<cl::list<Ice::VerboseItem>>::ReturnType
toSetterParam(const cl::list<Ice::VerboseItem> &Param) {
  Ice::VerboseMask VMask = Ice::IceV_None;
  // Don't generate verbose messages if routines to dump messages are not
  // available.
  if (BuildDefs::dump()) {
    for (unsigned i = 0; i != Param.size(); ++i)
      VMask |= Param[i];
  }
  return VMask;
}

template <>
ToSetterParam<cl::list<std::string>>::ReturnType
toSetterParam(const cl::list<std::string> &Param) {
  return *&Param;
}

} // end of anonymous namespace

void ClFlags::getParsedClFlags(ClFlags &OutFlags) {
#define X(Name, Type, ClType, ...) OutFlags.set##Name(toSetterParam(Name##Obj));
  COMMAND_LINE_FLAGS
#undef X

  // If any value needs a non-trivial parsed value, set it below.
  OutFlags.setAllowExternDefinedSymbols(AllowExternDefinedSymbolsObj ||
                                        DisableInternalObj);
  OutFlags.setDisableHybridAssembly(DisableHybridAssemblyObj ||
                                    (OutFileTypeObj != Ice::FT_Iasm));
  OutFlags.ForceO2.init(OutFlags.getForceO2String());
  OutFlags.SplitInsts.init(OutFlags.getSplitInstString());
  OutFlags.TestStatus.init(OutFlags.getTestStatusString());
  OutFlags.TimingFocus.init(OutFlags.getTimingFocusOnString());
  OutFlags.TranslateOnly.init(OutFlags.getTranslateOnlyString());
  OutFlags.VerboseFocus.init(OutFlags.getVerboseFocusOnString());
}

} // end of namespace Ice
