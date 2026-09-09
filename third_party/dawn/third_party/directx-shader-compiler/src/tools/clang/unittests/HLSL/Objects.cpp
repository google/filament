///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Objects.cpp                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HLSLTestData.h"
#include <stdint.h>

#include "dxc/Test/HlslTestUtils.h"

#include <exception>
#include <set>

///////////////////////////////////////////////////////////////////////////////
// Utilities.

namespace std {

/// Returns the first element of the container.
template <class Container>
auto first(Container &cont) -> decltype(*begin(cont)) {
  auto iter = begin(cont);
  return *iter;
}

/// Returns the first element of the container or the default value if empty.
template <typename Container, typename ElementType>
ElementType first_or_default(Container &cont, const ElementType &defaultValue) {
  auto iter = begin(cont);
  auto endIter = end(cont);
  if (iter == endIter)
    return defaultValue;
  return *iter;
}

} // namespace std

template <typename TEnumeration> class EnumFlagsIterator {
private:
  unsigned long _value;

public:
  using iterator_category = std::input_iterator_tag;
  using value_type = TEnumeration;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  EnumFlagsIterator(TEnumeration value) : _value(value) {}

  TEnumeration operator*() const {
    unsigned long l;
    _BitScanForward(&l, _value);
    return static_cast<TEnumeration>(1 << l);
  }

  EnumFlagsIterator<TEnumeration> &operator++() {
    unsigned long l;
    _BitScanForward(&l, _value);
    _value = _value & ~(1 << l);
    return *this;
  }

  bool equal(const EnumFlagsIterator<TEnumeration> &other) {
    return _value == other._value;
  }

  bool operator==(const EnumFlagsIterator<TEnumeration> &other) {
    return _value == other._value;
  }

  bool operator!=(const EnumFlagsIterator<TEnumeration> &other) {
    return _value != other._value;
  }
};

///////////////////////////////////////////////////////////////////////////////
// Shader model test data.

enum ShaderModel { ShaderModel4 = 4, ShaderModel5 = 5 };

enum ShaderType {
  /// Vertex shader.
  ST_Vertex = 1 << 0,

  /// Hull shader.
  ST_Hull = 1 << 1,

  /// Domain shader.
  ST_Domain = 1 << 2,

  /// Geometry shader type.
  ST_Geometry = 1 << 3,

  /// Pixel shader type.
  ST_Pixel = 1 << 4,

  /// Compute shader type.
  ST_Compute = 1 << 5,

  /// Hull and geometry shader types.
  ST_HullGeometry = ST_Hull | ST_Geometry,

  /// Hull and domain  shader types.
  ST_HullDomain = ST_Hull | ST_Domain,

  /// Pixel and compute shader types.
  ST_PixelCompute = ST_Pixel | ST_Compute,

  /// All shader types.
  ST_All = ST_Vertex | ST_Hull | ST_Domain | ST_Geometry | ST_Pixel | ST_Compute
};

typedef EnumFlagsIterator<ShaderType> ShaderTypeIterator;
ShaderTypeIterator begin(ShaderType value) { return ShaderTypeIterator(value); }
ShaderTypeIterator end(ShaderType value) {
  return ShaderTypeIterator((ShaderType)0);
}

/// Enumerates types of shader objects like textures or buffers.
enum ShaderObjectKind {
  /// Output buffer that appears as a stream the shader may append to.
  SOK_AppendStructuredBuffer,

  ///
  SOK_Buffer,

  SOK_ByteAddressBuffer,

  /// An input buffer that appears as a stream the shader may pull values from.
  SOK_ConsumeStructuredBuffer,

  /// Represents an array of control points that are available to the hull
  /// shader as inputs.
  SOK_InputPatch,

  /// Represents an array of output control points that are available to the
  /// hull shader's patch-constant function as well as the domain shader.
  SOK_OutputPatch,

  /// A read/write buffer.
  SOK_RWBuffer,

  /// A read/write buffer that indexes in bytes.
  SOK_RWByteAddressBuffer,

  /// A read/write buffer that can take a T type that is a structure.
  SOK_RWStructuredBuffer,

  /// A read/write resource.
  SOK_RWTexture1D,

  /// A read/write resource.
  SOK_RWTexture1DArray,

  /// A read/write resource.
  SOK_RWTexture2D,

  /// A read/write resource.
  SOK_RWTexture2DArray,

  /// A read/write resource.
  SOK_RWTexture3D,

  /// A stream-output object is a templated object that streams data out of the
  /// geometry-shader stage.
  SOK_StreamOutputLine,

  /// A stream-output object is a templated object that streams data out of the
  /// geometry-shader stage.
  SOK_StreamOutputPoint,

  /// A stream-output object is a templated object that streams data out of the
  /// geometry-shader stage.
  SOK_StreamOutputTriangle,

  /// A read-only buffer, which can take a T type that is a structure.
  SOK_StructuredBuffer,

