#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#define INLINE_TEST_METHOD_MARKUP
#include <WexTestClass.h>

#include "LongVectorTestData.h"

#include "ShaderOpTest.h"
#include "dxc/Support/Global.h"

#include "HlslExecTestUtils.h"
#include "HlslTestDataTypes.h"
#include "HlslTestUtils.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace LongVector {

//
// Data Types
//

template <typename T> constexpr bool is16BitType() {
  return std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t> ||
         std::is_same_v<T, HLSLHalf_t>;
}

struct DataType {
  const char *HLSLTypeString;
  bool Is16Bit;
  size_t HLSLSizeInBytes;
};

template <typename T> const DataType &getDataType() {
  static_assert(false && "Unknown data type");
}

#define DATA_TYPE(TYPE, HLSL_STRING, HLSL_SIZE)                                \
  template <> const DataType &getDataType<TYPE>() {                            \
    static DataType DataType{HLSL_STRING, is16BitType<TYPE>(), HLSL_SIZE};     \
    return DataType;                                                           \
  }

DATA_TYPE(HLSLBool_t, "bool", 4)
DATA_TYPE(int16_t, "int16_t", 2)
DATA_TYPE(int32_t, "int", 4)
DATA_TYPE(int64_t, "int64_t", 8)
DATA_TYPE(uint16_t, "uint16_t", 2)
DATA_TYPE(uint32_t, "uint32_t", 4)
DATA_TYPE(uint64_t, "uint64_t", 8)
DATA_TYPE(HLSLHalf_t, "half", 2)
DATA_TYPE(HLSLMin16Float_t, "min16float", 4)
DATA_TYPE(HLSLMin16Int_t, "min16int", 4)
DATA_TYPE(HLSLMin16Uint_t, "min16uint", 4)
DATA_TYPE(float, "float", 4)
DATA_TYPE(double, "double", 8)

#undef DATA_TYPE

using HLSLTestDataTypes::doValuesMatch;
using HLSLTestDataTypes::ValidationType;

template <typename T> constexpr bool isFloatingPointType() {
  return std::is_same_v<T, float> || std::is_same_v<T, double> ||
         std::is_same_v<T, HLSLHalf_t> || std::is_same_v<T, HLSLMin16Float_t>;
}

template <typename T> constexpr bool isMinPrecisionType() {
  return std::is_same_v<T, HLSLMin16Float_t> ||
         std::is_same_v<T, HLSLMin16Int_t> ||
         std::is_same_v<T, HLSLMin16Uint_t>;
}

//
// Operation Types
//

enum class OpType : unsigned {
#define OP(GROUP, SYMBOL, ARITY, INTRINSIC, OPERATOR, DEFINES, SHADER_NAME,    \
           INPUT_SET_1, INPUT_SET_2, INPUT_SET_3)                              \
  SYMBOL,
#include "LongVectorOps.def"
  NumOpTypes
};

struct Operation {
  size_t Arity;
  const char *Intrinsic;
  const char *Operator;
  const char *ExtraDefines;
  const char *ShaderName;
  InputSet InputSets[3];
  OpType Type;
};

static constexpr Operation Operations[] = {

#define OP(GROUP, SYMBOL, ARITY, INTRINSIC, OPERATOR, DEFINES, SHADER_NAME,    \
           INPUT_SET_1, INPUT_SET_2, INPUT_SET_3)                              \
  {ARITY,                                                                      \
   INTRINSIC,                                                                  \
   OPERATOR,                                                                   \
   DEFINES,                                                                    \
   SHADER_NAME,                                                                \
   {InputSet::INPUT_SET_1, InputSet::INPUT_SET_2, InputSet::INPUT_SET_3},      \
   OpType::SYMBOL},
#include "LongVectorOps.def"
};

constexpr const Operation &getOperation(OpType Op) {
  if (Op < OpType::NumOpTypes)
    return Operations[unsigned(Op)];
  std::abort();
}

static const std::unordered_set<OpType> LoadAndStoreOpTypes = {
    OpType::LoadAndStore_RDH_BAB_UAV, OpType::LoadAndStore_RDH_BAB_SRV,
    OpType::LoadAndStore_DT_BAB_UAV,  OpType::LoadAndStore_DT_BAB_SRV,
    OpType::LoadAndStore_RD_BAB_UAV,  OpType::LoadAndStore_RD_BAB_SRV,
    OpType::LoadAndStore_RDH_SB_UAV,  OpType::LoadAndStore_RDH_SB_SRV,
    OpType::LoadAndStore_DT_SB_UAV,   OpType::LoadAndStore_DT_SB_SRV,
    OpType::LoadAndStore_RD_SB_UAV,   OpType::LoadAndStore_RD_SB_SRV,
};

static bool IsStructuredBufferLoadAndStoreOp(OpType Op) {
  switch (Op) {
  case OpType::LoadAndStore_RDH_SB_UAV:
  case OpType::LoadAndStore_RDH_SB_SRV:
  case OpType::LoadAndStore_DT_SB_UAV:
  case OpType::LoadAndStore_DT_SB_SRV:
  case OpType::LoadAndStore_RD_SB_UAV:
  case OpType::LoadAndStore_RD_SB_SRV:
    return true;
  default:
    return false;
  }
}

// Helper to fill the test data from the shader buffer based on type.
// Convenient to be used when copying HLSL*_t types so we can use the
// underlying type.
template <typename T>
void fillLongVectorDataFromShaderBuffer(const MappedData &ShaderBuffer,
                                        std::vector<T> &TestData,
                                        size_t NumElements) {

  if constexpr (std::is_same_v<T, HLSLHalf_t>) {
    auto *ShaderBufferPtr =
        static_cast<const DirectX::PackedVector::HALF *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      TestData.push_back(HLSLHalf_t::FromHALF(ShaderBufferPtr[I]));
    return;
  }

  if constexpr (std::is_same_v<T, HLSLBool_t>) {
    auto *ShaderBufferPtr = static_cast<const int32_t *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      // HLSLBool_t has a int32_t based constructor.
      TestData.push_back(ShaderBufferPtr[I]);
    return;
  }

  auto *ShaderBufferPtr = static_cast<const T *>(ShaderBuffer.data());
  for (size_t I = 0; I < NumElements; I++)
    TestData.push_back(ShaderBufferPtr[I]);
  return;
}

template <typename T>
void logLongVector(const std::vector<T> &Values, const std::wstring &Name) {
  hlsl_test::LogCommentFmt(L"LongVector Name: %s", Name.c_str());

  const size_t LoggingWidth = 40;

  std::wstringstream Wss(L"");
  Wss << L"LongVector Values: ";
  Wss << L"[";
  const size_t NumElements = Values.size();
  for (size_t I = 0; I < NumElements; I++) {
    if (I % LoggingWidth == 0 && I != 0)
      Wss << L"\n ";
    Wss << Values[I];
    if (I != NumElements - 1)
      Wss << L", ";
  }
  Wss << L" ]";

  hlsl_test::LogCommentFmt(Wss.str().c_str());
}

template <typename T>
bool doVectorsMatch(const std::vector<T> &ActualValues,
                    const std::vector<T> &ExpectedValues, double Tolerance,
                    ValidationType ValidationType, bool VerboseLogging) {

  DXASSERT(
      ActualValues.size() == ExpectedValues.size(),
      "Programmer error: Actual and Expected vectors must be the same size.");

  if (VerboseLogging) {
    logLongVector(ActualValues, L"ActualValues");
    logLongVector(ExpectedValues, L"ExpectedValues");

    hlsl_test::LogCommentFmt(
        L"ValidationType: %s, Tolerance: %17g",
        ValidationType == ValidationType::Epsilon ? L"Epsilon" : L"ULP",
        Tolerance);
  }

  // Stash mismatched indexes for easy failure logging later
  std::vector<size_t> MismatchedIndexes;
  for (size_t I = 0; I < ActualValues.size(); I++) {
    if (!doValuesMatch(ActualValues[I], ExpectedValues[I], Tolerance,
                       ValidationType))
      MismatchedIndexes.push_back(I);
  }

  if (MismatchedIndexes.empty())
    return true;

  if (!MismatchedIndexes.empty()) {
    for (size_t Index : MismatchedIndexes) {
      std::wstringstream Wss(L"");
      Wss << std::setprecision(15); // Set precision for floating point types
      Wss << L"Mismatch at Index: " << Index;
      Wss << L" Actual Value:" << ActualValues[Index] << ",";
      Wss << L" Expected Value:" << ExpectedValues[Index];
      hlsl_test::LogErrorFmt(Wss.str().c_str());
    }
  }

  return false;
}

static WEX::Common::String getInputValueSetName(size_t Index) {
  using WEX::Common::String;
  using WEX::TestExecution::TestData;

  DXASSERT(Index >= 0 && Index <= 9, "Only single digit indices supported");

  String ParameterName = L"InputValueSetName";
  ParameterName.Append((wchar_t)(L'1' + Index));

  String ValueSetName;
  if (FAILED(TestData::TryGetValue(ParameterName, ValueSetName))) {
    String Name = L"DefaultInputValueSet";
    Name.Append((wchar_t)(L'1' + Index));
    return Name;
  }

  return ValueSetName;
}

std::string getCompilerOptionsString(
    const Operation &Operation, const DataType &OpDataType,
    const DataType &OutDataType, size_t VectorSize,
    std::optional<std::string> AdditionalOptions = std::nullopt) {
  std::stringstream CompilerOptions;

  if (OpDataType.Is16Bit || OutDataType.Is16Bit)
    CompilerOptions << " -enable-16bit-types";

  CompilerOptions << " -DTYPE=" << OpDataType.HLSLTypeString;
  CompilerOptions << " -DNUM=" << VectorSize;

  CompilerOptions << " -DOPERATOR=";
  CompilerOptions << Operation.Operator;

  CompilerOptions << " -DFUNC=";
  CompilerOptions << Operation.Intrinsic;

  CompilerOptions << " " << Operation.ExtraDefines;

  CompilerOptions << " -DOUT_TYPE=" << OutDataType.HLSLTypeString;

  CompilerOptions << " -DBASIC_OP_TYPE=0x" << std::hex << Operation.Arity;

  if (AdditionalOptions)
    CompilerOptions << " " << AdditionalOptions.value();

  return CompilerOptions.str();
}

// Helper to fill the shader buffer based on type. Convenient to be used when
// copying HLSL*_t types so we can copy the underlying type directly instead
// of the struct.
template <typename T>
void fillShaderBufferFromLongVectorData(std::vector<BYTE> &ShaderBuffer,
                                        const std::vector<T> &TestData) {

  // Note: DataSize for HLSLHalf_t and HLSLBool_t may be larger than the
  // underlying type in some cases. Thats fine. Resize just makes sure we have
  // enough space.
  const size_t NumElements = TestData.size();
  [[maybe_unused]] const size_t DataSize = sizeof(T) * NumElements;

  // Ensure the shader buffer is large enough. It should be pre-sized based on
  // the D3D12_RESOURCE_DESC for the associated D3D12_RESOURCE.
  DXASSERT_NOMSG(ShaderBuffer.size() >= DataSize);

  if constexpr (std::is_same_v<T, HLSLHalf_t>) {
    auto *ShaderBufferPtr =
        reinterpret_cast<DirectX::PackedVector::HALF *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      ShaderBufferPtr[I] = TestData[I].Val;
    return;
  }

  if constexpr (std::is_same_v<T, HLSLBool_t>) {
    auto *ShaderBufferPtr = reinterpret_cast<int32_t *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      ShaderBufferPtr[I] = TestData[I].Val;
    return;
  }

  auto *ShaderBufferPtr = reinterpret_cast<T *>(ShaderBuffer.data());
  for (size_t I = 0; I < NumElements; I++)
    ShaderBufferPtr[I] = TestData[I];
}

//
// Run the test.  Return std::nullopt if the test was skipped, otherwise returns
// the output buffer that was populated by the shader.
//
template <typename T> using InputSets = std::vector<std::vector<T>>;

template <typename OUT_TYPE, typename T>
std::optional<std::vector<OUT_TYPE>>
runTest(ID3D12Device *D3DDevice, bool VerboseLogging,
        const Operation &Operation, const InputSets<T> &Inputs,
        size_t ExpectedOutputSize,
        std::optional<std::string> AdditionalCompilerOptions) {
  DXASSERT_NOMSG(Inputs.size() == Operation.Arity);

  if (VerboseLogging) {
    for (size_t I = 0; I < Operation.Arity; ++I) {
      std::wstring Name = L"InputVector";
      Name += (wchar_t)(L'1' + I);
      logLongVector(Inputs[I], Name);
    }
  }

  const DataType &OpDataType = getDataType<T>();
  const DataType &OutDataType = getDataType<OUT_TYPE>();

  // We have to construct the string outside of the lambda. Otherwise it's
  // cleaned up when the lambda finishes executing but before the shader runs.
  std::string CompilerOptionsString =
      getCompilerOptionsString(Operation, OpDataType, OutDataType,
                               Inputs[0].size(), AdditionalCompilerOptions);

  dxc::SpecificDllLoader DxilDllLoader;
  CComPtr<IStream> TestXML;
  readHlslDataIntoNewStream(L"ShaderOpArith.xml", &TestXML, DxilDllLoader);
  auto ShaderOpSet = std::make_shared<st::ShaderOpSet>();
  st::ParseShaderOpSetFromStream(TestXML, ShaderOpSet.get());

  if (LoadAndStoreOpTypes.count(Operation.Type) > 0)
    configureLoadAndStoreShaderOp(Operation, OpDataType, Inputs[0].size(),
                                  sizeof(T), ShaderOpSet.get());

  // RunShaderOpTest is a helper function that handles resource creation
  // and setup. It also handles the shader compilation and execution. It takes
  // a callback that is called when the shader is compiled, but before it is
  // executed.
  std::shared_ptr<st::ShaderOpTestResult> TestResult =
      st::RunShaderOpTestAfterParse(
          D3DDevice, DxilDllLoader, Operation.ShaderName,
          [&](LPCSTR Name, std::vector<BYTE> &ShaderData,
              st::ShaderOp *ShaderOp) {
            if (VerboseLogging)
              hlsl_test::LogCommentFmt(
                  L"RunShaderOpTest CallBack. Resource Name: %S", Name);

            // This callback is called once for each resource defined for
            // "LongVectorOp" in ShaderOpArith.xml. All callbacks are fired for
            // each resource. We determine whether they are applicable to the
            // test case when they run.

            // Process the callback for the OutputVector resource.
            if (_stricmp(Name, "OutputVector") == 0) {
              // We only need to set the compiler options string once. So this
              // is a convenient place to do it.
              ShaderOp->Shaders.at(0).Arguments = CompilerOptionsString.c_str();

              return;
            }

            // Process the callback for the InputVector[1-3] resources
            for (size_t I = 0; I < 3; ++I) {
              std::string BufferName = "InputVector";
              BufferName += (char)('1' + I);
              if (_stricmp(Name, BufferName.c_str()) == 0) {
                if (I < Operation.Arity)
                  fillShaderBufferFromLongVectorData(ShaderData, Inputs[I]);
                return;
              }
            }

            LOG_ERROR_FMT_THROW(
                L"RunShaderOpTest CallBack. Unexpected Resource Name: %S",
                Name);
          },
          std::move(ShaderOpSet));

  // Extract the data from the shader result
  MappedData ShaderOutData;

  char *ReadBackName = "OutputVector";
  TestResult->Test->GetReadBackData(ReadBackName, &ShaderOutData);

  std::vector<OUT_TYPE> OutData;
  fillLongVectorDataFromShaderBuffer(ShaderOutData, OutData,
                                     ExpectedOutputSize);

  return OutData;
}

