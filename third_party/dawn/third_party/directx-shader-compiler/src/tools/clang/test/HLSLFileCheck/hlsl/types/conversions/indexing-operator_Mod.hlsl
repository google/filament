// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T vs_5_1 indexing-operator.hlsl

Buffer g_b;
StructuredBuffer<float4> g_sb;
Texture1D g_t1d;
Texture1DArray g_t1da;
Texture2D g_t2d;
Texture2DArray g_t2da;
Texture2DMS<float4, 8> g_t2dms;
Texture2DMSArray<float4, 8> g_t2dmsa;
Texture3D g_t3d;
TextureCube g_tc;
TextureCubeArray g_tca;

RWStructuredBuffer<float4> g_rw_sb;
RWBuffer<float4> g_rw_b;
RWTexture1D<float4> g_rw_t1d;
RWTexture1DArray<float4> g_rw_t1da;
RWTexture2D<float4> g_rw_t2d;
RWTexture2DArray<float4> g_rw_t2da;
RWTexture3D<float4> g_rw_t3d;

// No such thing as these:
// RWTexture2DMS g_rw_t2dms;
// RWTexture2DMSArray g_rw_t2dmsa;
// RWTextureCube g_rw_tc;
// RWTextureCubeArray g_rw_tca;

float test_vector_indexing()
{
  float4 f4 = { 1, 2, 3, 4 };
  float f = f4[0];
  /*verify-ast
    DeclStmt <col:3, col:18>
    `-VarDecl <col:3, col:17> f 'float'
      `-CXXOperatorCallExpr <col:13, col:17> 'float':'float' lvalue
        |-ImplicitCastExpr <col:15, col:17> 'float &(*)(unsigned int)' <FunctionToPointerDecay>
        | `-DeclRefExpr <col:15, col:17> 'float &(unsigned int)' lvalue CXXMethod 'operator[]' 'float &(unsigned int)'
        |-DeclRefExpr <col:13> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
        `-ImplicitCastExpr <col:16> 'unsigned int' <IntegralCast>
          `-IntegerLiteral <col:16> 'literal int' 0
  */
  //f = f4[0][1];
  return f;
}

float4 test_scalar_indexing()
{
  float4 f4 = 0;
  f4 += g_b[0];
  /*verify-ast
    CompoundAssignOperator <col:3, col:14> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:14> 'vector<float, 4>'
      |-ImplicitCastExpr <col:12, col:14> 'vector<float, 4> (*)(vector<uint, 1>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:12, col:14> 'vector<float, 4> (vector<uint, 1>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 1>) const'
      |-ImplicitCastExpr <col:9> 'const Buffer<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Buffer':'Buffer<vector<float, 4> >' lvalue Var 'g_b' 'Buffer':'Buffer<vector<float, 4> >'
      `-ImplicitCastExpr <col:13> 'vector<uint, 1>':'vector<uint, 1>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:13> 'unsigned int' <IntegralCast>
          `-IntegerLiteral <col:13> 'literal int' 0
  */
  f4 += g_t1d[0];
  /*verify-ast
    CompoundAssignOperator <col:3, col:16> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:16> 'vector<float, 4>'
      |-ImplicitCastExpr <col:14, col:16> 'vector<float, 4> (*)(vector<uint, 1>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:14, col:16> 'vector<float, 4> (vector<uint, 1>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 1>) const'
      |-ImplicitCastExpr <col:9> 'const Texture1D<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Texture1D':'Texture1D<vector<float, 4> >' lvalue Var 'g_t1d' 'Texture1D':'Texture1D<vector<float, 4> >'
      `-ImplicitCastExpr <col:15> 'vector<uint, 1>':'vector<uint, 1>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:15> 'unsigned int' <IntegralCast>
          `-IntegerLiteral <col:15> 'literal int' 0
  */
  f4 += g_sb[0];
  /*verify-ast
    CompoundAssignOperator <col:3, col:15> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:15> 'vector<float, 4>'
      |-ImplicitCastExpr <col:13, col:15> 'vector<float, 4> (*)(vector<uint, 1>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:13, col:15> 'vector<float, 4> (vector<uint, 1>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 1>) const'
      |-ImplicitCastExpr <col:9> 'const StructuredBuffer<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'StructuredBuffer<float4>':'StructuredBuffer<vector<float, 4> >' lvalue Var 'g_sb' 'StructuredBuffer<float4>':'StructuredBuffer<vector<float, 4> >'
      `-ImplicitCastExpr <col:14> 'vector<uint, 1>':'vector<uint, 1>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:14> 'unsigned int' <IntegralCast>
          `-IntegerLiteral <col:14> 'literal int' 0
  */
  return f4;
}

