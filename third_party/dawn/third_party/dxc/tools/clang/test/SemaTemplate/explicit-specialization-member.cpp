// RUN: %clang_cc1 -fsyntax-only -verify %s
template<typename T>
struct X0 {
  typedef T* type;
  
  void f0(T);
  void f1(type);
};

template<> void X0<char>::f0(char);
template<> void X0<char>::f1(type);

namespace PR6161 {
  template<typename _CharT>
  class numpunct : public locale::facet // expected-error{{use of undeclared identifier 'locale'}} \
              // expected-error{{expected class name}}
  {
    static locale::id id; // expected-error{{use of undeclared identifier}}
  };
  numpunct<char>::~numpunct();
}

namespace PR12331 {
  template<typename T> struct S {
    struct U { static const int n = 5; };
    enum E { e = U::n }; // expected-note {{implicit instantiation first required here}}
    int arr[e];
  };
  template<> struct S<int>::U { static const int n = sizeof(int); }; // expected-error {{explicit specialization of 'U' after instantiation}}
}

namespace PR18246 {
  template<typename T>
  class Baz {
  public:
    template<int N> void bar();
  };

  template<typename T>
  template<int N>
  void Baz<T>::bar() { // expected-note {{couldn't infer template argument 'N'}}
  }

  // FIXME: We shouldn't try to match this against a prior declaration if
  // template parameter matching failed.
  template<typename T>
  void Baz<T>::bar<0>() { // expected-error {{cannot specialize a member of an unspecialized template}} \
                          // expected-error {{no function template matches}}
  }
}

namespace PR19340 {
template<typename T> struct Helper {
  template<int N> static void func(const T *m) {} // expected-note {{failed template argument deduction}}
};

template<typename T> void Helper<T>::func<2>() {} // expected-error {{cannot specialize a member}} \
                                                  // expected-error {{no function template matches}}
}
