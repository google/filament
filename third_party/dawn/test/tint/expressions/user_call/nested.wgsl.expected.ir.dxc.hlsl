
void d() {
}

void c() {
  d();
}

void b() {
  c();
}

void a() {
  b();
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

