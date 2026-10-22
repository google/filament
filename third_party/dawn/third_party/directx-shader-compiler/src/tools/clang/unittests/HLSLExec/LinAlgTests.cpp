///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// LinAlgTests.cpp                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Execution tests for dx::linalg builtins                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// We need to keep & fix these warnings to integrate smoothly with HLK
#pragma warning(error : 4100 4242 4244 4267 4701 4389 4018)

#define INLINE_TEST_METHOD_MARKUP
#include <WexTestClass.h>

#include "ShaderOpTest.h"
#include "dxc/DXIL/DxilConstants.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/dxcapi.use.h"

#include "HlslExecTestUtils.h"
#include "HlslTestDataTypes.h"
#include "HlslTestUtils.h"

#include <climits>
#include <cstring>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define STREAM_FLOAT(stream, name, value)                                      \
  stream << std::showpoint << " -D" << name << "=" << value << "F"             \
         << std::noshowpoint
#include <variant>
#include <vector>

namespace LinAlg {

using hlsl::DXIL::ComponentType;
using hlsl::DXIL::MatrixLayout;
using hlsl::DXIL::MatrixScope;
using hlsl::DXIL::MatrixUse;

using HLSLTestDataTypes::doValuesMatch;
using HLSLTestDataTypes::HLSLHalf_t;
using HLSLTestDataTypes::ValidationType;

using VariantCompType = std::variant<std::vector<float>, std::vector<int32_t>,
                                     std::vector<HLSLHalf_t>>;
using MatrixDim = uint32_t;

/// Return the byte size of a single element for the given component type.
static uint8_t elementSize(ComponentType CT) {
  switch (CT) {
  case ComponentType::F16:
  case ComponentType::I16:
  case ComponentType::U16:
    return 2;
  case ComponentType::F64:
  case ComponentType::I64:
  case ComponentType::U64:
    return 8;
  default:
    return 4;
  }
}

struct MatrixParams {
  ComponentType CompType;
  MatrixDim M;
  MatrixDim N;
  MatrixUse Use;
  MatrixScope Scope;
  MatrixLayout Layout;
  int NumThreads;
  bool Enable16Bit;
  bool EmulateTest;

  size_t strideBytes() const {
    uint32_t ES = elementSize(CompType);
    if (Layout == MatrixLayout::RowMajor)
      return N * ES;
    if (Layout == MatrixLayout::ColumnMajor)
      return M * ES;
    // If not Row/Col major, spec says to use 0
    return 0;
  }

  size_t totalElements() const { return M * N; }

  size_t totalBytes() const { return totalElements() * elementSize(CompType); }
};

static std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE>
toCapabilityDataType(ComponentType CompType) {
  switch (CompType) {
  case ComponentType::I16:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16;
  case ComponentType::U16:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16;
  case ComponentType::I32:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32;
  case ComponentType::U32:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32;
  case ComponentType::F16:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16;
  case ComponentType::F32:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32;
  case ComponentType::I8:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT8;
  case ComponentType::U8:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT8;
  case ComponentType::F8_E4M3FN:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN;
  case ComponentType::F8_E5M2:
    return linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E5M2;
  default:
    return std::nullopt;
  }
}

static bool applyApplicability(linalg_test::Applicability Result,
                               LPCWSTR CaseName) {
  using linalg_test::Applicability;
  switch (Result) {
  case Applicability::Execute:
    return true;
  case Applicability::NotApplicable:
    hlsl_test::LogCommentFmt(
        L"Capability-gated case %s is not applicable on this device", CaseName);
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return false;
  case Applicability::Fail:
    hlsl_test::LogErrorFmt(L"Capability evaluation failed for case %s",
                           CaseName);
    VERIFY_IS_TRUE(false, "LinAlg capability evaluation failed");
    return false;
  }
  VERIFY_IS_TRUE(false, "Unknown LinAlg applicability result");
  return false;
}

// MatrixConstruction is queried with a full {M,K,N} multiply shape, but a
// single tile only pins two of those extents and leaves the third free:
//
//   Use          Tile   Pinned          Free
//   A            MxK    M=Rows, K=Cols  N
//   B            KxN    K=Rows, N=Cols  M
//   Accumulator  MxN    M=Rows, N=Cols  K
//
// The runtime accepts a shape when every extent is a positive multiple of a
// native tile, so the free extent must be swept until one is accepted. Missing
// an extent silently skips a test case, which is the dangerous direction, so
// the sweep is exhaustive rather than a sampled set: native tile extents are
// not required to be powers of two, and the specification's own example cites
// an 8x32x16 tile. Wave-Scope Matrix Dimensions guarantees at least one
// reported shape whose largest component is <= 16 for types of 16 bits or
// larger (<= 256 bits for smaller types), so a sweep to 128 is certain to
// reach a native extent whenever the device supports the type at all. Each
// probe is a CheckFeatureSupport call with no GPU work, so the sweep is cheap.
static constexpr UINT MaxFreeExtentProbe = 128;

static HRESULT supportsMatrixShape(
    ID3D12Device *Device, linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE Type,
    UINT WaveSize, MatrixUse Use, UINT Rows, UINT Columns, bool &Supported) {
  Supported = false;
  for (UINT FreeExtent = 1; FreeExtent <= MaxFreeExtentProbe; ++FreeExtent) {
    linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
    switch (Use) {
    case MatrixUse::A:
      Shape = {Rows, Columns, FreeExtent};
      break;
    case MatrixUse::B:
      Shape = {FreeExtent, Rows, Columns};
      break;
    case MatrixUse::Accumulator:
      Shape = {Rows, FreeExtent, Columns};
      break;
    default:
      return E_INVALIDARG;
    }

    linalg_test::MatrixConstructionSupport Construction;
    const HRESULT HR = linalg_test::queryMatrixConstruction(
        Device, {Type, WaveSize, Shape}, Construction);
    if (FAILED(HR))
      return HR;
    if (Construction.supported()) {
      Supported = true;
      return S_OK;
    }
  }
  return S_OK;
}

// The shaders declare [WaveSize(4, 128)], so a capability query is only
// meaningful for wave sizes the device can actually launch within that range.
static HRESULT queryLaunchableWaveSizes(ID3D12Device *Device, UINT &MinWaveSize,
                                        UINT &MaxWaveSize) {
  MinWaveSize = 0;
  MaxWaveSize = 0;

  D3D12_FEATURE_DATA_D3D12_OPTIONS1 WaveOptions = {};
  const HRESULT HR = Device->CheckFeatureSupport(
      D3D12_FEATURE_D3D12_OPTIONS1, &WaveOptions, sizeof(WaveOptions));
  if (FAILED(HR)) {
    hlsl_test::LogCommentFmt(L"Wave-size capability query failed: 0x%08x", HR);
    return HR;
  }
  if (!WaveOptions.WaveOps)
    return S_OK;

  const auto IsPowerOfTwo = [](UINT Value) {
    return Value != 0 && (Value & (Value - 1)) == 0;
  };
  if (!IsPowerOfTwo(WaveOptions.WaveLaneCountMin) ||
      !IsPowerOfTwo(WaveOptions.WaveLaneCountMax) ||
      WaveOptions.WaveLaneCountMax < WaveOptions.WaveLaneCountMin) {
    hlsl_test::LogCommentFmt(
        L"Wave-size capability response is malformed: WaveOps=%u, min=%u, "
        L"max=%u",
        WaveOptions.WaveOps, WaveOptions.WaveLaneCountMin,
        WaveOptions.WaveLaneCountMax);
    return E_UNEXPECTED;
  }

  MinWaveSize = WaveOptions.WaveLaneCountMin;
  MaxWaveSize = WaveOptions.WaveLaneCountMax;
  return S_OK;
}

// MATRIX_CONSTRUCTION is answered per wave size, so a case that queries it must
// also compile for the size it asked about. Callers pass SelectedWaveSize to
// their runner, which pins it with FORCED_WAVE_SIZE. Without that pin the
// shader declares WaveSize(4, 128), the driver picks whatever it likes, and the
// query answers a question the test never asks.
//
// Uses lists every matrix role the case constructs. A wave size only qualifies
// if every role is supported there, because the roles pin different extents of
// the same {M, K, N} shape.
static HRESULT
selectMatrixConstructionWaveSize(ID3D12Device *Device,
                                 const MatrixParams &Params,
                                 std::initializer_list<MatrixUse> Uses,
                                 bool &Supported, UINT &SelectedWaveSize) {
  Supported = false;
  SelectedWaveSize = 0;
  if (!Device || Uses.size() == 0 ||
      !linalg_test::isLegalScope(
          linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION,
          Params.Scope))
    return E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> DataType =
      toCapabilityDataType(Params.CompType);
  if (!DataType.has_value())
    return E_INVALIDARG;

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR) || !Tier.supported())
    return HR;

  UINT MinWaveSize = 0;
  UINT MaxWaveSize = 0;
  HR = queryLaunchableWaveSizes(Device, MinWaveSize, MaxWaveSize);
  if (FAILED(HR))
    return HR;
  if (MinWaveSize == 0) {
    hlsl_test::LogCommentFmt(
        L"Wave operations are unsupported; MatrixConstruction is not "
        L"applicable");
    return S_OK;
  }

  for (UINT WaveSize = 4; WaveSize <= 128; WaveSize *= 2) {
    if (WaveSize < MinWaveSize || WaveSize > MaxWaveSize ||
        WaveSize > static_cast<UINT>(Params.NumThreads))
      continue;

    bool AllRolesSupported = true;
    for (const MatrixUse Use : Uses) {
      bool ShapeSupported = false;
      HR = supportsMatrixShape(Device, *DataType, WaveSize, Use, Params.M,
                               Params.N, ShapeSupported);
      if (FAILED(HR))
        return HR;
      if (!ShapeSupported) {
        AllRolesSupported = false;
        break;
      }
    }

    if (AllRolesSupported) {
      hlsl_test::LogCommentFmt(
          L"MatrixConstruction capability matched wave=%u for the %ux%u tile",
          WaveSize, Params.M, Params.N);
      Supported = true;
      SelectedWaveSize = WaveSize;
      return S_OK;
    }
  }

  hlsl_test::LogCommentFmt(
      L"No MatrixConstruction query supports the %ux%u tile for any wave size "
      L"launchable within shader WaveSize(4,128) and a %d-thread group",
      Params.M, Params.N, Params.NumThreads);
  return S_OK;
}

// SelectedWaveSize is only meaningful when one of these helpers returns true.
// The selectors set it on the same path that reports support, so a case cleared
// to run always has a wave size to pin with FORCED_WAVE_SIZE. It stays 0 on the
// skip and failure paths, where the caller has already returned. The assert
// keeps that an invariant rather than a convention.
static bool matrixConstructionApplicable(ID3D12Device *Device,
                                         const MatrixParams &Params,
                                         std::initializer_list<MatrixUse> Uses,
                                         LPCWSTR CaseName,
                                         UINT &SelectedWaveSize) {
  bool Supported = false;
  const HRESULT QueryResult = selectMatrixConstructionWaveSize(
      Device, Params, Uses, Supported, SelectedWaveSize);
  if (!applyApplicability(
          linalg_test::classifyApplicability(
              QueryResult, Supported,
              linalg_test::CapabilityRequirement::CapabilityGated),
          CaseName))
    return false;

  VERIFY_IS_TRUE(SelectedWaveSize != 0,
                 "A case cleared to run must have a selected wave size");
  return true;
}

// Tier support is the only capability the matrix-free operations depend on.
// They construct no matrix, so there is no shape or wave size to query.
static bool linAlgTierApplicable(ID3D12Device *Device, LPCWSTR CaseName) {
  linalg_test::TierSupport Tier;
  const HRESULT QueryResult = linalg_test::queryTierSupport(Device, Tier);
  return applyApplicability(
      linalg_test::classifyApplicability(
          QueryResult, SUCCEEDED(QueryResult) && Tier.supported(),
          linalg_test::CapabilityRequirement::CapabilityGated),
      CaseName);
}

// Accumulation store reports its destinations separately: a device may support
// accumulating into a buffer but not into groupshared memory, or the reverse.
// Tier 1 requires no formats at all here, so every case is gated.
static bool
accumulateStoreApplicable(ID3D12Device *Device, ComponentType CompType,
                          linalg_test::AtomicDestination Destination,
                          LPCWSTR CaseName) {
  bool Supported = false;
  HRESULT QueryResult = E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> DataType =
      toCapabilityDataType(CompType);
  if (DataType.has_value()) {
    linalg_test::TierSupport Tier;
    QueryResult = linalg_test::queryTierSupport(Device, Tier);
    if (SUCCEEDED(QueryResult) && Tier.supported()) {
      linalg_test::AtomicAccumulateStoreSupport Support;
      QueryResult =
          linalg_test::queryAtomicAccumulateStore(Device, {*DataType}, Support);
      if (SUCCEEDED(QueryResult))
        Supported = Support.supports(Destination);
    }
  }

  return applyApplicability(
      linalg_test::classifyApplicability(
          QueryResult, Supported,
          linalg_test::CapabilityRequirement::CapabilityGated),
      CaseName);
}

// Tier 1 requires no outer product formats, so this is always gated.
static bool outerProductApplicable(ID3D12Device *Device,
                                   ComponentType InputCompType,
                                   ComponentType ResultCompType,
                                   LPCWSTR CaseName) {
  bool Supported = false;
  HRESULT QueryResult = E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> InputType =
      toCapabilityDataType(InputCompType);
  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> ResultType =
      toCapabilityDataType(ResultCompType);
  if (InputType.has_value() && ResultType.has_value()) {
    linalg_test::TierSupport Tier;
    QueryResult = linalg_test::queryTierSupport(Device, Tier);
    if (SUCCEEDED(QueryResult) && Tier.supported()) {
      linalg_test::ThreadOuterProductSupport Support;
      QueryResult = linalg_test::queryThreadOuterProduct(
          Device, {*InputType, *ResultType}, Support);
      if (SUCCEEDED(QueryResult))
        Supported = Support.supported();
    }
  }

  return applyApplicability(
      linalg_test::classifyApplicability(
          QueryResult, Supported,
          linalg_test::CapabilityRequirement::CapabilityGated),
      CaseName);
}

// Wave matrix multiply needs both the matrices and the operation itself, and
// both are answered per wave size, so they are resolved in one pass. Fp16 x
// Fp16 -> Fp16 is Optional at Tier 1, so these cases are gated rather than
// mandatory.
static HRESULT selectWaveMatMulWaveSize(ID3D12Device *Device,
                                        const MatrixParams &Params, MatrixDim K,
                                        bool &Supported,
                                        UINT &SelectedWaveSize) {
  Supported = false;
  SelectedWaveSize = 0;
  if (!Device ||
      !linalg_test::isLegalScope(
          linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY,
          Params.Scope))
    return E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> DataType =
      toCapabilityDataType(Params.CompType);
  if (!DataType.has_value())
    return E_INVALIDARG;

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR) || !Tier.supported())
    return HR;

  UINT MinWaveSize = 0;
  UINT MaxWaveSize = 0;
  HR = queryLaunchableWaveSizes(Device, MinWaveSize, MaxWaveSize);
  if (FAILED(HR))
    return HR;
  if (MinWaveSize == 0) {
    hlsl_test::LogCommentFmt(
        L"Wave operations are unsupported; WaveMatrixMultiply is not "
        L"applicable");
    return S_OK;
  }

  for (UINT WaveSize = 4; WaveSize <= 128; WaveSize *= 2) {
    if (WaveSize < MinWaveSize || WaveSize > MaxWaveSize ||
        WaveSize > static_cast<UINT>(Params.NumThreads))
      continue;

    linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape = {};
    Shape.M = Params.M;
    Shape.K = K;
    Shape.N = Params.N;

    // A multiply pins all three extents, so the construction query names the
    // exact {M,K,N} shape instead of sweeping a free extent per operand. One
    // answer covers all three operands, because a supported shape means A
    // (MxK), B (KxN) and the accumulator (MxN) can all be constructed.
    linalg_test::MatrixConstructionSupport Construction;
    HR = linalg_test::queryMatrixConstruction(
        Device, {*DataType, WaveSize, Shape}, Construction);
    if (FAILED(HR))
      return HR;
    if (!Construction.supported())
      continue;

    linalg_test::WaveMatrixMultiplySupport Support;
    HR = linalg_test::queryWaveMatrixMultiply(
        Device, {{WaveSize, *DataType, *DataType, *DataType}, Shape}, Support);
    if (FAILED(HR))
      return HR;
    if (Support.supported()) {
      hlsl_test::LogCommentFmt(
          L"WaveMatrixMultiply capability matched wave=%u for %ux%ux%u",
          WaveSize, Params.M, K, Params.N);
      Supported = true;
      SelectedWaveSize = WaveSize;
      return S_OK;
    }
  }

  hlsl_test::LogCommentFmt(
      L"No WaveMatrixMultiply query supports %ux%ux%u for any wave size "
      L"launchable within shader WaveSize(4,128) and a %d-thread group",
      Params.M, K, Params.N, Params.NumThreads);
  return S_OK;
}

static bool waveMatMulApplicable(ID3D12Device *Device,
                                 const MatrixParams &Params, MatrixDim K,
                                 LPCWSTR CaseName, UINT &SelectedWaveSize) {
  bool Supported = false;
  const HRESULT QueryResult =
      selectWaveMatMulWaveSize(Device, Params, K, Supported, SelectedWaveSize);
  if (!applyApplicability(
          linalg_test::classifyApplicability(
              QueryResult, Supported,
              linalg_test::CapabilityRequirement::CapabilityGated),
          CaseName))
    return false;

  VERIFY_IS_TRUE(SelectedWaveSize != 0,
                 "A case cleared to run must have a selected wave size");
  return true;
}

namespace cpu_oracle {

using TypedMatrixValues =
    std::variant<std::vector<HLSLHalf_t>, std::vector<float>,
                 std::vector<int32_t>, std::vector<uint32_t>>;

struct TypedMatrix {
  MatrixDim M;
  MatrixDim N;
  TypedMatrixValues Values;

  // Derived from the active alternative rather than stored alongside it, so
  // the component type and the stored elements cannot disagree.
  ComponentType compType() const;

  size_t totalElements() const {
    return static_cast<size_t>(M) * static_cast<size_t>(N);
  }
};

struct MatrixBufferLayout {
  MatrixLayout Layout;
  size_t OffsetBytes;
  size_t StrideBytes;
};

enum class ComparisonMode {
  // Every mode compares encoded component bits exactly. Implementation freedom
  // is expressed by enumerating the permitted results rather than by an Epsilon
  // or Ulp tolerance, so a conforming result must match a candidate bit for
  // bit.
  Exact,
  PermittedResults,
  Excluded,
};

// MatrixResultOracle models matrix-valued outputs. Operations whose observable
// result is a complete destination buffer need a whole-buffer oracle instead.
struct MatrixResultOracle {
  ComparisonMode Mode;
  std::vector<TypedMatrix> Candidates;
  std::wstring PublicRule;
};

template <typename T, ComponentType CT> struct NativeComponentTraits {
  static constexpr ComponentType CompType = CT;
  static constexpr size_t Size = sizeof(T);

  static void store(BYTE *Dest, const T &Value) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Component must be trivially copyable");
    std::memcpy(Dest, &Value, sizeof(Value));
  }

  static T load(const BYTE *Source) {
    T Value;
    std::memcpy(&Value, Source, sizeof(Value));
    return Value;
  }

  static bool exactMatch(const T &Actual, const T &Expected) {
    return std::memcmp(&Actual, &Expected, sizeof(T)) == 0;
  }

  static std::wstring format(const T &Value) {
    std::wstringstream Stream;
    Stream << Value;
    return Stream.str();
  }
};

template <typename T> struct ComponentTraits;

template <>
struct ComponentTraits<float>
    : NativeComponentTraits<float, ComponentType::F32> {
  static std::wstring format(const float &Value) {
    uint32_t Bits;
    std::memcpy(&Bits, &Value, sizeof(Bits));
    std::wstringstream Stream;
    Stream << Value << L" (bits=0x" << std::hex << Bits << L")";
    return Stream.str();
  }
};

template <>
struct ComponentTraits<int32_t>
    : NativeComponentTraits<int32_t, ComponentType::I32> {};

template <>
struct ComponentTraits<uint32_t>
    : NativeComponentTraits<uint32_t, ComponentType::U32> {};

template <> struct ComponentTraits<HLSLHalf_t> {
  static constexpr ComponentType CompType = ComponentType::F16;
  static constexpr size_t Size = sizeof(uint16_t);

  static void store(BYTE *Dest, const HLSLHalf_t &Value) {
    std::memcpy(Dest, &Value.Val, sizeof(Value.Val));
  }

  static HLSLHalf_t load(const BYTE *Source) {
    uint16_t Bits;
    std::memcpy(&Bits, Source, sizeof(Bits));
    return HLSLHalf_t::FromHALF(static_cast<DirectX::PackedVector::HALF>(Bits));
  }

  static bool exactMatch(const HLSLHalf_t &Actual, const HLSLHalf_t &Expected) {
    return Actual.Val == Expected.Val;
  }

  static std::wstring format(const HLSLHalf_t &Value) {
    std::wstringstream Stream;
    Stream << static_cast<float>(Value) << L" (bits=0x" << std::hex << Value.Val
           << L")";
    return Stream.str();
  }
};

ComponentType TypedMatrix::compType() const {
  return std::visit(
      [](const auto &Elements) {
        using ElementType =
            typename std::decay<decltype(Elements)>::type::value_type;
        return ComponentTraits<ElementType>::CompType;
      },
      Values);
}

static bool checkedMultiply(size_t Left, size_t Right, size_t &Result) {
  if (Right != 0 && Left > std::numeric_limits<size_t>::max() / Right)
    return false;
  Result = Left * Right;
  return true;
}

static bool checkedAdd(size_t Left, size_t Right, size_t &Result) {
  if (Left > std::numeric_limits<size_t>::max() - Right)
    return false;
  Result = Left + Right;
  return true;
}

static bool isSupportedComponentType(ComponentType CompType) {
  switch (CompType) {
  case ComponentType::F16:
  case ComponentType::F32:
  case ComponentType::I32:
  case ComponentType::U32:
    return true;
  default:
    return false;
  }
}

static LPCWSTR componentTypeName(ComponentType CompType) {
  switch (CompType) {
  case ComponentType::I16:
    return L"I16";
  case ComponentType::I8:
    return L"I8";
  case ComponentType::U8:
    return L"U8";
  case ComponentType::F16:
    return L"F16";
  case ComponentType::F32:
    return L"F32";
  case ComponentType::I32:
    return L"I32";
  case ComponentType::U32:
    return L"U32";
  case ComponentType::F8_E4M3FN:
    return L"F8_E4M3FN";
  case ComponentType::F8_E5M2:
    return L"F8_E5M2";
  default:
    return L"Unsupported";
  }
}

static LPCWSTR comparisonModeName(ComparisonMode Mode) {
  switch (Mode) {
  case ComparisonMode::Exact:
    return L"Exact";
  case ComparisonMode::PermittedResults:
    return L"PermittedResults";
  case ComparisonMode::Excluded:
    return L"Excluded";
  }
  return L"Unknown";
}

static bool isMatrixValid(const TypedMatrix &Matrix) {
  size_t ExpectedElements;
  if (Matrix.M == 0 || Matrix.N == 0 ||
      !checkedMultiply(static_cast<size_t>(Matrix.M),
                       static_cast<size_t>(Matrix.N), ExpectedElements))
    return false;

  return std::visit(
      [ExpectedElements](const auto &Elements) {
        return Elements.size() == ExpectedElements;
      },
      Matrix.Values);
}

template <typename T>
static std::optional<TypedMatrix> makeTypedMatrix(MatrixDim M, MatrixDim N,
                                                  std::vector<T> Values) {
  size_t ExpectedElements;
  if (M == 0 || N == 0 ||
      !checkedMultiply(static_cast<size_t>(M), static_cast<size_t>(N),
                       ExpectedElements) ||
      Values.size() != ExpectedElements) {
    hlsl_test::LogErrorFmt(
        L"Invalid typed matrix dimensions or element count: M=%u, N=%u, "
        L"elements=%zu",
        M, N, Values.size());
    return std::nullopt;
  }

  return TypedMatrix{M, N, std::move(Values)};
}

static std::optional<TypedMatrix>
makeSequentialMatrix(ComponentType CompType, MatrixDim M, MatrixDim N,
                     uint32_t StartingValue = 1) {
  size_t NumElements;
  if (M == 0 || N == 0 ||
      !checkedMultiply(static_cast<size_t>(M), static_cast<size_t>(N),
                       NumElements)) {
    hlsl_test::LogErrorFmt(L"Invalid sequential matrix dimensions: M=%u, N=%u",
                           M, N);
    return std::nullopt;
  }

  size_t LastValueSize;
  if (!checkedAdd(static_cast<size_t>(StartingValue), NumElements - 1,
                  LastValueSize)) {
    hlsl_test::LogErrorFmt(L"Sequential matrix value calculation overflowed");
    return std::nullopt;
  }
  const uint64_t LastValue = static_cast<uint64_t>(LastValueSize);

  switch (CompType) {
  case ComponentType::F16: {
    if (LastValue > 65504) {
      hlsl_test::LogErrorFmt(L"F16 sequential value is out of range: %llu",
                             LastValue);
      return std::nullopt;
    }
    std::vector<HLSLHalf_t> Values;
    Values.reserve(NumElements);
    for (size_t I = 0; I < NumElements; ++I)
      Values.emplace_back(static_cast<float>(
          static_cast<uint64_t>(StartingValue) + static_cast<uint64_t>(I)));
    return makeTypedMatrix(M, N, std::move(Values));
  }
  case ComponentType::F32: {
    if (LastValue > (1u << 24)) {
      hlsl_test::LogErrorFmt(
          L"F32 sequential integer cannot be represented exactly: %llu",
          LastValue);
      return std::nullopt;
    }
    std::vector<float> Values;
    Values.reserve(NumElements);
    for (size_t I = 0; I < NumElements; ++I)
      Values.push_back(static_cast<float>(static_cast<uint64_t>(StartingValue) +
                                          static_cast<uint64_t>(I)));
    return makeTypedMatrix(M, N, std::move(Values));
  }
  case ComponentType::I32: {
    if (LastValue >
        static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
      hlsl_test::LogErrorFmt(L"I32 sequential value is out of range: %llu",
                             LastValue);
      return std::nullopt;
    }
    std::vector<int32_t> Values;
    Values.reserve(NumElements);
    for (size_t I = 0; I < NumElements; ++I)
      Values.push_back(static_cast<int32_t>(
          static_cast<uint64_t>(StartingValue) + static_cast<uint64_t>(I)));
    return makeTypedMatrix(M, N, std::move(Values));
  }
  case ComponentType::U32: {
    if (LastValue > std::numeric_limits<uint32_t>::max()) {
      hlsl_test::LogErrorFmt(L"U32 sequential value is out of range: %llu",
                             LastValue);
      return std::nullopt;
    }
    std::vector<uint32_t> Values;
    Values.reserve(NumElements);
    for (size_t I = 0; I < NumElements; ++I)
      Values.push_back(static_cast<uint32_t>(
          static_cast<uint64_t>(StartingValue) + static_cast<uint64_t>(I)));
    return makeTypedMatrix(M, N, std::move(Values));
  }
  default:
    hlsl_test::LogErrorFmt(L"Unsupported sequential matrix component type: %u",
                           static_cast<uint32_t>(CompType));
    return std::nullopt;
  }
}

template <typename T>
static std::optional<TypedMatrix>
transposeTypedMatrix(const TypedMatrix &Source) {
  const std::vector<T> &SourceValues = std::get<std::vector<T>>(Source.Values);
  std::vector<T> Result(Source.totalElements());
  for (MatrixDim Row = 0; Row < Source.M; ++Row) {
    for (MatrixDim Column = 0; Column < Source.N; ++Column) {
      const size_t SourceIndex = static_cast<size_t>(Row) * Source.N + Column;
      const size_t ResultIndex = static_cast<size_t>(Column) * Source.M + Row;
      Result[ResultIndex] = SourceValues[SourceIndex];
    }
  }
  return makeTypedMatrix(Source.N, Source.M, std::move(Result));
}

static std::optional<TypedMatrix> transposeMatrix(const TypedMatrix &Source) {
  if (!isMatrixValid(Source)) {
    hlsl_test::LogErrorFmt(L"Cannot transpose an invalid typed matrix");
    return std::nullopt;
  }

  switch (Source.compType()) {
  case ComponentType::F16:
    return transposeTypedMatrix<HLSLHalf_t>(Source);
  case ComponentType::F32:
    return transposeTypedMatrix<float>(Source);
  case ComponentType::I32:
    return transposeTypedMatrix<int32_t>(Source);
  case ComponentType::U32:
    return transposeTypedMatrix<uint32_t>(Source);
  default:
    return std::nullopt;
  }
}

static bool isRowColLayout(MatrixLayout Layout) {
  return Layout == MatrixLayout::RowMajor ||
         Layout == MatrixLayout::ColumnMajor;
}

static std::optional<size_t>
getMatrixBufferSize(ComponentType CompType, MatrixDim M, MatrixDim N,
                    const MatrixBufferLayout &Layout) {
  if (!isSupportedComponentType(CompType) || M == 0 || N == 0 ||
      !isRowColLayout(Layout.Layout)) {
    hlsl_test::LogErrorFmt(
        L"Invalid matrix buffer description: component=%s, M=%u, N=%u, "
        L"layout=%u",
        componentTypeName(CompType), M, N,
        static_cast<uint32_t>(Layout.Layout));
    return std::nullopt;
  }

  const size_t ElementBytes = elementSize(CompType);
  const size_t MajorCount = Layout.Layout == MatrixLayout::RowMajor ? M : N;
  const size_t MinorCount = Layout.Layout == MatrixLayout::RowMajor ? N : M;
  size_t PackedMinorBytes;
  if (!checkedMultiply(MinorCount, ElementBytes, PackedMinorBytes)) {
    hlsl_test::LogErrorFmt(
        L"Matrix packed row or column byte calculation overflowed: "
        L"component=%s, M=%u, N=%u",
        componentTypeName(CompType), M, N);
    return std::nullopt;
  }
  if (Layout.StrideBytes < PackedMinorBytes) {
    hlsl_test::LogErrorFmt(
        L"Matrix stride is too small: component=%s, M=%u, N=%u, stride=%zu, "
        L"required=%zu",
        componentTypeName(CompType), M, N, Layout.StrideBytes,
        PackedMinorBytes);
    return std::nullopt;
  }

  size_t LastMajorOffset;
  size_t RequiredBytes;
  if (!checkedMultiply(MajorCount - 1, Layout.StrideBytes, LastMajorOffset) ||
      !checkedAdd(Layout.OffsetBytes, LastMajorOffset, RequiredBytes) ||
      !checkedAdd(RequiredBytes, PackedMinorBytes, RequiredBytes)) {
    hlsl_test::LogErrorFmt(L"Matrix buffer size calculation overflowed");
    return std::nullopt;
  }
  return RequiredBytes;
}

static std::optional<size_t>
getMatrixBufferSize(const TypedMatrix &Matrix,
                    const MatrixBufferLayout &Layout) {
  if (!isMatrixValid(Matrix)) {
    hlsl_test::LogErrorFmt(L"Cannot size an invalid typed matrix");
    return std::nullopt;
  }
  return getMatrixBufferSize(Matrix.compType(), Matrix.M, Matrix.N, Layout);
}

static std::optional<size_t>
getElementByteOffset(ComponentType CompType, MatrixDim M, MatrixDim N,
                     MatrixDim Row, MatrixDim Column,
                     const MatrixBufferLayout &Layout) {
  if (Row >= M || Column >= N)
    return std::nullopt;

  const size_t Major = Layout.Layout == MatrixLayout::RowMajor ? Row : Column;
  const size_t Minor = Layout.Layout == MatrixLayout::RowMajor ? Column : Row;
  size_t MajorOffset;
  size_t MinorOffset;
  size_t ByteOffset;
  if (!checkedMultiply(Major, Layout.StrideBytes, MajorOffset) ||
      !checkedMultiply(Minor, elementSize(CompType), MinorOffset) ||
      !checkedAdd(Layout.OffsetBytes, MajorOffset, ByteOffset) ||
      !checkedAdd(ByteOffset, MinorOffset, ByteOffset))
    return std::nullopt;
  return ByteOffset;
}

template <typename T>
static bool writeTypedMatrixBuffer(const TypedMatrix &Matrix,
                                   const MatrixBufferLayout &Layout,
                                   std::vector<BYTE> &Buffer) {
  const std::vector<T> &Values = std::get<std::vector<T>>(Matrix.Values);
  for (MatrixDim Row = 0; Row < Matrix.M; ++Row) {
    for (MatrixDim Column = 0; Column < Matrix.N; ++Column) {
      const size_t ValueIndex = static_cast<size_t>(Row) * Matrix.N + Column;
      std::optional<size_t> ByteOffset = getElementByteOffset(
          Matrix.compType(), Matrix.M, Matrix.N, Row, Column, Layout);
      if (!ByteOffset)
        return false;
      ComponentTraits<T>::store(Buffer.data() + *ByteOffset,
                                Values[ValueIndex]);
    }
  }
  return true;
}

static bool writeMatrixBuffer(const TypedMatrix &Matrix,
                              const MatrixBufferLayout &Layout,
                              std::vector<BYTE> &Buffer) {
  std::optional<size_t> RequiredBytes = getMatrixBufferSize(Matrix, Layout);
  if (!RequiredBytes || Buffer.size() < *RequiredBytes) {
    hlsl_test::LogErrorFmt(
        L"Matrix buffer is too small: actual=%zu, required=%zu", Buffer.size(),
        RequiredBytes.value_or(0));
    return false;
  }

  switch (Matrix.compType()) {
  case ComponentType::F16:
    return writeTypedMatrixBuffer<HLSLHalf_t>(Matrix, Layout, Buffer);
  case ComponentType::F32:
    return writeTypedMatrixBuffer<float>(Matrix, Layout, Buffer);
  case ComponentType::I32:
    return writeTypedMatrixBuffer<int32_t>(Matrix, Layout, Buffer);
  case ComponentType::U32:
    return writeTypedMatrixBuffer<uint32_t>(Matrix, Layout, Buffer);
  default:
    return false;
  }
}

template <typename T>
static std::optional<TypedMatrix>
decodeTypedMatrixBuffer(ComponentType CompType, MatrixDim M, MatrixDim N,
                        const MatrixBufferLayout &Layout, const BYTE *Buffer) {
  std::vector<T> Values(static_cast<size_t>(M) * N);
  for (MatrixDim Row = 0; Row < M; ++Row) {
    for (MatrixDim Column = 0; Column < N; ++Column) {
      const size_t ValueIndex = static_cast<size_t>(Row) * N + Column;
      std::optional<size_t> ByteOffset =
          getElementByteOffset(CompType, M, N, Row, Column, Layout);
      if (!ByteOffset)
        return std::nullopt;
      Values[ValueIndex] = ComponentTraits<T>::load(Buffer + *ByteOffset);
    }
  }
  return makeTypedMatrix(M, N, std::move(Values));
}

static std::optional<TypedMatrix>
decodeMatrixBuffer(ComponentType CompType, MatrixDim M, MatrixDim N,
                   const MatrixBufferLayout &Layout, const void *Buffer,
                   size_t BufferSize) {
  std::optional<size_t> RequiredBytes =
      getMatrixBufferSize(CompType, M, N, Layout);
  if (!Buffer || !RequiredBytes || BufferSize < *RequiredBytes) {
    hlsl_test::LogErrorFmt(
        L"Cannot decode matrix buffer: actual=%zu, required=%zu", BufferSize,
        RequiredBytes.value_or(0));
    return std::nullopt;
  }

  const BYTE *Bytes = static_cast<const BYTE *>(Buffer);
  switch (CompType) {
  case ComponentType::F16:
    return decodeTypedMatrixBuffer<HLSLHalf_t>(CompType, M, N, Layout, Bytes);
  case ComponentType::F32:
    return decodeTypedMatrixBuffer<float>(CompType, M, N, Layout, Bytes);
  case ComponentType::I32:
    return decodeTypedMatrixBuffer<int32_t>(CompType, M, N, Layout, Bytes);
  case ComponentType::U32:
    return decodeTypedMatrixBuffer<uint32_t>(CompType, M, N, Layout, Bytes);
  default:
    return std::nullopt;
  }
}

// The per-element arm of the bounds-checking rule: elements whose bytes lie
// entirely inside the view keep their value, and the rest read as the default
// element value of zero. Uses the same element size the offset maths uses, so
// the boundary cannot be computed two different ways.
template <typename T>
static std::optional<TypedMatrix>
zeroTypedElementsOutsideView(const TypedMatrix &Source,
                             const MatrixBufferLayout &Layout,
                             size_t ViewBytes) {
  const std::vector<T> &SourceValues = std::get<std::vector<T>>(Source.Values);
  const size_t ElementBytes = elementSize(Source.compType());
  std::vector<T> Result(Source.totalElements());
  for (MatrixDim Row = 0; Row < Source.M; ++Row) {
    for (MatrixDim Column = 0; Column < Source.N; ++Column) {
      const size_t Index = static_cast<size_t>(Row) * Source.N + Column;
      std::optional<size_t> ByteOffset = getElementByteOffset(
          Source.compType(), Source.M, Source.N, Row, Column, Layout);
      if (!ByteOffset)
        return std::nullopt;
      size_t ElementEnd;
      if (!checkedAdd(*ByteOffset, ElementBytes, ElementEnd))
        return std::nullopt;
      Result[Index] = ElementEnd <= ViewBytes ? SourceValues[Index] : T{};
    }
  }
  return makeTypedMatrix(Source.M, Source.N, std::move(Result));
}

static std::optional<TypedMatrix>
zeroElementsOutsideView(const TypedMatrix &Source,
                        const MatrixBufferLayout &Layout, size_t ViewBytes) {
  if (!isMatrixValid(Source)) {
    hlsl_test::LogErrorFmt(L"Cannot bound an invalid typed matrix to a view");
    return std::nullopt;
  }

  switch (Source.compType()) {
  case ComponentType::F16:
    return zeroTypedElementsOutsideView<HLSLHalf_t>(Source, Layout, ViewBytes);
  case ComponentType::F32:
    return zeroTypedElementsOutsideView<float>(Source, Layout, ViewBytes);
  case ComponentType::I32:
    return zeroTypedElementsOutsideView<int32_t>(Source, Layout, ViewBytes);
  case ComponentType::U32:
    return zeroTypedElementsOutsideView<uint32_t>(Source, Layout, ViewBytes);
  default:
    hlsl_test::LogErrorFmt(L"Unsupported component type for view bounding: %u",
                           static_cast<uint32_t>(Source.compType()));
    return std::nullopt;
  }
}

template <typename T>
static bool exactMatrixMatch(const TypedMatrix &Actual,
                             const TypedMatrix &Expected,
                             size_t &FirstMismatch) {
  const std::vector<T> &ActualValues = std::get<std::vector<T>>(Actual.Values);
  const std::vector<T> &ExpectedValues =
      std::get<std::vector<T>>(Expected.Values);
  for (size_t I = 0; I < ActualValues.size(); ++I) {
    if (!ComponentTraits<T>::exactMatch(ActualValues[I], ExpectedValues[I])) {
      FirstMismatch = I;
      return false;
    }
  }
  FirstMismatch = ActualValues.size();
  return true;
}

