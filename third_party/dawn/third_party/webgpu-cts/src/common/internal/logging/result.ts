import { LogMessageWithStack } from './log_message.js';

// MAINTENANCE_TODO: Add warn expectations
export type Expectation = 'pass' | 'skip' | 'fail';

export type Status = 'notrun' | 'running' | 'warn' | Expectation;

export interface TestCaseResult {
  status: Status;
  timems: number;
}

export interface LiveTestCaseResult extends TestCaseResult {
  logs?: LogMessageWithStack[];
}

/**
 * Raw data for a test log message.
 *
 * This form is sendable over a message channel, except `extra` may get mangled.
 */
export interface LogMessageRawData {
  name: string;
  message: string;
  stackHiddenMessage: string | undefined;
  stack: string | undefined;
  extra: unknown;
}

/**
 * Test case results in a form sendable over a message channel.
 *
 * Note `extra` may get mangled by postMessage.
 */
export interface TransferredTestCaseResult extends TestCaseResult {
  logs?: LogMessageRawData[];
}
