AddressSanitizer collects call stacks on the following events:
  * `malloc` and `free`
  * thread creation
  * failure

`malloc` and `free` happen relatively frequently and it is important to unwind the call stack fast.
AddressSanitizer uses a simple unwinder that relies on frame pointers.

If you don't care about `malloc`/`free` call stacks, simply disable the unwinder completely
(use `malloc_context_size=0` [runtime flag](Flags.md)).

Each stack frame needs to be symbolized (of course, if the binary is compiled with debug info).
Given a PC, we need to print
```
  #0xabcdf function_name file_name.cc:1234
```
<a href='Hidden comment: 
[http://llvm.org/bugs/show_bug.cgi?id=7554 LLVM issue].
'></a>

AddressSanitizer uses [llvm-symbolizer](http://llvm.org/docs/CommandGuide/llvm-symbolizer.html) binary from Clang 3.3+ distribution to symbolize the stack traces. Just make sure llvm-symbolizer is in `PATH` before running the binary or provide it in separate `ASAN_SYMBOLIZER_PATH` environment variable:
```
export ASAN_SYMBOLIZER_PATH=/path/to/llvm_build/bin/llvm-symbolizer
./a.out
```

If you want to disable symbolization for some reason, you may do so by providing empty string as `ASAN_SYMBOLIZER_PATH` value:
```
ASAN_SYMBOLIZER_PATH= ./a.out
```

You may also filter the log file through `scripts/asan_symbolize.py` to get the symbols. This script takes an optional parameter -- a file prefix. The substring `.*prefix` will be removed from the file names.
```
% ./a.out
    ...
    #0 0x402c77 (/home/you/address-sanitizer/asan/a.out+0x402c77)
    ...
% ./a.out 2>&1  | ../scripts/asan_symbolize.py
   ...
   #0 0x402c77 in main /home/you/address-sanitizer/asan/use-after-free.c:5
   ...
% ./a.out 2>&1  | ../scripts/asan_symbolize.py /you/
   ...
   #0 0x402c77 in main address-sanitizer/asan/use-after-free.c:5
   ...
```
To demangle functions names either add `-d` to `asan_symbolize.py` or use `c++filt`.

You may want to introduce your own format of the stack traces using `stack_trace_format` [runtime flag](Flags.md). For example:
```
% ./a.out
    ...
    #0 0x4b615d in main /home/you/use-after-free.cc:12:3
    ...
% ASAN_OPTIONS='stack_trace_format="[frame=%n, function=%f, location=%S]"' ./a.out
    ...
    [frame=0, function=main, location=/home/you/use-after-free.cc:12:3]
```