float4 test_vector2_indexing()
{
  int2 offset = { 1, 2 };
  float4 f4 = 0;
  f4 += g_t1da[offset];
  /*verify-ast
    CompoundAssignOperator <col:3, col:22> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:22> 'vector<float, 4>'
      |-ImplicitCastExpr <col:15, col:22> 'vector<float, 4> (*)(vector<uint, 2>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:15, col:22> 'vector<float, 4> (vector<uint, 2>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 2>) const'
      |-ImplicitCastExpr <col:9> 'const Texture1DArray<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Texture1DArray':'Texture1DArray<vector<float, 4> >' lvalue Var 'g_t1da' 'Texture1DArray':'Texture1DArray<vector<float, 4> >'
      `-ImplicitCastExpr <col:16> 'vector<uint, 2>' <HLSLCC_IntegralCast>
        `-ImplicitCastExpr <col:16> 'int2':'vector<int, 2>' <LValueToRValue>
          `-DeclRefExpr <col:16> 'int2':'vector<int, 2>' lvalue Var 'offset' 'int2':'vector<int, 2>'
  */
  f4 += g_t2d[offset];
  /*verify-ast
    CompoundAssignOperator <col:3, col:21> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:21> 'vector<float, 4>'
      |-ImplicitCastExpr <col:14, col:21> 'vector<float, 4> (*)(vector<uint, 2>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:14, col:21> 'vector<float, 4> (vector<uint, 2>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 2>) const'
      |-ImplicitCastExpr <col:9> 'const Texture2D<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Texture2D':'Texture2D<vector<float, 4> >' lvalue Var 'g_t2d' 'Texture2D':'Texture2D<vector<float, 4> >'
      `-ImplicitCastExpr <col:15> 'vector<uint, 2>' <HLSLCC_IntegralCast>
        `-ImplicitCastExpr <col:15> 'int2':'vector<int, 2>' <LValueToRValue>
          `-DeclRefExpr <col:15> 'int2':'vector<int, 2>' lvalue Var 'offset' 'int2':'vector<int, 2>'
  */
  f4 += g_t2dms[offset];
  return f4;
}

float4 test_vector3_indexing()
{
  int3 offset = { 1, 2, 3 };
  float4 f4 = 0;
  f4 += g_t2da[offset];
  /*verify-ast
    CompoundAssignOperator <col:3, col:22> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:22> 'vector<float, 4>'
      |-ImplicitCastExpr <col:15, col:22> 'vector<float, 4> (*)(vector<uint, 3>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:15, col:22> 'vector<float, 4> (vector<uint, 3>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 3>) const'
      |-ImplicitCastExpr <col:9> 'const Texture2DArray<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Texture2DArray':'Texture2DArray<vector<float, 4> >' lvalue Var 'g_t2da' 'Texture2DArray':'Texture2DArray<vector<float, 4> >'
      `-ImplicitCastExpr <col:16> 'vector<uint, 3>' <HLSLCC_IntegralCast>
        `-ImplicitCastExpr <col:16> 'int3':'vector<int, 3>' <LValueToRValue>
          `-DeclRefExpr <col:16> 'int3':'vector<int, 3>' lvalue Var 'offset' 'int3':'vector<int, 3>'
  */
  f4 += g_t2dmsa[offset];
  /*verify-ast
    CompoundAssignOperator <col:3, col:24> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:24> 'vector<float, 4>'
      |-ImplicitCastExpr <col:17, col:24> 'vector<float, 4> (*)(vector<uint, 3>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:17, col:24> 'vector<float, 4> (vector<uint, 3>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 3>) const'
      |-ImplicitCastExpr <col:9> 'const Texture2DMSArray<vector<float, 4>, 8>' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Texture2DMSArray<float4, 8>':'Texture2DMSArray<vector<float, 4>, 8>' lvalue Var 'g_t2dmsa' 'Texture2DMSArray<float4, 8>':'Texture2DMSArray<vector<float, 4>, 8>'
      `-ImplicitCastExpr <col:18> 'vector<uint, 3>' <HLSLCC_IntegralCast>
        `-ImplicitCastExpr <col:18> 'int3':'vector<int, 3>' <LValueToRValue>
          `-DeclRefExpr <col:18> 'int3':'vector<int, 3>' lvalue Var 'offset' 'int3':'vector<int, 3>'
  */
  f4 += g_t3d[offset];
  /*verify-ast
    CompoundAssignOperator <col:3, col:21> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:21> 'vector<float, 4>'
      |-ImplicitCastExpr <col:14, col:21> 'vector<float, 4> (*)(vector<uint, 3>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:14, col:21> 'vector<float, 4> (vector<uint, 3>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 3>) const'
      |-ImplicitCastExpr <col:9> 'const Texture3D<vector<float, 4> >' lvalue <NoOp>
      | `-DeclRefExpr <col:9> 'Texture3D':'Texture3D<vector<float, 4> >' lvalue Var 'g_t3d' 'Texture3D':'Texture3D<vector<float, 4> >'
      `-ImplicitCastExpr <col:15> 'vector<uint, 3>' <HLSLCC_IntegralCast>
        `-ImplicitCastExpr <col:15> 'int3':'vector<int, 3>' <LValueToRValue>
          `-DeclRefExpr <col:15> 'int3':'vector<int, 3>' lvalue Var 'offset' 'int3':'vector<int, 3>'
  */
  return f4;
}

