/*
---------------------------------------------------------------------------
Open Asset Import Library - Java Binding (jassimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URL;
import java.nio.ByteBuffer;


/**
 * Implementation of AiIOStream reading from a InputStream
 * 
 * @author Jesper Smith
 *
 */
public class AiInputStreamIOStream implements AiIOStream
{
   private final ByteArrayOutputStream os = new ByteArrayOutputStream(); 
   
   
   public AiInputStreamIOStream(URI uri) throws IOException {
      this(uri.toURL());
   }
   
   public AiInputStreamIOStream(URL url) throws IOException {
      this(url.openStream());
   }
   
   public AiInputStreamIOStream(InputStream is) throws IOException {
      int read;
      byte[] data = new byte[1024];
      while((read = is.read(data, 0, data.length)) != -1) {
         os.write(data, 0, read);
      }
      os.flush();
      
      is.close();
   }
   
   @Override
   public int getFileSize() {
      return os.size();
   }
   
   @Override
   public boolean read(ByteBuffer buffer) {
     ByteBufferOutputStream bos = new ByteBufferOutputStream(buffer);
     try
     {
        os.writeTo(bos);
     }
     catch (IOException e)
     {
        e.printStackTrace();
        return false;
     }
     return true;
   }
   
   /**
    * Internal helper class to copy the contents of an OutputStream
    * into a ByteBuffer. This avoids a copy.
    *
    */
   private static class ByteBufferOutputStream extends OutputStream {

      private final ByteBuffer buffer;
      
      public ByteBufferOutputStream(ByteBuffer buffer) {
         this.buffer = buffer;
      }
      
      @Override
      public void write(int b) throws IOException
      {
         buffer.put((byte) b);
      }
    
      @Override
      public void write(byte b[], int off, int len) throws IOException {
         buffer.put(b, off, len);
      }
   }
}

