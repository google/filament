<?php
$static_title = 'Khronos EGL Registry';

include_once("../../assets/static_pages/khr_page_top.php");
?>

<p> The EGL registry contains specifications of the core API; specifications
    of Khronos- and vendor-approved EGL extensions; header files
    corresponding to the specifications; an enumerant and function registry;
    and other related documentation. </p>

<h6> EGL Core API Specification and Headers </h6>

<p> The current version of EGL is EGL 1.5. </p>

<ul>
<li> <a href="specs/eglspec.1.5.pdf"> EGL 1.5 Specification </a> (updated
     August 27, 2014) and <a href="specs/eglspec.1.5.withchanges.pdf">
     Specification with changes marked </a>. </li>
<li> <a href="api/EGL/egl.h"> &lt;EGL/egl.h&gt; </a> for EGL 1.5. </li>
<li> <a href="api/EGL/eglext.h"> &lt;EGL/eglext.h&gt; </a> for EGL 1.5. </li>
<li> <a href="api/EGL/eglplatform.h"> &lt;EGL/eglplatform.h&gt; </a> for EGL
     1.5. </li>
<li> <a href="api/KHR/khrplatform.h"> &lt;KHR/khrplatform.h&gt; </a>
     (required by the current EGL and OpenGL ES headers). </li>
</ul>

<h6> Reference Pages, Technical Notes and White Papers </h6>

<ul>
<li> <a href="sdk/docs/man/"> EGL 1.5 reference pages </a>
<li> <a href="specs/EGLTechNote0001.html">EGL Technical Note #1 - EGL 1.4 and
     ancillary buffer preservation </a>
</ul>

<h6> Older Specifications </h6>

<p> Older versions of the EGL Specification provided for reference. </p>

<ul>
<li> <a href="specs/eglspec.1.4.pdf"> EGL 1.4 Specification </a> and
     <a href="specs/eglspec.1.4.withchanges.pdf"> Specification with changes
     marked </a>. </li>
<li> <a href="specs/eglspec.1.3.pdf"> EGL 1.3 Specification </a> </li>
<li> <a href="specs/eglspec.1.2.pdf"> EGL 1.2 Specification </a>
     and corresponding
     <a href="api/1.2/EGL/egl.h"> &lt;EGL/egl.h&gt; </a>. </li>
<li> <a href="specs/eglspec.1.1.pdf"> EGL 1.1 Specification </a>
     and corresponding
     <a href="api/1.1/EGL/egl.h"> &lt;EGL/egl.h&gt; </a>. </li>
<li> <a href="specs/eglspec.1.0.pdf"> EGL 1.0 Specification </a>
     and corresponding
     <a href="api/1.0/EGL/egl.h"> &lt;EGL/egl.h&gt; </a>. </li>
</ul>

<h6> EGL XML API Registry </h6>

<p> The database from which EGL enumerant ranges are reserved and the
    <tt>EGL/egl.h</tt> and <tt>EGL/eglext.h</tt> headers are built is called
    <a href="api/egl.xml"> egl.xml </a>. It uses an XML schema and
    processing scripts shared with the OpenGL and OpenGL ES registries. If
    you need to generate modified headers or modify egl.xml, clone the <a
    href="https://github.com/KhronosGroup/EGL-Registry"> EGL-Registry </a>
    git repository and see the <a href="README.md"> README.md </a>
    file to get started. </p>


<h6> <a name="otherextspecs"></a>
     Extension Specifications</h6>
