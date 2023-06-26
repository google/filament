#version 450

uniform sampler2D SPIRV_Cross_CombinedSampledImageSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_CombinedSampledImageSampler;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    ivec2 _154 = ivec3(int(gl_FragCoord.x * 1280.0), int(gl_FragCoord.y * 720.0), 0).xy;
    _entryPointOutput = ((texelFetch(SPIRV_Cross_CombinedSampledImageSPIRV_Cross_DummySampler, _154, 0) + texelFetch(SPIRV_Cross_CombinedSampledImageSPIRV_Cross_DummySampler, _154, 0)) + texture(SPIRV_Cross_CombinedSampledImageSampler, gl_FragCoord.xy)) + texture(SPIRV_Cross_CombinedSampledImageSampler, gl_FragCoord.xy);
}

