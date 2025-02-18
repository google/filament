#version 310 es
precision highp float;
precision highp int;

layout(location = 2) in float tint_interstage_location2;
void main_inner(float none) {
}
void main() {
  main_inner(tint_interstage_location2);
}
