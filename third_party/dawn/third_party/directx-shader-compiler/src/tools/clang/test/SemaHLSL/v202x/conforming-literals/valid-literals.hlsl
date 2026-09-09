// RUN: %dxc -T lib_6_3 -HV 202x -Whlsl-legacy-literal -verify %s
// RUN: %dxc -T lib_6_3 -HV 202x -Whlsl-legacy-literal -enable-16bit-types -verify %s
// RUN: %dxc -T lib_6_3 -HV 2021 -Whlsl-legacy-literal -verify %s
// RUN: %dxc -T lib_6_3 -HV 2021 -Whlsl-legacy-literal -enable-16bit-types -verify %s

template <typename T, typename U>
struct is_same {
  static const bool value = false;
};

template <typename T>
struct is_same<T, T> {
  static const bool value = true;
};

bool B; // Used for ternary operator tests below

#if !defined(__HLSL_ENABLE_16_BIT)
min16int i16;
min16uint u16;
min16float f16;
#endif

////////////////////////////////////////////////////////////////////////////////
// Literals Without Suffixes
////////////////////////////////////////////////////////////////////////////////

#if __HLSL_VERSION > 2021
_Static_assert(is_same<__decltype(1.0), float>::value, "Literals are now float");

_Static_assert(is_same<__decltype(0), int>::value, "0 is int");
_Static_assert(is_same<__decltype(1), int>::value, "1 is int");

// Decimal literals are always signed.
_Static_assert(is_same<__decltype(2147483647), int>::value, "2147483647 is int");
_Static_assert(is_same<__decltype(2147483648), int64_t>::value, "2147483648 is int64_t");
_Static_assert(is_same<__decltype(4294967296), int64_t>::value, "4294967296 is int64_t");

// This is an anomaly that exists in C as well as HLSL. This value can't be
// represented as a signed integer, but base-10 literals are always signed.
// Clang emits a warning that it is interpreting it as unsigned because that is
// not conforming to the C standard, and we get a slightly odd conversion
// warning. In HLSL `long long` and `uint64_t` (aka `long`) are the same size
// but not the same type in HLSL 202x.

// expected-warning@+1{{integer literal is too large to be represented in a signed integer type, interpreting as unsigned}}
static const uint64_t V = 9223372036854775808;

_Static_assert(is_same<__decltype(0x0), int>::value, "0x0 is int");
_Static_assert(is_same<__decltype(0x70000000), int>::value, "0x70000000 is int");
// expected-warning@+1{{literal value is treated as signed in HLSL 2021 and earlier, and unsigned in later language versions}}
_Static_assert(is_same<__decltype(0xF0000000), uint>::value, "0xF0000000 is uint");

_Static_assert(is_same<__decltype(0x7000000000000000), int64_t>::value, "0x7000000000000000 is int64_t");
// expected-warning@+1{{literal value is treated as signed in HLSL 2021 and earlier, and unsigned in later language versions}}
_Static_assert(is_same<__decltype(0xF000000000000000), uint64_t>::value, "0xF000000000000000 is uint64_t");

#else
// The `literal float` typename is not spellable so we cannot verify the truth
// in this way.
#if __HLSL_ENABLE_16_BIT
_Static_assert(!is_same<__decltype(1.0), float16_t>::value, "Literals are not float16_t");
#else
_Static_assert(!is_same<__decltype(1.0), min10float>::value, "Literals are not min10float");
_Static_assert(!is_same<__decltype(1.0), min16float>::value, "Literals are not min16float");
#endif

_Static_assert(!is_same<__decltype(1.0), half>::value, "Literals are not half");
_Static_assert(!is_same<__decltype(1.0), float>::value, "Literals are not float");
_Static_assert(!is_same<__decltype(1.0), double>::value, "Literals are not double");

#if __HLSL_ENABLE_16_BIT
_Static_assert(!is_same<__decltype(1), int16_t>::value, "Literals are not int16_t");
_Static_assert(!is_same<__decltype(1), uint16_t>::value, "Literals are not uint16_t");
#else
_Static_assert(!is_same<__decltype(1), min16int>::value, "Literals are not min16int");

_Static_assert(!is_same<__decltype(1), min16uint>::value, "Literals are not min16uint");
#endif

