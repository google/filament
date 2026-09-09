// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v4f1:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK: [[v4f2:%[0-9]+]] = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
// CHECK: [[v4f3:%[0-9]+]] = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
// CHECK: [[v4f4:%[0-9]+]] = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
// CHECK: [[v4f5:%[0-9]+]] = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
// CHECK: [[v4f6:%[0-9]+]] = OpConstantComposite %v4float %float_6 %float_6 %float_6 %float_6
// CHECK: [[v4f7:%[0-9]+]] = OpConstantComposite %v4float %float_7 %float_7 %float_7 %float_7
// CHECK: [[v4f8:%[0-9]+]] = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
// CHECK: [[v4f9:%[0-9]+]] = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9

float4 myfunc() {
  int a, b;
  bool cond = true;

  while(cond) {
    switch(b) {
// CHECK: %switch_1 = OpLabel
      case 1:
        a = 1;
// CHECK: OpReturnValue [[v4f1]]
        return float4(1.0, 1.0, 1.0, 1.0);
// CHECK-NEXT: %switch_2 = OpLabel
      case 2: {
        a = 3;
// CHECK: OpReturnValue [[v4f2]]
        {return float4(2.0, 2.0, 2.0, 2.0);}   // Return from function.
        a = 4;                                 // No SPIR-V should be emitted for this statement.
        break;                                 // No SPIR-V should be emitted for this statement.
      }
// CHECK-NEXT: %switch_5 = OpLabel
      case 5 : {
        a = 5;
// CHECK: OpReturnValue [[v4f3]]
        {{return float4(3.0, 3.0, 3.0, 3.0);}} // Return from function.
        a = 6;                                 // No SPIR-V should be emitted for this statement.
      }
// CHECK-NEXT: %switch_default = OpLabel
      default:
        for (int i=0; i<10; ++i) {
          if (cond) {
// CHECK: %if_true = OpLabel
// CHECK-NEXT: OpReturnValue [[v4f4]]
            return float4(4.0, 4.0, 4.0, 4.0);    // Return from function.
            return float4(5.0, 5.0, 5.0, 5.0);    // No SPIR-V should be emitted for this statement.
            continue;                             // No SPIR-V should be emitted for this statement.
            break;                                // No SPIR-V should be emitted for this statement.
            ++a;                                  // No SPIR-V should be emitted for this statement.
          } else {
// CHECK-NEXT: %if_false = OpLabel
// CHECK-NEXT: OpReturnValue [[v4f6]]
            return float4(6.0, 6.0, 6.0, 6.0);;   // Return from function
            continue;                             // No SPIR-V should be emitted for this statement.
            break;                                // No SPIR-V should be emitted for this statement.
            ++a;                                  // No SPIR-V should be emitted for this statement.
          }
        }
// CHECK: %for_merge = OpLabel

// CHECK-NEXT: OpReturnValue [[v4f7]]
        // Return from function.
        // Even though this statement will never be executed [because both "if" and "else" above have return statements],
        // SPIR-V code should be emitted for it as we do not analyze the logic.
        return float4(7.0, 7.0, 7.0, 7.0);
    }
// CHECK: %switch_merge = OpLabel

// CHECK-NEXT: OpReturnValue [[v4f8]]
    // Return from function.
    // Even though this statement will never be executed [because all "case" statements above contain a return statement],
    // SPIR-V code should be emitted for it as we do not analyze the logic.
    return float4(8.0, 8.0, 8.0, 8.0);
  }
// CHECK: %while_merge = OpLabel

// CHECK-NEXT: OpReturnValue [[v4f9]]
  // Return from function.
  // Even though this statement will never be executed [because any iteration of the loop above executes a return statement],
  // SPIR-V code should be emitted for it as we do not analyze the logic.
  return float4(9.0, 9.0, 9.0, 9.0);

// CHECK-NEXT: OpFunctionEnd
}

void main() {
  float4 result = myfunc();
}