// LoadAndStore operations dynamically configure the UAV/SRV formats and sizes
// based on the vector size and data type. We also adjust the format and flags
// based on whether we're using raw buffers or structured buffers.
void configureLoadAndStoreShaderOp(const Operation &Operation,
                                   const DataType &OpDataType,
                                   size_t VectorSize, size_t ElementSize,
                                   st::ShaderOpSet *ShaderOpSet) {

  DXASSERT_NOMSG(LoadAndStoreOpTypes.count(Operation.Type) > 0);

  st::ShaderOp *ShaderOp = ShaderOpSet->GetShaderOp(Operation.ShaderName);
  DXASSERT(ShaderOp, "Invalid ShaderOp name");

  // When using DXGI_FORMAT_R32_TYPELESS (raw buffer cases) we need to compute
  // the number of 32-bit elements required to hold the vector.
  const UINT Num32BitElements =
      static_cast<UINT>((VectorSize * OpDataType.HLSLSizeInBytes + 3) / 4);

  const UINT StructureByteStride = static_cast<UINT>(ElementSize * VectorSize);

  const bool IsSB = IsStructuredBufferLoadAndStoreOp(Operation.Type);
  if (!ShaderOp->DescriptorHeaps.empty()) {
    DXASSERT(ShaderOp->DescriptorHeaps.size() == 1,
             "Programmer error: Expecting a single descriptor heap for "
             "LoadAndStore tests");

    for (auto &D : ShaderOp->DescriptorHeaps[0].Descriptors) {
      const bool IsUAV = (_stricmp(D.Kind, "UAV") == 0);
      DXASSERT(IsUAV || (_stricmp(D.Kind, "SRV") == 0),
               "Programmer error: Expecting UAV or SRV descriptors only");

      if (IsSB) {
        if (IsUAV) {
          D.UavDesc.Format = DXGI_FORMAT_UNKNOWN;
          D.UavDesc.Buffer.NumElements = 1; // One StructuredBuffer
          D.UavDesc.Buffer.StructureByteStride = StructureByteStride;
        } else {
          D.SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
          D.SrvDesc.Buffer.NumElements = 1; // One StructuredBuffer
          D.SrvDesc.Buffer.StructureByteStride = StructureByteStride;
        }
      } else { // Raw buffer
        if (IsUAV) {
          D.UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
          D.UavDesc.Buffer.NumElements = Num32BitElements;
          D.UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        } else {
          D.SrvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
          D.SrvDesc.Buffer.NumElements = Num32BitElements;
          D.SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        }
      }
    }
  }

  const UINT BufferWidth = IsSB ? StructureByteStride : (Num32BitElements * 4);
  for (auto &R : ShaderOp->Resources)
    R.Desc.Width = BufferWidth;
}

template <typename T>
std::vector<T> buildTestInput(InputSet InputSet, size_t SizeToTest) {
  const std::vector<T> &RawValueSet = getInputSet<T>(InputSet);

  std::vector<T> ValueSet;
  ValueSet.reserve(SizeToTest);
  for (size_t I = 0; I < SizeToTest; ++I)
    ValueSet.push_back(RawValueSet[I % RawValueSet.size()]);

  return ValueSet;
}

template <typename T>
InputSets<T> buildTestInputs(size_t VectorSize, const InputSet OpInputSets[3],
                             size_t Arity) {
  InputSets<T> Inputs;

  for (size_t I = 0; I < Arity; ++I)
    Inputs.push_back(buildTestInput<T>(OpInputSets[I], VectorSize));

  return Inputs;
}

struct ValidationConfig {
  double Tolerance = 0.0;
  ValidationType Type = ValidationType::Epsilon;

  static ValidationConfig Epsilon(double Tolerance) {
    return ValidationConfig{Tolerance, ValidationType::Epsilon};
  }

  static ValidationConfig Ulp(double Tolerance) {
    return ValidationConfig{Tolerance, ValidationType::Ulp};
  }
};

template <typename T, typename OUT_TYPE>
void runAndVerify(
    ID3D12Device *D3DDevice, bool VerboseLogging, const Operation &Operation,
    const InputSets<T> &Inputs, const std::vector<OUT_TYPE> &Expected,
    const ValidationConfig &ValidationConfig,
    std::optional<std::string> AdditionalCompilerOptions = std::nullopt) {

  std::optional<std::vector<OUT_TYPE>> Actual =
      runTest<OUT_TYPE>(D3DDevice, VerboseLogging, Operation, Inputs,
                        Expected.size(), AdditionalCompilerOptions);

  // If the test didn't run, don't verify anything.
  if (!Actual)
    return;

  VERIFY_IS_TRUE(doVectorsMatch(*Actual, Expected, ValidationConfig.Tolerance,
                                ValidationConfig.Type, VerboseLogging));
}

//
// Op definitions.  The main goal of this is to specify the validation
// configuration and how to build the Expected results for a given Op.
//
// Most Ops have a 1:1 mapping of input to output, and so can use the generic
// ExpectedBuilder.
//
// Ops that differ from this pattern can specialize ExpectedBuilder as
// necessary.
//

// Op - specializations are expected to have a ValidationConfig member and an
// appropriate overloaded function call operator.
template <OpType OP, typename T, size_t Arity> struct Op;

// ExpectedBuilder - specializations are expected to have buildExpectedData
// member functions.
template <OpType OP, typename T> struct ExpectedBuilder;

// Default Validation configuration - ULP for floating point types, exact
// matches for everything else.
template <typename T> struct DefaultValidation {
  ValidationConfig ValidationConfig;

  DefaultValidation() {
    if constexpr (isFloatingPointType<T>())
      ValidationConfig = ValidationConfig::Ulp(1.0f);
  }
};

// Strict Validation - Defaults to exact matches.
// Tolerance can be set to a non-zero value to allow for a wider range.
struct StrictValidation {
  ValidationConfig ValidationConfig;
};

// Macros to build up common patterns of Op definitions

#define OP_1(OP, VALIDATION, IMPL)                                             \
  template <typename T> struct Op<OP, T, 1> : VALIDATION {                     \
    T operator()(T A) { return IMPL; }                                         \
  }

#define OP_2(OP, VALIDATION, IMPL)                                             \
  template <typename T> struct Op<OP, T, 2> : VALIDATION {                     \
    T operator()(T A, T B) { return IMPL; }                                    \
  }

#define OP_3(OP, VALIDATION, IMPL)                                             \
  template <typename T> struct Op<OP, T, 3> : VALIDATION {                     \
    T operator()(T A, T B, T C) { return IMPL; }                               \
  }

#define STRICT_OP_1(OP, IMPL) OP_1(OP, StrictValidation, IMPL)

#define DEFAULT_OP_1(OP, IMPL) OP_1(OP, DefaultValidation<T>, IMPL)
#define DEFAULT_OP_2(OP, IMPL) OP_2(OP, DefaultValidation<T>, IMPL)
#define DEFAULT_OP_3(OP, IMPL) OP_3(OP, DefaultValidation<T>, IMPL)

//
// TernaryMath
//

DEFAULT_OP_3(OpType::Mad, (A * B + C));
DEFAULT_OP_3(OpType::Fma, (A * B + C));

//
// BinaryMath
//

DEFAULT_OP_2(OpType::Add, (A + B));
DEFAULT_OP_2(OpType::Subtract, (A - B));
DEFAULT_OP_2(OpType::Multiply, (A * B));
DEFAULT_OP_2(OpType::Divide, (A / B));

template <typename T> struct Op<OpType::Modulus, T, 2> : DefaultValidation<T> {
  T operator()(T A, T B) {
    if constexpr (std::is_same_v<T, float>)
      return std::fmod(A, B);
    else
      return A % B;
  }
};

DEFAULT_OP_2(OpType::Min, (std::min(A, B)));
DEFAULT_OP_2(OpType::Max, (std::max(A, B)));
DEFAULT_OP_2(OpType::Ldexp, (A * static_cast<T>(std::pow(2.0f, B))));

//
// Bitwise
//

template <typename T> T Saturate(T A) {
  if (A < static_cast<T>(0.0f))
    return static_cast<T>(0.0f);
  if (A > static_cast<T>(1.0f))
    return static_cast<T>(1.0f);
  return A;
}

template <typename T> T ReverseBits(T A) {
  T Result = 0;
  const size_t NumBits = sizeof(T) * 8;
  for (size_t I = 0; I < NumBits; I++) {
    Result <<= 1;
    Result |= (A & 1);
    A >>= 1;
  }
  return Result;
}

template <typename T> uint32_t CountBits(T A) {
  return static_cast<uint32_t>(std::bitset<sizeof(T) * 8>(A).count());
}

// General purpose bit scan from the MSB. Based on the value of LookingForZero
// returns the index of the first high/low bit found.
template <typename T> uint32_t ScanFromMSB(T A, bool LookingForZero) {
  if (A == 0)
    return std::numeric_limits<uint32_t>::max();

  constexpr uint32_t NumBits = sizeof(T) * 8;
  for (int32_t I = NumBits - 1; I >= 0; --I) {
    bool BitSet = (A & (static_cast<T>(1) << I)) != 0;
    if (BitSet != LookingForZero)
      return static_cast<uint32_t>(I);
  }
  return std::numeric_limits<uint32_t>::max();
}

template <typename T>
typename std::enable_if<std::is_signed<T>::value, uint32_t>::type
FirstBitHigh(T A) {
  const bool IsNegative = A < 0;
  return ScanFromMSB(A, IsNegative);
}

template <typename T>
typename std::enable_if<!std::is_signed<T>::value, uint32_t>::type
FirstBitHigh(T A) {
  return ScanFromMSB(A, false);
}

template <typename T> uint32_t FirstBitLow(T A) {
  const uint32_t NumBits = sizeof(T) * 8;

  if (A == 0)
    return std::numeric_limits<uint32_t>::max();

  for (uint32_t I = 0; I < NumBits; ++I) {
    if (A & (static_cast<T>(1) << I))
      return static_cast<T>(I);
  }

  return std::numeric_limits<uint32_t>::max();
}

DEFAULT_OP_2(OpType::And, (A & B));
DEFAULT_OP_2(OpType::Or, (A | B));
DEFAULT_OP_2(OpType::Xor, (A ^ B));

// HLSL/DXIL masks shift amounts to the low bits (4 bits for 16-bit, 5 bits for
// 32-bit, 6 bits for 64-bit). We must do the same in C++ to avoid undefined
// behavior when shift amount >= bit width, and to match GPU results.
template <typename T> T MaskShiftAmount(T ShiftAmount) {
  constexpr T ShiftMask = static_cast<T>(sizeof(T) * 8 - 1);
  return ShiftAmount & ShiftMask;
}

DEFAULT_OP_2(OpType::LeftShift, (A << MaskShiftAmount(B)));
DEFAULT_OP_2(OpType::RightShift, (A >> MaskShiftAmount(B)));
DEFAULT_OP_1(OpType::Saturate, (Saturate(A)));
DEFAULT_OP_1(OpType::ReverseBits, (ReverseBits(A)));

#define BITWISE_OP(OP, IMPL)                                                   \
  template <typename T> struct Op<OP, T, 1> : StrictValidation {               \
    uint32_t operator()(T A) { return IMPL; }                                  \
  }

BITWISE_OP(OpType::CountBits, (CountBits(A)));
BITWISE_OP(OpType::FirstBitHigh, (FirstBitHigh(A)));
BITWISE_OP(OpType::FirstBitLow, (FirstBitLow(A)));

#undef BITWISE_OP

//
// Unary
//

DEFAULT_OP_1(OpType::Initialize, (A));

template <typename T>
struct Op<OpType::ArrayOperator_StaticAccess, T, 1> : DefaultValidation<T> {};

template <typename T>
static std::vector<T> buildExpectedArrayAccess(const InputSets<T> &Inputs) {
  const size_t VectorSize = Inputs[0].size();
  std::vector<T> Expected;
  const size_t IndexCount = 6;
  Expected.resize(VectorSize);

  size_t IndexList[IndexCount] = {
      0, VectorSize - 1, 1, VectorSize - 2, VectorSize / 2, VectorSize / 2 + 1};
  size_t End = std::min(VectorSize, IndexCount);
  for (size_t I = 0; I < End; ++I)
    Expected[IndexList[I]] = Inputs[0][IndexList[I]];

  return Expected;
}

template <typename T>
struct ExpectedBuilder<OpType::ArrayOperator_StaticAccess, T> {
  static std::vector<T>
  buildExpected(Op<OpType::ArrayOperator_StaticAccess, T, 1>,
                const InputSets<T> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 1);
    return buildExpectedArrayAccess(Inputs);
  }
};

template <typename T>
struct Op<OpType::ArrayOperator_DynamicAccess, T, 2> : DefaultValidation<T> {};

template <typename T>
struct ExpectedBuilder<OpType::ArrayOperator_DynamicAccess, T> {
  static std::vector<T>
  buildExpected(Op<OpType::ArrayOperator_DynamicAccess, T, 2>,
                const InputSets<T> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 2);
    return buildExpectedArrayAccess(Inputs);
  }
};

//
// Cast
//

#define CAST_OP(OP, TYPE, IMPL)                                                \
  template <typename T> struct Op<OP, T, 1> : StrictValidation {               \
    TYPE operator()(T A) { return IMPL; }                                      \
  };

template <typename T> HLSLBool_t CastToBool(T A) { return (bool)A; }
template <> HLSLBool_t CastToBool(HLSLHalf_t A) { return (bool)((float)A); }

template <typename T> HLSLHalf_t CastToFloat16(T A) {
  return HLSLHalf_t(float(A));
}

template <typename T> float CastToFloat32(T A) { return (float)A; }

template <typename T> double CastToFloat64(T A) { return (double)A; }
template <> double CastToFloat64(HLSLHalf_t A) { return (double)((float)A); }

template <typename T> int16_t CastToInt16(T A) { return (int16_t)A; }
template <> int16_t CastToInt16(HLSLHalf_t A) { return (int16_t)((float)A); }

template <typename T> int32_t CastToInt32(T A) { return (int32_t)A; }
template <> int32_t CastToInt32(HLSLHalf_t A) { return (int32_t)((float)A); }

template <typename T> int64_t CastToInt64(T A) { return (int64_t)A; }
template <> int64_t CastToInt64(HLSLHalf_t A) { return (int64_t)((float)A); }

template <typename T> uint16_t CastToUint16(T A) { return (uint16_t)A; }
template <> uint16_t CastToUint16(HLSLHalf_t A) { return (uint16_t)((float)A); }

template <typename T> uint32_t CastToUint32(T A) { return (uint32_t)A; }
template <> uint32_t CastToUint32(HLSLHalf_t A) { return (uint32_t)((float)A); }

template <typename T> uint64_t CastToUint64(T A) { return (uint64_t)A; }
template <> uint64_t CastToUint64(HLSLHalf_t A) { return (uint64_t)((float)A); }

CAST_OP(OpType::CastToBool, HLSLBool_t, (CastToBool(A)));
CAST_OP(OpType::CastToInt16, int16_t, (CastToInt16(A)));
CAST_OP(OpType::CastToInt32, int32_t, (CastToInt32(A)));
CAST_OP(OpType::CastToInt64, int64_t, (CastToInt64(A)));
CAST_OP(OpType::CastToUint16, uint16_t, (CastToUint16(A)));
CAST_OP(OpType::CastToUint32, uint32_t, (CastToUint32(A)));
CAST_OP(OpType::CastToUint64, uint64_t, (CastToUint64(A)));
CAST_OP(OpType::CastToUint16_FromFP, uint16_t, (CastToUint16(A)));
CAST_OP(OpType::CastToUint32_FromFP, uint32_t, (CastToUint32(A)));
CAST_OP(OpType::CastToUint64_FromFP, uint64_t, (CastToUint64(A)));
CAST_OP(OpType::CastToFloat16, HLSLHalf_t, (CastToFloat16(A)));
CAST_OP(OpType::CastToFloat32, float, (CastToFloat32(A)));
CAST_OP(OpType::CastToFloat64, double, (CastToFloat64(A)));

#undef CAST_OP

//
// Trigonometric
//

// All trigonometric ops are floating point types. These trig functions are
// defined to have a max absolute error of 0.0008 as per the D3D functional
// specs. An example with this spec for sin and cos is available here:
// https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#22.10.20

