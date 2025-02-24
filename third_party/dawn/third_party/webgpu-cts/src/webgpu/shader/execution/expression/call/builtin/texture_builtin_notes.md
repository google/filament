# Texture Builtins Issues

Various GPUs and drivers have issues (bugs) related sampling textures. Below is a list of known issues. 
If you'd like to check your own GPU you can run
[all of the WGSL builtin function tests](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,*)
or you can run the individual texture builtin function tests

* [textureLoad](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureLoad:*)
* [textureGather](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureGather:*)
* [textureGatherCompare](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureGatherCompare:*)
* [textureSample](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSample:*)
* [textureSampleBaseClampToEdge](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSampleBaseClampToEdge:*)
* [textureSampleBias](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSampleBias:*)
* [textureSampleCompare](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSampleCompare:*)
* [textureSampleCompareLevel](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSampleCompareLevel:*)
* [textureSampleGrad](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSampleGrad:*)
* [textureSampleLevel](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureSampleLevel:*)
* [textureStore](https://gpuweb.github.io/cts/standalone/?debug=1&runnow=1&q=webgpu:shader,execution,expression,call,builtin,textureStore:*)

Note: A failure does not guarantee a bug in the GPU. Rather, the results need to be inspected.
Some results indicate a bug. Others indicate the test itself might be too sensitive.

------------------------------------------------------------------------------------------------------------
* gpu:Intel Comet Lake S UHD Graphics 630 (8086:9bc5-31.0.101.2127)
* os:Windows-10
* affected: `textureGather` with cube and cube-arrays.

cube maps never wrap - the GPU scales the conversion from cube coordinates to 2d array coordinates
such that it never has to sample across 2 faces like the specs require

Example failure: Below we can see we have an 8x8 cubemap and the conversion from cube coordinate to 2d coordinate
yields texel (7.776, 4.016).  That 7.776 is well past the center of texel 7 and so should sample to the right which
should end up on the next face. But, the GPU did not sample from the next face.

```
webgpu:shader,execution,expression,call,builtin,textureGather:depth_3d_coords:stage="c";format="depth16unorm";mode="c"
--> EXPECTATION FAILED: subcase: samplePoints="cube-edges"
    result was not as expected:
          size: [8, 8, 6]
      mipCount: 3
          call: textureGather(texture: T, sampler: S, coords: vec3f(0.6854506397689122, -0.7281135426590934, -0.002844193526012084))  // #0
              : as 3D texture coord: (0.970703125, 0.501953125, 0.5833333333333334)
              : as texel coord mip level[0]: (7.766, 4.016), face: 3(-y)
              : as texel coord mip level[1]: (3.883, 2.008), face: 3(-y)
              : as texel coord mip level[2]: (1.941, 1.004), face: 3(-y)
           got: 0.68110, 0.24022, 0.15828, 0.13054
      expected: 0.24022, 0.02240, 0.46531, 0.15828
      max diff: 0
     abs diffs: 0.44088, 0.21782, 0.30703, 0.02774
     rel diffs: 64.73%, 90.68%, 65.98%, 17.53%
     ulp diffs: 28893, 14275, 20121, 1818
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0), cube-layer: 0 (+x)                                                   | layer: 0 mip(0), cube-layer: 0 (+x) un-sampled
         0   1   2   3   4   5   6   7                                                    | 
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  | layer: 1 mip(0), cube-layer: 0 (-x) un-sampled
     0 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 2 mip(0), cube-layer: 0 (+y) un-sampled
     1 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 3 mip(0), cube-layer: 0 (-y) 
     2 │   │   │   │   │   │   │   │   │                                                  |      0   1   2   3   4   5   6   7 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
     3 │   │   │   │   │   │   │   │   │                                                  |  0 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     4 │   │   │   │   │   │   │   │   │                                                  |  1 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     5 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     6 │   │   │   │   │   │   │   │   │                                                  |  3 │   │   │   │   │   │   │ a │ b │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     7 │   │   │   │ a │ b │   │   │   │                                                  |  4 │   │   │   │   │   │   │ c │ d │
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
    a: mip(0) at: [ 3,  7,  0], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] |  5 │   │   │   │   │   │   │   │   │
    b: mip(0) at: [ 4,  7,  0], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] |    ├───┼───┼───┼───┼───┼───┼───┼───┤
    a: value: Depth: 0.46531                                                              |  6 │   │   │   │   │   │   │   │   │
    b: value: Depth: 0.02240                                                              |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  7 │   │   │   │   │   │   │   │   │
    layer: 1 mip(0), cube-layer: 0 (-x) un-sampled                                        |    └───┴───┴───┴───┴───┴───┴───┴───┘
                                                                                          | a: mip(0) at: [ 6,  3,  3], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000]
    layer: 2 mip(0), cube-layer: 0 (+y) un-sampled                                        | b: mip(0) at: [ 7,  3,  3], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000]
                                                                                          | c: mip(0) at: [ 6,  4,  3], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000]
    layer: 3 mip(0), cube-layer: 0 (-y)                                                   | d: mip(0) at: [ 7,  4,  3], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000]
         0   1   2   3   4   5   6   7                                                    | a: value: Depth: 0.13054
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  | b: value: Depth: 0.15828
     0 │   │   │   │   │   │   │   │   │                                                  | c: value: Depth: 0.68110
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | d: value: Depth: 0.24022
     1 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 4 mip(0), cube-layer: 0 (+z) un-sampled
     2 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 5 mip(0), cube-layer: 0 (-z) un-sampled
     3 │   │   │   │   │   │   │   │ c │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     4 │   │   │   │   │   │   │   │ d │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     5 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     6 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     7 │   │   │   │   │   │   │   │   │                                                  | 
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  | 
    c: mip(0) at: [ 7,  3,  3], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | 
    d: mip(0) at: [ 7,  4,  3], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | 
    c: value: Depth: 0.15828                                                              | 
    d: value: Depth: 0.24022                                                              | 
                                                                                          | 
    layer: 4 mip(0), cube-layer: 0 (+z) un-sampled                                        | 
                                                                                          | 
    layer: 5 mip(0), cube-layer: 0 (-z) un-sampled                                        | 
    
```

------------------------------------------------------------------------------------------------------------
* gpu:Intel Coffee Lake H UHD Graphics 630 (8086:3e9b)
* os:Mac-14.5
* affected: `textureGather`, `textureGatherCompare` with cube-arrays with all formats.

Only the first array layer is ever selected. 

Example failure: Below you can see the arrayIndex is 1 but the GPU sampled from arrayIndex 0


```
webgpu:shader,execution,expression,call,builtin,textureGather:depth_array_3d_coords:stage="c";format="depth16unorm";mode="c"

--> EXPECTATION FAILED: subcase: samplePoints="cube-edges";A="u32"
    result was not as expected:
          size: [8, 8, 24]
      mipCount: 3
          call: textureGather(texture: T, sampler: S, coords: vec3f(0.6854506397689122, -0.7281135426590934, -0.002844193526012084), arrayIndex: u32(1))  // #0
              : as 3D texture coord: (0.970703125, 0.501953125, 0.5833333333333334)
              : as texel coord mip level[0]: (7.766, 4.016), face: 3(-y)
              : as texel coord mip level[1]: (3.883, 2.008), face: 3(-y)
              : as texel coord mip level[2]: (1.941, 1.004), face: 3(-y)
           got: 0.33806, 0.21810, 0.82548, 0.33309
      expected: 0.07955, 0.02838, 0.79928, 0.37910
      max diff: 0
     abs diffs: 0.25852, 0.18972, 0.02620, 0.04601
     rel diffs: 76.47%, 86.99%, 3.17%, 12.14%
     ulp diffs: 16942, 12433, 1717, 3015
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0), cube-layer: 0 (+x) un-sampled                                        | layer: 0 mip(0), cube-layer: 0 (+x) 
                                                                                          |      0   1   2   3   4   5   6   7 
    layer: 1 mip(0), cube-layer: 0 (-x) un-sampled                                        |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
                                                                                          |  0 │   │   │   │   │   │   │   │   │
    layer: 2 mip(0), cube-layer: 0 (+y) un-sampled                                        |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  1 │   │   │   │   │   │   │   │   │
    layer: 3 mip(0), cube-layer: 0 (-y) un-sampled                                        |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  2 │   │   │   │   │   │   │   │   │
    layer: 4 mip(0), cube-layer: 0 (+z) un-sampled                                        |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  3 │   │   │   │   │   │   │   │   │
    layer: 5 mip(0), cube-layer: 0 (-z) un-sampled                                        |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  4 │   │   │   │   │   │   │   │   │
    layer: 6 mip(0), cube-layer: 1 (+x)                                                   |    ├───┼───┼───┼───┼───┼───┼───┼───┤
         0   1   2   3   4   5   6   7                                                    |  5 │   │   │   │   │   │   │   │   │
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     0 │   │   │   │   │   │   │   │   │                                                  |  6 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     1 │   │   │   │   │   │   │   │   │                                                  |  7 │   │   │   │ a │ b │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    └───┴───┴───┴───┴───┴───┴───┴───┘
     2 │   │   │   │   │   │   │   │   │                                                  | a: mip(0) at: [ 3,  7,  0], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000]
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | b: mip(0) at: [ 4,  7,  0], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000]
     3 │   │   │   │   │   │   │   │   │                                                  | a: value: Depth: 0.82548
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | b: value: Depth: 0.21810
     4 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 1 mip(0), cube-layer: 0 (-x) un-sampled
     5 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 2 mip(0), cube-layer: 0 (+y) un-sampled
     6 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 3 mip(0), cube-layer: 0 (-y) 
     7 │   │   │   │ a │ b │   │   │   │                                                  |      0   1   2   3   4   5   6   7 
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
    a: mip(0) at: [ 3,  7,  6], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] |  0 │   │   │   │   │   │   │   │   │
    b: mip(0) at: [ 4,  7,  6], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] |    ├───┼───┼───┼───┼───┼───┼───┼───┤
    a: value: Depth: 0.79928                                                              |  1 │   │   │   │   │   │   │   │   │
    b: value: Depth: 0.02838                                                              |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  2 │   │   │   │   │   │   │   │   │
    layer: 7 mip(0), cube-layer: 1 (-x) un-sampled                                        |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  3 │   │   │   │   │   │   │   │ c │
    layer: 8 mip(0), cube-layer: 1 (+y) un-sampled                                        |    ├───┼───┼───┼───┼───┼───┼───┼───┤
                                                                                          |  4 │   │   │   │   │   │   │   │ d │
    layer: 9 mip(0), cube-layer: 1 (-y)                                                   |    ├───┼───┼───┼───┼───┼───┼───┼───┤
         0   1   2   3   4   5   6   7                                                    |  5 │   │   │   │   │   │   │   │   │
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     0 │   │   │   │   │   │   │   │   │                                                  |  6 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     1 │   │   │   │   │   │   │   │   │                                                  |  7 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    └───┴───┴───┴───┴───┴───┴───┴───┘
     2 │   │   │   │   │   │   │   │   │                                                  | c: mip(0) at: [ 7,  3,  3], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000]
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | d: mip(0) at: [ 7,  4,  3], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000]
     3 │   │   │   │   │   │   │   │ c │                                                  | c: value: Depth: 0.33309
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | d: value: Depth: 0.33806
     4 │   │   │   │   │   │   │   │ d │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 4 mip(0), cube-layer: 0 (+z) un-sampled
     5 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 5 mip(0), cube-layer: 0 (-z) un-sampled
     6 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 6 mip(0), cube-layer: 1 (+x) un-sampled
     7 │   │   │   │   │   │   │   │   │                                                  | 
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  | layer: 7 mip(0), cube-layer: 1 (-x) un-sampled
    c: mip(0) at: [ 7,  3,  9], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | 
    d: mip(0) at: [ 7,  4,  9], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | layer: 8 mip(0), cube-layer: 1 (+y) un-sampled
    c: value: Depth: 0.37910                                                              | 
    d: value: Depth: 0.07955                                                              | layer: 9 mip(0), cube-layer: 1 (-y) un-sampled
                                                                                          | 
    layer: 10 mip(0), cube-layer: 1 (+z) un-sampled                                       | layer: 10 mip(0), cube-layer: 1 (+z) un-sampled
                                                                                          | 
    layer: 11 mip(0), cube-layer: 1 (-z) un-sampled                                       | layer: 11 mip(0), cube-layer: 1 (-z) un-sampled
                                                                                          | 
    layer: 12 mip(0), cube-layer: 2 (+x) un-sampled                                       | layer: 12 mip(0), cube-layer: 2 (+x) un-sampled
                                                                                          | 
    layer: 13 mip(0), cube-layer: 2 (-x) un-sampled                                       | layer: 13 mip(0), cube-layer: 2 (-x) un-sampled
                                                                                          | 
    layer: 14 mip(0), cube-layer: 2 (+y) un-sampled                                       | layer: 14 mip(0), cube-layer: 2 (+y) un-sampled
                                                                                          | 
    layer: 15 mip(0), cube-layer: 2 (-y) un-sampled                                       | layer: 15 mip(0), cube-layer: 2 (-y) un-sampled
                                                                                          | 
    layer: 16 mip(0), cube-layer: 2 (+z) un-sampled                                       | layer: 16 mip(0), cube-layer: 2 (+z) un-sampled
                                                                                          | 
    layer: 17 mip(0), cube-layer: 2 (-z) un-sampled                                       | layer: 17 mip(0), cube-layer: 2 (-z) un-sampled
                                                                                          | 
    layer: 18 mip(0), cube-layer: 3 (+x) un-sampled                                       | layer: 18 mip(0), cube-layer: 3 (+x) un-sampled
                                                                                          | 
    layer: 19 mip(0), cube-layer: 3 (-x) un-sampled                                       | layer: 19 mip(0), cube-layer: 3 (-x) un-sampled
                                                                                          | 
    layer: 20 mip(0), cube-layer: 3 (+y) un-sampled                                       | layer: 20 mip(0), cube-layer: 3 (+y) un-sampled
                                                                                          | 
    layer: 21 mip(0), cube-layer: 3 (-y) un-sampled                                       | layer: 21 mip(0), cube-layer: 3 (-y) un-sampled
                                                                                          | 
    layer: 22 mip(0), cube-layer: 3 (+z) un-sampled                                       | layer: 22 mip(0), cube-layer: 3 (+z) un-sampled
                                                                                          | 
    layer: 23 mip(0), cube-layer: 3 (-z) un-sampled                                       | layer: 23 mip(0), cube-layer: 3 (-z) un-sampled
```

------------------------------------------------------------------------------------------------------------
* gpu:Intel Coffee Lake H UHD Graphics 630 (8086:3e9b)
* os:Mac-14.5
* affected: `textureGather`, `textureGatherCompare` with cube with stencil8 format

Results are 0 or garbage.

Example Failures: Below on the first failure we see the GPU returned 0. On the 2nd failure we can see the GPU
read some place unrelated to the actual coordinates. Both the wrong face and the wrong location within the face.

```
webgpu:shader,execution,expression,call,builtin,textureGather:sampled_3d_coords:stage="f";format="stencil8";filt="nearest";mode="c"
--> EXPECTATION FAILED: subcase: C="i32";samplePoints="spiral"
    result was not as expected:
          size: [8, 8, 6]
      mipCount: 3
          call: textureGather(component: i32(0), texture: T, sampler: S, coords: vec3f(0.24196863579596511, 0.9529841655964164, 0.1824071254461891))  // #1
              : as 3D texture coord: (0.626953125, 0.595703125, 0.4166666666666667)
              : as texel coord mip level[0]: (5.016, 4.766), face: 2(+y)
              : as texel coord mip level[1]: (2.508, 2.383), face: 2(+y)
              : as texel coord mip level[2]: (1.254, 1.191), face: 2(+y)
           got: 0, 0, 0, 0
      expected: 229, 196, 99, 144
      max diff: 0
     abs diffs: 229, 196, 99, 144
     rel diffs: 100.00%, 100.00%, 100.00%, 100.00%
     ulp diffs: 229, 196, 99, 144
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0), cube-layer: 0 (+x) un-sampled                                        | 
                                                                                          | 
    layer: 1 mip(0), cube-layer: 0 (-x) un-sampled                                        | 
                                                                                          | 
    layer: 2 mip(0), cube-layer: 0 (+y)                                                   | 
         0   1   2   3   4   5   6   7                                                    | 
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  | 
     0 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     1 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     2 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     3 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     4 │   │   │   │   │ a │ b │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     5 │   │   │   │   │ c │ d │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     6 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     7 │   │   │   │   │   │   │   │   │                                                  | 
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  | 
    a: mip(0) at: [ 4,  4,  2], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | 
    b: mip(0) at: [ 5,  4,  2], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] | 
    c: mip(0) at: [ 4,  5,  2], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | 
    d: mip(0) at: [ 5,  5,  2], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] | 
    a: value: Stencil: 144                                                                | 
    b: value: Stencil:  99                                                                | 
    c: value: Stencil: 229                                                                | 
    d: value: Stencil: 196                                                                | 
                                                                                          | 
    layer: 3 mip(0), cube-layer: 0 (-y) un-sampled                                        | 
                                                                                          | 
    layer: 4 mip(0), cube-layer: 0 (+z) un-sampled                                        | 
                                                                                          | 
    layer: 5 mip(0), cube-layer: 0 (-z) un-sampled                                        | 
    

  - INFO: subcase: C="i32";samplePoints="texel-centre"
    OK
  - EXPECTATION FAILED: subcase: C="i32";samplePoints="cube-edges"
    result was not as expected:
          size: [8, 8, 6]
      mipCount: 3
          call: textureGather(component: i32(0), texture: T, sampler: S, coords: vec3f(-0.6854506397689122, -0.002844193526012084, -0.7281135426590934))  // #6
              : as 3D texture coord: (0.970703125, 0.501953125, 0.9166666666666666)
              : as texel coord mip level[0]: (7.766, 4.016), face: 5(-z)
              : as texel coord mip level[1]: (3.883, 2.008), face: 5(-z)
              : as texel coord mip level[2]: (1.941, 1.004), face: 5(-z)
           got: 66, 248, 0, 0
      expected: 31, 189, 27, 28
      max diff: 0
     abs diffs: 35, 59, 27, 28
     rel diffs: 53.03%, 23.79%, 100.00%, 100.00%
     ulp diffs: 35, 59, 27, 28
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0), cube-layer: 0 (+x) un-sampled                                        | layer: 0 mip(0), cube-layer: 0 (+x) un-sampled
                                                                                          | 
    layer: 1 mip(0), cube-layer: 0 (-x)                                                   | layer: 1 mip(0), cube-layer: 0 (-x) 
         0   1   2   3   4   5   6   7                                                    |      0   1   2   3   4   5   6   7 
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
     0 │   │   │   │   │   │   │   │   │                                                  |  0 │ a │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     1 │   │   │   │   │   │   │   │   │                                                  |  1 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     2 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     3 │ a │   │   │   │   │   │   │   │                                                  |  3 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     4 │ b │   │   │   │   │   │   │   │                                                  |  4 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     5 │   │   │   │   │   │   │   │   │                                                  |  5 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     6 │   │   │   │   │   │   │   │   │                                                  |  6 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     7 │   │   │   │   │   │   │   │   │                                                  |  7 │   │   │   │   │   │   │   │   │
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |    └───┴───┴───┴───┴───┴───┴───┴───┘
    a: mip(0) at: [ 0,  3,  1], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] | a: mip(0) at: [ 0,  0,  1], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000]
    b: mip(0) at: [ 0,  4,  1], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] | a: value: Stencil:   0
    a: value: Stencil:  27                                                                | 
    b: value: Stencil: 189                                                                | layer: 2 mip(0), cube-layer: 0 (+y) un-sampled
                                                                                          | 
    layer: 2 mip(0), cube-layer: 0 (+y) un-sampled                                        | layer: 3 mip(0), cube-layer: 0 (-y) un-sampled
                                                                                          | 
    layer: 3 mip(0), cube-layer: 0 (-y) un-sampled                                        | layer: 4 mip(0), cube-layer: 0 (+z) 
                                                                                          |      0   1   2   3   4   5   6   7 
    layer: 4 mip(0), cube-layer: 0 (+z) un-sampled                                        |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
                                                                                          |  0 │   │   │   │   │   │   │   │ b │
    layer: 5 mip(0), cube-layer: 0 (-z)                                                   |    ├───┼───┼───┼───┼───┼───┼───┼───┤
         0   1   2   3   4   5   6   7                                                    |  1 │   │   │   │   │   │   │   │   │
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     0 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     1 │   │   │   │   │   │   │   │   │                                                  |  3 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     2 │   │   │   │   │   │   │   │   │                                                  |  4 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     3 │   │   │   │   │   │   │   │ c │                                                  |  5 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     4 │   │   │   │   │   │   │   │ d │                                                  |  6 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     5 │   │   │   │   │   │   │   │   │                                                  |  7 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    └───┴───┴───┴───┴───┴───┴───┴───┘
     6 │   │   │   │   │   │   │   │   │                                                  | b: mip(0) at: [ 7,  0,  4], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000]
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | b: value: Stencil:  79
     7 │   │   │   │   │   │   │   │   │                                                  | 
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  | layer: 5 mip(0), cube-layer: 0 (-z) un-sampled
    c: mip(0) at: [ 7,  3,  5], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | 
    d: mip(0) at: [ 7,  4,  5], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | 
    c: value: Stencil:  28                                                                | 
    d: value: Stencil:  31                                                                | 
```

------------------------------------------------------------------------------------------------------------
* gpu:Intel Coffee Lake H UHD Graphics 630 (8086:3e9b)
* os:Mac-14.5
* affected: `textureGather`, `textureGatherCompare` with 2d-array with stencil8 format

Results are 0 or garbage.

Example Failure: See that not only did the GPU sample the wrong layer. It sampled the wrong mip level.
`textureGather` always samples mip level 0. There is no parameter to select a different mip level.

```
webgpu:shader,execution,expression,call,builtin,textureGather:sampled_array_2d_coords:stage="c";format="stencil8";filt="nearest";modeU="c";modeV="c";offset=false

  - EXPECTATION FAILED: subcase: samplePoints="spiral";C="u32";A="u32"
    result was not as expected:
          size: [8, 8, 4]
      mipCount: 3
          call: textureGather(component: u32(0), texture: T, sampler: S, coords: vec2f(0.5, 0.5), arrayIndex: u32(2))  // #1
              : as texel coord @ mip level[0]: (4.000, 4.000)
              : as texel coord @ mip level[1]: (2.000, 2.000)
              : as texel coord @ mip level[2]: (1.000, 1.000)
           got: 0, 0, 0, 5
      expected: 35, 217, 187, 99
      max diff: 0
     abs diffs: 35, 217, 187, 94
     rel diffs: 100.00%, 100.00%, 100.00%, 94.95%
     ulp diffs: 35, 217, 187, 94
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0) un-sampled                                                            | layer: 0 mip(1) un-sampled
                                                                                          | 
    layer: 1 mip(0) un-sampled                                                            | layer: 1 mip(1) 
                                                                                          |      0   1   2   3 
    layer: 2 mip(0)                                                                       |    ┌───┬───┬───┬───┐
         0   1   2   3   4   5   6   7                                                    |  0 │   │   │   │   │
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ├───┼───┼───┼───┤
     0 │   │   │   │   │   │   │   │   │                                                  |  1 │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┤
     1 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┤
     2 │   │   │   │   │   │   │   │   │                                                  |  3 │   │   │   │ a │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    └───┴───┴───┴───┘
     3 │   │   │   │ a │ b │   │   │   │                                                  | a: mip(1) at: [ 3,  3,  1], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000]
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | a: value: Stencil:  45
     4 │   │   │   │ c │ d │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 2 mip(1) un-sampled
     5 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 3 mip(1) un-sampled
     6 │   │   │   │   │   │   │   │   │                                                  | 
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | 
     7 │   │   │   │   │   │   │   │   │                                                  | 
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  | 
    a: mip(0) at: [ 3,  3,  2], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | 
    b: mip(0) at: [ 4,  3,  2], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] | 
    c: mip(0) at: [ 3,  4,  2], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | 
    d: mip(0) at: [ 4,  4,  2], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] | 
    a: value: Stencil:  99                                                                | 
    b: value: Stencil: 187                                                                | 
    c: value: Stencil:  35                                                                | 
    d: value: Stencil: 217                                                                | 
                                                                                          | 
    layer: 3 mip(0) un-sampled                                                            | 

```

------------------------------------------------------------------------------------------------------------
* gpu:Intel Coffee Lake H UHD Graphics 630 (8086:3e9b)
* os:Mac-15.0
* affected: `textureSampleGrad` with offset on 2d bc compressed formats

Offsets are wrong. Below we can see we start with coords `0.5, 0.5` so center. We computed mip level 6, clamped to the last
mip level which is 4x4 that means in texel coords we'd have `2.0, 2.0` (center of the mip between texels). The offset is `-2, -3`
so add the offset we're at `0, -1`. `addressModeU` equal `mirror-repeat`, `addressModeV` also equals `mirror-repeat`, so,
the coordinate becomes `0, 1` which is what the software sampler read but it's not what the GPU read.

```
webgpu:shader,execution,expression,call,builtin,textureSampleGrad:sampled_2d_coords:stage="c";format="bc2-rgba-unorm";filt="linear";modeU="m";modeV="m";offset=true

--> EXPECTATION FAILED: subcase: samplePoints="spiral"
    result was not as expected:
          size: [16, 16, 1]
      mipCount: 3
          call: textureSampleGrad(texture: T, sampler: S, coords: vec2f(0.5, 0.5), ddx: vec2f(-2.133585016010329, -3.2209952232660726), ddy: vec2f(-2.03114582283888, -4.092213170486502), offset: vec2(-2, -3))  // #0
              : as texel coord @ mip level[0]: (8.000, 8.000)
              : as texel coord @ mip level[1]: (4.000, 4.000)
              : as texel coord @ mip level[2]: (2.000, 2.000)
    gradient based mip level: 6.191740246150933
           got: 0.9450980424880981, 0.09509804099798203, 0.6284313797950745, 0.6333333253860474
      expected: 0.8705882430076599, 0.1882352977991104, 0.8705882430076599, 0.3333333358168602
      max diff: 0.027450980392156862
     abs diffs: 0.07450979948043823, 0.09313725680112839, 0.24215686321258545, 0.29999998956918716
     rel diffs: 7.88%, 49.48%, 27.82%, 47.37%
     ulp diffs: 1250067, 8257022, 4062718, 7829367
    
    
    ### WARNING: sample points are derived from un-compressed textures and may not match the
    actual GPU results of sampling a compressed texture. The test itself failed at this point
    (see expected: and got: above). We're only trying to determine what the GPU sampled, but
    we can not do that easily with compressed textures. ###
    
      sample points:
    expected:                                                | got:
                                                             | 
    layer: 0 mip(2)                                          | layer: 0 mip(2) 
         0   1   2   3                                       |      0   1   2   3 
       ╔═══╤═══╤═══╤═══╗                                     |    ╔═══╤═══╤═══╤═══╗
     0 ║ a │   │   │   ║                                     |  0 ║   │ a │ b │   ║
       ╟───┼───┼───┼───╢                                     |    ╟───┼───┼───┼───╢
     1 ║ b │   │   │   ║                                     |  1 ║   │ c │ d │   ║
       ╟───┼───┼───┼───╢                                     |    ╟───┼───┼───┼───╢
     2 ║   │   │   │   ║                                     |  2 ║   │   │   │   ║
       ╟───┼───┼───┼───╢                                     |    ╟───┼───┼───┼───╢
     3 ║   │   │   │   ║                                     |  3 ║   │   │   │   ║
       ╚═══╧═══╧═══╧═══╝                                     |    ╚═══╧═══╧═══╧═══╝
    a: mip(2) at: [ 0,  0,  0], weight: 0.50000              | a: mip(2) at: [ 1,  0,  0], weight: 0.25000
    b: mip(2) at: [ 0,  1,  0], weight: 0.50000              | b: mip(2) at: [ 2,  0,  0], weight: 0.25000
    a: value: R: 0.87059, G: 0.18824, B: 0.87059, A: 0.20000 | c: mip(2) at: [ 1,  1,  0], weight: 0.25000
    b: value: R: 0.87059, G: 0.18824, B: 0.87059, A: 0.46667 | d: mip(2) at: [ 2,  1,  0], weight: 0.25000
    mip level (2) weight: 1.00000                            | a: value: R: 0.93725, G: 0.10588, B: 0.65490, A: 0.73333
                                                             | b: value: R: 0.96863, G: 0.06275, B: 0.54902, A: 0.80000
                                                             | c: value: R: 0.93725, G: 0.10588, B: 0.65490, A: 0.93333
                                                             | d: value: R: 0.93725, G: 0.10588, B: 0.65490, A: 0.06667
                                                             | mip level (2) weight: 1.00000
```

* gpu:Intel Coffee Lake H UHD Graphics 630 (8086:3e9b)
* os:Mac-15.0
* affected: `textureSampleGrad` with offset on 3d r16float, r32float, r8snorm, r8unorm, rg11b10ufloat

Offsets are wrong

Example: coord is `0.5, 0.5, 0`. mip level 5 clamped to last level is mip level 2 which is 2x2x2 texels
coord converted to texel coords is `1, 1, 0` (center of first slice). Offset is `0, -4, -4` so adding
them together is `1, -3, -4`. `addressModeU`, `addressModeV`, and `addressModeW` are all `clamp-to-edge
which results in `1, 0, 0`. That's what the software sampler got but not the GPU.

```
webgpu:shader,execution,expression,call,builtin,textureSampleGrad:sampled_3d_coords:stage="c";format="r16float";dim="3d";filt="linear";modeU="c";modeV="c";modeW="c";offset=true

--> EXPECTATION FAILED: subcase: samplePoints="spiral"
    result was not as expected:
          size: [8, 8, 8]
      mipCount: 3
          call: textureSampleGrad(texture: T, sampler: S, coords: vec3f(0.5, 0.5, 0), ddx: vec3f(-1.1805078516481444, -3.0159496716223657, 3.1836248544277623), ddy: vec3f(-3.0395778830861673, 0.0389567477395758, 2.8203667661873624), offset: vec3(0, -4, -4))  // #1
              : as texel coord @ mip level[0]: (4.000, 4.000, 0.000)
              : as texel coord @ mip level[1]: (2.000, 2.000, 0.000)
              : as texel coord @ mip level[2]: (1.000, 1.000, 0.000)
    gradient based mip level: 5.183161751466675
           got: -930.00000, 0.00000, 0.00000, 1.00000
      expected: -561.50000, 0.00000, 0.00000, 1.00000
      max diff: 44
     abs diffs: 368.50000, 0.00000, 0.00000, 0.00000
     rel diffs: 39.62%, 0.00%, 0.00%, 0.00%
     ulp diffs: 737, 0, 0, 0
    
      sample points:
    expected:                                                   | got:
                                                                | 
    layer: 0 mip(2)                                             | layer: 0 mip(2) 
         0   1                                                  |      0   1 
       ┌───┬───┐                                                |    ┌───┬───┐
     0 │ a │ b │                                                |  0 │ a │   │
       ├───┼───┤                                                |    ├───┼───┤
     1 │   │   │                                                |  1 │   │   │
       └───┴───┘                                                |    └───┴───┘
    a: mip(2) at: [ 0,  0,  0], weight: 0.50000                 | a: mip(2) at: [ 0,  0,  0], weight: 1.00000
    b: mip(2) at: [ 1,  0,  0], weight: 0.50000                 | a: value: R: -930.00000, G: 0.00000, B: 0.00000, A: 1.00000
    a: value: R: -930.00000, G: 0.00000, B: 0.00000, A: 1.00000 | mip level (2) weight: 1.00000
    b: value: R: -193.00000, G: 0.00000, B: 0.00000, A: 1.00000 | 
    mip level (2) weight: 1.00000                               | layer: 1 mip(2) un-sampled
                                                                | 
    layer: 1 mip(2) un-sampled                                  | 
    
```

------------------------------------------------------------------------------------------------------------
* gpu:AMD Radeon RX 5500 XT (1002:7340)
* os:Mac-14.4.1
* affected: `textureGather` with sint/uint formats - all viewDimensions

GPU always samples the top left corner

Example failure: See we should have sampled the middle right but the GPU sampled the top left.

```
webgpu:shader,execution,expression,call,builtin,textureGather:sampled_2d_coords:stage="c";format="r16sint";filt="nearest";modeU="c";modeV="c";offset=false

--> EXPECTATION FAILED: subcase: C="i32";samplePoints="texel-centre"
    result was not as expected:
          size: [8, 8, 1]
      mipCount: 3
          call: textureGather(component: i32(0), texture: T, sampler: S, coords: vec2f(0.71875, 0.46875))  // #4
              : as texel coord @ mip level[0]: (5.750, 3.750)
              : as texel coord @ mip level[1]: (2.875, 1.875)
              : as texel coord @ mip level[2]: (1.438, 0.938)
           got: 32448, 19508, 19508, 32448
      expected: 7388, -23326, -18609, 10827
      max diff: 0
     abs diffs: 25060, 42834, 38117, 21621
     rel diffs: 77.23%, 183.63%, 195.39%, 66.63%
     ulp diffs: 25060, 42834, 38117, 21621
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0)                                                                       | layer: 0 mip(0) 
         0   1   2   3   4   5   6   7                                                    |      0   1   2   3   4   5   6   7 
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
     0 │   │   │   │   │   │   │   │   │                                                  |  0 │ a │ b │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     1 │   │   │   │   │   │   │   │   │                                                  |  1 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     2 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     3 │   │   │   │   │   │ a │ b │   │                                                  |  3 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     4 │   │   │   │   │   │ c │ d │   │                                                  |  4 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     5 │   │   │   │   │   │   │   │   │                                                  |  5 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     6 │   │   │   │   │   │   │   │   │                                                  |  6 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     7 │   │   │   │   │   │   │   │   │                                                  |  7 │   │   │   │   │   │   │   │   │
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |    └───┴───┴───┴───┴───┴───┴───┴───┘
    a: mip(0) at: [ 5,  3,  0], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | a: mip(0) at: [ 0,  0,  0], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 1.00000]
    b: mip(0) at: [ 6,  3,  0], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] | b: mip(0) at: [ 1,  0,  0], weights: [R: 0.00000, G: 1.00000, B: 1.00000, A: 0.00000]
    c: mip(0) at: [ 5,  4,  0], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | a: value: R: 32448, G:   0, B:   0, A:   1
    d: mip(0) at: [ 6,  4,  0], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] | b: value: R: 19508, G:   0, B:   0, A:   1
    a: value: R: 10827, G:   0, B:   0, A:   1                                            | 
    b: value: R: -18609, G:   0, B:   0, A:   1                                           | 
    c: value: R: 7388, G:   0, B:   0, A:   1                                             | 
    d: value: R: -23326, G:   0, B:   0, A:   1                                           | 
