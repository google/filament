// RUN: %dxc -E main -T cs_6_6 %s -D WAVESIZE=2 -verify
// RUN: %dxc -E main -T cs_6_6 %s -D WAVESIZE=13 -verify

[wavesize(WAVESIZE)] // expected-error{{WaveSize arguments must be between 4 and 128 and a power of 2}}
[numthreads(1,1,8)]
void main() {
}


[wavesize(4, 8)] 
[numthreads(1,1,8)] 
void inactive() {
}
