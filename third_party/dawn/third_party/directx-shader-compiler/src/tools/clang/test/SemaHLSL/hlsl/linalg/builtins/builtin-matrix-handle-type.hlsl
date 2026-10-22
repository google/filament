// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -verify %s

__builtin_LinAlgMatrix global_handle;

static __builtin_LinAlgMatrix static_handle;

groupshared __builtin_LinAlgMatrix gs_handle;

__builtin_LinAlgMatrix array[2];

cbuffer CB {
  __builtin_LinAlgMatrix cb_handle;
};

struct S {
  __builtin_LinAlgMatrix handle;
};

S s;

void f1(__builtin_LinAlgMatrix m);

__builtin_LinAlgMatrix f2();

// expected-error@+1 {{typedef redefinition with different types ('int' vs '__builtin_LinAlgMatrix')}}
typedef int __builtin_LinAlgMatrix;

[shader("compute")]
[numthreads(4,1,1)]
void main() {
  __builtin_LinAlgMatrix m;

  m = s.handle;

  f1(m);
  m = f2();

  // expected-error@+1 {{numeric type expected}}
  m++;
  
  // expected-error@+1 {{numeric type expected}}
  m = m * 2;

  // expected-error@+1 {{numeric type expected}}
  bool a = m && s.handle;
}
