// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Sin

float main ( uint mask:M, float a:A) : SV_Target 
{ 
   float r = a;
   mask = WaveActiveBitOr ( mask ) ;
    if (mask & 0xf) {
       r += sin(r);
    }
    
    float dd = ddx(a);
    return r + dd; 
}