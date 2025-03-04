// RUN: %dxc -Tlib_6_3  -Wno-unused-value  -verify %s
// RUN: %dxc -Tvs_6_0 -Wno-unused-value -verify %s

float overload1(float f) { return 1; }                        /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
double overload1(double f) { return 2; }                       /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
int overload1(int i) { return 3; }                             /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
uint overload1(uint i) { return 4; }                           /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
min12int overload1(min12int i) { return 5; }                   /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-warning {{'min12int' is promoted to 'min16int'}} expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}} */


static const float2 g_f2_arr[8] =
{
  float2 (-0.1234,  0.4321) / 0.8750,
  float2 ( 0.0000, -0.0012) / 0.8750,
  float2 ( 0.5555,  0.5555) / 0.8750,
  float2 (-0.6666,  0.0000) / 0.8750,
  float2 ( 0.3333, -0.0000) / 0.8750,
  float2 ( 0.0000,  0.3213) / 0.8750,
  float2 (-0.1234, -0.4567) / 0.8750,
  float2 ( 0.1255,  0.0000) / 0.8750,
};

float2 g_f2;
float2 rotation;
float fn_f_f(float r)
{
  float ambiguous1 = overload1(1.5);                           /* expected-error {{call to 'overload1' is ambiguous}} fxc-error {{X3067: 'overload1': ambiguous function call}} */
  float ambiguous2 = overload1(2);                             /* expected-error {{call to 'overload1' is ambiguous}} fxc-error {{X3067: 'overload1': ambiguous function call}} */
  // 1-digit integers go through separate code path, so also check multi-digit:
  float ambiguous3 = overload1(123);                           /* expected-error {{call to 'overload1' is ambiguous}} fxc-error {{X3067: 'overload1': ambiguous function call}} */
  float ambiguous4 = overload1(1.5 * 2);                       /* expected-error {{call to 'overload1' is ambiguous}} fxc-error {{X3067: 'overload1': ambiguous function call}} */
  float ambiguous5 = overload1(1.5 * 10);                      /* expected-error {{call to 'overload1' is ambiguous}} fxc-error {{X3067: 'overload1': ambiguous function call}} */

  int tap = 0;
  float2x2 rotationMatrix = { rotation.x, rotation.y, -rotation.y, rotation.x };
  /*verify-ast
    DeclStmt <col:3, col:80>
    `-VarDecl <col:3, col:79> col:12 used rotationMatrix 'float2x2':'matrix<float, 2, 2>' cinit
      `-InitListExpr <col:29, col:79> 'float2x2':'matrix<float, 2, 2>'
        |-ImplicitCastExpr <col:31, col:40> 'float' <LValueToRValue>
        | `-HLSLVectorElementExpr <col:31, col:40> 'const float' lvalue vectorcomponent x
        |   `-DeclRefExpr <col:31> 'const float2':'const vector<float, 2>' lvalue Var 'rotation' 'const float2':'const vector<float, 2>'
        |-ImplicitCastExpr <col:43, col:52> 'float' <LValueToRValue>
        | `-HLSLVectorElementExpr <col:43, col:52> 'const float' lvalue vectorcomponent y
        |   `-DeclRefExpr <col:43> 'const float2':'const vector<float, 2>' lvalue Var 'rotation' 'const float2':'const vector<float, 2>'
        |-UnaryOperator <col:55, col:65> 'float' prefix '-'
        | `-ImplicitCastExpr <col:56, col:65> 'float' <LValueToRValue>
        |   `-HLSLVectorElementExpr <col:56, col:65> 'const float' lvalue vectorcomponent y
        |     `-DeclRefExpr <col:56> 'const float2':'const vector<float, 2>' lvalue Var 'rotation' 'const float2':'const vector<float, 2>'
        `-ImplicitCastExpr <col:68, col:77> 'float' <LValueToRValue>
          `-HLSLVectorElementExpr <col:68, col:77> 'const float' lvalue vectorcomponent x
            `-DeclRefExpr <col:68> 'const float2':'const vector<float, 2>' lvalue Var 'rotation' 'const float2':'const vector<float, 2>'
  */
  float2 offs = mul(g_f2_arr[tap], rotationMatrix) * r;
  /*verify-ast
    DeclStmt <col:3, col:55>
    `-VarDecl <col:3, col:54> col:10 used offs 'float2':'vector<float, 2>' cinit
      `-BinaryOperator <col:17, col:54> 'vector<float, 2>':'vector<float, 2>' '*'
        |-CallExpr <col:17, col:50> 'vector<float, 2>':'vector<float, 2>'
        | |-ImplicitCastExpr <col:17> 'vector<float, 2> (*)(vector<float, 2>, matrix<float, 2, 2>)' <FunctionToPointerDecay>
        | | `-DeclRefExpr <col:17> 'vector<float, 2> (vector<float, 2>, matrix<float, 2, 2>)' lvalue Function 'mul' 'vector<float, 2> (vector<float, 2>, matrix<float, 2, 2>)'
        | |-ImplicitCastExpr <col:21, col:33> 'float2':'vector<float, 2>' <LValueToRValue>
        | | `-ArraySubscriptExpr <col:21, col:33> 'float2':'vector<float, 2>' lvalue
        | |   |-ImplicitCastExpr <col:21> 'const float2 [8]' <LValueToRValue>
        | |   | `-DeclRefExpr <col:21> 'const float2 [8]' lvalue Var 'g_f2_arr' 'const float2 [8]'
        | |   `-ImplicitCastExpr <col:30> 'int' <LValueToRValue>
        | |     `-DeclRefExpr <col:30> 'int' lvalue Var 'tap' 'int'
        | `-ImplicitCastExpr <col:36> 'float2x2':'matrix<float, 2, 2>' <LValueToRValue>
        |   `-DeclRefExpr <col:36> 'float2x2':'matrix<float, 2, 2>' lvalue Var 'rotationMatrix' 'float2x2':'matrix<float, 2, 2>'
        `-ImplicitCastExpr <col:54> 'vector<float, 2>':'vector<float, 2>' <HLSLVectorSplat>
          `-ImplicitCastExpr <col:54> 'float' <LValueToRValue>
            `-DeclRefExpr <col:54> 'float' lvalue ParmVar 'r' 'float'
  */
  return offs.x;
}

