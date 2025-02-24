struct main_inputs {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};


Texture2D tex : register(t0);
RWByteAddressBuffer result : register(u1);
void main_inner(uint3 GlobalInvocationId) {
  uint v = 0u;
  result.GetDimensions(v);
  uint v_1 = (min(((GlobalInvocationId.y * 128u) + GlobalInvocationId.x), ((v / 4u) - 1u)) * 4u);
  int v_2 = int(GlobalInvocationId.x);
  result.Store((0u + v_1), asuint(tex.Load(int3(int2(v_2, int(GlobalInvocationId.y)), int(0))).x));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationId);
}

