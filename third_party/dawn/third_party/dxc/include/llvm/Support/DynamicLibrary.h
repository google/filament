//===-- llvm/Support/DynamicLibrary.h - Portable Dynamic Library -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the sys::DynamicLibrary class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_DYNAMICLIBRARY_H
#define LLVM_SUPPORT_DYNAMICLIBRARY_H

#include <string>

namespace llvm {

class StringRef;

namespace sys {

  /// This class provides a portable interface to dynamic libraries which also
  /// might be known as shared libraries, shared objects, dynamic shared
  /// objects, or dynamic link libraries. Regardless of the terminology or the
  /// operating system interface, this class provides a portable interface that
  /// allows dynamic libraries to be loaded and searched for externally
  /// defined symbols. This is typically used to provide "plug-in" support.
  /// It also allows for symbols to be defined which don't live in any library,
  /// but rather the main program itself, useful on Windows where the main
  /// executable cannot be searched.
  ///
  /// Note: there is currently no interface for temporarily loading a library,
  /// or for unloading libraries when the LLVM library is unloaded.
  class DynamicLibrary {
    // Placeholder whose address represents an invalid library.
    // We use this instead of NULL or a pointer-int pair because the OS library
    // might define 0 or 1 to be "special" handles, such as "search all".
    static char Invalid;

    // Opaque data used to interface with OS-specific dynamic library handling.
    void *Data;

  public:
    explicit DynamicLibrary(void *data = &Invalid) : Data(data) {}

    /// Returns true if the object refers to a valid library.
    bool isValid() const { return Data != &Invalid; }

    /// Searches through the library for the symbol \p symbolName. If it is
    /// found, the address of that symbol is returned. If not, NULL is returned.
    /// Note that NULL will also be returned if the library failed to load.
    /// Use isValid() to distinguish these cases if it is important.
    /// Note that this will \e not search symbols explicitly registered by
    /// AddSymbol().
    void *getAddressOfSymbol(const char *symbolName);

    /// This function permanently loads the dynamic library at the given path.
    /// The library will only be unloaded when the program terminates.
    /// This returns a valid DynamicLibrary instance on success and an invalid
    /// instance on failure (see isValid()). \p *errMsg will only be modified
    /// if the library fails to load.
    ///
    /// It is safe to call this function multiple times for the same library.
    /// @brief Open a dynamic library permanently.
    static DynamicLibrary getPermanentLibrary(const char *filename,
                                              std::string *errMsg = nullptr);

    /// This function permanently loads the dynamic library at the given path.
    /// Use this instead of getPermanentLibrary() when you won't need to get
    /// symbols from the library itself.
    ///
    /// It is safe to call this function multiple times for the same library.
    static bool LoadLibraryPermanently(const char *Filename,
                                       std::string *ErrMsg = nullptr) {
      return !getPermanentLibrary(Filename, ErrMsg).isValid();
    }

    /// This function will search through all previously loaded dynamic
    /// libraries for the symbol \p symbolName. If it is found, the address of
    /// that symbol is returned. If not, null is returned. Note that this will
    /// search permanently loaded libraries (getPermanentLibrary()) as well
    /// as explicitly registered symbols (AddSymbol()).
    /// @throws std::string on error.
    /// @brief Search through libraries for address of a symbol
    static void *SearchForAddressOfSymbol(const char *symbolName);

    /// @brief Convenience function for C++ophiles.
    static void *SearchForAddressOfSymbol(const std::string &symbolName) {
      return SearchForAddressOfSymbol(symbolName.c_str());
    }

    /// This functions permanently adds the symbol \p symbolName with the
    /// value \p symbolValue.  These symbols are searched before any
    /// libraries.
    /// @brief Add searchable symbol/value pair.
    static void AddSymbol(StringRef symbolName, void *symbolValue);
  };

} // End sys namespace
} // End llvm namespace

#endif
