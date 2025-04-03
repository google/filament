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

  // CHECK: [[SAR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 0
  // F32: [[SAX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[SAR]], 0
  // I32: [[SA:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[SAR]], 0

  // CHECK: [[SBR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 8
  // F32: [[SBX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[SBR]], 0
  // I32: [[SB:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[SBR]], 0

  // F32: [[SA:%.*]] = fcmp fast une float [[SAX]], 0.000000e+00
  // F32: [[SB:%.*]] = fcmp fast une float [[SBX]], 0.000000e+00

  TYPE sa = buf.Load<TYPE>(0);
  TYPE sb = buf.Load<TYPE>(8);

  // I32: or i32 [[SB]], [[SA]]
  // F32: or i1 [[SA]], [[SB]]

  TYPE res = or(sb, sa);

  // CHECK: [[V1AR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 16
  // F32: [[V1AX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V1AR]], 0
  // I32: [[V1A:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V1AR]], 0

  // CHECK: [[V1BR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 24
  // F32: [[V1BX:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V1BR]], 0
  // I32: [[V1B:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V1BR]], 0

  // F32: [[V1B:%.*]] = fcmp fast une float [[V1BX]], 0.000000e+00
  // F32: [[V1A:%.*]] = fcmp fast une float [[V1AX]], 0.000000e+00

  vector<TYPE, 1> v1a = buf.Load< vector<TYPE, 1> >(16);
  vector<TYPE, 1> v1b = buf.Load< vector<TYPE, 1> >(24);

  // I32: or i32 [[V1B]], [[V1A]]
  // F32: or i1 [[V1A]], [[V1B]]

  vector<TYPE, 1> res1 = or(v1a, v1b);

  // CHECK: [[V3AR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 32
  // F32: [[V3AX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 0
  // F32: [[V3AX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 1
  // F32: [[V3AX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 2

  // I32: [[V3A0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 0
  // I32: [[V3A1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 1
  // I32: [[V3A2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3AR]], 2

  // CHECK: [[V3BR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 56
  // F32: [[V3BX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 0
  // F32: [[V3BX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 1
  // F32: [[V3BX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 2

  // I32: [[V3B0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 0
  // I32: [[V3B1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 1
  // I32: [[V3B2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[V3BR]], 2

  // F32: [[V3B0:%.*]] = fcmp fast une float [[V3BX0]], 0.000000e+00
  // F32: [[V3B1:%.*]] = fcmp fast une float [[V3BX1]], 0.000000e+00
  // F32: [[V3B2:%.*]] = fcmp fast une float [[V3BX2]], 0.000000e+00

  // F32: [[V3A0:%.*]] = fcmp fast une float [[V3AX0]], 0.000000e+00
  // F32: [[V3A1:%.*]] = fcmp fast une float [[V3AX1]], 0.000000e+00
  // F32: [[V3A2:%.*]] = fcmp fast une float [[V3AX2]], 0.000000e+00

  vector<TYPE, 3> v3a = buf.Load< vector<TYPE, 3> >(32);
  vector<TYPE, 3> v3b = buf.Load< vector<TYPE, 3> >(56);

  // I32: or i32 [[V3B0]], [[V3A0]]
  // I32: or i32 [[V3B1]], [[V3A1]]
  // I32: or i32 [[V3B2]], [[V3A2]]

  // F32: or i1 [[V3A0]], [[V3B0]]
  // F32: or i1 [[V3A1]], [[V3B1]]
  // F32: or i1 [[V3A2]], [[V3B2]]

  vector<TYPE, 3> res3 = or(v3a, v3b);

  // CHECK: [[MAR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 80
  // F32: [[MAX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 0
  // F32: [[MAX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 1
  // F32: [[MAX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 2
  // F32: [[MAX3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 3

  // I32: [[MA0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 0
  // I32: [[MA1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 1
  // I32: [[MA2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 2
  // I32: [[MA3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 3

  // CHECK: [[MAR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 96
  // F32: [[MAX4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 0
  // F32: [[MAX5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 1

  // I32: [[MA4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 0
  // I32: [[MA5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MAR]], 1

  // CHECK: [[MBR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 128
  // F32: [[MBX0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 0
  // F32: [[MBX1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 1
  // F32: [[MBX2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 2
  // F32: [[MBX3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 3

  // I32: [[MB0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 0
  // I32: [[MB1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 1
  // I32: [[MB2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 2
  // I32: [[MB3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 3

  // CHECK: [[MBR:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.bufferLoad.[[TY]](i32 68, %dx.types.Handle %{{.*}}, i32 144
  // F32: [[MBX4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 0
  // F32: [[MBX5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 1

  // I32: [[MB4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 0
  // I32: [[MB5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[MBR]], 1

  // F32: [[MB0:%.*]] = fcmp fast une float [[MBX0]], 0.000000e+00
  // F32: [[MB1:%.*]] = fcmp fast une float [[MBX1]], 0.000000e+00
  // F32: [[MB2:%.*]] = fcmp fast une float [[MBX2]], 0.000000e+00
  // F32: [[MB3:%.*]] = fcmp fast une float [[MBX3]], 0.000000e+00
  // F32: [[MB4:%.*]] = fcmp fast une float [[MBX4]], 0.000000e+00
  // F32: [[MB5:%.*]] = fcmp fast une float [[MBX5]], 0.000000e+00

  // F32: [[MA0:%.*]] = fcmp fast une float [[MAX0]], 0.000000e+00
  // F32: [[MA1:%.*]] = fcmp fast une float [[MAX1]], 0.000000e+00
  // F32: [[MA2:%.*]] = fcmp fast une float [[MAX2]], 0.000000e+00
  // F32: [[MA3:%.*]] = fcmp fast une float [[MAX3]], 0.000000e+00
  // F32: [[MA4:%.*]] = fcmp fast une float [[MAX4]], 0.000000e+00
  // F32: [[MA5:%.*]] = fcmp fast une float [[MAX5]], 0.000000e+00

  matrix<TYPE, 2, 3> mata = buf.Load< matrix<TYPE, 2, 3> >(80);
  matrix<TYPE, 2, 3> matb = buf.Load< matrix<TYPE, 2, 3> >(128);

  // I32: or i32 [[MB0]], [[MA0]]
  // I32: or i32 [[MB1]], [[MA1]]
  // I32: or i32 [[MB2]], [[MA2]]
  // I32: or i32 [[MB3]], [[MA3]]
  // I32: or i32 [[MB4]], [[MA4]]
  // I32: or i32 [[MB5]], [[MA5]]

  // F32: or i1 [[MA0]], [[MB0]]
  // F32: or i1 [[MA1]], [[MB1]]
  // F32: or i1 [[MA2]], [[MB2]]
  // F32: or i1 [[MA3]], [[MB3]]
  // F32: or i1 [[MA4]], [[MB4]]
  // F32: or i1 [[MA5]], [[MB5]]

  matrix<TYPE, 2, 3> resmat = or(mata, matb);

  return float4(res3 + resmat[0] + resmat[1], res + res1.x);
}
