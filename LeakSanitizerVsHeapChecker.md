# Introduction #

LeakSanitizer was designed to replace the gperftools [Heap Leak Checker](http://google-perftools.googlecode.com/svn/trunk/doc/heap_checker.html) and to bring leak detection to ASan users. This page documents LSan's advantages over its predecessor.

# Features and output #

LeakSanitizer uses a very similar algorithm to the one used by Heap Checker. We do some things differently though, in particular handling of thread-local data. Heap Checker has an issue which causes it to miss some leaks in the presence of threads (see gperftools [issue 537](https://code.google.com/p/gperftools/issues/detail?id=537)). LSan reports those leaks correctly. Also, when used on top of ASan, LSan will rely on ASan to tell it which memory ranges are valid. This helps to avoid picking up "junk" pointers from memory locations which are not legally accessible. Such pointers can mask real leaks.

LeakSanitizer reports more information about leaks. The stack traces look better (the symbolizer that we use reports source file name, line number, and the function’s type signature where Heap Checker reports only the function’s name). LSan also differentiates between direct and indirect leaks in its output. This gives useful information about which leaks should be prioritized, because fixing the direct leaks is likely to fix the indirect ones as well.

LeakSanitizer also has built-in support for suppressions. This proves useful in Chrome where we had to use a wrapper script around Heap Checker to suppress reports of known leaks. In addition, we support Heap-Checker-style in-code leak annotations.

Some of Heap Checker's less frequently used features are currently not supported by LeakSanitizer. In particular, we don't support:
  * local leak checking,
  * multiple leak checks per process,
  * 'draconian' mode,
  * the REGISTER\_HEAPCHECK\_CLEANUP macro,
  * dumping the call graph to a heap profile file.

# Performance #

Both LeakSanitizer and Heap Checker are built on top of thread caching allocators. However, Heap Checker negates the benefits of tcmalloc by installing a global lock on malloc/free. LSan keeps its allocator lock-free, which makes a difference in massively multithreaded malloc-intensive applications.

A [crude synthetic benchmark](http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/lsan/lit_tests/TestCases/high_allocator_contention.cc?view=markup&pathrev=189012) which executes 10 million malloc/free pairs in parallel produced the following numbers:

|                       | 1 thread  | 5 threads | 50 threads | 500 threads |
|:----------------------|:----------|:----------|:-----------|:------------|
| Heap Checker (no ASan)          | 1.7s      | 16.3s     | 14.9s      | 14.7s       |
| LSan on top of ASan        | 2.3s      | 1.1s      | 1.6s       | 1.9s        |
| LSan without ASan     | 0.7s      | 0.2s      | 0.1s       | 0.2s        |

The slowdown introduced by ASan instrumentation led to a disadvantage in the single-threaded run, but in multi-threaded Heap Checker runs most of the time was spent waiting on a spinlock. LSan consistently outperformed Heap Checker when used without ASan instrumentation.

We used Chromium's unit\_tests binary to benchmark real-world performance. Besides the total running time, we also measured the running time of user code (i.e. everything up to the leak checking phase), in order to get a better idea of the slowdown introduced.

|                       | user code | total  |
|:----------------------|:----------|:-------|
| Heap Checker (no ASan)          | 13m24s     | 14m11s |
| LSan on top of ASan        | 10m52s    | 11m00s |
| LSan without ASan     | 9m6s      | 9m13s  |