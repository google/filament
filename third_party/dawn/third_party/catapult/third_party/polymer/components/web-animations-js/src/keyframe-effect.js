// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//     You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//     See the License for the specific language governing permissions and
// limitations under the License.

(function(shared, scope, testing) {

  function EffectTime(timing) {
    var timeFraction = 0;
    var activeDuration = shared.calculateActiveDuration(timing);
    var effectTime = function(localTime) {
      return shared.calculateIterationProgress(activeDuration, localTime, timing);
    };
    effectTime._totalDuration = timing.delay + activeDuration + timing.endDelay;
    return effectTime;
  }

  scope.KeyframeEffect = function(target, effectInput, timingInput, id) {
    var effectTime = EffectTime(shared.normalizeTimingInput(timingInput));
    var interpolations = scope.convertEffectInput(effectInput);
    var timeFraction;
    var keyframeEffect = function() {
      WEB_ANIMATIONS_TESTING && console.assert(typeof timeFraction !== 'undefined');
      interpolations(target, timeFraction);
    };
    // Returns whether the keyframeEffect is in effect or not after the timing update.
    keyframeEffect._update = function(localTime) {
      timeFraction = effectTime(localTime);
      return timeFraction !== null;
    };
    keyframeEffect._clear = function() {
      interpolations(target, null);
    };
    keyframeEffect._hasSameTarget = function(otherTarget) {
      return target === otherTarget;
    };
    keyframeEffect._target = target;
    keyframeEffect._totalDuration = effectTime._totalDuration;
    keyframeEffect._id = id;
    return keyframeEffect;
  };

  if (WEB_ANIMATIONS_TESTING) {
    testing.webAnimations1KeyframeEffect = scope.KeyframeEffect;
    testing.effectTime = EffectTime;
  }

})(webAnimationsShared, webAnimations1, webAnimationsTesting);