```

------------------------------------------------------------------------------------------------------------
* gpu:apple:m1 / m2
* os:Mac-15.1.1, Mac-14.4.1
* affected: `textureSampleBias` with 2d and 2d-array coords with offset with repeat or mirror-repeat

GPU computes incorrect offsets

Example: We can see we predicted a mip level of 1.6 so we'll sample both mip level 1 and mip level 2 and both
the software sampler and the GPU appear to have sampled those levels. The texture coord is `0.208, 0.541`
which when converted to texels for level 1 are `1.250, 3.250`. We have an offset of `-2, -3` so adding
those together we get `-0.750, 0.250`. `addressModeU` is `clamp-to-edge` and `addressModeV` is `repeat`.
Only x is out out of range so the texel is `0, 0.250`.  0.250 is above the center of the to row and `addressModeV`
is repeat so we should wrap around to the bottom. The software sampler did this. The GPU did not.

Note: The issue only shows up in the CTS with compressed texture formats. You can repo the issue uncompressed texture formats
by editing `textureSampleBias.spec.ts` and changing `minSize: 8` to `minSize: 12`. Note that similar offsets and address modes are used
with `textureSample`, `textureSampleGrad` `textureSampleLevel` but no issue is seen there. This seems to be solely an issue with
`textureSampleBias`.

```
webgpu:shader,execution,expression,call,builtin,textureSampleBias:arrayed_2d_coords:format="bc7-rgba-unorm-srgb";filt="linear";modeU="c";modeV="r";offset=true

