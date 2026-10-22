// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

//CHECK: User defined type intrinsic arg must be struct

float main(float THit : t, uint HitKind : h, float2 f2 : F) {
  return ReportHit(THit, HitKind, f2);
}