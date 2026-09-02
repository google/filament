#ifndef HLSLEXECTESTUTILS_H
#define HLSLEXECTESTUTILS_H

#include <atlcomcli.h>
#include <d3d12.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <windows.h>

#include "ShaderOpTest.h"
#include "dxc/DXIL/DxilConstants.h"
#include "dxc/Support/dxcapi.use.h"

// The released Windows SDK does not declare the linear algebra API yet, and
// CheckFeatureSupport takes an untyped (void *, size) pair, so what this code
// depends on is a runtime layout rather than a compile-time type. Everything
// the tests need is therefore mirrored here.
//
// The whole shim is kept together in this one block so it can be deleted in
// one go. It is transcribed verbatim from d3d12.h - same type names, same
// field names, same enumerator names, same declaration order - so it can be
// diffed against the header directly, and so removing it leaves the use sites
// needing only the linalg_abi:: qualifier dropped.
//
// Enum-typed fields of the capability structures are declared as UINT because
// the values crossing CheckFeatureSupport are opaque here. The enumerations are
// the typed surface the tests use.
//
// None of it is taken on trust. When the preview SDK is present, the
// ASSERT_RUNTIME_* blocks at the end of this block pin every value, size,
// alignment and field offset to the real declarations. Those comparisons
// qualify both sides explicitly because the names are deliberately identical:
// unqualified lookup from inside the namespace would resolve to the local copy
// twice and assert nothing.
//
// For the same reason there is no using-directive for this namespace, and there
// must not be. The matrix-conversion helpers further down this header take the
// real D3D12 types, so pulling these names in unqualified would make every one
// of those declarations ambiguous in a preview-SDK build.
//
// TODO: delete this block once the linear algebra API ships in a released
// Windows SDK.
namespace linalg_abi {

// D3D12_FEATURE itself ships in the released SDK; only these two enumerators of
// it are missing, so they are mirrored as constants rather than as an
// enumeration.
constexpr D3D12_FEATURE D3D12_FEATURE_LINEAR_ALGEBRA_SUPPORT =
    static_cast<D3D12_FEATURE>(77);
constexpr D3D12_FEATURE D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT =
    static_cast<D3D12_FEATURE>(78);

enum D3D12_LINEAR_ALGEBRA_TIER {
  D3D12_LINEAR_ALGEBRA_TIER_NOT_SUPPORTED = 0,
  D3D12_LINEAR_ALGEBRA_TIER_1_0 = 0x10
};

enum D3D12_LINEAR_ALGEBRA_DATATYPE {
  D3D12_LINEAR_ALGEBRA_DATATYPE_NONE = 0,
  D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16 = 2,
  D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16 = 3,
  D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32 = 4,
  D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32 = 5,
  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16 = 7,
  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32 = 8,
  D3D12_LINEAR_ALGEBRA_DATATYPE_SINT8 = 18,
  D3D12_LINEAR_ALGEBRA_DATATYPE_UINT8 = 19,
  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN = 20,
  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E5M2 = 21
};

enum D3D12_LINEAR_ALGEBRA_OPERATION_TYPE {
  D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION = 0,
  D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY = 1,
  D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREADGROUP_MATRIX_MULTIPLY = 2,
  D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY = 3,
  D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_OUTER_PRODUCT = 4,
  D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE = 5
};

enum D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS {
  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE = 0,
  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED = 1,
  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_INPUTS = 2,
  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_OUTPUTS = 4,
  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_TRANSPOSE = 8
};

struct D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE {
  UINT M;
  UINT K;
  UINT N;
};

typedef D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE
    D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE;

struct D3D12_FEATURE_DATA_LINEAR_ALGEBRA_SUPPORT {
  UINT LinearAlgebraTier;
};

struct D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT {
  UINT ComponentType;
  UINT WaveSize;
  D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
  BOOL Supported;
};

struct D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS {
  UINT WaveSize;
  UINT MatrixAComponentType;
  UINT MatrixBComponentType;
  UINT AccumulatorComponentType;
};

struct D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_SUPPORT {
  D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS Inputs;
  D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
  UINT SupportFlags;
};

struct D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT {
  D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS WaveInputs;
  D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
  UINT SupportFlags;
  UINT MinThreadGroupSize;
  UINT MaxThreadGroupSize;
  UINT PreferredThreadGroupSize;
};

struct D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT {
  UINT VectorInputType;
  UINT MatrixInputType;
  UINT BiasInputType;
  UINT VectorResultType;
  UINT SupportFlags;
};

struct D3D12_LINEAR_ALGEBRA_THREAD_OUTER_PRODUCT_SUPPORT {
  UINT InputComponentType;
  UINT ResultComponentType;
  BOOL Supported;
};

struct D3D12_LINEAR_ALGEBRA_ATOMIC_ACCUMULATE_STORE_SUPPORT {
  UINT ComponentType;
  BOOL RWByteAddressBufferSupported;
  BOOL GroupSharedSupported;
};

struct D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT {
  UINT OperationType;
  union {
    D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT MatrixConstruction;
    D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_SUPPORT WaveMatrixMultiply;
    D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT
    ThreadGroupMatrixMultiply;
    D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT
    ThreadVectorMatrixMultiply;
    D3D12_LINEAR_ALGEBRA_THREAD_OUTER_PRODUCT_SUPPORT ThreadOuterProductSupport;
    D3D12_LINEAR_ALGEBRA_ATOMIC_ACCUMULATE_STORE_SUPPORT AccumulateStore;
  };
};

} // namespace linalg_abi

// Everything above is pinned to the real declarations here. This only compiles
// against a preview SDK, so it is not reached by an ordinary build; run a
// preview-SDK compile whenever the copies above are touched.
#if defined(DIRECT3D_LINEAR_ALGEBRA)

#define ASSERT_RUNTIME_ENUM(EnumeratorName)                                    \
  static_assert(static_cast<UINT>(linalg_abi::EnumeratorName) ==               \
                    static_cast<UINT>(::EnumeratorName),                       \
                "Linear algebra runtime enum value changed")

ASSERT_RUNTIME_ENUM(D3D12_FEATURE_LINEAR_ALGEBRA_SUPPORT);
ASSERT_RUNTIME_ENUM(D3D12_FEATURE_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_TIER_NOT_SUPPORTED);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_TIER_1_0);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_NONE);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_UINT16);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_SINT8);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_UINT8);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E5M2);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_MATRIX_CONSTRUCTION);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_WAVE_MATRIX_MULTIPLY);
ASSERT_RUNTIME_ENUM(
    D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREADGROUP_MATRIX_MULTIPLY);
ASSERT_RUNTIME_ENUM(
    D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_VECTOR_MATRIX_MULTIPLY);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_THREAD_OUTER_PRODUCT);
ASSERT_RUNTIME_ENUM(
    D3D12_LINEAR_ALGEBRA_OPERATION_TYPE_ATOMIC_ACCUMULATE_STORE);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED);
ASSERT_RUNTIME_ENUM(
    D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_INPUTS);
ASSERT_RUNTIME_ENUM(
    D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_OUTPUTS);
ASSERT_RUNTIME_ENUM(D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_TRANSPOSE);

#undef ASSERT_RUNTIME_ENUM

// The local copies share their names with the real header types, so these
// comparisons must be explicitly qualified: linalg_abi:: for the local copy and
// :: for the SDK declaration. Unqualified lookup from inside this namespace
// resolves to the local copy on both sides and would assert nothing.
#define ASSERT_RUNTIME_ABI(TypeName)                                           \
  static_assert(sizeof(linalg_abi::TypeName) == sizeof(::TypeName),            \
                "Linear algebra runtime ABI size changed");                    \
  static_assert(alignof(linalg_abi::TypeName) == alignof(::TypeName),          \
                "Linear algebra runtime ABI alignment changed")

ASSERT_RUNTIME_ABI(D3D12_FEATURE_DATA_LINEAR_ALGEBRA_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_THREAD_OUTER_PRODUCT_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_LINEAR_ALGEBRA_ATOMIC_ACCUMULATE_STORE_SUPPORT);
ASSERT_RUNTIME_ABI(D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT);

