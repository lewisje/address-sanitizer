# Introduction #

LeakSanitizer is a heap leak detector which is designed to run on top of AddressSanitizer / MemorySanitizer, or as a standalone tool.

# Operation #

LSan is a run-time tool which doesn't require any compiler instrumentation. The tool consists of two components: the leak checking module itself and the run-time environment, which includes an allocator, a thread registry, and interceptors for memory allocation / thread management functions. These are common components shared by most sanitizer tools, so eventually we should be able to run the leak checking module on top of ASan / MSan instead of using a dedicated environment.

The leak checker module is inactive until the very end of a process' lifetime. When invoked, it first halts the execution of the process. This ensures that the pointer graph does not change while we examine it, and allows us to collect any transient pointers from register contexts of running threads. To this effect, we run the leak checker as a separate task that attaches to the parent process with ptrace.

The next step is obtaining the root set of live memory. It must include at least the following:
  * global variables,
  * stacks of running threads,
  * general-purpose registers of running threads,
  * ELF thread-local storage and POSIX thread-specific data.

LSan scans this memory for byte patterns that look like pointers to heap blocks (interior pointers are treated the same as pointers to the beginning of a block). Those blocks are considered reachable and their contents are also treated as live memory. In this manner we discover all blocks reachable from the root set. We mark such blocks by setting a flag in the block's metadata. We then iterate over all existing heap blocks and report unreachable blocks as leaks, together with stack traces of corresponding allocations. Instead of reporting each allocation individually, it might be more useful to aggregate them by the last k frames in the stack trace.

Another useful feature is being able to distinguish between directly leaked blocks (not reachable from anywhere) and indirectly leaked blocks (reachable from other leaked blocks). This could be implemented as another pass over the set of leaked blocks, and the reachability flag would have to take 3 states instead of 2.

## Finding the root set ##

  * Global regions are obtained from `dl_iterate_phdr()`,
  * stack ranges are obtained from `pthread_getattr_np()`,
  * a thread's static TLS area can be found using the internal glibc function `_dl_get_tls_static_info()` (like in TSan). On x86 and x86-64 this area includes the thread descriptor, which contains pointers to thread-specific data. On the other hand, dynamically allocated TLS blocks can't always be reached easily through the thread descriptor. We handle them by intercepting the `__libc_memalign` calls that the linker uses to allocate dynamic TLS space, and considering all blocks allocated from the linker to be live.

## Suppression of known leaks ##

There are several ways by which a user might want to specify which leaks are to be ignored:
  * via Valgrind-like suppression files,
  * by calling a function which accepts a pointer to a specific leaked object (i.e. `SuppressObject()`),
  * by suppressing all allocations made between two execution points (i.e. between calls to `StartSuppressingLeaks()` / `StopSuppressingLeaks()`).
LSan could eventually support all of them. Suppressed blocks would have to be marked by setting the reachability flag to yet another state.

## Coupling LSan with other sanitizer tools ##

Thanks to a common allocator architecture, ASan and MSan should be able to accomodate leak checking with minimal changes. Both tools already record everything LSan needs to know about a heap block (e.g. stack trace). The only addition that must be made to the tool-specific block metadata is a flag describing the reachability state. This flag would be unused until leak checking begins. LSan would access tool-specific metadata through a proxy class LsanMetadata, which would be implemented differently in each tool.

LSan also needs to know some per-thread information, such as the address ranges occupied by stack, TLS and any thread-local internal data structures which should be excluded from the root set. Because this information needs to be recorded only once per thread, the cost of this change would be minimal.

At the point when threads are halted, some of them might be in the middle of a malloc/free call. Thus, care should be taken to avoid data races on block metadata. Because LSan ignores blocks that are not allocated to the user, the only write that has to be done atomically in the parent tool is to the "allocated" flag. The rest of the metadata can be modified non-atomically, as long as the block is not in the "allocated" state.