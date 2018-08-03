/*
---------------------------------------------------------------------------
Open Asset Import Library - Java Binding (jassimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms, 
with or without modification, are permitted provided that the following 
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/
package jassimp;


/**
 * Defines all shading modes supported by the library.<p>
 *
 * The list of shading modes has been taken from Blender.
 * See Blender documentation for more information. The API does
 * not distinguish between "specular" and "diffuse" shaders (thus the
 * specular term for diffuse shading models like Oren-Nayar remains
 * undefined).<p>
 * Again, this value is just a hint. Assimp tries to select the shader whose
 * most common implementation matches the original rendering results of the
 * 3D modeller which wrote a particular model as closely as possible.
 */
public enum AiShadingMode {
    /** 
     * Flat shading.<p>
     * 
     * Shading is done on per-face base, diffuse only. Also known as 
     * 'faceted shading'.
     */
    FLAT(0x1),


    /** 
     * Simple Gouraud shading. 
     */
    GOURAUD(0x2),


    /** 
     * Phong-Shading.
     */
    PHONG(0x3),


    /** 
     * Phong-Blinn-Shading.
     */
    BLINN(0x4),


    /** 
     * Toon-Shading per pixel.<p>
     *
     * Also known as 'comic' shader.
     */
    TOON(0x5),


    /** 
     * OrenNayar-Shading per pixel.<p>
     *
     * Extension to standard Lambertian shading, taking the roughness of the 
     * material into account
     */
    OREN_NAYAR(0x6),


    /** 
     * Minnaert-Shading per pixel.<p>
     *
     * Extension to standard Lambertian shading, taking the "darkness" of the 
     * material into account
     */
    MINNAERT(0x7),


    /** 
     * CookTorrance-Shading per pixel.<p>
     *
     * Special shader for metallic surfaces.
     */
    COOK_TORRANCE(0x8),


    /** 
     * No shading at all.<p>
     * 
     * Constant light influence of 1.0.
     */
    NO_SHADING(0x9),


    /** 
     * Fresnel shading.
     */
    FRESNEL(0xa);


    /**
     * Utility method for converting from c/c++ based integer enums to java 
     * enums.<p>
     * 
     * This method is intended to be used from JNI and my change based on
     * implementation needs.
     * 
     * @param rawValue an integer based enum value (as defined by assimp) 
     * @return the enum value corresponding to rawValue
     */
    static AiShadingMode fromRawValue(int rawValue) {
        for (AiShadingMode type : AiShadingMode.values()) {
            if (type.m_rawValue == rawValue) {
                return type;
            }
        }

        throw new IllegalArgumentException("unexptected raw value: " + 
                rawValue);
    }


    /**
     * Constructor.
     * 
     * @param rawValue maps java enum to c/c++ integer enum values
     */
    private AiShadingMode(int rawValue) {
        m_rawValue = rawValue;
    }


    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;
}