_Static_assert(!is_same<__decltype(1), int>::value, "Literals are not int");
_Static_assert(!is_same<__decltype(1), uint>::value, "Literals are not uint");
_Static_assert(!is_same<__decltype(1), int64_t>::value, "Literals are not int64_t");
_Static_assert(!is_same<__decltype(1), uint64_t>::value, "Literals are not uint64_t");

uint UnsignedBitMask32() {
  // expected-warning@+1{{literal value is treated as signed in HLSL 2021 and earlier, and unsigned in later language versions}}
  return 0xF0000000;
}

uint64_t UnsignedBitMask64() {
  // expected-warning@+1{{literal value is treated as signed in HLSL 2021 and earlier, and unsigned in later language versions}}
  return 0xF000000000000000;
}

uint SignedBitMask32() {
  return 0x70000000; // No warning
}

uint64_t SignedBitMask64() {
  return 0x7000000000000000; // No warning
}

uint OctUnsignedBitMask32() {
  // expected-warning@+1{{literal value is treated as signed in HLSL 2021 and earlier, and unsigned in later language versions}}
  return 020000000000;
}

uint64_t OctUnsignedBitMask64() {
  // expected-warning@+1{{literal value is treated as signed in HLSL 2021 and earlier, and unsigned in later language versions}}
  return 01000000000000000000000;
}

uint OctSignedBitMask32() {
  return 010000000000; // No warning
}

