//
// main1
//

static int a = int(0);
void uses_a() {
  a = (a + int(1));
}

[numthreads(1, 1, 1)]
void main1() {
  a = int(42);
  uses_a();
}

//
// main2
//

static int b = int(0);
void uses_b() {
  b = (b * int(2));
}

[numthreads(1, 1, 1)]
void main2() {
  b = int(7);
  uses_b();
}

//
// main3
//

static int a = int(0);
static int b = int(0);
void uses_a() {
  a = (a + int(1));
}

void uses_b() {
  b = (b * int(2));
}

void uses_a_and_b() {
  b = a;
}

void no_uses() {
}

void outer() {
  a = int(0);
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}

[numthreads(1, 1, 1)]
void main3() {
  outer();
  no_uses();
}

//
// main4
//

void no_uses() {
}

[numthreads(1, 1, 1)]
void main4() {
  no_uses();
}