#undef ASSERT_RUNTIME_ABI

#define ASSERT_RUNTIME_OFFSET(TypeName, Field)                                 \
  static_assert(offsetof(linalg_abi::TypeName, Field) ==                       \
                    offsetof(::TypeName, Field),                               \
                "Linear algebra runtime ABI field offset changed")

ASSERT_RUNTIME_OFFSET(D3D12_FEATURE_DATA_LINEAR_ALGEBRA_SUPPORT,
                      LinearAlgebraTier);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT,
                      ComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT,
                      WaveSize);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT, Shape);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_CONSTRUCTION_SUPPORT,
                      Supported);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE, M);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE, K);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE, N);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS,
                      WaveSize);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS,
                      MatrixAComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS,
                      MatrixBComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS,
                      AccumulatorComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_SUPPORT,
                      Inputs);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_SUPPORT, Shape);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_SUPPORT,
                      SupportFlags);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT,
                      WaveInputs);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT,
                      Shape);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT,
                      SupportFlags);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT,
                      MinThreadGroupSize);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT,
                      MaxThreadGroupSize);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREADGROUP_MATRIX_MULTIPLY_SUPPORT,
                      PreferredThreadGroupSize);
ASSERT_RUNTIME_OFFSET(
    D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT,
    VectorInputType);
ASSERT_RUNTIME_OFFSET(
    D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT,
    MatrixInputType);
ASSERT_RUNTIME_OFFSET(
    D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT, BiasInputType);
ASSERT_RUNTIME_OFFSET(
    D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT,
    VectorResultType);
ASSERT_RUNTIME_OFFSET(
    D3D12_LINEAR_ALGEBRA_THREAD_VECTOR_MATRIX_MULTIPLY_SUPPORT, SupportFlags);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREAD_OUTER_PRODUCT_SUPPORT,
                      InputComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREAD_OUTER_PRODUCT_SUPPORT,
                      ResultComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_THREAD_OUTER_PRODUCT_SUPPORT,
                      Supported);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_ATOMIC_ACCUMULATE_STORE_SUPPORT,
                      ComponentType);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_ATOMIC_ACCUMULATE_STORE_SUPPORT,
                      RWByteAddressBufferSupported);
ASSERT_RUNTIME_OFFSET(D3D12_LINEAR_ALGEBRA_ATOMIC_ACCUMULATE_STORE_SUPPORT,
                      GroupSharedSupported);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT, OperationType);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
    MatrixConstruction);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
    WaveMatrixMultiply);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
    ThreadGroupMatrixMultiply);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
    ThreadVectorMatrixMultiply);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
    ThreadOuterProductSupport);
ASSERT_RUNTIME_OFFSET(
    D3D12_FEATURE_DATA_LINEAR_ALGEBRA_MATRIX_OPERATION_SUPPORT,
    AccumulateStore);

#undef ASSERT_RUNTIME_OFFSET

#endif // defined(DIRECT3D_LINEAR_ALGEBRA)

// D3D_SHADER_MODEL_6_10 is not yet in the released Windows SDK. Define locally
// so the test can query 6.10 driver support. This should be removed once
// widely supported.
#if defined(D3D12_PREVIEW_SDK_VERSION) && D3D12_PREVIEW_SDK_VERSION < 720
static const D3D_SHADER_MODEL D3D_SHADER_MODEL_6_10 = (D3D_SHADER_MODEL)0x6a;
#endif

// Local highest shader model known to DXC. Update this when adding support
// for new shader models. Unlike D3D_HIGHEST_SHADER_MODEL from the SDK,
// this stays in sync with DXC's own capabilities.
#define DXC_HIGHEST_SHADER_MODEL D3D_SHADER_MODEL_6_10

bool useDxbc();

/// Manages D3D12 (Agility) SDK selection
///
/// Based on TAEF runtime parameters, this picks an appropriate D3D12 SDK.
///
/// TAEF parameters:
///
///  D3D12SDKPath: relative or absolute path to the D3D12 Agility SDK bin
///  directory. Absolute path is only supported on OS's that support
///  ID3D12DeviceFactory.
///
///  D3D12SDKVersion: requested SDK version
///
///    0: auto-detect (quietly fallback to inbox version)
///
///    1: auto-detect (fail if unable to use the auto-detected version)
///
///   >1: use specified version
class D3D12SDKSelector {
  CComPtr<ID3D12DeviceFactory> DeviceFactory;

public:
  D3D12SDKSelector();
  ~D3D12SDKSelector();

