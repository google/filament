
void a(inout int x) {
}

void b(inout int x) {
  a(x);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

