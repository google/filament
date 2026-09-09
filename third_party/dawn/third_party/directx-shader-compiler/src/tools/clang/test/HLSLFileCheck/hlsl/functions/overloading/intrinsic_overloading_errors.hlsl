// RUN: %dxc -T lib_6_4 %s | FileCheck %s

// This tests intrinsic overloads using parameter combinations that should fail, but didn't
// Uses intrinsics with *_NOCAST parameters in various ways that should be prohibited

//CHECK: error: no matching function for call to 'fma'
//CHECK: note: candidate function not viable: no known conversion from 'float' to 'double' for 3rd argument
// Verifies that $type parameters in intrinsic parameters doesn't allow them to be cast when they shouldn't
export
double MismatchedIntrins(double d1, double d2, float f) {
  return fma(d1, d2, f);
}

// Another example of the above as cited in #2693. Not fixed with the above due to arg count

//CHECK: error: no matching function for call to 'asfloat16'
//CHECK: note: candidate function not viable: no known conversion from 'uint' to 'half' for 1st argument
RWByteAddressBuffer buf;
void asfloat_16()
{
    buf.Store(0, asfloat16(uint(1)));
}

// Check that a valid intrinsic call doesn't allow a subsequent invalid call to pass
//CHECK: error: no matching function for call to 'fma'
//CHECK: note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument
export
double InvalidAfterValidIntrins(double d1, double d2, double d3,
                                float f1, float f2, float f3) {
  // This is valid and would let the next, invalid call slip through
  double ret = fma(d1, d2, d3);
  ret += fma(f1, f2, f3);
  return ret;
}

// Taken from issue #818. Used to produce ambiguous call errors even though the overloads are invalid
// CHECK: error: no matching function for call to 'asfloat'
// CHECK: note: candidate function not viable: no known conversion from 'const vector<double, 4>' to 'vector<float, 4>' for 1st argument
// CHECK: error: no matching function for call to 'asfloat'
// CHECK: note: candidate function not viable: no known conversion from 'const vector<int64_t, 4>' to 'vector<float, 4>' for 1st argument
// CHECK: error: no matching function for call to 'asfloat'
// CHECK: note: candidate function not viable: no known conversion from 'const vector<uint64_t, 4>' to 'vector<float, 4>' for 1st argument
float4 g_f;
int4 g_i;
uint4 g_u;
double4 g_f64;
int64_t4 g_i64;
uint64_t4 g_u64;

float4 AsFloatOverloads() {
  float4 f1 = 0;
  f1 += asfloat(g_f);
  f1 += asfloat(g_i);
  f1 += asfloat(g_u);
  f1 += asfloat(g_f64);
  f1 += asfloat(g_i64);
  f1 += asfloat(g_u64);
  return 1;
}

// Test using a struct somewhere it shouldn't be
// CHECK: error: no matching function for call to 'abs'
// CHECK: note: candidate function not viable: no known conversion from 'Struct' to 'float' for 1st argument
struct Struct { int x; };
export
int StructOverload()
{
  Struct s = { -1 };
  return abs(s);
}
// Test using a array somewhere it shouldn't be
// CHECK: error: no matching function for call to 'sign'
// CHECK: note: candidate function not viable: no known conversion from 'float [3]' to 'float' for 1st argument
export
int ArrayOverload()
{
  float arr[] = {1, -2, -3};
  return sign(arr);
}

// CHECK: error: no matching function for call to 'saturate'
// Test using a string somewhere it really shouldn't be
// CHECK: note: candidate function not viable: no known conversion from 'literal string' to 'float' for 1st argument
export
int StringOverload()
{
  return saturate("sponge");
}

// Test using a too short vector on an explicit intrinsic arg
// CHECK: error: no matching function for call to 'cross'
// CHECK: note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<float, 3>' for 2nd argument
export
float3 TooShortCast()
{
  float3 hot = {4,5,1};
  float2 bun = {0,0};
  return cross(hot, bun);
}

// Test const on an out param
// CHECK: error: no matching function for call to 'frexp'
// CHECK: note: candidate function not viable: 2nd argument ('const float') would lose const qualifier
export
float ConstCast()
{
  float pi = 3.1415926535897932;
  const float mmmm = 0;
  return frexp(pi, mmmm);
}
#if 0
// Tests for #2507, not yet fixed.
// To fix 2507, we need to change how we retrieve intrinsics.
// Currently, they are searched linearly with number of args being the first rejection criteria
// This doesn't allow detecting usage of one that matches by name, but differs by number

// Test instrinsic with too few arguments
export
float TooFewArgs()
{
  return atan2(0.0);
}

// Test instrinsic with too many arguments
export
float TooManyArgs()
{
  return atan(1.1, 2.2);
}
#endif
