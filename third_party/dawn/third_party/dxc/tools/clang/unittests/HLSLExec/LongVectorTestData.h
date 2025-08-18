#ifndef LONGVECTORTESTDATA_H
#define LONGVECTORTESTDATA_H

#include <Verify.h>
#include <limits>
#include <map>
#include <string>
#include <vector>

// A helper struct because C++ bools are 1 byte and HLSL bools are 4 bytes.
// Take int32_t as a constuctor argument and convert it to bool when needed.
// Comparisons cast to a bool because we only care if the bool representation is
// true or false.
struct HLSLBool_t {
  HLSLBool_t() : Val(0) {}
  HLSLBool_t(int32_t Val) : Val(Val) {}
  HLSLBool_t(bool Val) : Val(Val) {}
  HLSLBool_t(const HLSLBool_t &Other) : Val(Other.Val) {}

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
  HLSLHalf_t(DirectX::PackedVector::HALF Val) : Val(Val) {}
  HLSLHalf_t(const HLSLHalf_t &Other) : Val(Other.Val) {}
  HLSLHalf_t(const float F) {
    Val = DirectX::PackedVector::XMConvertFloatToHalf(F);
  }
  HLSLHalf_t(const double D) {
    float F = 0.0f;
    // We wrap '::max' in () to prevent it from being expanded as a
    // macro by the Windows SDK.
    if (D >= (std::numeric_limits<double>::max)())
      F = (std::numeric_limits<float>::max)();
    else if (D <= std::numeric_limits<double>::lowest())
      F = std::numeric_limits<float>::lowest();
    else
      F = static_cast<float>(D);

    Val = DirectX::PackedVector::XMConvertFloatToHalf(F);
  }
  HLSLHalf_t(const int I) {
    VERIFY_IS_TRUE(I == 0, L"HLSLHalf_t constructor with int override only "
                           L"meant for cases when initializing to 0.");
    const float F = static_cast<float>(I);
    Val = DirectX::PackedVector::XMConvertFloatToHalf(F);
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
    return HLSLHalf_t(DirectX::PackedVector::XMConvertFloatToHalf(A * B));
  }

  HLSLHalf_t operator+(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return HLSLHalf_t(DirectX::PackedVector::XMConvertFloatToHalf(A + B));
  }

  HLSLHalf_t operator-(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return HLSLHalf_t(DirectX::PackedVector::XMConvertFloatToHalf(A - B));
  }

  HLSLHalf_t operator/(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    return HLSLHalf_t(DirectX::PackedVector::XMConvertFloatToHalf(A / B));
  }

