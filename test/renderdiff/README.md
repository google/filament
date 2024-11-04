# Rendering Difference Test

We created a few scripts to run `gltf_viewer` and produce headless renderings.

This is mainly useful for continuous integration where GPUs are generally not available on cloud
machines. To perform software rasterization, these scripts are centered around [Mesa]'s software
rasterizers, but nothing bars us from using another rasterizer like [SwiftShader]. Additionally,
we should be able to use GPUs where available (though this is more of a future work).

The script `run.py` contains the core logic for taking input parameters (such as the test
description file) and then running gltf_viewer to produce the results.

In the `test` directory is a list of test descriptions that are specified in json.  Please see
`sample.json` to parse the structure.

[Mesa]: https://docs.mesa3d.org
[SwiftShader]: https://github.com/google/swiftshader