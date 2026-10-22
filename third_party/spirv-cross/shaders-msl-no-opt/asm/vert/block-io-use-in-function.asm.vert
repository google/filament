; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 102
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %a_color %a_texcoord %a_position %__1
               OpSource GLSL 450
               OpSourceExtension "GL_ARB_separate_shader_objects"
               OpSourceExtension "GL_ARB_shading_language_420pack"
               OpSourceExtension "GL_EXT_multiview"
               OpName %main "main"
               OpName %DrawWorldVS_ "DrawWorldVS("
               OpName %PerVertex "PerVertex"
               OpMemberName %PerVertex 0 "v_color"
               OpMemberName %PerVertex 1 "v_texPos"
               OpMemberName %PerVertex 2 "v_worldPosition"
               OpName %_ ""
               OpName %a_color "a_color"
               OpName %a_texcoord "a_texcoord"
               OpName %a_position "a_position"
               OpName %worldSpacePosition "worldSpacePosition"
               OpName %u_objToWorlds "u_objToWorlds"
               OpMemberName %u_objToWorlds 0 "u_objToWorld"
               OpName %__0 ""
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %__1 ""
               OpName %CameraData "CameraData"
               OpMemberName %CameraData 0 "u_projectFromView"
               OpMemberName %CameraData 1 "u_projectFromWorld"
               OpMemberName %CameraData 2 "u_clipFromPixels"
               OpMemberName %CameraData 3 "u_viewFromWorld"
               OpMemberName %CameraData 4 "u_worldFromView"
               OpMemberName %CameraData 5 "u_viewFromProject"
               OpMemberName %CameraData 6 "u_cameraPosition"
               OpMemberName %CameraData 7 "pad0"
               OpMemberName %CameraData 8 "u_cameraRight"
               OpMemberName %CameraData 9 "pad1"
               OpMemberName %CameraData 10 "u_cameraUp"
               OpMemberName %CameraData 11 "pad2"
               OpMemberName %CameraData 12 "u_cameraForward"
               OpMemberName %CameraData 13 "pad3"
               OpMemberName %CameraData 14 "u_clipPlanes"
               OpMemberName %CameraData 15 "u_invProjParams"
               OpMemberName %CameraData 16 "u_viewportSize"
               OpMemberName %CameraData 17 "u_invViewportSize"
               OpMemberName %CameraData 18 "u_viewportOffset"
               OpMemberName %CameraData 19 "u_fragToLightGrid"
               OpMemberName %CameraData 20 "u_screenToLightGrid"
               OpMemberName %CameraData 21 "pad4"
               OpMemberName %CameraData 22 "u_lightGridZParams"
               OpName %LightData "LightData"
               OpMemberName %LightData 0 "posRadius"
               OpMemberName %LightData 1 "color"
               OpMemberName %LightData 2 "axis"
               OpMemberName %LightData 3 "directionalParams"
               OpMemberName %LightData 4 "spotParams"
               OpMemberName %LightData 5 "shadowSize"
               OpMemberName %LightData 6 "cookieFromWorld"
               OpMemberName %LightData 7 "shadowFromWorld"
               OpMemberName %LightData 8 "shadowTextureValid"
               OpMemberName %LightData 9 "lightCookieValid"
               OpMemberName %LightData 10 "minLightDistance"
               OpMemberName %LightData 11 "pad"
               OpName %SceneSettings "SceneSettings"
               OpMemberName %SceneSettings 0 "u_cameras"
               OpMemberName %SceneSettings 1 "u_centerPosition"
               OpMemberName %SceneSettings 2 "u_centerRight"
               OpMemberName %SceneSettings 3 "u_centerUp"
               OpMemberName %SceneSettings 4 "u_centerForward"
               OpMemberName %SceneSettings 5 "u_times"
               OpMemberName %SceneSettings 6 "u_lightFXMask"
               OpMemberName %SceneSettings 7 "u_lights"
               OpMemberName %SceneSettings 8 "u_globalLightDir"
               OpMemberName %SceneSettings 9 "u_globalLightDiffuseColor"
               OpMemberName %SceneSettings 10 "u_globalLightSpecularColor"
               OpMemberName %SceneSettings 11 "u_distanceFog"
               OpMemberName %SceneSettings 12 "u_heightFog"
               OpMemberName %SceneSettings 13 "u_fogColor"
               OpMemberName %SceneSettings 14 "u_debugMode"
               OpName %__2 ""
               OpName %LightContext "LightContext"
               OpMemberName %LightContext 0 "u_ambientIBLTint"
               OpMemberName %LightContext 1 "u_specularIBLTint"
               OpMemberName %LightContext 2 "u_iblAABBMin"
               OpMemberName %LightContext 3 "u_iblAABBMax"
               OpMemberName %LightContext 4 "u_iblAABBCenter"
               OpMemberName %LightContext 5 "LightContext_unusued0"
               OpMemberName %LightContext 6 "u_exposureMultiplier"
               OpName %__3 ""
               OpName %u_sceneDepthMS "u_sceneDepthMS"
               OpName %u_sceneStencilMS "u_sceneStencilMS"
               OpName %u_lightGrid "u_lightGrid"
               OpName %u_texture "u_texture"
               OpDecorate %PerVertex Block
               OpDecorate %_ Location 1
               OpDecorate %a_color Location 2
               OpDecorate %a_texcoord Location 1
               OpDecorate %a_position Location 0
               OpMemberDecorate %u_objToWorlds 0 ColMajor
               OpMemberDecorate %u_objToWorlds 0 Offset 0
               OpMemberDecorate %u_objToWorlds 0 MatrixStride 16
               OpDecorate %u_objToWorlds Block
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 1
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %CameraData 0 ColMajor
               OpMemberDecorate %CameraData 0 Offset 0
               OpMemberDecorate %CameraData 0 MatrixStride 16
               OpMemberDecorate %CameraData 1 ColMajor
               OpMemberDecorate %CameraData 1 Offset 64
               OpMemberDecorate %CameraData 1 MatrixStride 16
               OpMemberDecorate %CameraData 2 ColMajor
               OpMemberDecorate %CameraData 2 Offset 128
               OpMemberDecorate %CameraData 2 MatrixStride 16
               OpMemberDecorate %CameraData 3 ColMajor
               OpMemberDecorate %CameraData 3 Offset 192
               OpMemberDecorate %CameraData 3 MatrixStride 16
               OpMemberDecorate %CameraData 4 ColMajor
               OpMemberDecorate %CameraData 4 Offset 256
               OpMemberDecorate %CameraData 4 MatrixStride 16
               OpMemberDecorate %CameraData 5 ColMajor
               OpMemberDecorate %CameraData 5 Offset 320
               OpMemberDecorate %CameraData 5 MatrixStride 16
               OpMemberDecorate %CameraData 6 Offset 384
               OpMemberDecorate %CameraData 7 Offset 396
               OpMemberDecorate %CameraData 8 Offset 400
               OpMemberDecorate %CameraData 9 Offset 412
               OpMemberDecorate %CameraData 10 Offset 416
               OpMemberDecorate %CameraData 11 Offset 428
               OpMemberDecorate %CameraData 12 Offset 432
               OpMemberDecorate %CameraData 13 Offset 444
               OpMemberDecorate %CameraData 14 Offset 448
               OpMemberDecorate %CameraData 15 Offset 456
               OpMemberDecorate %CameraData 16 Offset 464
               OpMemberDecorate %CameraData 17 Offset 472
               OpMemberDecorate %CameraData 18 Offset 480
               OpMemberDecorate %CameraData 19 Offset 488
               OpMemberDecorate %CameraData 20 Offset 496
               OpMemberDecorate %CameraData 21 Offset 504
               OpMemberDecorate %CameraData 22 Offset 512
               OpDecorate %_arr_CameraData_uint_2 ArrayStride 528
               OpMemberDecorate %LightData 0 Offset 0
               OpMemberDecorate %LightData 1 Offset 16
               OpMemberDecorate %LightData 2 Offset 32
               OpMemberDecorate %LightData 3 Offset 48
               OpMemberDecorate %LightData 4 Offset 64
               OpMemberDecorate %LightData 5 Offset 80
               OpMemberDecorate %LightData 6 ColMajor
               OpMemberDecorate %LightData 6 Offset 96
               OpMemberDecorate %LightData 6 MatrixStride 16
               OpMemberDecorate %LightData 7 ColMajor
               OpMemberDecorate %LightData 7 Offset 160
               OpMemberDecorate %LightData 7 MatrixStride 16
               OpMemberDecorate %LightData 8 Offset 224
               OpMemberDecorate %LightData 9 Offset 228
               OpMemberDecorate %LightData 10 Offset 232
               OpMemberDecorate %LightData 11 Offset 236
               OpDecorate %_arr_LightData_uint_8 ArrayStride 240
               OpMemberDecorate %SceneSettings 0 Offset 0
               OpMemberDecorate %SceneSettings 1 Offset 1056
               OpMemberDecorate %SceneSettings 2 Offset 1072
               OpMemberDecorate %SceneSettings 3 Offset 1088
               OpMemberDecorate %SceneSettings 4 Offset 1104
               OpMemberDecorate %SceneSettings 5 Offset 1120
               OpMemberDecorate %SceneSettings 6 Offset 1128
               OpMemberDecorate %SceneSettings 7 Offset 1136
               OpMemberDecorate %SceneSettings 8 Offset 3056
               OpMemberDecorate %SceneSettings 9 Offset 3072
               OpMemberDecorate %SceneSettings 10 Offset 3088
               OpMemberDecorate %SceneSettings 11 Offset 3104
               OpMemberDecorate %SceneSettings 12 Offset 3112
               OpMemberDecorate %SceneSettings 13 Offset 3120
               OpMemberDecorate %SceneSettings 14 Offset 3136
               OpDecorate %SceneSettings Block
               OpDecorate %__2 DescriptorSet 0
               OpDecorate %__2 Binding 0
               OpMemberDecorate %LightContext 0 Offset 0
               OpMemberDecorate %LightContext 1 Offset 16
               OpMemberDecorate %LightContext 2 Offset 32
               OpMemberDecorate %LightContext 3 Offset 48
               OpMemberDecorate %LightContext 4 Offset 64
               OpMemberDecorate %LightContext 5 Offset 76
               OpMemberDecorate %LightContext 6 Offset 80
               OpDecorate %LightContext Block
               OpDecorate %__3 DescriptorSet 0
               OpDecorate %__3 Binding 0
               OpDecorate %u_sceneDepthMS DescriptorSet 0
               OpDecorate %u_sceneDepthMS Binding 0
               OpDecorate %u_sceneStencilMS DescriptorSet 0
               OpDecorate %u_sceneStencilMS Binding 0
               OpDecorate %u_lightGrid DescriptorSet 0
               OpDecorate %u_lightGrid Binding 0
               OpDecorate %u_texture DescriptorSet 0
               OpDecorate %u_texture Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
  %PerVertex = OpTypeStruct %v4float %v2float %v3float
