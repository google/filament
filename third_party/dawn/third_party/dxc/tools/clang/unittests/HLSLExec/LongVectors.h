#ifndef LONGVECTORS_H
#define LONGVECTORS_H

#include <array>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <Verify.h>

#include "LongVectorTestData.h"
#include "ShaderOpTest.h"
#include "TableParameterHandler.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Test/HlslTestUtils.h"

namespace LongVector {

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

// Used so we can dynamically resolve the type of data stored out the output
// LongVector for a test case.
using VariantVector =
    std::variant<std::vector<HLSLBool_t>, std::vector<HLSLHalf_t>,
                 std::vector<float>, std::vector<double>, std::vector<int16_t>,
                 std::vector<int32_t>, std::vector<int64_t>,
                 std::vector<uint16_t>, std::vector<uint32_t>,
                 std::vector<uint64_t>>;

// A helper struct to clear a VariantVector using std::visit.
// Example usage: std::visit(ClearVariantVector{}, MyVariantVector);
struct ClearVariantVector {
  template <typename T> void operator()(std::vector<T> &vec) const {
    vec.clear();
  }
};

template <typename DataTypeT>
void fillShaderBufferFromLongVectorData(std::vector<BYTE> &ShaderBuffer,
                                        const std::vector<DataTypeT> &TestData);

template <typename DataTypeT>
void fillLongVectorDataFromShaderBuffer(const MappedData &ShaderBuffer,
                                        std::vector<DataTypeT> &TestData,
                                        size_t NumElements);

template <typename DataTypeT> constexpr bool isFloatingPointType() {
  return std::is_same_v<DataTypeT, float> ||
         std::is_same_v<DataTypeT, double> ||
         std::is_same_v<DataTypeT, HLSLHalf_t>;
}

template <typename DataTypeT> std::string getHLSLTypeString();

// Helpful metadata struct so we can define some common properties for a test in
// a single place. Intrinsic and Operator are passed in with -D defines to
// the compiler and expanded as macros in the HLSL code. For a better
// understanding of expansion you can reference the shader source used in
// ShaderOpArith.xml under the 'LongVectorOp' entry.
//
// OpTypeString : This is populated by the TableParamaterHandler parsing the
// LongVectorOpTable.xml file. It's used to find the enum value in one of the
// arrays below. Such as binaryOpTypeStringToOpMetaData.
//
// OpType : Populated via the lookup with OpTypeString.
//
// Intrinsic : May be empty. Used to expand the intrinsic name in the
// compiled HLSL code via macro expansion. See getCompilerOptionsString() in
// LongVector.cpp in addition to the shader source.
//
// Operator : Used to expand the operator in the compiled HLSL code via macro
// expansion. May be empty. See getCompilerOptionsString() in LongVector.cpp and
// 'LongVectorOp' entry ShaderOpArith.xml. Expands to things like '+', '-',
// '*', etc.
template <typename LongVectorOpTypeT> struct OpTypeMetaData {
  std::wstring OpTypeString;
  LongVectorOpTypeT OpType;
  std::optional<std::string> Intrinsic = std::nullopt;
  std::optional<std::string> Operator = std::nullopt;
};

template <typename T, size_t Length>
const OpTypeMetaData<T> &
getLongVectorOpType(const OpTypeMetaData<T> (&Values)[Length],
                    const std::wstring &OpTypeString);

enum ValidationType {
  ValidationType_Epsilon,
  ValidationType_Ulp,
};

enum BasicOpType {
  BasicOpType_Binary,
  BasicOpType_Unary,
  BasicOpType_ScalarBinary,
  BasicOpType_EnumValueCount
};

enum BinaryOpType {
  BinaryOpType_ScalarAdd,
  BinaryOpType_ScalarMultiply,
  BinaryOpType_ScalarSubtract,
  BinaryOpType_ScalarDivide,
  BinaryOpType_ScalarModulus,
  BinaryOpType_Multiply,
  BinaryOpType_Add,
  BinaryOpType_Subtract,
  BinaryOpType_Divide,
  BinaryOpType_Modulus,
  BinaryOpType_Min,
  BinaryOpType_Max,
  BinaryOpType_ScalarMin,
  BinaryOpType_ScalarMax,
  BinaryOpType_EnumValueCount
};

static const OpTypeMetaData<BinaryOpType> binaryOpTypeStringToOpMetaData[] = {
    {L"BinaryOpType_ScalarAdd", BinaryOpType_ScalarAdd, std::nullopt, "+"},
    {L"BinaryOpType_ScalarMultiply", BinaryOpType_ScalarMultiply, std::nullopt,
     "*"},
    {L"BinaryOpType_ScalarSubtract", BinaryOpType_ScalarSubtract, std::nullopt,
     "-"},
    {L"BinaryOpType_ScalarDivide", BinaryOpType_ScalarDivide, std::nullopt,
     "/"},
    {L"BinaryOpType_ScalarModulus", BinaryOpType_ScalarModulus, std::nullopt,
     "%"},
    {L"BinaryOpType_Add", BinaryOpType_Add, std::nullopt, "+"},
    {L"BinaryOpType_Multiply", BinaryOpType_Multiply, std::nullopt, "*"},
    {L"BinaryOpType_Subtract", BinaryOpType_Subtract, std::nullopt, "-"},
    {L"BinaryOpType_Divide", BinaryOpType_Divide, std::nullopt, "/"},
    {L"BinaryOpType_Modulus", BinaryOpType_Modulus, std::nullopt, "%"},
    {L"BinaryOpType_Min", BinaryOpType_Min, "min", ","},
    {L"BinaryOpType_Max", BinaryOpType_Max, "max", ","},
    {L"BinaryOpType_ScalarMin", BinaryOpType_ScalarMin, "min", ","},
    {L"BinaryOpType_ScalarMax", BinaryOpType_ScalarMax, "max", ","},
};

static_assert(_countof(binaryOpTypeStringToOpMetaData) ==
                  BinaryOpType_EnumValueCount,
              "binaryOpTypeStringToOpMetaData size mismatch. Did you "
              "add a new enum value?");

const OpTypeMetaData<BinaryOpType> &
getBinaryOpType(const std::wstring &OpTypeString);

enum UnaryOpType { UnaryOpType_Initialize, UnaryOpType_EnumValueCount };

static const OpTypeMetaData<UnaryOpType> unaryOpTypeStringToOpMetaData[] = {
    {L"UnaryOpType_Initialize", UnaryOpType_Initialize, "TestInitialize"},
};

static_assert(_countof(unaryOpTypeStringToOpMetaData) ==
                  UnaryOpType_EnumValueCount,
              "unaryOpTypeStringToOpMetaData size mismatch. Did you add "
              "a new enum value?");

const OpTypeMetaData<UnaryOpType> &
getUnaryOpType(const std::wstring &OpTypeString);

enum AsTypeOpType {
  AsTypeOpType_AsFloat,
  AsTypeOpType_AsFloat16,
  AsTypeOpType_AsInt,
  AsTypeOpType_AsInt16,
  AsTypeOpType_AsUint,
  AsTypeOpType_AsUint_SplitDouble,
  AsTypeOpType_AsUint16,
  AsTypeOpType_AsDouble,
  AsTypeOpType_EnumValueCount
};

static const OpTypeMetaData<AsTypeOpType> asTypeOpTypeStringToOpMetaData[] = {
    {L"AsTypeOpType_AsFloat", AsTypeOpType_AsFloat, "asfloat"},
    {L"AsTypeOpType_AsFloat16", AsTypeOpType_AsFloat16, "asfloat16"},
    {L"AsTypeOpType_AsInt", AsTypeOpType_AsInt, "asint"},
    {L"AsTypeOpType_AsInt16", AsTypeOpType_AsInt16, "asint16"},
    {L"AsTypeOpType_AsUint", AsTypeOpType_AsUint, "asuint"},
    {L"AsTypeOpType_AsUint_SplitDouble", AsTypeOpType_AsUint_SplitDouble,
     "TestAsUintSplitDouble"},
    {L"AsTypeOpType_AsUint16", AsTypeOpType_AsUint16, "asuint16"},
    {L"AsTypeOpType_AsDouble", AsTypeOpType_AsDouble, "asdouble", ","},
};

static_assert(_countof(asTypeOpTypeStringToOpMetaData) ==
                  AsTypeOpType_EnumValueCount,
              "asTypeOpTypeStringToOpMetaData size mismatch. Did you add "
              "a new enum value?");

const OpTypeMetaData<AsTypeOpType> &
getAsTypeOpType(const std::wstring &OpTypeString);

enum TrigonometricOpType {
  TrigonometricOpType_Acos,
  TrigonometricOpType_Asin,
  TrigonometricOpType_Atan,
  TrigonometricOpType_Cos,
  TrigonometricOpType_Cosh,
  TrigonometricOpType_Sin,
  TrigonometricOpType_Sinh,
  TrigonometricOpType_Tan,
  TrigonometricOpType_Tanh,
  TrigonometricOpType_EnumValueCount
};

static const OpTypeMetaData<TrigonometricOpType>
    trigonometricOpTypeStringToOpMetaData[] = {
        {L"TrigonometricOpType_Acos", TrigonometricOpType_Acos, "acos"},
        {L"TrigonometricOpType_Asin", TrigonometricOpType_Asin, "asin"},
        {L"TrigonometricOpType_Atan", TrigonometricOpType_Atan, "atan"},
        {L"TrigonometricOpType_Cos", TrigonometricOpType_Cos, "cos"},
        {L"TrigonometricOpType_Cosh", TrigonometricOpType_Cosh, "cosh"},
        {L"TrigonometricOpType_Sin", TrigonometricOpType_Sin, "sin"},
        {L"TrigonometricOpType_Sinh", TrigonometricOpType_Sinh, "sinh"},
        {L"TrigonometricOpType_Tan", TrigonometricOpType_Tan, "tan"},
        {L"TrigonometricOpType_Tanh", TrigonometricOpType_Tanh, "tanh"},
};

static_assert(
    _countof(trigonometricOpTypeStringToOpMetaData) ==
        TrigonometricOpType_EnumValueCount,
    "trigonometricOpTypeStringToOpMetaData size mismatch. Did you add "
    "a new enum value?");

const OpTypeMetaData<TrigonometricOpType> &
getTrigonometricOpType(const std::wstring &OpTypeString);

template <typename DataTypeT>
std::vector<DataTypeT> getInputValueSetByKey(const std::wstring &Key,
                                             bool LogKey = true) {
  if (LogKey)
    WEX::Logging::Log::Comment(
        WEX::Common::String().Format(L"Using Value Set Key: %s", Key.c_str()));
  return std::vector<DataTypeT>(LongVectorTestData<DataTypeT>::Data.at(Key));
}

// The TAEF test class.
template <typename DataTypeT> class TestConfig; // Forward declaration.
class OpTest {
public:
  BEGIN_TEST_CLASS(OpTest)
  END_TEST_CLASS()

