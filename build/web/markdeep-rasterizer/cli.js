#!/usr/bin/env node

const puppeteer = require('puppeteer');
const fs = require('fs');
const path = require('path');
const cheerio = require("cheerio");

const markdeepScript = '<script src="https://casual-effects.com/markdeep/latest/markdeep.min.js"></script>';

async function domDump(url) {
    const browser = await puppeteer.launch({headless: true});
    const page = await browser.newPage();
    await page.goto(url, {waitUntil: 'networkidle0'});
    const html = await page.content();
    await browser.close();
    return html;
}

var files = process.argv.slice(2);
if (files.length < 2) {
    console.log("npx markdeep-rasterizer <file1> ... <output_dir>");
    console.log("Each file must be a .md.html file.");
    console.log("The output directory is mandatory.");
    process.exit(1);
}

var outputDir = path.join(process.cwd(), files[files.length - 1]);
files = files.slice(0, files.length - 1);

files.forEach(function(file) {
    if (file.substring(file.length - '.md.html'.length, file.length) === '.md.html') {
        file = file.substring(0, file.length - '.md.html'.length);
    }

    var markdeepFile = path.join(process.cwd(), file + '.md.html');
    var outputFile = path.basename(file + '.html');

    domDump('file://' + markdeepFile + '?export').then(function(dom) {
        console.log('Processing ' + markdeepFile + '...');

        // Parse the DOM
        var $ = cheerio.load(dom);
        // Because of ?export Markdeep generated the output in a <pre> tag
        dom = $('pre').text();
        // We must remove the remaining invocation of Markdeep
        dom = dom.replace(markdeepScript, '');
        // Now replace <pre> with its own content so we can preserve headers
        $('pre').replaceWith(dom);

        fs.writeFile(path.join(outputDir, outputFile), $.html(), function(err) {
            if (err) console.log(err);
        });
    }).catch(function(e) {
        console.log('Could no process ' + markdeepFile + '!');
    });
});
