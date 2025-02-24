// RUN: %dxc -Tlib_6_3 -HV 2021 -verify %s
// RUN: %dxc -Tcs_6_0 -HV 2021 -verify %s

ByteAddressBuffer In;
RWBuffer<float> Out;

[shader("compute")]
[numthreads(1,1,1)]
void main()
{ 
  RWBuffer<float> FB = In.Load<RWBuffer<float> >(0); // expected-error {{Explicit template arguments on intrinsic Load must be a single numeric type}}
  Out[0] = FB[0];
}
