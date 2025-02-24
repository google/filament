/**
 * Indicates bounds that acceptance intervals need to be within to avoid inputs
 * being filtered out. This is used for const-eval tests, since going OOB will
 * cause a validation error not an execution error.
 */
export type IntervalFilter =
  | 'finite' // Expected to be finite in the interval numeric space
  | 'unfiltered'; // No expectations
