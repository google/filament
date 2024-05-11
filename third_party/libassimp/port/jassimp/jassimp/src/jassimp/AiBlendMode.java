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
 * Defines alpha-blend flags.<p>
 *
 * If you're familiar with OpenGL or D3D, these flags aren't new to you.
 * They define *how* the final color value of a pixel is computed, basing
 * on the previous color at that pixel and the new color value from the
 * material. The blend formula is:
 * <br><code>
 *   SourceColor * SourceBlend + DestColor * DestBlend
 * </code><br>
 * where <code>DestColor</code> is the previous color in the framebuffer at 
 * this position and <code>SourceColor</code> is the material color before the
 * transparency calculation.
 */
public enum AiBlendMode {
   /** 
    * Default blending.<p>
    * 
    * Formula:
    * <code>
    * SourceColor*SourceAlpha + DestColor*(1-SourceAlpha)
    * </code>
    */
   DEFAULT(0x0),

   
   /** 
    * Additive blending.<p>
    *
    * Formula:
    * <code>
    * SourceColor*1 + DestColor*1
    * </code>
    */
   ADDITIVE(0x1);
   
   
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
   static AiBlendMode fromRawValue(int rawValue) {
       for (AiBlendMode type : AiBlendMode.values()) {
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
   private AiBlendMode(int rawValue) {
       m_rawValue = rawValue;
   }


   /**
    * The mapped c/c++ integer enum value.
    */
   private final int m_rawValue;
}