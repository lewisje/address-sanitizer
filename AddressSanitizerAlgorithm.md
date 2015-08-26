

# Short version #
The run-time library replaces the `malloc` and `free` functions.
The memory around malloc-ed regions (red zones) is poisoned.
The `free`-ed memory is placed in quarantine and also poisoned.
Every memory access in the program is transformed by the compiler in the following way:

Before:
```
*address = ...;  // or: ... = *address;
```
After:
```
if (IsPoisoned(address)) {
  ReportError(address, kAccessSize, kIsWrite);
}
*address = ...;  // or: ... = *address;
```

The tricky part is how to implement `IsPoisoned` very fast and `ReportError` very compact.
Also, instrumenting some of the accesses may be [proven redundant](CompileTimeOptimizations.md).


# Memory mapping and Instrumentation #
The virtual address space is divided into 2 disjoint classes:
  * Main application memory (`Mem`): this memory is used by the regular application code.
  * Shadow memory (`Shadow`): this memory contains the shadow values (or metadata).
There is a correspondence between the shadow and the main application memory.
**Poisoning** a byte in the main memory means writing some special value into the corresponding shadow memory.

These 2 classes of memory should be organized in such a way that computing the shadow memory
(`MemToShadow`) is fast.

The instrumentation performed by the compiler:
```
shadow_address = MemToShadow(address);
if (ShadowIsPoisoned(shadow_address)) {
  ReportError(address, kAccessSize, kIsWrite);
}
```

## Mapping ##
AddressSanitizer maps 8 bytes of the application
memory into 1 byte of the shadow memory.

There are only 9 different values for any aligned 8 bytes of the application memory:
  * All 8 bytes in qword are unpoisoned (i.e. addressible). The shadow value is 0.
  * All 8 bytes in qword are poisoned (i.e. not addressible). The shadow value is negative.
  * First `k` bytes are unpoisoned, the rest `8-k` are poisoned. The shadow value is `k`.
This is guaranteed by the fact that `malloc` returns 8-byte aligned chunks of memory.
The only case where different bytes of an aligned qword have different state is the tail of
a malloc-ed region. For example, if we call `malloc(13)`, we will have one full unpoisoned
qword and one qword where 5 first bytes are unpoisoned.


The instrumentation looks like this:
```
byte *shadow_address = MemToShadow(address);
byte shadow_value = *shadow_address;
if (shadow_value) {
  if (SlowPathCheck(shadow_value, address, kAccessSize)) {
    ReportError(address, kAccessSize, kIsWrite);
  }
}
```

```
// Check the cases where we access first k bytes of the qword
// and these k bytes are unpoisoned.
bool SlowPathCheck(shadow_value, address, kAccessSize) {
  last_accessed_byte = (address & 7) + kAccessSize - 1;
  return (last_accessed_byte >= shadow_value);
}
```


`MemToShadow(ShadowAddr)` falls into the `ShadowGap` region
which is unaddressible. So, if the program tries to directly access a memory location
in the shadow region, it will crash.

### 64-bit ###
```
Shadow = (Mem >> 3) + 0x7fff8000;
```
| `[0x10007fff8000, 0x7fffffffffff]` | `HighMem`    |
|:-----------------------------------|:-------------|
| `[0x02008fff7000, 0x10007fff7fff]` | `HighShadow` |
| `[0x00008fff7000, 0x02008fff6fff]` | `ShadowGap`  |
| `[0x00007fff8000, 0x00008fff6fff]` | `LowShadow`  |
| `[0x000000000000, 0x00007fff7fff]` | `LowMem`     |

### 32 bit ###
```
Shadow = (Mem >> 3) + 0x20000000;
```
| `[0x40000000, 0xffffffff]` | `HighMem`          |
|:---------------------------|:-------------------|
| `[0x28000000, 0x3fffffff]` | `HighShadow`       |
| `[0x24000000, 0x27ffffff]` | `ShadowGap`        |
| `[0x20000000, 0x23ffffff]` | `LowShadow`        |
| `[0x00000000, 0x1fffffff]` | `LowMem`           |