float4 test_mips_indexing()
{
  // .mips[uint mipSlice, uint pos]
  uint offset = 1;
  float4 f4;
  return f4;
}

float4 test_mips_double_indexing()
{
  // .mips[uint mipSlice, uint pos]
  uint mipSlice = 1;
  uint pos = 1;
  uint2 pos2 = { 1, 2 };
  uint3 pos3 = { 1, 2, 3 };
  float4 f4;
  f4 += g_t1d.mips[mipSlice][pos];
  /*verify-ast
    CompoundAssignOperator <col:3, col:33> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:33> 'vector<float, 4>'
      |-ImplicitCastExpr <col:29, col:33> 'vector<float, 4> (*)(vector<uint, 1>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:29, col:33> 'vector<float, 4> (vector<uint, 1>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 1>) const'
      |-ImplicitCastExpr <col:9, col:28> 'const Texture1D<vector<float, 4> >::mips_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:28> 'Texture1D<vector<float, 4> >::mips_slice_type'
      |   |-ImplicitCastExpr <col:19, col:28> 'Texture1D<vector<float, 4> >::mips_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:19, col:28> 'Texture1D<vector<float, 4> >::mips_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture1D<vector<float, 4> >::mips_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:15> 'const Texture1D<vector<float, 4> >::mips_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:15> 'Texture1D<vector<float, 4> >::mips_type' lvalue .mips
      |   |   `-DeclRefExpr <col:9> 'Texture1D':'Texture1D<vector<float, 4> >' lvalue Var 'g_t1d' 'Texture1D':'Texture1D<vector<float, 4> >'
      |   `-DeclRefExpr <col:20> 'uint':'unsigned int' lvalue Var 'mipSlice' 'uint':'unsigned int'
      `-ImplicitCastExpr <col:30> 'vector<uint, 1>':'vector<uint, 1>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:30> 'uint':'unsigned int' <LValueToRValue>
          `-DeclRefExpr <col:30> 'uint':'unsigned int' lvalue Var 'pos' 'uint':'unsigned int'
  */
  f4 += g_t1da.mips[mipSlice][pos2];
  /*verify-ast
    CompoundAssignOperator <col:3, col:35> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:35> 'vector<float, 4>'
      |-ImplicitCastExpr <col:30, col:35> 'vector<float, 4> (*)(vector<uint, 2>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:30, col:35> 'vector<float, 4> (vector<uint, 2>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 2>) const'
      |-ImplicitCastExpr <col:9, col:29> 'const Texture1DArray<vector<float, 4> >::mips_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:29> 'Texture1DArray<vector<float, 4> >::mips_slice_type'
      |   |-ImplicitCastExpr <col:20, col:29> 'Texture1DArray<vector<float, 4> >::mips_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:20, col:29> 'Texture1DArray<vector<float, 4> >::mips_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture1DArray<vector<float, 4> >::mips_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:16> 'const Texture1DArray<vector<float, 4> >::mips_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:16> 'Texture1DArray<vector<float, 4> >::mips_type' lvalue .mips
      |   |   `-DeclRefExpr <col:9> 'Texture1DArray':'Texture1DArray<vector<float, 4> >' lvalue Var 'g_t1da' 'Texture1DArray':'Texture1DArray<vector<float, 4> >'
      |   `-DeclRefExpr <col:21> 'uint':'unsigned int' lvalue Var 'mipSlice' 'uint':'unsigned int'
      `-DeclRefExpr <col:31> 'uint2':'vector<uint, 2>' lvalue Var 'pos2' 'uint2':'vector<uint, 2>'
  */
  f4 += g_t2d.mips[mipSlice][pos2];
  /*verify-ast
    CompoundAssignOperator <col:3, col:34> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:34> 'vector<float, 4>'
      |-ImplicitCastExpr <col:29, col:34> 'vector<float, 4> (*)(vector<uint, 2>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:29, col:34> 'vector<float, 4> (vector<uint, 2>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 2>) const'
      |-ImplicitCastExpr <col:9, col:28> 'const Texture2D<vector<float, 4> >::mips_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:28> 'Texture2D<vector<float, 4> >::mips_slice_type'
      |   |-ImplicitCastExpr <col:19, col:28> 'Texture2D<vector<float, 4> >::mips_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:19, col:28> 'Texture2D<vector<float, 4> >::mips_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture2D<vector<float, 4> >::mips_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:15> 'const Texture2D<vector<float, 4> >::mips_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:15> 'Texture2D<vector<float, 4> >::mips_type' lvalue .mips
      |   |   `-DeclRefExpr <col:9> 'Texture2D':'Texture2D<vector<float, 4> >' lvalue Var 'g_t2d' 'Texture2D':'Texture2D<vector<float, 4> >'
      |   `-DeclRefExpr <col:20> 'uint':'unsigned int' lvalue Var 'mipSlice' 'uint':'unsigned int'
      `-DeclRefExpr <col:30> 'uint2':'vector<uint, 2>' lvalue Var 'pos2' 'uint2':'vector<uint, 2>'
  */
  f4 += g_t2da.mips[mipSlice][pos3];
  /*verify-ast
    CompoundAssignOperator <col:3, col:35> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:35> 'vector<float, 4>'
      |-ImplicitCastExpr <col:30, col:35> 'vector<float, 4> (*)(vector<uint, 3>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:30, col:35> 'vector<float, 4> (vector<uint, 3>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 3>) const'
      |-ImplicitCastExpr <col:9, col:29> 'const Texture2DArray<vector<float, 4> >::mips_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:29> 'Texture2DArray<vector<float, 4> >::mips_slice_type'
      |   |-ImplicitCastExpr <col:20, col:29> 'Texture2DArray<vector<float, 4> >::mips_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:20, col:29> 'Texture2DArray<vector<float, 4> >::mips_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture2DArray<vector<float, 4> >::mips_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:16> 'const Texture2DArray<vector<float, 4> >::mips_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:16> 'Texture2DArray<vector<float, 4> >::mips_type' lvalue .mips
      |   |   `-DeclRefExpr <col:9> 'Texture2DArray':'Texture2DArray<vector<float, 4> >' lvalue Var 'g_t2da' 'Texture2DArray':'Texture2DArray<vector<float, 4> >'
      |   `-DeclRefExpr <col:21> 'uint':'unsigned int' lvalue Var 'mipSlice' 'uint':'unsigned int'
      `-DeclRefExpr <col:31> 'uint3':'vector<uint, 3>' lvalue Var 'pos3' 'uint3':'vector<uint, 3>'
  */
  f4 += g_t3d.mips[mipSlice][pos3];
  /*verify-ast
    CompoundAssignOperator <col:3, col:34> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:34> 'vector<float, 4>'
      |-ImplicitCastExpr <col:29, col:34> 'vector<float, 4> (*)(vector<uint, 3>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:29, col:34> 'vector<float, 4> (vector<uint, 3>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 3>) const'
      |-ImplicitCastExpr <col:9, col:28> 'const Texture3D<vector<float, 4> >::mips_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:28> 'Texture3D<vector<float, 4> >::mips_slice_type'
      |   |-ImplicitCastExpr <col:19, col:28> 'Texture3D<vector<float, 4> >::mips_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:19, col:28> 'Texture3D<vector<float, 4> >::mips_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture3D<vector<float, 4> >::mips_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:15> 'const Texture3D<vector<float, 4> >::mips_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:15> 'Texture3D<vector<float, 4> >::mips_type' lvalue .mips
      |   |   `-DeclRefExpr <col:9> 'Texture3D':'Texture3D<vector<float, 4> >' lvalue Var 'g_t3d' 'Texture3D':'Texture3D<vector<float, 4> >'
      |   `-DeclRefExpr <col:20> 'uint':'unsigned int' lvalue Var 'mipSlice' 'uint':'unsigned int'
      `-DeclRefExpr <col:30> 'uint3':'vector<uint, 3>' lvalue Var 'pos3' 'uint3':'vector<uint, 3>'
  */
  return f4;
}

