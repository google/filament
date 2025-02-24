# Metal Backend Shaders

ANGLE's Metal backend provides a couple of features for inspecting the
Metal Shading Language (MSL) produced by ANGLE's shader translator.

## printMetalShaders

The [`printMetalShaders`
feature](https://chromium-review.googlesource.com/c/angle/angle/+/4480976),
which can be enabled in Chromium via the command line argument
`--enable-angle-features=printMetalShaders`, dumps the blob cache key
and translated MSL for all shaders compiled by ANGLE. This is
currently used to [regenerate precompiled
shaders](https://crbug.com/1423136) shipped with Chrome.

## Environment variable

Setting the environment variable `ANGLE_METAL_PRINT_MSL_ENABLE` to `1`
indicates to the Metal backend to print the translated shaders as
they're compiled:

```
export ANGLE_METAL_PRINT_MSL_ENABLE=1
/Applications/Google\ Chrome\ Canary.app/Contents/MacOS/Google\ Chrome\ Canary --use-angle=metal
```

To pass this environment to WebKit / Safari's GPU process, set the
environment variable `__XPC_ANGLE_METAL_PRINT_MSL_ENABLE` to `1`:

```
export __XPC_ANGLE_METAL_PRINT_MSL_ENABLE=1
/Applications/Safari\ Technology\ Preview.app/Contents/MacOS/Safari\ Technology\ Preview
```
