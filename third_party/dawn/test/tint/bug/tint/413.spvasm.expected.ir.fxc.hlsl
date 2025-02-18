
Texture2D<uint4> Src : register(t0);
RWTexture2D<uint4> Dst : register(u1);
void main_1() {
  uint4 srcValue = (0u).xxxx;
  srcValue = Src.Load(int3((int(0)).xx, int(0)));
  srcValue.x = (srcValue.x + 1u);
  uint4 x_27 = srcValue;
  Dst[(int(0)).xx] = x_27;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

