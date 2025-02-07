# Neovim configuration guide for SPIR-V disassembly files

This directory holds instructions to configure Neovim for SPIR-V assembly files (`.spvasm`)

At the end, Neovim should support:
* Syntax highlighting
* Jump to definition
* Find all references
* Symbol renaming
* Operand hover information
* Formatting
* Completion suggestions for all Opcodes and Ids

While the instructions here are specifically for Neovim, they should translate easily to vim.

## Dependencies

In order to build and install the Visual Studio Code language server extension, you will need to install and have on your `PATH` the following dependencies:
* [`golang 1.16+`](https://golang.org/)

## File type detection

Neovim's default config location is typically `~/.config/nvim` so the rest of the instructions assume that but it will need to be changed if your system is different.

Tell neovim that `*.spvasm` files should be treated as `spvasm` filetype
```bash
echo "au BufRead,BufNewFile *.spvasm                set filetype=spvasm" > ~/.config/nvim/ftdetect/spvasm.vim
```

## Syntax Highlighting

### Generate the syntax highlighting file
```bash
cd <spirv-tools dir>
mkdir -p build && cd build
# Any platform is fine, ninja is used an as example
cmake -G Ninja ..
ninja spirv-tools-vimsyntax
```

### Copy the syntax file 
```bash
cp spvasm.vim ~/.config/nvim/syntax/spvasm.vim
```

## Language Server

### Building the LSP (masOS / Linux)

Run `build_lsp.sh`
Copy `spirvls` and `spirv.json` to a location in `$PATH`

```bash
cd <spirv-tools dir>/utils/vscode
./build_lsp.sh
sudo cp spirvls/* /usr/local/bin/
```

### Building the LSP (Windows)

TODO

### Configuring Neovim

Configuration will depend a lot on your installed plugins but assuming you are using [nvim-lspconfig](https://github.com/neovim/nvim-lspconfig) the following should be sufficient.

```lua
local lspconfig = require 'lspconfig'
local configs = require 'lspconfig.configs'

if not configs.spvasm then
  configs.spvasm = {
    default_config = {
      cmd = { 'spirvls' },
      filetypes = { 'spvasm' },
      root_dir = function(fname)
        return '.'
      end,
      settings = {},
    },
  }
end

lspconfig.spvasm.setup {
  capabilities = require('cmp_nvim_lsp').default_capabilities(vim.lsp.protocol.make_client_capabilities()),
}
```
