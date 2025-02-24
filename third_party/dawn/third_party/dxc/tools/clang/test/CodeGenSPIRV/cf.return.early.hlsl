// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  int a, b;
  bool cond = true;

  while(cond) {
    switch(b) {
// CHECK: %switch_1 = OpLabel    
      case 1:
        a = 1;
// CHECK: OpReturn
        return;
// CHECK-NEXT: %switch_2 = OpLabel
      case 2: {
        a = 3;
// CHECK: OpReturn
        {return;}   // Return from function.
        a = 4;      // No SPIR-V should be emitted for this statement.
        break;      // No SPIR-V should be emitted for this statement.
      }
// CHECK-NEXT: %switch_5 = OpLabel
      case 5 : {
        a = 5;
// CHECK: OpReturn
        {{return;}} // Return from function.
        a = 6;      // No SPIR-V should be emitted for this statement.
      }
// CHECK-NEXT: %switch_default = OpLabel
      default:
        for (int i=0; i<10; ++i) {
          if (cond) {
// CHECK: %if_true = OpLabel
// CHECK-NEXT: OpReturn
            return;    // Return from function.
            return;    // No SPIR-V should be emitted for this statement.
            continue;  // No SPIR-V should be emitted for this statement.
            break;     // No SPIR-V should be emitted for this statement.
            ++a;       // No SPIR-V should be emitted for this statement.
          } else {
// CHECK-NEXT: %if_false = OpLabel
// CHECK-NEXT: OpReturn
            return;   // Return from function
            continue; // No SPIR-V should be emitted for this statement.
            break;    // No SPIR-V should be emitted for this statement.
            ++a;      // No SPIR-V should be emitted for this statement.
          }
        }
// CHECK: %for_merge = OpLabel

// CHECK-NEXT: OpReturn
        // Return from function.
        // Even though this statement will never be executed [because both "if" and "else" above have return statements],
        // SPIR-V code should be emitted for it as we do not analyze the logic.
        return;
    }
// CHECK: %switch_merge = OpLabel

// CHECK-NEXT: OpReturn
    // Return from function.
    // Even though this statement will never be executed [because all "case" statements above contain a return statement],
    // SPIR-V code should be emitted for it as we do not analyze the logic.
    return;
  }
// CHECK: %while_merge = OpLabel

// CHECK-NEXT: OpReturn
// CHECK-NEXT: OpFunctionEnd
}