float4 test_sample_indexing()
{
  // .sample[uint sampleSlice, uint pos]
  uint offset = 1;
  float4 f4;
  return f4;
}

float4 test_sample_double_indexing()
{
  // .sample[uint sampleSlice, uint pos]
  uint sampleSlice = 1;
  uint pos = 1;
  uint2 pos2 = { 1, 2 };
  uint3 pos3 = { 1, 2, 3 };
  float4 f4;
  f4 += g_t2dms.sample[sampleSlice][pos2];
  /*verify-ast
    CompoundAssignOperator <col:3, col:41> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:41> 'vector<float, 4>'
      |-ImplicitCastExpr <col:36, col:41> 'vector<float, 4> (*)(vector<uint, 2>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:36, col:41> 'vector<float, 4> (vector<uint, 2>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 2>) const'
      |-ImplicitCastExpr <col:9, col:35> 'const Texture2DMS<vector<float, 4>, 8>::sample_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:35> 'Texture2DMS<vector<float, 4>, 8>::sample_slice_type'
      |   |-ImplicitCastExpr <col:23, col:35> 'Texture2DMS<vector<float, 4>, 8>::sample_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:23, col:35> 'Texture2DMS<vector<float, 4>, 8>::sample_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture2DMS<vector<float, 4>, 8>::sample_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:17> 'const Texture2DMS<vector<float, 4>, 8>::sample_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:17> 'Texture2DMS<vector<float, 4>, 8>::sample_type' lvalue .sample
      |   |   `-DeclRefExpr <col:9> 'Texture2DMS<float4, 8>':'Texture2DMS<vector<float, 4>, 8>' lvalue Var 'g_t2dms' 'Texture2DMS<float4, 8>':'Texture2DMS<vector<float, 4>, 8>'
      |   `-DeclRefExpr <col:24> 'uint':'unsigned int' lvalue Var 'sampleSlice' 'uint':'unsigned int'
      `-DeclRefExpr <col:37> 'uint2':'vector<uint, 2>' lvalue Var 'pos2' 'uint2':'vector<uint, 2>'
  */
  f4 += g_t2dmsa.sample[sampleSlice][pos3];
  /*verify-ast
    CompoundAssignOperator <col:3, col:42> 'float4':'vector<float, 4>' lvalue '+=' ComputeLHSTy='vector<float, 4>' ComputeResultTy='vector<float, 4>'
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-CXXOperatorCallExpr <col:9, col:42> 'vector<float, 4>'
      |-ImplicitCastExpr <col:37, col:42> 'vector<float, 4> (*)(vector<uint, 3>) const' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:37, col:42> 'vector<float, 4> (vector<uint, 3>) const' lvalue CXXMethod 'operator[]' 'vector<float, 4> (vector<uint, 3>) const'
      |-ImplicitCastExpr <col:9, col:36> 'const Texture2DMSArray<vector<float, 4>, 8>::sample_slice_type' <NoOp>
      | `-CXXOperatorCallExpr <col:9, col:36> 'Texture2DMSArray<vector<float, 4>, 8>::sample_slice_type'
      |   |-ImplicitCastExpr <col:24, col:36> 'Texture2DMSArray<vector<float, 4>, 8>::sample_slice_type (*)(unsigned int) const' <FunctionToPointerDecay>
      |   | `-DeclRefExpr <col:24, col:36> 'Texture2DMSArray<vector<float, 4>, 8>::sample_slice_type (unsigned int) const' lvalue CXXMethod 'operator[]' 'Texture2DMSArray<vector<float, 4>, 8>::sample_slice_type (unsigned int) const'
      |   |-ImplicitCastExpr <col:9, col:18> 'const Texture2DMSArray<vector<float, 4>, 8>::sample_type' lvalue <NoOp>
      |   | `-MemberExpr <col:9, col:18> 'Texture2DMSArray<vector<float, 4>, 8>::sample_type' lvalue .sample
      |   |   `-DeclRefExpr <col:9> 'Texture2DMSArray<float4, 8>':'Texture2DMSArray<vector<float, 4>, 8>' lvalue Var 'g_t2dmsa' 'Texture2DMSArray<float4, 8>':'Texture2DMSArray<vector<float, 4>, 8>'
      |   `-DeclRefExpr <col:25> 'uint':'unsigned int' lvalue Var 'sampleSlice' 'uint':'unsigned int'
      `-DeclRefExpr <col:38> 'uint3':'vector<uint, 3>' lvalue Var 'pos3' 'uint3':'vector<uint, 3>'
  */
  return f4;
}

