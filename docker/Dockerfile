FROM ubuntu:18.04

RUN apt-get update

RUN apt-get install -y cmake ninja-build
RUN apt-get install -y clang-7 libglu1-mesa-dev libc++-7-dev libc++abi-7-dev
RUN apt-get install -y libxi-dev
RUN apt-get install -y emacs-nox git curl wget unzip

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 100
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 100
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

WORKDIR /tmp
RUN curl -L https://github.com/emscripten-core/emsdk/archive/1b1f08f.zip > emsdk.zip
RUN unzip emsdk.zip
RUN mv emsdk-* emsdk
WORKDIR /tmp/emsdk
RUN ./emsdk install latest
RUN ./emsdk activate latest
RUN /bin/bash -c "source ./emsdk_env.sh"

ENV PATH="/tmp/emsdk:/tmp/emsdk/upstream/emscripten:/tmp/emsdk/node/12.9.1_64bit/bin:${PATH}"

ENV EMSDK /tmp/emsdk
ENV EM_CONFIG /root/.emscripten
ENV EMSDK_NODE /tmp/emsdk/node/12.9.1_64bit/bin/node

WORKDIR /tmp
RUN git clone https://github.com/google/filament.git

WORKDIR /tmp/filament
RUN git checkout -b v1.4.4
RUN ./build.sh -p webgl release

EXPOSE 8000 8000
