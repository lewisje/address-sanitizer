
```
$ cat tmp/init-order/example/a.cc
extern int extern_global;
int __attribute__((noinline)) read_extern_global() {
  return extern_global;
}
int x = read_extern_global() + 1;
int main() { return 0; }


$ cat tmp/init-order/example/b.cc
int foo() { return 42; }
int extern_global = foo();


$ clang++ -O -g -fsanitize=address tmp/init-order/example/a.cc tmp/init-order/example/b.cc
$ ASAN_OPTIONS=check_initialization_order=true ./a.out
```
```
=================================================================
==2810==ERROR: AddressSanitizer: initialization-order-fiasco on address 0x0000010665a0 at pc 0x4262f5 bp 0x7fffc8b8c510 sp 0x7fffc8b8c508
READ of size 4 at 0x0000010665a0 thread T0
    #0 0x4262f4 in read_extern_global() tmp/init-order/example/a.cc:3
    #1 0x42636f in __cxx_global_var_init tmp/init-order/example/a.cc:6
    #2 0x42636f in global constructors keyed to a tmp/init-order/example/a.cc:8
    #3 0x4264ac in __libc_csu_init (a.out+0x4264ac)
    #4 0x7f852d1bb6ff (/lib/x86_64-linux-gnu/libc.so.6+0x216ff)
    #5 0x426204 (a.out+0x426204)
0x0000010665a0 is located 0 bytes inside of global variable 'extern_global' from 'tmp/init-order/example/b.cc' (0x10665a0) of size 4
SUMMARY: AddressSanitizer: initialization-order-fiasco tmp/init-order/example/a.cc:3 read_extern_global()
Shadow bytes around the buggy address:
  0x000080204c60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204c70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204c80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204c90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204ca0: 00 00 00 00 00 00 00 00 00 00 00 00 04 f9 f9 f9
=>0x000080204cb0: f9 f9 f9 f9[f6]f6 f6 f6 f6 f6 f6 f6 00 00 00 00
  0x000080204cc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204cd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204ce0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204cf0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x000080204d00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:     fa
  Heap right redzone:    fb
  Freed heap region:     fd
  Stack left redzone:    f1
  Stack mid redzone:     f2
  Stack right redzone:   f3
  Stack partial redzone: f4
  Stack after return:    f5
  Stack use after scope: f8
  Global redzone:        f9
  Global init order:     f6
  Poisoned by user:      f7
  ASan internal:         fe
==2810==ABORTING
```
Read CallStack about symolizing callstack