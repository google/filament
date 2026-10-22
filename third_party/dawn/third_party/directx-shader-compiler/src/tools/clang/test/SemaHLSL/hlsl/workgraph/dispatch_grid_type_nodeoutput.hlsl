// RUN: %dxc -Tlib_6_8 -enable-16bit-types -verify %s

template<typename T>
struct OutputRecord {

// expected-error@+48{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'float'}}
// expected-error@+47{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<float, 3>'}}
// expected-error@+46{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'int'}}
// expected-error@+45{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int, 3>'}}
// expected-error@+44{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<unsigned int, 4>'}}
// expected-error@+43{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'unsigned int [4]'}}
// expected-error@+42{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'half'}}
// expected-error@+41{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<half, 3>'}}
// expected-error@+40{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'int16_t'}}
// expected-error@+39{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int16_t, 3>'}}
// expected-error@+38{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<uint16_t, 4>'}}
// expected-error@+37{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'uint16_t [4]'}}
// expected-error@+36{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int16_t, 4>'}}
// expected-error@+35{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'matrix<int16_t, 2, 2>'}}
// expected-error@+34{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'S<int>'}}
// expected-error@+33{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'S<float>'}}
// expected-error@+32{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'float'}}
// expected-error@+31{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<float, 3>'}}
// expected-error@+30{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'int'}}
// expected-error@+29{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int, 3>'}}
// expected-error@+28{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<unsigned int, 4>'}}
// expected-error@+27{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'unsigned int [4]'}}
// expected-error@+26{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'half'}}
// expected-error@+25{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<half, 3>'}}
// expected-error@+24{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'int16_t'}}
// expected-error@+23{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int16_t, 3>'}}
// expected-error@+22{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<uint16_t, 4>'}}
// expected-error@+21{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'uint16_t [4]'}}
// expected-error@+20{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int16_t, 4>'}}
// expected-error@+19{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'matrix<int16_t, 2, 2>'}}
// expected-error@+18{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'S<int>'}}
// expected-error@+17{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'S<float>'}}
// expected-error@+16{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'float'}}
// expected-error@+15{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<float, 3>'}}
// expected-error@+14{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'int'}}
// expected-error@+13{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int, 3>'}}
// expected-error@+12{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<unsigned int, 4>'}}
// expected-error@+11{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'unsigned int [4]'}}
// expected-error@+10{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'half'}}
// expected-error@+9{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<half, 3>'}}
// expected-error@+8{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'int16_t'}}
// expected-error@+7{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int16_t, 3>'}}
// expected-error@+6{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<uint16_t, 4>'}}
// expected-error@+5{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'uint16_t [4]'}}
// expected-error@+4{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'vector<int16_t, 4>'}}
// expected-error@+3{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'matrix<int16_t, 2, 2>'}}
// expected-error@+2{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'S<int>'}}
// expected-error@+1{{SV_DispatchGrid can only be applied to a declaration of unsigned 32 or 16-bit integer scalar, vector, or array up to 3 elements, not 'S<float>'}}
  T grid : SV_DispatchGrid;
  float data;
};

#define OUTPUT_NODE_ENTRY(T,N) \
[Shader("node")] \
[NodeLaunch("coalescing")] \
[NumThreads(32, 1, 1)] \
void node_output_C_N(NodeOutput<OutputRecord< T > > output) {}\
[Shader("node")] \
[NodeLaunch("broadcasting")] \
[NodeDispatchGrid(1,1,1)] \
[NumThreads(32, 1, 1)] \
void node_output_B_N(NodeOutput<OutputRecord< T > > output) {}\
[Shader("node")] \
[NodeLaunch("thread")] \
void node_output_T_N(NodeOutput<OutputRecord< T > > output) {}

// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(float, 0);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(float3, 1);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(int, 2);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(int3, 3);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(uint4, 4);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(uint[4], 5);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(half, 6);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(half3, 7);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(int16_t, 8);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(int16_t3, 9);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(uint16_t4, 10);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(uint16_t[4], 11);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(int16_t4, 12);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(int16_t2x2, 13);
OUTPUT_NODE_ENTRY(uint16_t3 , 14);
OUTPUT_NODE_ENTRY(uint16_t[3] , 15);

template<typename T>
struct S {
  T a;
};
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(S<int> , 16);
// expected-note@+3{{NodeInput/Output record defined here}}
// expected-note@+2{{NodeInput/Output record defined here}}
// expected-note@+1{{NodeInput/Output record defined here}}
OUTPUT_NODE_ENTRY(S<float> , 17);
