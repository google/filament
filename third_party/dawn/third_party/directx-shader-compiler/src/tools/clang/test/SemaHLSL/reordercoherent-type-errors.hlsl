// RUN: %dxc -Tlib_6_9 -verify %s

reordercoherent RWTexture1D<float4> uav1 : register(u3);

// expected-error@+2 {{'reordercoherent' is not a valid modifier for a declaration of type 'Buffer<vector<float, 4> >'}}
// expected-note@+1 {{'reordercoherent' can only be applied to UAV objects}}
reordercoherent Buffer<float4> srv;

// expected-error@+2 {{'reordercoherent' is not a valid modifier for a declaration of type 'float'}}
// expected-note@+1 {{'reordercoherent' can only be applied to UAV objects}}
reordercoherent float m;

reordercoherent RWTexture2D<float> tex[12];
reordercoherent RWTexture2D<float> texMD[12][12];

// expected-error@+2 {{'reordercoherent' is not a valid modifier for a declaration of type 'float'}}
// expected-note@+1 {{'reordercoherent' can only be applied to UAV objects}}
reordercoherent float One() {
  return 1.0;
}

struct Record { uint index; };

// expected-error@+2 {{'reordercoherent' is not a valid modifier for a declaration of type 'RWDispatchNodeInputRecord<Record>'}}
// expected-note@+1 {{'reordercoherent' can only be applied to UAV objects}}
void func2(reordercoherent RWDispatchNodeInputRecord<Record> funcInputData) {}
