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
  // expected-error@+1{{use of undeclared identifier 'MaybeReorderThread'}}
  MaybeReorderThread(1);
}

[shader("closesthit")]
void closestHit(inout Payload pld, in Attribs attrs)
{
  // expected-error@+1{{use of undeclared identifier 'MaybeReorderThread'}}
  MaybeReorderThread(2);
}

[shader("miss")]
void missShader(inout Payload pld)
{
  // expected-error@+1{{use of undeclared identifier 'MaybeReorderThread'}}
  MaybeReorderThread(3);
} 
