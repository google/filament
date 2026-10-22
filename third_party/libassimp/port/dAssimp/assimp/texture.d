/*
---------------------------------------------------------------------------
Open Asset Import Library (ASSIMP)
---------------------------------------------------------------------------

Copyright (c) 2006-2009, ASSIMP Development Team

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

 * Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

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

/**
 * Contains helper structures to handle textures in Assimp.
 *
 * Used for file formats which embed their textures into the model file.
 * Supported are both normal textures, which are stored as uncompressed pixels,
 * and "compressed" textures, which are stored in a file format such as PNG or
 * TGA.
 */
module assimp.texture;

extern ( C ) {
   /**
    * Helper structure to represent a texel in a ARGB8888 format.
    *
    * Used by aiTexture.
    */
   struct aiTexel {
   align ( 1 ):
      ubyte b, g, r, a;
   }

   /**
    * Helper structure to describe an embedded texture.
    *
    * Usually textures are contained in external files but some file formats
    * embed them directly in the model file. There are two types of
    * embedded textures:
    *
    * <em>1. Uncompressed textures</em>: The color data is given in an
    * uncompressed format.
    *
    * <em>2. Compressed textures</em> stored in a file format like PNG or JPEG.
    * The raw file bytes are given so the application must utilize an image
    * decoder (e.g. DevIL) to get access to the actual color data.
    */
   struct aiTexture {
      /**
       * Width of the texture, in pixels.
       *
       * If <code>mHeight</code> is zero the texture is compressed in a format
       * like JPEG. In this case, this value specifies the size of the memory
       * area <code>pcData</code> is pointing to, in bytes.
       */
      uint mWidth;

      /**
       * Height of the texture, in pixels.
       *
       * If this value is zero, <code>pcData</code> points to an compressed
       * texture in any format (e.g. JPEG).
       */
      uint mHeight;

      /**
       * A hint from the loader to make it easier for applications to determine
       * the type of embedded compressed textures.
       *
       * If <code>mHeight</code> is not 0, this member is undefined. Otherwise
       * it is set set to '\0\0\0\0' if the loader has no additional
       * information about the texture file format used, or the file extension
       * of the format without a trailing dot. If there are multiple file
       * extensions for a format, the shortest extension is chosen (JPEG maps
       * to 'jpg', not to 'jpeg'). E.g. 'dds\0', 'pcx\0', 'jpg\0'. All
       * characters are lower-case. The fourth byte will always be '\0'.
       */
      char achFormatHint[4];

      /**
       * Data of the texture.
       *
       * Points to an array of <code>mWidth * mHeight</code>
       * <code>aiTexel</code>s. The format of the texture data is always
       * ARGB8888 to make the implementation for user of the library as easy as
       * possible.
       *
       * If <code>mHeight</code> is 0, this is a pointer to a memory buffer of
       * size <code>mWidth</code> containing the compressed texture data.
       */
      aiTexel* pcData;
   }
}
