# Settings

There are many settings that can be set for Validation layers. This is a brief overview of how to use them.

There are 4 ways to configure the settings: `vkconfig`, `application defined`, `vk_layer_settings.txt`, `environment variables`

## VkConfig

We suggest people to use [VkConfig](https://www.lunarg.com/introducing-the-new-vulkan-configurator-vkconfig/).

The GUI comes with the SDK, and takes the `VkLayer_khronos_validation.json` file and does **everything** for you!

## Application Defined

The application can now use the `VK_EXT_layer_settings` extension to do everything at `vkCreateInstance` time. (Don't worry, we implement the extension, so it will be supported 100% of the time!).

```c++
// Example how to turn on verbose mode for DebugPrintf
const VkBool32 verbose_value = true;
const VkLayerSettingEXT layer_setting = {"VK_LAYER_KHRONOS_validation", "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value};
VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

VkInstanceCreateInfo instance_ci = GetYourCreateInfo();
instance_ci.pNext = &layer_settings_create_info;
```

## vk_layer_settings.txt

There is info [elsewhere](https://vulkan.lunarg.com/doc/view/latest/windows/layer_configuration.html) to describe this file, but the short answer is to set the `VK_LAYER_SETTINGS_PATH` like the following:

```
# windows
set VK_LAYER_SETTINGS_PATH=C:\path\to\vk_layer_settings.txt

# linux
export VK_LAYER_SETTINGS_PATH=/path/to/vk_layer_settings.txt
```

and it will set things for you in that file. We have a [default example](../layers/vk_layer_settings.txt) file you can start with.

## Environment Variables

This is done for us via the `vkuCreateLayerSettingSet` call in the [Vulkan-Utility-Libraries](https://github.com/KhronosGroup/Vulkan-Utility-Libraries/).

As an example, in our `VkLayer_khronos_validation.json` file you will find something like `"key": "message_id_filter",`.

From here you just need to adjust it the naming and prefix depending on your platform:

```
# Windows
set VK_LAYER_MESSAGE_ID_FILTER=VUID-VkInstanceCreateInfo-pNext-pNext

# Linux
export VK_LAYER_MESSAGE_ID_FILTER=VUID-VkInstanceCreateInfo-pNext-pNext

# Android
adb setprop debug.vulkan.khronos_validation.message_id_filter=VUID-VkInstanceCreateInfo-pNext-pNext
```

## Finding available settings

How we suggest finding them:

1. Check VkConfig
2. View `VkLayer_khronos_validation.json` (it is where we define them all)
3. In [layer_options.cpp](../layers/layer_options.cpp) (it is where we parse and set the settings)

## Legacy

This is only here to document the legacy of how we got to this situation.

Long ago validation created its own system to parse environmental variables as well as the `vk_layer_settings.txt` flow.

Then we created `VK_EXT_validation_features` as way to set things at `vkCreateInstance` time. `VK_EXT_validation_flags` was also created after with the same goals in mind.

As more and more layers basically needed to do what Validation Layers were doing, we just ended up creating `VK_EXT_layer_settings` as a final solution. This extension has been the way forward to a better system of creating settings for layers.

Unfortunately, we still support some legacy names, so this prevents us from making everything consistent.