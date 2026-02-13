// RUN: %dxc -T lib_6_9 -E main %s -verify

struct
CustomAttrs {
  vector<float, 32> v;
  RWStructuredBuffer<float> buf;
};

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  CustomAttrs attrs;
  hit.GetAttributes(attrs);
  // expected-error@-1{{vectors of over 4 elements in attributes are not supported}}
  // expected-error@-2{{attributes type must be a user-defined type composed of only numeric types}}
}