static bool exactMatrixMatch(const TypedMatrix &Actual,
                             const TypedMatrix &Expected,
                             size_t &FirstMismatch) {
  if (!isMatrixValid(Actual) || !isMatrixValid(Expected) ||
      Actual.compType() != Expected.compType() || Actual.M != Expected.M ||
      Actual.N != Expected.N) {
    FirstMismatch = 0;
    return false;
  }

  switch (Actual.compType()) {
  case ComponentType::F16:
    return exactMatrixMatch<HLSLHalf_t>(Actual, Expected, FirstMismatch);
  case ComponentType::F32:
    return exactMatrixMatch<float>(Actual, Expected, FirstMismatch);
  case ComponentType::I32:
    return exactMatrixMatch<int32_t>(Actual, Expected, FirstMismatch);
  case ComponentType::U32:
    return exactMatrixMatch<uint32_t>(Actual, Expected, FirstMismatch);
  default:
    FirstMismatch = 0;
    return false;
  }
}

static std::wstring matrixValueString(const TypedMatrix &Matrix, size_t Index) {
  switch (Matrix.compType()) {
  case ComponentType::F16:
    return ComponentTraits<HLSLHalf_t>::format(
        std::get<std::vector<HLSLHalf_t>>(Matrix.Values)[Index]);
  case ComponentType::F32:
    return ComponentTraits<float>::format(
        std::get<std::vector<float>>(Matrix.Values)[Index]);
  case ComponentType::I32:
    return ComponentTraits<int32_t>::format(
        std::get<std::vector<int32_t>>(Matrix.Values)[Index]);
  case ComponentType::U32:
    return ComponentTraits<uint32_t>::format(
        std::get<std::vector<uint32_t>>(Matrix.Values)[Index]);
  default:
    return L"unsupported";
  }
}

static MatrixResultOracle exactResult(TypedMatrix Expected,
                                      std::wstring PublicRule) {
  return MatrixResultOracle{
      ComparisonMode::Exact, {std::move(Expected)}, std::move(PublicRule)};
}

static MatrixResultOracle permittedResults(std::vector<TypedMatrix> Candidates,
                                           std::wstring PublicRule) {
  return MatrixResultOracle{ComparisonMode::PermittedResults,
                            std::move(Candidates), std::move(PublicRule)};
}

static MatrixResultOracle excludedResult(std::wstring PublicRule) {
  return MatrixResultOracle{
      ComparisonMode::Excluded, {}, std::move(PublicRule)};
}

static bool isOracleValid(const MatrixResultOracle &Oracle) {
  if (Oracle.PublicRule.empty())
    return false;
  if (Oracle.Mode == ComparisonMode::Excluded)
    return Oracle.Candidates.empty();
  if (Oracle.Mode == ComparisonMode::Exact && Oracle.Candidates.size() != 1)
    return false;
  if (Oracle.Mode == ComparisonMode::PermittedResults &&
      Oracle.Candidates.size() < 2)
    return false;

  const TypedMatrix &First = Oracle.Candidates.front();
  if (!isMatrixValid(First))
    return false;
  for (const TypedMatrix &Candidate : Oracle.Candidates) {
    if (!isMatrixValid(Candidate) || Candidate.compType() != First.compType() ||
        Candidate.M != First.M || Candidate.N != First.N)
      return false;
  }
  return true;
}

static bool
matchesAnyCompleteCandidate(const TypedMatrix &Actual,
                            const MatrixResultOracle &Oracle,
                            std::vector<size_t> *FirstMismatches = nullptr) {
  if (!isOracleValid(Oracle) || Oracle.Mode == ComparisonMode::Excluded)
    return false;

  if (FirstMismatches)
    FirstMismatches->clear();
  for (const TypedMatrix &Candidate : Oracle.Candidates) {
    size_t FirstMismatch;
    if (exactMatrixMatch(Actual, Candidate, FirstMismatch))
      return true;
    if (FirstMismatches)
      FirstMismatches->push_back(FirstMismatch);
  }
  return false;
}

static bool verifyMatrixBuffer(const void *ActualBuffer,
                               size_t ActualBufferSize,
                               const MatrixBufferLayout &Layout,
                               const MatrixResultOracle &Oracle, bool Verbose) {
  if (!isOracleValid(Oracle)) {
    hlsl_test::LogErrorFmt(L"Invalid matrix oracle");
    return false;
  }
  if (Oracle.Mode == ComparisonMode::Excluded) {
    hlsl_test::LogErrorFmt(
        L"Excluded matrix result cannot be used as a success fallback: %s",
        Oracle.PublicRule.c_str());
    return false;
  }

  const TypedMatrix &Shape = Oracle.Candidates.front();
  std::optional<TypedMatrix> Actual =
      decodeMatrixBuffer(Shape.compType(), Shape.M, Shape.N, Layout,
                         ActualBuffer, ActualBufferSize);
  if (!Actual)
    return false;

  std::vector<size_t> FirstMismatches;
  if (matchesAnyCompleteCandidate(*Actual, Oracle, &FirstMismatches)) {
    if (Verbose) {
      hlsl_test::LogCommentFmt(
          L"Matrix comparison passed: component=%s, M=%u, N=%u, mode=%s, "
          L"rule=%s",
          componentTypeName(Shape.compType()), Shape.M, Shape.N,
          comparisonModeName(Oracle.Mode), Oracle.PublicRule.c_str());
    }
    return true;
  }

  hlsl_test::LogErrorFmt(
      L"No complete matrix candidate matched: component=%s, M=%u, N=%u, "
      L"mode=%s, rule=%s",
      componentTypeName(Shape.compType()), Shape.M, Shape.N,
      comparisonModeName(Oracle.Mode), Oracle.PublicRule.c_str());
  for (size_t CandidateIndex = 0; CandidateIndex < Oracle.Candidates.size();
       ++CandidateIndex) {
    const TypedMatrix &Candidate = Oracle.Candidates[CandidateIndex];
    const size_t Mismatch = FirstMismatches[CandidateIndex];
    const size_t Row = Mismatch / Shape.N;
    const size_t Column = Mismatch % Shape.N;
    hlsl_test::LogErrorFmt(
        L"Candidate %zu first mismatch at index=%zu, coordinate=(%zu,%zu): "
        L"actual=%s, expected=%s",
        CandidateIndex, Mismatch, Row, Column,
        matrixValueString(*Actual, Mismatch).c_str(),
        matrixValueString(Candidate, Mismatch).c_str());
  }
  return false;
}

// The bytes a matrix does not occupy -- the prologue before the offset, and
// the padding between rows when the stride exceeds a packed row -- must
// survive a store untouched.
//
// Comparing elements alone would not catch a store that damages the bytes
// around them. A store that ignored the stride entirely writes its elements to
// the wrong addresses and fails the element comparison anyway, but a store
// that places every element correctly and also widens its writes over the
// padding produces a correct matrix while silently corrupting whatever else
// shared the buffer.
// Seeding the destination with a poison pattern and checking that the
// non-element bytes still hold it separates those two cases.
//
// The pattern varies with the byte offset rather than repeating a single
// value. A constant would be indistinguishable from a store that happened to
// write that same value, and also from memory nobody wrote at all -- 0xcd, the
// obvious choice, is what the MSVC debug allocator fills fresh heap with.
// Multiplying the offset by an odd number keeps consecutive bytes distinct, so
// a store writing any constant over two or more adjacent bytes is always
// caught, and a single overwritten byte survives only if it happens to match
// the pattern at exactly that offset.
//
// Counting is kept separate from reporting so the check can be unit tested in
// both directions. verifyUntouchedBytes reports through Log::Error, which
// marks the calling test failed, so a test that deliberately supplies a
// corrupted buffer cannot call it.
static constexpr BYTE PoisonSeed = 0xa5;

static BYTE poisonByteAt(size_t Offset) {
  return static_cast<BYTE>(PoisonSeed ^ static_cast<BYTE>(Offset * 31u));
}

static void fillPoison(void *Buffer, size_t BufferSize) {
  BYTE *Bytes = static_cast<BYTE *>(Buffer);
  for (size_t I = 0; I < BufferSize; ++I)
    Bytes[I] = poisonByteAt(I);
}

// The store-side counterpart to zeroElementsOutsideView, expressed as bytes
// rather than values: a store the bounds check rejects never touches memory,
// so the elements the view does not admit whole are left holding the poison
// the destination was seeded with. Draws the boundary with the same inclusive
// end the load side uses.
static std::optional<std::vector<BYTE>>
storeBufferBoundedByView(const TypedMatrix &Source,
                         const MatrixBufferLayout &Layout, size_t ViewBytes) {
  if (!isMatrixValid(Source)) {
    hlsl_test::LogErrorFmt(L"Cannot bound an invalid typed matrix to a view");
    return std::nullopt;
  }

  std::optional<size_t> BufferSize = getMatrixBufferSize(Source, Layout);
  if (!BufferSize)
    return std::nullopt;

  std::vector<BYTE> Buffer(*BufferSize);
  fillPoison(Buffer.data(), Buffer.size());
  if (!writeMatrixBuffer(Source, Layout, Buffer))
    return std::nullopt;

  const size_t ElementBytes = elementSize(Source.compType());
  for (MatrixDim Row = 0; Row < Source.M; ++Row) {
    for (MatrixDim Column = 0; Column < Source.N; ++Column) {
      std::optional<size_t> ByteOffset = getElementByteOffset(
          Source.compType(), Source.M, Source.N, Row, Column, Layout);
      if (!ByteOffset)
        return std::nullopt;
      size_t ElementEnd;
      if (!checkedAdd(*ByteOffset, ElementBytes, ElementEnd))
        return std::nullopt;
      if (ElementEnd <= ViewBytes)
        continue;
      for (size_t I = *ByteOffset; I < ElementEnd; ++I)
        Buffer[I] = poisonByteAt(I);
    }
  }
  return Buffer;
}

// Byte-level counterpart to verifyMatrixBuffer. The permitted store outcomes
// differ in which bytes they leave alone rather than in the values they
// produce, and the poison pattern does not always decode to a comparable
// element, so they are compared as byte images.
static bool verifyStoreBuffer(const void *ActualBuffer, size_t ActualBufferSize,
                              const std::vector<std::vector<BYTE>> &Candidates,
                              const std::wstring &PublicRule, bool Verbose) {
  if (Candidates.size() < 2 || PublicRule.empty()) {
    hlsl_test::LogErrorFmt(
        L"Invalid store buffer oracle: candidates=%zu, public rule is %s",
        Candidates.size(), PublicRule.empty() ? L"empty" : L"present");
    return false;
  }

  const BYTE *Actual = static_cast<const BYTE *>(ActualBuffer);
  std::vector<size_t> FirstMismatches;
  for (const std::vector<BYTE> &Candidate : Candidates) {
    if (Candidate.size() != ActualBufferSize) {
      hlsl_test::LogErrorFmt(
          L"Store candidate is %zu bytes but the buffer read back is %zu",
          Candidate.size(), ActualBufferSize);
      return false;
    }

    size_t Mismatch = ActualBufferSize;
    for (size_t I = 0; I < ActualBufferSize; ++I) {
      if (Actual[I] != Candidate[I]) {
        Mismatch = I;
        break;
      }
    }
    if (Mismatch == ActualBufferSize) {
      if (Verbose)
        hlsl_test::LogCommentFmt(
            L"Store buffer matched a permitted outcome: %s",
            PublicRule.c_str());
      return true;
    }
    FirstMismatches.push_back(Mismatch);
  }

  hlsl_test::LogErrorFmt(L"No permitted store outcome matched: %s",
                         PublicRule.c_str());
  for (size_t I = 0; I < FirstMismatches.size(); ++I) {
    const size_t Offset = FirstMismatches[I];
    hlsl_test::LogErrorFmt(L"Candidate %zu first differs at byte %zu: "
                           L"actual=0x%02x, expected=0x%02x",
                           I, Offset, Actual[Offset], Candidates[I][Offset]);
  }
  return false;
}

// Returns the number of offending bytes, or nullopt if the buffer cannot hold
// the described matrix at all. FirstOffsets, when supplied, collects the
// leading offenders for diagnostics.
static std::optional<size_t>
countTouchedBytesOutsideElements(ComponentType CompType, MatrixDim M,
                                 MatrixDim N, const MatrixBufferLayout &Layout,
                                 const void *Buffer, size_t BufferSize,
                                 std::vector<size_t> *FirstOffsets = nullptr) {
  static constexpr size_t MaxReportedOffsets = 8;

  std::optional<size_t> RequiredBytes =
      getMatrixBufferSize(CompType, M, N, Layout);
  if (!RequiredBytes || BufferSize < *RequiredBytes)
    return std::nullopt;

  const size_t ElementBytes = elementSize(CompType);
  std::vector<bool> Owned(BufferSize, false);
  for (MatrixDim Row = 0; Row < M; ++Row) {
    for (MatrixDim Column = 0; Column < N; ++Column) {
      std::optional<size_t> ByteOffset =
          getElementByteOffset(CompType, M, N, Row, Column, Layout);
      if (!ByteOffset || *ByteOffset + ElementBytes > BufferSize)
        return std::nullopt;
      for (size_t I = 0; I < ElementBytes; ++I)
        Owned[*ByteOffset + I] = true;
    }
  }

  const BYTE *Bytes = static_cast<const BYTE *>(Buffer);
  size_t Corrupted = 0;
  for (size_t I = 0; I < BufferSize; ++I) {
    if (Owned[I] || Bytes[I] == poisonByteAt(I))
      continue;
    if (FirstOffsets && FirstOffsets->size() < MaxReportedOffsets)
      FirstOffsets->push_back(I);
    ++Corrupted;
  }
  return Corrupted;
}

// Reporting wrapper around countTouchedBytesOutsideElements for the execution
// tests.
static bool verifyUntouchedBytes(ComponentType CompType, MatrixDim M,
                                 MatrixDim N, const MatrixBufferLayout &Layout,
                                 const void *Buffer, size_t BufferSize,
                                 bool Verbose) {
  std::vector<size_t> FirstOffsets;
  std::optional<size_t> Corrupted = countTouchedBytesOutsideElements(
      CompType, M, N, Layout, Buffer, BufferSize, &FirstOffsets);

  if (!Corrupted) {
    hlsl_test::LogErrorFmt(
        L"Buffer of %zu bytes cannot hold the requested matrix layout",
        BufferSize);
    return false;
  }

  if (*Corrupted == 0) {
    if (Verbose)
      hlsl_test::LogCommentFmt(L"Every byte outside the stored elements still "
                               L"holds the poison pattern for seed 0x%02x",
                               PoisonSeed);
    return true;
  }

  for (size_t Offset : FirstOffsets)
    hlsl_test::LogErrorFmt(
        L"Byte %zu is outside every element but was overwritten: "
        L"actual=0x%02x, expected poison=0x%02x",
        Offset, static_cast<const BYTE *>(Buffer)[Offset],
        poisonByteAt(Offset));
  hlsl_test::LogErrorFmt(L"%zu bytes outside the stored elements were "
                         L"overwritten",
                         *Corrupted);
  return false;
}

} // namespace cpu_oracle

static std::string buildCompilerArgs(const MatrixParams &Params,
                                     const char *ExtraDefines = nullptr) {
  std::stringstream SS;
  SS << "-HV 2021";
  SS << " -DCOMP_TYPE=" << static_cast<int>(Params.CompType);
  SS << " -DM_DIM=" << Params.M;
  SS << " -DN_DIM=" << Params.N;
  SS << " -DUSE=" << static_cast<int>(Params.Use);
  SS << " -DSCOPE=" << static_cast<int>(Params.Scope);
  SS << " -DSTRIDE=" << Params.strideBytes();
  SS << " -DLAYOUT=" << static_cast<int>(Params.Layout);
  SS << " -DELEM_SIZE=" << static_cast<int>(elementSize(Params.CompType));
  SS << " -DNUMTHREADS=" << Params.NumThreads;
  switch (Params.CompType) {
  case ComponentType::F16:
    SS << " -DELEM_TYPE=half";
    break;
  case ComponentType::F32:
    SS << " -DELEM_TYPE=float";
    break;
  case ComponentType::I32:
    SS << " -DELEM_TYPE=int";
    break;
  case ComponentType::U32:
    SS << " -DELEM_TYPE=uint";
    break;
  default:
    VERIFY_IS_TRUE(false, "Unsupported LinAlg component type");
    break;
  }
  if (Params.Enable16Bit)
    SS << " -enable-16bit-types";
  if (ExtraDefines)
    SS << " " << ExtraDefines;
  return SS.str();
}

static bool verifyFloatBuffer(const float *Actual, const float *Expected,
                              size_t Count, bool Verbose,
                              float Tolerance = 0.0f) {
  bool Success = true;
  for (size_t I = 0; I < Count; I++) {
    if (!doValuesMatch(Actual[I], Expected[I], Tolerance,
                       ValidationType::Epsilon)) {
      hlsl_test::LogErrorFmt(L"Mismatch at index %zu: actual=%f, expected=%f",
                             I, static_cast<double>(Actual[I]),
                             static_cast<double>(Expected[I]));
      Success = false;
    } else if (Verbose) {
      hlsl_test::LogCommentFmt(L"  [%zu] actual=%f, expected=%f (OK)", I,
                               static_cast<double>(Actual[I]),
                               static_cast<double>(Expected[I]));
    }
  }
  return Success;
}

static bool verifyIntBuffer(const int32_t *Actual, const int32_t *Expected,
                            size_t Count, bool Verbose) {
  bool Success = true;
  for (size_t I = 0; I < Count; I++) {
    if (!doValuesMatch(Actual[I], Expected[I], 0.0, ValidationType::Epsilon)) {
      hlsl_test::LogErrorFmt(L"Mismatch at index %zu: actual=%d, expected=%d",
                             I, Actual[I], Expected[I]);
      Success = false;
    } else if (Verbose) {
      hlsl_test::LogCommentFmt(L"  [%zu] actual=%d, expected=%d (OK)", I,
                               Actual[I], Expected[I]);
    }
  }
  return Success;
}

static bool verifyHalfBuffer(const HLSLHalf_t *Actual,
                             const HLSLHalf_t *Expected, size_t Count,
                             bool Verbose, HLSLHalf_t Tolerance = 0.0f) {
  bool Success = true;
  for (size_t I = 0; I < Count; I++) {
    if (!doValuesMatch(Actual[I], Expected[I], Tolerance,
                       ValidationType::Epsilon)) {
      hlsl_test::LogErrorFmt(L"Mismatch at index %zu: actual=%f, expected=%f",
                             I, static_cast<float>(Actual[I]),
                             static_cast<float>(Expected[I]));
      Success = false;
    } else if (Verbose) {
      hlsl_test::LogCommentFmt(L"  [%zu] actual=%f, expected=%f (OK)", I,
                               static_cast<float>(Actual[I]),
                               static_cast<float>(Expected[I]));
    }
  }
  return Success;
}

static bool verifyComponentBuffer(ComponentType CompType, const void *Actual,
                                  VariantCompType Expected, size_t NumElements,
                                  bool Verbose) {
  switch (CompType) {
  case ComponentType::F32: {
    const float *ActualFloats = static_cast<const float *>(Actual);
    return verifyFloatBuffer(ActualFloats,
                             std::get<std::vector<float>>(Expected).data(),
                             NumElements, Verbose);
  }
  case ComponentType::I32: {
    const int32_t *ActualInts = static_cast<const int32_t *>(Actual);
    return verifyIntBuffer(ActualInts,
                           std::get<std::vector<int32_t>>(Expected).data(),
                           NumElements, Verbose);
  }
  case ComponentType::F16: {
    const HLSLHalf_t *ActualHalfs = static_cast<const HLSLHalf_t *>(Actual);
    return verifyHalfBuffer(ActualHalfs,
                            std::get<std::vector<HLSLHalf_t>>(Expected).data(),
                            NumElements, Verbose);
  }
  }
  return false;
}

static bool fillInputBuffer(LPCSTR Name, std::vector<BYTE> &Data,
                            ComponentType CompType, size_t NumElements,
                            size_t StartingVal = 1, bool Increment = true) {
  if (_stricmp(Name, "Input") != 0)
    return true;

  switch (CompType) {
  case ComponentType::F32:
  case ComponentType::I32:
  case ComponentType::F16:
    break;
  default:
    return false;
  }

  for (size_t I = 0; I < NumElements; ++I) {
    size_t Value = StartingVal + (Increment ? I : 0);
    switch (CompType) {
    case ComponentType::F32: {
      float *Ptr = reinterpret_cast<float *>(Data.data());
      Ptr[I] = static_cast<float>(Value);
      break;
    }
    case ComponentType::I32: {
      int32_t *Ptr = reinterpret_cast<int32_t *>(Data.data());
      Ptr[I] = static_cast<int32_t>(Value);
      break;
    }
    case ComponentType::F16: {
      HLSLHalf_t *Ptr = reinterpret_cast<HLSLHalf_t *>(Data.data());
      Ptr[I] = HLSLHalf_t(static_cast<float>(Value));
      break;
    }
    }
  }

  return true;
}

static VariantCompType makeExpectedMat(ComponentType CompType, MatrixDim M,
                                       MatrixDim N, float StartingVal,
                                       bool Increment = true,
                                       bool Transpose = false) {
  const size_t NumElements = M * N;
  std::vector<float> Floats(NumElements);
  std::vector<int32_t> Ints(NumElements);
  std::vector<HLSLHalf_t> Halfs(NumElements);

  for (size_t I = 0; I < M; ++I) {
    for (size_t J = 0; J < N; ++J) {
      size_t Value = I * N + J;
      size_t Idx = Transpose ? J * M + I : Value;
      switch (CompType) {
      case ComponentType::F32:
        Floats[Idx] = StartingVal + static_cast<float>(Increment ? Value : 0);
        break;
      case ComponentType::I32:
        VERIFY_IS_TRUE(StartingVal < static_cast<float>(
                                         std::numeric_limits<int32_t>::max()),
                       "Value too large to cast to int32_t");
        VERIFY_IS_TRUE(StartingVal > static_cast<float>(
                                         std::numeric_limits<int32_t>::min()),
                       "Value too small to cast to int32_t");
        Ints[Idx] = static_cast<int32_t>(StartingVal) +
                    static_cast<int32_t>(Increment ? Value : 0);
        break;
      case ComponentType::F16: {
        // Downcasting is safe here since HLSLHalf_t will clamp if F is too
        // large.
        float F = StartingVal + static_cast<float>(Increment ? Value : 0);
        Halfs[Idx] = HLSLHalf_t(F);
        break;
      }
      default:
        VERIFY_IS_TRUE(false, "Unable to fill unexpected ComponentType");
        break;
      }
    }
  }

  switch (CompType) {
  case ComponentType::F32:
    return Floats;
  case ComponentType::I32:
    return Ints;
  case ComponentType::F16:
    return Halfs;
  default:
    VERIFY_IS_TRUE(false, "Unable to fill unexpected ComponentType");
    return Floats;
  }
}

static VariantCompType makeExpectedVec(ComponentType CompType,
                                       MatrixDim NumElements, float StartingVal,
                                       bool Increment = true) {
  return makeExpectedMat(CompType, 1, NumElements, StartingVal, Increment,
                         false);
}

namespace matvec_interpretation {

static constexpr size_t OutputGuardBytes = 16;

struct CaseData {
  ComponentType MatrixType = ComponentType::Invalid;
  MatrixDim M = 0;
  MatrixDim N = 0;
  MatrixLayout Layout = MatrixLayout::RowMajor;
  ComponentType VectorInputType = ComponentType::Invalid;
  ComponentType InputInterpretation = ComponentType::Invalid;
  ComponentType BiasInputType = ComponentType::Invalid;
  ComponentType ResultType = ComponentType::Invalid;
  bool OutputSigned = true;
  std::vector<int64_t> MatrixValues;
  std::vector<int64_t> InterpretedVectorValues;
  std::vector<int64_t> BiasValues;
  std::wstring PublicRule;

  bool hasBias() const { return BiasInputType != ComponentType::Invalid; }
};

static void reportUnexpectedComponentType(LPCWSTR Function,
                                          ComponentType Type) {
  hlsl_test::LogErrorFmt(L"%s received an unexpected ComponentType: %s (%u)",
                         Function, cpu_oracle::componentTypeName(Type),
                         static_cast<unsigned>(Type));
  VERIFY_FAIL(L"Unexpected ComponentType");
}

static void reportUnsupportedComponentType(LPCWSTR Function,
                                           ComponentType Type) {
  hlsl_test::LogErrorFmt(L"%s does not yet support ComponentType: %s (%u)",
                         Function, cpu_oracle::componentTypeName(Type),
                         static_cast<unsigned>(Type));
  VERIFY_FAIL(L"Unsupported ComponentType");
}

static std::optional<size_t> componentByteSize(ComponentType Type) {
  switch (Type) {
  case ComponentType::I8:
  case ComponentType::U8:
  case ComponentType::F8_E4M3FN:
  case ComponentType::F8_E5M2:
    return 1;
  case ComponentType::I16:
  case ComponentType::U16:
  case ComponentType::F16:
  case ComponentType::BFloat16:
    return 2;
  case ComponentType::I32:
  case ComponentType::U32:
  case ComponentType::F32:
    return 4;
  case ComponentType::I64:
  case ComponentType::U64:
  case ComponentType::F64:
    return 8;
  default:
    reportUnexpectedComponentType(L"componentByteSize", Type);
    return std::nullopt;
  }
}

static MatrixDim elementsPerScalar(ComponentType Type) {
  switch (Type) {
  case ComponentType::I8:
  case ComponentType::U8:
  case ComponentType::F8_E4M3FN:
  case ComponentType::F8_E5M2:
    return 4;
  case ComponentType::BFloat16:
    return 2;
  case ComponentType::I16:
  case ComponentType::U16:
  case ComponentType::F16:
  case ComponentType::I32:
  case ComponentType::U32:
  case ComponentType::F32:
  case ComponentType::I64:
  case ComponentType::U64:
  case ComponentType::F64:
    return 1;
  default:
    reportUnexpectedComponentType(L"elementsPerScalar", Type);
    return 1;
  }
}

// True only for the byte-sized types that encodePackedVector can pack four to a
// uint. BFloat16 is also packed, but two to a uint, so it must not be routed
// through the byte packer.
static bool isPackedByteVector(ComponentType Type) {
  switch (Type) {
  case ComponentType::I8:
  case ComponentType::U8:
  case ComponentType::F8_E4M3FN:
  case ComponentType::F8_E5M2:
    return true;
  default:
    return false;
  }
}

static const char *storageTypeName(ComponentType Type) {
  if (isPackedByteVector(Type))
    return "uint";

  switch (Type) {
  case ComponentType::F16:
    return "half";
  case ComponentType::F32:
    return "float";
  case ComponentType::I32:
    return "int";
  case ComponentType::U32:
    return "uint";
  // Valid matrix component types that the host encoder cannot yet produce
  // values for.
  case ComponentType::I16:
  case ComponentType::U16:
  case ComponentType::I64:
  case ComponentType::U64:
  case ComponentType::F64:
  case ComponentType::BFloat16:
    reportUnsupportedComponentType(L"storageTypeName", Type);
    return nullptr;
  default:
    reportUnexpectedComponentType(L"storageTypeName", Type);
    return nullptr;
  }
}

static bool isEncodableComponentType(ComponentType Type) {
  switch (Type) {
  case ComponentType::I8:
  case ComponentType::U8:
  case ComponentType::F16:
  case ComponentType::F32:
  case ComponentType::I32:
  case ComponentType::U32:
    return true;
  default:
    return false;
  }
}

static MatrixDim storageElementCount(ComponentType Type,
                                     MatrixDim LogicalCount) {
  const MatrixDim PerScalar = elementsPerScalar(Type);
  return (LogicalCount + PerScalar - 1) / PerScalar;
}

static size_t storageElementByteSize(ComponentType Type) {
  return elementsPerScalar(Type) > 1 ? sizeof(uint32_t)
                                     : componentByteSize(Type).value_or(0);
}

template <typename T>
static std::vector<BYTE> encodeNativeVector(const std::vector<T> &Values) {
  static_assert(std::is_trivially_copyable<T>::value,
                "Vector values must be trivially copyable");
  std::vector<BYTE> Bytes(Values.size() * sizeof(T));
  if (!Bytes.empty())
    std::memcpy(Bytes.data(), Values.data(), Bytes.size());
  return Bytes;
}

static std::nullopt_t reportUnrepresentable(ComponentType Type, int64_t Value) {
  hlsl_test::LogErrorFmt(L"MatVec case value %lld is not representable as %s",
                         Value, cpu_oracle::componentTypeName(Type));
  return std::nullopt;
}

static std::optional<BYTE> encodeByte(ComponentType Type, int64_t Value) {
  if (Type == ComponentType::I8) {
    if (Value < std::numeric_limits<int8_t>::min() ||
        Value > std::numeric_limits<int8_t>::max())
      return reportUnrepresentable(Type, Value);
    return static_cast<BYTE>(static_cast<uint8_t>(static_cast<int8_t>(Value)));
  }
  if (Type == ComponentType::U8) {
    if (Value < 0 || Value > std::numeric_limits<uint8_t>::max())
      return reportUnrepresentable(Type, Value);
    return static_cast<BYTE>(Value);
  }
  return std::nullopt;
}

static std::optional<std::vector<BYTE>>
encodeComponents(ComponentType Type, const std::vector<int64_t> &Values) {
  switch (Type) {
  case ComponentType::I8:
  case ComponentType::U8: {
    std::vector<BYTE> Bytes;
    Bytes.reserve(Values.size());
    for (int64_t Value : Values) {
      std::optional<BYTE> Encoded = encodeByte(Type, Value);
      if (!Encoded)
        return std::nullopt;
      Bytes.push_back(*Encoded);
    }
    return Bytes;
  }
  case ComponentType::F16: {
    std::vector<HLSLHalf_t> Native;
    Native.reserve(Values.size());
    for (int64_t Value : Values) {
      const HLSLHalf_t Half(static_cast<float>(Value));
      if (static_cast<float>(Half) != static_cast<float>(Value))
        return reportUnrepresentable(Type, Value);
      Native.push_back(Half);
    }
    return encodeNativeVector(Native);
  }
  case ComponentType::F32: {
    std::vector<float> Native;
    Native.reserve(Values.size());
    for (int64_t Value : Values) {
      const float FloatValue = static_cast<float>(Value);
      if (static_cast<int64_t>(FloatValue) != Value)
        return reportUnrepresentable(Type, Value);
      Native.push_back(FloatValue);
    }
    return encodeNativeVector(Native);
  }
  case ComponentType::I32: {
    std::vector<int32_t> Native;
    Native.reserve(Values.size());
    for (int64_t Value : Values) {
      if (Value < std::numeric_limits<int32_t>::min() ||
          Value > std::numeric_limits<int32_t>::max())
        return reportUnrepresentable(Type, Value);
      Native.push_back(static_cast<int32_t>(Value));
    }
    return encodeNativeVector(Native);
  }
  case ComponentType::U32: {
    std::vector<uint32_t> Native;
    Native.reserve(Values.size());
    for (int64_t Value : Values) {
      if (Value < 0 ||
          static_cast<uint64_t>(Value) > std::numeric_limits<uint32_t>::max())
        return reportUnrepresentable(Type, Value);
      Native.push_back(static_cast<uint32_t>(Value));
    }
    return encodeNativeVector(Native);
  }
  default:
    return std::nullopt;
  }
}

static std::optional<std::vector<BYTE>>
encodePackedVector(ComponentType Type, const std::vector<int64_t> &Values) {
  if (!isPackedByteVector(Type))
    return std::nullopt;

  // Round the element count up to a whole number of 4-byte words. The count is
  // bounded by the matrix dimensions, so the addition cannot overflow.
  const size_t PaddedCount = (Values.size() + 3) & ~size_t(3);
  std::vector<BYTE> Bytes(PaddedCount, 0);

  for (size_t WordIndex = 0; WordIndex < PaddedCount / 4; ++WordIndex) {
    uint32_t Word = 0;
    for (size_t Lane = 0; Lane < 4; ++Lane) {
      const size_t ValueIndex = WordIndex * 4 + Lane;
      if (ValueIndex == Values.size())
        break;
      std::optional<BYTE> Encoded = encodeByte(Type, Values[ValueIndex]);
      if (!Encoded)
        return std::nullopt;
      // Lane zero occupies the least-significant byte of each uint.
      Word |= static_cast<uint32_t>(*Encoded) << (Lane * 8);
    }
    for (size_t ByteIndex = 0; ByteIndex < 4; ++ByteIndex)
      Bytes[WordIndex * 4 + ByteIndex] =
          static_cast<BYTE>(Word >> (ByteIndex * 8));
  }
  return Bytes;
}

static std::optional<size_t> matrixStrideBytes(const CaseData &Case) {
  const std::optional<size_t> ComponentSize =
      componentByteSize(Case.MatrixType);
  if (!ComponentSize)
    return std::nullopt;
  const size_t MinorCount =
      Case.Layout == MatrixLayout::RowMajor ? Case.N : Case.M;
  return MinorCount * *ComponentSize;
}

static std::optional<std::vector<BYTE>>
encodeMatrixBuffer(const CaseData &Case) {
  const std::optional<size_t> ComponentSize =
      componentByteSize(Case.MatrixType);
  const std::optional<size_t> Stride = matrixStrideBytes(Case);
  const std::optional<std::vector<BYTE>> Logical =
      encodeComponents(Case.MatrixType, Case.MatrixValues);
  if (!ComponentSize || !Stride || !Logical)
    return std::nullopt;

  const size_t MajorCount =
      Case.Layout == MatrixLayout::RowMajor ? Case.M : Case.N;
  std::vector<BYTE> Buffer(MajorCount * *Stride, 0);

  for (MatrixDim Row = 0; Row < Case.M; ++Row) {
    for (MatrixDim Column = 0; Column < Case.N; ++Column) {
      const size_t SourceIndex = static_cast<size_t>(Row) * Case.N + Column;
      const size_t SourceOffset = SourceIndex * *ComponentSize;
      const size_t DestinationOffset =
          Case.Layout == MatrixLayout::RowMajor
              ? static_cast<size_t>(Row) * *Stride + Column * *ComponentSize
              : static_cast<size_t>(Column) * *Stride + Row * *ComponentSize;
      std::memcpy(Buffer.data() + DestinationOffset,
                  Logical->data() + SourceOffset, *ComponentSize);
    }
  }
  return Buffer;
}

static std::vector<int64_t> calculateExpected(const CaseData &Case) {
  std::vector<int64_t> Expected(Case.M, 0);
  for (MatrixDim Row = 0; Row < Case.M; ++Row) {
    for (MatrixDim Column = 0; Column < Case.N; ++Column)
      Expected[Row] +=
          Case.MatrixValues[static_cast<size_t>(Row) * Case.N + Column] *
          Case.InterpretedVectorValues[Column];
    if (Case.hasBias())
      Expected[Row] += Case.BiasValues[Row];
  }
  return Expected;
}

static bool isCaseValid(const CaseData &Case) {
  // Dimensions.
  if (Case.M == 0 || Case.N == 0)
    return false;

  // Input counts must match the declared dimensions. MatrixDim is 32 bits, so
  // the row-by-column product cannot overflow a 64-bit comparison.
  if (Case.MatrixValues.size() != static_cast<uint64_t>(Case.M) * Case.N)
    return false;
  if (Case.InterpretedVectorValues.size() != Case.N)
    return false;

  // Layout.
  if (Case.Layout != MatrixLayout::RowMajor &&
      Case.Layout != MatrixLayout::ColumnMajor)
    return false;

  // Component types the host and the shader can both express.
  if (!isEncodableComponentType(Case.MatrixType))
    return false;
  if (!storageTypeName(Case.VectorInputType))
    return false;
  if (!storageTypeName(Case.ResultType))
    return false;

  if (Case.PublicRule.empty())
    return false;

  // A vector is either native or an InterpretedVector, which pairs a packed
  // vector with an interpretation type. A native element type paired with a
  // narrower interpretation is not a valid form.
  if (Case.VectorInputType == ComponentType::F32 &&
      Case.InputInterpretation != ComponentType::F32)
    return false;
  if (isPackedByteVector(Case.VectorInputType) &&
      Case.InputInterpretation != Case.VectorInputType)
    return false;

  // Bias values are present exactly when a bias type is declared.
  if (Case.hasBias() != !Case.BiasValues.empty())
    return false;
  if (Case.hasBias()) {
    if (Case.BiasValues.size() != Case.M)
      return false;
    if (Case.BiasInputType != Case.ResultType)
      return false;
    if (!storageTypeName(Case.BiasInputType))
      return false;
  }

  const bool ExpectedSigned = Case.ResultType != ComponentType::U32;
  return Case.OutputSigned == ExpectedSigned;
}

static std::optional<std::vector<BYTE>>
encodeVectorBuffer(const CaseData &Case) {
  if (isPackedByteVector(Case.VectorInputType))
    return encodePackedVector(Case.VectorInputType,
                              Case.InterpretedVectorValues);
  return encodeComponents(Case.VectorInputType, Case.InterpretedVectorValues);
}

static std::optional<std::vector<BYTE>>
encodeExpectedOutput(const CaseData &Case) {
  const std::vector<int64_t> Values = calculateExpected(Case);
  const std::optional<std::vector<BYTE>> Logical =
      encodeComponents(Case.ResultType, Values);
  if (!Logical)
    return std::nullopt;

  // Round the byte count up to a whole number of 4-byte words so that the
  // guard region starts on a word boundary. Both sizes are bounded by the
  // matrix dimensions, so neither addition can overflow.
  const size_t PaddedSize = (Logical->size() + 3) & ~size_t(3);
  const size_t BufferSize = PaddedSize + OutputGuardBytes;

  std::vector<BYTE> Buffer(BufferSize);
  cpu_oracle::fillPoison(Buffer.data(), Buffer.size());
  std::memcpy(Buffer.data(), Logical->data(), Logical->size());
  return Buffer;
}

static bool needs16BitTypes(ComponentType Type) {
  return Type == ComponentType::F16 || Type == ComponentType::I16 ||
         Type == ComponentType::U16;
}

static std::optional<std::string> buildCompilerArgs(const CaseData &Case) {
  const std::optional<size_t> MatrixStride = matrixStrideBytes(Case);
  const char *InputStorageType = storageTypeName(Case.VectorInputType);
  const char *OutputType = storageTypeName(Case.ResultType);
  const char *BiasStorageType =
      Case.hasBias() ? storageTypeName(Case.BiasInputType) : nullptr;
  if (!MatrixStride || !InputStorageType || !OutputType ||
      (Case.hasBias() && !BiasStorageType))
    return std::nullopt;

  std::stringstream Args;
  Args << "-HV 2021";
  Args << " -DMATRIX_COMP_TYPE=" << static_cast<int>(Case.MatrixType);
  Args << " -DM_DIM=" << Case.M;
  Args << " -DN_DIM=" << Case.N;
  Args << " -DMATRIX_STRIDE=" << *MatrixStride;
  Args << " -DMATRIX_LAYOUT=" << static_cast<int>(Case.Layout);
  Args << " -DINPUT_STORAGE_TYPE=" << InputStorageType;
  Args << " -DINPUT_STORAGE_COUNT="
       << storageElementCount(Case.VectorInputType, Case.N);
  Args << " -DINPUT_STORAGE_SIZE="
       << storageElementByteSize(Case.VectorInputType);
  Args << " -DINPUT_INTERP=" << static_cast<int>(Case.InputInterpretation);
  Args << " -DOUTPUT_TYPE=" << OutputType;
  Args << " -DOUTPUT_SIZE=" << componentByteSize(Case.ResultType).value_or(0);
  Args << " -DOUTPUT_SIGNED=" << (Case.OutputSigned ? 1 : 0);
  if (Case.hasBias()) {
    Args << " -DBIAS_STORAGE_TYPE=" << BiasStorageType;
    Args << " -DBIAS_STORAGE_COUNT="
         << storageElementCount(Case.BiasInputType, Case.M);
    Args << " -DBIAS_STORAGE_SIZE="
         << storageElementByteSize(Case.BiasInputType);
  }
  if (needs16BitTypes(Case.MatrixType) ||
      needs16BitTypes(Case.VectorInputType) ||
      needs16BitTypes(Case.BiasInputType) || needs16BitTypes(Case.ResultType))
    Args << " -enable-16bit-types";
  return Args.str();
}

static bool verifyExactBuffer(const void *ActualBuffer, size_t ActualSize,
                              const std::vector<BYTE> &Expected, bool Verbose) {
  if (ActualSize != Expected.size()) {
    hlsl_test::LogErrorFmt(
        L"MatVec output size mismatch: actual=%zu, expected=%zu", ActualSize,
        Expected.size());
    return false;
  }

  const BYTE *Actual = static_cast<const BYTE *>(ActualBuffer);
  size_t MismatchCount = 0;
  for (size_t I = 0; I < Expected.size(); ++I) {
    if (Actual[I] == Expected[I])
      continue;
    if (MismatchCount < 8)
      hlsl_test::LogErrorFmt(
          L"MatVec output byte %zu mismatch: actual=0x%02x, expected=0x%02x", I,
          Actual[I], Expected[I]);
    ++MismatchCount;
  }
  if (MismatchCount != 0) {
    hlsl_test::LogErrorFmt(L"%zu MatVec output bytes differed", MismatchCount);
    return false;
  }
  if (Verbose)
    hlsl_test::LogCommentFmt(
        L"All %zu MatVec output, padding, and guard bytes matched exactly",
        Expected.size());
  return true;
}

static const char MatVecMulShader[] = R"(
  #define USE_A 0
  #define SCOPE_THREAD 0

