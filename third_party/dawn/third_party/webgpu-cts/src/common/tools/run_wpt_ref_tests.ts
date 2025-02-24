import * as fs from 'fs';
import * as path from 'path';

import { chromium, firefox, webkit, Page, Browser } from 'playwright-core';

import { ScreenshotManager, readPng, writePng } from './image_utils.js';

declare function wptRefTestPageReady(): boolean;
declare function wptRefTestGetTimeout(): boolean;

const verbose = !!process.env.VERBOSE;
const kRefTestsBaseURL = 'http://localhost:8080/out/webgpu/web_platform/reftests';
const kRefTestsPath = 'src/webgpu/web_platform/reftests';
const kScreenshotPath = 'out-wpt-reftest-screenshots';

// note: technically we should use an HTML parser to find this to deal with whitespace
// attribute order, quotes, entities, etc but since we control the test source we can just
// make sure they match
const kRefLinkRE = /<link\s+rel="match"\s+href="(.*?)"/;
const kRefWaitClassRE = /class="reftest-wait"/;
const kFuzzy = /<meta\s+name="?fuzzy"?\s+content="(.*?)">/;

function printUsage() {
  console.log(`
run_wpt_ref_tests path-to-browser-executable [ref-test-name]

where ref-test-name is just a simple check for the test including the given string.
If not passed all ref tests are run

MacOS Chrome Example:
  node tools/run_wpt_ref_tests /Applications/Google\\ Chrome\\ Canary.app/Contents/MacOS/Google\\ Chrome\\ Canary

`);
}

// Get all of filenames that end with '.html'
function getRefTestNames(refTestPath: string) {
  return fs.readdirSync(refTestPath).filter(name => name.endsWith('.html'));
}

// Given a regex with one capture, return it or the empty string if no match.
function getRegexMatchCapture(re: RegExp, content: string) {
  const m = re.exec(content);
  return m ? m[1] : '';
}

type FileInfo = {
  content: string;
  refLink: string;
  refWait: boolean;
  fuzzy: string;
};

function readHTMLFile(filename: string): FileInfo {
  const content = fs.readFileSync(filename, { encoding: 'utf8' });
  return {
    content,
    refLink: getRegexMatchCapture(kRefLinkRE, content),
    refWait: kRefWaitClassRE.test(content),
    fuzzy: getRegexMatchCapture(kFuzzy, content),
  };
}

/**
 * This is workaround for a bug in Chrome. The bug is when in emulation mode
 * Chrome lets you set a devicePixelRatio but Chrome still renders in the
 * actual devicePixelRatio, at least on MacOS.
 * So, we compute the ratio and then use that.
 */
async function getComputedDevicePixelRatio(browser: Browser): Promise<number> {
  const context = await browser.newContext();
  const page = await context.newPage();
  await page.goto('data:text/html,<html></html>');
  await page.waitForLoadState('networkidle');
  const devicePixelRatio = await page.evaluate(() => {
    let resolve: (v: number) => void;
    const promise = new Promise(_resolve => (resolve = _resolve));
    const observer = new ResizeObserver(entries => {
      const devicePixelWidth = entries[0].devicePixelContentBoxSize[0].inlineSize;
      const clientWidth = entries[0].target.clientWidth;
      const devicePixelRatio = devicePixelWidth / clientWidth;
      resolve(devicePixelRatio);
    });
    observer.observe(document.documentElement);
    return promise;
  });
  await page.close();
  await context.close();
  return devicePixelRatio as number;
}

// Note: If possible, rather then start adding command line options to this tool,
// see if you can just make it work based off the path.
async function getBrowserInterface(executablePath: string) {
  const lc = executablePath.toLowerCase();
  if (lc.includes('chrom')) {
    const browser = await chromium.launch({
      executablePath,
      headless: false,
      args: ['--enable-unsafe-webgpu'],
    });
    const devicePixelRatio = await getComputedDevicePixelRatio(browser);
    const context = await browser.newContext({
      deviceScaleFactor: devicePixelRatio,
    });
    return { browser, context };
  } else if (lc.includes('firefox')) {
    const browser = await firefox.launch({
      executablePath,
      headless: false,
    });
    const context = await browser.newContext();
    return { browser, context };
  } else if (lc.includes('safari') || lc.includes('webkit')) {
    const browser = await webkit.launch({
      executablePath,
      headless: false,
    });
    const context = await browser.newContext();
    return { browser, context };
  } else {
    throw new Error(`could not guess browser from executable path: ${executablePath}`);
  }
}

