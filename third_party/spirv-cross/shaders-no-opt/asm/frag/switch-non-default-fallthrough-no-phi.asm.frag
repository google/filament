; SPIR-V
; Version: 1.3
; Generator: Google rspirv; 0
; Bound: 80
; Schema: 0
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint Fragment %1 "main" %2 %3
               OpExecutionMode %1 OriginUpperLeft
               OpMemberDecorate %_struct_14 0 Offset 0
               OpMemberDecorate %_struct_14 1 Offset 4
               OpMemberDecorate %_struct_15 0 Offset 0
               OpMemberDecorate %_struct_15 1 Offset 4
               OpDecorate %2 Location 0
               OpDecorate %3 Location 0
			   OpDecorate %2 Flat
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
       %bool = OpTypeBool
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_int = OpTypePointer Output %int
%_ptr_Function_int = OpTypePointer Function %int
       %void = OpTypeVoid
 %_struct_14 = OpTypeStruct %uint %int
 %_struct_15 = OpTypeStruct %int %int
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
         %24 = OpTypeFunction %void
          %2 = OpVariable %_ptr_Input_int Input
          %3 = OpVariable %_ptr_Output_int Output
     %uint_1 = OpConstant %uint 1
         %26 = OpUndef %_struct_14
     %uint_0 = OpConstant %uint 0
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
       %true = OpConstantTrue %bool
         %31 = OpUndef %int
      %false = OpConstantFalse %bool
%_ptr_Function_bool = OpTypePointer Function %bool
          %1 = OpFunction %void None %24
         %32 = OpLabel
         %76 = OpVariable %_ptr_Function_bool Function %false
         %33 = OpVariable %_ptr_Function__struct_15 Function
         %34 = OpVariable %_ptr_Function_int Function
         %35 = OpVariable %_ptr_Function_int Function
               OpSelectionMerge %72 None
               OpSwitch %uint_0 %73
         %73 = OpLabel
         %36 = OpLoad %int %2
         %37 = OpAccessChain %_ptr_Function_int %33 %uint_0
               OpStore %37 %int_0
         %38 = OpAccessChain %_ptr_Function_int %33 %uint_1
               OpStore %38 %int_10
               OpBranch %40
         %40 = OpLabel
         %41 = OpPhi %_struct_14 %26 %73 %42 %43
         %44 = OpPhi %int %int_0 %73 %45 %43
               OpLoopMerge %48 %43 None
               OpBranch %49
         %49 = OpLabel
         %52 = OpLoad %int %37
         %53 = OpLoad %int %38
         %54 = OpSLessThan %bool %52 %53
               OpSelectionMerge %55 None
               OpBranchConditional %54 %56 %57
         %57 = OpLabel
         %65 = OpCompositeInsert %_struct_14 %uint_0 %41 0
               OpBranch %55
         %56 = OpLabel
         %59 = OpLoad %int %37
         %60 = OpBitcast %int %uint_1
         %61 = OpIAdd %int %59 %60
               OpCopyMemory %34 %37
         %63 = OpLoad %int %34
               OpStore %35 %61
               OpCopyMemory %37 %35
         %64 = OpCompositeConstruct %_struct_14 %uint_1 %63
               OpBranch %55
         %55 = OpLabel
         %42 = OpPhi %_struct_14 %64 %56 %65 %57
         %66 = OpCompositeExtract %uint %42 0
         %67 = OpBitcast %int %66
               OpSelectionMerge %71 None
               OpSwitch %67 %69 0 %70 1 %71
         %71 = OpLabel
         %45 = OpIAdd %int %44 %36
               OpBranch %43
         %70 = OpLabel
               OpStore %3 %44
               OpStore %76 %true
               OpBranch %48
         %69 = OpLabel
               OpBranch %48
         %43 = OpLabel
               OpBranch %40
         %48 = OpLabel
         %79 = OpPhi %bool %false %69 %true %70
               OpSelectionMerge %77 None
               OpBranchConditional %79 %72 %77
         %77 = OpLabel
               OpBranch %72
         %72 = OpLabel
               OpReturn
               OpFunctionEnd
