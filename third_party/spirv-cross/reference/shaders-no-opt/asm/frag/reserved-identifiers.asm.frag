#version 450

layout(location = 0) out vec4 _RESERVED_IDENTIFIER_FIXUP_spvFoo;
layout(location = 1) out vec4 SPIRV_Cross_blah;
layout(location = 2) out vec4 _40Bar;
layout(location = 3) out vec4 _m40;
layout(location = 4) out vec4 _underscore_foo_bar_meep_;

void main()
{
    _RESERVED_IDENTIFIER_FIXUP_spvFoo = vec4(0.0);
    SPIRV_Cross_blah = vec4(1.0);
    _40Bar = vec4(2.0);
    _m40 = vec4(3.0);
    _underscore_foo_bar_meep_ = vec4(4.0);
}

