//===- subzero/src/IceTLS.h - thread_local workaround -----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines macros for working around the lack of support for
/// thread_local in MacOS 10.6.
///
/// This assumes std::thread is written in terms of pthread. Define
/// ICE_THREAD_LOCAL_HACK to enable the pthread workarounds.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETLS_H
#define SUBZERO_SRC_ICETLS_H

///
/// @defgroup /IceTLS Defines 5 macros for unifying thread_local and pthread:
/// @{
///
/// \def ICE_TLS_DECLARE_FIELD(Type, FieldName)
/// Declare a static thread_local field inside the current class definition.
/// "Type" needs to be a pointer type, such as int* or class Foo*.
///
/// \def ICE_TLS_DEFINE_FIELD(Type, ClassName, FieldName)
/// Define a static thread_local field outside of its class definition. The
/// field will ultimately be initialized to nullptr.
///
/// \def ICE_TLS_INIT_FIELD(FieldName)
/// Ensure the thread_local field is properly initialized. This is intended
/// to be called from within a static method of the field's class after main()
/// starts (to ensure that the pthread library is fully initialized) but before
/// any uses of ICE_TLS_GET_FIELD or ICE_TLS_SET_FIELD.
///
/// \def ICE_TLS_GET_FIELD(Type, FieldName)
/// Read the value of the static thread_local field. Must be done within the
/// context of its class.
///
/// \def ICE_TLS_SET_FIELD(FieldName, Value)
/// Write a value into the static thread_local field. Must be done within the
/// context of its class.

/// TODO(stichnot): Limit this define to only the platforms that absolutely
/// require it. And ideally, eventually remove this hack altogether.
///

///
/// \def ICE_THREAD_LOCAL_HACK
///
#ifndef ICE_THREAD_LOCAL_HACK
#define ICE_THREAD_LOCAL_HACK 1
#endif

#if ICE_THREAD_LOCAL_HACK

// For a static thread_local field F of a class C, instead of declaring and
// defining C::F, we create two static fields:
//   static pthread_key_t F__key;
//   static int F__initStatus;
//
// The F__initStatus field is used to hold the result of the
// pthread_key_create() call, where a zero value indicates success, and a
// nonzero value indicates failure or that ICE_TLS_INIT_FIELD() was never
// called. The F__key field is used as the argument to pthread_getspecific()
// and pthread_setspecific().

#include "llvm/Support/ErrorHandling.h"

#include <pthread.h>

#define ICE_TLS_DECLARE_FIELD(Type, FieldName)                                 \
  using FieldName##__type = Type;                                              \
  static pthread_key_t FieldName##__key;                                       \
  static int FieldName##__initStatus
#define ICE_TLS_DEFINE_FIELD(Type, ClassName, FieldName)                       \
  pthread_key_t ClassName::FieldName##__key;                                   \
  int ClassName::FieldName##__initStatus = 1
#define ICE_TLS_INIT_FIELD(FieldName)                                          \
  if (FieldName##__initStatus) {                                               \
    FieldName##__initStatus = pthread_key_create(&FieldName##__key, nullptr);  \
    if (FieldName##__initStatus)                                               \
      llvm::report_fatal_error("Failed to create pthread key");                \
  }
#define ICE_TLS_GET_FIELD(FieldName)                                           \
  (assert(FieldName##__initStatus == 0),                                       \
   static_cast<FieldName##__type>(pthread_getspecific(FieldName##__key)))
#define ICE_TLS_SET_FIELD(FieldName, Value)                                    \
  (assert(FieldName##__initStatus == 0),                                       \
   pthread_setspecific(FieldName##__key, (Value)))

#else // !ICE_THREAD_LOCAL_HACK

#if defined(_MSC_VER)
#define ICE_ATTRIBUTE_TLS __declspec(thread)
#else // !_MSC_VER
#define ICE_ATTRIBUTE_TLS thread_local
#endif // !_MSC_VER

#define ICE_TLS_DECLARE_FIELD(Type, FieldName)                                 \
  static ICE_ATTRIBUTE_TLS Type FieldName
#define ICE_TLS_DEFINE_FIELD(Type, ClassName, FieldName)                       \
  ICE_ATTRIBUTE_TLS Type ClassName::FieldName = nullptr
#define ICE_TLS_INIT_FIELD(FieldName)
#define ICE_TLS_GET_FIELD(FieldName) (FieldName)
#define ICE_TLS_SET_FIELD(FieldName, Value) (FieldName = (Value))

#endif // !ICE_THREAD_LOCAL_HACK

///
/// @}
///

#endif // SUBZERO_SRC_ICETLS_H
