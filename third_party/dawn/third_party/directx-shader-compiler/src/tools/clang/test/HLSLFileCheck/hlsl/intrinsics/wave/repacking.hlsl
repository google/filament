// RUN: %dxc -DREPACK_POINT_KIND=1 -T lib_6_5 %s | FileCheck %s
// RUN: %dxc -DREPACK_POINT_KIND=2 -T lib_6_5 %s | FileCheck %s
// RUN: %dxc -DREPACK_POINT_KIND=3 -T lib_6_5 %s | FileCheck %s

// Check that results of wave intrinsics are not re-used cross DXR repacking points.

#define REPACK_POINT_KIND_TRACERAY   1
#define REPACK_POINT_KIND_CALLSHADER 2
#define REPACK_POINT_KIND_REPORTHIT  3

struct Payload {
  unsigned int value;
};

struct HitAttributes {
  unsigned int value;
};

RaytracingAccelerationStructure myAccelerationStructure : register(t3);

// Helper to introduce a repacking point, passing the identifier as argument
// so we can find it in the generated DXIL.
// dep is used to introduce a dependency of the packing point
// on the passed value, and the returned value is guaranteed to depend
// on the result of the repacking point.
unsigned int RepackingPoint(unsigned int dependency, int identifier) {
  unsigned int result = dependency;
#if   REPACK_POINT_KIND == REPACK_POINT_KIND_CALLSHADER
  Payload p;
  p.value = dependency;
  CallShader(identifier, p);
  result += p.value;
#elif REPACK_POINT_KIND == REPACK_POINT_KIND_TRACERAY
  Payload p;
  p.value = dependency;
  RayDesc myRay = { float3(0., 0., 0.), 0., float3(0., 0., 0.), 1.0};
  TraceRay(myAccelerationStructure, 0, -1, 0, 0, identifier, myRay, p);
  result += p.value;
#elif REPACK_POINT_KIND == REPACK_POINT_KIND_REPORTHIT
  HitAttributes attrs;
  attrs.value = dependency;
  bool didAccept = ReportHit(0.0, identifier, attrs);
  if (didAccept) {
     result += 1;
  }
#else
#error "Unknown repack point kind"
#endif
  return result;
}

RWBuffer<unsigned int> output : register(u0, space0);

// Calls wave intrinsics before and after repacking points, and checks
// that both calls remain, as re-using the result from before the repacking
// point is invalid, because threads may be re-packed in between.
#if (REPACK_POINT_KIND == REPACK_POINT_KIND_TRACERAY) || \
    (REPACK_POINT_KIND == REPACK_POINT_KIND_CALLSHADER)
