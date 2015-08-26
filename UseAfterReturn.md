
# Introduction #

**Stack-use-after-return** bug appears when a stack object is used after
the function where this object is defined has returned.
Example (see also ExampleUseAfterReturn):
```
int *ptr;
void FunctionThatEscapesLocalObject() {
  int local[100];
  ptr = &local[0];
}
// *ptr is used later
```

AddressSanitizer currently does not attempt to detect these bugs by default,
only with an additional flag run-time: ASAN\_OPTIONS=detect\_stack\_use\_after\_return=1 (Clang only, starting from [r191186](https://code.google.com/p/address-sanitizer/source/detail?r=191186))

# Algorithm #
Detection of stack-use-after-return is similar to detection of heap-use-after-free,
but the **quarantine** should be implemented in a different way.

Once a function has returned, its stack memory is reused by the next call instruction.
So in order to implemented quarantine for the stack memory we need to promote stack to heap.
The current implementation does it like this:

Before:
```
void foo() {
  int local;
  escape_addr(&local);
}
```
After:
```
void foo() {
  char redzone1[32];
  int local;
  char redzone2[32+28];
  char *fake_stack = __asan_stack_malloc(&local, 96);
  poison_redzones(fake_stack);  // Done by the inlined instrumentation code.
  escape_addr(fake_stack + 32);
  __asan_stack_free(stack, &local, 96)
}
```

**`__asan_stack_malloc(real_stack, frame_size)`** allocates a **fake frame**
(`frame_size` bytes) from a thread-local heap-like structure (**fake stack**).
Every fake frame comes unpoisoned and then the redzones are poisoned in the instrumented
function code.

**`__asan_stack_free(fake_stack, real_stack, frame_size)`**
poisons the entire fake frame and deallocates it.

# Memory consumption #

The **Fake Stack** allocator uses a fixed amount of memory per each thread.
The allocator has 11 size classes (from `2**6` bytes to `2**16` bytes),
every size class has fixed amount of chunks.
If the given size class is fully used, the allocator will return 0 and thus regular stack will be used
(i.e. stack-use-after-return detection will not work for the given function call).
The amount of memory given to every size class is proportional to the size of thread's real stack,
but not more than `2**max_uar_stack_size_log` (by default, `2**20`)
and not less than `2**min_uar_stack_size_log` (by default, `2**16`). See also: [Flags](Flags.md).

So, with the default 8Mb stack size and default [Flags](Flags.md) each size class will get `2**20` bytes and thus every thread will mmap ~11Mb for Fake Stack.

The bigger the Fake Stack the better your chances to catch a stack-use-after-return and get a correct report, but the greater is the memory consumption.

# Performance #
Detecting stack-use-after-return is expensive in both CPU and RAM:
  * Allocating fake frames introduces two function calls per every function with non-empty frame.
  * The entire fake frames should be unpoisoned on entry and poisoned on exit, as opposed to poisoning just the redzones in the default mode.

These are the performance numbers on SPEC 2006. We've compared pure asan against ASAN\_OPTIONS=detect\_stack\_use\_after\_return=1 both with `-fsanitize=address -O2`.
4 benchmarks get 30% extra slowdown and one gets almost 2x extra slowdown. Other 14 benchmarks are not affected at all.
|       400.perlbench|      1269.00|      1653.00|         **1.30**|
|:-------------------|:------------|:------------|:----------------|
|           401.bzip2|       845.00|       847.00|         1.00    |
|             403.gcc|       611.00|       629.00|         1.03    |
|             429.mcf|       582.00|       581.00|         1.00    |
|           445.gobmk|       841.00|      1093.00|         **1.30**|
|           456.hmmer|       902.00|       924.00|         1.02    |
|           458.sjeng|       996.00|      1244.00|         **1.25**|
|      462.libquantum|       541.00|       511.00|         0.94    |
|         464.h264ref|      1266.00|      1288.00|         1.02    |
|         471.omnetpp|       567.00|       564.00|         0.99    |
|           473.astar|       636.00|       642.00|         1.01    |
|       483.xalancbmk|       465.00|       595.00|         **1.28**|
|            433.milc|       651.00|       652.00|         1.00    |
|            444.namd|       599.00|       597.00|         1.00    |
|          447.dealII|       616.00|       610.00|         0.99    |
|          450.soplex|       358.00|       363.00|         1.01    |
|          453.povray|       432.00|       821.00|         **1.90**|
|             470.lbm|       384.00|       378.00|         0.98    |
|         482.sphinx3|       930.00|       927.00|         1.00    |


# Garbage collection #
We have an experimental API to support interoperability of AddressSanitizer's fake stack with garbage collection.
See http://llvm.org/viewvc/llvm-project?view=revision&revision=200908 and stay tuned for more news.

# Compatibility #
The fake stack may be incompatible with some low-level code that
uses certain assumptions about the stack memory layout.
  * Code that takes an address of a local variable and assumes the variable is localed on the real stack.
  * Implementations of C++ garbage collection may require special attention. See above.

TODO