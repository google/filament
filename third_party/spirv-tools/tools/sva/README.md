# SVA

SPIR-V Assember for WebGPU. The SPIR-V Assembler is a JavaScript library to
convert SPIR-V assembly (as produced by spirv-dis in SPIR-V Tools) into a
SPIR-V binary. The assembler assumes it is generating WebGPU SPIR-V and thus has
the following limitations.

 * Only 32 bit integers and floats supported
 * Only GLSL accepted as an extended instruction set
 * Doesn't support ! syntax for integers
 * Doesn't support hex encoding for float

```shell
yarn install
yarn test
```

You can also use `yarn watch` to watch all of the files and re-run tests as
needed.

## Webserver
Using `yarn serve` will start a webserver on localhost:5000. If you load the
`tests/index.html` file this will load the SVA files into browser.

## Command Line
There is a simple assembler binary with can be executed from the command line.

```shell
yarn sva tests/simple.spv_asm
```

The above will generate a `o.sva` file in the current directory.

## Update spirv.data.json

If there is a new spirv-headers release update the externals folder checkout
and then:

```shell
./tools/process_grammar.rb > src/spirv.data.json
```
