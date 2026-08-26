// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// Make sure global used for init is not removed.
// CHECK:float c;
// CHECK:float a;
// CHECK:float d;
// CHECK:int e;

float c;
float a;

struct S {
  float x;
  float b;
};

static S s = {c, a};

float d;
static uint cast = d;

int e;
static int init = e;

float main() : SV_Target {
  return s.x + s.b + cast + init;
}