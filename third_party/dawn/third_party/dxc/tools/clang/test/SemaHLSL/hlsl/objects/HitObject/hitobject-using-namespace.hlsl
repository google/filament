// RUN: %dxc -T lib_6_9 %s -verify

// This test checks that HitObject can be used with 'using namespace dx' instead of explicit namespace prefix
// expected-no-diagnostics

struct [raypayload] Payload
{
    float elem
          : write(caller,anyhit,closesthit,miss)
          : read(caller,anyhit,closesthit,miss);
};

struct Attribs { float2 barys; };

using namespace dx;

[shader("raygeneration")]
void main()
{
  HitObject hit;
  MaybeReorderThread(hit);
}

[shader("closesthit")]
void closestHit(inout Payload pld, in Attribs attrs)
{
  // Create a HitObject
  HitObject hit;
}

[shader("miss")]
void missShader(inout Payload pld)
{
  // Also test using a static method
  HitObject hit = HitObject::MakeNop();
} 
