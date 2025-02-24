
Texture2DArray<float4> t_f : register(t0);
Texture2DArray<int4> t_i : register(t1);
Texture2DArray<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint4 v = (0u).xxxx;
  t_f.GetDimensions(uint(int(1)), v.x, v.y, v.z, v.w);
  uint2 fdims = v.xy;
  uint4 v_1 = (0u).xxxx;
  t_i.GetDimensions(uint(int(1)), v_1.x, v_1.y, v_1.z, v_1.w);
  uint2 idims = v_1.xy;
  uint4 v_2 = (0u).xxxx;
  t_u.GetDimensions(uint(int(1)), v_2.x, v_2.y, v_2.z, v_2.w);
  uint2 udims = v_2.xy;
}

