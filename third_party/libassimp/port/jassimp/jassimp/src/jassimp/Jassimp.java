/*
---------------------------------------------------------------------------
Open Asset Import Library - Java Binding (jassimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team

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
import java.nio.ByteBuffer;
import java.util.EnumSet;
import java.util.Set;



/**
 * Entry point to the jassimp library.<p>
 * 
 * Use {@link #importFile(String, Set)} to load a file.
 * 
 * <h3>General Notes and Pitfalls</h3>
 * Due to the loading via JNI, strings (for example as returned by the 
 * <code>getName()</code> methods) are not interned. You should therefore 
 * compare strings the way it should be done, i.e, via <code>equals()</code>. 
 * Pointer comparison will fail.
 */
public final class Jassimp {

    /**
     * The default wrapper provider using built in types.
     */
    public static final AiWrapperProvider<?, ?, ?, ?, ?> BUILTIN = 
            new AiBuiltInWrapperProvider();
    

    /**
     * Imports a file via assimp without post processing.
     * 
     * @param filename the file to import
     * @return the loaded scene
     * @throws IOException if an error occurs
     */
    public static AiScene importFile(String filename) throws IOException {
        
        return importFile(filename, EnumSet.noneOf(AiPostProcessSteps.class));
    }
    
    /**
     * Imports a file via assimp without post processing.
     * 
     * @param filename the file to import
     * @param ioSystem ioSystem to load files, or null for default
     * @return the loaded scene
     * @throws IOException if an error occurs
     */
    public static AiScene importFile(String filename, AiIOSystem<?> ioSystem) 
          throws IOException {
       
       return importFile(filename, EnumSet.noneOf(AiPostProcessSteps.class), ioSystem);
    }
    
    
    /**
     * Imports a file via assimp.
     * 
     * @param filename the file to import
     * @param postProcessing post processing flags
     * @return the loaded scene, or null if an error occurred
     * @throws IOException if an error occurs
     */
    public static AiScene importFile(String filename, 
                                     Set<AiPostProcessSteps> postProcessing) 
                                           throws IOException {
        return importFile(filename, postProcessing, null);
    }
    
    /**
     * Imports a file via assimp.
     * 
     * @param filename the file to import
     * @param postProcessing post processing flags
     * @param ioSystem ioSystem to load files, or null for default
     * @return the loaded scene, or null if an error occurred
     * @throws IOException if an error occurs
     */
    public static AiScene importFile(String filename, 
            Set<AiPostProcessSteps> postProcessing, AiIOSystem<?> ioSystem) 
                  throws IOException {
        
       loadLibrary();
       
        return aiImportFile(filename, AiPostProcessSteps.toRawValue(
                postProcessing), ioSystem);
    }
    
    
    /**
     * Returns the size of a struct or ptimitive.<p>
     * 
     * @return the result of sizeof call
     */
    public static native int getVKeysize();

    /**
     * @see #getVKeysize
     */
    public static native int getQKeysize();

    /**
     * @see #getVKeysize
     */
    public static native int getV3Dsize();

    /**
     * @see #getVKeysize
     */
    public static native int getfloatsize();

    /**
     * @see #getVKeysize
     */
    public static native int getintsize();

    /**
     * @see #getVKeysize
     */
    public static native int getuintsize();

    /**
     * @see #getVKeysize
     */
    public static native int getdoublesize();

    /**
     * @see #getVKeysize
     */
    public static native int getlongsize();

