

# Introduction #
On July 2013 Intel released documentation on the upcoming instruction set extensions,
including the Memory Protection Extensions (MPX). Here we will discuss the applicability of MPX for memory error detection.
Links: [MPX-enabled GCC wiki](http://gcc.gnu.org/wiki/Intel%20MPX%20support%20in%20the%20GCC%20compiler);
[Using the MPX-enabled GCC and SDE (emulator)](http://software.intel.com/en-us/articles/using-intel-mpx-with-the-intel-software-development-emulator);
[Fresh documentation on Intel ISA which includes MPX](http://download-software.intel.com/sites/default/files/319433-015.pdf);
[Intel Pointer Checker](http://software.intel.com/en-us/articles/pointer-checker-feature-in-intel-parallel-studio-xe-2013-how-is-it-different-from-static).
Some external feedback: [1](https://groups.google.com/d/msg/comp.arch/iKAACmTrTQs/bzqG5Dp-FPEJ), [2](http://www.cs.rutgers.edu/news/articles/2013/07/24/intel-memory-protection-extensions), [3](http://lists.cs.uiuc.edu/pipermail/cfe-dev/2014-September/039088.html).

# Using SDE #
## Set up ##
Intel documentation is too verbose, here is a very short summary for Linux x86\_64:
  * From https://secure-software.intel.com/en-us/protected-download/267266/144917
    * Get 2013-07-22-mpx-runtime-external-lin.tar.bz2
    * Get sde-external-6.1.0-2013-07-22-lin.tar.bz2
  * From http://software.intel.com/en-us/articles/intel-software-development-emulator :
    * Get binutils-x86-64-static-avx512-mpx-sha.tar.bz2
  * Extract all archives into $MPX\_HOME and set the following environment variables:
```
export MPX_BINUTILS=$MPX_HOME/binutils_x86_64_static_avx512-mpx-sha
export MPX_RUNTIME_LIB=$MPX_HOME/2013-07-22-mpx-runtime-external-lin
export SDE_KIT=$MPX_HOME/sde-external-6.1.0-2013-07-22-lin
```

Now you need to compile and install the mpx-enabled gcc
(don't take the binaries from Intel, they may not work):
```
DIR=`pwd`
svn co svn://gcc.gnu.org/svn/gcc/branches/mpx # last tested r201897
mkdir mpx-build
cd mpx-build
../gcc/configure --prefix=$DIR/mpx-inst --enable-languages=c,c++ --enable-mpx \
  --with-as=$MPX_BINUTILS/bin/as --with-ld=$MPX_BINUTILS/bin/ld
make -j && make install
export MPX_GCC=$DIR/mpx-inst
```

## Run ##
```
% cat global_buffer_overflow.c 
#include <stdio.h>
int g[10];
int main(int argc, char **argv) {
  printf("g: %p %p\n", g, g+10);
  int x = g[argc * 10];
  printf("finishing\n");
  return x;
}
% $MPX_GCC/bin/gcc -fcheck-pointers -mmpx -L$MPX_RUNTIME_LIB -B$MPX_BINUTILS/bin -lmpx-runtime64 \
 -Wl,-rpath,$MPX_RUNTIME_LIB  global_buffer_overflow.c
# Use CHKP_RT_MODE=count to continue running after bug reports
% CHKP_RT_MODE=count $SDE_KIT/sde -mpx-mode -- ./a.out
g: 0x600bb0 0x600bd8
Bound violation detected,status 0x1 at 0x4006ce
finishing
```


I was able to build 401.bzip2 benchmark using this command line:

```
$MPX_GCC/bin/gcc -fcheck-pointers -mmpx -L$MPX_RUNTIME_LIB -B$MPX_BINUTILS/bin -lmpx-runtime64\ 
 -Wl,-rpath,$MPX_RUNTIME_LIB *.c -w -DSPEC_CPU -DNDEBUG -DSPEC_CPU_LP64  \
 -fno-mpx-use-fast-string-functions -fno-mpx-use-nochk-string-functions  -O1
```

The emulator ($SDE\_KIT/sde) is a PIN-based tool and has ~30x slowdown.

# Performance #
MPX has several different instructions that have very different performance properties:
  * BNDCU/BNDCL/BNDMK -- pure arithmetic, supposedly very fast.
  * BNDMOV -- move the BNDx registers from/to memory, mostly used to spill/fill registers. When accessing memory on stack should hit the L1 cache and thus be fast.
  * BNDLDX/BNDSTX -- access the Bound Table, a 2-layer cache-like data structure. Supposedly very slow (accesses two different cache lines).

Every memory access that needs to be checked will be instrumented with BNDCU/BNDCL (compiler optimizations may apply).
Since BNDCU/BNDCL are expected to be very fast, the slowdown will be defined by the ratio of the number of executed BNDCU/BNDCL vs BNDLDX/BNDSTX/BNDMOV.

The SDE allows to collect the number of executed instructions using the [`-mix` switch](http://software.intel.com/en-us/articles/intel-software-development-emulator).
We've collected stats on SPEC 2006, see the [spreadsheet](https://docs.google.com/spreadsheet/ccc?key=0Asb2KOp6b-AkdFAtWW5IYl9rNXk5OGM0YmROMmtnMXc#gid=0).

For some benchmarks (e.g. 444.namd or 462.libquantum)  dozenes of BNDCU/BNDCL are executed per single BNDLDX/BNDSTX.
For similar applications we can expect that MPX-based bug detection tools will be lightning fast.

For many other benchmarks (e.g. 400.perlbench, 429.mcf, 483.xalancbmk, 471.omnetpp)
the number of expensive BNDMOV and very expensive BNDLDX/BNDSTX instruction is comparable to the number of checks.
For these applications we expect MPX to be slower than alternative software-only solutions, such as AddressSanitizer.

Note that the data is preliminary because we've built the benchmarks
and without the MPX-enabled glibc.

# False positives #
## False positive with atomic pointers ##
http://software.intel.com/en-us/forums/topic/413959
```
% cat cxx11_ptr_check.cc 
// Example of a false positive with Pointer Checker or MPX.
// The false report happens because the pointer update
// and the metadata update together do not happen atomically.
#include <atomic>
#include <thread>
#include <iostream>
#include <assert.h>
std::atomic<int *> p;
int A, B;
void Thread1() { for (int i = 0; i < 100000; i++) p = &A; }
void Thread2() { for (int i = 0; i < 100000; i++) p = &B; }
void Thread3() { for (int i = 0; i < 100000; i++) assert(*p == 0); }

int main() {
  std::cout << "A=" << &A << " B=" << &B << std::endl;
  p = &A;
  std::thread t1(Thread1);
  std::thread t2(Thread2);
  std::thread t3(Thread3);
  t1.join();
  t2.join();
  t3.join();
}
% $MPX_GCC/bin/g++ -std=c++0x -fcheck-pointers -mmpx -L$MPX_RUNTIME_LIB -B$MPX_BINUTILS/bin \
 -lmpx-runtime64 -Wl,-rpath,$MPX_RUNTIME_LIB cxx11_ptr_check.cc 
% $SDE_KIT/sde   -mpx-mode -- ./a.out
A=0x606d28 B=0x606d2c
Bound violation detected,status 0x1 at 0x4012b8
```

## False positive with un-instrumented code ##
This limitation is acknowledged by Intel developers (http://gcc.gnu.org/wiki/Intel%20MPX%20support%20in%20the%20GCC%20compiler#Mixing_instrumented_and_legacy_code ):

> Note that in rare cases Bounds Table mechanism may miss bounds changes.
> We may model a case when legacy code rewrites a pointer in a memory with pointer of the same value but with different bounds.
> In such case false bound violation may occur. User is responsible for avoiding such cases.
> To get higher level of protection try to use instrumentation for modules generating external data.

```
==> mpxfp1.c <==
#include <stdio.h>
int *p;
extern int Foo(int idx);
extern int Bar(int idx);
void Set(int *x) { p = x; }
int Get(int idx) { return p[idx]; }
int main() {
  return Foo(5) + Bar(15);
}

==> mpxfp2.c <==
#include <stdio.h>
void Set(int *x);
int Get(int idx);
int Foo(int idx) {
  int b[22];
  int a[10];  // Address of 'a' must match 'a' from Bar()
  printf("Foo: a=%p b=%p\n", a, b);
  Set(a);
  return Get(idx);
}

==> mpxfp3.c <==
#include <stdio.h>
extern int *p;
int Get(int idx);
int Bar(int idx) {
  int a[20];  // Address of 'a' must match 'a' from Bar()
  int b[20];
  printf("Bar: a=%p b=%p\n", a, b);
  p = a;
  return Get(idx);
}
% $MPX_GCC/bin/g++ -O  -fcheck-pointers -mmpx -L$MPX_RUNTIME_LIB -B$MPX_BINUTILS/bin -lmpx-runtime64 -c mpxfp[12].c
% $MPX_GCC/bin/g++ -O -c mpxfp3.c  # No instrumentation
% $MPX_GCC/bin/g++ -fcheck-pointers -mmpx -L$MPX_RUNTIME_LIB -B$MPX_BINUTILS/bin -lmpx-runtime64 \
 -Wl,-rpath,$MPX_RUNTIME_LIB mpxfp[123].o
% $SDE_KIT/sde   -mpx-mode -- ./a.out
Foo: a=0x7ffff074d8f0 b=0x7ffff074d920
Bar: a=0x7ffff074d8f0 b=0x7ffff074d940
Bound violation detected,status 0x1 at 0x4006ce
```

Another example is when a heap memory address is reused: http://software.intel.com/en-us/forums/topic/413960

## False positives caused by compiler optimizations ##
We expect that the compiler optimizations will be causing a major headache to implementers of MPX-based checkers.

One example (mpx-gcc [r201896](https://code.google.com/p/address-sanitizer/source/detail?r=201896)):
```
% cat two_arrays.cc 
struct Bar {
  virtual ~Bar() { }
};
struct Foo {
  Bar x[3], y[3];
};
int main() {
  Foo f;
}
% $MPX_GCC/bin/g++ -fcheck-pointers -mmpx -O2  -L$MPX_RUNTIME_LIB -B$MPX_BINUTILS/bin -lmpx-runtime64 \
  -Wl,-rpath,$MPX_RUNTIME_LIB two_arrays.cc && $SDE_KIT/sde -mpx-mode -mpx_stats -- ./a.out
   Bound violation detected,status 0x1 at 0x4007ed
```
Here the compiler creates 2 loops to destruct `x` and `y`.
The second loop (destruction of `x`) uses the pointer that starts from the beginning of `y`,
i.e. which is out of bounds right away.

This class of false positives is avoidable with careful analysis of compiler optimizations.

## Variable size fields ##
Variable size fields typically cause false positives with MPX, which is quite expected.
The compiler has a special attribute to mark variable size fields as such: `__attribute__((bnd_variable_size))`.
We had to apply this attribute
in 8 places in [Chromium](http://dev.chromium.org) sources (7 in  the ICU code) to get Chromium's `base_unittests` running.
Example (third\_party/icu/source/common/ucmndata.h)
```
  typedef struct {
       uint32_t count;
       UDataOffsetTOCEntry entry[2] __attribute__((bnd_variable_size));    /* Actual size of array is from count. */
  } UDataOffsetTOC;
```


# Comparison with AddressSanitizer #
MPX is not yet available in hardware, so this section is speculation based
on our evaluation of
[Intel Pointer Checker](http://software.intel.com/en-us/articles/pointer-checker-feature-in-intel-parallel-studio-xe-2013-how-is-it-different-from-static)
(software-only implementation of MPX-like checker) and the MPX-enabled gcc (which can be run under emulator).
## MPX strengths ##
MPX-based tool can find in-struct buffer overflows:
```
  struct X {int a[10], b[20]; }; ...
  X x; ...
  x.a[15];  // Overflows to x.b[5]
```

MPX-based tool can find buffer overflows of any size since it does not rely on redzones:
```
int a[10]; ... 
a[1000000] = 0;  // Will be detected by MPX
```

MPX is expected to be very fast if the ratio of BNDCU/BNDLDX is large (i.e. for programs with long loops iterating over arrays).

## MPX weaknesses ##
  * MPX can not find use-after-free bugs
  * MPX has false positives with atomic pointers
  * MPX has false positives if some of the code is not instrumented
  * MPX is (as we expect) very slow for code working with lots of pointers (trees, lists, graphs, etc). This is partially confirmed by very small ratio of BNDCU/BNDLDX on 483.xalancbmk (above).
  * MPX has up to 4x overhead in RAM if the program has lots of pointers (trees, lists, graphs, etc).
  * MPX may be hard to deploy on legacy code where pointers to members are used to access other members (e.g. at least 7 SPEC benchmarks have errors).

# Biased conclusion #
A **very biased** conclusion: Intel MPX might be useful for in-struct buffer overflow detection, and for general buffer overflow detection in programs with lots of arrays and few pointers.
However AddressSanitizer (and, if implemented, AddressSanitizerInHardware) is more useful: faster, finds more bugs, easier to deploy.

# Random Thoughts #
BNDLDX/BNDSTX are (I guess) very slow since they access two cache lines.
Instead of BNDLDX/BNDSTX a tool may use a simple directly mapped shadow and use BNDMOV to read/write bounds.
If we instrument the entire program (with all libraries) we don't need to check the pointer value (as in BNDLDX)
and can have 2x shadow instead of 4x shadow.
TODO: rephrase this better.