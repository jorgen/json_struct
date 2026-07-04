# Image for the clang sanitizer CI jobs.
#
# MemorySanitizer requires a fully MSan-instrumented C++ runtime, and MSan is
# clang-only. We pin clang/LLVM to a fixed major version from apt.llvm.org
# instead of a rolling distro toolchain, so compiler diagnostics and codegen do
# not drift between image rebuilds (a rolling clang previously started rejecting
# Catch2's __COUNTER__ under -Werror).
FROM ubuntu:24.04

ARG LLVM_VERSION=20

ENV DEBIAN_FRONTEND=noninteractive

# Build tooling + clang/LLVM pinned from apt.llvm.org. g++ is installed so the
# address/undefined jobs (which use clang with the default libstdc++) have the
# standard library headers available.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
         ca-certificates gnupg wget \
         cmake ninja-build git python3 g++ \
    && wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
         | gpg --dearmor -o /usr/share/keyrings/llvm-archive-keyring.gpg \
    && . /etc/os-release \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/${VERSION_CODENAME}/ llvm-toolchain-${VERSION_CODENAME}-${LLVM_VERSION} main" \
         > /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
         clang-${LLVM_VERSION} lld-${LLVM_VERSION} libclang-rt-${LLVM_VERSION}-dev \
    && ln -sf /usr/bin/clang-${LLVM_VERSION}   /usr/bin/clang \
    && ln -sf /usr/bin/clang++-${LLVM_VERSION} /usr/bin/clang++ \
    && rm -rf /var/lib/apt/lists/*

ENV CC=clang
ENV CXX=clang++

# Build an MSan-instrumented libunwind/libc++abi/libc++ that matches the pinned
# clang and install it to /opt. libc++abi is built with the LLVM unwinder, so
# libunwind must be built and installed too, otherwise linking against the
# instrumented libc++ fails with "cannot find -lunwind".
RUN CLANG_VERSION=$(clang --version | sed -n 's/.*clang version \([0-9][0-9.]*\).*/\1/p' | head -n1) \
    && git clone --depth=1 --branch llvmorg-${CLANG_VERSION} https://github.com/llvm/llvm-project \
    && cmake -G Ninja -S llvm-project/runtimes -B llvm-project/build \
         -DLLVM_ENABLE_RUNTIMES="libunwind;libcxx;libcxxabi" \
         -DLLVM_USE_SANITIZER=MemoryWithOrigins \
         -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
         -DLIBCXX_CXX_ABI=libcxxabi \
         -DLIBCXXABI_LIBCXX_INCLUDES=ON \
         -DLIBCXX_INCLUDE_TESTS=OFF \
         -DLIBCXX_INCLUDE_BENCHMARKS=OFF \
         -DCMAKE_INSTALL_PREFIX=/opt \
    && ninja -C llvm-project/build unwind cxx cxxabi \
    && ninja -C llvm-project/build install-unwind install-cxx install-cxxabi \
    && rm -rf llvm-project

ENV MSAN_CFLAGS="-std=c++20 -fsanitize=memory -nostdinc++ -I/opt/include -I/opt/include/c++/v1"
ENV MSAN_LFLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/lib -lc++abi -Wl,-rpath,/opt/lib"
