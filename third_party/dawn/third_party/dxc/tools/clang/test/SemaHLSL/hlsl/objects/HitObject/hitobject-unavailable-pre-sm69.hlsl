// RUN: %dxc -T lib_6_8 %s -verify

// Check that the HitObject is unavailable pre SM 6.9.

[shader("raygeneration")]
void main() {
  // expected-error@+3{{intrinsic dx::HitObject::MakeNop potentially used by ''main'' requires shader model 6.9 or greater}}
  // expected-error@+2{{potential misuse of built-in type 'dx::HitObject' in shader model lib_6_8; introduced in shader model 6.9}}
  // expected-error@+1{{potential misuse of built-in type 'dx::HitObject' in shader model lib_6_8; introduced in shader model 6.9}}
  dx::HitObject hit = dx::HitObject::MakeNop();
}
