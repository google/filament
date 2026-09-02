// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: @main

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
#ifdef VERIFY_FXC
#endif

// All parameters that come after a parameter with a default initializer must specify a default initializer.
void fn_params_complete(int a = 0, int b = 0);
void fn_params_last(int a, int b = 0);

// Look at the 'mul' issues.
void fn_mul_test() {
  //
  // mul is a particularly interesting intrinsic, because it's
  // overloaded on the input types and the result type varies
  // significantly depending on the shape of inputs.
  //
  float f = 0;
  float2 f2 = { 1, 2 };
  float4 f4 = { 1, 2, 3, 4 };
  float3x4 f34 = { f4, f4, f4 }; // 3 rows, 4 columns
  float4x2 f42 = { f4, f4 }; // 4 rows, 2 columns
  float4x4 f44 = { f4, f4, f4, f4 };
}

float fn_float_arr(float arr[2]) { // 	expected-note {{candidate function not viable: no known conversion from 'float2' to 'float [2]' for 1st argument}} fxc-pass {{}}
  arr[0] = 123;
  return arr[0];
}

float fn_float_arr_out(out float arr[2]) {
  arr[0] = 1;
  arr[1] = 2;
  return 1;
}

typedef float float_arr_2[2];

#if FIXED_ARRAYS_ON_RETURN
float_arr_2 arr_return() {
  float_arr_2 arr;
  arr[0] = 0; arr[1] = 1;
  return arr;
}
#endif

bool fn_bool() {
  float a [ 2 ] ; 
  a [ 0 ] = 1 - 2 ; 
  a [ 1 ] = 2 + 3 ; 
  float b1 , b2 ; 
  b1 = min(a[0], a[1]);
  b2 = max(a[0], a[1]);
  return ( a [ 1 ] < b1 || b2 < a [ 0 ] ) ; 
}

struct Texture2DArrayFloat {
  Texture2DArray<float> tex;
  SamplerState smp;
  
  float SampleLevel(float2 coord, float arrayIndex, float level)
  {
    return tex.SampleLevel(smp, float3 (coord, arrayIndex), level);
  }

  float4 Gather(float2 coord, float arrayIndex)
  {
    return tex.Gather(smp, float3 (coord, arrayIndex));
  }
};

float SampleLevelFromConst(const Texture2DArray < float > tex, SamplerState smp, float2 coord, float arrayIndex, float level) {
  return tex.SampleLevel(smp, float3 (coord, arrayIndex), level);
}

int g_arr[3];
bool fn_unroll_early() {
  [unroll] // expected-warning {{attribute 'unroll' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-pass {{}}
  int aj;
  for (aj = 0; aj < 3; aj++) {
    if (g_arr[aj]) return true;
  }
  return false;
}