// Verify subscript access through a const parameter.
struct my_struct { float2 f2; float4 f4; };
float fn_sb(const StructuredBuffer < my_struct > sb) {
  float4 f4 = sb[0].f4; // works fine
  /*verify-ast
    DeclStmt <col:3, col:23>
    `-VarDecl <col:3, col:21> f4 'float4':'vector<float, 4>'
      `-MemberExpr <col:15, col:21> 'float4':'vector<float, 4>' .f4
        `-CXXOperatorCallExpr <col:15, col:19> 'my_struct'
          |-ImplicitCastExpr <col:17, col:19> 'my_struct (*)(vector<uint, 1>) const' <FunctionToPointerDecay>
          | `-DeclRefExpr <col:17, col:19> 'my_struct (vector<uint, 1>) const' lvalue CXXMethod 'operator[]' 'my_struct (vector<uint, 1>) const'
          |-DeclRefExpr <col:15> 'const StructuredBuffer<my_struct>':'const StructuredBuffer<my_struct>' lvalue ParmVar 'sb' 'const StructuredBuffer<my_struct>':'const StructuredBuffer<my_struct>'
          `-ImplicitCastExpr <col:18> 'vector<uint, 1>':'vector<uint, 1>' <HLSLVectorSplat>
            `-ImplicitCastExpr <col:18> 'unsigned int' <IntegralCast>
              `-IntegerLiteral <col:18> 'literal int' 0
  */
  return f4.x;
}