  TEST_CLASS_SETUP(classSetup);

  BEGIN_TEST_METHOD(binaryOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:LongVectorOpTable.xml#BinaryOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(trigonometricOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:LongVectorOpTable.xml#TrigonometricOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(unaryOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:LongVectorOpTable.xml#UnaryOpTable")
  END_TEST_METHOD()

  BEGIN_TEST_METHOD(asTypeOpTest)
  TEST_METHOD_PROPERTY(L"DataSource",
                       L"Table:LongVectorOpTable.xml#AsTypeOpTable")
  END_TEST_METHOD()

  template <typename LongVectorOpTypeT>
  void dispatchTestByDataType(const OpTypeMetaData<LongVectorOpTypeT> &OpTypeMD,
                              std::wstring DataType,
                              TableParameterHandler &Handler);

  template <>
  void
  dispatchTestByDataType(const OpTypeMetaData<TrigonometricOpType> &OpTypeMD,
                         std::wstring DataType, TableParameterHandler &Handler);

  template <typename DataTypeT, typename LongVectorOpTypeT>
  void
  dispatchTestByVectorLength(const OpTypeMetaData<LongVectorOpTypeT> &OpTypeMD,
                             TableParameterHandler &Handler);

  template <typename DataTypeT>
  void testBaseMethod(std::unique_ptr<TestConfig<DataTypeT>> &TestConfig);

private:
  dxc::DxcDllSupport DxcDllSupport;
  bool Initialized = false;
};

template <typename DataTypeT>
bool doValuesMatch(DataTypeT A, DataTypeT B, float Tolerance, ValidationType);
bool doValuesMatch(HLSLBool_t A, HLSLBool_t B, float, ValidationType);
bool doValuesMatch(HLSLHalf_t A, HLSLHalf_t B, float Tolerance,
                   ValidationType ValidationType);
bool doValuesMatch(float A, float B, float Tolerance,
                   ValidationType ValidationType);
bool doValuesMatch(double A, double B, float Tolerance,
                   ValidationType ValidationType);

template <typename DataTypeT>
bool doVectorsMatch(const std::vector<DataTypeT> &ActualValues,
                    const std::vector<DataTypeT> &ExpectedValues,
                    float Tolerance, ValidationType ValidationType);

template <typename DataTypeT>
void logLongVector(const std::vector<DataTypeT> &Values,
                   const std::wstring &Name);

// Helps handle the test configuration for LongVector operations.
// It was particularly useful helping manage logic of computing expected values
// and verifying the output. Especially helpful due to templating on the
// different data types and giving us a relatively clean way to leverage
// different logic paths for different HLSL instrinsics while keeping the main
// test code pretty generic.
template <typename DataTypeT> class TestConfig {
public:
  virtual ~TestConfig() = default;

  bool isBinaryOp() const {
    return BasicOpType == BasicOpType_Binary ||
           BasicOpType == BasicOpType_ScalarBinary;
  }

  bool isUnaryOp() const { return BasicOpType == BasicOpType_Unary; }

  bool isScalarOp() const { return BasicOpType == BasicOpType_ScalarBinary; }

  // Helpers to get the hlsl type as a string for a given C++ type.
  std::string getHLSLInputTypeString() const;
  virtual std::string getHLSLOutputTypeString() const;

  virtual void
  computeExpectedValues(const std::vector<DataTypeT> &InputVector1);
  virtual void
  computeExpectedValues(const std::vector<DataTypeT> &InputVector1,
                        const std::vector<DataTypeT> &InputVector2);
  void computeExpectedValues(const std::vector<DataTypeT> &InputVector1,
                             const DataTypeT &ScalarInput);

  void setInputValueSet1(const std::wstring &InputValueSetName) {
    InputValueSetName1 = InputValueSetName;
  }

  void setInputValueSet2(const std::wstring &InputValueSetName) {
    InputValueSetName2 = InputValueSetName;
  }

  void setLengthToTest(size_t LengthToTest) {
    // Make sure we clear the expected vector when setting a new length.
    // The TestConfig may be getting reused.
    std::visit(ClearVariantVector{}, ExpectedVector);

    this->LengthToTest = LengthToTest;
  }

  size_t getLengthToTest() const { return LengthToTest; }

  std::vector<DataTypeT> getInputValueSet1() const {
    return getInputValueSet(1);
  }

  std::vector<DataTypeT> getInputValueSet2() const {
    return getInputValueSet(2);
  }

  std::vector<DataTypeT> getInputArgsArray() const;

  float getTolerance() const { return Tolerance; }
  ValidationType getValidationType() const { return ValidationType; }

  std::string getCompilerOptionsString() const;

  virtual bool
  verifyOutput(const std::shared_ptr<st::ShaderOpTestResult> &TestResult);

private:
  std::vector<DataTypeT> getInputValueSet(size_t ValueSetIndex) const;

  // The input value sets are used to fill the shader buffer.
  std::wstring InputValueSetName1 = L"DefaultInputValueSet1";
  std::wstring InputValueSetName2 = L"DefaultInputValueSet2";
  // No default args array
  std::wstring InputArgsArrayName = L"";

protected:
  // Prevent instances of TestConfig from being created directly. Want to force
  // a derived class to be used for creation.
  template <typename LongVectorOpTypeT>
  TestConfig(const OpTypeMetaData<LongVectorOpTypeT> &OpTypeMd)
      : OpTypeName(OpTypeMd.OpTypeString), Intrinsic(OpTypeMd.Intrinsic),
        Operator(OpTypeMd.Operator) {}

  // Templated version to be used when the output data type does not match the
  // input data type.
  template <typename OutputDataTypeT>
  bool verifyOutput(const std::shared_ptr<st::ShaderOpTestResult> &TestResult);

  // The appropriate computeExpectedValue should be implemented in derived
  // classes. Impelemented as virtual here to prevent requiring all derived
  // classes from needing to implement. The OS builds disable RTTI, so using
  // dynamic casting to expose interfaces for these based on type isn't an
  // option. COM is the usual solution for this. But it's not worth it to add
  // all of the COM overhead to this class just for that.
  virtual DataTypeT
  computeExpectedValue([[maybe_unused]] const DataTypeT &A,
                       [[maybe_unused]] const DataTypeT &B) const {
    LOG_ERROR_FMT_THROW(L"E_NOT_IMPL: computeExpectedValue for a Binary Op");
    return DataTypeT();
  }
  virtual DataTypeT
  computeExpectedValue([[maybe_unused]] const DataTypeT &A) const {
    LOG_ERROR_FMT_THROW(L"E_NOT_IMPL: computeExpectedValue for a Unary Op");
    return DataTypeT();
  }

  // To be used for the value of -DOPERATOR
  std::optional<std::string> Operator;
  // To be used for the value of -DFUNC
  std::optional<std::string> Intrinsic;
  // Used to add special -D defines to the compiler options.
  std::optional<std::string> SpecialDefines = std::nullopt;
  BasicOpType BasicOpType = BasicOpType_EnumValueCount;
  float Tolerance = 0.0;
  ValidationType ValidationType = ValidationType::ValidationType_Epsilon;
  // Default the TypedOutputVector to use DataTypeT, Ops that don't have a
  // matching output type will override this.
  VariantVector ExpectedVector = std::vector<DataTypeT>{};
  size_t LengthToTest = 0;

  // Just used for logging purposes.
  std::wstring OpTypeName = L"UnknownOpType";
}; // class TestConfig

template <typename DataTypeT>
class TestConfigAsType : public TestConfig<DataTypeT> {
public:
  TestConfigAsType(const OpTypeMetaData<AsTypeOpType> &OpTypeMd);

