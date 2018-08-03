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
 * The data structures necessary to use Assimip with a custom IO system.
 */
module assimp.fileIO;

import assimp.types;

extern ( C ) {
   // aiFile callbacks
   alias size_t function( aiFile*, char*, size_t, size_t ) aiFileWriteProc;
   alias size_t function(  aiFile*, char*, size_t, size_t ) aiFileReadProc;
   alias size_t function( aiFile* ) aiFileTellProc;
   alias void function( aiFile* ) aiFileFlushProc;
   alias aiReturn function( aiFile*, size_t, aiOrigin ) aiFileSeek;

   // aiFileIO callbacks
   alias aiFile* function( aiFileIO*, char*, char* ) aiFileOpenProc;
   alias void function( aiFileIO*,  aiFile* ) aiFileCloseProc;

   /**
    * Represents user-defined data.
    */
   alias char* aiUserData;

   /**
    * File system callbacks.
    *
    * Provided are functions to open and close files. Supply a custom structure
    * to the import function. If you don't, a default implementation is used.
    * Use custom file systems to enable reading from other sources, such as
    * ZIPs or memory locations.
    */
   struct aiFileIO {
      /**
       * Function used to open a new file
       */
      aiFileOpenProc OpenProc;

      /**
       * Function used to close an existing file
       */
      aiFileCloseProc CloseProc;

      /**
       * User-defined, opaque data.
       */
      aiUserData UserData;
   }

   /**
    * File callbacks.
    *
    * Actually, it's a data structure to wrap a set of <code>fXXXX</code>
    * (e.g <code>fopen()</code>) replacement functions.
    *
    * The default implementation of the functions utilizes the <code>fXXX</code>
    * functions from the CRT. However, you can supply a custom implementation
    * to Assimp by passing a custom <code>aiFileIO</code>. Use this to enable
    * reading from other sources such as ZIP archives or memory locations.
    */
   struct aiFile {
      /**
       * Callback to read from a file.
       */
      aiFileReadProc ReadProc;

      /**
       * Callback to write to a file.
       */
      aiFileWriteProc WriteProc;

      /**
       * Callback to retrieve the current position of the file cursor
       * (<code>ftell()</code>).
       */
      aiFileTellProc TellProc;

      /**
       * Callback to retrieve the size of the file, in bytes.
       */
      aiFileTellProc FileSizeProc;

      /**
       * Callback to set the current position of the file cursor
       * (<code>fseek()</code>).
       */
      aiFileSeek SeekProc;

      /**
       * Callback to flush the file contents.
       */
      aiFileFlushProc FlushProc;

      /**
       * User-defined, opaque data.
       */
      aiUserData UserData;
   }
}
