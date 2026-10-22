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
 * The data structures which are used to store the imported animation data.
 */
module assimp.animation;

import assimp.math;
import assimp.types;

extern ( C ) {
   /**
    * A time-value pair specifying a certain 3D vector for the given time.
    */
   struct aiVectorKey {
      /**
       * The time of this key.
       */
      double mTime;

      /**
       * The value of this key.
       */
      aiVector3D mValue;
   }

   /**
    * A time-value pair specifying a rotation for the given time. For joint
    * animations, the rotation is usually expressed using a quaternion.
    */
   struct aiQuatKey {
      /**
       * The time of this key.
       */
      double mTime;

      /**
       * The value of this key.
       */
      aiQuaternion mValue;
   }

   /**
    * Defines how an animation channel behaves outside the defined time
    * range. This corresponds to <code>aiNodeAnim.mPreState</code> and
    * <code>aiNodeAnim.mPostState</code>.
    */
   enum aiAnimBehaviour : uint {
      /**
       * The value from the default node transformation is used.
       */
      DEFAULT  = 0x0,

      /**
       * The nearest key value is used without interpolation.
       */
      CONSTANT = 0x1,

      /**
       * The value of the nearest two keys is linearly extrapolated for the
       * current time value.
       */
      LINEAR = 0x2,

      /**
       * The animation is repeated.
       *
       * If the animation key go from n to m and the current time is t, use the
       * value at (t-n) % (|m-n|).
       */
      REPEAT = 0x3
   }

   /**
    * Describes the animation of a single node.
    *
    * The name specifies the bone/node which is affected by this animation
    * channel. The keyframes are given in three separate series of values, one
    * each for position, rotation and scaling. The transformation matrix
    * computed from these values replaces the node's original transformation
    * matrix at a specific time. This means all keys are absolute and not
    * relative to the bone default pose.
    *
    * The order in which the transformations are applied is –
    * as usual – scaling, rotation, translation.
    *
    * Note: All keys are returned in their correct, chronological order.
    *    Duplicate keys don't pass the validation step. Most likely there will
    *    be no negative time values, but they are not forbidden (so
    *    implementations need to cope with them!).
    */
   struct aiNodeAnim {
      /**
       * The name of the node affected by this animation. The node must exist
       * and it must be unique.
       */
      aiString mNodeName;

      /**
       * The number of position keys.
       */
      uint mNumPositionKeys;

      /**
       * The position keys of this animation channel. Positions are specified
       * as 3D vectors. The array is <code>mNumPositionKeys</code> in size.
       *
       * If there are position keys, there will also be at least one scaling
       * and one rotation key.
       */
      aiVectorKey* mPositionKeys;

      /**
       * The number of rotation keys.
       */
      uint mNumRotationKeys;

      /**
       * The rotation keys of this animation channel. Rotations are given as
       * quaternions. The array is <code>mNumRotationKeys</code> in size.
       *
       * If there are rotation keys, there will also be at least one scaling
       * and one position key.
       */
      aiQuatKey* mRotationKeys;


      /**
       * The number of scaling keys.
       */
      uint mNumScalingKeys;

      /**
       * The scaling keys of this animation channel. Scalings are specified as
       * 3D vectors. The array is <code>mNumScalingKeys</code> in size.
       *
       * If there are scaling keys, there will also be at least one position
       * and one rotation key.
       */
      aiVectorKey* mScalingKeys;


      /**
       * Defines how the animation behaves before the first key is encountered.
       *
       * The default value is <code>aiAnimBehaviour.DEFAULT</code> (the original
       * transformation matrix of the affected node is used).
       */
      aiAnimBehaviour mPreState;

      /**
       * Defines how the animation behaves after the last key was processed.
       *
       * The default value is <code>aiAnimBehaviour.DEFAULT</code> (the original
       * transformation matrix of the affected node is used).
       */
      aiAnimBehaviour mPostState;
   }

   /**
    * An animation consists of keyframe data for a number of nodes.
    *
    * For each node affected by the animation, a separate series of data is
    * given.
    */
   struct aiAnimation {
      /**
       * The name of the animation.
       *
       * If the modeling package this data was
       * exported from does support only a single animation channel, this
       * name is usually empty (length is zero).
       */
      aiString mName;

      /**
       * Duration of the animation in ticks.
       */
      double mDuration;

      /**
       * Ticks per second. 0 if not specified in the imported file.
       */
      double mTicksPerSecond;

      /**
       * The number of bone animation channels.
       *
       * Each channel affects a single node.
       */
      uint mNumChannels;

      /**
       * The node animation channels. The array is <code>mNumChannels</code>
       * in size.
       *
       * Each channel affects a single node.
       */
      aiNodeAnim** mChannels;
   }
}