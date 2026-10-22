///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// ExtensionTest.cpp                                                         //
//                                                                           //
// Provides tests for the language extension APIs.                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilOperations.h"
#include "dxc/HLSL/HLOperationLowerExtension.h"
#include "dxc/HlslIntrinsicOp.h"
#include "dxc/Support/microcom.h"
#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"
#include "dxc/dxcapi.internal.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Regex.h"

///////////////////////////////////////////////////////////////////////////////
// Support for test intrinsics.

// $result = test_fn(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnArgs[] = {
    {"test_fn", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

// void test_proc(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestProcArgs[] = {
    {"test_proc", 0, 0, LITEMPLATE_VOID, 0, LICOMPTYPE_VOID, 0, 0},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

// $result = test_poly(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnCustomArgs[] = {
    {"test_poly", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
     IA_C},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

// $result = test_int(int<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnIntArgs[] = {
    {"test_int", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_INT, 1, IA_C},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_INT, 1, IA_C}};

// $result = test_nolower(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnNoLowerArgs[] = {
    {"test_nolower", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
     IA_C},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

// void test_pack_0(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnPack0[] = {
    {"test_pack_0", 0, 0, LITEMPLATE_VOID, 0, LICOMPTYPE_VOID, 0, 0},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

// $result = test_pack_1()
static const HLSL_INTRINSIC_ARGUMENT TestFnPack1[] = {
    {"test_pack_1", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_FLOAT, 1,
     2},
};

// $result = test_pack_2(any_vector<any_cardinality> value1,
// any_vector<any_cardinality> value2)
static const HLSL_INTRINSIC_ARGUMENT TestFnPack2[] = {
    {"test_pack_2", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
     IA_C},
    {"value1", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C},
    {"value2", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C},
};

// $scalar = test_pack_3(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnPack3[] = {
    {"test_pack_3", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_FLOAT, 1,
     1},
    {"value1", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_FLOAT, 1, 2},
};

// float<2> = test_pack_4(float<3> value)
static const HLSL_INTRINSIC_ARGUMENT TestFnPack4[] = {
    {"test_pack_4", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_FLOAT, 1,
     2},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_FLOAT, 1, 3},
};

// float<2> = test_rand()
static const HLSL_INTRINSIC_ARGUMENT TestRand[] = {
    {"test_rand", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_FLOAT, 1, 2},
};

// uint = test_rand(uint x)
static const HLSL_INTRINSIC_ARGUMENT TestUnsigned[] = {
    {"test_unsigned", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_UINT, 1,
     1},
    {"x", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 1},
};

// float2 = MyBufferOp(uint2 addr)
static const HLSL_INTRINSIC_ARGUMENT TestMyBufferOp[] = {
    {"MyBufferOp", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_FLOAT, 1,
     2},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
};

// float2 = CustomLoadOp(uint2 addr)
static const HLSL_INTRINSIC_ARGUMENT TestCustomLoadOp[] = {
    {"CustomLoadOp", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_FLOAT, 1,
     2},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
};

// float2 = CustomLoadOp(uint2 addr, bool val)
static const HLSL_INTRINSIC_ARGUMENT TestCustomLoadOpBool[] = {
    {"CustomLoadOp", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_FLOAT, 1,
     2},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
    {"val", AR_QUAL_IN, 2, LITEMPLATE_SCALAR, 2, LICOMPTYPE_BOOL, 1, 1},
};

// float2 = CustomLoadOp(uint2 addr, bool val, uint2 subscript)
static const HLSL_INTRINSIC_ARGUMENT TestCustomLoadOpSubscript[] = {
    {"CustomLoadOp", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_FLOAT, 1,
     2},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
    {"val", AR_QUAL_IN, 2, LITEMPLATE_SCALAR, 2, LICOMPTYPE_BOOL, 1, 1},
    {"subscript", AR_QUAL_IN, 3, LITEMPLATE_VECTOR, 3, LICOMPTYPE_UINT, 1, 2},
};

// bool<> = test_isinf(float<> x)
static const HLSL_INTRINSIC_ARGUMENT TestIsInf[] = {
    {"test_isinf", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_BOOL, 1,
     IA_C},
    {"x", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_FLOAT, 1, IA_C},
};