template <typename T, OpType OP> struct TrigonometricValidation {
  ValidationConfig ValidationConfig = ValidationConfig::Epsilon(0.0008f);
};

// Half precision trig functions have a larger tolerance due to their lower
// precision. Note that the D3D spec
// does not mention half precision trig functions.
template <OpType OP> struct TrigonometricValidation<HLSLHalf_t, OP> {
  ValidationConfig ValidationConfig = ValidationConfig::Epsilon(0.003f);
};

// For the half precision trig functions with an infinite range in either
// direction we use 2 ULPs of tolerance instead.
template <> struct TrigonometricValidation<HLSLHalf_t, OpType::Cosh> {
  ValidationConfig ValidationConfig = ValidationConfig::Ulp(2.0f);
};

template <> struct TrigonometricValidation<HLSLHalf_t, OpType::Tan> {
  ValidationConfig ValidationConfig = ValidationConfig::Ulp(2.0f);
};

template <> struct TrigonometricValidation<HLSLHalf_t, OpType::Sinh> {
  ValidationConfig ValidationConfig = ValidationConfig::Ulp(2.0f);
};

// Min precision trig tolerances: same as half precision since min precision
// is at least 16-bit and our doValuesMatch compares in half-precision space.
template <OpType OP> struct TrigonometricValidation<HLSLMin16Float_t, OP> {
  ValidationConfig ValidationConfig = ValidationConfig::Epsilon(0.003f);
};

template <> struct TrigonometricValidation<HLSLMin16Float_t, OpType::Cosh> {
  ValidationConfig ValidationConfig = ValidationConfig::Ulp(2.0f);
};

template <> struct TrigonometricValidation<HLSLMin16Float_t, OpType::Tan> {
  ValidationConfig ValidationConfig = ValidationConfig::Ulp(2.0f);
};

template <> struct TrigonometricValidation<HLSLMin16Float_t, OpType::Sinh> {
  ValidationConfig ValidationConfig = ValidationConfig::Ulp(2.0f);
};

#define TRIG_OP(OP, IMPL)                                                      \
  template <typename T> struct Op<OP, T, 1> : TrigonometricValidation<T, OP> { \
    T operator()(T A) { return IMPL; }                                         \
  }

TRIG_OP(OpType::Acos, (std::acos(A)));
TRIG_OP(OpType::Asin, (std::asin(A)));
TRIG_OP(OpType::Atan, (std::atan(A)));
TRIG_OP(OpType::Cos, (std::cos(A)));
TRIG_OP(OpType::Cosh, (std::cosh(A)));
TRIG_OP(OpType::Sin, (std::sin(A)));
TRIG_OP(OpType::Sinh, (std::sinh(A)));
TRIG_OP(OpType::Tan, (std::tan(A)));
TRIG_OP(OpType::Tanh, (std::tanh(A)));

#undef TRIG_OP

//
// AsType
//

// We don't have std::bit_cast in C++17, so we define our own version.
template <typename ToT, typename FromT>
typename std::enable_if<sizeof(ToT) == sizeof(FromT) &&
                            std::is_trivially_copyable<FromT>::value &&
                            std::is_trivially_copyable<ToT>::value,
                        ToT>::type
bit_cast(const FromT &Src) {
  ToT Dst;
  std::memcpy(&Dst, &Src, sizeof(ToT));
  return Dst;
}

#define AS_TYPE_OP(OP, TYPE, IMPL)                                             \
  template <typename T> struct Op<OP, T, 1> : StrictValidation {               \
    TYPE operator()(T A) { return IMPL; }                                      \
  };

// asFloat16

template <typename T> HLSLHalf_t asFloat16(T);
template <> HLSLHalf_t asFloat16(HLSLHalf_t A) { return A; }
template <> HLSLHalf_t asFloat16(int16_t A) {
  return HLSLHalf_t::FromHALF(bit_cast<DirectX::PackedVector::HALF>(A));
}
template <> HLSLHalf_t asFloat16(uint16_t A) {
  return HLSLHalf_t::FromHALF(bit_cast<DirectX::PackedVector::HALF>(A));
}

AS_TYPE_OP(OpType::AsFloat16, HLSLHalf_t, (asFloat16(A)));

// asInt16

template <typename T> int16_t asInt16(T);
template <> int16_t asInt16(HLSLHalf_t A) { return bit_cast<int16_t>(A.Val); }
template <> int16_t asInt16(int16_t A) { return A; }
template <> int16_t asInt16(uint16_t A) { return bit_cast<int16_t>(A); }

AS_TYPE_OP(OpType::AsInt16, int16_t, (asInt16(A)));

// asUint16

template <typename T> uint16_t asUint16(T);
template <> uint16_t asUint16<HLSLHalf_t>(HLSLHalf_t A) {
  return bit_cast<uint16_t>(A.Val);
}
template <> uint16_t asUint16(uint16_t A) { return A; }
template <> uint16_t asUint16(int16_t A) { return bit_cast<uint16_t>(A); }

AS_TYPE_OP(OpType::AsUint16, uint16_t, (asUint16(A)));

// asFloat

template <typename T> float asFloat(T);
template <> float asFloat(float A) { return float(A); }
template <> float asFloat(int32_t A) { return bit_cast<float>(A); }
template <> float asFloat(uint32_t A) { return bit_cast<float>(A); }

AS_TYPE_OP(OpType::AsFloat, float, (asFloat(A)));

// asInt

template <typename T> int32_t asInt(T);
template <> int32_t asInt(float A) { return bit_cast<int32_t>(A); }
template <> int32_t asInt(int32_t A) { return A; }
template <> int32_t asInt(uint32_t A) { return bit_cast<int32_t>(A); }

AS_TYPE_OP(OpType::AsInt, int32_t, (asInt(A)));

// asUint

template <typename T> unsigned int asUint(T);
template <> unsigned int asUint(unsigned int A) { return A; }
template <> unsigned int asUint(float A) { return bit_cast<unsigned int>(A); }
template <> unsigned int asUint(int A) { return bit_cast<unsigned int>(A); }

AS_TYPE_OP(OpType::AsUint, uint32_t, (asUint(A)));

// asDouble

template <> struct Op<OpType::AsDouble, uint32_t, 2> : StrictValidation {
  double operator()(uint32_t LowBits, uint32_t HighBits) {
    uint64_t Bits = (static_cast<uint64_t>(HighBits) << 32) | LowBits;
    double Result;
    std::memcpy(&Result, &Bits, sizeof(Result));
    return Result;
  }
};

// splitDouble
//
// splitdouble is special because it's a function that takes a double and
// outputs two values. To handle this special case we override various bits of
// the testing machinary.

template <>
struct Op<OpType::AsUint_SplitDouble, double, 1> : StrictValidation {};

// Specialized version of ExpectedBuilder for the splitdouble case. The
// expected output for this has all the Low values followed by all the High
// values.
template <> struct ExpectedBuilder<OpType::AsUint_SplitDouble, double> {
  static std::vector<uint32_t>
  buildExpected(Op<OpType::AsUint_SplitDouble, double, 1> &,
                const InputSets<double> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    size_t VectorSize = Inputs[0].size();

    std::vector<uint32_t> Expected;
    Expected.resize(VectorSize * 2);

    for (size_t I = 0; I < VectorSize; ++I) {
      uint32_t Low, High;
      splitDouble(Inputs[0][I], Low, High);
      Expected[I] = Low;
      Expected[I + VectorSize] = High;
    }

    return Expected;
  }

  static void splitDouble(const double A, uint32_t &LowBits,
                          uint32_t &HighBits) {
    uint64_t Bits = 0;
    std::memcpy(&Bits, &A, sizeof(Bits));
    LowBits = static_cast<uint32_t>(Bits & 0xFFFFFFFF);
    HighBits = static_cast<uint32_t>(Bits >> 32);
  }
};

//
// Unary Math
//

template <typename T> T UnaryMathAbs(T A) {
  if constexpr (std::is_unsigned_v<T> || std::is_same_v<T, HLSLMin16Uint_t>)
    return A;
  else
    return static_cast<T>(std::abs(A));
}

DEFAULT_OP_1(OpType::Abs, (UnaryMathAbs(A)));

// Sign is special because the return type doesn't match the input type.
template <typename T> struct Op<OpType::Sign, T, 1> : DefaultValidation<T> {
  int32_t operator()(T A) {
    const T Zero = T();

    if (A > Zero)
      return 1;
    if (A < Zero)
      return -1;
    return 0;
  }
};

DEFAULT_OP_1(OpType::Ceil, (std::ceil(A)));
DEFAULT_OP_1(OpType::Exp, (std::exp(A)));
DEFAULT_OP_1(OpType::Floor, (std::floor(A)));
DEFAULT_OP_1(OpType::Frac, (A - static_cast<T>(std::floor(A))));
DEFAULT_OP_1(OpType::Log, (std::log(A)));
DEFAULT_OP_1(OpType::Rcp, (static_cast<T>(1.0) / A));
DEFAULT_OP_1(OpType::Round, (std::round(A)));
DEFAULT_OP_1(OpType::Rsqrt,
             (static_cast<T>(1.0) / static_cast<T>(std::sqrt(A))));
DEFAULT_OP_1(OpType::Sqrt, (std::sqrt(A)));
DEFAULT_OP_1(OpType::Trunc, (std::trunc(A)));
DEFAULT_OP_1(OpType::Exp2, (std::exp2(A)));
DEFAULT_OP_1(OpType::Log10, (std::log10(A)));
DEFAULT_OP_1(OpType::Log2, (std::log2(A)));

// Frexp has a return value as well as an output paramater. So we handle it
// with special logic. Frexp is only supported for fp32 values.
template <> struct Op<OpType::Frexp, float, 1> : DefaultValidation<float> {};

template <> struct ExpectedBuilder<OpType::Frexp, float> {
  static std::vector<float> buildExpected(Op<OpType::Frexp, float, 1> &,
                                          const InputSets<float> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    // Expected values size is doubled. In the first half we store the
    // Mantissas and in the second half we store the Exponents. This way we
    // can leverage the existing logic which verify expected values in a
    // single vector. We just need to make sure that we organize the output in
    // the same way in the shader and when we read it back.

    size_t VectorSize = Inputs[0].size();

    std::vector<float> Expected;
    Expected.resize(VectorSize * 2);

    for (size_t I = 0; I < VectorSize; ++I) {
      int Exp = 0;
      float Man = std::frexp(Inputs[0][I], &Exp);

      // std::frexp returns a signed mantissa. But the HLSL implmentation
      // returns an unsigned mantissa.
      Man = std::abs(Man);

      Expected[I] = Man;

      // std::frexp returns the exponent as an int, but HLSL stores it as a
      // float. However, the HLSL exponents fractional component is always 0.
      // So it can conversion between float and int is safe.
      Expected[I + VectorSize] = static_cast<float>(Exp);
    }

    return Expected;
  }
};

//
// Binary Comparison
//

#define BINARY_COMPARISON_OP(OP, IMPL)                                         \
  template <typename T> struct Op<OP, T, 2> : StrictValidation {               \
    HLSLBool_t operator()(T A, T B) { return IMPL; }                           \
  };

BINARY_COMPARISON_OP(OpType::LessThan, (A < B));
BINARY_COMPARISON_OP(OpType::LessEqual, (A <= B));
BINARY_COMPARISON_OP(OpType::GreaterThan, (A > B));
BINARY_COMPARISON_OP(OpType::GreaterEqual, (A >= B));
BINARY_COMPARISON_OP(OpType::Equal, (A == B));
BINARY_COMPARISON_OP(OpType::NotEqual, (A != B));

//
// Binary Logical
//

DEFAULT_OP_2(OpType::Logical_And, (A && B));
DEFAULT_OP_2(OpType::Logical_Or, (A || B));

// Ternary Logical
//

OP_3(OpType::Select, StrictValidation, (static_cast<bool>(A) ? B : C));

//
// Reduction
//

#define REDUCTION_OP(OP, STDFUNC)                                              \
  template <typename T> struct Op<OP, T, 1> : StrictValidation {};             \
  template <typename T> struct ExpectedBuilder<OP, T> {                        \
    static std::vector<HLSLBool_t> buildExpected(Op<OP, T, 1> &,               \
                                                 const InputSets<T> &Inputs) { \
      const bool Res = STDFUNC(Inputs[0].begin(), Inputs[0].end(),             \
                               [](T A) { return A != static_cast<T>(0); });    \
      return std::vector<HLSLBool_t>{Res};                                     \
    }                                                                          \
  };

REDUCTION_OP(OpType::Any_Mixed, (std::any_of));
REDUCTION_OP(OpType::Any_NoZero, (std::any_of));
REDUCTION_OP(OpType::Any_Zero, (std::any_of));

REDUCTION_OP(OpType::All_Mixed, (std::all_of));
REDUCTION_OP(OpType::All_NoZero, (std::all_of));
REDUCTION_OP(OpType::All_Zero, (std::all_of));

#undef REDUCTION_OP

template <typename T> struct Op<OpType::Dot, T, 2> : StrictValidation {};
template <typename T> struct ExpectedBuilder<OpType::Dot, T> {
  // For Dot, buildExpected is a special case: it also computes an absolute
  // epsilon for validation because Dot is a compound operation. Expected value
  // is computed by multiplying and accumulating in fp64 for higher precision.
  // Absolute epsilon is computed by reordering the accumulation into a
  // worst-case sequence, then summing the per-step epsilons to produce a
  // conservative error tolerance for the entire Dot operation.
  static std::vector<T> buildExpected(Op<OpType::Dot, T, 2> &Op,
                                      const InputSets<T> &Inputs) {

    std::vector<double> PositiveProducts;
    std::vector<double> NegativeProducts;

    const size_t VectorSize = Inputs[0].size();

    // Floating point ops have a tolerance of 0.5 ULPs per operation as per the
    // DX spec.
    const double ULPTolerance = 0.5;

    // Accumulate in fp64 to improve precision.
    double DotProduct = 0.0;      // computed reference result
    double AbsoluteEpsilon = 0.0; // computed tolerance
    for (size_t I = 0; I < VectorSize; ++I) {
      double Product = Inputs[0][I] * Inputs[1][I];
      AbsoluteEpsilon += computeAbsoluteEpsilon<T>(Product, ULPTolerance);

      DotProduct += Product;

      if (Product >= 0.0)
        PositiveProducts.push_back(Product);
      else
        NegativeProducts.push_back(Product);
    }

    // Sort each by magnitude so that we can accumulate them in worst case
    // order.
    std::sort(PositiveProducts.begin(), PositiveProducts.end(),
              std::greater<double>());
    std::sort(NegativeProducts.begin(), NegativeProducts.end());

    // Helper to sum the products and compute/add to the running absolute
    // epsilon total.
    auto SumProducts = [&AbsoluteEpsilon,
                        ULPTolerance](const std::vector<double> &Values) {
      double Sum = Values.empty() ? 0.0 : Values[0];
      for (size_t I = 1; I < Values.size(); ++I) {
        Sum += Values[I];
        AbsoluteEpsilon += computeAbsoluteEpsilon<T>(Sum, ULPTolerance);
      }
      return Sum;
    };

    // Accumulate products in the worst case order while computing the absolute
    // epsilon error for each intermediate step. And accumulate that error.
    const double SumPos = SumProducts(PositiveProducts);
    const double SumNeg = SumProducts(NegativeProducts);

    if (!PositiveProducts.empty() && !NegativeProducts.empty())
      AbsoluteEpsilon +=
          computeAbsoluteEpsilon<T>((SumPos + SumNeg), ULPTolerance);

    Op.ValidationConfig = ValidationConfig::Epsilon(AbsoluteEpsilon);

    std::vector<T> Expected;
    Expected.push_back(static_cast<T>(DotProduct));
    return Expected;
  }
};