  bool createDevice(ID3D12Device **D3DDevice,
                    D3D_SHADER_MODEL TestModel = D3D_SHADER_MODEL_6_0,
                    bool SkipUnsupported = true);
};

bool isWarp(ID3D12Device *D3DDevice);

void readHlslDataIntoNewStream(LPCWSTR RelativePath, IStream **Stream,
                               dxc::SpecificDllLoader &Support);

bool doesDeviceSupportInt64(ID3D12Device *pDevice);
bool doesDeviceSupportDouble(ID3D12Device *pDevice);
bool doesDeviceSupportWaveOps(ID3D12Device *pDevice);
bool doesDeviceSupportBarycentrics(ID3D12Device *pDevice);
bool doesDeviceSupportNative16bitOps(ID3D12Device *pDevice);
bool doesDeviceSupportMeshShaders(ID3D12Device *pDevice);
bool doesDeviceSupportRayTracing(ID3D12Device *pDevice);
bool doesDeviceSupportMeshAmpDerivatives(ID3D12Device *pDevice);
bool doesDeviceSupportTyped64Atomics(ID3D12Device *pDevice);
bool doesDeviceSupportHeap64Atomics(ID3D12Device *pDevice);
bool doesDeviceSupportShared64Atomics(ID3D12Device *pDevice);
bool doesDeviceSupportAdvancedTexOps(ID3D12Device *pDevice);
bool doesDeviceSupportWritableMSAA(ID3D12Device *pDevice);
bool doesDeviceSupportEnhancedBarriers(ID3D12Device *pDevice);
bool doesDeviceSupportRelaxedFormatCasting(ID3D12Device *pDevice);
bool isFallbackPathEnabled();

// Local copies of the D3D12 linear algebra enumerations and matrix shape. The
// released Windows SDK does not declare them yet, but the capability queries
// below must work without the preview SDK: CheckFeatureSupport takes an untyped
// (void *, size) pair, so they need the values, not the SDK's declarations.
//

namespace linalg_test {

// Execution scope is DXIL's MatrixScope. Its enumerators are sequential rather
// than disjoint bits, so scope sets are built with scopeBit() below.
using ScopeFlags = UINT;

constexpr ScopeFlags scopeBit(hlsl::DXIL::MatrixScope Scope) {
  return 1u << static_cast<UINT>(Scope);
}

enum class AtomicDestination {
  RWByteAddressBuffer,
  GroupShared,
};

enum class CapabilityRequirement {
  Mandatory,
  CapabilityGated,
};

enum class Applicability {
  Execute,
  NotApplicable,
  Fail,
};

struct TierSupport {
  linalg_abi::D3D12_LINEAR_ALGEBRA_TIER LinearAlgebraTier =
      linalg_abi::D3D12_LINEAR_ALGEBRA_TIER_NOT_SUPPORTED;

  bool supported() const {
    return LinearAlgebraTier !=
           linalg_abi::D3D12_LINEAR_ALGEBRA_TIER_NOT_SUPPORTED;
  }
};

// Capability queries name a concrete shape and the runtime answers whether it
// is supported; it applies the "positive multiple of a native tile" rule
// itself. Callers therefore ask about the shape they intend to use rather than
// reasoning about native tiles on their behalf.
struct MatrixConstructionQuery {
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE ComponentType;
  UINT WaveSize;
  linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
};

struct MatrixConstructionSupport {
  BOOL Supported = FALSE;

  bool valid() const;
  bool supported() const;
};

struct WaveMatrixMultiplyInputs {
  UINT WaveSize;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixAComponentType;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixBComponentType;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE AccumulatorComponentType;
};

struct WaveMatrixMultiplyQuery {
  WaveMatrixMultiplyInputs Inputs;
  linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
};

struct WaveMatrixMultiplySupport {
  linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS SupportFlags =
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE;

