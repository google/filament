//===- llvm/Support/Process.h -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// Provides a library for accessing information about this process and other
/// processes on the operating system. Also provides means of spawning
/// subprocess for commands. The design of this library is modeled after the
/// proposed design of the Boost.Process library, and is design specifically to
/// follow the style of standard libraries and potentially become a proposal
/// for a standard library.
///
/// This file declares the llvm::sys::Process class which contains a collection
/// of legacy static interfaces for extracting various information about the
/// current process. The goal is to migrate users of this API over to the new
/// interfaces.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_PROCESS_H
#define LLVM_SUPPORT_PROCESS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/TimeValue.h"
#include <system_error>

namespace llvm {
class StringRef;

namespace sys {


/// \brief A collection of legacy interfaces for querying information about the
/// current executing process.
class Process {
public:
  static unsigned getPageSize();

  /// \brief Return process memory usage.
  /// This static function will return the total amount of memory allocated
  /// by the process. This only counts the memory allocated via the malloc,
  /// calloc and realloc functions and includes any "free" holes in the
  /// allocated space.
  static size_t GetMallocUsage();

  /// This static function will set \p user_time to the amount of CPU time
  /// spent in user (non-kernel) mode and \p sys_time to the amount of CPU
  /// time spent in system (kernel) mode.  If the operating system does not
  /// support collection of these metrics, a zero TimeValue will be for both
  /// values.
  /// \param elapsed Returns the TimeValue::now() giving current time
  /// \param user_time Returns the current amount of user time for the process
  /// \param sys_time Returns the current amount of system time for the process
  static void GetTimeUsage(TimeValue &elapsed, TimeValue &user_time,
                           TimeValue &sys_time);

  /// This function makes the necessary calls to the operating system to
  /// prevent core files or any other kind of large memory dumps that can
  /// occur when a program fails.
  /// @brief Prevent core file generation.
  static void PreventCoreFiles();

  // This function returns the environment variable \arg name's value as a UTF-8
  // string. \arg Name is assumed to be in UTF-8 encoding too.
  static Optional<std::string> GetEnv(StringRef name);

  /// This function searches for an existing file in the list of directories
  /// in a PATH like environment variable, and returns the first file found,
  /// according to the order of the entries in the PATH like environment
  /// variable.
  static Optional<std::string> FindInEnvPath(const std::string& EnvName,
                                             const std::string& FileName);

  /// This function returns a SmallVector containing the arguments passed from
  /// the operating system to the program.  This function expects to be handed
  /// the vector passed in from main.
  static std::error_code
  GetArgumentVector(SmallVectorImpl<const char *> &Args,
                    ArrayRef<const char *> ArgsFromMain,
                    SpecificBumpPtrAllocator<char> &ArgAllocator);

  // This functions ensures that the standard file descriptors (input, output,
  // and error) are properly mapped to a file descriptor before we use any of
  // them.  This should only be called by standalone programs, library
  // components should not call this.
  static std::error_code FixupStandardFileDescriptors();

  // This function safely closes a file descriptor.  It is not safe to retry
  // close(2) when it returns with errno equivalent to EINTR; this is because
  // *nixen cannot agree if the file descriptor is, in fact, closed when this
  // occurs.
  //
  // N.B. Some operating systems, due to thread cancellation, cannot properly
  // guarantee that it will or will not be closed one way or the other!
  static std::error_code SafelyCloseFileDescriptor(int FD);

  /// This function determines if the standard input is connected directly
  /// to a user's input (keyboard probably), rather than coming from a file
  /// or pipe.
  static bool StandardInIsUserInput();

  /// This function determines if the standard output is connected to a
  /// "tty" or "console" window. That is, the output would be displayed to
  /// the user rather than being put on a pipe or stored in a file.
  static bool StandardOutIsDisplayed();

  /// This function determines if the standard error is connected to a
  /// "tty" or "console" window. That is, the output would be displayed to
  /// the user rather than being put on a pipe or stored in a file.
  static bool StandardErrIsDisplayed();

  /// This function determines if the given file descriptor is connected to
  /// a "tty" or "console" window. That is, the output would be displayed to
  /// the user rather than being put on a pipe or stored in a file.
  static bool FileDescriptorIsDisplayed(int fd);

  /// This function determines if the given file descriptor is displayd and
  /// supports colors.
  static bool FileDescriptorHasColors(int fd);

  /// This function determines the number of columns in the window
  /// if standard output is connected to a "tty" or "console"
  /// window. If standard output is not connected to a tty or
  /// console, or if the number of columns cannot be determined,
  /// this routine returns zero.
  static unsigned StandardOutColumns();

  /// This function determines the number of columns in the window
  /// if standard error is connected to a "tty" or "console"
  /// window. If standard error is not connected to a tty or
  /// console, or if the number of columns cannot be determined,
  /// this routine returns zero.
  static unsigned StandardErrColumns();

  /// This function determines whether the terminal connected to standard
  /// output supports colors. If standard output is not connected to a
  /// terminal, this function returns false.
  static bool StandardOutHasColors();

  /// This function determines whether the terminal connected to standard
  /// error supports colors. If standard error is not connected to a
  /// terminal, this function returns false.
  static bool StandardErrHasColors();

  /// Enables or disables whether ANSI escape sequences are used to output
  /// colors. This only has an effect on Windows.
  /// Note: Setting this option is not thread-safe and should only be done
  /// during initialization.
  static void UseANSIEscapeCodes(bool enable);

  /// Whether changing colors requires the output to be flushed.
  /// This is needed on systems that don't support escape sequences for
  /// changing colors.
  static bool ColorNeedsFlush();

  /// This function returns the colorcode escape sequences.
  /// If ColorNeedsFlush() is true then this function will change the colors
  /// and return an empty escape sequence. In that case it is the
  /// responsibility of the client to flush the output stream prior to
  /// calling this function.
  static const char *OutputColor(char c, bool bold, bool bg);

  /// Same as OutputColor, but only enables the bold attribute.
  static const char *OutputBold(bool bg);

  /// This function returns the escape sequence to reverse forground and
  /// background colors.
  static const char *OutputReverse();

  /// Resets the terminals colors, or returns an escape sequence to do so.
  static const char *ResetColor();

  /// Get the result of a process wide random number generator. The
  /// generator will be automatically seeded in non-deterministic fashion.
  static unsigned GetRandomNumber();
};

}
}

#endif
