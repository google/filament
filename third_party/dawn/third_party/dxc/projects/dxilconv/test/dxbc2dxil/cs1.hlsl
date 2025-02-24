// FXC command line: fxc /T cs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




RWByteAddressBuffer uav0;

[numthreads(16, 8, 3)]
void main(uint3 tid : SV_DispatchThreadID,
          uint3 gid : SV_GroupID,
          uint3 tid2 : SV_GroupThreadID,
          uint groupIdx : SV_GroupIndex)
{
  uint Offset = tid.x*16*8 + gid.y*1024 + tid2.z + groupIdx;
  float f0 = asfloat(uav0.Load(Offset));
  
  AllMemoryBarrierWithGroupSync();
//  DeviceMemoryBarrierWithGroupSync();
//  GroupMemoryBarrierWithGroupSync();

  f0 += 3.f;
  uav0.Store(Offset, asuint(f0));

//  AllMemoryBarrier();
  DeviceMemoryBarrier();
//  GroupMemoryBarrier();
}
