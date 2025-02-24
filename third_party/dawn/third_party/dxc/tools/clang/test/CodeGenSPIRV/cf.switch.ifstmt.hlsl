// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

int foo() { return 200; }

void main() {

// CHECK:      %a = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: %b = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: %c = OpVariable %_ptr_Function_int Function

  int a,b,c;
  const int r = 20;
  const int s = 40;
  const int t = 3*r+2*s;


  ////////////////////////////////////////
  // DefaultStmt is the first statement //
  ////////////////////////////////////////

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_1:%[0-9]+]] = OpIEqual %bool [[a0]] %int_1
// CHECK-NEXT: OpSelectionMerge %if_merge_0 None
// CHECK-NEXT: OpBranchConditional [[is_a_1]] %if_true %if_false
// CHECK-NEXT: %if_true = OpLabel
// CHECK-NEXT: OpStore %b %int_1
// CHECK-NEXT: OpBranch %if_merge_0
// CHECK-NEXT: %if_false = OpLabel
// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_2:%[0-9]+]] = OpIEqual %bool [[a1]] %int_2
// CHECK-NEXT: OpSelectionMerge %if_merge None
// CHECK-NEXT: OpBranchConditional [[is_a_2]] %if_true_0 %if_false_0
// CHECK-NEXT: %if_true_0 = OpLabel
// CHECK-NEXT: OpStore %b %int_2
// CHECK-NEXT: OpBranch %if_merge
// CHECK-NEXT: %if_false_0 = OpLabel
// CHECK-NEXT: OpStore %b %int_0
// CHECK-NEXT: OpStore %b %int_1
// CHECK-NEXT: OpBranch %if_merge
// CHECK-NEXT: %if_merge = OpLabel
// CHECK-NEXT: OpBranch %if_merge_0
// CHECK-NEXT: %if_merge_0 = OpLabel
  [branch] switch(a) {
    default:
      b=0;
    case 1:
      b=1;
      break;
    case 2:
      b=2;
  }


  //////////////////////////////////////////////
  // DefaultStmt in the middle of other cases //
  //////////////////////////////////////////////

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_10:%[0-9]+]] = OpIEqual %bool [[a2]] %int_10
// CHECK-NEXT: OpSelectionMerge %if_merge_2 None
// CHECK-NEXT: OpBranchConditional [[is_a_10]] %if_true_1 %if_false_1
// CHECK-NEXT: %if_true_1 = OpLabel
// CHECK-NEXT: OpStore %b %int_1
// CHECK-NEXT: OpStore %b %int_0
// CHECK-NEXT: OpStore %b %int_2
// CHECK-NEXT: OpBranch %if_merge_2
// CHECK-NEXT: %if_false_1 = OpLabel
// CHECK-NEXT: [[a3:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_20:%[0-9]+]] = OpIEqual %bool [[a3]] %int_20
// CHECK-NEXT: OpSelectionMerge %if_merge_1 None
// CHECK-NEXT: OpBranchConditional [[is_a_20]] %if_true_2 %if_false_2
// CHECK-NEXT: %if_true_2 = OpLabel
// CHECK-NEXT: OpStore %b %int_2
// CHECK-NEXT: OpBranch %if_merge_1
// CHECK-NEXT: %if_false_2 = OpLabel
// CHECK-NEXT: OpStore %b %int_0
// CHECK-NEXT: OpStore %b %int_2
// CHECK-NEXT: OpBranch %if_merge_1
// CHECK-NEXT: %if_merge_1 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_2
// CHECK-NEXT: %if_merge_2 = OpLabel
  [branch] switch(a) {
    case 10:
      b=1;
    default:
      b=0;
    case 20:
      b=2;
      break;
  }

  ///////////////////////////////////////////////
  // Various CaseStmt and BreakStmt topologies //
  // DefaultStmt is the last statement         //
  ///////////////////////////////////////////////