  /// A read-only resource.
  SOK_Texture1D,

  /// A read-only resource.
  SOK_Texture1DArray,

  /// A read-only resource.
  SOK_Texture2D,

  /// A read-only resource.
  SOK_Texture2DArray,

  /// A read-only resource.
  SOK_Texture2DMS,

  /// A read-only resource.
  SOK_Texture2DArrayMS,

  /// A read-only resource.
  SOK_Texture3D,

  /// A read-only resource.
  SOK_TextureCube,

  /// A read-only resource.
  SOK_TextureCubeArray
};

struct ShaderObjectDataItem {
  ShaderObjectKind Kind;
  const char *TypeName;
  ShaderModel MinShaderModel;
  ShaderType ValidShaderTypes;
};

static const ShaderObjectDataItem ShaderObjectData[] = {
    {SOK_AppendStructuredBuffer, "AppendStructuredBuffer", ShaderModel5,
     ST_PixelCompute},
    {SOK_Buffer, "Buffer", ShaderModel5, ST_All},
    {SOK_ByteAddressBuffer, "ByteAddressBuffer", ShaderModel5, ST_All},
    {SOK_ConsumeStructuredBuffer, "ConsumeStructuredBuffer", ShaderModel5,
     ST_PixelCompute},
    {SOK_InputPatch, "InputPatch", ShaderModel5, ST_HullGeometry},
    {SOK_OutputPatch, "OutputPatch", ShaderModel5, ST_HullDomain},
    {SOK_RWBuffer, "RWBuffer", ShaderModel5, ST_PixelCompute},
    {SOK_RWByteAddressBuffer, "RWByteAddressBuffer", ShaderModel5,
     ST_PixelCompute},
    {SOK_RWStructuredBuffer, "RWStructuredBuffer", ShaderModel5,
     ST_PixelCompute},
    {SOK_RWTexture1D, "RWTexture1D", ShaderModel5, ST_PixelCompute},
    {SOK_RWTexture1DArray, "RWTexture1DArray", ShaderModel5, ST_PixelCompute},
    {SOK_RWTexture2D, "RWTexture2D", ShaderModel5, ST_PixelCompute},
    {SOK_RWTexture2DArray, "RWTexture2DArray", ShaderModel5, ST_PixelCompute},
    {SOK_RWTexture3D, "RWTexture3D", ShaderModel5, ST_PixelCompute},
    {SOK_StreamOutputLine, "LineStream", ShaderModel4, ST_Geometry},
    {SOK_StreamOutputPoint, "PointStream", ShaderModel4, ST_Geometry},
    {SOK_StreamOutputTriangle, "TriangleStream", ShaderModel4, ST_Geometry},
    {SOK_StructuredBuffer, "StructuredBuffer", ShaderModel5, ST_All},
    {SOK_Texture1D, "Texture1D", ShaderModel4, ST_All},
    {SOK_Texture1DArray, "Texture1DArray", ShaderModel4, ST_All},
    {SOK_Texture2D, "Texture2D", ShaderModel4, ST_All},
    {SOK_Texture2DArray, "Texture2DArray", ShaderModel4, ST_All},
    {SOK_Texture2DMS, "Texture2DMS", ShaderModel4, ST_All},
    {SOK_Texture2DArrayMS, "Texture2DMSArray", ShaderModel4, ST_All},
    {SOK_Texture3D, "Texture3D", ShaderModel4, ST_All},
    {SOK_TextureCube, "TextureCube", ShaderModel4, ST_All},
    {SOK_TextureCubeArray, "TextureCubeArray", ShaderModel4, ST_All}};

/// Enumerates the template shapes.
/// This is a crude simplification of what the type system could do,
/// but it covers all cases without unused generality.
enum ShaderObjectTemplateKind {
  /// No parameters.
  SOTK_NoParams,

  /// Single parameter of type scalar or vector.
  SOTK_SingleSVParam,

  /// Single parameter of type scalar, vector or struct.
  SOTK_SingleSVCParam,

  /// Two parameters; a scalar or vector type and a sample count.
  SOTK_SVAndSampleCountParams,

  /// Two parameters; a scalar or vector type and a control point count (1-32).
  SOTK_SVCAndControlPointCountParams
};

struct ShaderObjectTemplateDataItem {
  ShaderObjectKind Kind;
  ShaderObjectTemplateKind TemplateKind;
  bool TemplateParamsOptional;
};

static const bool OptionalTrue = true;
static const bool OptionalFalse = false;

