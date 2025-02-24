
void f() {
  uint2 v2 = (3u).xx;
  uint3 v3 = (3u).xxx;
  uint4 v4 = (3u).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

