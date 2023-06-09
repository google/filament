; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 7
; Bound: 83
; Schema: 0
               OpCapability Shader
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vColor
               OpExecutionMode %main OriginUpperLeft
          %1 = OpString "test.frag"
               OpSource GLSL 450 %1 "// OpModuleProcessed entry-point main
// OpModuleProcessed client vulkan100
// OpModuleProcessed target-env vulkan1.0
// OpModuleProcessed entry-point main
#line 1
#version 450

layout(location = 0) in float vColor;
layout(location = 0) out float FragColor;

void func()
{
	FragColor = 1.0;
	FragColor = 2.0;
	if (vColor < 0.0)
	{
		FragColor = 3.0;
	}
	else
	{
		FragColor = 4.0;
	}

	for (int i = 0; i < 40 + vColor; i += int(vColor) + 5)
	{
		FragColor += 0.2;
		FragColor += 0.3;
	}

	switch (int(vColor))
	{
		case 0:
			FragColor += 0.2;
			break;

		case 1:
			FragColor += 0.4;
			break;

		default:
			FragColor += 0.8;
			break;
	}

	do
	{
		FragColor += 10.0 + vColor;
	} while(FragColor < 100.0);
}

void main()
{
	func();
}
"
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %func_ "func("
               OpName %FragColor "FragColor"
               OpName %vColor "vColor"
               OpName %i "i"
               OpDecorate %FragColor Location 0
               OpDecorate %vColor Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
%_ptr_Input_float = OpTypePointer Input %float
     %vColor = OpVariable %_ptr_Input_float Input
    %float_0 = OpConstant %float 0
       %bool = OpTypeBool
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
        %int = OpTypeInt 32 1

			   ; Should be ignored
               OpLine %1 5 0

%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
   %float_40 = OpConstant %float 40
%float_0_200000003 = OpConstant %float 0.200000003
%float_0_300000012 = OpConstant %float 0.300000012
      %int_5 = OpConstant %int 5

			   ; Should be ignored
               OpLine %1 5 0

%float_0_400000006 = OpConstant %float 0.400000006
%float_0_800000012 = OpConstant %float 0.800000012
   %float_10 = OpConstant %float 10
  %float_100 = OpConstant %float 100
       %main = OpFunction %void None %4
               OpLine %1 46 0
          %6 = OpLabel
               OpLine %1 48 0
         %82 = OpFunctionCall %void %func_
               OpReturn
               OpFunctionEnd

			   ; Should be ignored
               OpLine %1 5 0

      %func_ = OpFunction %void None %4
               OpLine %1 6 0
          %8 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
               OpLine %1 8 0
               OpStore %FragColor %float_1
               OpLine %1 9 0
               OpStore %FragColor %float_2
               OpLine %1 10 0
         %16 = OpLoad %float %vColor
         %19 = OpFOrdLessThan %bool %16 %float_0
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %23
         %20 = OpLabel
               OpLine %1 12 0
               OpStore %FragColor %float_3
               OpBranch %21
         %23 = OpLabel
               OpLine %1 16 0
               OpStore %FragColor %float_4
               OpBranch %21
         %21 = OpLabel
               OpLine %1 19 0
               OpStore %i %int_0
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %31 %32 None
               OpBranch %33
         %33 = OpLabel
         %34 = OpLoad %int %i
         %35 = OpConvertSToF %float %34
         %37 = OpLoad %float %vColor
         %38 = OpFAdd %float %float_40 %37
         %39 = OpFOrdLessThan %bool %35 %38
               OpBranchConditional %39 %30 %31
         %30 = OpLabel
               OpLine %1 21 0
         %41 = OpLoad %float %FragColor
         %42 = OpFAdd %float %41 %float_0_200000003
               OpStore %FragColor %42
               OpLine %1 22 0
         %44 = OpLoad %float %FragColor
         %45 = OpFAdd %float %44 %float_0_300000012
               OpStore %FragColor %45
               OpBranch %32
         %32 = OpLabel
               OpLine %1 19 0
         %46 = OpLoad %float %vColor
         %47 = OpConvertFToS %int %46
         %49 = OpIAdd %int %47 %int_5
         %50 = OpLoad %int %i
         %51 = OpIAdd %int %50 %49
               OpStore %i %51
               OpBranch %29
         %31 = OpLabel
               OpLine %1 25 0
         %52 = OpLoad %float %vColor
         %53 = OpConvertFToS %int %52
               OpSelectionMerge %57 None
               OpSwitch %53 %56 0 %54 1 %55
         %56 = OpLabel
               OpLine %1 36 0
         %66 = OpLoad %float %FragColor
         %67 = OpFAdd %float %66 %float_0_800000012
               OpStore %FragColor %67
               OpLine %1 37 0
               OpBranch %57
         %54 = OpLabel
               OpLine %1 28 0
         %58 = OpLoad %float %FragColor
         %59 = OpFAdd %float %58 %float_0_200000003
               OpStore %FragColor %59
               OpLine %1 29 0
               OpBranch %57
         %55 = OpLabel
               OpLine %1 32 0
         %62 = OpLoad %float %FragColor
         %63 = OpFAdd %float %62 %float_0_400000006
               OpStore %FragColor %63
               OpLine %1 33 0
               OpBranch %57
         %57 = OpLabel
               OpBranch %70
               OpLine %1 43 0
         %70 = OpLabel
               OpLoopMerge %72 %73 None
               OpBranch %71
         %71 = OpLabel
               OpLine %1 42 0
         %75 = OpLoad %float %vColor
         %76 = OpFAdd %float %float_10 %75
         %77 = OpLoad %float %FragColor
         %78 = OpFAdd %float %77 %76
               OpStore %FragColor %78
               OpBranch %73
         %73 = OpLabel
               OpLine %1 43 0
         %79 = OpLoad %float %FragColor
         %81 = OpFOrdLessThan %bool %79 %float_100
               OpBranchConditional %81 %70 %72
         %72 = OpLabel
               OpReturn
               OpFunctionEnd
