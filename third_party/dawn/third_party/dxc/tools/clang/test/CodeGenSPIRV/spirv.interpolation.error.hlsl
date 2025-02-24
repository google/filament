// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct PSInput {
  linear        int  int_b: INTB;
  centroid      int1 int_c: INTC;
  noperspective int2 int_e: INTE;
  sample        int3 int_f: INTF;

  linear        uint  uint_b: UINTB;
  centroid      uint1 uint_c: UINTC;
  noperspective uint2 uint_e: UINTE;
  sample        uint3 uint_f: UINTF;

  linear        bool  bool_b: BOOLB;
  centroid      bool1 bool_c: BOOLC;
  noperspective bool2 bool_e: BOOLE;
  sample        bool3 bool_f: BOOLF;
};

// CHECK: :4:22: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :5:22: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :6:22: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :7:22: error: only nointerpolation mode allowed for integer input parameters in pixel shader

// CHECK: :9:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :10:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :11:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :12:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader

// CHECK: :14:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :15:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :16:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader
// CHECK: :17:23: error: only nointerpolation mode allowed for integer input parameters in pixel shader

float4 main(PSInput input) : SV_Target {
  return 1.0;
}

