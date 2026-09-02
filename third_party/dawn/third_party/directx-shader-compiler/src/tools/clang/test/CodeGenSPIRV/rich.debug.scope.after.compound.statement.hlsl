// RUN: %dxc -T vs_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

struct VS_OUTPUT {
  float4 pos : SV_POSITION;
};

//CHECK: [[fn:%[0-9]+]] = OpExtInst %void [[ext:%[0-9]+]] DebugFunction
//CHECK: [[bb0:%[0-9]+]] = OpExtInst %void [[ext]] DebugLexicalBlock {{%[0-9]+}} 14 {{[0-9]+}} [[fn]]
//CHECK: [[bb1:%[0-9]+]] = OpExtInst %void [[ext]] DebugLexicalBlock {{%[0-9]+}} 21 {{[0-9]+}} [[bb0]]
//CHECK: [[bb2:%[0-9]+]] = OpExtInst %void [[ext]] DebugLexicalBlock {{%[0-9]+}} 27 {{[0-9]+}} [[bb1]]
//CHECK: [[a:%[0-9]+]] = OpExtInst %void [[ext]] DebugLocalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 32 {{[0-9]+}} [[bb2]]

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
//CHECK: OpLabel
//CHECK: DebugScope [[bb0]]
  float a = 1.0;
  float b = 2.0;
  float c = 3.0;
  float x = a + b + c;
  {
//CHECK:      DebugScope [[bb1]]
//CHECK-NEXT: OpLine [[file:%[0-9]+]] 24
    float a = 3.0;
    float b = 4.0;
    x += a + b + c;
    {
//CHECK:      DebugScope [[bb2]]
//CHECK-NEXT: OpLine [[file_0:%[0-9]+]] 32
//CHECK-NEXT: OpStore [[var_a:%[a-zA-Z0-9_]+]] %float_6
//CHECK:      DebugDeclare [[a]] [[var_a]]
      float a = 6.0;
      x += a + b + c;
    }
//CHECK:      DebugScope [[bb1]]
//CHECK-NEXT: OpLine [[file_1:%[0-9]+]] 37
    x += a + b + c;
  }
//CHECK:      DebugScope [[bb0]]
//CHECK-NEXT: OpLine [[file_2:%[0-9]+]] 41
  x += a + b + c;

  VS_OUTPUT vout;
  vout.pos = pos;
  vout.pos.w += x * 0.000001;
  return vout;
}
