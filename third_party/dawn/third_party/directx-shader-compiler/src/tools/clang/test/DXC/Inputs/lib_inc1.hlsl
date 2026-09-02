// Make sure include works for lib share compile.


#ifndef X

#include "lib_inc0.hlsl"

#include "lib_inc0_b.hlsl"

#endif

cbuffer B {
  float b;
}

#define M  6
