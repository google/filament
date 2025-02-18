#version 310 es
precision highp float;
precision highp int;


struct In {
  float none;
  float member_1;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
  float perspective_default;
  float linear_default;
};

layout(location = 0) in float tint_interstage_location0;
layout(location = 1) flat in float tint_interstage_location1;
layout(location = 2) in float tint_interstage_location2;
layout(location = 3) centroid in float tint_interstage_location3;
layout(location = 4) in float tint_interstage_location4;
layout(location = 5) in float tint_interstage_location5;
layout(location = 6) centroid in float tint_interstage_location6;
layout(location = 7) in float tint_interstage_location7;
layout(location = 8) in float tint_interstage_location8;
layout(location = 9) in float tint_interstage_location9;
void main_inner(In v) {
}
void main() {
  main_inner(In(tint_interstage_location0, tint_interstage_location1, tint_interstage_location2, tint_interstage_location3, tint_interstage_location4, tint_interstage_location5, tint_interstage_location6, tint_interstage_location7, tint_interstage_location8, tint_interstage_location9));
}
