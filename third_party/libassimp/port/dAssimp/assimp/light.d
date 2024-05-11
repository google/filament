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
 * Contains the data structures which are used to store the imported information
 * about the light sources in the scene.
 */
module assimp.light;

import assimp.math;
import assimp.types;

extern ( C ) {
   /**
    * Enumerates all supported types of light sources.
    */
   enum aiLightSourceType : uint {
      UNDEFINED = 0x0,

      /**
       * A directional light source has a well-defined direction but is
       * infinitely far away. That's quite a good approximation for sun light.
       */
      DIRECTIONAL = 0x1,

      /**
       * A point light source has a well-defined position in space but no
       * direction – it emits light in all directions. A normal bulb is a point
       * light.
       */
      POINT = 0x2,

      /**
       * A spot light source emits light in a specific angle. It has a position
       * and a direction it is pointing to. A good example for a spot light is
       * a light spot in sport arenas.
       */
      SPOT = 0x3
   }

   /**
    * Helper structure to describe a light source.
    *
    * Assimp supports multiple sorts of light sources, including directional,
    * point and spot lights. All of them are defined with just a single
    * structure and distinguished by their parameters.
    *
    * Note: Some file formats (such as 3DS, ASE) export a "target point" – the
    * point a spot light is looking at (it can even be animated). Assimp
    * writes the target point as a subnode of a spotlights's main node, called
    * <code>[spotName].Target</code>. However, this is just additional
    * information then, the transformation tracks of the main node make the
    * spot light already point in the right direction.
   */
   struct aiLight {
      /**
       * The name of the light source.
       *
       * There must be a node in the scenegraph with the same name. This node
       * specifies the position of the light in the scenehierarchy and can be
       * animated.
       */
      aiString mName;

      /**
       * The type of the light source.
       *
       * <code>aiLightSource.UNDEFINED</code> is not a valid value for this
       * member.
       */
      aiLightSourceType mType;

      /**
       * Position of the light source in space. Relative to the transformation
       * of the node corresponding to the light.
       *
       * The position is undefined for directional lights.
       */
      aiVector3D mPosition;

      /**
       * Direction of the light source in space. Relative to the transformation
       * of the node corresponding to the light.
       *
       * The direction is undefined for point lights. The vector may be
       * normalized, but it needn't.
       */
      aiVector3D mDirection;

      /**
       * Constant light attenuation factor.
       *
       * The intensity of the light source at a given distance
       * <code>d</code> from the light's position is
       * <code>1/( att0 + att1 * d + att2 * d * d )</code>. This member
       * corresponds to the <code>att0</code> variable in the equation.
       *
       * Naturally undefined for directional lights.
       */
      float mAttenuationConstant;

      /**
       * Linear light attenuation factor.
       *
       * The intensity of the light source at a given distance
       * <code>d</code> from the light's position is
       * <code>1/( att0 + att1 * d + att2 * d * d )</code>. This member
       * corresponds to the <code>att1</code> variable in the equation.
       *
       * Naturally undefined for directional lights.
       */
      float mAttenuationLinear;

      /**
       * Quadratic light attenuation factor.
       *
       * The intensity of the light source at a given distance
       * <code>d</code> from the light's position is
       * <code>1/( att0 + att1 * d + att2 * d * d )</code>. This member
       * corresponds to the <code>att2</code> variable in the equation.
       *
       * Naturally undefined for directional lights.
       */
      float mAttenuationQuadratic;

      /**
       * Diffuse color of the light source
       *
       * The diffuse light color is multiplied with the diffuse material color
       * to obtain the final color that contributes to the diffuse shading term.
       */
      aiColor3D mColorDiffuse;

      /**
       * Specular color of the light source
       *
       * The specular light color is multiplied with the specular material
       * color to obtain the final color that contributes to the specular
       * shading term.
       */
      aiColor3D mColorSpecular;

      /**
       * Ambient color of the light source
       *
       * The ambient light color is multiplied with the ambient material color
       * to obtain the final color that contributes to the ambient shading term.
       *
       * Most renderers will ignore this value it, is just a remaining of the
       * fixed-function pipeline that is still supported by quite many file
       * formats.
       */
      aiColor3D mColorAmbient;

      /**
       * Inner angle of a spot light's light cone.
       *
       * The spot light has maximum influence on objects inside this angle. The
       * angle is given in radians. It is 2PI for point lights and undefined
       * for directional lights.
       */
      float mAngleInnerCone;

      /**
       * Outer angle of a spot light's light cone.
       *
       * The spot light does not affect objects outside this angle. The angle
       * is given in radians. It is 2PI for point lights and undefined for
       * directional lights. The outer angle must be greater than or equal to
       * the inner angle.
       *
       * It is assumed that the application uses a smooth interpolation between
       * the inner and the outer cone of the spot light.
       */
      float mAngleOuterCone;
   }
}
