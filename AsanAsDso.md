## ASAN as a shared library ##

Until recently, ASAN runtime in Clang on Linux was offered only in the form of static library (e.g. `libclang_rt.asan-x86_64.a`). This has recently been changed and you can now ask for shared runtime (aka ASAN-DSO) by cmaking with
-DCOMPILER\_RT\_BUILD\_SHARED\_ASAN=ON (and then compiling your code with `-fsanitize=address -shared-libasan`).

On most other platforms ASAN is offered only in the form of DSO due to platform limitations.
The GCC variant of ASAN offers both static and DSO variants and DSO is the default.

ASAN-DSO is _not_ (yet?) officially supported. Use it at your own risk.

Cons:
  * Worse performance (ASAN run-time is called via PLT even in the main executable)
  * [Issue 147](https://code.google.com/p/address-sanitizer/issues/detail?id=147) (can't use -static-libstdc++)
  * Harder deployment (need to carry the DSO around)
  * `__asan_init` is not called from `preinit_array` and so there is a risk that an instrumented code will get called before `__asan_init` (may cause SEGV at startup; still unlikely)
  * Spurious warnings from libsanitizer when debug output is enabled (e.g. "AddressSanitizer: failed to intercept 'memcpy'")

Pros:
  * Smaller disk usage and memory footprint when multiple processes are running with asan.
  * Potential ability to bring old ASAN-ified binaries to new systems
  * Ability to LD\_PRELOAD ASAN-DSO, see below.

## ASAN and LD\_PRELOAD ##

It is possible to use ASAN in this scenario (e.g. JVM+JNI, Python+SWIG):
  * There is third-party executable binary which can not be recompiled
  * It loads shared libraries that can be recompiled and we want to test them with ASAN

A simple solution is to build ASAN-DSO and LD\_PRELOAD it into the process, however the devil is in the detail.
  * If the process creates sub-processes, they will also have ASAN-DSO preloaded, which may be undesirable.
  * You have to ensure that version of LD\_PRELOAD'ed ASAN-DSO matches ASAN-DSO that was used to build sanitized shared libraries
  * TODO (stay tuned)