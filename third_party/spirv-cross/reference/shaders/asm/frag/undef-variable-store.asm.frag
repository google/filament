#version 450

vec4 _48;
vec4 _31;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec4 _37;
    do
    {
        vec2 _35 = vec2(0.0);
        if (_35.x != 0.0)
        {
            _37 = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        }
        else
        {
            _37 = vec4(1.0, 1.0, 0.0, 1.0);
            break;
        }
        _37 = _48;
        break;
    } while (false);
    _entryPointOutput = _37;
}

