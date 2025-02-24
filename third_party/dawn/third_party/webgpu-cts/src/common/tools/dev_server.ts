import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';

import * as babel from '@babel/core';
import * as chokidar from 'chokidar';
import * as express from 'express';
import * as morgan from 'morgan';
import * as portfinder from 'portfinder';
import * as serveIndex from 'serve-index';

import { makeListing } from './crawl.js';

// Make sure that makeListing doesn't cache imported spec files. See crawl().
process.env.STANDALONE_DEV_SERVER = '1';

function usage(rc: number): void {
  console.error(`\
Usage:
  tools/dev_server
  tools/dev_server 0.0.0.0
  npm start
  npm start 0.0.0.0

By default, serves on localhost only. If the argument 0.0.0.0 is passed, serves on all interfaces.
`);
  process.exit(rc);
}

const srcDir = path.resolve(__dirname, '../../');

// Import the project's babel.config.js. We'll use the same config for the runtime compiler.
const babelConfig = {
  ...require(path.resolve(srcDir, '../babel.config.js'))({
    cache: () => {
      /* not used */
    },
  }),
  sourceMaps: 'inline',
};

// Caches for the generated listing file and compiled TS sources to speed up reloads.
// Keyed by suite name
const listingCache = new Map<string, string>();
// Keyed by the path to the .ts file, without src/
const compileCache = new Map<string, string>();

console.log('Watching changes in', srcDir);
const watcher = chokidar.watch(srcDir, {
  persistent: true,
});

/**
 * Handler to dirty the compile cache for changed .ts files.
 */
function dirtyCompileCache(absPath: string, stats?: fs.Stats) {
  const relPath = path.relative(srcDir, absPath);
  if ((stats === undefined || stats.isFile()) && relPath.endsWith('.ts')) {
    const tsUrl = relPath;
    if (compileCache.has(tsUrl)) {
      console.debug('Dirtying compile cache', tsUrl);
    }
    compileCache.delete(tsUrl);
  }
}

/**
 * Handler to dirty the listing cache for:
 *  - Directory changes
 *  - .spec.ts changes
 *  - README.txt changes
 * Also dirties the compile cache for changed files.
 */
function dirtyListingAndCompileCache(absPath: string, stats?: fs.Stats) {
  const relPath = path.relative(srcDir, absPath);

  const segments = relPath.split(path.sep);
  // The listing changes if the directories change, or if a .spec.ts file is added/removed.
  const listingChange =
    // A directory or a file with no extension that we can't stat.
    // (stat doesn't work for deletions)
    ((path.extname(relPath) === '' && (stats === undefined || !stats.isFile())) ||
      // A spec file
      relPath.endsWith('.spec.ts') ||
      // A README.txt
      path.basename(relPath, 'txt') === 'README') &&
    segments.length > 0;
  if (listingChange) {
    const suite = segments[0];
    if (listingCache.has(suite)) {
      console.debug('Dirtying listing cache', suite);
    }
    listingCache.delete(suite);
  }

  dirtyCompileCache(absPath, stats);
}

watcher.on('add', dirtyListingAndCompileCache);
watcher.on('unlink', dirtyListingAndCompileCache);
watcher.on('addDir', dirtyListingAndCompileCache);
watcher.on('unlinkDir', dirtyListingAndCompileCache);
watcher.on('change', dirtyCompileCache);

const app = express();

// Send Chrome Origin Trial tokens
app.use((_req, res, next) => {
  next();
});

// Set up logging
app.use(morgan('dev'));

// Serve the standalone runner directory
app.use('/standalone', express.static(path.resolve(srcDir, '../standalone')));
// Add out-wpt/ build dir for convenience
app.use('/out-wpt', express.static(path.resolve(srcDir, '../out-wpt')));
app.use('/docs/tsdoc', express.static(path.resolve(srcDir, '../docs/tsdoc')));

// Serve a suite's listing.js file by crawling the filesystem for all tests.
app.get('/out/:suite([a-zA-Z0-9_-]+)/listing.js', async (req, res, next) => {
  const suite = req.params['suite'];

  if (listingCache.has(suite)) {
    res.setHeader('Content-Type', 'application/javascript');
    res.send(listingCache.get(suite));
    return;
  }

  try {
    const listing = await makeListing(path.resolve(srcDir, suite, 'listing.ts'));
    const result = `export const listing = ${JSON.stringify(listing, undefined, 2)}`;

    listingCache.set(suite, result);
    res.setHeader('Content-Type', 'application/javascript');
    res.send(result);
  } catch (err) {
    next(err);
  }
});

// Serve .as_worker.js files by generating the necessary wrapper.
app.get('/out/:suite([a-zA-Z0-9_-]+)/webworker/:filepath(*).as_worker.js', (req, res, next) => {
  const { suite, filepath } = req.params;
  const result = `\
import { g } from '/out/${suite}/${filepath}.spec.js';
import { wrapTestGroupForWorker } from '/out/common/runtime/helper/wrap_for_worker.js';

wrapTestGroupForWorker(g);
`;
  res.setHeader('Content-Type', 'application/javascript');
  res.send(result);
});

// Serve all other .js files by fetching the source .ts file and compiling it.
app.get('/out/**/*.js', async (req, res, next) => {
  const jsUrl = path.relative('/out', req.url);
  const tsUrl = jsUrl.replace(/\.js$/, '.ts');
  if (compileCache.has(tsUrl)) {
    res.setHeader('Content-Type', 'application/javascript');
    res.send(compileCache.get(tsUrl));
    return;
  }

  let absPath = path.join(srcDir, tsUrl);
  if (!fs.existsSync(absPath)) {
    // The .ts file doesn't exist. Try .js file in case this is a .js/.d.ts pair.
    absPath = path.join(srcDir, jsUrl);
  }

  try {
    const result = await babel.transformFileAsync(absPath, babelConfig);
    if (result && result.code) {
      compileCache.set(tsUrl, result.code);

      res.setHeader('Content-Type', 'application/javascript');
      res.send(result.code);
    } else {
      throw new Error(`Failed compile ${tsUrl}.`);
    }
  } catch (err) {
    next(err);
  }
});

// Serve everything else (not .js) as static, and directories as directory listings.
app.use('/out', serveIndex(path.resolve(srcDir, '../src')));
app.use('/out', express.static(path.resolve(srcDir, '../src')));

void (async () => {
  let host = '127.0.0.1';
  if (process.argv.length >= 3) {
    if (process.argv.length !== 3) usage(1);
    if (process.argv[2] === '0.0.0.0') {
      host = '0.0.0.0';
    } else {
      usage(1);
    }
  }

  console.log(`Finding an available port on ${host}...`);
  const kPortFinderStart = 8080;
  const port = await portfinder.getPortPromise({ host, port: kPortFinderStart });

  watcher.on('ready', () => {
    // Listen on the available port.
    app.listen(port, host, () => {
      console.log('Standalone test runner running at:');
      if (host === '0.0.0.0') {
        for (const iface of Object.values(os.networkInterfaces())) {
          for (const details of iface || []) {
            if (details.family === 'IPv4') {
              console.log(`  http://${details.address}:${port}/standalone/`);
            }
          }
        }
      } else {
        console.log(`  http://${host}:${port}/standalone/`);
      }
    });
  });
})();