static const ShaderObjectTemplateDataItem ShaderObjectTemplateData[] = {
    {SOK_AppendStructuredBuffer, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_Buffer, SOTK_SingleSVParam, OptionalFalse},
    {SOK_ByteAddressBuffer, SOTK_NoParams, OptionalTrue},
    {SOK_ConsumeStructuredBuffer, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_InputPatch, SOTK_SVCAndControlPointCountParams, OptionalFalse},
    {SOK_OutputPatch, SOTK_SVCAndControlPointCountParams, OptionalFalse},
    {SOK_RWBuffer, SOTK_SingleSVParam, OptionalFalse},
    {SOK_RWByteAddressBuffer, SOTK_NoParams, OptionalTrue},
    {SOK_RWStructuredBuffer, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_RWTexture1D, SOTK_SingleSVParam, OptionalFalse},
    {SOK_RWTexture1DArray, SOTK_SingleSVParam, OptionalFalse},
    {SOK_RWTexture2D, SOTK_SingleSVParam, OptionalFalse},
    {SOK_RWTexture2DArray, SOTK_SingleSVParam, OptionalFalse},
    {SOK_RWTexture3D, SOTK_SingleSVParam, OptionalFalse},
    {SOK_StreamOutputLine, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_StreamOutputPoint, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_StreamOutputTriangle, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_StructuredBuffer, SOTK_SingleSVCParam, OptionalFalse},
    {SOK_Texture1D, SOTK_SingleSVParam, OptionalTrue},
    {SOK_Texture1DArray, SOTK_SingleSVParam, OptionalTrue},
    {SOK_Texture2D, SOTK_SingleSVParam, OptionalTrue},
    {SOK_Texture2DArray, SOTK_SingleSVParam, OptionalTrue},
    {SOK_Texture2DMS, SOTK_SVAndSampleCountParams, OptionalFalse},
    {SOK_Texture2DArrayMS, SOTK_SVAndSampleCountParams, OptionalFalse},
    {SOK_Texture3D, SOTK_SingleSVParam, OptionalTrue},
    {SOK_TextureCube, SOTK_SingleSVParam, OptionalTrue},
    {SOK_TextureCubeArray, SOTK_SingleSVParam, OptionalTrue}};

static const ShaderObjectTemplateDataItem &
GetTemplateData(const ShaderObjectDataItem &sod) {
  static_assert(_countof(ShaderObjectTemplateData) ==
                    _countof(ShaderObjectData),
                "otherwise lookup tables have different elements");
  struct Unary {
    ShaderObjectKind ObjectKind;
    Unary(ShaderObjectKind ok) : ObjectKind(ok) {}
    bool operator()(const ShaderObjectTemplateDataItem &i) {
      return i.Kind == ObjectKind;
    }
  };
  Unary filter(sod.Kind);
  auto iter = std::find_if(std::begin(ShaderObjectTemplateData),
                           std::end(ShaderObjectTemplateData), filter);
  assert(iter != std::end(ShaderObjectTemplateData));
  return *iter;
}

static int CountOptionalTemplateArguments(
    const ShaderObjectTemplateDataItem &templateData) {
  if (!templateData.TemplateParamsOptional) {
    return 0;
  }

  switch (templateData.TemplateKind) {
  default:
  case SOTK_NoParams:
    return 0;
  case SOTK_SingleSVParam:
    return 1;
  case SOTK_SingleSVCParam:
    return 1;
  case SOTK_SVAndSampleCountParams:
    return 2;
  case SOTK_SVCAndControlPointCountParams:
    return 2;
  }
}

// - a RWBuffer supports globallycoherent storage class to generate memory
// barriers
// TODO: ByteAddressBuffer is supported on SM4 on compute shaders
// TODO: RWByteAddressBuffer is supported on SM4 on compute shaders
// TODO: RWStructuredBuffer is supported on SM4 on compute shaders

#include "dxc/HlslIntrinsicOp.h"
#include "dxc/dxcapi.internal.h"
#include "gen_intrin_main_tables_15.h"

struct ShaderObjectIntrinsicDataItem {
  // Kind of shader object described.
  ShaderObjectKind Kind;

  // Pointer to first intrinsic in table.
  const HLSL_INTRINSIC *Intrinsics;

  // Count of elements in intrinsic table.
  size_t IntrinsicCount;
};