uint fn_f3_f3io_u(float3 wn, inout float3 tsn)
{
  uint  e3 = 0;
  float d1 = (wn.x + wn.y + wn.z) * 0.5;
  /*verify-ast
    DeclStmt <col:3, col:40>
    `-VarDecl <col:3, col:37> col:9 used d1 'float' cinit
      `-BinaryOperator <col:14, col:37> 'float' '*'
        |-ParenExpr <col:14, col:33> 'float'
        | `-BinaryOperator <col:15, col:32> 'float' '+'
        |   |-BinaryOperator <col:15, col:25> 'float' '+'
        |   | |-ImplicitCastExpr <col:15, col:18> 'float' <LValueToRValue>
        |   | | `-HLSLVectorElementExpr <col:15, col:18> 'float' lvalue vectorcomponent x
        |   | |   `-DeclRefExpr <col:15> 'float3':'vector<float, 3>' lvalue ParmVar 'wn' 'float3':'vector<float, 3>'
        |   | `-ImplicitCastExpr <col:22, col:25> 'float' <LValueToRValue>
        |   |   `-HLSLVectorElementExpr <col:22, col:25> 'float' lvalue vectorcomponent y
        |   |     `-DeclRefExpr <col:22> 'float3':'vector<float, 3>' lvalue ParmVar 'wn' 'float3':'vector<float, 3>'
        |   `-ImplicitCastExpr <col:29, col:32> 'float' <LValueToRValue>
        |     `-HLSLVectorElementExpr <col:29, col:32> 'float' lvalue vectorcomponent z
        |       `-DeclRefExpr <col:29> 'float3':'vector<float, 3>' lvalue ParmVar 'wn' 'float3':'vector<float, 3>'
        `-ImplicitCastExpr <col:37> 'float' <FloatingCast>
          `-FloatingLiteral <col:37> 'literal float' 5.000000e-01
  */
  float d2 = wn.x - d1;
  float d3 = wn.y - d1;
  float d4 = wn.z - d1;
  float dm = max(max(d1, d2), max(d3, d4));

  float3 nn = tsn;
  if (d2 == dm) { e3 = 1; nn *= float3 (1, -1, -1); dm += 2; }
  if (d3 == dm) { e3 = 2; nn *= float3 (-1, 1, -1); dm += 2; }
  if (d4 == dm) { e3 = 3; nn *= float3 (-1, -1, 1); }

  tsn.z = nn.x + nn.y + nn.z;
  tsn.y = nn.z - nn.x;
  tsn.x = tsn.z - 3 * nn.y;

  const float sqrt_2 = 1.414213562373f;
  const float sqrt_3 = 1.732050807569f;
  const float sqrt_6 = 2.449489742783f;

  tsn *= float3 (1.0 / sqrt_6, 1.0 / sqrt_2, 1.0 / sqrt_3);
  /*verify-ast
    CompoundAssignOperator <col:3, col:58> 'float3':'vector<float, 3>' lvalue '*=' ComputeLHSTy='float3':'vector<float, 3>' ComputeResultTy='float3':'vector<float, 3>'
    |-DeclRefExpr <col:3> 'float3':'vector<float, 3>' lvalue ParmVar 'tsn' 'float3 &__restrict'
    `-CXXFunctionalCastExpr <col:10, col:58> 'float3':'vector<float, 3>' functional cast to float3 <NoOp>
      `-InitListExpr <col:17, col:58> 'float3':'vector<float, 3>'
        |-BinaryOperator <col:18, col:24> 'float' '/'
        | |-ImplicitCastExpr <col:18> 'float' <FloatingCast>
        | | `-FloatingLiteral <col:18> 'literal float' 1.000000e+00
        | `-ImplicitCastExpr <col:24> 'float' <LValueToRValue>
        |   `-DeclRefExpr <col:24> 'const float' lvalue Var 'sqrt_6' 'const float'
        |-BinaryOperator <col:32, col:38> 'float' '/'
        | |-ImplicitCastExpr <col:32> 'float' <FloatingCast>
        | | `-FloatingLiteral <col:32> 'literal float' 1.000000e+00
        | `-ImplicitCastExpr <col:38> 'float' <LValueToRValue>
        |   `-DeclRefExpr <col:38> 'const float' lvalue Var 'sqrt_2' 'const float'
        `-BinaryOperator <col:46, col:52> 'float' '/'
          |-ImplicitCastExpr <col:46> 'float' <FloatingCast>
          | `-FloatingLiteral <col:46> 'literal float' 1.000000e+00
          `-ImplicitCastExpr <col:52> 'float' <LValueToRValue>
            `-DeclRefExpr <col:52> 'const float' lvalue Var 'sqrt_3' 'const float'
  */

  return e3;
}

