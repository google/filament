// RUN: %dxc -Tlib_6_3 -HV 2018 -Wno-unused-value -verify %s

void test() {
  BuiltInTrianglePositions positions;
  positions.p0 = float3(0.0f, 0.0f, 0.0f);
  positions.p1 = float3(1.0f, 0.0f, 0.0f);
  positions.p2 = float3(0.0f, 1.0f, 0.0f);
  
  float3 p0 = positions.p0;
  float3 p1 = positions.p1;
  float3 p2 = positions.p2;
  
  // Test that it's a proper struct with the right fields
  float x = positions.p0.x;
  float y = positions.p1.y;
  float z = positions.p2.z;
}

// expected-no-diagnostics