// int = test_ibfe(uint width, uint offset, uint val)
static const HLSL_INTRINSIC_ARGUMENT TestIBFE[] = {
    {"test_ibfe", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_INT, 1, 1},
    {"width", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
    {"offset", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
    {"val", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
};

// float2 = MySamplerOp(uint2 addr)
static const HLSL_INTRINSIC_ARGUMENT TestMySamplerOp[] = {
    {"MySamplerOp", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_FLOAT, 1,
     2},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
};

// $result = wave_proc(any_vector<any_cardinality> value)
static const HLSL_INTRINSIC_ARGUMENT WaveProcArgs[] = {
    {"wave_proc", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
     IA_C},
    {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

// uint = Texutre1D.MyTextureOp(uint addr, uint offset)
static const HLSL_INTRINSIC_ARGUMENT TestMyTexture1DOp_0[] = {
    {"MyTextureOp", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_UINT, 1,
     1},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
    {"offset", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
};

// uint = Texutre1D.MyTextureOp(uint addr, uint offset, uint val)
static const HLSL_INTRINSIC_ARGUMENT TestMyTexture1DOp_1[] = {
    {"MyTextureOp", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_UINT, 1,
     1},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
    {"offset", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
    {"val", AR_QUAL_IN, 1, LITEMPLATE_SCALAR, 1, LICOMPTYPE_UINT, 1, 1},
};

// uint2 = Texture2D.MyTextureOp(uint2 addr, uint2 val)
static const HLSL_INTRINSIC_ARGUMENT TestMyTexture2DOp[] = {
    {"MyTextureOp", AR_QUAL_OUT, 0, LITEMPLATE_VECTOR, 0, LICOMPTYPE_UINT, 1,
     1},
    {"addr", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
    {"val", AR_QUAL_IN, 1, LITEMPLATE_VECTOR, 1, LICOMPTYPE_UINT, 1, 2},
};

// float = test_overload(float a, uint b, double c)
static const HLSL_INTRINSIC_ARGUMENT TestOverloadArgs[] = {
    {"test_overload", AR_QUAL_OUT, 0, LITEMPLATE_SCALAR, 0, LICOMPTYPE_NUMERIC,
     1, IA_C},
    {"a", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_FLOAT, 1, IA_C},
    {"b", AR_QUAL_IN, 2, LITEMPLATE_ANY, 2, LICOMPTYPE_UINT, 1, IA_C},
    {"c", AR_QUAL_IN, 3, LITEMPLATE_SCALAR, 3, LICOMPTYPE_DOUBLE, 1, IA_C},
};

struct Intrinsic {
  LPCWSTR hlslName;
  const char *dxilName;
  const char *strategy;
  HLSL_INTRINSIC hlsl;
};
const char *DEFAULT_NAME = "";

// llvm::array_lengthof that returns a UINT instead of size_t
template <class T, std::size_t N> UINT countof(T (&)[N]) {
  return static_cast<UINT>(N);
}

Intrinsic Intrinsics[] = {
    {L"test_fn",
     DEFAULT_NAME,
     "r",
     {1, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnArgs), TestFnArgs}},
    {L"test_proc",
     DEFAULT_NAME,
     "r",
     {2, 0, 0, -1, countof(TestProcArgs), TestProcArgs}},
    {L"test_poly",
     "test_poly.$o",
     "r",
     {3, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnCustomArgs),
      TestFnCustomArgs}},
    {L"test_int",
     "test_int",
     "r",
     {4, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnIntArgs), TestFnIntArgs}},
    {L"test_nolower",
     "test_nolower.$o",
     "n",
     {5, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnNoLowerArgs),
      TestFnNoLowerArgs}},
    {L"test_pack_0",
     "test_pack_0.$o",
     "p",
     {6, 0, 0, -1, countof(TestFnPack0), TestFnPack0}},
    {L"test_pack_1",
     "test_pack_1.$o",
     "p",
     {7, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnPack1), TestFnPack1}},
    {L"test_pack_2",
     "test_pack_2.$o",
     "p",
     {8, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnPack2), TestFnPack2}},
    {L"test_pack_3",
     "test_pack_3.$o",
     "p",
     {9, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestFnPack3), TestFnPack3}},
    {L"test_pack_4",
     "test_pack_4.$o",
     "p",
     {10, 0, 0, -1, countof(TestFnPack4), TestFnPack4}},
    {L"test_rand",
     "test_rand",
     "r",
     {11, 0, 0, -1, countof(TestRand), TestRand}},
    {L"test_isinf",
     "test_isinf",
     "d",
     {13, INTRIN_FLAG_READ_ONLY | INTRIN_FLAG_READ_NONE, 0, -1,
      countof(TestIsInf), TestIsInf}},
    {L"test_ibfe",
     "test_ibfe",
     "d",
     {14, INTRIN_FLAG_READ_ONLY | INTRIN_FLAG_READ_NONE, 0, -1,
      countof(TestIBFE), TestIBFE}},
    // Make this intrinsic have the same opcode as an hlsl intrinsic with an
    // unsigned counterpart for testing purposes.
    {L"test_unsigned",
     "test_unsigned",
     "n",
     {static_cast<unsigned>(hlsl::IntrinsicOp::IOP_min), INTRIN_FLAG_READ_NONE,
      0, -1, countof(TestUnsigned), TestUnsigned}},
    {L"wave_proc",
     DEFAULT_NAME,
     "r",
     {16, INTRIN_FLAG_READ_NONE | INTRIN_FLAG_IS_WAVE, 0, -1,
      countof(WaveProcArgs), WaveProcArgs}},
    {L"test_o_1",
     "test_o_1.$o:1",
     "r",
     {18, INTRIN_FLAG_READ_NONE | INTRIN_FLAG_IS_WAVE, 0, -1,
      countof(TestOverloadArgs), TestOverloadArgs}},
    {L"test_o_2",
     "test_o_2.$o:2",
     "r",
     {19, INTRIN_FLAG_READ_NONE | INTRIN_FLAG_IS_WAVE, 0, -1,
      countof(TestOverloadArgs), TestOverloadArgs}},
    {L"test_o_3",
     "test_o_3.$o:3",
     "r",
     {20, INTRIN_FLAG_READ_NONE | INTRIN_FLAG_IS_WAVE, 0, -1,
      countof(TestOverloadArgs), TestOverloadArgs}},
    // custom lowering with both optional arguments and vector exploding.
    // Arg 0 = Opcode
    // Arg 1 = Pass as is
    // Arg 2:?i1 = Optional boolean argument
    // Arg 3.0:?i32 = Optional x component (in i32) of 3rd HLSL arg
    // Arg 3.1:?i32 = Optional y component (in i32) of 3rd HLSL arg
    {L"CustomLoadOp",
     "CustomLoadOp",
     "c:{\"default\" : \"0,1,2:?i1,3.0:?i32,3.1:?i32\"}",
     {21, INTRIN_FLAG_READ_ONLY, 0, -1, countof(TestCustomLoadOp),
      TestCustomLoadOp}},
    {L"CustomLoadOp",
     "CustomLoadOp",
     "c:{\"default\" : \"0,1,2:?i1,3.0:?i32,3.1:?i32\"}",
     {21, INTRIN_FLAG_READ_ONLY, 0, -1, countof(TestCustomLoadOpBool),
      TestCustomLoadOpBool}},
    {L"CustomLoadOp",
     "CustomLoadOp",
     "c:{\"default\" : \"0,1,2:?i1,3.0:?i32,3.1:?i32\"}",
     {21, INTRIN_FLAG_READ_ONLY, 0, -1, countof(TestCustomLoadOpSubscript),
      TestCustomLoadOpSubscript}},
};

Intrinsic BufferIntrinsics[] = {
    {L"MyBufferOp",
     "MyBufferOp",
     "m",
     {12, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestMyBufferOp),
      TestMyBufferOp}},
};

// Test adding a method to an object that normally has no methods (SamplerState
// will do).
Intrinsic SamplerIntrinsics[] = {
    {L"MySamplerOp",
     "MySamplerOp",
     "m",
     {15, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestMySamplerOp),
      TestMySamplerOp}},
};

// Define a lowering string to target a common dxil extension operation defined
// like this:
//
// @MyTextureOp(i32 opcode, %dx.types.Handle, i32 addr0, i32 addr1, i32 offset,
// i32 val0, i32 val1);
//
//  This would produce the following lowerings (assuming the MyTextureOp opcode
//  is 17)
//
//  hlsl: Texture1D.MyTextureOp(a, b)
//  hl:   @MyTextureOp(17, handle, a, b)
//  dxil: @MyTextureOp(17, handle, a, undef, b, undef, undef)
//
//  hlsl: Texture1D.MyTextureOp(a, b, c)
//  hl:   @MyTextureOp(17, handle, a, b, c)
//  dxil: @MyTextureOp(17, handle, a, undef, b, c, undef)
//
//  hlsl: Texture2D.MyTextureOp(a, c)
//  hl:   @MyTextureOp(17, handle, a, c)
//  dxil: @MyTextureOp(17, handle, a.x, a.y, undef, c.x, c.y)
//
static const char *MyTextureOp_LoweringInfo =
    "m:{"
    "\"default\"   : \"0,1,2.0,2.1,3,4.0:?i32,4.1:?i32\","
    "\"Texture2D\" : \"0,1,2.0,2.1,-1:?i32,3.0,3.1\""
    "}";
Intrinsic Texture1DIntrinsics[] = {
    {L"MyTextureOp",
     "MyTextureOp",
     MyTextureOp_LoweringInfo,
     {17, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestMyTexture1DOp_0),
      TestMyTexture1DOp_0}},
    {L"MyTextureOp",
     "MyTextureOp",
     MyTextureOp_LoweringInfo,
     {17, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestMyTexture1DOp_1),
      TestMyTexture1DOp_1}},
};

Intrinsic Texture2DIntrinsics[] = {
    {L"MyTextureOp",
     "MyTextureOp",
     MyTextureOp_LoweringInfo,
     {17, INTRIN_FLAG_READ_NONE, 0, -1, countof(TestMyTexture2DOp),
      TestMyTexture2DOp}},
};

class IntrinsicTable {
public:
  IntrinsicTable(const wchar_t *ns, Intrinsic *begin, Intrinsic *end)
      : m_namespace(ns), m_begin(begin), m_end(end) {}

  struct SearchResult {
    Intrinsic *intrinsic;
    uint64_t index;

    SearchResult() : SearchResult(nullptr, 0) {}
    SearchResult(Intrinsic *i, uint64_t n) : intrinsic(i), index(n) {}
    operator bool() { return intrinsic != nullptr; }
  };

