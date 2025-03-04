// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: call void @dx.op.callShader.struct.Parameter(i32 159

struct Parameter {
   float2 t;
   int3 t2;
};

float4 emit(uint shader, inout Parameter p )  {
  CallShader(shader, p);

   return 2.6;
}