  bool valid() const;
  bool supported() const;
};

struct ThreadGroupMatrixMultiplyQuery {
  WaveMatrixMultiplyInputs WaveInputs;
  linalg_abi::D3D12_LINEAR_ALGEBRA_MATRIX_SHAPE Shape;
};

struct ThreadGroupMatrixMultiplySupport {
  linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS SupportFlags =
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE;
  UINT MinThreadGroupSize = 0;
  UINT MaxThreadGroupSize = 0;
  UINT PreferredThreadGroupSize = 0;

  bool valid() const;
  bool supported() const;
  bool supportsThreadGroupSize(UINT ThreadGroupSize) const;
};

struct ThreadVectorMatrixMultiplyQuery {
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE VectorInputType;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixInputType;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE BiasInputType;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE VectorResultType;
};

struct ThreadVectorMatrixMultiplySupport {
  linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS SupportFlags =
      linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE MatrixInputType =
      linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE_NONE;

  bool valid() const;
  bool supported() const;
};

struct ThreadOuterProductQuery {
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE InputComponentType;
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE ResultComponentType;
};

struct ThreadOuterProductSupport {
  bool Supported = false;

  bool supported() const { return Supported; }
};

struct AtomicAccumulateStoreQuery {
  linalg_abi::D3D12_LINEAR_ALGEBRA_DATATYPE ComponentType;
};

struct AtomicAccumulateStoreSupport {
  bool RWByteAddressBufferSupported = false;
  bool GroupSharedSupported = false;

  bool supports(AtomicDestination Destination) const;
};

bool hasFlag(
    linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS Value,
    linalg_abi::D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS Flag);
ScopeFlags
legalScopes(linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE Operation);
bool isLegalScope(linalg_abi::D3D12_LINEAR_ALGEBRA_OPERATION_TYPE Operation,
                  hlsl::DXIL::MatrixScope Scope);
Applicability classifyApplicability(HRESULT QueryResult, bool Supported,
                                    CapabilityRequirement Requirement);

HRESULT queryTierSupport(ID3D12Device *Device, TierSupport &Support);
HRESULT queryMatrixConstruction(ID3D12Device *Device,
                                const MatrixConstructionQuery &Query,
                                MatrixConstructionSupport &Support);
HRESULT queryWaveMatrixMultiply(ID3D12Device *Device,
                                const WaveMatrixMultiplyQuery &Query,
                                WaveMatrixMultiplySupport &Support);
HRESULT
queryThreadGroupMatrixMultiply(ID3D12Device *Device,
                               const ThreadGroupMatrixMultiplyQuery &Query,
                               ThreadGroupMatrixMultiplySupport &Support);
HRESULT
queryThreadVectorMatrixMultiply(ID3D12Device *Device,
                                const ThreadVectorMatrixMultiplyQuery &Query,
                                ThreadVectorMatrixMultiplySupport &Support);
HRESULT queryThreadOuterProduct(ID3D12Device *Device,
                                const ThreadOuterProductQuery &Query,
                                ThreadOuterProductSupport &Support);
HRESULT queryAtomicAccumulateStore(ID3D12Device *Device,
                                   const AtomicAccumulateStoreQuery &Query,
                                   AtomicAccumulateStoreSupport &Support);

} // namespace linalg_test

// TODO(#8661): Remove me when GroupSharedLimit is available in a released
// Windows SDK.
#if defined(D3D12_PREVIEW_SDK_VERSION)
UINT getMaxGroupSharedMemoryCS(ID3D12Device *Device);
UINT getMaxGroupSharedMemoryAS(ID3D12Device *Device);
UINT getMaxGroupSharedMemoryMS(ID3D12Device *Device);
#endif // defined(D3D12_PREVIEW_SDK_VERSION)

/// Create a ShaderOp for a compute shader dispatch.
std::unique_ptr<st::ShaderOp>
createComputeOp(const char *Source, const char *Target, const char *RootSig,
                const char *Args = nullptr, UINT DispatchX = 1,
                UINT DispatchY = 1, UINT DispatchZ = 1);