  SearchResult Search(const wchar_t *name, std::ptrdiff_t startIndex) const {
    Intrinsic *begin = m_begin + startIndex;
    assert(std::distance(begin, m_end) >= 0);
    if (IsStar(name))
      return BuildResult(begin);

    Intrinsic *found = std::find_if(begin, m_end, [name](const Intrinsic &i) {
      return wcscmp(i.hlslName, name) == 0;
    });

    return BuildResult(found);
  }

  SearchResult Search(unsigned opcode) const {
    Intrinsic *begin = m_begin;
    assert(std::distance(begin, m_end) >= 0);

    Intrinsic *found = std::find_if(begin, m_end, [opcode](const Intrinsic &i) {
      return i.hlsl.Op == opcode;
    });

    return BuildResult(found);
  }

  bool MatchesNamespace(const wchar_t *ns) const {
    return wcscmp(m_namespace, ns) == 0;
  }

private:
  const wchar_t *m_namespace;
  Intrinsic *m_begin;
  Intrinsic *m_end;

  bool IsStar(const wchar_t *name) const { return wcscmp(name, L"*") == 0; }

  SearchResult BuildResult(Intrinsic *found) const {
    if (found == m_end)
      return SearchResult{nullptr, std::numeric_limits<uint64_t>::max()};

    return SearchResult{found,
                        static_cast<uint64_t>(std::distance(m_begin, found))};
  }
};

class TestIntrinsicTable : public IDxcIntrinsicTable {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)
  std::vector<IntrinsicTable> m_tables;

public:
  TestIntrinsicTable(Intrinsic *Intrinsics, unsigned IntrinsicsCount)
      : m_dwRef(0) {
    m_tables.push_back(
        IntrinsicTable(L"", Intrinsics, Intrinsics + IntrinsicsCount));
    m_tables.push_back(IntrinsicTable(L"Buffer", std::begin(BufferIntrinsics),
                                      std::end(BufferIntrinsics)));
    m_tables.push_back(IntrinsicTable(L"SamplerState",
                                      std::begin(SamplerIntrinsics),
                                      std::end(SamplerIntrinsics)));
    m_tables.push_back(IntrinsicTable(L"Texture1D",
                                      std::begin(Texture1DIntrinsics),
                                      std::end(Texture1DIntrinsics)));
    m_tables.push_back(IntrinsicTable(L"Texture2D",
                                      std::begin(Texture2DIntrinsics),
                                      std::end(Texture2DIntrinsics)));
  }

  TestIntrinsicTable()
      : TestIntrinsicTable(::Intrinsics, _countof(::Intrinsics)) {}

  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIntrinsicTable>(this, iid, ppvObject);
  }

  HRESULT STDMETHODCALLTYPE GetTableName(LPCSTR *pTableName) override {
    *pTableName = "test";
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE LookupIntrinsic(LPCWSTR typeName,
                                            LPCWSTR functionName,
                                            const HLSL_INTRINSIC **pIntrinsic,
                                            UINT64 *pLookupCookie) override {
    if (typeName == nullptr)
      return E_FAIL;

    // Search for matching intrinsic name in matching namespace.
    IntrinsicTable::SearchResult result;
    for (const IntrinsicTable &table : m_tables) {
      if (table.MatchesNamespace(typeName)) {
        result = table.Search(functionName, *pLookupCookie);
        break;
      }
    }

    if (result) {
      *pIntrinsic = &result.intrinsic->hlsl;
      *pLookupCookie = result.index + 1;
    } else {
      *pIntrinsic = nullptr;
      *pLookupCookie = 0;
    }

    return result.intrinsic ? S_OK : E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE GetLoweringStrategy(UINT opcode,
                                                LPCSTR *pStrategy) override {
    Intrinsic *intrinsic = FindByOpcode(opcode);

    if (!intrinsic)
      return E_FAIL;

    *pStrategy = intrinsic->strategy;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetIntrinsicName(UINT opcode,
                                             LPCSTR *pName) override {
    Intrinsic *intrinsic = FindByOpcode(opcode);

    if (!intrinsic)
      return E_FAIL;

    *pName = intrinsic->dxilName;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetDxilOpCode(UINT opcode,
                                          UINT *pDxilOpcode) override {
    if (opcode == 13) {
      *pDxilOpcode = static_cast<UINT>(hlsl::OP::OpCode::IsInf);
      return S_OK;
    } else if (opcode == 14) {
      *pDxilOpcode = static_cast<UINT>(hlsl::OP::OpCode::Ibfe);
      return S_OK;
    }
    return E_FAIL;
  }

  Intrinsic *FindByOpcode(UINT opcode) {
    IntrinsicTable::SearchResult result;
    for (const IntrinsicTable &table : m_tables) {
      result = table.Search(opcode);
      if (result)
        break;
    }
    return result.intrinsic;
  }
};

// A class to test semantic define validation.
// It takes a list of defines that when present should cause errors
// and defines that should cause warnings. A more realistic validator
// would look at the values and make sure (for example) they are
// the correct type (integer, string, etc).
class TestSemanticDefineValidator : public IDxcSemanticDefineValidator {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)
  std::vector<std::string> m_errorDefines;
  std::vector<std::string> m_warningDefines;

public:
  TestSemanticDefineValidator(const std::vector<std::string> &errorDefines,
                              const std::vector<std::string> &warningDefines)
      : m_dwRef(0), m_errorDefines(errorDefines),
        m_warningDefines(warningDefines) {}
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcSemanticDefineValidator>(this, iid,
                                                              ppvObject);
  }

  virtual HRESULT STDMETHODCALLTYPE GetSemanticDefineWarningsAndErrors(
      LPCSTR pName, LPCSTR pValue, IDxcBlobEncoding **ppWarningBlob,
      IDxcBlobEncoding **ppErrorBlob) override {
    if (!pName || !pValue || !ppWarningBlob || !ppErrorBlob)
      return E_FAIL;

    auto Check = [pName](const std::vector<std::string> &errors,
                         IDxcBlobEncoding **blob) {
      if (std::find(errors.begin(), errors.end(), pName) != errors.end()) {
        dxc::DxCompilerDllLoader dllSupport;
        VERIFY_SUCCEEDED(dllSupport.Initialize());
        std::string error("bad define: ");
        error.append(pName);
        Utf8ToBlob(dllSupport, error.c_str(), blob);
      }
    };
    Check(m_errorDefines, ppErrorBlob);
    Check(m_warningDefines, ppWarningBlob);

    return S_OK;
  }
};
static void CheckOperationFailed(IDxcOperationResult *pResult) {
  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_FAILED(status);
}

static std::string GetCompileErrors(IDxcOperationResult *pResult) {
  CComPtr<IDxcBlobEncoding> pErrors;
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  if (!pErrors)
    return "";
  return BlobToUtf8(pErrors);
}

class Compiler {
public:
  Compiler(dxc::DxCompilerDllLoader &dll) : m_dllSupport(dll) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    VERIFY_SUCCEEDED(pCompiler.QueryInterface(&pLangExtensions));
  }
  void RegisterSemanticDefine(LPCWSTR define) {
    VERIFY_SUCCEEDED(pLangExtensions->RegisterSemanticDefine(define));
  }
  void RegisterSemanticDefineExclusion(LPCWSTR define) {
    VERIFY_SUCCEEDED(pLangExtensions->RegisterSemanticDefineExclusion(define));
  }
  void SetSemanticDefineValidator(IDxcSemanticDefineValidator *validator) {
    pTestSemanticDefineValidator = validator;
    VERIFY_SUCCEEDED(pLangExtensions->SetSemanticDefineValidator(
        pTestSemanticDefineValidator));
  }
  void SetSemanticDefineMetaDataName(const char *name) {
    VERIFY_SUCCEEDED(
        pLangExtensions->SetSemanticDefineMetaDataName("test.defs"));
  }
  void SetTargetTriple(const char *name) {
    VERIFY_SUCCEEDED(pLangExtensions->SetTargetTriple(name));
  }
  void RegisterIntrinsicTable(IDxcIntrinsicTable *table) {
    pTestIntrinsicTable = table;
    VERIFY_SUCCEEDED(
        pLangExtensions->RegisterIntrinsicTable(pTestIntrinsicTable));
  }

  IDxcOperationResult *Compile(const char *program) {
    return Compile(program, {}, {});
  }

  IDxcOperationResult *Compile(const char *program,
                               const std::vector<LPCWSTR> &arguments,
                               const std::vector<DxcDefine> defs,
                               LPCWSTR target = L"ps_6_0") {
    Utf8ToBlob(m_dllSupport, program, &pCodeBlob);
    VERIFY_SUCCEEDED(pCompiler->Compile(
        pCodeBlob, L"hlsl.hlsl", L"main", target,
        const_cast<LPCWSTR *>(arguments.data()), arguments.size(), defs.data(),
        defs.size(), nullptr, &pCompileResult));

    return pCompileResult;
  }

  std::string Disassemble() {
    CComPtr<IDxcBlob> pBlob;
    CheckOperationSucceeded(pCompileResult, &pBlob);
    return DisassembleProgram(m_dllSupport, pBlob);
  }

  dxc::DxCompilerDllLoader &m_dllSupport;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcLangExtensions3> pLangExtensions;
  CComPtr<IDxcBlobEncoding> pCodeBlob;
  CComPtr<IDxcOperationResult> pCompileResult;
  CComPtr<IDxcSemanticDefineValidator> pTestSemanticDefineValidator;
  CComPtr<IDxcIntrinsicTable> pTestIntrinsicTable;
};