Texture1D t1d;

void my_subscripts()
{
  int i;
  int2 i2;
  int ai2[2];
  int2x2 i22;

  int ai1[1];
  int ai65536[65536];

  i2[0] = 1;
  ai2[0] = 1;
  i22[0][0] = 2;

  // TODO: these warnings are issued for the operator[] overloads defined automatically; it would be better to (a)
  // not emit warnings while considering for overloads, and instead only emit (a single one) when noting why candidates
  // were rejected.
  //
  i22[1.5][0] = 2; // expected-warning {{implicit conversion from 'literal float' to 'unsigned int' changes value from 1.5 to 1}} fxc-pass {{}}
  /*verify-ast
    BinaryOperator <col:3, col:17> 'float':'float' '='
    |-CXXOperatorCallExpr <col:3, col:13> 'float':'float' lvalue
    | |-ImplicitCastExpr <col:11, col:13> 'float &(*)(unsigned int)' <FunctionToPointerDecay>
    | | `-DeclRefExpr <col:11, col:13> 'float &(unsigned int)' lvalue CXXMethod 'operator[]' 'float &(unsigned int)'
    | |-CXXOperatorCallExpr <col:3, col:10> 'vector<float, 2>':'vector<float, 2>' lvalue
    | | |-ImplicitCastExpr <col:6, col:10> 'vector<float, 2> &(*)(unsigned int)' <FunctionToPointerDecay>
    | | | `-DeclRefExpr <col:6, col:10> 'vector<float, 2> &(unsigned int)' lvalue CXXMethod 'operator[]' 'vector<float, 2> &(unsigned int)'
    | | |-DeclRefExpr <col:3> 'int2x2':'matrix<int, 2, 2>' lvalue Var 'i22' 'int2x2':'matrix<int, 2, 2>'
    | | `-ImplicitCastExpr <col:7> 'unsigned int' <FloatingToIntegral>
    | |   `-FloatingLiteral <col:7> 'literal float' 1.500000e+00
    | `-ImplicitCastExpr <col:12> 'unsigned int' <IntegralCast>
    |   `-IntegerLiteral <col:12> 'literal int' 0
    `-ImplicitCastExpr <col:17> 'float':'float' <IntegralToFloating>
      `-IntegerLiteral <col:17> 'literal int' 2
  */
  i22[0] = i2;
  /*verify-ast
    BinaryOperator <col:3, col:12> 'vector<float, 2>':'vector<float, 2>' '='
    |-CXXOperatorCallExpr <col:3, col:8> 'vector<float, 2>':'vector<float, 2>' lvalue
    | |-ImplicitCastExpr <col:6, col:8> 'vector<float, 2> &(*)(unsigned int)' <FunctionToPointerDecay>
    | | `-DeclRefExpr <col:6, col:8> 'vector<float, 2> &(unsigned int)' lvalue CXXMethod 'operator[]' 'vector<float, 2> &(unsigned int)'
    | |-DeclRefExpr <col:3> 'int2x2':'matrix<int, 2, 2>' lvalue Var 'i22' 'int2x2':'matrix<int, 2, 2>'
    | `-ImplicitCastExpr <col:7> 'unsigned int' <IntegralCast>
    |   `-IntegerLiteral <col:7> 'literal int' 0
    `-ImplicitCastExpr <col:12> 'vector<float, 2>' <HLSLCC_IntegralToFloating>
      `-ImplicitCastExpr <col:12> 'int2':'vector<int, 2>' <LValueToRValue>
        `-DeclRefExpr <col:12> 'int2':'vector<int, 2>' lvalue Var 'i2' 'int2':'vector<int, 2>'
  */

  // Floats are fine.
  i2[1.5f] = 1; // expected-warning {{implicit conversion from 'float' to 'unsigned int' changes value from 1.5 to 1}} fxc-pass {{}}
  /*verify-ast
    BinaryOperator <col:3, col:14> 'float':'float' '='
    |-CXXOperatorCallExpr <col:3, col:10> 'float':'float' lvalue
    | |-ImplicitCastExpr <col:5, col:10> 'float &(*)(unsigned int)' <FunctionToPointerDecay>
    | | `-DeclRefExpr <col:5, col:10> 'float &(unsigned int)' lvalue CXXMethod 'operator[]' 'float &(unsigned int)'
    | |-DeclRefExpr <col:3> 'int2':'vector<int, 2>' lvalue Var 'i2' 'int2':'vector<int, 2>'
    | `-ImplicitCastExpr <col:6> 'unsigned int' <FloatingToIntegral>
    |   `-FloatingLiteral <col:6> 'float' 1.500000e+00
    `-ImplicitCastExpr <col:14> 'float':'float' <IntegralToFloating>
      `-IntegerLiteral <col:14> 'literal int' 1
  */
  float fone = 1;
  i2[fone] = 1;
  /*verify-ast
    BinaryOperator <col:3, col:14> 'float':'float' '='
    |-CXXOperatorCallExpr <col:3, col:10> 'float':'float' lvalue
    | |-ImplicitCastExpr <col:5, col:10> 'float &(*)(unsigned int)' <FunctionToPointerDecay>
    | | `-DeclRefExpr <col:5, col:10> 'float &(unsigned int)' lvalue CXXMethod 'operator[]' 'float &(unsigned int)'
    | |-DeclRefExpr <col:3> 'int2':'vector<int, 2>' lvalue Var 'i2' 'int2':'vector<int, 2>'
    | `-ImplicitCastExpr <col:6> 'unsigned int' <FloatingToIntegral>
    |   `-ImplicitCastExpr <col:6> 'float' <LValueToRValue>
    |     `-DeclRefExpr <col:6> 'float' lvalue Var 'fone' 'float'
    `-ImplicitCastExpr <col:14> 'float':'float' <IntegralToFloating>
      `-IntegerLiteral <col:14> 'literal int' 1
  */
}

float4 plain(float4 param4 /* : FOO */) /*: FOO */{
  return 0; //  test_mips_double_indexing();
}

void main() {
}