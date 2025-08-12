// RUN: %dxc -Tlib_6_3 -HV 2021 -verify %s
// RUN: %dxc -Tcs_6_0 -HV 2021 -verify %s

ByteAddressBuffer In;
RWBuffer<float> Out;

template <typename T>
struct Foo {
  // expected-note@+1{{'RWBuffer<float>' field declared here}}
  T Member;
};

template <typename T>
struct MyTemplate {
  T GetValue(ByteAddressBuffer srv, uint offset) {
    // expected-error@+2{{Explicit template arguments on intrinsic Load must be a single numeric type}}
    // expected-error@+1{{object 'RWBuffer<float>' is not allowed in builtin template parameters}}
    return srv.Load<T>(offset);
  }
};
template <typename T>
T GetValue(uint offset) {
  MyTemplate<T> myTemplate;
  // expected-error@+2{{scalar, vector, or matrix expected}}
  // expected-note@+1{{in instantiation of member function 'MyTemplate<RWBuffer<float> >::GetValue' requested here}}
  return myTemplate.GetValue(In, offset) +
  // expected-error@+2{{Explicit template arguments on intrinsic Load must be a single numeric type}}
  // expected-error@+1{{object 'RWBuffer<float>' is not allowed in builtin template parameters}}
         In.Load<Foo<T> >(offset + 4).Member;
}

// expected-note@+1{{forward declaration of 'Incomplete'}}
struct Incomplete;

[shader("compute")]
[numthreads(1,1,1)]
void main()
{ 
  RWBuffer<float> FB = In.Load<RWBuffer<float> >(0);
  // expected-error@-1{{Explicit template arguments on intrinsic Load must be a single numeric type}}
  // expected-error@-2{{object 'RWBuffer<float>' is not allowed in builtin template parameters}}

  Out[0] = FB[0];

  // Ok:
  Out[4] = GetValue<float>(4);
  
  // expected-note@?{{'Load' declared here}}
  // expected-error@+1{{calling 'Load' with incomplete return type 'Incomplete'}}
  Out[8] = In.Load<Incomplete>(8);

  // expected-note@+1 2 {{in instantiation of function template specialization 'GetValue<RWBuffer<float> >' requested here}}
  RWBuffer<float> FB2 = GetValue<RWBuffer<float> >(16);
}
