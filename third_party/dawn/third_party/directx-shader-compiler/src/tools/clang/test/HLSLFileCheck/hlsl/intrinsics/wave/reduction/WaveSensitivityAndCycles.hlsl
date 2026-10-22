// RUN: %dxc -T lib_6_3 %s | FileCheck %s -input-file=stderr

// CHECK: 18:19: warning: Gradient operations are not affected by wave-sensitive data or control flow
// CHECK: 37:19: warning: Gradient operations are not affected by wave-sensitive data or control flow
// CHECK: 49:12: warning: Gradient operations are not affected by wave-sensitive data or control flow
// CHECK: 50:12: warning: Gradient operations are not affected by wave-sensitive data or control flow

export
float main1(float dep : DEPTH) : SV_TARGET {
  // Loop creates cycle in dependency graph
  while( dep < 100.0 )
    dep += 1.0;
  // wave-sensitive and gradiant ops to prevent disabling sensitivity search
  float wave = QuadReadLaneAt( dep, 0 );
  float grad = ddx( dep );

  // Gradiant that depends on wave op
  return ddy(wave + grad);
}

export
float main2(float dep0 : DEPTH0, float dep1 : DEPTH0) : SV_TARGET {
  // Double loop creates cycle in dependency graph
  // and also phis at the start that depend on blocks with branches
  // that depend on cycle dependant values to get entered
  while( dep0 < 100.0 ) {
    while( dep1 < 100.0 ) {
      dep0 += 1.0;
      dep1 += 1.0;
    }
  }
  // wave-sensitive and gradiant ops to prevent disabling sensitivity search
  float wave = QuadReadLaneAt( dep1, 0 );
  float grad = ddx( dep0 );

  // Gradiant that dep0ends on wave op to produce warning
  return ddy(wave + grad);
}

export
float main3(float dep0 : DEPTH0, float dep1 : DEPTH0) : SV_TARGET {
  // Double loop creates cycle in dependency graph
  // and also phis at the start that depend on blocks with branches
  // that depend on cycle dependant values to get entered
  while( dep0 < 100.0 ) {
    while( dep1 < 100.0 ) {
      // Backward dependant Gradiant that depends on wave op
      // These are in a block that was entirely unvisited previously
      dep0 += ddx(dep0);
      dep1 += ddy(dep1);
    }
    dep0 = QuadReadLaneAt( dep0, 0);
    dep1 = QuadReadLaneAt( dep1, 0);
  }
  return dep0 + dep1;
}