// CHECK:      [[d0:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_1:%[0-9]+]] = OpIEqual %bool [[d0]] %int_1
// CHECK-NEXT: OpSelectionMerge %if_merge_10 None
// CHECK-NEXT: OpBranchConditional [[is_d_1]] %if_true_3 %if_false_3
// CHECK-NEXT: %if_true_3 = OpLabel
// CHECK-NEXT: OpStore %b %int_1
// CHECK-NEXT: [[foo:%[0-9]+]] = OpFunctionCall %int %foo
// CHECK-NEXT: OpStore %c [[foo]]
// CHECK-NEXT: OpStore %b %int_2
// CHECK-NEXT: OpBranch %if_merge_10
// CHECK-NEXT: %if_false_3 = OpLabel
// CHECK-NEXT: [[d1:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_2:%[0-9]+]] = OpIEqual %bool [[d1]] %int_2
// CHECK-NEXT: OpSelectionMerge %if_merge_9 None
// CHECK-NEXT: OpBranchConditional [[is_d_2]] %if_true_4 %if_false_4
// CHECK-NEXT: %if_true_4 = OpLabel
// CHECK-NEXT: OpStore %b %int_2
// CHECK-NEXT: OpBranch %if_merge_9
// CHECK-NEXT: %if_false_4 = OpLabel
// CHECK-NEXT: [[d2:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_3:%[0-9]+]] = OpIEqual %bool [[d2]] %int_3
// CHECK-NEXT: OpSelectionMerge %if_merge_8 None
// CHECK-NEXT: OpBranchConditional [[is_d_3]] %if_true_5 %if_false_5
// CHECK-NEXT: %if_true_5 = OpLabel
// CHECK-NEXT: OpStore %b %int_3
// CHECK-NEXT: OpBranch %if_merge_8
// CHECK-NEXT: %if_false_5 = OpLabel
// CHECK-NEXT: [[d3:%[0-9]+]] = OpLoad %int %d
// TODO: We should try to const fold `t` and avoid the following OpLoad:
// CHECK-NEXT: [[t:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[is_d_eq_t:%[0-9]+]] = OpIEqual %bool [[d3]] [[t]]
// CHECK-NEXT: OpSelectionMerge %if_merge_7 None
// CHECK-NEXT: OpBranchConditional [[is_d_eq_t]] %if_true_6 %if_false_6
// CHECK-NEXT: %if_true_6 = OpLabel
// CHECK-NEXT: [[t1:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: OpStore %b [[t1]]
// CHECK-NEXT: OpStore %b %int_5
// CHECK-NEXT: OpBranch %if_merge_7
// CHECK-NEXT: %if_false_6 = OpLabel
// CHECK-NEXT: [[d4:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_4:%[0-9]+]] = OpIEqual %bool [[d4]] %int_4
// CHECK-NEXT: OpSelectionMerge %if_merge_6 None
// CHECK-NEXT: OpBranchConditional [[is_d_4]] %if_true_7 %if_false_7
// CHECK-NEXT: %if_true_7 = OpLabel
// CHECK-NEXT: OpStore %b %int_5
// CHECK-NEXT: OpBranch %if_merge_6
// CHECK-NEXT: %if_false_7 = OpLabel
// CHECK-NEXT: [[d5:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_5:%[0-9]+]] = OpIEqual %bool [[d5]] %int_5
// CHECK-NEXT: OpSelectionMerge %if_merge_5 None
// CHECK-NEXT: OpBranchConditional [[is_d_5]] %if_true_8 %if_false_8
// CHECK-NEXT: %if_true_8 = OpLabel
// CHECK-NEXT: OpStore %b %int_5
// CHECK-NEXT: OpBranch %if_merge_5
// CHECK-NEXT: %if_false_8 = OpLabel
// CHECK-NEXT: [[d6:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_6:%[0-9]+]] = OpIEqual %bool [[d6]] %int_6
// CHECK-NEXT: OpSelectionMerge %if_merge_4 None
// CHECK-NEXT: OpBranchConditional [[is_d_6]] %if_true_9 %if_false_9
// CHECK-NEXT: %if_true_9 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_4
// CHECK-NEXT: %if_false_9 = OpLabel
// CHECK-NEXT: [[d7:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[is_d_7:%[0-9]+]] = OpIEqual %bool [[d7]] %int_7
// CHECK-NEXT: OpSelectionMerge %if_merge_3 None
// CHECK-NEXT: OpBranchConditional [[is_d_7]] %if_true_10 %if_false_10
// CHECK-NEXT: %if_true_10 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_3
// CHECK-NEXT: %if_false_10 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_3
// CHECK-NEXT: %if_merge_3 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_4
// CHECK-NEXT: %if_merge_4 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_5
// CHECK-NEXT: %if_merge_5 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_6
// CHECK-NEXT: %if_merge_6 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_7
// CHECK-NEXT: %if_merge_7 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_8
// CHECK-NEXT: %if_merge_8 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_9
// CHECK-NEXT: %if_merge_9 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_10
// CHECK-NEXT: %if_merge_10 = OpLabel
  [branch] switch(int d = 5) {
    case 1:
      b=1;
      c=foo();
    case 2:
      b=2;
      break;
    case 3:
    {
      b=3;
      break;
    }
    case t:
      b=t;
    case 4:
    case 5:
      b=5;
      break;
    case 6: {
    case 7:
      break;}
    default:
      break;
  }


  //////////////////////////
  // No Default statement //
  //////////////////////////

// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_100:%[0-9]+]] = OpIEqual %bool [[a4]] %int_100
// CHECK-NEXT: OpSelectionMerge %if_merge_11 None
// CHECK-NEXT: OpBranchConditional [[is_a_100]] %if_true_11 %if_merge_11
// CHECK-NEXT: %if_true_11 = OpLabel
// CHECK-NEXT: OpStore %b %int_100
// CHECK-NEXT: OpBranch %if_merge_11
// CHECK-NEXT: %if_merge_11 = OpLabel
  [branch] switch(a) {
    case 100:
      b=100;
      break;
  }


  /////////////////////////////////////////////////////////
  // No cases. Only a default                            //
  // This means the default body will always be executed //
  /////////////////////////////////////////////////////////

// CHECK-NEXT: OpStore %b %int_100
// CHECK-NEXT: OpStore %c %int_200
  [branch] switch(a) {
    default:
      b=100;
      c=200;
      break;
  }


  ////////////////////////////////////////////////////////////
  // Nested Switch with branching                           //
  // The two inner switch statements should be executed for //
  // both cases of the outer switch (case 300 and case 400) //
  ////////////////////////////////////////////////////////////

// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_300:%[0-9]+]] = OpIEqual %bool [[a5]] %int_300
// CHECK-NEXT: OpSelectionMerge %if_merge_17 None
// CHECK-NEXT: OpBranchConditional [[is_a_300]] %if_true_12 %if_false_12
// CHECK-NEXT: %if_true_12 = OpLabel
// CHECK-NEXT: OpStore %b %int_300
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: [[is_c_500:%[0-9]+]] = OpIEqual %bool [[c0]] %int_500
// CHECK-NEXT: OpSelectionMerge %if_merge_13 None
// CHECK-NEXT: OpBranchConditional [[is_c_500]] %if_true_13 %if_false_11
// CHECK-NEXT: %if_true_13 = OpLabel
// CHECK-NEXT: OpStore %b %int_500
// CHECK-NEXT: OpBranch %if_merge_13
// CHECK-NEXT: %if_false_11 = OpLabel
// CHECK-NEXT: [[c1:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: [[is_c_600:%[0-9]+]] = OpIEqual %bool [[c1]] %int_600
// CHECK-NEXT: OpSelectionMerge %if_merge_12 None
// CHECK-NEXT: OpBranchConditional [[is_c_600]] %if_true_14 %if_merge_12
// CHECK-NEXT: %if_true_14 = OpLabel
// CHECK-NEXT: OpStore %a %int_600
// CHECK-NEXT: OpStore %b %int_600
// CHECK-NEXT: OpBranch %if_merge_12
// CHECK-NEXT: %if_merge_12 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_13
// CHECK-NEXT: %if_merge_13 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_17
// CHECK-NEXT: %if_false_12 = OpLabel
// CHECK-NEXT: [[a6:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[is_a_400:%[0-9]+]] = OpIEqual %bool [[a6]] %int_400
// CHECK-NEXT: OpSelectionMerge %if_merge_16 None
// CHECK-NEXT: OpBranchConditional [[is_a_400]] %if_true_15 %if_merge_16
// CHECK-NEXT: %if_true_15 = OpLabel
// CHECK-NEXT: [[c2:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: [[is_c_500_again:%[0-9]+]] = OpIEqual %bool [[c2]] %int_500
// CHECK-NEXT: OpSelectionMerge %if_merge_15 None
// CHECK-NEXT: OpBranchConditional [[is_c_500_again]] %if_true_16 %if_false_13
// CHECK-NEXT: %if_true_16 = OpLabel
// CHECK-NEXT: OpStore %b %int_500
// CHECK-NEXT: OpBranch %if_merge_15
// CHECK-NEXT: %if_false_13 = OpLabel
// CHECK-NEXT: [[c3:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: [[is_c_600_again:%[0-9]+]] = OpIEqual %bool [[c3]] %int_600
// CHECK-NEXT: OpSelectionMerge %if_merge_14 None
// CHECK-NEXT: OpBranchConditional [[is_c_600_again]] %if_true_17 %if_merge_14
// CHECK-NEXT: %if_true_17 = OpLabel
// CHECK-NEXT: OpStore %a %int_600
// CHECK-NEXT: OpStore %b %int_600
// CHECK-NEXT: OpBranch %if_merge_14
// CHECK-NEXT: %if_merge_14 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_15
// CHECK-NEXT: %if_merge_15 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_16
// CHECK-NEXT: %if_merge_16 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_17
// CHECK-NEXT: %if_merge_17 = OpLabel
  [branch] switch(a) {
    case 300:
      b=300;
    case 400:
      [branch] switch(c) {
        case 500:
          b=500;
          break;
        case 600:
          [branch] switch(b) {
            default:
            a=600;
            b=600;
          }
      }
  }

}
