/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

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

----------------------------------------------------------------------
*/

/** @file  LWOAnimation.h
 *  @brief LWOAnimationResolver utility class
 *
 *  This is for all lightwave-related file format, not only LWO.
 *  LWS isthe main purpose.
*/
#ifndef AI_LWO_ANIMATION_INCLUDED
#define AI_LWO_ANIMATION_INCLUDED

//
#include <vector>
#include <list>

struct aiNodeAnim;
struct aiVectorKey;

namespace Assimp {
namespace LWO {

// ---------------------------------------------------------------------------
/** \brief List of recognized LWO envelopes
 */
enum EnvelopeType
{
    EnvelopeType_Position_X = 0x1,
    EnvelopeType_Position_Y = 0x2,
    EnvelopeType_Position_Z = 0x3,

    EnvelopeType_Rotation_Heading = 0x4,
    EnvelopeType_Rotation_Pitch = 0x5,
    EnvelopeType_Rotation_Bank = 0x6,

    EnvelopeType_Scaling_X = 0x7,
    EnvelopeType_Scaling_Y = 0x8,
    EnvelopeType_Scaling_Z = 0x9,

    // -- currently not yet handled
    EnvelopeType_Color_R = 0xa,
    EnvelopeType_Color_G = 0xb,
    EnvelopeType_Color_B = 0xc,

    EnvelopeType_Falloff_X = 0xd,
    EnvelopeType_Falloff_Y = 0xe,
    EnvelopeType_Falloff_Z = 0xf,

    EnvelopeType_Unknown
};

// ---------------------------------------------------------------------------
/** \brief List of recognized LWO interpolation modes
 */
enum InterpolationType
{
    IT_STEP, IT_LINE, IT_TCB, IT_HERM, IT_BEZI, IT_BEZ2
};


// ---------------------------------------------------------------------------
/** \brief List of recognized LWO pre or post range behaviours
 */
enum PrePostBehaviour
{
    PrePostBehaviour_Reset        = 0x0,
    PrePostBehaviour_Constant     = 0x1,
    PrePostBehaviour_Repeat       = 0x2,
    PrePostBehaviour_Oscillate    = 0x3,
    PrePostBehaviour_OffsetRepeat = 0x4,
    PrePostBehaviour_Linear       = 0x5
};

// ---------------------------------------------------------------------------
/** \brief Data structure for a LWO animation keyframe
 */
struct Key
{
    Key()
        : time(),
        value(),
        inter   (IT_LINE),
        params()
    {}

    //! Current time
    double time;

    //! Current value
    float value;

    //! How to interpolate this key with previous key?
    InterpolationType inter;

    //! Interpolation parameters
    float params[5];


    // for std::find()
    operator double () {
        return time;
    }
};

// ---------------------------------------------------------------------------
/** \brief Data structure for a LWO animation envelope
 */
struct Envelope
{
    Envelope()
        :   index()
        ,   type    (EnvelopeType_Unknown)
        ,   pre     (PrePostBehaviour_Constant)
        ,   post    (PrePostBehaviour_Constant)

        ,   old_first (0)
        ,   old_last  (0)
    {}

    //! Index of this envelope
    unsigned int index;

    //! Type of envelope
    EnvelopeType type;

    //! Pre and post-behaviour
    PrePostBehaviour pre,post;

    //! Keyframes for this envelope
    std::vector<Key> keys;

    // temporary data for AnimResolver
    size_t old_first,old_last;
};

// ---------------------------------------------------------------------------
//! @def AI_LWO_ANIM_FLAG_SAMPLE_ANIMS
//! Flag for AnimResolver, subsamples the input data with the rate specified
//! by AnimResolver::SetSampleRate().
#define AI_LWO_ANIM_FLAG_SAMPLE_ANIMS 0x1


// ---------------------------------------------------------------------------
//! @def AI_LWO_ANIM_FLAG_START_AT_ZERO
//! Flag for AnimResolver, ensures that the animations starts at zero.
#define AI_LWO_ANIM_FLAG_START_AT_ZERO 0x2

// ---------------------------------------------------------------------------
/** @brief Utility class to build Assimp animations from LWO envelopes.
 *
 *  Used for both LWO and LWS (MOT also).
 */
class AnimResolver
{
public:

    // ------------------------------------------------------------------
    /** @brief Construct an AnimResolver from a given list of envelopes
     *  @param envelopes Input envelopes. May be empty.
     *  @param Output tick rate, per second
     *  @note The input envelopes are possibly modified.
     */
    AnimResolver(std::list<Envelope>& envelopes, double tick);

public:

    // ------------------------------------------------------------------
    /** @brief Extract the bind-pose transformation matrix.
     *  @param out Receives bind-pose transformation matrix
     */
    void ExtractBindPose(aiMatrix4x4& out);

