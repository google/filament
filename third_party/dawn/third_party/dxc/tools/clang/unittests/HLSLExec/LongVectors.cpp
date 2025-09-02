#include "LongVectors.h"
#include "HlslExecTestUtils.h"
#include <iomanip>

namespace LongVector {

template <typename T, size_t Length>
const OpTypeMetaData<T> &
getLongVectorOpType(const OpTypeMetaData<T> (&Values)[Length],
                    const std::wstring &OpTypeString) {
  for (size_t I = 0; I < Length; I++) {
    if (Values[I].OpTypeString == OpTypeString)
      return Values[I];
  }

  LOG_ERROR_FMT_THROW(L"Invalid LongVectorOpType string: %s",
                      OpTypeString.c_str());

  // We need to return something to satisfy the compiler. We can't annotate
  // LOG_ERROR_FMT_THROW with [[noreturn]] because the TAEF VERIFY_* macros that
  // it uses are re-mapped on Unix to not throw exceptions, so they naturally
  // return. If we hit this point it is a programmer error when implementing a
  // test. Specifically, an entry for this OpTypeString is missing in the
  // static LongVectorOpTypeStringToOpMetaData array. Or something has been
  // corrupted. Test execution is invalid at this point. Usin std::abort() keeps
  // the compiler happy about no return path. And LOG_ERROR_FMT_THROW will still
  // provide a useful error message via gtest logging on Unix systems.
  std::abort();
}

const OpTypeMetaData<BinaryOpType> &
getBinaryOpType(const std::wstring &OpTypeString) {
  return getLongVectorOpType<BinaryOpType>(binaryOpTypeStringToOpMetaData,
                                           OpTypeString);
}

const OpTypeMetaData<UnaryOpType> &
getUnaryOpType(const std::wstring &OpTypeString) {
  return getLongVectorOpType<UnaryOpType>(unaryOpTypeStringToOpMetaData,
                                          OpTypeString);
}

const OpTypeMetaData<AsTypeOpType> &
getAsTypeOpType(const std::wstring &OpTypeString) {
  return getLongVectorOpType<AsTypeOpType>(asTypeOpTypeStringToOpMetaData,
                                           OpTypeString);
}

const OpTypeMetaData<TrigonometricOpType> &
getTrigonometricOpType(const std::wstring &OpTypeString) {
  return getLongVectorOpType<TrigonometricOpType>(
      trigonometricOpTypeStringToOpMetaData, OpTypeString);
}

// Helper to fill the test data from the shader buffer based on type. Convenient
// to be used when copying HLSL*_t types so we can use the underlying type.
template <typename DataTypeT>
void fillLongVectorDataFromShaderBuffer(const MappedData &ShaderBuffer,
                                        std::vector<DataTypeT> &TestData,
                                        size_t NumElements) {

  if constexpr (std::is_same_v<DataTypeT, HLSLHalf_t>) {
    auto ShaderBufferPtr =
        static_cast<const DirectX::PackedVector::HALF *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      // HLSLHalf_t has a DirectX::PackedVector::HALF based constructor.
      TestData.push_back(ShaderBufferPtr[I]);
    return;
  }

  if constexpr (std::is_same_v<DataTypeT, HLSLBool_t>) {
    auto ShaderBufferPtr = static_cast<const int32_t *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      // HLSLBool_t has a int32_t based constructor.
      TestData.push_back(ShaderBufferPtr[I]);
    return;
  }

  auto ShaderBufferPtr = static_cast<const DataTypeT *>(ShaderBuffer.data());
  for (size_t I = 0; I < NumElements; I++)
    TestData.push_back(ShaderBufferPtr[I]);
  return;
}

template <typename DataTypeT>
bool doValuesMatch(DataTypeT A, DataTypeT B, float Tolerance, ValidationType) {
  if (Tolerance == 0.0f)
    return A == B;

  DataTypeT Diff = A > B ? A - B : B - A;
  return Diff <= Tolerance;
}

bool doValuesMatch(HLSLBool_t A, HLSLBool_t B, float, ValidationType) {
  return A == B;
}

bool doValuesMatch(HLSLHalf_t A, HLSLHalf_t B, float Tolerance,
                   ValidationType ValidationType) {
  switch (ValidationType) {
  case ValidationType_Epsilon:
    return CompareHalfEpsilon(A.Val, B.Val, Tolerance);
  case ValidationType_Ulp:
    return CompareHalfULP(A.Val, B.Val, Tolerance);
  default:
    WEX::Logging::Log::Error(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

bool doValuesMatch(float A, float B, float Tolerance,
                   ValidationType ValidationType) {
  switch (ValidationType) {
  case ValidationType_Epsilon:
    return CompareFloatEpsilon(A, B, Tolerance);
  case ValidationType_Ulp: {
    // Tolerance is in ULPs. Convert to int for the comparison.
    const int IntTolerance = static_cast<int>(Tolerance);
    return CompareFloatULP(A, B, IntTolerance);
  };
  default:
    WEX::Logging::Log::Error(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

bool doValuesMatch(double A, double B, float Tolerance,
                   ValidationType ValidationType) {
  switch (ValidationType) {
  case ValidationType_Epsilon:
    return CompareDoubleEpsilon(A, B, Tolerance);
  case ValidationType_Ulp: {
    // Tolerance is in ULPs. Convert to int64_t for the comparison.
    const int64_t IntTolerance = static_cast<int64_t>(Tolerance);
    return CompareDoubleULP(A, B, IntTolerance);
  };
  default:
    WEX::Logging::Log::Error(
        L"Invalid ValidationType. Expecting Epsilon or ULP.");
    return false;
  }
}

template <typename DataTypeT>
bool doVectorsMatch(const std::vector<DataTypeT> &ActualValues,
                    const std::vector<DataTypeT> &ExpectedValues,
                    float Tolerance, ValidationType ValidationType) {

  DXASSERT(
      ActualValues.size() == ExpectedValues.size(),
      "Programmer error: Actual and Expected vectors must be the same size.");

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
      WEX::Logging::Log::Error(Wss.str().c_str());
    }
  }

  return false;
}

// A helper to fill the expected vector with computed values. Lets us factor out
// the re-used std::get_if code and the for loop.
template <typename DataTypeT, typename ComputeFnT>
void fillExpectedVector(VariantVector &ExpectedVector, size_t Count,
                        ComputeFnT ComputeFn) {
  auto *TypedExpectedValues =
      std::get_if<std::vector<DataTypeT>>(&ExpectedVector);

  VERIFY_IS_NOT_NULL(TypedExpectedValues,
                     L"Expected vector is not of the correct type.");

  for (size_t Index = 0; Index < Count; ++Index)
    TypedExpectedValues->push_back(ComputeFn(Index));
}

template <typename DataTypeT>
void logLongVector(const std::vector<DataTypeT> &Values,
                   const std::wstring &Name) {
  WEX::Logging::Log::Comment(
      WEX::Common::String().Format(L"LongVector Name: %s", Name.c_str()));

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

  WEX::Logging::Log::Comment(Wss.str().c_str());
}

template <typename DataTypeT> std::string getHLSLTypeString() {
  if (std::is_same_v<DataTypeT, HLSLBool_t>)
    return "bool";
  if (std::is_same_v<DataTypeT, HLSLHalf_t>)
    return "half";
  if (std::is_same_v<DataTypeT, float>)
    return "float";
  if (std::is_same_v<DataTypeT, double>)
    return "double";
  if (std::is_same_v<DataTypeT, int16_t>)
    return "int16_t";
  if (std::is_same_v<DataTypeT, int32_t>)
    return "int";
  if (std::is_same_v<DataTypeT, int64_t>)
    return "int64_t";
  if (std::is_same_v<DataTypeT, uint16_t>)
    return "uint16_t";
  if (std::is_same_v<DataTypeT, uint32_t>)
    return "uint32_t";
  if (std::is_same_v<DataTypeT, uint64_t>)
    return "uint64_t";

  std::string ErrStr("getHLSLTypeString() Unsupported type: ");
  ErrStr.append(typeid(DataTypeT).name());
  VERIFY_IS_TRUE(false, ErrStr.c_str());
  return "UnknownType";
}

// These are helper arrays to be used with the TableParameterHandler that parses
// the LongVectorOpTable.xml file for us.
static TableParameter BinaryOpParameters[] = {
    {L"DataType", TableParameter::STRING, true},
    {L"OpTypeEnum", TableParameter::STRING, true},
    {L"InputValueSetName1", TableParameter::STRING, false},
    {L"InputValueSetName2", TableParameter::STRING, false},
};

static TableParameter UnaryOpParameters[] = {
    {L"DataType", TableParameter::STRING, true},
    {L"OpTypeEnum", TableParameter::STRING, true},
    {L"InputValueSetName1", TableParameter::STRING, false},
};

static TableParameter AsTypeOpParameters[] = {
    // DataTypeOut is determined at runtime based on the OpType.
    // For example...AsUint has an output type of uint32_t.
    {L"DataTypeIn", TableParameter::STRING, true},
    {L"OpTypeEnum", TableParameter::STRING, true},
    {L"InputValueSetName1", TableParameter::STRING, false},
    {L"InputValueSetName2", TableParameter::STRING, false},
};

bool OpTest::classSetup() {
  // Run this only once.
  if (!Initialized) {
    Initialized = true;

    HMODULE Runtime = LoadLibraryW(L"d3d12.dll");
    if (Runtime == NULL)
      return false;
    // Do not: FreeLibrary(hRuntime);
    // If we actually free the library, it defeats the purpose of
    // enableAgilitySDK and enableExperimentalMode.

    HRESULT HR;
    HR = enableAgilitySDK(Runtime);

    if (FAILED(HR))
      hlsl_test::LogCommentFmt(L"Unable to enable Agility SDK - 0x%08x.", HR);
    else if (HR == S_FALSE)
      hlsl_test::LogCommentFmt(L"Agility SDK not enabled.");
    else
      hlsl_test::LogCommentFmt(L"Agility SDK enabled.");

    HR = enableExperimentalMode(Runtime);
    if (FAILED(HR))
      hlsl_test::LogCommentFmt(
          L"Unable to enable shader experimental mode - 0x%08x.", HR);
    else if (HR == S_FALSE)
      hlsl_test::LogCommentFmt(L"Experimental mode not enabled.");
    else
      hlsl_test::LogCommentFmt(L"Experimental mode enabled.");

    HR = enableDebugLayer();
    if (FAILED(HR))
      hlsl_test::LogCommentFmt(L"Unable to enable debug layer - 0x%08x.", HR);
    else if (HR == S_FALSE)
      hlsl_test::LogCommentFmt(L"Debug layer not enabled.");
    else
      hlsl_test::LogCommentFmt(L"Debug layer enabled.");
  }

  return true;
}

TEST_F(OpTest, binaryOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  using namespace WEX::Common;

  const size_t TableSize = sizeof(BinaryOpParameters) / sizeof(TableParameter);
  TableParameterHandler Handler(BinaryOpParameters, TableSize);

  std::wstring DataType(Handler.GetTableParamByName(L"DataType")->m_str);
  std::wstring OpTypeString(Handler.GetTableParamByName(L"OpTypeEnum")->m_str);

  auto OpTypeMD = getBinaryOpType(OpTypeString);
  dispatchTestByDataType(OpTypeMD, DataType, Handler);
}

TEST_F(OpTest, trigonometricOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  const size_t TableSize = sizeof(UnaryOpParameters) / sizeof(TableParameter);
  TableParameterHandler Handler(UnaryOpParameters, TableSize);

  std::wstring DataType(Handler.GetTableParamByName(L"DataType")->m_str);
  std::wstring OpTypeString(Handler.GetTableParamByName(L"OpTypeEnum")->m_str);

  auto OpTypeMD = getTrigonometricOpType(OpTypeString);
  dispatchTestByDataType(OpTypeMD, DataType, Handler);
}

TEST_F(OpTest, unaryOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  const size_t TableSize = sizeof(UnaryOpParameters) / sizeof(TableParameter);
  TableParameterHandler Handler(UnaryOpParameters, TableSize);

  std::wstring DataType(Handler.GetTableParamByName(L"DataType")->m_str);
  std::wstring OpTypeString(Handler.GetTableParamByName(L"OpTypeEnum")->m_str);

  auto OpTypeMD = getUnaryOpType(OpTypeString);
  dispatchTestByDataType(OpTypeMD, DataType, Handler);
}

TEST_F(OpTest, asTypeOpTest) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  const size_t TableSize = sizeof(AsTypeOpParameters) / sizeof(TableParameter);
  TableParameterHandler Handler(AsTypeOpParameters, TableSize);

  std::wstring DataTypeIn(Handler.GetTableParamByName(L"DataTypeIn")->m_str);
  std::wstring OpTypeString(Handler.GetTableParamByName(L"OpTypeEnum")->m_str);

  auto OpTypeMD = getAsTypeOpType(OpTypeString);
  dispatchTestByDataType(OpTypeMD, DataTypeIn, Handler);
}

template <typename LongVectorOpTypeT>
void OpTest::dispatchTestByDataType(
    const OpTypeMetaData<LongVectorOpTypeT> &OpTypeMd, std::wstring DataType,
    TableParameterHandler &Handler) {
  using namespace WEX::Common;

  if (DataType == L"bool")
    dispatchTestByVectorLength<HLSLBool_t>(OpTypeMd, Handler);
  else if (DataType == L"int16")
    dispatchTestByVectorLength<int16_t>(OpTypeMd, Handler);
  else if (DataType == L"int32")
    dispatchTestByVectorLength<int32_t>(OpTypeMd, Handler);
  else if (DataType == L"int64")
    dispatchTestByVectorLength<int64_t>(OpTypeMd, Handler);
  else if (DataType == L"uint16")
    dispatchTestByVectorLength<uint16_t>(OpTypeMd, Handler);
  else if (DataType == L"uint32")
    dispatchTestByVectorLength<uint32_t>(OpTypeMd, Handler);
  else if (DataType == L"uint64")
    dispatchTestByVectorLength<uint64_t>(OpTypeMd, Handler);
  else if (DataType == L"float16")
    dispatchTestByVectorLength<HLSLHalf_t>(OpTypeMd, Handler);
  else if (DataType == L"float32")
    dispatchTestByVectorLength<float>(OpTypeMd, Handler);
  else if (DataType == L"float64")
    dispatchTestByVectorLength<double>(OpTypeMd, Handler);
  else
    VERIFY_FAIL(
        String().Format(L"DataType: %ls is not recognized.", DataType.c_str()));
}

template <>
void OpTest::dispatchTestByDataType(
    const OpTypeMetaData<TrigonometricOpType> &OpTypeMd, std::wstring DataType,
    TableParameterHandler &Handler) {
  using namespace WEX::Common;

  if (DataType == L"float16")
    dispatchTestByVectorLength<HLSLHalf_t>(OpTypeMd, Handler);
  else if (DataType == L"float32")
    dispatchTestByVectorLength<float>(OpTypeMd, Handler);
  else if (DataType == L"float64")
    dispatchTestByVectorLength<double>(OpTypeMd, Handler);
  else
    LOG_ERROR_FMT_THROW(
        L"Trigonometric ops are only supported for floating point types. "
        L"DataType: %ls is not recognized.",
        DataType.c_str());
}

template <typename DataTypeT, typename LongVectorOpTypeT>
void OpTest::dispatchTestByVectorLength(
    const OpTypeMetaData<LongVectorOpTypeT> &OpTypeMd,
    TableParameterHandler &Handler) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  auto TestConfig = makeTestConfig<DataTypeT>(OpTypeMd);

  // InputValueSetName1 is optional. So the string may be empty. An empty
  // string will result in the default value set for this DataType being used.
  std::wstring InputValueSet1(
      Handler.GetTableParamByName(L"InputValueSetName1")->m_str);
  if (!InputValueSet1.empty())
    TestConfig->setInputValueSet1(InputValueSet1);

  // InputValueSetName2 is optional. So the string may be empty. An empty
  // string will result in the default value set for this DataType being used.
  if (TestConfig->isBinaryOp()) {
    std::wstring InputValueSet2(
        Handler.GetTableParamByName(L"InputValueSetName2")->m_str);
    if (!InputValueSet2.empty())
      TestConfig->setInputValueSet2(InputValueSet2);
  }

  // Manual override to test a specific vector size. Convenient for debugging
  // issues.
  size_t InputSizeToTestOverride = 0;
  WEX::TestExecution::RuntimeParameters::TryGetValue(L"LongVectorInputSize",
                                                     InputSizeToTestOverride);

  std::vector<size_t> InputVectorSizes;
  if (InputSizeToTestOverride)
    InputVectorSizes.push_back(InputSizeToTestOverride);
  else
    InputVectorSizes = {3, 4, 5, 16, 17, 35, 100, 256, 1024};

  for (auto SizeToTest : InputVectorSizes) {
    // We could create a new config for each test case with the new length, but
    // that feels wasteful. Instead, we just update the length to test.
    TestConfig->setLengthToTest(SizeToTest);
    testBaseMethod<DataTypeT>(TestConfig);
  }
}

template <typename DataTypeT>
void OpTest::testBaseMethod(
    std::unique_ptr<TestConfig<DataTypeT>> &TestConfig) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  const size_t VectorLengthToTest = TestConfig->getLengthToTest();

  hlsl_test::LogCommentFmt(L"Running LongVectorOpTestBase<%S, %zu>",
                           typeid(DataTypeT).name(), VectorLengthToTest);

  bool LogInputs = false;
  WEX::TestExecution::RuntimeParameters::TryGetValue(L"LongVectorLogInputs",
                                                     LogInputs);

  CComPtr<ID3D12Device> D3DDevice;
  if (!createDevice(&D3DDevice, ExecTestUtils::D3D_SHADER_MODEL_6_9, false)) {
#ifdef _HLK_CONF
    LOG_ERROR_FMT_THROW(
        L"Device does not support SM 6.9. Can't run these tests.");
#else
    WEX::Logging::Log::Comment(
        "Device does not support SM 6.9. Can't run these tests.");
    WEX::Logging::Log::Result(WEX::Logging::TestResults::Skipped);
    return;
#endif
  }

  std::vector<DataTypeT> InputVector1;
  InputVector1.reserve(VectorLengthToTest);
  std::vector<DataTypeT> InputVector2; // May be unused, but must be defined.
  InputVector2.reserve(VectorLengthToTest);
  std::vector<DataTypeT> ScalarInput; // May be unused, but must be defined.
  const bool IsVectorBinaryOp =
      TestConfig->isBinaryOp() && !TestConfig->isScalarOp();

  std::vector<DataTypeT> InputVector1ValueSet = TestConfig->getInputValueSet1();
  std::vector<DataTypeT> InputVector2ValueSet =
      TestConfig->isBinaryOp() ? TestConfig->getInputValueSet2()
                               : std::vector<DataTypeT>();

  if (TestConfig->isScalarOp())
    // Scalar ops are always binary ops. So InputVector2ValueSet is initialized
    // with values above.
    ScalarInput.push_back(InputVector2ValueSet[0]);

  // Fill the input vectors with values from the value set. Repeat the values
  // when we reach the end of the value set.
  for (size_t Index = 0; Index < VectorLengthToTest; Index++) {
    InputVector1.push_back(
        InputVector1ValueSet[Index % InputVector1ValueSet.size()]);

    if (IsVectorBinaryOp)
      InputVector2.push_back(
          InputVector2ValueSet[Index % InputVector2ValueSet.size()]);
  }

  if (IsVectorBinaryOp)
    TestConfig->computeExpectedValues(InputVector1, InputVector2);
  else if (TestConfig->isScalarOp())
    TestConfig->computeExpectedValues(InputVector1, ScalarInput[0]);
  else // Must be a unary op
    TestConfig->computeExpectedValues(InputVector1);

  if (LogInputs) {
    logLongVector(InputVector1, L"InputVector1");

    if (IsVectorBinaryOp)
      logLongVector(InputVector2, L"InputVector2");
    else if (TestConfig->isScalarOp())
      logLongVector(ScalarInput, L"ScalarInput");
  }

  // We have to construct the string outside of the lambda. Otherwise it's
  // cleaned up when the lambda finishes executing but before the shader runs.
  std::string CompilerOptionsString = TestConfig->getCompilerOptionsString();

  // The name of the shader we want to use in ShaderOpArith.xml. Could also add
  // logic to set this name in ShaderOpArithTable.xml so we can use different
  // shaders for different tests.
  LPCSTR ShaderName = "LongVectorOp";
  // ShaderOpArith.xml defines the input/output resources and the shader source.
  CComPtr<IStream> TestXML;
  readHlslDataIntoNewStream(L"ShaderOpArith.xml", &TestXML, DxcDllSupport);

  // RunShaderOpTest is a helper function that handles resource creation
  // and setup. It also handles the shader compilation and execution. It takes a
  // callback that is called when the shader is compiled, but before it is
  // executed.
  std::shared_ptr<st::ShaderOpTestResult> TestResult = st::RunShaderOpTest(
      D3DDevice, DxcDllSupport, TestXML, ShaderName,
      [&](LPCSTR Name, std::vector<BYTE> &ShaderData, st::ShaderOp *ShaderOp) {
        hlsl_test::LogCommentFmt(L"RunShaderOpTest CallBack. Resource Name: %S",
                                 Name);

        // This callback is called once for each resource defined for
        // "LongVectorOp" in ShaderOpArith.xml. All callbacks are fired for each
        // resource. We determine whether they are applicable to the test case
        // when they run.

        // Process the callback for the OutputVector resource.
        if (0 == _stricmp(Name, "OutputVector")) {
          // We only need to set the compiler options string once. So this is a
          // convenient place to do it.
          ShaderOp->Shaders.at(0).Arguments = CompilerOptionsString.c_str();

          return;
        }

        // Process the callback for the InputFuncArgs resource.
        if (0 == _stricmp(Name, "InputFuncArgs")) {
          if (TestConfig->isScalarOp())
            fillShaderBufferFromLongVectorData(ShaderData, ScalarInput);
          return;
        }

        // Process the callback for the InputVector1 resource.
        if (0 == _stricmp(Name, "InputVector1")) {
          fillShaderBufferFromLongVectorData(ShaderData, InputVector1);
          return;
        }

        // Process the callback for the InputVector2 resource.
        if (0 == _stricmp(Name, "InputVector2")) {
          if (IsVectorBinaryOp)
            fillShaderBufferFromLongVectorData(ShaderData, InputVector2);
          return;
        }

        LOG_ERROR_FMT_THROW(
            L"RunShaderOpTest CallBack. Unexpected Resource Name: %S", Name);
      });

  // The TestConfig object handles the logic for extracting the shader output
  // based on the op type.
  VERIFY_SUCCEEDED(TestConfig->verifyOutput(TestResult));
}

// Helper to fill the shader buffer based on type. Convenient to be used when
// copying HLSL*_t types so we can copy the underlying type directly instead of
// the struct.
template <typename DataTypeT>
void fillShaderBufferFromLongVectorData(
    std::vector<BYTE> &ShaderBuffer, const std::vector<DataTypeT> &TestData) {

  // Note: DataSize for HLSLHalf_t and HLSLBool_t may be larger than the
  // underlying type in some cases. Thats fine. Resize just makes sure we have
  // enough space.
  const size_t NumElements = TestData.size();
  const size_t DataSize = sizeof(DataTypeT) * NumElements;
  ShaderBuffer.resize(DataSize);

  if constexpr (std::is_same_v<DataTypeT, HLSLHalf_t>) {
    auto ShaderBufferPtr =
        reinterpret_cast<DirectX::PackedVector::HALF *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      ShaderBufferPtr[I] = TestData[I].Val;
    return;
  }

  if constexpr (std::is_same_v<DataTypeT, HLSLBool_t>) {
    auto ShaderBufferPtr = reinterpret_cast<int32_t *>(ShaderBuffer.data());
    for (size_t I = 0; I < NumElements; I++)
      ShaderBufferPtr[I] = TestData[I].Val;
    return;
  }

  auto ShaderBufferPtr = reinterpret_cast<DataTypeT *>(ShaderBuffer.data());
  for (size_t I = 0; I < NumElements; I++)
    ShaderBufferPtr[I] = TestData[I];
  return;
}

template <typename DataTypeT>
std::string TestConfig<DataTypeT>::getHLSLInputTypeString() const {
  return getHLSLTypeString<DataTypeT>();
}

template <typename DataTypeT>
std::string TestConfig<DataTypeT>::getHLSLOutputTypeString() const {

  // Normal case, output matches input type ( DataTypeT )
  if (auto *Vec = std::get_if<std::vector<DataTypeT>>(&ExpectedVector))
    return getHLSLTypeString<DataTypeT>();

  // Non normal cases should be handled in a derived TestConfig class. i.e
  // TestConfigAsType::getHLSLOutputTypeString()
  LOG_ERROR_FMT_THROW(
      L"getHLSLOutputTypeString() called with an unsupported op type: %ls",
      OpTypeName.c_str());
  return "UnknownType";
}

// Returns the compiler options string to be used for the shader compilation.
// Reference ShaderOpArith.xml and the 'LongVectorOp' shader source to see how
// the defines are used in the shader code.
template <typename DataTypeT>
std::string TestConfig<DataTypeT>::getCompilerOptionsString() const {

  std::stringstream CompilerOptions("");
  std::string HLSLInputType = getHLSLInputTypeString();

  CompilerOptions << "-DTYPE=";
  CompilerOptions << HLSLInputType;
  CompilerOptions << " -DNUM=";
  CompilerOptions << LengthToTest;
  const bool Is16BitType =
      (HLSLInputType == "int16_t" || HLSLInputType == "uint16_t" ||
       HLSLInputType == "half");
  CompilerOptions << (Is16BitType ? " -enable-16bit-types" : "");
  CompilerOptions << " -DOPERATOR=";
  CompilerOptions << (Operator ? *Operator : " ");

  if (isBinaryOp()) {
    CompilerOptions << " -DOPERAND2=";
    CompilerOptions << (isScalarOp() ? "InputScalar" : "InputVector2");

    if (isScalarOp())
      CompilerOptions << " -DIS_SCALAR_OP=1";
    else
      CompilerOptions << " -DIS_BINARY_VECTOR_OP=1";

    CompilerOptions << " -DFUNC=";
    CompilerOptions << (Intrinsic ? *Intrinsic : "");
  } else { // Unary Op
    CompilerOptions << " -DFUNC=";
    CompilerOptions << (Intrinsic ? *Intrinsic : "");
    // Not used for unary ops, but needs to be a " " for compilation of the
    // shader after macro expansion.
    CompilerOptions << " -DOPERAND2= ";
  }

  // For most of the ops this string is empty.
  CompilerOptions << (SpecialDefines ? *SpecialDefines : " ");

  std::string HLSLOutputType = getHLSLOutputTypeString();
  CompilerOptions << " -DOUT_TYPE=";
  CompilerOptions << HLSLOutputType;

  return CompilerOptions.str();
}

template <typename DataTypeT>
std::vector<DataTypeT>
TestConfig<DataTypeT>::getInputValueSet(size_t ValueSetIndex) const {

  // Calling with ValueSetIndex == 2 is only valid for binary ops.
  DXASSERT_NOMSG(!(ValueSetIndex == 2 && !isBinaryOp()));

  std::wstring InputValueSetName = L"";
  if (ValueSetIndex == 1)
    InputValueSetName = InputValueSetName1;
  else if (ValueSetIndex == 2)
    InputValueSetName = InputValueSetName2;
  else
    VERIFY_FAIL("Invalid ValueSetIndex");

  return getInputValueSetByKey<DataTypeT>(InputValueSetName);
}

// Public version of verifyOutput. Handles logic to dispatch to the correct
// templated verifyOutput based on the expected output type.
template <typename DataTypeT>
bool TestConfig<DataTypeT>::verifyOutput(
    const std::shared_ptr<st::ShaderOpTestResult> &TestResult) {
  // First try the most common case where the output datatype matches the input
  // datatype (DataTypeT). std::get_if will return a null pointer if the variant
  // isn't holding a std::vector<DataTypeT>
  if (auto TypedExpectedValues =
          std::get_if<std::vector<DataTypeT>>(&ExpectedVector)) {
    return verifyOutput<DataTypeT>(TestResult);
  }

  // If we get here, its likely a programmer error. DataTypeT is the DataType
  // passed in to the TestConfig when its created. The only time the
  // ExpectedVector has a diffferent type is for a handful of ops, such as
  // casting, where the output type is different from the input type. But proper
  // dispatching to verifyOutput with the correct data type is intended to be
  // handled by derived classes. Hence, we throw an error here. See callers of
  // the private version of verifyOutput for examples of proper usage.
  LOG_ERROR_FMT_THROW(
      L"verifyOutput() called with an unsupported expected vector type: %ls.",
      typeid(ExpectedVector).name());
  return false;
}

// Private version of verifyOutput. Expected to be called internally when we've
// resolved what the expected output type is.
template <typename DataTypeT>
template <typename OutputDataTypeT>
bool TestConfig<DataTypeT>::verifyOutput(
    const std::shared_ptr<st::ShaderOpTestResult> &TestResult) {

  if (auto TypedExpectedValues =
          std::get_if<std::vector<OutputDataTypeT>>(&ExpectedVector)) {
    MappedData ShaderOutData;
    TestResult->Test->GetReadBackData("OutputVector", &ShaderOutData);

    // For most of the ops, the output vector size is the same as the input
    // vector size. But some, such as the AsUint_SplitDouble op, have an
    // output vector size that is double the input vector size.
    const size_t OutputVectorSize = (*TypedExpectedValues).size();

    std::vector<OutputDataTypeT> ActualValues;
    fillLongVectorDataFromShaderBuffer(ShaderOutData, ActualValues,
                                       OutputVectorSize);

    return doVectorsMatch(ActualValues, *TypedExpectedValues, Tolerance,
                          ValidationType);
  }

  // This is the private TestConfig::VerifyOutput. If this is hitting its most
  // likely a new test case for a new op type that is misconfigured.
  LOG_ERROR_FMT_THROW(L"PRIVATE verifyOutput() called with an unsupported "
                      L"expected vector type: %ls.",
                      typeid(ExpectedVector).name());
  return false;
}

// Generic computeExpectedValues for Unary ops. Derived classes override
// computeExpectedValue.
template <typename DataTypeT>
void TestConfig<DataTypeT>::computeExpectedValues(
    const std::vector<DataTypeT> &InputVector1) {
  fillExpectedVector<DataTypeT>(
      ExpectedVector, InputVector1.size(),
      [&](size_t Index) { return computeExpectedValue(InputVector1[Index]); });
}

// Generic computeExpectedValues for Binary ops. Derived classes override
// computeExpectedValue.
template <typename DataTypeT>
void TestConfig<DataTypeT>::computeExpectedValues(
    const std::vector<DataTypeT> &InputVector1,
    const std::vector<DataTypeT> &InputVector2) {
  fillExpectedVector<DataTypeT>(
      ExpectedVector, InputVector1.size(), [&](size_t Index) {
        return computeExpectedValue(InputVector1[Index], InputVector2[Index]);
      });
}

// Generic computeExpectedValues for Scalar Binary ops. Derived classes override
// computeExpectedValue.
template <typename DataTypeT>
void TestConfig<DataTypeT>::computeExpectedValues(
    const std::vector<DataTypeT> &InputVector1, const DataTypeT &ScalarInput) {
  fillExpectedVector<DataTypeT>(
      ExpectedVector, InputVector1.size(), [&](size_t Index) {
        return computeExpectedValue(InputVector1[Index], ScalarInput);
      });
}

template <typename DataTypeT>
TestConfigAsType<DataTypeT>::TestConfigAsType(
    const OpTypeMetaData<AsTypeOpType> &OpTypeMd)
    : TestConfig<DataTypeT>(OpTypeMd), OpType(OpTypeMd.OpType) {

  BasicOpType = BasicOpType_Unary;

  switch (OpType) {
  case AsTypeOpType_AsFloat16:
    ExpectedVector = std::vector<HLSLHalf_t>{};
    break;
  case AsTypeOpType_AsFloat:
    ExpectedVector = std::vector<float>{};
    break;
  case AsTypeOpType_AsInt:
    ExpectedVector = std::vector<int32_t>{};
    break;
  case AsTypeOpType_AsInt16:
    ExpectedVector = std::vector<int16_t>{};
    break;
  case AsTypeOpType_AsUint:
    ExpectedVector = std::vector<uint32_t>{};
    break;
  case AsTypeOpType_AsUint_SplitDouble:
    SpecialDefines = " -DFUNC_ASUINT_SPLITDOUBLE=1";
    ExpectedVector = std::vector<uint32_t>{};
    break;
  case AsTypeOpType_AsUint16:
    ExpectedVector = std::vector<uint16_t>{};
    break;
  case AsTypeOpType_AsDouble:
    ExpectedVector = std::vector<double>{};
    BasicOpType = BasicOpType_Binary;
    break;
  default:
    VERIFY_FAIL("Invalid AsTypeOpType");
  }
}

template <typename DataTypeT>
void TestConfigAsType<DataTypeT>::computeExpectedValues(
    const std::vector<DataTypeT> &InputVector1) {

  switch (OpType) {
  case AsTypeOpType_AsFloat16:
    fillExpectedVector<HLSLHalf_t>(
        ExpectedVector, InputVector1.size(),
        [&](size_t Index) { return asFloat16(InputVector1[Index]); });
    return;
  case AsTypeOpType_AsFloat:
    fillExpectedVector<float>(
        ExpectedVector, InputVector1.size(),
        [&](size_t Index) { return asFloat(InputVector1[Index]); });
    return;
  case AsTypeOpType_AsInt:
    fillExpectedVector<int32_t>(
        ExpectedVector, InputVector1.size(),
        [&](size_t Index) { return asInt(InputVector1[Index]); });
    return;
  case AsTypeOpType_AsInt16:
    fillExpectedVector<int16_t>(
        ExpectedVector, InputVector1.size(),
        [&](size_t Index) { return asInt16(InputVector1[Index]); });
    return;
  case AsTypeOpType_AsUint:
    fillExpectedVector<uint32_t>(
        ExpectedVector, InputVector1.size(),
        [&](size_t Index) { return asUint(InputVector1[Index]); });
    return;
  case AsTypeOpType_AsUint_SplitDouble: {
    // SplitDouble is a special case. We fill the first half of the expected
    // vector with the expected low bits of each input double and the second
    // half with the high bits of each input double. Doing things this way
    // helps keep the rest of the generic logic in the LongVector test code
    // simple.
    auto *TypedExpectedValues =
        std::get_if<std::vector<uint32_t>>(&ExpectedVector);
    VERIFY_IS_NOT_NULL(TypedExpectedValues,
                       L"Expected vector is not of the correct type.");
    TypedExpectedValues->resize(InputVector1.size() * 2);
    uint32_t LowBits, HighBits;
    const size_t InputSize = InputVector1.size();
    for (size_t Index = 0; Index < InputSize; ++Index) {
      splitDouble(InputVector1[Index], LowBits, HighBits);
      (*TypedExpectedValues)[Index] = LowBits;
      (*TypedExpectedValues)[Index + InputSize] = HighBits;
    }
    return;
  }
  case AsTypeOpType_AsUint16:
    fillExpectedVector<uint16_t>(
        ExpectedVector, InputVector1.size(),
        [&](size_t Index) { return asUint16(InputVector1[Index]); });
    return;
  default:
    LOG_ERROR_FMT_THROW(L"Unsupported AsType op: %ls", OpTypeName.c_str());
  }
}

template <typename DataTypeT>
void TestConfigAsType<DataTypeT>::computeExpectedValues(
    const std::vector<DataTypeT> &InputVector1,
    const std::vector<DataTypeT> &InputVector2) {

  // AsTypeOpType_AsDouble is the only binary op type for AsType. The rest are
  // Unary ops.
  DXASSERT_NOMSG(OpType == AsTypeOpType_AsDouble);

  fillExpectedVector<double>(
      ExpectedVector, InputVector1.size(), [&](size_t Index) {
        return asDouble(InputVector1[Index], InputVector2[Index]);
      });
}

template <typename DataTypeT>
std::string TestConfigAsType<DataTypeT>::getHLSLOutputTypeString() const {

  switch (OpType) {
  case AsTypeOpType_AsFloat16:
    return getHLSLTypeString<HLSLHalf_t>();
  case AsTypeOpType_AsFloat:
    return getHLSLTypeString<float>();
  case AsTypeOpType_AsInt:
    return getHLSLTypeString<int32_t>();
  case AsTypeOpType_AsInt16:
    return getHLSLTypeString<int16_t>();
  case AsTypeOpType_AsUint:
    return getHLSLTypeString<uint32_t>();
  case AsTypeOpType_AsUint_SplitDouble:
    return getHLSLTypeString<uint32_t>();
  case AsTypeOpType_AsUint16:
    return getHLSLTypeString<uint16_t>();
  case AsTypeOpType_AsDouble:
    return getHLSLTypeString<double>();
  default:
    LOG_ERROR_FMT_THROW(
        L"getHLSLOutputTypeString() called with an unsupported op type: %ls",
        OpTypeName.c_str());
    return "UnknownType";
  }
}

// Override verifyOutput for AsTypeOpType as the output type for these ops
// doesn't match the input type of the config. Calls a private templated version
// of verifyOutput with the correct data type based on the op.
template <typename DataTypeT>
bool TestConfigAsType<DataTypeT>::verifyOutput(
    const std::shared_ptr<st::ShaderOpTestResult> &TestResult) {

  switch (OpType) {
  case AsTypeOpType_AsFloat:
    return TestConfig::verifyOutput<float>(TestResult);
  case AsTypeOpType_AsFloat16:
    return TestConfig::verifyOutput<HLSLHalf_t>(TestResult);
  case AsTypeOpType_AsInt:
    return TestConfig::verifyOutput<int32_t>(TestResult);
  case AsTypeOpType_AsInt16:
    return TestConfig::verifyOutput<int16_t>(TestResult);
  case AsTypeOpType_AsUint:
    return TestConfig::verifyOutput<uint32_t>(TestResult);
  case AsTypeOpType_AsUint_SplitDouble:
    return TestConfig::verifyOutput<uint32_t>(TestResult);
  case AsTypeOpType_AsUint16:
    return TestConfig::verifyOutput<uint16_t>(TestResult);
  case AsTypeOpType_AsDouble:
    return TestConfig::verifyOutput<double>(TestResult);
  default:
    LOG_ERROR_FMT_THROW(
        L"verifyOutput() called with an unsupported AsTypeOpType: %ls",
        OpTypeName.c_str());
    return false;
  }
}

template <typename DataTypeT>
TestConfigTrigonometric<DataTypeT>::TestConfigTrigonometric(
    const OpTypeMetaData<TrigonometricOpType> &OpTypeMd)
    : TestConfig<DataTypeT>(OpTypeMd), OpType(OpTypeMd.OpType) {

  static_assert(
      isFloatingPointType<DataTypeT>(),
      "Trigonometric ops are only supported for floating point types.");

  BasicOpType = BasicOpType_Unary;

  // All trigonometric ops are floating point types.
  // These trig functions are defined to have a max absolute error of 0.0008
  // as per the D3D functional specs. An example with this spec for sin and
  // cos is available here:
  // https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#22.10.20
  ValidationType = ValidationType_Epsilon;
  if (std::is_same_v<DataTypeT, HLSLHalf_t>)
    Tolerance = 0.0010f;
  else if (std::is_same_v<DataTypeT, float>)
    Tolerance = 0.0008f;
}

// computeExpectedValue Trigonometric
template <typename DataTypeT>
DataTypeT TestConfigTrigonometric<DataTypeT>::computeExpectedValue(
    const DataTypeT &A) const {

  switch (OpType) {
  case TrigonometricOpType_Acos:
    return std::acos(A);
  case TrigonometricOpType_Asin:
    return std::asin(A);
  case TrigonometricOpType_Atan:
    return std::atan(A);
  case TrigonometricOpType_Cos:
    return std::cos(A);
  case TrigonometricOpType_Cosh:
    return std::cosh(A);
  case TrigonometricOpType_Sin:
    return std::sin(A);
  case TrigonometricOpType_Sinh:
    return std::sinh(A);
  case TrigonometricOpType_Tan:
    return std::tan(A);
  case TrigonometricOpType_Tanh:
    return std::tanh(A);
  default:
    LOG_ERROR_FMT_THROW(L"Unknown TrigonometricOpType: %ls",
                        OpTypeName.c_str());
    return DataTypeT();
  }
}

template <typename DataTypeT>
TestConfigUnary<DataTypeT>::TestConfigUnary(
    const OpTypeMetaData<UnaryOpType> &OpTypeMd)
    : TestConfig<DataTypeT>(OpTypeMd), OpType(OpTypeMd.OpType) {

  BasicOpType = BasicOpType_Unary;

  switch (OpType) {
  case UnaryOpType_Initialize:
    SpecialDefines = " -DFUNC_INITIALIZE=1";
    break;
  default:
    VERIFY_FAIL("Invalid UnaryOpType");
  }
}

template <typename DataTypeT>
DataTypeT
TestConfigUnary<DataTypeT>::computeExpectedValue(const DataTypeT &A) const {
  if (OpType != UnaryOpType_Initialize) {
    LOG_ERROR_FMT_THROW(L"computeExpectedValue(const DataTypeT &A, "
                        L"UnaryOpType OpType) called on an "
                        L"unrecognized unary op: %ls",
                        OpTypeName.c_str());
    return DataTypeT();
  }

  return DataTypeT(A);
}

template <typename DataTypeT>
TestConfigBinary<DataTypeT>::TestConfigBinary(
    const OpTypeMetaData<BinaryOpType> &OpTypeMd)
    : TestConfig<DataTypeT>(OpTypeMd), OpType(OpTypeMd.OpType) {

  if (isFloatingPointType<DataTypeT>()) {
    Tolerance = 1;
    ValidationType = ValidationType_Ulp;
  }

  switch (OpType) {
  case BinaryOpType_ScalarAdd:
  case BinaryOpType_ScalarMultiply:
  case BinaryOpType_ScalarSubtract:
  case BinaryOpType_ScalarDivide:
  case BinaryOpType_ScalarModulus:
  case BinaryOpType_ScalarMin:
  case BinaryOpType_ScalarMax:
    BasicOpType = BasicOpType_ScalarBinary;
    break;
  case BinaryOpType_Multiply:
  case BinaryOpType_Add:
  case BinaryOpType_Subtract:
  case BinaryOpType_Divide:
  case BinaryOpType_Modulus:
  case BinaryOpType_Min:
  case BinaryOpType_Max:
    BasicOpType = BasicOpType_Binary;
    break;
  default:
    VERIFY_FAIL("Invalid BinaryOpType");
  }
}

template <typename DataTypeT>
DataTypeT
TestConfigBinary<DataTypeT>::computeExpectedValue(const DataTypeT &A,
                                                  const DataTypeT &B) const {
  switch (OpType) {
  case BinaryOpType_ScalarAdd:
    return A + B;
  case BinaryOpType_ScalarMultiply:
    return A * B;
  case BinaryOpType_ScalarSubtract:
    return A - B;
  case BinaryOpType_ScalarDivide:
    return A / B;
  case BinaryOpType_ScalarModulus:
    return mod(A, B);
  case BinaryOpType_Multiply:
    return A * B;
  case BinaryOpType_Add:
    return A + B;
  case BinaryOpType_Subtract:
    return A - B;
  case BinaryOpType_Divide:
    return A / B;
  case BinaryOpType_Modulus:
    return mod(A, B);
  case BinaryOpType_Min:
    // std::max and std::min are wrapped in () to avoid collisions with the //
    // macro defintions for min and max in windows.h
    return (std::min)(A, B);
  case BinaryOpType_Max:
    return (std::max)(A, B);
  case BinaryOpType_ScalarMin:
    return (std::min)(A, B);
  case BinaryOpType_ScalarMax:
    return (std::max)(A, B);
  default:
    LOG_ERROR_FMT_THROW(L"Unknown BinaryOpType: %ls", OpTypeName.c_str());
    return DataTypeT();
  }
}

}; // namespace LongVector