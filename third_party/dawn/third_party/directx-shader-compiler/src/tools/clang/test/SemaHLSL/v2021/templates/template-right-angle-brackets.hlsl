// RUN: %dxc -T lib_6_3 -HV 2021 -verify %s

// In HLSL 2021, two consecutive right angle brackets (>>) in a template
// argument list must be separated by a space. Using >> without spaces should
// produce an error.

template<typename T>
struct Outer { T val; };

template<typename T>
struct Inner { T val; };

// Nested template type using >> without space: should be an error in HLSL 2021.
Outer<Inner<float>> g_nested; // expected-error {{a space is required between consecutive right angle brackets (use '> >')}}

template<typename T>
struct Wrapper {
  Outer<Inner<T>> field; // expected-error {{a space is required between consecutive right angle brackets (use '> >')}}
};

// Function return type with nested template and >>.
Outer<Inner<int>> getVal(); // expected-error {{a space is required between consecutive right angle brackets (use '> >')}}

// Function parameter with nested template and >>.
void takeVal(Outer<Inner<int>> v); // expected-error {{a space is required between consecutive right angle brackets (use '> >')}}

// Three right angle brackets (>>>) in a template should also error.
template<typename T, typename U>
struct Pair { T first; U second; };

Outer<Pair<float, Inner<int>>> g_triple; // expected-error {{a space is required between consecutive right angle brackets (use '> >')}}
