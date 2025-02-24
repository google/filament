// RUN: %dxc -T ps_6_2 -E main -fspv-debug=rich -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK:         [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:    [[varNameB:%[0-9]+]] = OpString "b"
// CHECK:    [[varNameA:%[0-9]+]] = OpString "a"
// CHECK: [[varNameCond:%[0-9]+]] = OpString "cond"
// CHECK:    [[varNameC:%[0-9]+]] = OpString "c"

// CHECK:  [[uintType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Unsigned
// CHECK: [[floatType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Float
// CHECK: [[float4Type:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[floatType]] 4
// CHECK:    [[source:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: [[compileUnit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit 1 4 [[source]] HLSL
// CHECK:      [[main:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction

// CHECK: [[mainFnLexBlock:%[0-9]+]] = OpExtInst %void [[set]] DebugLexicalBlock [[source]] 32 1 [[main]]
// CHECK: [[ifLexBlock:%[0-9]+]] = OpExtInst %void [[set]] DebugLexicalBlock [[source]] 37 13 [[mainFnLexBlock]]
// CHECK: [[tempLexBlock:%[0-9]+]] = OpExtInst %void [[set]] DebugLexicalBlock [[source]] 40 5 [[ifLexBlock]]

// CHECK:              {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[varNameB]] [[uintType]] [[source]] 41 12 [[tempLexBlock]] FlagIsLocal

// CHECK:   [[intType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Signed
// CHECK:            {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[varNameA]] [[intType]] [[source]] 38 9 [[ifLexBlock]] FlagIsLocal

// CHECK:  [[boolType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Boolean
// CHECK:                {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[varNameCond]] [[boolType]] [[source]] 34 8 [[mainFnLexBlock]] FlagIsLocal

// CHECK:                {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[varNameC]] [[float4Type]] [[source]] 33 10 [[mainFnLexBlock]] FlagIsLocal


float4 main(float4 color : COLOR) : SV_TARGET
{
  float4 c = 0.xxxx;
  bool cond = c.x == 0;


  if (cond) {
    int a = 2;
    c = c + c;
    {
      uint b = 3;
      c = c + c;
    }
  }


  return color + c;
}


