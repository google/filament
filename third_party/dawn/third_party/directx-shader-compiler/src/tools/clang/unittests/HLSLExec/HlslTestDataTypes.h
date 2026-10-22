#ifndef HLSLTESTDATATYPES_H
#define HLSLTESTDATATYPES_H

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <ostream>

#include <windows.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "HlslTestUtils.h"
#include "dxc/Support/Global.h"

// These types bridge the gap between C++ and HLSL type representations.
namespace HLSLTestDataTypes {

// A helper struct because C++ bools are 1 byte and HLSL bools are 4 bytes.
// Take int32_t as a constuctor argument and convert it to bool when needed.
// Comparisons cast to a bool because we only care if the bool representation is
// true or false.
struct HLSLBool_t {
  HLSLBool_t() : Val(0) {}
  HLSLBool_t(int32_t Val) : Val(Val) {}
  HLSLBool_t(bool Val) : Val(Val) {}

  bool operator==(const HLSLBool_t &Other) const {
    return static_cast<bool>(Val) == static_cast<bool>(Other.Val);
  }

  bool operator!=(const HLSLBool_t &Other) const {
    return static_cast<bool>(Val) != static_cast<bool>(Other.Val);
  }

  bool operator<(const HLSLBool_t &Other) const { return Val < Other.Val; }

  bool operator>(const HLSLBool_t &Other) const { return Val > Other.Val; }

  bool operator<=(const HLSLBool_t &Other) const { return Val <= Other.Val; }

  bool operator>=(const HLSLBool_t &Other) const { return Val >= Other.Val; }

  HLSLBool_t operator*(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val * Other.Val);
  }

  HLSLBool_t operator+(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val + Other.Val);
  }

  HLSLBool_t operator-(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val - Other.Val);
  }

  HLSLBool_t operator/(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val / Other.Val);
  }

  HLSLBool_t operator%(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val % Other.Val);
  }

  HLSLBool_t operator&&(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val && Other.Val);
  }

  HLSLBool_t operator||(const HLSLBool_t &Other) const {
    return HLSLBool_t(Val || Other.Val);
  }

  bool AsBool() const { return static_cast<bool>(Val); }

  operator bool() const { return AsBool(); }
  operator int16_t() const { return (int16_t)(AsBool()); }
  operator int32_t() const { return (int32_t)(AsBool()); }
  operator int64_t() const { return (int64_t)(AsBool()); }
  operator uint16_t() const { return (uint16_t)(AsBool()); }
  operator uint32_t() const { return (uint32_t)(AsBool()); }
  operator uint64_t() const { return (uint64_t)(AsBool()); }
  operator float() const { return (float)(AsBool()); }
  operator double() const { return (double)(AsBool()); }

  // So we can construct std::wstrings using std::wostream
  friend std::wostream &operator<<(std::wostream &Os, const HLSLBool_t &Obj) {
    Os << static_cast<bool>(Obj.Val);
    return Os;
  }

  // So we can construct std::strings using std::ostream
  friend std::ostream &operator<<(std::ostream &Os, const HLSLBool_t &Obj) {
    Os << static_cast<bool>(Obj.Val);
    return Os;
  }

  int32_t Val = 0;
};

//  No native float16 type in C++ until C++23 . So we use uint16_t to represent
//  it. Simple little wrapping struct to help handle the right behavior.
struct HLSLHalf_t {
  HLSLHalf_t() : Val(0) {}
  HLSLHalf_t(const float F) {
    Val = DirectX::PackedVector::XMConvertFloatToHalf(F);
  }
  HLSLHalf_t(const double D) {
    float F;
    if (D >= std::numeric_limits<double>::max())
      F = std::numeric_limits<float>::max();
    else if (D <= std::numeric_limits<double>::lowest())
      F = std::numeric_limits<float>::lowest();
    else
      F = static_cast<float>(D);

    Val = DirectX::PackedVector::XMConvertFloatToHalf(F);
  }
  HLSLHalf_t(const uint32_t U) {
    float F = static_cast<float>(U);
    Val = DirectX::PackedVector::XMConvertFloatToHalf(F);
  }