  ByteAddressBuffer MatrixInput : register(t0);
  ByteAddressBuffer VectorInput : register(t1);
  RWByteAddressBuffer Output : register(u2);

  [numthreads(1, 1, 1)]
  void main() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        MATRIX_COMP_TYPE, M_DIM, N_DIM, USE_A, SCOPE_THREAD)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, MatrixInput, 0, MATRIX_STRIDE, MATRIX_LAYOUT, 128);

    vector<INPUT_STORAGE_TYPE, INPUT_STORAGE_COUNT> InVec;
    for (uint I = 0; I < INPUT_STORAGE_COUNT; ++I) {
      InVec[I] =
        VectorInput.Load<INPUT_STORAGE_TYPE>(I * INPUT_STORAGE_SIZE);
    }

    vector<OUTPUT_TYPE, M_DIM> OutVec;
    __builtin_LinAlg_MatrixVectorMultiply(
      OutVec, Mat, OUTPUT_SIGNED, InVec, INPUT_INTERP);

    for (uint I = 0; I < M_DIM; ++I) {
      Output.Store<OUTPUT_TYPE>(I * OUTPUT_SIZE, OutVec[I]);
    }
  }
)";

static const char MatVecMulAddShader[] = R"(
  #define USE_A 0
  #define SCOPE_THREAD 0

  ByteAddressBuffer MatrixInput : register(t0);
  ByteAddressBuffer VectorInput : register(t1);
  ByteAddressBuffer BiasInput : register(t2);
  RWByteAddressBuffer Output : register(u3);

  [numthreads(1, 1, 1)]
  void main() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        MATRIX_COMP_TYPE, M_DIM, N_DIM, USE_A, SCOPE_THREAD)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, MatrixInput, 0, MATRIX_STRIDE, MATRIX_LAYOUT, 128);

    vector<INPUT_STORAGE_TYPE, INPUT_STORAGE_COUNT> InVec;
    for (uint I = 0; I < INPUT_STORAGE_COUNT; ++I) {
      InVec[I] =
        VectorInput.Load<INPUT_STORAGE_TYPE>(I * INPUT_STORAGE_SIZE);
    }

    vector<BIAS_STORAGE_TYPE, BIAS_STORAGE_COUNT> BiasVec;
    for (uint I = 0; I < BIAS_STORAGE_COUNT; ++I) {
      BiasVec[I] = BiasInput.Load<BIAS_STORAGE_TYPE>(I * BIAS_STORAGE_SIZE);
    }

    vector<OUTPUT_TYPE, M_DIM> OutVec;
    __builtin_LinAlg_MatrixVectorMultiplyAdd(
      OutVec, Mat, OUTPUT_SIGNED, InVec, INPUT_INTERP, BiasVec);

    for (uint I = 0; I < M_DIM; ++I) {
      Output.Store<OUTPUT_TYPE>(I * OUTPUT_SIZE, OutVec[I]);
    }
  }
)";

static HRESULT querySupport(ID3D12Device *Device, const CaseData &Case,
                            bool &TierSupported, bool &Supported) {
  TierSupported = false;
  Supported = false;
  if (!Device)
    return E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> VectorType =
      toCapabilityDataType(Case.VectorInputType);
  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> MatrixType =
      toCapabilityDataType(Case.MatrixType);
  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> BiasType =
      Case.hasBias() ? toCapabilityDataType(Case.BiasInputType)
                     : std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE>(
                           linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE);
  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> ResultType =
      toCapabilityDataType(Case.ResultType);
  if (!VectorType || !MatrixType || !BiasType || !ResultType)
    return E_INVALIDARG;

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR))
    return HR;
  TierSupported = Tier.supported();
  if (!TierSupported)
    return S_OK;

  linalg_test::ThreadVectorMatrixMultiplySupport Multiply;
  HR = linalg_test::queryThreadVectorMatrixMultiply(
      Device, {*VectorType, *MatrixType, *BiasType, *ResultType}, Multiply);
  if (FAILED(HR))
    return HR;

  Supported = Multiply.supported();
  if (!Supported)
    hlsl_test::LogCommentFmt(
        L"ThreadVectorMatrixMultiply reports vector=%u matrix=%u bias=%u "
        L"result=%u layout=%u is unsupported",
        static_cast<UINT>(*VectorType), static_cast<UINT>(*MatrixType),
        static_cast<UINT>(*BiasType), static_cast<UINT>(*ResultType),
        static_cast<UINT>(Case.Layout));
  return S_OK;
}

static void runCase(ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
                    const CaseData &Case, bool Verbose) {
  const bool Valid = isCaseValid(Case);
  VERIFY_IS_TRUE(Valid, "Invalid MatVec interpretation case");
  if (!Valid)
    return;

  const std::optional<std::vector<BYTE>> MatrixBuffer =
      encodeMatrixBuffer(Case);
  const std::optional<std::vector<BYTE>> VectorBuffer =
      encodeVectorBuffer(Case);
  const std::optional<std::vector<BYTE>> BiasBuffer =
      Case.hasBias() ? encodeComponents(Case.BiasInputType, Case.BiasValues)
                     : std::optional<std::vector<BYTE>>();
  const std::optional<std::vector<BYTE>> ExpectedOutput =
      encodeExpectedOutput(Case);
  const std::optional<std::string> Args = buildCompilerArgs(Case);
  VERIFY_IS_TRUE(MatrixBuffer.has_value());
  VERIFY_IS_TRUE(VectorBuffer.has_value());
  VERIFY_IS_TRUE(!Case.hasBias() || BiasBuffer.has_value());
  VERIFY_IS_TRUE(ExpectedOutput.has_value());
  VERIFY_IS_TRUE(Args.has_value());
  if (!MatrixBuffer || !VectorBuffer || (Case.hasBias() && !BiasBuffer) ||
      !ExpectedOutput || !Args)
    return;

  const char *Shader = Case.hasBias() ? MatVecMulAddShader : MatVecMulShader;
  const char *RootSignature = Case.hasBias()
                                  ? "SRV(t0), SRV(t1), SRV(t2), UAV(u3)"
                                  : "SRV(t0), SRV(t1), UAV(u2)";
  compileShader(DxcSupport, Shader, "cs_6_10", *Args, Verbose);

  auto Op = createComputeOp(Shader, "cs_6_10", RootSignature, Args->c_str());
  addSRVBuffer(Op.get(), "MatrixInput", MatrixBuffer->size(), "byname");
  addSRVBuffer(Op.get(), "VectorInput", VectorBuffer->size(), "byname");
  if (Case.hasBias())
    addSRVBuffer(Op.get(), "BiasInput", BiasBuffer->size(), "byname");
  addUAVBuffer(Op.get(), "Output", ExpectedOutput->size(), true, "byname");
  addRootView(Op.get(), 0, "MatrixInput");
  addRootView(Op.get(), 1, "VectorInput");
  if (Case.hasBias()) {
    addRootView(Op.get(), 2, "BiasInput");
    addRootView(Op.get(), 3, "Output");
  } else {
    addRootView(Op.get(), 2, "Output");
  }

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
                    if (_stricmp(Name, "Output") == 0) {
                      cpu_oracle::fillPoison(Data.data(), Data.size());
                      return;
                    }

                    const std::vector<BYTE> *Source = nullptr;
                    if (_stricmp(Name, "MatrixInput") == 0)
                      Source = &*MatrixBuffer;
                    else if (_stricmp(Name, "VectorInput") == 0)
                      Source = &*VectorBuffer;
                    else if (Case.hasBias() && _stricmp(Name, "BiasInput") == 0)
                      Source = &*BiasBuffer;
                    VERIFY_IS_TRUE(Source != nullptr,
                                   "Unexpected MatVec resource initializer");
                    if (!Source)
                      return;
                    VERIFY_IS_TRUE(Data.size() == Source->size(),
                                   "MatVec resource initializer size mismatch");
                    if (Data.size() == Source->size())
                      std::memcpy(Data.data(), Source->data(), Data.size());
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(verifyExactBuffer(OutData.data(), OutData.size(),
                                   *ExpectedOutput, Verbose));
}

static void runCapabilityChecked(ID3D12Device *Device,
                                 dxc::SpecificDllLoader &DxcSupport,
                                 const CaseData &Case,
                                 linalg_test::CapabilityRequirement Requirement,
                                 LPCWSTR CaseName, bool Verbose) {
  bool TierSupported = false;
  bool Supported = false;
  const HRESULT QueryResult =
      querySupport(Device, Case, TierSupported, Supported);
  const linalg_test::CapabilityRequirement Effective =
      SUCCEEDED(QueryResult) && !TierSupported
          ? linalg_test::CapabilityRequirement::CapabilityGated
          : Requirement;
  if (!applyApplicability(
          linalg_test::classifyApplicability(QueryResult, Supported, Effective),
          CaseName))
    return;
  runCase(Device, DxcSupport, Case, Verbose);
}

static CaseData makeNonUniformF16Case(MatrixLayout Layout) {
  CaseData Case = {};
  Case.MatrixType = ComponentType::F16;
  Case.M = 4;
  Case.N = 8;
  Case.Layout = Layout;
  Case.VectorInputType = ComponentType::F16;
  Case.InputInterpretation = ComponentType::F16;
  Case.ResultType = ComponentType::F16;
  Case.MatrixValues = {
      1,  0, -1, 2, -2, 3, -3, 1,  0, 1,  2, -1, 3, -2, 1,  -3,
      -1, 2, 0,  1, -2, 1, 3,  -1, 2, -1, 1, 0,  1, -3, -2, 3,
  };
  Case.InterpretedVectorValues = {1, -2, 3, -1, 2, -3, 1, 2};
  Case.PublicRule =
      Layout == MatrixLayout::RowMajor
          ? L"Exact non-uniform F16 RowMajor matrix-vector dot products"
          : L"Exact non-uniform F16 ColumnMajor matrix-vector dot products";
  return Case;
}

static CaseData makeSInt8Case() {
  CaseData Case = {};
  Case.MatrixType = ComponentType::I8;
  Case.M = 4;
  Case.N = 8;
  Case.Layout = MatrixLayout::RowMajor;
  Case.VectorInputType = ComponentType::I8;
  Case.InputInterpretation = ComponentType::I8;
  Case.ResultType = ComponentType::I32;
  Case.MatrixValues = {
      1, -2, 3, -4, 5, -6, 7, -8, -1, 2,  -3, 4,  -5, 6,  -7, 8,
      1, 1,  1, 1,  1, 1,  1, 1,  -8, -7, -6, -5, -4, -3, -2, -1,
  };
  Case.InterpretedVectorValues = {1, -1, 2, -2, 3, -3, 4, -4};
  Case.PublicRule =
      L"Exact packed SInt8 vector times SInt8 matrix dot products";
  return Case;
}

static CaseData makeUInt8Case() {
  CaseData Case = {};
  Case.MatrixType = ComponentType::U8;
  Case.M = 4;
  Case.N = 8;
  Case.Layout = MatrixLayout::RowMajor;
  Case.VectorInputType = ComponentType::U8;
  Case.InputInterpretation = ComponentType::U8;
  Case.ResultType = ComponentType::I32;
  Case.MatrixValues = {
      255, 1, 2,   3, 4,   5, 6,   7, 128, 127, 1, 1,   1, 1,   1, 1,
      200, 0, 200, 0, 200, 0, 200, 0, 0,   200, 0, 200, 0, 200, 0, 200,
  };
  Case.InterpretedVectorValues = {1, 255, 2, 254, 3, 253, 4, 252};
  Case.PublicRule =
      L"Exact packed UInt8 vector times UInt8 matrix dot products";
  return Case;
}

static CaseData makeUInt32OutputCase() {
  CaseData Case = {};
  Case.MatrixType = ComponentType::U32;
  Case.M = 4;
  Case.N = 8;
  Case.Layout = MatrixLayout::RowMajor;
  Case.VectorInputType = ComponentType::U32;
  Case.InputInterpretation = ComponentType::U32;
  Case.ResultType = ComponentType::U32;
  Case.OutputSigned = false;
  Case.MatrixValues = {
      2147483648LL, 0, 0,   0, 0,   0, 0,   0, 1, 2,   3, 4,   5, 6,   7, 8,
      100,          0, 100, 0, 100, 0, 100, 0, 0, 200, 0, 200, 0, 200, 0, 200,
  };
  Case.InterpretedVectorValues = {1, 1, 1, 1, 1, 1, 1, 1};
  Case.PublicRule =
      L"Exact native UInt32 matrix-vector results with unsigned output";
  return Case;
}

} // namespace matvec_interpretation

// Harness self-check for the CPU oracle. Deliberately carries no Kits metadata
// so HLK runs never select it; drivers are not certified against this class.
class LinAlgCPUOracleTests {
public:
  BEGIN_TEST_CLASS(LinAlgCPUOracleTests)
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_METHOD(TypedMatrixBufferRoundTrip);
  TEST_METHOD(MatrixProductOracle);
  TEST_METHOD(UntouchedByteVerification);
  TEST_METHOD(ViewBoundedElements);
  TEST_METHOD(ViewBoundedStoreBytes);
  TEST_METHOD(MatVecHostOracle);
  TEST_METHOD(FP8HostOracle);
};

void LinAlgCPUOracleTests::TypedMatrixBufferRoundTrip() {
  using namespace cpu_oracle;

  auto VerifyScalarEncoding = [](const std::optional<TypedMatrix> &Matrix,
                                 const std::vector<BYTE> &ExpectedBytes) {
    if (!Matrix)
      return false;
    MatrixBufferLayout Layout = {
        MatrixLayout::RowMajor,
        /*OffsetBytes=*/0,
        /*StrideBytes=*/ExpectedBytes.size(),
    };
    std::vector<BYTE> ActualBytes(ExpectedBytes.size(), 0);
    MatrixResultOracle Oracle =
        exactResult(*Matrix, L"Host scalar encoding and decoding");
    return writeMatrixBuffer(*Matrix, Layout, ActualBytes) &&
           ActualBytes == ExpectedBytes &&
           verifyMatrixBuffer(ActualBytes.data(), ActualBytes.size(), Layout,
                              Oracle, /*Verbose=*/false);
  };

  VERIFY_IS_TRUE(VerifyScalarEncoding(
      makeTypedMatrix<HLSLHalf_t>(1, 1, {HLSLHalf_t(1.5f)}), {0x00, 0x3e}));
  VERIFY_IS_TRUE(VerifyScalarEncoding(makeTypedMatrix<float>(1, 1, {-2.5f}),
                                      {0x00, 0x00, 0x20, 0xc0}));
  VERIFY_IS_TRUE(VerifyScalarEncoding(makeTypedMatrix<int32_t>(1, 1, {-7}),
                                      {0xf9, 0xff, 0xff, 0xff}));
  VERIFY_IS_TRUE(
      VerifyScalarEncoding(makeTypedMatrix<uint32_t>(1, 1, {0x89abcdefu}),
                           {0xef, 0xcd, 0xab, 0x89}));

  const uint32_t AdjacentFloatBits = 0x3f800001;
  float AdjacentFloat;
  std::memcpy(&AdjacentFloat, &AdjacentFloatBits, sizeof(AdjacentFloat));
  VERIFY_IS_TRUE(
      ComponentTraits<float>::format(AdjacentFloat).find(L"3f800001") !=
      std::wstring::npos);

  std::optional<TypedMatrix> Matrix =
      makeTypedMatrix<uint32_t>(2, 3, {1, 2, 3, 4, 5, 6});
  VERIFY_IS_TRUE(Matrix.has_value());

  MatrixBufferLayout RowMajor = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/4,
      /*StrideBytes=*/16,
  };
  std::optional<size_t> RowBytes = getMatrixBufferSize(*Matrix, RowMajor);
  VERIFY_IS_TRUE(RowBytes.has_value());
  std::vector<BYTE> RowBuffer(*RowBytes, 0xcd);
  VERIFY_IS_TRUE(writeMatrixBuffer(*Matrix, RowMajor, RowBuffer));
  const std::vector<BYTE> ExpectedRowBuffer = {
      0xcd, 0xcd, 0xcd, 0xcd, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
      0x00, 0x03, 0x00, 0x00, 0x00, 0xcd, 0xcd, 0xcd, 0xcd, 0x04, 0x00,
      0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
  };
  VERIFY_IS_TRUE(RowBuffer == ExpectedRowBuffer);
  MatrixResultOracle Exact =
      exactResult(*Matrix, L"Host exact row-major matrix encoding");
  VERIFY_IS_TRUE(verifyMatrixBuffer(RowBuffer.data(), RowBuffer.size(),
                                    RowMajor, Exact, /*Verbose=*/false));

  MatrixBufferLayout ColumnMajor = {
      MatrixLayout::ColumnMajor,
      /*OffsetBytes=*/4,
      /*StrideBytes=*/12,
  };
  std::optional<size_t> ColumnBytes = getMatrixBufferSize(*Matrix, ColumnMajor);
  VERIFY_IS_TRUE(ColumnBytes.has_value());
  std::vector<BYTE> ColumnBuffer(*ColumnBytes, 0xcd);
  VERIFY_IS_TRUE(writeMatrixBuffer(*Matrix, ColumnMajor, ColumnBuffer));
  const std::vector<BYTE> ExpectedColumnBuffer = {
      0xcd, 0xcd, 0xcd, 0xcd, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
      0xcd, 0xcd, 0xcd, 0xcd, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
      0xcd, 0xcd, 0xcd, 0xcd, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
  };
  VERIFY_IS_TRUE(ColumnBuffer == ExpectedColumnBuffer);
  VERIFY_IS_TRUE(verifyMatrixBuffer(ColumnBuffer.data(), ColumnBuffer.size(),
                                    ColumnMajor, Exact, /*Verbose=*/false));

  std::optional<TypedMatrix> Transposed = transposeMatrix(*Matrix);
  std::optional<TypedMatrix> ExpectedTranspose =
      makeTypedMatrix<uint32_t>(3, 2, {1, 4, 2, 5, 3, 6});
  VERIFY_IS_TRUE(Transposed.has_value());
  VERIFY_IS_TRUE(ExpectedTranspose.has_value());
  size_t FirstMismatch;
  VERIFY_IS_TRUE(
      exactMatrixMatch(*Transposed, *ExpectedTranspose, FirstMismatch));

  std::optional<TypedMatrix> MixedActual =
      makeTypedMatrix<uint32_t>(1, 2, {1, 4});
  std::optional<TypedMatrix> CandidateA =
      makeTypedMatrix<uint32_t>(1, 2, {1, 2});
  std::optional<TypedMatrix> CandidateB =
      makeTypedMatrix<uint32_t>(1, 2, {3, 4});
  VERIFY_IS_TRUE(MixedActual.has_value());
  VERIFY_IS_TRUE(CandidateA.has_value());
  VERIFY_IS_TRUE(CandidateB.has_value());
  MatrixResultOracle Permitted =
      permittedResults({*CandidateA, *CandidateB},
                       L"Host whole-result permitted candidate semantics");
  VERIFY_IS_FALSE(matchesAnyCompleteCandidate(*MixedActual, Permitted));
  Permitted.Candidates.push_back(*MixedActual);
  VERIFY_IS_TRUE(matchesAnyCompleteCandidate(*MixedActual, Permitted));

  MatrixResultOracle Excluded =
      excludedResult(L"Host excluded-oracle classification");
  VERIFY_IS_FALSE(matchesAnyCompleteCandidate(*Matrix, Excluded));

  MatrixParams Params = {};
  Params.M = 2;
  Params.N = 3;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 4;
  Params.CompType = ComponentType::I32;
  VERIFY_IS_TRUE(buildCompilerArgs(Params).find(" -DELEM_TYPE=int") !=
                 std::string::npos);
  Params.CompType = ComponentType::U32;
  VERIFY_IS_TRUE(buildCompilerArgs(Params).find(" -DELEM_TYPE=uint") !=
                 std::string::npos);
}

// The padding check is verified here rather than only through the execution
// tests because a GPU round trip cannot easily produce a store that places
// every element correctly and still damages the bytes around them, which is
// the single case this check exists to catch.
void LinAlgCPUOracleTests::UntouchedByteVerification() {
  using namespace cpu_oracle;

  // A 2x3 uint32 matrix at a 4 byte offset with a 16 byte stride occupies
  // bytes 4..15 and 20..31, leaving a 4 byte prologue at 0..3 and 4 bytes of
  // padding at 16..19.
  std::optional<TypedMatrix> Matrix =
      makeTypedMatrix<uint32_t>(2, 3, {1, 2, 3, 4, 5, 6});
  VERIFY_IS_TRUE(Matrix.has_value());

  const MatrixBufferLayout Layout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/4,
      /*StrideBytes=*/16,
  };
  std::optional<size_t> Size = getMatrixBufferSize(*Matrix, Layout);
  VERIFY_IS_TRUE(Size.has_value());
  VERIFY_ARE_EQUAL(size_t(32), *Size);

  std::vector<BYTE> Buffer(*Size);
  fillPoison(Buffer.data(), Buffer.size());
  VERIFY_IS_TRUE(writeMatrixBuffer(*Matrix, Layout, Buffer));

  auto CountTouched = [&Layout](const std::vector<BYTE> &Bytes) {
    return countTouchedBytesOutsideElements(ComponentType::U32, 2, 3, Layout,
                                            Bytes.data(), Bytes.size());
  };

  // A correctly encoded buffer leaves every non-element byte poisoned.
  std::optional<size_t> Clean = CountTouched(Buffer);
  VERIFY_IS_TRUE(Clean.has_value());
  VERIFY_ARE_EQUAL(size_t(0), *Clean);
  VERIFY_IS_TRUE(verifyUntouchedBytes(ComponentType::U32, 2, 3, Layout,
                                      Buffer.data(), Buffer.size(),
                                      /*Verbose=*/false));

  // Damaging an element is the element comparison's job, not this check's, so
  // the count must stay at zero.
  std::vector<BYTE> ElementTouched = Buffer;
  ElementTouched[4] ^= 0xff;
  std::optional<size_t> AfterElement = CountTouched(ElementTouched);
  VERIFY_IS_TRUE(AfterElement.has_value());
  VERIFY_ARE_EQUAL(size_t(0), *AfterElement);

  // Damaging the prologue or the inter-row padding is what this check exists
  // to catch, so each one must be counted.
  for (size_t Offset : {size_t(0), size_t(16)}) {
    std::vector<BYTE> PaddingTouched = Buffer;
    PaddingTouched[Offset] ^= 0xff;
    std::optional<size_t> AfterPadding = CountTouched(PaddingTouched);
    VERIFY_IS_TRUE(AfterPadding.has_value());
    VERIFY_ARE_EQUAL(size_t(1), *AfterPadding);
  }

  // Every non-element byte damaged at once is still counted exactly.
  std::vector<BYTE> AllTouched(*Size);
  fillPoison(AllTouched.data(), AllTouched.size());
  for (BYTE &Byte : AllTouched)
    Byte = static_cast<BYTE>(~Byte);
  VERIFY_IS_TRUE(writeMatrixBuffer(*Matrix, Layout, AllTouched));
  std::optional<size_t> AfterAll = CountTouched(AllTouched);
  VERIFY_IS_TRUE(AfterAll.has_value());
  VERIFY_ARE_EQUAL(size_t(8), *AfterAll);

  // A buffer filled with one repeated value is fully detected, which is the
  // reason the pattern varies with the offset. A constant poison would score
  // zero here whenever the store happened to pick that same value, and 0xcd in
  // particular is what the MSVC debug allocator leaves in memory nobody wrote.
  std::vector<BYTE> ConstantFill(*Size, BYTE(0xcd));
  std::optional<size_t> AfterConstant = CountTouched(ConstantFill);
  VERIFY_IS_TRUE(AfterConstant.has_value());
  VERIFY_ARE_EQUAL(size_t(8), *AfterConstant);

  // The case a constant poison cannot survive: a store that writes a value the
  // poison pattern itself uses. Because the pattern varies, that value matches
  // at exactly one offset, so seven of the eight non-element bytes are still
  // caught. A constant poison would match everywhere and report nothing.
  std::vector<BYTE> PoisonValuedFill(*Size, poisonByteAt(0));
  std::optional<size_t> AfterPoisonValued = CountTouched(PoisonValuedFill);
  VERIFY_IS_TRUE(AfterPoisonValued.has_value());
  VERIFY_ARE_EQUAL(size_t(7), *AfterPoisonValued);

  // No two adjacent bytes share a poison value, so a constant written over any
  // two neighbours cannot hide in both.
  for (size_t Offset = 1; Offset < *Size; ++Offset)
    VERIFY_ARE_NOT_EQUAL(poisonByteAt(Offset - 1), poisonByteAt(Offset));

  // The diagnostic list is capped but the count is not, so the two have to be
  // checked against a buffer with more offenders than the cap. A 2x3 uint32
  // matrix at a 16 byte offset with a 16 byte stride occupies bytes 16..27 and
  // 32..43, leaving twenty bytes outside the elements.
  const MatrixBufferLayout PaddedLayout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/16,
      /*StrideBytes=*/16,
  };
  std::optional<size_t> PaddedSize =
      getMatrixBufferSize(ComponentType::U32, 2, 3, PaddedLayout);
  VERIFY_IS_TRUE(PaddedSize.has_value());
  VERIFY_ARE_EQUAL(size_t(44), *PaddedSize);

  std::vector<BYTE> AllPaddingTouched(*PaddedSize);
  fillPoison(AllPaddingTouched.data(), AllPaddingTouched.size());
  for (BYTE &Byte : AllPaddingTouched)
    Byte = static_cast<BYTE>(~Byte);
  std::vector<size_t> ReportedOffsets;
  std::optional<size_t> AfterPadded = countTouchedBytesOutsideElements(
      ComponentType::U32, 2, 3, PaddedLayout, AllPaddingTouched.data(),
      AllPaddingTouched.size(), &ReportedOffsets);
  VERIFY_IS_TRUE(AfterPadded.has_value());
  VERIFY_ARE_EQUAL(size_t(20), *AfterPadded);
  VERIFY_ARE_EQUAL(size_t(8), ReportedOffsets.size());
  for (size_t I = 0; I < ReportedOffsets.size(); ++I)
    VERIFY_ARE_EQUAL(I, ReportedOffsets[I]);

  // Below the cap every offender is reported, and by its offset in the buffer
  // rather than its position among the offenders.
  std::vector<BYTE> TwoPaddingBytes(*PaddedSize);
  fillPoison(TwoPaddingBytes.data(), TwoPaddingBytes.size());
  TwoPaddingBytes[28] ^= 0xff;
  TwoPaddingBytes[29] ^= 0xff;
  std::vector<size_t> TwoOffsets;
  std::optional<size_t> AfterTwo = countTouchedBytesOutsideElements(
      ComponentType::U32, 2, 3, PaddedLayout, TwoPaddingBytes.data(),
      TwoPaddingBytes.size(), &TwoOffsets);
  VERIFY_IS_TRUE(AfterTwo.has_value());
  VERIFY_ARE_EQUAL(size_t(2), *AfterTwo);
  VERIFY_ARE_EQUAL(size_t(2), TwoOffsets.size());
  VERIFY_ARE_EQUAL(size_t(28), TwoOffsets[0]);
  VERIFY_ARE_EQUAL(size_t(29), TwoOffsets[1]);

  // A buffer too small for the layout cannot be checked at all.
  std::vector<BYTE> TooSmall(*Size - 1);
  fillPoison(TooSmall.data(), TooSmall.size());
  VERIFY_IS_FALSE(CountTouched(TooSmall).has_value());
}

// The per-element arm of the bounds-checking rule is derived on the host, so
// the boundary it draws is checked here rather than only through a GPU round
// trip, where a wrong boundary and a wrong implementation would be
// indistinguishable.
void LinAlgCPUOracleTests::ViewBoundedElements() {
  using namespace cpu_oracle;

  // A 2x3 uint32 matrix at a 4 byte offset with a 16 byte stride puts its
  // elements at bytes 4, 8, 12, 20, 24 and 28, each 4 bytes wide, so they end
  // at 8, 12, 16, 24, 28 and 32.
  std::optional<TypedMatrix> Matrix =
      makeTypedMatrix<uint32_t>(2, 3, {1, 2, 3, 4, 5, 6});
  VERIFY_IS_TRUE(Matrix.has_value());

  const MatrixBufferLayout Layout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/4,
      /*StrideBytes=*/16,
  };

  auto BoundedEquals = [&](size_t ViewBytes,
                           const std::vector<uint32_t> &Expected) {
    std::optional<TypedMatrix> Bounded =
        zeroElementsOutsideView(*Matrix, Layout, ViewBytes);
    std::optional<TypedMatrix> Want = makeTypedMatrix<uint32_t>(2, 3, Expected);
    if (!Bounded || !Want)
      return false;
    size_t FirstMismatch;
    return exactMatrixMatch(*Bounded, *Want, FirstMismatch);
  };

  // A view covering the whole buffer changes nothing, and an empty view
  // zeroes everything.
  VERIFY_IS_TRUE(BoundedEquals(32, {1, 2, 3, 4, 5, 6}));
  VERIFY_IS_TRUE(BoundedEquals(0, {0, 0, 0, 0, 0, 0}));

  // A view ending at 24 admits the element that ends exactly there and
  // excludes the rest, which is the inclusive end the rule requires.
  VERIFY_IS_TRUE(BoundedEquals(24, {1, 2, 3, 4, 0, 0}));

  // One byte short of that boundary drops the straddling element whole. An
  // element is either wholly inside the view or it is not there at all.
  VERIFY_IS_TRUE(BoundedEquals(23, {1, 2, 3, 0, 0, 0}));

  // The gap between the rows is not addressable, so a view that reaches into
  // the padding admits no further elements.
  VERIFY_IS_TRUE(BoundedEquals(19, {1, 2, 3, 0, 0, 0}));

  // A view sized past the buffer cannot admit more than the buffer holds.
  VERIFY_IS_TRUE(BoundedEquals(1024, {1, 2, 3, 4, 5, 6}));
}

// The store side draws the same boundary but leaves the excluded elements
// holding poison rather than zero, so it is checked here as bytes.
void LinAlgCPUOracleTests::ViewBoundedStoreBytes() {
  using namespace cpu_oracle;

  // The layout ViewBoundedElements uses: elements at bytes 4, 8, 12, 20, 24
  // and 28, each 4 bytes wide, in a 32 byte buffer.
  std::optional<TypedMatrix> Matrix =
      makeTypedMatrix<uint32_t>(2, 3, {1, 2, 3, 4, 5, 6});
  VERIFY_IS_TRUE(Matrix.has_value());

  const MatrixBufferLayout Layout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/4,
      /*StrideBytes=*/16,
  };
  static constexpr size_t ElementOffsets[] = {4, 8, 12, 20, 24, 28};

  // Bit I is set when element I holds its value. Every element must hold
  // either that or the poison a rejected store leaves behind, so an oracle
  // that zeroed the rejected elements instead fails here rather than
  // reporting them as merely unwritten.
  auto WrittenMask = [&](size_t ViewBytes) {
    std::optional<std::vector<BYTE>> Buffer =
        storeBufferBoundedByView(*Matrix, Layout, ViewBytes);
    VERIFY_IS_TRUE(Buffer.has_value());
    VERIFY_ARE_EQUAL(Buffer->size(), static_cast<size_t>(32));
    unsigned Mask = 0;
    for (unsigned I = 0; I < 6; ++I) {
      const size_t Offset = ElementOffsets[I];
      const uint32_t Value = I + 1;
      BYTE Written[sizeof(Value)];
      memcpy(Written, &Value, sizeof(Value));
      bool HoldsValue = true;
      bool HoldsPoison = true;
      for (size_t B = 0; B < sizeof(Value); ++B) {
        if ((*Buffer)[Offset + B] != Written[B])
          HoldsValue = false;
        if ((*Buffer)[Offset + B] != poisonByteAt(Offset + B))
          HoldsPoison = false;
      }
      VERIFY_IS_TRUE(HoldsValue || HoldsPoison,
                     "A view bounded store element held neither its value nor "
                     "the poison it was seeded with");
      if (HoldsValue)
        Mask |= 1u << I;
    }
    return Mask;
  };

  // A view covering the whole buffer writes everything, and an empty view
  // drops the whole store, so it writes nothing.
  VERIFY_ARE_EQUAL(WrittenMask(32), 0x3fu);
  VERIFY_ARE_EQUAL(WrittenMask(0), 0x00u);

  // A view ending at 24 admits the element that ends exactly there.
  VERIFY_ARE_EQUAL(WrittenMask(24), 0x0fu);

  // One byte short of that boundary drops the straddling element whole,
  // including the part of it the view does reach.
  VERIFY_ARE_EQUAL(WrittenMask(23), 0x07u);

  // The prologue before the offset and the padding between rows belong to no
  // element, so a full store must leave both holding poison.
  std::optional<std::vector<BYTE>> Full =
      storeBufferBoundedByView(*Matrix, Layout, 32);
  VERIFY_IS_TRUE(Full.has_value());
  std::optional<size_t> Corrupted = countTouchedBytesOutsideElements(
      ComponentType::U32, 2, 3, Layout, Full->data(), Full->size());
  VERIFY_IS_TRUE(Corrupted.has_value());
  VERIFY_ARE_EQUAL(*Corrupted, static_cast<size_t>(0));
}

void LinAlgCPUOracleTests::MatVecHostOracle() {
  using namespace matvec_interpretation;

  const std::vector<BYTE> PackedBytes = {0xff, 0x02, 0xfd, 0x04,
                                         0x05, 0x00, 0x00, 0x00};
  const std::optional<std::vector<BYTE>> PackedSInt8 =
      encodePackedVector(ComponentType::I8, {-1, 2, -3, 4, 5});
  VERIFY_IS_TRUE(PackedSInt8.has_value(), "SInt8 packing failed");
  VERIFY_IS_TRUE(PackedSInt8 == PackedBytes,
                 "SInt8 packing produced unexpected bytes");

  const std::optional<std::vector<BYTE>> PackedUInt8 =
      encodePackedVector(ComponentType::U8, {255, 2, 253, 4, 5});
  VERIFY_IS_TRUE(PackedUInt8.has_value(), "UInt8 packing failed");
  VERIFY_IS_TRUE(PackedUInt8 == PackedBytes,
                 "UInt8 packing produced unexpected bytes");

  CaseData DotCase = {};
  DotCase.M = 2;
  DotCase.N = 3;
  DotCase.MatrixValues = {1, 2, 3, -1, 4, 0};
  DotCase.InterpretedVectorValues = {4, -2, 5};
  DotCase.BiasInputType = ComponentType::I32;
  DotCase.BiasValues = {7, -3};
  const std::vector<int64_t> Dot = calculateExpected(DotCase);
  VERIFY_IS_TRUE(Dot == std::vector<int64_t>({22, -15}),
                 "Biased dot product oracle returned the wrong values");
}

class LinAlgCapabilityTests {
public:
  BEGIN_TEST_CLASS(LinAlgCapabilityTests)
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_METHOD(CapabilityPolicyAndPredicates);
};

void LinAlgCapabilityTests::CapabilityPolicyAndPredicates() {
  using namespace linalg_test;

  VERIFY_IS_TRUE(
      classifyApplicability(S_OK, true, CapabilityRequirement::Mandatory) ==
      Applicability::Execute);
  VERIFY_IS_TRUE(classifyApplicability(
                     S_OK, false, CapabilityRequirement::CapabilityGated) ==
                 Applicability::NotApplicable);
  VERIFY_IS_TRUE(
      classifyApplicability(S_OK, false, CapabilityRequirement::Mandatory) ==
      Applicability::Fail);
  VERIFY_IS_TRUE(
      classifyApplicability(E_UNEXPECTED, true,
                            CapabilityRequirement::CapabilityGated) ==
      Applicability::Fail);

  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION,
      MatrixScope::Wave));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION,
      MatrixScope::ThreadGroup));
  VERIFY_IS_FALSE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION,
      MatrixScope::Thread));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY,
      MatrixScope::Wave));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::
          D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREADGROUP_MATRIX_MULTIPLY,
      MatrixScope::ThreadGroup));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::
          D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY,
      MatrixScope::Thread));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_OUTER_PRODUCT,
      MatrixScope::Thread));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE,
      MatrixScope::Thread));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE,
      MatrixScope::Wave));
  VERIFY_IS_TRUE(isLegalScope(
      linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE,
      MatrixScope::ThreadGroup));

  MatrixConstructionSupport Construction = {TRUE};
  VERIFY_IS_TRUE(Construction.valid());
  VERIFY_IS_TRUE(Construction.supported());
  MatrixConstructionSupport UnsupportedConstruction = {FALSE};
  VERIFY_IS_TRUE(UnsupportedConstruction.valid());
  VERIFY_IS_FALSE(UnsupportedConstruction.supported());
  // The runtime contract is a canonical BOOL; anything else is a driver bug.
  MatrixConstructionSupport InvalidConstruction = {2};
  VERIFY_IS_FALSE(InvalidConstruction.valid());
  VERIFY_IS_FALSE(InvalidConstruction.supported());

  WaveMatrixMultiplySupport Wave = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED};
  VERIFY_IS_TRUE(Wave.valid());
  VERIFY_IS_TRUE(Wave.supported());
  WaveMatrixMultiplySupport UnsupportedWave = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE};
  VERIFY_IS_TRUE(UnsupportedWave.valid());
  VERIFY_IS_FALSE(UnsupportedWave.supported());
  WaveMatrixMultiplySupport InvalidWave = {
      static_cast<
          linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED) |
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_INPUTS)),
  };
  VERIFY_IS_FALSE(InvalidWave.valid());

  ThreadGroupMatrixMultiplySupport ThreadGroup = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED,
      32,
      128,
      64,
  };
  VERIFY_IS_TRUE(ThreadGroup.valid());
  VERIFY_IS_TRUE(ThreadGroup.supportsThreadGroupSize(64));
  VERIFY_IS_FALSE(ThreadGroup.supportsThreadGroupSize(48));
  ThreadGroup.PreferredThreadGroupSize = 48;
  VERIFY_IS_FALSE(ThreadGroup.valid());
  ThreadGroup = {
      static_cast<
          linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED) |
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_TRANSPOSE)),
      32,
      128,
      64,
  };
  VERIFY_IS_FALSE(ThreadGroup.valid());

  ThreadVectorMatrixMultiplySupport ThreadVector = {
      static_cast<
          linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED) |
          static_cast<UINT>(
              linalg_abi::
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_TRANSPOSE)),
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
  };
  VERIFY_IS_TRUE(ThreadVector.valid());
  VERIFY_IS_TRUE(ThreadVector.supported());
  ThreadVector.SupportFlags = static_cast<
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(
      static_cast<UINT>(
          linalg_abi::
              D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED) |
      static_cast<UINT>(
          linalg_abi::
              D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_INPUTS));
  VERIFY_IS_FALSE(ThreadVector.valid());
  ThreadVector.MatrixInputType =
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN;
  VERIFY_IS_TRUE(ThreadVector.valid());
  ThreadVector.SupportFlags = linalg_abi::
      D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_INPUTS;
  VERIFY_IS_FALSE(ThreadVector.valid());

  ThreadOuterProductSupport OuterProduct = {true};
  VERIFY_IS_TRUE(OuterProduct.supported());
  AtomicAccumulateStoreSupport Atomic = {true, false};
  VERIFY_IS_TRUE(Atomic.supports(AtomicDestination::RWByteAddressBuffer));
  VERIFY_IS_FALSE(Atomic.supports(AtomicDestination::GroupShared));

  VERIFY_ARE_EQUAL(
      0u, static_cast<UINT>(linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE));
  MatrixConstructionQuery ConstructionQuery = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32, 32, {8, 8, 8}};
  WaveMatrixMultiplyInputs WaveInputs = {
      32,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
  };
  WaveMatrixMultiplyQuery WaveQuery = {WaveInputs, {16, 16, 16}};
  ThreadGroupMatrixMultiplyQuery ThreadGroupQuery = {
      WaveInputs,
      {16, 16, 16},
  };
  ThreadVectorMatrixMultiplyQuery ThreadVectorQuery = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
  };
  ThreadOuterProductQuery OuterProductQuery = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
  };
  AtomicAccumulateStoreQuery AtomicQuery = {
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16};
  VERIFY_ARE_EQUAL(32u, ConstructionQuery.WaveSize);
  VERIFY_ARE_EQUAL(8u, ConstructionQuery.Shape.K);
  VERIFY_ARE_EQUAL(32u, WaveQuery.Inputs.WaveSize);
  VERIFY_ARE_EQUAL(16u, WaveQuery.Shape.M);
  VERIFY_ARE_EQUAL(32u, ThreadGroupQuery.WaveInputs.WaveSize);
  VERIFY_ARE_EQUAL(16u, ThreadGroupQuery.Shape.M);
  VERIFY_IS_TRUE(ThreadVectorQuery.BiasInputType ==
                 linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE);
  VERIFY_IS_TRUE(OuterProductQuery.InputComponentType ==
                 linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16);
  VERIFY_IS_TRUE(AtomicQuery.ComponentType ==
                 linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16);
}

