// RUN: %dxc -T lib_6_9 %s -verify

struct Payload {
    float value : write(caller) : read(caller);
};

[shader("raygeneration")]
void RayGen()
{
    RayDesc ray;
    const Payload p = {0.0};
    dx::HitObject obj = dx::HitObject::MakeMiss(0, 0, ray);
    dx::HitObject::Invoke(obj, p); // expected-error{{no matching function for call to 'Invoke'}}
}