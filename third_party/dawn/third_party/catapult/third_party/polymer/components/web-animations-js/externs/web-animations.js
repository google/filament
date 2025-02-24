/*
 * Copyright 2016 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */


/**
 * @fileoverview Basic externs for the Web Animations API. This is not
 * nessecarily exhaustive. For more information, see the spec-
 *   https://w3c.github.io/web-animations
 * @externs
 */


/**
 * @param {!Array<!Object>} frames
 * @param {(number|AnimationEffectTimingProperties)=} opt_options
 * @return {!Animation}
 */
Element.prototype.animate = function(frames, opt_options) {};


/**
 * @interface
 * @extends {EventTarget}
 */
var Animation = function() {};

/**
 * @return {undefined}
 */
Animation.prototype.cancel = function() {};

/**
 * @return {undefined}
 */
Animation.prototype.finish = function() {};

/**
 * @return {undefined}
 */
Animation.prototype.reverse = function() {};

/**
 * @return {undefined}
 */
Animation.prototype.pause = function() {};

/**
 * @return {undefined}
 */
Animation.prototype.play = function() {};

/** @type {number} */
Animation.prototype.startTime;

/** @type {number} */
Animation.prototype.currentTime;

/** @type {number} */
Animation.prototype.playbackRate;

/** @type {string} */
Animation.prototype.playState;

/** @type {?function(!Event)} */
Animation.prototype.oncancel;

/** @type {?function(!Event)} */
Animation.prototype.onfinish;


/**
 * @typedef {{
 *   delay: (number|undefined),
 *   endDelay: (number|undefined),
 *   fillMode: (string|undefined),
 *   iterationStart: (number|undefined),
 *   iterations: (number|undefined),
 *   duration: (number|string|undefined),
 *   direction: (string|undefined),
 *   easing: (string|undefined)
 * }}
 */
var AnimationEffectTimingProperties;


/**
 * @interface
 */
var AnimationEffectTiming = function() {};

/** @type {number} */
AnimationEffectTiming.prototype.delay;

/** @type {number} */
AnimationEffectTiming.prototype.endDelay;

/** @type {string} */
AnimationEffectTiming.prototype.fillMode;

/** @type {number} */
AnimationEffectTiming.prototype.iterationStart;

/** @type {number} */
AnimationEffectTiming.prototype.iterations;

/** @type {number|string} */
AnimationEffectTiming.prototype.duration;

/** @type {string} */
AnimationEffectTiming.prototype.direction;

/** @type {string} */
AnimationEffectTiming.prototype.easing;
