// RUN: %dxc -Tlib_6_3 -verify -HV 2021 %s
// RUN: %dxc -Tvs_6_0 -verify -HV 2021 %s

template<typename T>
struct Foo {
  T foo(Texture2D<vector<T, 4> > tex) {   /* expected-error {{'MyStruct' cannot be used as a type parameter where a scalar is required}} */
    return tex[uint2(0,0)].x;
  }
};

struct MyStruct {
  float f;
};

[shader("vertex")]
void main() {
  Foo<MyStruct> foo;                      /* expected-note {{in instantiation of template class 'Foo<MyStruct>' requested here}} */
}