  void
  computeExpectedValues(const std::vector<DataTypeT> &InputVector1) override;
  void
  computeExpectedValues(const std::vector<DataTypeT> &InputVector1,
                        const std::vector<DataTypeT> &InputVector2) override;
  std::string getHLSLOutputTypeString() const override;
  bool verifyOutput(
      const std::shared_ptr<st::ShaderOpTestResult> &TestResult) override;

private:
  template <typename DataTypeInT>
  HLSLHalf_t asFloat16([[maybe_unused]] const DataTypeInT &A) const {
    LOG_ERROR_FMT_THROW(L"Programmer Error: Invalid AsFloat16 DataTypeInT: %s",
                        typeid(DataTypeInT).name());
    return HLSLHalf_t();
  }

  HLSLHalf_t asFloat16(const HLSLHalf_t &A) const { return HLSLHalf_t(A.Val); }

  HLSLHalf_t asFloat16(const int16_t &A) const {
    return HLSLHalf_t(bit_cast<DirectX::PackedVector::HALF>(A));
  }

  HLSLHalf_t asFloat16(const uint16_t &A) const {
    return HLSLHalf_t(bit_cast<DirectX::PackedVector::HALF>(A));
  }

  template <typename DataTypeInT> float asFloat(const DataTypeInT &) const {
    LOG_ERROR_FMT_THROW(L"Programmer Error: Invalid AsFloat DataTypeInT: %S",
                        typeid(DataTypeInT).name());
    return 0.0f;
  }

