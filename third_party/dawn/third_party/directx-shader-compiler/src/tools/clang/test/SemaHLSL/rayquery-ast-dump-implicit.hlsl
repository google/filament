// RUN: %dxc -T vs_6_9 -E main -ast-dump-implicit %s | FileCheck %s

float main(RayDesc rayDesc : RAYDESC) : OUT {  
  return 0;
}

// CHECK: VarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit RAY_FLAG_FORCE_OMM_2_STATE 'const unsigned int' static cinit
// CHECK: IntegerLiteral 0x{{.+}} <<invalid sloc>> 'const unsigned int' 1024
// CHECK: AvailabilityAttr 0x{{.+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// CHECK: VarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS 'const unsigned int' static cinit
// CHECK: IntegerLiteral 0x{{.+}} <<invalid sloc>> 'const unsigned int' 1
// CHECK: AvailabilityAttr 0x{{.+}} <<invalid sloc>> Implicit  6.9 0 0 ""

