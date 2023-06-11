#version 450

uniform sampler2D SPIRV_Cross_CombinedparamSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_CombinedSampledImageSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_CombinedparamSampler;
uniform sampler2D SPIRV_Cross_CombinedSampledImageSampler;

layout(location = 0) out vec4 _entryPointOutput;

vec4 sample_fetch(ivec3 UV, sampler2D SPIRV_Cross_CombinedtexSPIRV_Cross_DummySampler)
{
    return texelFetch(SPIRV_Cross_CombinedtexSPIRV_Cross_DummySampler, UV.xy, UV.z);
}

vec4 sample_sampler(vec2 UV, sampler2D SPIRV_Cross_CombinedtexSampler)
{
    return texture(SPIRV_Cross_CombinedtexSampler, UV);
}

vec4 _main(vec4 xIn)
{
    ivec3 coord = ivec3(int(xIn.x * 1280.0), int(xIn.y * 720.0), 0);
    ivec3 param = coord;
    vec4 value = sample_fetch(param, SPIRV_Cross_CombinedparamSPIRV_Cross_DummySampler);
    value += texelFetch(SPIRV_Cross_CombinedSampledImageSPIRV_Cross_DummySampler, coord.xy, coord.z);
    vec2 param_1 = xIn.xy;
    value += sample_sampler(param_1, SPIRV_Cross_CombinedparamSampler);
    value += texture(SPIRV_Cross_CombinedSampledImageSampler, xIn.xy);
    return value;
}

void main()
{
    vec4 xIn = gl_FragCoord;
    vec4 param = xIn;
    _entryPointOutput = _main(param);
}

