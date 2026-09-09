// RUN: %dxc -T lib_6_9 %s -verify

struct
[raypayload]
Payload
{
    int a : read(closesthit, miss) : write(anyhit);
    dx::HitObject hit;
};

struct
[raypayload]
PayloadLV
{
    int a : read(closesthit, miss) : write(anyhit);
    vector<float, 5> b : read(closesthit, miss) : write(anyhit);
};

[shader("raygeneration")]
void RayGen()
{
  // expected-error@+3{{payload parameter 'payload_in_rg' must be a user-defined type composed of only numeric types}}
  // expected-error@+2{{object 'dx::HitObject' is not allowed in payload parameters}}
  // expected-note@8{{'dx::HitObject' field declared here}}
  Payload payload_in_rg;
  dx::HitObject::Invoke( dx::HitObject(), payload_in_rg );

  // expected-error@+1{{vectors of over 4 elements in payload parameters are not supported}}
  PayloadLV payload_with_lv;
  dx::HitObject::Invoke( dx::HitObject(), payload_with_lv );
}