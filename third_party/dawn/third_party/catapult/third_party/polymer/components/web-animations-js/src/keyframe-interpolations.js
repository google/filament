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

  scope.convertEffectInput = function(effectInput) {
    var keyframes = shared.normalizeKeyframes(effectInput);
    var propertySpecificKeyframeGroups = makePropertySpecificKeyframeGroups(keyframes);
    var interpolations = makeInterpolations(propertySpecificKeyframeGroups);
    return function(target, fraction) {
      if (fraction != null) {
        interpolations.filter(function(interpolation) {
          return fraction >= interpolation.applyFrom && fraction < interpolation.applyTo;
        }).forEach(function(interpolation) {
          var offsetFraction = fraction - interpolation.startOffset;
          var localDuration = interpolation.endOffset - interpolation.startOffset;
          var scaledLocalTime = localDuration == 0 ? 0 : interpolation.easingFunction(offsetFraction / localDuration);
          scope.apply(target, interpolation.property, interpolation.interpolation(scaledLocalTime));
        });
      } else {
        for (var property in propertySpecificKeyframeGroups)
          if (property != 'offset' && property != 'easing' && property != 'composite')
            scope.clear(target, property);
      }
    };
  };


  function makePropertySpecificKeyframeGroups(keyframes) {
    var propertySpecificKeyframeGroups = {};

    for (var i = 0; i < keyframes.length; i++) {
      for (var member in keyframes[i]) {
        if (member != 'offset' && member != 'easing' && member != 'composite') {
          var propertySpecificKeyframe = {
            offset: keyframes[i].offset,
            easing: keyframes[i].easing,
            value: keyframes[i][member]
          };
          propertySpecificKeyframeGroups[member] = propertySpecificKeyframeGroups[member] || [];
          propertySpecificKeyframeGroups[member].push(propertySpecificKeyframe);
        }
      }
    }

    for (var groupName in propertySpecificKeyframeGroups) {
      var group = propertySpecificKeyframeGroups[groupName];
      if (group[0].offset != 0 || group[group.length - 1].offset != 1) {
        throw {
          type: DOMException.NOT_SUPPORTED_ERR,
          name: 'NotSupportedError',
          message: 'Partial keyframes are not supported'
        };
      }
    }
    return propertySpecificKeyframeGroups;
  }


  function makeInterpolations(propertySpecificKeyframeGroups) {
    var interpolations = [];
    for (var groupName in propertySpecificKeyframeGroups) {
      var keyframes = propertySpecificKeyframeGroups[groupName];
      for (var i = 0; i < keyframes.length - 1; i++) {
        var startIndex = i;
        var endIndex = i + 1;
        var startOffset = keyframes[startIndex].offset;
        var endOffset = keyframes[endIndex].offset;
        var applyFrom = startOffset;
        var applyTo = endOffset;

        if (i == 0) {
          applyFrom = -Infinity;
          WEB_ANIMATIONS_TESTING && console.assert(startOffset == 0);
          if (endOffset == 0) {
            endIndex = startIndex;
          }
        }
        if (i == keyframes.length - 2) {
          applyTo = Infinity;
          WEB_ANIMATIONS_TESTING && console.assert(endOffset == 1);
          if (startOffset == 1) {
            startIndex = endIndex;
          }
        }

        interpolations.push({
          applyFrom: applyFrom,
          applyTo: applyTo,
          startOffset: keyframes[startIndex].offset,
          endOffset: keyframes[endIndex].offset,
          easingFunction: shared.parseEasingFunction(keyframes[startIndex].easing),
          property: groupName,
          interpolation: scope.propertyInterpolation(groupName,
              keyframes[startIndex].value,
              keyframes[endIndex].value)
        });
      }
    }
    interpolations.sort(function(leftInterpolation, rightInterpolation) {
      return leftInterpolation.startOffset - rightInterpolation.startOffset;
    });
    return interpolations;
  }


  if (WEB_ANIMATIONS_TESTING) {
    testing.makePropertySpecificKeyframeGroups = makePropertySpecificKeyframeGroups;
    testing.makeInterpolations = makeInterpolations;
  }

})(webAnimationsShared, webAnimations1, webAnimationsTesting);
