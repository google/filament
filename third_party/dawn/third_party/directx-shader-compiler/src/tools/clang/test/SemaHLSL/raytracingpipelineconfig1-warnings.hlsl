// RUN: %dxc -T lib_6_8 -verify %s

// expected-warning@+1{{potential misuse of built-in constant 'RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model lib_6_8; introduced in shader model 6.9}} 
RaytracingPipelineConfig1 rpc = { 32, RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS };


