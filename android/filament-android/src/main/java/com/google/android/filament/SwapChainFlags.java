/*
 * Copyright (C) 2024 The Android Open Source Project
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

package com.google.android.filament;

// Note: SwapChainFlags is kept separate from SwapChain so that UiHelper does not need to depend
// on SwapChain. This allows clients to use UiHelper without requiring all of Filament's Java
// classes.

/**
 * Flags that a <code>SwapChain</code> can be created with to control behavior.
 *
 * @see Engine#createSwapChain
 * @see Engine#createSwapChainFromNativeSurface
 */
public final class SwapChainFlags {

    public static final long CONFIG_DEFAULT = 0x0;

    /**
     * This flag indicates that the <code>SwapChain</code> must be allocated with an
     * alpha-channel.
     */
    public static final long CONFIG_TRANSPARENT = 0x1;

    /**
     * This flag indicates that the <code>SwapChain</code> may be used as a source surface
     * for reading back render results.  This config must be set when creating
     * any <code>SwapChain</code>  that will be used as the source for a blit operation.
     *
     * @see Renderer#copyFrame
     */
    public static final long CONFIG_READABLE = 0x2;

    /**
     * Indicates that the native X11 window is an XCB window rather than an XLIB window.
     * This is ignored on non-Linux platforms and in builds that support only one X11 API.
     */
    public static final long CONFIG_ENABLE_XCB = 0x4;

    /**
     * Indicates that the SwapChain must automatically perform linear to sRGB encoding.
     *
     * This flag is ignored if isSRGBSwapChainSupported() is false.
     *
     * When using this flag, post-processing should be disabled.
     *
     * @see SwapChain#isSRGBSwapChainSupported
     * @see View#setPostProcessingEnabled
     */
    public static final long CONFIG_SRGB_COLORSPACE = 0x10;

    /**
     * Indicates that this SwapChain should allocate a stencil buffer in addition to a depth buffer.
     *
     * This flag is necessary when using View::setStencilBufferEnabled and rendering directly into
     * the SwapChain (when post-processing is disabled).
     *
     * The specific format of the stencil buffer depends on platform support. The following pixel
     * formats are tried, in order of preference:
     *
     * Depth only (without CONFIG_HAS_STENCIL_BUFFER):
     * - DEPTH32F
     * - DEPTH24
     *
     * Depth + stencil (with CONFIG_HAS_STENCIL_BUFFER):
     * - DEPTH32F_STENCIL8
     * - DEPTH24F_STENCIL8
     *
     * Note that enabling the stencil buffer may hinder depth precision and should only be used if
     * necessary.
     *
     * @see View#setStencilBufferEnabled
     * @see View#setPostProcessingEnabled
     */
    public static final long CONFIG_HAS_STENCIL_BUFFER = 0x20;

    /**
     * The SwapChain contains protected content. Only supported when isProtectedContentSupported()
     * is true.
     */
    public static final long CONFIG_PROTECTED_CONTENT   = 0x40;
}