/// Add a UAV buffer resource to a ShaderOp.
void addUAVBuffer(st::ShaderOp *Op, const char *Name, UINT64 Width,
                  bool ReadBack, const char *Init = "zero");

/// Add a SRV buffer resource to a ShaderOp.
void addSRVBuffer(st::ShaderOp *Op, const char *Name, UINT64 Width,
                  const char *Init = "zero");

/// Bind a resource to a root view parameter by index.
void addRootView(st::ShaderOp *Op, UINT Index, const char *ResName);

/// Add a raw-buffer UAV descriptor for a resource to a shader-visible
/// descriptor heap, creating the heap on first use. ViewBytes sizes the view
/// independently of the resource it looks at, which is the whole point of
/// binding this way: a root view is a bare address, so the shader has no
/// dimensions to bounds check against, while a descriptor carries them.
/// Descriptors are consumed in the order added, matching the register order of
/// the ranges declared in the root signature.
void addHeapRawUAV(st::ShaderOp *Op, const char *HeapName, const char *ResName,
                   UINT64 ViewBytes);

/// Bind a descriptor heap to a root descriptor-table parameter by index.
void addRootTable(st::ShaderOp *Op, UINT Index, const char *HeapName);

/// Run a programmatically-built ShaderOp and return the result.
std::shared_ptr<st::ShaderOpTestResult> runShaderOp(
    ID3D12Device *Device, dxc::SpecificDllLoader &DxcSupport,
    std::unique_ptr<st::ShaderOp> Op,
    st::ShaderOpTest::TInitCallbackFn InitCallback = nullptr,
    st::ShaderOpTest::TCommandCallbackFn PostDispatchCallback = nullptr);

/// Compiles an HLSL shader using the DXC API to verify it is well-formed.
/// Fails the test on compile error.
void compileShader(dxc::SpecificDllLoader &DxcSupport, const char *Source,
                   const char *Target, const std::string &Args,
                   bool VerboseLogging = false);

// Host-side linear-algebra matrix-conversion helpers. These need the D3D12
// linear-algebra API (the D3D12_LINEAR_ALGEBRA_* types and the conversion
// methods on ID3D12DevicePreview / ID3D12GraphicsCommandListPreview), which is
// gated behind the preview SDK's DIRECT3D_LINEAR_ALGEBRA feature macro. When
// absent, these helpers and the tests using them are compiled out (they Skip at
// runtime).
#if defined(DIRECT3D_LINEAR_ALGEBRA)
/// Query the number of bytes required to store an NumRows x NumColumns matrix
/// of the given datatype in the specified device layout.
UINT getLinAlgMatrixByteSize(ID3D12Device *Device, UINT NumRows,
                             UINT NumColumns,
                             D3D12_LINEAR_ALGEBRA_DATATYPE DataType,
                             D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT Layout,
                             UINT Stride);

/// Record a GPU matrix layout conversion onto \p List using
/// ID3D12GraphicsCommandListPreview::ConvertLinearAlgebraMatrix. Both
/// \p SrcBuffer (in \p SrcLayout) and \p DestBuffer (receiving \p DestLayout)
/// must be passed in the D3D12_RESOURCE_STATE_UNORDERED_ACCESS state; the
/// conversion requires the source in NON_PIXEL_SHADER_RESOURCE, so this helper
/// transitions it and leaves the destination in UNORDERED_ACCESS. The caller is
/// responsible for ensuring that writes to \p SrcBuffer have completed before
/// this conversion reads it.
void recordLinAlgMatrixConversion(
    ID3D12GraphicsCommandList *List, ID3D12Resource *SrcBuffer, UINT SrcSize,
    ID3D12Resource *DestBuffer, UINT DestSize, UINT NumRows, UINT NumColumns,
    D3D12_LINEAR_ALGEBRA_DATATYPE DataType,
    D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT SrcLayout, UINT SrcStride,
    D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT DestLayout, UINT DestStride);
#endif // defined(DIRECT3D_LINEAR_ALGEBRA)

#endif // HLSLEXECTESTUTILS_H
