/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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

/** @file  LWOAnimation.cpp
 *  @brief LWOAnimationResolver utility class
 *
 *  It's a very generic implementation of LightWave's system of
 *  componentwise-animated stuff. The one and only fully free
 *  implementation of LightWave envelopes of which I know.
*/


#if (!defined ASSIMP_BUILD_NO_LWO_IMPORTER) && (!defined ASSIMP_BUILD_NO_LWS_IMPORTER)

#include <functional>

// internal headers
#include "LWOFileData.h"
#include <assimp/anim.h>

using namespace Assimp;
using namespace Assimp::LWO;

// ------------------------------------------------------------------------------------------------
// Construct an animation resolver from a given list of envelopes
AnimResolver::AnimResolver(std::list<Envelope>& _envelopes,double tick)
    : envelopes   (_envelopes)
    , sample_rate (0.)
    , envl_x(), envl_y(), envl_z()
    , end_x(), end_y(), end_z()
    , flags()
    , sample_delta()
{
    trans_x = trans_y = trans_z = NULL;
    rotat_x = rotat_y = rotat_z = NULL;
    scale_x = scale_y = scale_z = NULL;

    first = last = 150392.;

    // find transformation envelopes
    for (std::list<LWO::Envelope>::iterator it = envelopes.begin(); it != envelopes.end(); ++it) {

        (*it).old_first = 0;
        (*it).old_last  = (*it).keys.size()-1;

        if ((*it).keys.empty()) continue;
        switch ((*it).type) {

            // translation
            case LWO::EnvelopeType_Position_X:
                trans_x = &*it;break;
            case LWO::EnvelopeType_Position_Y:
                trans_y = &*it;break;
            case LWO::EnvelopeType_Position_Z:
                trans_z = &*it;break;

                // rotation
            case LWO::EnvelopeType_Rotation_Heading:
                rotat_x = &*it;break;
            case LWO::EnvelopeType_Rotation_Pitch:
                rotat_y = &*it;break;
            case LWO::EnvelopeType_Rotation_Bank:
                rotat_z = &*it;break;

                // scaling
            case LWO::EnvelopeType_Scaling_X:
                scale_x = &*it;break;
            case LWO::EnvelopeType_Scaling_Y:
                scale_y = &*it;break;
            case LWO::EnvelopeType_Scaling_Z:
                scale_z = &*it;break;
            default:
                continue;
        };

        // convert from seconds to ticks
        for (std::vector<LWO::Key>::iterator d = (*it).keys.begin(); d != (*it).keys.end(); ++d)
            (*d).time *= tick;

        // set default animation range (minimum and maximum time value for which we have a keyframe)
        first = std::min(first, (*it).keys.front().time );
        last  = std::max(last,  (*it).keys.back().time );
    }

    // deferred setup of animation range to increase performance.
    // typically the application will want to specify its own.
    need_to_setup = true;
}

// ------------------------------------------------------------------------------------------------
// Reset all envelopes to their original contents
void AnimResolver::ClearAnimRangeSetup()
{
    for (std::list<LWO::Envelope>::iterator it = envelopes.begin(); it != envelopes.end(); ++it) {

        (*it).keys.erase((*it).keys.begin(),(*it).keys.begin()+(*it).old_first);
        (*it).keys.erase((*it).keys.begin()+(*it).old_last+1,(*it).keys.end());
    }
}