%_ptr_Output_PerVertex = OpTypePointer Output %PerVertex
          %_ = OpVariable %_ptr_Output_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
    %a_color = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%_ptr_Input_v2float = OpTypePointer Input %v2float
 %a_texcoord = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v2float = OpTypePointer Output %v2float
      %int_2 = OpConstant %int 2
%_ptr_Input_v3float = OpTypePointer Input %v3float
 %a_position = OpVariable %_ptr_Input_v3float Input
%_ptr_Output_v3float = OpTypePointer Output %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat4v4float = OpTypeMatrix %v4float 4
%u_objToWorlds = OpTypeStruct %mat4v4float
%_ptr_Uniform_u_objToWorlds = OpTypePointer Uniform %u_objToWorlds
        %__0 = OpVariable %_ptr_Uniform_u_objToWorlds Uniform
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
    %float_1 = OpConstant %float 1
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
        %__1 = OpVariable %_ptr_Output_gl_PerVertex Output
 %CameraData = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v2float %v2float %v2float %v2float %v2float %v2float %v2float %v2float %v4float
     %uint_2 = OpConstant %uint 2
%_arr_CameraData_uint_2 = OpTypeArray %CameraData %uint_2
  %LightData = OpTypeStruct %v4float %v3float %v4float %v3float %v4float %v4float %mat4v4float %mat4v4float %uint %uint %float %float
     %uint_8 = OpConstant %uint 8
