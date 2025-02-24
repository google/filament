TextureCube<float4> t_f : register(t0);
TextureCube<int4> t_i : register(t1);
TextureCube<uint4> t_u : register(t2);

[numthreads(1, 1, 1)]
void main() {
  uint3 tint_tmp;
  t_f.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint2 fdims = tint_tmp.xy;
  uint3 tint_tmp_1;
  t_i.GetDimensions(1, tint_tmp_1.x, tint_tmp_1.y, tint_tmp_1.z);
  uint2 idims = tint_tmp_1.xy;
  uint3 tint_tmp_2;
  t_u.GetDimensions(1, tint_tmp_2.x, tint_tmp_2.y, tint_tmp_2.z);
  uint2 udims = tint_tmp_2.xy;
  return;
}
