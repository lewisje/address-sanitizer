# Introduction #

LeakSanitizer is a memory leak detector which is integrated into AddressSanitizer. The tool is supported on x86\_64 Linux.

LeakSanitizer is enabled by default in all ASan builds. LSan lies dormant until the very end of the process, at which point there is an extra leak detection phase. In performance-critical scenarios, LSan can also be used without ASan instrumentation.

See also: LeakSanitizerDesignDocument, LeakSanitizerVsHeapChecker

# Using LeakSanitizer #

To use LSan, simply build your program with AddressSanitizer:

```
$ cat memory-leak.c 
#include <stdlib.h>

void *p;

int main() {
  p = malloc(7);
  p = 0; // The memory is leaked here.
  return 0;
}
$ clang -fsanitize=address -g memory-leak.c
$ ./a.out 

=================================================================
==7829==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 7 byte(s) in 1 object(s) allocated from:
    #0 0x42c0c5 in __interceptor_malloc /usr/home/hacker/llvm/projects/compiler-rt/lib/asan/asan_malloc_linux.cc:74
    #1 0x43ef81 in main /usr/home/hacker/memory-leak.c:6
    #2 0x7fef044b876c in __libc_start_main /build/buildd/eglibc-2.15/csu/libc-start.c:226

SUMMARY: AddressSanitizer: 7 byte(s) leaked in 1 allocation(s).
```

If you want to run an ASan-instrumented program without leak detection, you can pass `detect_leaks=0` in the `ASAN_OPTIONS` environment variable.

## Stand-alone mode ##

If you just need leak detection, and don't want to bear the ASan slowdown, you can build with `-fsanitize=leak` instead of `-fsanitize=address`. This will link your program against a runtime library containing just the bare necessities required for LeakSanitizer to work. No compile-time instrumentation will be applied.

Be aware that the stand-alone mode is less well tested compared to running LSan on top of ASan.

## Flags ##

You can fine-tune LeakSanitizer's behavior through the `LSAN_OPTIONS` environment variable.

| flag| default | description |
|:----|:--------|:------------|
| exitcode | 23      | If non-zero, LSan will call `_exit(exitcode)` upon detecting leaks. This can be different from the exit code used to signal ASan errors. |
| max\_leaks | 0       | If non-zero, report only this many top leaks. |
| suppressions  | 

&lt;none&gt;

 | Path to file containing suppression rules (see below) |
| print\_suppressions  | 1       | If 1, print statistics for matched suppressions. |
| report\_objects  | 0       | If 1, LSan will report the addresses of individual leaked objects. |
| use\_unaligned | 0       | If 0, LSan will only consider properly aligned 8-byte patterns when looking for pointers. Set to 1 to include unaligned patterns. This refers to the pointer itself, not the memory being pointed at. |

Leak detection is also affected by certain [ASan flags](https://code.google.com/p/address-sanitizer/wiki/Flags). If you're not happy with the stack traces you see, check out `fast_unwind_on_malloc`, `malloc_context_size` and `strip_path_prefix`. Those flags go in `ASAN_OPTIONS` as usual. However, if you built with `-fsanitize=leak`, put them in `LSAN_OPTIONS` instead (and use `LSAN_SYMBOLIZER_PATH` to pass the symbolizer path).

## Suppressions ##

You can instruct LeakSanitizer to ignore certain leaks by passing in a suppressions file. The file must contain one suppression rule per line, each rule being of the form `leak:<pattern>`. The pattern will be substring-matched against the symbolized stack trace of the leak. If either function name, source file name or binary file name matches, the leak report will be suppressed.

```
$ cat suppr.txt 
# This is a known leak.
leak:FooBar
$ cat lsan-suppressed.cc 
#include <stdlib.h>

void FooBar() {
  malloc(7);
}

void Baz() {
  malloc(5);
}

int main() {
  FooBar();
  Baz();
  return 0;
}
$ clang++ lsan-suppressed.cc -fsanitize=address
$ ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=suppr.txt ./a.out

=================================================================
==26475==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 5 byte(s) in 1 object(s) allocated from:
    #0 0x44f2de in malloc /usr/home/hacker/llvm/projects/compiler-rt/lib/asan/asan_malloc_linux.cc:74
    #1 0x464e86 in Baz() (/usr/home/hacker/a.out+0x464e86)
    #2 0x464fb4 in main (/usr/home/hacker/a.out+0x464fb4)
    #3 0x7f7e760b476c in __libc_start_main /build/buildd/eglibc-2.15/csu/libc-start.c:226

-----------------------------------------------------
Suppressions used:
  count      bytes template
      1          7 FooBar
-----------------------------------------------------

SUMMARY: AddressSanitizer: 5 byte(s) leaked in 1 allocation(s).
```

The special symbols `^` and `$` match the beginning and the end of string.