// The test that requires this is pending complete
// support for primitive types
#if 0
const ShaderObjectIntrinsicDataItem ShaderObjectIntrinsicData[] = {
  { SOK_AppendStructuredBuffer, g_AppendStructuredBufferMethods, _countof(g_AppendStructuredBufferMethods) },
  { SOK_Buffer, g_BufferMethods, _countof(g_BufferMethods) },
  { SOK_ByteAddressBuffer, g_ByteAddressBufferMethods, _countof(g_ByteAddressBufferMethods) },
  { SOK_ConsumeStructuredBuffer, g_ConsumeStructuredBufferMethods, _countof(g_ByteAddressBufferMethods) },
  { SOK_InputPatch, nullptr, 0 },
  { SOK_OutputPatch, nullptr, 0 },
  { SOK_RWBuffer, g_RWBufferMethods, _countof(g_RWBufferMethods) },
  { SOK_RWByteAddressBuffer, g_RWByteAddressBufferMethods, _countof(g_RWByteAddressBufferMethods) },
  { SOK_RWStructuredBuffer, g_RWStructuredBufferMethods, _countof(g_RWStructuredBufferMethods) },
  { SOK_RWTexture1D, g_RWTexture1DMethods, _countof(g_RWTexture1DMethods) },
  { SOK_RWTexture1DArray, g_RWTexture1DArrayMethods, _countof(g_RWTexture1DArrayMethods) },
  { SOK_RWTexture2D, g_RWTexture2DMethods, _countof(g_RWTexture2DMethods) },
  { SOK_RWTexture2DArray, g_RWTexture2DArrayMethods, _countof(g_RWTexture2DArrayMethods) },
  { SOK_RWTexture3D, g_RWTexture3DMethods, _countof(g_RWTexture3DMethods) },
  { SOK_StreamOutputLine, g_StreamMethods, _countof(g_StreamMethods) },
  { SOK_StreamOutputPoint, g_StreamMethods, _countof(g_StreamMethods) },
  { SOK_StreamOutputTriangle, g_StreamMethods, _countof(g_StreamMethods) },
  { SOK_StructuredBuffer, g_StructuredBufferMethods, _countof(g_StructuredBufferMethods) },
  { SOK_Texture1D, g_Texture1DMethods, _countof(g_Texture1DMethods) },
  { SOK_Texture1DArray, g_Texture1DArrayMethods, _countof(g_Texture1DArrayMethods) },
  { SOK_Texture2D, g_Texture2DMethods, _countof(g_Texture2DMethods) },
  { SOK_Texture2DArray, g_Texture2DArrayMethods, _countof(g_Texture2DArrayMethods) },
  { SOK_Texture2DMS, g_Texture2DMSMethods, _countof(g_Texture2DMSMethods) },
  { SOK_Texture2DArrayMS, g_Texture2DArrayMSMethods, _countof(g_Texture2DArrayMSMethods) },
  { SOK_Texture3D, g_Texture3DMethods, _countof(g_Texture3DMethods) },
  { SOK_TextureCube, g_TextureCUBEMethods, _countof(g_TextureCUBEMethods) },
  { SOK_TextureCubeArray, g_TextureCUBEArrayMethods, _countof(g_TextureCUBEArrayMethods) }
};

static
const ShaderObjectIntrinsicDataItem& GetIntrinsicData(const ShaderObjectDataItem& sod)
{
  for (unsigned i = 0; i < _countof(ShaderObjectIntrinsicData); i++)
  {
    if (sod.Kind == ShaderObjectIntrinsicData[i].Kind)
    {
      return ShaderObjectIntrinsicData[i];
    }
  }

  throw std::runtime_error("cannot find shader object kind");
}
#endif