///////////////////////////////////////////////////////////////////////////////
// Extension unit tests.

#ifdef _WIN32
class ExtensionTest {
#else
class ExtensionTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(ExtensionTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  dxc::DxCompilerDllLoader m_dllSupport;

  TEST_METHOD(EvalAttributeCollision)
  TEST_METHOD(NoUnwind)
  TEST_METHOD(DCE)
  TEST_METHOD(DefineWhenRegisteredThenPreserved)
  TEST_METHOD(DefineValidationError)
  TEST_METHOD(DefineValidationWarning)
  TEST_METHOD(DefineNoValidatorOk)
  TEST_METHOD(DefineFromMacro)
  TEST_METHOD(DefineContradictionFail)
  TEST_METHOD(DefineOverrideSource)
  TEST_METHOD(DefineOverrideDefinesList)
  TEST_METHOD(DefineOverrideCommandLine)
  TEST_METHOD(DefineOverrideNone)
  TEST_METHOD(DefineOverrideDeterministicOutput)
  TEST_METHOD(OptionFromDefineGVN)
  TEST_METHOD(OptionFromDefineStructurizeReturns)
  TEST_METHOD(OptionFromDefineLifetimeMarkers)
  TEST_METHOD(TargetTriple)
  TEST_METHOD(IntrinsicWhenAvailableThenUsed)
  TEST_METHOD(CustomIntrinsicName)
  TEST_METHOD(NoLowering)
  TEST_METHOD(PackedLowering)
  TEST_METHOD(ReplicateLoweringWhenOnlyVectorIsResult)
  TEST_METHOD(UnsignedOpcodeIsUnchanged)
  TEST_METHOD(ResourceExtensionIntrinsic)
  TEST_METHOD(NameLoweredWhenNoReplicationNeeded)
  TEST_METHOD(DxilLoweringVector1)
  TEST_METHOD(DxilLoweringVector2)
  TEST_METHOD(DxilLoweringScalar)
  TEST_METHOD(SamplerExtensionIntrinsic)
  TEST_METHOD(WaveIntrinsic)
  TEST_METHOD(ResourceExtensionIntrinsicCustomLowering1)
  TEST_METHOD(ResourceExtensionIntrinsicCustomLowering2)
  TEST_METHOD(ResourceExtensionIntrinsicCustomLowering3)
  TEST_METHOD(CustomOverloadArg1)
  TEST_METHOD(CustomLoadOp)
};

TEST_F(ExtensionTest, DefineWhenRegisteredThenPreserved) {
  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.RegisterSemanticDefineExclusion(L"FOOBAR");
  c.SetSemanticDefineValidator(
      new TestSemanticDefineValidator({"FOOLALA"}, {}));
  c.SetSemanticDefineMetaDataName("test.defs");
  c.Compile("#define FOOTBALL AWESOME\n"
            "#define FOOTLOOSE TOO\n"
            "#define FOOBAR 123\n"
            "#define FOOD\n"
            "#define FOO 1 2 3\n"
            "float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            {L"/Vd"}, {{L"FOODEF", L"1"}});
  std::string disassembly = c.Disassemble();
  // Check for root named md node. It contains pointers to md nodes for each
  // define.
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
  // #define FOODEF 1
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("!{!\"FOODEF\", !\"1\"}"));
  // #define FOOTBALL AWESOME
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("!{!\"FOOTBALL\", !\"AWESOME\"}"));
  // #define FOOTLOOSE TOO
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("!{!\"FOOTLOOSE\", !\"TOO\"}"));
  // #define FOOD
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!{!\"FOOD\", !\"\"}"));
  // #define FOO 1 2 3
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("!{!\"FOO\", !\"1 2 3\"}"));
  // FOOBAR should be excluded.
  VERIFY_IS_TRUE(disassembly.npos == disassembly.find("!{!\"FOOBAR\""));
}

TEST_F(ExtensionTest, DefineValidationError) {
  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.SetSemanticDefineValidator(new TestSemanticDefineValidator({"FOO"}, {}));
  IDxcOperationResult *pCompileResult =
      c.Compile("#define FOO 1\n"
                "float4 main() : SV_Target {\n"
                "  return 0;\n"
                "}\n",
                {L"/Vd"}, {});

  // Check that validation error causes compile failure.
  CheckOperationFailed(pCompileResult);
  std::string errors = GetCompileErrors(pCompileResult);
  // Check that the error message is for the validation failure.
  VERIFY_IS_TRUE(errors.npos !=
                 errors.find("hlsl.hlsl:1:9: error: bad define: FOO"));
}

TEST_F(ExtensionTest, DefineValidationWarning) {
  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.SetSemanticDefineValidator(new TestSemanticDefineValidator({}, {"FOO"}));
  IDxcOperationResult *pCompileResult =
      c.Compile("#define FOO 1\n"
                "float4 main() : SV_Target {\n"
                "  return 0;\n"
                "}\n",
                {L"/Vd"}, {});

  std::string errors = GetCompileErrors(pCompileResult);
  // Check that the error message is for the validation failure.
  VERIFY_IS_TRUE(errors.npos !=
                 errors.find("hlsl.hlsl:1:9: warning: bad define: FOO"));

  // Check the define is still emitted.
  std::string disassembly = c.Disassemble();
  // Check for root named md node. It contains pointers to md nodes for each
  // define.
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!hlsl.semdefs"));
  // #define FOO 1
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!{!\"FOO\", !\"1\"}"));
}