  // PackedVector::HALF is a uint16. Make sure we don't ever accidentally
  // convert one of these to a HLSLHalf_t by arithmetically converting it to a
  // float.
  HLSLHalf_t(DirectX::PackedVector::HALF) = delete;

  static double GetULP(HLSLHalf_t A) {
    DXASSERT(!std::isnan(A) && !std::isinf(A),
             "ULP of NaN or infinity is undefined");

    HLSLHalf_t Next = A;
    ++Next.Val;

    double NextD = Next;
    double AD = A;
    return NextD - AD;
  }

  static HLSLHalf_t FromHALF(DirectX::PackedVector::HALF Half) {
    HLSLHalf_t H;
    H.Val = Half;
    return H;
  }

  // Implicit conversion to float for use with things like std::acos, std::tan,
  // etc
  operator float() const {
    return DirectX::PackedVector::XMConvertHalfToFloat(Val);
  }

  bool operator==(const HLSLHalf_t &Other) const {
    // Convert to floats to properly handle the '0 == -0' case which must
    // compare to true but have different uint16_t values.
    // That is, 0 == -0 is true. We store Val as a uint16_t.
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return A == B;
  }

  bool operator<(const HLSLHalf_t &Other) const {
    return DirectX::PackedVector::XMConvertHalfToFloat(Val) <
           DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
  }

  bool operator>(const HLSLHalf_t &Other) const {
    return DirectX::PackedVector::XMConvertHalfToFloat(Val) >
           DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
  }

  // Used by tolerance checks in the tests.
  bool operator>(float F) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    return A > F;
  }

  bool operator<(float F) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    return A < F;
  }

  bool operator<=(const HLSLHalf_t &Other) const {
    return DirectX::PackedVector::XMConvertHalfToFloat(Val) <=
           DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
  }

  bool operator>=(const HLSLHalf_t &Other) const {
    return DirectX::PackedVector::XMConvertHalfToFloat(Val) >=
           DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
  }

  bool operator!=(const HLSLHalf_t &Other) const { return Val != Other.Val; }

  HLSLHalf_t operator*(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return FromHALF(DirectX::PackedVector::XMConvertFloatToHalf(A * B));
  }

  HLSLHalf_t operator+(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return FromHALF((DirectX::PackedVector::XMConvertFloatToHalf(A + B)));
  }

  HLSLHalf_t operator-(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return FromHALF(DirectX::PackedVector::XMConvertFloatToHalf(A - B));
  }

  HLSLHalf_t operator/(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return FromHALF(DirectX::PackedVector::XMConvertFloatToHalf(A / B));
  }

  HLSLHalf_t operator%(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    const float C = std::fmod(A, B);
    return FromHALF(DirectX::PackedVector::XMConvertFloatToHalf(C));
  }

  // So we can construct std::wstrings using std::wostream
  friend std::wostream &operator<<(std::wostream &Os, const HLSLHalf_t &Obj) {
    Os << DirectX::PackedVector::XMConvertHalfToFloat(Obj.Val);
    return Os;
  }

  // So we can construct std::wstrings using std::wostream
  friend std::ostream &operator<<(std::ostream &Os, const HLSLHalf_t &Obj) {
    Os << DirectX::PackedVector::XMConvertHalfToFloat(Obj.Val);
    return Os;
  }

  // HALF is an alias to uint16_t
  DirectX::PackedVector::HALF Val = 0;
};

// Min precision wrapper types. Without -enable-16bit-types, min precision types
// are 32-bit in DXIL storage. These thin wrappers provide distinct C++ types
// that map to different HLSL type strings via DATA_TYPE.
struct HLSLMin16Float_t {
  constexpr HLSLMin16Float_t() : Val(0.0f) {}
  constexpr HLSLMin16Float_t(float F) : Val(F) {}
  constexpr HLSLMin16Float_t(double D) : Val(static_cast<float>(D)) {}
  explicit constexpr HLSLMin16Float_t(int I) : Val(static_cast<float>(I)) {}
  explicit constexpr HLSLMin16Float_t(uint32_t U)
      : Val(static_cast<float>(U)) {}