  float asFloat(const float &A) const { return float(A); }
  float asFloat(const int32_t &A) const { return bit_cast<float>(A); }
  float asFloat(const uint32_t &A) const { return bit_cast<float>(A); }

  template <typename DataTypeInT>
  int32_t asInt([[maybe_unused]] const DataTypeInT &A) const {
    // This path is unexpected outside of an issue when brining up new tests. So
    // throwing an exception is appropriate.
    LOG_ERROR_FMT_THROW(L"Programmer Error: Invalid AsInt DataTypeInT: %S",
                        typeid(DataTypeInT).name());
    return 0;
  }

  int32_t asInt(const float &A) const { return bit_cast<int32_t>(A); }
  int32_t asInt(const int32_t &A) const { return A; }
  int32_t asInt(const uint32_t &A) const { return bit_cast<int32_t>(A); }

  template <typename DataTypeInT>
  int16_t asInt16([[maybe_unused]] const DataTypeInT &A) const {
    // This path is unexpected outside of an issue when brining up new tests. So
    // throwing an exception is appropriate.
    LOG_ERROR_FMT_THROW(L"Programmer Error: Invalid AsInt16 DataTypeInT: %S",
                        typeid(DataTypeInT).name());
    return 0;
  }

  int16_t asInt16(const HLSLHalf_t &A) const {
    return bit_cast<int16_t>(A.Val);
  }
  int16_t asInt16(const int16_t &A) const { return A; }
  int16_t asInt16(const uint16_t &A) const { return bit_cast<int16_t>(A); }