// Parses a fuzzy spec as defined here
// https://web-platform-tests.org/writing-tests/reftests.html#fuzzy-matching
// Note: This is not robust but the tests will eventually be run in the real wpt.
function parseFuzzy(fuzzy: string) {
  if (!fuzzy) {
    return { maxDifference: [0, 0], totalPixels: [0, 0] };
  } else {
    const parts = fuzzy.split(';');
    if (parts.length !== 2) {
      throw Error(`unhandled fuzzy format: ${fuzzy}`);
    }
    const ranges = parts.map(part => {
      const range = part
        .replace(/[a-zA-Z=]/g, '')
        .split('-')
        .map(v => parseInt(v));
      return range.length === 1 ? [0, range[0]] : range;
    });
    return {
      maxDifference: ranges[0],
      totalPixels: ranges[1],
    };
  }
}

// Compares two images using the algorithm described in the web platform tests
// https://web-platform-tests.org/writing-tests/reftests.html#fuzzy-matching
// If they are different will write out a diff mask.
function compareImages(
  filename1: string,
  filename2: string,
  fuzzy: string,
  diffName: string,
  startingRow: number = 0
) {
  const img1 = readPng(filename1);
  const img2 = readPng(filename2);
  const { width, height } = img1;
  if (img2.width !== width || img2.height !== height) {
    console.error('images are not the same size:', filename1, filename2);
    return;
  }

  const { maxDifference, totalPixels } = parseFuzzy(fuzzy);

  const diffData = Buffer.alloc(width * height * 4);
  const diffPixels = new Uint32Array(diffData.buffer);
  const kRed = 0xff0000ff;
  const kWhite = 0xffffffff;
  const kYellow = 0xff00ffff;

  let numPixelsDifferent = 0;
  let anyPixelsOutOfRange = false;
  for (let y = startingRow; y < height; ++y) {
    for (let x = 0; x < width; ++x) {
      const offset = y * width + x;
      let isDifferent = false;
      let outOfRange = false;
      for (let c = 0; c < 4 && !outOfRange; ++c) {
        const off = offset * 4 + c;
        const v0 = img1.data[off];
        const v1 = img2.data[off];
        const channelDiff = Math.abs(v0 - v1);
        outOfRange ||= channelDiff < maxDifference[0] || channelDiff > maxDifference[1];
        isDifferent ||= channelDiff > 0;
      }
      numPixelsDifferent += isDifferent ? 1 : 0;
      anyPixelsOutOfRange ||= outOfRange;
      diffPixels[offset] = outOfRange ? kRed : isDifferent ? kYellow : kWhite;
    }
  }

  const pass =
    !anyPixelsOutOfRange &&
    numPixelsDifferent >= totalPixels[0] &&
    numPixelsDifferent <= totalPixels[1];
  if (!pass) {
    writePng(diffName, width, height, diffData);
    console.error(
      `FAIL: too many differences in: ${filename1} vs ${filename2}
       ${numPixelsDifferent} differences, expected: ${totalPixels[0]}-${totalPixels[1]} with range: ${maxDifference[0]}-${maxDifference[1]}
       wrote difference to: ${diffName};
      `
    );
  } else {
    console.log(`PASS`);
  }
  return pass;
}

function exists(filename: string) {
  try {
    fs.accessSync(filename);
    return true;
  } catch (e) {
    return false;
  }
}

async function waitForPageRender(page: Page) {
  await page.evaluate(() => {
    return new Promise(resolve => requestAnimationFrame(resolve));
  });
}

// returns true if the page timed out.
async function runPage(page: Page, url: string, refWait: boolean) {
  console.log('  loading:', url);
  // we need to load about:blank to force the browser to re-render
  // else the previous page may still be visible if the page we are loading fails
  await page.goto('about:blank');
  await page.waitForLoadState('domcontentloaded');
  await waitForPageRender(page);

  await page.goto(url);
  await page.waitForLoadState('domcontentloaded');
  await waitForPageRender(page);

  if (refWait) {
    await page.waitForFunction(() => wptRefTestPageReady());
    const timeout = await page.evaluate(() => wptRefTestGetTimeout());
    if (timeout) {
      return true;
    }
  }
  return false;
}

