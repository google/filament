#version 450

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 10u
#endif
const uint s = SPIRV_CROSS_CONSTANT_ID_0;
const bool _13 = (s > 20u);
const uint f = _13 ? 30u : 50u;

layout(location = 0) out float FragColor;

void main()
{
    FragColor = float(f);
}

