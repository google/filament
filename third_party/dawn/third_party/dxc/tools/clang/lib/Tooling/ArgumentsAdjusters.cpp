//===--- ArgumentsAdjusters.cpp - Command line arguments adjuster ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains definitions of classes which implement ArgumentsAdjuster
// interface.
//
//===----------------------------------------------------------------------===//

#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
namespace tooling {

/// Add -fsyntax-only option to the commnand line arguments.
ArgumentsAdjuster getClangSyntaxOnlyAdjuster() {
  return [](const CommandLineArguments &Args) {
    CommandLineArguments AdjustedArgs;
    for (size_t i = 0, e = Args.size(); i != e; ++i) {
      StringRef Arg = Args[i];
      // FIXME: Remove options that generate output.
      if (!Arg.startswith("-fcolor-diagnostics") &&
          !Arg.startswith("-fdiagnostics-color"))
        AdjustedArgs.push_back(Args[i]);
    }
    AdjustedArgs.push_back("-fsyntax-only");
    return AdjustedArgs;
  };
}

ArgumentsAdjuster getClangStripOutputAdjuster() {
  return [](const CommandLineArguments &Args) {
    CommandLineArguments AdjustedArgs;
    for (size_t i = 0, e = Args.size(); i < e; ++i) {
      StringRef Arg = Args[i];
      if (!Arg.startswith("-o"))
        AdjustedArgs.push_back(Args[i]);

      if (Arg == "-o") {
        // Output is specified as -o foo. Skip the next argument also.
        ++i;
      }
      // Else, the output is specified as -ofoo. Just do nothing.
    }
    return AdjustedArgs;
  };
}

ArgumentsAdjuster getInsertArgumentAdjuster(const CommandLineArguments &Extra,
                                            ArgumentInsertPosition Pos) {
  return [Extra, Pos](const CommandLineArguments &Args) {
    CommandLineArguments Return(Args);

    CommandLineArguments::iterator I;
    if (Pos == ArgumentInsertPosition::END) {
      I = Return.end();
    } else {
      I = Return.begin();
      ++I; // To leave the program name in place
    }

    Return.insert(I, Extra.begin(), Extra.end());
    return Return;
  };
}

ArgumentsAdjuster getInsertArgumentAdjuster(const char *Extra,
                                            ArgumentInsertPosition Pos) {
  return getInsertArgumentAdjuster(CommandLineArguments(1, Extra), Pos);
}

ArgumentsAdjuster combineAdjusters(ArgumentsAdjuster First,
                                   ArgumentsAdjuster Second) {
  return [First, Second](const CommandLineArguments &Args) {
    return Second(First(Args));
  };
}

} // end namespace tooling
} // end namespace clang

