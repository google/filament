import * as fs from 'fs';
import * as path from 'path';
import * as process from 'process';

import { Cacheable, dataCache, setIsBuildingDataCache } from '../framework/data_cache.js';
import { crc32, toHexString } from '../util/crc32.js';
import { parseImports } from '../util/parse_imports.js';

function usage(rc: number): void {
  console.error(`Usage: tools/gen_cache [options] [SUITE_DIRS...]

For each suite in SUITE_DIRS, pre-compute data that is expensive to generate
at runtime and store it under 'src/resources/cache'. If the data file is found
then the DataCache will load this instead of building the expensive data at CTS
runtime.
Note: Due to differences in gzip compression, different versions of node can
produce radically different binary cache files. gen_cache uses the hashes of the
source files to determine whether a cache file is 'up to date'. This is faster
and does not depend on the compressed output.

Options:
  --help          Print this message and exit.
  --list          Print the list of output files without writing them.
  --force         Rebuild cache even if they're up to date
  --validate      Check the cache is up to date
  --verbose       Print each action taken.
`);
  process.exit(rc);
}

// Where the cache is generated
const outDir = 'src/resources/cache';

let forceRebuild = false;
let mode: 'emit' | 'list' | 'validate' = 'emit';
let verbose = false;

const nonFlagsArgs: string[] = [];

for (const arg of process.argv) {
  if (arg.startsWith('-')) {
    switch (arg) {
      case '--list': {
        mode = 'list';
        break;
      }
      case '--help': {
        usage(0);
        break;
      }
      case '--force': {
        forceRebuild = true;
        break;
      }
      case '--verbose': {
        verbose = true;
        break;
      }
      case '--validate': {
        mode = 'validate';
        break;
      }
      default: {
        console.log('unrecognized flag: ', arg);
        usage(1);
      }
    }
  } else {
    nonFlagsArgs.push(arg);
  }
}

if (nonFlagsArgs.length < 3) {
  usage(0);
}

dataCache.setStore({
  load: (path: string) => {
    return new Promise<Uint8Array>((resolve, reject) => {
      fs.readFile(`data/${path}`, (err, data) => {
        if (err !== null) {
          reject(err.message);
        } else {
          resolve(data);
        }
      });
    });
  },
});
setIsBuildingDataCache();

const cacheFileSuffix = __filename.endsWith('.ts') ? '.cache.ts' : '.cache.js';

/**
 * @returns a list of all the files under 'dir' that has the given extension
 * @param dir the directory to search
 * @param ext the extension of the files to find
 */
function glob(dir: string, ext: string) {
  const files: string[] = [];
  for (const file of fs.readdirSync(dir)) {
    const path = `${dir}/${file}`;
    if (fs.statSync(path).isDirectory()) {
      for (const child of glob(path, ext)) {
        files.push(`${file}/${child}`);
      }
    }

    if (path.endsWith(ext) && fs.statSync(path).isFile()) {
      files.push(file);
    }
  }
  return files;
}

/**
 * Exception type thrown by SourceHasher.hashFile() when a file annotated with
 * MUST_NOT_BE_IMPORTED_BY_DATA_CACHE is transitively imported by a .cache.ts file.
 */
class InvalidImportException {
  constructor(path: string) {
    this.stack = [path];
  }
  toString(): string {
    return `invalid transitive import for cache:\n  ${this.stack.join('\n  ')}`;
  }
  readonly stack: string[];
}
/**
 * SourceHasher is a utility for producing a hash of a source .ts file and its imported source files.
 */
class SourceHasher {
  /**
   * @param path the source file path
   * @returns a hash of the source file and all of its imported dependencies.
   */
  public hashOf(path: string) {
    this.u32Array[0] = this.hashFile(path);
    return this.u32Array[0].toString(16);
  }

  hashFile(path: string): number {
    if (!fs.existsSync(path) && path.endsWith('.js')) {
      path = path.substring(0, path.length - 2) + 'ts';
    }

    const cached = this.hashes.get(path);
    if (cached !== undefined) {
      return cached;
    }

    this.hashes.set(path, 0); // Store a zero hash to handle cyclic imports

    const content = fs.readFileSync(path, { encoding: 'utf-8' });
    const normalized = content.replace('\r\n', '\n');
    let hash = crc32(normalized);
    for (const importPath of parseImports(path, normalized)) {
      try {
        const importHash = this.hashFile(importPath);
        hash = this.hashCombine(hash, importHash);
      } catch (ex) {
        if (ex instanceof InvalidImportException) {
          ex.stack.push(path);
          throw ex;
        }
      }
    }

    if (content.includes('MUST_NOT_BE_IMPORTED_BY_DATA_CACHE')) {
      throw new InvalidImportException(path);
    }

    this.hashes.set(path, hash);
    return hash;
  }

