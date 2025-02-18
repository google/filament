
Texture1D<float4> t_f : register(t0);
Texture1D<int4> t_i : register(t1);
Texture1D<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint2 v = (0u).xx;
  t_f.GetDimensions(uint(int(1)), v.x, v.y);
  uint fdims = v.x;
  uint2 v_1 = (0u).xx;
  t_i.GetDimensions(uint(int(1)), v_1.x, v_1.y);
  uint idims = v_1.x;
  uint2 v_2 = (0u).xx;
  t_u.GetDimensions(uint(int(1)), v_2.x, v_2.y);
  uint udims = v_2.x;
}

