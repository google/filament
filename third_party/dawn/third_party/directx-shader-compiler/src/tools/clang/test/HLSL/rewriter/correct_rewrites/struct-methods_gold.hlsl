// Rewrite unchanged result:
struct MyTestStruct {
  uint4 data[2];
  uint getData1()   {
    return uint(this.data[1].z);
  }


  float3 getDataAsFloat()   {
    return float3(asfloat(this.data[0].x), asfloat(this.data[0].y), asfloat(this.data[0].z));
  }


};