    // ------------------------------------------------------------------
    /** @brief Extract a node animation channel
     *  @param out Receives a pointer to a newly allocated node anim.
     *    If there's just one keyframe defined, *out is set to NULL and
     *    no animation channel is computed.
     *  @param flags Any combination of the AI_LWO_ANIM_FLAG_XXX flags.
     */
    void ExtractAnimChannel(aiNodeAnim** out, unsigned int flags = 0);


    // ------------------------------------------------------------------
    /** @brief Set the sampling rate for ExtractAnimChannel().
     *
     *  Non-linear interpolations are subsampled with this rate (keys
     *  per second). Closer sampling positions, if existent, are kept.
     *  The sampling rate defaults to 0, if this value is not changed and
     *  AI_LWO_ANIM_FLAG_SAMPLE_ANIMS is specified for ExtractAnimChannel(),
     *  the class finds a suitable sample rate by itself.
     */
    void SetSampleRate(double sr) {
        sample_rate = sr;
    }

    // ------------------------------------------------------------------
    /** @brief Getter for SetSampleRate()
     */
    double GetSampleRate() const {
        return sample_rate;
    }

    // ------------------------------------------------------------------
    /** @brief Set the animation time range
     *
     *  @param first Time where the animation starts, in ticks
     *  @param last  Time where the animation ends, in ticks
     */
    void SetAnimationRange(double _first, double _last) {
        first = _first;
        last  = _last;

        ClearAnimRangeSetup();
        UpdateAnimRangeSetup();
    }

protected:

    // ------------------------------------------------------------------
    /** @brief Build linearly subsampled keys from 3 single envelopes
     *  @param out Receives output keys
     *  @param envl_x X-component envelope
     *  @param envl_y Y-component envelope
     *  @param envl_z Z-component envelope
     *  @param flags Any combination of the AI_LWO_ANIM_FLAG_XXX flags.
     *  @note Up to two input envelopes may be NULL
     */
    void GetKeys(std::vector<aiVectorKey>& out,
        LWO::Envelope* envl_x,
        LWO::Envelope* envl_y,
        LWO::Envelope* envl_z,
        unsigned int flags);

    // ------------------------------------------------------------------
    /** @brief Resolve a single animation key by applying the right
     *  interpolation to it.
     *  @param cur Current key
     *  @param envl Envelope working on
     *  @param time time to be interpolated
     *  @param fill Receives the interpolated output value.
     */
    void DoInterpolation(std::vector<LWO::Key>::const_iterator cur,
        LWO::Envelope* envl,double time, float& fill);

    // ------------------------------------------------------------------
    /** @brief Almost the same, except we won't handle pre/post
     *  conditions here.
     *  @see DoInterpolation
     */
    void DoInterpolation2(std::vector<LWO::Key>::const_iterator beg,
        std::vector<LWO::Key>::const_iterator end,double time, float& fill);

    // ------------------------------------------------------------------
    /** @brief Interpolate 2 tracks if one is given
     *
     *  @param out Receives extra output keys
     *  @param key_out Primary output key
     *  @param time Time to interpolate for
     */
    void InterpolateTrack(std::vector<aiVectorKey>& out,
        aiVectorKey& key_out,double time);

    // ------------------------------------------------------------------
    /** @brief Subsample an animation track by a given sampling rate
     *
     *  @param out Receives output keys. Last key at input defines the
     *    time where subsampling starts.
     *  @param time Time to end subsampling at
     *  @param sample_delta Time delta between two samples
     */
    void SubsampleAnimTrack(std::vector<aiVectorKey>& out,
        double time,double sample_delta);

    // ------------------------------------------------------------------
    /** @brief Delete all keys which we inserted to match anim setup
     */
    void ClearAnimRangeSetup();

    // ------------------------------------------------------------------
    /** @brief Insert extra keys to match LWO's pre and post behaviours
     *  in a given time range [first...last]
     */
    void UpdateAnimRangeSetup();

private:
    std::list<Envelope>& envelopes;
    double sample_rate;

    LWO::Envelope* trans_x, *trans_y, *trans_z;
    LWO::Envelope* rotat_x, *rotat_y, *rotat_z;
    LWO::Envelope* scale_x, *scale_y, *scale_z;

    double first, last;
    bool need_to_setup;

    // temporary storage
    LWO::Envelope* envl_x, * envl_y, * envl_z;
    std::vector<LWO::Key>::const_iterator cur_x,cur_y,cur_z;
    bool end_x, end_y, end_z;

    unsigned int flags;
    double sample_delta;
};

} // end namespace LWO
} // end namespace Assimp

#endif // !! AI_LWO_ANIMATION_INCLUDED
