// Copyright 2016 Google Inc. All rights reserved.
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

(function(shared) {
  // If the polyfill is being loaded in a context where Element.animate is
  // supported but object-form syntax is not, then creating an animation
  // using the new syntax will either have no effect or will throw an exception.
  // In either case, we want to proceed to load this part of the polyfill.
  //
  // The test animation uses an opacity other than the one the element already
  // has, and doesn't need to change during the animation for the test to work.
  // After the test, the element's opacity will be left how we found it:
  // - If the animation is not created, the test will leave the element's
  //   opacity untouched at originalOpacity.
  // - If the animation is created, it will be cancelled, and leave the
  //   element's opacity at originalOpacity.
  // - If the animation is somehow created and runs without being cancelled,
  //   when it finishes after 1ms, it will cease to have any effect (because
  //   fill is not specified), and opacity will again be left at originalOpacity.
  var element = document.documentElement;
  var animation = null;
  var animated = false;
  try {
    var originalOpacity = getComputedStyle(element).getPropertyValue('opacity');
    var testOpacity = originalOpacity == '0' ? '1' : '0';
    animation = element.animate({'opacity': [testOpacity, testOpacity]},
        {duration: 1});
    animation.currentTime = 0;
    animated = getComputedStyle(element).getPropertyValue('opacity') == testOpacity;
  } catch (error) {
  } finally {
    if (animation)
      animation.cancel();
  }
  if (animated) {
    return;
  }

  var originalElementAnimate = window.Element.prototype.animate;
  window.Element.prototype.animate = function(effectInput, options) {
    if (window.Symbol && Symbol.iterator && Array.prototype.from && effectInput[Symbol.iterator]) {
      // Handle custom iterables in most browsers by converting to an array
      effectInput = Array.from(effectInput);
    }

    if (!Array.isArray(effectInput) && effectInput !== null) {
      effectInput = shared.convertToArrayForm(effectInput);
    }

    return originalElementAnimate.call(this, effectInput, options);
  };
})(webAnimationsShared);
