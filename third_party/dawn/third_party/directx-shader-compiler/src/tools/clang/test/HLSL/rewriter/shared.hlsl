shared cbuffer cb0 {
    float X;
};

shared tbuffer tb0 {
    float Y;
};

shared Buffer<int> g_intBuffer;
shared RWByteAddressBuffer g_byteBuffer;

shared Texture1D<double> g_tex1d;
shared Texture1DArray<int> g_tex1dArray;
shared Texture2D<float> g_tex2d;
shared Texture2DArray<float> g_tex2dArray;
shared Texture2DMS<half> g_texture2dms;
shared Texture2DMSArray<float> g_texture2dmsArray;

float main() : SV_Target {
    return -X + Y;
}