template <typename T>
static double computeAbsoluteEpsilon(double A, double ULPTolerance) {
  DXASSERT((!isinf(A) && !isnan(A)),
           "Input values should not produce inf or nan results");

  // ULP is a positive value by definition. So, working with abs(A) simplifies
  // our logic for computing ULP in the first place.
  A = std::abs(A);

  double ULP = 0.0;

  if constexpr (std::is_same_v<T, HLSLHalf_t>)
    ULP = HLSLHalf_t::GetULP(A);
  else if constexpr (std::is_same_v<T, HLSLMin16Float_t>) {
    // Min precision floats may be computed at float16 on the GPU, so use
    // half-precision ULP for tolerance. Reuse HLSLHalf_t::GetULP which
    // computes ULP by incrementing the float16 bit representation.
    ULP = HLSLHalf_t::GetULP(HLSLHalf_t(static_cast<float>(A)));
  } else
    ULP =
        std::nextafter(static_cast<T>(A), std::numeric_limits<T>::infinity()) -
        static_cast<T>(A);

  return ULP * ULPTolerance;
}

template <typename T>
struct Op<OpType::ShuffleVector, T, 1> : DefaultValidation<T> {};
template <typename T> struct ExpectedBuilder<OpType::ShuffleVector, T> {
  static std::vector<T> buildExpected(Op<OpType::ShuffleVector, T, 1>,
                                      const InputSets<T> &Inputs) {
    std::vector<T> Expected(Inputs[0].size(), Inputs[0][0]);
    return Expected;
  }
};

//
// Loading and Storing of Buffers
//

STRICT_OP_1(OpType::LoadAndStore_RDH_BAB_UAV, (A));
STRICT_OP_1(OpType::LoadAndStore_RDH_BAB_SRV, (A));
STRICT_OP_1(OpType::LoadAndStore_DT_BAB_UAV, (A));
STRICT_OP_1(OpType::LoadAndStore_DT_BAB_SRV, (A));
STRICT_OP_1(OpType::LoadAndStore_RD_BAB_UAV, (A));
STRICT_OP_1(OpType::LoadAndStore_RD_BAB_SRV, (A));
STRICT_OP_1(OpType::LoadAndStore_RDH_SB_UAV, (A));
STRICT_OP_1(OpType::LoadAndStore_RDH_SB_SRV, (A));
STRICT_OP_1(OpType::LoadAndStore_DT_SB_UAV, (A));
STRICT_OP_1(OpType::LoadAndStore_DT_SB_SRV, (A));
STRICT_OP_1(OpType::LoadAndStore_RD_SB_UAV, (A));
STRICT_OP_1(OpType::LoadAndStore_RD_SB_SRV, (A));

//
// Float Ops
//

#define FLOAT_SPECIAL_OP(OP, IMPL)                                             \
  template <typename T> struct Op<OP, T, 1> : StrictValidation {               \
    HLSLBool_t operator()(T A) { return IMPL; }                                \
  };

FLOAT_SPECIAL_OP(OpType::IsFinite, (std::isfinite(A)));
FLOAT_SPECIAL_OP(OpType::IsInf, (std::isinf(A)));
FLOAT_SPECIAL_OP(OpType::IsNan, (std::isnan(A)));
#undef FLOAT_SPECIAL_OP

template <typename T> struct Op<OpType::ModF, T, 1> : DefaultValidation<T> {};

template <typename T> static T modF(T Input, T &OutParam);

template <> float modF(float Input, float &OutParam) {
  return std::modf(Input, &OutParam);
}

template <> HLSLHalf_t modF(HLSLHalf_t Input, HLSLHalf_t &OutParam) {
  float Exp = 0.0f;
  float Man = std::modf(float(Input), &Exp);
  OutParam = HLSLHalf_t(Exp);
  return Man;
}

template <typename T> struct ExpectedBuilder<OpType::ModF, T> {
  static std::vector<T> buildExpected(Op<OpType::ModF, T, 1> &,
                                      const InputSets<T> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 1);
    size_t VectorSize = Inputs[0].size();

    std::vector<T> Expected;
    Expected.resize(VectorSize * 2);

    for (size_t I = 0; I < VectorSize; ++I) {
      T Exp;
      T Man = modF(Inputs[0][I], Exp);
      Expected[I] = Man;
      Expected[I + VectorSize] = Exp;
    }

    return Expected;
  }
};

//
// Derivative Ops
//

// Coarse derivatives (ddx/ddy): All lanes in quad get same result
// Fine derivatives (ddx_fine/ddy_fine): Each lane gets unique result
// For testing, we validate results on lane 3 to keep validation generic
//
// The value of A in each lane is computed by : A = A + LaneID*2
//
// Top right (lane 1) - Top Left (lane 0)
DEFAULT_OP_1(OpType::DerivativeDdx, ((A + T(2.0f)) - (A + T(0.0f))));
// Lower left (lane 2) -  Top Left (lane 0)
DEFAULT_OP_1(OpType::DerivativeDdy, ((A + T(4.0f)) - (A + T(0.0f))));

// Bottom right (lane 3) - Bottom left (lane 2)
DEFAULT_OP_1(OpType::DerivativeDdxFine, ((A + T(6.0f)) - (A + T(4.0f))));
// Bottom right (lane 3) - Top right (lane 1)
DEFAULT_OP_1(OpType::DerivativeDdyFine, ((A + T(6.0f)) - (A + T(2.0f))));

//
// Quad Read Ops
//

// We keep things generic so we can re-use this macro for all quad ops.
// The lane we write to is determined via a defines in the shader code.
// See TestQuadRead in ShaderOpArith.xml.
// For all cases we simply fill the vector on that lane with the value of the
// third element.
#define QUAD_READ_OP(OP, ARITY)                                                \
  template <typename T> struct Op<OP, T, ARITY> : DefaultValidation<T> {};     \
  template <typename T> struct ExpectedBuilder<OP, T> {                        \
    static std::vector<T> buildExpected(Op<OP, T, ARITY> &,                    \
                                        const InputSets<T> &Inputs) {          \
      DXASSERT_NOMSG(Inputs.size() == ARITY);                                  \
      std::vector<T> Expected;                                                 \
      const size_t VectorSize = Inputs[0].size();                              \
      Expected.assign(VectorSize, Inputs[0][2]);                               \
      return Expected;                                                         \
    }                                                                          \
  };

QUAD_READ_OP(OpType::QuadReadLaneAt, 2);
QUAD_READ_OP(OpType::QuadReadAcrossX, 1);
QUAD_READ_OP(OpType::QuadReadAcrossY, 1);
QUAD_READ_OP(OpType::QuadReadAcrossDiagonal, 1);

#undef QUAD_READ_OP

//
// Wave Ops
//

#define WAVE_OP(OP, IMPL)                                                      \
  template <typename T> struct Op<OP, T, 1> : DefaultValidation<T> {           \
    T operator()(T A, UINT WaveSize) { return IMPL; }                          \
  };

template <typename T> T waveActiveSum(T A, UINT WaveSize) {
  T WaveSizeT = static_cast<T>(WaveSize);
  return A * WaveSizeT;
}

WAVE_OP(OpType::WaveActiveSum, (waveActiveSum(A, WaveSize)));

template <typename T> T waveActiveMin(T A, UINT WaveSize) {
  std::vector<T> Values;
  // Add the 'WaveLaneID' to A.
  for (UINT I = 0; I < WaveSize; ++I)
    Values.push_back(A + static_cast<T>(I));
  return *std::min_element(Values.begin(), Values.end());
}

WAVE_OP(OpType::WaveActiveMin, (waveActiveMin(A, WaveSize)));

template <typename T> T waveActiveMax(T A, UINT WaveSize) {
  std::vector<T> Values;
  // Add the 'WaveLaneID' to A.
  for (UINT I = 0; I < WaveSize; ++I)
    Values.push_back(A + static_cast<T>(I));
  return *std::max_element(Values.begin(), Values.end());
}

WAVE_OP(OpType::WaveActiveMax, (waveActiveMax(A, WaveSize)));

template <typename T> T waveActiveProduct(T A, UINT WaveSize) {
  // We want to avoid overflow of a large product. So, the WaveActiveProdFn has
  // an input set of all 1's and we modify the value of the largest lane to be
  // equal to the lane index in the shader.
  return A * static_cast<T>(WaveSize - 1);
}

WAVE_OP(OpType::WaveActiveProduct, (waveActiveProduct(A, WaveSize)));

template <typename T> T waveActiveBitAnd(T A, UINT) {
  // We set the LSB to 0 in one of the lanes.
  return static_cast<T>(A & ~static_cast<T>(1));
}

WAVE_OP(OpType::WaveActiveBitAnd, (waveActiveBitAnd(A, WaveSize)));

template <typename T> T waveActiveBitOr(T A, UINT) {
  // We set the LSB to 1 in one of the lanes.
  return static_cast<T>(A | static_cast<T>(1));
}

WAVE_OP(OpType::WaveActiveBitOr, (waveActiveBitOr(A, WaveSize)));

template <typename T> T waveActiveBitXor(T A, UINT) {
  // We clear the LSB in every lane except the last lane which sets it to 1.
  return static_cast<T>(A | static_cast<T>(1));
}

WAVE_OP(OpType::WaveActiveBitXor, (waveActiveBitXor(A, WaveSize)));

WAVE_OP(OpType::WaveMultiPrefixBitAnd, waveMultiPrefixBitAnd(A, WaveSize));

template <typename T> T waveMultiPrefixBitAnd(T A, UINT) {
  // All lanes in the group mask use a mask to filter for only the second and
  // third LSBs.
  return static_cast<T>(A & static_cast<T>(0x6));
}

WAVE_OP(OpType::WaveMultiPrefixBitOr, waveMultiPrefixBitOr(A, WaveSize));

template <typename T> T waveMultiPrefixBitOr(T A, UINT) {
  // All lanes in the group mask clear the second LSB.
  return static_cast<T>(A & ~static_cast<T>(0x2));
}

template <typename T>
struct Op<OpType::WaveMultiPrefixBitXor, T, 1> : StrictValidation {};

template <typename T> struct ExpectedBuilder<OpType::WaveMultiPrefixBitXor, T> {
  static std::vector<T> buildExpected(Op<OpType::WaveMultiPrefixBitXor, T, 1> &,
                                      const InputSets<T> &Inputs, UINT) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    std::vector<T> Expected;
    const size_t VectorSize = Inputs[0].size();

    // We get a little creative for MultiPrefixBitXor. The mask we use for the
    // group in the shader is 0xE (0b1110), which includes lanes 1, 2, and 3.
    // Prefix ops don't include the value of the current lane in their result.
    // So, for this test we store the result of WaveMultiPrefixBitXor from lane
    // 3. This means only the values from lanes 1 and 2 contribute to the result
    // at lane 3.
    //
    // In the shader:
    // - Lane 0: Set to 0 (not in mask, shouldn't affect result)
    // - Lane 1: Keeps original input values
    // - Lane 2: Lower half + last element set to 0, upper half keeps input
    // - Lane 3: Stores the prefix XOR result (lanes 1 XOR lanes 2)
    //
    // Expected result: Lower half matches input (lane 1 XOR 0), upper half is
    // 0s, except last element matches input.
    for (size_t I = 0; I < VectorSize / 2; ++I)
      Expected.push_back(Inputs[0][I]);
    for (size_t I = VectorSize / 2; I < VectorSize - 1; ++I)
      Expected.push_back(0);

    // We also set the last element to 0 on lane 2 so the last element in the
    // output vector matches the last element in the input vector.
    Expected.push_back(Inputs[0][VectorSize - 1]);

    return Expected;
  }
};

template <typename T>
struct Op<OpType::WaveActiveAllEqual, T, 1> : StrictValidation {};

template <typename T> struct ExpectedBuilder<OpType::WaveActiveAllEqual, T> {
  static std::vector<HLSLBool_t>
  buildExpected(Op<OpType::WaveActiveAllEqual, T, 1> &,
                const InputSets<T> &Inputs, UINT) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    std::vector<HLSLBool_t> Expected;
    const size_t VectorSize = Inputs[0].size();
    Expected.assign(VectorSize, static_cast<HLSLBool_t>(true));
    // We set the last element to a different value on a single lane.
    Expected[VectorSize - 1] = static_cast<HLSLBool_t>(false);

    return Expected;
  }
};

template <typename T>
struct Op<OpType::WaveReadLaneAt, T, 1> : StrictValidation {};

template <typename T> struct ExpectedBuilder<OpType::WaveReadLaneAt, T> {
  static std::vector<T> buildExpected(Op<OpType::WaveReadLaneAt, T, 1> &,
                                      const InputSets<T> &Inputs, UINT) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    std::vector<T> Expected;
    const size_t VectorSize = Inputs[0].size();
    // Simple test, on the lane that we read we also fill the vector with the
    // value of the first element.
    Expected.assign(VectorSize, Inputs[0][0]);

    return Expected;
  }
};

template <typename T>
struct Op<OpType::WaveReadLaneFirst, T, 1> : StrictValidation {};

template <typename T> struct ExpectedBuilder<OpType::WaveReadLaneFirst, T> {
  static std::vector<T> buildExpected(Op<OpType::WaveReadLaneFirst, T, 1> &,
                                      const InputSets<T> &Inputs, UINT) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    std::vector<T> Expected;
    const size_t VectorSize = Inputs[0].size();
    // Simple test, on the lane that we read we also fill the vector with the
    // value of the first element.
    Expected.assign(VectorSize, Inputs[0][0]);

    return Expected;
  }
};

WAVE_OP(OpType::WavePrefixSum, (wavePrefixSum(A, WaveSize)));

template <typename T> T wavePrefixSum(T A, UINT WaveSize) {
  // We test the prefix sum in the 'middle' lane. This choice is arbitrary.
  return A * static_cast<T>(WaveSize / 2);
}

WAVE_OP(OpType::WaveMultiPrefixSum, (waveMultiPrefixSum(A, WaveSize)));

template <typename T> T waveMultiPrefixSum(T A, UINT) {
  return A * static_cast<T>(2u);
}

WAVE_OP(OpType::WavePrefixProduct, (wavePrefixProduct(A, WaveSize)));

template <typename T> T wavePrefixProduct(T A, UINT) {
  // We test the the prefix product in the 3rd lane to avoid overflow issues.
  // So the result is A * A.
  return A * A;
}

WAVE_OP(OpType::WaveMultiPrefixProduct, (waveMultiPrefixProduct(A, WaveSize)));

template <typename T> T waveMultiPrefixProduct(T A, UINT) {
  // The group mask has 3 lanes.
  return A * A;
}

template <typename T> struct Op<OpType::WaveMatch, T, 1> : StrictValidation {};

static void WriteExpectedValueForLane(UINT *Dest, const UINT LaneID,
                                      const std::bitset<128> &ExpectedValue) {
  std::bitset<128> Lo32Mask;
  Lo32Mask.set();
  Lo32Mask >>= 128 - 32;

  UINT Offset = 4 * LaneID;
  for (uint32_t I = 0; I < 4; I++) {
    uint32_t V = ((ExpectedValue >> (I * 32)) & Lo32Mask).to_ulong();
    Dest[Offset++] = V;
  }
}

