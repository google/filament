#pragma once
#include "napi.h"

namespace Napi {

// Needs this here since the MaybeUnwrap() functions need to be in the
// same namespace as their arguments for C++ argument-dependent lookup
#ifdef NAPI_CPP_CUSTOM_NAMESPACE
namespace NAPI_CPP_CUSTOM_NAMESPACE {
#endif

// Use this when a variable or parameter is unused in order to explicitly
// silence a compiler warning about that.
template <typename T>
inline void USE(T&&) {}

/**
 * A test helper that converts MaybeOrValue<T> to T by checking that
 * MaybeOrValue is NOT an empty Maybe when NODE_ADDON_API_ENABLE_MAYBE is
 * defined.
 *
 * Do nothing when NODE_ADDON_API_ENABLE_MAYBE is not defined.
 */
template <typename T>
inline T MaybeUnwrap(MaybeOrValue<T> maybe) {
#if defined(NODE_ADDON_API_ENABLE_MAYBE)
  return maybe.Unwrap();
#else
  return maybe;
#endif
}

/**
 * A test helper that converts MaybeOrValue<T> to T by getting the value that
 * wrapped by the Maybe or return the default_value if the Maybe is empty when
 * NODE_ADDON_API_ENABLE_MAYBE is defined.
 *
 * Do nothing when NODE_ADDON_API_ENABLE_MAYBE is not defined.
 */
template <typename T>
inline T MaybeUnwrapOr(MaybeOrValue<T> maybe, const T& default_value) {
#if defined(NODE_ADDON_API_ENABLE_MAYBE)
  return maybe.UnwrapOr(default_value);
#else
  USE(default_value);
  return maybe;
#endif
}

/**
 * A test helper that converts MaybeOrValue<T> to T by getting the value that
 * wrapped by the Maybe or return false if the Maybe is empty when
 * NODE_ADDON_API_ENABLE_MAYBE is defined.
 *
 * Copying the value to out when NODE_ADDON_API_ENABLE_MAYBE is not defined.
 */
template <typename T>
inline bool MaybeUnwrapTo(MaybeOrValue<T> maybe, T* out) {
#if defined(NODE_ADDON_API_ENABLE_MAYBE)
  return maybe.UnwrapTo(out);
#else
  *out = maybe;
  return true;
#endif
}

#ifdef NAPI_CPP_CUSTOM_NAMESPACE
}  // namespace NAPI_CPP_CUSTOM_NAMESPACE
#endif

}  // namespace Napi
