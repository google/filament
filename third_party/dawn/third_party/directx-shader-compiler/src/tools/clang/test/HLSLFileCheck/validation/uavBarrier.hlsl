// RUN: %dxc -E main -T ps_6_0 %s

// CHECK: 
//at 0x1f56df38678 inside block entry of function main.flat Opcode must be defined in target shader model
//at 0x1f56df384b8 inside block entry of function main.flat Opcode must be defined in target shader model
//at 0x1f56df39478 inside block entry of function main.flat Opcode must be defined in target shader model
//at 0x1f56df39398 inside block entry of function main.flat Opcode must be defined in target shader model





RWTexture2D<float4> uav1 : register(u3);

float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  float4 r = 0;
  uint status;
  r += uav1[b];

  DeviceMemoryBarrier();

  r += uav1.Load(a);
  uav1.Load(a, status); r += status;
  uav1.Load(a, status); r += status;

  DeviceMemoryBarrier();

  uav1[b] = r;
  return r;
}
