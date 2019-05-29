#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0, std140) uniform Buff
{
    mediump uint TestVal;
} _15;

layout(location = 0) out vec4 fsout_Color;

void main()
{
    fsout_Color = vec4(1.0);
    switch (_15.TestVal)
    {
        case 0u:
        {
            fsout_Color = vec4(0.100000001490116119384765625);
            break;
        }
        case 1u:
        {
            fsout_Color = vec4(0.20000000298023223876953125);
            break;
        }
    }
}