%_arr_LightData_uint_8 = OpTypeArray %LightData %uint_8
%SceneSettings = OpTypeStruct %_arr_CameraData_uint_2 %v3float %v3float %v3float %v3float %v2float %uint %_arr_LightData_uint_8 %v3float %v3float %v3float %v2float %v2float %v4float %uint
%_ptr_Uniform_SceneSettings = OpTypePointer Uniform %SceneSettings
        %__2 = OpVariable %_ptr_Uniform_SceneSettings Uniform
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
     %uint_6 = OpConstant %uint 6
     %uint_7 = OpConstant %uint 7
     %uint_9 = OpConstant %uint 9
    %uint_10 = OpConstant %uint 10
    %uint_11 = OpConstant %uint 11
    %uint_12 = OpConstant %uint 12
    %uint_13 = OpConstant %uint 13
    %uint_14 = OpConstant %uint 14
%LightContext = OpTypeStruct %v3float %v3float %v3float %v3float %v3float %float %float
%_ptr_Uniform_LightContext = OpTypePointer Uniform %LightContext
        %__3 = OpVariable %_ptr_Uniform_LightContext Uniform
         %86 = OpTypeImage %float 2D 0 0 1 1 Unknown
         %87 = OpTypeSampledImage %86
%_ptr_UniformConstant_87 = OpTypePointer UniformConstant %87
%u_sceneDepthMS = OpVariable %_ptr_UniformConstant_87 UniformConstant
         %90 = OpTypeImage %uint 2D 0 0 1 1 Unknown
         %91 = OpTypeSampledImage %90
