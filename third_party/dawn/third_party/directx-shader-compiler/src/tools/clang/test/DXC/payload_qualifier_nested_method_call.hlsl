// REQUIRES: dxil-1-7
// RUN: %dxc -T lib_6_7 %s -enable-payload-qualifiers | FileCheck %s

// Regression test for two crashes in the payload-access analysis triggered by
// calling a member method under -enable-payload-qualifiers:
//  - a method on a nested payload field used to be mistaken for a field
//    access, crashing in cast<FieldDecl>() with "llvm::cast<X>() argument of
//    incompatible type!"
//  - a method on the payload itself has no payload argument to bind, which
//    used to leave DxrShaderDiagnoseInfo::Payload uninitialized and fault.
// Compiling to a complete module is the actual check here.
// CHECK: !dx.entryPoints

struct Export {
  uint x;
  float hitT;
  uint getType() { return x >> 24; }
};

struct [raypayload] P {
  Export e : write(caller, closesthit) : read(caller, closesthit);
  uint tag : write(caller, closesthit) : read(caller, closesthit);

  // A method on the payload itself: the analysis has no explicit payload
  // argument to bind here, so it must skip the call rather than crash.
  uint getTag() { return tag; }
};

RaytracingAccelerationStructure asScene : register(t0);
RWBuffer<float> outBuf : register(u0);

[shader("closesthit")]
void chs(inout P p, in BuiltInTriangleIntersectionAttributes a) {
  // Method call on a nested payload field.
  p.e.x = p.e.getType();
  // Method call on the payload itself.
  p.tag = p.getTag() + 1;
}

[shader("raygeneration")]
void rgs() {
  P p = (P)0;
  RayDesc r = (RayDesc)0;
  TraceRay(asScene, 0, 0xFF, 0, 0, 0, r, p);
  outBuf[0] = p.e.hitT + p.tag;
}

