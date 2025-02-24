Texture1D<float4> t_f : register(t0);
Texture1D<int4> t_i : register(t1);
Texture1D<uint4> t_u : register(t2);

[numthreads(1, 1, 1)]
void main() {
  uint2 tint_tmp;
  t_f.GetDimensions(1, tint_tmp.x, tint_tmp.y);
  uint fdims = tint_tmp.x;
  uint2 tint_tmp_1;
  t_i.GetDimensions(1, tint_tmp_1.x, tint_tmp_1.y);
  uint idims = tint_tmp_1.x;
  uint2 tint_tmp_2;
  t_u.GetDimensions(1, tint_tmp_2.x, tint_tmp_2.y);
  uint udims = tint_tmp_2.x;
  return;
}