// ------------------------------------------------------------------------------------------------
// Insert additional keys to match LWO's pre& post behaviours.
void AnimResolver::UpdateAnimRangeSetup()
{
    // XXX doesn't work yet (hangs if more than one envelope channels needs to be interpolated)

    for (std::list<LWO::Envelope>::iterator it = envelopes.begin(); it != envelopes.end(); ++it) {
        if ((*it).keys.empty()) continue;

        const double my_first = (*it).keys.front().time;
        const double my_last  = (*it).keys.back().time;

        const double delta = my_last-my_first;
        const size_t old_size = (*it).keys.size();

        const float value_delta = (*it).keys.back().value - (*it).keys.front().value;

        // NOTE: We won't handle reset, linear and constant here.
        // See DoInterpolation() for their implementation.

        // process pre behaviour
        switch ((*it).pre) {
            case LWO::PrePostBehaviour_OffsetRepeat:
            case LWO::PrePostBehaviour_Repeat:
            case LWO::PrePostBehaviour_Oscillate:
                {
                const double start_time = delta - std::fmod(my_first-first,delta);
                std::vector<LWO::Key>::iterator n = std::find_if((*it).keys.begin(),(*it).keys.end(),
                    [start_time](double t) { return start_time > t; }),m;

                size_t ofs = 0;
                if (n != (*it).keys.end()) {
                    // copy from here - don't use iterators, insert() would invalidate them
                    ofs = (*it).keys.end()-n;
                    (*it).keys.insert((*it).keys.begin(),ofs,LWO::Key());

                    std::copy((*it).keys.end()-ofs,(*it).keys.end(),(*it).keys.begin());
                }

                // do full copies. again, no iterators
                const unsigned int num = (unsigned int)((my_first-first) / delta);
                (*it).keys.resize((*it).keys.size() + num*old_size);

                n = (*it).keys.begin()+ofs;
                bool reverse = false;
                for (unsigned int i = 0; i < num; ++i) {
                    m = n+old_size*(i+1);
                    std::copy(n,n+old_size,m);

                    if ((*it).pre == LWO::PrePostBehaviour_Oscillate && (reverse = !reverse))
                        std::reverse(m,m+old_size-1);
                }

                // update time values
                n = (*it).keys.end() - (old_size+1);
                double cur_minus = delta;
                unsigned int tt = 1;
                for (const double tmp =  delta*(num+1);cur_minus <= tmp;cur_minus += delta,++tt) {
                    m = (delta == tmp ? (*it).keys.begin() :  n - (old_size+1));
                    for (;m != n; --n) {
                        (*n).time -= cur_minus;

                        // offset repeat? add delta offset to key value
                        if ((*it).pre == LWO::PrePostBehaviour_OffsetRepeat) {
                            (*n).value += tt * value_delta;
                        }
                    }
                }
                break;
                }
            default:
                // silence compiler warning
                break;
        }

        // process post behaviour
        switch ((*it).post) {

            case LWO::PrePostBehaviour_OffsetRepeat:
            case LWO::PrePostBehaviour_Repeat:
            case LWO::PrePostBehaviour_Oscillate:

                break;

            default:
                // silence compiler warning
                break;
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Extract bind pose matrix
void AnimResolver::ExtractBindPose(aiMatrix4x4& out)
{
    // If we have no envelopes, return identity
    if (envelopes.empty()) {
        out = aiMatrix4x4();
        return;
    }
    aiVector3D angles, scaling(1.f,1.f,1.f), translation;

    if (trans_x) translation.x = trans_x->keys[0].value;
    if (trans_y) translation.y = trans_y->keys[0].value;
    if (trans_z) translation.z = trans_z->keys[0].value;

    if (rotat_x) angles.x = rotat_x->keys[0].value;
    if (rotat_y) angles.y = rotat_y->keys[0].value;
    if (rotat_z) angles.z = rotat_z->keys[0].value;

    if (scale_x) scaling.x = scale_x->keys[0].value;
    if (scale_y) scaling.y = scale_y->keys[0].value;
    if (scale_z) scaling.z = scale_z->keys[0].value;

    // build the final matrix
    aiMatrix4x4 s,rx,ry,rz,t;
    aiMatrix4x4::RotationZ(angles.z, rz);
    aiMatrix4x4::RotationX(angles.y, rx);
    aiMatrix4x4::RotationY(angles.x, ry);
    aiMatrix4x4::Translation(translation,t);
    aiMatrix4x4::Scaling(scaling,s);
    out = t*ry*rx*rz*s;
}

// ------------------------------------------------------------------------------------------------
// Do a single interpolation on a channel
void AnimResolver::DoInterpolation(std::vector<LWO::Key>::const_iterator cur,
    LWO::Envelope* envl,double time, float& fill)
{
    if (envl->keys.size() == 1) {
        fill = envl->keys[0].value;
        return;
    }

    // check whether we're at the beginning of the animation track
    if (cur == envl->keys.begin()) {

        // ok ... this depends on pre behaviour now
        // we don't need to handle repeat&offset repeat&oszillate here, see UpdateAnimRangeSetup()
        switch (envl->pre)
        {
        case LWO::PrePostBehaviour_Linear:
            DoInterpolation2(cur,cur+1,time,fill);
            return;

        case LWO::PrePostBehaviour_Reset:
            fill = 0.f;
            return;

        default : //case LWO::PrePostBehaviour_Constant:
            fill = (*cur).value;
            return;
        }
    }
    // check whether we're at the end of the animation track
    else if (cur == envl->keys.end()-1 && time > envl->keys.rbegin()->time) {
        // ok ... this depends on post behaviour now
        switch (envl->post)
        {
        case LWO::PrePostBehaviour_Linear:
            DoInterpolation2(cur,cur-1,time,fill);
            return;

        case LWO::PrePostBehaviour_Reset:
            fill = 0.f;
            return;

        default : //case LWO::PrePostBehaviour_Constant:
            fill = (*cur).value;
            return;
        }
    }

    // Otherwise do a simple interpolation
    DoInterpolation2(cur-1,cur,time,fill);
}

// ------------------------------------------------------------------------------------------------
// Almost the same, except we won't handle pre/post conditions here
void AnimResolver::DoInterpolation2(std::vector<LWO::Key>::const_iterator beg,
    std::vector<LWO::Key>::const_iterator end,double time, float& fill)
{
    switch ((*end).inter) {

        case LWO::IT_STEP:
            // no interpolation at all - take the value of the last key
            fill = (*beg).value;
            return;
        default:

            // silence compiler warning
            break;
    }
    // linear interpolation - default
    double duration = (*end).time - (*beg).time;
    if (duration > 0.0) {
        fill = (*beg).value + ((*end).value - (*beg).value)*(float)(((time - (*beg).time) / duration));
    } else {
        fill = (*beg).value;
    }
}

// ------------------------------------------------------------------------------------------------
// Subsample animation track by given key values
void AnimResolver::SubsampleAnimTrack(std::vector<aiVectorKey>& /*out*/,
    double /*time*/ ,double /*sample_delta*/ )
{
    //ai_assert(out.empty() && sample_delta);

    //const double time_start = out.back().mTime;
//  for ()
}

// ------------------------------------------------------------------------------------------------
// Track interpolation
void AnimResolver::InterpolateTrack(std::vector<aiVectorKey>& out,aiVectorKey& fill,double time)
{
    // subsample animation track?
    if (flags & AI_LWO_ANIM_FLAG_SAMPLE_ANIMS) {
        SubsampleAnimTrack(out,time, sample_delta);
    }

    fill.mTime = time;

    // get x
    if ((*cur_x).time == time) {
        fill.mValue.x = (*cur_x).value;

        if (cur_x != envl_x->keys.end()-1) /* increment x */
            ++cur_x;
        else end_x = true;
    }
    else DoInterpolation(cur_x,envl_x,time,(float&)fill.mValue.x);

    // get y
    if ((*cur_y).time == time) {
        fill.mValue.y = (*cur_y).value;

        if (cur_y != envl_y->keys.end()-1) /* increment y */
            ++cur_y;
        else end_y = true;
    }
    else DoInterpolation(cur_y,envl_y,time,(float&)fill.mValue.y);

    // get z
    if ((*cur_z).time == time) {
        fill.mValue.z = (*cur_z).value;

        if (cur_z != envl_z->keys.end()-1) /* increment z */
            ++cur_z;
        else end_x = true;
    }
    else DoInterpolation(cur_z,envl_z,time,(float&)fill.mValue.z);
}

// ------------------------------------------------------------------------------------------------
// Build linearly subsampled keys from three single envelopes, one for each component (x,y,z)
void AnimResolver::GetKeys(std::vector<aiVectorKey>& out,
    LWO::Envelope* _envl_x,
    LWO::Envelope* _envl_y,
    LWO::Envelope* _envl_z,
    unsigned int _flags)
{
    envl_x = _envl_x;
    envl_y = _envl_y;
    envl_z = _envl_z;
    flags  = _flags;

    // generate default channels if none are given
    LWO::Envelope def_x, def_y, def_z;
    LWO::Key key_dummy;
    key_dummy.time = 0.f;
    if ((envl_x && envl_x->type == LWO::EnvelopeType_Scaling_X) ||
        (envl_y && envl_y->type == LWO::EnvelopeType_Scaling_Y) ||
        (envl_z && envl_z->type == LWO::EnvelopeType_Scaling_Z)) {
        key_dummy.value = 1.f;
    }
    else key_dummy.value = 0.f;

    if (!envl_x) {
        envl_x = &def_x;
        envl_x->keys.push_back(key_dummy);
    }
    if (!envl_y) {
        envl_y = &def_y;
        envl_y->keys.push_back(key_dummy);
    }
    if (!envl_z) {
        envl_z = &def_z;
        envl_z->keys.push_back(key_dummy);
    }

    // guess how many keys we'll get
    size_t reserve;
    double sr = 1.;
    if (flags & AI_LWO_ANIM_FLAG_SAMPLE_ANIMS) {
        if (!sample_rate)
            sr = 100.f;
        else sr = sample_rate;
        sample_delta = 1.f / sr;

        reserve = (size_t)(
            std::max( envl_x->keys.rbegin()->time,
            std::max( envl_y->keys.rbegin()->time, envl_z->keys.rbegin()->time )) * sr);
    }
    else reserve = std::max(envl_x->keys.size(),std::max(envl_x->keys.size(),envl_z->keys.size()));
    out.reserve(reserve+(reserve>>1));

    // Iterate through all three arrays at once - it's tricky, but
    // rather interesting to implement.
    cur_x = envl_x->keys.begin();
    cur_y = envl_y->keys.begin();
    cur_z = envl_z->keys.begin();

    end_x = end_y = end_z = false;
    while (1) {

        aiVectorKey fill;

        if ((*cur_x).time == (*cur_y).time && (*cur_x).time == (*cur_z).time ) {

            // we have a keyframe for all of them defined .. this means
            // we don't need to interpolate here.
            fill.mTime = (*cur_x).time;

            fill.mValue.x = (*cur_x).value;
            fill.mValue.y = (*cur_y).value;
            fill.mValue.z = (*cur_z).value;

            // subsample animation track
            if (flags & AI_LWO_ANIM_FLAG_SAMPLE_ANIMS) {
                //SubsampleAnimTrack(out,cur_x, cur_y, cur_z, d, sample_delta);
            }
        }

        // Find key with lowest time value
        else if ((*cur_x).time <= (*cur_y).time && !end_x) {

            if ((*cur_z).time <= (*cur_x).time && !end_z) {
                InterpolateTrack(out,fill,(*cur_z).time);
            }
            else {
                InterpolateTrack(out,fill,(*cur_x).time);
            }
        }
        else if ((*cur_z).time <= (*cur_y).time && !end_y)  {
            InterpolateTrack(out,fill,(*cur_y).time);
        }
        else if (!end_y) {
            // welcome on the server, y
            InterpolateTrack(out,fill,(*cur_y).time);
        }
        else {
            // we have reached the end of at least 2 channels,
            // only one is remaining. Extrapolate the 2.
            if (end_y) {
                InterpolateTrack(out,fill,(end_x ? (*cur_z) : (*cur_x)).time);
            }
            else if (end_x) {
                InterpolateTrack(out,fill,(end_z ? (*cur_y) : (*cur_z)).time);
            }
            else { // if (end_z)
                InterpolateTrack(out,fill,(end_y ? (*cur_x) : (*cur_y)).time);
            }
        }
        double lasttime = fill.mTime;
        out.push_back(fill);

        if (lasttime >= (*cur_x).time) {
            if (cur_x != envl_x->keys.end()-1)
                ++cur_x;
            else end_x = true;
        }
        if (lasttime >= (*cur_y).time) {
            if (cur_y != envl_y->keys.end()-1)
                ++cur_y;
            else end_y = true;
        }
        if (lasttime >= (*cur_z).time) {
            if (cur_z != envl_z->keys.end()-1)
                ++cur_z;
            else end_z = true;
        }

        if( end_x && end_y && end_z ) /* finished? */
            break;
    }

    if (flags & AI_LWO_ANIM_FLAG_START_AT_ZERO) {
        for (std::vector<aiVectorKey>::iterator it = out.begin(); it != out.end(); ++it)
            (*it).mTime -= first;
    }
}

// ------------------------------------------------------------------------------------------------
// Extract animation channel
void AnimResolver::ExtractAnimChannel(aiNodeAnim** out, unsigned int flags /*= 0*/)
{
    *out = NULL;


    //FIXME: crashes if more than one component is animated at different timings, to be resolved.

    // If we have no envelopes, return NULL
    if (envelopes.empty()) {
        return;
    }

    // We won't spawn an animation channel if we don't have at least one envelope with more than one keyframe defined.
    const bool trans = ((trans_x && trans_x->keys.size() > 1) || (trans_y && trans_y->keys.size() > 1) || (trans_z && trans_z->keys.size() > 1));
    const bool rotat = ((rotat_x && rotat_x->keys.size() > 1) || (rotat_y && rotat_y->keys.size() > 1) || (rotat_z && rotat_z->keys.size() > 1));
    const bool scale = ((scale_x && scale_x->keys.size() > 1) || (scale_y && scale_y->keys.size() > 1) || (scale_z && scale_z->keys.size() > 1));
    if (!trans && !rotat && !scale)
        return;

    // Allocate the output animation
    aiNodeAnim* anim = *out = new aiNodeAnim();

    // Setup default animation setup if necessary
    if (need_to_setup) {
        UpdateAnimRangeSetup();
        need_to_setup = false;
    }

    // copy translation keys
    if (trans) {
        std::vector<aiVectorKey> keys;
        GetKeys(keys,trans_x,trans_y,trans_z,flags);

        anim->mPositionKeys = new aiVectorKey[ anim->mNumPositionKeys = static_cast<unsigned int>(keys.size()) ];
        std::copy(keys.begin(),keys.end(),anim->mPositionKeys);
    }

    // copy rotation keys
    if (rotat) {
        std::vector<aiVectorKey> keys;
        GetKeys(keys,rotat_x,rotat_y,rotat_z,flags);

        anim->mRotationKeys = new aiQuatKey[ anim->mNumRotationKeys = static_cast<unsigned int>(keys.size()) ];

        // convert heading, pitch, bank to quaternion
        // mValue.x=Heading=Rot(Y), mValue.y=Pitch=Rot(X), mValue.z=Bank=Rot(Z)
        // Lightwave's rotation order is ZXY
        aiVector3D X(1.0,0.0,0.0);
        aiVector3D Y(0.0,1.0,0.0);
        aiVector3D Z(0.0,0.0,1.0);
        for (unsigned int i = 0; i < anim->mNumRotationKeys; ++i) {
            aiQuatKey& qk = anim->mRotationKeys[i];
            qk.mTime  = keys[i].mTime;
            qk.mValue = aiQuaternion(Y,keys[i].mValue.x)*aiQuaternion(X,keys[i].mValue.y)*aiQuaternion(Z,keys[i].mValue.z);
        }
    }

    // copy scaling keys
    if (scale) {
        std::vector<aiVectorKey> keys;
        GetKeys(keys,scale_x,scale_y,scale_z,flags);

        anim->mScalingKeys = new aiVectorKey[ anim->mNumScalingKeys = static_cast<unsigned int>(keys.size()) ];
        std::copy(keys.begin(),keys.end(),anim->mScalingKeys);
    }
}


#endif // no lwo or no lws
