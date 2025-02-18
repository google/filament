
Texture2DMS<float4> t_f : register(t0);
Texture2DMS<int4> t_i : register(t1);
Texture2DMS<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(v.x, v.y, v.z);
  uint2 fdims = v.xy;
  uint3 v_1 = (0u).xxx;
  t_i.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint2 idims = v_1.xy;
  uint3 v_2 = (0u).xxx;
  t_u.GetDimensions(v_2.x, v_2.y, v_2.z);
  uint2 udims = v_2.xy;
}

