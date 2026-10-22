#version 110

#extension GL_EXT_texture_array : enable

uniform sampler1DArray s1DA;
uniform sampler2DArray s2DA;
uniform sampler1DArrayShadow s1DAS;
uniform sampler2DArrayShadow s2DAS;

void foo()
{
    float f;
    vec2 v2;
    vec3 v3;
    vec4 v4;

    v4 = texture1DArray(s1DA, v2);
    v4 = texture2DArray(s2DA, v3);
    v4 = shadow1DArray(s1DAS, v3);
    v4 = shadow2DArray(s2DAS, v4);

    v4 = texture1DArray(s1DA, v2, f);
    v4 = texture2DArray(s2DA, v3, f);
    v4 = shadow1DArray(s1DAS, v3, f);
	
    v4 = texture1DArrayLod(s1DA, v2, f);
    v4 = texture2DArrayLod(s2DA, v3, f);
    v4 = shadow1DArrayLod(s1DAS, v3, f);
}

void main()
{
    foo();
}
