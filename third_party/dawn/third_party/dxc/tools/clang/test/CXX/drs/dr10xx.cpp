// RUN: %clang_cc1 -std=c++98 %s -verify -fexceptions -fcxx-exceptions -pedantic-errors
// RUN: %clang_cc1 -std=c++11 %s -verify -fexceptions -fcxx-exceptions -pedantic-errors
// RUN: %clang_cc1 -std=c++14 %s -verify -fexceptions -fcxx-exceptions -pedantic-errors
// RUN: %clang_cc1 -std=c++1z %s -verify -fexceptions -fcxx-exceptions -pedantic-errors

// expected-no-diagnostics

namespace std {
  __extension__ typedef __SIZE_TYPE__ size_t;

  template<typename T> struct initializer_list {
    const T *p; size_t n;
    initializer_list(const T *p, size_t n);
  };
}

namespace dr1048 { // dr1048: 3.6
  struct A {};
  const A f();
  A g();
  typedef const A CA;
#if __cplusplus >= 201103L
  // ok: we deduce non-const A in each case.
  A &&a = [] (int n) {
    while (1) switch (n) {
      case 0: return f();
      case 1: return g();
      case 2: return A();
      case 3: return CA();
    }
  } (0);
#endif
}

namespace dr1070 { // dr1070: 3.5
#if __cplusplus >= 201103L
  struct A {
    A(std::initializer_list<int>);
  };
  struct B {
    int i;
    A a;
  };
  B b = {1};
  struct C {
    std::initializer_list<int> a;
    B b;
    std::initializer_list<double> c;
  };
  C c = {};
#endif
}
