/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_BACKEND_TEST_WORKAROUNDS_H
#define TNT_BACKEND_TEST_WORKAROUNDS_H

#if defined(FILAMENT_SUPPORTS_WEBGPU)
// consider, if needed, adding (as Vulkan also has transient attachments): || defined(FILAMENT_SUPPORTS_VULKAN)
// TODO: support TextureUsage::READ_PIXELS (or something like that)
//        and search-and-replace TEXTURE_USAGE_READ_PIXELS with
//       "| TextureUsage::READ_PIXELS" when it is available.
//       Until then, this serves as a workaround for WebGPU testing, such that
//       it can easily be swapped out for the "right" thing when that is available.
//       The problem this is trying to address is that the WebGPU backend will
//       assign a texture with wgpu::TextureUsage::TransientAttachment if the Filament
//       usage flags don't indicate the pixels are needed CPU-side. If transient attachment is
//       assigned to the texture, the texture will not be available later for readPixels, which
//       is often needed for the unit tests. If we suggest that
//       the texture needs to support being a blit source, then TransientAttachment will
//       not get assigned. However, this is not quite right in the test, because a test
//       may not really want to do a blit with the texture, just read the pixels from it later for
//       image comparison (renderdiff).
//       Ideally, the test could express this with a TextureUsage::READ_PIXELS flag, but that
//       does not yet exist. So, a hack would be to explicitly set the BLIT_SRC flag instead in
//       its place.
#define TEXTURE_USAGE_READ_PIXELS | TextureUsage::BLIT_SRC
#else
#define TEXTURE_USAGE_READ_PIXELS
#endif

#endif // TNT_BACKEND_TEST_WORKAROUNDS_H
