# What is it? #
**Static initialization order fiasco** is a common issue in C++ programs relating to the order in which global objects are constructed.  The order in which constructors for global objects in different source files run is unspecified. [Sections 10.14-10.18](http://www.parashift.com/c++-faq/static-init-order.html) of Parashift C++ FAQ offers a pretty good explanation in of what the problem is, and what can be done to correct it. Here is a short example:

```
$ cat tmp/init-order/example/a.cc
#include <stdio.h>
extern int extern_global;
int __attribute__((noinline)) read_extern_global() {
  return extern_global;
}
int x = read_extern_global() + 1;
int main() {
  printf("%d\n", x);
  return 0;
}

$ cat tmp/init-order/example/b.cc
int foo() { return 42; }
int extern_global = foo();
```

Here the value if `x` depends on the value of `extern_global`, which may or may not be initialized with "42" depending on the order in which the initializers for translation units run:
```
$ clang++ tmp/init-order/example/a.cc tmp/init-order/example/b.cc && ./a.out 
1
$ clang++ tmp/init-order/example/b.cc tmp/init-order/example/a.cc && ./a.out 
43
```

Such bugs are hard to spot and may stay unnoticed until some irrelevant changes in the code (or compiler, or link strategy) change the code behavior, often breaking it in an unexpected way.

# Initialization-order checker in AddressSanitizer #
AddressSanitizer inserts checks into the compiled program aimed at detecting initialization-order problems. They are turned off by default,
and you have to pass [run-time flags](https://code.google.com/p/address-sanitizer/wiki/Flags) to enable them. See [example usage](https://code.google.com/p/address-sanitizer/wiki/ExampleInitOrderFiasco).

## Loose init-order checking ##
This mode reports an error if initializer for a global variable accesses dynamically initialized global from another translation unit, **which is not yet initialized**.
Use `check_initialization_order=true` to activate it:
```
$ clang++ -fsanitize=address -g tmp/init-order/example/a.cc tmp/init-order/example/b.cc
$ ASAN_OPTIONS=check_initialization_order=true ./a.out
=================================================================
==26772==ERROR: AddressSanitizer: initialization-order-fiasco on address 0x000001068820 at pc 0x427e74 bp 0x7ffff8295010 sp 0x7ffff8295008
READ of size 4 at 0x000001068820 thread T0
    #0 0x427e73 in read_extern_global() tmp/init-order/example/a.cc:4
    #1 0x42806c in __cxx_global_var_init tmp/init-order/example/a.cc:7
    #2 0x4280d5 in global constructors keyed to a tmp/init-order/example/a.cc:10
    #3 0x42823c in __libc_csu_init (a.out+0x42823c)
    #4 0x7f9afdbdb6ff (/lib/x86_64-linux-gnu/libc.so.6+0x216ff)
    #5 0x427d64 (a.out+0x427d64)
0x000001068820 is located 0 bytes inside of global variable 'extern_global' from 'tmp/init-order/example/b.cc' (0x1068820) of size 4
SUMMARY: AddressSanitizer: initialization-order-fiasco tmp/init-order/example/a.cc:4 read_extern_global()
<...>
```

In some sense, this mode reports existing problems. It may not report an error if the order of initialization changes:
```
$ clang++ -fsanitize=address -g tmp/init-order/example/b.cc tmp/init-order/example/a.cc
$ ASAN_OPTIONS=check_initialization_order=true ./a.out
43
```

## Strict init-order checking ##
This mode reports an error if initializer for a global variable accesses **any** dynamically initialized global from another translation unit. Use additional variable `strict_init_order=true` to activate it:
```
$ clang++ -fsanitize=address -g tmp/init-order/example/b.cc tmp/init-order/example/a.cc
$ ASAN_OPTIONS=check_initialization_order=true:strict_init_order=true ./a.out
=================================================================
==27853==ERROR: AddressSanitizer: initialization-order-fiasco on address 0x0000010687e0 at pc 0x427f74 bp 0x7fff3d076ba0 sp 0x7fff3d076b98
READ of size 4 at 0x0000010687e0 thread T0
    #0 0x427f73 in read_extern_global() tmp/init-order/example/a.cc:4
<...>
```

In this way you may also find potential initialization-order problems.

## False positives ##
  * strict init-order checking may report false positives when the access to already-initialized globals from another translation units is expected, or when the specific order of construction is enforced by, e.g. using shared libraries.
  * loose init-order checking may report false positives on dynamically initialized globals, which can still be safely accessed before initialization (e.g. if their constructor does nothing).

## Blacklist ##
You may suppress false positives of init-order checker on certain global variables by using the `-fsanitize-blacklist=path/to/blacklist.txt` option. Relevant blacklist entries are:
```
# Disable init-order checking for a single variable:
global:bad_variable=init
# Disable checking for all variables of a given type:
type:Namespace::ClassName=init
# Disable checking for all variables in given files:
src:path/to/bad/files/*=init
```

## Performance ##
Init-order slows down the program startup. Its complexity is `O(NM)`, where `N` is the total number of dynamically initialized global variables in the binary, and `M` is the total number of translation units.