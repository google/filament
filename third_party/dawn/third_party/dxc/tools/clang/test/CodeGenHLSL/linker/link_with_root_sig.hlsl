



#define     RS \
    RootSignature\
    (\
       "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)"\
    )

struct Vertex
{
    float4 position     : POSITION0;
    float4 color        : COLOR0;
};

struct Interpolants
{
    float4 position : SV_POSITION0;
    float4 color    : COLOR0;
};


[shader("vertex")]
[RS]
Interpolants vs_main( Vertex In )
{
    return In;
}