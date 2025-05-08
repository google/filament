// RUN: %dxc -T ps_6_0 -HV 2021 -DTYPE=bool %s | FileCheck %s --check-prefixes=CHECK,I32
// RUN: %dxc -T ps_6_0 -HV 2018 -DTYPE=bool %s | FileCheck %s --check-prefixes=CHECK,I32
// RUN: %dxc -T ps_6_0 -HV 2021 -DTYPE=int %s | FileCheck %s --check-prefixes=CHECK,I32
// RUN: %dxc -T ps_6_0 -HV 2018 -DTYPE=int %s | FileCheck %s --check-prefixes=CHECK,I32
// RUN: %dxc -T ps_6_0 -HV 2021 -DTYPE=float %s | FileCheck %s --check-prefixes=CHECK,F32
// RUN: %dxc -T ps_6_0 -HV 2018 -DTYPE=float %s | FileCheck %s --check-prefixes=CHECK,F32

// I32: %dx.types.ResRet.[[TY:i32]] = type { [[TYPE:i32]]
// F32: %dx.types.ResRet.[[TY:f32]] = type { [[TYPE:float]]

// CHECK-LABEL: define void @main

ByteAddressBuffer buf;

float4 main() : SV_Target {

  // CHECK-DAG: [[SAR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 0
  // CHECK-DAG: [[SAX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[SAR]], 0
  // I32-DAG: [[SA:%.*]] = icmp ne i32 [[SAX]], 0
  // F32-DAG: [[SA:%.*]] = fcmp fast une float [[SAX]], 0.000000e+00

  // CHECK-DAG: [[SBR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 8
  // CHECK-DAG: [[SBX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[SBR]], 0
  // I32-DAG: [[SB:%.*]] = icmp ne i32 [[SBX]], 0
  // F32-DAG: [[SB:%.*]] = fcmp fast une float [[SBX]], 0.000000e+00

  TYPE sb = buf.Load<TYPE>(8);
  TYPE sa = buf.Load<TYPE>(0);

  // CHECK: and i1 [[SB]], [[SA]]
  TYPE res = and(sa, sb);

  // CHECK-DAG: [[V1AR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 16
  // CHECK-DAG: [[V1AX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V1AR]], 0
  // I32-DAG: [[V1A:%.*]] = icmp ne i32 [[V1AX]], 0
  // F32-DAG: [[V1A:%.*]] = fcmp fast une float [[V1AX]], 0.000000e+00

  // CHECK-DAG: [[V1BR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 24
  // CHECK-DAG: [[V1BX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V1BR]], 0
  // I32-DAG: [[V1B:%.*]] = icmp ne i32 [[V1BX]], 0
  // F32-DAG: [[V1B:%.*]] = fcmp fast une float [[V1BX]], 0.000000e+00

  vector<TYPE, 1> v1b = buf.Load< vector<TYPE, 1> >(24);
  vector<TYPE, 1> v1a = buf.Load< vector<TYPE, 1> >(16);

  // CHECK: and i1 [[V1B]], [[V1A]]
  vector<TYPE, 1> res1 = and(v1a, v1b);

  // CHECK-DAG: [[V3AR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 32
  // CHECK-DAG: [[V3AX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 0
  // CHECK-DAG: [[V3AX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 1
  // CHECK-DAG: [[V3AX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 2

  // I32-DAG: [[V3A0:%.*]] = icmp ne i32 [[V3AX0]], 0
  // I32-DAG: [[V3A1:%.*]] = icmp ne i32 [[V3AX1]], 0
  // I32-DAG: [[V3A2:%.*]] = icmp ne i32 [[V3AX2]], 0

  // F32-DAG: [[V3A0:%.*]] = fcmp fast une float [[V3AX0]], 0.000000e+00
  // F32-DAG: [[V3A1:%.*]] = fcmp fast une float [[V3AX1]], 0.000000e+00
  // F32-DAG: [[V3A2:%.*]] = fcmp fast une float [[V3AX2]], 0.000000e+00

  // CHECK-DAG: [[V3BR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 56
  // CHECK-DAG: [[V3BX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 0
  // CHECK-DAG: [[V3BX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 1
  // CHECK-DAG: [[V3BX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 2

  // I32-DAG: [[V3B0:%.*]] = icmp ne i32 [[V3BX0]], 0
  // I32-DAG: [[V3B1:%.*]] = icmp ne i32 [[V3BX1]], 0
  // I32-DAG: [[V3B2:%.*]] = icmp ne i32 [[V3BX2]], 0

  // F32-DAG: [[V3B0:%.*]] = fcmp fast une float [[V3BX0]], 0.000000e+00
  // F32-DAG: [[V3B1:%.*]] = fcmp fast une float [[V3BX1]], 0.000000e+00
  // F32-DAG: [[V3B2:%.*]] = fcmp fast une float [[V3BX2]], 0.000000e+00

  vector<TYPE, 3> v3b = buf.Load< vector<TYPE, 3> >(56);
  vector<TYPE, 3> v3a = buf.Load< vector<TYPE, 3> >(32);

  // CHECK: and i1 [[V3B0]], [[V3A0]]
  // CHECK: and i1 [[V3B1]], [[V3A1]]
  // CHECK: and i1 [[V3B2]], [[V3A2]]
  vector<TYPE, 3> res3 = and(v3a, v3b);

  // CHECK-DAG: [[MAR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 80
  // CHECK-DAG: [[MAX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 0
  // CHECK-DAG: [[MAX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 1
  // CHECK-DAG: [[MAX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 2
  // CHECK-DAG: [[MAX3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 3
  // CHECK-DAG: [[MAR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 96
  // CHECK-DAG: [[MAX4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 0
  // CHECK-DAG: [[MAX5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 1

  // I32-DAG: [[MA0:%.*]] = icmp ne i32 [[MAX0]], 0
  // I32-DAG: [[MA1:%.*]] = icmp ne i32 [[MAX1]], 0
  // I32-DAG: [[MA2:%.*]] = icmp ne i32 [[MAX2]], 0
  // I32-DAG: [[MA3:%.*]] = icmp ne i32 [[MAX3]], 0
  // I32-DAG: [[MA4:%.*]] = icmp ne i32 [[MAX4]], 0
  // I32-DAG: [[MA5:%.*]] = icmp ne i32 [[MAX5]], 0

  // F32-DAG: [[MA0:%.*]] = fcmp fast une float [[MAX0]], 0.000000e+00
  // F32-DAG: [[MA1:%.*]] = fcmp fast une float [[MAX1]], 0.000000e+00
  // F32-DAG: [[MA2:%.*]] = fcmp fast une float [[MAX2]], 0.000000e+00
  // F32-DAG: [[MA3:%.*]] = fcmp fast une float [[MAX3]], 0.000000e+00
  // F32-DAG: [[MA4:%.*]] = fcmp fast une float [[MAX4]], 0.000000e+00
  // F32-DAG: [[MA5:%.*]] = fcmp fast une float [[MAX5]], 0.000000e+00

  // CHECK-DAG: [[MBR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 128
  // CHECK-DAG: [[MBX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 0
  // CHECK-DAG: [[MBX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 1
  // CHECK-DAG: [[MBX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 2
  // CHECK-DAG: [[MBX3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 3
  // CHECK-DAG: [[MBR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 144
  // CHECK-DAG: [[MBX4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 0
  // CHECK-DAG: [[MBX5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 1

  // I32-DAG: [[MB0:%.*]] = icmp ne i32 [[MBX0]], 0
  // I32-DAG: [[MB1:%.*]] = icmp ne i32 [[MBX1]], 0
  // I32-DAG: [[MB2:%.*]] = icmp ne i32 [[MBX2]], 0
  // I32-DAG: [[MB3:%.*]] = icmp ne i32 [[MBX3]], 0
  // I32-DAG: [[MB4:%.*]] = icmp ne i32 [[MBX4]], 0
  // I32-DAG: [[MB5:%.*]] = icmp ne i32 [[MBX5]], 0

  // F32-DAG: [[MB0:%.*]] = fcmp fast une float [[MBX0]], 0.000000e+00
  // F32-DAG: [[MB1:%.*]] = fcmp fast une float [[MBX1]], 0.000000e+00
  // F32-DAG: [[MB2:%.*]] = fcmp fast une float [[MBX2]], 0.000000e+00
  // F32-DAG: [[MB3:%.*]] = fcmp fast une float [[MBX3]], 0.000000e+00
  // F32-DAG: [[MB4:%.*]] = fcmp fast une float [[MBX4]], 0.000000e+00
  // F32-DAG: [[MB5:%.*]] = fcmp fast une float [[MBX5]], 0.000000e+00

  matrix<TYPE, 2, 3> matb = buf.Load< matrix<TYPE, 2, 3> >(128);
  matrix<TYPE, 2, 3> mata = buf.Load< matrix<TYPE, 2, 3> >(80);

  // CHECK: and i1 [[MB0]], [[MA0]]
  // CHECK: and i1 [[MB1]], [[MA1]]
  // CHECK: and i1 [[MB2]], [[MA2]]
  // CHECK: and i1 [[MB3]], [[MA3]]
  // CHECK: and i1 [[MB4]], [[MA4]]
  // CHECK: and i1 [[MB5]], [[MA5]]
  matrix<TYPE, 2, 3> resmat = and(mata, matb);

  return float4(res3 + resmat[0] + resmat[1], res + res1.x);
}
