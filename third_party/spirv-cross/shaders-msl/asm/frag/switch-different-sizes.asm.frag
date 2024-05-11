; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 10
; Bound: 42
; Schema: 0
               OpCapability Shader
               OpCapability Int8
               OpCapability Int16
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %sw0 "sw0"
               OpName %result "result"
               OpName %sw1 "sw1"
               OpName %sw2 "sw2"
               OpName %sw3 "sw3"
               OpDecorate %sw1 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %sw2 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
                     %void = OpTypeVoid
                        %3 = OpTypeFunction %void
                      %int = OpTypeInt 32 1
                 %lowp_int = OpTypeInt 8 1
                %highp_int = OpTypeInt 16 1
        %_ptr_Function_int = OpTypePointer Function %int
   %_ptr_Function_lowp_int = OpTypePointer Function %lowp_int
  %_ptr_Function_highp_int = OpTypePointer Function %highp_int
                   %int_42 = OpConstant %int 42
                    %int_0 = OpConstant %int 0
                  %int_420 = OpConstant %int 420
                   %int_10 = OpConstant %int 10
                  %int_512 = OpConstant %int 512
              %lowp_int_10 = OpConstant %lowp_int 10
             %highp_int_10 = OpConstant %highp_int 10
                     %main = OpFunction %void None %3
                        %5 = OpLabel
                      %sw0 = OpVariable %_ptr_Function_int Function
                   %result = OpVariable %_ptr_Function_int Function
                      %sw1 = OpVariable %_ptr_Function_lowp_int Function
                      %sw2 = OpVariable %_ptr_Function_highp_int Function
                      %sw3 = OpVariable %_ptr_Function_highp_int Function
                             OpStore %sw0 %int_42
                             OpStore %result %int_0
                       %12 = OpLoad %int %sw0
                             OpSelectionMerge %16 None
                             OpSwitch %12 %16 -42 %13 420 %14 -1234 %15
                       %13 = OpLabel
                             OpStore %result %int_42
                             OpBranch %14
                       %14 = OpLabel
                             OpStore %result %int_420
                             OpBranch %15
                       %15 = OpLabel
                             OpStore %result %int_420
                             OpBranch %16
                       %16 = OpLabel
                             OpStore %sw1 %lowp_int_10
                       %21 = OpLoad %lowp_int %sw1
                             OpSelectionMerge %25 None
                             OpSwitch %21 %25 -42 %22 42 %23 -123 %24
                       %22 = OpLabel
                             OpStore %result %int_42
                             OpBranch %23
                       %23 = OpLabel
                             OpStore %result %int_420
                             OpBranch %24
                       %24 = OpLabel
                             OpStore %result %int_512
                             OpBranch %25
                       %25 = OpLabel
                             OpStore %sw2 %highp_int_10
                       %29 = OpLoad %highp_int %sw2
                             OpSelectionMerge %33 None
                             OpSwitch %29 %33 -42 %30 42 %31 -1234 %32
                       %30 = OpLabel
                             OpStore %result %int_42
                             OpBranch %31
                       %31 = OpLabel
                             OpStore %result %int_420
                             OpBranch %32
                       %32 = OpLabel
                             OpStore %result %int_512
                             OpBranch %33
                       %33 = OpLabel
                             OpStore %sw3 %highp_int_10
                       %36 = OpLoad %highp_int %sw3
                             OpSelectionMerge %40 None
                             OpSwitch %36 %40 -42 %37 42 %38 -1234 %39
                       %37 = OpLabel
                             OpStore %result %int_42
                             OpBranch %38
                       %38 = OpLabel
                             OpStore %result %int_420
                             OpBranch %39
                       %39 = OpLabel
                             OpStore %result %int_512
                             OpBranch %40
                       %40 = OpLabel
                             OpReturn
                             OpFunctionEnd
