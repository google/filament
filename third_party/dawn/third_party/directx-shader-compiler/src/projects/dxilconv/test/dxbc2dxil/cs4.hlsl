// FXC command line: fxc /T cs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




RWByteAddressBuffer uav0;

uint gIdx1;

struct Foo
{
  float c;
  min16float a;
  float b;
  min16int d;
  uint e;
};
groupshared Foo g1[16*8*3];

[numthreads(16, 8, 3)]
void main(uint3 tid : SV_DispatchThreadID,
          uint3 gid : SV_GroupID,
          uint3 tid2 : SV_GroupThreadID,
          uint groupThreadIdx : SV_GroupIndex)
{
  uint oldVal;
  uint Offset = tid.x*16*8 + gid.y*1024 + tid2.z + groupThreadIdx;
  float f0 = asfloat(uav0.Load(Offset));
  uint compValue = gid.y;

  g1[groupThreadIdx].a = (min16float)f0 + 2;
  g1[groupThreadIdx].b = f0;
  g1[groupThreadIdx].d = f0;
  g1[groupThreadIdx].e = f0;

  InterlockedAdd(g1[groupThreadIdx].e, tid.x, oldVal); f0 += oldVal;
  InterlockedOr (g1[groupThreadIdx].e, tid.x, oldVal);

  InterlockedCompareExchange(g1[groupThreadIdx].e, compValue, tid.x, oldVal);
  InterlockedCompareExchange(g1[groupThreadIdx].e, compValue, tid.x, oldVal); f0 += oldVal;

  AllMemoryBarrierWithGroupSync();

  f0 += g1[groupThreadIdx + gIdx1].e;

  AllMemoryBarrierWithGroupSync();
  uav0.Store(Offset, asuint(f0));
}
