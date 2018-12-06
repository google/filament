The folder contains a simple Filament demo that uses webpack, TypeScript, and `import`.

webpack combines `src/app.ts` with `node_modules/filament/filament.js` and generates
`public/main.js`. It also copies `filament.wasm` and `index.html`.

To build and run the sample, do:

    npm install
    npm run build
    cd public
    npx http-server
