// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

//CHECK:%[[FirstLane:[a-zA-Z0-9]+]] = call i32 @dx.op.waveReadLaneFirst.i32(i32 118,
// Make sure use FirstLane.
//CHECK:uitofp i32 %[[FirstLane]] to float


float main(uint i:I) : SV_Target {
  const uint uniformIndex = WaveReadLaneFirst ( i ) ;
  if ( uniformIndex == i)
  {
    return sin(uniformIndex);
  } else {
    return 13;
  }
}