//////////////////////////////////////////////////////////////////////////////
// Constant evaluation.
void fn_const_eval() {
  const float f_one = 1;
  const double d_zero = 0;
  const uint u = 3;
  const int i = 4;
  const int i_neg_4 = -4;
  uint u_var = 10;

  // Operators:
  float f_ice_plus[+u];
  float f_ice_minus[-i_neg_4];
  float f_ice_add[u + 2];
  float f_ice_sub[u - 2];
  float f_ice_mul[u * 2];
  float f_ice_div[u / 2];
  float f_ice_rem[u % 2];
  float f_ice_shl[u << 1];
  float f_ice_shr[u >> 1];
  float f_ice_ternary[u > 1 ? 3 : 4];                          /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_lt[u < 1 ? 3 : 4];                               /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_le[u <= 1 ? 3 : 4];                              /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_gt[u > 1 ? 3 : 4];                               /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_ge[u >= 1 ? 3 : 4];                              /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_eq[u == 1 ? 3 : 4];                              /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_ne[u != 1 ? 3 : 4];                              /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_bin_and[u & 0x1];
  float f_ice_bin_or[u | 0x1];
  float f_ice_bin_xor[u ^ 0x1];
  float f_ice_log_and[u && true ? 3 : 4];                      /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float f_ice_log_or[u || false ? 3 : 4];                      /* fxc-error {{X3058: array dimensions must be literal scalar expressions}} */

  // This fails, as it should, but the error recovery isn't very good
  // and the error messages could be improved.
  float f_ice_assign[u_var = 3]; // expected-error {{expected ']'}} expected-error {{variable length arrays are not supported in HLSL}} expected-note {{to match this '['}} fxc-error {{X3058: array dimensions must be literal scalar expressions}}

  // Data types:
  // Check with all primitive types.
  // Check with vectors and matrices.

  // Precision modes:
  // Check with IEEE strict or relaxed.

  // Intrinsics:
  // Based on the intrinsic table. Intrinsics on objects aren't supported for constant folding.
//    (UINT)hlsl::IntrinsicOp::IOP_AddUint64, 3, g_Intrinsics_Args0,
//NO  (UINT)hlsl::IntrinsicOp::IOP_AllMemoryBarrier, 1, g_Intrinsics_Args1,
//NO  (UINT)hlsl::IntrinsicOp::IOP_AllMemoryBarrierWithGroupSync, 1, g_Intrinsics_Args2,
//NO  (UINT)hlsl::IntrinsicOp::IOP_CheckAccessFullyMapped, 2, g_Intrinsics_Args3,
//    (UINT)hlsl::IntrinsicOp::IOP_D3DCOLORtoUBYTE4, 2, g_Intrinsics_Args4,
//NO  (UINT)hlsl::IntrinsicOp::IOP_DeviceMemoryBarrier, 1, g_Intrinsics_Args5,
//NO  (UINT)hlsl::IntrinsicOp::IOP_DeviceMemoryBarrierWithGroupSync, 1, g_Intrinsics_Args6,
//NO  (UINT)hlsl::IntrinsicOp::IOP_EvaluateAttributeAtSample, 3, g_Intrinsics_Args7,
//NO  (UINT)hlsl::IntrinsicOp::IOP_EvaluateAttributeCentroid, 2, g_Intrinsics_Args8,
//NO  (UINT)hlsl::IntrinsicOp::IOP_EvaluateAttributeSnapped, 3, g_Intrinsics_Args9,
//NO  (UINT)hlsl::IntrinsicOp::IOP_GetRenderTargetSampleCount, 1, g_Intrinsics_Args10,
//NO  (UINT)hlsl::IntrinsicOp::IOP_GetRenderTargetSamplePosition, 2, g_Intrinsics_Args11,
//NO  (UINT)hlsl::IntrinsicOp::IOP_GroupMemoryBarrier, 1, g_Intrinsics_Args12,
//NO  (UINT)hlsl::IntrinsicOp::IOP_GroupMemoryBarrierWithGroupSync, 1, g_Intrinsics_Args13,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedAdd, 3, g_Intrinsics_Args14,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedAdd, 4, g_Intrinsics_Args15,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedAnd, 3, g_Intrinsics_Args16,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedAnd, 4, g_Intrinsics_Args17,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedCompareExchange, 5, g_Intrinsics_Args18,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedCompareStore, 4, g_Intrinsics_Args19,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedExchange, 4, g_Intrinsics_Args20,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedMax, 3, g_Intrinsics_Args21,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedMax, 4, g_Intrinsics_Args22,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedMin, 3, g_Intrinsics_Args23,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedMin, 4, g_Intrinsics_Args24,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedOr, 3, g_Intrinsics_Args25,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedOr, 4, g_Intrinsics_Args26,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedXor, 3, g_Intrinsics_Args27,
//NO  (UINT)hlsl::IntrinsicOp::IOP_InterlockedXor, 4, g_Intrinsics_Args28,
//NO  (UINT)hlsl::IntrinsicOp::IOP_NonUniformResourceIndex, 2, g_Intrinsics_Args29,
//NO  (UINT)hlsl::IntrinsicOp::IOP_Process2DQuadTessFactorsAvg, 6, g_Intrinsics_Args30,
//NO  (UINT)hlsl::IntrinsicOp::IOP_Process2DQuadTessFactorsMax, 6, g_Intrinsics_Args31,
//NO  (UINT)hlsl::IntrinsicOp::IOP_Process2DQuadTessFactorsMin, 6, g_Intrinsics_Args32,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessIsolineTessFactors, 5, g_Intrinsics_Args33,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessQuadTessFactorsAvg, 6, g_Intrinsics_Args34,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessQuadTessFactorsMax, 6, g_Intrinsics_Args35,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessQuadTessFactorsMin, 6, g_Intrinsics_Args36,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessTriTessFactorsAvg, 6, g_Intrinsics_Args37,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessTriTessFactorsMax, 6, g_Intrinsics_Args38,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ProcessTriTessFactorsMin, 6, g_Intrinsics_Args39,
//NO  (UINT)hlsl::IntrinsicOp::IOP_abort, 1, g_Intrinsics_Args40,
//    (UINT)hlsl::IntrinsicOp::IOP_abs, 2, g_Intrinsics_Args41,
//    (UINT)hlsl::IntrinsicOp::IOP_acos, 2, g_Intrinsics_Args42,
//    (UINT)hlsl::IntrinsicOp::IOP_all, 2, g_Intrinsics_Args43,
//    (UINT)hlsl::IntrinsicOp::IOP_any, 2, g_Intrinsics_Args44,
//    (UINT)hlsl::IntrinsicOp::IOP_asdouble, 3, g_Intrinsics_Args45,
//    (UINT)hlsl::IntrinsicOp::IOP_asfloat, 2, g_Intrinsics_Args46,
//    (UINT)hlsl::IntrinsicOp::IOP_asin, 2, g_Intrinsics_Args47,
//    (UINT)hlsl::IntrinsicOp::IOP_asint, 2, g_Intrinsics_Args48,
//    (UINT)hlsl::IntrinsicOp::IOP_asuint, 4, g_Intrinsics_Args49,
//    (UINT)hlsl::IntrinsicOp::IOP_asuint, 2, g_Intrinsics_Args50,
//    (UINT)hlsl::IntrinsicOp::IOP_atan, 2, g_Intrinsics_Args51,
//    (UINT)hlsl::IntrinsicOp::IOP_atan2, 3, g_Intrinsics_Args52,
//    (UINT)hlsl::IntrinsicOp::IOP_ceil, 2, g_Intrinsics_Args53,
//    (UINT)hlsl::IntrinsicOp::IOP_clamp, 4, g_Intrinsics_Args54,
//NO  (UINT)hlsl::IntrinsicOp::IOP_clip, 2, g_Intrinsics_Args55,
//    (UINT)hlsl::IntrinsicOp::IOP_cos, 2, g_Intrinsics_Args56,
//    (UINT)hlsl::IntrinsicOp::IOP_cosh, 2, g_Intrinsics_Args57,
//    (UINT)hlsl::IntrinsicOp::IOP_countbits, 2, g_Intrinsics_Args58,
//    (UINT)hlsl::IntrinsicOp::IOP_cross, 3, g_Intrinsics_Args59,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ddx, 2, g_Intrinsics_Args60,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ddx_coarse, 2, g_Intrinsics_Args61,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ddx_fine, 2, g_Intrinsics_Args62,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ddy, 2, g_Intrinsics_Args63,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ddy_coarse, 2, g_Intrinsics_Args64,
//NO  (UINT)hlsl::IntrinsicOp::IOP_ddy_fine, 2, g_Intrinsics_Args65,
//    (UINT)hlsl::IntrinsicOp::IOP_degrees, 2, g_Intrinsics_Args66,
//    (UINT)hlsl::IntrinsicOp::IOP_determinant, 2, g_Intrinsics_Args67,
//    (UINT)hlsl::IntrinsicOp::IOP_distance, 3, g_Intrinsics_Args68,
//    (UINT)hlsl::IntrinsicOp::IOP_dot, 3, g_Intrinsics_Args69,
//    (UINT)hlsl::IntrinsicOp::IOP_dst, 3, g_Intrinsics_Args70,
//    (UINT)hlsl::IntrinsicOp::IOP_exp, 2, g_Intrinsics_Args71,
//    (UINT)hlsl::IntrinsicOp::IOP_exp2, 2, g_Intrinsics_Args72,
//    (UINT)hlsl::IntrinsicOp::IOP_f16tof32, 2, g_Intrinsics_Args73,
//    (UINT)hlsl::IntrinsicOp::IOP_f32tof16, 2, g_Intrinsics_Args74,
//NO  (UINT)hlsl::IntrinsicOp::IOP_faceforward, 4, g_Intrinsics_Args75,
//    (UINT)hlsl::IntrinsicOp::IOP_firstbithigh, 2, g_Intrinsics_Args76,
//    (UINT)hlsl::IntrinsicOp::IOP_firstbitlow, 2, g_Intrinsics_Args77,
//    (UINT)hlsl::IntrinsicOp::IOP_floor, 2, g_Intrinsics_Args78,
//    (UINT)hlsl::IntrinsicOp::IOP_fma, 4, g_Intrinsics_Args79,
//    (UINT)hlsl::IntrinsicOp::IOP_fmod, 3, g_Intrinsics_Args80,
//    (UINT)hlsl::IntrinsicOp::IOP_frac, 2, g_Intrinsics_Args81,
//    (UINT)hlsl::IntrinsicOp::IOP_frexp, 3, g_Intrinsics_Args82,
//    (UINT)hlsl::IntrinsicOp::IOP_fwidth, 2, g_Intrinsics_Args83,
//    (UINT)hlsl::IntrinsicOp::IOP_isfinite, 2, g_Intrinsics_Args84,
//    (UINT)hlsl::IntrinsicOp::IOP_isinf, 2, g_Intrinsics_Args85,
//    (UINT)hlsl::IntrinsicOp::IOP_isnan, 2, g_Intrinsics_Args86,
//    (UINT)hlsl::IntrinsicOp::IOP_ldexp, 3, g_Intrinsics_Args87,
//    (UINT)hlsl::IntrinsicOp::IOP_length, 2, g_Intrinsics_Args88,
//    (UINT)hlsl::IntrinsicOp::IOP_lerp, 4, g_Intrinsics_Args89,
//    (UINT)hlsl::IntrinsicOp::IOP_lit, 4, g_Intrinsics_Args90,
//    (UINT)hlsl::IntrinsicOp::IOP_log, 2, g_Intrinsics_Args91,
//    (UINT)hlsl::IntrinsicOp::IOP_log10, 2, g_Intrinsics_Args92,
//    (UINT)hlsl::IntrinsicOp::IOP_log2, 2, g_Intrinsics_Args93,
//    (UINT)hlsl::IntrinsicOp::IOP_mad, 4, g_Intrinsics_Args94,
//    (UINT)hlsl::IntrinsicOp::IOP_max, 3, g_Intrinsics_Args95,
  double intrin_max = max(f_one, d_zero);
  // The following assertion will fail because static assert expects an ICE,
  // but the intrin_max subexpression isn't one (even if the binary comparsion
  // expression is).
  // _Static_assert(intrin_max == 2, "expected 2");
  _Static_assert(max(1, 2) == 2, "expected 2");             /* fxc-error {{X3004: undeclared identifier '_Static_assert'}} */
  _Static_assert(max(1, -2) == 1, "expected 1");            /* fxc-error {{X3004: undeclared identifier '_Static_assert'}} */
  //    (UINT)hlsl::IntrinsicOp::IOP_min, 3, g_Intrinsics_Args96,
//    (UINT)hlsl::IntrinsicOp::IOP_modf, 3, g_Intrinsics_Args97,
//    (UINT)hlsl::IntrinsicOp::IOP_msad4, 4, g_Intrinsics_Args98,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args99,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args100,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args101,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args102,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args103,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args104,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args105,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args106,
//    (UINT)hlsl::IntrinsicOp::IOP_mul, 3, g_Intrinsics_Args107,
//NO  (UINT)hlsl::IntrinsicOp::IOP_noise, 2, g_Intrinsics_Args108,
//    (UINT)hlsl::IntrinsicOp::IOP_normalize, 2, g_Intrinsics_Args109,
//    (UINT)hlsl::IntrinsicOp::IOP_pow, 3, g_Intrinsics_Args110,
//    (UINT)hlsl::IntrinsicOp::IOP_radians, 2, g_Intrinsics_Args111,
//    (UINT)hlsl::IntrinsicOp::IOP_rcp, 2, g_Intrinsics_Args112,
//    (UINT)hlsl::IntrinsicOp::IOP_reflect, 3, g_Intrinsics_Args113,
//    (UINT)hlsl::IntrinsicOp::IOP_refract, 4, g_Intrinsics_Args114,
//    (UINT)hlsl::IntrinsicOp::IOP_reversebits, 2, g_Intrinsics_Args115,
//    (UINT)hlsl::IntrinsicOp::IOP_round, 2, g_Intrinsics_Args116,
//    (UINT)hlsl::IntrinsicOp::IOP_rsqrt, 2, g_Intrinsics_Args117,
//    (UINT)hlsl::IntrinsicOp::IOP_saturate, 2, g_Intrinsics_Args118,
//    (UINT)hlsl::IntrinsicOp::IOP_sign, 2, g_Intrinsics_Args119,
//    (UINT)hlsl::IntrinsicOp::IOP_sin, 2, g_Intrinsics_Args120,
//    (UINT)hlsl::IntrinsicOp::IOP_sincos, 4, g_Intrinsics_Args121,
//    (UINT)hlsl::IntrinsicOp::IOP_sinh, 2, g_Intrinsics_Args122,
//    (UINT)hlsl::IntrinsicOp::IOP_smoothstep, 4, g_Intrinsics_Args123,
//??  (UINT)hlsl::IntrinsicOp::IOP_source_mark, 1, g_Intrinsics_Args124,
//    (UINT)hlsl::IntrinsicOp::IOP_sqrt, 2, g_Intrinsics_Args125,
//    (UINT)hlsl::IntrinsicOp::IOP_step, 3, g_Intrinsics_Args126,
//    (UINT)hlsl::IntrinsicOp::IOP_tan, 2, g_Intrinsics_Args127,
//    (UINT)hlsl::IntrinsicOp::IOP_tanh, 2, g_Intrinsics_Args128,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex1D, 3, g_Intrinsics_Args129,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex1D, 5, g_Intrinsics_Args130,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex1Dbias, 3, g_Intrinsics_Args131,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex1Dgrad, 5, g_Intrinsics_Args132,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex1Dlod, 3, g_Intrinsics_Args133,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex1Dproj, 3, g_Intrinsics_Args134,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex2D, 3, g_Intrinsics_Args135,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex2D, 5, g_Intrinsics_Args136,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex2Dbias, 3, g_Intrinsics_Args137,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex2Dgrad, 5, g_Intrinsics_Args138,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex2Dlod, 3, g_Intrinsics_Args139,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex2Dproj, 3, g_Intrinsics_Args140,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex3D, 3, g_Intrinsics_Args141,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex3D, 5, g_Intrinsics_Args142,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex3Dbias, 3, g_Intrinsics_Args143,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex3Dgrad, 5, g_Intrinsics_Args144,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex3Dlod, 3, g_Intrinsics_Args145,
//NO  (UINT)hlsl::IntrinsicOp::IOP_tex3Dproj, 3, g_Intrinsics_Args146,
//NO  (UINT)hlsl::IntrinsicOp::IOP_texCUBE, 3, g_Intrinsics_Args147,
//NO  (UINT)hlsl::IntrinsicOp::IOP_texCUBE, 5, g_Intrinsics_Args148,
//NO  (UINT)hlsl::IntrinsicOp::IOP_texCUBEbias, 3, g_Intrinsics_Args149,
//NO  (UINT)hlsl::IntrinsicOp::IOP_texCUBEgrad, 5, g_Intrinsics_Args150,
//NO  (UINT)hlsl::IntrinsicOp::IOP_texCUBElod, 3, g_Intrinsics_Args151,
//NO  (UINT)hlsl::IntrinsicOp::IOP_texCUBEproj, 3, g_Intrinsics_Args152,
//    (UINT)hlsl::IntrinsicOp::IOP_transpose, 2, g_Intrinsics_Args153,
//    (UINT)hlsl::IntrinsicOp::IOP_trunc, 2, g_Intrinsics_Args154,
}

