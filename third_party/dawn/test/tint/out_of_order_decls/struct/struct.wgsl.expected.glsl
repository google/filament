#version 310 es
precision highp float;
precision highp int;


struct S2 {
  int m;
};

struct S1 {
  S2 m;
};

void main() {
  S1 v = S1(S2(0));
}
