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

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;

/**
 * IOSystem based on the Java classloader.<p>
 * 
 * This IOSystem allows loading models directly from the 
 * classpath. No extraction to the file system is 
 * necessary.
 * 
 * @author Jesper Smith
 *
 */
public class AiClassLoaderIOSystem implements AiIOSystem<AiInputStreamIOStream>
{
   private final Class<?> clazz;
   private final ClassLoader classLoader;
  
   /**
    * Construct a new AiClassLoaderIOSystem.<p>
    * 
    * This constructor uses a ClassLoader to resolve
    * resources.
    * 
    * @param classLoader classLoader to resolve resources.
    */
   public AiClassLoaderIOSystem(ClassLoader classLoader) {
      this.clazz = null;
      this.classLoader = classLoader;
   }

   /**
    * Construct a new AiClassLoaderIOSystem.<p>
    * 
    * This constructor uses a Class to resolve
    * resources.
    * 
    * @param class<?> class to resolve resources.
    */
   public AiClassLoaderIOSystem(Class<?> clazz) {
      this.clazz = clazz;
      this.classLoader = null;
   }
   

   @Override
   public AiInputStreamIOStream open(String filename, String ioMode) {
      try {
         
         InputStream is;
         
         if(clazz != null) {
            is = clazz.getResourceAsStream(filename);
         }
         else if (classLoader != null) {
            is = classLoader.getResourceAsStream(filename);
         }
         else {
            System.err.println("[" + getClass().getSimpleName() + 
                "] No class or classLoader provided to resolve " + filename);
            return null;
         }
         
         if(is != null) {
            return new AiInputStreamIOStream(is);
         }
         else {
            System.err.println("[" + getClass().getSimpleName() + 
                               "] Cannot find " + filename);
            return null;
         }
      }
      catch (IOException e) {
         e.printStackTrace();
         return null;
      }
   }

   @Override
   public void close(AiInputStreamIOStream file) {
   }

   @Override
   public boolean exists(String path)
   {
      URL url = null;
      if(clazz != null) {
         url = clazz.getResource(path);
      }
      else if (classLoader != null) {
         url = classLoader.getResource(path);
      }

      
      if(url == null)
      {
         return false;
      }

	  return true;
      
   }

   @Override
   public char getOsSeparator()
   {
      return '/';
   }

}
