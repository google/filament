struct MyTestStruct
{
  uint4 data[2];

  uint getData1() { 
    return uint (data[1].z); 
  }

  float3 getDataAsFloat() { 
    return float3 (asfloat(data[0].x), asfloat(data[0].y), asfloat(data[0].z)); 
  }

};