EXPECTATION FAILED: subcase: samplePoints="texel-centre";A="i32"
result was not as expected:
      size: [12, 12, 4]
  mipCount: 3
      call: textureSampleBias(texture: T, sampler: S, coords: vec2f(0.20833333333333334, 0.5416666666666666) + derivativeBase * derivativeMult(vec2f(0, 2445.079343096111)), arrayIndex: i32(0), bias: f32(-9.655665566213429), offset: vec2(-2, -3))  // #0
          : as texel coord @ mip level[0]: (2.500, 6.500)
          : as texel coord @ mip level[1]: (1.250, 3.250)
          : as texel coord @ mip level[2]: (0.625, 1.625)
implicit derivative based mip level: 11.25567 (without bias)
                       clamped bias: -9.65567
                mip level with bias: 1.60000
       got: 0.37631, 0.22320, 0.34188, 1.00000
  expected: 0.37688, 0.34020, 0.34110, 1.00000
  max diff: 0.027450980392156862
 abs diffs: 0.00056, 0.11701, 0.00078, 0.00000
 rel diffs: 0.15%, 34.39%, 0.23%, 0.00%
 ulp diffs: 0, 28, 0, 0


### WARNING: sample points are derived from un-compressed textures and may not match the
actual GPU results of sampling a compressed texture. The test itself failed at this point
(see expected: and got: above). We're only trying to determine what the GPU sampled, but
we can not do that easily with compressed textures. ###

  sample points:
