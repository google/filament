// RUN: %dxc -Tps_6_0 -verify %s

// expected-error@+1 {{overloading non-member 'operator==' is not allowed}}
bool operator==(int lhs, int rhs) {
    return true;
}

struct A {
  float a;
  int b;
};

// expected-error@+1 {{overloading non-member 'operator>' is not allowed}}
bool operator>(A a0, A a1) {
  return a1.a > a0.a && a1.b > a0.b;
}
// expected-error@+1 {{overloading non-member 'operator==' is not allowed}}
bool operator==(A a0, int i) {
    return a0.b == i;
}
// expected-error@+1 {{overloading non-member 'operator<' is not allowed}}
bool operator<(A a0, float f) {
   return a0.a < f;
}
// expected-error@+1 {{overloading 'operator++' is not allowed}}
A operator++(A a0) {
  a0.a++;
  a0.b++;
  return a0;
}

void main() {}