// RUN: %clang_cc1 -fsyntax-only -std=c++11 -verify %s
template<typename T>
class X0 {
  friend T;
};

class Y1 { };
enum E1 { };
X0<Y1> x0a;
X0<Y1 *> x0b;
X0<int> x0c;
X0<E1> x0d;

template<typename T>
class X1 {
  friend typename T::type; // expected-error{{no type named 'type' in 'Y1'}}
};

struct Y2 {
  struct type { };
};

struct Y3 {
  typedef int type;
};

X1<Y2> x1a;
X1<Y3> x1b;
X1<Y1> x1c; // expected-note{{in instantiation of template class 'X1<Y1>' requested here}}

template<typename T> class B;

template<typename T>
class A {
  T x;
public:
  class foo {};
  static int y;
  template <typename S> friend class B<S>::ty; // expected-warning {{dependent nested name specifier 'B<S>::' for friend class declaration is not supported}}
};

template<typename T> class B { typedef int ty; };

template<> class B<int> {
  class ty {
    static int f(A<int> &a) { return a.y; } // ok, befriended
  };
};
int f(A<char> &a) { return a.y; } // FIXME: should be an error

struct {
  // Ill-formed
  int friend; // expected-error {{'friend' must appear first in a non-function declaration}}
  unsigned friend int; // expected-error {{'friend' must appear first in a non-function declaration}}
  const volatile friend int; // expected-error {{'friend' must appear first in a non-function declaration}}
  int
          friend; // expected-error {{'friend' must appear first in a non-function declaration}}

  // OK
  int friend foo(void);
  friend int;
  friend const volatile int;
      friend

  float;
  template<typename T> friend class A<T>::foo; // expected-warning {{not supported}}
} a;

void testA() { (void)sizeof(A<int>); }
