package jassimp;
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
public class AiMetadataEntry
{
   public enum AiMetadataType
   {
      AI_BOOL, AI_INT32, AI_UINT64, AI_FLOAT, AI_DOUBLE, AI_AISTRING, AI_AIVECTOR3D
   }

   private AiMetadataType mType;
   private Object mData;

   public AiMetadataType getMetaDataType()
   {
      return mType;
   }

   public Object getData()
   {
      return mData;
   }

   public static boolean getAiBoolAsBoolean(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_BOOL);

      return (boolean) metadataEntry.mData;
   }

   public static int getAiInt32AsInteger(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_INT32);

      return (int) metadataEntry.mData;
   }

   public static long getAiUint64AsLong(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_UINT64);

      return (long) metadataEntry.mData;
   }

   public static float getAiFloatAsFloat(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_FLOAT);

      return (float) metadataEntry.mData;
   }

   public static double getAiDoubleAsDouble(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_DOUBLE);

      return (double) metadataEntry.mData;
   }

   public static String getAiStringAsString(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_AISTRING);

      return (String) metadataEntry.mData;
   }

   public static AiVector getAiAiVector3DAsAiVector(AiMetadataEntry metadataEntry)
   {
      checkTypeBeforeCasting(metadataEntry, AiMetadataType.AI_AIVECTOR3D);

      return (AiVector) metadataEntry.mData;
   }

   private static void checkTypeBeforeCasting(AiMetadataEntry entry, AiMetadataType expectedType)
   {
      if(entry.mType != expectedType)
      {
         throw new RuntimeException("Cannot cast entry of type " + entry.mType.name() + " to " + expectedType.name());
      }
   }
}