template <typename T> struct ExpectedBuilder<OpType::WaveMatch, T> {
  static std::vector<UINT> buildExpected(Op<OpType::WaveMatch, T, 1> &,
                                         const InputSets<T> &Inputs,
                                         const UINT WaveSize) {
    // This test sets lanes (0, min(VectorSize/2, WaveSize/2), and
    // min(VectorSize-1, WaveSize-1)) to unique values and has them modify the
    // vector at their respective indices. Remaining lanes remain unchanged.
    DXASSERT_NOMSG(Inputs.size() == 1);

    const UINT VectorSize = static_cast<UINT>(Inputs[0].size());
    std::vector<UINT> Expected;
    Expected.assign(WaveSize * 4, 0);

    const UINT MidLaneID = std::min(VectorSize / 2, WaveSize / 2);
    const UINT LastLaneID = std::min(VectorSize - 1, WaveSize - 1);

    // Use a std::bitset<128> to represent the uint4 returned by WaveMatch as
    // its convenient this way in c++
    std::bitset<128> DefaultExpectedValue;

    for (UINT I = 0; I < WaveSize; ++I)
      DefaultExpectedValue.set(I);

    DefaultExpectedValue.reset(0);
    DefaultExpectedValue.reset(MidLaneID);
    DefaultExpectedValue.reset(LastLaneID);

    for (UINT LaneID = 0; LaneID < WaveSize; ++LaneID) {
      if (LaneID == 0 || LaneID == MidLaneID || LaneID == LastLaneID) {
        std::bitset<128> ExpectedValue(0);
        ExpectedValue.set(LaneID);
        WriteExpectedValueForLane(Expected.data(), LaneID, ExpectedValue);
        continue;
      }
      WriteExpectedValueForLane(Expected.data(), LaneID, DefaultExpectedValue);
    }

    return Expected;
  }
};

#undef WAVE_OP

//
// dispatchTest
//

template <OpType OP, typename T> struct ExpectedBuilder {

  static auto buildExpected(Op<OP, T, 1> Op, const InputSets<T> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    std::vector<decltype(Op(T()))> Expected;
    Expected.reserve(Inputs[0].size());

    for (size_t I = 0; I < Inputs[0].size(); ++I)
      Expected.push_back(Op(Inputs[0][I]));

    return Expected;
  }

  static auto buildExpected(Op<OP, T, 2> Op, const InputSets<T> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 2);

    std::vector<decltype(Op(T(), T()))> Expected;
    Expected.reserve(Inputs[0].size());

    for (size_t I = 0; I < Inputs[0].size(); ++I)
      Expected.push_back(Op(Inputs[0][I], Inputs[1][I]));

    return Expected;
  }

  static auto buildExpected(Op<OP, T, 3> Op, const InputSets<T> &Inputs) {
    DXASSERT_NOMSG(Inputs.size() == 3);

    std::vector<decltype(Op(T(), T(), T()))> Expected;
    Expected.reserve(Inputs[0].size());

    for (size_t I = 0; I < Inputs[0].size(); ++I)
      Expected.push_back(Op(Inputs[0][I], Inputs[1][I], Inputs[2][I]));

    return Expected;
  }

  static auto buildExpected(Op<OP, T, 1> Op, const InputSets<T> &Inputs,
                            UINT WaveSize) {
    DXASSERT_NOMSG(Inputs.size() == 1);

    std::vector<decltype(Op(T(), WaveSize))> Expected;
    Expected.reserve(Inputs[0].size());

    for (size_t I = 0; I < Inputs[0].size(); ++I)
      Expected.push_back(Op(Inputs[0][I], WaveSize));

    return Expected;
  }
};

#if defined(_M_IX86) && !defined(_HLK_CONF)
constexpr bool isX86WarpMemoryLimitedOp(OpType Op) {
  switch (Op) {
  case OpType::QuadReadLaneAt:
  case OpType::QuadReadAcrossX:
  case OpType::QuadReadAcrossY:
  case OpType::QuadReadAcrossDiagonal:
  case OpType::AsDouble:
  case OpType::AsUint_SplitDouble:
    return true;
  default:
    return false;
  }
}
#endif

template <typename T, OpType OP>
std::vector<size_t> getInputSizesToTest(size_t OverrideInputSize,
                                        ID3D12Device *D3DDevice) {
  std::vector<size_t> InputVectorSizes;
  const std::array<size_t, 8> DefaultInputSizes = {3,  5,   16,  17,
                                                   35, 100, 256, 1024};

  if (OverrideInputSize)
    InputVectorSizes.push_back(OverrideInputSize);
  else {
    // StructuredBuffers have a max size of 2048 bytes.
    size_t MaxInputSize =
        IsStructuredBufferLoadAndStoreOp(OP) ? 2048 / sizeof(T) : 1024;

#if defined(_M_IX86) && !defined(_HLK_CONF)
    // X86 Execution tests running against WARP cannot run the
    // largest vector sizes within the available address space for
    // some Ops. Cap the max size to work around that.
    if (isX86WarpMemoryLimitedOp(OP) && isWarp(D3DDevice))
      MaxInputSize = std::min(MaxInputSize, size_t{256});
#else
    (void)D3DDevice;
#endif

    for (size_t Size : DefaultInputSizes) {
      if (Size <= MaxInputSize)
        InputVectorSizes.push_back(Size);
    }

    if (InputVectorSizes.empty() || MaxInputSize != InputVectorSizes.back())
      InputVectorSizes.push_back(MaxInputSize);
  }

  return InputVectorSizes;
}

template <typename T, OpType OP>
void dispatchTest(ID3D12Device *D3DDevice, bool VerboseLogging,
                  size_t OverrideInputSize) {

  const std::vector<size_t> InputVectorSizes =
      getInputSizesToTest<T, OP>(OverrideInputSize, D3DDevice);

  constexpr const Operation &Operation = getOperation(OP);
  Op<OP, T, Operation.Arity> Op;

  for (size_t VectorSize : InputVectorSizes) {
    std::vector<std::vector<T>> Inputs =
        buildTestInputs<T>(VectorSize, Operation.InputSets, Operation.Arity);

    auto Expected = ExpectedBuilder<OP, T>::buildExpected(Op, Inputs);

    runAndVerify(D3DDevice, VerboseLogging, Operation, Inputs, Expected,
                 Op.ValidationConfig);
  }
}

template <typename T, OpType OP>
void dispatchWaveOpTest(ID3D12Device *D3DDevice, bool VerboseLogging,
                        size_t OverrideInputSize, UINT WaveSize) {

  const std::vector<size_t> InputVectorSizes =
      getInputSizesToTest<T, OP>(OverrideInputSize, D3DDevice);

  constexpr const Operation &Operation = getOperation(OP);
  Op<OP, T, Operation.Arity> Op;

  const std::string AdditionalCompilerOptions =
      "-DWAVE_SIZE=" + std::to_string(WaveSize) +
      " -DNUMTHREADS_XYZ=" + std::to_string(WaveSize) + ",1,1 ";

  for (size_t VectorSize : InputVectorSizes) {
    std::vector<std::vector<T>> Inputs =
        buildTestInputs<T>(VectorSize, Operation.InputSets, Operation.Arity);

    auto Expected = ExpectedBuilder<OP, T>::buildExpected(Op, Inputs, WaveSize);

    runAndVerify(D3DDevice, VerboseLogging, Operation, Inputs, Expected,
                 Op.ValidationConfig, AdditionalCompilerOptions);
  }
}

} // namespace LongVector

using namespace LongVector;

