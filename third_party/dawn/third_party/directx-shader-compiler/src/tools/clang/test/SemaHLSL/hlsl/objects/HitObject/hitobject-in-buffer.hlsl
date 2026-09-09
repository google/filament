// RUN: %dxc -T lib_6_9 %s -verify

// expected-error@+1{{object 'dx::HitObject' is not allowed in structured buffers}}
RWStructuredBuffer<dx::HitObject> InvalidBuffer;
