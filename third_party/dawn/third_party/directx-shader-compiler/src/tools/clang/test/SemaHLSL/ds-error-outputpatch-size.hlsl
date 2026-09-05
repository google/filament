// RUN: %dxc -T ds_6_0 -E main -verify %s

struct ControlPoint {
  float position : MY_BOOL;
};

ControlPoint main(const OutputPatch<ControlPoint, 0> patch) { // expected-error {{OutputPatch element count must be greater than 0}}
  return patch[0];
}

