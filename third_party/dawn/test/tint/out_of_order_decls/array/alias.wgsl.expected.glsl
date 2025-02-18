#version 310 es
precision highp float;
precision highp int;

int A[4] = int[4](0, 0, 0, 0);
void main() {
  A[0u] = 1;
}