TEST_F(ExtensionTest, DefineNoValidatorOk) {
  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.Compile("#define FOO 1\n"
            "float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            {L"/Vd"}, {});

  std::string disassembly = c.Disassemble();
  // Check the define is emitted.
  // #define FOO 1
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!{!\"FOO\", !\"1\"}"));
}

TEST_F(ExtensionTest, DefineFromMacro) {
  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.Compile("#define BAR 1\n"
            "#define FOO BAR\n"
            "float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            {L"/Vd"}, {});

  std::string disassembly = c.Disassemble();
  // Check the define is emitted.
  // #define FOO 1
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!{!\"FOO\", !\"1\"}"));
}

// Test failure of contradictory optimization toggles
TEST_F(ExtensionTest, DefineContradictionFail) {
  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  IDxcOperationResult *pCompileResult = c.Compile(
      "#define FOO 1\n"
      "float4 main() : SV_Target {\n"
      "  return 0;\n"
      "}\n",
      {L"/Vd", L"-opt-disable", L"whatever", L"-opt-enable", L"whatever"}, {});

  CheckOperationFailed(pCompileResult);
  std::string errors = GetCompileErrors(pCompileResult);
  // Check that the error message is for the option contradiction
  VERIFY_IS_TRUE(errors.npos !=
                 errors.find("Contradictory use of -opt-disable and "
                             "-opt-enable with \"whatever\""));

  Compiler c2(m_dllSupport);
  c2.RegisterSemanticDefine(L"FOO*");
  pCompileResult = c2.Compile("#define FOO 1\n"
                              "float4 main() : SV_Target {\n"
                              "  return 0;\n"
                              "}\n",
                              {L"/Vd", L"-opt-select", L"yook", L"butterUP",
                               L"-opt-select", L"yook", L"butterdown"},
                              {});

  CheckOperationFailed(pCompileResult);
  errors = GetCompileErrors(pCompileResult);
  // Check that the error message is for the option contradiction
  VERIFY_IS_TRUE(errors.npos !=
                 errors.find("Contradictory -opt-selects for \"yook\""));
}

// Test that the override-semdef flag can override a define from the source.
TEST_F(ExtensionTest, DefineOverrideSource) {

  // Compile baseline without the override.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("#define FOO_A 1\n"
              "float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd"}, {});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"1\"}"));
  }

  // Compile with an override.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("#define FOO_A 1\n"
              "float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd", L"-override-semdef", L"FOO_A=7"}, {});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"7\"}"));
  }
}

// Test that the override-semdef flag can override a define from the compile
// defines list.
TEST_F(ExtensionTest, DefineOverrideDefinesList) {
  // Compile baseline without the override.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd"}, {{L"FOO_A", L"1"}});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"1\"}"));
  }

  // Compile with an override.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd", L"-override-semdef", L"FOO_A=7"}, {{L"FOO_A", L"1"}});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"7\"}"));
  }
}

// Test that the override-semdef flag can override a define from the command
// line.
TEST_F(ExtensionTest, DefineOverrideCommandLine) {
  // Compile baseline without the override.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd", L"/DFOO_A=1"}, {});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"1\"}"));
  }

  // Compile with an override after the /D define.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd", L"/DFOO_A=1", L"-override-semdef", L"FOO_A=7"},
              {{L"FOO_A", L"1"}});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"7\"}"));
  }

  // Compile with an override before the /D define.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd", L"-override-semdef", L"FOO_A=7", L"/DFOO_A=1"},
              {{L"FOO_A", L"1"}});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"7\"}"));
  }
}

// Test that the override-semdef flag can override a define when there is no
// previous def.
TEST_F(ExtensionTest, DefineOverrideNone) {
  // Compile baseline without the define.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd"}, {});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos == disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos ==
                   disassembly.find("!{!\"FOO_A\", !\"7\"}"));
  }

  // Compile with an override.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile("float4 main() : SV_Target {\n"
              "  return 0;\n"
              "}\n",
              {L"/Vd", L"-override-semdef", L"FOO_A=7"}, {});
    std::string disassembly = c.Disassemble();
    // Check for root named md node. It contains pointers to md nodes for each
    // define.
    VERIFY_IS_TRUE(disassembly.npos != disassembly.find("!test.defs"));
    // Make sure we get the overrriden value.
    VERIFY_IS_TRUE(disassembly.npos !=
                   disassembly.find("!{!\"FOO_A\", !\"7\"}"));
  }
}

TEST_F(ExtensionTest, DefineOverrideDeterministicOutput) {

  std::string source = "#define FOO_B 1\n"
                       "#define FOO_A 1\n"
                       "float4 main() : SV_Target {\n"
                       "  return 0;\n"
                       "}\n";

  std::string baselineOutput;
  // Compile baseline.
  {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile(source.data(),
              {L"/Vd", L"-override-semdef", L"FOO_A=7", L"-override-semdef",
               L"FOO_B=7"},
              {});
    baselineOutput = c.Disassemble();
  }

  // Compile NUM_COMPILES times and make sure the output always matches.
  enum { NUM_COMPILES = 10 };
  for (int i = 0; i < NUM_COMPILES; ++i) {
    Compiler c(m_dllSupport);
    c.RegisterSemanticDefine(L"FOO*");
    c.SetSemanticDefineMetaDataName("test.defs");
    c.Compile(source.data(),
              {L"/Vd", L"-override-semdef", L"FOO_A=7", L"-override-semdef",
               L"FOO_B=7"},
              {});
    std::string disassembly = c.Disassemble();
    // Make sure the outputs are the same.
    VERIFY_ARE_EQUAL(baselineOutput, disassembly);
  }
}

// Test setting of codegen options from semantic defines
TEST_F(ExtensionTest, OptionFromDefineGVN) {

  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.Compile("float4 main(float a : A) : SV_Target {\n"
            "  float res = sin(a);\n"
            "  return res + sin(a);\n"
            "}\n",
            {L"/Vd", L"-DFOO_DISABLE_GVN"}, {});

  std::string disassembly = c.Disassemble();
  // Verify that GVN is disabled by the presence
  // of the second sin(), which GVN would have removed
  llvm::Regex regex(
      "call float @dx.op.unary.f32.*\n.*call float @dx.op.unary.f32");
  std::string regexErrors;
  VERIFY_IS_TRUE(regex.isValid(regexErrors));
  VERIFY_IS_TRUE(regex.match(disassembly));
}

// Test setting of codegen options from semantic defines
TEST_F(ExtensionTest, OptionFromDefineStructurizeReturns) {

  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.Compile("int i;\n"
            "float main(float4 a:A) : SV_Target {\n"
            "float c = 0;\n"
            "if (i < 0) {\n"
            "  if (a.w > 2)\n"
            "    return -1;\n"
            "  c += a.z;\n"
            "}\n"
            "return c;\n"
            "}\n",
            {L"/Vd", L"-fcgl", L"-DFOO_ENABLE_STRUCTURIZE_RETURNS"}, {});

  std::string disassembly = c.Disassemble();
  // Verify that structurize returns is enabled by the presence
  // of the associated annotation. Just a simple test to
  // verify that it's on. No need to go into detail here
  llvm::Regex regex("bReturned.* = alloca i1");
  std::string regexErrors;
  VERIFY_IS_TRUE(regex.isValid(regexErrors));
  VERIFY_IS_TRUE(regex.match(disassembly));
}

