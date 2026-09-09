// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Note: Even though the HLSL documentation contains a version of "firstbitlow" that 
// takes signed integer(s) and returns signed integer(s), the frontend always generates
// the AST using the overloaded version that takes unsigned integer(s) and returns
// unsigned integer(s).

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  int   sint_1;
  int4  sint_4;
  uint  uint_1;
  uint4 uint_4;

// CHECK: {{%[0-9]+}} = OpExtInst %uint [[glsl]] FindILsb {{%[0-9]+}}
  int fbl =  firstbitlow(sint_1);

// CHECK: {{%[0-9]+}} = OpExtInst %v4uint [[glsl]] FindILsb {{%[0-9]+}}
  int4 fbl4 =  firstbitlow(sint_4);

// CHECK: {{%[0-9]+}} = OpExtInst %uint [[glsl]] FindILsb {{%[0-9]+}}
  uint ufbl =  firstbitlow(uint_1);

// CHECK: {{%[0-9]+}} = OpExtInst %v4uint [[glsl]] FindILsb {{%[0-9]+}}
  uint4 ufbl4 =  firstbitlow(uint_4);
}