  template <typename DataTypeInT>
  uint16_t asUint16([[maybe_unused]] const DataTypeInT &A) const {
    // This path is unexpected outside of an issue when brining up new tests. So
    // throwing an exception is appropriate.
    LOG_ERROR_FMT_THROW(L"Programmer Error: Invalid AsUint16 DataTypeInT: %S",
                        typeid(DataTypeInT).name());
    return 0;
  }

  uint16_t asUint16(const HLSLHalf_t &A) const {
    return bit_cast<uint16_t>(A.Val);
  }
  uint16_t asUint16(const uint16_t &A) const { return A; }
  uint16_t asUint16(const int16_t &A) const { return bit_cast<uint16_t>(A); }

  template <typename DataTypeInT>
  unsigned int asUint([[maybe_unused]] const DataTypeInT &A) const {
    // This path is unexpected outside of an issue when brining up new tests. So
    // throwing an exception is appropriate.
    LOG_ERROR_FMT_THROW(L"Programmer Error: Invalid AsUint DataTypeInT: %S",
                        typeid(DataTypeInT).name());
    return 0;
  }

  unsigned int asUint(const unsigned int &A) const { return A; }
  unsigned int asUint(const float &A) const {
    return bit_cast<unsigned int>(A);
  }
  unsigned int asUint(const int &A) const { return bit_cast<unsigned int>(A); }

