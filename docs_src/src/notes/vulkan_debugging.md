# Debugging Vulkan

## Enable Validation Logs

Simply install the LunarG SDK (it's fast and easy), then make sure you've got the following
environment variables set up in your **bashrc** file. For example:

```
export VULKAN_SDK='/path_to_home/VulkanSDK/1.3.216.0/x86_64'
export VK_LAYER_PATH="$VULKAN_SDK/etc/explicit_layer.d"
export PATH="$VULKAN_SDK/bin:$PATH"
```

As long as you're running a debug build of Filament, you should now see extra debugging spew in your
console if there are any errors or performance issues being caught by validation.
