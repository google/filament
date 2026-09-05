// RUN: %dxc -T vs_6_5 -E main -verify %s

// tests that RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS usage will emit
// one warning for each incompatible availability attribute decl,
// when the compilation target is less than shader model 6.9.

namespace MyNamespace {
  // expected-warning@+1{{potential misuse of built-in constant 'RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model vs_6_5; introduced in shader model 6.9}}
  static const int badVar = RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS;
}

// expected-warning@+1{{potential misuse of built-in constant 'RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model vs_6_5; introduced in shader model 6.9}}
groupshared const int otherBadVar = RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS;

int retNum(){
  // expected-warning@+1{{potential misuse of built-in constant 'RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model vs_6_5; introduced in shader model 6.9}}
  return RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS;
}

int retNumUncalled(){
  // no diagnostic expected here
  return RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS;
}

RaytracingAccelerationStructure RTAS;
void main(uint i : IDX, RayDesc rayDesc : RAYDESC) {

  int x = MyNamespace::badVar + otherBadVar + retNum();
  RayQuery<0> rayQuery0a;

  if (x > 4){
    rayQuery0a.TraceRayInline(RTAS, 8, 2, rayDesc);
  }
  else{
    rayQuery0a.TraceRayInline(RTAS, 16, 2, rayDesc);
  }

  // expected-error@+2{{A non-zero value for the RayQueryFlags template argument requires shader model 6.9 or above.}}
  // expected-warning@+1{{potential misuse of built-in constant 'RAY_FLAG_FORCE_OMM_2_STATE' in shader model vs_6_5; introduced in shader model 6.9}}
  RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, 1> rayQuery0b;

  // expected-warning@+2{{potential misuse of built-in constant 'RAY_FLAG_FORCE_OMM_2_STATE' in shader model vs_6_5; introduced in shader model 6.9}}
  // expected-warning@+1{{potential misuse of built-in constant 'RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model vs_6_5; introduced in shader model 6.9}}
  RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS> rayQuery0d;

}