<ol>
<li value=1> <a href="extensions/KHR/EGL_KHR_config_attribs.txt">EGL_KHR_config_attribs</a>
</li>
<li value=2> <a href="extensions/KHR/EGL_KHR_lock_surface.txt">EGL_KHR_lock_surface</a>
</li>
<li value=3> <a href="extensions/KHR/EGL_KHR_image.txt">EGL_KHR_image</a>
</li>
<li value=4> <a href="extensions/KHR/EGL_KHR_vg_parent_image.txt">EGL_KHR_vg_parent_image</a>
</li>
<li value=5> <a href="extensions/KHR/EGL_KHR_gl_image.txt">EGL_KHR_gl_texture_2D_image</a>
     <br> <a href="extensions/KHR/EGL_KHR_gl_image.txt">EGL_KHR_gl_texture_cubemap_image</a>
     <br> <a href="extensions/KHR/EGL_KHR_gl_image.txt">EGL_KHR_gl_texture_3D_image</a>
     <br> <a href="extensions/KHR/EGL_KHR_gl_image.txt">EGL_KHR_gl_renderbuffer_image</a>
</li>
<li value=6> <a href="extensions/KHR/EGL_KHR_reusable_sync.txt">EGL_KHR_reusable_sync</a>
</li>
<li value=8> <a href="extensions/KHR/EGL_KHR_image_base.txt">EGL_KHR_image_base</a>
</li>
<li value=9> <a href="extensions/KHR/EGL_KHR_image_pixmap.txt">EGL_KHR_image_pixmap</a>
</li>
<li value=10> <a href="extensions/IMG/EGL_IMG_context_priority.txt">EGL_IMG_context_priority</a>
</li>
<li value=14> <a href="extensions/NOK/EGL_NOK_texture_from_pixmap.txt">EGL_NOK_texture_from_pixmap</a>
</li>
<li value=16> <a href="extensions/KHR/EGL_KHR_lock_surface2.txt">EGL_KHR_lock_surface2</a>
</li>
<li value=17> <a href="extensions/NV/EGL_NV_coverage_sample.txt">EGL_NV_coverage_sample</a>
</li>
<li value=18> <a href="extensions/NV/EGL_NV_depth_nonlinear.txt">EGL_NV_depth_nonlinear</a>
</li>
<li value=19> <a href="extensions/NV/EGL_NV_sync.txt">EGL_NV_sync</a>
</li>
<li value=20> <a href="extensions/KHR/EGL_KHR_fence_sync.txt">EGL_KHR_fence_sync</a>
</li>
<li value=23> <a href="extensions/NOK/EGL_NOK_swap_region2.txt">EGL_NOK_swap_region2</a>
</li>
<li value=24> <a href="extensions/HI/EGL_HI_clientpixmap.txt">EGL_HI_clientpixmap</a>
</li>
<li value=25> <a href="extensions/HI/EGL_HI_colorformats.txt">EGL_HI_colorformats</a>
</li>
<li value=26> <a href="extensions/MESA/EGL_MESA_drm_image.txt">EGL_MESA_drm_image</a>
</li>
<li value=27> <a href="extensions/NV/EGL_NV_post_sub_buffer.txt">EGL_NV_post_sub_buffer</a>
</li>
<li value=28> <a href="extensions/ANGLE/EGL_ANGLE_query_surface_pointer.txt">EGL_ANGLE_query_surface_pointer</a>
</li>
<li value=29> <a href="extensions/ANGLE/EGL_ANGLE_surface_d3d_texture_2d_share_handle.txt">EGL_ANGLE_surface_d3d_texture_2d_share_handle</a>
</li>
<li value=30> <a href="extensions/NV/EGL_NV_coverage_sample_resolve.txt">EGL_NV_coverage_sample_resolve</a>
</li>
<li value=31> <a href="extensions/NV/EGL_NV_system_time.txt">EGL_NV_system_time</a>
</li>
<li value=32> <a href="extensions/KHR/EGL_KHR_stream.txt">EGL_KHR_stream</a>
     <br> <a href="extensions/KHR/EGL_KHR_stream.txt">EGL_KHR_stream_attrib</a>
