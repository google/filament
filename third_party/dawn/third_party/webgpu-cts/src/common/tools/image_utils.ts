import * as fs from 'fs';

import { Page } from 'playwright-core';
import { PNG } from 'pngjs';
import { screenshot, WindowInfo } from 'screenshot-ftw';

// eslint-disable-next-line ban/ban
const waitMS = (ms: number) => new Promise(resolve => setTimeout(resolve, ms));

export function readPng(filename: string) {
  const data = fs.readFileSync(filename);
  return PNG.sync.read(data);
}

export function writePng(filename: string, width: number, height: number, data: Buffer) {
  const png = new PNG({ colorType: 6, width, height });
  for (let i = 0; i < data.byteLength; ++i) {
    png.data[i] = data[i];
  }
  const buffer = PNG.sync.write(png);
  fs.writeFileSync(filename, buffer);
}

export class ScreenshotManager {
  window?: WindowInfo;

  async init(page: Page) {
    // set the title to some random number so we can find the window by title
    const title: string = await page.evaluate(() => {
      const title = `t-${Math.random()}`;
      document.title = title;
      return title;
    });

    // wait for the window to show up
    let window;
    for (let i = 0; !window && i < 100; ++i) {
      await waitMS(50);
      const windows = await screenshot.getWindows();
      window = windows.find(window => window.title.includes(title));
    }
    if (!window) {
      throw Error(`could not find window: ${title}`);
    }
    this.window = window;
  }

  async takeScreenshot(page: Page, screenshotName: string) {
    // await page.screenshot({ path: screenshotName });

    // we need to set the url and title since the screenshot will include the chrome
    await page.evaluate(() => {
      document.title = 'screenshot';
      window.history.replaceState({}, '', '/screenshot');
    });
    await screenshot.captureWindowById(screenshotName, this.window!.id);
  }
}
