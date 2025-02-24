Texture3D<float4> t_f : register(t0);
Texture3D<int4> t_i : register(t1);
Texture3D<uint4> t_u : register(t2);

[numthreads(1, 1, 1)]
void main() {
  uint4 tint_tmp;
  t_f.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  uint3 fdims = tint_tmp.xyz;
  uint4 tint_tmp_1;
  t_i.GetDimensions(1, tint_tmp_1.x, tint_tmp_1.y, tint_tmp_1.z, tint_tmp_1.w);
  uint3 idims = tint_tmp_1.xyz;
  uint4 tint_tmp_2;
  t_u.GetDimensions(1, tint_tmp_2.x, tint_tmp_2.y, tint_tmp_2.z, tint_tmp_2.w);
  uint3 udims = tint_tmp_2.xyz;
  return;
}
