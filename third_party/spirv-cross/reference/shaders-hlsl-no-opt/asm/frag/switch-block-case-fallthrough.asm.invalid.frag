static int vIndex;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    nointerpolation int vIndex : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    int i = 0;
    int j;
    int _33;
    int _34;
    switch (vIndex)
    {
        case 0:
        {
            _33 = 3;
            j = _33;
            _34 = 0;
            j = _34;
            break;
        }
        default:
        {
            _33 = 2;
            j = _33;
            _34 = 0;
            j = _34;
            break;
        }
        case 1:
        case 11:
        {
            _34 = 1;
            j = _34;
            break;
        }
        case 2:
        {
            break;
        }
        case 3:
        {
            if (vIndex > 3)
            {
                i = 0;
                break;
            }
            else
            {
                break;
            }
        }
        case 4:
        {
            i = 0;
            break;
        }
        case 5:
        {
            i = 0;
            break;
        }
    }
    FragColor = float(i).xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