## Ultra compact shadow ##
It is possible to use even more compact shadow memory, e.g.
```
Shadow = (Mem >> 7) | kOffset;
```
Experiments are in flight.

## Report Error ##
The `ReportError` could be implemented as a call (this is the default now),
but there are some other, slightly more efficient and/or more compact solutions.
At some point the default behaviour **was**:
  * copy the failure address to `%rax` (`%eax`).
  * execute `ud2` (generates SIGILL)
  * Encode access type and size in a one-byte instruction which follows `ud2`.
Overal these 3 instructions require 5-6 bytes of machine code.

It is possible to use just a single instruction (e.g. `ud2`), but this will require
to have a full disassembler in the run-time library (or some other hacks).

## Stack ##
In order to catch stack buffer overflow, AddressSanitizer instruments the code like this:

Original code:
```
void foo() {
  char a[8];
  ...
  return;
}
```
Instrumented code:
```
void foo() {
  char redzone1[32];  // 32-byte aligned
  char a[8];          // 32-byte aligned
  char redzone2[24];
  char redzone3[32];  // 32-byte aligned
  int  *shadow_base = MemToShadow(redzone1);
  shadow_base[0] = 0xffffffff;  // poison redzone1
  shadow_base[1] = 0xffffff00;  // poison redzone2, unpoison 'a'
  shadow_base[2] = 0xffffffff;  // poison redzone3
  ...
  shadow_base[0] = shadow_base[1] = shadow_base[2] = 0; // unpoison all
  return;
}
```

## Examples of instrumented code (x86\_64) ##

```
# long load8(long *a) { return *a; }
0000000000000030 <load8>:
  30:	48 89 f8             	mov    %rdi,%rax
  33:	48 c1 e8 03          	shr    $0x3,%rax
  37:	80 b8 00 80 ff 7f 00 	cmpb   $0x0,0x7fff8000(%rax)
  3e:	75 04                	jne    44 <load8+0x14>
  40:	48 8b 07             	mov    (%rdi),%rax   <<<<<< original load
  43:	c3                   	retq   
  44:	52                   	push   %rdx
  45:	e8 00 00 00 00       	callq  __asan_report_load8
```
```
# int  load4(int *a)  { return *a; }
0000000000000000 <load4>:
   0:	48 89 f8             	mov    %rdi,%rax
   3:	48 89 fa             	mov    %rdi,%rdx
   6:	48 c1 e8 03          	shr    $0x3,%rax
   a:	83 e2 07             	and    $0x7,%edx
   d:	0f b6 80 00 80 ff 7f 	movzbl 0x7fff8000(%rax),%eax
  14:	83 c2 03             	add    $0x3,%edx
  17:	38 c2                	cmp    %al,%dl
  19:	7d 03                	jge    1e <load4+0x1e>
  1b:	8b 07                	mov    (%rdi),%eax    <<<<<< original load
  1d:	c3                   	retq   
  1e:	84 c0                	test   %al,%al
  20:	74 f9                	je     1b <load4+0x1b>
  22:	50                   	push   %rax
  23:	e8 00 00 00 00       	callq  __asan_report_load4
```

## Unaligned accesses ##
The current compact mapping will not catch unaligned partially out-of-bound accesses:
```
int *x = new int[2]; // 8 bytes: [0,7].
int *u = (int*)((char*)x + 6);
*u = 1;  // Access to range [6-9]
```

A viable solution is described in http://code.google.com/p/address-sanitizer/issues/detail?id=100 but it comes at a performance cost.


# Run-time library #
## Malloc ##
The run-time library replaces `malloc`/`free` and provides error reporting functions like `__asan_report_load8`.

`malloc` allocates the requested amount of memory with redzones around it.
The shadow values corresponding to the redzones are poisoned
and the shadow values for the main memory region are cleared.

`free` poisons shadow values for the entire region and puts the chunk of memory
into a quarantine queue (such that this chunk
will not be returned again by malloc during some period of time).

# Comments? #
Send comments to address-sanitizer@googlegroups.com