expected:                                                | got:
                                                         | 
layer: 0 mip(1)                                          | layer: 0 mip(1) 
     0   1   2   3   4   5                               |      0   1   2   3   4   5 
   ╔═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╗                     |    ╔═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╗
 0 ║ a │   │   │   ║   │   │░░░│░░░║                     |  0 ║   │   │   │   ║   │   │░░░│░░░║
   ╟───┼───┼───┼───╫───┼───┼───┼───╢                     |    ╟───┼───┼───┼───╫───┼───┼───┼───╢
 1 ║   │   │   │   ║   │   │░░░│░░░║                     |  1 ║   │   │   │   ║   │   │░░░│░░░║
   ╟───┼───┼───┼───╫───┼───┼───┼───╢                     |    ╟───┼───┼───┼───╫───┼───┼───┼───╢
 2 ║   │   │   │   ║   │   │░░░│░░░║                     |  2 ║ a │   │   │   ║   │   │░░░│░░░║
   ╟───┼───┼───┼───╫───┼───┼───┼───╢                     |    ╟───┼───┼───┼───╫───┼───┼───┼───╢
 3 ║   │   │   │   ║   │   │░░░│░░░║                     |  3 ║ b │   │   │   ║   │   │░░░│░░░║
   ╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣                     |    ╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣
 4 ║   │   │   │   ║   │   │░░░│░░░║                     |  4 ║   │   │   │   ║   │   │░░░│░░░║
   ╟───┼───┼───┼───╫───┼───┼───┼───╢                     |    ╟───┼───┼───┼───╫───┼───┼───┼───╢
 5 ║ b │   │   │   ║   │   │░░░│░░░║                     |  5 ║   │   │   │   ║   │   │░░░│░░░║
   ╟───┼───┼───┼───╫───┼───┼───┼───╢                     |    ╟───┼───┼───┼───╫───┼───┼───┼───╢
 6 ║░░░│░░░│░░░│░░░║░░░│░░░│░░░│░░░║                     |  6 ║░░░│░░░│░░░│░░░║░░░│░░░│░░░│░░░║
   ╟───┼───┼───┼───╫───┼───┼───┼───╢                     |    ╟───┼───┼───┼───╫───┼───┼───┼───╢
 7 ║░░░│░░░│░░░│░░░║░░░│░░░│░░░│░░░║                     |  7 ║░░░│░░░│░░░│░░░║░░░│░░░│░░░│░░░║
   ╚═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╝                     |    ╚═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╝
