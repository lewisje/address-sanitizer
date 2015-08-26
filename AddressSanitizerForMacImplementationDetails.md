# AddressSanitizer for Mac: don’t be afraid. #

**ATTENTION**: most of the implementation details of ASan on Mac have changed in the past years and aren't reflected in this document. Please refer to the source. In particular, we are no longer using mach\_override (which used to be a maintenance hell and didn't work on iOS) in favor of dyld interposition (see http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/interception/interception.h?view=markup) and re-exec()ution. This allows us to hook the allocation/deallocation routines early and eliminates the need to replace CFAllocator and wrap various CF... functions.
Overall, we've stopped relying on most of the undocumented functions and assumptions.

This document should cover the implementation details of AddressSanitizer for Mac OS, because there are notable differences from Linux. We are almost sure that not all the possible issues are covered here (and in the code), so any comments and suggestions are welcome.



## Overview ##

AddressSanitizer is a dynamic detector of addressability bugs like buffer overflows (on the heap, stack and global variables) use-after-free errors for heap allocations and use-after-return errors for stack allocations.
The AddressSanitizer algorithm is described at http://code.google.com/p/address-sanitizer/wiki/AddressSanitizerAlgorithm, but we’ll repeat it here for the sake of completeness.

Every 8 bytes of application memory are backed up with a byte of shadow memory which tells how many of those 8 bytes are addressable. Each memory access in a program is instrumented with code that checks the addressability of the memory region being accessed before the actual memory access. If the memory is unaddressable, an error is reported.
AddressSanitizer consists of an LLVM instrumentation pass and a runtime library.
The instrumentation pass adds the code that checks the shadow memory before each memory access. It also rearranges the variables on the stack and in the data segment by adding “poisoned” (unaddressable) redzones to them. For each module in the client program the instrumentation pass emits a static constructor that calls `__asan_init()` (the routine that initializes the runtime library if it wasn’t initialized yet) and `__asan_register_globals()` (tells the runtime about the globals in the module and initializes their redzones). Similarly, a static destructor that tears the globals down by calling `__asan_unregister_globals()` is emitted.

The runtime library manages the shadow memory for the application. It replaces the client memory allocation routines so that poisoned redzones are added to each allocated chunk of memory. It also poisons the redzones around global variables (on program startup) and around stack variables (on function entry).
The runtime library intercepts a number of system library functions (http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/asan/asan_interceptors.cc?view=markup). This is needed for two reasons:
to keep track of the program state (e.g. thread creation and destruction, stack context manipulation with `setjmp()/longjmp()`, signal handling)
to improve the tool’s precision by replacing non-instrumented functions that are popular causes of memory errors (`memcpy()`, `memmove()`, `strcmp()` etc.)

We’ve added the `-faddress-sanitizer` flag to Clang, the LLVM front-end for C language family. For the source files compiled with Clang it produces the instrumented object files. For the object files it invokes ld, which links them together with the runtime library into a single binary. Instrumented and non-instrumented modules may be linked together if the two conditions are met:
the modules keep their binary compatibility with each other (and the system libraries), otherwise something may crash at startup;
code from non-instrumented modules that is executed before `__asan_init()` cannot call code from the instrumented modules, otherwise a segmentation violation will occur while trying to access the shadow memory. In fact it is almost impossible to ensure this at compile time, so the best option is to instrument the whole program.

An important design decision taken in the early days of AddressSanitizer is that it should be as transparent to the user as possible. Therefore it’s usually enough to pass `-faddress-sanitizer` to `clang`/`clang++` in order to compile and link the instrumented program, and the resulting binary should just work as it would normally do, and report memory errors in the case they occur. We try to minimize the use of any additional options needed to run the instrumented program, because it usually complicates the deployment process and thwart the users.

## AddressSanitizer and other memory tools ##

Some comparison of existing memory tools is done at http://code.google.com/p/address-sanitizer/wiki/ComparisonOfMemoryTools.

### Valgrind Memcheck ###

