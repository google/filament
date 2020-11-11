#version 450

layout(binding = 0, std140) uniform type_gCBuffarrayIndex
{
    uint gArrayIndex;
} gCBuffarrayIndex;

uniform sampler2D SPIRV_Cross_Combinedg_textureArray0SPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_Combinedg_textureArray1SPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_Combinedg_textureArray2SPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_Combinedg_textureArray3SPIRV_Cross_DummySampler;

layout(location = 0) out vec4 out_var_SV_TARGET;

vec4 _32;

void main()
{
    vec4 _80;
    do
    {
        vec4 _77;
        bool _78;
        switch (gCBuffarrayIndex.gArrayIndex)
        {
            case 0u:
            {
                _77 = texelFetch(SPIRV_Cross_Combinedg_textureArray0SPIRV_Cross_DummySampler, ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), 0).xy, 0);
                _78 = true;
                break;
            }
            case 1u:
            {
                _77 = texelFetch(SPIRV_Cross_Combinedg_textureArray1SPIRV_Cross_DummySampler, ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), 0).xy, 0);
                _78 = true;
                break;
            }
            case 2u:
            {
                _77 = texelFetch(SPIRV_Cross_Combinedg_textureArray2SPIRV_Cross_DummySampler, ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), 0).xy, 0);
                _78 = true;
                break;
            }
            case 3u:
            {
                _77 = texelFetch(SPIRV_Cross_Combinedg_textureArray3SPIRV_Cross_DummySampler, ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), 0).xy, 0);
                _78 = true;
                break;
            }
            default:
            {
                _77 = _32;
                _78 = false;
                break;
            }
        }
        if (_78)
        {
            _80 = _77;
            break;
        }
        _80 = vec4(0.0, 1.0, 0.0, 1.0);
        break;
    } while(false);
    out_var_SV_TARGET = _80;
}

