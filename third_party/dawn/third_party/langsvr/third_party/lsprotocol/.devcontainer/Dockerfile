FROM mcr.microsoft.com/devcontainers/base:bookworm
RUN sudo apt update

# Pyenv installation of Python versions is missing below packages.
RUN sudo apt install libbz2-dev libncurses5-dev libffi-dev libreadline-dev libsqlite3-dev liblzma-dev -y

# Install fish.
RUN sudo apt install fish -y