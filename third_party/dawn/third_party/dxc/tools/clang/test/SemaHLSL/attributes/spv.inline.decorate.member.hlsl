// REQUIRES: spirv
// RUN: %dxc -T ps_6_0 -E main -verify -spirv %s

struct S
{
    [[vk::ext_decorate_id(/*offset*/ 35, 0)]] float4 f1; /* expected-error{{'ext_decorate_id' attribute only applies to functions, variables, parameters, and types}} */
    [[vk::ext_decorate_string(/*offset*/ 35, "16")]] float4 f2; /* expected-error{{'ext_decorate_string' attribute only applies to functions, variables, parameters, and types}} */
};

float4 main() : SV_TARGET
{

}
