// RUN: %dxc -spirv -T lib_6_5 -E AnyHit1 -fspv-target-env=vulkan1.3 -Wno-effects-syntax -fcgl %s | FileCheck %s

// CHECK-NOT: error
struct DataType
{
	uint u;
	bool b;
};

static DataType data;

ByteAddressBuffer buffer : register ( t4 , space3 )  ; 

struct PayloadShadow_t 
{ 
    float m_flVisibility ; 
} ; 

[ shader ( "anyhit" ) ] 
void AnyHit1 ( inout PayloadShadow_t payload , in BuiltInTriangleIntersectionAttributes attrs ) 
{
	data = buffer.Load<DataType>( 0 );
} 
