#version 310 es
precision highp float;
precision highp int;


struct S {
  int m;
};

S A[4] = S[4](S(0), S(0), S(0), S(0));
void main() {
  A[0u] = S(1);
}
