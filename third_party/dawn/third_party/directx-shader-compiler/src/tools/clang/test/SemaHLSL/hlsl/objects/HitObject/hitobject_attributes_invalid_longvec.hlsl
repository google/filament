// RUN: %dxc -T lib_6_9 -E main %s -verify

struct
CustomAttrs {
  vector<float, 32> v;
  int y;
};

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  // expected-error@+2{{vectors of over 4 elements in attributes are not supported}}
  CustomAttrs attrs;
  hit.GetAttributes(attrs);
}
