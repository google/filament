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
 * Provides facilities for dynamically loading the Assimp library.
 *
 * Currently requires Tango, but there is no reason why Phobos could not be
 * supported too.
 */
module assimp.loader;

import assimp.api;
import tango.io.Stdout;
import tango.sys.SharedLib;

const uint ASSIMP_BINDINGS_MAJOR = 2;
const uint ASSIMP_BINDINGS_MINOR = 0;

/**
 * Loader class for dynamically loading the Assimp library.
 *
 * The library is »reference-counted«, meaning that the library is not
 * unloaded on a call to <code>unload()</code> if there are still other
 * references to it.
 */
struct Assimp {
public:
   /**
    * Loads the library if it is not already loaded and increases the
    * reference counter.
    *
    * The library file (<code>libassimp.so</code> on POSIX systems,
    * <code>Assimp32.dll</code> on Win32) is loaded via Tango's SharedLib
    * class.
    */
   static void load() {
      if ( m_sRefCount == 0 ) {
         version ( Posix ) {
            version ( OSX ) {
               m_sLibrary = SharedLib.load( "libassimp.dylib" );
            } else {
               m_sLibrary = SharedLib.load( "libassimp.so" );
            }
         }
         version ( Win32 ) {
            m_sLibrary = SharedLib.load( "Assimp32.dll" );
         }

         // Versioning
         mixin( bindCode( "aiGetLegalString" ) );
         mixin( bindCode( "aiGetVersionMinor" ) );
         mixin( bindCode( "aiGetVersionMajor" ) );
         mixin( bindCode( "aiGetVersionRevision" ) );
         mixin( bindCode( "aiGetCompileFlags" ) );

         // Check for version mismatch between the external, dynamically loaded
         // library and the version the bindings were created against.
         uint libMajor = aiGetVersionMajor();
         uint libMinor = aiGetVersionMinor();

         if ( ( libMajor < ASSIMP_BINDINGS_MAJOR ) ||
            ( libMinor < ASSIMP_BINDINGS_MINOR ) ) {
            Stdout.format(
               "WARNING: Assimp version too old (loaded library: {}.{}, " ~
                  "bindings: {}.{})!",
               libMajor,
               libMinor,
               ASSIMP_BINDINGS_MAJOR,
               ASSIMP_BINDINGS_MINOR
            ).newline;
         }

         if ( libMajor > ASSIMP_BINDINGS_MAJOR ) {
            Stdout.format(
               "WARNING: Assimp version too new (loaded library: {}.{}, " ~
                  "bindings: {}.{})!",
               libMajor,
               libMinor,
               ASSIMP_BINDINGS_MAJOR,
               ASSIMP_BINDINGS_MINOR
            ).newline;
         }

         // General API
         mixin( bindCode( "aiImportFile" ) );
         mixin( bindCode( "aiImportFileEx" ) );
         mixin( bindCode( "aiImportFileFromMemory" ) );
         mixin( bindCode( "aiApplyPostProcessing" ) );
         mixin( bindCode( "aiGetPredefinedLogStream" ) );
         mixin( bindCode( "aiAttachLogStream" ) );
         mixin( bindCode( "aiEnableVerboseLogging" ) );
         mixin( bindCode( "aiDetachLogStream" ) );
         mixin( bindCode( "aiDetachAllLogStreams" ) );
         mixin( bindCode( "aiReleaseImport" ) );
         mixin( bindCode( "aiGetErrorString" ) );
         mixin( bindCode( "aiIsExtensionSupported" ) );
         mixin( bindCode( "aiGetExtensionList" ) );
         mixin( bindCode( "aiGetMemoryRequirements" ) );
         mixin( bindCode( "aiSetImportPropertyInteger" ) );
         mixin( bindCode( "aiSetImportPropertyFloat" ) );
         mixin( bindCode( "aiSetImportPropertyString" ) );

         // Mathematical functions
         mixin( bindCode( "aiCreateQuaternionFromMatrix" ) );
         mixin( bindCode( "aiDecomposeMatrix" ) );
         mixin( bindCode( "aiTransposeMatrix4" ) );
         mixin( bindCode( "aiTransposeMatrix3" ) );
         mixin( bindCode( "aiTransformVecByMatrix3" ) );
         mixin( bindCode( "aiTransformVecByMatrix4" ) );
         mixin( bindCode( "aiMultiplyMatrix4" ) );
         mixin( bindCode( "aiMultiplyMatrix3" ) );
         mixin( bindCode( "aiIdentityMatrix3" ) );
         mixin( bindCode( "aiIdentityMatrix4" ) );

         // Material system
         mixin( bindCode( "aiGetMaterialProperty" ) );
         mixin( bindCode( "aiGetMaterialFloatArray" ) );
         mixin( bindCode( "aiGetMaterialIntegerArray" ) );
         mixin( bindCode( "aiGetMaterialColor" ) );
         mixin( bindCode( "aiGetMaterialString" ) );
         mixin( bindCode( "aiGetMaterialTextureCount" ) );
         mixin( bindCode( "aiGetMaterialTexture" ) );
      }
      ++m_sRefCount;
   }

   /**
    * Decreases the reference counter and unloads the library if this was the
    * last reference.
    */
   static void unload() {
      assert( m_sRefCount > 0 );
      --m_sRefCount;

      if ( m_sRefCount == 0 ) {
         m_sLibrary.unload();
      }
   }

private:
   /// Current number of references to the library.
   static uint m_sRefCount;

   /// Library handle.
   static SharedLib m_sLibrary;
}

/**
 * Private helper function which constructs the bind command for a symbol to
 * keep the code DRY.
 */
private char[] bindCode( char[] symbol ) {
   return symbol ~ " = cast( typeof( " ~ symbol ~
      " ) )m_sLibrary.getSymbol( `" ~ symbol ~ "` );";
}