class DxilConf_SM610_LinAlg {
public:
  BEGIN_TEST_CLASS(DxilConf_SM610_LinAlg)
  TEST_CLASS_PROPERTY("Kits.TestName",
                      "D3D12 - Shader Model 6.10 - LinAlg Matrix Operations")
  TEST_CLASS_PROPERTY("Kits.TestId", "a1b2c3d4-e5f6-7890-abcd-ef1234567890")
  TEST_CLASS_PROPERTY(
      "Kits.Description",
      "Validates SM 6.10 linear algebra matrix operations execute correctly")
  TEST_CLASS_PROPERTY(
      "Kits.Specification",
      "Device.Graphics.D3D12.DXILCore.ShaderModel610.CoreRequirement")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(setupClass);
  TEST_METHOD_SETUP(setupMethod);

  // Load/Store/Accumulate Descriptor
  TEST_METHOD(LoadStoreDescriptor_Wave_16x16_F16);
  TEST_METHOD(LoadStoreDescriptor_Wave_4x8_F16_RowMajorOffsetPadded);
  TEST_METHOD(LoadStoreDescriptor_Wave_4x8_F32_RowMajorToColumnMajor);
  TEST_METHOD(LoadStoreDescriptor_Wave_4x8_F16_RowMajorToColumnMajor);
  TEST_METHOD(LoadDescriptorOOB_Wave_16x16_F16_PartialView);
  TEST_METHOD(LoadDescriptorOOB_Wave_4x8_F16_OffsetPaddedPartialView);
  TEST_METHOD(StoreDescriptorOOB_Wave_16x16_F16_PartialView);
  TEST_METHOD(StoreDescriptorOOB_Wave_4x8_F16_OffsetPaddedPartialView);
  TEST_METHOD(SplatStore_Wave_16x16_F16);
  TEST_METHOD(AccumulateDescriptor_Wave_16x16_F16);

  // Load/Store/Accumulate Memory
  TEST_METHOD(LoadMemory_Wave_16x16_F16);
  TEST_METHOD(StoreMemory_Wave_16x16_F16);
  TEST_METHOD(AccumulateMemory_Wave_16x16_F16);
  TEST_METHOD(LoadStoreMemory_Wave_4x8_F16_RowMajorOffsetPadded);
  TEST_METHOD(LoadStoreMemory_Wave_4x8_F32_ColumnMajorOffsetPadded);
  TEST_METHOD(LoadStoreMemory_ThreadGroup_4x8_F16);

  // Element access
  TEST_METHOD(ElementAccess_Wave_16x16_F16);
  TEST_METHOD(ElementAccess_Wave_4x8_F32);
  TEST_METHOD(ElementSet_Wave_16x16_F16);
  TEST_METHOD(ElementGetOOB_Wave_4x8_F32);
  TEST_METHOD(ElementSetOOB_Wave_4x8_F32);
  TEST_METHOD(ElementGetOOB_Wave_16x16_F16);
  TEST_METHOD(ElementSetOOB_Wave_16x16_F16);

  // Cast/Convert
  TEST_METHOD(CopyConvert_Wave_16x16_F16);
  TEST_METHOD(CopyConvert_Wave_16x16_F16_Transpose);
  TEST_METHOD(CopyConvert_Wave_4x8_F32_Transpose);

  // Matrix Matrix Arithmetic
  TEST_METHOD(MatMatMul_Wave_16x16x16_F16);
  TEST_METHOD(MatMatMul_Wave_8x32x16_F16_NonUniform);
  TEST_METHOD(MatMatMul_Wave_8x32x16_F16_ToF32);
  TEST_METHOD(MatMatMul_Wave_16x16x16_I32);
  TEST_METHOD(MatMatMulAccum_Wave_16x16x16_F16);
  TEST_METHOD(MatMatMulAccum_Wave_8x32x16_F16_ToF32_NonUniform);
  TEST_METHOD(MatMatMul_ThreadGroup_8x16x8_F16_NonUniform);
  TEST_METHOD(MatMatMulAccum_ThreadGroup_8x16x8_F16_ToF32_NonUniform);
  TEST_METHOD(MatMatMul_ThreadGroup_8x8x8_I32);
  TEST_METHOD(MatAccum_Wave_16x16_F16);
  TEST_METHOD(MatAccum_Wave_8x32_F16_BUse_NonUniform);

  // Matrix Vector Arithmetic
  TEST_METHOD(MatVecMul_Thread_16x16_F16);
  TEST_METHOD(MatVecMul_Thread_4x8_F32);
  TEST_METHOD(MatVecMul_Thread_4x8_F16_NonUniform);
  TEST_METHOD(MatVecMul_Thread_4x8_F16_ColumnMajor);
  TEST_METHOD(MatVecMul_Thread_4x8_I8_Interpreted);
  TEST_METHOD(MatVecMul_Thread_4x8_U8_Interpreted);
  TEST_METHOD(MatVecMul_Thread_4x8_U32_UnsignedOutput);
  TEST_METHOD(MatVecMulAdd_Thread_16x16_F16);
  TEST_METHOD(MatVecMulAdd_Thread_4x8_F32);
  TEST_METHOD(MatVecMulAdd_Thread_4x8_F16_IndependentBias);
  TEST_METHOD(OuterProduct_Thread_16x16_F16);

  // Query Accumulator Layout
  TEST_METHOD(QueryAccumLayout);

  // Convert
  TEST_METHOD(Convert);

  // CopyConvert / Convert coverage
  TEST_METHOD(CopyConvert_Wave_4x8_F16_ToF32);
  TEST_METHOD(CopyConvert_Wave_4x8_F32_ToF16_Transpose);
  TEST_METHOD(Convert_I16_ToI32_Exact);
  TEST_METHOD(Convert_F32_ToI16_RTNE_Saturate);
  TEST_METHOD(Convert_F16_ToE4M3FN_AndBack);
  TEST_METHOD(Convert_F16_ToE5M2_AndBack);

  // Vector Accumulate
  TEST_METHOD(VectorAccumulateDescriptor_Thread_F16);
  TEST_METHOD(VectorAccumulateDescriptor_Thread_F16_Length8_NonZero);
  TEST_METHOD(VectorAccumulateDescriptor_Thread_F32_Length8_NonZero);

private:
  CComPtr<ID3D12Device> D3DDevice;
  dxc::SpecificDllLoader DxcSupport;
  bool VerboseLogging = false;
  bool Initialized = false;
  std::optional<D3D12SDKSelector> D3D12SDK;

  WEX::TestExecution::SetVerifyOutput VerifyOutput{
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures};
};

bool DxilConf_SM610_LinAlg::setupClass() {
  if (!Initialized) {
    Initialized = true;
    VERIFY_SUCCEEDED(
        DxcSupport.InitializeForDll(dxc::kDxCompilerLib, "DxcCreateInstance"));
    D3D12SDK = D3D12SDKSelector();
    WEX::TestExecution::RuntimeParameters::TryGetValue(L"VerboseLogging",
                                                       VerboseLogging);

    if (!D3D12SDK->createDevice(&D3DDevice, D3D_SHADER_MODEL_6_10, false)) {
#ifdef _HLK_CONF
      hlsl_test::LogErrorFmt(
          L"Device creation failed. Expected a driver supporting SM6.10");
#else
      hlsl_test::LogWarningFmt(
          L"Device creation failed. Expected a driver supporting SM6.10");
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
#endif
      return false;
    }
  }

  return true;
}

bool DxilConf_SM610_LinAlg::setupMethod() {
  // If the device is healthy, exit otherwise it's possible a previous test
  // case caused a device removal. So we need to try and create a new device.
  if (D3DDevice && D3DDevice->GetDeviceRemovedReason() == S_OK)
    return true;

  hlsl_test::LogCommentFmt(L"Device was lost!");
  D3DDevice.Release();

  hlsl_test::LogCommentFmt(L"Recreating device");

  return D3D12SDK->createDevice(&D3DDevice, D3D_SHADER_MODEL_6_10, false);
}

// The alignment the descriptor shader declares to both builtins. Proposal 0035
// requires the first element's address -- the resource base plus the offset --
// to meet it.
static constexpr size_t DescriptorDeclaredAlignment = 128;

static const char LoadStoreDescriptorShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, LOAD_OFFSET, LOAD_STRIDE, LOAD_LAYOUT, DECLARED_ALIGN);
    __builtin_LinAlg_MatrixStoreToDescriptor(
      Mat, Output, STORE_OFFSET, STORE_STRIDE, STORE_LAYOUT, DECLARED_ALIGN);
  }
)";

// The base is a runtime property that no compile-time check can see, so check
// it against the real GPU address.
static void verifyDescriptorBaseAlignment(st::ShaderOpTest *Test, LPCSTR Name,
                                          size_t OffsetBytes) {
  // GetResource hands back a borrowed pointer without an AddRef.
  ID3D12Resource *Resource = nullptr;
  Test->GetResource(Name, &Resource);
  VERIFY_IS_NOT_NULL(Resource);

  const UINT64 ElementAddress = Resource->GetGPUVirtualAddress() + OffsetBytes;
  VERIFY_IS_TRUE(ElementAddress % DescriptorDeclaredAlignment == 0,
                 "Descriptor buffer's first element does not meet the "
                 "alignment the shader declares");
}

static void
runLoadStoreDescriptor(ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
                       const MatrixParams &Params,
                       const cpu_oracle::MatrixBufferLayout &LoadLayout,
                       const cpu_oracle::MatrixBufferLayout &StoreLayout,
                       bool Verbose, UINT ForcedWaveSize = 0) {
  std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeSequentialMatrix(Params.CompType, Params.M, Params.N);
  VERIFY_IS_TRUE(Input.has_value(),
                 "Unable to construct typed LoadStoreDescriptor input");

  std::optional<size_t> InputSize =
      cpu_oracle::getMatrixBufferSize(*Input, LoadLayout);
  std::optional<size_t> OutputSize =
      cpu_oracle::getMatrixBufferSize(*Input, StoreLayout);
  VERIFY_IS_TRUE(InputSize.has_value() && OutputSize.has_value(),
                 "Unable to size the LoadStoreDescriptor buffers");

  std::stringstream ExtraDefs;
  ExtraDefs << " -DLOAD_OFFSET=" << LoadLayout.OffsetBytes;
  ExtraDefs << " -DLOAD_STRIDE=" << LoadLayout.StrideBytes;
  ExtraDefs << " -DLOAD_LAYOUT=" << static_cast<int>(LoadLayout.Layout);
  ExtraDefs << " -DSTORE_OFFSET=" << StoreLayout.OffsetBytes;
  ExtraDefs << " -DSTORE_STRIDE=" << StoreLayout.StrideBytes;
  ExtraDefs << " -DSTORE_LAYOUT=" << static_cast<int>(StoreLayout.Layout);
  ExtraDefs << " -DDECLARED_ALIGN=" << DescriptorDeclaredAlignment;

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, LoadStoreDescriptorShader, "cs_6_10", Args,
                Verbose);

  const cpu_oracle::TypedMatrix InputMatrix = *Input;
  cpu_oracle::MatrixResultOracle Oracle = cpu_oracle::exactResult(
      InputMatrix, L"HLSL proposal 0035 MatrixLoadFromDescriptor and "
                   L"MatrixStoreToDescriptor "
                   L"round trip at the requested offset, stride and layout");

  // Two UAV buffers, load from one, store to the other. The destination is
  // filled by name rather than zeroed so unowned bytes carry the poison.
  //
  // Bound through a descriptor table rather than as root views. A root view is
  // a bare GPU address, and proposal 0035 exempts root descriptors from bounds
  // checking precisely because they carry no dimensions. Binding through a
  // heap gives each buffer a view whose extent the implementation can see.
  auto Op = createComputeOp(LoadStoreDescriptorShader, "cs_6_10",
                            "DescriptorTable(UAV(u0), UAV(u1))", Args.c_str());
  addUAVBuffer(Op.get(), "Input", *InputSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", *OutputSize, true, "byname");
  addHeapRawUAV(Op.get(), "ResHeap", "Input", *InputSize);
  addHeapRawUAV(Op.get(), "ResHeap", "Output", *OutputSize);
  addRootTable(Op.get(), 0, "ResHeap");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [InputMatrix, LoadLayout](LPCSTR Name, std::vector<BYTE> &Data,
                                st::ShaderOp *) {
        cpu_oracle::fillPoison(Data.data(), Data.size());
        if (_stricmp(Name, "Input") != 0)
          return;
        VERIFY_IS_TRUE(
            cpu_oracle::writeMatrixBuffer(InputMatrix, LoadLayout, Data),
            "Unable to encode typed LoadStoreDescriptor input");
      },
      [LoadLayout, StoreLayout](ID3D12GraphicsCommandList *,
                                st::ShaderOpTest *Test) {
        verifyDescriptorBaseAlignment(Test, "Input", LoadLayout.OffsetBytes);
        verifyDescriptorBaseAlignment(Test, "Output", StoreLayout.OffsetBytes);
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(cpu_oracle::verifyMatrixBuffer(OutData.data(), OutData.size(),
                                                StoreLayout, Oracle, Verbose));
  VERIFY_IS_TRUE(cpu_oracle::verifyUntouchedBytes(
      Params.CompType, Params.M, Params.N, StoreLayout, OutData.data(),
      OutData.size(), Verbose));
}

// Proposal 0035 permits two bounds-checking behaviors. An implementation may
// zero the whole matrix when any element falls outside the view, or zero only
// the elements that do, and both are conformant. A single expected buffer
// would therefore be wrong by construction, so the oracle carries both
// outcomes and accepts a complete match against either one.
//
// What makes the test discriminating is that the source buffer is allocated
// and written in full and only its *view* is shortened, so the bytes past the
// view hold real matrix data rather than zeros. An implementation that does no
// bounds checking at all reads that data back and matches neither candidate.
static void runLoadDescriptorOutOfBounds(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
    const MatrixParams &Params, const cpu_oracle::MatrixBufferLayout &Layout,
    size_t InputViewBytes, bool Verbose, UINT ForcedWaveSize = 0) {
  std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeSequentialMatrix(Params.CompType, Params.M, Params.N);
  VERIFY_IS_TRUE(Input.has_value(),
                 "Unable to construct typed LoadDescriptorOOB input");

  std::optional<size_t> BufferSize =
      cpu_oracle::getMatrixBufferSize(*Input, Layout);
  VERIFY_IS_TRUE(BufferSize.has_value(),
                 "Unable to size the LoadDescriptorOOB buffers");
  VERIFY_IS_TRUE(InputViewBytes < *BufferSize,
                 "The source view must be shorter than its buffer");

  std::optional<cpu_oracle::TypedMatrix> PerElement =
      cpu_oracle::zeroElementsOutsideView(*Input, Layout, InputViewBytes);
  // The whole-matrix arm is the per-element arm with nothing in view.
  std::optional<cpu_oracle::TypedMatrix> WholeMatrix =
      cpu_oracle::zeroElementsOutsideView(*Input, Layout, 0);
  VERIFY_IS_TRUE(PerElement.has_value() && WholeMatrix.has_value(),
                 "Unable to derive the LoadDescriptorOOB candidates");

  // The test requires both in-bounds and out-of-bounds elements. If none are
  // in bounds, the two permitted results are identical. If all are in bounds,
  // an implementation that performs no bounds checking would still pass.
  size_t FirstMismatch;
  const bool HasInBoundsElement =
      !cpu_oracle::exactMatrixMatch(*PerElement, *WholeMatrix, FirstMismatch);
  VERIFY_IS_TRUE(HasInBoundsElement,
                 "The source view must include at least one complete element");

  const bool HasOutOfBoundsElement =
      !cpu_oracle::exactMatrixMatch(*PerElement, *Input, FirstMismatch);
  VERIFY_IS_TRUE(HasOutOfBoundsElement,
                 "The source view must exclude at least one element");

  std::stringstream ExtraDefs;
  ExtraDefs << " -DLOAD_OFFSET=" << Layout.OffsetBytes;
  ExtraDefs << " -DLOAD_STRIDE=" << Layout.StrideBytes;
  ExtraDefs << " -DLOAD_LAYOUT=" << static_cast<int>(Layout.Layout);
  ExtraDefs << " -DSTORE_OFFSET=" << Layout.OffsetBytes;
  ExtraDefs << " -DSTORE_STRIDE=" << Layout.StrideBytes;
  ExtraDefs << " -DSTORE_LAYOUT=" << static_cast<int>(Layout.Layout);
  ExtraDefs << " -DDECLARED_ALIGN=" << DescriptorDeclaredAlignment;

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, LoadStoreDescriptorShader, "cs_6_10", Args,
                Verbose);

  cpu_oracle::MatrixResultOracle Oracle = cpu_oracle::permittedResults(
      {*PerElement, *WholeMatrix},
      L"HLSL proposal 0035 bounds checking on MatrixLoadFromDescriptor: "
      L"either the whole matrix or only the out-of-view elements read as the "
      L"default element value");

  // Only the source view is short. The destination is viewed in full so that
  // the store cannot be bounds checked as well, which would leave the observed
  // result attributable to either operation.
  const cpu_oracle::TypedMatrix InputMatrix = *Input;
  auto Op = createComputeOp(LoadStoreDescriptorShader, "cs_6_10",
                            "DescriptorTable(UAV(u0), UAV(u1))", Args.c_str());
  addUAVBuffer(Op.get(), "Input", *BufferSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", *BufferSize, true, "byname");
  addHeapRawUAV(Op.get(), "ResHeap", "Input", InputViewBytes);
  addHeapRawUAV(Op.get(), "ResHeap", "Output", *BufferSize);
  addRootTable(Op.get(), 0, "ResHeap");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [InputMatrix, Layout](LPCSTR Name, std::vector<BYTE> &Data,
                            st::ShaderOp *) {
        cpu_oracle::fillPoison(Data.data(), Data.size());
        if (_stricmp(Name, "Input") != 0)
          return;
        // Written in full, including the part the view does not cover.
        VERIFY_IS_TRUE(cpu_oracle::writeMatrixBuffer(InputMatrix, Layout, Data),
                       "Unable to encode typed LoadDescriptorOOB input");
      },
      [Layout](ID3D12GraphicsCommandList *, st::ShaderOpTest *Test) {
        verifyDescriptorBaseAlignment(Test, "Input", Layout.OffsetBytes);
        verifyDescriptorBaseAlignment(Test, "Output", Layout.OffsetBytes);
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(cpu_oracle::verifyMatrixBuffer(OutData.data(), OutData.size(),
                                                Layout, Oracle, Verbose));
  VERIFY_IS_TRUE(cpu_oracle::verifyUntouchedBytes(
      Params.CompType, Params.M, Params.N, Layout, OutData.data(),
      OutData.size(), Verbose));
}

// Stores through a destination view shorter than its buffer. Both permitted
// outcomes leave the bytes past the view holding poison, so the comparison is
// byte level rather than matrix level. This cannot by itself fail an
// implementation that stores nothing, since dropping the whole store is one of
// those outcomes; the LoadStoreDescriptor cases require the store to happen.
static void runStoreDescriptorOutOfBounds(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
    const MatrixParams &Params, const cpu_oracle::MatrixBufferLayout &Layout,
    size_t OutputViewBytes, bool Verbose, UINT ForcedWaveSize = 0) {
  std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeSequentialMatrix(Params.CompType, Params.M, Params.N);
  VERIFY_IS_TRUE(Input.has_value(),
                 "Unable to construct typed StoreDescriptorOOB input");

  std::optional<size_t> BufferSize =
      cpu_oracle::getMatrixBufferSize(*Input, Layout);
  VERIFY_IS_TRUE(BufferSize.has_value(),
                 "Unable to size the StoreDescriptorOOB buffers");
  VERIFY_IS_TRUE(OutputViewBytes < *BufferSize,
                 "The destination view must be shorter than its buffer");

  std::optional<std::vector<BYTE>> PerElement =
      cpu_oracle::storeBufferBoundedByView(*Input, Layout, OutputViewBytes);
  std::optional<std::vector<BYTE>> WholeStore =
      cpu_oracle::storeBufferBoundedByView(*Input, Layout, 0);
  std::optional<std::vector<BYTE>> Unbounded =
      cpu_oracle::storeBufferBoundedByView(*Input, Layout, *BufferSize);
  VERIFY_IS_TRUE(PerElement.has_value() && WholeStore.has_value() &&
                     Unbounded.has_value(),
                 "Unable to derive the StoreDescriptorOOB candidates");

  VERIFY_IS_TRUE(*PerElement != *WholeStore,
                 "The destination view must admit at least one whole element");
  VERIFY_IS_TRUE(*PerElement != *Unbounded,
                 "The destination view must exclude at least one element");

  std::stringstream ExtraDefs;
  ExtraDefs << " -DLOAD_OFFSET=" << Layout.OffsetBytes;
  ExtraDefs << " -DLOAD_STRIDE=" << Layout.StrideBytes;
  ExtraDefs << " -DLOAD_LAYOUT=" << static_cast<int>(Layout.Layout);
  ExtraDefs << " -DSTORE_OFFSET=" << Layout.OffsetBytes;
  ExtraDefs << " -DSTORE_STRIDE=" << Layout.StrideBytes;
  ExtraDefs << " -DSTORE_LAYOUT=" << static_cast<int>(Layout.Layout);
  ExtraDefs << " -DDECLARED_ALIGN=" << DescriptorDeclaredAlignment;

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, LoadStoreDescriptorShader, "cs_6_10", Args,
                Verbose);

  // Only the destination view is short. The source is viewed in full so the
  // load cannot be bounds checked as well, which would leave the observed
  // result attributable to either operation.
  const cpu_oracle::TypedMatrix InputMatrix = *Input;
  auto Op = createComputeOp(LoadStoreDescriptorShader, "cs_6_10",
                            "DescriptorTable(UAV(u0), UAV(u1))", Args.c_str());
  addUAVBuffer(Op.get(), "Input", *BufferSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", *BufferSize, true, "byname");
  addHeapRawUAV(Op.get(), "ResHeap", "Input", *BufferSize);
  addHeapRawUAV(Op.get(), "ResHeap", "Output", OutputViewBytes);
  addRootTable(Op.get(), 0, "ResHeap");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [InputMatrix, Layout](LPCSTR Name, std::vector<BYTE> &Data,
                            st::ShaderOp *) {
        cpu_oracle::fillPoison(Data.data(), Data.size());
        if (_stricmp(Name, "Input") != 0)
          return;
        VERIFY_IS_TRUE(cpu_oracle::writeMatrixBuffer(InputMatrix, Layout, Data),
                       "Unable to encode typed StoreDescriptorOOB input");
      },
      [Layout](ID3D12GraphicsCommandList *, st::ShaderOpTest *Test) {
        verifyDescriptorBaseAlignment(Test, "Input", Layout.OffsetBytes);
        verifyDescriptorBaseAlignment(Test, "Output", Layout.OffsetBytes);
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(cpu_oracle::verifyStoreBuffer(
      OutData.data(), OutData.size(), {*PerElement, *WholeStore},
      L"HLSL proposal 0035 bounds checking on MatrixStoreToDescriptor: either "
      L"the whole store or only the out-of-view element stores become a no-op",
      Verbose));
}

// No offset and a tightly packed stride: a matrix occupying the whole buffer.
static cpu_oracle::MatrixBufferLayout packedLayout(const MatrixParams &Params) {
  return cpu_oracle::MatrixBufferLayout{
      Params.Layout,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/Params.strideBytes(),
  };
}

// Where the padded cases put the matrix. Independent of the alignment above,
// which is the contract rather than a placement, but constrained by it.
static constexpr size_t DescriptorAlignedOffset = 128;
static_assert(DescriptorAlignedOffset % DescriptorDeclaredAlignment == 0,
              "descriptor offset must keep the first element aligned");

void DxilConf_SM610_LinAlg::LoadStoreDescriptor_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"LoadStoreDescriptor_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runLoadStoreDescriptor(D3DDevice, DxcSupport, Params, packedLayout(Params),
                         packedLayout(Params), VerboseLogging,
                         SelectedWaveSize);
}

// Places the matrix at a non-zero offset and pads the row stride, so the
// destination holds bytes the store must not touch: a 128-byte prologue and
// three 16-byte gaps between its four rows. A store that addresses by element
// index rather than by the supplied stride writes into that padding, which the
// untouched-byte check catches and the element comparison cannot.
void DxilConf_SM610_LinAlg::
    LoadStoreDescriptor_Wave_4x8_F16_RowMajorOffsetPadded() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {Params.Use},
          L"LoadStoreDescriptor_Wave_4x8_F16_RowMajorOffsetPadded",
          SelectedWaveSize))
    return;

  // A packed row of 8 F16 values is 16 bytes; 32 leaves a 16-byte gap between
  // rows while remaining a legal multiple of 16.
  const cpu_oracle::MatrixBufferLayout Layout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/32,
  };

  runLoadStoreDescriptor(D3DDevice, DxcSupport, Params, Layout, Layout,
                         VerboseLogging, SelectedWaveSize);
}

// Loads RowMajor and stores ColumnMajor, which a shared layout cannot express:
// with the same layout on both sides, an implementation that ignores the
// layout argument entirely still round trips byte-identically, because the
// mapping it applies to the load it applies again to the store. Reading one
// layout and writing the other stops the two from cancelling.
void DxilConf_SM610_LinAlg::
    LoadStoreDescriptor_Wave_4x8_F32_RowMajorToColumnMajor() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {Params.Use},
          L"LoadStoreDescriptor_Wave_4x8_F32_RowMajorToColumnMajor",
          SelectedWaveSize))
    return;

  // Source rows of 8 F32 values are 32 bytes packed, padded here to 48.
  const cpu_oracle::MatrixBufferLayout LoadLayout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/48,
  };

  // Destination columns of 4 F32 values are 16 bytes, which is already a legal
  // stride, so the column-major side is stored packed.
  const cpu_oracle::MatrixBufferLayout StoreLayout = {
      MatrixLayout::ColumnMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/16,
  };

  runLoadStoreDescriptor(D3DDevice, DxcSupport, Params, LoadLayout, StoreLayout,
                         VerboseLogging, SelectedWaveSize);
}

// The same cross-layout axis on F16, because no tier is required to support
// Fp32 matrices and the F32 case above can skip in its entirety. The shape
// must stay non-square: swapping the two layouts transposes on load and back
// on store, and for a square matrix those cancel byte for byte whatever
// strides are used.
void DxilConf_SM610_LinAlg::
    LoadStoreDescriptor_Wave_4x8_F16_RowMajorToColumnMajor() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {Params.Use},
          L"LoadStoreDescriptor_Wave_4x8_F16_RowMajorToColumnMajor",
          SelectedWaveSize))
    return;

  // Source rows of 8 F16 values are 16 bytes packed, padded here to 48.
  const cpu_oracle::MatrixBufferLayout LoadLayout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/48,
  };

  // Destination columns of 4 F16 values are 8 bytes, padded here to 16 so the
  // column-major side carries a gap of its own rather than sitting packed.
  const cpu_oracle::MatrixBufferLayout StoreLayout = {
      MatrixLayout::ColumnMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/16,
  };

  runLoadStoreDescriptor(D3DDevice, DxcSupport, Params, LoadLayout, StoreLayout,
                         VerboseLogging, SelectedWaveSize);
}

// Half the source matrix lies outside the view the descriptor carries. The
// boundary is deliberately placed mid-row rather than on a row boundary, so an
// implementation that bounds checks a row at a time cannot pass it.
void DxilConf_SM610_LinAlg::LoadDescriptorOOB_Wave_16x16_F16_PartialView() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"LoadDescriptorOOB_Wave_16x16_F16_"
                                    L"PartialView",
                                    SelectedWaveSize))
    return;

  // Packed, so the buffer is 16 rows of 32 bytes. A 264 byte view holds the
  // first 132 elements: rows 0 to 7 whole, then four elements of row 8.
  runLoadDescriptorOutOfBounds(D3DDevice, DxcSupport, Params,
                               packedLayout(Params), /*InputViewBytes=*/264,
                               VerboseLogging, SelectedWaveSize);
}

// The same behaviour where the matrix is offset and its rows are padded, so
// the view boundary falls in a different place for byte offsets than it does
// for element indices. An implementation that bounds checks by element index
// keeps elements this view does not reach.
void DxilConf_SM610_LinAlg::
    LoadDescriptorOOB_Wave_4x8_F16_OffsetPaddedPartialView() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"LoadDescriptorOOB_Wave_4x8_F16_"
                                    L"OffsetPaddedPartialView",
                                    SelectedWaveSize))
    return;

  const cpu_oracle::MatrixBufferLayout Layout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/32,
  };

  // Elements sit at 128 + 32*Row + 2*Column. A 172 byte view holds row 0
  // whole and columns 0 to 5 of row 1, so it cuts within a row and stops
  // short of the padding rather than on it.
  runLoadDescriptorOutOfBounds(D3DDevice, DxcSupport, Params, Layout,
                               /*InputViewBytes=*/172, VerboseLogging,
                               SelectedWaveSize);
}

// The same two views on the destination instead of the source, so the rule
// being exercised is bounds checking on the store rather than on the load.
void DxilConf_SM610_LinAlg::StoreDescriptorOOB_Wave_16x16_F16_PartialView() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"StoreDescriptorOOB_Wave_16x16_F16_"
                                    L"PartialView",
                                    SelectedWaveSize))
    return;

  // Packed, so the buffer is 16 rows of 32 bytes. A 260 byte view admits the
  // first 130 elements: rows 0 to 7 whole, then two of row 8. Ending two
  // elements into the row keeps the boundary off the round multiples a
  // coarser-than-per-element bounds check would land on.
  runStoreDescriptorOutOfBounds(D3DDevice, DxcSupport, Params,
                                packedLayout(Params), /*OutputViewBytes=*/260,
                                VerboseLogging, SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::
    StoreDescriptorOOB_Wave_4x8_F16_OffsetPaddedPartialView() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"StoreDescriptorOOB_Wave_4x8_F16_"
                                    L"OffsetPaddedPartialView",
                                    SelectedWaveSize))
    return;

  const cpu_oracle::MatrixBufferLayout Layout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/DescriptorAlignedOffset,
      /*StrideBytes=*/32,
  };

  // Elements sit at 128 + 32*Row + 2*Column. A 172 byte view holds row 0
  // whole and columns 0 to 5 of row 1, so it cuts within a row and stops
  // short of the padding rather than on it.
  runStoreDescriptorOutOfBounds(D3DDevice, DxcSupport, Params, Layout,
                                /*OutputViewBytes=*/172, VerboseLogging,
                                SelectedWaveSize);
}

static const char SplatStoreShader[] = R"(
  RWByteAddressBuffer Output : register(u0);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_FillMatrix(Mat, FILL_VALUE);
    __builtin_LinAlg_MatrixStoreToDescriptor(
      Mat, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runSplatStore(ID3D12Device *Device,
                          dxc::SpecificDllLoader &DxcSupport,
                          const MatrixParams &Params, float FillValue,
                          bool Verbose, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  STREAM_FLOAT(ExtraDefs, "FILL_VALUE", FillValue);

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, SplatStoreShader, "cs_6_10", Args, Verbose);

  auto Expected =
      makeExpectedMat(Params.CompType, Params.M, Params.N, FillValue, false);

  auto Op =
      createComputeOp(SplatStoreShader, "cs_6_10", "UAV(u0)", Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::SplatStore_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"SplatStore_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runSplatStore(D3DDevice, DxcSupport, Params, 42.0f, VerboseLogging,
                SelectedWaveSize);
}

static const char AccumulateDescriptorShader[] = R"(
  #define USE_ACC 2

  ByteAddressBuffer Input : register(t0);
  RWByteAddressBuffer Output : register(u1);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);
    __builtin_LinAlg_MatrixAccumulateToDescriptor(
      Mat, Output, 0, STRIDE, LAYOUT, 128);
    __builtin_LinAlg_MatrixAccumulateToDescriptor(
      Mat, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runAccumulateDescriptor(ID3D12Device *Device,
                                    dxc::SpecificDllLoader &DxcSupport,
                                    const MatrixParams &Params, int FillValue,
                                    bool Verbose, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, AccumulateDescriptorShader, "cs_6_10", Args,
                Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N,
                                  static_cast<float>(FillValue) * 2, false);

  auto Op = createComputeOp(AccumulateDescriptorShader, "cs_6_10",
                            "SRV(t0), UAV(u1)", Args.c_str());
  addSRVBuffer(Op.get(), "Input", BufferSize, "byname");
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [NumElements, Params, FillValue](LPCSTR Name, std::vector<BYTE> &Data,
                                       st::ShaderOp *) {
        VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType, NumElements,
                                       /*StartingVal=*/FillValue,
                                       /*Increment=*/false),
                       "Saw unsupported component type");
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::AccumulateDescriptor_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"AccumulateDescriptor_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;
  if (!accumulateStoreApplicable(
          D3DDevice, Params.CompType,
          linalg_test::AtomicDestination::RWByteAddressBuffer,
          L"AccumulateDescriptor_Wave_16x16_F16"))
    return;

  runAccumulateDescriptor(D3DDevice, DxcSupport, Params, 12, VerboseLogging,
                          SelectedWaveSize);
}

// Element access constructs a wave-scope matrix and then reads or writes its
// components, so applicability is exactly MatrixConstruction for the tile the
// case declares. D3D12LinearAlgebraRuntimeFeatureSupport.md guarantees only
// that some shape whose largest component is 16 or less is reported for a
// supported type, and directs applications wanting smaller shapes to query
// them case by case. Neither Fp32 nor Fp16 matrices are required at Tier 1, so
// every element-access case is capability gated rather than mandatory.
static const char ElementAccessShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);

  // flatten the 2D index into a 1D index then scale by element size
  // Always store row-major and work it out in the test runner
  uint coordToByteOffset(uint2 coord) {
    return (coord.x * N_DIM + coord.y) * ELEM_SIZE;
  }

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);

    // Copy Matrix values from input to output without assuming order
    for (uint I = 0; I < __builtin_LinAlg_MatrixLength(Mat); ++I) {
      uint2 Coord = __builtin_LinAlg_MatrixGetCoordinate(Mat, I);
      uint Offset = coordToByteOffset(Coord);
      ELEM_TYPE Elem;
      __builtin_LinAlg_MatrixGetElement(Elem, Mat, I);
      Output.Store<ELEM_TYPE>(Offset, Elem);
    }

    // Save the matrix length that this thread saw. The length is written
    // to the output right after the matrix, offset by the thread index
    uint LenIdx = (M_DIM * N_DIM * ELEM_SIZE) + (threadID * sizeof(uint));
    uint Len = __builtin_LinAlg_MatrixLength(Mat);
    Output.Store<uint>(LenIdx, Len);
  }
)";

static void runElementAccess(ID3D12Device *Device,
                             dxc::SpecificDllLoader &DxcSupport,
                             const MatrixParams &Params, bool Verbose,
                             UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t NumThreads = Params.NumThreads;
  const size_t MatrixSize = Params.totalBytes();
  // OutputBuf needs to fit the Matrix plus one uint per thread
  const size_t OutputBufSize = MatrixSize + NumThreads * sizeof(uint32_t);

  std::stringstream ExtraDefs;
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;
  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, ElementAccessShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N, 1);

  auto Op = createComputeOp(ElementAccessShader, "cs_6_10", "UAV(u0), UAV(u1)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Input", MatrixSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", OutputBufSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [NumElements, Params](LPCSTR Name, std::vector<BYTE> &Data,
                                        st::ShaderOp *) {
                    VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType,
                                                   NumElements),
                                   "Saw unsupported component type");
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  // Verify the front of the buffer is a list of elements of the expected type
  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));

  // Verify the end of the buffer is NumThreads number of lengths, whose
  // sum is greater than or equal to NumElements
  const BYTE *Out = static_cast<const BYTE *>(OutData.data());
  const uint32_t *Lengths =
      reinterpret_cast<const uint32_t *>(Out + MatrixSize);
  uint32_t TotalLength = 0;
  for (size_t I = 0; I < NumThreads; ++I)
    TotalLength += Lengths[I];
  VERIFY_IS_GREATER_THAN_OR_EQUAL(
      TotalLength, NumElements, "Sum of all lengths must be gte num elements");
}

void DxilConf_SM610_LinAlg::ElementAccess_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementAccess_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runElementAccess(D3DDevice, DxcSupport, Params, VerboseLogging,
                   SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::ElementAccess_Wave_4x8_F32() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = false;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementAccess_Wave_4x8_F32",
                                    SelectedWaveSize))
    return;

  // Non-square dimensions make the row-major coordinate mapping observable: a
  // transposed GetCoordinate would land inside the matrix for a square tile
  // but out of it here.
  runElementAccess(D3DDevice, DxcSupport, Params, VerboseLogging,
                   SelectedWaveSize);
}