a: mip(1) at: [ 0,  0,  0], weight: 0.30097              | a: mip(1) at: [ 0,  2,  0], weight: 0.10147
b: mip(1) at: [ 0,  5,  0], weight: 0.10032              | b: mip(1) at: [ 0,  3,  0], weight: 0.30466
a: value: R: 0.46203, G: 0.37631, B: 0.37631, A: 1.00000 | a: value: R: 0.37631, G: 0.22320, B: 0.34188, A: 1.00000
b: value: R: 0.12479, G: 0.93016, B: 0.23077, A: 1.00000 | b: value: R: 0.37631, G: 0.22320, B: 0.34188, A: 1.00000
mip level (1) weight: 0.40129                            | mip level (1) weight: 0.40613
                                                         | 
layer: 1 mip(1) un-sampled                               | layer: 1 mip(1) un-sampled
                                                         | 
layer: 2 mip(1) un-sampled                               | layer: 2 mip(1) un-sampled
                                                         | 
layer: 3 mip(1) un-sampled                               | layer: 3 mip(1) un-sampled
                                                         | 
layer: 0 mip(2)                                          | layer: 0 mip(2) 
     0   1   2                                           |      0   1   2 
   ╔═══╤═══╤═══╤═══╗                                     |    ╔═══╤═══╤═══╤═══╗
 0 ║   │   │   │░░░║                                     |  0 ║   │   │   │░░░║
   ╟───┼───┼───┼───╢                                     |    ╟───┼───┼───┼───╢
 1 ║ c │   │   │░░░║                                     |  1 ║ c │   │   │░░░║
   ╟───┼───┼───┼───╢                                     |    ╟───┼───┼───┼───╢
 2 ║ d │   │   │░░░║                                     |  2 ║ d │   │   │░░░║
   ╟───┼───┼───┼───╢                                     |    ╟───┼───┼───┼───╢
 3 ║░░░│░░░│░░░│░░░║                                     |  3 ║░░░│░░░│░░░│░░░║
   ╚═══╧═══╧═══╧═══╝                                     |    ╚═══╧═══╧═══╧═══╝
