struct computeMain_inputs {
  uint3 global_id : SV_DispatchThreadID;
};


RWByteAddressBuffer drawOut : register(u5);
static uint cubeVerts = 0u;
void computeMain_inner(uint3 global_id) {
  uint v = 0u;
  drawOut.InterlockedAdd(0u, cubeVerts, v);
  uint firstVertex = v;
}

[numthreads(1, 1, 1)]
void computeMain(computeMain_inputs inputs) {
  computeMain_inner(inputs.global_id);
}