uint64_t OctSignedBitMask64() {
  return 0400000000000000000000; // No warning
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Integer literals With Suffixes
////////////////////////////////////////////////////////////////////////////////

#if __HLSL_VERSION > 2021
_Static_assert(is_same<__decltype(1l), int64_t>::value, "1l is int64_t");
_Static_assert(is_same<__decltype(1ul), uint64_t>::value, "1ul is uint64_t");
_Static_assert(is_same<__decltype(1lu), uint64_t>::value, "1lu is uint64_t");

// HLSL 2021 does not define a `long long` type, so the suffix should be
// invalid.
_Static_assert(is_same<__decltype(1ll), int64_t>::value, "1ll is int64_t");
_Static_assert(is_same<__decltype(1ull), uint64_t>::value, "1ull is uint64_t");
_Static_assert(is_same<__decltype(1llu), uint64_t>::value, "1llu is uint64_t");

// Verify that the size of `long long` is the same as the size of `int64_t`.
_Static_assert(sizeof(__decltype(1ll)) == sizeof(int64_t), "sizeof(1ll) == sizeof(int64_t)");
_Static_assert(sizeof(__decltype(1llu)) == sizeof(uint64_t), "sizeof(1llu) == sizeof(uint64_t)");

#else
_Static_assert(is_same<__decltype(1l), int32_t>::value, "1l is int32_t");
_Static_assert(is_same<__decltype(1ul), uint32_t>::value, "1ul is uint32_t");
_Static_assert(is_same<__decltype(1lu), uint32_t>::value, "1lu is uint32_t");

_Static_assert(is_same<__decltype(1ll), int64_t>::value, "1ll is int64_t");
_Static_assert(is_same<__decltype(1ull), uint64_t>::value, "1ull is uint64_t");
_Static_assert(is_same<__decltype(1llu), uint64_t>::value, "1llu is uint64_t");
#endif

////////////////////////////////////////////////////////////////////////////////
// Ternary operators on integer literals
////////////////////////////////////////////////////////////////////////////////

#if __HLSL_VERSION > 2021
_Static_assert(is_same<__decltype(B ? 1 : 1), int>::value, "B ? 1 : 1 is int");

_Static_assert(is_same<__decltype(B ? 1l : 1), int64_t>::value, "B ? 1l : 1 is int64_t");
_Static_assert(is_same<__decltype(B ? 1 : 1l), int64_t>::value, "B ? 1 : 1l is int64_t");

_Static_assert(is_same<__decltype(B ? 1ul : 1), uint64_t>::value, "B ? 1ul : 1 is uint64_t");
_Static_assert(is_same<__decltype(B ? 1 : 1ul), uint64_t>::value, "B ? 1 : 1ul is uint64_t");

#if !defined(__HLSL_ENABLE_16_BIT)
_Static_assert(is_same<__decltype(B ? i16 : i16), min16int>::value, "B ? (min16int) : (min16int) is min16int");
_Static_assert(is_same<__decltype(B ? u16 : u16), min16uint>::value, "B ? (min16uint) : (min16uint) is min16uint");
#endif

#else
_Static_assert(!is_same<__decltype(B ? 1 : 1), min12int>::value, "B ? 1 : 1 is literal int");
_Static_assert(!is_same<__decltype(B ? 1 : 1), min16int>::value, "B ? 1 : 1 is literal int");
_Static_assert(!is_same<__decltype(B ? 1 : 1), min16uint>::value, "B ? 1 : 1 is literal int");
_Static_assert(!is_same<__decltype(B ? 1 : 1), uint>::value, "B ? 1 : 1 is literal int");
_Static_assert(!is_same<__decltype(B ? 1 : 1), int>::value, "B ? 1 : 1 is literal int");
_Static_assert(!is_same<__decltype(B ? 1 : 1), uint64_t>::value, "B ? 1 : 1 is literal int");
_Static_assert(!is_same<__decltype(B ? 1 : 1), int64_t>::value, "B ? 1 : 1 is literal int");


_Static_assert(is_same<__decltype(B ? 1l : 1), int32_t>::value, "B ? 1l : 1 is int32_t");
_Static_assert(is_same<__decltype(B ? 1 : 1l), int32_t>::value, "B ? 1 : 1l is int32_t");

_Static_assert(is_same<__decltype(B ? 1ul : 1), uint32_t>::value, "B ? 1ul : 1 is uint32_t");
_Static_assert(is_same<__decltype(B ? 1 : 1ul), uint32_t>::value, "B ? 1 : 1ul is uint32_t");

_Static_assert(is_same<__decltype(B ? 1ll : 1), int64_t>::value, "B ? 1ll : 1 is int64_t");
_Static_assert(is_same<__decltype(B ? 1 : 1ll), int64_t>::value, "B ? 1 : 1ll is int64_t");

_Static_assert(is_same<__decltype(B ? 1ull : 1), uint64_t>::value, "B ? 1ull : 1 is uint64_t");
_Static_assert(is_same<__decltype(B ? 1 : 1ull), uint64_t>::value, "B ? 1 : 1ull is uint64_t");

#if !defined(__HLSL_ENABLE_16_BIT)
_Static_assert(is_same<__decltype(B ? i16 : i16), min16int>::value, "B ? (min16int) : (min16int) is min16int");
_Static_assert(is_same<__decltype(B ? u16 : u16), min16uint>::value, "B ? (min16uint) : (min16uint) is min16uint");
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Floating point literals With Suffixes
////////////////////////////////////////////////////////////////////////////////

#if __HLSL_VERSION > 2021 || defined(__HLSL_ENABLE_16_BIT)
_Static_assert(is_same<__decltype(1.0h), half>::value, "1.0h is half");
#else
_Static_assert(is_same<__decltype(1.0h), float>::value, "1.0h is float");
#endif

_Static_assert(is_same<__decltype(1.0f), float>::value, "1.0f is float");
_Static_assert(is_same<__decltype(1.0l), double>::value, "1.0l is double");

////////////////////////////////////////////////////////////////////////////////
// Ternary operators on floating point literals
////////////////////////////////////////////////////////////////////////////////

#if __HLSL_VERSION > 2021
_Static_assert(is_same<__decltype(B ? 1.0 : 1.0), float>::value, "B ? 1.0 : 1.0 is float");
#else
_Static_assert(!is_same<__decltype(B ? 1.0 : 1.0), min16float>::value, "B ? 1.0 : 1.0 is literal float");
_Static_assert(!is_same<__decltype(B ? 1.0 : 1.0), half>::value, "B ? 1.0 : 1.0 is literal float");
_Static_assert(!is_same<__decltype(B ? 1.0 : 1.0), float>::value, "B ? 1.0 : 1.0 is literal float");
_Static_assert(!is_same<__decltype(B ? 1.0 : 1.0), double>::value, "B ? 1.0 : 1.0 is literal float");
#endif


_Static_assert(is_same<__decltype(B ? 1.0l : 1.0l), double>::value, "B ? 1.0l : 1.0l is double");
_Static_assert(is_same<__decltype(B ? 1.0f : 1.0f), float>::value, "B ? 1.0f : 1.0f is float");


_Static_assert(is_same<__decltype(B ? 1.0f : 1.0l), double>::value, "B ? 1.0f : 1.0l is double");
_Static_assert(is_same<__decltype(B ? 1.0l : 1.0f), double>::value, "B ? 1.0l : 1.0f is double");

_Static_assert(is_same<__decltype(B ? 1.0l : 1.0), double>::value, "B ? 1.0l : 1.0 is double");
_Static_assert(is_same<__decltype(B ? 1.0 : 1.0l), double>::value, "B ? 1.0 : 1.0l is double");
_Static_assert(is_same<__decltype(B ? 1.0f : 1.0), float>::value, "B ? 1.0f : 1.0 is float");
_Static_assert(is_same<__decltype(B ? 1.0 : 1.0f), float>::value, "B ? 1.0 : 1.0f is float");

#if __HLSL_VERSION > 2021 || defined(__HLSL_ENABLE_16_BIT)
_Static_assert(is_same<__decltype(B ? 1.0h : 1.0h), half>::value, "B ? 1.0h : 1.0h is half");
#else
_Static_assert(is_same<__decltype(B ? 1.0h : 1.0h), float>::value, "B ? 1.0h : 1.0h is float");
#endif

_Static_assert(is_same<__decltype(B ? 1.0f : 1.0h), float>::value, "B ? 1.0f : 1.0h is float");
_Static_assert(is_same<__decltype(B ? 1.0h : 1.0f), float>::value, "B ? 1.0h : 1.0f is float");

_Static_assert(is_same<__decltype(B ? 1.0l : 1.0h), double>::value, "B ? 1.0l : 1.0h is double");
_Static_assert(is_same<__decltype(B ? 1.0h : 1.0l), double>::value, "B ? 1.0h : 1.0l is double");

#if __HLSL_VERSION > 2021 || !defined(__HLSL_ENABLE_16_BIT)
_Static_assert(is_same<__decltype(B ? 1.0h : 1.0), float>::value, "B ? 1.0h : 1.0 is float");
_Static_assert(is_same<__decltype(B ? 1.0 : 1.0h), float>::value, "B ? 1.0 : 1.0h is float");
#else
_Static_assert(is_same<__decltype(B ? 1.0h : 1.0), half>::value, "B ? 1.0h : 1.0 is half");
_Static_assert(is_same<__decltype(B ? 1.0 : 1.0h), half>::value, "B ? 1.0 : 1.0h is half");
#endif

#if !defined(__HLSL_ENABLE_16_BIT)

_Static_assert(is_same<__decltype(B ? f16 : f16), min16float>::value, "B ? (min16float) : (min16float) is min16float");

#if __HLSL_VERSION > 2021
_Static_assert(is_same<__decltype(B ? f16 : 1.0), float>::value, "B ? (min16float) : 1.0 is float");
_Static_assert(is_same<__decltype(B ? 1.0 : f16), float>::value, "B ? 1.0 : (min16float) is float");

_Static_assert(is_same<__decltype(B ? f16 : 1.0h), half>::value, "B ? (min16float) : 1.0h is half");
_Static_assert(is_same<__decltype(B ? 1.0h : f16), half>::value, "B ? 1.0h : (min16float) is half");
#else
_Static_assert(is_same<__decltype(B ? f16 : 1.0), min16float>::value, "B ? (min16float) : 1.0 is min16float");
_Static_assert(is_same<__decltype(B ? 1.0 : f16), min16float>::value, "B ? 1.0 : (min16float) is min16float");

_Static_assert(is_same<__decltype(B ? f16 : 1.0h), float>::value, "B ? (min16float) : 1.0h is float");
_Static_assert(is_same<__decltype(B ? 1.0h : f16), float>::value, "B ? 1.0h : (min16float) is float");
#endif

_Static_assert(is_same<__decltype(B ? f16 : 1.0f), float>::value, "B ? (min16float) : 1.0f is float");
_Static_assert(is_same<__decltype(B ? f16 : 1.0l), double>::value, "B ? (min16float) : 1.0l is double");

_Static_assert(is_same<__decltype(B ? 1.0f : f16), float>::value, "B ? 1.0f : (min16float) is float");
_Static_assert(is_same<__decltype(B ? 1.0l : f16), double>::value, "B ? 1.0l : (min16float) is double");

#endif