    /**
     * Returns a human readable error description.<p>
     * 
     * This method can be called when one of the import methods fails, i.e.,
     * throws an exception, to get a human readable error description.
     * 
     * @return the error string
     */
    public static native String getErrorString();
    
    
    /**
     * Returns the active wrapper provider.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).
     * 
     * @return the active wrapper provider
     */
    public static AiWrapperProvider<?, ?, ?, ?, ?> getWrapperProvider() {
        return s_wrapperProvider;
    }
    
    
    /**
     * Sets a new wrapper provider.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).
     * 
     * @param wrapperProvider the new wrapper provider
     */
    public static void setWrapperProvider(AiWrapperProvider<?, ?, ?, ?, ?> 
            wrapperProvider) {
        
        s_wrapperProvider = wrapperProvider;
    }
    
    
    public static void setLibraryLoader(JassimpLibraryLoader libraryLoader) {
       s_libraryLoader = libraryLoader;
    }
    
    
    /**
     * Helper method for wrapping a matrix.<p>
     * 
     * Used by JNI, do not modify!
     * 
     * @param data the matrix data
     * @return the wrapped matrix
     */
    static Object wrapMatrix(float[] data) {
        return s_wrapperProvider.wrapMatrix4f(data);
    }
    
    
    /**
     * Helper method for wrapping a color (rgb).<p>
     * 
     * Used by JNI, do not modify!
     * 
     * @param red red component
     * @param green green component
     * @param blue blue component
     * @return the wrapped color
     */
    static Object wrapColor3(float red, float green, float blue) {
        return wrapColor4(red, green, blue, 1.0f);
    }
    
    
    /**
     * Helper method for wrapping a color (rgba).<p>
     * 
     * Used by JNI, do not modify!
     * 
     * @param red red component
     * @param green green component
     * @param blue blue component
     * @param alpha alpha component
     * @return the wrapped color
     */
    static Object wrapColor4(float red, float green, float blue, float alpha) {
        ByteBuffer temp = ByteBuffer.allocate(4 * 4);
        temp.putFloat(red);
        temp.putFloat(green);
        temp.putFloat(blue);
        temp.putFloat(alpha);
        temp.flip();
        return s_wrapperProvider.wrapColor(temp, 0);
    }
    
    
    /**
     * Helper method for wrapping a vector.<p>
     * 
     * Used by JNI, do not modify!
     * 
     * @param x x component
     * @param y y component
     * @param z z component
     * @return the wrapped vector
     */
    static Object wrapVec3(float x, float y, float z) {
        ByteBuffer temp = ByteBuffer.allocate(3 * 4);
        temp.putFloat(x);
        temp.putFloat(y);
        temp.putFloat(z);
        temp.flip();
        return s_wrapperProvider.wrapVector3f(temp, 0, 3);
    }
    
    
    /**
     * Helper method for wrapping a scene graph node.<p>
     * 
     * Used by JNI, do not modify!
     * 
     * @param parent the parent node
     * @param matrix the transformation matrix
     * @param meshRefs array of matrix references
     * @param name the name of the node
     * @return the wrapped matrix
     */
    static Object wrapSceneNode(Object parent, Object matrix, int[] meshRefs,
            String name) {
        
        return s_wrapperProvider.wrapSceneNode(parent, matrix, meshRefs, name);
    }
    
    /**
     * Helper method to load the library using the provided JassimpLibraryLoader.<p>
     * 
     * Synchronized to avoid race conditions.
     */
    private static void loadLibrary()
    {
       if(!s_libraryLoaded)
       {
          synchronized(s_libraryLoadingLock)
          {
             if(!s_libraryLoaded)
             {
                s_libraryLoader.loadLibrary();
                NATIVE_AIVEKTORKEY_SIZE = getVKeysize();
                NATIVE_AIQUATKEY_SIZE = getQKeysize();
                NATIVE_AIVEKTOR3D_SIZE = getV3Dsize();
                NATIVE_FLOAT_SIZE = getfloatsize();
                NATIVE_INT_SIZE = getintsize();
                NATIVE_UINT_SIZE = getuintsize();
                NATIVE_DOUBLE_SIZE = getdoublesize();
                NATIVE_LONG_SIZE = getlongsize();
                
                s_libraryLoaded = true;
             }
          }
          
       }
    }
    
    /**
     * The native interface.
     * 
     * @param filename the file to load
     * @param postProcessing post processing flags
     * @return the loaded scene, or null if an error occurred
     * @throws IOException if an error occurs
     */
    private static native AiScene aiImportFile(String filename, 
            long postProcessing, AiIOSystem<?> ioSystem) throws IOException;
    
    
    /**
     * The active wrapper provider.
     */
    private static AiWrapperProvider<?, ?, ?, ?, ?> s_wrapperProvider = 
            new AiBuiltInWrapperProvider();
    
    
    /**
     * The library loader to load the native library.
     */
    private static JassimpLibraryLoader s_libraryLoader = 
            new JassimpLibraryLoader();
   
    /**
     * Status flag if the library is loaded.
     * 
     * Volatile to avoid problems with double checked locking.
     * 
     */
    private static volatile boolean s_libraryLoaded = false;
    
    /**
     * Lock for library loading.
     */
    private static final Object s_libraryLoadingLock = new Object();
    
    /**
     * Pure static class, no accessible constructor.
     */
    private Jassimp() {
        /* nothing to do */
    }
    
    public static int NATIVE_AIVEKTORKEY_SIZE; 
    public static int NATIVE_AIQUATKEY_SIZE; 
    public static int NATIVE_AIVEKTOR3D_SIZE; 
    public static int NATIVE_FLOAT_SIZE; 
    public static int NATIVE_INT_SIZE; 
    public static int NATIVE_UINT_SIZE; 
    public static int NATIVE_DOUBLE_SIZE; 
    public static int NATIVE_LONG_SIZE; 

}
