## Short summary ##
Pure C apps seem to work fine, except for the stacks missing file/lineno in the reports (see [issue 48](https://code.google.com/p/address-sanitizer/issues/detail?id=48) - only function+offset info is available).

The ASan runtime library builds using MSVC `cl` just fine.

One has to use MSVC's `link` to link the Clang-generated `.o` files and the runtime.

C++ support is blocked by the unfinished clang++/Win implementation (see [issue 56](https://code.google.com/p/address-sanitizer/issues/detail?id=56)).

Most likely it's possible to mix `cl`-built non-instrumented C++ code with `clang`-built instrumented C code.

# Try it #
_For up-to-date instructions, see the [build steps](http://code.google.com/p/address-sanitizer/source/browse/trunk/build/scripts/slave/buildbot_standard.bat) our internal buildbot is performing to test._

First, you’ll need to have Clang built:
```
set REV=-r HEAD
:: Replace “HEAD” with a revision number if you want
svn co %REV% http://llvm.org/svn/llvm-project/llvm/trunk llvm
svn co %REV% http://llvm.org/svn/llvm-project/cfe/trunk llvm/tools/clang
svn co %REV% http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler_rt

mkdir llvm-build
cd llvm-build

:: If devenv takes too much time and you observe "Function.cpp" near the end of the output,
:: please LLVM.sln and disable optimizations for LLVMCore/Function.cpp in the Release build.
cmake -G"Visual Studio 9 2008" ..\llvm && devenv LLVM.sln /Build Release /Project clang

cd ..
```

Now, build the runtime library:
```
cd compiler_rt\lib\asan
cl /nologo /MP /MT /Zi /I.. /I../../include /c *.cc ../interception/*.cc ../sanitizer_common/*.cc
lib /nologo /out:asan_rtl.lib *.obj
cd ..\..\..
```

Try to build your app with Clang:
```
path-to\llvm-build\bin\Release\clang.exe -fsanitize=address -g -c <your_files>
link /debug /incremental:no /out:myprogname.exe /nologo <your_obj_files> path-to\rtl\asan_rtl.lib /defaultlib:libcmt
myprogname.exe
```

## Known problems ##
See the corresponding [issue tracker label](http://code.google.com/p/address-sanitizer/issues/list?q=label:OpSys-Windows).

## SPEC CPU2006 results ##
We’ve run ASan/Win on SPEC tests.
It has shown the very same slowdown as the Linux version (see [here](http://code.google.com/p/address-sanitizer/wiki/PerformanceNumbers)) on the C tests.

The C++ tests were not run.

ASan has found the same UAF in perlbench and stack OOB on h264ref [seen on Linux](http://code.google.com/p/address-sanitizer/wiki/FoundBugs).

The global OOB on h264ref was not detected - since this feature is not implemented yet on Windows. (or are they? [issue 49](https://code.google.com/p/address-sanitizer/issues/detail?id=49))<br>However, it has found one similar heap-OOB access on h264ref which needs to be investigated. See <a href='https://code.google.com/p/address-sanitizer/issues/detail?id=50'>issue 50</a> for the details.<br>
<br>
<h2>Chromium</h2>
The plan is to try Chromium tests with runtime but no instrumentation first,<br>
then add instrumentation of C code, then ...<br>
<br>
Stay tuned!