; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 13
; Schema: 0
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %_ %foo %gl_Position
OpSource GLSL 450
OpName %main "main"
OpName %Vert "Vert"
OpMemberName %Vert 0 "a"
OpMemberName %Vert 1 "b"
OpName %_ ""
OpName %Foo "Foo"
OpMemberName %Foo 0 "c"
OpMemberName %Foo 1 "d"
OpName %foo "foo"
OpDecorate %Vert Block
OpDecorate %_ Location 0
OpDecorate %foo Location 2
OpDecorate %gl_Position BuiltIn Position
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%Vert = OpTypeStruct %float %float
%vec4 = OpTypeVector %float 4
%ptr_Output_vec4 = OpTypePointer Output %vec4
%_ptr_Output_Vert = OpTypePointer Output %Vert
%zero_vert = OpConstantNull %Vert
%_ = OpVariable %_ptr_Output_Vert Output %zero_vert
%gl_Position = OpVariable %ptr_Output_vec4 Output
%Foo = OpTypeStruct %float %float
%_ptr_Output_Foo = OpTypePointer Output %Foo
%zero_foo = OpConstantNull %Foo
%blank = OpConstantNull %vec4
%foo = OpVariable %_ptr_Output_Foo Output %zero_foo
%main = OpFunction %void None %3
%5 = OpLabel
OpStore %gl_Position %blank
OpReturn
OpFunctionEnd