c: mip(2) at: [ 0,  1,  0], weight: 0.52387              | c: mip(2) at: [ 0,  1,  0], weight: 0.51961
d: mip(2) at: [ 0,  2,  0], weight: 0.07484              | d: mip(2) at: [ 0,  2,  0], weight: 0.07426
c: value: R: 0.37631, G: 0.22320, B: 0.34188, A: 1.00000 | c: value: R: 0.37631, G: 0.22320, B: 0.34188, A: 1.00000
d: value: R: 0.37631, G: 0.22320, B: 0.34188, A: 1.00000 | d: value: R: 0.37631, G: 0.22320, B: 0.34188, A: 1.00000
mip level (2) weight: 0.59871                            | mip level (2) weight: 0.59387
                                                         | 
layer: 1 mip(2) un-sampled                               | layer: 1 mip(2) un-sampled
                                                         | 
layer: 2 mip(2) un-sampled                               | layer: 2 mip(2) un-sampled
                                                         | 
layer: 3 mip(2) un-sampled                               | layer: 3 mip(2) un-sampled
```


------------------------------------------------------------------------------------------------------------
* device_type:Pixel 6 (oriole)
* os:Android
* affected: all texture builtins that take an offset. 

`mirror-repeat` with offset doesn't work.

Example: Below the coords are `0.09375, 0.09375`. Converted to texel coords they are `0.750, 0.750` so just
right and below the center of the top left texel. Adding the offset `-8, -2` takes us to `-7.25, -1.25`.
mirroring takes so to `7.25, 1.25` which is left and above the center of `d` in the "expected:` diagram 
below. The GPU didn't sample there.

