# Compiler flags #
| flag| default | description |
|:----|:--------|:------------|
| -fsanitize=address |         | Enable AddressSanitizer |
| -fno-omit-frame-pointer |         | Leave frame pointers. Allows the fast unwinder to function properly. |
| -fsanitize-blacklist=path |         | Pass a [blacklist file](#Turning_off_instrumentation.md) |
| -fno-common |         | Do not treat global variable in C as common variables (allows ASan to instrument them) |

ASan-specific compile-time flags are passed via clang flag `-mllvm <flag>`. In most cases you don't need them.
| flag| default | description |
|:----|:--------|:------------|
| -asan-stack   | 1       | Detect overflow/underflow for stack objects |
| -asan-globals | 1       | Detect overflow/underflow for global objects |

# Run-time flags #

Most run-time flags are passed to AddressSanitizer via `ASAN_OPTIONS` environment variable like this:
```
ASAN_OPTIONS=verbosity=1:malloc_context_size=20 ./a.out
```

Note that below list may (and probably is) incomplete. Also older versions of ASan may not support some of the listed flags. To get the idea of what's supported in your version, run
```
ASAN_OPTIONS=help=1 ./a.out
```

| **Flag** | **Default value** | **Description** |
|:---------|:------------------|:----------------|
| verbosity  | 0                 | Be more verbose (mostly for testing the tool itself) |
| malloc\_context\_size | 30                | Number of frames in `malloc`/`free` stack traces. Valid values are 0...256.  |
| redzone  | 16                | Size of the minimal redzone. Since LLVM 3.3 asan uses adaptive redzones for heap, i.e. for large heap allocations the redzones are larger. |
| log\_path | (none)            | Path to the log files. If log\_path=PATH is given, every process will write error reports to PATH.PID. By default all reports are written to stderr. |
| sleep\_before\_dying | 0                 | Sleep for this number of seconds before exiting the process on failure. |
| quarantine\_size\_mb | 256               | Size of quarantine (in Mb) for finding use-after-free errors. Lower values save memory but increase false negatives rate. |
| quarantine\_size | 0                 | Deprecated since [r225337](http://llvm.org/viewvc/llvm-project?rev=225337&view=rev), use quarantine\_size\_mb |
| fast\_unwind\_on\_fatal | 0                 | Use fast unwinder when found a bug. The fast one relies on frame pointers, while slow one does not. See also CallStack |
| fast\_unwind\_on\_malloc | 1                 | Same as `fast_unwind_on_fatal`, but control unwinding inside `malloc`/`free`, i.e. performance critical. See also CallStack |
| exitcode | 1                 | Call `_exit(exitcode)` on error.  |
| abort\_on\_error | 0                 | If `1`, call `abort()` instead of `_exit(exitcode)` on error.|
| allow\_user\_poisoning  | 1                 | Allow/disallow ManualPoisoning |
| strict\_memcmp | 1                 | When set to `1` (default), `memcmp("foo", "bar", 100)` is treated as a bug |
| allocator\_may\_return\_null | 0                 | If false, the allocator will crash instead of returning 0 on out-of-memory. |
| alloc\_dealloc\_mismatch | 1                 | When set to `1`, checks for malloc()/new/new[.md](.md) vs. free()/delete/delete[.md](.md) mismatches. Default: 0 on Mac and Windows, 1 otherwise |
| detect\_stack\_use\_after\_return | 0                 | If 1, will try to detect UseAfterReturn errors |
| min\_uar\_stack\_size\_log | 16                | Controls the size of Fake Stack for UseAfterReturn detector |
| max\_uar\_stack\_size\_log | 20                | Controls the size of Fake Stack for UseAfterReturn detector |
| debug    | 0                 | If set, prints some debugging information and does additional checks. |
| disable\_core | 0/1               | Disable the core dumper. Since asan consumes many terabytes of virtual memory on 64-bit, dumping core is unwise. Default: 0 on 32-bit and 1 on 64-bit. |
| use\_madv\_dontdump | 1                 | Instructs kernel to not store the (huge) shadow in core file. |
| handle\_segv | 1                 | When set to 1, AddressSanitizer installs its own handler for SIGSEGV |
| full\_address\_space | 0/1               | Sanitize complete address space; by default kernel area on 32-bit platforms will not be sanitized. |
| allow\_user\_segv\_handler | 0                 | When set to 1, allows user to override SIGSEGV handler installed by ASan |
| check\_initialization\_order | 0                 | When set to 1, detect existing [InitOrderFiasco](ExampleInitOrderFiasco.md) problems. |
| report\_globals | 1                 | Controls the way to handle globals (0 - don't detect buffer overflow on globals, 1 - detect buffer overflow, 2 - print data about registered globals). |
| start\_deactivated | 0                 | If true, ASan tweaks a bunch of other flags (quarantine, redzone, heap poisoning) to reduce memory consumption as much as possible, and restores them to original values when the first instrumented module is loaded into the process. This is mainly intended to be used on Android. |
| strict\_init\_order | 0                 | When set to 1, [InitOrderFiasco](ExampleInitOrderFiasco.md) also finds potential init order problems. |
| strip\_path\_prefix | ""                | When `strip_path_prefix=PREFIX` the substring `.*PREFIX` will be removed from the reported file names. |
| detect\_leaks | 1                 | If 1, enables memory leak detection. See LeakSanitizer (Linux/x86\_64-only). |
| use\_sigaltstack | 1                 | If 1, `sigaltstack` is called at start up time. This will provide verbose reports in case of stack overflow.  |
| color    | auto              | Print reports in color. Possible values: auto|always|never. By default, color is enabled if printing to terminal. |
| coverage | 0                 | Enables coverage collection. See AsanCoverage. |
| coverage\_dir | ./                | Controls the path at which coverage is saved. |
| coverage\_direct | 0                 | Enables writing coverage directly to a memory-mapped file (compared to once at the end of the program). See AsanCoverage. |
| coverage\_pcs | 1                 | If 1, will dump the coverage as PCs. See AsanCoverage. |
| coverage\_bitset | 0                 | If 1, will dump the coverage as bitset. See AsanCoverage. |
| detect\_odr\_violation | 2                 | If > 0, detect [ODR Violations](OneDefinitionRuleViolation.md) |
| stack\_frame\_format | "DEFAULT"         | Format of the stackframe. Placeholders in user-provided string are replaced with actual data. See  [sanitizer\_stacktrace\_printer.h](http://llvm.org/viewvc/llvm-project/compiler-rt/trunk/lib/sanitizer_common/sanitizer_stacktrace_printer.h?view=markup) for format description. |
| mmap\_limit\_mb | 0                 | If > 0, abort if more than this number of megabytes were mmap-ed by user. Experimental, may change in future! |
|          |                   |                 |
|hard\_rss\_limit\_mb| 0                 | Experimental. Hard RSS limit in Mb. If non-zero, a background thread is spawned at startup which periodically reads RSS and aborts the process if the limit is reached |
| soft\_rss\_limit\_mb | 0                 | Experimental. Soft RSS limit in Mb. If non-zero, a background thread is spawned at startup which periodically reads RSS. If the limit is reached all subsequent malloc/new calls will fail or return NULL (depending on the value of allocator\_may\_return\_null) until the RSS goes below the soft limit. This limit does not affect memory allocations other than malloc/new. |
| include  | (none)            | Parse more flags from the file at a given path. File contents are parsed exactly as if they replaced the include=/path flag. This flag can be used multiple times. Nested includes are supported as well. |