// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

struct S {
    float  a;
    float3 b;
    int2 c;
};

struct T {
    float  a;
    float3 b;
    S      c;
    int2   d;
};

StructuredBuffer<S> mySBuffer1 : register(t1);
StructuredBuffer<T> mySBuffer2 : register(t2);

RWStructuredBuffer<S> mySBuffer3 : register(u3);
RWStructuredBuffer<T> mySBuffer4 : register(u4);

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK: [[T:%[0-9]+]] = OpString "T"
// CHECK: [[S:%[0-9]+]] = OpString "S"
// CHECK: [[RW_S:%[0-9]+]] = OpString "@type.RWStructuredBuffer.S"
// CHECK: [[param:%[0-9]+]] = OpString "TemplateParam"
// CHECK: [[inputBuffer:%[0-9]+]] = OpString "inputBuffer"
// CHECK: [[RW_T:%[0-9]+]] = OpString "@type.RWStructuredBuffer.T"
// CHECK: [[SB_T:%[0-9]+]] = OpString "@type.StructuredBuffer.T"
// CHECK: [[SB_S:%[0-9]+]] = OpString "@type.StructuredBuffer.S"

// CHECK: [[T_ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[T]] Structure
// CHECK: [[S_ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[S]] Structure

// CHECK: [[none:%[0-9]+]] = OpExtInst %void [[set]] DebugInfoNone
// CHECK: [[RW_S_ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[RW_S]] Class {{%[0-9]+}} 0 0 {{%[0-9]+}} {{%[0-9]+}} [[none]]
// CHECK: [[param2:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplateParameter [[param]] [[S_ty]]
// CHECK: [[RW_S_temp:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplate [[RW_S_ty]] [[param2]]

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[inputBuffer]] [[RW_S_temp]]

// CHECK: [[RW_T_ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[RW_T]] Class {{%[0-9]+}} 0 0 {{%[0-9]+}} {{%[0-9]+}} [[none]]
// CHECK: [[param3:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplateParameter [[param]] [[T_ty]]
// CHECK: [[RW_T_temp:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplate [[RW_T_ty]] [[param3]]

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugGlobalVariable {{%[0-9]+}} [[RW_T_temp]] {{%[0-9]+}} {{[0-9]+}} {{[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %mySBuffer4
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugGlobalVariable {{%[0-9]+}} [[RW_S_temp]] {{%[0-9]+}} {{[0-9]+}} {{[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %mySBuffer3

// CHECK: [[SB_T_ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[SB_T]] Class {{%[0-9]+}} 0 0 {{%[0-9]+}} {{%[0-9]+}} [[none]]
// CHECK: [[param1:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplateParameter [[param]] [[T_ty]]
// CHECK: [[SB_T_temp:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplate [[SB_T_ty]] [[param1]]

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugGlobalVariable {{%[0-9]+}} [[SB_T_temp]] {{%[0-9]+}} {{[0-9]+}} {{[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %mySBuffer2

// CHECK: [[SB_S_ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[SB_S]] Class {{%[0-9]+}} 0 0 {{%[0-9]+}} {{%[0-9]+}} [[none]]
// CHECK: [[param0:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplateParameter [[param]] [[S_ty]]
// CHECK: [[SB_S_temp:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeTemplate [[SB_S_ty]] [[param0]]

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugGlobalVariable {{%[0-9]+}} [[SB_S_temp]] {{%[0-9]+}} {{[0-9]+}} {{[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %mySBuffer1

void foo(RWStructuredBuffer<S> inputBuffer) {
  inputBuffer[0].a = 0;
}

float4 main() : SV_Target {
    foo(mySBuffer3);
    return 1.0;
}