</li>
<li value=33> <a href="extensions/KHR/EGL_KHR_stream_consumer_gltexture.txt">EGL_KHR_stream_consumer_gltexture</a>
</li>
<li value=34> <a href="extensions/KHR/EGL_KHR_stream_producer_eglsurface.txt">EGL_KHR_stream_producer_eglsurface</a>
</li>
<li value=35> <a href="extensions/KHR/EGL_KHR_stream_producer_aldatalocator.txt">EGL_KHR_stream_producer_aldatalocator</a>
</li>
<li value=36> <a href="extensions/KHR/EGL_KHR_stream_fifo.txt">EGL_KHR_stream_fifo</a>
</li>
<li value=37> <a href="extensions/EXT/EGL_EXT_create_context_robustness.txt">EGL_EXT_create_context_robustness</a>
</li>
<li value=38> <a href="extensions/ANGLE/EGL_ANGLE_d3d_share_handle_client_buffer.txt">EGL_ANGLE_d3d_share_handle_client_buffer</a>
</li>
<li value=39> <a href="extensions/KHR/EGL_KHR_create_context.txt">EGL_KHR_create_context</a>
</li>
<li value=40> <a href="extensions/KHR/EGL_KHR_surfaceless_context.txt">EGL_KHR_surfaceless_context</a>
</li>
<li value=41> <a href="extensions/KHR/EGL_KHR_stream_cross_process_fd.txt">EGL_KHR_stream_cross_process_fd</a>
</li>
<li value=42> <a href="extensions/EXT/EGL_EXT_multiview_window.txt">EGL_EXT_multiview_window</a>
</li>
<li value=43> <a href="extensions/KHR/EGL_KHR_wait_sync.txt">EGL_KHR_wait_sync</a>
</li>
<li value=44> <a href="extensions/NV/EGL_NV_post_convert_rounding.txt">EGL_NV_post_convert_rounding</a>
</li>
<li value=45> <a href="extensions/NV/EGL_NV_native_query.txt">EGL_NV_native_query</a>
</li>
<li value=46> <a href="extensions/NV/EGL_NV_3dvision_surface.txt">EGL_NV_3dvision_surface</a>
</li>
<li value=47> <a href="extensions/ANDROID/EGL_ANDROID_framebuffer_target.txt">EGL_ANDROID_framebuffer_target</a>
</li>
<li value=48> <a href="extensions/ANDROID/EGL_ANDROID_blob_cache.txt">EGL_ANDROID_blob_cache</a>
</li>
<li value=49> <a href="extensions/ANDROID/EGL_ANDROID_image_native_buffer.txt">EGL_ANDROID_image_native_buffer</a>
</li>
<li value=50> <a href="extensions/ANDROID/EGL_ANDROID_native_fence_sync.txt">EGL_ANDROID_native_fence_sync</a>
</li>
<li value=51> <a href="extensions/ANDROID/EGL_ANDROID_recordable.txt">EGL_ANDROID_recordable</a>
</li>
<li value=52> <a href="extensions/EXT/EGL_EXT_buffer_age.txt">EGL_EXT_buffer_age</a>
</li>
<li value=53> <a href="extensions/EXT/EGL_EXT_image_dma_buf_import.txt">EGL_EXT_image_dma_buf_import</a>
</li>
<li value=54> <a href="extensions/ARM/EGL_ARM_pixmap_multisample_discard.txt">EGL_ARM_pixmap_multisample_discard</a>
</li>
<li value=55> <a href="extensions/EXT/EGL_EXT_swap_buffers_with_damage.txt">EGL_EXT_swap_buffers_with_damage</a>
</li>
<li value=56> <a href="extensions/NV/EGL_NV_stream_sync.txt">EGL_NV_stream_sync</a>
</li>
<li value=57> <a href="extensions/EXT/EGL_EXT_platform_base.txt">EGL_EXT_platform_base</a>
</li>
<li value=58> <a href="extensions/EXT/EGL_EXT_client_extensions.txt">EGL_EXT_client_extensions</a>
</li>
<li value=59> <a href="extensions/EXT/EGL_EXT_platform_x11.txt">EGL_EXT_platform_x11</a>
</li>
<li value=60> <a href="extensions/KHR/EGL_KHR_cl_event.txt">EGL_KHR_cl_event</a>
</li>
<li value=61> <a href="extensions/KHR/EGL_KHR_get_all_proc_addresses.txt">EGL_KHR_get_all_proc_addresses</a>
     <br> <a href="extensions/KHR/EGL_KHR_get_all_proc_addresses.txt">EGL_KHR_client_get_all_proc_addresses</a>
