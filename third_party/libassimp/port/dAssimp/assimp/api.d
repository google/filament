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
 * The C-style interface to the Open Asset import library.
 *
 * All functions of the C API have been collected in this module as function
 * pointers, which are set by the dynamic library loader
 * (<code>assimp.loader</code>).
 */
module assimp.api;

import assimp.fileIO;
import assimp.material;
import assimp.math;
import assimp.scene;
import assimp.types;

extern ( C ) {
   /**
    * Reads the given file and returns its content.
    *
    * If the call succeeds, the imported data is returned in an <code>aiScene</code>
    * structure. The data is intended to be read-only, it stays property of the
    * Assimp library and will be stable until <code>aiReleaseImport()</code> is
    * called. After you're done with it, call <code>aiReleaseImport()</code> to
    * free the resources associated with this file.
    *
    * If an error is encountered, null is returned instead. Call
    * <code>aiGetErrorString()</code> to retrieve a human-readable error
    * description.
    *
    * Params:
    *    pFile = Path and filename of the file to be imported,
    *       expected to be a null-terminated C-string. null is not a valid value.
    *    pFlags = Optional post processing steps to be executed after a
    *       successful import. Provide a bitwise combination of the
    *       <code>aiPostProcessSteps</code> flags. If you wish to inspect the
    *       imported scene first in order to fine-tune your post-processing
    *       setup, consider to use <code>aiApplyPostProcessing()</code>.
    *
    * Returns:
    *    A pointer to the imported data, null if the import failed.
    */
   aiScene* function( char* pFile, uint pFile ) aiImportFile;

   /**
    * Reads the given file using user-defined I/O functions and returns its
    * content.
    *
    * If the call succeeds, the imported data is returned in an <code>aiScene</code>
    * structure. The data is intended to be read-only, it stays property of the
    * Assimp library and will be stable until <code>aiReleaseImport()</code> is
    * called. After you're done with it, call <code>aiReleaseImport()</code> to
    * free the resources associated with this file.
    *
    * If an error is encountered, null is returned instead. Call
    * <code>aiGetErrorString()</code> to retrieve a human-readable error
    * description.
    *
    * Params:
    *    pFile = Path and filename of the file to be imported,
    *       expected to be a null-terminated C-string. null is not a valid value.
    *    pFlags = Optional post processing steps to be executed after a
    *       successful import. Provide a bitwise combination of the
    *       <code>aiPostProcessSteps</code> flags. If you wish to inspect the
    *       imported scene first in order to fine-tune your post-processing
    *       setup, consider to use <code>aiApplyPostProcessing()</code>.
    *    pFS = An aiFileIO which will be used to open the model file itself
    *       and any other files the loader needs to open.
    *
    * Returns:
    *    A pointer to the imported data, null if the import failed.
    */
   aiScene* function( char* pFile, uint pFlags, aiFileIO* pFS ) aiImportFileEx;

   /**
    * Reads the scene from the given memory buffer.
    *
    * Reads the given file using user-defined I/O functions and returns its
    * content.
    *
    * If the call succeeds, the imported data is returned in an <code>aiScene</code>
    * structure. The data is intended to be read-only, it stays property of the
    * Assimp library and will be stable until <code>aiReleaseImport()</code> is
    * called. After you're done with it, call <code>aiReleaseImport()</code> to
    * free the resources associated with this file.
    *
    * If an error is encountered, null is returned instead. Call
    * <code>aiGetErrorString()</code> to retrieve a human-readable error
    * description.
    *
    * Params:
    *    pBuffer = Pointer to the scene data.
    *    pLength = Size of pBuffer in bytes.
    *    pFlags = Optional post processing steps to be executed after a
    *       successful import. Provide a bitwise combination of the
    *       <code>aiPostProcessSteps</code> flags. If you wish to inspect the
    *       imported scene first in order to fine-tune your post-processing
    *       setup, consider to use <code>aiApplyPostProcessing()</code>.
    *    pHint = An additional hint to the library. If this is a non empty
    *       string, the library looks for a loader to support the file
    *       extension specified and passes the file to the first matching
    *       loader. If this loader is unable to complete the request, the
    *       library continues and tries to determine the file format on its
    *       own, a task that may or may not be successful.
    *
    * Returns:
    *    A pointer to the imported data, null if the import failed.
    *
    * Note:
    *    This is a straightforward way to decode models from memory buffers,
    *    but it doesn't handle model formats spreading their data across
    *    multiple files or even directories. Examples include OBJ or MD3, which
    *    outsource parts of their material stuff into external scripts. If you
    *    need the full functionality, provide a custom IOSystem to make Assimp
    *    find these files.
    */
   aiScene* function(
      char* pBuffer,
      uint pLength,
      uint pFlags,
      char* pHint
   ) aiImportFileFromMemory;

   /**
    * Apply post-processing to an already-imported scene.
    *
    * This is strictly equivalent to calling <code>aiImportFile()</code> or
    * <code>aiImportFileEx()</code> with the same flags. However, you can use
    * this separate function to inspect the imported scene first to fine-tune
    * your post-processing setup.
    *
    * Params:
    *    pScene = Scene to work on.
    *    pFlags = Provide a bitwise combination of the
    *       <code>aiPostProcessSteps</code> flags.
    *
    * Returns:
    *    A pointer to the post-processed data. Post processing is done in-place,
    *    meaning this is still the same <code>aiScene</code> which you passed
    *    for pScene. However, if post-processing failed, the scene could now be
    *    null. That's quite a rare case, post processing steps are not really
    *    designed to fail. To be exact, <code>aiProcess.ValidateDS</code> is
    *    currently the only post processing step which can actually cause the
    *    scene to be reset to null.
    */
   aiScene* function( aiScene* pScene, uint pFlags ) aiApplyPostProcessing;

   /**
    * Get one of the predefined log streams. This is the quick'n'easy solution
    * to access Assimp's log system. Attaching a log stream can slightly reduce
    * Assimp's overall import performance.
    *
    * Examples:
    * ---
    * aiLogStream stream = aiGetPredefinedLogStream(
    *    aiDefaultLogStream.FILE, "assimp.log.txt" );
    * if ( stream.callback !is null ) {
    *    aiAttachLogStream( &stream );
    * }
    * ---
    *
    * Params:
    *    pStreams = The log stream destination.
    *    file = Solely for the <code>aiDefaultLogStream.FILE</code> flag:
    *       specifies the file to write to. Pass null for all other flags.
    *
    * Returns:
    *    The log stream, null if something went wrong.
    */
   aiLogStream function( aiDefaultLogStream pStreams, char* file ) aiGetPredefinedLogStream;

   /**
    * Attach a custom log stream to the libraries' logging system.
    *
    * Attaching a log stream can slightly reduce Assimp's overall import
    * performance. Multiple log-streams can be attached.
    *
    * Params:
    *    stream = Describes the new log stream.
    *
    * Note: To ensure proper destruction of the logging system, you need to
    *    manually call <code>aiDetachLogStream()</code> on every single log
    *    stream you attach. Alternatively, <code>aiDetachAllLogStreams()</code>
    *    is provided.
    */
   void function( aiLogStream* stream ) aiAttachLogStream;

   /**
    * Enable verbose logging.
    *
    * Verbose logging includes debug-related stuff and detailed import
    * statistics. This can have severe impact on import performance and memory
    * consumption. However, it might be useful to find out why a file is not
    * read correctly.
    *
    * Param:
    *    d = Whether verbose logging should be enabled.
    */
   void function( aiBool d ) aiEnableVerboseLogging;

   /**
    * Detach a custom log stream from the libraries' logging system.
    *
    * This is the counterpart of #aiAttachPredefinedLogStream. If you attached a stream,
    * don't forget to detach it again.
    *
    * Params:
    *    stream = The log stream to be detached.
    *
    * Returns:
    *    <code>aiReturn.SUCCESS</code> if the log stream has been detached
    *    successfully.
    *
    * See: <code>aiDetachAllLogStreams</code>
    */
   aiReturn function( aiLogStream* stream ) aiDetachLogStream;

   /**
    * Detach all active log streams from the libraries' logging system.
    *
    * This ensures that the logging system is terminated properly and all
    * resources allocated by it are actually freed. If you attached a stream,
    * don't forget to detach it again.
    *
    * See: <code>aiAttachLogStream</code>, <code>aiDetachLogStream</code>
    */
   void function() aiDetachAllLogStreams;

   /**
    * Releases all resources associated with the given import process.
    *
    * Call this function after you're done with the imported data.
    *
    * Params:
    *    pScene = The imported data to release. null is a valid value.
    */
   void function( aiScene* pScene ) aiReleaseImport;

   /**
    * Returns the error text of the last failed import process.
    *
    * Returns:
    *    A textual description of the error that occurred at the last importing
    *    process. null if there was no error. There can't be an error if you
    *    got a non-null <code>aiScene</code> from
    *    <code>aiImportFile()/aiImportFileEx()/aiApplyPostProcessing()</code>.
    */
   char* function() aiGetErrorString;

   /**
    * Returns whether a given file extension is supported by this Assimp build.
    *
    * Params:
    *    szExtension = Extension for which to query support. Must include a
    *       leading dot '.'. Example: ".3ds", ".md3"
    *
    * Returns:
    *    <code>TRUE</code> if the file extension is supported.
    */
   aiBool function( char* szExtension ) aiIsExtensionSupported;

   /**
    * Gets a list of all file extensions supported by ASSIMP.
    *
    * Format of the list: "*.3ds;*.obj;*.dae".
    *
    * If a file extension is contained in the list this does, of course, not
    * mean that Assimp is able to load all files with this extension.
    *
    * Params:
    *    szOut = String to receive the extension list. null is not a valid
    *       parameter.
    */
   void function( aiString* szOut ) aiGetExtensionList;

   /**
    * Gets the storage required by an imported asset
    *
    * Params:
    *    pIn = Asset to query storage size for.
    *    info = Data structure to be filled.
    */
   void function( aiScene* pIn, aiMemoryInfo* info ) aiGetMemoryRequirements;

   /**
    * Sets an integer property.
    *
    * Properties are always shared by all imports. It is not possible to
    * specify them per import.
    *
    * Params:
    *    szName = Name of the configuration property to be set. All supported
    *       public properties are defined in the <code>config</code> module.
    *    value = New value for the property.
    */
   void function( char* szName, int value ) aiSetImportPropertyInteger;

   /**
    * Sets a floating-point property.
    *
    * Properties are always shared by all imports. It is not possible to
    * specify them per import.
    *
    * Params:
    *    szName = Name of the configuration property to be set. All supported
    *       public properties are defined in the <code>config</code> module.
    *    value = New value for the property.
    */
   void function( char* szName, float value ) aiSetImportPropertyFloat;

   /**
    * Sets a string property.
    *
    * Properties are always shared by all imports. It is not possible to
    * specify them per import.
    *
    * Params:
    *    szName = Name of the configuration property to be set. All supported
    *       public properties are defined in the <code>config</code> module.
    *    st = New value for the property.
    */
   void function( char* szName, aiString* st ) aiSetImportPropertyString;


   /*
    * Mathematical helper functions.
    */

   /**
    * Constructs a quaternion from a 3x3 rotation matrix.
    *
    * Params:
    *    quat = Receives the output quaternion.
    *    mat = Matrix to 'quaternionize'.
    */
   void function( aiQuaternion* quat, aiMatrix3x3* mat ) aiCreateQuaternionFromMatrix;

   /**
    * Decomposes a transformation matrix into its rotational, translational and
    * scaling components.
    *
    * Params:
    *    mat = Matrix to decompose.
    *    scaling = Receives the scaling component.
    *    rotation = Receives the rotational component.
    *    position = Receives the translational component.
    */
   void function(
      aiMatrix4x4* mat,
      aiVector3D* scaling,
      aiQuaternion* rotation,
      aiVector3D* position
   ) aiDecomposeMatrix;

   /**
    * Transposes a 4x4 matrix (in-place).
    *
    * Params:
    *    mat = The matrix to be transposed.
    */
   void function( aiMatrix4x4* mat ) aiTransposeMatrix4;

   /**
    * Transposes a 3x3 matrix (in-place).
    *
    * Params:
    *    mat = The matrix to be transposed.
    */
   void function( aiMatrix3x3* mat ) aiTransposeMatrix3;

   /**
    * Transforms a vector by a 3x3 matrix (in-place).
    *
    * Params:
    *    vec = Vector to be transformed.
    *    mat = Matrix to transform the vector with.
    */
   void function( aiVector3D* vec, aiMatrix3x3* mat ) aiTransformVecByMatrix3;

   /**
    * Transforms a vector by a 4x4 matrix (in-place).
    *
    * Params:
    *    vec = Vector to be transformed.
    *    mat = Matrix to transform the vector with.
    */
   void function( aiVector3D* vec, aiMatrix4x4* mat ) aiTransformVecByMatrix4;

   /**
    * Multiplies two 4x4 matrices.
    *
    * Params:
    *    dst = First factor, receives result.
    *    src = Matrix to be multiplied with 'dst'.
    */
   void function( aiMatrix4x4* dst, aiMatrix4x4* src ) aiMultiplyMatrix4;

   /**
    * Multiplies two 3x3 matrices.
    *
    * Params:
    *    dst = First factor, receives result.
    *    src = Matrix to be multiplied with 'dst'.
    */
   void function( aiMatrix3x3* dst, aiMatrix3x3* src ) aiMultiplyMatrix3;

   /**
    * Constructs a 3x3 identity matrix.
    *
    * Params:
    *    mat = Matrix to receive its personal identity.
    */
   void function( aiMatrix3x3* mat ) aiIdentityMatrix3;

   /**
    * Constructs a 4x4 identity matrix.
    *
    * Params:
    *    mat = Matrix to receive its personal identity.
    */
   void function( aiMatrix4x4* mat ) aiIdentityMatrix4;


   /*
    * Material system functions.
    */

   /**
    * Retrieves a material property with a specific key from the material.
    *
    * Params:
    *    pMat = Pointer to the input material. May not be null.
    *    pKey = Key to search for. One of the <code>AI_MATKEY_XXX</code>
    *       constants.
    *    type = Specifies the <code>aiTextureType</code> of the texture to be
    *       retrieved, 0 for non-texture properties.
    *    index = Index of the texture to be retrieved,
    *       0 for non-texture properties.
    *    pPropOut = Pointer to receive a pointer to a valid
    *       <code>aiMaterialProperty</code> structure or null if the key has
    *       not been found.
    */
   aiReturn function(
     aiMaterial* pMat,
     char* pKey,
     uint type,
     uint index,
     aiMaterialProperty** pPropOut
   ) aiGetMaterialProperty;

   /**
    * Retrieves a single float value or an array of float values from the
    * material.
    *
    * Examples:
    * ---
    * const FLOATS_IN_UV_TRANSFORM = ( aiUVTransform.sizeof / float.sizeof );
    * uint valuesRead = FLOATS_IN_UV_TRANSFORM;
    * bool success =
    *    ( aiGetMaterialFloatArray( &material, AI_MATKEY_UVTRANSFORM,
    *       aiTextureType.DIFFUSE, 0, cast( float* ) &trafo, &valuesRead ) ==
    *       aiReturn.SUCCESS ) &&
    *    ( valuesRead == FLOATS_IN_UV_TRANSFORM );
    * ---
    *
    * Params:
    *    pMat = Pointer to the input material. May not be null.
    *    pKey = Key to search for. One of the AI_MATKEY_XXX constants.
    *    type = Specifies the <code>aiTextureType</code> of the texture to be
    *       retrieved, 0 for non-texture properties.
    *    index = Index of the texture to be retrieved,
    *       0 for non-texture properties.
    *    pOut = Pointer to a buffer to receive the result.
    *    pMax = Specifies the size of the given buffer in floats. Receives the
    *       number of values (not bytes!) read. null to read a scalar property.
    *
    * Returns:
    *    Specifies whether the key has been found. If not, the output arrays
    *    remains unmodified and pMax is set to 0.
    */
   aiReturn function(
      aiMaterial* pMat,
      char* pKey,
      uint type,
      uint index,
      float* pOut,
      uint* pMax = null
   ) aiGetMaterialFloatArray;

   /**
    * Convenience alias for <code>aiGetMaterialFloatArray()</code>.
    */
   alias aiGetMaterialFloatArray aiGetMaterialFloat;

   /**
    * Retrieves a single integer value or an array of integer values from the
    * material.
    *
    * See: <code>aiGetMaterialFloatArray()</code>
    */
   aiReturn function(
      aiMaterial* pMat,
      char* pKey,
      uint type,
      uint index,
      int* pOut,
      uint* pMax = null
   ) aiGetMaterialIntegerArray;

   /**
    * Convenience alias for <code>aiGetMaterialIntegerArray()</code>.
    */
   alias aiGetMaterialIntegerArray aiGetMaterialInteger;

   /**
    * Retrieves a color value from the material.
    *
    * See: <code>aiGetMaterialFloatArray()</code>
    */
   aiReturn function(
      aiMaterial* pMat,
      char* pKey,
      uint type,
      uint index,
      aiColor4D* pOut
   ) aiGetMaterialColor;

   /**
    * Retrieves a string value from the material.
    *
    * See: <code>aiGetMaterialFloatArray()</code>
    */
   aiReturn function(
      aiMaterial* pMat,
      char* pKey,
      uint type,
      uint index,
      aiString* pOut
   ) aiGetMaterialString;

   /**
    * Get the number of textures for a particular texture type.
    *
    * Params:
    *    pMat = Pointer to the input material. May not be NULL
    *    type = Texture type to check for
    *
    * Returns:
    *    Number of textures for this type.
    */
   uint function( aiMaterial* pMat, aiTextureType type ) aiGetMaterialTextureCount;

   /**
    * Helper function to get all values pertaining to a particular texture slot
    * from a material structure.
    *
    * This function is provided just for convenience. You could also read the
    * texture by parsing all of its properties manually. This function bundles
    * all of them in a huge function monster.
    *
    * Params:
    *    mat = Pointer to the input material. May not be null.
    *    type = Specifies the texture stack (<code>aiTextureType</code>) to
    *       read from.
    *    index = Index of the texture. The function fails if the requested
    *       index is not available for this texture type.
    *       <code>aiGetMaterialTextureCount()</code> can be used to determine
    *       the number of textures in a particular texture stack.
    *    path = Receives the output path. null is not a valid value.
    *    mapping = Receives the texture mapping mode to be used.
    *       Pass null if you are not interested in this information.
    *    uvindex = For UV-mapped textures: receives the index of the UV source
    *       channel. Unmodified otherwise. Pass null if you are not interested
    *       in this information.
    *    blend = Receives the blend factor for the texture.
    *       Pass null if you are not interested in this information.
    *    op = Receives the texture blend operation to be perform between this
    *       texture and the previous texture. Pass null if you are not
    *       interested in this information.
    *    mapmode = Receives the mapping modes to be used for the texture. Pass
    *       a pointer to an array of two aiTextureMapMode's (one for each axis,
    *       UV order) or null if you are not interested in this information.
    *
    * Returns:
    *    <code>aiReturn.SUCCESS</code> on success, otherwise something else.
    */
   aiReturn function(
      aiMaterial* mat,
      aiTextureType type,
      uint index,
      aiString* path,
      aiTextureMapping* mapping = null,
      uint* uvindex = null,
      float* blend = null,
      aiTextureOp* op = null,
      aiTextureMapMode* mapmode = null
   ) aiGetMaterialTexture;


   /*
    * Versioning functions.
    */

   /**
    * Returns a string with legal copyright and licensing information about
    * Assimp.
    *
    * The string may include multiple lines.
    *
    * Returns:
    *    Pointer to static string.
    */
   char* function() aiGetLegalString;

   /**
    * Returns the current minor version number of the Assimp library.
    *
    * Returns:
    *    Minor version of the Assimp library.
    */
   uint function() aiGetVersionMinor;

   /**
    * Returns the current major version number of the Assimp library.
    *
    * Returns:
    *    Major version of the Assimp library.
    */
   uint function() aiGetVersionMajor;

   /**
    * Returns the repository revision of the Assimp library.
    *
    * Returns:
    *    SVN Repository revision number of the Assimp library.
    */
   uint function() aiGetVersionRevision;

   /**
    * Returns the flags Assimp was compiled with.
    *
    * Returns:
    *    Any bitwise combination of the ASSIMP_CFLAGS_xxx constants.
    */
   uint function() aiGetCompileFlags;
}
