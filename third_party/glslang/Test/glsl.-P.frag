#version 450

layout(location=0) out vec4 color;

void main()
{
    #ifndef TEST1
    #error TEST1 is not defined
    #endif

    #ifndef TEST2
    #error TEST2 is not defined
    #endif

    #ifndef TEST3
    #error TEST3 is not defined
    #endif

    color = vec4(1.0);
}
