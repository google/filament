// RUN: %clang_cc1 -fsyntax-only -verify %s
template<typename T, int N = 2> struct X; // expected-note{{template is declared here}}

X<int, 1> *x1;
X<int> *x2;

X<> *x3; // expected-error{{too few template arguments for class template 'X'}}

template<typename U = float, int M> struct X;

X<> *x4;

template<typename T = int> struct Z { };
template struct Z<>;

// PR4362
template<class T> struct a { };
template<> struct a<int> { static const bool v = true; };

template<class T, bool = a<T>::v> struct p { }; // expected-error {{no member named 'v'}}

template struct p<bool>; // expected-note {{in instantiation of default argument for 'p<bool>' required here}}
template struct p<int>;

// PR5187
template<typename T, typename U>
struct A;

template<typename T, typename U = T>
struct A;

template<typename T, typename U>
struct A {
  void f(A<T>);
};

template<typename T>
struct B { };

template<>
struct B<void> {
  typedef B<void*> type;
};

// Nested default arguments for template parameters.
template<typename T> struct X1 { };

template<typename T>
struct X2 {
  template<typename U = typename X1<T>::type> // expected-error{{no type named 'type' in 'X1<int>'}} \
                                              // expected-error{{no type named 'type' in 'X1<char>'}}
  struct Inner1 { }; // expected-note{{template is declared here}}
  
  template<T Value = X1<T>::value> // expected-error{{no member named 'value' in 'X1<int>'}} \
                                   // expected-error{{no member named 'value' in 'X1<char>'}}
  struct NonType1 { }; // expected-note{{template is declared here}}
  
  template<T Value>
  struct Inner2 { };
  
  template<typename U>
  struct Inner3 {
    template<typename X = T, typename V = U>
    struct VeryInner { };
    
    template<T Value1 = sizeof(T), T Value2 = sizeof(U), 
             T Value3 = Value1 + Value2>
    struct NonType2 { };
  };
};

X2<int> x2i; // expected-note{{in instantiation of template class 'X2<int>' requested here}}
X2<int>::Inner1<float> x2iif;

X2<int>::Inner1<> x2bad; // expected-error{{too few template arguments for class template 'Inner1'}}

X2<int>::NonType1<'a'> x2_nontype1;
X2<int>::NonType1<> x2_nontype1_bad; // expected-error{{too few template arguments for class template 'NonType1'}}

// Check multi-level substitution into template type arguments
X2<int>::Inner3<float>::VeryInner<> vi;
X2<char>::Inner3<int>::NonType2<> x2_deep_nontype; // expected-note{{in instantiation of template class 'X2<char>' requested here}}

template<typename T, typename U>
struct is_same { static const bool value = false; };

template<typename T>
struct is_same<T, T> { static const bool value = true; };

int array1[is_same<__typeof__(vi), 
               X2<int>::Inner3<float>::VeryInner<int, float> >::value? 1 : -1];

int array2[is_same<__typeof(x2_deep_nontype),
                   X2<char>::Inner3<int>::NonType2<sizeof(char), sizeof(int), 
                                    sizeof(char)+sizeof(int)> >::value? 1 : -1];

// Template template parameter defaults
template<template<typename T> class X = X2> struct X3 { };
int array3[is_same<X3<>, X3<X2> >::value? 1 : -1];

struct add_pointer {
  template<typename T>
  struct apply {
    typedef T* type;
  };
};

template<typename T, template<typename> class X = T::template apply>
  struct X4;
int array4[is_same<X4<add_pointer>, 
                   X4<add_pointer, add_pointer::apply> >::value? 1 : -1];

template<int> struct X5 {}; // expected-note{{has a different type 'int'}}
template<long> struct X5b {};
template<typename T, 
         template<T> class B = X5> // expected-error{{template template argument has different}} \
                                   // expected-note{{previous non-type template parameter}}
  struct X6 {};

X6<int> x6a;
X6<long> x6b; // expected-note{{while checking a default template argument}}
X6<long, X5b> x6c;


template<template<class> class X = B<int> > struct X7; // expected-error{{must be a class template}}

namespace PR9643 {
  template<typename T> class allocator {};
  template<typename T, typename U = allocator<T> > class vector {};

  template<template<typename U, typename = allocator<U> > class container,
           typename DT>
  container<DT> initializer(const DT& d) {
    return container<DT>();
  }

  void f() {
    vector<int, allocator<int> > v = initializer<vector>(5);
  }
}

namespace PR16288 {
  template<typename X>
  struct S {
    template<typename T = int, typename U> // expected-warning {{C++11}}
    void f();
  };
  template<typename X>
  template<typename T, typename U>
  void S<X>::f() {}
}

namespace DR1635 {
  template <class T> struct X {
    template <class U = typename T::type> static void f(int) {} // expected-error {{type 'int' cannot be used prior to '::' because it has no members}} \
                                                                // expected-warning {{C++11}}
    static void f(...) {}
  };

  int g() { X<int>::f(0); } // expected-note {{in instantiation of template class 'DR1635::X<int>' requested here}}
}
