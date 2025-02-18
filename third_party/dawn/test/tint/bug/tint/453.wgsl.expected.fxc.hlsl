Texture2D<uint4> Src : register(t0);
RWTexture2D<uint4> Dst : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint4 srcValue = uint4(0u, 0u, 0u, 0u);
  uint4 x_22 = Src.Load(int3(0, 0, 0));
  srcValue = x_22;
  uint x_24 = srcValue.x;
  uint x_25 = (x_24 + 1u);
  uint4 x_27 = srcValue;
  Dst[(0).xx] = x_27.xxxx;
  return;
}
