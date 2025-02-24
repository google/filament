
TextureCube<float4> t_f : register(t0);
TextureCube<int4> t_i : register(t1);
TextureCube<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(uint(int(1)), v.x, v.y, v.z);
  uint2 fdims = v.xy;
  uint3 v_1 = (0u).xxx;
  t_i.GetDimensions(uint(int(1)), v_1.x, v_1.y, v_1.z);
  uint2 idims = v_1.xy;
  uint3 v_2 = (0u).xxx;
  t_u.GetDimensions(uint(int(1)), v_2.x, v_2.y, v_2.z);
  uint2 udims = v_2.xy;
}

