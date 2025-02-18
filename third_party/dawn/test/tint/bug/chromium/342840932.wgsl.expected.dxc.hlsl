[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

Texture1D<uint4> image_dup_src : register(t0);
RWTexture1D<uint4> image_dst : register(u1);
