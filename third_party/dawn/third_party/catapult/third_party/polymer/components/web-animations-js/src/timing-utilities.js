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

(function(shared, testing) {

  var fills = 'backwards|forwards|both|none'.split('|');
  var directions = 'reverse|alternate|alternate-reverse'.split('|');
  var linear = function(x) { return x; };

  function cloneTimingInput(timingInput) {
    if (typeof timingInput == 'number') {
      return timingInput;
    }
    var clone = {};
    for (var m in timingInput) {
      clone[m] = timingInput[m];
    }
    return clone;
  }

  function AnimationEffectTiming() {
    this._delay = 0;
    this._endDelay = 0;
    this._fill = 'none';
    this._iterationStart = 0;
    this._iterations = 1;
    this._duration = 0;
    this._playbackRate = 1;
    this._direction = 'normal';
    this._easing = 'linear';
    this._easingFunction = linear;
  }

  function isInvalidTimingDeprecated() {
    return shared.isDeprecated('Invalid timing inputs', '2016-03-02', 'TypeError exceptions will be thrown instead.', true);
  }

  AnimationEffectTiming.prototype = {
    _setMember: function(member, value) {
      this['_' + member] = value;
      if (this._effect) {
        this._effect._timingInput[member] = value;
        this._effect._timing = shared.normalizeTimingInput(this._effect._timingInput);
        this._effect.activeDuration = shared.calculateActiveDuration(this._effect._timing);
        if (this._effect._animation) {
          this._effect._animation._rebuildUnderlyingAnimation();
        }
      }
    },
    get playbackRate() {
      return this._playbackRate;
    },
    set delay(value) {
      this._setMember('delay', value);
    },
    get delay() {
      return this._delay;
    },
    set endDelay(value) {
      this._setMember('endDelay', value);
    },
    get endDelay() {
      return this._endDelay;
    },
    set fill(value) {
      this._setMember('fill', value);
    },
    get fill() {
      return this._fill;
    },
    set iterationStart(value) {
      if ((isNaN(value) || value < 0) && isInvalidTimingDeprecated()) {
        throw new TypeError('iterationStart must be a non-negative number, received: ' + timing.iterationStart);
      }
      this._setMember('iterationStart', value);
    },
    get iterationStart() {
      return this._iterationStart;
    },
    set duration(value) {
      if (value != 'auto' && (isNaN(value) || value < 0) && isInvalidTimingDeprecated()) {
        throw new TypeError('duration must be non-negative or auto, received: ' + value);
      }
      this._setMember('duration', value);
    },
    get duration() {
      return this._duration;
    },
    set direction(value) {
      this._setMember('direction', value);
    },
    get direction() {
      return this._direction;
    },
    set easing(value) {
      this._easingFunction = parseEasingFunction(normalizeEasing(value));
      this._setMember('easing', value);
    },
    get easing() {
      return this._easing;
    },
    set iterations(value) {
      if ((isNaN(value) || value < 0) && isInvalidTimingDeprecated()) {
        throw new TypeError('iterations must be non-negative, received: ' + value);
      }
      this._setMember('iterations', value);
    },
    get iterations() {
      return this._iterations;
    }
  };

  function makeTiming(timingInput, forGroup, effect) {
    var timing = new AnimationEffectTiming();
    if (forGroup) {
      timing.fill = 'both';
      timing.duration = 'auto';
    }
    if (typeof timingInput == 'number' && !isNaN(timingInput)) {
      timing.duration = timingInput;
    } else if (timingInput !== undefined) {
      Object.getOwnPropertyNames(timingInput).forEach(function(property) {
        if (timingInput[property] != 'auto') {
          if (typeof timing[property] == 'number' || property == 'duration') {
            if (typeof timingInput[property] != 'number' || isNaN(timingInput[property])) {
              return;
            }
          }
          if ((property == 'fill') && (fills.indexOf(timingInput[property]) == -1)) {
            return;
          }
          if ((property == 'direction') && (directions.indexOf(timingInput[property]) == -1)) {
            return;
          }
          if (property == 'playbackRate' && timingInput[property] !== 1 && shared.isDeprecated('AnimationEffectTiming.playbackRate', '2014-11-28', 'Use Animation.playbackRate instead.')) {
            return;
          }
          timing[property] = timingInput[property];
        }
      });
    }
    return timing;
  }

  function numericTimingToObject(timingInput) {
    if (typeof timingInput == 'number') {
      if (isNaN(timingInput)) {
        timingInput = { duration: 0 };
      } else {
        timingInput = { duration: timingInput };
      }
    }
    return timingInput;
  }

  function normalizeTimingInput(timingInput, forGroup) {
    timingInput = shared.numericTimingToObject(timingInput);
    return makeTiming(timingInput, forGroup);
  }

  function cubic(a, b, c, d) {
    if (a < 0 || a > 1 || c < 0 || c > 1) {
      return linear;
    }
    return function(x) {
      if (x <= 0) {
        var start_gradient = 0;
        if (a > 0)
          start_gradient = b / a;
        else if (!b && c > 0)
          start_gradient = d / c;
        return start_gradient * x;
      }
      if (x >= 1) {
        var end_gradient = 0;
        if (c < 1)
          end_gradient = (d - 1) / (c - 1);
        else if (c == 1 && a < 1)
          end_gradient = (b - 1) / (a - 1);
        return 1 + end_gradient * (x - 1);
      }

      var start = 0, end = 1;
      while (start < end) {
        var mid = (start + end) / 2;
        function f(a, b, m) { return 3 * a * (1 - m) * (1 - m) * m + 3 * b * (1 - m) * m * m + m * m * m};
        var xEst = f(a, c, mid);
        if (Math.abs(x - xEst) < 0.00001) {
          return f(b, d, mid);
        }
        if (xEst < x) {
          start = mid;
        } else {
          end = mid;
        }
      }
      return f(b, d, mid);
    }
  }

  var Start = 1;
  var Middle = 0.5;
  var End = 0;

  function step(count, pos) {
    return function(x) {
      if (x >= 1) {
        return 1;
      }
      var stepSize = 1 / count;
      x += pos * stepSize;
      return x - x % stepSize;
    }
  }

  var presets = {
    'ease': cubic(0.25, 0.1, 0.25, 1),
    'ease-in': cubic(0.42, 0, 1, 1),
    'ease-out': cubic(0, 0, 0.58, 1),
    'ease-in-out': cubic(0.42, 0, 0.58, 1),
    'step-start': step(1, Start),
    'step-middle': step(1, Middle),
    'step-end': step(1, End)
  };

  var styleForCleaning = null;
  var numberString = '\\s*(-?\\d+\\.?\\d*|-?\\.\\d+)\\s*';
  var cubicBezierRe = new RegExp('cubic-bezier\\(' + numberString + ',' + numberString + ',' + numberString + ',' + numberString + '\\)');
  var stepRe = /steps\(\s*(\d+)\s*,\s*(start|middle|end)\s*\)/;

  function normalizeEasing(easing) {
    if (!styleForCleaning) {
      styleForCleaning = document.createElement('div').style;
    }
    styleForCleaning.animationTimingFunction = '';
    styleForCleaning.animationTimingFunction = easing;
    var normalizedEasing = styleForCleaning.animationTimingFunction;
    if (normalizedEasing == '' && isInvalidTimingDeprecated()) {
      throw new TypeError(easing + ' is not a valid value for easing');
    }
    return normalizedEasing;
  }

  function parseEasingFunction(normalizedEasing) {
    if (normalizedEasing == 'linear') {
      return linear;
    }
    var cubicData = cubicBezierRe.exec(normalizedEasing);
    if (cubicData) {
      return cubic.apply(this, cubicData.slice(1).map(Number));
    }
    var stepData = stepRe.exec(normalizedEasing);
    if (stepData) {
      return step(Number(stepData[1]), {'start': Start, 'middle': Middle, 'end': End}[stepData[2]]);
    }
    var preset = presets[normalizedEasing];
    if (preset) {
      return preset;
    }
    // At this point none of our parse attempts succeeded; the easing is invalid.
    // Fall back to linear in the interest of not crashing the page.
    return linear;
  }

  function calculateActiveDuration(timing) {
    return Math.abs(repeatedDuration(timing) / timing.playbackRate);
  }

  function repeatedDuration(timing) {
    // https://w3c.github.io/web-animations/#calculating-the-active-duration
    if (timing.duration === 0 || timing.iterations === 0) {
      return 0;
    }
    return timing.duration * timing.iterations;
  }

  var PhaseNone = 0;
  var PhaseBefore = 1;
  var PhaseAfter = 2;
  var PhaseActive = 3;

  function calculatePhase(activeDuration, localTime, timing) {
    // https://w3c.github.io/web-animations/#animation-effect-phases-and-states
    if (localTime == null) {
      return PhaseNone;
    }

    var endTime = timing.delay + activeDuration + timing.endDelay;
    if (localTime < Math.min(timing.delay, endTime)) {
      return PhaseBefore;
    }
    if (localTime >= Math.min(timing.delay + activeDuration, endTime)) {
      return PhaseAfter;
    }

    return PhaseActive;
  }

  function calculateActiveTime(activeDuration, fillMode, localTime, phase, delay) {
    // https://w3c.github.io/web-animations/#calculating-the-active-time
    switch (phase) {
      case PhaseBefore:
        if (fillMode == 'backwards' || fillMode == 'both')
          return 0;
        return null;
      case PhaseActive:
        return localTime - delay;
      case PhaseAfter:
        if (fillMode == 'forwards' || fillMode == 'both')
          return activeDuration;
        return null;
      case PhaseNone:
        return null;
    }
  }

  function calculateOverallProgress(iterationDuration, phase, iterations, activeTime, iterationStart) {
    // https://w3c.github.io/web-animations/#calculating-the-overall-progress
    var overallProgress = iterationStart;
    if (iterationDuration === 0) {
      if (phase !== PhaseBefore) {
        overallProgress += iterations;
      }
    } else {
      overallProgress += activeTime / iterationDuration;
    }
    return overallProgress;
  }

  function calculateSimpleIterationProgress(overallProgress, iterationStart, phase, iterations, activeTime, iterationDuration) {
    // https://w3c.github.io/web-animations/#calculating-the-simple-iteration-progress

    var simpleIterationProgress = (overallProgress === Infinity) ? iterationStart % 1 : overallProgress % 1;
    if (simpleIterationProgress === 0 && phase === PhaseAfter && iterations !== 0 &&
        (activeTime !== 0 || iterationDuration === 0)) {
      simpleIterationProgress = 1;
    }
    return simpleIterationProgress;
  }

  function calculateCurrentIteration(phase, iterations, simpleIterationProgress, overallProgress) {
    // https://w3c.github.io/web-animations/#calculating-the-current-iteration
    if (phase === PhaseAfter && iterations === Infinity) {
      return Infinity;
    }
    if (simpleIterationProgress === 1) {
      return Math.floor(overallProgress) - 1;
    }
    return Math.floor(overallProgress);
  }

  function calculateDirectedProgress(playbackDirection, currentIteration, simpleIterationProgress) {
    // https://w3c.github.io/web-animations/#calculating-the-directed-progress
    var currentDirection = playbackDirection;
    if (playbackDirection !== 'normal' && playbackDirection !== 'reverse') {
      var d = currentIteration;
      if (playbackDirection === 'alternate-reverse') {
        d += 1;
      }
      currentDirection = 'normal';
      if (d !== Infinity && d % 2 !== 0) {
        currentDirection = 'reverse';
      }
    }
    if (currentDirection === 'normal') {
      return simpleIterationProgress;
    }
    return 1 - simpleIterationProgress;
  }

  function calculateIterationProgress(activeDuration, localTime, timing) {
    var phase = calculatePhase(activeDuration, localTime, timing);
    var activeTime = calculateActiveTime(activeDuration, timing.fill, localTime, phase, timing.delay);
    if (activeTime === null)
      return null;

    var overallProgress = calculateOverallProgress(timing.duration, phase, timing.iterations, activeTime, timing.iterationStart);
    var simpleIterationProgress = calculateSimpleIterationProgress(overallProgress, timing.iterationStart, phase, timing.iterations, activeTime, timing.duration);
    var currentIteration = calculateCurrentIteration(phase, timing.iterations, simpleIterationProgress, overallProgress);
    var directedProgress = calculateDirectedProgress(timing.direction, currentIteration, simpleIterationProgress);

    // https://w3c.github.io/web-animations/#calculating-the-transformed-progress
    // https://w3c.github.io/web-animations/#calculating-the-iteration-progress
    return timing._easingFunction(directedProgress);
  }

  shared.cloneTimingInput = cloneTimingInput;
  shared.makeTiming = makeTiming;
  shared.numericTimingToObject = numericTimingToObject;
  shared.normalizeTimingInput = normalizeTimingInput;
  shared.calculateActiveDuration = calculateActiveDuration;
  shared.calculateIterationProgress = calculateIterationProgress;
  shared.calculatePhase = calculatePhase;
  shared.normalizeEasing = normalizeEasing;
  shared.parseEasingFunction = parseEasingFunction;

  if (WEB_ANIMATIONS_TESTING) {
    testing.normalizeTimingInput = normalizeTimingInput;
    testing.normalizeEasing = normalizeEasing;
    testing.parseEasingFunction = parseEasingFunction;
    testing.calculateActiveDuration = calculateActiveDuration;
    testing.calculatePhase = calculatePhase;
    testing.PhaseNone = PhaseNone;
    testing.PhaseBefore = PhaseBefore;
    testing.PhaseActive = PhaseActive;
    testing.PhaseAfter = PhaseAfter;
  }

})(webAnimationsShared, webAnimationsTesting);