```
webgpu:shader,execution,expression,call,builtin,textureGather:sampled_array_2d_coords:stage="f";format="rg8unorm";filt="linear";modeU="m";modeV="m";offset=true

  - EXPECTATION FAILED: subcase: samplePoints="texel-centre";C="i32";A="i32"
    result was not as expected:
          size: [8, 8, 4]
      mipCount: 3
          call: textureGather(component: i32(1), texture: T, sampler: S, coords: vec2f(0.09375, 0.09375), arrayIndex: i32(1), offset: vec2(-8, -2))  // #2
              : as texel coord @ mip level[0]: (0.750, 0.750)
              : as texel coord @ mip level[1]: (0.375, 0.375)
              : as texel coord @ mip level[2]: (0.188, 0.188)
           got: 0.84706, 0.48235, 0.27843, 0.43922
      expected: 0.87843, 0.98824, 0.22745, 0.42353
      max diff: 0.027450980392156862
     abs diffs: 0.03137, 0.50588, 0.05098, 0.01569
     rel diffs: 3.57%, 51.19%, 18.31%, 3.57%
     ulp diffs: 8, 129, 13, 4
    
      sample points:
    expected:                                                                             | got:
                                                                                          | 
    layer: 0 mip(0) un-sampled                                                            | layer: 0 mip(0) un-sampled
                                                                                          | 
    layer: 1 mip(0)                                                                       | layer: 1 mip(0) 
         0   1   2   3   4   5   6   7                                                    |      0   1   2   3   4   5   6   7 
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
     0 │   │   │   │   │   │   │ a │ b │                                                  |  0 │ a │ b │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     1 │   │   │   │   │   │   │ c │ d │                                                  |  1 │ c │ d │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     2 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     3 │   │   │   │   │   │   │   │   │                                                  |  3 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     4 │   │   │   │   │   │   │   │   │                                                  |  4 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     5 │   │   │   │   │   │   │   │   │                                                  |  5 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     6 │   │   │   │   │   │   │   │   │                                                  |  6 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     7 │   │   │   │   │   │   │   │   │                                                  |  7 │   │   │   │   │   │   │   │   │
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |    └───┴───┴───┴───┴───┴───┴───┴───┘
    a: mip(0) at: [ 6,  0,  1], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] | a: mip(0) at: [ 0,  0,  1], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000]
    b: mip(0) at: [ 7,  0,  1], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] | b: mip(0) at: [ 1,  0,  1], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000]
    c: mip(0) at: [ 6,  1,  1], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] | c: mip(0) at: [ 0,  1,  1], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000]
    d: mip(0) at: [ 7,  1,  1], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] | d: mip(0) at: [ 1,  1,  1], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000]
    a: value: R: 0.78824, G: 0.98824, B: 0.00000, A: 1.00000                              | a: value: R: 0.51765, G: 0.84706, B: 0.00000, A: 1.00000
    b: value: R: 0.41569, G: 0.87843, B: 0.00000, A: 1.00000                              | b: value: R: 0.14118, G: 0.48235, B: 0.00000, A: 1.00000
    c: value: R: 0.94510, G: 0.22745, B: 0.00000, A: 1.00000                              | c: value: R: 0.47843, G: 0.43922, B: 0.00000, A: 1.00000
    d: value: R: 0.41176, G: 0.42353, B: 0.00000, A: 1.00000                              | d: value: R: 0.20784, G: 0.27843, B: 0.00000, A: 1.00000
                                                                                          | 
    layer: 2 mip(0) un-sampled                                                            | layer: 2 mip(0) un-sampled
                                                                                          | 
    layer: 3 mip(0) un-sampled                                                            | layer: 3 mip(0) un-sampled
    
```

------------------------------------------------------------------------------------------------------------
* device_type:Pixel 4 (flame)
* os:Android
* affected: `textureSampleGrad` with offset, possible only on 2x2 mip level

GPU returns 0,0,0,0

Example: gradient select mip level 4.9, clamped to last mip level (2). "got: 0,0,0,0"

```
webgpu:shader,execution,expression,call,builtin,textureSampleGrad:sampled_array_2d_coords:stage="c";format="rgba8unorm";filt="linear";modeU="c";modeV="c";offset=true

--> EXPECTATION FAILED: subcase: samplePoints="spiral";A="u32"
    result was not as expected:
          size: [8, 8, 4]
      mipCount: 3
          call: textureSampleGrad(texture: T, sampler: S, coords: vec2f(1.125, 0.21875), arrayIndex: u32(4), ddx: vec2f(-2.033471787115559, 0.8228135952958837), ddy: vec2f(-1.1368875707266852, -3.1678385741543025), offset: vec2(-8, -4))  // #23
              : as texel coord @ mip level[0]: (9.000, 1.750)
              : as texel coord @ mip level[1]: (4.500, 0.875)
              : as texel coord @ mip level[2]: (2.250, 0.438)
    gradient based mip level: 4.750892457451435
           got: 0.00000, 0.00000, 0.00000, 0.00000
      expected: 0.81176, 0.22745, 0.36471, 0.48627
      max diff: 0.027450980392156862
     abs diffs: 0.81176, 0.22745, 0.36471, 0.48627
     rel diffs: 100.00%, 100.00%, 100.00%, 100.00%
     ulp diffs: 207, 58, 93, 124
    
      sample points:
    expected:                                                | got:
                                                             | 
    layer: 0 mip(2) un-sampled                               | layer: 0 mip(2) un-sampled
                                                             | 
    layer: 1 mip(2) un-sampled                               | layer: 1 mip(2) un-sampled
                                                             | 
    layer: 2 mip(2) un-sampled                               | layer: 2 mip(2) un-sampled
                                                             | 
    layer: 3 mip(2)                                          | layer: 3 mip(2) 
         0   1                                               |      0   1 
       ┌───┬───┐                                             |    ┌───┬───┐
     0 │ a │   │                                             |  0 │ a │   │
       ├───┼───┤                                             |    ├───┼───┤
     1 │   │   │                                             |  1 │   │   │
       └───┴───┘                                             |    └───┴───┘
    a: mip(2) at: [ 0,  0,  3], weight: 1.00000              | a: mip(2) at: [ 0,  0,  3], weight: 1.00000
    a: value: R: 0.81176, G: 0.22745, B: 0.36471, A: 0.48627 | a: value: R: 0.81176, G: 0.22745, B: 0.36471, A: 0.48627
    mip level (2) weight: 1.00000                            | mip level (2) weight: 1.00000
    
```

