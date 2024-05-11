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
 * Contains the data structures which store the hierarchy fo the imported data.
 */
module assimp.scene;

import assimp.animation;
import assimp.camera;
import assimp.light;
import assimp.math;
import assimp.mesh;
import assimp.material;
import assimp.texture;
import assimp.types;

extern ( C ) {
   /**
    * A node in the imported hierarchy.
    *
    * Each node has name, a parent node (except for the root node), a
    * transformation relative to its parent and possibly several child nodes.
    * Simple file formats don't support hierarchical structures, for these
    * formats the imported scene does consist of only a single root node with
    * no childs.
    */
   struct aiNode {
      /**
       * The name of the node.
       *
       * The name might be empty (length of zero) but all nodes which need to
       * be accessed afterwards by bones or animations are usually named.
       * Multiple nodes may have the same name, but nodes which are accessed
       * by bones (see <code>aiBone</code> and <code>aiMesh.mBones</code>)
       * <em>must</em> be unique.
       *
       * Cameras and lights are assigned to a specific node name – if there are
       * multiple nodes with this name, they are assigned to each of them.
       *
       * There are no limitations regarding the characters contained in this
       * string. You should be able to handle stuff like whitespace, tabs,
       * linefeeds, quotation marks, ampersands, …
       */
      aiString mName;

      /**
       * The transformation relative to the node's parent.
       */
      aiMatrix4x4 mTransformation;

      /**
       * Parent node.
       *
       * null if this node is the root node.
       */
      aiNode* mParent;

      /**
       * The number of child nodes of this node.
       */
      uint mNumChildren;

      /**
       * The child nodes of this node.
       *
       * null if <code>mNumChildren</code> is 0.
       */
      aiNode** mChildren;

      /**
       * The number of meshes of this node.
       */
      int mNumMeshes;

      /**
       * The meshes of this node.
       *
       * Each entry is an index for <code>aiScene.mMeshes</code>.
       */
      uint* mMeshes;
   }

   /**
    * Flags which are combinated in <code>aiScene.mFlags</code> to store
    * auxiliary information about the imported scene.
    */
   enum aiSceneFlags : uint {
      /**
       * Specifies that the scene data structure that was imported is not
       * complete.
       *
       * This flag bypasses some internal validations and allows the import of
       * animation skeletons, material libraries or camera animation paths
       * using Assimp. Most applications won't support such data.
       */
      INCOMPLETE = 0x1,

      /**
       * This flag is set by the validation post-processing step
       * (<code>aiProcess.ValidateDS</code>) if the validation was successful.
       *
       * In a validated scene you can be sure that any cross references in the
       * data structure (e.g. vertex indices) are valid.
       */
      VALIDATED = 0x2,

      /**
       * This flag is set by the validation post-processing step
       * (<code>aiProcess.ValidateDS</code>) if the validation is successful
       * but some issues have been found.
       *
       * This can for example mean that a texture that does not exist is
       * referenced by a material or that the bone weights for a vertex don't
       * sum to 1. In most cases you should still be able to use the import.
       *
       * This flag could be useful for applications which don't capture
       * Assimp's log output.
       */
      VALIDATION_WARNING = 0x4,

      /**
       * This flag is currently only set by the
       * <code>aiProcess.JoinIdenticalVertices</code> post-processing step. It
       * indicates that the vertices of the output meshes aren't in the
       * internal verbose format anymore. In the verbose format all vertices
       * are unique, no vertex is ever referenced by more than one face.
       */
      NON_VERBOSE_FORMAT = 0x8,

      /**
       * Denotes pure height-map terrain data. Pure terrains usually consist of
       * quads, sometimes triangles, in a regular grid. The x,y coordinates of
       * all vertex positions refer to the x,y coordinates on the terrain
       * height map, the z-axis stores the elevation at a specific point.
       *
       * TER (Terragen) and HMP (3D Game Studio) are height map formats.
       *
       * Note: Assimp is probably not the best choice for loading <em>huge</em>
       *    terrains – fully triangulated data takes extremely much storage
       *    space and should be avoided as long as possible (typically you will
       *    perform the triangulation when you actually need to render it).
       */
      FLAGS_TERRAIN = 0x10
   }

   /**
    * The root structure of the imported data.
    *
    * Everything that was imported from the given file can be accessed from here.
    * Objects of this class are generally maintained and owned by Assimp, not
    * by the caller. You shouldn't want to instance it, nor should you ever try to
    * delete a given scene on your own.
    */
   struct aiScene {
      /**
       * Any combination of the <code>aiSceneFlags</code>. By default, this
       * value is 0, no flags are set.
       *
       * Most applications will want to reject all scenes with the
       * <code>aiSceneFlags.INCOMPLETE</code> bit set.
       */
      uint mFlags;

      /**
       * The root node of the hierarchy.
       *
       * There will always be at least the root node if the import was
       * successful (and no special flags have been set). Presence of further
       * nodes depends on the format and contents of the imported file.
       */
      aiNode* mRootNode;

      /**
       * The number of meshes in the scene.
       */
      uint mNumMeshes;

      /**
       * The array of meshes.
       *
       * Use the indices given in the <code>aiNode</code> structure to access
       * this array. The array is <code>mNumMeshes</code> in size.
       *
       * If the <code>aiSceneFlags.INCOMPLETE</code> flag is not set, there
       * will always be at least one mesh.
       */
      aiMesh** mMeshes;

      /**
       * The number of materials in the scene.
       */
      uint mNumMaterials;

      /**
       * The array of meshes.
       *
       * Use the indices given in the <code>aiMesh</code> structure to access
       * this array. The array is <code>mNumMaterials</code> in size.
       *
       * If the <code>aiSceneFlags.INCOMPLETE</code> flag is not set, there
       * will always be at least one material.
       */
      aiMaterial** mMaterials;

      /**
       * The number of animations in the scene.
       */
      uint mNumAnimations;

      /**
       * The array of animations.
       *
       * All animations imported from the given file are listed here. The array
       * is <code>mNumAnimations</code> in size.
      */
      aiAnimation** mAnimations;

      /**
       * The number of textures embedded into the file.
       */
      uint mNumTextures;

      /**
       * The array of embedded textures.
       *
       * Not many file formats embed their textures into the file. An example
       * is Quake's <code>MDL</code> format (which is also used by some
       * GameStudio versions).
       */
      aiTexture** mTextures;

      /**
       * The number of light sources in the scene.
       *
       * Light sources are fully optional, in most cases this attribute will be
       * 0.
       */
      uint mNumLights;

      /**
       * The array of light sources.
       *
       * All light sources imported from the given file are listed here. The
       * array is <code>mNumLights</code> in size.
       */
      aiLight** mLights;

      /**
       * The number of cameras in the scene.
       *
       * Cameras are fully optional, in most cases this attribute
       * will be 0.
       */
      uint mNumCameras;

      /**
       * The array of cameras.
       *
       * All cameras imported from the given file are listed here. The array is
       * <code>mNumCameras</code> in size.
       *
       * The first camera in the array (if existing) is the default camera view
       * at the scene.
       */
      aiCamera** mCameras;
   }
}
