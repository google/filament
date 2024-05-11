cbuffer UBO : register(b0)
{
    int _13_cond : packoffset(c0);
    int _13_cond2 : packoffset(c0.y);
};


static float4 FragColor;
static float4 vInput;

struct SPIRV_Cross_Input
{
    float4 vInput : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    bool frog = false;
    switch (_13_cond)
    {
        case 1:
        {
            if (_13_cond2 < 50)
            {
                break;
            }
            else
            {
                discard;
            }
            break; // unreachable workaround
        }
        default:
        {
            frog = true;
            break;
        }
    }
    bool4 _45 = frog.xxxx;
    FragColor = float4(_45.x ? 10.0f.xxxx.x : 20.0f.xxxx.x, _45.y ? 10.0f.xxxx.y : 20.0f.xxxx.y, _45.z ? 10.0f.xxxx.z : 20.0f.xxxx.z, _45.w ? 10.0f.xxxx.w : 20.0f.xxxx.w);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vInput = stage_input.vInput;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