</li>
<li value=62> <a href="extensions/MESA/EGL_MESA_platform_gbm.txt">EGL_MESA_platform_gbm</a>
</li>
<li value=63> <a href="extensions/EXT/EGL_EXT_platform_wayland.txt">EGL_EXT_platform_wayland</a>
</li>
<li value=64> <a href="extensions/KHR/EGL_KHR_lock_surface3.txt">EGL_KHR_lock_surface3</a>
</li>
<li value=65> <a href="extensions/KHR/EGL_KHR_cl_event2.txt">EGL_KHR_cl_event2</a>
</li>
<li value=66> <a href="extensions/KHR/EGL_KHR_gl_colorspace.txt">EGL_KHR_gl_colorspace</a>
</li>
<li value=67> <a href="extensions/EXT/EGL_EXT_protected_surface.txt">EGL_EXT_protected_surface</a>
</li>
<li value=68> <a href="extensions/KHR/EGL_KHR_platform_android.txt">EGL_KHR_platform_android</a>
</li>
<li value=69> <a href="extensions/KHR/EGL_KHR_platform_gbm.txt">EGL_KHR_platform_gbm</a>
</li>
<li value=70> <a href="extensions/KHR/EGL_KHR_platform_wayland.txt">EGL_KHR_platform_wayland</a>
</li>
<li value=71> <a href="extensions/KHR/EGL_KHR_platform_x11.txt">EGL_KHR_platform_x11</a>
</li>
<li value=72> <a href="extensions/EXT/EGL_EXT_device_base.txt">EGL_EXT_device_base</a>
</li>
<li value=73> <a href="extensions/EXT/EGL_EXT_platform_device.txt">EGL_EXT_platform_device</a>
</li>
<li value=74> <a href="extensions/NV/EGL_NV_device_cuda.txt">EGL_NV_device_cuda</a>
</li>
<li value=75> <a href="extensions/NV/EGL_NV_cuda_event.txt">EGL_NV_cuda_event</a>
</li>
<li value=76> <a href="extensions/TIZEN/EGL_TIZEN_image_native_buffer.txt">EGL_TIZEN_image_native_buffer</a>
</li>
<li value=77> <a href="extensions/TIZEN/EGL_TIZEN_image_native_surface.txt">EGL_TIZEN_image_native_surface</a>
</li>
<li value=78> <a href="extensions/EXT/EGL_EXT_output_base.txt">EGL_EXT_output_base</a>
</li>
<li value=79> <a href="extensions/EXT/EGL_EXT_device_drm.txt">EGL_EXT_device_drm</a>
     <br> <a href="extensions/EXT/EGL_EXT_device_drm.txt">EGL_EXT_output_drm</a>
</li>
<li value=80> <a href="extensions/EXT/EGL_EXT_device_openwf.txt">EGL_EXT_device_openwf</a>
     <br> <a href="extensions/EXT/EGL_EXT_device_openwf.txt">EGL_EXT_output_openwf</a>