  /** Simple non-cryptographic hash combiner */
  hashCombine(a: number, b: number): number {
    return crc32(`${toHexString(a)} ${toHexString(b)}`);
  }

  private hashes = new Map<string, number>();
  private u32Array = new Uint32Array(1);
}

void (async () => {
  const suiteDirs = nonFlagsArgs.slice(2); // skip <exe> <js>
  for (const suiteDir of suiteDirs) {
    await build(suiteDir);
  }
})();

async function build(suiteDir: string) {
  if (!fs.existsSync(suiteDir)) {
    console.error(`Could not find ${suiteDir}`);
    process.exit(1);
  }

  // Load  hashes.json
  const fileHashJsonPath = `${outDir}/hashes.json`;
  let fileHashes: Record<string, string> = {};
  if (fs.existsSync(fileHashJsonPath)) {
    const json = fs.readFileSync(fileHashJsonPath, { encoding: 'utf8' });
    fileHashes = JSON.parse(json);
  }

  // Crawl files and convert paths to be POSIX-style, relative to suiteDir.
  const filesToEnumerate = glob(suiteDir, cacheFileSuffix)
    .map(p => `${suiteDir}/${p}`)
    .sort();

  const fileHasher = new SourceHasher();
  const cacheablePathToTS = new Map<string, string>();
  const errors: Array<string> = [];

  for (const file of filesToEnumerate) {
    const pathWithoutExtension = file.substring(0, file.length - 3);
    const mod = await import(`../../../${pathWithoutExtension}.js`);
    if (mod.d?.serialize !== undefined) {
      const cacheable = mod.d as Cacheable<unknown>;

      {
        // Check for collisions
        const existing = cacheablePathToTS.get(cacheable.path);
        if (existing !== undefined) {
          errors.push(
            `'${cacheable.path}' is emitted by both:
    '${existing}'
and
    '${file}'`
          );
        }
        cacheablePathToTS.set(cacheable.path, file);
      }

      const outPath = `${outDir}/${cacheable.path}`;
      const fileHash = fileHasher.hashOf(file);

      switch (mode) {
        case 'emit': {
          if (!forceRebuild && fileHashes[cacheable.path] === fileHash) {
            if (verbose) {
              console.log(`'${outPath}' is up to date`);
            }
            continue;
          }
          console.log(`building '${outPath}'`);
          const data = await cacheable.build();
          const serialized = cacheable.serialize(data);
          fs.mkdirSync(path.dirname(outPath), { recursive: true });
          fs.writeFileSync(outPath, serialized, 'binary');
          fileHashes[cacheable.path] = fileHash;
          break;
        }
        case 'list': {
          console.log(outPath);
          break;
        }
        case 'validate': {
          if (fileHashes[cacheable.path] !== fileHash) {
            errors.push(
              `'${outPath}' needs rebuilding. Generate with 'npx grunt run:generate-cache'`
            );
          } else if (verbose) {
            console.log(`'${outPath}' is up to date`);
          }
        }
      }
    }
  }

  // Check that there aren't stale files in the cache directory
  for (const file of glob(outDir, '.bin')) {
    if (cacheablePathToTS.get(file) === undefined) {
      switch (mode) {
        case 'emit':
          fs.rmSync(file);
          break;
        case 'validate':
          errors.push(
            `cache file '${outDir}/${file}' is no longer generated. Remove with 'npx grunt run:generate-cache'`
          );
          break;
      }
    }
  }

  // Update hashes.json
  if (mode === 'emit') {
    const json = JSON.stringify(fileHashes, undefined, '  ');
    fs.writeFileSync(fileHashJsonPath, json, { encoding: 'utf8' });
  }

  if (errors.length > 0) {
    for (const error of errors) {
      console.error(error);
    }
    process.exit(1);
  }
}