  template <typename DataTypeInT>
  void splitDouble([[maybe_unused]] const DataTypeInT &A,
                   [[maybe_unused]] uint32_t &LowBits,
                   [[maybe_unused]] uint32_t &HighBits) const {
    // This path is unexpected outside of an issue when brining up new tests. So
    // throwing an exception is appropriate.
    LOG_ERROR_FMT_THROW(L"Programmer Error: splitDouble only accepts a double "
                        L"as input. Have DataTypeInT: %s",
                        typeid(DataTypeInT).name());
  }

  void splitDouble(const double &A, uint32_t &LowBits,
                   uint32_t &HighBits) const {
    uint64_t Bits = 0;
    std::memcpy(&Bits, &A, sizeof(Bits));
    LowBits = static_cast<uint32_t>(Bits & 0xFFFFFFFF);
    HighBits = static_cast<uint32_t>(Bits >> 32);
  }

  template <typename DataTypeInT>
  double asDouble([[maybe_unused]] const DataTypeInT &LowBits,
                  [[maybe_unused]] const DataTypeInT &HighBits) const {
    // This path is unexpected outside of an issue when brining up new tests. So
    // throwing an exception is appropriate.
    LOG_ERROR_FMT_THROW(L"Programmer Error: asDouble only accepts two uint32_t "
                        L"inputs. Have DataTypeInT : %S",
                        typeid(DataTypeInT).name());
    return 0.0;
  }

