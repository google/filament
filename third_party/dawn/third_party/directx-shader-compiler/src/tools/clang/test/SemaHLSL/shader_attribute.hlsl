// RUN: %dxc -Tlib_6_8 -verify %s

[shader("compute")] /* expected-error {{shader attribute type 'compute' conflicts with shader attribute type 'vertex'}} */
[shader("vertex")]  /* expected-note {{conflicting attribute is here}} */ 
[ numthreads( 64, 2, 2 ) ] 

void CVMain() {
}

[shader("vertex")] /* expected-error {{shader attribute type 'vertex' conflicts with shader attribute type 'pixel'}} */
[shader("pixel")]  /* expected-note {{conflicting attribute is here}} */ 
[ numthreads( 64, 2, 2 ) ] 
void VGMain() {
}

[shader("vertex")] /* expected-error {{shader attribute type 'vertex' conflicts with shader attribute type 'node'}} */
[shader("node")]   /* expected-note {{conflicting attribute is here}} */ 
[NodeDispatchGrid(2,1,1)]
[ numthreads( 64, 2, 2 ) ] 
void VNMain() {
}

[shader("compute")] /* expected-error {{shader attribute type 'compute' conflicts with shader attribute type 'node'}} */
[shader("vertex")]  /* expected-error {{shader attribute type 'vertex' conflicts with shader attribute type 'node'}} */ 
[shader("node")]    /* expected-note {{conflicting attribute is here}} */ /* expected-note {{conflicting attribute is here}} */ 
[NodeDispatchGrid(2,1,1)]
[ numthreads( 64, 2, 2 ) ] 
void CVNMain() {
}

[shader("mesh")]  /* expected-error {{shader attribute type 'mesh' conflicts with shader attribute type 'pixel'}} */
[shader("pixel")] /* expected-note {{conflicting attribute is here}} */
[ numthreads( 64, 2, 2 ) ]
void MPMain() {
}

[shader("compute")] 
[shader("vertex")]  /* expected-error {{shader attribute type 'vertex' conflicts with shader attribute type 'compute'}} */
[shader("compute")] /* expected-note {{conflicting attribute is here}} */
[ numthreads( 64, 2, 2 ) ]
void CVCMain() {
}

[shader("node")]   /* expected-error {{shader attribute type 'node' conflicts with shader attribute type 'pixel'}} */
[shader("vertex")] /* expected-error {{shader attribute type 'vertex' conflicts with shader attribute type 'pixel'}} */
[shader("pixel")]  /* expected-note {{conflicting attribute is here}} */  /* expected-note {{conflicting attribute is here}} */ 
void NVPMain() {
}

[shader("I'm invalid")]   /* expected-error {{attribute 'shader' must have one of these values: compute,vertex,pixel,hull,domain,geometry,raygeneration,intersection,anyhit,closesthit,miss,callable,mesh,amplification,node}} */
void InvalidMain() {
}
