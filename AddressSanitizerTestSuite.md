# Introduction #

You may run AddressSanitizer test suite with any toolchain at hand (Clang or GCC). Basically, tests verify that the compiler understands `-fsanitize=address` flag and produces working executables, which crash with expected error messages.

Test suite is stored in [compiler-rt](http://compiler-rt.llvm.org) repository
and is kept in sync with upstream LLVM version of AddressSanitizer, so
some new tests may not work with older toolchains. GCC support in test-suite is still experimental, so be prepared to be prepared to encounter some problems.

# How to run test suite #
  * Download and install [CMake](http://cmake.org) (you'll need at least CMake 2.8.8).
  * Checkout LLVM sources and build LLVM tools. You may omit this step if you already have LLVM build tree somewhere in your system. Note, that you will need fresh enough host compiler (see HowToBuild section):
```
svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm_tmp_src
mkdir llvm_tmp_obj && cd llvm_tmp_obj
cmake -DCMAKE_BUILD_TYPE=Release ../llvm_tmp_src
make -j20
```
  * Checkout compiler-rt repository:
```
cd ..
svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt_src
```
  * Configure compiler-rt repository with just-built llvm-config and **the toolchain you want to test**:
```
  mkdir compiler-rt_obj && cd compiler-rt_obj
  cmake -DCMAKE_C_COMPILER=/your/c/compiler \
        -DCMAKE_CXX_COMPILER=/your/cxx/compiler \
        -DLLVM_CONFIG_PATH=../llvm_tmp_obj/bin/llvm-config \
        ../compiler-rt_src
```
  * Run AddressSanitizer test suite!
```
  make check-asan
```

# Known problems #
The following test may fail if you run the test-suite with GCC:
  * Tests that were added/modifed after the last merge into GCC tree.
  * Tests that include user-available ASan headers.
  * Tests for features not yet available in GCC (e.g. `-fsanitize-blacklist`, `-fsanitize=init-order`).