------------------------------------------------------------------------------------------------------------
* gpu: AMD Radeon Pro WX 3200 (radeonsi polaris12 LLVM 17.0.6)) DRIVER_VENDOR=Mesa, DRIVER_VERSION=24.0.6  
* os: Linux
* affected: `textureGather` and `textureGatherCompare` with cube and cube-array and integer texture formats

samples never wrap at edges.

Example failure: Below we have a 8x8 cubemap. The conversion from cube coords to texel coords results in
texel (0.016, 5.766) in the -y face. tx = 0.016 means the coord is left of center of the left most texel and so should wrap
of the -y to the -x face. We can see it does in the software sampler but not in GPU.


```
webgpu:shader,execution,expression,call,builtin,textureGather:sampled_3d_coords:stage="c";format="r8uint";filt="nearest";mode="r" - fail:
-->  - EXPECTATION FAILED: subcase: C="i32";samplePoints="spiral"
    result was not as expected:
          size: [8, 8, 6]
      mipCount: 3
          call: textureGather(component: i32(0), texture: T, sampler: S, coords: vec3f(-0.6735527753936539, -0.6761941588265702, -0.29847632791954076))  // #13
              : as 3D texture coord: (0.001953125, 0.720703125, 0.5833333333333334)
              : as texel coord mip level[0]: (0.016, 5.766), face: 3(-y)
              : as texel coord mip level[1]: (0.008, 2.883), face: 3(-y)
              : as texel coord mip level[2]: (0.004, 1.441), face: 3(-y)
           got: 185, 100, 31, 164
      expected: 102, 185, 164, 234
      max diff: 0
     abs diffs: 83, 85, 133, 70
     rel diffs: 44.86%, 45.95%, 81.10%, 29.91%
     ulp diffs: 83, 85, 133, 70

      sample points:
    expected:                                                                             | got:
                                                                                          |
    layer: 0 mip(0), cube-layer: 0 (+x) un-sampled                                        | layer: 0 mip(0), cube-layer: 0 (+x) un-sampled
                                                                                          |
    layer: 1 mip(0), cube-layer: 0 (-x)                                                   | layer: 1 mip(0), cube-layer: 0 (-x) un-sampled
         0   1   2   3   4   5   6   7                                                    |
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  | layer: 2 mip(0), cube-layer: 0 (+y) un-sampled
     0 │   │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 3 mip(0), cube-layer: 0 (-y)
     1 │   │   │   │   │   │   │   │   │                                                  |      0   1   2   3   4   5   6   7
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ┌───┬───┬───┬───┬───┬───┬───┬───┐
     2 │   │   │   │   │   │   │   │   │                                                  |  0 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     3 │   │   │   │   │   │   │   │   │                                                  |  1 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     4 │   │   │   │   │   │   │   │   │                                                  |  2 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     5 │   │   │   │   │   │   │   │   │                                                  |  3 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     6 │   │   │   │   │   │   │   │   │                                                  |  4 │   │   │   │   │   │   │   │   │
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
     7 │   │ a │ b │   │   │   │   │   │                                                  |  5 │ a │ b │   │   │   │   │   │   │
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |    ├───┼───┼───┼───┼───┼───┼───┼───┤
    a: mip(0) at: [ 1,  7,  1], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000] |  6 │ c │ d │   │   │   │   │   │   │
    b: mip(0) at: [ 2,  7,  1], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000] |    ├───┼───┼───┼───┼───┼───┼───┼───┤
    a: value: R: 102, G:   0, B:   0, A:   1                                              |  7 │   │   │   │   │   │   │   │   │
    b: value: R: 234, G:   0, B:   0, A:   1                                              |    └───┴───┴───┴───┴───┴───┴───┴───┘
                                                                                          | a: mip(0) at: [ 0,  5,  3], weights: [R: 0.00000, G: 0.00000, B: 0.00000, A: 1.00000]
    layer: 2 mip(0), cube-layer: 0 (+y) un-sampled                                        | b: mip(0) at: [ 1,  5,  3], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000]
                                                                                          | c: mip(0) at: [ 0,  6,  3], weights: [R: 1.00000, G: 0.00000, B: 0.00000, A: 0.00000]
    layer: 3 mip(0), cube-layer: 0 (-y)                                                   | d: mip(0) at: [ 1,  6,  3], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000]
         0   1   2   3   4   5   6   7                                                    | a: value: R: 164, G:   0, B:   0, A:   1
       ┌───┬───┬───┬───┬───┬───┬───┬───┐                                                  | b: value: R:  31, G:   0, B:   0, A:   1
     0 │   │   │   │   │   │   │   │   │                                                  | c: value: R: 185, G:   0, B:   0, A:   1
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | d: value: R: 100, G:   0, B:   0, A:   1
     1 │   │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 4 mip(0), cube-layer: 0 (+z) un-sampled
     2 │   │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  | layer: 5 mip(0), cube-layer: 0 (-z) un-sampled
     3 │   │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |
     4 │   │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |
     5 │ c │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |
     6 │ d │   │   │   │   │   │   │   │                                                  |
       ├───┼───┼───┼───┼───┼───┼───┼───┤                                                  |
     7 │   │   │   │   │   │   │   │   │                                                  |
       └───┴───┴───┴───┴───┴───┴───┴───┘                                                  |
    c: mip(0) at: [ 0,  5,  3], weights: [R: 0.00000, G: 0.00000, B: 1.00000, A: 0.00000] |
    d: mip(0) at: [ 0,  6,  3], weights: [R: 0.00000, G: 1.00000, B: 0.00000, A: 0.00000] |
    c: value: R: 164, G:   0, B:   0, A:   1                                              |
    d: value: R: 185, G:   0, B:   0, A:   1                                              |
                                                                                          |
    layer: 4 mip(0), cube-layer: 0 (+z) un-sampled                                        |
                                                                                          |
    layer: 5 mip(0), cube-layer: 0 (-z) un-sampled                                        |
```
