/** Defined by WPT. Like `setTimeout`, but applies a timeout multiplier for slow test systems. */
declare const step_timeout: undefined | typeof setTimeout;

/**
 * Equivalent of `setTimeout`, but redirects to WPT's `step_timeout` when it is defined.
 */
export const timeout = typeof step_timeout !== 'undefined' ? step_timeout : setTimeout;