static const char ElementSetShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);

    // Increment every element by 5
    for (uint I = 0; I < __builtin_LinAlg_MatrixLength(Mat); ++I) {
      ELEM_TYPE Elem;
      __builtin_LinAlg_MatrixGetElement(Elem, Mat, I);
      Elem = Elem + 5;
      __builtin_LinAlg_MatrixSetElement(Mat, Mat, I, Elem);
    }

    __builtin_LinAlg_MatrixStoreToDescriptor(
      Mat, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runElementSet(ID3D12Device *Device,
                          dxc::SpecificDllLoader &DxcSupport,
                          const MatrixParams &Params, bool Verbose,
                          UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t MatrixSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;
  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, ElementSetShader, "cs_6_10", Args, Verbose);

  // Start counting from 6 since each element was increased by 5
  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N, 6);

  auto Op = createComputeOp(ElementSetShader, "cs_6_10", "UAV(u0), UAV(u1)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Input", MatrixSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", MatrixSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [NumElements, Params](LPCSTR Name, std::vector<BYTE> &Data,
                                        st::ShaderOp *) {
                    VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType,
                                                   NumElements),
                                   "Saw unsupported component type");
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  // Verify the front of the buffer is a list of elements of the expected type
  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::ElementSet_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementSet_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runElementSet(D3DDevice, DxcSupport, Params, VerboseLogging,
                SelectedWaveSize);
}

// Length() is thread local, so the first index past a lane's own length is
// already out of bounds even though the wave collectively holds more elements.
// Probing Length() and a index far beyond it covers both a driver that clamps
// only at the wave total and one that wraps a large index back into range.
static constexpr UINT FarOOBOffset = 64;

// Per-lane record: {uint Length, uint Executed, float Just, float Far}.
static constexpr UINT OOBRecordSize = 16;

// Seeds every output byte so a lane that never writes cannot be mistaken for a
// lane that correctly wrote the specified zero. The output buffer must be
// created "byname" for this to run at all: ShaderOpTest only invokes the
// initializer callback for that mode, and the default "zero" mode would leave
// the buffer holding exactly the value the out-of-bounds read is required to
// produce, making the comparison vacuous.
static constexpr BYTE OOBSentinelByte = 0xCD;

// Seeds the shader's destination locals. Distinct from zero, so a read that is
// dropped rather than performed cannot masquerade as a correct out-of-bounds
// result, and exactly representable in F32.
static constexpr int OOBGetPoisonValue = 999;

static const char ElementGetOOBShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);

    uint Len = __builtin_LinAlg_MatrixLength(Mat);

    // Seeded so that a dropped read leaves a value distinguishable from the
    // zero a correct out-of-bounds read must produce.
    ELEM_TYPE Just = (ELEM_TYPE)POISON_VALUE;
    __builtin_LinAlg_MatrixGetElement(Just, Mat, Len);
    ELEM_TYPE Far = (ELEM_TYPE)POISON_VALUE;
    __builtin_LinAlg_MatrixGetElement(Far, Mat, Len + FAR_OOB_OFFSET);

    // Record unconditionally so the runner can tell that this lane ran.
    uint Base = threadID * OOB_RECORD_SIZE;
    Output.Store<uint>(Base + 0, Len);
    Output.Store<uint>(Base + 4, 1);
    // Widened to float so the runner can read a fixed-width record whatever
    // the element type: a half store would leave the record's upper two bytes
    // holding the sentinel. Half to float is lossless, so no value is masked.
    Output.Store<float>(Base + 8, (float)Just);
    Output.Store<float>(Base + 12, (float)Far);
  }
)";

// Reads back the {Length, Executed} half of each lane record and checks the
// wave actually ran. Returns the total element count the wave reported.
static uint32_t verifyOOBLaneRecords(const BYTE *Records, size_t NumThreads,
                                     UINT SelectedWaveSize, UINT RecordStride,
                                     size_t NumElements, bool Verbose) {
  uint32_t ExecutedLanes = 0;
  uint32_t TotalLength = 0;
  for (size_t I = 0; I < NumThreads; ++I) {
    const BYTE *Record = Records + I * RecordStride;
    uint32_t Length = 0;
    uint32_t Executed = 0;
    memcpy(&Length, Record, sizeof(Length));
    memcpy(&Executed, Record + 4, sizeof(Executed));
    if (Executed != 1)
      continue;
    ++ExecutedLanes;
    TotalLength += Length;
    if (Verbose)
      hlsl_test::LogCommentFmt(L"lane %u reported Length=%u",
                               static_cast<UINT>(I), Length);
  }

  // Only wave 0 runs, so exactly the lanes of the wave the capability query
  // selected must have written a record.
  VERIFY_ARE_EQUAL(ExecutedLanes, SelectedWaveSize,
                   "Every lane of the selected wave must execute");
  VERIFY_IS_GREATER_THAN_OR_EQUAL(
      TotalLength, static_cast<uint32_t>(NumElements),
      "Sum of all lengths must be gte num elements");
  return TotalLength;
}

static void runElementGetOOB(ID3D12Device *Device,
                             dxc::SpecificDllLoader &DxcSupport,
                             const MatrixParams &Params, bool Verbose,
                             UINT ForcedWaveSize) {
  VERIFY_IS_TRUE(Params.CompType == ComponentType::F32 ||
                     Params.CompType == ComponentType::F16,
                 "Out-of-bounds Get records widen the element to float");
  const size_t NumElements = Params.totalElements();
  const size_t NumThreads = Params.NumThreads;
  const size_t MatrixSize = Params.totalBytes();
  const size_t OutputBufSize = NumThreads * OOBRecordSize;

  std::stringstream ExtraDefs;
  ExtraDefs << " -DFAR_OOB_OFFSET=" << FarOOBOffset;
  ExtraDefs << " -DOOB_RECORD_SIZE=" << OOBRecordSize;
  ExtraDefs << " -DPOISON_VALUE=" << OOBGetPoisonValue;
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;
  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, ElementGetOOBShader, "cs_6_10", Args, Verbose);

  auto Op = createComputeOp(ElementGetOOBShader, "cs_6_10", "UAV(u0), UAV(u1)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Input", MatrixSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", OutputBufSize, true, "byname");
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [NumElements, Params](LPCSTR Name, std::vector<BYTE> &Data,
                                        st::ShaderOp *) {
                    if (_stricmp(Name, "Output") == 0) {
                      std::fill(Data.begin(), Data.end(), OOBSentinelByte);
                      return;
                    }
                    VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType,
                                                   NumElements),
                                   "Saw unsupported component type");
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  const BYTE *Out = static_cast<const BYTE *>(OutData.data());

  verifyOOBLaneRecords(Out, NumThreads, ForcedWaveSize, OOBRecordSize,
                       NumElements, Verbose);

  // 0035-linalg-matrix.md: reading an index outside [0, Length()-1] yields
  // zero cast to the element type.
  for (size_t I = 0; I < NumThreads; ++I) {
    const BYTE *Record = Out + I * OOBRecordSize;
    uint32_t Executed = 0;
    memcpy(&Executed, Record + 4, sizeof(Executed));
    if (Executed != 1)
      continue;

    float Just = 0.0f;
    float Far = 0.0f;
    memcpy(&Just, Record + 8, sizeof(Just));
    memcpy(&Far, Record + 12, sizeof(Far));
    VERIFY_ARE_EQUAL(Just, 0.0f,
                     "Get at Length() must return zero cast to the element "
                     "type");
    VERIFY_ARE_EQUAL(Far, 0.0f,
                     "Get far past Length() must return zero cast to the "
                     "element type");
  }
}

void DxilConf_SM610_LinAlg::ElementGetOOB_Wave_4x8_F32() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = false;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementGetOOB_Wave_4x8_F32",
                                    SelectedWaveSize))
    return;

  runElementGetOOB(D3DDevice, DxcSupport, Params, VerboseLogging,
                   SelectedWaveSize);
}

static const char ElementSetOOBShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);

    uint Len = __builtin_LinAlg_MatrixLength(Mat);

    // Both indices are outside this lane's range, so both writes must be
    // no-ops and the stored matrix must still equal the loaded one.
    __builtin_LinAlg_MatrixSetElement(Mat, Mat, Len, (ELEM_TYPE)POISON_VALUE);
    __builtin_LinAlg_MatrixSetElement(Mat, Mat, Len + FAR_OOB_OFFSET,
                                      (ELEM_TYPE)POISON_VALUE);

    __builtin_LinAlg_MatrixStoreToDescriptor(
      Mat, Output, 0, STRIDE, LAYOUT, 128);

    uint Base = MATRIX_BYTES + threadID * OOB_RECORD_SIZE;
    Output.Store<uint>(Base + 0, Len);
    Output.Store<uint>(Base + 4, 1);
  }
)";

static void runElementSetOOB(ID3D12Device *Device,
                             dxc::SpecificDllLoader &DxcSupport,
                             const MatrixParams &Params, bool Verbose,
                             UINT ForcedWaveSize) {
  const size_t NumElements = Params.totalElements();
  const size_t NumThreads = Params.NumThreads;
  const size_t MatrixSize = Params.totalBytes();
  const size_t OutputBufSize = MatrixSize + NumThreads * OOBRecordSize;

  // Distinct from every sequential input value, so a stray write is visible.
  const int PoisonValue = 999;

  std::stringstream ExtraDefs;
  ExtraDefs << " -DFAR_OOB_OFFSET=" << FarOOBOffset;
  ExtraDefs << " -DOOB_RECORD_SIZE=" << OOBRecordSize;
  ExtraDefs << " -DMATRIX_BYTES=" << MatrixSize;
  ExtraDefs << " -DPOISON_VALUE=" << PoisonValue;
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;
  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, ElementSetOOBShader, "cs_6_10", Args, Verbose);

  // The matrix must come back exactly as it went in.
  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N, 1);

  auto Op = createComputeOp(ElementSetOOBShader, "cs_6_10", "UAV(u0), UAV(u1)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Input", MatrixSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", OutputBufSize, true, "byname");
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [NumElements, Params](LPCSTR Name, std::vector<BYTE> &Data,
                                        st::ShaderOp *) {
                    if (_stricmp(Name, "Output") == 0) {
                      std::fill(Data.begin(), Data.end(), OOBSentinelByte);
                      return;
                    }
                    VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType,
                                                   NumElements),
                                   "Saw unsupported component type");
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  const BYTE *Out = static_cast<const BYTE *>(OutData.data());

  verifyOOBLaneRecords(Out + MatrixSize, NumThreads, ForcedWaveSize,
                       OOBRecordSize, NumElements, Verbose);

  // 0035-linalg-matrix.md: setting an index outside [0, Length()-1] is a
  // no-op, so no poisoned value may appear anywhere in the matrix.
  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::ElementSetOOB_Wave_4x8_F32() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = false;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementSetOOB_Wave_4x8_F32",
                                    SelectedWaveSize))
    return;

  runElementSetOOB(D3DDevice, DxcSupport, Params, VerboseLogging,
                   SelectedWaveSize);
}

// Out-of-bounds element access on F16. Both cases above pin the boundary
// behaviour to F32, which no tier is required to support, so a conforming
// F16-only device would exercise neither.
void DxilConf_SM610_LinAlg::ElementGetOOB_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementGetOOB_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runElementGetOOB(D3DDevice, DxcSupport, Params, VerboseLogging,
                   SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::ElementSetOOB_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"ElementSetOOB_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runElementSetOOB(D3DDevice, DxcSupport, Params, VerboseLogging,
                   SelectedWaveSize);
}

static const char CopyConvertShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);
  RWByteAddressBuffer SourceAfter : register(u2);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Src;
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(DST_COMP_TYPE, DST_M_DIM, DST_N_DIM, USE, SCOPE)]]
      Dst;

    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Src, Input, 0, SRC_STRIDE, LAYOUT, 128);
    __builtin_LinAlg_CopyConvertMatrix(Dst, Src, TRANSPOSE);
    __builtin_LinAlg_MatrixStoreToDescriptor(
      Dst, Output, 0, DST_STRIDE, LAYOUT, 128);
    __builtin_LinAlg_MatrixStoreToDescriptor(
      Src, SourceAfter, 0, SRC_STRIDE, LAYOUT, 128);
  }
)";

static HRESULT selectCopyConvertWaveSize(ID3D12Device *Device,
                                         const MatrixParams &Params,
                                         ComponentType DestinationCompType,
                                         bool Transpose, bool &Supported,
                                         UINT &SelectedWaveSize) {
  Supported = false;
  SelectedWaveSize = 0;
  if (!Device || Params.Use != MatrixUse::A ||
      !linalg_test::isLegalScope(
          linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION,
          Params.Scope))
    return E_INVALIDARG;

  std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> SourceType =
      toCapabilityDataType(Params.CompType);
  std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> DestinationType =
      toCapabilityDataType(DestinationCompType);
  if (!SourceType.has_value() || !DestinationType.has_value())
    return E_INVALIDARG;

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR) || !Tier.supported())
    return HR;

  UINT MinWaveSize = 0;
  UINT MaxWaveSize = 0;
  HR = queryLaunchableWaveSizes(Device, MinWaveSize, MaxWaveSize);
  if (FAILED(HR))
    return HR;
  if (MinWaveSize == 0) {
    hlsl_test::LogCommentFmt(
        L"Wave operations are unsupported; MatrixConstruction is not "
        L"applicable");
    return S_OK;
  }

  MatrixParams Destination = Params;
  Destination.CompType = DestinationCompType;
  if (Transpose) {
    Destination.M = Params.N;
    Destination.N = Params.M;
  }

  for (UINT WaveSize = 4; WaveSize <= 128; WaveSize *= 2) {
    if (WaveSize < MinWaveSize || WaveSize > MaxWaveSize ||
        WaveSize > static_cast<UINT>(Params.NumThreads))
      continue;

    bool SourceSupported = false;
    HR = supportsMatrixShape(Device, *SourceType, WaveSize, MatrixUse::A,
                             Params.M, Params.N, SourceSupported);
    if (FAILED(HR))
      return HR;

    bool DestinationSupported = false;
    HR =
        supportsMatrixShape(Device, *DestinationType, WaveSize, MatrixUse::A,
                            Destination.M, Destination.N, DestinationSupported);
    if (FAILED(HR))
      return HR;

    if (SourceSupported && DestinationSupported) {
      hlsl_test::LogCommentFmt(
          L"CopyConvert capability matched wave=%u for source=%ux%u and "
          L"destination=%ux%u",
          WaveSize, Params.M, Params.N, Destination.M, Destination.N);
      Supported = true;
      SelectedWaveSize = WaveSize;
      return S_OK;
    }
  }

  hlsl_test::LogCommentFmt(
      L"No MatrixConstruction query supports CopyConvert source=%ux%u and "
      L"destination=%ux%u for any wave size launchable within shader "
      L"WaveSize(4,128) and a %d-thread group",
      Params.M, Params.N, Destination.M, Destination.N, Params.NumThreads);
  return S_OK;
}

static bool copyConvertApplicable(ID3D12Device *Device,
                                  const MatrixParams &Params,
                                  ComponentType DestinationCompType,
                                  bool Transpose, LPCWSTR CaseName,
                                  UINT &SelectedWaveSize) {
  bool Supported = false;
  const HRESULT QueryResult =
      selectCopyConvertWaveSize(Device, Params, DestinationCompType, Transpose,
                                Supported, SelectedWaveSize);
  if (!applyApplicability(
          linalg_test::classifyApplicability(
              QueryResult, Supported,
              linalg_test::CapabilityRequirement::CapabilityGated),
          CaseName))
    return false;

  VERIFY_IS_TRUE(SelectedWaveSize != 0,
                 "A case cleared to run must have a selected wave size");
  return true;
}

static void runCopyConvert(ID3D12Device *Device,
                           dxc::SpecificDllLoader &DxcSupport,
                           const MatrixParams &Params,
                           ComponentType DestinationCompType, bool Verbose,
                           bool Transpose, UINT ForcedWaveSize = 0) {
  MatrixParams DstParams = Params;
  DstParams.CompType = DestinationCompType;
  if (Transpose) {
    DstParams.M = Params.N;
    DstParams.N = Params.M;
  }

  std::stringstream ExtraDefs;
  ExtraDefs << " -DTRANSPOSE=" << Transpose;
  ExtraDefs << " -DDST_COMP_TYPE=" << static_cast<UINT>(DestinationCompType);
  ExtraDefs << " -DDST_M_DIM=" << DstParams.M;
  ExtraDefs << " -DDST_N_DIM=" << DstParams.N;
  ExtraDefs << " -DSRC_STRIDE=" << Params.strideBytes();
  ExtraDefs << " -DDST_STRIDE=" << DstParams.strideBytes();
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, CopyConvertShader, "cs_6_10", Args, Verbose);

  std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeSequentialMatrix(Params.CompType, Params.M, Params.N);
  VERIFY_IS_TRUE(Input.has_value(),
                 "Unable to construct typed CopyConvert input");
  std::optional<cpu_oracle::TypedMatrix> Converted =
      cpu_oracle::makeSequentialMatrix(DstParams.CompType, Params.M, Params.N);
  VERIFY_IS_TRUE(Converted.has_value(),
                 "Unable to construct typed CopyConvert conversion oracle");
  if (!Input.has_value() || !Converted.has_value())
    return;
  std::optional<cpu_oracle::TypedMatrix> Expected =
      Transpose ? cpu_oracle::transposeMatrix(*Converted) : Converted;
  VERIFY_IS_TRUE(Expected.has_value(),
                 "Unable to construct independent CopyConvert oracle");
  if (!Expected.has_value())
    return;

  cpu_oracle::MatrixBufferLayout SourceLayout = {
      Params.Layout,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/Params.strideBytes(),
  };
  cpu_oracle::MatrixBufferLayout DestinationLayout = {
      DstParams.Layout,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/DstParams.strideBytes(),
  };
  std::optional<size_t> SourceBufferSize =
      cpu_oracle::getMatrixBufferSize(*Input, SourceLayout);
  std::optional<size_t> DestinationBufferSize =
      cpu_oracle::getMatrixBufferSize(*Expected, DestinationLayout);
  VERIFY_IS_TRUE(SourceBufferSize.has_value(),
                 "Unable to size typed CopyConvert input");
  VERIFY_IS_TRUE(DestinationBufferSize.has_value(),
                 "Unable to size typed CopyConvert output");
  if (!SourceBufferSize.has_value() || !DestinationBufferSize.has_value())
    return;

  cpu_oracle::TypedMatrix InputMatrix = *Input;
  cpu_oracle::MatrixResultOracle Oracle = cpu_oracle::exactResult(
      *Expected,
      L"HLSL proposal 0035 CopyConvertMatrix transpose and descriptor layout");
  cpu_oracle::MatrixResultOracle SourceOracle = cpu_oracle::exactResult(
      *Input, L"CopyConvertMatrix leaves the source matrix unmodified");

  auto Op = createComputeOp(CopyConvertShader, "cs_6_10",
                            "UAV(u0), UAV(u1), UAV(u2)", Args.c_str());
  addUAVBuffer(Op.get(), "Input", *SourceBufferSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", *DestinationBufferSize, true);
  addUAVBuffer(Op.get(), "SourceAfter", *SourceBufferSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");
  addRootView(Op.get(), 2, "SourceAfter");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [InputMatrix, SourceLayout](LPCSTR Name, std::vector<BYTE> &Data,
                                  st::ShaderOp *) {
        if (_stricmp(Name, "Input") != 0)
          return;
        VERIFY_IS_TRUE(
            cpu_oracle::writeMatrixBuffer(InputMatrix, SourceLayout, Data),
            "Unable to encode typed CopyConvert input");
      });

  MappedData OutData;
  MappedData SourceAfterData;
  Result->Test->GetReadBackData("Output", &OutData);
  Result->Test->GetReadBackData("SourceAfter", &SourceAfterData);

  VERIFY_IS_TRUE(cpu_oracle::verifyMatrixBuffer(
      OutData.data(), OutData.size(), DestinationLayout, Oracle, Verbose));
  VERIFY_IS_TRUE(cpu_oracle::verifyMatrixBuffer(
      SourceAfterData.data(), SourceAfterData.size(), SourceLayout,
      SourceOracle, Verbose));
}

void DxilConf_SM610_LinAlg::CopyConvert_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!copyConvertApplicable(D3DDevice, Params, ComponentType::F16,
                             /*Transpose=*/false, L"CopyConvert_Wave_16x16_F16",
                             SelectedWaveSize))
    return;

  runCopyConvert(D3DDevice, DxcSupport, Params, ComponentType::F16,
                 VerboseLogging,
                 /*Transpose=*/false, SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::CopyConvert_Wave_16x16_F16_Transpose() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!copyConvertApplicable(D3DDevice, Params, ComponentType::F16,
                             /*Transpose=*/true,
                             L"CopyConvert_Wave_16x16_F16_Transpose",
                             SelectedWaveSize))
    return;

  runCopyConvert(D3DDevice, DxcSupport, Params, ComponentType::F16,
                 VerboseLogging,
                 /*Transpose=*/true, SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::CopyConvert_Wave_4x8_F32_Transpose() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = false;

  UINT SelectedWaveSize = 0;
  if (!copyConvertApplicable(D3DDevice, Params, ComponentType::F32,
                             /*Transpose=*/true,
                             L"CopyConvert_Wave_4x8_F32_Transpose",
                             SelectedWaveSize))
    return;

  // Non-square dimensions make the destination shape and row stride observable.
  runCopyConvert(D3DDevice, DxcSupport, Params, ComponentType::F32,
                 VerboseLogging,
                 /*Transpose=*/true, SelectedWaveSize);
}

static const char MatMatMulShader[] = R"(
  #define USE_A 0
  #define USE_B 1
  #define USE_ACC 2

  RWByteAddressBuffer Output : register(u0);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, K_DIM, USE_A, SCOPE)]]
      MatA;
    __builtin_LinAlg_FillMatrix(MatA, A_FILL);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, K_DIM, N_DIM, USE_B, SCOPE)]]
      MatB;
    __builtin_LinAlg_FillMatrix(MatB, B_FILL);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      MatC;
    __builtin_LinAlg_MatrixMatrixMultiply(MatC, MatA, MatB);

    __builtin_LinAlg_MatrixStoreToDescriptor(
      MatC, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runMatMatMul(ID3D12Device *Device,
                         dxc::SpecificDllLoader &DxcSupport,
                         const MatrixParams &Params, bool Verbose, MatrixDim K,
                         float AFill, float BFill, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DK_DIM=" << K;
  STREAM_FLOAT(ExtraDefs, "A_FILL", AFill);
  STREAM_FLOAT(ExtraDefs, "B_FILL", BFill);

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, MatMatMulShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N,
                                  AFill * BFill * K, /*Increment=*/false);

  auto Op =
      createComputeOp(MatMatMulShader, "cs_6_10", "UAV(u0)", Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::MatMatMul_Wave_16x16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!waveMatMulApplicable(D3DDevice, Params, /*K=*/16,
                            L"MatMatMul_Wave_16x16x16_F16", SelectedWaveSize))
    return;

  runMatMatMul(D3DDevice, DxcSupport, Params, VerboseLogging, /*K=*/16,
               /*AFill=*/2.0f, /*BFill=*/3.0f, SelectedWaveSize);
}

static const char MatMatMulAccumShader[] = R"(
  #define USE_A 0
  #define USE_B 1
  #define USE_ACC 2

  RWByteAddressBuffer Output : register(u0);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, K_DIM, USE_A, SCOPE)]]
      MatA;
    __builtin_LinAlg_FillMatrix(MatA, A_FILL);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, K_DIM, N_DIM, USE_B, SCOPE)]]
      MatB;
    __builtin_LinAlg_FillMatrix(MatB, B_FILL);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      MatC;
    __builtin_LinAlg_FillMatrix(MatC, C_FILL);

    __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(MatC, MatA, MatB, MatC);

    __builtin_LinAlg_MatrixStoreToDescriptor(
      MatC, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runMatMatMulAccum(ID3D12Device *Device,
                              dxc::SpecificDllLoader &DxcSupport,
                              const MatrixParams &Params, bool Verbose,
                              MatrixDim K, float AFill, float BFill,
                              float CFill, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DK_DIM=" << K;
  STREAM_FLOAT(ExtraDefs, "A_FILL", AFill);
  STREAM_FLOAT(ExtraDefs, "B_FILL", BFill);
  STREAM_FLOAT(ExtraDefs, "C_FILL", CFill);

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, MatMatMulAccumShader, "cs_6_10", Args, Verbose);

  auto Expected =
      makeExpectedMat(Params.CompType, Params.M, Params.N,
                      AFill * BFill * K + CFill, /*Increment=*/false);

  auto Op =
      createComputeOp(MatMatMulAccumShader, "cs_6_10", "UAV(u0)", Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::MatMatMulAccum_Wave_16x16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!waveMatMulApplicable(D3DDevice, Params, /*K=*/16,
                            L"MatMatMulAccum_Wave_16x16x16_F16",
                            SelectedWaveSize))
    return;

  runMatMatMulAccum(D3DDevice, DxcSupport, Params, VerboseLogging, /*K=*/16,
                    /*AFill=*/2.0f, /*BFill=*/3.0f, /*CFill=*/4.0f,
                    SelectedWaveSize);
}

static const char MatAccumShader[] = R"(
  #define USE_A 0
  #define USE_ACC 2

  RWByteAddressBuffer Output : register(u0);

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    if (GetGroupWaveIndex() != 0)
      return;

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      MatLHS;
    __builtin_LinAlg_FillMatrix(MatLHS, LHS_FILL);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_A, SCOPE)]]
      MatRHS;
    __builtin_LinAlg_FillMatrix(MatRHS, RHS_FILL);

    __builtin_LinAlg_MatrixAccumulate(MatLHS, MatLHS, MatRHS);

    __builtin_LinAlg_MatrixStoreToDescriptor(
      MatLHS, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runMatAccum(ID3D12Device *Device,
                        dxc::SpecificDllLoader &DxcSupport,
                        const MatrixParams &Params, bool Verbose, float LHSFill,
                        float RHSFill, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  STREAM_FLOAT(ExtraDefs, "LHS_FILL", LHSFill);
  STREAM_FLOAT(ExtraDefs, "RHS_FILL", RHSFill);

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, MatAccumShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N,
                                  LHSFill + RHSFill, /*Increment=*/false);

  auto Op = createComputeOp(MatAccumShader, "cs_6_10", "UAV(u0)", Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::MatAccum_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  // MatAccum builds both an accumulator and an A matrix, and the two roles pin
  // different extents of the same shape, so both must be constructible.
  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {MatrixUse::Accumulator, MatrixUse::A},
          L"MatAccum_Wave_16x16_F16", SelectedWaveSize))
    return;

  runMatAccum(D3DDevice, DxcSupport, Params, VerboseLogging,
              /*LHSFill=*/2.0f, /*RHSFill=*/3.0f, SelectedWaveSize);
}

namespace cpu_oracle {

static std::vector<int64_t>
multiplyIntegerMatrices(MatrixDim M, MatrixDim K, MatrixDim N,
                        const std::vector<int64_t> &MatrixA,
                        const std::vector<int64_t> &MatrixB,
                        const std::vector<int64_t> *Accumulator = nullptr) {
  std::vector<int64_t> Result(static_cast<size_t>(M) * N);
  for (MatrixDim Row = 0; Row < M; ++Row) {
    for (MatrixDim Column = 0; Column < N; ++Column) {
      const size_t ResultIndex = static_cast<size_t>(Row) * N + Column;
      int64_t Sum = Accumulator ? (*Accumulator)[ResultIndex] : 0;
      for (MatrixDim Inner = 0; Inner < K; ++Inner)
        Sum += MatrixA[static_cast<size_t>(Row) * K + Inner] *
               MatrixB[static_cast<size_t>(Inner) * N + Column];
      Result[ResultIndex] = Sum;
    }
  }
  return Result;
}

static std::optional<std::vector<BYTE>>
encodeLogicalMatrixBuffer(const MatrixParams &Matrix,
                          const std::vector<int64_t> &Values) {
  if (Matrix.M == 0 || Matrix.N == 0)
    return std::nullopt;
  if (Values.size() != static_cast<size_t>(Matrix.M) * Matrix.N)
    return std::nullopt;
  if (Matrix.Layout != MatrixLayout::RowMajor &&
      Matrix.Layout != MatrixLayout::ColumnMajor)
    return std::nullopt;

  std::optional<TypedMatrix> LogicalMatrix;
  switch (Matrix.CompType) {
  case ComponentType::F16: {
    std::vector<HLSLHalf_t> TypedValues;
    TypedValues.reserve(Values.size());
    for (int64_t Value : Values) {
      if (Value < -2048 || Value > 2048)
        return std::nullopt;
      TypedValues.emplace_back(static_cast<float>(Value));
    }
    LogicalMatrix = makeTypedMatrix(Matrix.M, Matrix.N, std::move(TypedValues));
    break;
  }
  case ComponentType::F32: {
    static constexpr int64_t MaxExactInteger = int64_t(1) << 24;
    std::vector<float> TypedValues;
    TypedValues.reserve(Values.size());
    for (int64_t Value : Values) {
      if (Value < -MaxExactInteger || Value > MaxExactInteger)
        return std::nullopt;
      TypedValues.push_back(static_cast<float>(Value));
    }
    LogicalMatrix = makeTypedMatrix(Matrix.M, Matrix.N, std::move(TypedValues));
    break;
  }
  case ComponentType::I32: {
    std::vector<int32_t> TypedValues;
    TypedValues.reserve(Values.size());
    for (int64_t Value : Values) {
      if (Value < std::numeric_limits<int32_t>::min() ||
          Value > std::numeric_limits<int32_t>::max())
        return std::nullopt;
      TypedValues.push_back(static_cast<int32_t>(Value));
    }
    LogicalMatrix = makeTypedMatrix(Matrix.M, Matrix.N, std::move(TypedValues));
    break;
  }
  case ComponentType::U32: {
    std::vector<uint32_t> TypedValues;
    TypedValues.reserve(Values.size());
    for (int64_t Value : Values) {
      if (Value < 0 ||
          static_cast<uint64_t>(Value) > std::numeric_limits<uint32_t>::max())
        return std::nullopt;
      TypedValues.push_back(static_cast<uint32_t>(Value));
    }
    LogicalMatrix = makeTypedMatrix(Matrix.M, Matrix.N, std::move(TypedValues));
    break;
  }
  default:
    return std::nullopt;
  }
  if (!LogicalMatrix)
    return std::nullopt;

  const MatrixBufferLayout Layout = {
      Matrix.Layout,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/Matrix.strideBytes(),
  };
  std::optional<size_t> BufferSize =
      getMatrixBufferSize(*LogicalMatrix, Layout);
  if (!BufferSize)
    return std::nullopt;

  std::vector<BYTE> Buffer(*BufferSize, 0);
  if (!writeMatrixBuffer(*LogicalMatrix, Layout, Buffer))
    return std::nullopt;
  return Buffer;
}

} // namespace cpu_oracle

void LinAlgCPUOracleTests::MatrixProductOracle() {
  using namespace cpu_oracle;

  const std::vector<int64_t> Product =
      multiplyIntegerMatrices(/*M=*/2, /*K=*/3, /*N=*/2,
                              /*MatrixA=*/{1, 2, 3, 4, 5, 6},
                              /*MatrixB=*/{7, 8, 9, 10, 11, 12});
  VERIFY_IS_TRUE(Product == std::vector<int64_t>({58, 64, 139, 154}));

  const std::vector<int64_t> InitialAccumulator = {1, -1, 2, -2};
  const std::vector<int64_t> Accumulated = multiplyIntegerMatrices(
      /*M=*/2, /*K=*/3, /*N=*/2,
      /*MatrixA=*/{1, 2, 3, 4, 5, 6},
      /*MatrixB=*/{7, 8, 9, 10, 11, 12}, &InitialAccumulator);
  VERIFY_IS_TRUE(Accumulated == std::vector<int64_t>({59, 63, 141, 152}));
}

static bool matrixArithmeticNeeds16BitTypes(ComponentType CompType) {
  return CompType == ComponentType::F16 || CompType == ComponentType::I16 ||
         CompType == ComponentType::U16;
}

static MatrixParams makeMatrixArithmeticParams(ComponentType CompType,
                                               MatrixDim M, MatrixDim N,
                                               MatrixUse Use, MatrixScope Scope,
                                               UINT NumThreads) {
  MatrixParams Params = {};
  Params.CompType = CompType;
  Params.M = M;
  Params.N = N;
  Params.Use = Use;
  Params.Scope = Scope;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = static_cast<int>(NumThreads);
  Params.Enable16Bit = matrixArithmeticNeeds16BitTypes(CompType);
  return Params;
}

static std::vector<int64_t>
makeMatrixArithmeticPattern(MatrixDim M, MatrixDim N, int64_t RowScale,
                            int64_t ColumnScale, int64_t Modulus,
                            int64_t Center) {
  VERIFY_IS_TRUE(M != 0 && N != 0 && Modulus > 0);
  if (M == 0 || N == 0 || Modulus <= 0)
    return {};

  std::vector<int64_t> Values(static_cast<size_t>(M) * N);
  for (MatrixDim Row = 0; Row < M; ++Row) {
    for (MatrixDim Column = 0; Column < N; ++Column) {
      Values[static_cast<size_t>(Row) * N + Column] =
          (static_cast<int64_t>(Row) * RowScale +
           static_cast<int64_t>(Column) * ColumnScale) %
              Modulus -
          Center;
    }
  }
  return Values;
}

enum class MatrixMultiplyOperation {
  Multiply,
  MultiplyAccumulate,
};

struct MatrixMultiplyCase {
  ComponentType MatrixAType = ComponentType::Invalid;
  ComponentType MatrixBType = ComponentType::Invalid;
  ComponentType AccumulatorType = ComponentType::Invalid;
  MatrixDim M = 0;
  MatrixDim K = 0;
  MatrixDim N = 0;
  MatrixMultiplyOperation Operation = MatrixMultiplyOperation::Multiply;
  std::vector<int64_t> MatrixAValues;
  std::vector<int64_t> MatrixBValues;
  std::vector<int64_t> AccumulatorValues;
  std::wstring PublicRule;

  bool accumulates() const {
    return Operation == MatrixMultiplyOperation::MultiplyAccumulate;
  }
};

static bool isMatrixMultiplyCaseValid(const MatrixMultiplyCase &Case) {
  if (Case.M == 0 || Case.K == 0 || Case.N == 0)
    return false;
  if (Case.MatrixAValues.size() != static_cast<size_t>(Case.M) * Case.K)
    return false;
  if (Case.MatrixBValues.size() != static_cast<size_t>(Case.K) * Case.N)
    return false;
  if (Case.accumulates() &&
      Case.AccumulatorValues.size() != static_cast<size_t>(Case.M) * Case.N)
    return false;
  if (!Case.accumulates() && !Case.AccumulatorValues.empty())
    return false;
  if (!toCapabilityDataType(Case.MatrixAType))
    return false;
  if (!toCapabilityDataType(Case.MatrixBType))
    return false;
  if (!toCapabilityDataType(Case.AccumulatorType))
    return false;
  return !Case.PublicRule.empty();
}

static HRESULT matrixMultiplyRolesConstructible(
    ID3D12Device *Device, const MatrixMultiplyCase &Case, UINT WaveSize,
    linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixAType,
    linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixBType,
    linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE AccumulatorType,
    bool &Constructible) {
  Constructible = false;
  struct ConstructionRole {
    linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE Type;
    MatrixUse Use;
    UINT Rows;
    UINT Columns;
  };
  const ConstructionRole Roles[] = {
      {MatrixAType, MatrixUse::A, Case.M, Case.K},
      {MatrixBType, MatrixUse::B, Case.K, Case.N},
      {AccumulatorType, MatrixUse::Accumulator, Case.M, Case.N},
  };

  for (const ConstructionRole &Role : Roles) {
    bool RoleConstructible = false;
    const HRESULT HR =
        supportsMatrixShape(Device, Role.Type, WaveSize, Role.Use, Role.Rows,
                            Role.Columns, RoleConstructible);
    if (FAILED(HR))
      return HR;
    if (!RoleConstructible)
      return S_OK;
  }

  Constructible = true;
  return S_OK;
}

static HRESULT selectWaveArithmeticMultiplyWaveSize(
    ID3D12Device *Device, const MatrixMultiplyCase &Case, LPCWSTR CaseName,
    bool &Supported, UINT &SelectedWaveSize) {
  Supported = false;
  SelectedWaveSize = 0;
  if (!Device)
    return E_INVALIDARG;
  if (!CaseName)
    return E_INVALIDARG;
  if (!linalg_test::isLegalScope(
          linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY,
          MatrixScope::Wave))
    return E_INVALIDARG;

  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixAType =
      *toCapabilityDataType(Case.MatrixAType);
  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixBType =
      *toCapabilityDataType(Case.MatrixBType);
  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE AccumulatorType =
      *toCapabilityDataType(Case.AccumulatorType);

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR) || !Tier.supported())
    return HR;

  UINT MinWaveSize = 0;
  UINT MaxWaveSize = 0;
  HR = queryLaunchableWaveSizes(Device, MinWaveSize, MaxWaveSize);
  if (FAILED(HR))
    return HR;
  if (MinWaveSize == 0)
    return S_OK;

  const linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape = {
      Case.M,
      Case.K,
      Case.N,
  };

  for (UINT WaveSize = 4; WaveSize <= 128; WaveSize *= 2) {
    if (WaveSize < MinWaveSize || WaveSize > MaxWaveSize)
      continue;

    bool RolesConstructible = false;
    HR = matrixMultiplyRolesConstructible(Device, Case, WaveSize, MatrixAType,
                                          MatrixBType, AccumulatorType,
                                          RolesConstructible);
    if (FAILED(HR))
      return HR;
    if (!RolesConstructible)
      continue;

    linalg_test::WaveMatrixMultiplySupport Multiply;
    HR = linalg_test::queryWaveMatrixMultiply(
        Device, {{WaveSize, MatrixAType, MatrixBType, AccumulatorType}, Shape},
        Multiply);
    if (FAILED(HR))
      return HR;
    if (!Multiply.supported())
      continue;

    hlsl_test::LogCommentFmt(
        L"Wave arithmetic capability matched wave=%u, shape=(%u,%u,%u) for %s",
        WaveSize, Case.M, Case.K, Case.N, CaseName);
    Supported = true;
    SelectedWaveSize = WaveSize;
    return S_OK;
  }

  hlsl_test::LogCommentFmt(
      L"No required MatrixConstruction roles and WaveMatrixMultiply "
      L"capability intersect for %s",
      CaseName);
  return S_OK;
}

static UINT selectThreadGroupMatMulSize(
    const linalg_test::ThreadGroupMatrixMultiplySupport &Support,
    UINT WaveSize) {
  constexpr UINT MaxThreadsPerGroup =
      D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
  if (!Support.supported() || WaveSize == 0)
    return 0;

  if (Support.PreferredThreadGroupSize > WaveSize &&
      Support.PreferredThreadGroupSize <= MaxThreadsPerGroup &&
      Support.supportsThreadGroupSize(Support.PreferredThreadGroupSize))
    return Support.PreferredThreadGroupSize;

  const UINT MaxThreadGroupSize =
      std::min(Support.MaxThreadGroupSize, MaxThreadsPerGroup);
  for (UINT ThreadGroupSize = Support.MinThreadGroupSize;
       ThreadGroupSize <= MaxThreadGroupSize;
       ThreadGroupSize += Support.MinThreadGroupSize) {
    if (ThreadGroupSize > WaveSize)
      return ThreadGroupSize;
  }

  if (Support.PreferredThreadGroupSize <= MaxThreadGroupSize &&
      Support.supportsThreadGroupSize(Support.PreferredThreadGroupSize))
    return Support.PreferredThreadGroupSize;
  if (Support.MinThreadGroupSize <= MaxThreadGroupSize)
    return Support.MinThreadGroupSize;
  return 0;
}

