
void foo() {
  int explicitStride[2] = (int[2])0;
  int implictStride[2] = (int[2])0;
  int v[2] = explicitStride;
  implictStride = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

