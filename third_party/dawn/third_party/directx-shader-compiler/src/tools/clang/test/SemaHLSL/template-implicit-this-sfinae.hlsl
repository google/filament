// RUN: %dxc -T cs_6_0 %s 2>&1 | FileCheck %s --check-prefix=CHECK_COMPILE
// RUN: %dxc -T cs_6_0 -ast-dump %s 2>&1 | FileCheck %s

// Test that template instantiation of member functions with SFINAE patterns
// involving static const members (accessed via implicit 'this') does not
// assert or crash. In HLSL, 'this' is an lvalue reference (not a pointer),
// so CXXDependentScopeMemberExpr must store the class type T (not T*) as the
// base type to avoid HLSL's ban on pointer types during template instantiation.

namespace hlsl {
template <bool B, typename T> struct enable_if {};
template <typename T> struct enable_if<true, T> {
  using type = T;
};
template <typename T> struct is_arithmetic {
  static const bool value = false;
};
template <> struct is_arithmetic<float> {
  static const bool value = true;
};
template <> struct is_arithmetic<int> {
  static const bool value = true;
};
} // namespace hlsl

template <typename T>
struct Wrapper {
  static const bool IsArithmetic = hlsl::is_arithmetic<T>::value;

  // SFINAE on a static const bool member accessed via implicit 'this'.
  // This pattern previously triggered an assert when built with
  // LLVM_ENABLE_ASSERTIONS=ON because template instantiation tried to rebuild
  // the implicit 'this' pointer type, which is illegal in HLSL.
  template <typename U = T>
  typename hlsl::enable_if<IsArithmetic, U>::type Get() {
    return val;
  }

  T val;
};

// Verify that in the uninstantiated template the implicit 'this' is an lvalue
// of the class type (not a pointer), so HLSL's pointer ban is not violated.
// CHECK: CXXThisExpr {{.*}} 'Wrapper<T>' lvalue this

// Verify the same holds in the Wrapper<float> instantiation's Get method.
// CHECK: CXXThisExpr {{.*}} 'Wrapper<float>' lvalue this

// Verify the same holds in the Wrapper<int> instantiation's Get method.
// CHECK: CXXThisExpr {{.*}} 'Wrapper<int>' lvalue this

// Verify no CXXThisExpr carries a pointer type for 'this'.
// CHECK-NOT: CXXThisExpr {{.*}} '{{[^']*}} \*' lvalue this

// CHECK_COMPILE: define void @main
[numthreads(1, 1, 1)]
void main() {
  Wrapper<float> wf;
  wf.val = 1.0f;
  float f = wf.Get();

  Wrapper<int> wi;
  wi.val = 2;
  int i = wi.Get();
}