static HRESULT selectThreadGroupMatMulConfiguration(
    ID3D12Device *Device, const MatrixMultiplyCase &Case, LPCWSTR CaseName,
    bool &Supported, UINT &SelectedWaveSize, UINT &SelectedThreadGroupSize) {
  Supported = false;
  SelectedWaveSize = 0;
  SelectedThreadGroupSize = 0;
  if (!Device)
    return E_INVALIDARG;
  if (!CaseName)
    return E_INVALIDARG;

  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixAType =
      *toCapabilityDataType(Case.MatrixAType);
  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixBType =
      *toCapabilityDataType(Case.MatrixBType);
  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE AccumulatorType =
      *toCapabilityDataType(Case.AccumulatorType);

  // The HRESULT reports whether the capability queries themselves ran; the
  // Supported out-parameter reports whether a usable configuration was found.
  // Leaving Supported false and returning success is how a case is routed to a
  // capability-gated skip instead of a failure.
  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  VERIFY_SUCCEEDED(HR, "Linear algebra tier query must succeed");
  if (!Tier.supported())
    return S_OK;

  UINT MinWaveSize = 0;
  UINT MaxWaveSize = 0;
  HR = queryLaunchableWaveSizes(Device, MinWaveSize, MaxWaveSize);
  VERIFY_SUCCEEDED(HR, "Launchable wave size query must succeed");
  if (MinWaveSize == 0) {
    hlsl_test::LogCommentFmt(
        L"Wave operations are unsupported; ThreadGroupMatrixMultiply is not "
        L"applicable");
    return S_OK;
  }

  const linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape = {
      Case.M,
      Case.K,
      Case.N,
  };

  for (UINT WaveSize = 4; WaveSize <= 128; WaveSize *= 2) {
    if (WaveSize < MinWaveSize || WaveSize > MaxWaveSize)
      continue;

    bool RolesConstructible = false;
    HR = matrixMultiplyRolesConstructible(Device, Case, WaveSize, MatrixAType,
                                          MatrixBType, AccumulatorType,
                                          RolesConstructible);
    VERIFY_SUCCEEDED(HR, "Matrix role construction query must succeed");
    if (!RolesConstructible)
      continue;

    linalg_test::ThreadGroupMatrixMultiplySupport Multiply;
    HR = linalg_test::queryThreadGroupMatrixMultiply(
        Device, {{WaveSize, MatrixAType, MatrixBType, AccumulatorType}, Shape},
        Multiply);
    VERIFY_SUCCEEDED(HR, "ThreadGroup matrix multiply query must succeed");
    if (!Multiply.supported())
      continue;

    const UINT ThreadGroupSize =
        selectThreadGroupMatMulSize(Multiply, WaveSize);
    if (ThreadGroupSize == 0) {
      hlsl_test::LogCommentFmt(
          L"ThreadGroupMatrixMultiply supports %s at wave=%u, but no legal "
          L"shader group size is available: min=%u, max=%u, preferred=%u",
          CaseName, WaveSize, Multiply.MinThreadGroupSize,
          Multiply.MaxThreadGroupSize, Multiply.PreferredThreadGroupSize);
      continue;
    }

    hlsl_test::LogCommentFmt(
        L"ThreadGroup matrix arithmetic capability matched wave=%u, "
        L"threads=%u, crossWave=%u, shape=(%u,%u,%u) for %s",
        WaveSize, ThreadGroupSize, ThreadGroupSize > WaveSize, Case.M, Case.K,
        Case.N, CaseName);
    Supported = true;
    SelectedWaveSize = WaveSize;
    SelectedThreadGroupSize = ThreadGroupSize;
    return S_OK;
  }

  hlsl_test::LogCommentFmt(
      L"No executable ThreadGroupMatrixMultiply configuration supports %s",
      CaseName);
  // Every query succeeded and none reported support, so Supported stays false
  // and the case is skipped rather than failed.
  return S_OK;
}

static bool threadGroupMatMulApplicable(ID3D12Device *Device,
                                        const MatrixMultiplyCase &Case,
                                        LPCWSTR CaseName,
                                        UINT &SelectedWaveSize,
                                        UINT &SelectedThreadGroupSize) {
  bool Supported = false;
  const HRESULT QueryResult = selectThreadGroupMatMulConfiguration(
      Device, Case, CaseName, Supported, SelectedWaveSize,
      SelectedThreadGroupSize);
  if (!applyApplicability(
          linalg_test::classifyApplicability(
              QueryResult, Supported,
              linalg_test::CapabilityRequirement::CapabilityGated),
          CaseName))
    return false;

  VERIFY_IS_TRUE(SelectedWaveSize != 0,
                 "A case cleared to run must have a selected wave size");
  VERIFY_IS_TRUE(
      SelectedThreadGroupSize != 0,
      "A ThreadGroup case cleared to run must have a selected group size");
  return true;
}

static const char MatrixMultiplyShader[] = R"(
  #define USE_A 0
  #define USE_B 1
  #define USE_ACC 2
  #define LAYOUT_ROW_MAJOR 0

  ByteAddressBuffer MatrixAInput : register(t0);
  ByteAddressBuffer MatrixBInput : register(t1);
#if DO_ACCUMULATE
  ByteAddressBuffer AccumulatorInput : register(t2);
  RWByteAddressBuffer Output : register(u3);
#else
  RWByteAddressBuffer Output : register(u2);
#endif

  [WaveSize(FORCED_WAVE_SIZE)]
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        MATRIX_A_COMP_TYPE, M_DIM, K_DIM, USE_A, MATRIX_SCOPE)]]
      MatA;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      MatA, MatrixAInput, 0, MATRIX_A_STRIDE, LAYOUT_ROW_MAJOR, 128);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        MATRIX_B_COMP_TYPE, K_DIM, N_DIM, USE_B, MATRIX_SCOPE)]]
      MatB;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      MatB, MatrixBInput, 0, MATRIX_B_STRIDE, LAYOUT_ROW_MAJOR, 128);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        ACCUMULATOR_COMP_TYPE, M_DIM, N_DIM, USE_ACC, MATRIX_SCOPE)]]
      Result;
#if DO_ACCUMULATE
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        ACCUMULATOR_COMP_TYPE, M_DIM, N_DIM, USE_ACC, MATRIX_SCOPE)]]
      Accumulator;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Accumulator, AccumulatorInput, 0, ACCUMULATOR_STRIDE,
      LAYOUT_ROW_MAJOR, 128);
    __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(
      Result, MatA, MatB, Accumulator);
#else
    __builtin_LinAlg_MatrixMatrixMultiply(Result, MatA, MatB);
#endif

    __builtin_LinAlg_MatrixStoreToDescriptor(
      Result, Output, 0, ACCUMULATOR_STRIDE, LAYOUT_ROW_MAJOR, 128);
  }
)";

static std::optional<std::string>
buildMatrixMultiplyCompilerArgs(const MatrixMultiplyCase &Case,
                                MatrixScope Scope, UINT WaveSize,
                                UINT NumThreads) {
  if (Scope != MatrixScope::Wave && Scope != MatrixScope::ThreadGroup)
    return std::nullopt;
  if (WaveSize == 0 || NumThreads == 0)
    return std::nullopt;

  const MatrixParams MatrixA = makeMatrixArithmeticParams(
      Case.MatrixAType, Case.M, Case.K, MatrixUse::A, Scope, NumThreads);
  const MatrixParams MatrixB = makeMatrixArithmeticParams(
      Case.MatrixBType, Case.K, Case.N, MatrixUse::B, Scope, NumThreads);
  const MatrixParams Accumulator =
      makeMatrixArithmeticParams(Case.AccumulatorType, Case.M, Case.N,
                                 MatrixUse::Accumulator, Scope, NumThreads);

  std::stringstream SS;
  SS << "-HV 2021";
  SS << " -DMATRIX_A_COMP_TYPE=" << static_cast<int>(Case.MatrixAType);
  SS << " -DMATRIX_B_COMP_TYPE=" << static_cast<int>(Case.MatrixBType);
  SS << " -DACCUMULATOR_COMP_TYPE=" << static_cast<int>(Case.AccumulatorType);
  SS << " -DMATRIX_SCOPE=" << static_cast<int>(Scope);
  SS << " -DM_DIM=" << Case.M;
  SS << " -DK_DIM=" << Case.K;
  SS << " -DN_DIM=" << Case.N;
  SS << " -DMATRIX_A_STRIDE=" << MatrixA.strideBytes();
  SS << " -DMATRIX_B_STRIDE=" << MatrixB.strideBytes();
  SS << " -DACCUMULATOR_STRIDE=" << Accumulator.strideBytes();
  SS << " -DNUMTHREADS=" << NumThreads;
  SS << " -DFORCED_WAVE_SIZE=" << WaveSize;
  SS << " -DDO_ACCUMULATE=" << static_cast<int>(Case.accumulates());
  if (matrixArithmeticNeeds16BitTypes(Case.MatrixAType) ||
      matrixArithmeticNeeds16BitTypes(Case.MatrixBType) ||
      matrixArithmeticNeeds16BitTypes(Case.AccumulatorType))
    SS << " -enable-16bit-types";
  return SS.str();
}

static bool
verifyMatrixArithmeticMatrix(const void *Actual, size_t ActualSize,
                             const MatrixParams &Params,
                             const std::vector<int64_t> &ExpectedValues,
                             const std::wstring &PublicRule, bool Verbose) {
  const std::optional<std::vector<BYTE>> ExpectedBuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(Params, ExpectedValues);
  VERIFY_IS_TRUE(ExpectedBuffer.has_value());
  if (!ExpectedBuffer)
    return false;

  const cpu_oracle::MatrixBufferLayout Layout = {
      Params.Layout,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/Params.strideBytes(),
  };
  const std::optional<cpu_oracle::TypedMatrix> Expected =
      cpu_oracle::decodeMatrixBuffer(Params.CompType, Params.M, Params.N,
                                     Layout, ExpectedBuffer->data(),
                                     ExpectedBuffer->size());
  VERIFY_IS_TRUE(Expected.has_value());
  if (!Expected)
    return false;

  const cpu_oracle::MatrixResultOracle Oracle =
      cpu_oracle::exactResult(*Expected, PublicRule);
  return cpu_oracle::verifyMatrixBuffer(Actual, ActualSize, Layout, Oracle,
                                        Verbose);
}

static void runMatrixMultiplyCase(ID3D12Device *Device,
                                  dxc::SpecificDllLoader &DxcSupport,
                                  const MatrixMultiplyCase &Case,
                                  MatrixScope Scope, UINT WaveSize,
                                  UINT NumThreads, bool Verbose) {
  VERIFY_IS_TRUE(WaveSize != 0);
  VERIFY_IS_TRUE(NumThreads != 0);
  if (WaveSize == 0 || NumThreads == 0)
    return;

  const MatrixParams MatrixA = makeMatrixArithmeticParams(
      Case.MatrixAType, Case.M, Case.K, MatrixUse::A, Scope, NumThreads);
  const MatrixParams MatrixB = makeMatrixArithmeticParams(
      Case.MatrixBType, Case.K, Case.N, MatrixUse::B, Scope, NumThreads);
  const MatrixParams Accumulator =
      makeMatrixArithmeticParams(Case.AccumulatorType, Case.M, Case.N,
                                 MatrixUse::Accumulator, Scope, NumThreads);

  const std::optional<std::vector<BYTE>> MatrixABuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(MatrixA, Case.MatrixAValues);
  const std::optional<std::vector<BYTE>> MatrixBBuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(MatrixB, Case.MatrixBValues);
  const std::optional<std::vector<BYTE>> AccumulatorBuffer =
      Case.accumulates() ? cpu_oracle::encodeLogicalMatrixBuffer(
                               Accumulator, Case.AccumulatorValues)
                         : std::optional<std::vector<BYTE>>();
  const std::vector<int64_t> Expected = cpu_oracle::multiplyIntegerMatrices(
      Case.M, Case.K, Case.N, Case.MatrixAValues, Case.MatrixBValues,
      Case.accumulates() ? &Case.AccumulatorValues : nullptr);
  const std::optional<std::string> Args =
      buildMatrixMultiplyCompilerArgs(Case, Scope, WaveSize, NumThreads);
  VERIFY_IS_TRUE(MatrixABuffer.has_value());
  VERIFY_IS_TRUE(MatrixBBuffer.has_value());
  VERIFY_IS_TRUE(!Case.accumulates() || AccumulatorBuffer.has_value());
  VERIFY_IS_TRUE(Args.has_value());
  if (!MatrixABuffer || !MatrixBBuffer ||
      (Case.accumulates() && !AccumulatorBuffer) || !Args)
    return;

  const std::optional<std::vector<BYTE>> ExpectedBuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(Accumulator, Expected);
  VERIFY_IS_TRUE(ExpectedBuffer.has_value());
  if (!ExpectedBuffer)
    return;

  const char *RootSignature = Case.accumulates()
                                  ? "SRV(t0), SRV(t1), SRV(t2), UAV(u3)"
                                  : "SRV(t0), SRV(t1), UAV(u2)";
  compileShader(DxcSupport, MatrixMultiplyShader, "cs_6_10", *Args, Verbose);

  auto Op = createComputeOp(MatrixMultiplyShader, "cs_6_10", RootSignature,
                            Args->c_str());
  addSRVBuffer(Op.get(), "MatrixAInput", MatrixABuffer->size(), "byname");
  addSRVBuffer(Op.get(), "MatrixBInput", MatrixBBuffer->size(), "byname");
  if (Case.accumulates())
    addSRVBuffer(Op.get(), "AccumulatorInput", AccumulatorBuffer->size(),
                 "byname");
  addUAVBuffer(Op.get(), "Output", ExpectedBuffer->size(), true);
  addRootView(Op.get(), 0, "MatrixAInput");
  addRootView(Op.get(), 1, "MatrixBInput");
  if (Case.accumulates()) {
    addRootView(Op.get(), 2, "AccumulatorInput");
    addRootView(Op.get(), 3, "Output");
  } else {
    addRootView(Op.get(), 2, "Output");
  }

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [MatrixABuffer, MatrixBBuffer, AccumulatorBuffer,
       &Case](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
        const std::vector<BYTE> *Source = nullptr;
        if (_stricmp(Name, "MatrixAInput") == 0)
          Source = &*MatrixABuffer;
        else if (_stricmp(Name, "MatrixBInput") == 0)
          Source = &*MatrixBBuffer;
        else if (Case.accumulates() && _stricmp(Name, "AccumulatorInput") == 0)
          Source = &*AccumulatorBuffer;
        if (!Source)
          return;
        VERIFY_IS_TRUE(Data.size() == Source->size());
        if (Data.size() != Source->size())
          return;
        std::memcpy(Data.data(), Source->data(), Data.size());
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(verifyMatrixArithmeticMatrix(OutData.data(), OutData.size(),
                                              Accumulator, Expected,
                                              Case.PublicRule, Verbose));
}

static void runWaveMultiplyCase(ID3D12Device *Device,
                                dxc::SpecificDllLoader &DxcSupport,
                                const MatrixMultiplyCase &Case,
                                LPCWSTR CaseName, bool Verbose) {
  VERIFY_IS_TRUE(isMatrixMultiplyCaseValid(Case));
  if (!isMatrixMultiplyCaseValid(Case))
    return;

  bool Supported = false;
  UINT SelectedWaveSize = 0;
  const HRESULT QueryResult = selectWaveArithmeticMultiplyWaveSize(
      Device, Case, CaseName, Supported, SelectedWaveSize);
  if (!applyApplicability(
          linalg_test::classifyApplicability(
              QueryResult, Supported,
              linalg_test::CapabilityRequirement::CapabilityGated),
          CaseName))
    return;
  VERIFY_IS_TRUE(SelectedWaveSize != 0);
  if (SelectedWaveSize == 0)
    return;

  runMatrixMultiplyCase(Device, DxcSupport, Case, MatrixScope::Wave,
                        SelectedWaveSize, SelectedWaveSize, Verbose);
}

static MatrixMultiplyCase
makeRectangularF16WaveMultiplyCase(ComponentType AccumulatorType,
                                   MatrixMultiplyOperation Operation) {
  MatrixMultiplyCase Case = {};
  Case.MatrixAType = ComponentType::F16;
  Case.MatrixBType = ComponentType::F16;
  Case.AccumulatorType = AccumulatorType;
  Case.M = 8;
  Case.K = 32;
  Case.N = 16;
  Case.Operation = Operation;
  Case.MatrixAValues = makeMatrixArithmeticPattern(Case.M, Case.K, 3, 2, 5, 2);
  Case.MatrixBValues = makeMatrixArithmeticPattern(Case.K, Case.N, 1, 3, 7, 3);
  if (Case.accumulates()) {
    Case.AccumulatorValues =
        makeMatrixArithmeticPattern(Case.M, Case.N, 2, 1, 5, 2);
    Case.PublicRule =
        L"Exact non-uniform F16 product plus an independent F32 accumulator";
  } else if (AccumulatorType == ComponentType::F32) {
    Case.PublicRule =
        L"Exact non-uniform F16 matrix product stored in an F32 accumulator";
  } else {
    Case.PublicRule = L"Exact non-uniform rectangular F16 matrix product";
  }
  return Case;
}

void DxilConf_SM610_LinAlg::MatMatMul_Wave_8x32x16_F16_NonUniform() {
  const MatrixMultiplyCase Case = makeRectangularF16WaveMultiplyCase(
      ComponentType::F16, MatrixMultiplyOperation::Multiply);
  runWaveMultiplyCase(D3DDevice, DxcSupport, Case,
                      L"MatMatMul_Wave_8x32x16_F16_NonUniform", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatMatMul_Wave_8x32x16_F16_ToF32() {
  const MatrixMultiplyCase Case = makeRectangularF16WaveMultiplyCase(
      ComponentType::F32, MatrixMultiplyOperation::Multiply);
  runWaveMultiplyCase(D3DDevice, DxcSupport, Case,
                      L"MatMatMul_Wave_8x32x16_F16_ToF32", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatMatMulAccum_Wave_8x32x16_F16_ToF32_NonUniform() {
  const MatrixMultiplyCase Case = makeRectangularF16WaveMultiplyCase(
      ComponentType::F32, MatrixMultiplyOperation::MultiplyAccumulate);
  runWaveMultiplyCase(D3DDevice, DxcSupport, Case,
                      L"MatMatMulAccum_Wave_8x32x16_F16_ToF32_NonUniform",
                      VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatMatMul_Wave_16x16x16_I32() {
  MatrixMultiplyCase Case = {};
  Case.MatrixAType = ComponentType::I32;
  Case.MatrixBType = ComponentType::I32;
  Case.AccumulatorType = ComponentType::I32;
  Case.M = 16;
  Case.K = 16;
  Case.N = 16;
  Case.MatrixAValues = makeMatrixArithmeticPattern(Case.M, Case.K, 3, 2, 5, 2);
  Case.MatrixBValues = makeMatrixArithmeticPattern(Case.K, Case.N, 1, 3, 7, 3);
  Case.PublicRule = L"Exact non-uniform I32 matrix product";
  runWaveMultiplyCase(D3DDevice, DxcSupport, Case,
                      L"MatMatMul_Wave_16x16x16_I32", VerboseLogging);
}

static void runThreadGroupMultiplyCase(ID3D12Device *Device,
                                       dxc::SpecificDllLoader &DxcSupport,
                                       const MatrixMultiplyCase &Case,
                                       LPCWSTR CaseName, bool Verbose) {
  VERIFY_IS_TRUE(isMatrixMultiplyCaseValid(Case));
  if (!isMatrixMultiplyCaseValid(Case))
    return;

  UINT SelectedWaveSize = 0;
  UINT SelectedThreadGroupSize = 0;
  if (!threadGroupMatMulApplicable(Device, Case, CaseName, SelectedWaveSize,
                                   SelectedThreadGroupSize))
    return;

  runMatrixMultiplyCase(Device, DxcSupport, Case, MatrixScope::ThreadGroup,
                        SelectedWaveSize, SelectedThreadGroupSize, Verbose);
}

static MatrixMultiplyCase
makeRectangularF16ThreadGroupMultiplyCase(ComponentType AccumulatorType,
                                          MatrixMultiplyOperation Operation) {
  MatrixMultiplyCase Case = {};
  Case.MatrixAType = ComponentType::F16;
  Case.MatrixBType = ComponentType::F16;
  Case.AccumulatorType = AccumulatorType;
  Case.M = 8;
  Case.K = 16;
  Case.N = 8;
  Case.Operation = Operation;
  Case.MatrixAValues = makeMatrixArithmeticPattern(Case.M, Case.K, 3, 2, 5, 2);
  Case.MatrixBValues = makeMatrixArithmeticPattern(Case.K, Case.N, 1, 3, 7, 3);
  if (Case.accumulates()) {
    Case.AccumulatorValues =
        makeMatrixArithmeticPattern(Case.M, Case.N, 2, 1, 5, 2);
    Case.PublicRule =
        L"Exact non-uniform ThreadGroup F16 product plus an independent F32 "
        L"accumulator";
  } else if (AccumulatorType == ComponentType::F32) {
    Case.PublicRule =
        L"Exact non-uniform ThreadGroup F16 matrix product stored in an F32 "
        L"accumulator";
  } else {
    Case.PublicRule =
        L"Exact non-uniform ThreadGroup F16 product with rectangular inputs";
  }
  return Case;
}

void DxilConf_SM610_LinAlg::MatMatMul_ThreadGroup_8x16x8_F16_NonUniform() {
  const MatrixMultiplyCase Case = makeRectangularF16ThreadGroupMultiplyCase(
      ComponentType::F16, MatrixMultiplyOperation::Multiply);
  runThreadGroupMultiplyCase(D3DDevice, DxcSupport, Case,
                             L"MatMatMul_ThreadGroup_8x16x8_F16_NonUniform",
                             VerboseLogging);
}

void DxilConf_SM610_LinAlg::
    MatMatMulAccum_ThreadGroup_8x16x8_F16_ToF32_NonUniform() {
  const MatrixMultiplyCase Case = makeRectangularF16ThreadGroupMultiplyCase(
      ComponentType::F32, MatrixMultiplyOperation::MultiplyAccumulate);
  runThreadGroupMultiplyCase(
      D3DDevice, DxcSupport, Case,
      L"MatMatMulAccum_ThreadGroup_8x16x8_F16_ToF32_NonUniform",
      VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatMatMul_ThreadGroup_8x8x8_I32() {
  MatrixMultiplyCase Case = {};
  Case.MatrixAType = ComponentType::I32;
  Case.MatrixBType = ComponentType::I32;
  Case.AccumulatorType = ComponentType::I32;
  Case.M = 8;
  Case.K = 8;
  Case.N = 8;
  Case.MatrixAValues = makeMatrixArithmeticPattern(Case.M, Case.K, 3, 2, 5, 2);
  Case.MatrixBValues = makeMatrixArithmeticPattern(Case.K, Case.N, 1, 3, 7, 3);
  Case.PublicRule = L"Exact non-uniform ThreadGroup I32 matrix product";
  runThreadGroupMultiplyCase(D3DDevice, DxcSupport, Case,
                             L"MatMatMul_ThreadGroup_8x8x8_I32",
                             VerboseLogging);
}

static const char WaveAccumulateBUseShader[] = R"(
  #define USE_ACC 2

  ByteAddressBuffer AccumulatorInput : register(t0);
  ByteAddressBuffer RHSInput : register(t1);
  RWByteAddressBuffer Output : register(u2);

  [WaveSize(FORCED_WAVE_SIZE)]
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      Accumulator;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Accumulator, AccumulatorInput, 0, STRIDE, LAYOUT, 128);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      RHS;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      RHS, RHSInput, 0, STRIDE, LAYOUT, 128);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      Result;
    __builtin_LinAlg_MatrixAccumulate(Result, Accumulator, RHS);
    __builtin_LinAlg_MatrixStoreToDescriptor(
      Result, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

void DxilConf_SM610_LinAlg::MatAccum_Wave_8x32_F16_BUse_NonUniform() {
  MatrixParams Params = makeMatrixArithmeticParams(
      ComponentType::F16, /*M=*/8, /*N=*/32, MatrixUse::B, MatrixScope::Wave,
      /*NumThreads=*/128);

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {MatrixUse::Accumulator, MatrixUse::B},
          L"MatAccum_Wave_8x32_F16_BUse_NonUniform", SelectedWaveSize))
    return;
  Params.NumThreads = static_cast<int>(SelectedWaveSize);

  const std::vector<int64_t> AccumulatorValues =
      makeMatrixArithmeticPattern(Params.M, Params.N, 2, 1, 7, 3);
  const std::vector<int64_t> RHSValues =
      makeMatrixArithmeticPattern(Params.M, Params.N, 1, 3, 5, 2);
  std::vector<int64_t> ExpectedValues(AccumulatorValues.size());
  for (size_t I = 0; I < ExpectedValues.size(); ++I)
    ExpectedValues[I] = AccumulatorValues[I] + RHSValues[I];

  const MatrixParams AccumulatorParams = makeMatrixArithmeticParams(
      ComponentType::F16, Params.M, Params.N, MatrixUse::Accumulator,
      MatrixScope::Wave, SelectedWaveSize);
  const std::optional<std::vector<BYTE>> AccumulatorBuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(AccumulatorParams,
                                            AccumulatorValues);
  const std::optional<std::vector<BYTE>> RHSBuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(Params, RHSValues);
  const std::optional<std::vector<BYTE>> ExpectedBuffer =
      cpu_oracle::encodeLogicalMatrixBuffer(AccumulatorParams, ExpectedValues);
  VERIFY_IS_TRUE(AccumulatorBuffer.has_value());
  VERIFY_IS_TRUE(RHSBuffer.has_value());
  VERIFY_IS_TRUE(ExpectedBuffer.has_value());
  if (!AccumulatorBuffer || !RHSBuffer || !ExpectedBuffer)
    return;

  std::stringstream ExtraDefs;
  ExtraDefs << " -DFORCED_WAVE_SIZE=" << SelectedWaveSize;
  const std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());
  compileShader(DxcSupport, WaveAccumulateBUseShader, "cs_6_10", Args,
                VerboseLogging);

  auto Op = createComputeOp(WaveAccumulateBUseShader, "cs_6_10",
                            "SRV(t0), SRV(t1), UAV(u2)", Args.c_str());
  addSRVBuffer(Op.get(), "AccumulatorInput", AccumulatorBuffer->size(),
               "byname");
  addSRVBuffer(Op.get(), "RHSInput", RHSBuffer->size(), "byname");
  addUAVBuffer(Op.get(), "Output", ExpectedBuffer->size(), true);
  addRootView(Op.get(), 0, "AccumulatorInput");
  addRootView(Op.get(), 1, "RHSInput");
  addRootView(Op.get(), 2, "Output");

  auto Result =
      runShaderOp(D3DDevice, DxcSupport, std::move(Op),
                  [AccumulatorBuffer, RHSBuffer](
                      LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
                    const std::vector<BYTE> *Source = nullptr;
                    if (_stricmp(Name, "AccumulatorInput") == 0)
                      Source = &*AccumulatorBuffer;
                    else if (_stricmp(Name, "RHSInput") == 0)
                      Source = &*RHSBuffer;
                    if (!Source)
                      return;
                    VERIFY_IS_TRUE(Data.size() == Source->size());
                    if (Data.size() != Source->size())
                      return;
                    std::memcpy(Data.data(), Source->data(), Data.size());
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(verifyMatrixArithmeticMatrix(
      OutData.data(), OutData.size(), AccumulatorParams, ExpectedValues,
      L"Exact non-uniform F16 accumulator plus a B-use F16 matrix",
      VerboseLogging));
}

static const char MatVecMulShader[] = R"(
  #define USE_A 0
  #define SCOPE_THREAD 0

  ByteAddressBuffer Input : register(t0);
  RWByteAddressBuffer Output : register(u1);

  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_A, SCOPE_THREAD)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);

    vector<ELEM_TYPE, N_DIM> InVec;
    for (uint I = 0; I < N_DIM; ++I) {
      InVec[I] = Input.Load<ELEM_TYPE>(I * ELEM_SIZE);
    }

    vector<ELEM_TYPE, M_DIM> OutVec;
    __builtin_LinAlg_MatrixVectorMultiply(
      OutVec, Mat, OUTPUT_SIGNED, InVec, IN_INTERP);

    for (uint I = 0; I < M_DIM; ++I) {
      Output.Store<ELEM_TYPE>(I * ELEM_SIZE, OutVec[I]);
    }
  }
)";

// Thread-scope vector-matrix multiplication is described entirely by its type
// combination. D3D12LinearAlgebraRuntimeFeatureSupport.md scopes
// MatrixConstruction to "wave-scope and group-scope matrices" and states there
// is no requirement around thread-scope vector-matrix multiplication
// dimensions, which is why neither the support struct nor the enumeration
// entry for this operation carries a shape. Applicability therefore rests on
// ThreadVectorMatrixMultiply alone.
static HRESULT queryMatVecMulSupport(ID3D12Device *Device,
                                     const MatrixParams &Params,
                                     ComponentType InputInterp, bool HasBias,
                                     bool &TierSupported, bool &Supported) {
  TierSupported = false;
  Supported = false;
  if (!Device ||
      !linalg_test::isLegalScope(
          linalg_abi::
              D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY,
          Params.Scope))
    return E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> MatrixType =
      toCapabilityDataType(Params.CompType);
  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> VectorType =
      toCapabilityDataType(InputInterp);
  if (!MatrixType.has_value() || !VectorType.has_value())
    return E_INVALIDARG;

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR))
    return HR;
  TierSupported = Tier.supported();
  if (!TierSupported)
    return S_OK;

  // The shaders declare the bias and result vectors with the matrix component
  // type. A multiply with no bias is expressed as DATATYPE_NONE, which Tier 1
  // requires alongside a bias type matching the result type.
  const linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE BiasType =
      HasBias ? *MatrixType : linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE;

  linalg_test::ThreadVectorMatrixMultiplySupport Multiply;
  HR = linalg_test::queryThreadVectorMatrixMultiply(
      Device, {*VectorType, *MatrixType, BiasType, *MatrixType}, Multiply);
  if (FAILED(HR))
    return HR;
  if (!Multiply.supported()) {
    hlsl_test::LogCommentFmt(
        L"ThreadVectorMatrixMultiply reports vector=%u matrix=%u bias=%u "
        L"result=%u is unsupported",
        static_cast<UINT>(*VectorType), static_cast<UINT>(*MatrixType),
        static_cast<UINT>(BiasType), static_cast<UINT>(*MatrixType));
    return S_OK;
  }

  Supported = true;
  return S_OK;
}

static bool matVecMulApplicable(ID3D12Device *Device,
                                const MatrixParams &Params,
                                ComponentType InputInterp, bool HasBias,
                                linalg_test::CapabilityRequirement Requirement,
                                LPCWSTR CaseName) {
  bool TierSupported = false;
  bool Supported = false;
  const HRESULT QueryResult = queryMatVecMulSupport(
      Device, Params, InputInterp, HasBias, TierSupported, Supported);

  // A device that does not implement linear algebra at all is outside the
  // Tier 1 requirements, so it skips rather than failing even where the
  // configuration is mandatory.
  const linalg_test::CapabilityRequirement Effective =
      SUCCEEDED(QueryResult) && !TierSupported
          ? linalg_test::CapabilityRequirement::CapabilityGated
          : Requirement;

  return applyApplicability(
      linalg_test::classifyApplicability(QueryResult, Supported, Effective),
      CaseName);
}

static void runMatVecMul(ID3D12Device *Device,
                         dxc::SpecificDllLoader &DxcSupport,
                         const MatrixParams &Params, bool Verbose,
                         int FillValue, bool OutputSigned,
                         ComponentType InputInterp) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DOUTPUT_SIGNED=" << OutputSigned;
  ExtraDefs << " -DIN_INTERP=" << static_cast<int>(InputInterp);

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, MatVecMulShader, "cs_6_10", Args, Verbose);

  auto Expected =
      makeExpectedVec(Params.CompType, Params.M,
                      static_cast<float>(FillValue * FillValue * Params.N),
                      /*Increment=*/false);

  auto Op = createComputeOp(MatVecMulShader, "cs_6_10", "SRV(t0), UAV(u1)",
                            Args.c_str());
  addSRVBuffer(Op.get(), "Input", BufferSize, "byname");
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [NumElements, Params, FillValue](LPCSTR Name, std::vector<BYTE> &Data,
                                       st::ShaderOp *) {
        VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType, NumElements,
                                       /*StartingVal=*/FillValue,
                                       /*Increment=*/false),
                       "Saw unsupported component type");
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, Params.M, Verbose));
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Scope = MatrixScope::Thread;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 1;
  Params.Enable16Bit = true;

  // Tier 1 requires Fp16 vector x Fp16 matrix -> Fp16, and requires a bias
  // matching the result type as well as no bias at all, so a Tier 1 device
  // reporting this unsupported is a conformance failure rather than a skip.
  if (!matVecMulApplicable(D3DDevice, Params, ComponentType::F16,
                           /*HasBias=*/false,
                           linalg_test::CapabilityRequirement::Mandatory,
                           L"MatVecMul_Thread_16x16_F16"))
    return;

  runMatVecMul(D3DDevice, DxcSupport, Params, VerboseLogging,
               /*FillValue=*/2, /*OutputSigned=*/true, ComponentType::F16);
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_4x8_F32() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Scope = MatrixScope::Thread;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 1;

  // Fp32 vector x Fp32 matrix -> Fp32 is absent from the Tier 1 table, so it
  // is optional and a device reporting it unsupported skips.
  if (!matVecMulApplicable(D3DDevice, Params, ComponentType::F32,
                           /*HasBias=*/false,
                           linalg_test::CapabilityRequirement::CapabilityGated,
                           L"MatVecMul_Thread_4x8_F32"))
    return;

  runMatVecMul(D3DDevice, DxcSupport, Params, VerboseLogging,
               /*FillValue=*/2, /*OutputSigned=*/true, ComponentType::F32);
}

static const char MatVecMulAddShader[] = R"(
  #define USE_A 0
  #define SCOPE_THREAD 0

  ByteAddressBuffer Input : register(t0);
  RWByteAddressBuffer Output : register(u1);

  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_A, SCOPE_THREAD)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromDescriptor(
      Mat, Input, 0, STRIDE, LAYOUT, 128);

    vector<ELEM_TYPE, N_DIM> InVec;
    for (uint I = 0; I < N_DIM; ++I) {
      InVec[I] = Input.Load<ELEM_TYPE>(I * ELEM_SIZE);
    }

    vector<ELEM_TYPE, M_DIM> BiasVec;
    for (uint I = 0; I < M_DIM; ++I) {
      BiasVec[I] = Input.Load<ELEM_TYPE>(I * ELEM_SIZE);
    }

    vector<ELEM_TYPE, M_DIM> OutVec;
    __builtin_LinAlg_MatrixVectorMultiplyAdd(
      OutVec, Mat, OUTPUT_SIGNED, InVec, IN_INTERP, BiasVec);

    for (uint I = 0; I < M_DIM; ++I) {
      Output.Store<ELEM_TYPE>(I * ELEM_SIZE, OutVec[I]);
    }
  }
)";

static void runMatVecMulAdd(ID3D12Device *Device,
                            dxc::SpecificDllLoader &DxcSupport,
                            const MatrixParams &Params, bool Verbose,
                            int FillValue, bool OutputSigned,
                            ComponentType InputInterp) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DOUTPUT_SIGNED=" << OutputSigned;
  ExtraDefs << " -DIN_INTERP=" << static_cast<int>(InputInterp);

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, MatVecMulAddShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedVec(
      Params.CompType, Params.M,
      static_cast<float>(FillValue * FillValue * Params.N + FillValue),
      /*Increment=*/false);

  auto Op = createComputeOp(MatVecMulAddShader, "cs_6_10", "SRV(t0), UAV(u1)",
                            Args.c_str());
  addSRVBuffer(Op.get(), "Input", BufferSize, "byname");
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [NumElements, Params, FillValue](LPCSTR Name, std::vector<BYTE> &Data,
                                       st::ShaderOp *) {
        VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType, NumElements,
                                       /*StartingVal=*/FillValue,
                                       /*Increment=*/false),
                       "Saw unsupported component type");
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, Params.M, Verbose));
}

void DxilConf_SM610_LinAlg::MatVecMulAdd_Thread_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Scope = MatrixScope::Thread;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 1;
  Params.Enable16Bit = true;

  // Required by Tier 1: Fp16 throughout, with a bias matching the result type.
  if (!matVecMulApplicable(D3DDevice, Params, ComponentType::F16,
                           /*HasBias=*/true,
                           linalg_test::CapabilityRequirement::Mandatory,
                           L"MatVecMulAdd_Thread_16x16_F16"))
    return;

  runMatVecMulAdd(D3DDevice, DxcSupport, Params, VerboseLogging,
                  /*FillValue=*/2, /*OutputSigned=*/true, ComponentType::F16);
}

void DxilConf_SM610_LinAlg::MatVecMulAdd_Thread_4x8_F32() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Scope = MatrixScope::Thread;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 1;

  // Optional: see MatVecMul_Thread_4x8_F32.
  if (!matVecMulApplicable(D3DDevice, Params, ComponentType::F32,
                           /*HasBias=*/true,
                           linalg_test::CapabilityRequirement::CapabilityGated,
                           L"MatVecMulAdd_Thread_4x8_F32"))
    return;

  runMatVecMulAdd(D3DDevice, DxcSupport, Params, VerboseLogging,
                  /*FillValue=*/2, /*OutputSigned=*/true, ComponentType::F32);
}

// Map a DXIL ComponentType to the D3D12 linear-algebra datatype used by the
// host-side matrix conversion API.
#if defined(DIRECT3D_LINEAR_ALGEBRA)
static D3D12_LINEAR_ALGEBRA_DATATYPE toLinAlgDataType(ComponentType CT) {
  switch (CT) {
  case ComponentType::F16:
    return D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16;
  case ComponentType::F32:
    return D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32;
  case ComponentType::I16:
    return D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16;
  case ComponentType::U16:
    return D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16;
  case ComponentType::I32:
    return D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32;
  case ComponentType::U32:
    return D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32;
  default:
    VERIFY_IS_TRUE(false, "Unsupported component type for linalg conversion");
    return D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16;
  }
}

static const char OuterProductShader[] = R"(
  #define SCOPE_THREAD 0

  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);

  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    vector<ELEM_TYPE, M_DIM> VecA;
    for (uint I = 0; I < M_DIM; ++I) {
      VecA[I] = Input.Load<ELEM_TYPE>(I * ELEM_SIZE);
    }

    uint EndVecA = M_DIM * ELEM_SIZE;

    vector<ELEM_TYPE, N_DIM> VecB;
    for (uint I = 0; I < N_DIM; ++I) {
      VecB[I] = Input.Load<ELEM_TYPE>(EndVecA + I * ELEM_SIZE);
    }

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE_THREAD)]]
      Mat;
    __builtin_LinAlg_MatrixOuterProduct(Mat, VecA, VecB);

    // Outer product accumulators are stored in the OuterProductOptimal layout
    // Matching the dx::linalg header's thread-scoped
    // InterlockedAccumulate. The alignment argument must be a non-zero
    // multiple of 128; the matrix starts at offset 0 in a buffer D3D12 aligns
    // far more strongly than that.
    __builtin_LinAlg_MatrixAccumulateToDescriptor(
      Mat, Output, 0, STRIDE, LAYOUT, 128);
  }
)";