[Memcheck](http://valgrind.org/docs/manual/mc-manual.html) is based on [Valgrind](http://valgrind.org/), a binary instrumentation framework, so all the advantages and disadvantages of DBI apply to it. It does not require code recompilation and is able to find the memory errors in the shared libraries and JITted code as well as in the main executable. On the other hand, it is unable to find bugs on global and stack variables. Memcheck is also quite slow, mainly because of the translation/instrumentation overhead and the program threads being serialized.
Memcheck also allows the user to detect problems related to uninitialized values, which requires a more complex shadow state and is not the goal of AddressSanitizer.

### libgmalloc ###

Some people compare AddressSanitizer to [`libgmalloc`](https://developer.apple.com/library/mac/#documentation/darwin/reference/manpages/man3/libgmalloc.3.html). These two tools are a bit similar, but they may find different bugs. libgmalloc places each heap allocation on its own virtual memory page, which is unmapped when the allocated memory chunk is freed. This allows libgmalloc to easily detect use-after-free bugs without instrumenting the program or the system libraries, but at the same time page-level granularity does not allow to catch the accesses right behind the array bounds.
libgmalloc does not manage stack memory and the globals, thus it can’t detect stack and global buffer overflows.

## Clang and compile-time stuff ##

### Instrumentation ###

Mac-specific bits of the ASan instrumentation pass (http://llvm.org/viewvc/llvm-project/llvm/trunk/lib/Transforms/Instrumentation/AddressSanitizer.cpp?view=markup) are related to handling ObjC variables and functions.

**`.cstring` section.** All the globals put into this section are compressed at link time: the linker removes all the spare \0 symbols after the string terminator, thus breaking the redzones. We do not instrument these globals.

**`__OBJC` section and `__DATA,_objc_` segment.** The ObjC runtime assumes that the global objects from these places conform to /usr/include/objc/runtime.h, so we can’t add redzones to them either.

**Constant CFString instances.** Each constant CFString consists of two parts: the string buffer is emitted into the `__TEXT,__cstring,cstring_literals` section, and the constant NSConstantString structure referencing that buffer is placed into `__DATA,__cfstring`.
There’s no point in adding redzones to that NSConstantString structure, and it causes the linker to crash on OS X 10.7 (see http://code.google.com/p/address-sanitizer/issues/detail?id=32), so we do not instrument those, too.

**ObjC load methods.** An ObjC class may have a +load method, which is called at the time the module is loaded into the memory by dyld. This is pretty early, and at this moment `__asan_init()` may have not been called yet. To avoid the pre-initialization crashes, we insert a call to `__asan_init()` at the beginning of each function which name contains `“load]”`.

Generally speaking, it’s possible to compile ObjC code on Linux, so there is nothing specific to Mac OS here. But so far these features were tested on Mac only, as long as they were needed only for it.

### Compiling and linking ###

In the case when `-faddress-sanitizer` is passed to the linker when building an executable binary, we link it with `libclang_rt.asan_osx.a`, the ASan runtime library (a fat binary containing both 32- and 64-bit versions of our runtime), and the CoreFoundation framework, on which the runtime library depends (see http://llvm.org/viewvc/llvm-project/cfe/trunk/lib/Driver/ToolChains.cpp?view=markup).
If one links dynamic libraries or Mach-O bundles (selected by the `-dynamiclib` and `-bundle` command line flags), the resulting binary will probably lack the symbols provided by the runtime library (at least there’ll be no `__asan_init()`). We mark those symbols as dynamic\_lookup, so that the linking succeeds (see http://llvm.org/viewvc/llvm-project/cfe/trunk/lib/Driver/Tools.cpp?view=markup). This may result in hiding problems with unresolved symbols in other libraries, but those will be detected at runtime or when doing a regular build without ASan.

## Runtime library ##

### Early initialization ###

Unlike `ld.so`, `dyld` on Mac OS does not support the `.preinit_array` section (the way we’re calling `__asan_init()` before any of the static constructors is invoked on Android).
For the sake of user experience, we deliberately do not want to use any wrappers and preloaded libraries to jump in early, so the only remaining option is to instrument each static constructors’ section (see above).
Another approach worth to consider is to patch the resulting executable after it has been linked. This may work, but will certainly complicate the tool usage.

### Overriding functions ###

Because of the two-level namespaces used by many applications, it is generally impossible to reliably intercept the functions by providing replacements for them and obtaining the original function pointers via `dlsym()`. This is why we’re using function patching based on the mach\_star framework (https://github.com/rentzsch/mach_star/tree/master/mach_override).
We had to make some changes to the `mach_override.{c,h}` in order to make it fit to our needs:
externalize the functions that allocate and deallocate the branch islands, because AddressSanitizer adds some limitations on their placement (e.g. they should not overlap with the shadow regions or be more than 2 Gb far from the functions being intercepted). This has sped up the program execution significantly, because previously the library used to probe the virtual memory space looking for a free page, which resulted in 3M calls to `vm_allocate()` per program run (http://code.google.com/p/address-sanitizer/issues/detail?id=24);
add a number of instructions unsupported by the trunk mach\_override, including support for short jump instructions, ja and je (http://code.google.com/p/address-sanitizer/issues/detail?id=53);
because the client library may include its own copy of mach\_override library, we need to make sure it is named differently (otherwise mutual calling of instrumented and non-instrumented functions may result in problems, see http://code.google.com/p/address-sanitizer/issues/detail?id=22)

After having implemented mach\_override-based function patching (and having written the above text) we’ve found out that Mac OS X allows another method of function interposing based on the `“__DATA,__interpose”` section in the library (http://code.google.com/p/address-sanitizer/issues/detail?id=64). However there is a drawback that currently prevents us from employing this technique: looks like it relies on `DYLD_INSERT_LIBRARIES` (analog of `LD_PRELOAD` on Linux), which we currently do not want to use.

### Hooking the memory allocations ###

Replacing the memory allocation routines is trickier on Mac OS than on Linux, because there can be several heap allocators in a program, and each of them has to be intercepted.
Mac OS uses so-called malloc zones, which are described in `/usr/include/malloc/malloc.h`
Each malloc zone has a corresponding malloc\_zone\_t instance describing it.
This is a data structure that contains pointers to interface functions like `malloc_zone_malloc()`, `malloc_zone_calloc()`, `malloc_zone_free()` (similar to `malloc()`/`calloc()`/`free()`, but with an additional `malloc_zone_t` parameter).

```
typedef struct _malloc_zone_t {
    void        *reserved1;
    void        *reserved2;
    size_t      (*size)[...]
    void        *(*malloc)[...]
    void        *(*calloc)[...]
    void        *(*valloc)[...]
    void        (*free)[...]
    void        *(*realloc)[...]
    void        (*destroy)[...]
    const char  *zone_name;
    unsigned    (*batch_malloc)[...]
    void        (*batch_free)[...]
    struct malloc_introspection_t *introspect;
    unsigned    version;
    void *(*memalign)(struct _malloc_zone_t *zone, size_t alignment, size_t size);
    void (*free_definite_size)(struct _malloc_zone_t *zone, void *ptr, size_t size);
    size_t      (*pressure_relief)[...]
} malloc_zone_t;
```

Unlike on Linux, it is possible to ask if the zone owns a particular allocation and what is its size.
This theoretically allows to find the owner of a given allocation by calling `malloc_zone_from_ptr()`. However not all the system allocators implement the `size()` function correctly. Because of this it is impossible to tell whether a particular address has been allocated by some unknown memory zone (this is not a problem on Linux, where there’s typically only one allocator), or if it is a random address. We have to disable reporting double frees on Mac to avoid false positives.

Our code replacing the malloc zones (http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/asan/asan_malloc_mac.cc?view=markup) was mainly taken from Google Perftools (http://code.google.com/p/gperftools/source/browse/trunk/src/libc_override_osx.h). The main difference is that we’re replacing both the default malloc zone and the default CFAllocator (doing so requires additional magic with constant strings, see below).

We do not do anything with the garbage collected allocators.

### Threading, TLS and Grand Central Dispatch ###

AddressSanitizer needs to keep track of the threads that exist during the program execution.
For every thread an `AsanThreadSummary` object is created.

For historical reasons (because Clang did not support the `__thread` keyword on Mac OS prior to 10.7), we are using thread-specific data (TSD) provided by libpthread. In order to minimize the differences between ASan on Linux and Mac we’re using TSD on Linux as well. This approach has a drawback: data in the TSD is destroyed when the thread is exited, and the order of destruction of the different TSD pieces is undefined. Previously we’ve used to ensure our destructor was the very last one, but now we’re deliberately leaking the corresponding AsanThreadSummary object on all the platforms to guarantee that it outlives its thread.

Starting at OS X 10.6 Apple introduced Grand Central Dispatch, a mechanism for running parallel tasks on a thread pool. The threads in that pool are created by bsdthread\_create, the same system call used by `pthread_create()`. But intercepting a system call is harder than intercepting a function, so we choose to wrap a number of libdispatch API functions instead of that. This allows us to hijack the callbacks sent to the worker threads and tell the runtime library that a new thread has appeared.

### File mappings and symbolization ###

AddressSanitizer sometimes needs to know the list of memory mappings in the process address space. There are two cases when it is necessary: to detect which image does a stack frame belong to, and to check whether any of the mapped libraries overlap with the shadow memory range at startup (this is possible if ASLR is on, see below).

Initially we had been using an interface class from gperftools (http://code.google.com/p/gperftools/) that queried /proc/self/maps on Linux and emulated the same behaviour on Mac using the dyld API (_dyld\_get\_image_{name,header,vmaddr\_slide}()). This means we can only get the list of images mapped by the process, not all the existing mappings. However this hadn’t been an issue, because the mappings that do not correspond to binary images typically do not contain program code (ASan can’t symbolize JITted code), and there are no such mappings at program startup.

We've rewritten the gperftools code heavily now (http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/asan/asan_procmaps.h?view=markup), but it is still using the same API.

Currently AddressSanitizer uses out-of-process symbolization performed by a Python script, asan\_symbolize.py, which looks for object+offset pairs in the crash logs and replaces them with symbol:file:line triples obtained from addr2line on Linux or atos on Mac OS. Because the report may contain stacks of memory allocation/deallocation that had happened in the past, it sometimes may be impossible to find the binary that contains a particular address (e.g. if it was in a library that had already been unloaded). We are aware of this problem and know how to fix it, but it hadn’t been noticed in the wild yet.

### Runtime flags ###

Runtime flags that affect ASan behavior can be passed via the `ASAN_OPTIONS` env var, e.g. `ASAN_OPTIONS=verbosity=1 ./program_name`. Mac OS-specific flags are listed below.
  * `replace_cfallocator` (bool, default=1): replace the default CFAllocator. Since doing this correctly is rather complicated (see “Various problems”), new bugs in ASan can be sometimes worked around by setting `replace_cfallocator=0`;
  * `mac_ignore_invalid_free` (bool, default=0): when an invalid pointer is being freed (currently via CFAllocatorDeallocate() or malloc\_zone\_free() only), print a warning instead of crashing. This also may be useful to work around newer bugs.

### Various problems ###

#### `memcpy()` and `memmove()` ####

AddressSanitizer wraps `memcpy()` and `memmove()` in order to check that the memory regions being accessed are addressable, and that the parameters of `memcpy()` do not overlap.
It turns out that on Mac OS 10.7 `memcpy()` and `memmove()` are the same function (see http://code.google.com/p/address-sanitizer/issues/detail?id=34), so wrapping both of them makes us do the checks twice and report overlapping parameters for `memmove()` as well.
To avoid this, we wrap only `memmove()` on non-Snow Leopard systems (http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/asan/asan_mac.cc?view=markup).

#### Address space layout randomization (ASLR) ####

Address space layout randomization is a security method that prevents attacks relying on a particular addresses of the application and library segments. Because the application is loaded before `__asan_init()` is called, any of its segments may intersect with the shadow memory ranges, which is likely to result in incorrect behavior (the `mmap()` and `mprotect()` calls setting up the memory will corrupt the data or the code of the app). Because AddressSanitizer is intended to be used by software developers, it should be safe to disable ASLR for the program built with AddressSanitizer.

On OS X 10.6 ASLR is really aggressive, so even the smallest 32-bit application is very likely to intersect with the shadow. The randomization can be disabled with the DYLD\_NO\_PIE env var, which is being set manually by the users (http://code.google.com/p/address-sanitizer/issues/detail?id=29 is the corresponding bug). We haven’t faced any problems with ASLR on 10.7, thus we didn’t need to disable it, although there is a known method to do so: http://reverse.put.as/2011/08/11/how-gdb-disables-aslr-in-mac-os-x-lion/

#### Slow shutdown on 64-bit apps ####

AddressSanitizer needs to mmap one eighth of the address space available to the client program, which is 16 Tb on the 64-bit systems. Mapping such a big amount of memory bloats the virtual page table and causes the page lookup operations to take more time. As a result, the process shutdown may take up to half a second (and probably more on slower machines). This should not be a big problem for real world applications, but our test suite that forks hundreds of child processes becomes rather slow.
We’ve reported the bug to Apple, see http://openradar.appspot.com/10699643.

#### `CFStringCreateCopy()` ####

`CFStringCreateCopy()` is a function for copying CFString instances. Normally it does not copy constant strings (http://opensource.apple.com/source/CF/CF-476.19/CFString.c), but if the default CoreFoundation allocator is replaced using `CFAllocatorSetDefault()`, it starts to. We believe this is incorrect (Apple Radar [bug 11164715](https://code.google.com/p/address-sanitizer/issues/detail?id=1164715) filed, see also http://openradar.appspot.com/11164715), but since AddressSanitizer replaces the default allocator, we still have to deal with it (http://code.google.com/p/address-sanitizer/issues/detail?id=10)
We wrap `CFStringCreateCopy()` in the ASan runtime library and check if its second argument is a constant string. If it is, we return it as is, otherwise the original `CFStringCreateCopy()` is called.

#### Wrapping `__CFInitialize()` ####

For some applications (e.g. Chromium built in the shared library mode) either `CFAllocatorSetDefault()` (on Snow Leopard) or `CFAllocatorCreate()` (on Lion) called from `__asan_init()` may crash because the default system allocator hasn’t been initialized properly. To avoid this, we have to replace the default CFAllocator strictly after `__CFInitialize()` has been called already. We wrap `__CFInitialize()` and check whether `kCFAllocatorSystemDefault` is initialized in `__asan_init()`. If it is, the default allocator is replaced in `__asan_init()`, otherwise it’s done in the `__CFInitialize()` wrapper.

See http://code.google.com/p/address-sanitizer/issues/detail?id=87 for more details.