  constexpr operator float() const { return Val; }

  bool operator==(const HLSLMin16Float_t &O) const { return Val == O.Val; }
  bool operator!=(const HLSLMin16Float_t &O) const { return Val != O.Val; }
  bool operator<(const HLSLMin16Float_t &O) const { return Val < O.Val; }
  bool operator>(const HLSLMin16Float_t &O) const { return Val > O.Val; }
  bool operator<=(const HLSLMin16Float_t &O) const { return Val <= O.Val; }
  bool operator>=(const HLSLMin16Float_t &O) const { return Val >= O.Val; }

  HLSLMin16Float_t operator+(const HLSLMin16Float_t &O) const {
    return HLSLMin16Float_t(Val + O.Val);
  }
  HLSLMin16Float_t operator-(const HLSLMin16Float_t &O) const {
    return HLSLMin16Float_t(Val - O.Val);
  }
  HLSLMin16Float_t operator*(const HLSLMin16Float_t &O) const {
    return HLSLMin16Float_t(Val * O.Val);
  }
  HLSLMin16Float_t operator/(const HLSLMin16Float_t &O) const {
    return HLSLMin16Float_t(Val / O.Val);
  }
  HLSLMin16Float_t operator%(const HLSLMin16Float_t &O) const {
    return HLSLMin16Float_t(std::fmod(Val, O.Val));
  }

  friend std::wostream &operator<<(std::wostream &Os,
                                   const HLSLMin16Float_t &Obj) {
    Os << Obj.Val;
    return Os;
  }
  friend std::ostream &operator<<(std::ostream &Os,
                                  const HLSLMin16Float_t &Obj) {
    Os << Obj.Val;
    return Os;
  }

  float Val;
};
struct HLSLMin16Int_t {
  constexpr HLSLMin16Int_t() : Val(0) {}
  constexpr HLSLMin16Int_t(int32_t I) : Val(I) {}
  constexpr HLSLMin16Int_t(int64_t I) : Val(static_cast<int32_t>(I)) {}
  constexpr HLSLMin16Int_t(uint32_t U) : Val(static_cast<int32_t>(U)) {}
  constexpr HLSLMin16Int_t(uint64_t U) : Val(static_cast<int32_t>(U)) {}
  constexpr HLSLMin16Int_t(float F) : Val(static_cast<int32_t>(F)) {}
  constexpr HLSLMin16Int_t(double D) : Val(static_cast<int32_t>(D)) {}

  constexpr operator int32_t() const { return Val; }

  bool operator==(const HLSLMin16Int_t &O) const { return Val == O.Val; }
  bool operator!=(const HLSLMin16Int_t &O) const { return Val != O.Val; }
  bool operator<(const HLSLMin16Int_t &O) const { return Val < O.Val; }
  bool operator>(const HLSLMin16Int_t &O) const { return Val > O.Val; }
  bool operator<=(const HLSLMin16Int_t &O) const { return Val <= O.Val; }
  bool operator>=(const HLSLMin16Int_t &O) const { return Val >= O.Val; }

