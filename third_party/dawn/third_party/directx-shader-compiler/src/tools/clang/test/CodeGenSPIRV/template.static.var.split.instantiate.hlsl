// RUN: %dxc -T cs_6_8 -E main -fcgl %s -spirv | FileCheck %s


template<typename T>
struct MyClass {
  const static T array[2];
};

// CHECK: %array = OpVariable %_ptr_Private__arr_int_uint_2 Private
// CHECK: [[init_var:%[a-zA-Z0-9_]+]] = OpCompositeConstruct %_arr_int_uint_2 %int_1 %int_2
// CHECK: OpStore %array [[init_var]]
template<typename T> const static T MyClass<T>::array[2] = { 1, 2 };

[numthreads(1, 1, 1)]
void main() {
// CHECK: [[ptr:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_int %array %int_1
// CHECK: [[tmp:%[a-zA-Z0-9_]+]] = OpLoad %int [[ptr]]
// CHECK:                          OpStore %tmp [[tmp]]
  int tmp = MyClass<int>::array[1];
}

