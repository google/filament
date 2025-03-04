// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// CHECK: error: ISO C++ forbids forward references to 'enum' types

template<typename T>
struct X {
  enum E e;
};