//////////////////////////////////////////////////////////////////////////////
// ICE.
void fn_ice() {
  // Scalar ints.
  static uint s_One = 1;
  static uint s_Two = uint(s_One) + 1;
  float arr_s_One[s_One];    /* expected-error {{variable length arrays are not supported in HLSL}} fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float arr_s_Two[s_Two];    /* expected-error {{variable length arrays are not supported in HLSL}} fxc-error {{X3058: array dimensions must be literal scalar expressions}} */

  static const uint sc_One = 1;
  static const uint sc_Two = uint(sc_One) + 1;
  float arr_sc_One[sc_One];
  float arr_sc_Two[sc_Two];

  // Vector ints.
  static uint1 v_One = 1;
  static uint1 v_Two = uint1(v_One) + 1;
  float arr_v_One[v_One];    /* expected-error {{size of array has non-integer type 'uint1'}} fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float arr_v_Two[v_Two];    /* expected-error {{size of array has non-integer type 'uint1'}} fxc-error {{X3058: array dimensions must be literal scalar expressions}} */

  static const uint1 vc_One = 1;
  static const uint1 vc_Two = uint1(vc_One) + 1;
  float arr_vc_One[vc_One];  /* expected-error {{size of array has non-integer type 'uint1'}} fxc-error {{X3058: array dimensions must be literal scalar expressions}} */
  float arr_vc_Two[vc_Two];  /* expected-error {{size of array has non-integer type 'uint1'}} fxc-error {{X3058: array dimensions must be literal scalar expressions}} */

  // Note: here dxc is different from fxc, where a const integral vector can be used in ICE.
  // It would be desirable to have this supported.
  float arr_vc_One[vc_One.x];  /* expected-error {{variable length arrays are not supported in HLSL}} fxc-pass {{}} */
  float arr_vc_Two[vc_Two.x];  /* expected-error {{variable length arrays are not supported in HLSL}} fxc-pass {{}} */
}

[shader("vertex")]
void main() {
}
