//===- subzero/src/IceBuildDefs.h - Translator build defines ----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Define the Ice::BuildDefs namespace
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEBUILDDEFS_H
#define SUBZERO_SRC_ICEBUILDDEFS_H

namespace Ice {
/// \brief Defines constexpr functions that express various Subzero build
/// system defined values.
///
/// These resulting constexpr functions allow code to in effect be
/// conditionally compiled without having to do this using the older C++
/// preprocessor solution.

/** \verbatim

 For example whenever the value of FEATURE_SUPPORTED is needed, instead
 of (except in these constexpr functions):

 #if FEATURE_SUPPORTED ...
 ...
 #endif

 We can have:

 namespace Ice {
 namespace BuildDefs {

 // Use this form when FEATURE_SUPPORTED is guaranteed to be defined on the
 // C++ compiler command line as 0 or 1.
 constexpr bool hasFeature() { return FEATURE_SUPPORTED; }

 or

 // Use this form when FEATURE_SUPPORTED may not necessarily be defined on
 // the C++ compiler command line.
 constexpr bool hasFeature() {
 #if FEATURE_SUPPORTED
   return true;
 #else // !FEATURE_SUPPORTED
   return false;
 #endif // !FEATURE_SUPPORTED
 }

 ...} // end of namespace BuildDefs
 } // end of namespace Ice


 And later in the code:

 if (Ice::BuildDefs::hasFeature() {
    ...
 }

 \endverbatim

 Since hasFeature() returns a constexpr, an optimizing compiler will know to
 keep or discard the above fragment. In addition, the code will always be
 looked at by the compiler which eliminates the problem with defines in that
 if you don't build that variant, you don't even know if the code would
 compile unless you build with that variant.

  **/

namespace BuildDefs {

// The ALLOW_* etc. symbols must be #defined to zero or non-zero.
/// Return true if ALLOW_DUMP is defined as a non-zero value
constexpr bool dump() { return ALLOW_DUMP; }
/// Return true if ALLOW_TIMERS is defined as a non-zero value
constexpr bool timers() { return ALLOW_TIMERS; }
/// Return true if ALLOW_LLVM_CL is defined as a non-zero value
// TODO(stichnot): this ALLOW_LLVM_CL is a TBD option which will
// allow for replacement of llvm:cl command line processor with a
// smaller footprint version for Subzero.
constexpr bool llvmCl() { return ALLOW_LLVM_CL; }
/// Return true if ALLOW_LLVM_IR is defined as a non-zero value
constexpr bool llvmIr() { return ALLOW_LLVM_IR; }
/// Return true if ALLOW_LLVM_IR_AS_INPUT is defined as a non-zero value
constexpr bool llvmIrAsInput() { return ALLOW_LLVM_IR_AS_INPUT; }
/// Return true if ALLOW_MINIMAL_BUILD is defined as a non-zero value
constexpr bool minimal() { return ALLOW_MINIMAL_BUILD; }
/// Return true if ALLOW_WASM is defined as a non-zero value
constexpr bool wasm() { return ALLOW_WASM; }

/// Return true if NDEBUG is defined
constexpr bool asserts() {
#ifdef NDEBUG
  return false;
#else  // !NDEBUG
  return true;
#endif // !NDEBUG
}

/// Return true if PNACL_BROWSER_TRANSLATOR is defined
constexpr bool browser() {
#if PNACL_BROWSER_TRANSLATOR
  return true;
#else  // !PNACL_BROWSER_TRANSLATOR
  return false;
#endif // !PNACL_BROWSER_TRANSLATOR
}

/// Return true if ALLOW_EXTRA_VALIDATION is defined
constexpr bool extraValidation() {
#if ALLOW_EXTRA_VALIDATION
  return true;
#else  // !ALLOW_EXTRA_VALIDATION
  return false;
#endif // !ALLOW_EXTRA_VALIDATION
}

} // end of namespace BuildDefs
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEBUILDDEFS_H