// TAEF test entry points
#define HLK_TEST(Op, DataType)                                                 \
  TEST_METHOD(Op##_##DataType) { runTest<DataType, OpType::Op>(); }

#define HLK_WAVEOP_TEST(Op, DataType)                                          \
  TEST_METHOD(Op##_##DataType) {                                               \
    BEGIN_TEST_METHOD_PROPERTIES()                                             \
    TEST_METHOD_PROPERTY(                                                      \
        "Kits.Specification",                                                  \
        "Device.Graphics.D3D12.DXILCore.ShaderModel69.CoreRequirement")        \
    END_TEST_METHOD_PROPERTIES()                                               \
    runWaveOpTest<DataType, OpType::Op>();                                     \
  }

class TestClassCommon {
public:
  bool setupClass() {
    WEX::TestExecution::SetVerifyOutput verifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

    // Run this only once.
    if (!Initialized) {
      Initialized = true;

      D3D12SDK = D3D12SDKSelector();

      WEX::TestExecution::RuntimeParameters::TryGetValue(L"VerboseLogging",
                                                         VerboseLogging);
      if (VerboseLogging)
        hlsl_test::LogCommentFmt(L"Verbose logging is enabled for this test.");
      else
        hlsl_test::LogCommentFmt(L"Verbose logging is disabled for this test.");

      WEX::TestExecution::RuntimeParameters::TryGetValue(L"InputSize",
                                                         OverrideInputSize);

      WEX::TestExecution::RuntimeParameters::TryGetValue(L"WaveLaneCount",
                                                         OverrideWaveLaneCount);

      bool IsRITP = false;
      WEX::TestExecution::RuntimeParameters::TryGetValue(L"RITP", IsRITP);

      if (IsRITP) {
        if (!OverrideInputSize)
          // Help keep test runtime down for RITP runs
          OverrideInputSize = 10;
        else
          hlsl_test::LogWarningFmt(
              L"RITP is enabled but InputSize is also set. Will use the"
              L"InputSize value: %d.",
              OverrideInputSize);
      }

      bool FailIfRequirementsNotMet = false;
#ifdef _HLK_CONF
      FailIfRequirementsNotMet = true;
#endif
      WEX::TestExecution::RuntimeParameters::TryGetValue(
          L"FailIfRequirementsNotMet", FailIfRequirementsNotMet);

      const bool SkipUnsupported = !FailIfRequirementsNotMet;
      if (!D3D12SDK->createDevice(&D3DDevice, D3D_SHADER_MODEL_6_9,
                                  SkipUnsupported)) {
        if (FailIfRequirementsNotMet)
          hlsl_test::LogErrorFmt(
              L"Device Creation failed, resulting in test failure, since "
              L"FailIfRequirementsNotMet is set. The expectation is that this "
              L"test will only be executed if something has previously "
              L"determined that the system meets the requirements of this "
              L"test.");

        return false;
      }
    }

    return true;
  }

  bool setupMethod() {
    // It's possible a previous test case caused a device removal. If it did we
    // need to try and create a new device.
    if (D3DDevice && D3DDevice->GetDeviceRemovedReason() != S_OK) {
      hlsl_test::LogCommentFmt(L"Device was lost!");
      D3DDevice.Release();
    }

    if (!D3DDevice) {
      hlsl_test::LogCommentFmt(L"Creating device");

      // We expect this to succeed, and fail if it doesn't, because classSetup()
      // has already ensured that the system configuration meets the
      // requirements of all the tests in this class.
      const bool SkipUnsupported = false;

      VERIFY_IS_TRUE(D3D12SDK->createDevice(&D3DDevice, D3D_SHADER_MODEL_6_9,
                                            SkipUnsupported));
    }

    return true;
  }

  template <typename T, OpType OP> void runWaveOpTest() {
    WEX::TestExecution::SetVerifyOutput VerifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

    UINT WaveSize = 0;

    if (OverrideWaveLaneCount > 0) {
      WaveSize = OverrideWaveLaneCount;
      hlsl_test::LogCommentFmt(
          L"Using overridden WaveLaneCount of %d for this test.", WaveSize);
    } else {
      D3D12_FEATURE_DATA_D3D12_OPTIONS1 WaveOpts;
      VERIFY_SUCCEEDED(D3DDevice->CheckFeatureSupport(
          D3D12_FEATURE_D3D12_OPTIONS1, &WaveOpts, sizeof(WaveOpts)));

      WaveSize = WaveOpts.WaveLaneCountMin;
    }

    DXASSERT_NOMSG(WaveSize > 0);
    DXASSERT((WaveSize & (WaveSize - 1)) == 0, "must be a power of 2");

    dispatchWaveOpTest<T, OP>(D3DDevice, VerboseLogging, OverrideInputSize,
                              WaveSize);
  }

  template <typename T, OpType OP> void runTest() {
    WEX::TestExecution::SetVerifyOutput verifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

    dispatchTest<T, OP>(D3DDevice, VerboseLogging, OverrideInputSize);
  }

protected:
  CComPtr<ID3D12Device> D3DDevice;

private:
  bool Initialized = false;
  std::optional<D3D12SDKSelector> D3D12SDK;
  bool VerboseLogging = false;
  size_t OverrideInputSize = 0;
  UINT OverrideWaveLaneCount = 0;
};

class DxilConf_SM69_Vectorized_Core : public TestClassCommon {
public:
  BEGIN_TEST_CLASS(DxilConf_SM69_Vectorized_Core)
  TEST_CLASS_PROPERTY("Kits.TestName",
                      "D3D12 - Shader Model 6.9 - Vectorized DXIL - Core Tests")
  TEST_CLASS_PROPERTY("Kits.TestId", "81db1ff8-5bc5-48a1-8d7b-600fc600a677")
  TEST_CLASS_PROPERTY("Kits.Description",
                      "Validates required SM 6.9 vectorized DXIL operations")
  TEST_CLASS_PROPERTY(
      "Kits.Specification",
      "Device.Graphics.D3D12.DXILCore.ShaderModel69.CoreRequirement")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(setupClass) { return TestClassCommon::setupClass(); }
  TEST_METHOD_SETUP(setupMethod) { return TestClassCommon::setupMethod(); }

  // TernaryMath
  HLK_TEST(Mad, uint16_t);
  HLK_TEST(Mad, uint32_t);
  HLK_TEST(Mad, uint64_t);
  HLK_TEST(Mad, int16_t);
  HLK_TEST(Mad, int32_t);
  HLK_TEST(Mad, int64_t);
  HLK_TEST(Mad, HLSLHalf_t);
  HLK_TEST(Mad, float);

  // BinaryMath
  HLK_TEST(Add, HLSLBool_t);
  HLK_TEST(Subtract, HLSLBool_t);
  HLK_TEST(Add, int16_t);
  HLK_TEST(Subtract, int16_t);
  HLK_TEST(Multiply, int16_t);
  HLK_TEST(Divide, int16_t);
  HLK_TEST(Modulus, int16_t);
  HLK_TEST(Min, int16_t);
  HLK_TEST(Max, int16_t);
  HLK_TEST(Add, int32_t);
  HLK_TEST(Subtract, int32_t);
  HLK_TEST(Multiply, int32_t);
  HLK_TEST(Divide, int32_t);
  HLK_TEST(Modulus, int32_t);
  HLK_TEST(Min, int32_t);
  HLK_TEST(Max, int32_t);
  HLK_TEST(Add, int64_t);
  HLK_TEST(Subtract, int64_t);
  HLK_TEST(Multiply, int64_t);
  HLK_TEST(Divide, int64_t);
  HLK_TEST(Modulus, int64_t);
  HLK_TEST(Min, int64_t);
  HLK_TEST(Max, int64_t);
  HLK_TEST(Add, uint16_t);
  HLK_TEST(Subtract, uint16_t);
  HLK_TEST(Multiply, uint16_t);
  HLK_TEST(Divide, uint16_t);
  HLK_TEST(Modulus, uint16_t);
  HLK_TEST(Min, uint16_t);
  HLK_TEST(Max, uint16_t);
  HLK_TEST(Add, uint32_t);
  HLK_TEST(Subtract, uint32_t);
  HLK_TEST(Multiply, uint32_t);
  HLK_TEST(Divide, uint32_t);
  HLK_TEST(Modulus, uint32_t);
  HLK_TEST(Min, uint32_t);
  HLK_TEST(Max, uint32_t);
  HLK_TEST(Add, uint64_t);
  HLK_TEST(Subtract, uint64_t);
  HLK_TEST(Multiply, uint64_t);
  HLK_TEST(Divide, uint64_t);
  HLK_TEST(Modulus, uint64_t);
  HLK_TEST(Min, uint64_t);
  HLK_TEST(Max, uint64_t);
  HLK_TEST(Add, HLSLHalf_t);
  HLK_TEST(Subtract, HLSLHalf_t);
  HLK_TEST(Multiply, HLSLHalf_t);
  HLK_TEST(Divide, HLSLHalf_t);
  HLK_TEST(Modulus, HLSLHalf_t);
  HLK_TEST(Min, HLSLHalf_t);
  HLK_TEST(Max, HLSLHalf_t);
  HLK_TEST(Ldexp, HLSLHalf_t);
  HLK_TEST(Add, float);
  HLK_TEST(Subtract, float);
  HLK_TEST(Multiply, float);
  HLK_TEST(Divide, float);
  HLK_TEST(Modulus, float);
  HLK_TEST(Min, float);
  HLK_TEST(Max, float);
  HLK_TEST(Ldexp, float);

  // Bitwise
  HLK_TEST(And, uint16_t);
  HLK_TEST(Or, uint16_t);
  HLK_TEST(Xor, uint16_t);
  HLK_TEST(ReverseBits, uint16_t);
  HLK_TEST(CountBits, uint16_t);
  HLK_TEST(FirstBitHigh, uint16_t);
  HLK_TEST(FirstBitLow, uint16_t);
  HLK_TEST(LeftShift, uint16_t);
  HLK_TEST(RightShift, uint16_t);
  HLK_TEST(And, uint32_t);
  HLK_TEST(Or, uint32_t);
  HLK_TEST(Xor, uint32_t);
  HLK_TEST(LeftShift, uint32_t);
  HLK_TEST(RightShift, uint32_t);
  HLK_TEST(ReverseBits, uint32_t);
  HLK_TEST(CountBits, uint32_t);
  HLK_TEST(FirstBitHigh, uint32_t);
  HLK_TEST(FirstBitLow, uint32_t);
  HLK_TEST(And, uint64_t);
  HLK_TEST(Or, uint64_t);
  HLK_TEST(Xor, uint64_t);
  HLK_TEST(LeftShift, uint64_t);
  HLK_TEST(RightShift, uint64_t);
  HLK_TEST(ReverseBits, uint64_t);
  HLK_TEST(CountBits, uint64_t);
  HLK_TEST(FirstBitHigh, uint64_t);
  HLK_TEST(FirstBitLow, uint64_t);
  HLK_TEST(And, int16_t);
  HLK_TEST(Or, int16_t);
  HLK_TEST(Xor, int16_t);
  HLK_TEST(LeftShift, int16_t);
  HLK_TEST(RightShift, int16_t);
  HLK_TEST(ReverseBits, int16_t);
  HLK_TEST(CountBits, int16_t);
  HLK_TEST(FirstBitHigh, int16_t);
  HLK_TEST(FirstBitLow, int16_t);
  HLK_TEST(And, int32_t);
  HLK_TEST(Or, int32_t);
  HLK_TEST(Xor, int32_t);
  HLK_TEST(LeftShift, int32_t);
  HLK_TEST(RightShift, int32_t);
  HLK_TEST(ReverseBits, int32_t);
  HLK_TEST(CountBits, int32_t);
  HLK_TEST(FirstBitHigh, int32_t);
  HLK_TEST(FirstBitLow, int32_t);
  HLK_TEST(And, int64_t);
  HLK_TEST(Or, int64_t);
  HLK_TEST(Xor, int64_t);
  HLK_TEST(LeftShift, int64_t);
  HLK_TEST(RightShift, int64_t);
  HLK_TEST(ReverseBits, int64_t);
  HLK_TEST(CountBits, int64_t);
  HLK_TEST(FirstBitHigh, int64_t);
  HLK_TEST(FirstBitLow, int64_t);
  HLK_TEST(Saturate, HLSLHalf_t);
  HLK_TEST(Saturate, float);

  // Unary
  HLK_TEST(Initialize, HLSLBool_t);
  HLK_TEST(ArrayOperator_StaticAccess, HLSLBool_t);
  HLK_TEST(ArrayOperator_DynamicAccess, HLSLBool_t);
  HLK_TEST(Initialize, int16_t);
  HLK_TEST(ArrayOperator_StaticAccess, int16_t);
  HLK_TEST(ArrayOperator_DynamicAccess, int16_t);
  HLK_TEST(Initialize, int32_t);
  HLK_TEST(ArrayOperator_StaticAccess, int32_t);
  HLK_TEST(ArrayOperator_DynamicAccess, int32_t);
  HLK_TEST(Initialize, int64_t);
  HLK_TEST(ArrayOperator_StaticAccess, int64_t);
  HLK_TEST(ArrayOperator_DynamicAccess, int64_t);
  HLK_TEST(Initialize, uint16_t);
  HLK_TEST(ArrayOperator_StaticAccess, uint16_t);
  HLK_TEST(ArrayOperator_DynamicAccess, uint16_t);
  HLK_TEST(Initialize, uint32_t);
  HLK_TEST(ArrayOperator_StaticAccess, uint32_t);
  HLK_TEST(ArrayOperator_DynamicAccess, uint32_t);
  HLK_TEST(Initialize, uint64_t);
  HLK_TEST(ArrayOperator_StaticAccess, uint64_t);
  HLK_TEST(ArrayOperator_DynamicAccess, uint64_t);
  HLK_TEST(Initialize, HLSLHalf_t);
  HLK_TEST(ArrayOperator_StaticAccess, HLSLHalf_t);
  HLK_TEST(ArrayOperator_DynamicAccess, HLSLHalf_t);
  HLK_TEST(Initialize, float);
  HLK_TEST(ArrayOperator_StaticAccess, float);
  HLK_TEST(ArrayOperator_DynamicAccess, float);

  HLK_TEST(ShuffleVector, HLSLBool_t);
  HLK_TEST(ShuffleVector, int16_t);
  HLK_TEST(ShuffleVector, int32_t);
  HLK_TEST(ShuffleVector, int64_t);
  HLK_TEST(ShuffleVector, uint16_t);
  HLK_TEST(ShuffleVector, uint32_t);
  HLK_TEST(ShuffleVector, uint64_t);
  HLK_TEST(ShuffleVector, HLSLHalf_t);
  HLK_TEST(ShuffleVector, float);

  // Explicit Cast
  HLK_TEST(CastToInt16, HLSLBool_t);
  HLK_TEST(CastToInt32, HLSLBool_t);
  HLK_TEST(CastToInt64, HLSLBool_t);
  HLK_TEST(CastToUint16, HLSLBool_t);
  HLK_TEST(CastToUint32, HLSLBool_t);
  HLK_TEST(CastToUint64, HLSLBool_t);
  HLK_TEST(CastToFloat16, HLSLBool_t);
  HLK_TEST(CastToFloat32, HLSLBool_t);

  HLK_TEST(CastToBool, HLSLHalf_t);
  HLK_TEST(CastToInt16, HLSLHalf_t);
  HLK_TEST(CastToInt32, HLSLHalf_t);
  HLK_TEST(CastToInt64, HLSLHalf_t);
  HLK_TEST(CastToUint16_FromFP, HLSLHalf_t);
  HLK_TEST(CastToUint32_FromFP, HLSLHalf_t);
  HLK_TEST(CastToUint64_FromFP, HLSLHalf_t);
  HLK_TEST(CastToFloat32, HLSLHalf_t);

  HLK_TEST(CastToBool, float);
  HLK_TEST(CastToInt16, float);
  HLK_TEST(CastToInt32, float);
  HLK_TEST(CastToInt64, float);
  HLK_TEST(CastToUint16_FromFP, float);
  HLK_TEST(CastToUint32_FromFP, float);
  HLK_TEST(CastToUint64_FromFP, float);
  HLK_TEST(CastToFloat16, float);

  HLK_TEST(CastToBool, uint16_t);
  HLK_TEST(CastToInt16, uint16_t);
  HLK_TEST(CastToInt32, uint16_t);
  HLK_TEST(CastToInt64, uint16_t);
  HLK_TEST(CastToUint32, uint16_t);
  HLK_TEST(CastToUint64, uint16_t);
  HLK_TEST(CastToFloat16, uint16_t);
  HLK_TEST(CastToFloat32, uint16_t);

  HLK_TEST(CastToBool, uint32_t);
  HLK_TEST(CastToInt16, uint32_t);
  HLK_TEST(CastToInt32, uint32_t);
  HLK_TEST(CastToInt64, uint32_t);
  HLK_TEST(CastToUint16, uint32_t);
  HLK_TEST(CastToUint64, uint32_t);
  HLK_TEST(CastToFloat16, uint32_t);
  HLK_TEST(CastToFloat32, uint32_t);

  HLK_TEST(CastToBool, uint64_t);
  HLK_TEST(CastToInt16, uint64_t);
  HLK_TEST(CastToInt32, uint64_t);
  HLK_TEST(CastToInt64, uint64_t);
  HLK_TEST(CastToUint16, uint64_t);
  HLK_TEST(CastToUint32, uint64_t);
  HLK_TEST(CastToFloat16, uint64_t);
  HLK_TEST(CastToFloat32, uint64_t);

  HLK_TEST(CastToBool, int16_t);
  HLK_TEST(CastToInt32, int16_t);
  HLK_TEST(CastToInt64, int16_t);
  HLK_TEST(CastToUint16, int16_t);
  HLK_TEST(CastToUint32, int16_t);
  HLK_TEST(CastToUint64, int16_t);
  HLK_TEST(CastToFloat16, int16_t);
  HLK_TEST(CastToFloat32, int16_t);

  HLK_TEST(CastToBool, int32_t);
  HLK_TEST(CastToInt16, int32_t);
  HLK_TEST(CastToInt64, int32_t);
  HLK_TEST(CastToUint16, int32_t);
  HLK_TEST(CastToUint32, int32_t);
  HLK_TEST(CastToUint64, int32_t);
  HLK_TEST(CastToFloat16, int32_t);
  HLK_TEST(CastToFloat32, int32_t);

  HLK_TEST(CastToBool, int64_t);
  HLK_TEST(CastToInt16, int64_t);
  HLK_TEST(CastToInt32, int64_t);
  HLK_TEST(CastToUint16, int64_t);
  HLK_TEST(CastToUint32, int64_t);
  HLK_TEST(CastToUint64, int64_t);
  HLK_TEST(CastToFloat16, int64_t);
  HLK_TEST(CastToFloat32, int64_t);

  // Trigonometric
  HLK_TEST(Acos, HLSLHalf_t);
  HLK_TEST(Asin, HLSLHalf_t);
  HLK_TEST(Atan, HLSLHalf_t);
  HLK_TEST(Cos, HLSLHalf_t);
  HLK_TEST(Cosh, HLSLHalf_t);
  HLK_TEST(Sin, HLSLHalf_t);
  HLK_TEST(Sinh, HLSLHalf_t);
  HLK_TEST(Tan, HLSLHalf_t);
  HLK_TEST(Tanh, HLSLHalf_t);
  HLK_TEST(Acos, float);
  HLK_TEST(Asin, float);
  HLK_TEST(Atan, float);
  HLK_TEST(Cos, float);
  HLK_TEST(Cosh, float);
  HLK_TEST(Sin, float);
  HLK_TEST(Sinh, float);
  HLK_TEST(Tan, float);
  HLK_TEST(Tanh, float);

  // AsType
  HLK_TEST(AsFloat16, int16_t);
  HLK_TEST(AsInt16, int16_t);
  HLK_TEST(AsUint16, int16_t);
  HLK_TEST(AsFloat, int32_t);
  HLK_TEST(AsInt, int32_t);
  HLK_TEST(AsUint, int32_t);
  HLK_TEST(AsFloat16, uint16_t);
  HLK_TEST(AsInt16, uint16_t);
  HLK_TEST(AsUint16, uint16_t);
  HLK_TEST(AsFloat, uint32_t);
  HLK_TEST(AsInt, uint32_t);
  HLK_TEST(AsUint, uint32_t);
  HLK_TEST(AsFloat16, HLSLHalf_t);
  HLK_TEST(AsInt16, HLSLHalf_t);
  HLK_TEST(AsUint16, HLSLHalf_t);

  // Unary Math
  HLK_TEST(Abs, int16_t);
  HLK_TEST(Sign, int16_t);
  HLK_TEST(Abs, int32_t);
  HLK_TEST(Sign, int32_t);
  HLK_TEST(Abs, int64_t);
  HLK_TEST(Sign, int64_t);
  HLK_TEST(Abs, uint16_t);
  HLK_TEST(Sign, uint16_t);
  HLK_TEST(Abs, uint32_t);
  HLK_TEST(Sign, uint32_t);
  HLK_TEST(Abs, uint64_t);
  HLK_TEST(Sign, uint64_t);
  HLK_TEST(Abs, HLSLHalf_t);
  HLK_TEST(Ceil, HLSLHalf_t);
  HLK_TEST(Exp, HLSLHalf_t);
  HLK_TEST(Floor, HLSLHalf_t);
  HLK_TEST(Frac, HLSLHalf_t);
  HLK_TEST(Log, HLSLHalf_t);
  HLK_TEST(Rcp, HLSLHalf_t);
  HLK_TEST(Round, HLSLHalf_t);
  HLK_TEST(Rsqrt, HLSLHalf_t);
  HLK_TEST(Sign, HLSLHalf_t);
  HLK_TEST(Sqrt, HLSLHalf_t);
  HLK_TEST(Trunc, HLSLHalf_t);
  HLK_TEST(Exp2, HLSLHalf_t);
  HLK_TEST(Log10, HLSLHalf_t);
  HLK_TEST(Log2, HLSLHalf_t);
  HLK_TEST(Abs, float);
  HLK_TEST(Ceil, float);
  HLK_TEST(Exp, float);
  HLK_TEST(Floor, float);
  HLK_TEST(Frac, float);
  HLK_TEST(Log, float);
  HLK_TEST(Rcp, float);
  HLK_TEST(Round, float);
  HLK_TEST(Rsqrt, float);
  HLK_TEST(Sign, float);
  HLK_TEST(Sqrt, float);
  HLK_TEST(Trunc, float);
  HLK_TEST(Exp2, float);
  HLK_TEST(Log10, float);
  HLK_TEST(Log2, float);
  HLK_TEST(Frexp, float);

  // Float Special
  HLK_TEST(IsFinite, HLSLHalf_t);
  HLK_TEST(IsInf, HLSLHalf_t);
  HLK_TEST(IsNan, HLSLHalf_t);
  HLK_TEST(ModF, HLSLHalf_t);

  HLK_TEST(IsFinite, float);
  HLK_TEST(IsInf, float);
  HLK_TEST(IsNan, float);
  HLK_TEST(ModF, float);

  // Binary Comparison
  HLK_TEST(LessThan, int16_t);
  HLK_TEST(LessEqual, int16_t);
  HLK_TEST(GreaterThan, int16_t);
  HLK_TEST(GreaterEqual, int16_t);
  HLK_TEST(Equal, int16_t);
  HLK_TEST(NotEqual, int16_t);
  HLK_TEST(LessThan, int32_t);
  HLK_TEST(LessEqual, int32_t);
  HLK_TEST(GreaterThan, int32_t);
  HLK_TEST(GreaterEqual, int32_t);
  HLK_TEST(Equal, int32_t);
  HLK_TEST(NotEqual, int32_t);
  HLK_TEST(LessThan, int64_t);
  HLK_TEST(LessEqual, int64_t);
  HLK_TEST(GreaterThan, int64_t);
  HLK_TEST(GreaterEqual, int64_t);
  HLK_TEST(Equal, int64_t);
  HLK_TEST(NotEqual, int64_t);
  HLK_TEST(LessThan, uint16_t);
  HLK_TEST(LessEqual, uint16_t);
  HLK_TEST(GreaterThan, uint16_t);
  HLK_TEST(GreaterEqual, uint16_t);
  HLK_TEST(Equal, uint16_t);
  HLK_TEST(NotEqual, uint16_t);
  HLK_TEST(LessThan, uint32_t);
  HLK_TEST(LessEqual, uint32_t);
  HLK_TEST(GreaterThan, uint32_t);
  HLK_TEST(GreaterEqual, uint32_t);
  HLK_TEST(Equal, uint32_t);
  HLK_TEST(NotEqual, uint32_t);
  HLK_TEST(LessThan, uint64_t);
  HLK_TEST(LessEqual, uint64_t);
  HLK_TEST(GreaterThan, uint64_t);
  HLK_TEST(GreaterEqual, uint64_t);
  HLK_TEST(Equal, uint64_t);
  HLK_TEST(NotEqual, uint64_t);
  HLK_TEST(LessThan, HLSLHalf_t);
  HLK_TEST(LessEqual, HLSLHalf_t);
  HLK_TEST(GreaterThan, HLSLHalf_t);
  HLK_TEST(GreaterEqual, HLSLHalf_t);
  HLK_TEST(Equal, HLSLHalf_t);
  HLK_TEST(NotEqual, HLSLHalf_t);
  HLK_TEST(LessThan, float);
  HLK_TEST(LessEqual, float);
  HLK_TEST(GreaterThan, float);
  HLK_TEST(GreaterEqual, float);
  HLK_TEST(Equal, float);
  HLK_TEST(NotEqual, float);

  // Binary Logical
  HLK_TEST(Logical_And, HLSLBool_t);
  HLK_TEST(Logical_Or, HLSLBool_t);

  // Ternary Logical
  HLK_TEST(Select, HLSLBool_t);
  HLK_TEST(Select, int16_t);
  HLK_TEST(Select, int32_t);
  HLK_TEST(Select, int64_t);
  HLK_TEST(Select, uint16_t);
  HLK_TEST(Select, uint32_t);
  HLK_TEST(Select, uint64_t);
  HLK_TEST(Select, HLSLHalf_t);
  HLK_TEST(Select, float);

  // Reduction
  HLK_TEST(Any_Mixed, HLSLBool_t);
  HLK_TEST(Any_Zero, HLSLBool_t);
  HLK_TEST(Any_NoZero, HLSLBool_t);
  HLK_TEST(All_Mixed, HLSLBool_t);
  HLK_TEST(All_Zero, HLSLBool_t);
  HLK_TEST(All_NoZero, HLSLBool_t);

  HLK_TEST(Any_Mixed, int16_t);
  HLK_TEST(Any_Zero, int16_t);
  HLK_TEST(Any_NoZero, int16_t);
  HLK_TEST(All_Mixed, int16_t);
  HLK_TEST(All_Zero, int16_t);
  HLK_TEST(All_NoZero, int16_t);

  HLK_TEST(Any_Mixed, int32_t);
  HLK_TEST(Any_Zero, int32_t);
  HLK_TEST(Any_NoZero, int32_t);
  HLK_TEST(All_Mixed, int32_t);
  HLK_TEST(All_Zero, int32_t);
  HLK_TEST(All_NoZero, int32_t);

  HLK_TEST(Any_Mixed, int64_t);
  HLK_TEST(Any_Zero, int64_t);
  HLK_TEST(Any_NoZero, int64_t);
  HLK_TEST(All_Mixed, int64_t);
  HLK_TEST(All_Zero, int64_t);
  HLK_TEST(All_NoZero, int64_t);

  HLK_TEST(Dot, HLSLHalf_t);

  HLK_TEST(Dot, float);

  // LoadAndStore
  // BAB == Byte Address Buffer
  // RDH == Resource Descriptor Heap
  // RD == Root Descriptor
  // DT == Descriptor Table
  // SB == Structured Buffer
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, HLSLHalf_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, HLSLHalf_t);

  HLK_TEST(LoadAndStore_RDH_BAB_SRV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, HLSLBool_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, HLSLBool_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, HLSLBool_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, HLSLBool_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, HLSLBool_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, HLSLBool_t);

  HLK_TEST(LoadAndStore_RDH_BAB_SRV, int16_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, int16_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, int16_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, int16_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, int16_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, int16_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, int16_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, int16_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, int16_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, int16_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, int16_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, int16_t);

  HLK_TEST(LoadAndStore_RDH_BAB_SRV, int32_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, int32_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, int32_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, int32_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, int32_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, int32_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, int32_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, int32_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, int32_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, int32_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, int32_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, int32_t);

  HLK_TEST(LoadAndStore_RDH_BAB_SRV, int64_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, int64_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, int64_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, int64_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, int64_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, int64_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, int64_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, int64_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, int64_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, int64_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, int64_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, int64_t);

  HLK_TEST(LoadAndStore_RDH_BAB_SRV, uint16_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, uint16_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, uint16_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, uint16_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, uint16_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, uint16_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, uint16_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, uint16_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, uint16_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, uint16_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, uint16_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, uint16_t);

  HLK_TEST(LoadAndStore_RDH_BAB_UAV, uint32_t);
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, uint32_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, uint32_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, uint32_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, uint32_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, uint32_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, uint32_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, uint32_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, uint32_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, uint32_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, uint32_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, uint32_t);

  HLK_TEST(LoadAndStore_RDH_BAB_UAV, uint64_t);
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, uint64_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, uint64_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, uint64_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, uint64_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, uint64_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, uint64_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, uint64_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, uint64_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, uint64_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, uint64_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, uint64_t);

  HLK_TEST(LoadAndStore_RDH_BAB_UAV, float);
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, float);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, float);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, float);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, float);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, float);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, float);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, float);
  HLK_TEST(LoadAndStore_DT_SB_UAV, float);
  HLK_TEST(LoadAndStore_DT_SB_SRV, float);
  HLK_TEST(LoadAndStore_RD_SB_UAV, float);
  HLK_TEST(LoadAndStore_RD_SB_SRV, float);

  // Derivative
  HLK_TEST(DerivativeDdx, HLSLHalf_t);
  HLK_TEST(DerivativeDdy, HLSLHalf_t);
  HLK_TEST(DerivativeDdxFine, HLSLHalf_t);
  HLK_TEST(DerivativeDdyFine, HLSLHalf_t);
  HLK_TEST(DerivativeDdx, float);
  HLK_TEST(DerivativeDdy, float);
  HLK_TEST(DerivativeDdxFine, float);
  HLK_TEST(DerivativeDdyFine, float);

  // Quad
  HLK_TEST(QuadReadLaneAt, HLSLBool_t);
  HLK_TEST(QuadReadAcrossX, HLSLBool_t);
  HLK_TEST(QuadReadAcrossY, HLSLBool_t);
  HLK_TEST(QuadReadAcrossDiagonal, HLSLBool_t);
  HLK_TEST(QuadReadLaneAt, int16_t);
  HLK_TEST(QuadReadAcrossX, int16_t);
  HLK_TEST(QuadReadAcrossY, int16_t);
  HLK_TEST(QuadReadAcrossDiagonal, int16_t);
  HLK_TEST(QuadReadLaneAt, int32_t);
  HLK_TEST(QuadReadAcrossX, int32_t);
  HLK_TEST(QuadReadAcrossY, int32_t);
  HLK_TEST(QuadReadAcrossDiagonal, int32_t);
  HLK_TEST(QuadReadLaneAt, int64_t);
  HLK_TEST(QuadReadAcrossX, int64_t);
  HLK_TEST(QuadReadAcrossY, int64_t);
  HLK_TEST(QuadReadAcrossDiagonal, int64_t);
  HLK_TEST(QuadReadLaneAt, uint16_t);
  HLK_TEST(QuadReadAcrossX, uint16_t);
  HLK_TEST(QuadReadAcrossY, uint16_t);
  HLK_TEST(QuadReadAcrossDiagonal, uint16_t);
  HLK_TEST(QuadReadLaneAt, uint32_t);
  HLK_TEST(QuadReadAcrossX, uint32_t);
  HLK_TEST(QuadReadAcrossY, uint32_t);
  HLK_TEST(QuadReadAcrossDiagonal, uint32_t);
  HLK_TEST(QuadReadLaneAt, uint64_t);
  HLK_TEST(QuadReadAcrossX, uint64_t);
  HLK_TEST(QuadReadAcrossY, uint64_t);
  HLK_TEST(QuadReadAcrossDiagonal, uint64_t);
  HLK_TEST(QuadReadLaneAt, HLSLHalf_t);
  HLK_TEST(QuadReadAcrossX, HLSLHalf_t);
  HLK_TEST(QuadReadAcrossY, HLSLHalf_t);
  HLK_TEST(QuadReadAcrossDiagonal, HLSLHalf_t);
  HLK_TEST(QuadReadLaneAt, float);
  HLK_TEST(QuadReadAcrossX, float);
  HLK_TEST(QuadReadAcrossY, float);
  HLK_TEST(QuadReadAcrossDiagonal, float);

  // Wave
  HLK_WAVEOP_TEST(WaveActiveAllEqual, HLSLBool_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, HLSLBool_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, HLSLBool_t);
  HLK_WAVEOP_TEST(WaveMatch, HLSLBool_t);

  HLK_WAVEOP_TEST(WaveActiveSum, int16_t);
  HLK_WAVEOP_TEST(WaveActiveMin, int16_t);
  HLK_WAVEOP_TEST(WaveActiveMax, int16_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, int16_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, int16_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, int16_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, int16_t);
  HLK_WAVEOP_TEST(WavePrefixSum, int16_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, int16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, int16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, int16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, int16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, int16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, int16_t);
  HLK_WAVEOP_TEST(WaveMatch, int16_t);
  HLK_WAVEOP_TEST(WaveActiveSum, int32_t);
  HLK_WAVEOP_TEST(WaveActiveMin, int32_t);
  HLK_WAVEOP_TEST(WaveActiveMax, int32_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, int32_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, int32_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, int32_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, int32_t);
  HLK_WAVEOP_TEST(WavePrefixSum, int32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, int32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, int32_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, int32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, int32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, int32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, int32_t);
  HLK_WAVEOP_TEST(WaveMatch, int32_t);
  HLK_WAVEOP_TEST(WaveActiveSum, int64_t);
  HLK_WAVEOP_TEST(WaveActiveMin, int64_t);
  HLK_WAVEOP_TEST(WaveActiveMax, int64_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, int64_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, int64_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, int64_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, int64_t);
  HLK_WAVEOP_TEST(WavePrefixSum, int64_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, int64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, int64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, int64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, int64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, int64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, int64_t);
  HLK_WAVEOP_TEST(WaveMatch, int64_t);

  // Note: WaveActiveBit* ops don't support uint16_t in HLSL
  // But the WaveMultiPrefixBit ops support all int and uint types
  HLK_WAVEOP_TEST(WaveActiveSum, uint16_t);
  HLK_WAVEOP_TEST(WaveActiveMin, uint16_t);
  HLK_WAVEOP_TEST(WaveActiveMax, uint16_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, uint16_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, uint16_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, uint16_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, uint16_t);
  HLK_WAVEOP_TEST(WavePrefixSum, uint16_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, uint16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, uint16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, uint16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, uint16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, uint16_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, uint16_t);
  HLK_WAVEOP_TEST(WaveMatch, uint16_t);
  HLK_WAVEOP_TEST(WaveActiveSum, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveMin, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveMax, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveBitAnd, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveBitOr, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveBitXor, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, uint32_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, uint32_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, uint32_t);
  HLK_WAVEOP_TEST(WavePrefixSum, uint32_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, uint32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, uint32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, uint32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, uint32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, uint32_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, uint32_t);
  HLK_WAVEOP_TEST(WaveMatch, uint32_t);
  HLK_WAVEOP_TEST(WaveActiveSum, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveMin, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveMax, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveBitAnd, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveBitOr, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveBitXor, uint64_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, uint64_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, uint64_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, uint64_t);
  HLK_WAVEOP_TEST(WavePrefixSum, uint64_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, uint64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, uint64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, uint64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, uint64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, uint64_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, uint64_t);
  HLK_WAVEOP_TEST(WaveMatch, uint64_t);

  HLK_WAVEOP_TEST(WaveActiveSum, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveActiveMin, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveActiveMax, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, HLSLHalf_t);
  HLK_WAVEOP_TEST(WavePrefixSum, HLSLHalf_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveMatch, HLSLHalf_t);
  HLK_WAVEOP_TEST(WaveActiveSum, float);
  HLK_WAVEOP_TEST(WaveActiveMin, float);
  HLK_WAVEOP_TEST(WaveActiveMax, float);
  HLK_WAVEOP_TEST(WaveActiveProduct, float);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, float);
  HLK_WAVEOP_TEST(WaveReadLaneAt, float);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, float);
  HLK_WAVEOP_TEST(WavePrefixSum, float);
  HLK_WAVEOP_TEST(WavePrefixProduct, float);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, float);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, float);
  HLK_WAVEOP_TEST(WaveMatch, float);

  // ---- HLSLMin16Float_t (mirrors applicable HLSLHalf_t ops) ----

  // TernaryMath
  HLK_TEST(Mad, HLSLMin16Float_t);

  // BinaryMath
  HLK_TEST(Add, HLSLMin16Float_t);
  HLK_TEST(Subtract, HLSLMin16Float_t);
  HLK_TEST(Multiply, HLSLMin16Float_t);
  HLK_TEST(Divide, HLSLMin16Float_t);
  HLK_TEST(Modulus, HLSLMin16Float_t);
  HLK_TEST(Min, HLSLMin16Float_t);
  HLK_TEST(Max, HLSLMin16Float_t);
  HLK_TEST(Ldexp, HLSLMin16Float_t);

  // Saturate
  HLK_TEST(Saturate, HLSLMin16Float_t);

  // Unary
  HLK_TEST(Initialize, HLSLMin16Float_t);
  HLK_TEST(ArrayOperator_StaticAccess, HLSLMin16Float_t);
  HLK_TEST(ArrayOperator_DynamicAccess, HLSLMin16Float_t);
  HLK_TEST(ShuffleVector, HLSLMin16Float_t);

  // Cast
  HLK_TEST(CastToBool, HLSLMin16Float_t);
  HLK_TEST(CastToInt32, HLSLMin16Float_t);
  HLK_TEST(CastToInt64, HLSLMin16Float_t);
  HLK_TEST(CastToUint32_FromFP, HLSLMin16Float_t);
  HLK_TEST(CastToUint64_FromFP, HLSLMin16Float_t);
  HLK_TEST(CastToFloat32, HLSLMin16Float_t);
  // Note: CastToInt16, CastToUint16_FromFP, CastToFloat16 excluded —
  // 16-bit output types require -enable-16bit-types which changes min
  // precision semantics (min16float becomes half), breaking the test
  // infrastructure's 32-bit buffer I/O assumptions.

  // Trigonometric
  HLK_TEST(Acos, HLSLMin16Float_t);
  HLK_TEST(Asin, HLSLMin16Float_t);
  HLK_TEST(Atan, HLSLMin16Float_t);
  HLK_TEST(Cos, HLSLMin16Float_t);
  HLK_TEST(Cosh, HLSLMin16Float_t);
  HLK_TEST(Sin, HLSLMin16Float_t);
  HLK_TEST(Sinh, HLSLMin16Float_t);
  HLK_TEST(Tan, HLSLMin16Float_t);
  HLK_TEST(Tanh, HLSLMin16Float_t);

  // UnaryMath
  HLK_TEST(Abs, HLSLMin16Float_t);
  HLK_TEST(Ceil, HLSLMin16Float_t);
  HLK_TEST(Exp, HLSLMin16Float_t);
  HLK_TEST(Floor, HLSLMin16Float_t);
  HLK_TEST(Frac, HLSLMin16Float_t);
  HLK_TEST(Log, HLSLMin16Float_t);
  HLK_TEST(Rcp, HLSLMin16Float_t);
  HLK_TEST(Round, HLSLMin16Float_t);
  HLK_TEST(Rsqrt, HLSLMin16Float_t);
  HLK_TEST(Sign, HLSLMin16Float_t);
  HLK_TEST(Sqrt, HLSLMin16Float_t);
  HLK_TEST(Trunc, HLSLMin16Float_t);
  HLK_TEST(Exp2, HLSLMin16Float_t);
  HLK_TEST(Log10, HLSLMin16Float_t);
  HLK_TEST(Log2, HLSLMin16Float_t);

  // BinaryComparison
  HLK_TEST(LessThan, HLSLMin16Float_t);
  HLK_TEST(LessEqual, HLSLMin16Float_t);
  HLK_TEST(GreaterThan, HLSLMin16Float_t);
  HLK_TEST(GreaterEqual, HLSLMin16Float_t);
  HLK_TEST(Equal, HLSLMin16Float_t);
  HLK_TEST(NotEqual, HLSLMin16Float_t);

  // Select
  HLK_TEST(Select, HLSLMin16Float_t);

  // Dot
  HLK_TEST(Dot, HLSLMin16Float_t);

  // LoadAndStore
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, HLSLMin16Float_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, HLSLMin16Float_t);

  // Derivative
  HLK_TEST(DerivativeDdx, HLSLMin16Float_t);
  HLK_TEST(DerivativeDdy, HLSLMin16Float_t);
  HLK_TEST(DerivativeDdxFine, HLSLMin16Float_t);
  HLK_TEST(DerivativeDdyFine, HLSLMin16Float_t);

  // Quad
  HLK_TEST(QuadReadLaneAt, HLSLMin16Float_t);
  HLK_TEST(QuadReadAcrossX, HLSLMin16Float_t);
  HLK_TEST(QuadReadAcrossY, HLSLMin16Float_t);
  HLK_TEST(QuadReadAcrossDiagonal, HLSLMin16Float_t);

  // Wave
  HLK_WAVEOP_TEST(WaveActiveSum, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveActiveMin, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveActiveMax, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WavePrefixSum, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, HLSLMin16Float_t);
  HLK_WAVEOP_TEST(WaveMatch, HLSLMin16Float_t);

  // ---- HLSLMin16Int_t (mirrors applicable int16_t ops) ----

  // TernaryMath
  HLK_TEST(Mad, HLSLMin16Int_t);

  // BinaryMath
  // Note: Divide and Modulus excluded — HLSL does not support signed integer
  // division on minimum-precision types.
  HLK_TEST(Add, HLSLMin16Int_t);
  HLK_TEST(Subtract, HLSLMin16Int_t);
  HLK_TEST(Multiply, HLSLMin16Int_t);
  HLK_TEST(Min, HLSLMin16Int_t);
  HLK_TEST(Max, HLSLMin16Int_t);

  // Bitwise
  HLK_TEST(And, HLSLMin16Int_t);
  HLK_TEST(Or, HLSLMin16Int_t);
  HLK_TEST(Xor, HLSLMin16Int_t);
  HLK_TEST(LeftShift, HLSLMin16Int_t);
  HLK_TEST(RightShift, HLSLMin16Int_t);
  // Note: ReverseBits, CountBits, FirstBitHigh, FirstBitLow excluded -
  // DXC promotes min precision to i32 before these intrinsics, so they
  // don't operate at min precision.

  // UnaryMath
  HLK_TEST(Abs, HLSLMin16Int_t);
  HLK_TEST(Sign, HLSLMin16Int_t);

  // Unary
  HLK_TEST(Initialize, HLSLMin16Int_t);
  HLK_TEST(ArrayOperator_StaticAccess, HLSLMin16Int_t);
  HLK_TEST(ArrayOperator_DynamicAccess, HLSLMin16Int_t);
  HLK_TEST(ShuffleVector, HLSLMin16Int_t);

  // Cast
  HLK_TEST(CastToBool, HLSLMin16Int_t);
  HLK_TEST(CastToInt32, HLSLMin16Int_t);
  HLK_TEST(CastToInt64, HLSLMin16Int_t);
  HLK_TEST(CastToUint32, HLSLMin16Int_t);
  HLK_TEST(CastToUint64, HLSLMin16Int_t);
  HLK_TEST(CastToFloat32, HLSLMin16Int_t);
  // Note: CastToUint16, CastToFloat16 excluded — see min16float note.

  // BinaryComparison
  HLK_TEST(LessThan, HLSLMin16Int_t);
  HLK_TEST(LessEqual, HLSLMin16Int_t);
  HLK_TEST(GreaterThan, HLSLMin16Int_t);
  HLK_TEST(GreaterEqual, HLSLMin16Int_t);
  HLK_TEST(Equal, HLSLMin16Int_t);
  HLK_TEST(NotEqual, HLSLMin16Int_t);

  // Select
  HLK_TEST(Select, HLSLMin16Int_t);

  // Reduction
  HLK_TEST(Any_Mixed, HLSLMin16Int_t);
  HLK_TEST(Any_Zero, HLSLMin16Int_t);
  HLK_TEST(Any_NoZero, HLSLMin16Int_t);
  HLK_TEST(All_Mixed, HLSLMin16Int_t);
  HLK_TEST(All_Zero, HLSLMin16Int_t);
  HLK_TEST(All_NoZero, HLSLMin16Int_t);

  // LoadAndStore
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, HLSLMin16Int_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, HLSLMin16Int_t);

  // Quad
  HLK_TEST(QuadReadLaneAt, HLSLMin16Int_t);
  HLK_TEST(QuadReadAcrossX, HLSLMin16Int_t);
  HLK_TEST(QuadReadAcrossY, HLSLMin16Int_t);
  HLK_TEST(QuadReadAcrossDiagonal, HLSLMin16Int_t);

  // Wave
  HLK_WAVEOP_TEST(WaveActiveSum, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveActiveMin, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveActiveMax, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WavePrefixSum, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, HLSLMin16Int_t);
  HLK_WAVEOP_TEST(WaveMatch, HLSLMin16Int_t);

  // ---- HLSLMin16Uint_t (mirrors applicable uint16_t ops) ----

  // TernaryMath
  HLK_TEST(Mad, HLSLMin16Uint_t);

  // BinaryMath
  HLK_TEST(Add, HLSLMin16Uint_t);
  HLK_TEST(Subtract, HLSLMin16Uint_t);
  HLK_TEST(Multiply, HLSLMin16Uint_t);
  HLK_TEST(Divide, HLSLMin16Uint_t);
  HLK_TEST(Modulus, HLSLMin16Uint_t);
  HLK_TEST(Min, HLSLMin16Uint_t);
  HLK_TEST(Max, HLSLMin16Uint_t);

  // Bitwise
  HLK_TEST(And, HLSLMin16Uint_t);
  HLK_TEST(Or, HLSLMin16Uint_t);
  HLK_TEST(Xor, HLSLMin16Uint_t);
  HLK_TEST(LeftShift, HLSLMin16Uint_t);
  HLK_TEST(RightShift, HLSLMin16Uint_t);
  // Note: ReverseBits, CountBits, FirstBitHigh, FirstBitLow excluded -
  // DXC promotes min precision to i32 before these intrinsics, so they
  // don't operate at min precision.

  // UnaryMath
  HLK_TEST(Abs, HLSLMin16Uint_t);
  HLK_TEST(Sign, HLSLMin16Uint_t);

  // Unary
  HLK_TEST(Initialize, HLSLMin16Uint_t);
  HLK_TEST(ArrayOperator_StaticAccess, HLSLMin16Uint_t);
  HLK_TEST(ArrayOperator_DynamicAccess, HLSLMin16Uint_t);
  HLK_TEST(ShuffleVector, HLSLMin16Uint_t);

  // Cast
  HLK_TEST(CastToBool, HLSLMin16Uint_t);
  HLK_TEST(CastToInt32, HLSLMin16Uint_t);
  HLK_TEST(CastToInt64, HLSLMin16Uint_t);
  HLK_TEST(CastToUint32, HLSLMin16Uint_t);
  HLK_TEST(CastToUint64, HLSLMin16Uint_t);
  HLK_TEST(CastToFloat32, HLSLMin16Uint_t);
  // Note: CastToInt16, CastToFloat16 excluded — see min16float note.

  // BinaryComparison
  HLK_TEST(LessThan, HLSLMin16Uint_t);
  HLK_TEST(LessEqual, HLSLMin16Uint_t);
  HLK_TEST(GreaterThan, HLSLMin16Uint_t);
  HLK_TEST(GreaterEqual, HLSLMin16Uint_t);
  HLK_TEST(Equal, HLSLMin16Uint_t);
  HLK_TEST(NotEqual, HLSLMin16Uint_t);

  // Select
  HLK_TEST(Select, HLSLMin16Uint_t);

  // LoadAndStore
  HLK_TEST(LoadAndStore_RDH_BAB_SRV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RDH_BAB_UAV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_DT_BAB_SRV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_DT_BAB_UAV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RD_BAB_SRV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RD_BAB_UAV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RDH_SB_SRV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RDH_SB_UAV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_DT_SB_SRV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_DT_SB_UAV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RD_SB_SRV, HLSLMin16Uint_t);
  HLK_TEST(LoadAndStore_RD_SB_UAV, HLSLMin16Uint_t);

  // Quad
  HLK_TEST(QuadReadLaneAt, HLSLMin16Uint_t);
  HLK_TEST(QuadReadAcrossX, HLSLMin16Uint_t);
  HLK_TEST(QuadReadAcrossY, HLSLMin16Uint_t);
  HLK_TEST(QuadReadAcrossDiagonal, HLSLMin16Uint_t);

  // Wave
  HLK_WAVEOP_TEST(WaveActiveSum, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveActiveMin, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveActiveMax, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveActiveProduct, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveActiveAllEqual, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveReadLaneAt, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveReadLaneFirst, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WavePrefixSum, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WavePrefixProduct, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixSum, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixProduct, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitAnd, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitOr, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveMultiPrefixBitXor, HLSLMin16Uint_t);
  HLK_WAVEOP_TEST(WaveMatch, HLSLMin16Uint_t);
};

