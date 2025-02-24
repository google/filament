import { ValidationTest } from '../api/validation/validation_test.js';

export class CompatibilityTest extends ValidationTest {
  override async init() {
    await super.init();
  }

  /**
   * Expect a validation error inside the callback.
   * except when not in compat mode.
   *
   * Tests should always do just one WebGPU call in the callback, to make sure that's what's tested.
   */
  expectValidationErrorInCompatibilityMode(fn: () => void, shouldError: boolean = true): void {
    this.expectValidationError(fn, this.isCompatibility && shouldError);
  }

  /**
   * Expect the specified WebGPU error to be generated when running the provided function
   * except when not in compat mode.
   */
  expectGPUErrorInCompatibilityMode<R>(
    filter: GPUErrorFilter,
    fn: () => R,
    shouldError: boolean = true
  ): R {
    return this.expectGPUError(filter, fn, this.isCompatibility && shouldError);
  }
}
