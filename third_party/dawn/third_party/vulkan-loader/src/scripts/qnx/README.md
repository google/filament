QNX maintains it's own slightly modified ICD loader.

Instead of using /etc/ for layers and drivers JSONs - it asks Screen Composition Manager for the all settings and fetches the lists from it.

Like it is currently done for OpenGL ES, OpenCL, etc.

More information:
https://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.screen/topic/manual/cscreen_config_intro.html#screen_khronos_egl_display__vk-exps

This QNX build is mostly for the hardware vendors, so they could test Vulkan drivers before QNX with Vulkan support was officially released using the Linux way of drivers loading.

Some customers still can use this if they don't need to run Screen composition manager. For example, for Vulkan-SC used as compute engine and offscreen renderer.
