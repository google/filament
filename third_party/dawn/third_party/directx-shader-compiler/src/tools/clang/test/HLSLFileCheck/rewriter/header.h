struct A {
  float a;
};

struct B : A {
  float b;
};
struct X {
float a;
};
// test macro
#ifdef TEST
float bbb;
#endif

struct C {
  B b;
  float c;
  float Get() { return c + b.b + b.a; }
};

struct D {
  C c;
};