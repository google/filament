// RUN: %dxc /Zi /HV 2021 /T vs_6_2 /E main %s | FileCheck %s

RWByteAddressBuffer BufferOut : register(u0);

// The store of p.y into the temporary was not
// getting extended to 32 bits, so uninitialized
// bits of the temporary were used.  7366161.
struct foo {
  int x : 8;
  int : 8;
  int y : 16;
};

int main(struct foo p : IN0, int x : IN1) : OUT {
  // CHECK: %[[val:.*]] = {{.*}}loadInput{{.*}} i32 0, i32 0, {{.*}}
  // CHECK:               ashr i32 %[[val]]
  BufferOut.Store(p.x, asuint(p.y));
  x = (p.y > x ? x : p.y);
  return x;
}

struct M {
  uint f0;
  uint bf0 : 8;
  uint bf1 : 24;
  uint f1;
  uint : 8;
  uint bf2 : 16;
  uint : 8;
  uint f2;
};

void m(uint3 xyz : IN) {
  M m;

  m.f0 = 0;
  m.bf1 = xyz.x;
  m.f1 = xyz.y;
  m.bf2 = xyz.z;
  m.f2 = 1;
  BufferOut.Store(m.f1*16, asuint(m.bf1 + m.bf2));
  return;
}

struct S {
  uint : 8;
  uint : 8;
  uint x : 8;
} s;
// CHECK: !DIDerivedType(tag: DW_TAG_member, name: "x", {{.*}}, size: 8, align: 32, offset: 16)

// Below adapted from clang test suite.

// PR14638; make sure this doesn't crash.
struct A {
  bool m_sorted : 1;
};

void func1(bool b, A a1)
{
  if ((a1.m_sorted = b)) {}
}

struct TEST2
{
  int subid:32;
  int :0;
};

typedef struct _TEST3
{
  TEST2 foo;
  TEST2 foo2;
} TEST3;

void test() {
  TEST3 test =
    {
      {0, 0},
      {0, 0}
    };
}

struct P1 {
  uint l_Packed;
  uint k_Packed : 6,
    i_Packed : 15,
    j_Packed : 11;

};

struct P2 {
  uint l_Packed;
  uint k_Packed : 6,
    i_Packed : 15;
  uint c;

};

struct P1 sM_Packed;

int p() : OUT {
  struct P2 x;
  return (x.i_Packed != 0);
}

struct Bork {
  unsigned int f1 : 3;
  unsigned int f2 : 30;
};

void bork(Bork hdr : IN) {
  hdr.f1 = 7;
  hdr.f2 = 927;
}

struct F {
  bool b1 : 1;
  bool b2 : 7;
};

bool f() : OUT
{
  F f = { true, true };

  return f.b1 != 1;
}

struct s8_0 { uint : 0; };
struct s8_1 { double x; };
struct s8 { s8_0 a; s8_1 b; };
s8 g() : OUT { s8 s; s.b.x = 0.0; return s; }

// Exclude quoted source file
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}