static void runOuterProduct(ID3D12Device *Device,
                            dxc::SpecificDllLoader &DxcSupport,
                            const MatrixParams &Params, bool Verbose) {
  VERIFY_IS_TRUE(
      Params.Layout == MatrixLayout::OuterProductOptimal,
      "Outer product must output its matrix in OuterProductOptimal layout");
  VERIFY_IS_TRUE(Params.Use == MatrixUse::Accumulator,
                 "Outer product must output an accumulator matrix");
  const size_t NumVecElements = Params.M + Params.N;
  const size_t InBuffSize = NumVecElements * elementSize(Params.CompType);
  const size_t NumMatElements = Params.totalElements();
  const D3D12_LINEAR_ALGEBRA_DATATYPE DataType =
      toLinAlgDataType(Params.CompType);

  const UINT OutBufferSize = getLinAlgMatrixByteSize(
      Device, Params.M, Params.N, DataType,
      D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL, /*Stride=*/0);

  const UINT RowMajorStride =
      static_cast<UINT>(Params.N * elementSize(Params.CompType));
  const UINT RowMajorSize = getLinAlgMatrixByteSize(
      Device, Params.M, Params.N, DataType,
      D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT_ROW_MAJOR, RowMajorStride);

  std::string Args = buildCompilerArgs(Params);

  compileShader(DxcSupport, OuterProductShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N, 4,
                                  /*Increment=*/false);

  auto Op = createComputeOp(OuterProductShader, "cs_6_10", "UAV(u0), UAV(u1)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Input", InBuffSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", OutBufferSize, /*ReadBack=*/false);
  addUAVBuffer(Op.get(), "OutputRowMajor", RowMajorSize, /*ReadBack=*/true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [NumVecElements, Params](LPCSTR Name, std::vector<BYTE> &Data,
                               st::ShaderOp *) {
        VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType,
                                       NumVecElements,
                                       /*StartingVal=*/2, /*Increment=*/false),
                       "Saw unsupported component type");
      },
      [OutBufferSize, RowMajorSize, RowMajorStride, DataType,
       Params](ID3D12GraphicsCommandList *List, st::ShaderOpTest *Test) {
        ID3D12Resource *OptimalBuffer = nullptr;
        ID3D12Resource *RowMajorBuffer = nullptr;
        Test->GetResource("Output", &OptimalBuffer);
        Test->GetResource("OutputRowMajor", &RowMajorBuffer);
        recordLinAlgMatrixConversion(
            List, OptimalBuffer, OutBufferSize, RowMajorBuffer, RowMajorSize,
            Params.M, Params.N, DataType,
            D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL,
            /*SrcStride=*/0, D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT_ROW_MAJOR,
            RowMajorStride);
      });

  MappedData OutData;
  Result->Test->GetReadBackData("OutputRowMajor", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumMatElements, Verbose));
}
#endif // defined(DIRECT3D_LINEAR_ALGEBRA)

void DxilConf_SM610_LinAlg::OuterProduct_Thread_16x16_F16() {
#if defined(DIRECT3D_LINEAR_ALGEBRA)
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Thread;
  Params.Layout = MatrixLayout::OuterProductOptimal;
  Params.NumThreads = 1;
  Params.Enable16Bit = true;

  // Tier 1 requires no outer product formats at all, so this is gated.
  if (!outerProductApplicable(D3DDevice, Params.CompType, Params.CompType,
                              L"OuterProduct_Thread_16x16_F16"))
    return;

  // The shader accumulates its result into an RWByteAddressBuffer, which is
  // reported independently of the outer product itself. A device may produce
  // the outer product yet not support accumulating this component type into a
  // buffer, so the destination has to be gated too or that device fails to
  // create the pipeline instead of skipping.
  if (!accumulateStoreApplicable(
          D3DDevice, Params.CompType,
          linalg_test::AtomicDestination::RWByteAddressBuffer,
          L"OuterProduct_Thread_16x16_F16"))
    return;

  runOuterProduct(D3DDevice, DxcSupport, Params, VerboseLogging);
#else
#ifdef _HLK_CONF
  // HLK forbids skipping, so treat the missing linear-algebra matrix-conversion
  // API as a failure rather than emitting a (compiled-out) skip.
  hlsl_test::LogErrorFmt(L"OuterProduct_Thread_16x16_F16 requires the "
                         L"linear-algebra matrix-conversion API "
                         L"(DIRECT3D_LINEAR_ALGEBRA), which this build lacks");
#else
  WEX::Logging::Log::Comment(
      L"Skipping OuterProduct_Thread_16x16_F16: built against a D3D12 SDK "
      L"without the linear-algebra matrix-conversion API "
      L"(DIRECT3D_LINEAR_ALGEBRA undefined); the host-side conversion helpers "
      L"are compiled out.");
  WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
#endif // _HLK_CONF
#endif // defined(DIRECT3D_LINEAR_ALGEBRA)
}

static const char QueryAccumLayoutValueShader[] = R"(
  RWByteAddressBuffer Output : register(u0);

  [numthreads(1, 1, 1)]
  void main() {
    uint Layout = __builtin_LinAlg_MatrixQueryAccumulatorLayout();
    Output.Store<uint>(0, Layout);
  }
)";

static void runQueryAccumLayoutValue(ID3D12Device *Device,
                                     dxc::SpecificDllLoader &DxcSupport,
                                     bool Verbose) {
  const std::string Args = "-HV 2021";
  const size_t BufferSize = sizeof(uint32_t);

  compileShader(DxcSupport, QueryAccumLayoutValueShader, "cs_6_10", Args,
                Verbose);

  auto Op = createComputeOp(QueryAccumLayoutValueShader, "cs_6_10", "UAV(u0)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true, "byname");
  addRootView(Op.get(), 0, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
                    if (_stricmp(Name, "Output") == 0)
                      cpu_oracle::fillPoison(Data.data(), Data.size());
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(OutData.size() == BufferSize);
  if (OutData.size() != BufferSize)
    return;

  uint32_t Layout;
  std::memcpy(&Layout, OutData.data(), sizeof(Layout));
  VERIFY_IS_TRUE(Layout == static_cast<uint32_t>(MatrixUse::A) ||
                 Layout == static_cast<uint32_t>(MatrixUse::B));
  if (Verbose)
    hlsl_test::LogCommentFmt(L"AccumulatorLayout = %u", Layout);
}

static const char QueryAccumLayoutShader[] = R"(
  #define USE_A 0
  #define USE_B 1
  #define USE_ACC 2

  RWByteAddressBuffer Output : register(u0);

  [WaveSize(FORCED_WAVE_SIZE)]
  [numthreads(NUMTHREADS, 1, 1)]
  void main() {
    uint Layout = __builtin_LinAlg_MatrixQueryAccumulatorLayout();

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      Accumulator;
    __builtin_LinAlg_FillMatrix(Accumulator, 2.0);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_A, SCOPE)]]
      MatrixA;
    __builtin_LinAlg_FillMatrix(MatrixA, 3.0);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE_B, SCOPE)]]
      MatrixB;
    __builtin_LinAlg_FillMatrix(MatrixB, 7.0);

    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(
        COMP_TYPE, M_DIM, N_DIM, USE_ACC, SCOPE)]]
      Result;
    if (Layout == USE_A)
      __builtin_LinAlg_MatrixAccumulate(Result, Accumulator, MatrixA);
    else
      __builtin_LinAlg_MatrixAccumulate(Result, Accumulator, MatrixB);

    __builtin_LinAlg_MatrixStoreToDescriptor(
      Result, Output, 0, STRIDE, LAYOUT, 128);

    // Only lane zero publishes the queried layout, avoiding a UAV write race.
    if (WaveGetLaneIndex() == 0)
      Output.Store<uint>(LAYOUT_OFFSET, Layout);
  }
)";

static void runQueryAccumLayout(ID3D12Device *Device,
                                dxc::SpecificDllLoader &DxcSupport,
                                MatrixParams Params, UINT SelectedWaveSize,
                                bool Verbose) {
  Params.NumThreads = static_cast<int>(SelectedWaveSize);
  const size_t MatrixBytes = Params.totalBytes();
  const size_t BufferSize = MatrixBytes + sizeof(uint32_t);

  std::stringstream ExtraDefs;
  ExtraDefs << " -DFORCED_WAVE_SIZE=" << SelectedWaveSize;
  ExtraDefs << " -DLAYOUT_OFFSET=" << MatrixBytes;
  const std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, QueryAccumLayoutShader, "cs_6_10", Args, Verbose);

  auto Op = createComputeOp(QueryAccumLayoutShader, "cs_6_10", "UAV(u0)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true, "byname");
  addRootView(Op.get(), 0, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
                    if (_stricmp(Name, "Output") == 0)
                      cpu_oracle::fillPoison(Data.data(), Data.size());
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(OutData.size() == BufferSize);
  if (OutData.size() != BufferSize)
    return;

  uint32_t Layout;
  std::memcpy(&Layout, static_cast<const BYTE *>(OutData.data()) + MatrixBytes,
              sizeof(Layout));
  VERIFY_IS_TRUE(Layout == static_cast<uint32_t>(MatrixUse::A) ||
                 Layout == static_cast<uint32_t>(MatrixUse::B));
  if (Layout != static_cast<uint32_t>(MatrixUse::A) &&
      Layout != static_cast<uint32_t>(MatrixUse::B))
    return;

  const int64_t ExpectedValue =
      Layout == static_cast<uint32_t>(MatrixUse::A) ? 5 : 9;
  const std::vector<int64_t> ExpectedValues(Params.totalElements(),
                                            ExpectedValue);
  VERIFY_IS_TRUE(verifyMatrixArithmeticMatrix(
      OutData.data(), OutData.size(), Params, ExpectedValues,
      L"Accumulator layout selects the matching A-use or B-use accumulate "
      L"path",
      Verbose));
  if (Verbose)
    hlsl_test::LogCommentFmt(L"AccumulatorLayout = %u", Layout);
}

void DxilConf_SM610_LinAlg::QueryAccumLayout() {
  if (!linAlgTierApplicable(D3DDevice, L"QueryAccumLayout"))
    return;

  MatrixParams Params = makeMatrixArithmeticParams(
      ComponentType::F16, /*M=*/4, /*N=*/8, MatrixUse::Accumulator,
      MatrixScope::Wave, /*NumThreads=*/128);

  bool Supported = false;
  UINT SelectedWaveSize = 0;
  const HRESULT QueryResult = selectMatrixConstructionWaveSize(
      D3DDevice, Params, {MatrixUse::Accumulator, MatrixUse::A, MatrixUse::B},
      Supported, SelectedWaveSize);
  VERIFY_SUCCEEDED(QueryResult);
  if (FAILED(QueryResult))
    return;

  if (Supported) {
    VERIFY_IS_TRUE(SelectedWaveSize != 0);
    if (SelectedWaveSize == 0)
      return;
    runQueryAccumLayout(D3DDevice, DxcSupport, Params, SelectedWaveSize,
                        VerboseLogging);
  } else {
    // The query itself is matrix-free, so preserve its tier-only coverage when
    // this observable F16 tile is unavailable.
    runQueryAccumLayoutValue(D3DDevice, DxcSupport, VerboseLogging);
  }
}

static const char LoadMemoryShader[] = R"(
  RWByteAddressBuffer Input : register(u0);
  RWByteAddressBuffer Output : register(u1);
  groupshared ELEM_TYPE GsData[M_DIM * N_DIM];

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    for (uint Index = threadID; Index < M_DIM * N_DIM;
         Index += NUMTHREADS) {
      GsData[Index] = Input.Load<ELEM_TYPE>(Index * ELEM_SIZE);
    }

    GroupMemoryBarrierWithGroupSync();

    if (GetGroupWaveIndex() == 0) {
      __builtin_LinAlgMatrix
        [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
        Mat;
      __builtin_LinAlg_MatrixLoadFromMemory(
        Mat, GsData, OFFSET / ELEM_SIZE, STRIDE / ELEM_SIZE, LAYOUT);
      __builtin_LinAlg_MatrixStoreToDescriptor(
        Mat, Output, OFFSET, STRIDE, LAYOUT, 128);
    }
  }
)";

static void runLoadMemory(ID3D12Device *Device,
                          dxc::SpecificDllLoader &DxcSupport,
                          const MatrixParams &Params, bool Verbose,
                          UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DOFFSET=" << 0;

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, LoadMemoryShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N, 1);

  auto Op = createComputeOp(LoadMemoryShader, "cs_6_10", "UAV(u0), UAV(u1)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Input", BufferSize, false, "byname");
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [NumElements, Params](LPCSTR Name, std::vector<BYTE> &Data,
                                        st::ShaderOp *) {
                    VERIFY_IS_TRUE(fillInputBuffer(Name, Data, Params.CompType,
                                                   NumElements),
                                   "Saw unsupported component type");
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::LoadMemory_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"LoadMemory_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runLoadMemory(D3DDevice, DxcSupport, Params, VerboseLogging,
                SelectedWaveSize);
}

static const char StoreMemoryShader[] = R"(
  RWByteAddressBuffer Output : register(u0);
  groupshared ELEM_TYPE GsData[M_DIM * N_DIM];

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    for (uint Index = threadID; Index < M_DIM * N_DIM;
         Index += NUMTHREADS) {
      GsData[Index] = (ELEM_TYPE)0;
    }

    GroupMemoryBarrierWithGroupSync();

    if (GetGroupWaveIndex() == 0) {
      __builtin_LinAlgMatrix
        [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
        Mat;
      __builtin_LinAlg_FillMatrix(Mat, FILL_VALUE);

      __builtin_LinAlg_MatrixStoreToMemory(
        Mat, GsData, OFFSET / ELEM_SIZE, STRIDE / ELEM_SIZE, LAYOUT);
    }

    GroupMemoryBarrierWithGroupSync();

    for (uint Index = threadID; Index < M_DIM * N_DIM;
         Index += NUMTHREADS) {
      Output.Store<ELEM_TYPE>(Index * ELEM_SIZE, GsData[Index]);
    }
  }
)";

static void runStoreMemory(ID3D12Device *Device,
                           dxc::SpecificDllLoader &DxcSupport,
                           const MatrixParams &Params, bool Verbose,
                           float FillValue, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DOFFSET=" << 0;
  STREAM_FLOAT(ExtraDefs, "FILL_VALUE", FillValue);

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, StoreMemoryShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N,
                                  FillValue, /*Increment=*/false);

  auto Op =
      createComputeOp(StoreMemoryShader, "cs_6_10", "UAV(u0)", Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::StoreMemory_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"StoreMemory_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;

  runStoreMemory(D3DDevice, DxcSupport, Params, VerboseLogging,
                 /*FillValue=*/7.0f, SelectedWaveSize);
}

static const char AccumulateMemoryShader[] = R"(
  RWByteAddressBuffer Output : register(u0);
  groupshared ELEM_TYPE GsData[M_DIM * N_DIM];

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    for (uint Index = threadID; Index < M_DIM * N_DIM;
         Index += NUMTHREADS) {
      GsData[Index] = FILL_VALUE;
    }

    GroupMemoryBarrierWithGroupSync();

    if (GetGroupWaveIndex() == 0) {
      __builtin_LinAlgMatrix
        [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
        Mat;
      __builtin_LinAlg_FillMatrix(Mat, FILL_VALUE);

      __builtin_LinAlg_MatrixAccumulateToMemory(
        Mat, GsData, COMP_TYPE, OFFSET / ELEM_SIZE, STRIDE / ELEM_SIZE, LAYOUT);
    }

    GroupMemoryBarrierWithGroupSync();

    for (uint Index = threadID; Index < M_DIM * N_DIM;
         Index += NUMTHREADS) {
      Output.Store<ELEM_TYPE>(Index * ELEM_SIZE, GsData[Index]);
    }
  }
)";

static void runAccumulateMemory(ID3D12Device *Device,
                                dxc::SpecificDllLoader &DxcSupport,
                                const MatrixParams &Params, bool Verbose,
                                float FillValue, UINT ForcedWaveSize = 0) {
  const size_t NumElements = Params.totalElements();
  const size_t BufferSize = Params.totalBytes();

  std::stringstream ExtraDefs;
  ExtraDefs << " -DOFFSET=" << 0;
  STREAM_FLOAT(ExtraDefs, "FILL_VALUE", FillValue);

  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, AccumulateMemoryShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedMat(Params.CompType, Params.M, Params.N,
                                  FillValue * 2, /*Increment=*/false);

  auto Op = createComputeOp(AccumulateMemoryShader, "cs_6_10", "UAV(u0)",
                            Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(Params.CompType, OutData.data(),
                                       Expected, NumElements, Verbose));
}

static void runPaddedGroupSharedAccumulateCase(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport, bool Verbose);

void DxilConf_SM610_LinAlg::AccumulateMemory_Wave_16x16_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 16;
  Params.N = 16;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"AccumulateMemory_Wave_16x16_F16",
                                    SelectedWaveSize))
    return;
  if (!accumulateStoreApplicable(D3DDevice, Params.CompType,
                                 linalg_test::AtomicDestination::GroupShared,
                                 L"AccumulateMemory_Wave_16x16_F16"))
    return;

  runAccumulateMemory(D3DDevice, DxcSupport, Params, VerboseLogging,
                      /*FillValue=*/7.0f, SelectedWaveSize);
  runPaddedGroupSharedAccumulateCase(D3DDevice, DxcSupport, VerboseLogging);
}

static constexpr size_t GroupSharedTrailingGuardElements = 4;

static bool
getGroupSharedBufferDescription(const MatrixParams &Params,
                                const cpu_oracle::MatrixBufferLayout &Layout,
                                size_t &BufferSize, UINT &NumElements) {
  const size_t ElementBytes = elementSize(Params.CompType);
  const std::optional<size_t> RequiredBytes = cpu_oracle::getMatrixBufferSize(
      Params.CompType, Params.M, Params.N, Layout);
  size_t GuardBytes;
  // We must have gotten a valid size for the matrix buffer.
  if (!RequiredBytes.has_value())
    return false;

  // The offset, stride and size must be whole multiples of the element size.
  if (Layout.OffsetBytes % ElementBytes != 0)
    return false;
  if (Layout.StrideBytes % ElementBytes != 0)
    return false;
  if (*RequiredBytes % ElementBytes != 0)
    return false;

  // Safely compute the GuardBytes and BufferSize.
  if (!cpu_oracle::checkedMultiply(GroupSharedTrailingGuardElements,
                                   ElementBytes, GuardBytes))
    return false;
  if (!cpu_oracle::checkedAdd(*RequiredBytes, GuardBytes, BufferSize))
    return false;

  // The element count must fit in a uint32_t.
  if (BufferSize / ElementBytes > std::numeric_limits<uint32_t>::max())
    return false;

  NumElements = static_cast<UINT>(BufferSize / ElementBytes);
  return true;
}

static std::optional<std::vector<BYTE>>
makeGroupSharedTypedBuffer(ComponentType CompType, size_t BufferSize,
                           uint32_t FillValue) {
  const size_t ElementBytes = elementSize(CompType);
  if (BufferSize == 0 || BufferSize % ElementBytes != 0)
    return std::nullopt;

  std::vector<BYTE> Buffer(BufferSize);
  switch (CompType) {
  case ComponentType::F16: {
    const HLSLHalf_t Value(static_cast<float>(FillValue));
    for (size_t Offset = 0; Offset < BufferSize; Offset += ElementBytes)
      cpu_oracle::ComponentTraits<HLSLHalf_t>::store(Buffer.data() + Offset,
                                                     Value);
    break;
  }
  case ComponentType::F32: {
    const float Value = static_cast<float>(FillValue);
    for (size_t Offset = 0; Offset < BufferSize; Offset += ElementBytes)
      cpu_oracle::ComponentTraits<float>::store(Buffer.data() + Offset, Value);
    break;
  }
  case ComponentType::I32: {
    const int32_t Value = static_cast<int32_t>(FillValue);
    for (size_t Offset = 0; Offset < BufferSize; Offset += ElementBytes)
      cpu_oracle::ComponentTraits<int32_t>::store(Buffer.data() + Offset,
                                                  Value);
    break;
  }
  case ComponentType::U32: {
    for (size_t Offset = 0; Offset < BufferSize; Offset += ElementBytes)
      cpu_oracle::ComponentTraits<uint32_t>::store(Buffer.data() + Offset,
                                                   FillValue);
    break;
  }
  default:
    return std::nullopt;
  }
  return Buffer;
}

static bool verifyGroupSharedTypedBuffer(ComponentType CompType,
                                         const void *ActualBuffer,
                                         size_t ActualBufferSize,
                                         const std::vector<BYTE> &Expected,
                                         LPCWSTR Rule, bool Verbose) {
  if (!ActualBuffer || !Rule || ActualBufferSize != Expected.size()) {
    hlsl_test::LogErrorFmt(
        L"Invalid group-shared buffer comparison: actual=%zu, expected=%zu",
        ActualBufferSize, Expected.size());
    return false;
  }

  const BYTE *Actual = static_cast<const BYTE *>(ActualBuffer);
  size_t MismatchCount = 0;
  for (size_t Offset = 0; Offset < Expected.size(); ++Offset) {
    if (Actual[Offset] == Expected[Offset])
      continue;
    if (MismatchCount < 8) {
      hlsl_test::LogErrorFmt(
          L"Group-shared buffer mismatch at byte %zu: actual=0x%02x, "
          L"expected=0x%02x",
          Offset, Actual[Offset], Expected[Offset]);
    }
    ++MismatchCount;
  }

  if (MismatchCount != 0) {
    hlsl_test::LogErrorFmt(
        L"%zu bytes differ for exact group-shared buffer rule: %s",
        MismatchCount, Rule);
    return false;
  }

  if (Verbose) {
    hlsl_test::LogCommentFmt(
        L"Exact group-shared buffer comparison passed: component=%s, "
        L"elements=%zu, rule=%s",
        cpu_oracle::componentTypeName(CompType),
        Expected.size() / elementSize(CompType), Rule);
  }
  return true;
}

static const char GroupSharedTransferShader[] = R"(
  #define SCOPE_WAVE 1
  #define SCOPE_THREAD_GROUP 2

  ByteAddressBuffer SourceInit : register(t0);
  ByteAddressBuffer DestinationInit : register(t1);
  RWByteAddressBuffer Output : register(u2);

  groupshared ELEM_TYPE SourceData[SRC_ELEMENTS];
  groupshared ELEM_TYPE DestinationData[DST_ELEMENTS];

  void TransferMatrix() {
    __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
      Mat;
    __builtin_LinAlg_MatrixLoadFromMemory(
      Mat, SourceData, SRC_OFFSET, SRC_STRIDE, SRC_LAYOUT);
    __builtin_LinAlg_MatrixStoreToMemory(
      Mat, DestinationData, DST_OFFSET, DST_STRIDE, DST_LAYOUT);
  }

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    for (uint Index = threadID; Index < SRC_ELEMENTS;
         Index += NUMTHREADS) {
      SourceData[Index] =
        SourceInit.Load<ELEM_TYPE>(Index * ELEM_SIZE);
    }
    for (uint Index = threadID; Index < DST_ELEMENTS;
         Index += NUMTHREADS) {
      DestinationData[Index] =
        DestinationInit.Load<ELEM_TYPE>(Index * ELEM_SIZE);
    }

    GroupMemoryBarrierWithGroupSync();

    #if SCOPE == SCOPE_WAVE
    if (GetGroupWaveIndex() == 0)
      TransferMatrix();
    #elif SCOPE == SCOPE_THREAD_GROUP
    TransferMatrix();
    #else
    #error Group-shared transfer requires Wave or ThreadGroup scope
    #endif

    GroupMemoryBarrierWithGroupSync();

    for (uint Index = threadID; Index < DST_ELEMENTS;
         Index += NUMTHREADS) {
      Output.Store<ELEM_TYPE>(Index * ELEM_SIZE, DestinationData[Index]);
    }
  }
)";

static void
runGroupSharedTransfer(ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
                       const MatrixParams &Params,
                       const cpu_oracle::MatrixBufferLayout &SourceLayout,
                       const cpu_oracle::MatrixBufferLayout &DestinationLayout,
                       bool Verbose, UINT ForcedWaveSize = 0) {
  if (!Device ||
      (Params.Scope != MatrixScope::Wave &&
       Params.Scope != MatrixScope::ThreadGroup) ||
      Params.Use != MatrixUse::A) {
    VERIFY_IS_TRUE(false, "Invalid group-shared transfer parameters");
    return;
  }

  size_t SourceBufferSize;
  UINT SourceElements;
  size_t DestinationBufferSize;
  UINT DestinationElements;
  if (!getGroupSharedBufferDescription(Params, SourceLayout, SourceBufferSize,
                                       SourceElements) ||
      !getGroupSharedBufferDescription(Params, DestinationLayout,
                                       DestinationBufferSize,
                                       DestinationElements)) {
    VERIFY_IS_TRUE(false, "Invalid group-shared buffer description");
    return;
  }

  std::optional<cpu_oracle::TypedMatrix> Values =
      cpu_oracle::makeSequentialMatrix(Params.CompType, Params.M, Params.N);
  std::optional<std::vector<BYTE>> Source =
      makeGroupSharedTypedBuffer(Params.CompType, SourceBufferSize, 91);
  std::optional<std::vector<BYTE>> DestinationInitial =
      makeGroupSharedTypedBuffer(Params.CompType, DestinationBufferSize, 90);
  if (!Values.has_value() || !Source.has_value() ||
      !DestinationInitial.has_value() ||
      !cpu_oracle::writeMatrixBuffer(*Values, SourceLayout, *Source)) {
    VERIFY_IS_TRUE(false, "Failed to build group-shared transfer inputs");
    return;
  }

  std::vector<BYTE> Expected = *DestinationInitial;
  if (!cpu_oracle::writeMatrixBuffer(*Values, DestinationLayout, Expected)) {
    VERIFY_IS_TRUE(false, "Failed to build group-shared transfer expectation");
    return;
  }

  const size_t ElementBytes = elementSize(Params.CompType);
  std::stringstream ExtraDefs;
  ExtraDefs << " -DSRC_ELEMENTS=" << SourceElements;
  ExtraDefs << " -DSRC_OFFSET=" << SourceLayout.OffsetBytes / ElementBytes;
  ExtraDefs << " -DSRC_STRIDE=" << SourceLayout.StrideBytes / ElementBytes;
  ExtraDefs << " -DSRC_LAYOUT=" << static_cast<UINT>(SourceLayout.Layout);
  ExtraDefs << " -DDST_ELEMENTS=" << DestinationElements;
  ExtraDefs << " -DDST_OFFSET=" << DestinationLayout.OffsetBytes / ElementBytes;
  ExtraDefs << " -DDST_STRIDE=" << DestinationLayout.StrideBytes / ElementBytes;
  ExtraDefs << " -DDST_LAYOUT=" << static_cast<UINT>(DestinationLayout.Layout);
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  const std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());
  compileShader(DxcSupport, GroupSharedTransferShader, "cs_6_10", Args,
                Verbose);

  auto Op = createComputeOp(GroupSharedTransferShader, "cs_6_10",
                            "SRV(t0), SRV(t1), UAV(u2)", Args.c_str());
  addSRVBuffer(Op.get(), "SourceInit", Source->size(), "byname");
  addSRVBuffer(Op.get(), "DestinationInit", DestinationInitial->size(),
               "byname");
  addUAVBuffer(Op.get(), "Output", Expected.size(), true);
  addRootView(Op.get(), 0, "SourceInit");
  addRootView(Op.get(), 1, "DestinationInit");
  addRootView(Op.get(), 2, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
                    if (_stricmp(Name, "SourceInit") == 0)
                      Data = *Source;
                    else if (_stricmp(Name, "DestinationInit") == 0)
                      Data = *DestinationInitial;
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(verifyGroupSharedTypedBuffer(
      Params.CompType, OutData.data(), OutData.size(), Expected,
      L"MatrixLoadFromMemory and MatrixStoreToMemory preserve the exact "
      L"destination layout, padding and guards",
      Verbose));
}

static void runBidirectionalGroupSharedTransfer(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
    const MatrixParams &Params,
    const cpu_oracle::MatrixBufferLayout &TargetLayout,
    const cpu_oracle::MatrixBufferLayout &CanonicalLayout, bool Verbose,
    UINT ForcedWaveSize = 0) {
  hlsl_test::LogCommentFmt(L"Group-shared transfer: target to canonical");
  runGroupSharedTransfer(Device, DxcSupport, Params, TargetLayout,
                         CanonicalLayout, Verbose, ForcedWaveSize);
  hlsl_test::LogCommentFmt(L"Group-shared transfer: canonical to target");
  runGroupSharedTransfer(Device, DxcSupport, Params, CanonicalLayout,
                         TargetLayout, Verbose, ForcedWaveSize);
}

void DxilConf_SM610_LinAlg::
    LoadStoreMemory_Wave_4x8_F16_RowMajorOffsetPadded() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 64;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {Params.Use},
          L"LoadStoreMemory_Wave_4x8_F16_RowMajorOffsetPadded",
          SelectedWaveSize))
    return;

  const cpu_oracle::MatrixBufferLayout Target = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/8,
      /*StrideBytes=*/24,
  };
  const cpu_oracle::MatrixBufferLayout Canonical = {
      MatrixLayout::ColumnMajor,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/8,
  };
  runBidirectionalGroupSharedTransfer(D3DDevice, DxcSupport, Params, Target,
                                      Canonical, VerboseLogging,
                                      SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::
    LoadStoreMemory_Wave_4x8_F32_ColumnMajorOffsetPadded() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::ColumnMajor;
  Params.NumThreads = 64;
  Params.Enable16Bit = false;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(
          D3DDevice, Params, {Params.Use},
          L"LoadStoreMemory_Wave_4x8_F32_ColumnMajorOffsetPadded",
          SelectedWaveSize))
    return;

  const cpu_oracle::MatrixBufferLayout Target = {
      MatrixLayout::ColumnMajor,
      /*OffsetBytes=*/16,
      /*StrideBytes=*/24,
  };
  const cpu_oracle::MatrixBufferLayout Canonical = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/32,
  };
  runBidirectionalGroupSharedTransfer(D3DDevice, DxcSupport, Params, Target,
                                      Canonical, VerboseLogging,
                                      SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::LoadStoreMemory_ThreadGroup_4x8_F16() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::ThreadGroup;
  Params.Layout = MatrixLayout::ColumnMajor;
  Params.NumThreads = 64;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!matrixConstructionApplicable(D3DDevice, Params, {Params.Use},
                                    L"LoadStoreMemory_ThreadGroup_4x8_F16",
                                    SelectedWaveSize))
    return;

  const cpu_oracle::MatrixBufferLayout Target = {
      MatrixLayout::ColumnMajor,
      /*OffsetBytes=*/8,
      /*StrideBytes=*/12,
  };
  const cpu_oracle::MatrixBufferLayout Canonical = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/16,
  };
  runBidirectionalGroupSharedTransfer(D3DDevice, DxcSupport, Params, Target,
                                      Canonical, VerboseLogging,
                                      SelectedWaveSize);
}

static const char GroupSharedAccumulateShader[] = R"(
  ByteAddressBuffer Initial : register(t0);
  RWByteAddressBuffer Output : register(u1);
  groupshared ELEM_TYPE GsData[MEM_ELEMENTS];

  #ifdef FORCED_WAVE_SIZE
  [WaveSize(FORCED_WAVE_SIZE)]
  #else
  [WaveSize(4, 128)]
  #endif
  [numthreads(NUMTHREADS, 1, 1)]
  void main(uint threadID : SV_GroupIndex) {
    for (uint Index = threadID; Index < MEM_ELEMENTS;
         Index += NUMTHREADS) {
      GsData[Index] = Initial.Load<ELEM_TYPE>(Index * ELEM_SIZE);
    }

    GroupMemoryBarrierWithGroupSync();

    if (GetGroupWaveIndex() == 0) {
      __builtin_LinAlgMatrix
        [[__LinAlgMatrix_Attributes(COMP_TYPE, M_DIM, N_DIM, USE, SCOPE)]]
        Mat;
      __builtin_LinAlg_FillMatrix(Mat, 0);
      for (uint I = 0; I < __builtin_LinAlg_MatrixLength(Mat); ++I) {
        uint2 Coord = __builtin_LinAlg_MatrixGetCoordinate(Mat, I);
        __builtin_LinAlg_MatrixSetElement(
          Mat, Mat, I,
          (ELEM_TYPE)(ACCUMULATE_START + Coord.x * N_DIM + Coord.y));
      }
      __builtin_LinAlg_MatrixAccumulateToMemory(
        Mat, GsData, COMP_TYPE, MEM_OFFSET, MEM_STRIDE, MEM_LAYOUT);
    }

    GroupMemoryBarrierWithGroupSync();

    for (uint Index = threadID; Index < MEM_ELEMENTS;
         Index += NUMTHREADS) {
      Output.Store<ELEM_TYPE>(Index * ELEM_SIZE, GsData[Index]);
    }
  }
)";

static void runGroupSharedAccumulate(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
    const MatrixParams &Params,
    const cpu_oracle::MatrixBufferLayout &MemoryLayout, uint32_t InitialValue,
    uint32_t AccumulateStartingValue, bool Verbose, UINT ForcedWaveSize = 0) {
  if (!Device || Params.CompType != ComponentType::F16 ||
      Params.Scope != MatrixScope::Wave ||
      Params.Use != MatrixUse::Accumulator ||
      InitialValue >
          (std::numeric_limits<uint32_t>::max)() - AccumulateStartingValue) {
    VERIFY_IS_TRUE(false, "Invalid group-shared accumulate parameters");
    return;
  }

  size_t BufferSize;
  UINT NumElements;
  if (!getGroupSharedBufferDescription(Params, MemoryLayout, BufferSize,
                                       NumElements)) {
    VERIFY_IS_TRUE(false, "Invalid group-shared accumulate buffer");
    return;
  }

  const size_t MatrixElements = Params.totalElements();
  std::optional<cpu_oracle::TypedMatrix> InitialMatrix =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(
          Params.M, Params.N,
          std::vector<HLSLHalf_t>(
              MatrixElements, HLSLHalf_t(static_cast<float>(InitialValue))));
  std::optional<cpu_oracle::TypedMatrix> ExpectedMatrix =
      cpu_oracle::makeSequentialMatrix(Params.CompType, Params.M, Params.N,
                                       InitialValue + AccumulateStartingValue);
  std::optional<std::vector<BYTE>> Initial =
      makeGroupSharedTypedBuffer(Params.CompType, BufferSize, 90);
  std::optional<std::vector<BYTE>> Expected =
      makeGroupSharedTypedBuffer(Params.CompType, BufferSize, 90);
  if (!InitialMatrix.has_value() || !ExpectedMatrix.has_value() ||
      !Initial.has_value() || !Expected.has_value() ||
      !cpu_oracle::writeMatrixBuffer(*InitialMatrix, MemoryLayout, *Initial) ||
      !cpu_oracle::writeMatrixBuffer(*ExpectedMatrix, MemoryLayout,
                                     *Expected)) {
    VERIFY_IS_TRUE(false, "Failed to build group-shared accumulate oracle");
    return;
  }

  const size_t ElementBytes = elementSize(Params.CompType);
  std::stringstream ExtraDefs;
  ExtraDefs << " -DMEM_ELEMENTS=" << NumElements;
  ExtraDefs << " -DMEM_OFFSET=" << MemoryLayout.OffsetBytes / ElementBytes;
  ExtraDefs << " -DMEM_STRIDE=" << MemoryLayout.StrideBytes / ElementBytes;
  ExtraDefs << " -DMEM_LAYOUT=" << static_cast<UINT>(MemoryLayout.Layout);
  ExtraDefs << " -DACCUMULATE_START=" << AccumulateStartingValue;
  if (ForcedWaveSize != 0)
    ExtraDefs << " -DFORCED_WAVE_SIZE=" << ForcedWaveSize;

  const std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());
  compileShader(DxcSupport, GroupSharedAccumulateShader, "cs_6_10", Args,
                Verbose);

  auto Op = createComputeOp(GroupSharedAccumulateShader, "cs_6_10",
                            "SRV(t0), UAV(u1)", Args.c_str());
  addSRVBuffer(Op.get(), "Initial", Initial->size(), "byname");
  addUAVBuffer(Op.get(), "Output", Expected->size(), true);
  addRootView(Op.get(), 0, "Initial");
  addRootView(Op.get(), 1, "Output");

  auto Result =
      runShaderOp(Device, DxcSupport, std::move(Op),
                  [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
                    if (_stricmp(Name, "Initial") == 0)
                      Data = *Initial;
                  });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  VERIFY_IS_TRUE(verifyGroupSharedTypedBuffer(
      Params.CompType, OutData.data(), OutData.size(), *Expected,
      L"MatrixAccumulateToMemory adds each logical matrix value to 12 while "
      L"preserving exact padding and guards",
      Verbose));
}

static void runPaddedGroupSharedAccumulateCase(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport, bool Verbose) {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::Accumulator;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 64;
  Params.Enable16Bit = true;

  bool ConstructionSupported = false;
  UINT SelectedWaveSize = 0;
  const HRESULT ConstructionQuery = selectMatrixConstructionWaveSize(
      Device, Params, {Params.Use}, ConstructionSupported, SelectedWaveSize);
  const linalg_test::Applicability ConstructionApplicability =
      linalg_test::classifyApplicability(
          ConstructionQuery, ConstructionSupported,
          linalg_test::CapabilityRequirement::CapabilityGated);
  if (ConstructionApplicability == linalg_test::Applicability::Fail) {
    VERIFY_IS_TRUE(
        false, "Padded group-shared accumulation construction query failed");
    return;
  }
  if (ConstructionApplicability == linalg_test::Applicability::NotApplicable) {
    hlsl_test::LogCommentFmt(
        L"Padded 4x8 group-shared accumulation is not applicable: matrix "
        L"construction is unsupported");
    return;
  }
  VERIFY_IS_TRUE(SelectedWaveSize != 0,
                 "A supported padded accumulation needs a selected wave size");

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> DataType =
      toCapabilityDataType(Params.CompType);
  VERIFY_IS_TRUE(DataType.has_value(),
                 "Padded accumulation component type must be queryable");
  if (!DataType.has_value())
    return;

  linalg_test::AtomicAccumulateStoreSupport Support;
  const HRESULT AtomicQuery =
      linalg_test::queryAtomicAccumulateStore(Device, {*DataType}, Support);
  const linalg_test::Applicability AtomicApplicability =
      linalg_test::classifyApplicability(
          AtomicQuery,
          SUCCEEDED(AtomicQuery) &&
              Support.supports(linalg_test::AtomicDestination::GroupShared),
          linalg_test::CapabilityRequirement::CapabilityGated);
  if (AtomicApplicability == linalg_test::Applicability::Fail) {
    VERIFY_IS_TRUE(false,
                   "Padded group-shared accumulation support query failed");
    return;
  }
  if (AtomicApplicability == linalg_test::Applicability::NotApplicable) {
    hlsl_test::LogCommentFmt(
        L"Padded 4x8 group-shared accumulation is not applicable: the F16 "
        L"atomic destination is unsupported");
    return;
  }

  const cpu_oracle::MatrixBufferLayout Memory = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/8,
      /*StrideBytes=*/24,
  };
  runGroupSharedAccumulate(Device, DxcSupport, Params, Memory,
                           /*InitialValue=*/12,
                           /*AccumulateStartingValue=*/1, Verbose,
                           SelectedWaveSize);
}

