## gdb ##
You can use [gdb](http://www.gnu.org/software/gdb/) with
binaries built by AddressSanitizer in a usual way.
When AddressSanitizer finds a bug it calls one of the functions `__asan_report_{load,store}{1,2,4,8,16}`
which in turn calls `__asan_report_error`.
If you want gdb to stop before asan reports an error, set a breakpoint on `__asan_report_error`. 

&lt;BR&gt;


If you want gdb to stop after asan has reported an error, set a breakpoint on `AsanDie` or use `ASAN_OPTIONS=abort_on_error=1`.

Inside gdb you can ask asan to describe a memory location:
```
(gdb) set overload-resolution off
(gdb) p __asan_describe_address(0x7ffff73c3f80)
0x7ffff73c3f80 is located 0 bytes inside of 10-byte region [0x7ffff73c3f80,0x7ffff73c3f8a)
freed by thread T0 here: 
...
```

TODO