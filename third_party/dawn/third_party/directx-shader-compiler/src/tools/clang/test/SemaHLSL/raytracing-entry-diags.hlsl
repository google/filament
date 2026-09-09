// RUN: %dxc -Tlib_6_3   -verify %s


[shader("anyhit")]
void anyhit_param0( inout RayDesc D1, RayDesc D2 ) { }

[shader("anyhit")]
void anyhit_param1( inout BuiltInTriangleIntersectionAttributes A1, BuiltInTriangleIntersectionAttributes A2 ) { }

// expected-error@+3{{payload parameter 'A1' must be a user-defined type composed of only numeric types}}
// expected-error@+2{{attributes parameter 'A2' must be a user-defined type composed of only numeric types}}
[shader("anyhit")]
void anyhit_param2( inout Texture2D A1, float4 A2 ) { }

// expected-error@+2{{payload parameter 'D1' must be 'inout'}}
[shader("anyhit")]
void anyhit_param3( RayDesc D1, RayDesc D2 ) { }

// expected-error@+2{{payload parameter 'D1' must be 'inout'}}
[shader("anyhit")]
void anyhit_param4( in RayDesc D1, RayDesc D2 ) { }

// expected-error@+2{{payload parameter 'D1' must be 'inout'}}
[shader("anyhit")]
void anyhit_param5( out RayDesc D1, RayDesc D2 ) { }

[shader("anyhit")]
void anyhit_param6( in out RayDesc D1, RayDesc D2 ) { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'anyhit': 0 parameter(s) provided, expected two parameters for payload and attributes}}
[shader("anyhit")]
void anyhit_param7() { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'anyhit': 1 parameter(s) provided, expected two parameters for payload and attributes}}
[shader("anyhit")]
void anyhit_param8( inout RayDesc D1) { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'anyhit': 3 parameter(s) provided, expected two parameters for payload and attributes}}
[shader("anyhit")]
void anyhit_param9( inout RayDesc D1, RayDesc D2, float f) { }

[shader("anyhit")]
float anyhit_param10( in out RayDesc D1, RayDesc D2 ) { } // expected-error{{return type for 'anyhit' shaders must be void}}

// expected-error@+2{{intersection attributes parameter 'D2' must be 'in'}}
[shader("anyhit")]
void anyhit_param11( inout RayDesc D1, out RayDesc D2 ) { }

// expected-error@+2{{intersection attributes parameter 'D2' must be 'in'}}
[shader("anyhit")]
void anyhit_param12( inout RayDesc D1, inout RayDesc D2 ) { }

// expected-error@+2{{payload parameter 'payload' must be 'inout'}}
[shader("closesthit")]
void closesthit_payload0( RayDesc payload, RayDesc attr ) {}

// expected-error@+2{{payload parameter 'payload' must be 'inout'}}
[shader("closesthit")]
void closesthit_payload1( in RayDesc payload, RayDesc attr ) {}

// expected-error@+2{{payload parameter 'payload' must be 'inout'}}
[shader("closesthit")]
void closesthit_payload2( out RayDesc payload, RayDesc attr ) {}

[shader("closesthit")]
void closesthit_payload3( inout RayDesc payload, RayDesc attr ) {}

[shader("closesthit")]
void closesthit_payload4( in out RayDesc payload, RayDesc attr ) {}

// expected-error@+3{{payload parameter 'f1' must be a user-defined type composed of only numeric types}}
// expected-error@+2{{attributes parameter 'f2' must be a user-defined type composed of only numeric types}}
[shader("closesthit")]
void closesthit_payload5( inout float f1, float f2 ) { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'closesthit': 0 parameter(s) provided, expected two parameters for payload and attributes}}
[shader("closesthit")]
void closesthit_payload6() { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'closesthit': 1 parameter(s) provided, expected two parameters for payload and attributes}}
[shader("closesthit")]
void closesthit_payload7( inout RayDesc D1) { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'closesthit': 3 parameter(s) provided, expected two parameters for payload and attributes}}
[shader("closesthit")]
void closesthit_payload8( inout RayDesc D1, RayDesc attr, float f) { }

[shader("closesthit")]
float closesthit_payload9( inout RayDesc payload, RayDesc attr ) {} // expected-error{{return type for 'closesthit' shaders must be void}}

