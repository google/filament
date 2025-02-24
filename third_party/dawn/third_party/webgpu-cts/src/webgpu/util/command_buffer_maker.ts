import { ResourceState, GPUTestBase } from '../gpu_test.js';

export const kRenderEncodeTypes = ['render pass', 'render bundle'] as const;
export type RenderEncodeType = (typeof kRenderEncodeTypes)[number];
export const kProgrammableEncoderTypes = ['compute pass', ...kRenderEncodeTypes] as const;
export type ProgrammableEncoderType = (typeof kProgrammableEncoderTypes)[number];
export const kEncoderTypes = ['non-pass', ...kProgrammableEncoderTypes] as const;
export type EncoderType = (typeof kEncoderTypes)[number];

// Look up the type of the encoder based on `T`. If `T` is a union, this will be too!
type EncoderByEncoderType<T extends EncoderType> = {
  'non-pass': GPUCommandEncoder;
  'compute pass': GPUComputePassEncoder;
  'render pass': GPURenderPassEncoder;
  'render bundle': GPURenderBundleEncoder;
}[T];

/** See {@link webgpu/api/validation/validation_test.ValidationTest.createEncoder |
 * GPUTest.createEncoder()}. */
export class CommandBufferMaker<T extends EncoderType> {
  /** `GPU___Encoder` for recording commands into. */
  // Look up the type of the encoder based on `T`. If `T` is a union, this will be too!
  readonly encoder: EncoderByEncoderType<T>;

  /**
   * Finish any passes, finish and record any bundles, and finish/return the command buffer. Any
   * errors are ignored and the GPUCommandBuffer (which may be an error buffer) is returned.
   */
  readonly finish: () => GPUCommandBuffer;

  /**
   * Finish any passes, finish and record any bundles, and finish/return the command buffer.
   * Checks for validation errors in (only) the appropriate finish call.
   */
  readonly validateFinish: (shouldSucceed: boolean) => GPUCommandBuffer;

  /**
   * Finish the command buffer and submit it. Checks for validation errors in either the submit or
   * the appropriate finish call, depending on the state of a resource used in the encoding.
   */
  readonly validateFinishAndSubmit: (
    shouldBeValid: boolean,
    submitShouldSucceedIfValid: boolean
  ) => void;

  /**
   * `validateFinishAndSubmit()` based on the state of a resource in the command encoder.
   * - `finish()` should fail if the resource is 'invalid'.
   * - Only `submit()` should fail if the resource is 'destroyed'.
   */
  readonly validateFinishAndSubmitGivenState: (resourceState: ResourceState) => void;

  constructor(
    t: GPUTestBase,
    encoder: EncoderByEncoderType<EncoderType>,
    finish: () => GPUCommandBuffer
  ) {
    // TypeScript introduces an intersection type here where we don't want one.
    this.encoder = encoder as EncoderByEncoderType<T>;
    this.finish = finish;

    // Define extra methods like this, otherwise they get unbound when destructured, e.g.:
    //   const { encoder, validateFinishAndSubmit } = t.createEncoder(type);
    // Alternatively, do not destructure, and call member functions, e.g.:
    //   const encoder = t.createEncoder(type);
    //   encoder.validateFinish(true);
    this.validateFinish = (shouldSucceed: boolean) => {
      return t.expectGPUError('validation', this.finish, !shouldSucceed);
    };

    this.validateFinishAndSubmit = (
      shouldBeValid: boolean,
      submitShouldSucceedIfValid: boolean
    ) => {
      const commandBuffer = this.validateFinish(shouldBeValid);
      if (shouldBeValid) {
        t.expectValidationError(() => t.queue.submit([commandBuffer]), !submitShouldSucceedIfValid);
      }
    };

    this.validateFinishAndSubmitGivenState = (resourceState: ResourceState) => {
      this.validateFinishAndSubmit(resourceState !== 'invalid', resourceState !== 'destroyed');
    };
  }
}
