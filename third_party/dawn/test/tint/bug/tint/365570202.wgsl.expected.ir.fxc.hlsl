
RWTexture2D<float4> tex : register(u0);
[numthreads(1, 1, 1)]
void main() {
  tex[(int(0)).xx] = (0.0f).xxxx;
}

