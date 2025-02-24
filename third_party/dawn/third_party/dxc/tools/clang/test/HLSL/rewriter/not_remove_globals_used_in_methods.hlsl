RWBuffer<uint> u;

struct ST
{
	void Store(uint i, uint v)
	{
	  u[i] = v;
	}
};

[numthreads(8, 8, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
  ST s;
  s.Store(id.x,id.y);
}
