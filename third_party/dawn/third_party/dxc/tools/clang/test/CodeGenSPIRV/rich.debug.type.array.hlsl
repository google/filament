// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK:       [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:  [[boolName:%[0-9]+]] = OpString "bool"
// CHECK:     [[SName:%[0-9]+]] = OpString "S"
// CHECK:   [[intName:%[0-9]+]] = OpString "int"
// CHECK:  [[uintName:%[0-9]+]] = OpString "uint"
// CHECK: [[floatName:%[0-9]+]] = OpString "float"
// CHECK:           %uint_32 = OpConstant %uint 32

// CHECK:   [[bool:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[boolName]] %uint_32 Boolean
// CHECK: [[S:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[SName]]
// CHECK:   [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[intName]] %uint_32 Signed
// CHECK:  [[uint:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[uintName]] %uint_32 Unsigned
// CHECK: [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[floatName]] %uint_32 Float
// CHECK:        {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeArray [[S]] %uint_8
// CHECK: [[boolv4:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[bool]] 4
// CHECK:        {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeArray [[boolv4]] %uint_7
// CHECK:       {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeArray [[float]] %uint_8 %uint_4
// CHECK:       {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeArray [[int]] %uint_8
// CHECK:       {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeArray [[uint]] %uint_4

void main() {
    const uint size = 4 * 3 - 4;

    uint  x[4];
    int   y[size];
    float z[size][4];
    bool4 v[7];

    struct S {
      uint a;
      int b;
      bool c;
    } w[size];
}