static const char ConvertShader[] = R"(
  #define CT_F16 8
  #define CT_F32 9

  RWByteAddressBuffer Output : register(u0);

  [numthreads(1, 1, 1)]
  void main() {
    vector<half, 4> InVec = {1.0, 2.0, 3.0, 4.0};
    vector<float, 4> OutVec;
    __builtin_LinAlg_Convert(OutVec, InVec, CT_F16, CT_F32);
    Output.Store<float>(0, OutVec.x);
    Output.Store<float>(4, OutVec.y);
    Output.Store<float>(8, OutVec.z);
    Output.Store<float>(12, OutVec.w);
  }
)";

static void runConvert(ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
                       bool Verbose) {
  std::string Args = "-HV 2021 -enable-16bit-types";
  MatrixDim NumElements = 4;
  size_t BufferSize = elementSize(ComponentType::F32) * NumElements;

  compileShader(DxcSupport, ConvertShader, "cs_6_10", Args, Verbose);

  auto Expected = makeExpectedVec(ComponentType::F32, NumElements, 1.0);

  auto Op = createComputeOp(ConvertShader, "cs_6_10", "UAV(u0)", Args.c_str());
  addUAVBuffer(Op.get(), "Output", BufferSize, true);
  addRootView(Op.get(), 0, "Output");

  auto Result = runShaderOp(Device, DxcSupport, std::move(Op));

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);

  VERIFY_IS_TRUE(verifyComponentBuffer(ComponentType::F32, OutData.data(),
                                       Expected, NumElements, Verbose));
}

void DxilConf_SM610_LinAlg::Convert() {
  // Operates on vectors rather than matrices, so tier support is the only
  // capability it needs.
  if (!linAlgTierApplicable(D3DDevice, L"Convert"))
    return;

  runConvert(D3DDevice, DxcSupport, VerboseLogging);
}

static const char VectorAccumulateDescriptorShader[] = R"(
  ByteAddressBuffer Input : register(t0);
  RWByteAddressBuffer Output : register(u1);

  [numthreads(1, 1, 1)]
  void main() {
    vector<ELEM_TYPE, VECTOR_LENGTH> InVec;
    for (uint I = 0; I < VECTOR_LENGTH; ++I) {
      InVec[I] = Input.Load<ELEM_TYPE>(I * ELEM_SIZE);
    }
    __builtin_LinAlg_VectorAccumulateToDescriptor(
      Output, START_OFFSET, 64, InVec);
  }
)";

static void runVectorAccumulateDescriptor(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
    const cpu_oracle::TypedMatrix &Input,
    const cpu_oracle::TypedMatrix &Initial,
    const cpu_oracle::TypedMatrix &Expected, UINT StartOffsetBytes,
    std::wstring PublicRule, bool Verbose) {
  VERIFY_ARE_EQUAL(1u, Input.M, "Vector input must have one row");
  VERIFY_ARE_EQUAL(Input.compType(), Initial.compType(),
                   "Input and destination component types must match");
  VERIFY_ARE_EQUAL(Initial.compType(), Expected.compType(),
                   "Expected and destination component types must match");
  VERIFY_ARE_EQUAL(Initial.M, Expected.M,
                   "Expected and destination row counts must match");
  VERIFY_ARE_EQUAL(Initial.N, Expected.N,
                   "Expected and destination column counts must match");
  VERIFY_IS_GREATER_THAN_OR_EQUAL(
      Initial.totalElements(), Input.totalElements(),
      "Destination must hold the input vector and any guard elements");

  const cpu_oracle::MatrixBufferLayout InputLayout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/0,
      /*StrideBytes=*/Input.N * elementSize(Input.compType()),
  };
  const cpu_oracle::MatrixBufferLayout OutputLayout = {
      MatrixLayout::RowMajor,
      /*OffsetBytes=*/StartOffsetBytes,
      /*StrideBytes=*/Initial.N * elementSize(Initial.compType()),
  };
  const std::optional<size_t> InputSize =
      cpu_oracle::getMatrixBufferSize(Input, InputLayout);
  const std::optional<size_t> OutputSize =
      cpu_oracle::getMatrixBufferSize(Initial, OutputLayout);
  VERIFY_IS_TRUE(InputSize.has_value(), "Unable to size vector input buffer");
  VERIFY_IS_TRUE(OutputSize.has_value(),
                 "Unable to size vector destination buffer");
  if (!InputSize)
    return;
  if (!OutputSize)
    return;

  std::vector<BYTE> InputBytes(*InputSize);
  std::vector<BYTE> InitialBytes(*OutputSize);
  // Seed the destination with poison so the bytes the accumulation must leave
  // alone, including everything below StartOffsetBytes, carry a known value
  // through to readback rather than a zero the operation could also produce.
  cpu_oracle::fillPoison(InitialBytes.data(), InitialBytes.size());
  VERIFY_IS_TRUE(cpu_oracle::writeMatrixBuffer(Input, InputLayout, InputBytes),
                 "Unable to encode vector input buffer");
  VERIFY_IS_TRUE(
      cpu_oracle::writeMatrixBuffer(Initial, OutputLayout, InitialBytes),
      "Unable to encode vector destination buffer");

  MatrixParams Params = {};
  Params.CompType = Input.compType();
  Params.M = Input.M;
  Params.N = Input.N;
  Params.NumThreads = 1;
  Params.Enable16Bit = Input.compType() == ComponentType::F16;
  std::stringstream ExtraDefs;
  ExtraDefs << "-DVECTOR_LENGTH=" << Input.totalElements();
  ExtraDefs << " -DSTART_OFFSET=" << StartOffsetBytes;
  const std::string Args = buildCompilerArgs(Params, ExtraDefs.str().c_str());

  compileShader(DxcSupport, VectorAccumulateDescriptorShader, "cs_6_10", Args,
                Verbose);

  auto Op = createComputeOp(VectorAccumulateDescriptorShader, "cs_6_10",
                            "SRV(t0), UAV(u1)", Args.c_str());
  addSRVBuffer(Op.get(), "Input", InputBytes.size(), "byname");
  addUAVBuffer(Op.get(), "Output", InitialBytes.size(), true, "byname");
  addRootView(Op.get(), 0, "Input");
  addRootView(Op.get(), 1, "Output");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [&](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
        const std::vector<BYTE> *Source = nullptr;
        if (strcmp(Name, "Input") == 0)
          Source = &InputBytes;
        else if (strcmp(Name, "Output") == 0)
          Source = &InitialBytes;
        VERIFY_IS_TRUE(Source != nullptr,
                       "Unexpected vector accumulation resource initializer");
        if (!Source)
          return;
        VERIFY_ARE_EQUAL(Source->size(), Data.size(),
                         "Vector accumulation initializer size mismatch");
        if (Source->size() == Data.size())
          std::memcpy(Data.data(), Source->data(), Data.size());
      });

  MappedData OutData;
  Result->Test->GetReadBackData("Output", &OutData);
  const cpu_oracle::MatrixResultOracle Oracle =
      cpu_oracle::exactResult(Expected, std::move(PublicRule));
  VERIFY_IS_TRUE(cpu_oracle::verifyMatrixBuffer(OutData.data(), OutData.size(),
                                                OutputLayout, Oracle, Verbose));
  VERIFY_IS_TRUE(cpu_oracle::verifyUntouchedBytes(
      Initial.compType(), Initial.M, Initial.N, OutputLayout, OutData.data(),
      OutData.size(), Verbose));
}

void DxilConf_SM610_LinAlg::VectorAccumulateDescriptor_Thread_F16() {
  // Tier 1 requires no accumulation store formats, so this is gated.
  if (!accumulateStoreApplicable(
          D3DDevice, ComponentType::F16,
          linalg_test::AtomicDestination::RWByteAddressBuffer,
          L"VectorAccumulateDescriptor_Thread_F16"))
    return;

  const auto Half = [](float Value) { return HLSLHalf_t(Value); };
  const std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(
          1, 4, {Half(1), Half(2), Half(3), Half(4)});
  const std::optional<cpu_oracle::TypedMatrix> Initial =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(
          1, 4, {Half(0), Half(0), Half(0), Half(0)});
  const std::optional<cpu_oracle::TypedMatrix> Expected =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(
          1, 4, {Half(1), Half(2), Half(3), Half(4)});
  VERIFY_IS_TRUE(Input.has_value());
  VERIFY_IS_TRUE(Initial.has_value());
  VERIFY_IS_TRUE(Expected.has_value());
  if (!Input)
    return;
  if (!Initial)
    return;
  if (!Expected)
    return;

  runVectorAccumulateDescriptor(
      D3DDevice, DxcSupport, *Input, *Initial, *Expected,
      /*StartOffsetBytes=*/0, L"Exact F16 vector descriptor accumulation",
      VerboseLogging);
}

void DxilConf_SM610_LinAlg::
    VectorAccumulateDescriptor_Thread_F16_Length8_NonZero() {
  if (!accumulateStoreApplicable(
          D3DDevice, ComponentType::F16,
          linalg_test::AtomicDestination::RWByteAddressBuffer,
          L"VectorAccumulateDescriptor_Thread_F16_Length8_NonZero"))
    return;

  const auto Half = [](float Value) { return HLSLHalf_t(Value); };
  const std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(1, 8,
                                              {Half(-3), Half(2), Half(5),
                                               Half(-1), Half(4), Half(1),
                                               Half(-2), Half(6)});
  const std::optional<cpu_oracle::TypedMatrix> Initial =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(
          1, 10,
          {Half(10), Half(11), Half(12), Half(13), Half(14), Half(15), Half(16),
           Half(17), Half(123), Half(-321)});
  const std::optional<cpu_oracle::TypedMatrix> Expected =
      cpu_oracle::makeTypedMatrix<HLSLHalf_t>(
          1, 10,
          {Half(7), Half(13), Half(17), Half(12), Half(18), Half(16), Half(14),
           Half(23), Half(123), Half(-321)});
  VERIFY_IS_TRUE(Input.has_value());
  VERIFY_IS_TRUE(Initial.has_value());
  VERIFY_IS_TRUE(Expected.has_value());
  if (!Input)
    return;
  if (!Initial)
    return;
  if (!Expected)
    return;

  runVectorAccumulateDescriptor(
      D3DDevice, DxcSupport, *Input, *Initial, *Expected,
      /*StartOffsetBytes=*/64,
      L"Exact F16 length-8 accumulation at a non-zero offset onto non-zero "
      L"values with guard lanes",
      VerboseLogging);
}

void DxilConf_SM610_LinAlg::
    VectorAccumulateDescriptor_Thread_F32_Length8_NonZero() {
  if (!accumulateStoreApplicable(
          D3DDevice, ComponentType::F32,
          linalg_test::AtomicDestination::RWByteAddressBuffer,
          L"VectorAccumulateDescriptor_Thread_F32_Length8_NonZero"))
    return;

  const std::optional<cpu_oracle::TypedMatrix> Input =
      cpu_oracle::makeTypedMatrix<float>(1, 8, {6, -2, 2, 3, -5, 4, 1, -1});
  const std::optional<cpu_oracle::TypedMatrix> Initial =
      cpu_oracle::makeTypedMatrix<float>(
          1, 10, {20, 21, 22, 23, 24, 25, 26, 27, 123456, -654321});
  const std::optional<cpu_oracle::TypedMatrix> Expected =
      cpu_oracle::makeTypedMatrix<float>(
          1, 10, {26, 19, 24, 26, 19, 29, 27, 26, 123456, -654321});
  VERIFY_IS_TRUE(Input.has_value());
  VERIFY_IS_TRUE(Initial.has_value());
  VERIFY_IS_TRUE(Expected.has_value());
  if (!Input)
    return;
  if (!Initial)
    return;
  if (!Expected)
    return;

  runVectorAccumulateDescriptor(
      D3DDevice, DxcSupport, *Input, *Initial, *Expected,
      /*StartOffsetBytes=*/64,
      L"Exact F32 length-8 accumulation at a non-zero offset onto non-zero "
      L"values with guard lanes",
      VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_4x8_F16_NonUniform() {
  const matvec_interpretation::CaseData Case =
      matvec_interpretation::makeNonUniformF16Case(MatrixLayout::RowMajor);
  matvec_interpretation::runCapabilityChecked(
      D3DDevice, DxcSupport, Case,
      linalg_test::CapabilityRequirement::Mandatory,
      L"MatVecMul_Thread_4x8_F16_NonUniform", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_4x8_F16_ColumnMajor() {
  const matvec_interpretation::CaseData Case =
      matvec_interpretation::makeNonUniformF16Case(MatrixLayout::ColumnMajor);
  matvec_interpretation::runCapabilityChecked(
      D3DDevice, DxcSupport, Case,
      linalg_test::CapabilityRequirement::Mandatory,
      L"MatVecMul_Thread_4x8_F16_ColumnMajor", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_4x8_I8_Interpreted() {
  const matvec_interpretation::CaseData Case =
      matvec_interpretation::makeSInt8Case();
  matvec_interpretation::runCapabilityChecked(
      D3DDevice, DxcSupport, Case,
      linalg_test::CapabilityRequirement::Mandatory,
      L"MatVecMul_Thread_4x8_I8_Interpreted", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_4x8_U8_Interpreted() {
  const matvec_interpretation::CaseData Case =
      matvec_interpretation::makeUInt8Case();
  matvec_interpretation::runCapabilityChecked(
      D3DDevice, DxcSupport, Case,
      linalg_test::CapabilityRequirement::Mandatory,
      L"MatVecMul_Thread_4x8_U8_Interpreted", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatVecMul_Thread_4x8_U32_UnsignedOutput() {
  const matvec_interpretation::CaseData Case =
      matvec_interpretation::makeUInt32OutputCase();
  matvec_interpretation::runCapabilityChecked(
      D3DDevice, DxcSupport, Case,
      linalg_test::CapabilityRequirement::CapabilityGated,
      L"MatVecMul_Thread_4x8_U32_UnsignedOutput", VerboseLogging);
}

void DxilConf_SM610_LinAlg::MatVecMulAdd_Thread_4x8_F16_IndependentBias() {
  matvec_interpretation::CaseData Case =
      matvec_interpretation::makeNonUniformF16Case(MatrixLayout::RowMajor);
  Case.BiasInputType = ComponentType::F16;
  Case.BiasValues = {-5, 7, 3, -9};
  Case.PublicRule =
      L"Exact non-uniform F16 dots plus independent non-uniform bias";
  matvec_interpretation::runCapabilityChecked(
      D3DDevice, DxcSupport, Case,
      linalg_test::CapabilityRequirement::Mandatory,
      L"MatVecMulAdd_Thread_4x8_F16_IndependentBias", VerboseLogging);
}

struct ConvertThreadVectorMatrixMultiplyEntry {
  UINT VectorInputType;
  UINT MatrixInputType;
  UINT BiasInputType;
  UINT VectorResultType;
  UINT SupportFlags;
};

struct ConvertThreadVectorMatrixMultiplyEnumeration {
  UINT OperationType;
  UINT NumEntries;
  ConvertThreadVectorMatrixMultiplyEntry *ThreadVectorMatrixMultiply;
};

static constexpr D3D12_FEATURE ConvertOperationEnumerationFeature =
    static_cast<D3D12_FEATURE>(80);

static constexpr linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE
    ConvertMatrixInputTypes[] = {
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT8,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT8,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E5M2,
};

static constexpr linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE
    ConvertVectorResultTypes[] = {
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
        linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
};

#if defined(DIRECT3D_LINEAR_ALGEBRA)
static_assert(
    static_cast<UINT>(ConvertOperationEnumerationFeature) ==
    static_cast<UINT>(D3D12_FEATURE_LINEAR_ALGEBRA_OPERATION_ENUMERATION));
static_assert(
    sizeof(ConvertThreadVectorMatrixMultiplyEntry) ==
    sizeof(
        D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_ENUMERATION_ENTRY));
static_assert(sizeof(ConvertThreadVectorMatrixMultiplyEnumeration) ==
              sizeof(D3D12_FEATURE_DATA_LINEAR_ALGEBRA_OPERATION_ENUMERATION));
#endif

static HRESULT queryConvertSourceSupport(ID3D12Device *Device,
                                         ComponentType SourceCompType,
                                         bool &Supported) {
  Supported = false;
  if (!Device)
    return E_INVALIDARG;

  switch (SourceCompType) {
  case ComponentType::I32:
  case ComponentType::U32:
  case ComponentType::F32:
    Supported = true;
    return S_OK;
  case ComponentType::I16:
  case ComponentType::U16:
  case ComponentType::F16: {
    D3D12_FEATURE_DATA_D3D12_OPTIONS4 Options = {};
    const HRESULT HR = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4,
                                                   &Options, sizeof(Options));
    if (FAILED(HR)) {
      hlsl_test::LogCommentFmt(
          L"Native 16-bit source capability query failed: 0x%08x", HR);
      return HR;
    }
    Supported = Options.Native16BitShaderOpsSupported != FALSE;
    return S_OK;
  }
  default:
    return S_OK;
  }
}

static HRESULT queryConvertDestinationEnumeration(
    ID3D12Device *Device,
    linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE DestinationType,
    bool &Supported) {
  Supported = false;
  ConvertThreadVectorMatrixMultiplyEnumeration Query = {};
  Query.OperationType = static_cast<UINT>(
      linalg_abi::
          D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY);
  HRESULT HR = Device->CheckFeatureSupport(ConvertOperationEnumerationFeature,
                                           &Query, sizeof(Query));
  if (FAILED(HR))
    return HR;

  std::vector<ConvertThreadVectorMatrixMultiplyEntry> Entries(Query.NumEntries);
  if (!Entries.empty()) {
    Query.ThreadVectorMatrixMultiply = Entries.data();
    const UINT Capacity = Query.NumEntries;
    HR = Device->CheckFeatureSupport(ConvertOperationEnumerationFeature, &Query,
                                     sizeof(Query));
    if (FAILED(HR))
      return HR;

    if (Query.NumEntries > Capacity)
      return E_UNEXPECTED;

    Entries.resize(Query.NumEntries);
  }

  for (const ConvertThreadVectorMatrixMultiplyEntry &Entry : Entries) {
    if (Entry.VectorInputType == static_cast<UINT>(DestinationType) &&
        linalg_test::hasFlag(
            static_cast<
                linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS>(
                Entry.SupportFlags),
            linalg_abi::
                D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED)) {
      hlsl_test::LogCommentFmt(
          L"Convert destination support matched VectorInputType=%u via "
          L"enumeration",
          static_cast<UINT>(DestinationType));
      Supported = true;
      return S_OK;
    }
  }
  return S_OK;
}

static HRESULT queryConvertDestinationGranular(
    ID3D12Device *Device,
    linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE DestinationType,
    bool &Supported) {
  Supported = false;
  using LinAlgDataType = linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE;
  for (const LinAlgDataType MatrixInputType : ConvertMatrixInputTypes) {
    for (const LinAlgDataType VectorResultType : ConvertVectorResultTypes) {
      const LinAlgDataType BiasInputTypes[] = {
          linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE,
          VectorResultType,
      };
      for (const LinAlgDataType BiasInputType : BiasInputTypes) {
        linalg_test::ThreadVectorMatrixMultiplySupport Multiply;
        const HRESULT HR = linalg_test::queryThreadVectorMatrixMultiply(
            Device,
            {DestinationType, MatrixInputType, BiasInputType, VectorResultType},
            Multiply);
        if (FAILED(HR))
          return HR;

        if (Multiply.supported()) {
          hlsl_test::LogCommentFmt(
              L"Convert destination support matched VectorInputType=%u via "
              L"granular query",
              static_cast<UINT>(DestinationType));
          Supported = true;
          return S_OK;
        }
      }
    }
  }
  return S_OK;
}

static HRESULT queryConvertSupport(ID3D12Device *Device,
                                   ComponentType SourceCompType,
                                   ComponentType DestinationCompType,
                                   bool &Supported) {
  Supported = false;
  if (!Device)
    return E_INVALIDARG;

  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE> SourceType =
      toCapabilityDataType(SourceCompType);
  const std::optional<linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE>
      DestinationType = toCapabilityDataType(DestinationCompType);
  if (!SourceType.has_value() || !DestinationType.has_value())
    return E_INVALIDARG;
#if defined(DIRECT3D_LINEAR_ALGEBRA)
  VERIFY_ARE_EQUAL(static_cast<UINT>(*SourceType),
                   static_cast<UINT>(toLinAlgDataType(SourceCompType)));
  VERIFY_ARE_EQUAL(static_cast<UINT>(*DestinationType),
                   static_cast<UINT>(toLinAlgDataType(DestinationCompType)));
#endif

  linalg_test::TierSupport Tier;
  HRESULT HR = linalg_test::queryTierSupport(Device, Tier);
  if (FAILED(HR) || !Tier.supported())
    return HR;

  bool SourceSupported = false;
  HR = queryConvertSourceSupport(Device, SourceCompType, SourceSupported);
  if (FAILED(HR) || !SourceSupported)
    return HR;

  HR = queryConvertDestinationEnumeration(Device, *DestinationType, Supported);
  if (HR == DXGI_ERROR_UNSUPPORTED) {
    hlsl_test::LogCommentFmt(
        L"Convert operation enumeration is unavailable; falling back to the "
        L"granular vector-input query");
    HR = queryConvertDestinationGranular(Device, *DestinationType, Supported);
  }
  if (SUCCEEDED(HR) && !Supported)
    hlsl_test::LogCommentFmt(
        L"No supported ThreadVectorMatrixMultiply configuration advertises "
        L"VectorInputType=%u",
        static_cast<UINT>(*DestinationType));

  return HR;
}

static bool convertTypesApplicable(ID3D12Device *Device,
                                   ComponentType SourceCompType,
                                   ComponentType DestinationCompType,
                                   LPCWSTR CaseName) {
  bool Supported = false;
  const HRESULT QueryResult = queryConvertSupport(
      Device, SourceCompType, DestinationCompType, Supported);
  if (!applyApplicability(
          linalg_test::classifyApplicability(
              QueryResult, Supported,
              linalg_test::CapabilityRequirement::CapabilityGated),
          CaseName))
    return false;

  return true;
}

template <typename T>
static std::vector<BYTE> encodeConvertVector(std::initializer_list<T> Values) {
  std::vector<T> NativeValues(Values);
  std::vector<BYTE> Bytes(NativeValues.size() * sizeof(T));
  std::memcpy(Bytes.data(), NativeValues.data(), Bytes.size());
  return Bytes;
}

static bool verifyConvertBytes(const void *ActualBuffer,
                               size_t ActualBufferSize,
                               const std::vector<BYTE> &Expected,
                               LPCWSTR PublicRule, bool Verbose) {
  if (ActualBufferSize != Expected.size()) {
    hlsl_test::LogErrorFmt(
        L"Convert output size mismatch: actual=%zu, expected=%zu, rule=%s",
        ActualBufferSize, Expected.size(), PublicRule);
    return false;
  }

  const BYTE *Actual = static_cast<const BYTE *>(ActualBuffer);
  bool Success = true;
  for (size_t I = 0; I < Expected.size(); ++I) {
    if (Actual[I] != Expected[I]) {
      hlsl_test::LogErrorFmt(
          L"Convert byte %zu mismatch: actual=0x%02x, expected=0x%02x, "
          L"rule=%s",
          I, Actual[I], Expected[I], PublicRule);
      Success = false;
    } else if (Verbose) {
      hlsl_test::LogCommentFmt(L"  output[%zu]=0x%02x", I, Actual[I]);
    }
  }
  return Success;
}

static const char ConvertI16ToI32CoverageShader[] = R"(
  RWByteAddressBuffer Output : register(u0);

  [numthreads(1, 1, 1)]
  void main() {
    vector<int16_t, 8> InVec = {
      -32768, -12345, -1024, -1, 0, 1, 12345, 32767
    };
    vector<int, 8> OutVec;
    __builtin_LinAlg_Convert(OutVec, InVec, SRC_TYPE, DST_TYPE);
    for (uint I = 0; I < 8; ++I)
      Output.Store<int>(I * 4, OutVec[I]);
  }
)";

static const char ConvertF32ToI16CoverageShader[] = R"(
  RWByteAddressBuffer Output : register(u0);

  [numthreads(1, 1, 1)]
  void main() {
    vector<float, 8> InVec = {
      -40000.0F, -32768.5F, -2.5F, -1.5F,
      1.5F, 2.5F, 32767.5F, 40000.0F
    };
    vector<int16_t, 8> OutVec;
    __builtin_LinAlg_Convert(OutVec, InVec, SRC_TYPE, DST_TYPE);
    for (uint I = 0; I < 8; ++I)
      Output.Store<int16_t>(I * 2, OutVec[I]);
  }
)";

static const char ConvertF16FP8CoverageShader[] = R"(
  ByteAddressBuffer DecodeInput : register(t0);
  RWByteAddressBuffer Output : register(u0);

  [numthreads(1, 1, 1)]
  void main() {
    vector<half, 8> EncodeInput = {
      0.0, 1.0, -1.0, 1.15625, -1.21875, 2.3125, 5.625, -13.25
    };
    vector<uint, 2> Packed;
    __builtin_LinAlg_Convert(Packed, EncodeInput, SRC_TYPE, DST_TYPE);

    vector<uint, 2> HostPacked = {
      DecodeInput.Load<uint>(0), DecodeInput.Load<uint>(4)
    };
    vector<half, 8> Decoded;
    __builtin_LinAlg_Convert(Decoded, HostPacked, DST_TYPE, SRC_TYPE);

    Output.Store<uint>(0, Packed.x);
    Output.Store<uint>(4, Packed.y);
    for (uint I = 0; I < 8; ++I)
      Output.Store<half>(8 + I * 2, Decoded[I]);
  }
)";

static void runExactConvert(ID3D12Device *Device,
                            dxc::SpecificDllLoader &DxcSupport,
                            const char *Shader, const std::string &Args,
                            const std::vector<BYTE> &Expected,
                            LPCWSTR PublicRule, bool Verbose,
                            const std::vector<BYTE> *Input = nullptr) {
  compileShader(DxcSupport, Shader, "cs_6_10", Args, Verbose);

  auto Op = createComputeOp(
      Shader, "cs_6_10", Input ? "SRV(t0), UAV(u0)" : "UAV(u0)", Args.c_str());
  UINT OutputRootIndex = 0;
  if (Input) {
    VERIFY_IS_TRUE(!Input->empty(), "Convert input must not be empty");
    if (Input->empty())
      return;
    addSRVBuffer(Op.get(), "DecodeInput", Input->size(), "byname");
    addRootView(Op.get(), 0, "DecodeInput");
    OutputRootIndex = 1;
  }
  addUAVBuffer(Op.get(), "Output", Expected.size(), true);
  addRootView(Op.get(), OutputRootIndex, "Output");

  auto Result = runShaderOp(
      Device, DxcSupport, std::move(Op),
      [Input](LPCSTR Name, std::vector<BYTE> &Data, st::ShaderOp *) {
        if (Input && _stricmp(Name, "DecodeInput") == 0)
          Data = *Input;
      });

  MappedData OutputData;
  Result->Test->GetReadBackData("Output", &OutputData);
  VERIFY_IS_TRUE(verifyConvertBytes(OutputData.data(), OutputData.size(),
                                    Expected, PublicRule, Verbose));
}

struct FP8Format {
  unsigned ExponentBits;
  unsigned MantissaBits;
  unsigned ExponentBias;
  bool HasInfinity;
};

static std::optional<FP8Format> getFP8Format(ComponentType CompType) {
  switch (CompType) {
  case ComponentType::F8_E4M3FN:
    return FP8Format{/*ExponentBits=*/4, /*MantissaBits=*/3,
                     /*ExponentBias=*/7, /*HasInfinity=*/false};
  case ComponentType::F8_E5M2:
    return FP8Format{/*ExponentBits=*/5, /*MantissaBits=*/2,
                     /*ExponentBias=*/15, /*HasInfinity=*/true};
  default:
    return std::nullopt;
  }
}

static std::optional<BYTE> encodeHalfToFP8(HLSLHalf_t Value,
                                           ComponentType CompType) {
  const std::optional<FP8Format> Format = getFP8Format(CompType);
  if (!Format)
    return std::nullopt;

  const unsigned Sign = Value.Val >> 15;
  const unsigned SourceExponent = (Value.Val >> 10) & 0x1f;
  const unsigned SourceMantissa = Value.Val & 0x3ff;
  if (SourceExponent == 0)
    return SourceMantissa == 0
               ? std::optional<BYTE>(static_cast<BYTE>(Sign << 7))
               : std::nullopt;
  if (SourceExponent == 0x1f)
    return std::nullopt;

  int DestinationExponent = static_cast<int>(SourceExponent) - 15 +
                            static_cast<int>(Format->ExponentBias);
  if (DestinationExponent <= 0)
    return std::nullopt;

  const unsigned Shift = 10 - Format->MantissaBits;
  unsigned DestinationMantissa = SourceMantissa >> Shift;
  const unsigned Remainder = SourceMantissa & ((1u << Shift) - 1);
  const unsigned Halfway = 1u << (Shift - 1);
  if (Remainder > Halfway) {
    ++DestinationMantissa;
  } else if (Remainder == Halfway) {
    // Ties round to even.
    if (DestinationMantissa & 1u)
      ++DestinationMantissa;
  }
  if (DestinationMantissa == (1u << Format->MantissaBits)) {
    DestinationMantissa = 0;
    ++DestinationExponent;
  }

  const unsigned MaxExponent = (1u << Format->ExponentBits) - 1;
  const unsigned MaxFiniteExponent =
      Format->HasInfinity ? MaxExponent - 1 : MaxExponent;
  if (DestinationExponent > static_cast<int>(MaxFiniteExponent))
    return std::nullopt;
  if (!Format->HasInfinity) {
    // E4M3FN reserves the all-ones significand at the top exponent for NaN.
    const bool TopExponent =
        DestinationExponent == static_cast<int>(MaxExponent);
    const bool AllOnesMantissa =
        DestinationMantissa == (1u << Format->MantissaBits) - 1;
    if (TopExponent && AllOnesMantissa)
      return std::nullopt;
  }

  return static_cast<BYTE>(
      (Sign << 7) |
      (static_cast<unsigned>(DestinationExponent) << Format->MantissaBits) |
      DestinationMantissa);
}

struct FP8ConvertData {
  std::vector<BYTE> DecodeInput;
  std::vector<BYTE> ExpectedOutput;
};

struct FP8TestVectors {
  std::vector<HLSLHalf_t> EncodeValues;
  std::vector<BYTE> HandEncoded;
  std::vector<BYTE> DecodeInput;
  std::vector<BYTE> HandDecoded;
};

static std::optional<FP8TestVectors> getFP8TestVectors(ComponentType CompType) {
  // Chosen to discard mantissa bits without landing on a midpoint tie.
  const std::vector<HLSLHalf_t> EncodeValues = {
      HLSLHalf_t(0.0f),     HLSLHalf_t(1.0f),      HLSLHalf_t(-1.0f),
      HLSLHalf_t(1.15625f), HLSLHalf_t(-1.21875f), HLSLHalf_t(2.3125f),
      HLSLHalf_t(5.625f),   HLSLHalf_t(-13.25f)};

  switch (CompType) {
  case ComponentType::F8_E4M3FN:
    return FP8TestVectors{EncodeValues,
                          {0x00, 0x38, 0xb8, 0x39, 0xba, 0x41, 0x4b, 0xd5},
                          {0x3b, 0xbc, 0x45, 0xc9, 0x52, 0xd6, 0x29, 0xa2},
                          {0x80, 0x3d, 0x00, 0xbe, 0x80, 0x42, 0x80, 0xc4, 0x00,
                           0x49, 0x00, 0xcb, 0x80, 0x34, 0x00, 0xb1}};
  case ComponentType::F8_E5M2:
    return FP8TestVectors{EncodeValues,
                          {0x00, 0x3c, 0xbc, 0x3d, 0xbd, 0x41, 0x46, 0xcb},
                          {0x3e, 0xbf, 0x45, 0xc8, 0x4b, 0xce, 0x32, 0xb5},
                          {0x00, 0x3e, 0x00, 0xbf, 0x00, 0x45, 0x00, 0xc8, 0x00,
                           0x4b, 0x00, 0xce, 0x00, 0x32, 0x00, 0xb5}};
  default:
    return std::nullopt;
  }
}

static std::optional<FP8ConvertData>
makeFP8ConvertData(ComponentType CompType) {
  const std::optional<FP8TestVectors> Vectors = getFP8TestVectors(CompType);
  if (!Vectors)
    return std::nullopt;

  std::vector<BYTE> ExpectedOutput = Vectors->HandEncoded;
  ExpectedOutput.insert(ExpectedOutput.end(), Vectors->HandDecoded.begin(),
                        Vectors->HandDecoded.end());
  return FP8ConvertData{Vectors->DecodeInput, std::move(ExpectedOutput)};
}

void LinAlgCPUOracleTests::FP8HostOracle() {
  for (ComponentType CompType :
       {ComponentType::F8_E4M3FN, ComponentType::F8_E5M2}) {
    const std::optional<FP8TestVectors> Vectors = getFP8TestVectors(CompType);
    VERIFY_IS_TRUE(Vectors.has_value(), "Missing FP8 test vectors");
    if (!Vectors)
      continue;

    std::vector<BYTE> Encoded;
    for (HLSLHalf_t Value : Vectors->EncodeValues) {
      const std::optional<BYTE> ByteValue = encodeHalfToFP8(Value, CompType);
      if (!ByteValue) {
        hlsl_test::LogErrorFmt(
            L"%s is not representable as %s (half bits 0x%04x)",
            std::to_wstring(static_cast<float>(Value)).c_str(),
            cpu_oracle::componentTypeName(CompType), Value.Val);
        VERIFY_FAIL(L"FP8 host encoder rejected a test value");
        return;
      }
      Encoded.push_back(*ByteValue);
    }

    VERIFY_IS_TRUE(Encoded == Vectors->HandEncoded,
                   "Host FP8 encoder disagrees with the hand-derived table");

    // The decode input must not be the encoder's own output, or a shader that
    // echoed its input would pass the decode half of the execution test.
    VERIFY_IS_TRUE(Vectors->DecodeInput != Vectors->HandEncoded,
                   "FP8 decode input must be independent of encode output");
  }
}

static std::string buildConvertArgs(ComponentType SourceCompType,
                                    ComponentType DestinationCompType) {
  std::stringstream Args;
  Args << "-HV 2021 -enable-16bit-types";
  Args << " -DSRC_TYPE=" << static_cast<UINT>(SourceCompType);
  Args << " -DDST_TYPE=" << static_cast<UINT>(DestinationCompType);
  return Args.str();
}

void DxilConf_SM610_LinAlg::CopyConvert_Wave_4x8_F16_ToF32() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F16;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!copyConvertApplicable(D3DDevice, Params, ComponentType::F32,
                             /*Transpose=*/false,
                             L"CopyConvert_Wave_4x8_F16_ToF32",
                             SelectedWaveSize))
    return;
  runCopyConvert(D3DDevice, DxcSupport, Params, ComponentType::F32,
                 VerboseLogging, /*Transpose=*/false, SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::CopyConvert_Wave_4x8_F32_ToF16_Transpose() {
  MatrixParams Params = {};
  Params.CompType = ComponentType::F32;
  Params.M = 4;
  Params.N = 8;
  Params.Use = MatrixUse::A;
  Params.Scope = MatrixScope::Wave;
  Params.Layout = MatrixLayout::RowMajor;
  Params.NumThreads = 128;
  Params.Enable16Bit = true;

  UINT SelectedWaveSize = 0;
  if (!copyConvertApplicable(D3DDevice, Params, ComponentType::F16,
                             /*Transpose=*/true,
                             L"CopyConvert_Wave_4x8_F32_ToF16_Transpose",
                             SelectedWaveSize))
    return;
  runCopyConvert(D3DDevice, DxcSupport, Params, ComponentType::F16,
                 VerboseLogging, /*Transpose=*/true, SelectedWaveSize);
}

void DxilConf_SM610_LinAlg::Convert_I16_ToI32_Exact() {
  if (!convertTypesApplicable(D3DDevice, ComponentType::I16, ComponentType::I32,
                              L"Convert_I16_ToI32_Exact"))
    return;

  runExactConvert(
      D3DDevice, DxcSupport, ConvertI16ToI32CoverageShader,
      buildConvertArgs(ComponentType::I16, ComponentType::I32),
      encodeConvertVector<int32_t>(
          {-32768, -12345, -1024, -1, 0, 1, 12345, 32767}),
      L"Integer widening preserves every exactly representable value",
      VerboseLogging);
}

static void runFP8ConvertCase(ID3D12Device *Device,
                              dxc::SpecificDllLoader &DxcSupport,
                              ComponentType FP8CompType,
                              const FP8ConvertData &Data, bool Verbose) {
  const std::wstring PublicRule =
      std::wstring(cpu_oracle::componentTypeName(FP8CompType)) +
      L" packed fields and independent F16 decode";
  runExactConvert(Device, DxcSupport, ConvertF16FP8CoverageShader,
                  buildConvertArgs(ComponentType::F16, FP8CompType),
                  Data.ExpectedOutput, PublicRule.c_str(), Verbose,
                  &Data.DecodeInput);
}

void DxilConf_SM610_LinAlg::Convert_F32_ToI16_RTNE_Saturate() {
  if (!convertTypesApplicable(D3DDevice, ComponentType::F32, ComponentType::I16,
                              L"Convert_F32_ToI16_RTNE_Saturate"))
    return;

  runExactConvert(D3DDevice, DxcSupport, ConvertF32ToI16CoverageShader,
                  buildConvertArgs(ComponentType::F32, ComponentType::I16),
                  encodeConvertVector<int16_t>(
                      {-32768, -32768, -2, -2, 2, 2, 32767, 32767}),
                  L"Float-to-integer conversion is RTNE with signed saturation",
                  VerboseLogging);
}

void DxilConf_SM610_LinAlg::Convert_F16_ToE4M3FN_AndBack() {
  const std::optional<FP8ConvertData> Data =
      makeFP8ConvertData(ComponentType::F8_E4M3FN);
  VERIFY_IS_TRUE(Data.has_value(), "Unable to construct the host FP8 oracle");
  if (!Data)
    return;
  compileShader(DxcSupport, ConvertF16FP8CoverageShader, "cs_6_10",
                buildConvertArgs(ComponentType::F16, ComponentType::F8_E4M3FN),
                VerboseLogging);

  if (!convertTypesApplicable(D3DDevice, ComponentType::F16,
                              ComponentType::F8_E4M3FN,
                              L"Convert_F16_ToE4M3FN_AndBack"))
    return;
  if (!convertTypesApplicable(D3DDevice, ComponentType::U32, ComponentType::F16,
                              L"Convert_F16_ToE4M3FN_AndBack"))
    return;
  runFP8ConvertCase(D3DDevice, DxcSupport, ComponentType::F8_E4M3FN, *Data,
                    VerboseLogging);
}

void DxilConf_SM610_LinAlg::Convert_F16_ToE5M2_AndBack() {
  const std::optional<FP8ConvertData> Data =
      makeFP8ConvertData(ComponentType::F8_E5M2);
  VERIFY_IS_TRUE(Data.has_value(), "Unable to construct the host FP8 oracle");
  if (!Data)
    return;
  compileShader(DxcSupport, ConvertF16FP8CoverageShader, "cs_6_10",
                buildConvertArgs(ComponentType::F16, ComponentType::F8_E5M2),
                VerboseLogging);

  if (!convertTypesApplicable(D3DDevice, ComponentType::F16,
                              ComponentType::F8_E5M2,
                              L"Convert_F16_ToE5M2_AndBack"))
    return;
  if (!convertTypesApplicable(D3DDevice, ComponentType::U32, ComponentType::F16,
                              L"Convert_F16_ToE5M2_AndBack"))
    return;
  runFP8ConvertCase(D3DDevice, DxcSupport, ComponentType::F8_E5M2, *Data,
                    VerboseLogging);
}

} // namespace LinAlg