// expected-error@+2{{intersection attributes parameter 'attr' must be 'in'}}
[shader("closesthit")]
void closesthit_payload10( inout RayDesc payload, out RayDesc attr ) {}

// expected-error@+2{{intersection attributes parameter 'attr' must be 'in'}}
[shader("closesthit")]
void closesthit_payload11( inout RayDesc payload, inout RayDesc attr ) {}

// expected-error@+2{{payload parameter 'payload' must be 'inout'}}
[shader("miss")]
void miss_payload0( RayDesc payload){}

// expected-error@+2{{payload parameter 'payload' must be 'inout'}}
[shader("miss")]
void miss_payload1( in RayDesc payload){}

// expected-error@+2{{payload parameter 'payload' must be 'inout'}}
[shader("miss")]
void miss_payload2( out RayDesc payload){}

[shader("miss")]
void miss_payload3( inout RayDesc payload){}

[shader("miss")]
void miss_payload4( in out RayDesc payload){}

// expected-error@+2{{payload parameter 'f1' must be a user-defined type composed of only numeric types}}
[shader("miss")]
void miss_payload5( inout float f1 ) { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'miss': 0 parameter(s) provided, expected one payload parameter}}
[shader("miss")]
void miss_payload6( ) { }

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'miss': 3 parameter(s) provided, expected one payload parameter}}
[shader("miss")]
void miss_payload7(inout RayDesc payload, float f1, float f2 ) { }

[shader("miss")]
float miss_payload8( inout RayDesc payload) { } // expected-error{{return type for 'miss' shaders must be void}}

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'intersection': 1 parameter(s) provided, expected no parameters}}
[shader("intersection")]
float intersection_param(float4 extra) // expected-error{{return type for 'intersection' shaders must be void}}
{
  return extra.x;
}

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'raygeneration': 1 parameter(s) provided, expected no parameters}}
[shader("raygeneration")]
float raygen_param(float4 extra) // expected-error{{return type for 'raygeneration' shaders must be void}}
{
  return extra.x;
}

struct MyPayload {
  float4 color;
  uint2 pos;
};

// expected-error@+2{{payload parameter 'payload' must be a user-defined type composed of only numeric types}}
[shader("miss")]
void miss_udt( inout PointStream<MyPayload> payload ) {}

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'callable': 0 parameter(s) provided, expected one argument parameter}}
[shader("callable")]
void callable0() {}

// expected-error@+2{{callable parameter 'V' must be a user-defined type composed of only numeric types}}
[shader("callable")]
void callable1(inout float4 V) {}

[shader("callable")]
void callable2(MyPayload payload) {} // expected-error{{callable parameter 'payload' must be 'inout'}}

[shader("callable")]
void callable3(in MyPayload payload) {} // expected-error{{callable parameter 'payload' must be 'inout'}}

[shader("callable")]
void callable4(out MyPayload payload) {} // expected-error{{callable parameter 'payload' must be 'inout'}}

[shader("callable")]
void callable5(in out MyPayload payload) {}

[shader("callable")]
void callable6(inout MyPayload payload) {}

// expected-error@+2{{incorrect number of entry parameters for raytracing stage 'callable': 2 parameter(s) provided, expected one argument parameter}}
[shader("callable")]
void callable7(inout MyPayload payload, float F) {}

[shader("callable")]
float callable8(inout MyPayload payload) {} // expected-error{{return type for 'callable' shaders must be void}}

// expected-note@+1 6 {{forward declaration of 'Incomplete'}}
struct Incomplete;

// expected-error@+3{{variable has incomplete type 'Incomplete'}}
// expected-error@+2{{variable has incomplete type '__restrict Incomplete'}}
[shader("anyhit")]
void anyhit_incomplete( inout Incomplete A1, Incomplete A2) { }

// expected-error@+3{{variable has incomplete type 'Incomplete'}}
// expected-error@+2{{variable has incomplete type '__restrict Incomplete'}}
[shader("closesthit")]
void closesthit_incomplete( inout Incomplete payload, Incomplete attr ) {}

// expected-error@+2{{variable has incomplete type '__restrict Incomplete'}}
[shader("miss")]
void miss_incomplete( inout Incomplete payload) { }

// expected-error@+2{{variable has incomplete type '__restrict Incomplete'}}
[shader("callable")]
void callable_incomplete(inout Incomplete payload) {}