// The test fixture.
#ifdef _WIN32
class ObjectTest {
#else
class ObjectTest : public ::testing::Test {
#endif
private:
  HlslIntellisenseSupport m_isenseSupport;

public:
  BEGIN_TEST_CLASS(ObjectTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(ObjectTestSetup);

  TEST_METHOD(DeclareLocalObject)
  TEST_METHOD(OptionalTemplateArgs)
  TEST_METHOD(MissingTemplateArgs)
  TEST_METHOD(TooManyTemplateArgs)
  TEST_METHOD(PassAsParameter)
  TEST_METHOD(AssignVariables)
  TEST_METHOD(AssignReturnResult)
  TEST_METHOD(PassToInoutArgs)
  TEST_METHOD(TemplateArgConstraints)
  TEST_METHOD(FunctionInvoke)

  void FormatTypeNameAndPreamble(const ShaderObjectDataItem &sod,
                                 char (&typeName)[64],
                                 const char **preambleDecl) {
    *preambleDecl = "";

    auto templateData = GetTemplateData(sod);
    switch (templateData.TemplateKind) {
    case SOTK_NoParams:
      sprintf_s(typeName, _countof(typeName), "%s", sod.TypeName);
      break;
    case SOTK_SingleSVParam:
      sprintf_s(typeName, _countof(typeName), "%s<float4>", sod.TypeName);
      break;
    case SOTK_SingleSVCParam:
      *preambleDecl = "struct MY_STRUCT { float4 f4; bool b; int3 i3; };\n";
      sprintf_s(typeName, _countof(typeName), "%s<MY_STRUCT>", sod.TypeName);
      break;
    case SOTK_SVAndSampleCountParams:
    case SOTK_SVCAndControlPointCountParams:
    default:
      sprintf_s(typeName, _countof(typeName), "%s<float4, 1>", sod.TypeName);
      break;
    }
  }

  std::string BuildDeclarationFunction(const ShaderObjectDataItem &sod) {
    return BuildDeclarationFunction(sod, 0, false);
  }

  std::string BuildDeclarationFunction(const ShaderObjectDataItem &sod,
                                       int missingTemplateCount,
                                       bool collapseEmptyArgs) {
    char result[256];

    auto templateData = GetTemplateData(sod);
    const char StructDecl[] =
        "struct MY_STRUCT { float4 f4; bool b; int3 i3; };\n";
    const char StructParam[] = "<MY_STRUCT>\n";
    const char VectorParam[] = "<float4>\n";
    const char VectorAndCountParam[] = "<float4, 4>\n";
    const char EmptyTemplateArgs[] = "<>";
    const char MissingTemplateArgs[] = "";

    // Default setup for a 'SOTK_NoParams' case.
    const char *StructDeclFragment = "";
    const char *TemplateDeclFragment = MissingTemplateArgs;

    if (templateData.TemplateKind == SOTK_SingleSVParam &&
        missingTemplateCount == 0) {
      TemplateDeclFragment = VectorParam;
    } else if (templateData.TemplateKind == SOTK_SingleSVCParam &&
               missingTemplateCount == 0) {
      StructDeclFragment = StructDecl;
      TemplateDeclFragment = StructParam;
    } else if ((templateData.TemplateKind == SOTK_SVAndSampleCountParams ||
                templateData.TemplateKind ==
                    SOTK_SVCAndControlPointCountParams)) {
      if (missingTemplateCount == 0) {
        TemplateDeclFragment = VectorAndCountParam;
      } else if (missingTemplateCount == 1) {
        TemplateDeclFragment = VectorParam;
      }
    }

    // Allow 'Object<>' to collapse to 'Object' when collapseEmptyArgs is set.
    if (templateData.TemplateKind != SOTK_NoParams &&
        TemplateDeclFragment == MissingTemplateArgs) {
      TemplateDeclFragment =
          collapseEmptyArgs ? MissingTemplateArgs : EmptyTemplateArgs;
    }

    sprintf_s(result, _countof(result),
              "%s"
              "float ps(float4 color : COLOR) { %s%s localVar; return 0; }",
              StructDeclFragment, sod.TypeName, TemplateDeclFragment);

    return std::string(result);
  }

  std::string
  BuildDeclarationFunctionTooManyArgs(const ShaderObjectDataItem &sod) {
    char result[256];

    auto templateData = GetTemplateData(sod);
    switch (templateData.TemplateKind) {
    case SOTK_NoParams:
      sprintf_s(
          result, _countof(result),
          "float ps(float4 color : COLOR) { %s localVar<float>; return 0; }",
          sod.TypeName);
      break;
    case SOTK_SingleSVParam:
      sprintf_s(result, _countof(result),
                "float ps(float4 color : COLOR) { %s<float4, 1> localVar; "
                "return 0; }",
                sod.TypeName);
      break;
    case SOTK_SingleSVCParam:
      sprintf_s(result, _countof(result),
                "struct MY_STRUCT { float4 f4; bool b; int3 i3; };\n"
                "float ps(float4 color : COLOR) { %s<MY_STRUCT, 1> localVar; "
                "return 0; }",
                sod.TypeName);
      break;
    case SOTK_SVAndSampleCountParams:
    case SOTK_SVCAndControlPointCountParams:
    default:
      sprintf_s(result, _countof(result),
                "float ps(float4 color : COLOR) { %s<float4, 4, 4> localVar; "
                "return 0; }",
                sod.TypeName);
      break;
    }

    return std::string(result);
  }

  std::string BuildPassAsParameter(const ShaderObjectDataItem &sod) {
    char result[256];
    char typeName[64];
    const char *preambleDecl;

    FormatTypeNameAndPreamble(sod, typeName, &preambleDecl);

    std::string parmType = typeName;
    // Stream-output objects must be declared as inout.
    switch (sod.Kind) {
    case SOK_StreamOutputLine:
    case SOK_StreamOutputPoint:
    case SOK_StreamOutputTriangle:
      parmType = "inout " + parmType;
      break;
    default:
      // Other kinds need no alteration
      break;
    }

    sprintf_s(result, _countof(result),
              "%s"
              "void f(%s parameter) { }\n"
              "float ps(float4 color : COLOR) { %s localVar; f(localVar); "
              "return 0; }",
              preambleDecl, parmType.c_str(), typeName);

    return std::string(result);
  }

  std::string BuildAssignment(const ShaderObjectDataItem &sod) {
    char result[256];
    char typeName[64];
    const char *preambleDecl;

    FormatTypeNameAndPreamble(sod, typeName, &preambleDecl);

    sprintf_s(result, _countof(result),
              "%s"
              "float ps(float4 color : COLOR) { %s lv1; %s lv2; lv1 = lv2; "
              "return 0; }",
              preambleDecl, typeName, typeName);

    return std::string(result);
  }

  std::string BuildAssignmentFromResult(const ShaderObjectDataItem &sod) {
    char result[256];
    char typeName[64];
    const char *preambleDecl;

    FormatTypeNameAndPreamble(sod, typeName, &preambleDecl);

    sprintf_s(result, _countof(result),
              "%s"
              "%s f() { %s local; return local; }\n"
              "float ps(float4 color : COLOR) { %s lv1 = f(); return 0; }",
              preambleDecl, typeName, typeName, typeName);

    return std::string(result);
  }

  void CheckCompiles(const std::string &text, bool expected) {
    CheckCompiles(text.c_str(), text.size(), expected);
  }

  void CheckCompiles(const char *text, size_t textLen, bool expected) {
    CompilationResult result(
        CompilationResult::CreateForProgram(text, textLen));

    // Uncomment the line to print out the AST unconditionally.
    // printf("%s", result.BuildASTString().c_str());

    if (expected != result.ParseSucceeded()) {
      EXPECT_EQ(expected, result.ParseSucceeded());
      // TODO: log this out
      //<< "for program " << text << "\n with AST\n" << result.BuildASTString()
      //<< "and errors\n" << result.GetTextForErrors();
    }
  }
};

bool ObjectTest::ObjectTestSetup() {
  m_isenseSupport.Initialize();
  return m_isenseSupport.IsEnabled();
}

TEST_F(ObjectTest, DeclareLocalObject) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);
    CheckCompiles(BuildDeclarationFunction(sod), true);
  }
}

