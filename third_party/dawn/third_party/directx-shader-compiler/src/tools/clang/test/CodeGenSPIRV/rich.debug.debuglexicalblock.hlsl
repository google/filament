// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[debugSet:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:   [[debugSource:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugSource
// CHECK:          [[main:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugFunction
// CHECK: [[mainFnLexBlock:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugLexicalBlock [[debugSource]] 13 1 [[main]]
// CHECK: [[whileLoopLexBlock:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugLexicalBlock [[debugSource]] 21 3 [[mainFnLexBlock]]
// CHECK: [[ifStmtLexBlock:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugLexicalBlock [[debugSource]] 26 20 [[whileLoopLexBlock]]
// CHECK: [[tempLexBlock:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugLexicalBlock [[debugSource]] 28 7 [[ifStmtLexBlock]]
// CHECK: [[forLoopLexBlock:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugLexicalBlock [[debugSource]] 15 12 [[mainFnLexBlock]]

float4 main(float4 color : COLOR) : SV_TARGET
{
  float4 c = 0.xxxx;
  for (;;) {
    float4 a = 0.xxxx;
    float4 b = 1.xxxx;
    c = c + a + b;
  }
  while (c.x)
  {
    float4 a = 0.xxxx;
    float4 b = 1.xxxx;
    c = c + a + b;

    if (bool(c.x)) {
      c = c + c;
      {
        c = c + c;
      }
    }
  }

  return color + c;
}

