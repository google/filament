// RUN: %dxc -T ps_6_7 %s | FileCheck %s

// Test for RWTexture2DMS and RWTexture2DMSArray types and their operations

RWTexture2DMS<float4,8> g_rw_t2dms;
RWTexture2DMSArray<uint,8> g_rw_t2dmsa;


//CHECK: Advanced Texture Ops
//CHECK: Writeable MSAA Textures

float4 test2DMS(uint sampleSlice, uint2 pos2, uint3 pos3) {
  uint w, h, s, w2, h2, s2, e;
  uint fw, fh, fs, fw2, fh2, fs2, fe;
  uint status;

  g_rw_t2dms.GetDimensions(w, h, s);
  g_rw_t2dmsa.GetDimensions(w2, h2, e, s2);

  float4 res = uint4(w, h + s, w2 + e, h2 + s2);

  g_rw_t2dms.GetDimensions(fw, fh, fs);
  g_rw_t2dmsa.GetDimensions(fw2, fh2, fe, fs2);

  res *= float4(fw, fh + fs, fw2 + fe, fh2 + fs2);

  res -= g_rw_t2dms.GetSamplePosition(sampleSlice).xyxy;
  res -= g_rw_t2dmsa.GetSamplePosition(sampleSlice).xyxy;

  res += g_rw_t2dms.Load(pos2 + 1, sampleSlice + 1);
  res += g_rw_t2dmsa.Load(pos3 + 1, sampleSlice + 1);

  res += g_rw_t2dms.Load(pos2 + 2, sampleSlice + 2, status);
  res += status;
  res += g_rw_t2dmsa.Load(pos3 + 2, sampleSlice + 2, status);
  res += status;

  res += g_rw_t2dms[pos2];
  res += g_rw_t2dmsa[pos3];

  res += g_rw_t2dms.sample[sampleSlice][pos2];
  res += g_rw_t2dmsa.sample[sampleSlice][pos3];

  g_rw_t2dms[pos2] = res;
  g_rw_t2dmsa[pos3] = res.x;

  g_rw_t2dms.sample[sampleSlice][pos2] = res;
  g_rw_t2dmsa.sample[sampleSlice][pos3] = res.x;

  return res;
}

float4 main(uint sampleSlice : S, uint2 pos2 : PP, uint3 pos3 : PPP) : SV_Target {
  float4 res = 0.0;
  // Collect important param values
  // CHECK: [[POS3X:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  // CHECK: [[POS3Y:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 1, i32 undef)
  // CHECK: [[POS3Z:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 2, i32 undef)
  // CHECK: [[POS2X:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  // CHECK: [[POS2Y:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 1, i32 undef)
  // CHECK: [[SLICE:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)

  // Test with constant values
  //CHECK: @dx.op.getDimensions(i32 72
  //CHECK: @dx.op.getDimensions(i32 72
  //CHECK: @dx.op.texture2DMSGetSamplePosition(i32 75, %dx.types.Handle %{{.*}}, i32 1)
  //CHECK: @dx.op.texture2DMSGetSamplePosition(i32 75, %dx.types.Handle %{{.*}}, i32 1)
  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 2, i32 3, i32 4
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 2, i32 5, i32 6, i32 7
  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 3, i32 4, i32 5
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 3, i32 6, i32 7, i32 8
  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 0, i32 2, i32 3
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 0, i32 4, i32 5, i32 6
  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 1, i32 2, i32 3
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 1, i32 4, i32 5, i32 6
  // CHECK:  @dx.op.textureStoreSample.f32(i32 225, %dx.types.Handle %{{.*}}, i32 2, i32 3, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, i8 15, i32 0)
  // CHECK:  @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle %{{.*}}, i32 4, i32 5, i32 6, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i8 15, i32 0)
  // CHECK:  @dx.op.textureStoreSample.f32(i32 225, %dx.types.Handle %{{.*}}, i32 2, i32 3, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, i8 15, i32 1)
  // CHECK:  @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle %{{.*}}, i32 4, i32 5, i32 6, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i8 15, i32 1)

  res += test2DMS(1, uint2(2,3), uint3(4,5,6));

  // Test with params
  //CHECK: @dx.op.getDimensions(i32 72
  //CHECK: @dx.op.getDimensions(i32 72
  //CHECK: @dx.op.texture2DMSGetSamplePosition(i32 75, %dx.types.Handle %{{.*}}, i32 [[SLICE]])
  //CHECK: @dx.op.texture2DMSGetSamplePosition(i32 75, %dx.types.Handle %{{.*}}, i32 [[SLICE]])
  //CHECK: [[SLICEp1:%.*]] = add i32 [[SLICE]], 1
  //CHECK: [[POS2Xp1:%.*]] = add i32 [[POS2X]], 1
  //CHECK: [[POS2Yp1:%.*]] = add i32 [[POS2Y]], 1
  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 [[SLICEp1]], i32 [[POS2Xp1]], i32 [[POS2Yp1]]
  //CHECK: [[POS3Xp1:%.*]] = add i32 [[POS3X]], 1
  //CHECK: [[POS3Yp1:%.*]] = add i32 [[POS3Y]], 1
  //CHECK: [[POS3Zp1:%.*]] = add i32 [[POS3Z]], 1
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 [[SLICEp1]], i32 [[POS3Xp1]], i32 [[POS3Yp1]], i32 [[POS3Zp1]]
  //CHECK: [[SLICEp2:%.*]] = add i32 [[SLICE]], 2
  //CHECK: [[POS2Xp2:%.*]] = add i32 [[POS2X]], 2
  //CHECK: [[POS2Yp2:%.*]] = add i32 [[POS2Y]], 2
  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 [[SLICEp2]], i32 [[POS2Xp2]], i32 [[POS2Yp2]]
  //CHECK: [[POS3Xp2:%.*]] = add i32 [[POS3X]], 2
  //CHECK: [[POS3Yp2:%.*]] = add i32 [[POS3Y]], 2
  //CHECK: [[POS3Zp2:%.*]] = add i32 [[POS3Z]], 2
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 [[SLICEp2]], i32 [[POS3Xp2]], i32 [[POS3Yp2]], i32 [[POS3Zp2]]

  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 0, i32 [[POS2X]], i32 [[POS2Y]]
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 0, i32 [[POS3X]], i32 [[POS3Y]], i32 [[POS3Z]]

  //CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %{{.*}}, i32 [[SLICE]], i32 [[POS2X]], i32 [[POS2Y]]
  //CHECK: @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %{{.*}}, i32 [[SLICE]], i32 [[POS3X]], i32 [[POS3Y]], i32 [[POS3Z]]

  // CHECK:  @dx.op.textureStoreSample.f32(i32 225, %dx.types.Handle %{{.*}}, i32 [[POS2X]], i32 [[POS2Y]], i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, i8 15, i32 0)
  // CHECK:  @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle %{{.*}}, i32 [[POS3X]], i32 [[POS3Y]], i32 [[POS3Z]], i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i8 15, i32 0)
  // CHECK:  @dx.op.textureStoreSample.f32(i32 225, %dx.types.Handle %{{.*}}, i32 [[POS2X]], i32 [[POS2Y]], i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, i8 15, i32 [[SLICE]])
  // CHECK:  @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle %{{.*}}, i32 [[POS3X]], i32 [[POS3Y]], i32 [[POS3Z]], i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i8 15, i32 [[SLICE]])

  res += test2DMS(sampleSlice, pos2, pos3);

  return res;
}
