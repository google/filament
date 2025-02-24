'use strict';

const { resolve: resolvePath } = require('path');
const { writeFile } = require('fs/promises');
const { runClang } = require('./clang-utils');

/** @typedef {{ js_native_api_symbols: string[]; node_api_symbols: string[]; }} SymbolInfo */

/**
 * @param {number} [version]
 * @returns {Promise<SymbolInfo>}
 */
async function getSymbolsForVersion(version) {
    try {
        const { stdout } = await runClang([ '-ast-dump=json', '-fsyntax-only', '-fno-diagnostics-color', `-DNAPI_VERSION=${version}`, resolvePath(__dirname, '..', 'include', 'node_api.h')])

        const ast = JSON.parse(stdout);

        /** @type {SymbolInfo} */
        const symbols = { js_native_api_symbols: [], node_api_symbols: [] };

        for (const statement of ast.inner) {
            if (statement.kind !== 'FunctionDecl') {
                continue;
            }

            const name = statement.name;
            const file = statement.loc.includedFrom?.file;

            if (file) {
                symbols.js_native_api_symbols.push(name);
            } else {
                symbols.node_api_symbols.push(name);
            }
        }

        symbols.js_native_api_symbols.sort();
        symbols.node_api_symbols.sort();

        return symbols;
    } catch (err) {
        if (err.code === 'ENOENT') {
            throw new Error('This tool requires clang to be installed.');
        }
        throw err;
    }
}

/** @returns {Promise<{maxVersion: number, symbols: {[x: string]: SymbolInfo}}>} */
async function getAllSymbols() {
    /** @type {{[x: string]: SymbolInfo}} */
    const allSymbols = {};
    let version = 1;

    console.log('Processing symbols from clang:')
    while (true) {
        const symbols = await getSymbolsForVersion(version);

        if (version > 1) {
            const previousSymbols = allSymbols[`v${version - 1}`];
            if (previousSymbols.js_native_api_symbols.length == symbols.js_native_api_symbols.length && previousSymbols.node_api_symbols.length === symbols.node_api_symbols.length) {
                --version;
                break;
            }
        }
        allSymbols[`v${version}`] = symbols;
        console.log(`  v${version}: ${symbols.js_native_api_symbols.length} js_native_api_symbols, ${symbols.node_api_symbols.length} node_api_symbols`);
        ++version;
    }

    return {
        maxVersion: version,
        symbols: allSymbols
    };
}

/**
 * @param {SymbolInfo} previousSymbols
 * @param {SymbolInfo} currentSymbols
 * @returns {SymbolInfo}
 */
function getUniqueSymbols(previousSymbols, currentSymbols) {
    /** @type {SymbolInfo} */
    const symbols = { js_native_api_symbols: [], node_api_symbols: [] };
    for (const symbol of currentSymbols.js_native_api_symbols) {
        if (!previousSymbols.js_native_api_symbols.includes(symbol)) {
            symbols.js_native_api_symbols.push(symbol);
        }
    }
    for (const symbol of currentSymbols.node_api_symbols) {
        if (!previousSymbols.node_api_symbols.includes(symbol)) {
            symbols.node_api_symbols.push(symbol);
        }
    }
    return symbols;
}

/**
 * @param {string[]} strings
 */
function joinStrings(strings, prependNewLine = false) {
    if (strings.length === 0) return '';
    return `${prependNewLine ? ',\n        ' : ''}'${strings.join("',\n        '")}'`;
}

async function getSymbolData() {
    const { maxVersion, symbols } = await getAllSymbols();

    let data = `'use strict'

const v1 = {
    js_native_api_symbols: [
        ${joinStrings(symbols.v1.js_native_api_symbols)}
    ],
    node_api_symbols: [
        ${joinStrings(symbols.v1.node_api_symbols)}
    ]
}
`;

    for (let version = 2; version <= maxVersion; ++version) {
        const newSymbols = getUniqueSymbols(symbols[`v${version - 1}`], symbols[`v${version}`]);

        data += `
const v${version} = {
    js_native_api_symbols: [
        ...v${version - 1}.js_native_api_symbols${joinStrings(newSymbols.js_native_api_symbols, true)}
    ],
    node_api_symbols: [
        ...v${version - 1}.node_api_symbols${joinStrings(newSymbols.node_api_symbols, true)}
    ]
}
`;
    }

    data += `
module.exports = {
    ${new Array(maxVersion).fill(undefined).map((_, i) => `v${i + 1}`).join(',\n    ')}
}
`
    return data;
}

async function main() {
    const path = resolvePath(__dirname, '../symbols.js');
    const data = await getSymbolData();
    console.log(`Writing symbols to ${path}`)
    return writeFile(path, data);
}

main().catch(e => {
    console.error(e);
    process.exitCode = 1;
});
