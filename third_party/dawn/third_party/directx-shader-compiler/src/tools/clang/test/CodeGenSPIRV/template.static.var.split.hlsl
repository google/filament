// RUN: %dxc -T cs_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-NOT: OpVariable

template<typename T>
struct MyClass {
  const static T array[2];
};

template<typename T> const static T MyClass<T>::array[2] = { 1, 2 };

[numthreads(1, 1, 1)]
void main() {
}
