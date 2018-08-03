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
 * Contains the material system which stores the imported material information.
 */
module assimp.material;

import assimp.math;
import assimp.types;

extern ( C ) {
   /**
    * Default material names for meshes without UV coordinates.
    */
   const char* AI_DEFAULT_MATERIAL_NAME = "aiDefaultMat";

   /**
    * Default material names for meshes with UV coordinates.
    */
   const char* AI_DEFAULT_TEXTURED_MATERIAL_NAME = "TexturedDefaultMaterial";

   /**
    * Defines how the Nth texture of a specific type is combined with the
    * result of all previous layers.
    *
    * Example (left: key, right: value):
    * <pre> DiffColor0     - gray
    * DiffTextureOp0 - aiTextureOpMultiply
    * DiffTexture0   - tex1.png
    * DiffTextureOp0 - aiTextureOpAdd
    * DiffTexture1   - tex2.png</pre>
    * Written as equation, the final diffuse term for a specific pixel would be:
    * <pre>diffFinal = DiffColor0 * sampleTex( DiffTexture0, UV0 ) +
    *     sampleTex( DiffTexture1, UV0 ) * diffContrib;</pre>
    * where <code>diffContrib</code> is the intensity of the incoming light for
    * that pixel.
    */
   enum aiTextureOp : uint {
      /**
       * <code>T = T1 * T2</code>
       */
      Multiply = 0x0,

      /**
       * <code>T = T1 + T2</code>
       */
      Add = 0x1,

      /**
       * <code>T = T1 - T2</code>
       */
      Subtract = 0x2,

      /**
       * <code>T = T1 / T2</code>
       */
      Divide = 0x3,

      /**
       * <code>T = ( T1 + T2 ) - ( T1 * T2 )</code>
       */
      SmoothAdd = 0x4,

      /**
       * <code>T = T1 + ( T2 - 0.5 )</code>
       */
      SignedAdd = 0x5
   }

   /**
    * Defines how UV coordinates outside the <code>[0..1]</code> range are
    * handled.
    *
    * Commonly referred to as 'wrapping mode'.
    */
   enum aiTextureMapMode : uint {
      /**
       * A texture coordinate <code>u | v</code> is translated to
       * <code>(u%1) | (v%1)</code>.
       */
      Wrap = 0x0,

      /**
       * Texture coordinates are clamped to the nearest valid value.
       */
      Clamp = 0x1,

      /**
       * If the texture coordinates for a pixel are outside
       * <code>[0..1]</code>, the texture is not applied to that pixel.
       */
      Decal = 0x3,

      /**
       * A texture coordinate <code>u | v</code> becomes
       * <code>(u%1) | (v%1)</code> if <code>(u-(u%1))%2</code> is
       * zero and <code>(1-(u%1)) | (1-(v%1))</code> otherwise.
       */
      Mirror = 0x2
   }

   /**
    * Defines how the mapping coords for a texture are generated.
    *
    * Real-time applications typically require full UV coordinates, so the use of
    * the <code>aiProcess.GenUVCoords</code> step is highly recommended. It
    * generates proper UV channels for non-UV mapped objects, as long as an
    * accurate description how the mapping should look like (e.g spherical) is
    * given. See the <code>AI_MATKEY_MAPPING</code> property for more details.
    */
   enum aiTextureMapping : uint {
      /**
       * The mapping coordinates are taken from an UV channel.
       *
       * The <code>AI_MATKEY_UVSRC</code> key specifies from which (remember,
       * meshes can have more than one UV channel).
       */
      UV = 0x0,

      /**
       * Spherical mapping.
       */
      SPHERE = 0x1,

      /**
       * Cylindrical mapping.
       */
      CYLINDER = 0x2,

      /**
       * Cubic mapping.
       */
      BOX = 0x3,

      /**
       * Planar mapping.
       */
      PLANE = 0x4,

      /**
       * Undefined mapping.
       */
      OTHER = 0x5
   }

   /**
    * Defines the purpose of a texture
    *
    * This is a very difficult topic. Different 3D packages support different
    * kinds of textures. For very common texture types, such as bumpmaps, the
    * rendering results depend on implementation details in the rendering
    * pipelines of these applications. Assimp loads all texture references from
    * the model file and tries to determine which of the predefined texture
    * types below is the best choice to match the original use of the texture
    * as closely as possible.
    *
    * In content pipelines you'll usually define how textures have to be
    * handled, and the artists working on models have to conform to this
    * specification, regardless which 3D tool they're using.
    */
   enum aiTextureType : uint {
      /**
       * No texture, but the value to be used for
       * <code>aiMaterialProperty.mSemantic</code> for all material properties
       * <em>not</em> related to textures.
       */
      NONE = 0x0,

      /**
       * The texture is combined with the result of the diffuse lighting
       * equation.
       */
      DIFFUSE = 0x1,

      /**
       * The texture is combined with the result of the specular lighting
       * equation.
       */
      SPECULAR = 0x2,

      /**
       * The texture is combined with the result of the ambient lighting
       * equation.
       */
      AMBIENT = 0x3,

      /**
       * The texture is added to the result of the lighting calculation. It
       * isn't influenced by incoming light.
       */
      EMISSIVE = 0x4,

      /**
       * The texture is a height map.
       *
       * By convention, higher grey-scale values stand for higher elevations
       * from the base height.
       */
      HEIGHT = 0x5,

      /**
       * The texture is a (tangent space) normal-map.
       *
       * Again, there are several conventions for tangent-space normal maps.
       * Assimp does (intentionally) not differenciate here.
       */
      NORMALS = 0x6,

      /**
       * The texture defines the glossiness of the material.
       *
       * The glossiness is in fact the exponent of the specular (phong)
       * lighting equation. Usually there is a conversion function defined to
       * map the linear color values in the texture to a suitable exponent.
       */
      SHININESS = 0x7,

      /**
       * The texture defines per-pixel opacity.
       *
       * Usually white means opaque and black means transparent.
       */
      OPACITY = 0x8,

      /**
       * Displacement texture.
       *
       * The exact purpose and format is application-dependent. Higher color
       * values stand for higher vertex displacements.
       */
      DISPLACEMENT = 0x9,

      /**
       * Lightmap or ambient occlusion texture.
       *
       * Both lightmaps and dedicated ambient occlusion maps are covered by
       * this material property. The texture contains a scaling value for the
       * final color value of a pixel. Its intensity is not affected by
       * incoming light.
       */
      LIGHTMAP = 0xA,

      /**
       * Reflection texture.
       *
       * Contains the color of a perfect mirror reflection. Rarely used, almost
       * never for real-time applications.
       */
      REFLECTION = 0xB,

      /**
       * Unknown texture.
       *
       * A texture reference that does not match any of the definitions above is
       * considered to be 'unknown'. It is still imported, but is excluded from
       * any further postprocessing.
       */
      UNKNOWN = 0xC
   }

   /**
    * Defines all shading models supported by the library
    *
    * The list of shading modes has been taken from Blender. See Blender
    * documentation for more information. The API does not distinguish between
    * "specular" and "diffuse" shaders (thus the specular term for diffuse
    * shading models like Oren-Nayar remains undefined).
    *
    * Again, this value is just a hint. Assimp tries to select the shader whose
    * most common implementation matches the original rendering results of the
    * 3D modeller which wrote a particular model as closely as possible.
    */
   enum aiShadingMode : uint {
      /**
       * Flat shading.
       *
       * Shading is done on per-face base diffuse only. Also known as
       * »faceted shading«.
       */
      Flat = 0x1,

      /**
       * Simple Gouraud shading.
       */
      Gouraud =   0x2,

      /**
       * Phong-Shading.
       */
      Phong = 0x3,

      /**
       * Phong-Blinn-Shading.
       */
      Blinn = 0x4,

      /**
       * Per-pixel toon shading.
       *
       * Often referred to as »comic shading«.
       */
      Toon = 0x5,

      /**
       * Per-pixel Oren-Nayar shading.
       *
       * Extension to standard Lambertian shading, taking the roughness of the
       * material into account.
       */
      OrenNayar = 0x6,

      /**
       * Per-pixel Minnaert shading.
       *
       * Extension to standard Lambertian shading, taking the "darkness" of the
       * material into account.
       */
      Minnaert = 0x7,

      /**
       * Per-pixel Cook-Torrance shading.
       *
       * Special shader for metallic surfaces.
       */
      CookTorrance = 0x8,

      /**
       * No shading at all.
       *
       * Constant light influence of 1.
       */
      NoShading = 0x9,

      /**
       * Fresnel shading.
       */
      Fresnel = 0xa
   }

   /**
    * Defines some mixed flags for a particular texture.
    *
    * Usually you'll instruct your cg artists how textures have to look like
    * and how they will be processed in your application. However, if you use
    * Assimp for completely generic loading purposes you might also need to
    * process these flags in order to display as many 'unknown' 3D models as
    * possible correctly.
    *
    * This corresponds to the <code>AI_MATKEY_TEXFLAGS</code> property.
    */
   enum aiTextureFlags : uint {
      /**
       * The texture's color values have to be inverted (i.e. <code>1-n</code>
       * component-wise).
       */
      Invert = 0x1,

      /**
       * Explicit request to the application to process the alpha channel of the
       * texture.
       *
       * Mutually exclusive with <code>IgnoreAlpha</code>. These flags are
       * set if the library can say for sure that the alpha channel is used/is
       * not used. If the model format does not define this, it is left to the
       * application to decide whether the texture alpha channel – if any – is
       * evaluated or not.
       */
      UseAlpha = 0x2,

      /**
       * Explicit request to the application to ignore the alpha channel of the
       * texture.
       *
       * Mutually exclusive with <code>UseAlpha</code>.
       */
      IgnoreAlpha = 0x4
   }


   /**
    * Defines alpha-blend flags.
    *
    * If you're familiar with OpenGL or D3D, these flags aren't new to you.
    * They define how the final color value of a pixel is computed, based on
    * the previous color at that pixel and the new color value from the
    * material.
    *
    * The basic blending formula is
    * <code>SourceColor * SourceBlend + DestColor * DestBlend</code>,
    * where <code>DestColor</code> is the previous color in the framebuffer at
    * this position and <code>SourceColor</code> is the material color before
    * the transparency calculation.
    *
    * This corresponds to the <code>AI_MATKEY_BLEND_FUNC</code> property.
    */
   enum aiBlendMode :uint {
      /**
       * Formula:
       * <code>SourceColor * SourceAlpha + DestColor * (1 - SourceAlpha)</code>
       */
      Default = 0x0,

      /**
       * Additive blending.
       *
       * Formula: <code>SourceColor*1 + DestColor*1</code>
       */
      Additive = 0x1
   }

   /**
    * Defines how an UV channel is transformed.
    *
    * This is just a helper structure for the <code>AI_MATKEY_UVTRANSFORM</code>
    * key. See its documentation for more details.
    */
   struct aiUVTransform {
   align ( 1 ) :
      /**
       * Translation on the u and v axes.
       *
       * The default value is (0|0).
       */
      aiVector2D mTranslation;

      /**
       * Scaling on the u and v axes.
       *
       * The default value is (1|1).
       */
      aiVector2D mScaling;

      /**
       * Rotation - in counter-clockwise direction.
       *
       * The rotation angle is specified in radians. The rotation center is
       * 0.5|0.5. The default value is 0.
       */
      float mRotation;
   }

   /**
    * A very primitive RTTI system to store the data type of a material
    * property.
    */
   enum aiPropertyTypeInfo : uint {
      /**
       * Array of single-precision (32 bit) floats.
       *
       * It is possible to use <code>aiGetMaterialInteger[Array]()</code> to
       * query properties stored in floating-point format. The material system
       * performs the type conversion automatically.
       */
      Float = 0x1,

      /**
       * aiString property.
       *
       * Arrays of strings aren't possible, <code>aiGetMaterialString()</code>
       * must be used to query a string property.
       */
      String = 0x3,

      /**
       * Array of (32 bit) integers.
       *
       * It is possible to use <code>aiGetMaterialFloat[Array]()</code> to
       * query properties stored in integer format. The material system
       * performs the type conversion automatically.
       */
      Integer = 0x4,

      /**
       * Simple binary buffer, content undefined. Not convertible to anything.
       */
      Buffer = 0x5
   }

   /**
    * Data structure for a single material property.
    *
    * As an user, you'll probably never need to deal with this data structure.
    * Just use the provided <code>aiGetMaterialXXX()</code> functions to query
    * material properties easily. Processing them manually is faster, but it is
    * not the recommended way. It isn't worth the effort.
    *
    * Material property names follow a simple scheme:
    *
    * <code>$[name]</code>: A public property, there must be a corresponding
    * AI_MATKEY_XXX constant.
    *
    * <code>?[name]</code>: Also public, but ignored by the
    * <code>aiProcess.RemoveRedundantMaterials</code> post-processing step.
    *
    * <code>~[name]</code>: A temporary property for internal use.
    */
   struct aiMaterialProperty {
      /**
       * Specifies the name of the property (key).
       *
       * Keys are generally case insensitive.
       */
      aiString mKey;

      /**
       * For texture properties, this specifies the exact usage semantic.
       *
       * For non-texture properties, this member is always 0 (or rather
       * <code>aiTextureType.NONE</code>).
       */
      uint mSemantic;

      /**
       * For texture properties, this specifies the index of the texture.
       *
       * For non-texture properties, this member is always 0.
       */
      uint mIndex;

      /**
       * Size of the buffer <code>mData</code> is pointing to (in bytes).
       *
       * This value may not be 0.
       */
      uint mDataLength;

      /**
       * Type information for the property.
       *
       * Defines the data layout inside the data buffer. This is used by the
       * library internally to perform debug checks and to utilize proper type
       * conversions.
       */
      aiPropertyTypeInfo mType;

      /**
       * Binary buffer to hold the property's value.
       *
       * The size of the buffer is always <code>mDataLength</code>.
       */
      char* mData;
   }

   /**
    * Data structure for a material
    *
    * Material data is stored using a key-value structure. A single key-value
    * pair is called a <em>material property</em>. The properties can be
    * queried using the <code>aiMaterialGetXXX</code> family of functions. The
    * library defines a set of standard keys (AI_MATKEY_XXX).
    */
   struct aiMaterial {
      /**
       * List of all material properties loaded.
       */
      aiMaterialProperty** mProperties;

      /**
       * Number of properties loaded.
       */
      uint mNumProperties;
      uint mNumAllocated; /// ditto
   }

   /**
    * Standard material property keys. Always pass 0 for texture type and index
    * when querying these keys.
    */
   const char* AI_MATKEY_NAME = "?mat.name";
   const char* AI_MATKEY_TWOSIDED = "$mat.twosided"; /// ditto
   const char* AI_MATKEY_SHADING_MODEL = "$mat.shadingm"; /// ditto
   const char* AI_MATKEY_ENABLE_WIREFRAME = "$mat.wireframe"; /// ditto
   const char* AI_MATKEY_BLEND_FUNC = "$mat.blend"; /// ditto
   const char* AI_MATKEY_OPACITY = "$mat.opacity"; /// ditto
   const char* AI_MATKEY_BUMPSCALING = "$mat.bumpscaling"; /// ditto
   const char* AI_MATKEY_SHININESS = "$mat.shininess"; /// ditto
   const char* AI_MATKEY_REFLECTIVITY = "$mat.reflectivity"; /// ditto
   const char* AI_MATKEY_SHININESS_STRENGTH = "$mat.shinpercent"; /// ditto
   const char* AI_MATKEY_REFRACTI = "$mat.refracti"; /// ditto
   const char* AI_MATKEY_COLOR_DIFFUSE = "$clr.diffuse"; /// ditto
   const char* AI_MATKEY_COLOR_AMBIENT = "$clr.ambient"; /// ditto
   const char* AI_MATKEY_COLOR_SPECULAR = "$clr.specular"; /// ditto
   const char* AI_MATKEY_COLOR_EMISSIVE = "$clr.emissive"; /// ditto
   const char* AI_MATKEY_COLOR_TRANSPARENT = "$clr.transparent"; /// ditto
   const char* AI_MATKEY_COLOR_REFLECTIVE = "$clr.reflective"; /// ditto
   const char* AI_MATKEY_GLOBAL_BACKGROUND_IMAGE = "?bg.global"; /// ditto

   /**
    * Texture material property keys. Do not forget to specify texture type and
    * index for these keys.
    */
   const char* AI_MATKEY_TEXTURE = "$tex.file";
   const char* AI_MATKEY_UVWSRC = "$tex.uvwsrc"; /// ditto
   const char* AI_MATKEY_TEXOP = "$tex.op"; /// ditto
   const char* AI_MATKEY_MAPPING = "$tex.mapping"; /// ditto
   const char* AI_MATKEY_TEXBLEND = "$tex.blend"; /// ditto
   const char* AI_MATKEY_MAPPINGMODE_U = "$tex.mapmodeu"; /// ditto
   const char* AI_MATKEY_MAPPINGMODE_V = "$tex.mapmodev"; /// ditto
   const char* AI_MATKEY_TEXMAP_AXIS = "$tex.mapaxis"; /// ditto
   const char* AI_MATKEY_UVTRANSFORM = "$tex.uvtrafo"; /// ditto
   const char* AI_MATKEY_TEXFLAGS = "$tex.flags"; /// ditto
}
