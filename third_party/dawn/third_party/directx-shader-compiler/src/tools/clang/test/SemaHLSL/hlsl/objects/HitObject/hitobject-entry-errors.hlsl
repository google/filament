// RUN: %dxc -T lib_6_9 %s -verify

struct [raypayload] Payload
{
    float elem
          : write(caller,anyhit,closesthit,miss)
          : read(caller,anyhit,closesthit,miss);
};

struct Attribs { float2 barys; };

dx::HitObject UseHitObject() {
  return dx::HitObject::MakeNop();
}

// expected-note@+3{{entry function defined here}}
[shader("compute")]
[numthreads(4,4,4)]
void mainHitCS(uint ix : SV_GroupIndex, uint3 id : SV_GroupThreadID) {
// expected-error@-7{{dx::HitObject is unavailable in shader stage 'compute' (requires 'raygeneration', 'closesthit' or 'miss')}}
  UseHitObject();
}

// expected-note@+2{{entry function defined here}}
[shader("callable")]
void mainHitCALL(inout Attribs attrs) {
// expected-error@-14{{dx::HitObject is unavailable in shader stage 'callable' (requires 'raygeneration', 'closesthit' or 'miss')}}
  UseHitObject();
}

// expected-note@+2{{entry function defined here}}
[shader("intersection")]
void mainHitIS() {
// expected-error@-21{{dx::HitObject is unavailable in shader stage 'intersection' (requires 'raygeneration', 'closesthit' or 'miss')}}
  UseHitObject();
}

// expected-note@+2{{entry function defined here}}
[shader("anyhit")]
void mainHitAH(inout Payload pld, in Attribs attrs) {
// expected-error@-28{{dx::HitObject is unavailable in shader stage 'anyhit' (requires 'raygeneration', 'closesthit' or 'miss')}}
  UseHitObject();
}

[shader("raygeneration")]
void mainHitRG() {
  UseHitObject();
}

[shader("closesthit")]
void mainHitCH(inout Payload pld, in Attribs attrs) {
  UseHitObject();
}

[shader("miss")]
void mainHitMS(inout Payload pld) {
  UseHitObject();
}