  double asDouble(const uint32_t &LowBits, const uint32_t &HighBits) const {
    uint64_t Bits = (static_cast<uint64_t>(HighBits) << 32) | LowBits;
    double Result;
    std::memcpy(&Result, &Bits, sizeof(Result));
    return Result;
  }

  AsTypeOpType OpType = AsTypeOpType_EnumValueCount;
};

template <typename DataTypeT>
class TestConfigTrigonometric : public TestConfig<DataTypeT> {
public:
  TestConfigTrigonometric(const OpTypeMetaData<TrigonometricOpType> &OpTypeMd);
  DataTypeT computeExpectedValue(const DataTypeT &A) const override;

private:
  TrigonometricOpType OpType = TrigonometricOpType_EnumValueCount;
};

template <typename DataTypeT>
class TestConfigUnary : public TestConfig<DataTypeT> {
public:
  TestConfigUnary(const OpTypeMetaData<UnaryOpType> &OpTypeMd);
  DataTypeT computeExpectedValue(const DataTypeT &A) const override;

private:
  UnaryOpType OpType = UnaryOpType_EnumValueCount;
};

template <typename DataTypeT>
class TestConfigBinary : public TestConfig<DataTypeT> {
public:
  TestConfigBinary(const OpTypeMetaData<BinaryOpType> &OpTypeMd);
  DataTypeT computeExpectedValue(const DataTypeT &A,
                                 const DataTypeT &B) const override;

private:
  BinaryOpType OpType = BinaryOpType_EnumValueCount;

  // Helpers so we do the right thing for float types. HLSLHalf_t is handled in
  // an operator overload.
  template <typename DataTypeT>
  DataTypeT mod(const DataTypeT &A, const DataTypeT &B) const {
    return A % B;
  }

  template <> float mod(const float &A, const float &B) const {
    return std::fmod(A, B);
  }

  template <> double mod(const double &A, const double &B) const {
    return std::fmod(A, B);
  }
};

template <typename DataTypeT>
std::unique_ptr<TestConfig<DataTypeT>>
makeTestConfig(const OpTypeMetaData<UnaryOpType> &OpTypeMetaData) {
  return std::make_unique<TestConfigUnary<DataTypeT>>(OpTypeMetaData);
}

template <typename DataTypeT>
std::unique_ptr<TestConfig<DataTypeT>>
makeTestConfig(const OpTypeMetaData<BinaryOpType> &OpTypeMetaData) {
  return std::make_unique<TestConfigBinary<DataTypeT>>(OpTypeMetaData);
}

template <typename DataTypeT>
std::unique_ptr<TestConfig<DataTypeT>>
makeTestConfig(const OpTypeMetaData<TrigonometricOpType> &OpTypeMetaData) {
  return std::make_unique<TestConfigTrigonometric<DataTypeT>>(OpTypeMetaData);
}

template <typename DataTypeT>
std::unique_ptr<TestConfig<DataTypeT>>
makeTestConfig(const OpTypeMetaData<AsTypeOpType> &OpTypeMetaData) {
  return std::make_unique<TestConfigAsType<DataTypeT>>(OpTypeMetaData);
}

}; // namespace LongVector

#endif // LONGVECTORS_H
