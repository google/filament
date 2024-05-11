// Copyright 2017 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
'use_strict';

const assert = require('assert');
const draco = require('./draco-animation');

// Global variables to keep track of which modules have been intiailized.
let encoderModuleInitialized = false;
let decoderModuleInitialized = false;

// Callback function when encoder module has been intialized.
let dracoEncoderType = {};
dracoEncoderType['onModuleLoaded'] = function(module) {
  encoderModuleInitialized = true;
  console.log('Encoder Module Initialized!');
  moduleInitialized();
};

// Callback function when decoder module has been intialized.
let dracoDecoderType = {};
dracoDecoderType['onModuleLoaded'] = function(module) {
  decoderModuleInitialized = true;
  console.log('Decoder Module Initialized!');
  moduleInitialized();
};

// The code to create the encoder and decoder modules is asynchronous.
// draco will call the 'onModuleLoaded' callback when the different
// modules have been fully initialized.
const encoderModule = draco.createEncoderModule(dracoEncoderType);
const decoderModule = draco.createDecoderModule(dracoDecoderType);

function generateAnimationData(numKeyframes, numAnimations, numComponents) {
  const timestamps = new Float32Array(numKeyframes);

  const keyframes = new Array(numAnimations);
  for (let i = 0; i < numAnimations; ++i) {
    keyframes[i] = new Float32Array(numKeyframes * numComponents[i]);
  }

  for (let frameId = 0; frameId < numKeyframes; ++frameId) {
    timestamps[frameId] = frameId;
    for (let keyframeId = 0; keyframeId < numAnimations; ++keyframeId) {
      for (let j = 0; j < numComponents[keyframeId]; ++j) {
        // Set an arbitrary deterministic value.
        keyframes[keyframeId][frameId * numComponents[keyframeId] + j] =
            (frameId * numComponents[keyframeId] + j) * (keyframeId + 1);
      }
    }
  }

  const animation = {
    timestamps : timestamps,
    keyframes : keyframes
  };

  return animation;
}

function encodeAnimation(animation) {
  const encoder = new encoderModule.AnimationEncoder();
  const animationBuilder = new encoderModule.AnimationBuilder();
  const dracoAnimation = new encoderModule.KeyframeAnimation();

  // Add timestamps to animation.
  const numKeyframes = animation.timestamps.length;
  assert(animationBuilder.SetTimestamps(dracoAnimation, numKeyframes,
        animation.timestamps));

  // Add keyframes.
  for (let keyframeId = 0; keyframeId < animation.keyframes.length;
      ++keyframeId) {
    const numComponents =
        animation.keyframes[keyframeId].length / animation.timestamps.length;
    assert(animationBuilder.AddKeyframes(dracoAnimation, numKeyframes,
          numComponents, animation.keyframes[keyframeId]) == keyframeId + 1);
  }

  const encodedData = new encoderModule.DracoInt8Array();
  encoder.SetTimestampsQuantization(16);
  encoder.SetKeyframesQuantization(16);
  console.log("Quantized timestamps to 16 bits.");
  console.log("Quantized keyframes to 16 bits.");
  const encodedLen = encoder.EncodeAnimationToDracoBuffer(dracoAnimation,
                                                          encodedData);

  encoderModule.destroy(dracoAnimation);
  encoderModule.destroy(animationBuilder);
  encoderModule.destroy(encoder);

  assert(encodedLen > 0, "Failed encoding.");
  console.log("Encoded size: " + encodedLen);

  const outputBuffer = new ArrayBuffer(encodedLen);
  const outputData = new Int8Array(outputBuffer);
  for (let i = 0; i < encodedLen; ++i) {
    const data0 = encodedData.GetValue(i);
    outputData[i] = encodedData.GetValue(i);
  }
  encoderModule.destroy(encodedData);

  return outputBuffer;
}

function decodeAnimation(encodedData) {
  const buffer = new decoderModule.DecoderBuffer();
  buffer.Init(new Int8Array(encodedData), encodedData.byteLength);
  const decoder = new decoderModule.AnimationDecoder();

  const dracoAnimation = new decoderModule.KeyframeAnimation();
  decoder.DecodeBufferToKeyframeAnimation(buffer, dracoAnimation);
  assert(dracoAnimation.ptr != 0, "Invalid animation.");


  const numKeyframes = dracoAnimation.num_frames();

  const timestampAttData = new decoderModule.DracoFloat32Array();
  assert(decoder.GetTimestamps(dracoAnimation, timestampAttData));
  const timestamps = new Float32Array(numKeyframes);
  for (let i = 0; i < numKeyframes; ++i) {
      timestamps[i] = timestampAttData.GetValue(i);
  }

  const numAnimations = dracoAnimation.num_animations();
  const keyframes = new Array(numAnimations);
  for (let keyframeId = 0; keyframeId < numAnimations; ++keyframeId) {
    const animationAttData = new decoderModule.DracoFloat32Array();
    // The id of keyframe attribute starts at 1.
    assert(decoder.GetKeyframes(dracoAnimation, keyframeId + 1,
          animationAttData));
    keyframes[keyframeId] = new Float32Array(animationAttData.size());
    for (let i = 0; i < animationAttData.size(); ++i) {
      keyframes[keyframeId][i] = animationAttData.GetValue(i);
    }
  }

  const animation = {
    timestamps : timestamps,
    keyframes : keyframes
  }

  console.log("Decoding successful.");

  return animation;
}

function compareAnimation(animation, decodedAnimation) {
  console.log("Comparing decoded animation data...");
  assert(animation.timestamps.length == decodedAnimation.timestamps.length);
  assert(animation.keyframes.length == decodedAnimation.keyframes.length);

  console.log("Done.");
}

function moduleInitialized() {
  if (encoderModuleInitialized && decoderModuleInitialized) {
    // Create animation with 50 frames and two animations.
    // The first animation has 3 components and the second has 4 components.
    const animation = generateAnimationData(100, 2, [3, 4]);
    const encodedData = encodeAnimation(animation);

    const decodedAnimation = decodeAnimation(encodedData);
    compareAnimation(animation, decodedAnimation);
  }
}