</li>
<li value=81> <a href="extensions/EXT/EGL_EXT_stream_consumer_egloutput.txt">EGL_EXT_stream_consumer_egloutput</a>
</li>
<li value=83> <a href="extensions/KHR/EGL_KHR_partial_update.txt">EGL_KHR_partial_update</a>
</li>
<li value=84> <a href="extensions/KHR/EGL_KHR_swap_buffers_with_damage.txt">EGL_KHR_swap_buffers_with_damage</a>
</li>
<li value=85> <a href="extensions/ANGLE/EGL_ANGLE_window_fixed_size.txt">EGL_ANGLE_window_fixed_size</a>
</li>
<li value=86> <a href="extensions/EXT/EGL_EXT_yuv_surface.txt">EGL_EXT_yuv_surface</a>
</li>
<li value=87> <a href="extensions/MESA/EGL_MESA_image_dma_buf_export.txt">EGL_MESA_image_dma_buf_export</a>
</li>
<li value=88> <a href="extensions/EXT/EGL_EXT_device_enumeration.txt">EGL_EXT_device_enumeration</a>
</li>
<li value=89> <a href="extensions/EXT/EGL_EXT_device_query.txt">EGL_EXT_device_query</a>
</li>
<li value=90> <a href="extensions/ANGLE/EGL_ANGLE_device_d3d.txt">EGL_ANGLE_device_d3d</a>
</li>
<li value=91> <a href="extensions/KHR/EGL_KHR_create_context_no_error.txt">EGL_KHR_create_context_no_error</a>
</li>
<li value=92> <a href="extensions/KHR/EGL_KHR_debug.txt">EGL_KHR_debug</a>
</li>
<li value=93> <a href="extensions/NV/EGL_NV_stream_metadata.txt">EGL_NV_stream_metadata</a>
</li>
<li value=94> <a href="extensions/NV/EGL_NV_stream_consumer_gltexture_yuv.txt">EGL_NV_stream_consumer_gltexture_yuv</a>
</li>
<li value=95> <a href="extensions/IMG/EGL_IMG_image_plane_attribs.txt">EGL_IMG_image_plane_attribs</a>
</li>
<li value=96> <a href="extensions/KHR/EGL_KHR_mutable_render_buffer.txt">EGL_KHR_mutable_render_buffer</a>
</li>
<li value=97> <a href="extensions/EXT/EGL_EXT_protected_content.txt">EGL_EXT_protected_content</a>
</li>
<li value=98> <a href="extensions/ANDROID/EGL_ANDROID_presentation_time.txt">EGL_ANDROID_presentation_time</a>
</li>
<li value=99> <a href="extensions/ANDROID/EGL_ANDROID_create_native_client_buffer.txt">EGL_ANDROID_create_native_client_buffer</a>
</li>
<li value=100> <a href="extensions/ANDROID/EGL_ANDROID_front_buffer_auto_refresh.txt">EGL_ANDROID_front_buffer_auto_refresh</a>
</li>
<li value=101> <a href="extensions/KHR/EGL_KHR_no_config_context.txt">EGL_KHR_no_config_context</a>
</li>
<li value=102> <a href="https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_context_flush_control.txt">EGL_KHR_context_flush_control</a>
</li>
<li value=103> <a href="extensions/ARM/EGL_ARM_implicit_external_sync.txt">EGL_ARM_implicit_external_sync</a>
</li>
<li value=104> <a href="extensions/MESA/EGL_MESA_platform_surfaceless.txt">EGL_MESA_platform_surfaceless</a>
</li>
<li value=105> <a href="extensions/EXT/EGL_EXT_image_dma_buf_import_modifiers.txt">EGL_EXT_image_dma_buf_import_modifiers</a>
</li>
<li value=106> <a href="extensions/EXT/EGL_EXT_pixel_format_float.txt">EGL_EXT_pixel_format_float</a>
</li>
<li value=107> <a href="extensions/EXT/EGL_EXT_gl_colorspace_bt2020_linear.txt">EGL_EXT_gl_colorspace_bt2020_linear</a>
     <br> <a href="extensions/EXT/EGL_EXT_gl_colorspace_bt2020_linear.txt">EGL_EXT_gl_colorspace_bt2020_pq</a>
