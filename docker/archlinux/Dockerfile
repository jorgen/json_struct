FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
  pacman -S --noconfirm base-devel llvm clang libc++ cmake ninja git vim python3

ENV CXX=clang++
ENV CC=clang

RUN CLANG_VERSION=$(clang --version | head -n1 | sed 's/.*version \([0-9.]*\).*/\1/') && \
    git clone --depth=1 --branch llvmorg-${CLANG_VERSION} https://github.com/llvm/llvm-project && \
    cd llvm-project && \
    mkdir build; \
    cmake -G Ninja -S runtimes -B build \
      -DLLVM_ENABLE_RUNTIMES="libunwind;libcxx;libcxxabi" \
      -DLLVM_USE_SANITIZER=MemoryWithOrigins \
      -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
      -DLIBCXX_CXX_ABI=libcxxabi \
      -DLIBCXXABI_LIBCXX_INCLUDES=ON \
      -DCMAKE_INSTALL_PREFIX=/opt && \
    ninja -C build unwind cxx cxxabi && \
    ninja -C build install-unwind install-cxx install-cxxabi

ENV MSAN_CFLAGS="-std=c++20 -fsanitize=memory -nostdinc++ -I/opt/include -I/opt/include/c++/v1"
ENV MSAN_LFLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/lib -lc++abi -Wl,-rpath,/opt/lib"
