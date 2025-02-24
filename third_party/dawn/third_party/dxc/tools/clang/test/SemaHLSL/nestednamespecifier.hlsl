// RUN: %dxc -T cs_6_0 -E main -HV 2021 -verify %s

// expected-no-diagnostics

template <typename T>
class C {
  C cast();
};

template <class T>
C<T>
C<T>::cast() {
  C result;
  return result;
}

[numthreads(64, 1, 1)] void main() {}