TEST_F(ObjectTest, OptionalTemplateArgs) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    const ShaderObjectTemplateDataItem &templateData = GetTemplateData(sod);
    int argCount = CountOptionalTemplateArguments(templateData);
    if (argCount == 0) {
      continue;
    }

    for (int missingCount = 1; missingCount <= argCount; missingCount++) {
      CheckCompiles(BuildDeclarationFunction(sod, missingCount, false), true);
    }
  }
}

TEST_F(ObjectTest, MissingTemplateArgs) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    const ShaderObjectTemplateDataItem &templateData = GetTemplateData(sod);
    int argCount = CountOptionalTemplateArguments(templateData);
    if (argCount == 0) {
      continue;
    }

    CheckCompiles(BuildDeclarationFunction(sod, argCount, true), true);
  }
}

TEST_F(ObjectTest, TooManyTemplateArgs) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    CheckCompiles(BuildDeclarationFunctionTooManyArgs(sod), false);
  }
}

TEST_F(ObjectTest, PassAsParameter) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    CheckCompiles(BuildPassAsParameter(sod), true);
  }
}

TEST_F(ObjectTest, AssignVariables) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    CheckCompiles(BuildAssignment(sod), true);
  }
}

TEST_F(ObjectTest, AssignReturnResult) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    CheckCompiles(BuildAssignmentFromResult(sod), true);
  }
}

TEST_F(ObjectTest, PassToInoutArgs) {
  for (const auto &sod : ShaderObjectData) {
    // Speed up the test by building one large program with all inout parameter
    // modifiers per object.
    std::stringstream programText;
    unsigned uniqueId = 0;
    for (const auto &iop : InOutParameterModifierData) {

      switch (sod.Kind) {
      case SOK_StreamOutputLine:
      case SOK_StreamOutputPoint:
      case SOK_StreamOutputTriangle:
        // Stream-output objects can only be inout. Skip other cases.
        if (std::string(iop.Keyword) != "inout")
          continue;
        break;
      default:
        // other cases can be what they want
        break;
      }

      char typeName[64];
      const char *preambleDecl;

      // When shader models are validated, run through all of them.
      // for (const auto &st : sod.ValidShaderTypes)
      // const auto st = std::first(sod.ValidShaderTypes);

      FormatTypeNameAndPreamble(sod, typeName, &preambleDecl);
      if (uniqueId == 0) { // do this only once
        programText << preambleDecl << std::endl;
      }

      programText << "float ps_" << uniqueId << "(" << iop.Keyword << " "
                  << typeName << " o) { return 1.0f; }" << std::endl;
      programText << "void caller_" << uniqueId << "() { " << typeName
                  << " lv; ps_" << uniqueId << "(lv); }" << std::endl;
      programText << std::endl;
      uniqueId++;
    }

    std::string programTextStr(programText.str());
    CheckCompiles(programTextStr.c_str(), programTextStr.size(), true);
  }
}

class TemplateSampleDataItem {
public:
  TemplateSampleDataItem() {}
  TemplateSampleDataItem(const TemplateSampleDataItem &other)
      : Preamble(other.Preamble), TypeName(other.TypeName),
        IsValid(other.IsValid) {}

  TemplateSampleDataItem(const char *preamble, const char *typeName,
                         bool isValid)
      : Preamble(preamble), TypeName(typeName), IsValid(isValid) {}

  std::string Preamble;
  std::string TypeName;
  bool IsValid;
};