// Test setting of codegen options from semantic defines
TEST_F(ExtensionTest, OptionFromDefineLifetimeMarkers) {
  std::string shader =
      "\n"
      "float foo(float a) {\n"
      "float res[2] = {a, 2 * 2};\n"
      "return res[a];\n"
      "}\n"
      "float4 main(float a : A) : SV_Target { return foo(a); }\n";

  Compiler c(m_dllSupport);
  c.RegisterSemanticDefine(L"FOO*");
  c.Compile(shader.data(), {L"/Vd", L"-DFOO_DISABLE_LIFETIME_MARKERS"}, {},
            L"ps_6_6");

  std::string disassembly = c.Disassemble();
  Compiler c2(m_dllSupport);
  c2.Compile(shader.data(), {L"/Vd", L""}, {}, L"ps_6_6");
  std::string disassembly2 = c2.Disassemble();
  // Make sure lifetime marker not exist with FOO_DISABLE_LIFETIME_MARKERS.
  VERIFY_IS_TRUE(disassembly.find("lifetime") == std::string::npos);
  VERIFY_IS_TRUE(disassembly.find("FOO_DISABLE_LIFETIME_MARKERS\", !\"1\"") !=
                 std::string::npos);
  // Make sure lifetime marker exist by default.
  VERIFY_IS_TRUE(disassembly2.find("lifetime") != std::string::npos);
}

TEST_F(ExtensionTest, TargetTriple) {
  Compiler c(m_dllSupport);
  c.SetTargetTriple("dxil-ms-win32");
  c.Compile("float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            {L"/Vd"}, {});

  std::string disassembly = c.Disassemble();
  // Check the triple is updated.
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("dxil-ms-win32"));
}

TEST_F(ExtensionTest, IntrinsicWhenAvailableThenUsed) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("float2 main(float2 v : V, int2 i : I) : SV_Target {\n"
            "  test_proc(v);\n"
            "  float2 a = test_fn(v);\n"
            "  int2 b = test_fn(i);\n"
            "  return a + b;\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Things to call out:
  // - result is float, not a vector
  // - mangled name contains the 'test' and '.r' parts
  // - opcode is first i32 argument
  // - second argument is float, ie it got scalarized
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call void "
                                  "@\"test.\\01?test_proc@@YAXV?$vector@M$"
                                  "01@@@Z.r\"(i32 2, float"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call float "
                                  "@\"test.\\01?test_fn@@YA?AV?$vector@M$"
                                  "01@@V1@@Z.r\"(i32 1, float"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call i32 "
                                  "@\"test.\\01?test_fn@@YA?AV?$vector@H$"
                                  "01@@V1@@Z.r\"(i32 1, i32"));

  // - attributes are added to the declaration (the # at the end of the decl)
  //   TODO: would be nice to check for the actual attribute (e.g. readonly)
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("declare float "
                                  "@\"test.\\01?test_fn@@YA?AV?$vector@M$"
                                  "01@@V1@@Z.r\"(i32, float) #"));
}

TEST_F(ExtensionTest, CustomIntrinsicName) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("float2 main(float2 v : V, int2 i : I) : SV_Target {\n"
            "  float2 a = test_poly(v);\n"
            "  int2   b = test_poly(i);\n"
            "  int2   c = test_int(i);\n"
            "  return a + b + c;\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // - custom name works for polymorphic function
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call float @test_poly.float(i32 3, float"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call i32 @test_poly.i32(i32 3, i32"));

  // - custom name works for non-polymorphic function
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call i32 @test_int(i32 4, i32"));
}

TEST_F(ExtensionTest, NoLowering) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("float2 main(float2 v : V, int2 i : I) : SV_Target {\n"
            "  float2 a = test_nolower(v);\n"
            "  float2 b = test_nolower(i);\n"
            "  return a + b;\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // - custom name works for non-lowered function
  // - non-lowered function has vector type as argument
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find(
          "call <2 x float> @test_nolower.float(i32 5, <2 x float>"));
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find("call <2 x i32> @test_nolower.i32(i32 5, <2 x i32>"));
}

TEST_F(ExtensionTest, PackedLowering) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("float2 main(float2 v1 : V1, float2 v2 : V2, float3 v3 : V3) : "
            "SV_Target {\n"
            "  test_pack_0(v1);\n"
            "  int2   a = test_pack_1();\n"
            "  float2 b = test_pack_2(v1, v2);\n"
            "  float  c = test_pack_3(v1);\n"
            "  float2 d = test_pack_4(v3);\n"
            "  return a + b + float2(c, c);\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // - pack strategy changes vectors into structs
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find("call void @test_pack_0.float(i32 6, { float, float }"));
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find("call { float, float } @test_pack_1.float(i32 7)"));
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find(
          "call { float, float } @test_pack_2.float(i32 8, { float, float }"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find(
                     "call float @test_pack_3.float(i32 9, { float, float }"));
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find("call { float, float } @test_pack_4.float(i32 10, { "
                       "float, float, float }"));
}

TEST_F(ExtensionTest, ReplicateLoweringWhenOnlyVectorIsResult) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("float2 main(float2 v1 : V1, float2 v2 : V2, float3 v3 : V3) : "
            "SV_Target {\n"
            "  return test_rand();\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // - replicate strategy works for vector results
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call float @test_rand(i32 11)"));
}

TEST_F(ExtensionTest, UnsignedOpcodeIsUnchanged) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("uint main(uint v1 : V1) : SV_Target {\n"
            "  return test_unsigned(v1);\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // - opcode is unchanged when it matches an hlsl intrinsic with
  //   an unsigned version.
  // This should use the same value as IOP_min.
  std::string matchStr;
  std::ostringstream ss(matchStr);
  ss << "call i32 @test_unsigned(i32 " << (unsigned)hlsl::IntrinsicOp::IOP_min
     << ", ";

  VERIFY_IS_TRUE(disassembly.npos != disassembly.find(ss.str()));
}

TEST_F(ExtensionTest, ResourceExtensionIntrinsic) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("Buffer<float2> buf;"
            "float2 main(uint2 v1 : V1) : SV_Target {\n"
            "  return buf.MyBufferOp(uint2(1, 2));\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Things to check
  // - return type is translated to dx.types.ResRet
  // - buffer is translated to dx.types.Handle
  // - vector is exploded
  llvm::Regex regex("call %dx.types.ResRet.f32 @MyBufferOp\\(i32 12, "
                    "%dx.types.Handle %.*, i32 1, i32 2\\)");
  std::string regexErrors;
  VERIFY_IS_TRUE(regex.isValid(regexErrors));
  VERIFY_IS_TRUE(regex.match(disassembly));
}

