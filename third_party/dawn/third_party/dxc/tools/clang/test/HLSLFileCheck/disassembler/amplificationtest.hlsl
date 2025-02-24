// RUN: %dxc /T as_6_6 /E ASMain %s | FileCheck %s
// CHECK: Amplification Shader
// CHECK: NumThreads=(2,4,1)
 

struct payloadStruct
{ 
    uint myArbitraryData; 
}; 
 
[numthreads(2,4,1)] 
void ASMain (in uint3 groupID : SV_GroupID)    
{ 
    payloadStruct p; 
    p.myArbitraryData = 3; 
    DispatchMesh(1,1,1,p);
}