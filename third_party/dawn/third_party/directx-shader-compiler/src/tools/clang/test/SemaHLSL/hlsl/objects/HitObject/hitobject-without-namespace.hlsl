// RUN: %dxc -T lib_6_9 %s -verify

struct [raypayload] Payload
{
    float elem
          : write(caller,anyhit,closesthit,miss)
          : read(caller,anyhit,closesthit,miss);
};

struct Attribs { float2 barys; };

[shader("raygeneration")]
void main()
{
  // expected-error@+1{{unknown type name 'HitObject'}}
  HitObject hit;
}

[shader("closesthit")]
void closestHit(inout Payload pld, in Attribs attrs)
{
  // expected-error@+1{{unknown type name 'HitObject'}}
  HitObject hit;
}

[shader("miss")]
void missShader(inout Payload pld)
{
  // expected-error@+1{{unknown type name 'HitObject'}}
  HitObject hit;
}

// Also test API methods
[shader("raygeneration")]
void main2()
{
  // expected-error@+1{{use of undeclared identifier 'HitObject'}}
  HitObject::MakeNop();
} 