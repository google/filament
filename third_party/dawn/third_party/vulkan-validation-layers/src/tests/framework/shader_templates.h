/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

static const char kMinimalShaderGlsl[] = R"glsl(
    #version 460
    void main() {}
)glsl";

static const char kVertexMinimalGlsl[] = R"glsl(
    #version 460
    void main() {
       gl_Position = vec4(1);
    }
)glsl";

// for GPU-AV tests, we need the vertex shader to actually run and produce
// work for the next shader stages
static const char kVertexDrawPassthroughGlsl[] = R"glsl(
    #version 450
    vec2 vertices[3];
    void main(){
        vertices[0] = vec2(-1.0, -1.0);
        vertices[1] = vec2( 1.0, -1.0);
        vertices[2] = vec2( 0.0,  1.0);
        gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    }
)glsl";

static const char kVertexPointSizeGlsl[] = R"glsl(
    #version 460
    out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
    };
    void main() {
        gl_Position = vec4(1);
        gl_PointSize = 1.0;
    }
)glsl";

static char const kGeometryMinimalGlsl[] = R"glsl(
    #version 460
    layout(triangles) in;
    layout(triangle_strip, max_vertices=3) out;
    void main() {
       gl_Position = vec4(1);
       EmitVertex();
    }
)glsl";

static char const kGeometryPointSizeGlsl[] = R"glsl(
    #version 460
    layout (points) in;
    layout (points) out;
    layout (max_vertices = 1) out;
    in gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
    };
    void main() {
       gl_Position = vec4(1);
       gl_PointSize = 1.0;
       EmitVertex();
    }
)glsl";

static const char kTessellationControlMinimalGlsl[] = R"glsl(
    #version 460
    layout(vertices=3) out;
    void main() {
       gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
       gl_TessLevelInner[0] = 1;
    }
)glsl";

static const char kTessellationEvalMinimalGlsl[] = R"glsl(
    #version 460
    layout(triangles, equal_spacing, cw) in;
    void main() { gl_Position = vec4(1); }
)glsl";

static const char kFragmentMinimalGlsl[] = R"glsl(
    #version 460
    layout(location = 0) out vec4 uFragColor;
    void main(){
       uFragColor = vec4(0,1,0,1);
    }
)glsl";

static const char kFragmentSamplerGlsl[] = R"glsl(
    #version 460
    layout(set=0, binding=0) uniform sampler2D s;
    layout(location=0) out vec4 x;
    void main(){
       x = texture(s, vec2(1));
    }
)glsl";

static const char kFragmentUniformGlsl[] = R"glsl(
    #version 460
    layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
    layout(location=0) out vec4 x;
    void main(){
       x = vec4(bar.y);
    }
)glsl";

static char const kFragmentSubpassLoadGlsl[] = R"glsl(
    #version 460
    layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
    void main() {
        vec4 color = subpassLoad(x);
    }
)glsl";

[[maybe_unused]] static const char *kTaskMinimalGlsl = R"glsl(
    #version 460
    #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
    layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
    void main() {
        EmitMeshTasksEXT(1u, 1u, 1u);
    }
)glsl";

[[maybe_unused]] static const char *kMeshMinimalGlsl = R"glsl(
    #version 460
    #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
    layout(max_vertices = 3, max_primitives=1) out;
    layout(triangles) out;
    void main() {}
)glsl";

[[maybe_unused]] static const char *kRayTracingMinimalGlsl = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
    void main() {}
)glsl";

[[maybe_unused]] static char const kRayTracingPayloadMinimalGlsl[] = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : enable
        layout(location = 0) rayPayloadInEXT float hitValue;

        void main() {
            hitValue = 1.0;
        }
    )glsl";

[[maybe_unused]] static const char *kRayTracingNVMinimalGlsl = R"glsl(
    #version 460
    #extension GL_NV_ray_tracing : require
    void main() {}
)glsl";

