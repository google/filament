// RUN: %dxc -T ps_6_0 -E PSMain -fcgl  %s -verify

static const integral_constant MyVar; // expected-error{{unknown type name 'integral_constant'}}
static const SpirvType MyVar; // expected-error{{unknown type name 'SpirvType'}}
static const SpirvOpaqueType MyVar; // expected-error{{unknown type name 'SpirvOpaqueType'}}
static const Literal MyVar; // expected-error{{unknown type name 'Literal'}}

float4 PSMain() : SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}

