// RUN: %dxc -T ps_6_6 -HV 202x -verify %s


int someFunc() { return 7; }

float4 main(float4 pos : SV_Position) : SV_Target {
    // expected-error@+1 {{'auto' cannot deduce the type of 'ResourceDescriptorHeap'}}
    auto tex = ResourceDescriptorHeap[0];
    // expected-error@+1 {{'auto' cannot deduce the type of 'SamplerDescriptorHeap'}}
    auto samp = SamplerDescriptorHeap[0];

    // expected-error@+1 {{'auto' cannot deduce the type of 'ResourceDescriptorHeap'}}
    auto heap = ResourceDescriptorHeap;
    // expected-error@+1 {{'auto' cannot deduce the type of 'SamplerDescriptorHeap'}}
    auto sheap = SamplerDescriptorHeap;

    // expected-error@+1 {{'auto' cannot deduce the type of 'SamplerDescriptorHeap'}}
    auto pSamp = (SamplerDescriptorHeap[0]);
    // expected-error@+1 {{'auto' cannot deduce the type of 'ResourceDescriptorHeap'}}
    auto pHeap = (ResourceDescriptorHeap);

    // Negative case
    Texture2D<float4> tex2 = ResourceDescriptorHeap[1];
    auto sampled = tex2.Sample((SamplerState)SamplerDescriptorHeap[0], pos.xy);

    return float4(0, 0, 0, 0);
}