TEST_F(ExtensionTest, CustomLoadOp) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  auto result =
      c.Compile("float2 main(uint2 v1 : V1) : SV_Target {\n"
                "  float2 a = CustomLoadOp(uint2(1,2));\n"
                "  float2 b = CustomLoadOp(uint2(3,4),1);\n"
                "  float2 c = CustomLoadOp(uint2(5,6),1,uint2(7,8));\n"
                "  return a+b+c;\n"
                "}\n",
                {L"/Vd"}, {});
  CheckOperationResultMsgs(result, {}, true, false);
  std::string disassembly = c.Disassemble();

  // Things to check
  // - return type is 2xfloat
  // - input type is 2x struct
  // - function overload handles variable arguments (and replaces them with
  // undefs)
  // - output struct gets converted to vector
  LPCSTR expected[] = {
      "%1 = call { float, float } @CustomLoadOp(i32 21, { i32, i32 } { i32 1, "
      "i32 2 }, i1 undef, i32 undef, i32 undef)",
      "%2 = extractvalue { float, float } %1, 0",
      "%3 = extractvalue { float, float } %1, 1",
      "%4 = call { float, float } @CustomLoadOp(i32 21, { i32, i32 } { i32 3, "
      "i32 4 }, i1 true, i32 undef, i32 undef)",
      "%5 = extractvalue { float, float } %4, 0",
      "%6 = extractvalue { float, float } %4, 1",
      "%7 = call { float, float } @CustomLoadOp(i32 21, { i32, i32 } { i32 5, "
      "i32 6 }, i1 true, i32 7, i32 8)",
      "%8 = extractvalue { float, float } %7, 0",
      "%9 = extractvalue { float, float } %7, 1"};

  CheckMsgs(disassembly.c_str(), disassembly.length(), expected,
            llvm::array_lengthof(expected), false);
}

TEST_F(ExtensionTest, NameLoweredWhenNoReplicationNeeded) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("int main(int v1 : V1) : SV_Target {\n"
            "  return test_int(v1);\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Make sure the name is still lowered even when no replication
  // is needed because a non-vector overload of the function
  // was used.
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("call i32 @test_int("));
}