#define HLK_TEST_DOUBLE(Op, DataType)                                          \
  TEST_METHOD(Op##_##DataType) {                                               \
    BEGIN_TEST_METHOD_PROPERTIES()                                             \
    TEST_METHOD_PROPERTY("Kits.Specification",                                 \
                         "Device.Graphics.D3D12.DXILCore.ShaderModel69."       \
                         "DoublePrecision.Optional")                           \
    END_TEST_METHOD_PROPERTIES()                                               \
    runTest<DataType, OpType::Op>();                                           \
  }

#define HLK_WAVEOP_TEST_DOUBLE(Op, DataType)                                   \
  TEST_METHOD(Op##_##DataType) {                                               \
    BEGIN_TEST_METHOD_PROPERTIES()                                             \
    TEST_METHOD_PROPERTY("Kits.Specification",                                 \
                         "Device.Graphics.D3D12.DXILCore.ShaderModel69."       \
                         "DoublePrecision.Optional")                           \
    END_TEST_METHOD_PROPERTIES()                                               \
    runWaveOpTest<DataType, OpType::Op>();                                     \
  }

class DxilConf_SM69_Vectorized_Double : public TestClassCommon {
public:
  BEGIN_TEST_CLASS(DxilConf_SM69_Vectorized_Double)
  TEST_CLASS_PROPERTY(
      "Kits.TestName",
      "D3D12 - Shader Model 6.9 - Vectorized DXIL - Double Precision Tests")
  TEST_CLASS_PROPERTY("Kits.TestId", "3a7a9687-6d99-469d-9951-795c2fcbe94d")
  TEST_CLASS_PROPERTY(
      "Kits.Description",
      "Validates required double precision SM 6.9 vectorized DXIL operations")
  TEST_METHOD_PROPERTY(
      "Kits.Specification",
      "Device.Graphics.D3D12.DXILCore.ShaderModel69.DoublePrecision.Optional")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(setupClass) {
    const bool result = TestClassCommon::setupClass();
#ifndef _HLK_CONF
    if (result && !doesDeviceSupportDouble(D3DDevice)) {
      WEX::Logging::Log::Comment(
          L"Skipping test as device does not support double precision.");
      WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
      return false;
    }
#endif
    return result;
  }