[shader("miss")] void Miss(inout Payload p) {
#else // REPACK_POINT_KIND_REPORTHIT
[shader("intersection")] void Intersection() {
#endif
  // Opaque value the compiler cannot reason about to prevent optimizations.
  // At the end we store the resulting value back to the buffer so the
  // test code cannot be optimized out.
  unsigned int opaque = output[DispatchRaysIndex().x];
  // Passed as argument to wave intrinsics taking an argument to
  // ensure repeated calls to intrinsics use the same argument.
  // Otherwise the argument being different already prevents re-use,
  // rendering the test pointless.
  unsigned int commonArg = opaque;

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 0
  // CHECK: @dx.op.waveIsFirstLane(i32 110
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 1
  // CHECK: @dx.op.waveIsFirstLane(i32 110
  opaque += RepackingPoint(opaque, 0);
  opaque += WaveIsFirstLane();
  opaque += RepackingPoint(opaque, 1);
  opaque += WaveIsFirstLane();

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 2
  // CHECK: @dx.op.waveGetLaneIndex(i32 111
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 3
  // CHECK: @dx.op.waveGetLaneIndex(i32 111
  opaque += RepackingPoint(opaque, 2);
  opaque += WaveGetLaneIndex();
  opaque += RepackingPoint(opaque, 3);
  opaque += WaveGetLaneIndex();

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 4
  // CHECK: @dx.op.waveAnyTrue(i32 113, i1
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 5
  // CHECK: @dx.op.waveAnyTrue(i32 113, i1
  opaque += RepackingPoint(opaque, 4);
  opaque += WaveActiveAnyTrue(commonArg == 17);
  opaque += RepackingPoint(opaque, 5);
  opaque += WaveActiveAnyTrue(commonArg == 17);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 6
  // CHECK: @dx.op.waveAllTrue(i32 114, i1
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 7
  // CHECK: @dx.op.waveAllTrue(i32 114, i1
  opaque += RepackingPoint(opaque, 6);
  opaque += WaveActiveAllTrue(commonArg == 17);
  opaque += RepackingPoint(opaque, 7);
  opaque += WaveActiveAllTrue(commonArg == 17);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 8
  // CHECK: @dx.op.waveActiveAllEqual.i32(i32 115, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 9
  // CHECK: @dx.op.waveActiveAllEqual.i32(i32 115, i32
  opaque += RepackingPoint(opaque, 8);
  opaque += WaveActiveAllEqual(commonArg);
  opaque += RepackingPoint(opaque, 9);
  opaque += WaveActiveAllEqual(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 10
  // CHECK: call %dx.types.fouri32 @dx.op.waveActiveBallot(i32 116, i1
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 11
  // CHECK: call %dx.types.fouri32 @dx.op.waveActiveBallot(i32 116, i1
  opaque += RepackingPoint(opaque, 10);
  opaque += WaveActiveBallot(commonArg).x;
  opaque += RepackingPoint(opaque, 11);
  opaque += WaveActiveBallot(commonArg).x;

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 12
  // CHECK: @dx.op.waveReadLaneAt.i32(i32 117, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 13
  // CHECK: @dx.op.waveReadLaneAt.i32(i32 117, i32
  opaque += RepackingPoint(opaque, 12);
  opaque += WaveReadLaneAt(commonArg, 1);
  opaque += RepackingPoint(opaque, 13);
  opaque += WaveReadLaneAt(commonArg, 1);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 14
  // CHECK: @dx.op.waveReadLaneFirst.i32(i32 118, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 15
  // CHECK: @dx.op.waveReadLaneFirst.i32(i32 118, i32
  opaque += RepackingPoint(opaque, 14);
  opaque += WaveReadLaneFirst(commonArg);
  opaque += RepackingPoint(opaque, 15);
  opaque += WaveReadLaneFirst(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 16
  // CHECK: @dx.op.waveActiveOp.i32(i32 119, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 17
  // CHECK: @dx.op.waveActiveOp.i32(i32 119, i32
  opaque += RepackingPoint(opaque, 16);
  opaque += WaveActiveSum(commonArg);
  opaque += RepackingPoint(opaque, 17);
  opaque += WaveActiveSum(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 18
  // CHECK: @dx.op.waveActiveOp.i64(i32 119, i64
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 19
  // CHECK: @dx.op.waveActiveOp.i64(i32 119, i64
  opaque += RepackingPoint(opaque, 18);
  opaque += WaveActiveProduct(commonArg == 17 ? 1 : 0);
  opaque += RepackingPoint(opaque, 19);
  opaque += WaveActiveProduct(commonArg == 17 ? 1 : 0);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 20
  // CHECK: @dx.op.waveActiveBit.i32(i32 120, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 21
  // CHECK: @dx.op.waveActiveBit.i32(i32 120, i32
  opaque += RepackingPoint(opaque, 20);
  opaque += WaveActiveBitAnd(commonArg);
  opaque += RepackingPoint(opaque, 21);
  opaque += WaveActiveBitAnd(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 22
  // CHECK: @dx.op.waveActiveBit.i32(i32 120, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 23
  // CHECK: @dx.op.waveActiveBit.i32(i32 120, i32
  opaque += RepackingPoint(opaque, 22);
  opaque += WaveActiveBitXor(commonArg);
  opaque += RepackingPoint(opaque, 23);
  opaque += WaveActiveBitXor(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 24
  // CHECK: @dx.op.waveActiveOp.i32(i32 119, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 25
  // CHECK: @dx.op.waveActiveOp.i32(i32 119, i32
  opaque += RepackingPoint(opaque, 24);
  opaque += WaveActiveMin(commonArg);
  opaque += RepackingPoint(opaque, 25);
  opaque += WaveActiveMin(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 26
  // CHECK: @dx.op.waveActiveOp.i32(i32 119, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 27
  // CHECK: @dx.op.waveActiveOp.i32(i32 119, i32
  opaque += RepackingPoint(opaque, 26);
  opaque += WaveActiveMax(commonArg);
  opaque += RepackingPoint(opaque, 27);
  opaque += WaveActiveMax(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 28
  // CHECK: @dx.op.wavePrefixOp.i32(i32 121, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 29
  // CHECK: @dx.op.wavePrefixOp.i32(i32 121, i32
  opaque += RepackingPoint(opaque, 28);
  opaque += WavePrefixSum(commonArg);
  opaque += RepackingPoint(opaque, 29);
  opaque += WavePrefixSum(commonArg);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 30
  // CHECK: @dx.op.wavePrefixOp.i64(i32 121, i64
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 31
  // CHECK: @dx.op.wavePrefixOp.i64(i32 121, i64
  opaque += RepackingPoint(opaque, 30);
  opaque += WavePrefixProduct(commonArg == 17 ? 1 : 0);
  opaque += RepackingPoint(opaque, 31);
  opaque += WavePrefixProduct(commonArg == 17 ? 1 : 0);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 32
  // CHECK: @dx.op.waveAllOp(i32 135, i1
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 33
  // CHECK: @dx.op.waveAllOp(i32 135, i1
  opaque += RepackingPoint(opaque, 32);
  opaque += WaveActiveCountBits(commonArg == 17);
  opaque += RepackingPoint(opaque, 33);
  opaque += WaveActiveCountBits(commonArg == 17);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 34
  // CHECK: @dx.op.wavePrefixOp(i32 136, i1
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 35
  // CHECK: @dx.op.wavePrefixOp(i32 136, i1
  opaque += RepackingPoint(opaque, 34);
  opaque += WavePrefixCountBits(commonArg == 17);
  opaque += RepackingPoint(opaque, 35);
  opaque += WavePrefixCountBits(commonArg == 17);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 36
  // CHECK: call %dx.types.fouri32 @dx.op.waveMatch.i32(i32 165, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 37
  // CHECK: call %dx.types.fouri32 @dx.op.waveMatch.i32(i32 165, i32
  opaque += RepackingPoint(opaque, 36);
  uint4 mask = WaveMatch(commonArg);
  opaque += mask.x;
  opaque += RepackingPoint(opaque, 37);
  opaque += WaveMatch(commonArg).x;

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 38
  // CHECK: @dx.op.waveMultiPrefixOp.i32(i32 166, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 39
  // CHECK: @dx.op.waveMultiPrefixOp.i32(i32 166, i32
  opaque += RepackingPoint(opaque, 38);
  opaque += WaveMultiPrefixBitAnd(commonArg, mask);
  opaque += RepackingPoint(opaque, 39);
  opaque += WaveMultiPrefixBitAnd(commonArg, mask);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 40
  // CHECK: @dx.op.waveMultiPrefixOp.i32(i32 166, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 41
  // CHECK: @dx.op.waveMultiPrefixOp.i32(i32 166, i32
  opaque += RepackingPoint(opaque, 40);
  opaque += WaveMultiPrefixBitOr(commonArg, mask);
  opaque += RepackingPoint(opaque, 41);
  opaque += WaveMultiPrefixBitOr(commonArg, mask);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 42
  // CHECK: @dx.op.waveMultiPrefixOp.i32(i32 166, i32
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 43
  // CHECK: @dx.op.waveMultiPrefixOp.i32(i32 166, i32
  opaque += RepackingPoint(opaque, 42);
  opaque += WaveMultiPrefixBitXor(commonArg, mask);
  opaque += RepackingPoint(opaque, 43);
  opaque += WaveMultiPrefixBitXor(commonArg, mask);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 44
  // CHECK: @dx.op.waveMultiPrefixBitCount(i32 167, i1
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 45
  // CHECK: @dx.op.waveMultiPrefixBitCount(i32 167, i1
  opaque += RepackingPoint(opaque, 44);
  opaque += WaveMultiPrefixCountBits(commonArg == 17, mask);
  opaque += RepackingPoint(opaque, 45);
  opaque += WaveMultiPrefixCountBits(commonArg == 17, mask);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 46
  // CHECK: @dx.op.waveMultiPrefixOp.i64(i32 166, i64
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 47
  // CHECK: @dx.op.waveMultiPrefixOp.i64(i32 166, i64
  opaque += RepackingPoint(opaque, 46);
  opaque += WaveMultiPrefixProduct(commonArg == 17 ? 1 : 0, mask);
  opaque += RepackingPoint(opaque, 47);
  opaque += WaveMultiPrefixProduct(commonArg == 17 ? 1 : 0, mask);

  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 48
  // CHECK: @dx.op.waveMultiPrefixOp.i64(i32 166, i64
  // CHECK: @dx.op.{{traceRay|callShader|reportHit}}{{.*}} i32 49
  // CHECK: @dx.op.waveMultiPrefixOp.i64(i32 166, i64
  opaque += RepackingPoint(opaque, 48);
  opaque += WaveMultiPrefixSum(commonArg == 17 ? 1 : 0, mask);
  opaque += RepackingPoint(opaque, 49);
  opaque += WaveMultiPrefixSum(commonArg == 17 ? 1 : 0, mask);

  output[DispatchRaysIndex().x] = opaque;
}
