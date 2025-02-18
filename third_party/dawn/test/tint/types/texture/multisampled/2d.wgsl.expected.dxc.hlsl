Texture2DMS<float4> t_f : register(t0);
Texture2DMS<int4> t_i : register(t1);
Texture2DMS<uint4> t_u : register(t2);

[numthreads(1, 1, 1)]
void main() {
  uint3 tint_tmp;
  t_f.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint2 fdims = tint_tmp.xy;
  uint3 tint_tmp_1;
  t_i.GetDimensions(tint_tmp_1.x, tint_tmp_1.y, tint_tmp_1.z);
  uint2 idims = tint_tmp_1.xy;
  uint3 tint_tmp_2;
  t_u.GetDimensions(tint_tmp_2.x, tint_tmp_2.y, tint_tmp_2.z);
  uint2 udims = tint_tmp_2.xy;
  return;
}
