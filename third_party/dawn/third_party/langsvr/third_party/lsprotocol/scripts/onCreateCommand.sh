#!/bin/bash

# Install pyenv and Python versions here to avoid using shim.
curl https://pyenv.run | bash
echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc

export PYENV_ROOT="$HOME/.pyenv"
command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"
# eval "$(pyenv init -)" Comment this out and DO NOT use shim.
source ~/.bashrc

# Install Rust and Cargo
curl https://sh.rustup.rs -sSf | bash -s -- -y
echo 'source $HOME/.cargo/env' >> ~/.bashrc

# Install Python via pyenv .
pyenv install 3.8 3.9 3.10 3.11 3.12

# Set default Python version to 3.8 .
pyenv global 3.8

# Create Virutal environment.
pyenv exec python3.8 -m venv .venv

# Activate Virtual environment.
source /workspaces/lsprotocol/.venv/bin/activate
