// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// Make sure we are generating branches instead of selects.

// CHECK: @main
[RootSignature("")]
float4 main(float4 color : COLOR) : SV_Target
{
    int value = 0;
    
    // CHECK: br i1
    if (color.x < 0.5)
        value = 1;
        // CHECK: br
 
    // CHECK: br i1
    if (color.y < 0.5)
        value = 2;
        // CHECK: br 
        
    // CHECK: br i1
    if (color.z < 0.5)
        value = 3;
        // CHECK: br
        
    // CHECK: br i1
    if (color.w < 0.5)
        value = 4;
        // CHECK: br
                        
    float4 result = float4(value,1,1,1);
  
    return result;
}
