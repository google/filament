// RUN: %dxc -T ps_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"
// CHECK: OpExtension "SPV_GOOGLE_user_type"

// CHECK: OpDecorateString %a UserTypeGOOGLE "structuredbuffer:<float>"
StructuredBuffer<float> a;
// CHECK: OpDecorateString %b UserTypeGOOGLE "rwstructuredbuffer:<float>"
RWStructuredBuffer<float> b;
// CHECK: OpDecorateString %c UserTypeGOOGLE "appendstructuredbuffer:<float>"
AppendStructuredBuffer<float> c;
// CHECK: OpDecorateString %d UserTypeGOOGLE "consumestructuredbuffer:<float>"
ConsumeStructuredBuffer<float> d;
// CHECK: OpDecorateString %e UserTypeGOOGLE "texture1d:<float>"
Texture1D<float> e;
// CHECK: OpDecorateString %f UserTypeGOOGLE "texture2d:<float>"
Texture2D<float> f;
// CHECK: OpDecorateString %g UserTypeGOOGLE "texture3d:<float>"
Texture3D<float> g;
// CHECK: OpDecorateString %h UserTypeGOOGLE "texturecube:<float>"
TextureCube<float> h;
// CHECK: OpDecorateString %i UserTypeGOOGLE "texture1darray:<float>"
Texture1DArray<float> i;
// CHECK: OpDecorateString %j UserTypeGOOGLE "texture2darray:<float>"
Texture2DArray<float> j;
// CHECK: OpDecorateString %k UserTypeGOOGLE "texture2dms:<float>"
Texture2DMS<float> k;
// CHECK: OpDecorateString %l UserTypeGOOGLE "texture2dmsarray:<float>"
Texture2DMSArray<float> l;
// CHECK: OpDecorateString %m UserTypeGOOGLE "texturecubearray:<float>"
TextureCubeArray<float> m;
// CHECK: OpDecorateString %n UserTypeGOOGLE "rwtexture1d:<float>"
RWTexture1D<float> n;
// CHECK: OpDecorateString %o UserTypeGOOGLE "rwtexture2d:<float>"
RWTexture2D<float> o;
// CHECK: OpDecorateString %p UserTypeGOOGLE "rwtexture3d:<float>"
RWTexture3D<float> p;
// CHECK: OpDecorateString %q UserTypeGOOGLE "rwtexture1darray:<float>"
RWTexture1DArray<float> q;
// CHECK: OpDecorateString %r UserTypeGOOGLE "rwtexture2darray:<float>"
RWTexture2DArray<float> r;
// CHECK: OpDecorateString %s UserTypeGOOGLE "buffer:<float>"
Buffer<float> s;
// CHECK: OpDecorateString %t UserTypeGOOGLE "rwbuffer:<float>"
RWBuffer<float> t;
// CHECK: OpDecorateString %u UserTypeGOOGLE "texture2dmsarray:<float4,64>"
Texture2DMSArray<float4, 64> u;
// CHECK: OpDecorateString %v UserTypeGOOGLE "texture2dmsarray:<float4,64>"
const Texture2DMSArray<float4, 64> v;
// CHECK: OpDecorateString %t1 UserTypeGOOGLE "texture1d"
Texture1D t1;
// CHECK: OpDecorateString %t2 UserTypeGOOGLE "texture2d"
Texture2D t2;

// CHECK: OpDecorateString %eArr UserTypeGOOGLE "texture1d:<float>"
Texture1D<float> eArr[5];
// CHECK: OpDecorateString %fArr UserTypeGOOGLE "texture2d:<float>"
Texture2D<float> fArr[5];
// CHECK: OpDecorateString %gArr UserTypeGOOGLE "texture3d:<float>"
Texture3D<float> gArr[5];
// CHECK: OpDecorateString %hArr UserTypeGOOGLE "texturecube:<float>"
TextureCube<float> hArr[5];
// CHECK: OpDecorateString %iArr UserTypeGOOGLE "texture1darray:<float>"
Texture1DArray<float> iArr[5];
// CHECK: OpDecorateString %jArr UserTypeGOOGLE "texture2darray:<float>"
Texture2DArray<float> jArr[5];
// CHECK: OpDecorateString %kArr UserTypeGOOGLE "texture2dms:<float>"
Texture2DMS<float> kArr[5];
// CHECK: OpDecorateString %lArr UserTypeGOOGLE "texture2dmsarray:<float>"
Texture2DMSArray<float> lArr[5];
// CHECK: OpDecorateString %mArr UserTypeGOOGLE "texturecubearray:<float>"
TextureCubeArray<float> mArr[5];
// CHECK: OpDecorateString %nArr UserTypeGOOGLE "rwtexture1d:<float>"
RWTexture1D<float> nArr[5];
// CHECK: OpDecorateString %oArr UserTypeGOOGLE "rwtexture2d:<float>"
RWTexture2D<float> oArr[5];
// CHECK: OpDecorateString %pArr UserTypeGOOGLE "rwtexture3d:<float>"
RWTexture3D<float> pArr[5];
// CHECK: OpDecorateString %qArr UserTypeGOOGLE "rwtexture1darray:<float>"
RWTexture1DArray<float> qArr[5];
// CHECK: OpDecorateString %rArr UserTypeGOOGLE "rwtexture2darray:<float>"
RWTexture2DArray<float> rArr[5];
// CHECK: OpDecorateString %sArr UserTypeGOOGLE "buffer:<float>"
Buffer<float> sArr[5];
// CHECK: OpDecorateString %tArr UserTypeGOOGLE "rwbuffer:<float>"
RWBuffer<float> tArr[5];

// CHECK: OpDecorateString %MyCBuffer UserTypeGOOGLE "cbuffer"
cbuffer MyCBuffer { float x; };

// CHECK: OpDecorateString %MyTBuffer UserTypeGOOGLE "tbuffer"
tbuffer MyTBuffer { float y; };

// CHECK: OpDecorateString %bab UserTypeGOOGLE "byteaddressbuffer"
ByteAddressBuffer bab;

// CHECK: OpDecorateString %rwbab UserTypeGOOGLE "rwbyteaddressbuffer"
RWByteAddressBuffer rwbab;

// CHECK: OpDecorateString %rs UserTypeGOOGLE "raytracingaccelerationstructure"
RaytracingAccelerationStructure rs;

struct S {
    float  f1;
    float3 f2;
};

// CHECK: OpDecorateString %cb UserTypeGOOGLE "constantbuffer:<S>"
ConstantBuffer<S> cb;

// CHECK: OpDecorateString %tb UserTypeGOOGLE "texturebuffer:<S>"
TextureBuffer<S> tb;

float4 main() : SV_Target{
    return 0.0.xxxx;
}