static char const kShaderTileImageDepthReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageDepthReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %depth_output
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main EarlyFragmentTests
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %depth "depth"
               OpName %depth_output "depth_output"
               OpDecorate %depth_output Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%depth_output = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
      %depth = OpVariable %_ptr_Function_float Function
          %9 = OpDepthAttachmentReadEXT %float
               OpStore %depth %9
         %13 = OpLoad %float %depth
         %14 = OpCompositeConstruct %v4float %13 %13 %13 %13
               OpStore %depth_output %14
               OpReturn
               OpFunctionEnd
        )";

static char const kShaderTileImageStencilReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageStencilReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %stencil_output
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main EarlyFragmentTests
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %stencil "stencil"
               OpName %stencil_output "stencil_output"
               OpDecorate %9 RelaxedPrecision
               OpDecorate %stencil_output Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%stencil_output = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
    %stencil = OpVariable %_ptr_Function_uint Function
          %9 = OpStencilAttachmentReadEXT %uint
               OpStore %stencil %9
         %14 = OpLoad %uint %stencil
         %15 = OpConvertUToF %float %14
         %16 = OpCompositeConstruct %v4float %15 %15 %15 %15
               OpStore %stencil_output %16
               OpReturn
               OpFunctionEnd
        )";

static char const kShaderTileImageColorReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageColorReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color_output
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %color_output "color_output"
               OpName %color_f "color_f"
               OpDecorate %color_output Location 0
               OpDecorate %color_f Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%color_output = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float TileImageDataEXT 0 0 0 2 Unknown
%_ptr_TileImageEXT_10 = OpTypePointer TileImageEXT %10
    %color_f = OpVariable %_ptr_TileImageEXT_10 TileImageEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %color_f
         %14 = OpColorAttachmentReadEXT %v4float %13
               OpStore %color_output %14
               OpReturn
               OpFunctionEnd
        )";

static char const kShaderTileImageDepthStencilReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageDepthReadAccessEXT
               OpCapability TileImageStencilReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main EarlyFragmentTests
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %depth "depth"
               OpName %stencil "stencil"
               OpName %uFragColor "uFragColor"
               OpDecorate %13 RelaxedPrecision
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %3
          %5 = OpLabel
      %depth = OpVariable %_ptr_Function_float Function
    %stencil = OpVariable %_ptr_Function_uint Function
          %9 = OpDepthAttachmentReadEXT %float
               OpStore %depth %9
         %13 = OpStencilAttachmentReadEXT %uint
               OpStore %stencil %13
         %17 = OpLoad %float %depth
         %18 = OpLoad %uint %stencil
         %19 = OpConvertUToF %float %18
         %22 = OpCompositeConstruct %v4float %17 %19 %float_0 %float_1
               OpStore %uFragColor %22
               OpReturn
               OpFunctionEnd
        )";

[[maybe_unused]] static const char kRayGenGlsl[] = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : enable
        layout(set = 0, binding = 0, rgba8) uniform image2D image;
        layout(set = 0, binding = 1) uniform accelerationStructureEXT as;

        layout(location = 0) rayPayloadEXT float payload;

        void main()
        {
           vec4 col = vec4(0, 0, 0, 1);

           vec3 origin = vec3(float(gl_LaunchIDEXT.x)/float(gl_LaunchSizeEXT.x), float(gl_LaunchIDEXT.y)/float(gl_LaunchSizeEXT.y), 1.0);
           vec3 dir = vec3(0.0, 0.0, -1.0);

           payload = 0.5;
           traceRayEXT(as, gl_RayFlagsCullBackFacingTrianglesEXT, 0xff, 0, 1, 0, origin, 0.0, dir, 1000.0, 0);

           col.y = payload;

           imageStore(image, ivec2(gl_LaunchIDEXT.xy), col);
        }
    )glsl";

[[maybe_unused]] static char const *kMissGlsl = kRayTracingPayloadMinimalGlsl;
