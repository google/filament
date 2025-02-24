//
// main1
//
static int a = 0;

void uses_a() {
  a = (a + 1);
}

[numthreads(1, 1, 1)]
void main1() {
  a = 42;
  uses_a();
  return;
}
//
// main2
//
static int b = 0;

void uses_b() {
  b = (b * 2);
}

[numthreads(1, 1, 1)]
void main2() {
  b = 7;
  uses_b();
  return;
}
//
// main3
//
static int a = 0;
static int b = 0;

void uses_a() {
  a = (a + 1);
}

void uses_b() {
  b = (b * 2);
}

void uses_a_and_b() {
  b = a;
}

void no_uses() {
}

void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}

[numthreads(1, 1, 1)]
void main3() {
  outer();
  no_uses();
  return;
}
//
// main4
//
void no_uses() {
}

[numthreads(1, 1, 1)]
void main4() {
  no_uses();
  return;
}
