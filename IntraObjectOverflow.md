One class of bugs currently not properly detected by AddressSanitizer is **intra-object-overflow**:

```
struct S {
  int buffer[5];
  int other_field;
};

void Foo(S *s, int idx) {
  s->buffer[idx] = 0;  // if idx == 5, asan will not complain
}

```


This class of bugs will be detectable with the [Intel MPX](IntelMemoryProtectionExtensions.md).

In the meantime, we are trying the following approach in AddressSanitizer:
if the type is a C++ non-standard-layout class with a destructor, we add poisoned gaps (redzones) between fields in the class.

An early prototype of this functionality is available in clang trunk. Don't expect it to work outside of small tests (yet).
```
% clang++ -O1 -g -fsanitize-address-field-padding=1 -fsanitize=address \
   ~/llvm/projects/compiler-rt/test/asan/TestCases/intra-object-overflow.cc
% ./a.out 11
==23931==ERROR: AddressSanitizer: intra-object-overflow on address 0x60c00000bfd4 at pc 0x00000049d672...
WRITE of size 4 at 0x60c00000bfd4 thread T0
    #0 0x49d671 in Foo::set(int, int) llvm/projects/compiler-rt/test/asan/TestCases/intra-object-overflow.cc:15:30
    #1 0x49d48f in main llvm/projects/compiler-rt/test/asan/TestCases/intra-object-overflow.cc:27:3
    #2 0x7f1edaf47ec4 in __libc_start_main /build/buildd/eglibc-2.19/csu/libc-start.c:287
    #3 0x416d16 in _start (llvm_build/a.out+0x416d16)

0x60c00000bfd4 is located 84 bytes inside of 128-byte region [0x60c00000bf80,0x60c00000c000)
allocated by thread T0 here:
    #0 0x49c7cb in operator new(unsigned long) llvm/projects/compiler-rt/lib/asan/asan_new_delete.cc:62:35
    #1 0x49d475 in main llvm/projects/compiler-rt/test/asan/TestCases/intra-object-overflow.cc:26:3

```