float4 component_fun(uint input) {
  // The interesting case here is the second statement, where a float3 (by swizzle) and a scalar
  // yield a bool3, go through a ternary operator, and yield a float3.
  float4 output = float4 (((input >> 0) & 0xFF) / 255.f, ((input >> 8) & 0xFF) / 255.f, ((input >> 16) & 0xFF) / 255.f, ((input >> 24) & 0xFF) / 255.f);
  // TODO: Fix ternary operator (currently conditional is scalar bool)
  output.rgb = (output.rgb <= 0.04045f) ? output.rgb / 12.92f : pow((output.rgb + 0.055f) / 1.055f, 2.4f);
  /*verify-ast
    BinaryOperator <col:3, col:105> 'vector<float, 3>':'vector<float, 3>' '='
    |-HLSLVectorElementExpr <col:3, col:10> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent rgb
    | `-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'output' 'float4':'vector<float, 4>'
    `-ConditionalOperator <col:16, col:105> 'vector<float, 3>'
      |-ParenExpr <col:16, col:39> 'const bool'
      | `-BinaryOperator <col:17, col:31> 'const bool' '<='
      |   |-ImplicitCastExpr <col:17, col:24> 'vector<float, 3>':'vector<float, 3>' <LValueToRValue>
      |   | `-HLSLVectorElementExpr <col:17, col:24> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent rgb
      |   |   `-DeclRefExpr <col:17> 'float4':'vector<float, 4>' lvalue Var 'output' 'float4':'vector<float, 4>'
      |   `-ImplicitCastExpr <col:31> 'vector<float, 3>':'vector<float, 3>' <HLSLVectorSplat>
      |     `-FloatingLiteral <col:31> 'float' 4.045000e-02
      |-BinaryOperator <col:43, col:56> 'vector<float, 3>' '/'
      | |-ImplicitCastExpr <col:43, col:50> 'vector<float, 3>':'vector<float, 3>' <LValueToRValue>
      | | `-HLSLVectorElementExpr <col:43, col:50> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent rgb
      | |   `-DeclRefExpr <col:43> 'float4':'vector<float, 4>' lvalue Var 'output' 'float4':'vector<float, 4>'
      | `-ImplicitCastExpr <col:56> 'vector<float, 3>':'vector<float, 3>' <HLSLVectorSplat>
      |   `-FloatingLiteral <col:56> 'float' 1.292000e+01
      `-CallExpr <col:65, col:105> 'vector<float, 3>':'vector<float, 3>'
        |-ImplicitCastExpr <col:65> 'vector<float, 3> (*)(const vector<float, 3> &, const vector<float, 3> &)' <FunctionToPointerDecay>
        | `-DeclRefExpr <col:65> 'vector<float, 3> (const vector<float, 3> &, const vector<float, 3> &)' lvalue Function 'pow' 'vector<float, 3> (const vector<float, 3> &, const vector<float, 3> &)'
        |-BinaryOperator <col:69, col:93> 'vector<float, 3>' '/'
        | |-ParenExpr <col:69, col:89> 'vector<float, 3>'
        | | `-BinaryOperator <col:70, col:83> 'vector<float, 3>' '+'
        | |   |-ImplicitCastExpr <col:70, col:77> 'vector<float, 3>':'vector<float, 3>' <LValueToRValue>
        | |   | `-HLSLVectorElementExpr <col:70, col:77> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent rgb
        | |   |   `-DeclRefExpr <col:70> 'float4':'vector<float, 4>' lvalue Var 'output' 'float4':'vector<float, 4>'
        | |   `-ImplicitCastExpr <col:83> 'vector<float, 3>':'vector<float, 3>' <HLSLVectorSplat>
        | |     `-FloatingLiteral <col:83> 'float' 5.500000e-02
        | `-ImplicitCastExpr <col:93> 'vector<float, 3>':'vector<float, 3>' <HLSLVectorSplat>
        |   `-FloatingLiteral <col:93> 'float' 1.055000e+00
        `-ImplicitCastExpr <col:101> 'vector<float, 3>':'vector<float, 3>' <HLSLVectorSplat>
          `-FloatingLiteral <col:101> 'float' 2.400000e+00
  */
  return output;
}

float4 fn_cf2u(const float4 p1, uint p2) {
  return float4(1, 2, 3, 4);
}

float3 fn_cf3u(const float3 p1, uint p2) {
  return fn_cf2u(float4 (p1, 0.f), p2).xyz;
}

int call_int_oload(int i) { return i + 5; }     // expected-note {{candidate function}} fxc-pass {{}}
int call_int_oload(int2 i) { return i.y + 5; }  // expected-note {{candidate function}} fxc-pass {{}}

inline float minimum(float a, float b) { if (a > b) return b; return a; }
inline float2 minimum(const float2 a, const float2 b) {
  float2 result;
  for (int i = 0; i < 2; ++i)   {
    result[i] = a[i];
    if (result[i] > b[i])
      result[i] = b[i];
  }
  return result;
}
inline uint3 minimum(const uint3 a, const uint3 b) {
  uint3 result;
  for (int i = 0; i < 3; ++i) {
    result[i] = a[i];
    if (result[i] > b[i])
      result[i] = b[i];
  }
  return result;
}

void tryMinimums() {
  float3 tMin = { 1, 2, 3 };
  float3 tMax = { 3, 2, 1 };
  tMin = minimum(tMin, tMax);
}
void tryMinimumsWithOut(out float3 tMin, out float3 tMax) {
  tMin = minimum(tMin, tMax);
}

void fn_indexer_write(RWTexture2D < float4 > rw) {
  uint2 coord = { 1, 2 };
  rw[coord] = float4(1, 2, 3, 4);
}


[numthreads(8,8,1)]
void main() {
  float2 f2 = float2(1, 2);
  float arr2[2] = { 1, 2 };
  //fn_float_arr(arr2);
  
  f2 = arr2[0];
  f2 = arr2[arr2[0]];

  float3 result = { 0, 0, 0 };
  result.x += call_int_oload(int2(1, 2));
  result.x += call_int_oload(30);

  float3 tMin, tMax;
  tryMinimums();
  tryMinimumsWithOut(tMin, tMax);

  fn_float_arr_out(arr2);
  // arr2 = arr_return();
}
