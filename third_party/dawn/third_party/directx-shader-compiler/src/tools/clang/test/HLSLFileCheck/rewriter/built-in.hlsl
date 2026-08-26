// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// make sure build-in functions not crash.
// CHECK:min

float a;
float b;
float main() : SV_Target {
  return min(a, b);
}