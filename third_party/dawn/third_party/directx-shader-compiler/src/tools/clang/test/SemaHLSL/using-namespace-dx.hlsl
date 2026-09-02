// RUN: %dxc -T lib_6_9 -ast-dump-implicit %s | FileCheck %s

RaytracingAccelerationStructure Scene : register(t0, space0);

struct[raypayload] RayPayload {
  float4 color : write(caller) : read(closesthit);
};

namespace MyStuff {
  using namespace dx;
  void MaybeReorderThread(int2 V);
}

void MyStuff::MaybeReorderThread(int2 V) {
  MaybeReorderThread(V.x, V.y);
}

[shader("raygeneration")] void MyRaygenShader() {
  // Set the ray's extents.
  RayDesc ray;
  ray.Origin = float3(0, 0, 1);
  ray.Direction = float3(1, 0, 0);
  ray.TMin = 0.001;
  ray.TMax = 10000.0;

  RayPayload payload = {float4(0, 0, 0, 0)};
  
  using namespace dx;
  HitObject hit =
      HitObject::TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0,
                          ray, payload);

  int sortKey = 1;
  MaybeReorderThread(sortKey, 1);

  HitObject::Invoke(hit, payload);

  MyStuff::MaybeReorderThread(int2(sortKey, 1));
}

// Find the DeclRefExpr for the call to MaybeReorderThread:

// CHECK: NamespaceDecl {{.*}} implicit dx
// CHECK: FunctionDecl [[DeclAddr:0x[0-9a-fA-F]+]] <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} CoherenceHint 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} NumCoherenceHintBitsFromLSB 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 359
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.9 0 0 ""

// CHECK: FunctionDecl [[MyDeclAddr:0x[0-9a-fA-F]+]] parent {{.*}} used MaybeReorderThread 'void (int2)'
// CHECK: DeclRefExpr {{.*}} 'void (unsigned int, unsigned int)' lvalue Function [[DeclAddr]] 'MaybeReorderThread' 'void (unsigned int, unsigned int)'

// CHECK-LABEL: MyRaygenShader

// CHECK: DeclRefExpr {{.*}} 'void (unsigned int, unsigned int)' lvalue Function [[DeclAddr:0x[0-9a-fA-F]+]] 'MaybeReorderThread' 'void (unsigned int, unsigned int)'
// CHECK: DeclRefExpr {{.*}} 'void (int2)' lvalue Function [[MyDeclAddr:0x[0-9a-fA-F]+]] 'MaybeReorderThread' 'void (int2)'

