AddressSanitizer is fully integrated with the LLVM source tree.

## Checkout AddressSanitizer sources ##
```
# cd somewhere
# Get llvm, clang and compiler-rt
svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
cd llvm
R=$(svn info | grep Revision: | awk '{print $2}')
(cd tools && svn co -r $R http://llvm.org/svn/llvm-project/cfe/trunk clang)
(cd projects && svn co -r $R http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt)
```

To switch your checkout to read-write mode:
```
CNAME=your_name_here
cd llvm
svn switch --relocate http://llvm.org/svn/llvm-project/llvm/trunk https://$CNAME@llvm.org/svn/llvm-project/llvm/trunk
(cd tools && svn switch --relocate http://llvm.org/svn/llvm-project/cfe/trunk https://$CNAME@llvm.org/svn/llvm-project/cfe/trunk clang)
(cd projects && svn switch --relocate http://llvm.org/svn/llvm-project/compiler-rt/trunk https://$CNAME@llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt)
```

## Host compiler ##
In order to build fresh Clang you need some other compiler (e.g. GCC or an older version of Clang).
Since January 2014 Clang will not build with old compilers (at least GCC 4.7 is required),
so on older systems you may need to install a newer compiler separately. Follow [these instructions](http://llvm.org/docs/GettingStarted.html#getting-a-modern-host-c-toolchain) to use new GCC as your host compiler.

## Building AddressSanitizer with CMake (LLVM 3.2 or later) ##
  * Download and install [CMake](http://www.cmake.org/cmake/resources/software.html) (you'll need at least CMake 2.8.8).
  * Get llvm, clang and compiler-rt sources (see above).
  * Make sure you have a modern C++ toolchain (see above).
  * Set configuration and build LLVM.
```
mkdir llvm_cmake_build && cd llvm_cmake_build
# Choose the host compiler
# Choose CMAKE_BUILD_TYPE {Debug, Release}
# Choose LLVM_ENABLE_ASSERTIONS {ON,OFF}
# Choose LLVM_ENABLE_WERROR {ON,OFF}
# Set LLVM_TARGETS_TO_BUILD to X86 to speed up the build
[CC=clang CXX=clang++] cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON [-DLLVM_ENABLE_WERROR=ON] [-DLLVM_TARGETS_TO_BUILD=X86] /path/to/llvm/checkout
```

  * Now you can build and test LLVM code:
```
make -j12 # build everything
make check-all -j12 # build and run all tests (llvm+clang+ASan)
make check-sanitizer -j12 # build and run sanitizer_common tests
make check-asan -j12 # build and run ASan unit and output tests
```
> Note that dynamic runtime tests are not included into check-asan or check-all on platforms where static runtime is default; to run them, you'll need to manually
```
make -j12 check-asan-dynamic
```
> To build only ASan runtime library:
```
# On Linux:
make -j12 asan
# The runtime libraries are built at:
# llvm_cmake_build/lib/clang/<llvm_version>/lib/{linux,darwin}/
```
> To build/run only ASan unit tests:
```
make -j12 AsanUnitTests
cd projects/compiler-rt/lib/asan/tests
./Asan-x86_64-Test
./Asan-i386-Test
./Asan-x86_64-Noinst-Test
./Asan-i386-Noinst-Test
```

> To re-run a single ASan lit-style output test make sure you've built Clang and run:
```
# Make sure clang and llvm tools are in your PATH.
export PATH="/path/to/llvm_cmake_build/bin:$PATH"
# Run a specific output test.
cd llvm_cmake_build
bin/llvm-lit projects/compiler-rt/test/asan/64bitConfig/TestCases/deep_tail_call.cc
```
  * Visit [LLVM CMake guide](http://llvm.org/docs/CMake.html) to see the ways you can customize your LLVM/Clang/ASan build. For example, you can build LLVM/Clang with ASan-ified Clang:
```
mkdir llvm_cmake_build_asan
cd llvm_cmake_build_asan
export CC=clang
export CXX=clang++

cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_SANITIZER=Address /path/to/llvm/checkout
```

To run lint:
```
projects/compiler-rt/lib/sanitizer_common/scripts/check_lint.sh
```

## Ninja ##
If you want to get even faster incremental builds, use ninja: http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html

## Chrome ##
The Chromium team periodically bakes fresh LLVM/Clang binaries, which include AddressSanitizer support.
Simply execute the following:
```
mkdir -p tools/clang
cd tools/clang
svn co http://src.chromium.org/svn/trunk/src/tools/clang/scripts
cd ../../
tools/clang/scripts/update.sh
# Now use third_party/llvm-build/Release+Asserts/bin/{clang,clang++}
```