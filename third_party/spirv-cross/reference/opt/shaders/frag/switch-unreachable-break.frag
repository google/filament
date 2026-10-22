#version 450

layout(binding = 0, std140) uniform UBO
{
    int cond;
    int cond2;
} _13;

layout(location = 0) out vec4 FragColor;

void main()
{
    bool _49;
    switch (_13.cond)
    {
        case 1:
        {
            if (_13.cond2 < 50)
            {
                _49 = false;
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
            _49 = true;
            break;
        }
    }
    FragColor = mix(vec4(20.0), vec4(10.0), bvec4(_49));
}