  TEST_METHOD_SETUP(setupMethod) { return TestClassCommon::setupMethod(); }

  // TernaryMath
  HLK_TEST_DOUBLE(Fma, double);
  HLK_TEST_DOUBLE(Mad, double);

  // BinaryMath
  HLK_TEST_DOUBLE(Add, double);
  HLK_TEST_DOUBLE(Subtract, double);
  HLK_TEST_DOUBLE(Multiply, double);
  HLK_TEST_DOUBLE(Divide, double);
  HLK_TEST_DOUBLE(Min, double);
  HLK_TEST_DOUBLE(Max, double);

  // Bitwise
  HLK_TEST_DOUBLE(Saturate, double);

  // Unary
  HLK_TEST_DOUBLE(Initialize, double);
  HLK_TEST_DOUBLE(ArrayOperator_StaticAccess, double);
  HLK_TEST_DOUBLE(ArrayOperator_DynamicAccess, double);

  HLK_TEST_DOUBLE(ShuffleVector, double);

  // Explicit Cast
  HLK_TEST_DOUBLE(CastToBool, double);
  HLK_TEST_DOUBLE(CastToInt16, double);
  HLK_TEST_DOUBLE(CastToInt32, double);
  HLK_TEST_DOUBLE(CastToInt64, double);
  HLK_TEST_DOUBLE(CastToUint16_FromFP, double);
  HLK_TEST_DOUBLE(CastToUint32_FromFP, double);
  HLK_TEST_DOUBLE(CastToUint64_FromFP, double);
  HLK_TEST_DOUBLE(CastToFloat16, double);
  HLK_TEST_DOUBLE(CastToFloat32, double);

  // Explicit Cast to Double (from various types)
  HLK_TEST_DOUBLE(CastToFloat64, HLSLBool_t);
  HLK_TEST_DOUBLE(CastToFloat64, HLSLHalf_t);
  HLK_TEST_DOUBLE(CastToFloat64, float);
  HLK_TEST_DOUBLE(CastToFloat64, uint16_t);
  HLK_TEST_DOUBLE(CastToFloat64, uint32_t);
  HLK_TEST_DOUBLE(CastToFloat64, uint64_t);
  HLK_TEST_DOUBLE(CastToFloat64, int16_t);
  HLK_TEST_DOUBLE(CastToFloat64, int32_t);
  HLK_TEST_DOUBLE(CastToFloat64, int64_t);

  // AsType (double precision)
  HLK_TEST_DOUBLE(AsDouble, uint32_t);
  HLK_TEST_DOUBLE(AsUint_SplitDouble, double);

  // Unary Math
  HLK_TEST_DOUBLE(Abs, double);
  HLK_TEST_DOUBLE(Sign, double);

  // Binary Comparison
  HLK_TEST_DOUBLE(LessThan, double);
  HLK_TEST_DOUBLE(LessEqual, double);
  HLK_TEST_DOUBLE(GreaterThan, double);
  HLK_TEST_DOUBLE(GreaterEqual, double);
  HLK_TEST_DOUBLE(Equal, double);
  HLK_TEST_DOUBLE(NotEqual, double);

  // Ternary Logical
  HLK_TEST_DOUBLE(Select, double);

  // LoadAndStore
  HLK_TEST_DOUBLE(LoadAndStore_RDH_BAB_SRV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RDH_BAB_UAV, double);
  HLK_TEST_DOUBLE(LoadAndStore_DT_BAB_SRV, double);
  HLK_TEST_DOUBLE(LoadAndStore_DT_BAB_UAV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RD_BAB_SRV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RD_BAB_UAV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RDH_SB_SRV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RDH_SB_UAV, double);
  HLK_TEST_DOUBLE(LoadAndStore_DT_SB_SRV, double);
  HLK_TEST_DOUBLE(LoadAndStore_DT_SB_UAV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RD_SB_SRV, double);
  HLK_TEST_DOUBLE(LoadAndStore_RD_SB_UAV, double);

  // Quad
  HLK_TEST_DOUBLE(QuadReadLaneAt, double);
  HLK_TEST_DOUBLE(QuadReadAcrossX, double);
  HLK_TEST_DOUBLE(QuadReadAcrossY, double);
  HLK_TEST_DOUBLE(QuadReadAcrossDiagonal, double);

  // Wave
  HLK_WAVEOP_TEST_DOUBLE(WaveActiveSum, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveActiveMin, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveActiveMax, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveActiveProduct, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveActiveAllEqual, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveReadLaneAt, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveReadLaneFirst, double);
  HLK_WAVEOP_TEST_DOUBLE(WavePrefixSum, double);
  HLK_WAVEOP_TEST_DOUBLE(WavePrefixProduct, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveMultiPrefixSum, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveMultiPrefixProduct, double);
  HLK_WAVEOP_TEST_DOUBLE(WaveMatch, double);
};