std::vector<TemplateSampleDataItem>
CreateSampleDataForTemplateArg(const ShaderObjectDataItem &sod,
                               int templateIndex) {
  std::vector<TemplateSampleDataItem> result;
  char typeName[64];

  auto templateData = GetTemplateData(sod);
  assert(templateData.TemplateKind != SOTK_NoParams &&
         "shouldn't call CreateSampleDataForTemplateArg");
  switch (templateData.TemplateKind) {
  case SOTK_NoParams:
    assert(!"shouldn't call CreateSampleDataForTemplateArg");
    break;
  case SOTK_SingleSVParam:
  case SOTK_SingleSVCParam:
    sprintf_s(typeName, _countof(typeName), "%s<float4>", sod.TypeName);
    result.push_back(TemplateSampleDataItem("", typeName, true));

    sprintf_s(typeName, _countof(typeName), "%s<SamplerState>", sod.TypeName);
    result.push_back(TemplateSampleDataItem("", typeName, false));
    break;
  case SOTK_SVAndSampleCountParams:
  case SOTK_SVCAndControlPointCountParams:
    if (templateIndex == 0) {
      sprintf_s(typeName, _countof(typeName), "%s<float4, 4>", sod.TypeName);
      result.push_back(TemplateSampleDataItem("", typeName, true));

      sprintf_s(typeName, _countof(typeName), "%s<SamplerState, 4>",
                sod.TypeName);
      result.push_back(TemplateSampleDataItem("", typeName, false));
    } else {
      sprintf_s(typeName, _countof(typeName), "%s<float4, 1>", sod.TypeName);
      result.push_back(TemplateSampleDataItem("", typeName, true));

      sprintf_s(typeName, _countof(typeName), "%s<float4, 128>", sod.TypeName);
      result.push_back(TemplateSampleDataItem("", typeName, true));

      // These are deferred to back-end validation for now.
      // bool largeNumberValid =  sod.Kind == SOK_InputPatch || sod.Kind ==
      // SOK_OutputPatch;
      sprintf_s(typeName, _countof(typeName), "%s<float4, 129>", sod.TypeName);
      result.push_back(TemplateSampleDataItem("", typeName, false));
    }

    break;
  }

  return result;
}

TEST_F(ObjectTest, TemplateArgConstraints) {
  for (const auto &sod : ShaderObjectData) {
    // When shader models are validated, run through all of them.
    // for (const auto &st : sod.ValidShaderTypes)
    // const auto st = std::first(sod.ValidShaderTypes);

    const ShaderObjectTemplateDataItem &templateData = GetTemplateData(sod);
    int argCount = CountOptionalTemplateArguments(templateData);
    if (argCount == 0) {
      continue;
    }

    for (int i = 0; i < argCount; i++) {
      std::vector<TemplateSampleDataItem> sampleData =
          CreateSampleDataForTemplateArg(sod, i);
      for (auto sampleDataItem : sampleData) {
        char result[256];
        sprintf_s(result, _countof(result),
                  "%s"
                  "float ps(float4 color : COLOR) { %s lv1; %s lv2; lv1 = lv2; "
                  "return 0; }",
                  sampleDataItem.Preamble.c_str(),
                  sampleDataItem.TypeName.c_str(),
                  sampleDataItem.TypeName.c_str());

        CheckCompiles(result, strlen(result), sampleDataItem.IsValid);
      }
    }
  }
}

// The test that requires this function is pending complete
// support for primitive types
#if 0
static
std::string SelectComponentType(BYTE legalTypes)
{
  switch ((LEGAL_INTRINSIC_COMPTYPES)legalTypes)
  {
  case LICOMPTYPE_VOID: return "void";
  case LICOMPTYPE_BOOL: return "bool";
  case LICOMPTYPE_INT: return "int";
  case LICOMPTYPE_UINT: return "uint";
  case LICOMPTYPE_ANY_INT: return "int";
  case LICOMPTYPE_ANY_INT32: return "int";
  case LICOMPTYPE_UINT_ONLY: return "uint";
  case LICOMPTYPE_FLOAT: return "float";
  case LICOMPTYPE_ANY_FLOAT: return "float";
  case LICOMPTYPE_FLOAT_LIKE: return "float";
  case LICOMPTYPE_FLOAT_DOUBLE: return "double";
  case LICOMPTYPE_DOUBLE: return "double";
  case LICOMPTYPE_DOUBLE_ONLY: return "double";
  case LICOMPTYPE_NUMERIC: return "int";
  case LICOMPTYPE_NUMERIC32: return "float";
  case LICOMPTYPE_NUMERIC32_ONLY: return "double";
  case LICOMPTYPE_ANY: return "int";
  default: return "";
      //LICOMPTYPE_SAMPLER1D,
      //LICOMPTYPE_SAMPLER2D,
      //LICOMPTYPE_SAMPLER3D,
      //LICOMPTYPE_SAMPLERCUBE,
      //LICOMPTYPE_SAMPLERCMP,
      //LICOMPTYPE_SAMPLER,
      //LICOMPTYPE_STRING,
  }
}
#endif