  HLSLMin16Int_t operator+(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val + O.Val);
  }
  HLSLMin16Int_t operator-(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val - O.Val);
  }
  HLSLMin16Int_t operator*(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val * O.Val);
  }
  HLSLMin16Int_t operator/(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val / O.Val);
  }
  HLSLMin16Int_t operator%(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val % O.Val);
  }
  HLSLMin16Int_t operator&(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val & O.Val);
  }
  HLSLMin16Int_t operator|(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val | O.Val);
  }
  HLSLMin16Int_t operator^(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val ^ O.Val);
  }
  HLSLMin16Int_t operator<<(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val << O.Val);
  }
  HLSLMin16Int_t operator>>(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val >> O.Val);
  }
  HLSLMin16Int_t operator~() const { return HLSLMin16Int_t(~Val); }
  HLSLMin16Int_t &operator<<=(const HLSLMin16Int_t &O) {
    Val <<= O.Val;
    return *this;
  }
  HLSLMin16Int_t &operator>>=(const HLSLMin16Int_t &O) {
    Val >>= O.Val;
    return *this;
  }
  HLSLMin16Int_t &operator|=(const HLSLMin16Int_t &O) {
    Val |= O.Val;
    return *this;
  }
  HLSLMin16Int_t &operator&=(const HLSLMin16Int_t &O) {
    Val &= O.Val;
    return *this;
  }
  HLSLMin16Int_t &operator^=(const HLSLMin16Int_t &O) {
    Val ^= O.Val;
    return *this;
  }
  HLSLMin16Int_t operator&&(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val && O.Val);
  }
  HLSLMin16Int_t operator||(const HLSLMin16Int_t &O) const {
    return HLSLMin16Int_t(Val || O.Val);
  }
  friend std::wostream &operator<<(std::wostream &Os,
                                   const HLSLMin16Int_t &Obj) {
    Os << Obj.Val;
    return Os;
  }
  friend std::ostream &operator<<(std::ostream &Os, const HLSLMin16Int_t &Obj) {
    Os << Obj.Val;
    return Os;
  }

  int32_t Val;
};
struct HLSLMin16Uint_t {
  constexpr HLSLMin16Uint_t() : Val(0) {}
  constexpr HLSLMin16Uint_t(uint32_t U) : Val(U) {}
  constexpr HLSLMin16Uint_t(uint64_t U) : Val(static_cast<uint32_t>(U)) {}
  constexpr HLSLMin16Uint_t(int32_t I) : Val(static_cast<uint32_t>(I)) {}
  constexpr HLSLMin16Uint_t(float F) : Val(static_cast<uint32_t>(F)) {}
  constexpr HLSLMin16Uint_t(double D) : Val(static_cast<uint32_t>(D)) {}

  constexpr operator uint32_t() const { return Val; }

  bool operator==(const HLSLMin16Uint_t &O) const { return Val == O.Val; }
  bool operator!=(const HLSLMin16Uint_t &O) const { return Val != O.Val; }
  bool operator<(const HLSLMin16Uint_t &O) const { return Val < O.Val; }
  bool operator>(const HLSLMin16Uint_t &O) const { return Val > O.Val; }
  bool operator<=(const HLSLMin16Uint_t &O) const { return Val <= O.Val; }
  bool operator>=(const HLSLMin16Uint_t &O) const { return Val >= O.Val; }

  HLSLMin16Uint_t operator+(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val + O.Val);
  }
  HLSLMin16Uint_t operator-(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val - O.Val);
  }
  HLSLMin16Uint_t operator*(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val * O.Val);
  }
  HLSLMin16Uint_t operator/(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val / O.Val);
  }
  HLSLMin16Uint_t operator%(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val % O.Val);
  }
  HLSLMin16Uint_t operator&(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val & O.Val);
  }
  HLSLMin16Uint_t operator|(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val | O.Val);
  }
  HLSLMin16Uint_t operator^(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val ^ O.Val);
  }
  HLSLMin16Uint_t operator<<(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val << O.Val);
  }
  HLSLMin16Uint_t operator>>(const HLSLMin16Uint_t &O) const {
    return HLSLMin16Uint_t(Val >> O.Val);
  }
  HLSLMin16Uint_t operator~() const { return HLSLMin16Uint_t(~Val); }
  HLSLMin16Uint_t &operator<<=(const HLSLMin16Uint_t &O) {
    Val <<= O.Val;
    return *this;
  }
  HLSLMin16Uint_t &operator>>=(const HLSLMin16Uint_t &O) {
    Val >>= O.Val;
    return *this;
  }
  HLSLMin16Uint_t &operator|=(const HLSLMin16Uint_t &O) {
    Val |= O.Val;
    return *this;
  }
  HLSLMin16Uint_t &operator&=(const HLSLMin16Uint_t &O) {
    Val &= O.Val;
    return *this;
  }
  HLSLMin16Uint_t &operator^=(const HLSLMin16Uint_t &O) {
    Val ^= O.Val;
    return *this;
  }

  bool operator&&(const HLSLMin16Uint_t &O) const { return Val && O.Val; }
  bool operator||(const HLSLMin16Uint_t &O) const { return Val || O.Val; }
  friend std::wostream &operator<<(std::wostream &Os,
                                   const HLSLMin16Uint_t &Obj) {
    Os << Obj.Val;
    return Os;
  }
  friend std::ostream &operator<<(std::ostream &Os,
                                  const HLSLMin16Uint_t &Obj) {
    Os << Obj.Val;
    return Os;
  }

  uint32_t Val;
};