%_ptr_UniformConstant_91 = OpTypePointer UniformConstant %91
%u_sceneStencilMS = OpVariable %_ptr_UniformConstant_91 UniformConstant
         %94 = OpTypeImage %uint 3D 0 0 0 1 Unknown
         %95 = OpTypeSampledImage %94
%_ptr_UniformConstant_95 = OpTypePointer UniformConstant %95
%u_lightGrid = OpVariable %_ptr_UniformConstant_95 UniformConstant
         %98 = OpTypeImage %float 2D 0 1 0 1 Unknown
         %99 = OpTypeSampledImage %98
%_ptr_UniformConstant_99 = OpTypePointer UniformConstant %99
  %u_texture = OpVariable %_ptr_UniformConstant_99 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
         %70 = OpFunctionCall %void %DrawWorldVS_
               OpReturn
               OpFunctionEnd
%DrawWorldVS_ = OpFunction %void None %3
          %7 = OpLabel
%worldSpacePosition = OpVariable %_ptr_Function_v4float Function
         %19 = OpLoad %v4float %a_color
         %21 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %21 %19
         %25 = OpLoad %v2float %a_texcoord
         %27 = OpAccessChain %_ptr_Output_v2float %_ %int_1
               OpStore %27 %25
         %31 = OpLoad %v3float %a_position
         %33 = OpAccessChain %_ptr_Output_v3float %_ %int_2
               OpStore %33 %31
         %41 = OpAccessChain %_ptr_Uniform_mat4v4float %__0 %int_0
         %42 = OpLoad %mat4v4float %41
         %43 = OpLoad %v3float %a_position
         %45 = OpCompositeExtract %float %43 0
         %46 = OpCompositeExtract %float %43 1
         %47 = OpCompositeExtract %float %43 2
         %48 = OpCompositeConstruct %v4float %45 %46 %47 %float_1
         %49 = OpMatrixTimesVector %v4float %42 %48
               OpStore %worldSpacePosition %49
         %65 = OpAccessChain %_ptr_Uniform_mat4v4float %__2 %int_0 %int_0 %int_1
         %66 = OpLoad %mat4v4float %65
         %67 = OpLoad %v4float %worldSpacePosition
         %68 = OpMatrixTimesVector %v4float %66 %67
         %69 = OpAccessChain %_ptr_Output_v4float %__1 %int_0
               OpStore %69 %68
               OpReturn
               OpFunctionEnd
