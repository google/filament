# Enabling Metal validation layers

To enable Metal's validation layers in both ANGLE's standalone test
suites as well as in Chromium's GPU process, per [this
CL](https://chromium-review.googlesource.com/c/chromium/src/+/3584863),
set the following environment variables:

```
MTL_DEBUG_LAYER=1
MTL_DEBUG_LAYER_VALIDATE_LOAD_ACTIONS=1
MTL_DEBUG_LAYER_VALIDATE_STORE_ACTIONS=1
MTL_DEBUG_LAYER_VALIDATE_UNRETAINED_RESOURCES=4
```