</li>
<li value=108> <a href="extensions/EXT/EGL_EXT_gl_colorspace_scrgb_linear.txt">EGL_EXT_gl_colorspace_scrgb_linear</a>
</li>
<li value=109> <a href="extensions/EXT/EGL_EXT_surface_SMPTE2086_metadata.txt">EGL_EXT_surface_SMPTE2086_metadata</a>
</li>
<li value=110> <a href="extensions/NV/EGL_NV_stream_fifo_next.txt">EGL_NV_stream_fifo_next</a>
</li>
<li value=111> <a href="extensions/NV/EGL_NV_stream_fifo_synchronous.txt">EGL_NV_stream_fifo_synchronous</a>
</li>
<li value=112> <a href="extensions/NV/EGL_NV_stream_reset.txt">EGL_NV_stream_reset</a>
</li>
<li value=113> <a href="extensions/NV/EGL_NV_stream_frame_limits.txt">EGL_NV_stream_frame_limits</a>
</li>
<li value=114> <a href="extensions/NV/EGL_NV_stream_remote.txt">EGL_NV_stream_remote</a>
     <br> <a href="extensions/NV/EGL_NV_stream_remote.txt">EGL_NV_stream_cross_object</a>
     <br> <a href="extensions/NV/EGL_NV_stream_remote.txt">EGL_NV_stream_cross_display</a>
     <br> <a href="extensions/NV/EGL_NV_stream_remote.txt">EGL_NV_stream_cross_process</a>
     <br> <a href="extensions/NV/EGL_NV_stream_remote.txt">EGL_NV_stream_cross_partition</a>
     <br> <a href="extensions/NV/EGL_NV_stream_remote.txt">EGL_NV_stream_cross_system</a>
</li>
<li value=115> <a href="extensions/NV/EGL_NV_stream_socket.txt">EGL_NV_stream_socket</a>
     <br> <a href="extensions/NV/EGL_NV_stream_socket.txt">EGL_NV_stream_socket_unix</a>
     <br> <a href="extensions/NV/EGL_NV_stream_socket.txt">EGL_NV_stream_socket_inet</a>
