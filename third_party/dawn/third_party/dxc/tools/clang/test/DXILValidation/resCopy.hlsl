RWBuffer<uint> uav1;
RWBuffer<uint> uav2;

[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
    RWBuffer<uint> u = uav1;
    u[GI] = u[GI] + 1;
    u = uav2;
    u[GI] = GI+1;
}
