#version 450

layout(binding = 0, std140) uniform UBO
{
    int cond;
    int cond2;
} _13;

layout(location = 0) out vec4 FragColor;

void main()
{
    bool frog = false;
    switch (_13.cond)
    {
        case 1:
        {
            if (_13.cond2 < 50)
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
    FragColor = mix(vec4(20.0), vec4(10.0), bvec4(frog));
}

