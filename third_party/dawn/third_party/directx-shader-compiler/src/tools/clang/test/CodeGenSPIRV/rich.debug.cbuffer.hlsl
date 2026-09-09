// RUN: %dxc -T vs_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

struct S {
    float  f1;  // Size: 32,  Offset: [ 0 -  32]
    float3 f2;  // Size: 96,  Offset: [32 - 128]
};

cbuffer MyCbuffer : register(b1) {
    bool     a;    // Size:  32, Offset: [  0 -  32]
    int      b;    // Size:  32, Offset: [ 32 -  64]
    uint2    c;    // Size:  64, Offset: [ 64 - 128]
    float3x4 d;    // Size: 512, Offset: [128 - 640]
    S        s;    // Size: 128, Offset: [640 - 768]
    float    t[4]; // Size: 512, Offset: [768 - 1280]
};

cbuffer AnotherCBuffer : register(b2) {
    float3 m;  // Size:  96, Offset: [ 0  -  96]
    float4 n;  // Size: 128, Offset: [128 - 256]
}

// CHECK: [[AnotherCBuffer:%[0-9]+]] = OpExtInst %void [[ext:%[0-9]+]] DebugTypeComposite {{%[0-9]+}} Structure {{%[0-9]+}} 17 9 {{%[0-9]+}} {{%[0-9]+}} %uint_256 FlagIsProtected|FlagIsPrivate [[m:%[0-9]+]] [[n:%[0-9]+]]
// CHECK: [[n]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 19 12 [[AnotherCBuffer]] %uint_128 %uint_128
// CHECK: [[m]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 18 12 [[AnotherCBuffer]] %uint_0 %uint_96

// CHECK: [[S:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite {{%[0-9]+}} Structure {{%[0-9]+}} 3 8 {{%[0-9]+}} {{%[0-9]+}} %uint_128 FlagIsProtected|FlagIsPrivate [[f1:%[0-9]+]] [[f2:%[0-9]+]]
// CHECK: [[f2]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 5 12 [[S]] %uint_32 %uint_96
// CHECK: [[f1]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 4 11 [[S]] %uint_0 %uint_32

// CHECK: [[MyCbuffer:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite {{%[0-9]+}} Structure {{%[0-9]+}} 8 9 {{%[0-9]+}} {{%[0-9]+}} %uint_1280 FlagIsProtected|FlagIsPrivate [[a:%[0-9]+]] [[b:%[0-9]+]] [[c:%[0-9]+]] [[d:%[0-9]+]] [[s:%[0-9]+]] [[t:%[0-9]+]]
// CHECK: [[t]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 14 11 [[MyCbuffer]] %uint_768 %uint_512
// CHECK: [[s]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} [[S]] {{%[0-9]+}} 13 7 [[MyCbuffer]] %uint_640 %uint_128
// CHECK: [[d]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 12 14 [[MyCbuffer]] %uint_128 %uint_512
// CHECK: [[c]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 11 11 [[MyCbuffer]] %uint_64 %uint_64
// CHECK: [[b]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 10 9 [[MyCbuffer]] %uint_32 %uint_32
// CHECK: [[a]] = OpExtInst %void [[ext]] DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 9 10 [[MyCbuffer]] %uint_0 %uint_32

// CHECK: {{%[0-9]+}} = OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 17 9 {{%[0-9]+}} {{%[0-9]+}} %AnotherCBuffer
// CHECK: {{%[0-9]+}} = OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 8 9 {{%[0-9]+}} {{%[0-9]+}} %MyCbuffer

float  main() : A {
  return t[0] + m[0];
}