async function main() {
  const args = process.argv.slice(2);
  if (args.length < 1 || args.length > 2) {
    printUsage();
    return;
  }

  const [executablePath, refTestName] = args;

  if (!exists(executablePath)) {
    console.error(executablePath, 'does not exist');
    return;
  }

  const testNames = getRefTestNames(kRefTestsPath).filter(name =>
    refTestName ? name.includes(refTestName) : true
  );

  if (!exists(kScreenshotPath)) {
    fs.mkdirSync(kScreenshotPath, { recursive: true });
  }

  if (testNames.length === 0) {
    console.error(`no tests include "${refTestName}"`);
    return;
  }

  const { browser, context } = await getBrowserInterface(executablePath);
  const page = await context.newPage();

  const screenshotManager = new ScreenshotManager();
  await screenshotManager.init(page);

  if (verbose) {
    page.on('console', async msg => {
      const { url, lineNumber, columnNumber } = msg.location();
      const values = await Promise.all(msg.args().map(a => a.jsonValue()));
      console.log(`${url}:${lineNumber}:${columnNumber}:`, ...values);
    });
  }

  await page.addInitScript({
    content: `
    (() => {
      let timeout = false;
      setTimeout(() => timeout = true, 5000);

      window.wptRefTestPageReady = function() {
        return timeout || !document.documentElement.classList.contains('reftest-wait');
      };

      window.wptRefTestGetTimeout = function() {
        return timeout;
      };
    })();
    `,
  });

  type Result = {
    status: string;
    testName: string;
    refName: string;
    testScreenshotName: string;
    refScreenshotName: string;
    diffName: string;
  };
  const results: Result[] = [];
  const addResult = (
    status: string,
    testName: string,
    refName: string,
    testScreenshotName: string = '',
    refScreenshotName: string = '',
    diffName: string = ''
  ) => {
    results.push({ status, testName, refName, testScreenshotName, refScreenshotName, diffName });
  };

  for (const testName of testNames) {
    console.log('processing:', testName);
    const { refLink, refWait, fuzzy } = readHTMLFile(path.join(kRefTestsPath, testName));
    if (!refLink) {
      throw new Error(`could not find ref link in: ${testName}`);
    }
    const testURL = `${kRefTestsBaseURL}/${testName}`;
    const refURL = `${kRefTestsBaseURL}/${refLink}`;

    // Technically this is not correct but it fits the existing tests.
    // It assumes refLink is relative to the refTestsPath but it's actually
    // supposed to be relative to the test. It might also be an absolute
    // path. Neither of those cases exist at the time of writing this.
    const refFileInfo = readHTMLFile(path.join(kRefTestsPath, refLink));
    const testScreenshotName = path.join(kScreenshotPath, `${testName}-actual.png`);
    const refScreenshotName = path.join(kScreenshotPath, `${testName}-expected.png`);
    const diffName = path.join(kScreenshotPath, `${testName}-diff.png`);

    const timeoutTest = await runPage(page, testURL, refWait);
    if (timeoutTest) {
      addResult('TIMEOUT', testName, refLink);
      continue;
    }
    await screenshotManager.takeScreenshot(page, testScreenshotName);

    const timeoutRef = await runPage(page, refURL, refFileInfo.refWait);
    if (timeoutRef) {
      addResult('TIMEOUT', testName, refLink);
      continue;
    }
    await screenshotManager.takeScreenshot(page, refScreenshotName);

    const pass = compareImages(testScreenshotName, refScreenshotName, fuzzy, diffName);
    addResult(
      pass ? 'PASS' : 'FAILURE',
      testName,
      refLink,
      testScreenshotName,
      refScreenshotName,
      diffName
    );
  }

  console.log(
    `----results----\n${results
      .map(({ status, testName }) => `[ ${status.padEnd(7)} ] ${testName}`)
      .join('\n')}`
  );

  const imgLink = (filename: string, title: string) => {
    const name = path.basename(filename);
    return `
    <div class="screenshot">
      ${title}
      <a href="${name}" title="${name}">
        <img src="${name}" width="256"/>
      </a>
    </div>`;
  };

  const indexName = path.join(kScreenshotPath, 'index.html');
  fs.writeFileSync(
    indexName,
    `<!DOCTYPE html>
<html>
  <head>
    <style>
    .screenshot {
      display: inline-block;
      background: #CCC;
      margin-right: 5px;
      padding: 5px;
    }
    .screenshot a {
      display: block;
    }
    .screenshot
    </style>
  </head>
  <body>
  ${results
    .map(({ status, testName, refName, testScreenshotName, refScreenshotName, diffName }) => {
      return `
        <div>
           <div>[ ${status} ]: ${testName} ref: ${refName}</div>
           ${
             status === 'FAILURE'
               ? `${imgLink(testScreenshotName, 'actual')}
                  ${imgLink(refScreenshotName, 'ref')}
                  ${imgLink(diffName, 'diff')}`
               : ``
           }
        </div>
        <hr>
      `;
    })
    .join('\n')}
  </body>
</html>
  `
  );

  // the file:// with an absolute path makes it clickable in some terminals
  console.log(`\nsee: file://${path.resolve(indexName)}\n`);

  await page.close();
  await context.close();
  // I have no idea why it's taking ~30 seconds for playwright to close.
  console.log('-- [ done: waiting for browser to close ] --');
  await browser.close();
}

main().catch(e => {
  throw e;
});