enum class ValidationType {
  Epsilon,
  Ulp,
};

template <typename T>
inline bool doValuesMatch(T A, T B, double Tolerance, ValidationType) {
  if (Tolerance == 0.0)
    return A == B;

  T Diff = A > B ? A - B : B - A;
  return Diff <= Tolerance;
}

inline bool doValuesMatch(HLSLBool_t A, HLSLBool_t B, double, ValidationType) {
  return A == B;
}

inline bool doValuesMatch(HLSLHalf_t A, HLSLHalf_t B, double Tolerance,
                          ValidationType ValidationType) {
  switch (ValidationType) {
  case ValidationType::Epsilon:
    return CompareHalfEpsilon(A.Val, B.Val, static_cast<float>(Tolerance));
  case ValidationType::Ulp:
    return CompareHalfULP(A.Val, B.Val, static_cast<float>(Tolerance));
  default:
    hlsl_test::LogErrorFmt(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

// Min precision float comparison: convert to half and compare in fp16 space.
// This reuses the same tolerance values as HLSLHalf_t. Min precision is at
// least 16-bit, so fp16 tolerances are an upper bound for all cases.
inline bool doValuesMatch(HLSLMin16Float_t A, HLSLMin16Float_t B,
                          double Tolerance, ValidationType ValidationType) {
  auto HalfA = DirectX::PackedVector::XMConvertFloatToHalf(A.Val);
  auto HalfB = DirectX::PackedVector::XMConvertFloatToHalf(B.Val);
  switch (ValidationType) {
  case ValidationType::Epsilon:
    return CompareHalfEpsilon(HalfA, HalfB, static_cast<float>(Tolerance));
  case ValidationType::Ulp:
    return CompareHalfULP(HalfA, HalfB, static_cast<float>(Tolerance));
  default:
    hlsl_test::LogErrorFmt(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

inline bool doValuesMatch(HLSLMin16Int_t A, HLSLMin16Int_t B, double,
                          ValidationType) {
  return A == B;
}

inline bool doValuesMatch(HLSLMin16Uint_t A, HLSLMin16Uint_t B, double,
                          ValidationType) {
  return A == B;
}

inline bool doValuesMatch(float A, float B, double Tolerance,
                          ValidationType ValidationType) {
  switch (ValidationType) {
  case ValidationType::Epsilon:
    return CompareFloatEpsilon(A, B, static_cast<float>(Tolerance));
  case ValidationType::Ulp: {
    // Tolerance is in ULPs. Convert to int for the comparison.
    const int IntTolerance = static_cast<int>(Tolerance);
    return CompareFloatULP(A, B, IntTolerance);
  };
  default:
    hlsl_test::LogErrorFmt(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

inline bool doValuesMatch(double A, double B, double Tolerance,
                          ValidationType ValidationType) {
  switch (ValidationType) {
  case ValidationType::Epsilon:
    return CompareDoubleEpsilon(A, B, Tolerance);
  case ValidationType::Ulp: {
    // Tolerance is in ULPs. Convert to int64_t for the comparison.
    const int64_t IntTolerance = static_cast<int64_t>(Tolerance);
    return CompareDoubleULP(A, B, IntTolerance);
  };
  default:
    hlsl_test::LogErrorFmt(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

} // namespace HLSLTestDataTypes

#endif
