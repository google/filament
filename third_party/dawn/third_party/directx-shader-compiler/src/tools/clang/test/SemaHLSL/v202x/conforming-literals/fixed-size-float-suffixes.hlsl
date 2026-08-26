// RUN: %dxc -T lib_6_3 -HV 202x -enable-16bit-types -verify %s

// Verify that f16/F16/f32/F32/f64/F64 produce the expected types in HLSL 202x
// with native 16-bit types enabled.

template <typename T, typename U>
struct is_same {
  static const bool value = false;
};

template <typename T>
struct is_same<T, T> {
  static const bool value = true;
};

// expected-no-diagnostics

// 16-bit suffix -> float16_t (a.k.a. half / HalfTy).
_Static_assert(is_same<__decltype(1.0f16), half>::value, "1.0f16 is half");
_Static_assert(is_same<__decltype(1.0F16), half>::value, "1.0F16 is half");
_Static_assert(is_same<__decltype(2.5e1f16), half>::value, "2.5e1f16 is half");

// 32-bit suffix -> float, regardless of UseMinPrecision.
_Static_assert(is_same<__decltype(1.0f32), float>::value, "1.0f32 is float");
_Static_assert(is_same<__decltype(1.0F32), float>::value, "1.0F32 is float");
_Static_assert(is_same<__decltype(0.5e0f32), float>::value, "0.5e0f32 is float");

// 64-bit suffix -> double.
_Static_assert(is_same<__decltype(1.0f64), double>::value, "1.0f64 is double");
_Static_assert(is_same<__decltype(1.0F64), double>::value, "1.0F64 is double");
_Static_assert(is_same<__decltype(0.5e0f64), double>::value, "0.5e0f64 is double");

// Sanity check: the plain 'f' and 'h' suffixes still work.
_Static_assert(is_same<__decltype(1.0f), float>::value, "1.0f is float");
_Static_assert(is_same<__decltype(1.0h), half>::value, "1.0h is half");
_Static_assert(is_same<__decltype(1.0l), double>::value, "1.0l is double");

// The fixed-size suffix must not collide with an integer suffix.
_Static_assert(is_same<__decltype(1.f32), float>::value, ".f32 with no fraction digits is float");
