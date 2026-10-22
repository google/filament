// RUN: %dxc -Tlib_6_8 -verify %s
// RUN: %dxc -Tcs_6_0 -ECCMain -verify %s

[shader("compute")] 
[shader("compute")] 
[ numthreads( 64, 2, 2 ) ]  /* expected-no-diagnostics */
void CCMain() {
}

[shader("node")] 
[shader("node")] 
[nodedispatchgrid(8,1,1)]
[ numthreads( 64, 2, 2 ) ]  /* expected-no-diagnostics */
void NNMain() {
}
