
void a() {
}

void a__() {
}

void b() {
  a();
}

void b__() {
  a__();
}

[numthreads(1, 1, 1)]
void main() {
  b();
  b__();
}