TEST_F(ObjectTest, FunctionInvoke) {
  // This is pending complete support for primitive types - there are many
  // instances of uint usage, which isn't currently supported.
#if 0
  // Tests for too many or too few arguments are available as lit-based tests.
  // Invoke each method, assigning the result of each method invocation as necessary.
  uint64_t iteration = 0;
  for (const auto &sod : ShaderObjectData) {
    const ShaderObjectIntrinsicDataItem& intrinsicData = GetIntrinsicData(sod);
    const ShaderObjectTemplateDataItem& templateData = GetTemplateData(sod);

    for (size_t i = 0; i < intrinsicData.IntrinsicCount; i++) {
      const HLSL_INTRINSIC* intrinsic = &intrinsicData.Intrinsics[i];
      ++iteration;
      // Build a program that will call this intrinsic.

      // Select an element type for the class if needed.
      std::string objectType;
      std::string elementType;
      std::string elementParameters;
      switch (templateData.TemplateKind)
      {
      case ShaderObjectTemplateKind::SOTK_NoParams:
        break;
      case ShaderObjectTemplateKind::SOTK_SingleSVCParam:
      case ShaderObjectTemplateKind::SOTK_SingleSVParam:
        elementParameters = "<float3>";
        elementType = "float3";
        break;
      case ShaderObjectTemplateKind::SOTK_SVAndSampleCountParams:
      case ShaderObjectTemplateKind::SOTK_SVCAndControlPointCountParams:
        elementParameters = "<float3, 4>";
        elementType = "float3";
        break;
      }

      objectType = sod.TypeName + elementParameters;

      // Select types for all arguments.
      // This implementation is brain-dead but (a) different from production,
      // so it's a sensible oracle, and (b) very straightforward. We simply
      // keep instantiating argument types until we finished.
      std::string argumentTypes[g_MaxIntrinsicParamCount];
      {
        bool madeProgress = false;
        int argsRemaining = intrinsic->uNumArgs - 1;
        do
        {
          madeProgress = false;
          for (int i = 1; i < intrinsic->uNumArgs; i++)
          {
            if (!argumentTypes[i - 1].empty())
            {
              continue;
            }

            // Determine whether we can make an independent selection.
            if (intrinsic->pArgs[i].uTemplateId == INTRIN_TEMPLATE_FROM_TYPE)
            {
              argumentTypes[i - 1] = elementType;
              madeProgress = true;
              --argsRemaining;
              continue;
            }

            // An independent scalar.
            if (intrinsic->pArgs[i].uTemplateId == i && intrinsic->pArgs[i].uComponentTypeId == i &&
              intrinsic->pArgs[i].uLegalComponentTypes != LITEMPLATE_ANY &&
              intrinsic->pArgs[i].uLegalTemplates == LITEMPLATE_SCALAR)
            {
              argumentTypes[i - 1] = SelectComponentType(intrinsic->pArgs[i].uLegalComponentTypes);
              madeProgress = true;
              --argsRemaining;
              continue;
            }

            // An independent vector.
            if (intrinsic->pArgs[i].uTemplateId == i && intrinsic->pArgs[i].uComponentTypeId == i &&
              intrinsic->pArgs[i].uLegalComponentTypes != LITEMPLATE_ANY &&
              intrinsic->pArgs[i].uLegalTemplates == LITEMPLATE_VECTOR)
            {
              argumentTypes[i - 1] = SelectComponentType(intrinsic->pArgs[i].uLegalComponentTypes) + "2";
              madeProgress = true;
              --argsRemaining;
              continue;
            }

            // TODO: complete the assignment cases with primitive types
            // Determine whether we have all dependencies assigned for a selection.
          }
        } while (madeProgress && argsRemaining > 0);
        EXPECT_EQ(0, argsRemaining) <<
          "otherwise we're unable to complete the signature for " << intrinsic->pArgs[0].pName <<
          " in iteration " << iteration;
      }

      // Determine what the result type is.
      bool resultIsVoid = intrinsic->pArgs[0].uLegalTemplates == LITEMPLATE_VOID;
      std::string resultType;
      if (!resultIsVoid) {
      }

      // Assemble the program as a series of declarations, a method call
      // and possibly an assignment if there's a return value.
      std::stringstream program;
      std::string assignmentLeftHand(resultIsVoid ? "" : (resultType + " result = "));
      program << "// iteration " << iteration << "\n"
        "void f() {\n"
        "  " << objectType << " testObject;\n";
      for (int i = 1; i < intrinsic->uNumArgs; i++) {
        program << argumentTypes[i - 1] << ' ' << intrinsic->pArgs[i].pName << ";\n";
      }
      program << assignmentLeftHand << "testObject." << intrinsic->pArgs[0].pName << "(";
      for (int i = 1; i < intrinsic->uNumArgs; i++) {
        program << intrinsic->pArgs[i].pName;
        if (i != intrinsic->uNumArgs - 1) {
          program << ", ";
        }
      }
      program << ");\n}";

      // Verify it runs.
      CheckCompiles(program.str(), true); 
    }
  }
#endif
}

// TODO: force type promotion to occur for arguments.
