// RUN: %clang_cc1 -fsyntax-only -verify -std=c++11 -Wno-unused %s

int f(int, int = 0);

struct A {
  int x, y;
};
struct S {
  S(int, int);
  int n;
};

void test() {
  int a;
  int xs[10];
  ++a = 0; // ok
  a + ++a; // expected-warning {{unsequenced modification and access to 'a'}}
  a = ++a; // ok
  a + a++; // expected-warning {{unsequenced modification and access to 'a'}}
  a = a++; // expected-warning {{multiple unsequenced modifications to 'a'}}
  ++ ++a; // ok
  (a++, a++); // ok
  ++a + ++a; // expected-warning {{multiple unsequenced modifications to 'a'}}
  a++ + a++; // expected-warning {{multiple unsequenced modifications}}
  (a++, a) = 0; // ok, increment is sequenced before value computation of LHS
  a = xs[++a]; // ok
  a = xs[a++]; // expected-warning {{multiple unsequenced modifications}}
  (a ? xs[0] : xs[1]) = ++a; // expected-warning {{unsequenced modification and access}}
  a = (++a, ++a); // ok
  a = (a++, ++a); // ok
  a = (a++, a++); // expected-warning {{multiple unsequenced modifications}}
  f(a, a); // ok
  f(a = 0, a); // expected-warning {{unsequenced modification and access}}
  f(a, a += 0); // expected-warning {{unsequenced modification and access}}
  f(a = 0, a = 0); // expected-warning {{multiple unsequenced modifications}}
  a = f(++a); // ok
  a = f(a++); // ok
  a = f(++a, a++); // expected-warning {{multiple unsequenced modifications}}

  // Compound assignment "A OP= B" is equivalent to "A = A OP B" except that A
  // is evaluated only once.
  (++a, a) = 1; // ok
  (++a, a) += 1; // ok
  a = ++a; // ok
  a += ++a; // expected-warning {{unsequenced modification and access}}

  A agg1 = { a++, a++ }; // ok
  A agg2 = { a++ + a, a++ }; // expected-warning {{unsequenced modification and access}}

  S str1(a++, a++); // expected-warning {{multiple unsequenced modifications}}
  S str2 = { a++, a++ }; // ok
  S str3 = { a++ + a, a++ }; // expected-warning {{unsequenced modification and access}}

  struct Z { A a; S s; } z = { { ++a, ++a }, { ++a, ++a } }; // ok
  a = S { ++a, a++ }.n; // ok
  A { ++a, a++ }.x; // ok
  a = A { ++a, a++ }.x; // expected-warning {{unsequenced modifications}}
  A { ++a, a++ }.x + A { ++a, a++ }.y; // expected-warning {{unsequenced modifications}}

  (xs[2] && (a = 0)) + a; // ok
  (0 && (a = 0)) + a; // ok
  (1 && (a = 0)) + a; // expected-warning {{unsequenced modification and access}}

  (xs[3] || (a = 0)) + a; // ok
  (0 || (a = 0)) + a; // expected-warning {{unsequenced modification and access}}
  (1 || (a = 0)) + a; // ok

  (xs[4] ? a : ++a) + a; // ok
  (0 ? a : ++a) + a; // expected-warning {{unsequenced modification and access}}
  (1 ? a : ++a) + a; // ok
  (0 ? a : a++) + a; // expected-warning {{unsequenced modification and access}}
  (1 ? a : a++) + a; // ok
  (xs[5] ? ++a : ++a) + a; // FIXME: warn here

  (++a, xs[6] ? ++a : 0) + a; // expected-warning {{unsequenced modification and access}}

  // Here, the read of the fourth 'a' might happen before or after the write to
  // the second 'a'.
  a += (a++, a) + a; // expected-warning {{unsequenced modification and access}}

  int *p = xs;
  a = *(a++, p); // ok
  a = a++ && a; // ok

  A *q = &agg1;
  (q = &agg2)->y = q->x; // expected-warning {{unsequenced modification and access to 'q'}}

  // This has undefined behavior if a == 0; otherwise, the side-effect of the
  // increment is sequenced before the value computation of 'f(a, a)', which is
  // sequenced before the value computation of the '&&', which is sequenced
  // before the assignment. We treat the sequencing in '&&' as being
  // unconditional.
  a = a++ && f(a, a);

  // This has undefined behavior if a != 0. FIXME: We should diagnose this.
  (a && a++) + a;

  (xs[7] && ++a) * (!xs[7] && ++a); // ok

  xs[0] = (a = 1, a); // ok
  (a -= 128) &= 128; // ok
  ++a += 1; // ok

  xs[8] ? ++a + a++ : 0; // expected-warning {{multiple unsequenced modifications}}
  xs[8] ? 0 : ++a + a++; // expected-warning {{multiple unsequenced modifications}}
  xs[8] ? ++a : a++; // ok

  xs[8] && (++a + a++); // expected-warning {{multiple unsequenced modifications}}
  xs[8] || (++a + a++); // expected-warning {{multiple unsequenced modifications}}

  (__builtin_classify_type(++a) ? 1 : 0) + ++a; // ok
  (__builtin_constant_p(++a) ? 1 : 0) + ++a; // ok
  (__builtin_object_size(&(++a, a), 0) ? 1 : 0) + ++a; // ok
  (__builtin_expect(++a, 0) ? 1 : 0) + ++a; // expected-warning {{multiple unsequenced modifications}}
}
