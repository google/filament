void set_vector_element(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_1(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_2(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_vector_element_3(inout int2 vec, int idx, int val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_4(inout int3 vec, int idx, int val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_5(inout int4 vec, int idx, int val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_vector_element_6(inout uint2 vec, int idx, uint val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_7(inout uint3 vec, int idx, uint val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_8(inout uint4 vec, int idx, uint val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_vector_element_9(inout bool2 vec, int idx, bool val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_10(inout bool3 vec, int idx, bool val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_11(inout bool4 vec, int idx, bool val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

[numthreads(1, 1, 1)]
void main() {
  float2 v2f = float2(0.0f, 0.0f);
  float3 v3f = float3(0.0f, 0.0f, 0.0f);
  float4 v4f = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int2 v2i = int2(0, 0);
  int3 v3i = int3(0, 0, 0);
  int4 v4i = int4(0, 0, 0, 0);
  uint2 v2u = uint2(0u, 0u);
  uint3 v3u = uint3(0u, 0u, 0u);
  uint4 v4u = uint4(0u, 0u, 0u, 0u);
  bool2 v2b = bool2(false, false);
  bool3 v3b = bool3(false, false, false);
  bool4 v4b = bool4(false, false, false, false);
  {
    for(int i = 0; (i < 2); i = (i + 1)) {
      set_vector_element(v2f, min(uint(i), 1u), 1.0f);
      set_vector_element_1(v3f, min(uint(i), 2u), 1.0f);
      set_vector_element_2(v4f, min(uint(i), 3u), 1.0f);
      set_vector_element_3(v2i, min(uint(i), 1u), 1);
      set_vector_element_4(v3i, min(uint(i), 2u), 1);
      set_vector_element_5(v4i, min(uint(i), 3u), 1);
      set_vector_element_6(v2u, min(uint(i), 1u), 1u);
      set_vector_element_7(v3u, min(uint(i), 2u), 1u);
      set_vector_element_8(v4u, min(uint(i), 3u), 1u);
      set_vector_element_9(v2b, min(uint(i), 1u), true);
      set_vector_element_10(v3b, min(uint(i), 2u), true);
      set_vector_element_11(v4b, min(uint(i), 3u), true);
    }
  }
  return;
}
