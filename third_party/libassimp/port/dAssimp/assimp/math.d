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
 * Mathematical structures in which the imported data is stored.
 */
module assimp.math;

extern( C ) {
   /**
    * Represents a two-dimensional vector.
    */
   struct aiVector2D {
   align ( 1 ):
      float x, y;
   }

   /**
    * Represents a three-dimensional vector.
    */
   struct aiVector3D {
   align ( 1 ):
      float x, y, z;
   }

   /**
    * Represents a quaternion.
    */
   struct aiQuaternion {
      float w, x, y, z;
   }

   /**
    * Represents a row-major 3x3 matrix
    *
    * There is much confusion about matrix layouts (column vs. row order). This
    * is <em>always</em> a row-major matrix, even when using the
    * <code>ConvertToLeftHanded</code> post processing step.
    */
   struct aiMatrix3x3 {
      float a1, a2, a3;
      float b1, b2, b3;
      float c1, c2, c3;
   }

   /**
    * Represents a row-major 3x3 matrix
    *
    * There is much confusion about matrix layouts (column vs. row order). This
    * is <em>always</em> a row-major matrix, even when using the
    * <code>ConvertToLeftHanded</code> post processing step.
    */
   struct aiMatrix4x4 {
   align ( 1 ):
      float a1, a2, a3, a4;
      float b1, b2, b3, b4;
      float c1, c2, c3, c4;
      float d1, d2, d3, d4;
   }

   /**
    * Represents a plane in a three-dimensional, euclidean space
    */
   struct aiPlane {
   align ( 1 ):
      /**
       * Coefficients of the plane equation (<code>ax + by + cz = d</code>).
       */
      float a;
      float b; /// ditto
      float c; /// ditto
      float d; /// ditto
   }

   /**
    * Represents a ray.
    */
   struct aiRay {
   align ( 1 ):
      /**
       * Origin of the ray.
       */
      aiVector3D pos;

      /**
       * Direction of the ray.
       */
      aiVector3D dir;
   }

   /**
    * Represents a color in RGB space.
    */
   struct aiColor3D {
   align ( 1 ):
      /**
       * Red, green and blue values.
       */
      float r;
      float g; /// ditto
      float b; /// ditto
   }

   /**
    * Represents a color in RGB space including an alpha component.
    */
   struct aiColor4D {
   align ( 1 ):
      /**
       * Red, green, blue and alpha values.
       */
      float r;
      float g; /// ditto
      float b; /// ditto
      float a; /// ditto
   }
}
