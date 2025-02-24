/* eslint-disable no-process-exit, n/no-process-exit */
/* eslint-disable @typescript-eslint/no-namespace */

function node() {
  /* eslint-disable-next-line n/no-restricted-require */
  const { existsSync } = require('fs');

  return {
    type: 'node',
    existsSync,
    args: process.argv.slice(2),
    cwd: () => process.cwd(),
    exit: (code?: number | undefined) => process.exit(code),
  };
}

declare global {
  namespace Deno {
    function readFileSync(path: string): Uint8Array;
    const args: string[];
    const cwd: () => string;
    function exit(code?: number): never;
  }
}

function deno() {
  function existsSync(path: string) {
    try {
      Deno.readFileSync(path);
      return true;
    } catch (err) {
      return false;
    }
  }

  return {
    type: 'deno',
    existsSync,
    args: Deno.args,
    cwd: Deno.cwd,
    exit: Deno.exit,
  };
}

const sys = typeof globalThis.process !== 'undefined' ? node() : deno();

export default sys;
