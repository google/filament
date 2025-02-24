export type TestConfig = {
  /**
   * Enable debug-level logs (normally logged via `Fixture.debug()`).
   */
  enableDebugLogs: boolean;

  /**
   * Maximum number of subcases in flight at once, within a case. Once this many
   * are in flight, wait for a subcase to finish before starting the next one.
   */
  maxSubcasesInFlight: number;

  /**
   * Every `subcasesBetweenAttemptingGC` subcases, run `attemptGarbageCollection()`.
   * Setting to `Infinity` disables this. Setting to 1 attempts GC every time (slow!).
   */
  subcasesBetweenAttemptingGC: number;

  testHeartbeatCallback: () => void;

  noRaceWithRejectOnTimeout: boolean;

  /**
   * Logger for debug messages from the test framework
   * (that can't be captured in the logs of a test).
   */
  frameworkDebugLog?: (msg: string) => void;

  /**
   * Controls the emission of loops in constant-evaluation shaders under
   * 'webgpu:shader,execution,expression,*'
   * FXC is extremely slow to compile shaders with loops unrolled, where as the
   * MSL compiler is extremely slow to compile with loops rolled.
   */
  unrollConstEvalLoops: boolean;

  /**
   * Whether or not we're running in compatibility mode.
   */
  compatibility: boolean;

  /**
   * Whether or not to request a fallback adapter.
   */
  forceFallbackAdapter: boolean;

  /**
   * Enforce the default limits on the adapter
   */
  enforceDefaultLimits: boolean;

  /**
   * Whether to enable the `logToWebSocket` function used for out-of-band test logging.
   */
  logToWebSocket: boolean;
};

export const globalTestConfig: TestConfig = {
  enableDebugLogs: false,
  maxSubcasesInFlight: 100,
  subcasesBetweenAttemptingGC: 5000,
  testHeartbeatCallback: () => {},
  noRaceWithRejectOnTimeout: false,
  unrollConstEvalLoops: false,
  compatibility: false,
  forceFallbackAdapter: false,
  enforceDefaultLimits: false,
  logToWebSocket: false,
};
