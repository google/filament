#version 450
		
struct VertexOutput
{
    vec4 HPosition;
};

struct TestStruct
{
    vec3 position;
    float radius;
};

layout(binding = 0, std140) uniform CB0
{
    TestStruct CB0[16];
} _24;

layout(location = 0) out vec4 _entryPointOutput;

vec4 _main(VertexOutput IN)
{
    TestStruct st;
    st.position = _24.CB0[1].position;
    st.radius = _24.CB0[1].radius;
    vec4 col = vec4(st.position, st.radius);
    return col;
}

void main()
{
    VertexOutput IN;
    IN.HPosition = gl_FragCoord;
    VertexOutput param = IN;
    _entryPointOutput = _main(param);
}
