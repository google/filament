// RUN: %clang_cc1 -fsyntax-only -Wno-unused-value -verify -std=c++11 %s

enum E { e };

constexpr int id(int n) { return n; }

class C {

  int f() {
    int foo, bar;

    []; // expected-error {{expected body of lambda expression}}
    [+] {}; // expected-error {{expected variable name or 'this' in lambda capture list}}
    [foo+] {}; // expected-error {{expected ',' or ']' in lambda capture list}}
    [foo,&this] {}; // expected-error {{'this' cannot be captured by reference}}
    [&this] {}; // expected-error {{'this' cannot be captured by reference}}
    [&,] {}; // expected-error {{expected variable name or 'this' in lambda capture list}}
    [=,] {}; // expected-error {{expected variable name or 'this' in lambda capture list}}
    [] {}; 
    [=] (int i) {}; 
    [&] (int) mutable -> void {}; 
    [foo,bar] () { return 3; }; 
    [=,&foo] () {}; 
    [&,foo] () {}; 
    [this] () {}; 
    [] () -> class C { return C(); };
    [] () -> enum E { return e; };

    [] -> int { return 0; }; // expected-error{{lambda requires '()' before return type}}
    [] mutable -> int { return 0; }; // expected-error{{lambda requires '()' before 'mutable'}}
    [](int) -> {}; // PR13652 expected-error {{expected a type}}
    return 1;
  }

  void designator_or_lambda() {
    typedef int T; 
    const int b = 0; 
    const int c = 1;
    int d;
    int a1[1] = {[b] (T()) {}}; // expected-error{{no viable conversion from '(lambda}}
    int a2[1] = {[b] = 1 };
    int a3[1] = {[b,c] = 1 }; // expected-error{{expected ']'}} expected-note {{to match}}
    int a4[1] = {[&b] = 1 }; // expected-error{{integral constant expression must have integral or unscoped enumeration type, not 'const int *'}}
    int a5[3] = { []{return 0;}() };
    int a6[1] = {[this] = 1 }; // expected-error{{integral constant expression must have integral or unscoped enumeration type, not 'C *'}}
    int a7[1] = {[d(0)] { return d; } ()}; // expected-warning{{extension}}
    int a8[1] = {[d = 0] { return d; } ()}; // expected-warning{{extension}}
    int a9[1] = {[d = 0] = 1}; // expected-error{{is not an integral constant expression}}
    int a10[1] = {[id(0)] { return id; } ()}; // expected-warning{{extension}}
    int a11[1] = {[id(0)] = 1};
  }

  void delete_lambda(int *p) {
    delete [] p;
    delete [] (int*) { new int }; // ok, compound-literal, not lambda
    delete [] { return new int; } (); // expected-error{{expected expression}}
    delete [&] { return new int; } (); // ok, lambda
  }

  // We support init-captures in C++11 as an extension.
  int z;
  void init_capture() {
    [n(0)] () mutable -> int { return ++n; }; // expected-warning{{extension}}
    [n{0}] { return; }; // expected-error {{<initializer_list>}} expected-warning{{extension}} expected-warning{{will change meaning in a future version}}
    [n = 0] { return ++n; }; // expected-error {{captured by copy in a non-mutable}} expected-warning{{extension}}
    [n = {0}] { return; }; // expected-error {{<initializer_list>}} expected-warning{{extension}}
    [a([&b = z]{})](){}; // expected-warning 2{{extension}}

    int x = 4;
    auto y = [&r = x, x = x + 1]() -> int { // expected-warning 2{{extension}}
      r += 2;
      return x + 2;
    } ();
  }

  void attributes() {
    [] [[]] {}; // expected-error {{lambda requires '()' before attribute specifier}}
    [] __attribute__((noreturn)) {}; // expected-error {{lambda requires '()' before attribute specifier}}
    []() [[]]
      mutable {}; // expected-error {{expected body of lambda expression}}

    []() [[]] {};
    []() [[]] -> void {};
    []() mutable [[]] -> void {};
    []() mutable noexcept [[]] -> void {};

    // Testing GNU-style attributes on lambdas -- the attribute is specified
    // before the mutable specifier instead of after (unlike C++11).
    []() __attribute__((noreturn)) mutable { while(1); };
    []() mutable
      __attribute__((noreturn)) { while(1); }; // expected-error {{expected body of lambda expression}}
  }
};

template <typename>
void PR22122() {
  [](int) -> {}; // expected-error {{expected a type}}
}

template void PR22122<int>();

struct S {
  template <typename T>
  void m (T x =[0); // expected-error{{expected variable name or 'this' in lambda capture list}}
} s;

struct U {
  template <typename T>
  void m_fn1(T x = 0[0); // expected-error{{expected ']'}} expected-note{{to match this '['}}
} *U;