  HLSLHalf_t operator%(const HLSLHalf_t &Other) const {
    const float A = DirectX::PackedVector::XMConvertHalfToFloat(Val);
    const float B = DirectX::PackedVector::XMConvertHalfToFloat(Other.Val);
    const float C = std::fmod(A, B);
    return HLSLHalf_t(DirectX::PackedVector::XMConvertFloatToHalf(C));
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

template <typename T> struct LongVectorTestData {
  static const std::map<std::wstring, std::vector<T>> Data;
};

template <> struct LongVectorTestData<HLSLBool_t> {
  inline static const std::map<std::wstring, std::vector<HLSLBool_t>> Data = {
      {L"DefaultInputValueSet1",
       {false, true, false, false, false, false, true, true, true, true}},
      {L"DefaultInputValueSet2",
       {true, false, false, false, false, true, true, true, false, false}},
  };
};

template <> struct LongVectorTestData<int16_t> {
  inline static const std::map<std::wstring, std::vector<int16_t>> Data = {
      {L"DefaultInputValueSet1", {-6, 1, 7, 3, 8, 4, -3, 8, 8, -2}},
      {L"DefaultInputValueSet2", {5, -6, -3, -2, 9, 3, 1, -3, -7, 2}},
  };
};

template <> struct LongVectorTestData<int32_t> {
  inline static const std::map<std::wstring, std::vector<int32_t>> Data = {
      {L"DefaultInputValueSet1", {-6, 1, 7, 3, 8, 4, -3, 8, 8, -2}},
      {L"DefaultInputValueSet2", {5, -6, -3, -2, 9, 3, 1, -3, -7, 2}},
  };
};

template <> struct LongVectorTestData<int64_t> {
  inline static const std::map<std::wstring, std::vector<int64_t>> Data = {
      {L"DefaultInputValueSet1", {-6, 11, 7, 3, 8, 4, -3, 8, 8, -2}},
      {L"DefaultInputValueSet2", {5, -1337, -3, -2, 9, 3, 1, -3, 501, 2}},
  };
};

template <> struct LongVectorTestData<uint16_t> {
  inline static const std::map<std::wstring, std::vector<uint16_t>> Data = {
      {L"DefaultInputValueSet1", {1, 699, 3, 1023, 5, 6, 0, 8, 9, 10}},
      {L"DefaultInputValueSet2", {2, 111, 3, 4, 5, 9, 21, 8, 9, 10}},
  };
};

template <> struct LongVectorTestData<uint32_t> {
  inline static const std::map<std::wstring, std::vector<uint32_t>> Data = {
      {L"DefaultInputValueSet1", {1, 2, 3, 4, 5, 0, 7, 8, 9, 10}},
      {L"DefaultInputValueSet2", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
  };
};

template <> struct LongVectorTestData<uint64_t> {
  inline static const std::map<std::wstring, std::vector<uint64_t>> Data = {
      {L"DefaultInputValueSet1", {1, 2, 3, 4, 5, 0, 7, 1000, 9, 10}},
      {L"DefaultInputValueSet2", {1, 2, 1337, 4, 5, 6, 7, 8, 9, 10}},
  };
};

template <> struct LongVectorTestData<HLSLHalf_t> {
  inline static const std::map<std::wstring, std::vector<HLSLHalf_t>> Data = {
      {L"DefaultInputValueSet1",
       {-1.0, -1.0, 1.0, -0.01, 1.0, -0.01, 1.0, -0.01, 1.0, -0.01}},
      {L"DefaultInputValueSet2",
       {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}},
      {L"DefaultClampArgs", {-1.0, 1.0}}, // Min, Max values for clamp
      // Range [ -pi/2, pi/2]
      {L"TrigonometricInputValueSet_RangeHalfPi",
       {-1.073, 0.044, -1.047, 0.313, 1.447, -0.865, 1.364, -0.715, -0.800,
        0.541}},
      {L"TrigonometricInputValueSet_RangeOne",
       {0.331, 0.727, -0.957, 0.677, -0.025, 0.495, 0.855, -0.673, -0.678,
        -0.905}},
  };
};

template <> struct LongVectorTestData<float> {
  inline static const std::map<std::wstring, std::vector<float>> Data = {
      {L"DefaultInputValueSet1",
       {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}},
      {L"DefaultInputValueSet2",
       {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}},
      // Range [ -pi/2, pi/2]
      {L"TrigonometricInputValueSet_RangeHalfPi",
       {0.315f, -0.316f, 1.409f, -0.09f, -1.569f, 1.302f, -0.326f, 0.781f,
        -1.235f, 0.623f}},
      {L"TrigonometricInputValueSet_RangeOne",
       {0.727f, 0.331f, -0.957f, 0.677f, -0.025f, 0.495f, 0.855f, -0.673f,
        -0.678f, -0.905f}},
  };
};

template <> struct LongVectorTestData<double> {
  inline static const std::map<std::wstring, std::vector<double>> Data = {
      {L"DefaultInputValueSet1",
       {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}},
      {L"DefaultInputValueSet2",
       {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}},
      // Range [ -pi/2, pi/2]
      {L"TrigonometricInputValueSet_RangeHalfPi",
       {0.807, 0.605, 1.317, 0.188, 1.566, -1.507, 0.67, -1.553, 0.194,
        -0.883}},
      {L"TrigonometricInputValueSet_RangeOne",
       {0.331, 0.277, -0.957, 0.677, -0.025, 0.495, 0.855, -0.673, -0.678,
        -0.905}},
      {L"SplitDoubleInputValueSet",
       {0.0, -1.0, 1.0, -1.0, 12345678.87654321, -1.0, 1.0, -1.0, 1.0, -1.0}},
  };
};

#endif // LONGVECTORTESTDATA_H
