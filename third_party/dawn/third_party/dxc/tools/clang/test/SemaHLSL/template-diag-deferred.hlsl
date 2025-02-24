// RUN: %dxc -Tlib_6_6 -HV 2021 -verify %s

template<typename T> void neverInstantiated(uint2 pos) {
   globallycoherent T Val = 0.0f;
}

template<typename T> void doSomething(uint2 pos) {
   globallycoherent RWTexture2D<T> output = ResourceDescriptorHeap[0];
   globallycoherent T Val = 0.0f; // expected-error {{'globallycoherent' is not a valid modifier for a declaration of type 'float'}} expected-note {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
   output[pos] = Val;
}

template<typename T> void doSomething2(uint2 pos) {
   globallycoherent T output = ResourceDescriptorHeap[0];
   output[pos] = 0.0f;
}

void Fn() {
  // expected-note@+1{{in instantiation of function template specialization 'doSomething<float>' requested here}}
  doSomething<float>(uint2(0,0));
  doSomething2<RWTexture2D<float> >(uint2(0,0));
}
