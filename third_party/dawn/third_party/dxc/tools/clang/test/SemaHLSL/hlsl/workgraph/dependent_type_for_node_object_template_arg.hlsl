// RUN: %dxc -Tlib_6_8 -verify %s

// GroupNodeInputRecords may not have zero size record on DependentType.
// GroupNodeInputRecords may not non-struct/class DependentType. 

template<typename T>
struct Record {  }; // expected-note{{zero sized record defined here}}

template <typename T> void foo(GroupNodeInputRecords<T> data) {}

template <typename T> void bar() {
  GroupNodeInputRecords<Record<T> > data; // expected-error{{record used in GroupNodeInputRecords may not have zero size}}
  foo(data);
}

template <typename T> void bar2() {
  GroupNodeInputRecords<T > data; // expected-error{{'float' is not valid as a node record type - struct/class required}}
  foo(data);
}

void test() {
  bar<float>(); // expected-note{{in instantiation of function template specialization 'bar<float>' requested here}}
  bar2<float>();// expected-note{{in instantiation of function template specialization 'bar2<float>' requested here}}
}

template <typename T>
class CFoo {
  T var;
  void foo(GroupNodeInputRecords<T> data) {} // expected-error{{'int' is not valid as a node record type - struct/class required}}
};

void testCFoo() {
  CFoo<int> c; // expected-note{{in instantiation of template class 'CFoo<int>' requested here}}
}


template <typename T>
class CBar {
  T var;
  void bar(GroupNodeInputRecords< Texture2D<T> > data) {} // expected-error{{'Texture2D<float>' is not valid as a node record type - struct/class required}}
};

void testCBar() {
  CBar<float> c; // expected-note{{in instantiation of template class 'CBar<float>' requested here}}
}

struct ZeroSize {};

template<typename T>
struct RecordTemplate { T Value; }; // expected-note{{zero sized record defined here}}

void oopsie() {
  GroupNodeInputRecords<RecordTemplate<ZeroSize> > data; // expected-error{{record used in GroupNodeInputRecords may not have zero size}}
  foo(data);
}

void woo() {
  GroupNodeInputRecords<RecordTemplate<int> > data;
  foo(data);
}

template<typename T>
struct ForwardDecl; // expected-note{{template is declared here}}

void woot() {
  GroupNodeInputRecords<ForwardDecl<int> > data; // expected-error{{implicit instantiation of undefined template 'ForwardDecl<int>'}}
  foo(data);
}

template<typename T>
struct ForwardDecl { T Val; };
