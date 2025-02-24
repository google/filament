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
 * @fileoverview Basic externs for the Web Animations API (Level 2 / Groups).
 * This is not intended to be exhaustive, and requires the base externs from
 * web-animations.js.
 * @externs
 */


/**
 * @interface
 */
var AnimationEffectReadOnly = function() {};

/** @type {!AnimationEffectTiming} */
AnimationEffectReadOnly.prototype.timing;


/**
 * @param {Element} target
 * @param {!Array<!Object>} frames
 * @param {(number|AnimationEffectTimingProperties)=} opt_options
 * @constructor
 * @implements {AnimationEffectReadOnly}
 */
var KeyframeEffect = function(target, frames, opt_options) {};

/**
 * @return {!Array<!Object>}
 */
KeyframeEffect.prototype.getFrames = function() {};

/** @type {!AnimationEffectTiming} */
KeyframeEffect.prototype.timing;

/** @type {Element} */
KeyframeEffect.prototype.target;

/** @type {?function(number, !KeyframeEffect, !Animation)} */
KeyframeEffect.prototype.onsample;


/**
 * @param {!Array<!AnimationEffectReadOnly>} children
 * @param {AnimationEffectTimingProperties=} opt_timing
 * @constructor
 * @implements {AnimationEffectReadOnly}
 */
var SequenceEffect = function(children, opt_timing) {};

/** @type {!AnimationEffectTiming} */
SequenceEffect.prototype.timing;

/** @type {!Array<!AnimationEffectReadOnly>} */
SequenceEffect.prototype.children;


/**
 * @param {!Array<!AnimationEffectReadOnly>} children
 * @param {AnimationEffectTimingProperties=} opt_timing
 * @constructor
 * @implements {AnimationEffectReadOnly}
 */
var GroupEffect = function(children, opt_timing) {};

/** @type {!AnimationEffectTiming} */
GroupEffect.prototype.timing;

/** @type {!Array<!AnimationEffectReadOnly>} */
GroupEffect.prototype.children;


/**
 * @interface
 */
var AnimationTimeline = function() {};

/** @type {?number} */
AnimationTimeline.prototype.currentTime;

/**
 * @param {!AnimationEffectReadOnly} effect
 * @return {!Animation}
 */
AnimationTimeline.prototype.play = function(effect) {};

/**
 * @interface
 * @extends {AnimationTimeline}
 */
var DocumentTimeline = function() {};

/** @type {AnimationEffectReadOnly|undefined} */
Animation.prototype.effect;

/** @type {!DocumentTimeline} */
Document.prototype.timeline;
