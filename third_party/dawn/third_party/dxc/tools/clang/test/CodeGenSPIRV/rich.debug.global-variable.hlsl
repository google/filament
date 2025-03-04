// RUN: %dxc -T ps_6_2 -E main -fspv-debug=rich -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK:         [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK: [[varNameCond:%[0-9]+]] = OpString "cond"
// CHECK:    [[varNameC:%[0-9]+]] = OpString "c"

// CHECK: [[floatType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Float
// CHECK: [[float4Type:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[floatType]] 4
// CHECK:      [[source:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: [[compileUnit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit 1 4 [[source]] HLSL
// CHECK:  [[boolType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Boolean

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugGlobalVariable [[varNameCond]] [[boolType]] [[source]] 17 13 [[compileUnit]] [[varNameCond]] %cond FlagIsDefinition
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugGlobalVariable [[varNameC]] [[float4Type]] [[source]] 16 15 [[compileUnit]] [[varNameC]] %c FlagIsDefinition

static float4 c;
static bool cond;

float4 main(float4 color : COLOR) : SV_TARGET {
  c = 0.xxxx;
  cond = c.x == 0;

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