</li>
<li value=116> <a href="extensions/EXT/EGL_EXT_compositor.txt">EGL_EXT_compositor</a>
</li>
<li value=117> <a href="extensions/EXT/EGL_EXT_surface_CTA861_3_metadata.txt">EGL_EXT_surface_CTA861_3_metadata</a>
</li>
<li value=118> <a href="extensions/EXT/EGL_EXT_gl_colorspace_display_p3.txt">EGL_EXT_gl_colorspace_display_p3</a>
</li>
<li value=118> <a href="extensions/EXT/EGL_EXT_gl_colorspace_display_p3.txt">EGL_EXT_gl_colorspace_display_p3_linear</a>
</li>
<li value=119> <a href="extensions/EXT/EGL_EXT_gl_colorspace_scrgb.txt">EGL_EXT_gl_colorspace_scrgb (non-linear)</a>
</li>
<li value=120> <a href="extensions/EXT/EGL_EXT_image_implicit_sync_control.txt">EGL_EXT_image_implicit_sync_control</a>
</li>
<li value=121> <a href="extensions/EXT/EGL_EXT_bind_to_front.txt">EGL_EXT_bind_to_front</a>
</li>
<li value=122> <a href="extensions/ANDROID/EGL_ANDROID_get_frame_timestamps.txt">EGL_ANDROID_get_frame_timestamps</a>
</li>
<li value=123> <a href="extensions/ANDROID/EGL_ANDROID_get_native_client_buffer.txt">EGL_ANDROID_get_native_client_buffer</a>
</li>
<li value=124> <a href="extensions/NV/EGL_NV_context_priority_realtime.txt">EGL_NV_context_priority_realtime</a>
</li>
<li value=125> <a href="extensions/EXT/EGL_EXT_image_gl_colorspace.txt">EGL_EXT_image_gl_colorspace</a>
</li>
<li value=126> <a href="extensions/KHR/EGL_KHR_display_reference.txt">EGL_KHR_display_reference</a>
</li>
<li value=127> <a href="extensions/NV/EGL_NV_stream_flush.txt">EGL_NV_stream_flush</a>
</li>
<li value=128> <a href="extensions/EXT/EGL_EXT_sync_reuse.txt">EGL_EXT_sync_reuse</a>
</li>
<li value=129> <a href="extensions/EXT/EGL_EXT_client_sync.txt">EGL_EXT_client_sync</a>
</li>
<li value=130> <a href="extensions/EXT/EGL_EXT_gl_colorspace_display_p3_passthrough.txt">EGL_EXT_gl_colorspace_display_p3_passthrough</a>
</li>
<li value=131> <a href="extensions/MESA/EGL_MESA_query_driver.txt">EGL_MESA_query_driver</a>
</li>
<li value=132> <a href="extensions/ANDROID/EGL_ANDROID_GLES_layers.txt">EGL_ANDROID_GLES_layers</a>
</li>
<li value=133> <a href="extensions/NV/EGL_NV_n_buffer.txt">EGL_NV_n_buffer</a>
</li>
<li value=134> <a href="extensions/NV/EGL_NV_stream_origin.txt">EGL_NV_stream_origin</a>
</li>
<li value=135> <a href="extensions/NV/EGL_NV_stream_dma.txt">EGL_NV_stream_dma</a>
</li>
<li value=136> <a href="extensions/WL/EGL_WL_bind_wayland_display.txt">EGL_WL_bind_wayland_display</a>
</li>
<li value=137> <a href="extensions/WL/EGL_WL_create_wayland_buffer_from_image.txt">EGL_WL_create_wayland_buffer_from_image</a>
</li>
<li value=139> <a href="extensions/NV/EGL_NV_stream_consumer_eglimage.txt">EGL_NV_stream_consumer_eglimage</a>
</li>
<li value=140> <a href="extensions/EXT/EGL_EXT_device_query_name.txt">EGL_EXT_device_query_name</a>
</li>
<li value=141> <a href="extensions/EXT/EGL_EXT_platform_xcb.txt">EGL_EXT_platform_xcb</a>
</li>
<li value=142> <a href="extensions/ANGLE/EGL_ANGLE_sync_control_rate.txt">EGL_ANGLE_sync_control_rate</a>
</li>
<li value=143> <a href="extensions/EXT/EGL_EXT_device_persistent_id.txt">EGL_EXT_device_persistent_id</a>
</li>
<li value=144> <a href="extensions/EXT/EGL_EXT_device_drm_render_node.txt">EGL_EXT_device_drm_render_node</a>
</li>
<li value=145> <a href="extensions/EXT/EGL_EXT_config_select_group.txt">EGL_EXT_config_select_group</a>
</li>
<li value=146> <a href="extensions/EXT/EGL_EXT_present_opaque.txt">EGL_EXT_present_opaque</a>
</li>
<li value=147> <a href="extensions/EXT/EGL_EXT_surface_compression.txt">EGL_EXT_surface_compression</a>
</li>
</ol>

<h6> Providing Feedback on the Registry </h6>

<p> Khronos welcomes comments and bug reports. To provide feedback on the
    EGL registry itself (such as reporting missing content, bad links,
    etc.), file an issue in the <a
    href="https://github.com/KhronosGroup/EGL-Registry/issues">
    EGL-Registry </a> Github project. </p>

<p> For the EGL API, extensions, and headers, file a bug on the <a
    href="http://www.khronos.org/bugzilla/"> Khronos Bugzilla </a>. Make
    sure to fill in the &quot;Product&quot; field in the bug entry form as
    &quot;EGL&quot;, and pick appropriate values for the Component and other
    fields. </p>

<?php include_once("../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
