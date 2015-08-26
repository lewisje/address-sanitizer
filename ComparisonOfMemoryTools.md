# Summary #

|                                         | AddressSanitizer           | [Valgrind/Memcheck](http://valgrind.org) | [Dr. Memory](http://dynamorio.org/drmemory.html) | [Mudflap](http://gcc.gnu.org/wiki/Mudflap_Pointer_Debugging) | Guard Page  | [gperftools](https://code.google.com/p/gperftools/) |
|:----------------------------------------|:---------------------------|:-----------------------------------------|:-------------------------------------------------|:-------------------------------------------------------------|:------------|:----------------------------------------------------|
| technology                              | CTI                        | DBI                                      | DBI                                              | CTI                                                          | Library     | Library                                             |
| ARCH                                    | x86,ARM,PPC,...            | x86,ARM,PPC                              | x86                                              | all(?)                                                       | all(?)      | all(?)                                              |
| OS                                      | Linux, Mac, Windows, FreeBSD, Android| Linux, Mac                               | Windows, Linux                                   | Linux, Mac(?)                                                | All (1)     | Linux, Windows                                      |
| Slowdown                                | **2x**                       | 20x                                      | 10x                                              | 2x-40x                                                       | ?           | ?                                                   |
| Detects:                                |                            |                                          |                                                  |                                                              |             |                                                     |
| [Heap OOB](ExampleHeapOutOfBounds.md)       | **yes**                      | yes                                      | yes                                              | yes                                                          | some        | some                                                |
| [Stack OOB](ExampleStackOutOfBounds.md)     | **yes**                      | no                                       | no                                               | some                                                         | no          | no                                                  |
| [Global OOB](ExampleGlobalOutOfBounds.md)   | **yes**                      | no                                       | no                                               | ?                                                            | no          | no                                                  |
| [UAF](ExampleUseAfterFree.md)               | **yes**                      | yes                                      | yes                                              | yes                                                          | yes         | yes                                                 |
| [UAR](ExampleUseAfterReturn.md)             | **yes** (see UseAfterReturn)     | no                                       | no                                               | no                                                           | no          | no                                                  |
| UMR                                     | no (see [MemorySanitizer](https://code.google.com/p/memory-sanitizer/))                        | yes                                      | yes                                              | ?                                                            | no          | no                                                  |
| Leaks                                   | **yes** (see LeakSanitizer) | yes                                      | yes                                              | ?                                                            | no          | yes                                                 |


**DBI**: dynamic binary instrumentation 

&lt;BR&gt;


**CTI**: compile-time instrumentation 

&lt;BR&gt;


**UMR**: uninitialized memory reads 

&lt;BR&gt;


**UAF**: use-after-free (aka dangling pointer)  

&lt;BR&gt;


**UAR**: use-after-return 

&lt;BR&gt;


**OOB**: out-of-bounds  

&lt;BR&gt;


**x86**: includes 32- and 64-bit. 

&lt;BR&gt;


**Guard Page**: a family of memory error detectors ([Electric fence](http://perens.com/FreeSoftware/) or [DUMA](http://duma.sourceforge.net/) on Linux, Page Heap on Windows, Guard Malloc in Mac)
**gperftools**: various performance tools/error detectors bundled with TCMalloc. [Heap checker](http://gperftools.googlecode.com/svn/trunk/doc/heap_checker.html) (leak detector) is only available on Linux. [Debug allocator](https://code.google.com/p/gperftools/source/browse/src/debugallocation.cc) provides both guard pages and canary values for more precise detection of OOB writes, so it's better than guard page-only detectors.

# Performance numbers on SPEC cpu2006 #
  * AddressSanitizer: see PerformanceNumbers
  * Valgrind/Memcheck and Dr. Memory: see [Dr. Memory paper](http://static.googleusercontent.com/external_content/untrusted_dlcp/research.google.com/en/us/pubs/archive/37274.pdf)
  * Mudflap: slowdown varies from 2x to 40x
  * Guard Page: 2/3 cpu2006 benchmarks fail (tested using [DUMA](http://duma.sourceforge.net/) on a Linux box with 24G RAM)