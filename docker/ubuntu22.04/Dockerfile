FROM ubuntu:jammy
ARG llvmver=17

RUN apt update && \
  apt-get install -y software-properties-common gnupg apt-transport-https ca-certificates 

ADD docker/ubuntu22.04/llvm.list /etc/apt/sources.list.d/
ADD docker/ubuntu22.04/llvm-snapshot.gpg.key.gpg /etc/apt/trusted.gpg.d/

RUN apt update && \
  apt-get install -y ca-certificates build-essential clang-$llvmver libc++-$llvmver-dev libc++abi-$llvmver-dev libunwind-$llvmver-dev && \
  ln -s /usr/bin/clang++-$llvmver /usr/bin/clang++ && \
  ln -s /usr/bin/clang-$llvmver /usr/bin/clang

ENV CXX=clang++
ENV CC=clang

RUN apt install -y cmake ninja-build git vim

RUN git clone --depth=1 https://github.com/llvm/llvm-project && \
    cd llvm-project && \
    mkdir build; \
    cmake -G Ninja -S runtimes -B build -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" -DLLVM_USE_SANITIZER=MemoryWithOrigins  -DCMAKE_INSTALL_PREFIX=/opt && \
    ninja -C build cxx cxxabi && \
    ninja -C build install-cxx install-cxxabi

env MSAN_CFLAGS="-std=c++20 -fsanitize=memory -nostdinc++ -I/opt/include -I/opt/include/c++/v1"
env MSAN_LFLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/lib -lc++abi -Wl,-rpath,/opt/lib"
