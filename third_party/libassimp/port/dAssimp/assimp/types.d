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
 * Contains miscellaneous types used in Assimp's C API.
 */
module assimp.types;

extern ( C ) {
   /**
    * Our own C boolean type.
    */
   enum aiBool : int {
      FALSE = 0,
      TRUE = 1
   }

   /**
    * Type definition for log stream callback function pointers.
    */
   alias void function( char* message, char* user ) aiLogStreamCallback;

   /**
    * Represents a log stream. A log stream receives all log messages and
    * streams them somewhere.
    *
    * See: <code>aiGetPredefinedLogStream</code>,
    *    <code>aiAttachLogStream</code> and <code>aiDetachLogStream</code>.
    */
   struct aiLogStream {
      /**
       * Callback function to be called when a new message arrives.
       */
      aiLogStreamCallback callback;

      /**
       * User data to be passed to the callback.
       */
      char* user;
   }

   /**
    * Maximum dimension for <code>aiString</code>s.
    *
    * Assimp strings are zero terminated.
    */
   const size_t MAXLEN = 1024;

   /**
    * Represents an UTF-8 string, zero byte terminated.
    *
    * The length of such a string is limited to <code>MAXLEN</code> bytes
    * (excluding the terminal \0).
    *
    * The character set of an aiString is explicitly defined to be UTF-8. This
    * Unicode transformation was chosen in the belief that most strings in 3d
    * model files are limited to ASCII characters, thus the character set
    * needed to be ASCII compatible.
    *
    * Most text file loaders provide proper Unicode input file handling,
    * special unicode characters are correctly transcoded to UTF-8 and are kept
    * throughout the libraries' import pipeline.
    *
    * For most applications, it will be absolutely sufficient to interpret the
    * aiString as ASCII data and work with it as one would work with a plain
    * char[].
    *
    * To access an aiString from D you might want to use something like the
    * following piece of code:
    * ---
    * char[] importAiString( aiString* s ) {
    *  return s.data[ 0 .. s.length ];
    * }
    * ---
    */
   struct aiString {
      /**
       * Length of the string (excluding the terminal \0).
       *
       * This is <em>not</em> the logical length of strings containing UTF-8
       * multibyte sequences, but the number of bytes from the beginning of the
       * string to its end.
       */
      size_t length;

      /**
       * String buffer.
       *
       * Size limit is <code>MAXLEN</code>.
       */
      char data[ MAXLEN ];
   }

   /**
    * Standard return type for some library functions.
    */
   enum aiReturn : uint {
      /**
       * Indicates that a function was successful.
       */
      SUCCESS = 0x0,

      /**
       * Indicates that a function failed.
       */
      FAILURE = -0x1,

      /**
       * Indicates that not enough memory was available to perform the
       * requested operation.
       */
      OUTOFMEMORY = -0x3
   }

   /**
    * Seek origins (for the virtual file system API).
    */
   enum aiOrigin : uint {
      /**
       * Beginning of the file.
       */
      SET = 0x0,

      /**
       * Current position of the file pointer.
       */
      CUR = 0x1,

      /**
       * End of the file.
       *
       * Offsets must be negative.
       */
      END = 0x2
   }

   /**
    * Enumerates predefined log streaming destinations.
    *
    * Logging to these streams can be enabled with a single call to
    * <code>aiAttachPredefinedLogStream()</code>.
    */
   enum aiDefaultLogStream :uint {
      /**
       * Stream the log to a file.
       */
      FILE = 0x1,

      /**
       * Stream the log to standard output.
       */
      STDOUT = 0x2,

      /**
       * Stream the log to standard error.
       */
      STDERR = 0x4,

      /**
       * MSVC only: Stream the log the the debugger (this relies on
       * <code>OutputDebugString</code> from the Win32 SDK).
       */
      DEBUGGER = 0x8
   }

   /**
    * Stores the memory requirements for different components (e.g. meshes,
    * materials, animations) of an import. All sizes are in bytes.
    */
   struct aiMemoryInfo {
      /**
       * Storage allocated for texture data.
       */
      uint textures;

      /**
       * Storage allocated for material data.
       */
      uint materials;

      /**
       * Storage allocated for mesh data.
       */
      uint meshes;

      /**
       * Storage allocated for node data.
       */
      uint nodes;

      /**
       * Storage allocated for animation data.
       */
      uint animations;

      /**
       * Storage allocated for camera data.
       */
      uint cameras;

      /**
       * Storage allocated for light data.
       */
      uint lights;

      /**
       * Total storage allocated for the full import.
       */
      uint total;
   }
}
