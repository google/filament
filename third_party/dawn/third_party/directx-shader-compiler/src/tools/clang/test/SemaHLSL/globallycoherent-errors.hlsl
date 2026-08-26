// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tps_6_0 -verify %s

globallycoherent RWTexture1D<float4> uav1 : register(u3);
RWTexture1D<float4> uav2;

// expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'Buffer<vector<float, 4> >'}}
// expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
globallycoherent Buffer<float4> srv;

// expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'float'}}
// expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
globallycoherent float m;

globallycoherent RWTexture2D<float> tex[12];
globallycoherent RWTexture2D<float> texMD[12][12];

// expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'float'}}
// expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
globallycoherent float One() {
  return 1.0;
}

struct Record { uint index; };

void func2(globallycoherent RWDispatchNodeInputRecord<Record> funcInputData) {}
void func(RWDispatchNodeInputRecord<Record> funcInputData) {
  // expected-warning@+1 {{implicit conversion from 'RWDispatchNodeInputRecord<Record>' to 'globallycoherent RWDispatchNodeInputRecord<Record>' adds globallycoherent annotation}}
  func2(funcInputData);
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1,1,1)]
void node(globallycoherent RWDispatchNodeInputRecord<Record> inputData) {
  // expected-warning@+1 {{implicit conversion from 'globallycoherent RWDispatchNodeInputRecord<Record>' to 'RWDispatchNodeInputRecord<Record>' loses globallycoherent annotation}}
  RWDispatchNodeInputRecord<Record> localData = inputData;
  // expected-warning@+1 {{implicit conversion from 'globallycoherent RWDispatchNodeInputRecord<Record>' to 'RWDispatchNodeInputRecord<Record>' loses globallycoherent annotation}}
  func(inputData);
}

[shader("pixel")]
 float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  globallycoherent  RWTexture1D<float4> uav3 = uav1;

  // expected-warning@+1 {{implicit conversion from 'RWTexture1D<float4>' to 'globallycoherent RWTexture1D<float4>' adds globallycoherent annotation}}
  uav3 = uav2;

  // expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'float'}}
  // expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
  globallycoherent float x = 3;

  uav3[0] = srv[0];
  uav1[0] = 2;
  uav2[1] = 3;
  return 0;
}