TEST_F(ExtensionTest, DxilLoweringVector1) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("int main(float v1 : V1) : SV_Target {\n"
            "  return test_isinf(v1);\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Check that the extension was lowered to the correct dxil intrinsic.
  static_assert(9 == (unsigned)hlsl::OP::OpCode::IsInf,
                "isinf opcode changed?");
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call i1 @dx.op.isSpecialFloat.f32(i32 9"));
}

TEST_F(ExtensionTest, DxilLoweringVector2) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("int2 main(float2 v1 : V1) : SV_Target {\n"
            "  return test_isinf(v1);\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Check that the extension was lowered to the correct dxil intrinsic.
  static_assert(9 == (unsigned)hlsl::OP::OpCode::IsInf,
                "isinf opcode changed?");
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call i1 @dx.op.isSpecialFloat.f32(i32 9"));
}

TEST_F(ExtensionTest, DxilLoweringScalar) {
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("int main(uint v1 : V1, uint v2 : V2, uint v3 : V3) : SV_Target {\n"
            "  return test_ibfe(v1, v2, v3);\n"
            "}\n",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Check that the extension was lowered to the correct dxil intrinsic.
  static_assert(51 == (unsigned)hlsl::OP::OpCode::Ibfe, "ibfe opcode changed?");
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call i32 @dx.op.tertiary.i32(i32 51"));
}

TEST_F(ExtensionTest, SamplerExtensionIntrinsic) {
  // Test adding methods to objects that don't have any methods normally,
  // and therefore have null default intrinsic table.
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  auto result = c.Compile("SamplerState samp;"
                          "float2 main(uint2 v1 : V1) : SV_Target {\n"
                          "  return samp.MySamplerOp(uint2(1, 2));\n"
                          "}\n",
                          {L"/Vd"}, {});
  CheckOperationResultMsgs(result, {}, true, false);
  std::string disassembly = c.Disassemble();

  // Things to check
  // - works when SamplerState normally has no methods
  // - return type is translated to dx.types.ResRet
  // - buffer is translated to dx.types.Handle
  // - vector is exploded
  LPCSTR expected[] = {"call %dx.types.ResRet.f32 @MySamplerOp\\(i32 15, "
                       "%dx.types.Handle %.*, i32 1, i32 2\\)"};
  CheckMsgs(disassembly.c_str(), disassembly.length(), expected, 1, true);
}

// Takes a string to match, and a regex pattern string, returns the first match
// at index [0], as well as sub expressions starting at index [1]
static std::vector<std::string> Match(const std::string &str,
                                      const std::string pattern) {
  std::vector<std::string> ret;
  llvm::Regex regex(pattern);
  std::string err;
  VERIFY_IS_TRUE(regex.isValid(err));
  llvm::SmallVector<llvm::StringRef, 4> matches;
  if (!regex.match(str, &matches))
    return ret;
  ret.assign(matches.begin(), matches.end());
  return ret;
}

//
// Regression test for extension functions having the same opcode as the
// following HLSL intrinsics, and triggering DxilLegalizeEvalOperations to make
// incorrect assumptions about allocas associated with it, causing them to be
// removed.
//
TEST_F(ExtensionTest, EvalAttributeCollision) {
  static const HLSL_INTRINSIC_ARGUMENT Args[] = {
      {"collide_proc", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
       IA_C},
      {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

  hlsl::IntrinsicOp ops[] = {
      hlsl::IntrinsicOp::IOP_GetAttributeAtVertex,
      hlsl::IntrinsicOp::IOP_EvaluateAttributeSnapped,
      hlsl::IntrinsicOp::IOP_EvaluateAttributeCentroid,
      hlsl::IntrinsicOp::IOP_EvaluateAttributeAtSample,
  };

  for (hlsl::IntrinsicOp op : ops) {
    Intrinsic Intrinsic = {L"collide_proc",
                           "collide_proc",
                           "r",
                           {static_cast<unsigned>(op), INTRIN_FLAG_READ_ONLY, 0,
                            -1, countof(Args), Args}};
    Compiler c(m_dllSupport);
    c.RegisterIntrinsicTable(new TestIntrinsicTable(&Intrinsic, 1));
    c.Compile(R"(
        float2 main(float2 a  : A, float2 b : B) : SV_Target {
            float2 ret = b;
            ret.x = collide_proc(ret.x);
            return ret;
        }
      )",
              {L"/Vd", L"/Od"}, {});

    std::string disassembly = c.Disassemble();

    auto match1 = Match(
        disassembly,
        std::string("%([0-9.a-zA-Z]*) = call float @collide_proc\\(i32 ") +
            std::to_string(Intrinsic.hlsl.Op));
    VERIFY_IS_TRUE(match1.size() == 2U);
    VERIFY_IS_TRUE(
        Match(disassembly, std::string("call void @dx.op.storeOutput.f32\\(i32 "
                                       "5, i32 0, i32 0, i8 0, float %") +
                               match1[1])
            .size() != 0U);
  }
}

// Regression test for extension functions having no 'nounwind' attribute
TEST_F(ExtensionTest, NoUnwind) {
  static const HLSL_INTRINSIC_ARGUMENT Args[] = {
      {"test_proc", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
       IA_C},
      {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

  Intrinsic Intrinsic = {
      L"test_proc", "test_proc", "r", {1, 0, 0, -1, countof(Args), Args}};
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable(&Intrinsic, 1));
  c.Compile(R"(
      float main(float a : A) : SV_Target {
          return test_proc(a);
      }
    )",
            {L"/Vd", L"/Od"}, {});

  std::string disassembly = c.Disassemble();

  /*
   * We're looking for this:
   *   declare float @test_proc(i32, float) #1
   *   attributes #1 = { nounwind }
   */
  auto m1 =
      Match(disassembly,
            std::string("declare float @test_proc\\(i32, float\\) #([0-9]*)"));
  VERIFY_IS_TRUE(m1.size() == 2U);
  VERIFY_IS_TRUE(
      Match(disassembly, std::string("attributes #") + m1[1] + " = { nounwind")
          .size() != 0U);
}

// Regression test for extension function calls not getting DCE'ed becuase they
// had no 'nounwind' attribute
TEST_F(ExtensionTest, DCE) {
  static const HLSL_INTRINSIC_ARGUMENT Args[] = {
      {"test_proc", AR_QUAL_OUT, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1,
       IA_C},
      {"value", AR_QUAL_IN, 1, LITEMPLATE_ANY, 1, LICOMPTYPE_NUMERIC, 1, IA_C}};

  Intrinsic Intrinsic = {L"test_proc",
                         "test_proc",
                         "r",
                         {1, INTRIN_FLAG_READ_ONLY | INTRIN_FLAG_READ_NONE, 0,
                          -1, countof(Args), Args}};
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable(&Intrinsic, 1));
  c.Compile(R"(
      float main(float a : A) : SV_Target {
          float dce = test_proc(a);
          return 0;
      }
    )",
            {L"/Vd", L"/Od"}, {});

  std::string disassembly = c.Disassemble();

  VERIFY_IS_TRUE(disassembly.npos == disassembly.find("call float @test_proc"));
}

TEST_F(ExtensionTest, WaveIntrinsic) {
  // Test wave-sensitive intrinsic in breaked loop
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  c.Compile("StructuredBuffer<int> buf[]: register(t2);"
            "float2 main(float2 a : A, int b : B) : SV_Target {"
            "  int res = 0;"
            "  float2 u = {0,0};"
            "  for (;;) {"
            "    u += wave_proc(a);"
            "    if (a.x == u.x) {"
            "      res += buf[b][(int)u.y];"
            "      break;"
            "    }"
            "  }"
            "  return res;"
            "}",
            {L"/Vd"}, {});
  std::string disassembly = c.Disassemble();

  // Check that the wave op causes the break block to be retained
  VERIFY_IS_TRUE(
      disassembly.npos !=
      disassembly.find(
          "@dx.break.cond = internal constant [1 x i32] zeroinitializer"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("%1 = load i32, i32* getelementptr inbounds "
                                  "([1 x i32], [1 x i32]* @dx.break.cond"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("%2 = icmp eq i32 %1, 0"));
  VERIFY_IS_TRUE(disassembly.npos !=
                 disassembly.find("call float "
                                  "@\"test.\\01?wave_proc@@YA?AV?$vector@"
                                  "M$01@@V1@@Z.r\"(i32 16, float"));
  VERIFY_IS_TRUE(disassembly.npos != disassembly.find("br i1 %2"));
}

TEST_F(ExtensionTest, ResourceExtensionIntrinsicCustomLowering1) {
  // Test adding methods to objects that don't have any methods normally,
  // and therefore have null default intrinsic table.
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  auto result = c.Compile("Texture1D tex1;"
                          "float2 main() : SV_Target {\n"
                          "  return tex1.MyTextureOp(1,2,3);\n"
                          "}\n",
                          {L"/Vd"}, {});
  CheckOperationResultMsgs(result, {}, true, false);
  std::string disassembly = c.Disassemble();

  // Things to check
  // @MyTextureOp(i32 opcode, %dx.types.Handle, i32 addr0, i32 addr1, i32
  // offset, i32 val0, i32 val1);
  //
  // hlsl: Texture1D.MyTextureOp(a, b, c)
  // dxil: @MyTextureOp(17, handle, a, undef, b, c, undef)
  //
  LPCSTR expected[] = {
      "call %dx.types.ResRet.i32 @MyTextureOp\\(i32 17, %dx.types.Handle %.*, "
      "i32 1, i32 undef, i32 2, i32 3, i32 undef\\)",
  };
  CheckMsgs(disassembly.c_str(), disassembly.length(), expected, 1, true);
}

TEST_F(ExtensionTest, ResourceExtensionIntrinsicCustomLowering2) {
  // Test adding methods to objects that don't have any methods normally,
  // and therefore have null default intrinsic table.
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  auto result = c.Compile("Texture2D tex2;"
                          "float2 main() : SV_Target {\n"
                          "  return tex2.MyTextureOp(uint2(4,5), uint2(6,7));\n"
                          "}\n",
                          {L"/Vd"}, {});
  CheckOperationResultMsgs(result, {}, true, false);
  std::string disassembly = c.Disassemble();

  // Things to check
  // @MyTextureOp(i32 opcode, %dx.types.Handle, i32 addr0, i32 addr1, i32
  // offset, i32 val0, i32 val1);
  //
  // hlsl: Texture2D.MyTextureOp(a, c)
  // dxil: @MyTextureOp(17, handle, a.x, a.y, undef, c.x, c.y)
  LPCSTR expected[] = {
      "call %dx.types.ResRet.i32 @MyTextureOp\\(i32 17, %dx.types.Handle %.*, "
      "i32 4, i32 5, i32 undef, i32 6, i32 7\\)",
  };
  CheckMsgs(disassembly.c_str(), disassembly.length(), expected, 1, true);
}

TEST_F(ExtensionTest, ResourceExtensionIntrinsicCustomLowering3) {
  // Test adding methods to objects that don't have any methods normally,
  // and therefore have null default intrinsic table.
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  auto result = c.Compile("Texture1D tex1;"
                          "float2 main() : SV_Target {\n"
                          "  return tex1.MyTextureOp(1,2);\n"
                          "}\n",
                          {L"/Vd"}, {});
  CheckOperationResultMsgs(result, {}, true, false);
  std::string disassembly = c.Disassemble();

  // Things to check
  // @MyTextureOp(i32 opcode, %dx.types.Handle, i32 addr0, i32 addr1, i32
  // offset, i32 val0, i32 val1);
  //
  // hlsl: Texture1D.MyTextureOp(a, b)
  // dxil: @MyTextureOp(17, handle, a, undef, b, undef, undef)
  //
  LPCSTR expected[] = {
      "call %dx.types.ResRet.i32 @MyTextureOp\\(i32 17, %dx.types.Handle %.*, "
      "i32 1, i32 undef, i32 2, i32 undef, i32 undef\\)",
  };
  CheckMsgs(disassembly.c_str(), disassembly.length(), expected, 1, true);
}

TEST_F(ExtensionTest, CustomOverloadArg1) {
  // Test that we pick the overload name based on the first arg.
  Compiler c(m_dllSupport);
  c.RegisterIntrinsicTable(new TestIntrinsicTable());
  auto result = c.Compile("float main() : SV_Target {\n"
                          "  float o1 = test_o_1(1.0f, 2u, 4.0);\n"
                          "  float o2 = test_o_2(1.0f, 2u, 4.0);\n"
                          "  float o3 = test_o_3(1.0f, 2u, 4.0);\n"
                          "  return o1 + o2 + o3;\n"
                          "}\n",
                          {L"/Vd"}, {});
  CheckOperationResultMsgs(result, {}, true, false);
  std::string disassembly = c.Disassemble();

  // The function name should match the first arg (float)
  LPCSTR expected[] = {
      "call float @test_o_1.float(i32 18, float 1.000000e+00, i32 2, double "
      "4.000000e+00)",
      "call float @test_o_2.i32(i32 18, float 1.000000e+00, i32 2, double "
      "4.000000e+00)",
      "call float @test_o_3.double(i32 18, float 1.000000e+00, i32 2, double "
      "4.000000e+00)",
  };
  CheckMsgs(disassembly.c_str(), disassembly.length(), expected, 1, false);
}
